/*************************************************************************
 *
 *  $RCSfile: aqua_service.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: pluby $ $Date: 2001-03-15 19:25:36 $
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

#ifndef _AQUA_CLIPBOARD_HXX_
#include "aqua_clipboard.hxx"
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

// the service names
#define AQUA_CLIPBOARD_SERVICE_NAME "com.sun.star.datatransfer.clipboard.SystemClipboard"

// the implementation names
#define AQUA_CLIPBOARD_IMPL_NAME "com.sun.star.datatransfer.clipboard.AquaClipboard"

// the registry key names
#define AQUA_CLIPBOARD_REGKEY_NAME "/com.sun.star.datatransfer.clipboard.AquaClipboard/UNO/SERVICES/com.sun.star.datatransfer.clipboard.SystemClipboard"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer::clipboard;
using namespace aqua;

namespace aqua {

Reference< XInterface > SAL_CALL createInstance( const Reference< XMultiServiceFactory >& rServiceManager )
{
    return Reference< XInterface >( static_cast< XClipboard* >( new AquaClipboard() ) );
}

} // namespace aqua

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    sal_Bool bRetVal = sal_False;

    if ( pRegistryKey )
    {
        try
        {
            Reference< XRegistryKey > pXNewKey( static_cast< XRegistryKey* >( pRegistryKey ) );
            pXNewKey->createKey( OUString( RTL_CONSTASCII_USTRINGPARAM( AQUA_CLIPBOARD_REGKEY_NAME ) ) );
            bRetVal = sal_True;
        }
        catch( InvalidRegistryException& )
        {
            OSL_ENSHURE(sal_False, "InvalidRegistryException caught");
            bRetVal = sal_False;
        }
    }

    return bRetVal;
}

void* SAL_CALL component_getFactory( const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* pRegistryKey )
{
    void* pRet = 0;

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, AQUA_CLIPBOARD_IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray( )[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( AQUA_CLIPBOARD_SERVICE_NAME ) );

        //OUString( RTL_CONSTASCII_USTRINGPARAM( FPS_IMPL_NAME ) )
        Reference< XSingleServiceFactory > xFactory ( createOneInstanceFactory(
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
