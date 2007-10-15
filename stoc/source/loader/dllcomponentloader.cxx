/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dllcomponentloader.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 12:01:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include <stdlib.h>
#include <osl/file.h>
#include <vector>
#include <osl/mutex.hxx>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_SHLIB_HXX_
#include <cppuhelper/shlib.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX__
#include <cppuhelper/implementationentry.hxx>
#endif
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#define SERVICENAME "com.sun.star.loader.SharedLibrary"
#define IMPLNAME    "com.sun.star.comp.stoc.DLLComponentLoader"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::loader;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace rtl;
using namespace osl;

extern rtl_StandardModuleCount g_moduleCount;

namespace stoc_bootstrap
{
Sequence< OUString > loader_getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
            pNames = &seqNames;
        }
    }
    return *pNames;
}

OUString loader_getImplementationName()
{
    static OUString *pImplName = 0;
    if( ! pImplName )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pImplName )
        {
            static OUString implName( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME ) );
            pImplName = &implName;
        }
    }
    return *pImplName;
}
}

namespace stoc_loader
{
//*************************************************************************
// DllComponentLoader
//*************************************************************************
class DllComponentLoader
    : public WeakImplHelper3< XImplementationLoader,
                              XInitialization,
                              XServiceInfo >
{
public:
    DllComponentLoader( const Reference<XComponentContext> & xCtx );
    ~DllComponentLoader();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XImplementationLoader
    virtual Reference<XInterface> SAL_CALL activate( const OUString& implementationName, const OUString& implementationLoaderUrl, const OUString& locationUrl, const Reference<XRegistryKey>& xKey ) throw(CannotActivateFactoryException, RuntimeException);
    virtual sal_Bool SAL_CALL writeRegistryInfo( const Reference<XRegistryKey>& xKey, const OUString& implementationLoaderUrl, const OUString& locationUrl ) throw(CannotRegisterImplementationException, RuntimeException);

private:
    OUString expand_url( OUString const & url )
        SAL_THROW( (RuntimeException) );

    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
DllComponentLoader::DllComponentLoader( const Reference<XComponentContext> & xCtx )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_xSMgr.set( xCtx->getServiceManager(), UNO_QUERY );
}

//*************************************************************************
DllComponentLoader::~DllComponentLoader()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

//*************************************************************************
OUString SAL_CALL DllComponentLoader::getImplementationName(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return stoc_bootstrap::loader_getImplementationName();
}

//*************************************************************************
sal_Bool SAL_CALL DllComponentLoader::supportsService( const OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL DllComponentLoader::getSupportedServiceNames(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return stoc_bootstrap::loader_getSupportedServiceNames();
}

//*************************************************************************
void DllComponentLoader::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( 0, "dllcomponentloader::initialize should not be called !" );
//      if( aArgs.getLength() != 1 )
//      {
//          throw IllegalArgumentException();
//      }

//      Reference< XMultiServiceFactory > rServiceManager;

//      if( aArgs.getConstArray()[0].getValueType().getTypeClass() == TypeClass_INTERFACE )
//      {
//          aArgs.getConstArray()[0] >>= rServiceManager;
//      }

//      if( !rServiceManager.is() )
//      {
//          throw IllegalArgumentException();
//      }

//      m_xSMgr = rServiceManager;
}

//==================================================================================================
OUString DllComponentLoader::expand_url( OUString const & url )
    SAL_THROW( (RuntimeException) )
{
    try
    {
        return cppu::bootstrap_expandUri( url );
    }
    catch ( IllegalArgumentException & e )
    {
        throw RuntimeException( e.Message, e.Context );
    }
}

//*************************************************************************
Reference<XInterface> SAL_CALL DllComponentLoader::activate(
    const OUString & rImplName, const OUString &, const OUString & rLibName,
    const Reference< XRegistryKey > & xKey )

    throw(CannotActivateFactoryException, RuntimeException)
{
    return loadSharedLibComponentFactory(
        expand_url( rLibName ), OUString(), rImplName, m_xSMgr, xKey );
}


//*************************************************************************
sal_Bool SAL_CALL DllComponentLoader::writeRegistryInfo(
    const Reference< XRegistryKey > & xKey, const OUString &, const OUString & rLibName )

    throw(CannotRegisterImplementationException, RuntimeException)
{
    writeSharedLibComponentInfo(
        expand_url( rLibName ), OUString(), m_xSMgr, xKey );
    return sal_True;
}
}

namespace stoc_bootstrap
{
//*************************************************************************
Reference<XInterface> SAL_CALL DllComponentLoader_CreateInstance( const Reference<XComponentContext> & xCtx ) throw(Exception)
{
    Reference<XInterface> xRet;

    XImplementationLoader *pXLoader = (XImplementationLoader *)new stoc_loader::DllComponentLoader(xCtx);

    if (pXLoader)
    {
        xRet = Reference<XInterface>::query(pXLoader);
    }

    return xRet;
}

}

