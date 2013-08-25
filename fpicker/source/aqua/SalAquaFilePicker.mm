/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include "sal/config.h"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <osl/diagnose.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <FPServiceInfo.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "resourceprovider.hxx"

#include <osl/file.hxx>
#include "CFStringUtilities.hxx"
#include "NSString_OOoAdditions.hxx"
#include "NSURL_OOoAdditions.hxx"

#include <iostream>

#include "SalAquaFilePicker.hxx"

#include <objc/objc-runtime.h>

#pragma mark DEFINES

#define CLASS_NAME "SalAquaFilePicker"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    // controlling event notifications
    const bool STARTUP_SUSPENDED = true;
    const bool STARTUP_ALIVE     = false;

    uno::Sequence<rtl::OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<rtl::OUString> aRet(3);
        aRet[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ));
        aRet[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.SystemFilePicker" ));
        aRet[2] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.AquaFilePicker" ));
        return aRet;
    }
}

#pragma mark Constructor
//-----------------------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------------------

SalAquaFilePicker::SalAquaFilePicker()
  : SalAquaFilePicker_Base( m_rbHelperMtx )
   , m_pFilterHelper( NULL )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_pDelegate = [[AquaFilePickerDelegate alloc] initWithFilePicker:this];
    m_pControlHelper->setFilePickerDelegate(m_pDelegate);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

SalAquaFilePicker::~SalAquaFilePicker()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (NULL != m_pFilterHelper)
        delete m_pFilterHelper;

    [m_pDelegate release];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}


#pragma mark XFilePickerNotifier
//------------------------------------------------------------------------------------
// XFilePickerNotifier
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;
    m_xListener = xListener;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& )
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;
    m_xListener.clear();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XAsynchronousExecutableDialog
//-----------------------------------------------------------------------------------------
// XExecutableDialog functions
//-----------------------------------------------------------------------------------------
void SAL_CALL SalAquaFilePicker::setTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", aTitle);

    SolarMutexGuard aGuard;
    implsetTitle(aTitle);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

sal_Int16 SAL_CALL SalAquaFilePicker::execute() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    sal_Int16 retVal = 0;

    implInitialize();

    // if m_pDialog is nil after initialization, something must have gone wrong before
    // or there was no initialization (see issue http://www.openoffice.org/issues/show_bug.cgi?id=100214)
    if (m_pDialog == nil) {
        m_nDialogType = NAVIGATIONSERVICES_OPEN;
    }

    if (m_pFilterHelper) {
        m_pFilterHelper->SetFilters();
    }

    if (m_nDialogType == NAVIGATIONSERVICES_SAVE) {
        if (m_sSaveFileName.getLength() == 0) {
            //if no filename is set, NavigationServices will set the name to "untitled". We don't want this!
            //So let's try to get the window title to get the real untitled name
            NSWindow *frontWindow = [NSApp keyWindow];
            if (NULL != frontWindow) {
                NSString *windowTitle = [frontWindow title];
                if (windowTitle != nil) {
                    rtl::OUString ouName = [windowTitle OUString];
                    //a window title will typically be something like "Untitled1 - OpenOffice.org Writer"
                    //but we only want the "Untitled1" part of it
                    sal_Int32 indexOfDash = ouName.indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - ")));
                    if (indexOfDash > -1) {
                        m_sSaveFileName = ouName.copy(0,indexOfDash);
                        if (m_sSaveFileName.getLength() > 0) {
                            setDefaultName(m_sSaveFileName);
                        }
                    } else {
                        OSL_TRACE("no dash present in window title");
                    }
                } else {
                    OSL_TRACE("couldn't get window title");
                }
            } else {
                OSL_TRACE("no front window found");
            }
        }
    }

    //Set the delegate to be notified of certain events

    // I don't know why, but with gcc 4.2.1, this line results in the warning:
    // class 'AquaFilePickerDelegate' does not implement the 'NSOpenSavePanelDelegate' protocol
    // So instead of:
    // [m_pDialog setDelegate:m_pDelegate];
    // do:
    objc_msgSend(m_pDialog, @selector(setDelegate:), m_pDelegate);

    int nStatus = runandwaitforresult();

    [m_pDialog setDelegate:nil];

    switch( nStatus )
    {
        case NSOKButton:
            OSL_TRACE("The dialog returned OK");
            retVal = ExecutableDialogResults::OK;
            break;

        case NSCancelButton:
            OSL_TRACE("The dialog was cancelled by the user!");
            retVal = ExecutableDialogResults::CANCEL;
            break;

        default:
            throw uno::RuntimeException(
                      rtl::OUString("The dialog returned with an unknown result!"), 
                      static_cast<XFilePicker*>( static_cast<XFilePicker3*>( this ) ));
            break;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    return retVal;
}


#pragma mark XFilePicker
//-----------------------------------------------------------------------------------------
// XFilePicker functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::setMultiSelectionMode( sal_Bool bMode ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "multiSelectable?", bMode);

    SolarMutexGuard aGuard;

    if (m_nDialogType == NAVIGATIONSERVICES_OPEN) {
        [(NSOpenPanel*)m_pDialog setAllowsMultipleSelection:YES];
        OSL_TRACE("dialog allows multi-selection? %d", [(NSOpenPanel*)m_pDialog allowsMultipleSelection]);
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::setDefaultName( const rtl::OUString& aName )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "name", aName);

    SolarMutexGuard aGuard;

    m_sSaveFileName = aName;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::setDisplayDirectory( const rtl::OUString& rDirectory )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "directory", rDirectory);

    SolarMutexGuard aGuard;

    implsetDisplayDirectory(rDirectory);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaFilePicker::getDisplayDirectory() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    rtl::OUString retVal = implgetDisplayDirectory();

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);
    return retVal;
}

uno::Sequence<rtl::OUString> SAL_CALL SalAquaFilePicker::getFiles() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070 && HAVE_FEATURE_MACOSX_SANDBOX
    static NSUserDefaults *userDefaults;
    static bool triedUserDefaults = false;

    if (!triedUserDefaults)
    {
        userDefaults = [NSUserDefaults standardUserDefaults];
        triedUserDefaults = true;
    }
#endif

    // OSL_TRACE("starting work");
    /*
     * If more than one file is selected in an OpenDialog, then the first result
     * is the directory and the remaining results contain just the files' names
     * without the basedir path.
     */
    NSArray *files = nil;
    if (m_nDialogType == NAVIGATIONSERVICES_OPEN) {
        files = [(NSOpenPanel*)m_pDialog URLs];
    }
    else if (m_nDialogType == NAVIGATIONSERVICES_SAVE) {
        files = [NSArray arrayWithObjects:[m_pDialog URL], nil];
    }

    long nFiles = [files count];
    OSL_TRACE("# of items: %d", nFiles);

    uno::Sequence< rtl::OUString > aSelectedFiles(nFiles > 1 ? nFiles + 1 : nFiles);

    for(int nIndex = 0; nIndex < nFiles; nIndex += 1)
    {
        NSURL *url = [files objectAtIndex:nIndex];

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070 && HAVE_FEATURE_MACOSX_SANDBOX
        if (userDefaults != NULL &&
            [url respondsToSelector:@selector(bookmarkDataWithOptions:includingResourceValuesForKeys:relativeToURL:error:)])
        {
            // In the case of "Save As" when the user has input a new
            // file name, this call will return nil, as bookmarks can
            // (naturally) only be created for existing file system
            // objects. In that case, code at a much lower level, in
            // sal, takes care of creating a bookmark when a new file
            // has been created outside the sandbox.
            NSData *data = [url bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope
                         includingResourceValuesForKeys:nil
                                          relativeToURL:nil
                                                  error:nil];
            if (data != NULL)
            {
                [userDefaults setObject:data
                                 forKey:[@"bookmarkFor:" stringByAppendingString:[url absoluteString]]];
            }
        }
#endif

        OSL_TRACE("handling %s", [[url description] UTF8String]);
        InfoType info = FULLPATH;
        if (nFiles > 1) {
            //just get the file's name (only in OpenDialog)
            info = FILENAME;
        }
        OUString sFileOrDirURL = [url OUStringForInfo:info];

        //get the directory information, only on the first file processed
        if (nIndex == 0) {
            OUString sDirectoryURL = [url OUStringForInfo:PATHWITHOUTLASTCOMPONENT];

            if (nFiles > 1) {
                aSelectedFiles[0] = OUString(sDirectoryURL);
            }
        }

        short nSequenceIndex = nFiles > 1 ? nIndex + 1 : nIndex;
        aSelectedFiles[nSequenceIndex] = sFileOrDirURL;

        OSL_TRACE("Returned file in getFiles: \"%s\".", OUStringToOString(sFileOrDirURL, RTL_TEXTENCODING_UTF8).getStr());
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return aSelectedFiles;
}

#pragma mark XFilterManager
//-----------------------------------------------------------------------------------------
// XFilterManager functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::appendFilter( const rtl::OUString& aTitle, const rtl::OUString& aFilter )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    ensureFilterHelper();
    m_pFilterHelper->appendFilter( aTitle, aFilter );
    m_pControlHelper->setFilterControlNeeded(YES);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::setCurrentFilter( const rtl::OUString& aTitle )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    OSL_TRACE( "Setting current filter to %s",
               OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 ).getStr() );

    SolarMutexGuard aGuard;

    ensureFilterHelper();
    m_pFilterHelper->setCurrentFilter(aTitle);
    updateFilterUI();

    updateSaveFileNameExtension();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaFilePicker::getCurrentFilter() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    SolarMutexGuard aGuard;

    ensureFilterHelper();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return m_pFilterHelper->getCurrentFilter();
}

#pragma mark XFilterGroupManager
//-----------------------------------------------------------------------------------------
// XFilterGroupManager functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::appendFilterGroup( const rtl::OUString& sGroupTitle, const uno::Sequence<beans::StringPair>& aFilters )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    SolarMutexGuard aGuard;

    ensureFilterHelper();
    m_pFilterHelper->appendFilterGroup(sGroupTitle, aFilters);
    m_pControlHelper->setFilterControlNeeded(YES);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XFilePickerControlAccess
//------------------------------------------------------------------------------------
// XFilePickerControlAccess functions
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    m_pControlHelper->setValue(nControlId, nControlAction, rValue);

    if (nControlId == ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION && m_nDialogType == NAVIGATIONSERVICES_SAVE) {
        updateSaveFileNameExtension();
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

uno::Any SAL_CALL SalAquaFilePicker::getValue( sal_Int16 nControlId, sal_Int16 nControlAction )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    uno::Any aValue = m_pControlHelper->getValue(nControlId, nControlAction);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return aValue;
}

void SAL_CALL SalAquaFilePicker::enableControl( sal_Int16 nControlId, sal_Bool bEnable )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_pControlHelper->enableControl(nControlId, bEnable);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::setLabel( sal_Int16 nControlId, const ::rtl::OUString& aLabel )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    NSString* sLabel = [NSString stringWithOUString:aLabel];
    m_pControlHelper->setLabel( nControlId, sLabel ) ;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaFilePicker::getLabel( sal_Int16 nControlId )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return m_pControlHelper->getLabel(nControlId);
}

#pragma mark XInitialization
//------------------------------------------------------------------------------------
// XInitialization
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::initialize( const uno::Sequence<uno::Any>& aArguments )
throw( uno::Exception, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "arguments size", aArguments.getLength());

    SolarMutexGuard aGuard;

    // parameter checking
    uno::Any aAny;
    if( 0 == aArguments.getLength() )
        throw lang::IllegalArgumentException(rtl::OUString("no arguments"),
                                             static_cast<XFilePicker*>( static_cast<XFilePicker3*>(this) ), 1 );

    aAny = aArguments[0];

    if( ( aAny.getValueType() != ::getCppuType( ( sal_Int16* )0 ) ) &&
        (aAny.getValueType() != ::getCppuType( ( sal_Int8* )0 ) ) )
        throw lang::IllegalArgumentException(rtl::OUString( "invalid argument type" ),
                                             static_cast<XFilePicker*>( static_cast<XFilePicker3*>(this) ), 1 );

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    switch( templateId )
    {
        case FILEOPEN_SIMPLE:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_SIMPLE" );
            break;
        case FILESAVE_SIMPLE:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_SIMPLE" );
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION_PASSWORD" );
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS" );
            break;
        case FILESAVE_AUTOEXTENSION_SELECTION:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION_SELECTION" );
            break;
        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION_TEMPLATE" );
            break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE" );
            break;
        case FILEOPEN_PLAY:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_PLAY" );
            break;
        case FILEOPEN_READONLY_VERSION:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_READONLY_VERSION" );
            break;
        case FILEOPEN_LINK_PREVIEW:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_LINK_PREVIEW" );
            break;
        case FILESAVE_AUTOEXTENSION:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION" );
            break;
        default:
            throw lang::IllegalArgumentException(rtl::OUString("Unknown template"),
                                                 static_cast<XFilePicker*>( static_cast<XFilePicker3*>(this) ),
                                                 1 );
    }

    m_pControlHelper->initialize(templateId);

    implInitialize();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XCancellable
//------------------------------------------------------------------------------------
// XCancellable
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::cancel() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    if (m_pDialog != nil) {
        [m_pDialog cancel:nil];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XEventListener
//------------------------------------------------
// XEventListener
//------------------------------------------------

void SAL_CALL SalAquaFilePicker::disposing( const lang::EventObject& aEvent ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    uno::Reference<XFilePickerListener> xFilePickerListener( aEvent.Source, ::com::sun::star::uno::UNO_QUERY );

    if( xFilePickerListener.is() )
        removeFilePickerListener( xFilePickerListener );

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XServiceInfo
// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

rtl::OUString SAL_CALL SalAquaFilePicker::getImplementationName()
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    rtl::OUString retVal(RTL_CONSTASCII_USTRINGPARAM( FILE_PICKER_IMPL_NAME ));

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    return retVal;
}

sal_Bool SAL_CALL SalAquaFilePicker::supportsService( const rtl::OUString& sServiceName )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "service name", sServiceName);

    sal_Bool retVal = sal_False;

    uno::Sequence <rtl::OUString> supportedServicesNames = FilePicker_getSupportedServiceNames();

    for( sal_Int32 n = supportedServicesNames.getLength(); n--; ) {
        if( supportedServicesNames[n] == sServiceName ) {
            retVal = sal_True;
            break;
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return retVal;
}

uno::Sequence<rtl::OUString> SAL_CALL SalAquaFilePicker::getSupportedServiceNames()
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return FilePicker_getSupportedServiceNames();
}

#pragma mark Misc/Private
//-----------------------------------------------------------------------------------------
// FilePicker Event functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::fileSelectionChanged( FilePickerEvent aEvent )
{
    OSL_TRACE( "file selection changed");
    if (m_xListener.is())
        m_xListener->fileSelectionChanged( aEvent );
}

void SAL_CALL SalAquaFilePicker::directoryChanged( FilePickerEvent aEvent )
{
    OSL_TRACE("directory changed");
    if (m_xListener.is())
        m_xListener->directoryChanged( aEvent );
}

void SAL_CALL SalAquaFilePicker::controlStateChanged( FilePickerEvent aEvent )
{
    OSL_TRACE("control state changed");
    if (m_xListener.is())
        m_xListener->controlStateChanged( aEvent );
}

void SAL_CALL SalAquaFilePicker::dialogSizeChanged()
{
    OSL_TRACE("dialog size changed");
    if (m_xListener.is())
        m_xListener->dialogSizeChanged();
}

//--------------------------------------------------
// Misc
//-------------------------------------------------
void SalAquaFilePicker::ensureFilterHelper() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    if (NULL == m_pFilterHelper) {
        m_pFilterHelper = new FilterHelper;
        m_pControlHelper->setFilterHelper(m_pFilterHelper);
        [m_pDelegate setFilterHelper:m_pFilterHelper];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::implInitialize()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    //call super
    SalAquaPicker::implInitialize();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::updateFilterUI() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_pControlHelper->updateFilterUI();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::updateSaveFileNameExtension() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (m_nDialogType != NAVIGATIONSERVICES_SAVE) {
        return;
    }

    // we need to set this here again because initial setting does
    //[m_pDialog setExtensionHidden:YES];

    SolarMutexGuard aGuard;

    if (m_pControlHelper->isAutoExtensionEnabled() == false) {
        OSL_TRACE("allowing other file types");
        [m_pDialog setAllowedFileTypes:nil];
        [m_pDialog setAllowsOtherFileTypes:YES];
    } else {
        ensureFilterHelper();

        OUStringList aStringList = m_pFilterHelper->getCurrentFilterSuffixList();
        if( aStringList.empty()) // #i9328#
            return;

        rtl::OUString suffix = (*(aStringList.begin())).copy(1);
        NSString *requiredFileType = [NSString stringWithOUString:suffix];

        [m_pDialog setAllowedFileTypes:[NSArray arrayWithObjects:requiredFileType, nil]];

        OSL_TRACE("disallowing other file types");
        [m_pDialog setAllowsOtherFileTypes:NO];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::filterControlChanged() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (m_pDialog == nil) {
        return;
    }

    SolarMutexGuard aGuard;

    updateSaveFileNameExtension();

    [m_pDialog validateVisibleColumns];

    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    controlStateChanged( evt );

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
