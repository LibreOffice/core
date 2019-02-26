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

#include "VistaFilePicker.hxx"

#include "WinImplHelper.hxx"
#include "FPServiceInfo.hxx"
#include "shared.hxx"

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <officecfg/Office/Common.hxx>

#include <shlobj.h>

namespace fpicker{
namespace win32{
namespace vista{

namespace
{
    css::uno::Sequence< OUString > VistaFilePicker_getSupportedServiceNames()
    {
        css::uno::Sequence< OUString > aRet(3);
        aRet[0] = "com.sun.star.ui.dialogs.FilePicker";
        aRet[1] = "com.sun.star.ui.dialogs.SystemFilePicker";
        aRet[2] = "com.sun.star.ui.dialogs.SystemFolderPicker";
        return aRet;
    }
}

VistaFilePicker::VistaFilePicker(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR, bool bFolderPicker)
    : TVistaFilePickerBase  (m_aMutex                 )
    , m_xSMGR               (xSMGR                    )
    , m_rDialog             (new VistaFilePickerImpl())
    , m_aAsyncExecute       (m_rDialog                )
    , m_nFilePickerThreadId (0                        )
    , m_bInitialized        (false                    )
    , m_bFolderPicker       (bFolderPicker            )
{
}

VistaFilePicker::~VistaFilePicker()
{
}

void SAL_CALL VistaFilePicker::addFilePickerListener(const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_ADD_PICKER_LISTENER);
    rRequest->setArgument(PROP_PICKER_LISTENER, xListener);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

void SAL_CALL VistaFilePicker::removeFilePickerListener(const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_REMOVE_PICKER_LISTENER);
    rRequest->setArgument(PROP_PICKER_LISTENER, xListener);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

void VistaFilePicker::disposing(const css::lang::EventObject& /*aEvent*/)
{
}

void SAL_CALL VistaFilePicker::setMultiSelectionMode(sal_Bool bMode)
{
    ensureInit();

    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_MULTISELECTION_MODE);
    rRequest->setArgument(PROP_MULTISELECTION_MODE, bMode);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

void SAL_CALL VistaFilePicker::setTitle(const OUString& sTitle)
{
    ensureInit();

    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_TITLE);
    rRequest->setArgument(PROP_TITLE, sTitle);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

void SAL_CALL VistaFilePicker::appendFilter(const OUString& sTitle ,
                                            const OUString& sFilter)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_APPEND_FILTER);
    rRequest->setArgument(PROP_FILTER_TITLE, sTitle );
    rRequest->setArgument(PROP_FILTER_VALUE, sFilter);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

void SAL_CALL VistaFilePicker::setCurrentFilter(const OUString& sTitle)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_CURRENT_FILTER);
    rRequest->setArgument(PROP_FILTER_TITLE, sTitle);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

OUString SAL_CALL VistaFilePicker::getCurrentFilter()
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_CURRENT_FILTER);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);

    const  OUString sTitle = rRequest->getArgumentOrDefault(PROP_FILTER_TITLE, OUString());
    return sTitle;
}

void SAL_CALL VistaFilePicker::appendFilterGroup(const OUString&                              /*sGroupTitle*/,
                                                 const css::uno::Sequence< css::beans::StringPair >& rFilters   )
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_APPEND_FILTERGROUP);
    rRequest->setArgument(PROP_FILTER_GROUP, rFilters);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

void SAL_CALL VistaFilePicker::setDefaultName(const OUString& sName )
{
    ensureInit();

    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_DEFAULT_NAME);
    rRequest->setArgument(PROP_FILENAME, sName);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

void SAL_CALL VistaFilePicker::setDisplayDirectory(const OUString& sDirectory)
{
    ensureInit();

    bool bChanged = officecfg::Office::Common::Path::Info::WorkPathChanged::get(
        comphelper::getComponentContext(m_xSMGR));
    if (bChanged )
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create(
                comphelper::getComponentContext(m_xSMGR)));
        officecfg::Office::Common::Path::Info::WorkPathChanged::set(
            false, batch);
        batch->commit();
    }

    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_DIRECTORY);
    rRequest->setArgument(PROP_DIRECTORY, sDirectory);
    rRequest->setArgument(PROP_FORCE, bChanged);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

OUString SAL_CALL VistaFilePicker::getDisplayDirectory()
{
    ensureInit();

    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_DIRECTORY);
    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);
    const OUString sDirectory = rRequest->getArgumentOrDefault(PROP_DIRECTORY, OUString());

    return sDirectory;
}

// @deprecated can't be supported any longer ... see IDL description for further details
css::uno::Sequence< OUString > SAL_CALL VistaFilePicker::getFiles()
{
    css::uno::Sequence< OUString > lFiles = getSelectedFiles();
    // multiselection doesn't really work
    // so just retrieve the first url
    if (lFiles.getLength() > 1)
        lFiles.realloc(1);
    m_lLastFiles = lFiles;
    return lFiles;
}

css::uno::Sequence< OUString > SAL_CALL VistaFilePicker::getSelectedFiles()
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_SELECTED_FILES);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);

    const  css::uno::Sequence< OUString > lFiles = rRequest->getArgumentOrDefault(PROP_SELECTED_FILES, css::uno::Sequence< OUString >());
    m_lLastFiles = lFiles;
    return lFiles;
}

void VistaFilePicker::ensureInit()
{
    bool bInitialized(false);
    {
        osl::MutexGuard aGuard(m_aMutex);
        bInitialized = m_bInitialized;
    }

    if ( !bInitialized )
    {
        if (m_bFolderPicker)
        {
            RequestRef rRequest(new Request());
            rRequest->setRequest (VistaFilePickerImpl::E_CREATE_FOLDER_PICKER);
            if ( ! m_aAsyncExecute.isRunning())
                m_aAsyncExecute.create();
            m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
            {
                osl::MutexGuard aGuard(m_aMutex);
                m_bInitialized = true;
            }
        }
        else
        {
            css::uno::Sequence < css::uno::Any > aInitArguments(1);
            aInitArguments[0] <<= css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
            initialize(aInitArguments);
        }
    }
}

::sal_Int16 SAL_CALL VistaFilePicker::execute()
{
    ensureInit();

    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SHOW_DIALOG_MODAL);

    // if we want to show a modal window, the calling thread needs to process messages
    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::PROCESS_MESSAGES);

    const bool bOK          = rRequest->getArgumentOrDefault(PROP_DIALOG_SHOW_RESULT, false );
    m_lLastFiles = rRequest->getArgumentOrDefault(PROP_SELECTED_FILES    , css::uno::Sequence< OUString >());

    ::sal_Int16 nResult = css::ui::dialogs::ExecutableDialogResults::CANCEL;
    if (bOK)
        nResult = css::ui::dialogs::ExecutableDialogResults::OK;
    return nResult;
}

// XFilePicker

void SAL_CALL VistaFilePicker::setValue(      ::sal_Int16    nControlId    ,
                                              ::sal_Int16    nControlAction,
                                        const css::uno::Any& aValue        )
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_CONTROL_VALUE);
    rRequest->setArgument(PROP_CONTROL_ID    , nControlId    );
    rRequest->setArgument(PROP_CONTROL_ACTION, nControlAction);
    rRequest->setArgument(PROP_CONTROL_VALUE , aValue        );

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

css::uno::Any SAL_CALL VistaFilePicker::getValue(::sal_Int16 nControlId    ,
                                                 ::sal_Int16 nControlAction)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_CONTROL_VALUE);
    rRequest->setArgument(PROP_CONTROL_ID    , nControlId    );
    rRequest->setArgument(PROP_CONTROL_ACTION, nControlAction);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);
    return rRequest->getValue(PROP_CONTROL_VALUE);
}

void SAL_CALL VistaFilePicker::enableControl(::sal_Int16 nControlId,
                                             sal_Bool  bEnable   )
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_ENABLE_CONTROL);
    rRequest->setArgument(PROP_CONTROL_ID    , nControlId);
    rRequest->setArgument(PROP_CONTROL_ENABLE, bEnable   );

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

void SAL_CALL VistaFilePicker::setLabel(      ::sal_Int16      nControlId,
                                         const OUString& sLabel    )
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_SET_CONTROL_LABEL);
    rRequest->setArgument(PROP_CONTROL_ID   , nControlId);
    rRequest->setArgument(PROP_CONTROL_LABEL, sLabel    );

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);
}

OUString SAL_CALL VistaFilePicker::getLabel(::sal_Int16 nControlId)
{
    RequestRef rRequest(new Request());
    rRequest->setRequest (VistaFilePickerImpl::E_GET_CONTROL_LABEL);
    rRequest->setArgument(PROP_CONTROL_ID, nControlId);

    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::BLOCKED);
    const OUString sLabel = rRequest->getArgumentOrDefault(PROP_CONTROL_LABEL, OUString());
    return sLabel;
}

css::uno::Sequence< ::sal_Int16 > SAL_CALL VistaFilePicker::getSupportedImageFormats()
{
    return css::uno::Sequence< sal_Int16 >();
}

sal_Int32 SAL_CALL VistaFilePicker::getTargetColorDepth()
{
    return 0;
}

sal_Int32 SAL_CALL VistaFilePicker::getAvailableWidth()
{
    return 0;
}

sal_Int32 SAL_CALL VistaFilePicker::getAvailableHeight()
{
    return 0;
}

void SAL_CALL VistaFilePicker::setImage(      sal_Int16      /*nImageFormat*/,
                                         const css::uno::Any& /*aImage      */)
{
}

sal_Bool SAL_CALL VistaFilePicker::setShowState(sal_Bool /*bShowState*/)
{
    return false;
}

sal_Bool SAL_CALL VistaFilePicker::getShowState()
{
    return false;
}

void SAL_CALL VistaFilePicker::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
{
    if (lArguments.getLength() < 1)
        throw css::lang::IllegalArgumentException(
                "XInitialization::initialize() called without arguments.",
                static_cast< css::ui::dialogs::XFilePicker2* >( this ),
                1);

    sal_Int32         nTemplate = -1;
    lArguments[0] >>= nTemplate;

    bool bFileOpenDialog = true;
    ::sal_Int32 nFeatures       = 0;

    switch(nTemplate)
    {
        case css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE :
        {
            bFileOpenDialog = true;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE :
        {
            bFileOpenDialog = false;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD :
        {
            bFileOpenDialog  = false;
            nFeatures        |= FEATURE_AUTOEXTENSION;
            nFeatures        |= FEATURE_PASSWORD;
            nFeatures        |= FEATURE_GPGPASSWORD;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS :
        {
            bFileOpenDialog = false;
            nFeatures        |= FEATURE_AUTOEXTENSION;
            nFeatures        |= FEATURE_PASSWORD;
            nFeatures        |= FEATURE_FILTEROPTIONS;
            nFeatures        |= FEATURE_GPGPASSWORD;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION :
        {
            bFileOpenDialog  = false;
            nFeatures        |= FEATURE_AUTOEXTENSION;
            nFeatures        |= FEATURE_SELECTION;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE :
        {
            bFileOpenDialog  = false;
            nFeatures        |= FEATURE_AUTOEXTENSION;
            nFeatures        |= FEATURE_TEMPLATE;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE :
        {
            bFileOpenDialog  = true;
            nFeatures        |= FEATURE_LINK;
            nFeatures        |= FEATURE_PREVIEW;
            nFeatures        |= FEATURE_IMAGETEMPLATE;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR :
        {
            bFileOpenDialog  = true;
            nFeatures        |= FEATURE_LINK;
            nFeatures        |= FEATURE_PREVIEW;
            nFeatures        |= FEATURE_IMAGEANCHOR;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_PLAY :
        {
            bFileOpenDialog  = true;
            nFeatures        |= FEATURE_PLAY;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PLAY :
        {
            bFileOpenDialog  = true;
            nFeatures        |= FEATURE_LINK;
            nFeatures        |= FEATURE_PLAY;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION :
        {
            bFileOpenDialog  = true;
            nFeatures        |= FEATURE_READONLY;
            nFeatures        |= FEATURE_VERSION;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW :
        {
            bFileOpenDialog  = true;
            nFeatures        |= FEATURE_LINK;
            nFeatures        |= FEATURE_PREVIEW;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION :
        {
            bFileOpenDialog  = false;
            nFeatures        |= FEATURE_AUTOEXTENSION;
        }
        break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_PREVIEW :
        {
            bFileOpenDialog  = true;
            nFeatures        |= FEATURE_PREVIEW;
        }
        break;
    }
    css::uno::Reference<css::awt::XWindow> xParentWindow;
    if(lArguments.getLength() > 1)
    {
        lArguments[1] >>= xParentWindow;
    }
    RequestRef rRequest(new Request());
    if (bFileOpenDialog)
        rRequest->setRequest (VistaFilePickerImpl::E_CREATE_OPEN_DIALOG);
    else
        rRequest->setRequest (VistaFilePickerImpl::E_CREATE_SAVE_DIALOG);
    rRequest->setArgument(PROP_FEATURES, nFeatures);
    rRequest->setArgument(PROP_TEMPLATE_DESCR, nTemplate);
    if(xParentWindow.is())
        rRequest->setArgument(PROP_PARENT_WINDOW, xParentWindow);
    if ( ! m_aAsyncExecute.isRunning())
        m_aAsyncExecute.create();
    m_aAsyncExecute.triggerRequestThreadAware(rRequest, AsyncRequests::NON_BLOCKED);

    {
        osl::MutexGuard aGuard(m_aMutex);
        m_bInitialized = true;
    }
}

void SAL_CALL VistaFilePicker::cancel()
{
}

OUString SAL_CALL VistaFilePicker::getDirectory()
{
    ensureInit();
    css::uno::Sequence< OUString > aFileSeq = getSelectedFiles();
    assert(aFileSeq.getLength() <= 1);
    return aFileSeq.getLength() ? aFileSeq[0] : OUString();
}

void SAL_CALL VistaFilePicker::setDescription( const OUString& aDescription )
{
    setTitle(aDescription);
}

// XServiceInfo

OUString SAL_CALL VistaFilePicker::getImplementationName()
{
    if (m_bFolderPicker)
        return OUString(FOLDER_PICKER_IMPL_NAME);
    else
        return OUString(FILE_PICKER_IMPL_NAME);
}

sal_Bool SAL_CALL VistaFilePicker::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL VistaFilePicker::getSupportedServiceNames()
{
    return VistaFilePicker_getSupportedServiceNames();
}

} // namespace vista
} // namespace win32
} // namespace fpicker

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
