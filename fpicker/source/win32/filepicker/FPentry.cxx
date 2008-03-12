/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FPentry.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:31:09 $
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
#include "precompiled_fpicker.hxx"

//----------------------------------------------
//  includes of other projects
//----------------------------------------------

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _FILEPICKER_HXX_
#include "FilePicker.hxx"
#endif

#ifndef _FPSERVICEINFO_HXX_
#include "FPServiceInfo.hxx"
#endif

#pragma warning (disable:4917)
#include "VistaFilePicker.hxx"
#include "..\misc\WinImplHelper.hxx"
#include <stdio.h>

//-----------------------------------------------
// namespace directives
//-----------------------------------------------

using namespace ::rtl                       ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::container ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::registry  ;
using namespace ::cppu                      ;
using ::com::sun::star::ui::dialogs::XFilePicker;
using ::com::sun::star::ui::dialogs::XFilePicker2;

//------------------------------------------------
//
//------------------------------------------------

static Reference< XInterface > SAL_CALL createInstance(
    const Reference< XMultiServiceFactory >& rServiceManager )
{
    Reference< XInterface > xDlg;
    bool                    bVista = IsWindowsVista();

    if (bVista)
    {
        fprintf(stdout, "use special (vista) system file picker ...\n");
        xDlg.set(
            static_cast< XFilePicker2* >(
                new ::fpicker::win32::vista::VistaFilePicker( rServiceManager ) ) );
    }
    else
    {
        fprintf(stdout, "use normal system file picker ...\n");
        xDlg.set(
            static_cast< XFilePicker2* >(
                new CFilePicker( rServiceManager ) ) );
    }

    return xDlg;
}

//------------------------------------------------
// the three uno functions that will be exported
//------------------------------------------------

extern "C"
{

//------------------------------------------------
// component_getImplementationEnvironment
//------------------------------------------------

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//------------------------------------------------
//
//------------------------------------------------

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

//------------------------------------------------
//
//------------------------------------------------

void* SAL_CALL component_getFactory(
    const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* )
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

} // extern "C"
