/*************************************************************************
 *
 *  $RCSfile: factreg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:18 $
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

#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

#include "factreg.hxx"

using namespace io_stm;

extern "C"
{


void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

            OUString str = OUString::createFromAscii( "/" );
            str += OPipeImpl_getImplementationName();
            str += OUString::createFromAscii( "/UNO/SERVICES" );
            Reference< XRegistryKey >  xNewKey = xKey->createKey( str );
            xNewKey->createKey( OPipeImpl_getServiceName() );

            str = OUString::createFromAscii( "/" );
            str += ODataInputStream_getImplementationName();
            str += OUString::createFromAscii( "/UNO/SERVICES" );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( ODataInputStream_getServiceName() );

            str = OUString::createFromAscii( "/" );
            str += ODataOutputStream_getImplementationName();
            str += OUString::createFromAscii( "/UNO/SERVICES" );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( ODataOutputStream_getServiceName() );

            str = OUString::createFromAscii( "/" );
            str += OMarkableOutputStream_getImplementationName();
            str += OUString::createFromAscii( "/UNO/SERVICES" );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OMarkableOutputStream_getServiceName() );

            str = OUString::createFromAscii( "/" );
            str += OMarkableInputStream_getImplementationName();
            str += OUString::createFromAscii( "/UNO/SERVICES" );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OMarkableInputStream_getServiceName() );

            str = OUString::createFromAscii( "/" );
            str += OObjectInputStream_getImplementationName();
            str += OUString::createFromAscii( "/UNO/SERVICES" );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OObjectInputStream_getServiceName() );

            str = OUString::createFromAscii( "/" );
            str += OObjectOutputStream_getImplementationName();
            str += OUString::createFromAscii( "/UNO/SERVICES" );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OObjectOutputStream_getServiceName() );

            str = OUString::createFromAscii( "/" );
            str += OPumpImpl_getImplementationName();
            str += OUString::createFromAscii( "/UNO/SERVICES" );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OPumpImpl_getServiceName() );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}


//               createSingleFactory(
//              reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
//              OUString::createFromAscii( pImplName ),
//              acceptor_CreateInstance, acceptor_getSupportedServiceNames() ) );

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager )
    {
        Reference< XSingleServiceFactory > xRet;
        Reference< XMultiServiceFactory > xSMgr =
            reinterpret_cast< XMultiServiceFactory * > ( pServiceManager );

        OUString aImplementationName = OUString::createFromAscii( pImplName );

        if (aImplementationName == OPipeImpl_getImplementationName() )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        OPipeImpl_CreateInstance,
                                        OPipeImpl_getSupportedServiceNames() );
        }
        else if( aImplementationName == ODataInputStream_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        ODataInputStream_CreateInstance,
                                        ODataInputStream_getSupportedServiceNames() );
        }
        else if( aImplementationName == ODataOutputStream_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                    ODataOutputStream_CreateInstance,
                                        ODataOutputStream_getSupportedServiceNames() );
        }
        else if( aImplementationName == OMarkableOutputStream_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                    OMarkableOutputStream_CreateInstance,
                                        OMarkableOutputStream_getSupportedServiceNames() );
        }
        else if( aImplementationName == OMarkableInputStream_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OMarkableInputStream_CreateInstance,
                                        OMarkableInputStream_getSupportedServiceNames() );
        }
        else if( aImplementationName == OObjectInputStream_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OObjectInputStream_CreateInstance,
                                        OObjectInputStream_getSupportedServiceNames() );
        }
        else if( aImplementationName == OObjectOutputStream_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OObjectOutputStream_CreateInstance,
                                        OObjectOutputStream_getSupportedServiceNames() );
        }
        else if( aImplementationName == OPumpImpl_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OPumpImpl_CreateInstance,
                                        OPumpImpl_getSupportedServiceNames() );
        }

        if (xRet.is())
        {
            xRet->acquire();
            pRet = xRet.get();
        }
    }

    return pRet;
}



}
