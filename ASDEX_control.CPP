/************************************************* *****************************
* Compiler: Microsoft Visual Studio 2013
*
* Kuzmin, QUEST tokamak, Date : 2015.01.24
* Summary : Control of the ASDEX Gauge, sensitivity, filament current, cut off frequency and such.
*
********************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "resource.h"
#include "U2GPLIB.H"
#include "fstream"
#include <ctime>
#include <iostream>
#include <string>

WORD		MyGpibAdrs;		// USB-GPIB controller address (default is 0)
char		GpAdrsBuf[12];	// GPIB device (ASDEX gauge controller) address (default is 1)
char 		szBuf[256];	 	// working buffer
HINSTANCE	hInstance;		// Instance handler

HWND		hwnd_main;
HWND		hwnd_view;

typedef struct _STEP_PARAM {
	double		current;
	double		tim;
	BYTE		ramp;
} STEP_PARAM, *PSTEP_PARAM;

HANDLE		hThread;
BYTE		stopflg;

CRITICAL_SECTION crisec;
DWORD		g_time = 0;


// Prototype functions declaration
int SendCommand(PCHAR, PCHAR);
int SendQuery(PCHAR, PCHAR, PCHAR, int);
int SendQueryforBinary(PCHAR, PCHAR, PCHAR, int);

std::string lead_zeros(int value, int zeros)
{
	std::string s = std::to_string(value);
	std::string dest = std::string(zeros-s.length(), '0').append(s);
	return dest;
}

int logmsg(char* msg)
// writes logmessage to the text log file
{
	CreateDirectory("AG_log", NULL);
	// test output to a file
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);

	std::ofstream outfile;
	outfile.open(".\\AG_log\\asdex_log.txt", std::ios_base::app);
	outfile << (now->tm_year + 1900) << '-';
	outfile << lead_zeros((now->tm_mon + 1),2) << '-';
	outfile << lead_zeros(now->tm_mday,2) << ' ';
	outfile << lead_zeros(now->tm_hour,2) << ':';
	outfile << lead_zeros(now->tm_min,2) << ':';
	outfile << lead_zeros(now->tm_sec,2) << ' ';
	//outfile << "Initialization\n";
	outfile << msg;
	outfile << "\n";
	return 0;
}

/******************************************************************************
 *					SendCommand
 ******************************************************************************
 * Sends command to the specified equipment
 * Argument
 * GpAdrs - partner device address
 * Cmd - send command (ASCII)
 * Return value
 * - Return value of gp_wrt ()
 *****************************************************************************/
int SendCommand(PCHAR GpAdrs, PCHAR Cmd)
{
	// Command transmission
	return gp_wrt(GpAdrs, Cmd);
}

/******************************************************************************
 *					SendQuery
 ******************************************************************************
 * Sends a query to the specified equipment
 * Argument
 * GpAdrs - partner device address
 * Cmd - send query (ASCII)
 * PBuf - pointer of the receive buffer
 * Len - size of the receive buffer
 * Return value
 * Return value of gp_red ()
 *****************************************************************************/
int SendQuery(PCHAR GpAdrs, PCHAR Cmd, PCHAR pBuf, int Len)
{
	BOOL		Status; // shows the return value of the function

	// Submit Query
	Status = gp_wrt(GpAdrs, Cmd);

	// Keep buffer initialization.
	memset(pBuf, 0x00, Len);
	// And receive a response to the query
	return gp_red(GpAdrs, pBuf, Len);
}


/******************************************************************************
 *					SendQueryforBinary
 ******************************************************************************
* Sends a query to the specified equipment. Use if you want to receive binary data.
*
* Argument
* GpAdrs - partner device address
* Cmd - send query (ASCII)
* PBuf - pointer of the receive buffer
* Len - size of the receive buffer
* Return value
* Returns value of gp_tfrin ()
******************************************************************************/
int SendQueryforBinary(PCHAR GpAdrs, PCHAR Cmd, PCHAR pBuf, int Len)
{
	BOOL		Status; // I shows the return value of the function

	
	// クエリ送信
	Status = gp_wrt(GpAdrs, Cmd);

	// クエリに対する応答を受信
	return gp_tfrin(GpAdrs, Len, pBuf);
}


/******************************************************************************
 *					Cmd_OnCmdProStop
 ******************************************************************************
 * The stop sequence execution
* Argument
* Hwnd - dialog handle
* Return value
* - None
*****************************************************************************/
void Cmd_OnCmdProStop(HWND hwnd)
{

	EnableWindow(GetDlgItem(hwnd, IDB_TXDATA), TRUE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_INS), TRUE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_RUN), TRUE);
	EnableWindow(GetDlgItem(hwnd, IDCANCEL), TRUE);

	// Wait until you get the ownership of a critical section object
	EnterCriticalSection(&crisec);
	// Flag set
	stopflg = 1;
	// The release ownership of the critical section object
	LeaveCriticalSection(&crisec);

}

/******************************************************************************
*					Cmd_On_AG_Status
******************************************************************************
* Query Asdex gauge current settings. IDC_AGSTATUS
* Hwnd - dialog handle
* Return value
* - None
*
*****************************************************************************/
void Cmd_On_AG_Status(HWND hwnd)
{
	BOOL		Status; // Send function result will be passed to this variable.
	char		TxBuf[256]; // Transmission buffer
	char		RxBuf[512]; //Receiving buffer
	std::string		msg;

	sprintf(TxBuf, "FI??");
	Status = SendQuery(GpAdrsBuf, TxBuf, RxBuf, sizeof(RxBuf));
	if (Status != 0)
	{
		sprintf(szBuf, "Receive error: %d ", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}
	// At the receiving complete, displays the received data
	SetDlgItemText(hwnd, IDC_EDIT_FIV, RxBuf);
	msg += RxBuf;
	msg.erase(msg.size() - 1);

	sprintf(TxBuf, "FM??");
	Status = SendQuery(GpAdrsBuf, TxBuf, RxBuf, sizeof(RxBuf));
	if (Status != 0)
	{
		sprintf(szBuf, "Receive error: %d ", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}
	// At the receiving complete, displays the received data
	SetDlgItemText(hwnd, IDC_EDIT_FMV, RxBuf);
	msg += RxBuf;
	msg.erase(msg.size() - 1);

	sprintf(TxBuf, "EC??");
	Status = SendQuery(GpAdrsBuf, TxBuf, RxBuf, sizeof(RxBuf));
	if (Status != 0)
	{
		sprintf(szBuf, "Receive error: %d ", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}
	// At the receiving complete, displays the received data
	SetDlgItemText(hwnd, IDC_EDIT_ECV, RxBuf);
	msg += RxBuf;
	msg.erase(msg.size() - 1);

	sprintf(TxBuf, "CF??");
	Status = SendQuery(GpAdrsBuf, TxBuf, RxBuf, sizeof(RxBuf));
	if (Status != 0)
	{
		sprintf(szBuf, "Receive error: %d ", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}
	// At the receiving complete, displays the received data
	SetDlgItemText(hwnd, IDC_EDIT_CFV, RxBuf);
	msg += RxBuf;
	msg.erase(msg.size() - 1);

	sprintf(TxBuf, "GI??");
	Status = SendQuery(GpAdrsBuf, TxBuf, RxBuf, sizeof(RxBuf));
	if (Status != 0)
	{
		sprintf(szBuf, "Receive error: %d ", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}
	// At the receiving complete, displays the received data
	SetDlgItemText(hwnd, IDC_EDIT_GIV, RxBuf);
	msg += RxBuf;
	msg.erase(msg.size() - 1);

	sprintf(TxBuf, "SI??");
	Status = SendQuery(GpAdrsBuf, TxBuf, RxBuf, sizeof(RxBuf));
	if (Status != 0)
	{
		sprintf(szBuf, "Receive error: %d ", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}
	// At the receiving complete, displays the received data
	SetDlgItemText(hwnd, IDC_EDIT_SIV, RxBuf);
	msg += RxBuf;
	msg.erase(msg.size() - 1);

	SetDlgItemText(hwnd, IDT_STATUS, "Gauge monitor updated.");

	//converting std::string to char*
	char *cstr = new char[msg.length() + 1];
	strcpy(cstr, msg.c_str());

	logmsg(cstr);
}

/******************************************************************************
*					Cmd_On_Stop
******************************************************************************
* Stops ASDEX gauge measurment.
* Hwnd - dialog handle
* Return value
* - None
*
*****************************************************************************/

void Cmd_On_Stop(HWND hwnd)
{
	BOOL		Status; // Send function result will be passed to this variable.

	Status = SendCommand(GpAdrsBuf, "RST");
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}
	// After successfull transmition display accepted command
	Cmd_On_AG_Status(hwnd);
	SetDlgItemText(hwnd, IDT_STATUS, "Gauge STOPped, RST command sent.");
	
	EnableWindow(GetDlgItem(hwnd, IDC_START_AG), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDB_TXDATA), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_INS), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_RUN), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_STOP), FALSE);

	EnableWindow(GetDlgItem(hwnd, IDC_AGSTATUS), TRUE);
	EnableWindow(GetDlgItem(hwnd, IDB_INIT), TRUE);

}

/******************************************************************************
*					Cmd_On_AG_Start
******************************************************************************
* Sends initializing commands to start AG measurments. Simply copy past same sequence with
  given commands as a string. Could be optimized, but why? It works already!
* Hwnd - dialog handle
* Return value
* - None
*
*****************************************************************************/
void Cmd_On_AG_Start(HWND hwnd)
{
	BOOL		Status; // Send function result will be passed to this variable.
	char		TxBuf[256]; // Transmission buffer
	char		fm[256]; // max filament current
	char		ec[256]; // emission current
	char		si[256]; // sensitivity
	char		gi[256]; // gain
	char		cf[256]; // control frequency

	SetDlgItemText(hwnd, IDT_STATUS, "starting..");

	// 1st ASDEX gauge, at the wall
	if (SendMessage(GetDlgItem(hwnd, IDC_AG1), BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		sprintf(fm, "FM = 16.0");
		sprintf(ec, "EC = 0.500");
		sprintf(si, "SI = 1");
		sprintf(gi, "GI = 3");
		sprintf(cf, "CF = 01000");
	}
	// 2st ASDEX gauge, at the bottom divertor plate
	if (SendMessage(GetDlgItem(hwnd, IDC_AG2), BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		sprintf(fm, "FM = 16.0");
		sprintf(ec, "EC = 0.500");
		sprintf(si, "SI = 1");
		sprintf(gi, "GI = 4");
		sprintf(cf, "CF = 01150");
	}
	// 3st ASDEX gauge, at the top divertor plate
	if (SendMessage(GetDlgItem(hwnd, IDC_AG3), BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		sprintf(fm, "FM = 18.0");
		sprintf(ec, "EC = 0.500");
		sprintf(si, "SI = 1");
		sprintf(gi, "GI = 3");
		sprintf(cf, "CF = 01150");
	}

	// Initialize Gauge
	sprintf(TxBuf, "RST");
	Status = SendCommand(GpAdrsBuf, TxBuf);
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}

	//sprintf(TxBuf, "FM = 20.0");
	Status = SendCommand(GpAdrsBuf, fm);
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}

	//sprintf(TxBuf, "EC = 0.500");
	Status = SendCommand(GpAdrsBuf, ec);
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}

	//sprintf(TxBuf, "CF = 01000");
	Status = SendCommand(GpAdrsBuf, cf);
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}

	//sprintf(TxBuf, "SI = 1");
	Status = SendCommand(GpAdrsBuf, si);
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}

	//sprintf(TxBuf, "GI = 3");
	Status = SendCommand(GpAdrsBuf, gi);
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}

	sprintf(TxBuf, "SZ");
	Status = SendCommand(GpAdrsBuf, TxBuf);
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}

	sprintf(TxBuf, "ST");
	Status = SendCommand(GpAdrsBuf, TxBuf);
	if (Status != 0)
	{
		sprintf(szBuf, "Command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Command transmission", MB_OK);
		return;
	}

	Cmd_On_AG_Status(hwnd);
	SetDlgItemText(hwnd, IDT_STATUS, "ASDEX Gauge started");

}

/******************************************************************************
 *					Cmd_OnCmdTxData
 ******************************************************************************
* Sends a command to GPIB when send command button is pressed.
* Argument
* Hwnd - dialog handle
* Return value
* - None
*
 *****************************************************************************/
void Cmd_OnCmdTxData(HWND hwnd)
{
	int			TxSize;
	char		TxBuf[256]; // Transmission buffer
	char		RxBuf[512]; //Receiving buffer
	BOOL		Status; // Send function result will be passed to this variable.


	SetDlgItemText(hwnd, IDT_STATUS, "");
	SetDlgItemText(hwnd, IDE_TXDATA, "");
	SetDlgItemText(hwnd, IDE_RXDATA, "");

	// Set the transmit data
	TxSize = GetDlgItemText(hwnd, IDC_TXDATA, TxBuf, sizeof(TxBuf));

	// If not the final data is '?', And command transmission
	if(TxBuf[TxSize-1] != '?')
	{
		// Command transmission
		Status = SendCommand(GpAdrsBuf, TxBuf);	
		if(Status != 0)
		{
			sprintf(szBuf, "Command transmission error: %d", Status);
			MessageBox(hwnd, szBuf,"Command transmission", MB_OK);
			return;
		}
		// In transmission completion, it displays the transmitted data
		SetDlgItemText(hwnd, IDE_TXDATA, TxBuf);
	}
	else // If the final data is '?', And sends the query
	{
		Status = SendQuery(GpAdrsBuf, TxBuf, RxBuf, sizeof(RxBuf));
		if(Status != 0)
		{
			sprintf(szBuf, "Receive error: %d ", Status);
			MessageBox(hwnd, szBuf,"Command transmission", MB_OK);
			return;
		}
		// In transmission completion, it displays the transmitted data
		SetDlgItemText(hwnd, IDE_TXDATA, TxBuf);
		// At the receiving complete, displays the received data
		SetDlgItemText(hwnd, IDE_RXDATA, RxBuf);
	}
	Cmd_On_AG_Status(hwnd);
}


/******************************************************************************
 *					Cmd_OnCmdInit
 ******************************************************************************
 * - When Initialization button is pressed (communicates with the GPIB device by the default addresses)
 * Argument
 * Hwnd - dialog handle
 * Return value
 * - None
 *****************************************************************************/
void Cmd_OnCmdInit(HWND hwnd)
{
	BOOL		Status; // I shows the return value of the function
	char		TxBuf[256]; // Transmission buffer
	char		RxBuf[256]; // Receiving buffer

	SetDlgItemText(hwnd, IDT_STATUS, "");
	EnableWindow(GetDlgItem(hwnd, IDB_INIT), FALSE); // Disable the reset button

	// I put a GPIB device address to GpAdrsBuf
	GetDlgItemText(hwnd, IDE_GPADRS, GpAdrsBuf, sizeof(GpAdrsBuf));

	//test
	SendDlgItemMessage(hwnd, IDC_LED, STM_SETIMAGE, IDB_RED, IDB_RED);
	// GPIB Controller initialization
	Status = gp_init(MyGpibAdrs, 0, 0);
	if(Status != 0)
	{
		sprintf(szBuf, "gp_init() error: %d", Status);
		MessageBox(hwnd, szBuf, "Initialization", MB_OK);
		EnableWindow(GetDlgItem(hwnd, IDB_INIT), TRUE); // Can be used to initialize button
		logmsg(szBuf);
		return;
	}	

	gp_tmout(5); // GPIB Set the bus timeout time to 5 seconds
// Please be the function of the lower set by calling the case to be a communication error	
//	gp_setdelay(500); // ATN The delay time of the switching line is set 500 microseconds

	// Setting of the transmission and reception delimiter
	gp_delm("t", 0x8A); // EOI+LF
	gp_delm("l", 0x0A); // LF

	// IFC Reset
	gp_cli();

	// The REN line is TRUE
	gp_ren();

	// Device clear command transmission
	Status = gp_clr("");
	if(Status != 0)
	{
		sprintf(szBuf, "gp_clr() error: %d", Status);
		MessageBox(hwnd, szBuf, "Initialization", MB_OK);
		EnableWindow(GetDlgItem(hwnd, IDB_INIT), TRUE); // Can be used to initialize button
		return;
	}

	// Standard Event Status Register Read the (SESR), to clear.
	// By reading the SESR, it should be clear an unexpected SRQ
	sprintf(TxBuf, "*ESR?");
	Status = SendQuery(GpAdrsBuf, TxBuf, RxBuf, sizeof(RxBuf));
	if(Status != 0)
	{
		sprintf(szBuf, "SendQuery[%s] error: %d", TxBuf, Status);
		MessageBox(hwnd, szBuf, "Initialization", MB_OK);
		EnableWindow(GetDlgItem(hwnd, IDB_INIT), TRUE); // Can be used to initialize button
		return;
	}

	// Service Request Enable bitをOFF
	sprintf(TxBuf, "*SRE 0");
	Status = SendCommand(GpAdrsBuf, TxBuf);
	if(Status != 0)
	{
		sprintf(szBuf, "Initialization command transmission error: %d", Status);
		MessageBox(hwnd, szBuf, "Initialization", MB_OK);
		EnableWindow(GetDlgItem(hwnd, IDB_INIT), TRUE); // Can be used to initialize button
		return;
	}

	// I want to enable the various buttons
	EnableWindow(GetDlgItem(hwnd, IDB_TXDATA), TRUE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_INS), TRUE); 
	EnableWindow(GetDlgItem(hwnd, IDC_START_AG), TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_STOP), TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_AGSTATUS), TRUE);
	
	Cmd_On_AG_Status(hwnd);
	SetDlgItemText(hwnd, IDT_STATUS, "Initialization successful");
	logmsg("Initialization successful");
}


/******************************************************************************
 *				CenterDialog
 ******************************************************************************
* - The display a dialog in the center
* Argument
* Hwnd - dialog handle
* Return value
* - Always I return TRUE
*
 *****************************************************************************/
BOOL CenterDialog(HWND hwnd)
{
	int			xMid,  yMid;
	int			xLeft, yTop;
	int			Width, Height;
	RECT		rcDlg, rcScreen;


	SetRect(&rcScreen, 0, 0,
		GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	xMid = (rcScreen.left + rcScreen.right) / 2;
	yMid = (rcScreen.top + rcScreen.bottom) / 2;

	GetWindowRect(hwnd, &rcDlg);
	Width = rcDlg.right - rcDlg.left;
	Height= rcDlg.bottom - rcDlg.top;
	xLeft = xMid - Width / 2;
	yTop  = yMid - Height / 2;

	if(xLeft < 0)
		xLeft = 0;
	else if(xLeft + Width > rcScreen.right)
		xLeft = rcScreen.right - Width;

	if(yTop < 0)
		yTop = 0;
	else if(yTop + Height > rcScreen.bottom)
		yTop = rcScreen.bottom - Height;

	SetWindowPos(hwnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER);

	return TRUE;
}


/******************************************************************************
 *					Dlg_OnInitDialog
 ******************************************************************************
  * - Initialization of the main dialog window WM_INITDIALOG.
  * Argument
  * Hwnd - dialog handle
  * The hwndFocus - focused control handle
  * LParam - additional initialization information (not used)
  * Return value
  * -
 *
 *****************************************************************************/
BOOL Dlg_OnInitDialog (HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	BOOL		cnt; // Loop counter
	PCHAR		str[]={
					"RST",
					"FM = 16.0",
					"FM = 18.0",
					"EC = 0.500",
					"CF = 01000",
					"SI = 1",
					"GI = 3",
					"GI = 4",
					"SZ",
					"ST",
					"FI??",
					"CF??",
					"SI??",
					"GI??",
					"EC??",
					0}; // Combo box item for additional


	// And set the dialog icon
	SetClassLong(hwnd, GCL_HICON, (LONG) LoadIcon(GetWindowInstance(hwnd),
                                 MAKEINTRESOURCE(IDI_ICON1)));
	// Dialog to the center of the screen display
	CenterDialog(hwnd);

	// Add an item to the combo box, select the first item
	for(cnt = 0; str[cnt] != 0; cnt++)
		SendDlgItemMessage(hwnd, IDC_TXDATA, CB_ADDSTRING, 0, (LPARAM)str[cnt]);
	// Set the initial item.
	SendDlgItemMessage(hwnd, IDC_TXDATA, CB_SETCURSEL, 1, 0);


	// Disables unnecessary buttons 
	EnableWindow(GetDlgItem(hwnd, IDC_START_AG), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_AGSTATUS), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDB_TXDATA), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_INS), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_RUN), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDB_PRO_STOP), FALSE);

	// ASDEX GPIB default value (should be 1)
	sprintf_s(GpAdrsBuf, "%d", 1);
	SetDlgItemText(hwnd, IDE_GPADRS, GpAdrsBuf);

	// USB-GPIB converter dafault address (should be 0)
	MyGpibAdrs = 0;
	sprintf_s(szBuf, "%d", MyGpibAdrs);
	SetDlgItemText(hwnd, IDE_MYADRS, szBuf);

	// Text box edit box initialization
	SetDlgItemText(hwnd, IDT_STATUS, "");
	SetDlgItemText(hwnd, IDE_TXDATA, "");
	SetDlgItemText(hwnd, IDE_RXDATA, "");

	// Check AG1 radiobutton
	SendMessage(GetDlgItem(hwnd, IDC_AG3), BM_SETCHECK, 1, 0);
	
	return TRUE;
}


/******************************************************************************
 *					Dlg_OnDestroy
 ******************************************************************************
 * - I will do the dialog finalization on WM_DESTROY message.
 * If there is allocated resource in Dlg_OnInitDialog to release here.
 * Argument
 * Hwnd - dialog handle
 * Return value
 * - None
 *
 *****************************************************************************/
void Dlg_OnDestroy(HWND hwnd)
{
}


/******************************************************************************
 *					Cmd_OnCancel
 ******************************************************************************
 * - Processing when the end button is pressed
 * Argument
 * Hwnd - dialog handle
 * Return value
 * - None
 *****************************************************************************/
void Cmd_OnCancel(HWND hwnd)
{

	//Set the timeout to 1 second. (Not connected measures)
	gp_tmout(1);
	// I return the opponent equipment to the local state
	gp_lcl(GpAdrsBuf);
	// I make the REN to FALSE
	gp_lcl("");
	// I exit the main dialog
	EndDialog(hwnd, 0);
}


/******************************************************************************
 *					Dlg_OnCommand
 ******************************************************************************
 *
* Commentary
*
* - The processing the message events related to WM_COMMAND (pressing buttons??)
* Hwnd - dialog handle
* Id - identification ID such as control accelerator
* HwndCtl - of control handle
* CodeNotify - Notification code
*
* Return value
*
* - None
 *
 *****************************************************************************/
void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{

	// This is wrong. When I press start - all is done, 1.start 2. status 3.stop 4. txdata. This I should correct and app will work properly.
	switch(id)
	{
	case IDB_INIT:
		Cmd_OnCmdInit(hwnd);
		break; // and what if i put here return instead? Ah! I forgot break!!!
	case IDC_START_AG:
		Cmd_On_AG_Start(hwnd);
		break;
	case IDC_AGSTATUS:
		Cmd_On_AG_Status(hwnd);
		break;
	case IDC_STOP:
		Cmd_On_Stop(hwnd);
		break;
	case IDB_TXDATA:
		Cmd_OnCmdTxData(hwnd);
		break;
	case IDB_PRO_STOP:
		Cmd_OnCmdProStop(hwnd);
		break;
	case IDCANCEL:
		Cmd_OnCancel(hwnd);
		break;
	}
}


/******************************************************************************
 *					DlgProc_Main
 ******************************************************************************
 *
 * Commentary
 *
* - Dialog callback function
* I will call the appropriate processing function by the message that is sent to the dialog.
* Hwnd - dialog handle
* UMsg - message identification code
* WParam - the first message parameter
* LParam - Second message parameters
* Return value
* - TRUE if possible processing the message, it returns FALSE otherwise.
*****************************************************************************/
BOOL CALLBACK DlgProc_Main(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// Critical section initialization
		InitializeCriticalSection(&crisec);
		Dlg_OnInitDialog(hwnd, (HWND)wParam, lParam);
		return TRUE;
	case WM_COMMAND:
		Dlg_OnCommand(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
		return TRUE;
	case WM_DESTROY:
		// Critical section resources open
		DeleteCriticalSection(&crisec);
		Dlg_OnDestroy(hwnd);
		return TRUE;
	}
	return (FALSE);
}


/******************************************************************************
 *					WinMain
 ******************************************************************************
 *
 * Commentary
 *
 * - Entry point of the program
*
* Argument
*
* HinstExe - of applications current instance handle
* HinstPrev - 32-bit always NULL in Windows
* PszCmdLine - pointer (NULL termination) to the command line specified string
* NCmdShow - window shape specified
*
* Return value
*
* - I always returns 0
 *
 *****************************************************************************/
BOOL APIENTRY WinMain(HINSTANCE hinstExe, HINSTANCE hinstPrev, LPSTR pszCmdLine, BOOL nCmdShow)
{

	// And set the instance handle in a global variable
	hInstance = hinstExe;

	// I want to display the main dialog
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc_Main);

	return	0;
}

// End of File
