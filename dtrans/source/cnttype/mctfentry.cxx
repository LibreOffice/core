/*************************************************************************
 *
 *  $RCSfile: mctfentry.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 14:25:37 $
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

//-----------------------------------------------------------------
//  includes of other projects
//-----------------------------------------------------------------

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

#ifndef _MCNTTFACTORY_HXX_
#include "mcnttfactory.hxx"
#endif

//-----------------------------------------------------------------
// some defines
//-----------------------------------------------------------------

// the service names
#define MIMECONTENTTYPEFACTORY_SERVICE_NAME  "com.sun.star.datatransfer.MimeContentTypeFactory"

// the implementation names
#define MIMECONTENTTYPEFACTORY_IMPL_NAME  "com.sun.star.datatransfer.MimeCntTypeFactory"

// the registry key names
// a key under which this service will be registered, Format: -> "/ImplName/UNO/SERVICES/ServiceName"
//                        <     Implementation-Name    ></UNO/SERVICES/><    Service-Name           >
#define MIMECONTENTTYPEFACTORY_REGKEY_NAME  "/com.sun.star.datatransfer.MimeCntTypeFactory/UNO/SERVICES/com.sun.star.datatransfer.MimeContentTypeFactory"

//-----------------------------------------------------------------------------------------------------------
// namespace directives
//-----------------------------------------------------------------------------------------------------------

using namespace ::rtl                       ;
using namespace ::cppu                      ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::registry  ;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;

//-----------------------------------------------------------------
// create a static object to initialize the shell9x library
//-----------------------------------------------------------------

namespace
{

    //-----------------------------------------------------------------------------------------------------------
    // functions to create a new Clipboad instance; is needed by factory helper implementation
    // @param rServiceManager - service manager, useful if the component needs other uno services
    // so we should give it to every UNO-Implementation component
    //-----------------------------------------------------------------------------------------------------------

    Reference< XInterface > SAL_CALL createInstance( const Reference< XMultiServiceFactory >& rServiceManager )
    {
        return Reference< XInterface >( static_cast< XMimeContentTypeFactory* >( new CMimeContentTypeFactory( rServiceManager ) ) );
    }
}

//-----------------------------------------------------------------------------------------------------------
// the 3 important functions which will be exported
//-----------------------------------------------------------------------------------------------------------

extern "C"
{

//----------------------------------------------------------------------
// component_getImplementationEnvironment
//----------------------------------------------------------------------

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//-------------------------------------------------------------------------
// component_writeInfo - to register a UNO-Service
// to register a UNO-Service use: regcomp -register -r *.rdb -c *.dll
// to view the registry use: regview *.rdb /SERVICES/ServiceName
// (you must use the full services name e.g. com.sun.star.frame.FilePicker
//-------------------------------------------------------------------------

sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    sal_Bool bRetVal = sal_False;

    if ( pRegistryKey )
    {
        try
        {
            Reference< XRegistryKey > pXNewKey( static_cast< XRegistryKey* >( pRegistryKey ) );
            pXNewKey->createKey( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMECONTENTTYPEFACTORY_REGKEY_NAME ) ) );
            bRetVal = sal_True;
        }
        catch( InvalidRegistryException& )
        {
            OSL_ENSURE(sal_False, "InvalidRegistryException caught");
            bRetVal = sal_False;
        }
    }

    return bRetVal;
}

//----------------------------------------------------------------------
// component_getFactory
// returns a factory to create XFilePicker-Services
//----------------------------------------------------------------------

void* SAL_CALL component_getFactory( const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* pRegistryKey )
{
    void* pRet = 0;

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, MIMECONTENTTYPEFACTORY_IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray( )[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( MIMECONTENTTYPEFACTORY_SERVICE_NAME ) );

        Reference< XSingleServiceFactory > xFactory ( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createInstance,
            aSNS ) );
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

} // extern "C"
