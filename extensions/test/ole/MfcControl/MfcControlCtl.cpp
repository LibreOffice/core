// MfcControlCtl.cpp : Implementation of the CMfcControlCtrl ActiveX Control class.

#include "stdafx.h"
#include "MfcControl.h"
#include "MfcControlCtl.h"
#include "MfcControlPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMfcControlCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CMfcControlCtrl, COleControl)
    //{{AFX_MSG_MAP(CMfcControlCtrl)
    // NOTE - ClassWizard will add and remove message map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CMfcControlCtrl, COleControl)
    //{{AFX_DISPATCH_MAP(CMfcControlCtrl)
    DISP_FUNCTION(CMfcControlCtrl, "inShort", inShort, VT_I2, VTS_I2)
    DISP_FUNCTION(CMfcControlCtrl, "inLong", inLong, VT_I4, VTS_I4)
    DISP_FUNCTION(CMfcControlCtrl, "inString", inString, VT_BSTR, VTS_PBSTR)
    DISP_FUNCTION(CMfcControlCtrl, "inFloat", inFloat, VT_R4, VTS_R4)
    DISP_FUNCTION(CMfcControlCtrl, "inDouble", inDouble, VT_R8, VTS_R8)
    DISP_FUNCTION(CMfcControlCtrl, "inVariant", inVariant, VT_VARIANT, VTS_VARIANT)
    DISP_FUNCTION(CMfcControlCtrl, "inObject", inObject, VT_DISPATCH, VTS_DISPATCH)
    DISP_FUNCTION(CMfcControlCtrl, "outShort", outShort, VT_EMPTY, VTS_PI2)
    DISP_FUNCTION(CMfcControlCtrl, "outLong", outLong, VT_EMPTY, VTS_PI4)
    DISP_FUNCTION(CMfcControlCtrl, "outString", outString, VT_EMPTY, VTS_PBSTR)
    DISP_FUNCTION(CMfcControlCtrl, "outFloat", outFloat, VT_EMPTY, VTS_PR4)
    DISP_FUNCTION(CMfcControlCtrl, "outDouble", outDouble, VT_EMPTY, VTS_PR8)
    DISP_FUNCTION(CMfcControlCtrl, "outVariant", outVariant, VT_EMPTY, VTS_PVARIANT)
    DISP_FUNCTION(CMfcControlCtrl, "outObject", outObject, VT_EMPTY, VTS_PDISPATCH)
    //}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CMfcControlCtrl, COleControl)
    //{{AFX_EVENT_MAP(CMfcControlCtrl)
    // NOTE - ClassWizard will add and remove event map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CMfcControlCtrl, 1)
    PROPPAGEID(CMfcControlPropPage::guid)
END_PROPPAGEIDS(CMfcControlCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CMfcControlCtrl, "MFCCONTROL.MfcControlCtrl.1",
    0xac221fb6, 0xa0d8, 0x11d4, 0x83, 0x3b, 0, 0x50, 0x4, 0x52, 0x6a, 0xb4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CMfcControlCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DMfcControl =
        { 0xac221fb4, 0xa0d8, 0x11d4, { 0x83, 0x3b, 0, 0x50, 0x4, 0x52, 0x6a, 0xb4 } };
const IID BASED_CODE IID_DMfcControlEvents =
        { 0xac221fb5, 0xa0d8, 0x11d4, { 0x83, 0x3b, 0, 0x50, 0x4, 0x52, 0x6a, 0xb4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwMfcControlOleMisc =
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CMfcControlCtrl, IDS_MFCCONTROL, _dwMfcControlOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CMfcControlCtrl::CMfcControlCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CMfcControlCtrl

BOOL CMfcControlCtrl::CMfcControlCtrlFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Verify that your control follows apartment-model threading rules.
    // Refer to MFC TechNote 64 for more information.
    // If your control does not conform to the apartment-model rules, then
    // you must modify the code below, changing the 6th parameter from
    // afxRegApartmentThreading to 0.

    if (bRegister)
        return AfxOleRegisterControlClass(
            AfxGetInstanceHandle(),
            m_clsid,
            m_lpszProgID,
            IDS_MFCCONTROL,
            IDB_MFCCONTROL,
            afxRegApartmentThreading,
            _dwMfcControlOleMisc,
            _tlid,
            _wVerMajor,
            _wVerMinor);
    else
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlCtrl::CMfcControlCtrl - Constructor

CMfcControlCtrl::CMfcControlCtrl()
{
    InitializeIIDs(&IID_DMfcControl, &IID_DMfcControlEvents);

    // TODO: Initialize your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlCtrl::~CMfcControlCtrl - Destructor

CMfcControlCtrl::~CMfcControlCtrl()
{
    // TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlCtrl::OnDraw - Drawing function

void CMfcControlCtrl::OnDraw(
            CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
    // TODO: Replace the following code with your own drawing code.
    pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
    pdc->Ellipse(rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlCtrl::DoPropExchange - Persistence support

void CMfcControlCtrl::DoPropExchange(CPropExchange* pPX)
{
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);

    // TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlCtrl::OnResetState - Reset control to default state

void CMfcControlCtrl::OnResetState()
{
    COleControl::OnResetState();  // Resets defaults found in DoPropExchange

    // TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CMfcControlCtrl message handlers


short CMfcControlCtrl::inShort(short val)
{
    char buf[256];
    sprintf( buf, "inByte: value= %d", val);
    ::MessageBoxA( NULL, buf, "MFCCONTROL.MfcControl", MB_OK);
    return val+1;
}

long CMfcControlCtrl::inLong(long val)
{
    char buf[256];
    sprintf( buf, "inLong: value= %d", val);
    ::MessageBoxA( NULL, buf, "MFCCONTROL.MfcControl", MB_OK);
    return val+1;
}

BSTR CMfcControlCtrl::inString(BSTR* val)
{
    CString strResult;
    strResult= *val;
    char buf[256];
    sprintf( buf, "inString: value= %S", *val);
    ::MessageBoxA( NULL, buf, "MFCCONTROL.MfcControl", MB_OK);
    strResult += L" an appended string";
    return strResult.AllocSysString();
}

float CMfcControlCtrl::inFloat(float val)
{
    char buf[256];
    sprintf( buf, "inFloat: value= %f", val);
    ::MessageBoxA( NULL, buf, "MFCCONTROL.MfcControl", MB_OK);
    return val+1;
}

double CMfcControlCtrl::inDouble(double val)
{
    char buf[256];
    sprintf( buf, "inDouble: value= %g", val);
    ::MessageBoxA( NULL, buf, "MFCCONTROL.MfcControl", MB_OK);
    return val+1;
}

VARIANT CMfcControlCtrl::inVariant(const VARIANT FAR& val)
{
    VARIANT vaResult;
    VariantInit(&vaResult);
    VariantCopyInd( &vaResult, const_cast<VARIANT*>(&val));
    if( vaResult.vt == VT_BSTR)
    {
        char buf[256];
        sprintf( buf, "inVariant: value= %S", vaResult.bstrVal);
        ::MessageBoxA( NULL, buf, "MFCCONTROL.MfcControl", MB_OK);

    }
    return _variant_t( L" a string from CMfcControlCtrl::inVariant");
}

LPDISPATCH CMfcControlCtrl::inObject(LPDISPATCH val)
{
    char buf[256];
    _bstr_t bstr;
    HRESULT hr= S_OK;
    COleVariant var;
    DISPID id;
    OLECHAR* name=L"prpString";
    if( SUCCEEDED(hr= val->GetIDsOfNames( IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &id)))
    {
        DISPPARAMS params={0,0,0,0};
        hr= val->Invoke( id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &var,0,0);

    }

    if( var.vt== VT_BSTR)
        bstr= var.bstrVal;
    sprintf( buf, "inObject: value= %S", (wchar_t*)bstr);
    ::MessageBoxA( NULL, buf, "MFCCONTROL.MfcControl", MB_OK);

    return NULL;
}


void CMfcControlCtrl::outShort(short* val)
{
    *val= 123;
}

void CMfcControlCtrl::outLong(long* val)
{
    *val= 1234;
}

void CMfcControlCtrl::outString(BSTR FAR* val)
{
    *val= SysAllocString(L"A string from CMfcControlCtrl::outString ");
}

void CMfcControlCtrl::outFloat(float* val)
{
    *val= 3.14f;
}

void CMfcControlCtrl::outDouble(double* val)
{
    *val= 3.145;
}

void CMfcControlCtrl::outVariant(VARIANT FAR* val)
{
    VariantInit( val);
    val->vt= VT_BSTR;
    val->bstrVal= SysAllocString( L"a string in a VARIANT");
}

void CMfcControlCtrl::outObject(LPDISPATCH FAR* val)
{
    //{BFE10EBE-8584-11D4-005004526AB4}
    HRESULT hr= S_OK;
    CLSID clsTestControl;
    hr= CLSIDFromProgID( L"AxTestComponents.Basic", &clsTestControl);

    IDispatch* pDisp= NULL;
    hr= CoCreateInstance( clsTestControl, NULL, CLSCTX_ALL, __uuidof(IDispatch), (void**)&pDisp);

    if( SUCCEEDED( hr) && val)
    {
        COleVariant var;
        DISPID id;
        OLECHAR* name=L"prpString";
        if( SUCCEEDED(hr= pDisp->GetIDsOfNames( IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &id)))
        {
            COleVariant vaParam1(_T("this is property prpString of AxTestComponents.Basic"));
            DISPID dispidPut= DISPID_PROPERTYPUT;
            DISPPARAMS params;
            params.cArgs= 1;
            params.cNamedArgs= 1;
            params.rgdispidNamedArgs= &dispidPut;
            params.rgvarg= &vaParam1;

            hr= pDisp->Invoke( id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, &var,0,0);
            *val= pDisp;
        }

    }

}
// VT_I1
