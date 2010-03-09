/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "simplelogger.hxx"
#include <com/sun/star/registry/XRegistryKey.hpp>
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <rtl/ustrbuf.hxx>

namespace cssuno = com::sun::star::uno;
using rtl::OUString;
using rtl::OUStringBuffer;

//==============================================================================

static cssuno::Reference<cssuno::XInterface> SAL_CALL
    createSimpleLogger( const cssuno::Reference<cssuno::XComponentContext>& aContext)
{
    return * new logger::SimpleLogger(aContext,"configuration.log") ;
}
//==============================================================================

// adapted from the corresponding cppuhelper function
static sal_Bool component_writeInfoHelper_withSingleton(
    void *pServiceManager, void *pRegistryKey ,
    const cppu::ImplementationEntry entries[],
    char const * const singletons[])
{
    using namespace com::sun::star::registry;
    using rtl::OUString;

    sal_Bool bRet = sal_False;
    try
    {
        if( pRegistryKey )
        {
            for( sal_Int32 i = 0; entries[i].create ; i ++ )
            {
                rtl::OUStringBuffer buf( 124 );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/") );
                buf.append( entries[i].getImplementationName() );
                buf.appendAscii(RTL_CONSTASCII_STRINGPARAM( "/UNO/SERVICES" ) );
                cssuno::Reference< XRegistryKey > xNewKey(
                    static_cast< XRegistryKey * >( pRegistryKey )->createKey( buf.makeStringAndClear()  ) );

                cssuno::Sequence< OUString > const seq = entries[i].getSupportedServiceNames();
                for ( sal_Int32 nPos = 0 ; nPos < seq.getLength(); nPos ++ )
                    xNewKey->createKey( seq[nPos] );

                if (singletons[i])
                {
                    rtl::OUStringBuffer buf( 124 );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/") );
                    buf.append( entries[i].getImplementationName() );
                    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM( "/UNO/SINGLETONS/" ) );
                    buf.appendAscii(singletons[i]);
                    cssuno::Reference< XRegistryKey > xNewKey(
                        static_cast< XRegistryKey * >( pRegistryKey )->createKey( buf.makeStringAndClear()  ) );

                    xNewKey->setStringValue(entries[i].getImplementationName());
                }
            }
            bRet = sal_True;
        }
    }
    catch ( InvalidRegistryException & )
    {
        OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
    }
    catch ( cssuno::Exception & )
    {
        OSL_ENSURE( sal_False, "### Unexpected UNO Exception!" );
    }
    return bRet;
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createSimpleLogger,
        logger::SimpleLogger::getImplementationName_static,
        logger::SimpleLogger::getSupportedServiceNames_static,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL }
} ;
static const char * const kSingleton_names[] =
{
    "com.sun.star.configuration.theLogger",
    NULL
};

//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                                            const sal_Char **aEnvTypeName,
                                            uno_Environment **aEnvironment) {
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}
//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void *aServiceManager,
                                                 void *aRegistryKey) {
    return component_writeInfoHelper_withSingleton( aServiceManager,
                                                    aRegistryKey,
                                                    kImplementations_entries,
                                                    kSingleton_names) ;
}
//------------------------------------------------------------------------------

extern "C" void *component_getFactory(const sal_Char *aImplementationName,
                                      void *aServiceManager,
                                      void *aRegistryKey) {
    return cppu::component_getFactoryHelper(aImplementationName,
                                            aServiceManager,
                                            aRegistryKey,
                                            kImplementations_entries) ;
}
//------------------------------------------------------------------------------
