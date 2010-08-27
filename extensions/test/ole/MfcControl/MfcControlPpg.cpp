// MfcControlPpg.cpp : Implementation of the CMfcControlPropPage property page class.

#include "stdafx.h"
#include "MfcControl.h"
#include "MfcControlPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMfcControlPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CMfcControlPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CMfcControlPropPage)
    // NOTE - ClassWizard will add and remove message map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CMfcControlPropPage, "MFCCONTROL.MfcControlPropPage.1",
    0xac221fb7, 0xa0d8, 0x11d4, 0x83, 0x3b, 0, 0x50, 0x4, 0x52, 0x6a, 0xb4)


/////////////////////////////////////////////////////////////////////////////
// CMfcControlPropPage::CMfcControlPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CMfcControlPropPage

BOOL CMfcControlPropPage::CMfcControlPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_MFCCONTROL_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlPropPage::CMfcControlPropPage - Constructor

CMfcControlPropPage::CMfcControlPropPage() :
    COlePropertyPage(IDD, IDS_MFCCONTROL_PPG_CAPTION)
{
    //{{AFX_DATA_INIT(CMfcControlPropPage)
    // NOTE: ClassWizard will add member initialization here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlPropPage::DoDataExchange - Moves data between page and properties

void CMfcControlPropPage::DoDataExchange(CDataExchange* pDX)
{
    //{{AFX_DATA_MAP(CMfcControlPropPage)
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlPropPage message handlers
