/*************************************************************************
 *
 *  $RCSfile: loggerdfn.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-31 15:00:41 $
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

#include "simplelogger.hxx"

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
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
