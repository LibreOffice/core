#if !defined(AFX_MFCCONTROL_H__AC221FBC_A0D8_11D4_833B_005004526AB4__INCLUDED_)
#define AFX_MFCCONTROL_H__AC221FBC_A0D8_11D4_833B_005004526AB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MfcControl.h : main header file for MFCCONTROL.DLL

#if !defined( __AFXCTL_H__ )
    #error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMfcControlApp : See MfcControl.cpp for implementation.

class CMfcControlApp : public COleControlModule
{
public:
    BOOL InitInstance();
    int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCCONTROL_H__AC221FBC_A0D8_11D4_833B_005004526AB4__INCLUDED)
