/*************************************************************************
 *
 *  $RCSfile: dllcomponentloader.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:35:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdlib.h>
#include <osl/file.h>
#include <vector>
#include <osl/mutex.hxx>

#ifdef MACOSX
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
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

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::loader;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace rtl;

namespace stoc_loader
{

#define SERVICENAME "com.sun.star.loader.SharedLibrary"
#define IMPLNAME    "com.sun.star.comp.stoc.DLLComponentLoader"

//*************************************************************************
// DllComponentLoader
//*************************************************************************
class DllComponentLoader
    : public WeakImplHelper3< XImplementationLoader,
                              XInitialization,
                              XServiceInfo >
{
public:
    DllComponentLoader( const Reference<XMultiServiceFactory> & rXSMgr );
    ~DllComponentLoader();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XImplementationLoader
    virtual Reference<XInterface> SAL_CALL activate( const OUString& implementationName, const OUString& implementationLoaderUrl, const OUString& locationUrl, const Reference<XRegistryKey>& xKey ) throw(CannotActivateFactoryException, RuntimeException);
    virtual sal_Bool SAL_CALL writeRegistryInfo( const Reference<XRegistryKey>& xKey, const OUString& implementationLoaderUrl, const OUString& locationUrl ) throw(CannotRegisterImplementationException, RuntimeException);

private:
    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
DllComponentLoader::DllComponentLoader( const Reference<XMultiServiceFactory> & rXSMgr )
    : m_xSMgr( rXSMgr )
{
}

//*************************************************************************
DllComponentLoader::~DllComponentLoader()
{
}

//*************************************************************************
OUString SAL_CALL DllComponentLoader::getImplementationName(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
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
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL DllComponentLoader::getSupportedServiceNames_Static(  )
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}

//*************************************************************************
void DllComponentLoader::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArgs )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    if( aArgs.getLength() != 1 )
    {
        throw IllegalArgumentException();
    }

    Reference< XMultiServiceFactory > rServiceManager;

    if( aArgs.getConstArray()[0].getValueType().getTypeClass() == TypeClass_INTERFACE )
    {
        aArgs.getConstArray()[0] >>= rServiceManager;
    }

    if( !rServiceManager.is() )
    {
        throw IllegalArgumentException();
    }

    m_xSMgr = rServiceManager;
}


//*************************************************************************
Reference<XInterface> SAL_CALL DllComponentLoader::activate(
    const OUString & rImplName, const OUString &, const OUString & rLibName,
    const Reference< XRegistryKey > & xKey )

    throw(CannotActivateFactoryException, RuntimeException)
{
    return loadSharedLibComponentFactory( rLibName, OUString(), rImplName, m_xSMgr, xKey );
}


//*************************************************************************
sal_Bool SAL_CALL DllComponentLoader::writeRegistryInfo(
    const Reference< XRegistryKey > & xKey, const OUString &, const OUString & rLibName )

    throw(CannotRegisterImplementationException, RuntimeException)
{
    writeSharedLibComponentInfo( rLibName, OUString(), m_xSMgr, xKey );
    return sal_True;
}


//*************************************************************************
Reference<XInterface> SAL_CALL DllComponentLoader_CreateInstance( const Reference<XMultiServiceFactory> & rSMgr ) throw(Exception)
{
    Reference<XInterface> xRet;

    XImplementationLoader *pXLoader = (XImplementationLoader *)new DllComponentLoader(rSMgr);

    if (pXLoader)
    {
        xRet = Reference<XInterface>::query(pXLoader);
    }

    return xRet;
}

}

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL =
                ::stoc_loader::DllComponentLoader::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            ::stoc_loader::DllComponentLoader_CreateInstance,
            ::stoc_loader::DllComponentLoader::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}




