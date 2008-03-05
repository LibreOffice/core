/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SalAquaFolderPicker.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:37:59 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif

#ifndef  _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _FPSERVICEINFO_HXX_
#include <FPServiceInfo.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SALAQUAFOLDERPICKER_HXX_
#include "SalAquaFolderPicker.hxx"
#endif

#include <tools/urlobj.hxx>
#include <iostream>

#include "resourceprovider.hxx"

#ifndef _SV_RC_H
#include <tools/rc.hxx>
#endif

#include <osl/file.hxx>

#ifndef _CFSTRINGUTILITIES_HXX_
#include "CFStringUtilities.hxx"
#endif

#ifndef _NSSTRING_OOOADDITIONS_HXX_
#include "NSString_OOoAdditions.hxx"
#endif

#ifndef _NSURL_OOOADDITIONS_HXX_
#include "NSURL_OOoAdditions.hxx"
#endif

#pragma mark DEFINES
#define CLASS_NAME "SalAquaFolderPicker"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    // controling event notifications
    uno::Sequence<rtl::OUString> SAL_CALL FolderPicker_getSupportedServiceNames()
    {
        uno::Sequence<rtl::OUString> aRet(2);
        aRet[0] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.SystemFolderPicker" );
        aRet[1] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.AquaFolderPicker" );
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------------------
SalAquaFolderPicker::SalAquaFolderPicker( const uno::Reference<lang::XMultiServiceFactory>& xServiceMgr ) :
    m_xServiceMgr( xServiceMgr )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_nDialogType = NAVIGATIONSERVICES_DIRECTORY;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

//-----------------------------------------------------------------------------------------
// XExecutableDialog functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFolderPicker::setTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", aTitle);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    implsetTitle(aTitle);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

sal_Int16 SAL_CALL SalAquaFolderPicker::execute() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_Int16 retVal = 0;

    int nResult = runandwaitforresult();

    switch( nResult )
    {
    case NSOKButton:
        OSL_TRACE("Dialog returned with OK");
        retVal = ExecutableDialogResults::OK;
        break;

    case NSCancelButton:
        OSL_TRACE("Dialog was cancelled!");
        retVal = ExecutableDialogResults::CANCEL;
        break;

    default:
        throw uno::RuntimeException(rtl::OUString::createFromAscii("The dialog returned with an unknown result!"), static_cast< XFolderPicker* >( this ));
        break;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return retVal;
}

//------------------------------------------------------------------------------------
// XFolderPicker functions
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFolderPicker::setDisplayDirectory( const rtl::OUString& aDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "directory", aDirectory);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    implsetDisplayDirectory(aDirectory);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaFolderPicker::getDisplayDirectory() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OUString aDirectory = implgetDisplayDirectory();

    DBG_PRINT_EXIT(CLASS_NAME, __func__, aDirectory);

    return aDirectory;
}

rtl::OUString SAL_CALL SalAquaFolderPicker::getDirectory() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    NSArray *files = nil;
    if (m_nDialogType == NAVIGATIONSERVICES_DIRECTORY) {
        files = [(NSOpenPanel*)m_pDialog URLs];
    }

    long nFiles = [files count];
    OSL_TRACE("# of items: %d", nFiles);

    if (nFiles < 1) {
        throw uno::RuntimeException(rtl::OUString::createFromAscii("no directory selected"), static_cast< XFolderPicker* >( this ));
    }

    rtl::OUString aDirectory;

    NSURL *url = [files objectAtIndex:0];
    OSL_TRACE("handling %s", [[url description] UTF8String]);

    aDirectory = [url OUStringForInfo:FULLPATH];

    implsetDisplayDirectory(aDirectory);

    OSL_TRACE("dir url: %s", OUStringToOString(aDirectory, RTL_TEXTENCODING_UTF8).getStr());

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return aDirectory;
}

void SAL_CALL SalAquaFolderPicker::setDescription( const rtl::OUString& rDescription )
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "description", rDescription);

    [m_pDialog setMessage:[NSString stringWithOUString:rDescription]];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

rtl::OUString SAL_CALL SalAquaFolderPicker::getImplementationName()
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    rtl::OUString retVal = rtl::OUString::createFromAscii( FOLDER_PICKER_IMPL_NAME );

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    return retVal;
}

sal_Bool SAL_CALL SalAquaFolderPicker::supportsService( const rtl::OUString& sServiceName )
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "serviceName", sServiceName);

    sal_Bool retVal = sal_False;
    uno::Sequence <rtl::OUString> supportedServicesNames = FolderPicker_getSupportedServiceNames();

    for( sal_Int32 n = supportedServicesNames.getLength(); n--; ) {
        if( supportedServicesNames[n].compareTo( sServiceName ) == 0) {
            retVal = sal_True;
            break;
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);
    return retVal;
}

uno::Sequence<rtl::OUString> SAL_CALL SalAquaFolderPicker::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return FolderPicker_getSupportedServiceNames();
}

//------------------------------------------------------------------------------------
// XCancellable
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFolderPicker::cancel() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    [m_pDialog cancel:nil];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

// -------------------------------------------------
// XEventListener
// -------------------------------------------------

void SAL_CALL SalAquaFolderPicker::disposing( const lang::EventObject& )
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}
