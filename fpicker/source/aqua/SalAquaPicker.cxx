/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SalAquaPicker.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:38:36 $
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
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
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
#ifndef _SALAQUAPICKER_HXX_
#include "SalAquaPicker.hxx"
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _CFSTRINGUTILITIES_HXX_
#include "CFStringUtilities.hxx"
#endif
#include "NSString_OOoAdditions.hxx"

#include "SalAquaFilePicker.hxx"

#include <stdio.h>

#pragma mark DEFINES
#define CLASS_NAME "SalAquaPicker"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

// constructor
SalAquaPicker::SalAquaPicker()
: m_pDialog(NULL)
, m_pControlHelper(new ControlHelper())
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

SalAquaPicker::~SalAquaPicker()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (NULL != m_pControlHelper)
        delete m_pControlHelper;

    if (NULL != m_pDialog)
        [m_pDialog release];

    [pool release];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaPicker::implInitialize()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if (m_pDialog != nil) {
        return;
    }

    switch (m_nDialogType)
    {
        case NAVIGATIONSERVICES_OPEN:
            OSL_TRACE("NAVIGATIONSERVICES_OPEN");
            m_pDialog = [NSOpenPanel openPanel];
            [(NSOpenPanel*)m_pDialog setCanChooseDirectories:NO];
            [(NSOpenPanel*)m_pDialog setCanChooseFiles:YES];
            break;

        case NAVIGATIONSERVICES_SAVE:
            OSL_TRACE("NAVIGATIONSERVICES_SAVE");
            m_pDialog = [NSSavePanel savePanel];
            [(NSSavePanel*)m_pDialog setCanSelectHiddenExtension:YES];
            [(NSSavePanel*)m_pDialog setExtensionHidden:NO];
            break;

        case NAVIGATIONSERVICES_DIRECTORY:
            OSL_TRACE("NAVIGATIONSERVICES_DIRECTORY");
            m_pDialog = [NSOpenPanel openPanel];
            [(NSOpenPanel*)m_pDialog setCanChooseDirectories:YES];
            [(NSOpenPanel*)m_pDialog setCanChooseFiles:NO];
            break;

        default:
            OSL_TRACE("m_nDialogType is UNKNOWN: %d", m_nDialogType);
            break;
    }

    if (m_pDialog == nil) {
        OSL_TRACE("An error occurred while creating the dialog!");
    }
    else {
        [(NSOpenPanel*)m_pDialog setCanCreateDirectories:YES];
        //Retain the dialog instance or it will go away immediately
        [m_pDialog retain];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

int SalAquaPicker::run()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (m_pDialog == NULL) {
        //this is the case e.g. for the folder picker at this stage
        implInitialize();
    }

    NSView *userPane = m_pControlHelper->getUserPane();
    if (userPane != NULL) {
        [m_pDialog setAccessoryView:userPane];
    }

    int retVal = 0;

    NSString *startDirectory;
    if (m_sDisplayDirectory.getLength() > 0) {
        NSString *temp = [NSString stringWithOUString:m_sDisplayDirectory];
        NSURL *url = [NSURL URLWithString:temp];
        startDirectory = [url path];

        OSL_TRACE("start dir: %s", [startDirectory UTF8String]);
        // NSLog(@"%@", startDirectory);
    }
    else {
        startDirectory = NSHomeDirectory();
    }

    switch(m_nDialogType) {
        case NAVIGATIONSERVICES_DIRECTORY:
        case NAVIGATIONSERVICES_OPEN:
            retVal = [(NSOpenPanel*)m_pDialog runModalForDirectory:startDirectory file:nil types:nil];
            break;
        case NAVIGATIONSERVICES_SAVE:
            retVal = [m_pDialog runModalForDirectory:startDirectory file:[NSString stringWithOUString:((SalAquaFilePicker*)this)->getSaveFileName()]/*[m_pDialog saveFilename]*/];
            break;
        // [m_pDialog beginSheetForDirectory:startDirectory file:[m_pDialog saveFilename] modalForWindow:[NSApp keyWindow] modalDelegate:((SalAquaFilePicker*)this)->getDelegate() didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
        default:
            break;
    }


    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    [pool release];

    return retVal;
}

int SalAquaPicker::runandwaitforresult()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    int status = this->run();

    DBG_PRINT_EXIT(CLASS_NAME, __func__, status);
    return status;
}

void SAL_CALL SalAquaPicker::implsetDisplayDirectory( const rtl::OUString& aDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "directory", aDirectory);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if (aDirectory != m_sDisplayDirectory) {
        m_sDisplayDirectory = aDirectory;

        if (m_pDialog != nil) {
            //NSLog(@"would change now to:%@", [NSString stringWithOUString:aDirectory]);
            // [m_pDialog setDirectory:[NSString stringWithOUString:aDirectory]];
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaPicker::implgetDisplayDirectory() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__, m_sDisplayDirectory);

    return m_sDisplayDirectory;
}

void SAL_CALL SalAquaPicker::implsetTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", aTitle);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if (m_pDialog != nil) {
        [m_pDialog setTitle:[NSString stringWithOUString:aTitle]];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

