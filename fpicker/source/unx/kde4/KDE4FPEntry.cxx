/*************************************************************************
 *
 *
 *
 *
 *
 *
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

#include <cppuhelper/factory.hxx>

#include <com/sun/star/container/XSet.hpp>

#include <osl/diagnose.h>

#include "KDE4FilePicker.hxx"
#include "FPServiceInfo.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using ::com::sun::star::ui::dialogs::XFilePicker;

static Reference< XInterface > SAL_CALL createInstance( const Reference< XMultiServiceFactory >& serviceManager )
{
    return Reference< XInterface >(static_cast< XFilePicker* >( new KDE4FilePicker( serviceManager ) ) );
}

// the three uno functions that will be exported
extern "C"
{
    void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo( void*, void* pRegistryKey )
    {
        sal_Bool bRetVal = sal_True;

        if ( pRegistryKey )
        {
            try
            {
                Reference< XRegistryKey > pXNewKey( static_cast< XRegistryKey* >( pRegistryKey ) );
                pXNewKey->createKey( OUString::createFromAscii( FILE_PICKER_REGKEY_NAME ) );
            }
            catch( InvalidRegistryException& )
            {
                OSL_ENSURE( sal_False, "InvalidRegistryException caught" );
                bRetVal = sal_False;
            }
        }

        return bRetVal;
    }

    void* SAL_CALL component_getFactory( const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* )
    {
        void* pRet = 0;

        if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, FILE_PICKER_IMPL_NAME ) ) )
        {
            Sequence< OUString > aSNS( 1 );
            aSNS.getArray( )[0] = OUString::createFromAscii( FILE_PICKER_SERVICE_NAME );

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
}
