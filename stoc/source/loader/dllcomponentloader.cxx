/*************************************************************************
 *
 *  $RCSfile: dllcomponentloader.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:13:29 $
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

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif
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
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#if OSL_DEBUG_LEVEL > 0
#include <rtl/ustrbuf.hxx>
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

#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#define SERVICENAME "com.sun.star.loader.SharedLibrary"
#define IMPLNAME    "com.sun.star.comp.stoc.DLLComponentLoader"

#define EXPAND_PROTOCOL "vnd.sun.star.expand"
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::loader;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace rtl;
using namespace osl;

namespace stoc_loader
{
static rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static Sequence< OUString > loader_getSupportedServiceNames()
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

static OUString loader_getImplementationName()
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
    Reference< util::XMacroExpander > m_xMacroExpander;
    OUString expand_url( OUString const & url )
        SAL_THROW( (RuntimeException) );

    Reference< XComponentContext > m_xContext;
    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
DllComponentLoader::DllComponentLoader( const Reference<XComponentContext> & xCtx )
    : m_xContext( xCtx )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_xSMgr.set( m_xContext->getServiceManager(), UNO_QUERY );
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
    return loader_getImplementationName();
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
    return loader_getSupportedServiceNames();
}

//*************************************************************************
void DllComponentLoader::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArgs )
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
    if (0 == url.compareToAscii( RTL_CONSTASCII_STRINGPARAM(EXPAND_PROTOCOL ":") ))
    {
        if (! m_xMacroExpander.is())
        {
            Reference< util::XMacroExpander > xExpander;
            m_xContext->getValueByName(
                OUSTR("/singletons/com.sun.star.util.theMacroExpander") ) >>= xExpander;
            if (! xExpander.is())
            {
                throw DeploymentException(
                    OUSTR("no macro expander singleton available!"), Reference< XInterface >() );
            }
            MutexGuard guard( Mutex::getGlobalMutex() );
            if (! m_xMacroExpander.is())
            {
                m_xMacroExpander = xExpander;
            }
        }

        // cut protocol
        OUString macro( url.copy( sizeof (EXPAND_PROTOCOL ":") -1 ) );
        // decode uric class chars
        macro = Uri::decode( macro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        // expand macro string
        OUString ret( m_xMacroExpander->expandMacros( macro ) );
#if OSL_DEBUG_LEVEL > 0
        OUStringBuffer buf( 128 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("DllComponentLoader::expand_url(): ") );
        buf.append( url );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" => ") );
        buf.append( macro );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" => ") );
        buf.append( ret );
        OString str( OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
//         OSL_TRACE( str.getStr() );
        // use printf here, because OSL_TRACE forwards to printf directly, so %... gets lost
        fprintf( stderr, "%s\n", str.getStr() );
#endif
        return ret;
    }
    else
    {
        return url;
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


//*************************************************************************
Reference<XInterface> SAL_CALL DllComponentLoader_CreateInstance( const Reference<XComponentContext> & xCtx ) throw(Exception)
{
    Reference<XInterface> xRet;

    XImplementationLoader *pXLoader = (XImplementationLoader *)new DllComponentLoader(xCtx);

    if (pXLoader)
    {
        xRet = Reference<XInterface>::query(pXLoader);
    }

    return xRet;
}

}

using namespace stoc_loader;
static struct ImplementationEntry g_entries[] =
{
    {
        DllComponentLoader_CreateInstance, loader_getImplementationName,
        loader_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

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
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}



