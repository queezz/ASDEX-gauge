/*
 ***************************************************************************
 *
 *	Module Name : U2GPLIB.H
 *
 ***************************************************************************
 */


// Interruptions
#define 	WM_GPIB			  			WM_APP + 0x1000
#define 	GPIB_EVENTSTART  			0
#define 	GPIB_EVENTOCCUR				1
#define 	GPIB_EVENTSTOP				2
#define 	GPIB_EVENTERROR				3

#define 	DISABLE_SRQ_INTERRUPT		FALSE
#define 	ENABLE_SRQ_INTERRUPT		TRUE


#define DllImport	__declspec( dllimport )
#pragma comment(lib, "U2GPLIB")
/*
 *	Ratoc GPIB to USB controller DLL import
 */
#ifdef __cplusplus
extern "C" {
#endif
DllImport WORD APIENTRY OutPort( WORD Reg, WORD OutVal );
DllImport WORD APIENTRY InPort( WORD Reg );

DllImport int APIENTRY gp_init( WORD GpAdrs, WORD IOBase, WORD IrqNo );
DllImport int APIENTRY gp_cli( void );
DllImport int APIENTRY gp_ren( void );
DllImport int APIENTRY gp_clr( PCHAR adrs );
DllImport int APIENTRY gp_wrt( PCHAR adrs, PCHAR buf );
DllImport int APIENTRY gp_red( PCHAR adrs, PCHAR buf, int bufLen );
DllImport int APIENTRY gp_trg( PCHAR adrs );
DllImport int APIENTRY gp_wsrq( int WaitSecTime );
DllImport int APIENTRY gp_wsrqb( int WaitSecTime );
DllImport int APIENTRY gp_rds( PCHAR adrs, PUCHAR status_byte );
DllImport int APIENTRY gp_rds1( PCHAR adrs, PUCHAR status_byte );
DllImport int APIENTRY gp_srq( HWND hWnd, INT SrqMode );
DllImport int APIENTRY gp_lcl( PCHAR adrs );
DllImport int APIENTRY gp_llo( void );
DllImport int APIENTRY gp_tmout( int SecTime );
DllImport int APIENTRY gp_setdelay( int DelayTime );
DllImport int APIENTRY gp_count( void );
DllImport int APIENTRY gp_delm( char *mode, unsigned int dlm );
DllImport int APIENTRY gp_tfrout( PCHAR adrs, int bufLen, PCHAR buf );
DllImport int APIENTRY gp_tfrin( PCHAR adrs, int bufLen, PCHAR buf );
DllImport int APIENTRY gp_tfrinit( PCHAR adrs );
DllImport int APIENTRY gp_tfrins( int bufLen, PCHAR buf );
DllImport void APIENTRY gp_tfrend( void );
DllImport int APIENTRY gp_wtb( PCHAR buf );
DllImport int APIENTRY gp_myadr( void );

DllImport void APIENTRY gp_wait( int WaitSecTime );
DllImport void APIENTRY gp_strtodbl( BYTE *bPoint, double *val );
DllImport void APIENTRY gp_strtoflt( BYTE *bPoint, float *val );

// The following are used when more than one connection
DllImport HANDLE APIENTRY U2GpOpenUnit(USHORT UnitId);
DllImport void APIENTRY U2GpCloseUnit(HANDLE hUnit);
DllImport int APIENTRY U2GpEnumUnit(PUSHORT pUnitId, USHORT MaxUnit);

DllImport WORD APIENTRY U2OutPort( HANDLE hUnit, WORD Reg, WORD OutVal );
DllImport WORD APIENTRY U2InPort( HANDLE hUnit, WORD Reg );

DllImport int APIENTRY U2Gpinit( HANDLE hUnit, WORD GpAdrs );
DllImport int APIENTRY U2Gpcli( HANDLE hUnit );
DllImport int APIENTRY U2Gpren( HANDLE hUnit );
DllImport int APIENTRY U2Gpclr( HANDLE hUnit, PCHAR adrs );
DllImport int APIENTRY U2Gpwrt( HANDLE hUnit, PCHAR adrs, PCHAR buf );
DllImport int APIENTRY U2Gpred( HANDLE hUnit, PCHAR adrs, PCHAR buf, int bufLen );
DllImport int APIENTRY U2Gptrg( HANDLE hUnit, PCHAR adrs );
DllImport int APIENTRY U2Gpwsrq( HANDLE hUnit, int WaitSecTime );
DllImport int APIENTRY U2Gpwsrqb( HANDLE hUnit, int WaitSecTime );
DllImport int APIENTRY U2Gprds( HANDLE hUnit, PCHAR adrs, PUCHAR status_byte );
DllImport int APIENTRY U2Gprds1( HANDLE hUnit, PCHAR adrs, PUCHAR status_byte );
DllImport int APIENTRY U2Gpsrq( HANDLE hUnit, HWND hWnd, INT SrqMode );
DllImport int APIENTRY U2Gplcl( HANDLE hUnit, PCHAR adrs );
DllImport int APIENTRY U2Gpllo( HANDLE hUnit );
DllImport int APIENTRY U2Gptmout( HANDLE hUnit, int SecTime );
DllImport int APIENTRY U2Gpsetdelay( HANDLE hUnit, int DelayTime );
DllImport int APIENTRY U2Gpcount( HANDLE hUnit );
DllImport int APIENTRY U2Gpdelm( HANDLE hUnit, char *mode, unsigned int dlm );
DllImport int APIENTRY U2Gptfrout( HANDLE hUnit, PCHAR adrs, int bufLen, PCHAR buf );
DllImport int APIENTRY U2Gptfrin( HANDLE hUnit, PCHAR adrs, int bufLen, PCHAR buf );
DllImport int APIENTRY U2Gptfrinit( HANDLE hUnit, PCHAR adrs );
DllImport int APIENTRY U2Gptfrins( HANDLE hUnit, int bufLen, PCHAR buf );
DllImport void APIENTRY U2Gptfrend( HANDLE hUnit );
DllImport int APIENTRY U2Gpwtb( HANDLE hUnit, PCHAR buf );
DllImport int APIENTRY U2Gpmyadr( HANDLE hUnit );


// The following additional function
DllImport int APIENTRY gp_srqCheck(void);
DllImport int APIENTRY gp_wrtd(PCHAR buf, int bufLen);
DllImport int APIENTRY gp_tfroutd(int bufLen, PCHAR buf);
DllImport int APIENTRY gp_redd( PCHAR buf, int bufLen );

DllImport int APIENTRY U2GpsrqCheck(HANDLE hUnit);
DllImport int APIENTRY U2Gpwrtd(HANDLE hUnit, PCHAR buf, int bufLen);
DllImport int APIENTRY U2Gptfroutd(HANDLE hUnit, int bufLen, PCHAR buf);
DllImport int APIENTRY U2Gpredd(HANDLE hUnit, PCHAR buf, int bufLen);

DllImport int APIENTRY gp_redah(PCHAR adrs, PCHAR buf, int bufLen);
DllImport int APIENTRY gp_redrst(void);

DllImport int APIENTRY U2Gpredah(HANDLE hUnit, PCHAR adrs, PCHAR buf, int bufLen);
DllImport int APIENTRY U2Gpredrst(HANDLE hUnit);

#ifdef __cplusplus
}
#endif
