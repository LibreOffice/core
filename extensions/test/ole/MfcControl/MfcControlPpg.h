#if !defined(AFX_MFCCONTROLPPG_H__AC221FC5_A0D8_11D4_833B_005004526AB4__INCLUDED_)
#define AFX_MFCCONTROLPPG_H__AC221FC5_A0D8_11D4_833B_005004526AB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MfcControlPpg.h : Declaration of the CMfcControlPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CMfcControlPropPage : See MfcControlPpg.cpp.cpp for implementation.

class CMfcControlPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CMfcControlPropPage)
    DECLARE_OLECREATE_EX(CMfcControlPropPage)

// Constructor
public:
    CMfcControlPropPage();

// Dialog Data
    //{{AFX_DATA(CMfcControlPropPage)
    enum { IDD = IDD_PROPPAGE_MFCCONTROL };
        // NOTE - ClassWizard will add data members here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CMfcControlPropPage)
        // NOTE - ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCCONTROLPPG_H__AC221FC5_A0D8_11D4_833B_005004526AB4__INCLUDED)
