/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testfactreg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:34:37 $
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
#include <stdio.h>
#include <string.h>

#include <osl/diagnose.h>

#include <cppuhelper/factory.hxx>  // for EXTERN_SERVICE_CALLTYPE

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

#include "testfactreg.hxx"


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

            OUString str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OPipeTest_getImplementationName() +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            Reference< XRegistryKey >  xNewKey = xKey->createKey( str );
            xNewKey->createKey( OPipeTest_getServiceName() );

            str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OPumpTest_getImplementationName() +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OPumpTest_getServiceName() );

            str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                ODataStreamTest_getImplementationName(1) +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( ODataStreamTest_getServiceName(1) );

            str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                ODataStreamTest_getImplementationName(2) +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( ODataStreamTest_getServiceName(2) );

            str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OObjectStreamTest_getImplementationName(1) +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OObjectStreamTest_getServiceName(1) );

            str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OObjectStreamTest_getImplementationName(2) +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OObjectStreamTest_getServiceName(2) );

            str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OMarkableOutputStreamTest_getImplementationName() +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OMarkableOutputStreamTest_getServiceName() );

            str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OMarkableInputStreamTest_getImplementationName() +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OMarkableInputStreamTest_getServiceName() );

            str = OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                OMyPersistObject_getImplementationName() +
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") );
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OMyPersistObject_getServiceName() );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

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

        if (aImplementationName == OPipeTest_getImplementationName() )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        OPipeTest_CreateInstance,
                                        OPipeTest_getSupportedServiceNames() );
        }
        else if (aImplementationName == OPumpTest_getImplementationName() )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        OPumpTest_CreateInstance,
                                        OPumpTest_getSupportedServiceNames() );
        }

        else if( aImplementationName == ODataStreamTest_getImplementationName(1) ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        ODataStreamTest_CreateInstance,
                                        ODataStreamTest_getSupportedServiceNames(1) );
        }
        else if( aImplementationName == ODataStreamTest_getImplementationName(2) ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        ODataStreamTest_CreateInstance,
                                        ODataStreamTest_getSupportedServiceNames(2) );
        }
        else if( aImplementationName == OObjectStreamTest_getImplementationName(1) ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OObjectStreamTest_CreateInstance,
                                        OObjectStreamTest_getSupportedServiceNames(1) );
        }
        else if( aImplementationName == OObjectStreamTest_getImplementationName(2) ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OObjectStreamTest_CreateInstance,
                                        OObjectStreamTest_getSupportedServiceNames(2) );
        }
        else if( aImplementationName == OMarkableOutputStreamTest_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OMarkableOutputStreamTest_CreateInstance,
                                        OMarkableOutputStreamTest_getSupportedServiceNames() );
        }
        else if( aImplementationName == OMarkableInputStreamTest_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OMarkableInputStreamTest_CreateInstance,
                                        OMarkableInputStreamTest_getSupportedServiceNames() );
        }
        else if( aImplementationName == OMyPersistObject_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                    OMyPersistObject_CreateInstance,
                                        OMyPersistObject_getSupportedServiceNames() );
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

Sequence<sal_Int8 > createSeq( char * p )
{
    Sequence<sal_Int8> seq( strlen( p )+1 );
    strcpy( (char * ) seq.getArray() , p );
    return seq;
}

Sequence<sal_Int8> createIntSeq( sal_Int32 i )
{
    char pcCount[20];
    sprintf( pcCount , "%d" , i );
    return createSeq( pcCount );
}

