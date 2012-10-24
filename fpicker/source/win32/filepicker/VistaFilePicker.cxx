/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef _MSC_VER
#pragma warning (disable:4917)
#endif

#include "VistaFilePicker.hxx"

// Without IFileDialogCustomize we can't do much
#ifdef __IFileDialogCustomize_INTERFACE_DEFINED__

#include "WinFileOpenImpl.hxx"
#include "../misc/WinImplHelper.hxx"
#include "shared.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <tchar.h>

#ifdef _MSC_VER
#pragma warning (push, 1)
#endif
#include <shlobj.h>
#ifdef _MSC_VER
#pragma warning (pop)
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

namespace css = ::com::sun::star;

namespace fpicker{
namespace win32{
namespace vista{

#define FILE_PICKER_DLL_NAME  TEXT("fps.dll")

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    // controling event notifications
    const bool STARTUP_SUSPENDED = true;
    const bool STARTUP_ALIVE     = false;

    css::uno::Sequence< ::rtl::OUString > SAL_CALL VistaFilePicker_getSupportedServiceNames()
    {
        css::uno::Sequence< ::rtl::OUString > aRet(2);
        aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.dialogs.FilePicker"));
        aRet[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.dialogs.SystemFilePicker"));
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
VistaFilePicker::VistaFilePicker(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : TVistaFilePickerBase  (m_aMutex                 )
    , m_xSMGR               (xSMGR                    )
    , m_rDialog             (new VistaFilePickerImpl())
    , m_aAsyncExecute       (m_rDialog                )
    , m_nFilePickerThreadId (0                        )
    , m_bInitialized        (false                    )
{
}

//-----------------------------------------------------------------------------------------
VistaFilePicker::~VistaFilePicker()
{
}

//------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::addFilePickerListener(const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener)
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_ADD_PICKER_LISTENER);
    rRequest->setArgument(PROP_PICKER_LISTENER, xListener);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::removeFilePickerListener(const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_REMOVE_PICKER_LISTENER);
    rRequest->setArgument(PROP_PICKER_LISTENER, xListener);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

// -------------------------------------------------
void SAL_CALL VistaFilePicker::disposing(const css::lang::EventObject& /*aEvent*/)
    throw(css::uno::RuntimeException)
{
}

//------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::setMultiSelectionMode(::sal_Bool bMode)
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_MULTISELECTION_MODE);
    rRequest->setArgument(PROP_MULTISELECTION_MODE, bMode);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::setTitle(const ::rtl::OUString& sTitle)
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_TITLE);
    rRequest->setArgument(PROP_TITLE, sTitle);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::appendFilter(const ::rtl::OUString& sTitle ,
                                            const ::rtl::OUString& sFilter)
    throw(css::lang::IllegalArgumentException,
          css::uno::RuntimeException         )
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_APPEND_FILTER);
    rRequest->setArgument(PROP_FILTER_TITLE, sTitle );
    rRequest->setArgument(PROP_FILTER_VALUE, sFilter);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::setCurrentFilter(const ::rtl::OUString& sTitle)
    throw(css::lang::IllegalArgumentException,
          css::uno::RuntimeException         )
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_CURRENT_FILTER);
    rRequest->setArgument(PROP_FILTER_TITLE, sTitle);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
::rtl::OUString SAL_CALL VistaFilePicker::getCurrentFilter()
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_CURRENT_FILTER);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);

    const  ::rtl::OUString sTitle = rRequest->getArgumentOrDefault(PROP_FILTER_TITLE, ::rtl::OUString());
    return sTitle;
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::appendFilterGroup(const ::rtl::OUString&                              /*sGroupTitle*/,
                                                 const css::uno::Sequence< css::beans::StringPair >& rFilters   )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_APPEND_FILTERGROUP);
    rRequest->setArgument(PROP_FILTER_GROUP, rFilters);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::setDefaultName(const ::rtl::OUString& sName )
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_DEFAULT_NAME);
    rRequest->setArgument(PROP_FILENAME, sName);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePicker::setDisplayDirectory(const ::rtl::OUString& sDirectory)
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    const ::rtl::OUString aPackage( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common/"));
    const ::rtl::OUString aRelPath( RTL_CONSTASCII_USTRINGPARAM("Path/Info"));
    const ::rtl::OUString aKey( RTL_CONSTASCII_USTRINGPARAM("WorkPathChanged"));

    css::uno::Any aValue = ::comphelper::ConfigurationHelper::readDirectKey(
        comphelper::getComponentContext(m_xSMGR), aPackage, aRelPath, aKey, ::comphelper::ConfigurationHelper::E_READONLY);

    bool bChanged(false);
    if (( aValue >>= bChanged ) && bChanged )
    {
        ::comphelper::ConfigurationHelper::writeDirectKey(
            comphelper::getComponentContext(m_xSMGR), aPackage, aRelPath, aKey, css::uno::makeAny(false), ::comphelper::ConfigurationHelper::E_STANDARD);
    }

    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_DIRECTORY);
    rRequest->setArgument(PROP_DIRECTORY, sDirectory);
    rRequest->setArgument(PROP_FORCE, bChanged);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
::rtl::OUString SAL_CALL VistaFilePicker::getDisplayDirectory()
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_DIRECTORY);
    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
    const ::rtl::OUString sDirectory = rRequest->getArgumentOrDefault(PROP_FILENAME, ::rtl::OUString());

    return sDirectory;
}

//-----------------------------------------------------------------------------------------
// @deprecated cant be supported any longer ... see IDL description for further details
css::uno::Sequence< ::rtl::OUString > SAL_CALL VistaFilePicker::getFiles()
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_SELECTED_FILES);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);

    const  css::uno::Sequence< ::rtl::OUString > lFiles = rRequest->getArgumentOrDefault(PROP_SELECTED_FILES, css::uno::Sequence< ::rtl::OUString >());
    m_lLastFiles = lFiles;
    return lFiles;
}

//-----------------------------------------------------------------------------------------
css::uno::Sequence< ::rtl::OUString > SAL_CALL VistaFilePicker::getSelectedFiles()
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_SELECTED_FILES);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);

    const  css::uno::Sequence< ::rtl::OUString > lFiles = rRequest->getArgumentOrDefault(PROP_SELECTED_FILES, css::uno::Sequence< ::rtl::OUString >());
    m_lLastFiles = lFiles;
    return lFiles;
}

//-----------------------------------------------------------------------------------------
::sal_Int16 SAL_CALL VistaFilePicker::execute()
    throw(css::uno::RuntimeException)
{
    bool bInitialized(false);
    {
        osl::MutexGuard aGuard(m_aMutex);
        bInitialized = m_bInitialized;
    }

    if ( !bInitialized )
    {
        sal_Int16 nTemplateDescription = css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
        css::uno::Sequence < css::uno::Any > aInitArguments(1);
        aInitArguments[0] <<= nTemplateDescription;
        initialize(aInitArguments);
    }

    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SHOW_DIALOG_MODAL);

    // if we want to show a modal window, the calling thread needs to process messages
    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::PROCESS_MESSAGES);

    const ::sal_Bool bOK          = rRequest->getArgumentOrDefault(PROP_DIALOG_SHOW_RESULT, (::sal_Bool)sal_False                  );
                     m_lLastFiles = rRequest->getArgumentOrDefault(PROP_SELECTED_FILES    , css::uno::Sequence< ::rtl::OUString >());

    ::sal_Int16 nResult = css::ui::dialogs::ExecutableDialogResults::CANCEL;
    if (bOK)
        nResult = css::ui::dialogs::ExecutableDialogResults::OK;
    return nResult;
}

//------------------------------------------------------------------------------------
// XFilePicker functions
//------------------------------------------------------------------------------------

void SAL_CALL VistaFilePicker::setValue(      ::sal_Int16    nControlId    ,
                                              ::sal_Int16    nControlAction,
                                        const css::uno::Any& aValue        )
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_CONTROL_VALUE);
    rRequest->setArgument(PROP_CONTROL_ID    , nControlId    );
    rRequest->setArgument(PROP_CONTROL_ACTION, nControlAction);
    rRequest->setArgument(PROP_CONTROL_VALUE , aValue        );

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

css::uno::Any SAL_CALL VistaFilePicker::getValue(::sal_Int16 nControlId    ,
                                                 ::sal_Int16 nControlAction)
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_CONTROL_VALUE);
    rRequest->setArgument(PROP_CONTROL_ID    , nControlId    );
    rRequest->setArgument(PROP_CONTROL_ACTION, nControlAction);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);
    const css::uno::Any aValue = rRequest->getArgumentOrDefault(PROP_CONTROL_VALUE, css::uno::Any());
    return aValue;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL VistaFilePicker::enableControl(::sal_Int16 nControlId,
                                             ::sal_Bool  bEnable   )
    throw(css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_ENABLE_CONTROL);
    rRequest->setArgument(PROP_CONTROL_ID    , nControlId);
    rRequest->setArgument(PROP_CONTROL_ENABLE, bEnable   );

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL VistaFilePicker::setLabel(      ::sal_Int16      nControlId,
                                         const ::rtl::OUString& sLabel    )
    throw (css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_CONTROL_LABEL);
    rRequest->setArgument(PROP_CONTROL_ID   , nControlId);
    rRequest->setArgument(PROP_CONTROL_LABEL, sLabel    );

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

::rtl::OUString SAL_CALL VistaFilePicker::getLabel(::sal_Int16 nControlId)
    throw (css::uno::RuntimeException)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_CONTROL_LABEL);
    rRequest->setArgument(PROP_CONTROL_ID, nControlId);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);
    const ::rtl::OUString sLabel = rRequest->getArgumentOrDefault(PROP_CONTROL_LABEL, ::rtl::OUString());
    return sLabel;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

css::uno::Sequence< ::sal_Int16 > SAL_CALL VistaFilePicker::getSupportedImageFormats()
    throw (css::uno::RuntimeException)
{
    return css::uno::Sequence< sal_Int16 >();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL VistaFilePicker::getTargetColorDepth()
    throw (css::uno::RuntimeException)
{
    return 0;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL VistaFilePicker::getAvailableWidth()
    throw (css::uno::RuntimeException)
{
    return 0;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL VistaFilePicker::getAvailableHeight()
    throw (css::uno::RuntimeException)
{
    return 0;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void SAL_CALL VistaFilePicker::setImage(      sal_Int16      /*nImageFormat*/,
                                         const css::uno::Any& /*aImage      */)
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Bool SAL_CALL VistaFilePicker::setShowState(sal_Bool /*bShowState*/)
    throw (css::uno::RuntimeException)
{
    return sal_False;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Bool SAL_CALL VistaFilePicker::getShowState()
    throw (css::uno::RuntimeException)
{
    return sal_False;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void SAL_CALL VistaFilePicker::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    if (lArguments.getLength() < 1)
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "XInitialization::initialize() called without arguments." )),
                static_cast< css::ui::dialogs::XFilePicker2* >( this ),
                1);

    sal_Int32         nTemplate = -1;
    lArguments[0] >>= nTemplate;

    ::sal_Bool  bFileOpenDialog = sal_True;
    ::sal_Int32 nFeatures       = 0;

    switch(nTemplate)
    {
        case css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE :
        {
            bFileOpenDialog = sal_True;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE :
        {
            bFileOpenDialog = sal_False;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD :
        {
            bFileOpenDialog  = sal_False;
            nFeatures        |= FEATURE_AUTOEXTENSION;
            nFeatures        |= FEATURE_PASSWORD;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS :
        {
            bFileOpenDialog = sal_False;
            nFeatures        |= FEATURE_AUTOEXTENSION;
            nFeatures        |= FEATURE_PASSWORD;
            nFeatures        |= FEATURE_FILTEROPTIONS;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION :
        {
            bFileOpenDialog  = sal_False;
            nFeatures        |= FEATURE_AUTOEXTENSION;
            nFeatures        |= FEATURE_SELECTION;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE :
        {
            bFileOpenDialog  = sal_False;
            nFeatures        |= FEATURE_AUTOEXTENSION;
            nFeatures        |= FEATURE_TEMPLATE;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE :
        {
            bFileOpenDialog  = sal_True;
            nFeatures        |= FEATURE_LINK;
            nFeatures        |= FEATURE_PREVIEW;
            nFeatures        |= FEATURE_IMAGETEMPLATE;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_PLAY :
        {
            bFileOpenDialog  = sal_True;
            nFeatures        |= FEATURE_PLAY;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION :
        {
            bFileOpenDialog  = sal_True;
            nFeatures        |= FEATURE_READONLY;
            nFeatures        |= FEATURE_VERSION;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW :
        {
            bFileOpenDialog  = sal_True;
            nFeatures        |= FEATURE_LINK;
            nFeatures        |= FEATURE_PREVIEW;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION :
        {
            bFileOpenDialog  = sal_False;
            nFeatures        |= FEATURE_AUTOEXTENSION;
        }
        break;
    }

    RequestRef rRequest(new Request());
    if (bFileOpenDialog)
        rRequest->setRequest (VistaFilePickerImpl::E_CREATE_OPEN_DIALOG);
    else
        rRequest->setRequest (VistaFilePickerImpl::E_CREATE_SAVE_DIALOG);
    rRequest->setArgument(PROP_FEATURES, nFeatures);
    rRequest->setArgument(PROP_TEMPLATE_DESCR, nTemplate);
    if ( ! m_aAsyncExecute.isRunning())
        m_aAsyncExecute.create();
    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);

    {
        osl::MutexGuard aGuard(m_aMutex);
        m_bInitialized = true;
    }
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void SAL_CALL VistaFilePicker::cancel()
    throw(css::uno::RuntimeException)
{
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

::rtl::OUString SAL_CALL VistaFilePicker::getImplementationName()
    throw(css::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.fpicker.VistaFileDialog"));
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL VistaFilePicker::supportsService(const ::rtl::OUString& sServiceName)
    throw(css::uno::RuntimeException )
{
    css::uno::Sequence< ::rtl::OUString > lSupportedServicesNames = VistaFilePicker_getSupportedServiceNames();

    for (sal_Int32 n = lSupportedServicesNames.getLength(); n--;)
        if (lSupportedServicesNames[n].compareTo(sServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

css::uno::Sequence< ::rtl::OUString > SAL_CALL VistaFilePicker::getSupportedServiceNames()
    throw(css::uno::RuntimeException)
{
    return VistaFilePicker_getSupportedServiceNames();
}

} // namespace vista
} // namespace win32
} // namespace fpicker

#endif // __IFileDialogCustomize_INTERFACE_DEFINED__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
