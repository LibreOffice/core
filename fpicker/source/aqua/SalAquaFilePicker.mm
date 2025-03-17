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

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "resourceprovider.hxx"

#include <osl/file.hxx>
#include "NSString_OOoAdditions.hxx"
#include "NSURL_OOoAdditions.hxx"

#include <iostream>

#include "SalAquaFilePicker.hxx"

#include <objc/objc-runtime.h>

#pragma mark DEFINES

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace
{
    uno::Sequence<OUString> FilePicker_getSupportedServiceNames()
    {
        return { "com.sun.star.ui.dialogs.FilePicker",
                 "com.sun.star.ui.dialogs.SystemFilePicker",
                 "com.sun.star.ui.dialogs.AquaFilePicker" };
    }
}

#pragma mark Constructor

SalAquaFilePicker::SalAquaFilePicker()
  : SalAquaFilePicker_Base( m_rbHelperMtx )
   , m_pFilterHelper( nullptr )
{
    m_pDelegate = [[AquaFilePickerDelegate alloc] initWithFilePicker:this];
    m_pControlHelper->setFilePickerDelegate(m_pDelegate);
}

SalAquaFilePicker::~SalAquaFilePicker()
{
    if (nullptr != m_pFilterHelper)
        delete m_pFilterHelper;

    [m_pDelegate release];
}


#pragma mark XFilePickerNotifier

void SAL_CALL SalAquaFilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
{
    SolarMutexGuard aGuard;
    m_xListener = xListener;
}

void SAL_CALL SalAquaFilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& )
{
    SolarMutexGuard aGuard;
    m_xListener.clear();
}

#pragma mark XAsynchronousExecutableDialog

void SAL_CALL SalAquaFilePicker::setTitle( const OUString& aTitle )
{
    SolarMutexGuard aGuard;
    implsetTitle(aTitle);
}

sal_Int16 SAL_CALL SalAquaFilePicker::execute()
{
    SolarMutexGuard aGuard;

    sal_Int16 retVal = 0;

    implInitialize();

    // if m_pDialog is nil after initialization, something must have gone wrong before
    // or there was no initialization (see issue https://bz.apache.org/ooo/show_bug.cgi?id=100214)
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
            if (nullptr != frontWindow) {
                NSString *windowTitle = [frontWindow title];
                if (windowTitle != nil) {
                    OUString ouName = [windowTitle OUString];
                    //a window title will typically be something like "Untitled1 - OpenOffice.org Writer"
                    //but we only want the "Untitled1" part of it
                    sal_Int32 indexOfDash = ouName.indexOf(" - ");
                    if (indexOfDash > -1) {
                        m_sSaveFileName = ouName.copy(0,indexOfDash);
                        if (m_sSaveFileName.getLength() > 0) {
                            setDefaultName(m_sSaveFileName);
                        }
                    }
                }
            }
        }
    }

    //Set the delegate to be notified of certain events

    [m_pDialog setDelegate:m_pDelegate];

    int nStatus = runandwaitforresult();

    [m_pDialog setDelegate:nil];

    switch( nStatus )
    {
        case NSModalResponseOK:
            retVal = ExecutableDialogResults::OK;
            break;

        case NSModalResponseCancel:
            retVal = ExecutableDialogResults::CANCEL;
            break;

        default:
            throw uno::RuntimeException(
                      "The dialog returned with an unknown result!",
                      static_cast<XFilePicker*>( static_cast<XFilePicker3*>( this ) ));
            break;
    }

    return retVal;
}


#pragma mark XFilePicker

void SAL_CALL SalAquaFilePicker::setMultiSelectionMode( sal_Bool /* bMode */ )
{
    SolarMutexGuard aGuard;

    if (m_nDialogType == NAVIGATIONSERVICES_OPEN) {
        [static_cast<NSOpenPanel*>(m_pDialog) setAllowsMultipleSelection:YES];
    }
}

void SAL_CALL SalAquaFilePicker::setDefaultName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    m_sSaveFileName = aName;
}

void SAL_CALL SalAquaFilePicker::setDisplayDirectory( const OUString& rDirectory )
{
    SolarMutexGuard aGuard;

    implsetDisplayDirectory(rDirectory);
}

OUString SAL_CALL SalAquaFilePicker::getDisplayDirectory()
{
    OUString retVal = implgetDisplayDirectory();

    return retVal;
}

uno::Sequence<OUString> SAL_CALL SalAquaFilePicker::getFiles()
{
    uno::Sequence< OUString > aSelectedFiles = getSelectedFiles();
    // multiselection doesn't really work with getFiles
    // so just retrieve the first url
    if (aSelectedFiles.getLength() > 1)
        aSelectedFiles.realloc(1);

    return aSelectedFiles;
}

uno::Sequence<OUString> SAL_CALL SalAquaFilePicker::getSelectedFiles()
{
    SolarMutexGuard aGuard;

#if HAVE_FEATURE_MACOSX_SANDBOX
    static NSUserDefaults *userDefaults;
    static bool triedUserDefaults = false;

    if (!triedUserDefaults)
    {
        userDefaults = [NSUserDefaults standardUserDefaults];
        triedUserDefaults = true;
    }
#endif

    NSArray *files = nil;
    if (m_nDialogType == NAVIGATIONSERVICES_OPEN) {
        files = [static_cast<NSOpenPanel*>(m_pDialog) URLs];
    }
    else if (m_nDialogType == NAVIGATIONSERVICES_SAVE) {
        files = [NSArray arrayWithObjects:[m_pDialog URL], nil];
    }

    NSUInteger nFiles = [files count];
    SAL_INFO("fpicker.aqua", "# of items: " << nFiles);

    uno::Sequence< OUString > aSelectedFiles(nFiles);
    OUString* pSelectedFiles = aSelectedFiles.getArray();

    for(NSUInteger nIndex = 0; nIndex < nFiles; nIndex += 1)
    {
        NSURL *url = [files objectAtIndex:nIndex];

#if HAVE_FEATURE_MACOSX_SANDBOX
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

        OUString sFileOrDirURL = [url OUString];

        pSelectedFiles[nIndex] = sFileOrDirURL;
    }

    return aSelectedFiles;
}

#pragma mark XFilterManager

void SAL_CALL SalAquaFilePicker::appendFilter( const OUString& aTitle, const OUString& aFilter )
{
    SolarMutexGuard aGuard;

    ensureFilterHelper();
    m_pFilterHelper->appendFilter( aTitle, aFilter );
    m_pControlHelper->setFilterControlNeeded(true);
}

void SAL_CALL SalAquaFilePicker::setCurrentFilter( const OUString& aTitle )
{
    SolarMutexGuard aGuard;

    ensureFilterHelper();
    m_pFilterHelper->setCurrentFilter(aTitle);
    updateFilterUI();

    updateSaveFileNameExtension();
}

OUString SAL_CALL SalAquaFilePicker::getCurrentFilter()
{
    SolarMutexGuard aGuard;

    ensureFilterHelper();

    return m_pFilterHelper->getCurrentFilter();
}

#pragma mark XFilterGroupManager

void SAL_CALL SalAquaFilePicker::appendFilterGroup( const OUString&, const uno::Sequence<beans::StringPair>& aFilters )
{
    SolarMutexGuard aGuard;

    ensureFilterHelper();
    m_pFilterHelper->appendFilterGroup(aFilters);
    m_pControlHelper->setFilterControlNeeded(true);
}

#pragma mark XFilePickerControlAccess

void SAL_CALL SalAquaFilePicker::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue )
{
    SolarMutexGuard aGuard;

    m_pControlHelper->setValue(nControlId, nControlAction, rValue);

    if (nControlId == ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION && m_nDialogType == NAVIGATIONSERVICES_SAVE) {
        updateSaveFileNameExtension();
    }
}

uno::Any SAL_CALL SalAquaFilePicker::getValue( sal_Int16 nControlId, sal_Int16 nControlAction )
{
    uno::Any aValue = m_pControlHelper->getValue(nControlId, nControlAction);

    return aValue;
}

void SAL_CALL SalAquaFilePicker::enableControl( sal_Int16 nControlId, sal_Bool bEnable )
{
    m_pControlHelper->enableControl(nControlId, bEnable);
}

void SAL_CALL SalAquaFilePicker::setLabel( sal_Int16 nControlId, const OUString& aLabel )
{
    SolarMutexGuard aGuard;

    NSString* sLabel = [NSString stringWithOUString:aLabel];
    m_pControlHelper->setLabel( nControlId, sLabel ) ;
}

OUString SAL_CALL SalAquaFilePicker::getLabel( sal_Int16 nControlId )
{
    return m_pControlHelper->getLabel(nControlId);
}

#pragma mark XInitialization

void SAL_CALL SalAquaFilePicker::initialize( const uno::Sequence<uno::Any>& aArguments )
{
    SolarMutexGuard aGuard;

    // parameter checking
    uno::Any aAny;
    if( 0 == aArguments.getLength() )
        throw lang::IllegalArgumentException("no arguments",
                                             static_cast<XFilePicker*>( static_cast<XFilePicker3*>(this) ), 1 );

    aAny = aArguments[0];

    if( ( aAny.getValueType() != ::cppu::UnoType<sal_Int16>::get() ) &&
        (aAny.getValueType() != ::cppu::UnoType<sal_Int8>::get() ) )
        throw lang::IllegalArgumentException("invalid argument type",
                                             static_cast<XFilePicker*>( static_cast<XFilePicker3*>(this) ), 1 );

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    switch( templateId )
    {
        case FILEOPEN_SIMPLE:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        case FILESAVE_SIMPLE:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            break;
        case FILESAVE_AUTOEXTENSION_SELECTION:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            break;
        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        case FILEOPEN_PLAY:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        case FILEOPEN_LINK_PLAY:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        case FILEOPEN_READONLY_VERSION:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        case FILEOPEN_READONLY_VERSION_FILTEROPTIONS:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        case FILEOPEN_LINK_PREVIEW:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        case FILESAVE_AUTOEXTENSION:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            break;
        case FILEOPEN_PREVIEW:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            break;
        default:
            throw lang::IllegalArgumentException("Unknown template",
                                                 static_cast<XFilePicker*>( static_cast<XFilePicker3*>(this) ),
                                                 1 );
    }

    m_pControlHelper->initialize(templateId);

    implInitialize();
}

#pragma mark XCancellable

void SAL_CALL SalAquaFilePicker::cancel()
{
    SolarMutexGuard aGuard;

    if (m_pDialog != nil) {
        [m_pDialog cancel:nil];
    }
}

#pragma mark XEventListener

void SalAquaFilePicker::disposing( const lang::EventObject& aEvent )
{
    SolarMutexGuard aGuard;

    uno::Reference<XFilePickerListener> xFilePickerListener( aEvent.Source, css::uno::UNO_QUERY );

    if( xFilePickerListener.is() )
        removeFilePickerListener( xFilePickerListener );
}

#pragma mark XServiceInfo

OUString SAL_CALL SalAquaFilePicker::getImplementationName()
{
    return "com.sun.star.ui.dialogs.SalAquaFilePicker";
}

sal_Bool SAL_CALL SalAquaFilePicker::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence<OUString> SAL_CALL SalAquaFilePicker::getSupportedServiceNames()
{
    return FilePicker_getSupportedServiceNames();
}

#pragma mark Misc/Private

void SalAquaFilePicker::fileSelectionChanged( FilePickerEvent aEvent )
{
    if (m_xListener.is())
        m_xListener->fileSelectionChanged( aEvent );
}

void SalAquaFilePicker::directoryChanged( FilePickerEvent aEvent )
{
    if (m_xListener.is())
        m_xListener->directoryChanged( aEvent );
}

void SalAquaFilePicker::controlStateChanged( FilePickerEvent aEvent )
{
    if (m_xListener.is())
        m_xListener->controlStateChanged( aEvent );
}

void SalAquaFilePicker::dialogSizeChanged()
{
    if (m_xListener.is())
        m_xListener->dialogSizeChanged();
}


// Misc

void SalAquaFilePicker::ensureFilterHelper()
{
    SolarMutexGuard aGuard;

    if (nullptr == m_pFilterHelper) {
        m_pFilterHelper = new FilterHelper;
        m_pControlHelper->setFilterHelper(m_pFilterHelper);
        [m_pDelegate setFilterHelper:m_pFilterHelper];
    }
}

void SalAquaFilePicker::updateFilterUI()
{
    m_pControlHelper->updateFilterUI();
}

void SalAquaFilePicker::updateSaveFileNameExtension()
{
    if (m_nDialogType != NAVIGATIONSERVICES_SAVE) {
        return;
    }

    // we need to set this here again because initial setting does
    //[m_pDialog setExtensionHidden:YES];

    SolarMutexGuard aGuard;

    if (!m_pControlHelper->isAutoExtensionEnabled()) {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH // setAllowedFileTypes (12.0)
        [m_pDialog setAllowedFileTypes:nil];
        SAL_WNODEPRECATED_DECLARATIONS_POP
        [m_pDialog setAllowsOtherFileTypes:YES];
    } else {
        ensureFilterHelper();

        OUStringList aStringList = m_pFilterHelper->getCurrentFilterSuffixList();
        if( aStringList.empty()) // #i9328#
            return;

        OUString suffix = (*(aStringList.begin())).copy(1);
        NSString *requiredFileType = [NSString stringWithOUString:suffix];

        SAL_WNODEPRECATED_DECLARATIONS_PUSH // setAllowedFileTypes (12.0)
        [m_pDialog setAllowedFileTypes:[NSArray arrayWithObjects:requiredFileType, nil]];
        SAL_WNODEPRECATED_DECLARATIONS_POP

        [m_pDialog setAllowsOtherFileTypes:NO];
    }
}

void SalAquaFilePicker::filterControlChanged()
{
    if (m_pDialog == nil) {
        return;
    }

    SolarMutexGuard aGuard;

    updateSaveFileNameExtension();

    [m_pDialog validateVisibleColumns];

    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    controlStateChanged( evt );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
fpicker_SalAquaFilePicker_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SalAquaFilePicker());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
