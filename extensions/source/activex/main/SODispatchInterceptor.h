// SODispatchInterceptor.h: Definition of the SODispatchInterceptor class
//
//////////////////////////////////////////////////////////////////////

#if !defined __SODISPATCHINTERCEPTOR_H_
#define __SODISPATCHINTERCEPTOR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols
#include <ExDispID.h>
#include <ExDisp.h>
#include <shlguid.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif
#include <atlctl.h>

#include "so_activex.h"

class CSOActiveX;

/////////////////////////////////////////////////////////////////////////////
// SODispatchInterceptor

class SODispatchInterceptor :
    public IDispatchImpl<ISODispatchInterceptor, &IID_ISODispatchInterceptor, &LIBID_SO_ACTIVEXLib>,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<SODispatchInterceptor,&CLSID_SODispatchInterceptor>
{
    CComPtr<IDispatch>  m_xMaster;
    CComPtr<IDispatch>  m_xSlave;
    CSOActiveX*         m_xParentControl;
    CRITICAL_SECTION    mMutex;
public:
    SODispatchInterceptor() : m_xParentControl( NULL ) { InitializeCriticalSection(&mMutex); }
    ~SODispatchInterceptor() { ATLASSERT( !m_xParentControl ); DeleteCriticalSection(&mMutex); }

BEGIN_COM_MAP(SODispatchInterceptor)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISODispatchInterceptor)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(SODispatchInterceptor)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_SODISPATCHINTERCEPTOR)

    void SetParent( CSOActiveX* pParent )
    {
        ATLASSERT( !m_xParentControl );
        EnterCriticalSection( &mMutex );
        m_xParentControl = pParent;
        LeaveCriticalSection( &mMutex );
    }

    void ClearParent()
    {
        EnterCriticalSection( &mMutex );
        m_xParentControl = NULL;
        LeaveCriticalSection( &mMutex );
    }

// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// ISODispatchInterceptor

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getSlaveDispatchProvider(
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retVal)
        {
            *retVal = m_xSlave;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setSlaveDispatchProvider(
            /* [in] */ IDispatch __RPC_FAR *xNewDispatchProvider)
        {
            m_xSlave = xNewDispatchProvider;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getMasterDispatchProvider(
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retVal)
        {
            *retVal = m_xMaster;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setMasterDispatchProvider(
            /* [in] */ IDispatch __RPC_FAR *xNewSupplier)
        {
            m_xMaster = xNewSupplier;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE queryDispatch(
            /* [in] */ IDispatch __RPC_FAR *aURL,
            /* [in] */ BSTR aTargetFrameName,
            /* [in] */ long nSearchFlags,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retVal);

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE queryDispatches(
            /* [in] */ SAFEARRAY __RPC_FAR * aDescripts,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *retVal);

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE dispatch(
            /* [in] */ IDispatch __RPC_FAR *aURL,
            /* [in] */ SAFEARRAY __RPC_FAR * aArgs);

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE addStatusListener(
            /* [in] */ IDispatch __RPC_FAR *xControl,
            /* [in] */ IDispatch __RPC_FAR *aURL);

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeStatusListener(
            /* [in] */ IDispatch __RPC_FAR *xControl,
            /* [in] */ IDispatch __RPC_FAR *aURL);

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getInterceptedURLs(
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pVal);

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Bridge_implementedInterfaces(
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pVal)
        {
            *pVal = SafeArrayCreateVector( VT_BSTR, 0, 4 );

            if( !*pVal )
                return E_FAIL;

            long ix = 0;
            CComBSTR aInterface( OLESTR( "com.sun.star.frame.XDispatchProviderInterceptor" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            ix = 1;
            aInterface = CComBSTR( OLESTR( "com.sun.star.frame.XDispatchProvider" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            ix = 2;
            aInterface = CComBSTR( OLESTR( "com.sun.star.frame.XDispatch" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            ix = 3;
            aInterface = CComBSTR( OLESTR( "com.sun.star.frame.XInterceptorInfo" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            return S_OK;
        }
};

#endif // __SODISPATCHINTERCEPTOR_H_
