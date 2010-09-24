#if !defined(AFX_MFCCONTROLCTL_H__AC221FC3_A0D8_11D4_833B_005004526AB4__INCLUDED_)
#define AFX_MFCCONTROLCTL_H__AC221FC3_A0D8_11D4_833B_005004526AB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MfcControlCtl.h : Declaration of the CMfcControlCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CMfcControlCtrl : See MfcControlCtl.cpp for implementation.

class CMfcControlCtrl : public COleControl
{
    DECLARE_DYNCREATE(CMfcControlCtrl)

// Constructor
public:
    CMfcControlCtrl();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMfcControlCtrl)
    public:
    virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
    virtual void DoPropExchange(CPropExchange* pPX);
    virtual void OnResetState();
    //}}AFX_VIRTUAL

// Implementation
protected:
    ~CMfcControlCtrl();

    DECLARE_OLECREATE_EX(CMfcControlCtrl)    // Class factory and guid
    DECLARE_OLETYPELIB(CMfcControlCtrl)      // GetTypeInfo
    DECLARE_PROPPAGEIDS(CMfcControlCtrl)     // Property page IDs
    DECLARE_OLECTLTYPE(CMfcControlCtrl)     // Type name and misc status

// Message maps
    //{{AFX_MSG(CMfcControlCtrl)
        // NOTE - ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

// Dispatch maps
    //{{AFX_DISPATCH(CMfcControlCtrl)
    afx_msg short inShort(short val);
    afx_msg long inLong(long val);
    afx_msg BSTR inString(BSTR* val);
    afx_msg float inFloat(float val);
    afx_msg double inDouble(double val);
    afx_msg VARIANT inVariant(const VARIANT FAR& val);
    afx_msg LPDISPATCH inObject(LPDISPATCH val);
    afx_msg void outShort(short* val);
    afx_msg void outLong(long* val);
    afx_msg void outString(BSTR FAR* val);
    afx_msg void outFloat(float* val);
    afx_msg void outDouble(double* val);
    afx_msg void outVariant(VARIANT FAR* val);
    afx_msg void outObject(LPDISPATCH FAR* val);
    //}}AFX_DISPATCH
    DECLARE_DISPATCH_MAP()

// Event maps
    //{{AFX_EVENT(CMfcControlCtrl)
    //}}AFX_EVENT
    DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
    enum {
    //{{AFX_DISP_ID(CMfcControlCtrl)
    dispidInShort = 1L,
    dispidInLong = 2L,
    dispidInString = 3L,
    dispidInFloat = 4L,
    dispidInDouble = 5L,
    dispidInVariant = 6L,
    dispidInObject = 7L,
    dispidOutShort = 8L,
    dispidOutLong = 9L,
    dispidOutString = 10L,
    dispidOutFloat = 11L,
    dispidOutDouble = 12L,
    dispidOutVariant = 13L,
    dispidOutObject = 14L,
    //}}AFX_DISP_ID
    };
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCCONTROLCTL_H__AC221FC3_A0D8_11D4_833B_005004526AB4__INCLUDED)
// BYTE
