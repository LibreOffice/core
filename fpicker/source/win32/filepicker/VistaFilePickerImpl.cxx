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

#include "VistaFilePickerImpl.hxx"

// Without IFileDialogCustomize we can't do much
#ifdef __IFileDialogCustomize_INTERFACE_DEFINED__

#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <comphelper/sequenceasvector.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#ifdef __MINGW32__
#include <limits.h>
#endif
#include "../misc/WinImplHelper.hxx"

#include <shlguid.h>

 inline bool is_current_process_window(HWND hwnd)
{
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    return (pid == GetCurrentProcessId());
}

HWND choose_parent_window()
{
    HWND hwnd_parent = GetForegroundWindow();
    if (!is_current_process_window(hwnd_parent))
       hwnd_parent = GetDesktopWindow();
    return hwnd_parent;
}

namespace fpicker{
namespace win32{
namespace vista{

namespace css = ::com::sun::star;

//-----------------------------------------------------------------------------
// types, const etcpp.
//-----------------------------------------------------------------------------


static const ::sal_Int16 INVALID_CONTROL_ID     = -1;
static const ::sal_Int16 INVALID_CONTROL_ACTION = -1;

typedef ::comphelper::SequenceAsVector< ::rtl::OUString > TStringList;

// Guids used for IFileDialog::SetClientGuid
static const GUID CLIENTID_FILEDIALOG_SIMPLE        = {0xB8628FD3, 0xA3F5, 0x4845, 0x9B, 0x62, 0xD5, 0x1E, 0xDF, 0x97, 0xC4, 0x83};
static const GUID CLIENTID_FILEDIALOG_OPTIONS       = {0x93ED486F, 0x0D04, 0x4807, 0x8C, 0x44, 0xAC, 0x26, 0xCB, 0x6C, 0x5D, 0x36};
static const GUID CLIENTID_FILESAVE                 = {0x3B2E2261, 0x402D, 0x4049, 0xB0, 0xC0, 0x91, 0x13, 0xF8, 0x6E, 0x84, 0x7C};
static const GUID CLIENTID_FILESAVE_PASSWORD        = {0xC12D4F4C, 0x4D41, 0x4D4F, 0x97, 0xEF, 0x87, 0xF9, 0x8D, 0xB6, 0x1E, 0xA6};
static const GUID CLIENTID_FILESAVE_SELECTION       = {0x5B2482B3, 0x0358, 0x4E09, 0xAA, 0x64, 0x2B, 0x76, 0xB2, 0xA0, 0xDD, 0xFE};
static const GUID CLIENTID_FILESAVE_TEMPLATE        = {0x9996D877, 0x20D5, 0x424B, 0x9C, 0x2E, 0xD3, 0xB6, 0x31, 0xEC, 0xF7, 0xCE};
static const GUID CLIENTID_FILEOPEN_LINK_TEMPLATE   = {0x32237796, 0x1509, 0x49D1, 0xBB, 0x7E, 0x63, 0xAD, 0x36, 0xAE, 0x86, 0x8C};
static const GUID CLIENTID_FILEOPEN_PLAY            = {0x32CFB147, 0xF5AE, 0x4F90, 0xA1, 0xF1, 0x81, 0x20, 0x72, 0xBB, 0x2F, 0xC5};
static const GUID CLIENTID_FILEOPEN_LINK            = {0x39AC4BAE, 0x7D2D, 0x46BC, 0xBE, 0x2E, 0xF8, 0x8C, 0xB5, 0x65, 0x5E, 0x6A};

//-----------------------------------------------------------------------------
::rtl::OUString lcl_getURLFromShellItem (IShellItem* pItem)
{
    LPOLESTR pStr = NULL;
    ::rtl::OUString sURL;

    SIGDN   eConversion = SIGDN_FILESYSPATH;
    HRESULT hr          = pItem->GetDisplayName ( eConversion, &pStr );

    if ( FAILED(hr) )
    {
        eConversion = SIGDN_URL;
        hr          = pItem->GetDisplayName ( eConversion, &pStr );

        if ( FAILED(hr) )
            return ::rtl::OUString();

        sURL = ::rtl::OUString(reinterpret_cast<sal_Unicode*>(pStr));
    }
    else
    {
        ::osl::FileBase::getFileURLFromSystemPath( reinterpret_cast<sal_Unicode*>(pStr), sURL );
    }

    CoTaskMemFree (pStr);
    return sURL;
}

//-----------------------------------------------------------------------------------------
::std::vector< COMDLG_FILTERSPEC > lcl_buildFilterList(CFilterContainer& rContainer)
{
    const sal_Int32                          c      = rContainer.numFilter();
          sal_Int32                          i      = 0;
          ::std::vector< COMDLG_FILTERSPEC > lList  ;
          CFilterContainer::FILTER_ENTRY_T   aFilter;

    rContainer.beginEnumFilter( );
    while( rContainer.getNextFilter(aFilter) )
    {
        COMDLG_FILTERSPEC aSpec;

        aSpec.pszName = reinterpret_cast<LPCTSTR>(aFilter.first.getStr()) ;
        aSpec.pszSpec = reinterpret_cast<LPCTSTR>(aFilter.second.getStr());

        lList.push_back(aSpec);
    }

    return lList;
}

//-----------------------------------------------------------------------------------------
VistaFilePickerImpl::VistaFilePickerImpl()
    : m_iDialogOpen  ()
    , m_iDialogSave  ()
    , m_hLastResult  ()
    , m_lFilters     ()
    , m_lLastFiles   ()
    , m_iEventHandler(new VistaFilePickerEventHandler(this))
    , m_bInExecute   (sal_False)
    , m_bWasExecuted (sal_False)
    , m_sDirectory   ()
    , m_sFilename    ()
{
    m_hParentWindow = choose_parent_window();
}

//-------------------------------------------------------------------------------
VistaFilePickerImpl::~VistaFilePickerImpl()
{
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::before()
{
    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    // TRICKY .-)
    // osl::Thread class initializes COm already in MTA mode because it's needed
    // by VCL and UNO so. There is no way to change that from outside ...
    // but we need a STA environment ...
    // So we make it by try-and-error ...
    // If first CoInitialize will fail .. we unitialize COM initialize it new .-)

    m_hLastResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if ( FAILED(m_hLastResult) )
    {
        CoUninitialize();
        m_hLastResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::doRequest(const RequestRef& rRequest)
{
    try
    {
        switch(rRequest->getRequest())
        {
            case E_ADD_PICKER_LISTENER :
                    impl_sta_addFilePickerListener(rRequest);
                    break;

            case E_REMOVE_PICKER_LISTENER :
                    impl_sta_removeFilePickerListener(rRequest);
                    break;

            case E_APPEND_FILTER :
                    impl_sta_appendFilter(rRequest);
                    break;

            case E_APPEND_FILTERGROUP :
                    impl_sta_appendFilterGroup(rRequest);
                    break;

            case E_SET_CURRENT_FILTER :
                    impl_sta_setCurrentFilter(rRequest);
                    break;

            case E_GET_CURRENT_FILTER :
                    impl_sta_getCurrentFilter(rRequest);
                    break;

            case E_CREATE_OPEN_DIALOG :
                    impl_sta_CreateOpenDialog(rRequest);
                    break;

            case E_CREATE_SAVE_DIALOG :
                    impl_sta_CreateSaveDialog(rRequest);
                    break;

            case E_SET_MULTISELECTION_MODE :
                    impl_sta_SetMultiSelectionMode(rRequest);
                    break;

            case E_SET_TITLE :
                    impl_sta_SetTitle(rRequest);
                    break;

            case E_SET_FILENAME:
                impl_sta_SetFileName(rRequest);
                break;

            case E_SET_DIRECTORY :
                    impl_sta_SetDirectory(rRequest);
                    break;

            case E_GET_DIRECTORY :
                    impl_sta_GetDirectory(rRequest);
                    break;

            case E_SET_DEFAULT_NAME :
                    impl_sta_SetDefaultName(rRequest);
                    break;

            case E_GET_SELECTED_FILES :
                    impl_sta_getSelectedFiles(rRequest);
                    break;

            case E_SHOW_DIALOG_MODAL :
                    impl_sta_ShowDialogModal(rRequest);
                    break;

            case E_SET_CONTROL_VALUE :
                    impl_sta_SetControlValue(rRequest);
                    break;

            case E_GET_CONTROL_VALUE :
                    impl_sta_GetControlValue(rRequest);
                    break;

            case E_SET_CONTROL_LABEL :
                    impl_sta_SetControlLabel(rRequest);
                    break;

            case E_GET_CONTROL_LABEL :
                    impl_sta_GetControlLabel(rRequest);
                    break;

            case E_ENABLE_CONTROL :
                    impl_sta_EnableControl(rRequest);
                    break;

            // no default: let the compiler detect changes on enum ERequest !
        }
    }
    catch(...)
    {}
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::after()
{
    CoUninitialize();
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_addFilePickerListener(const RequestRef& rRequest)
{
    // SYNCHRONIZED outside !
    const css::uno::Reference< css::ui::dialogs::XFilePickerListener > xListener = rRequest->getArgumentOrDefault(PROP_PICKER_LISTENER, css::uno::Reference< css::ui::dialogs::XFilePickerListener >());
    if ( ! xListener.is())
        return;

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    TFileDialogEvents iHandler = m_iEventHandler;
    aLock.clear();
    // <- SYNCHRONIZED

    VistaFilePickerEventHandler* pHandlerImpl = (VistaFilePickerEventHandler*)iHandler.get();
    if (pHandlerImpl)
        pHandlerImpl->addFilePickerListener(xListener);
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_removeFilePickerListener(const RequestRef& rRequest)
{
    // SYNCHRONIZED outside !
    const css::uno::Reference< css::ui::dialogs::XFilePickerListener > xListener = rRequest->getArgumentOrDefault(PROP_PICKER_LISTENER, css::uno::Reference< css::ui::dialogs::XFilePickerListener >());
    if ( ! xListener.is())
        return;

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    TFileDialogEvents iHandler = m_iEventHandler;
    aLock.clear();
    // <- SYNCHRONIZED

    VistaFilePickerEventHandler* pHandlerImpl = (VistaFilePickerEventHandler*)iHandler.get();
    if (pHandlerImpl)
        pHandlerImpl->removeFilePickerListener(xListener);
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_appendFilter(const RequestRef& rRequest)
{
    const ::rtl::OUString sTitle  = rRequest->getArgumentOrDefault(PROP_FILTER_TITLE, ::rtl::OUString());
    const ::rtl::OUString sFilter = rRequest->getArgumentOrDefault(PROP_FILTER_VALUE, ::rtl::OUString());

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    m_lFilters.addFilter(sTitle, sFilter);
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_appendFilterGroup(const RequestRef& rRequest)
{
    const css::uno::Sequence< css::beans::StringPair > aFilterGroup  =
        rRequest->getArgumentOrDefault(PROP_FILTER_GROUP, css::uno::Sequence< css::beans::StringPair >());

    // SYNCHRONIZED->
    ::rtl::OUString aEmpty;
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    if ( m_lFilters.numFilter() > 0 && aFilterGroup.getLength() > 0 )
        m_lFilters.addFilter( STRING_SEPARATOR, aEmpty, sal_True );

    ::sal_Int32 c = aFilterGroup.getLength();
    ::sal_Int32 i = 0;
    for (i=0; i<c; ++i)
    {
        const css::beans::StringPair& rFilter = aFilterGroup[i];
        m_lFilters.addFilter(rFilter.First, rFilter.Second);
    }
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_setCurrentFilter(const RequestRef& rRequest)
{
    const ::rtl::OUString sTitle  = rRequest->getArgumentOrDefault(PROP_FILTER_TITLE, ::rtl::OUString());

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    m_lFilters.setCurrentFilter(sTitle);
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_getCurrentFilter(const RequestRef& rRequest)
{
    TFileDialog iDialog = impl_getBaseDialogInterface();
    UINT        nIndex  = UINT_MAX;
    HRESULT     hResult = iDialog->GetFileTypeIndex(&nIndex);
    if (
        ( FAILED(hResult)    ) ||
        ( nIndex == UINT_MAX )      // COM dialog sometimes return S_OK for empty filter lists .-(
       )
        return;

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    ::rtl::OUString sTitle;
    ::sal_Int32     nRealIndex = (nIndex-1); // COM dialog base on 1 ... filter container on 0 .-)
    if (
        (nRealIndex >= 0                         ) &&
        (m_lFilters.getFilter(nRealIndex, sTitle))
       )
        rRequest->setArgument(PROP_FILTER_TITLE, sTitle);
    else if ( nRealIndex == -1 ) // Dialog not visible yet
    {
        sTitle = m_lFilters.getCurrentFilter();
        rRequest->setArgument(PROP_FILTER_TITLE, sTitle);
    }

    aLock.clear();
    // <- SYNCHRONIZED
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_CreateOpenDialog(const RequestRef& rRequest)
{
    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    m_hLastResult = m_iDialogOpen.create();
    if (FAILED(m_hLastResult))
        return;

    TFileDialog iDialog;
#ifdef __MINGW32__
    m_iDialogOpen->QueryInterface(IID_IFileDialog, (void **)(&iDialog));
#else
    m_iDialogOpen.query(&iDialog);
#endif

    TFileDialogEvents iHandler = m_iEventHandler;

    aLock.clear();
    // <- SYNCHRONIZED

    DWORD nFlags = 0;
    iDialog->GetOptions ( &nFlags );

    nFlags &= ~FOS_FORCESHOWHIDDEN;
    nFlags |=  FOS_PATHMUSTEXIST;
    nFlags |=  FOS_FILEMUSTEXIST;
    nFlags |=  FOS_OVERWRITEPROMPT;
    nFlags |=  FOS_DONTADDTORECENT;

    iDialog->SetOptions ( nFlags );

    ::sal_Int32 nFeatures = rRequest->getArgumentOrDefault(PROP_FEATURES, (::sal_Int32)0);
    ::sal_Int32 nTemplate = rRequest->getArgumentOrDefault(PROP_TEMPLATE_DESCR, (::sal_Int32)0);
    impl_sta_enableFeatures(nFeatures, nTemplate);

    VistaFilePickerEventHandler* pHandlerImpl = (VistaFilePickerEventHandler*)iHandler.get();
    if (pHandlerImpl)
        pHandlerImpl->startListening(iDialog);
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_CreateSaveDialog(const RequestRef& rRequest)
{
    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    m_hLastResult = m_iDialogSave.create();
    if (FAILED(m_hLastResult))
        return;

    TFileDialogEvents  iHandler = m_iEventHandler;
    TFileDialog        iDialog;
#ifdef __MINGW32__
    m_iDialogSave->QueryInterface(IID_IFileDialog, (void **)(&iDialog));
#else
    m_iDialogSave.query(&iDialog);
#endif

    aLock.clear();
    // <- SYNCHRONIZED

    DWORD nFlags = 0;
    iDialog->GetOptions ( &nFlags );

    nFlags &= ~FOS_FORCESHOWHIDDEN;
    nFlags |=  FOS_PATHMUSTEXIST;
    nFlags |=  FOS_FILEMUSTEXIST;
    nFlags |=  FOS_OVERWRITEPROMPT;
    nFlags |=  FOS_DONTADDTORECENT;

    iDialog->SetOptions ( nFlags );

    ::sal_Int32 nFeatures = rRequest->getArgumentOrDefault(PROP_FEATURES, (::sal_Int32)0);
    ::sal_Int32 nTemplate = rRequest->getArgumentOrDefault(PROP_TEMPLATE_DESCR, (::sal_Int32)0);
    impl_sta_enableFeatures(nFeatures, nTemplate);

    VistaFilePickerEventHandler* pHandlerImpl = (VistaFilePickerEventHandler*)iHandler.get();
    if (pHandlerImpl)
        pHandlerImpl->startListening(iDialog);
}

//-------------------------------------------------------------------------------
static const ::sal_Int32 GROUP_VERSION         =   1;
static const ::sal_Int32 GROUP_TEMPLATE        =   2;
static const ::sal_Int32 GROUP_IMAGETEMPLATE   =   3;
static const ::sal_Int32 GROUP_CHECKBOXES      =   4;

//-------------------------------------------------------------------------------
static void setLabelToControl(CResourceProvider& rResourceProvider, TFileDialogCustomize iCustom, sal_uInt16 nControlId)
{
    ::rtl::OUString aLabel = rResourceProvider.getResString(nControlId);
    aLabel = SOfficeToWindowsLabel(aLabel);
    iCustom->SetControlLabel(nControlId, reinterpret_cast<LPCWSTR>(aLabel.getStr()) );
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_enableFeatures(::sal_Int32 nFeatures, ::sal_Int32 nTemplate)
{
    GUID aGUID = {};
    switch (nTemplate)
    {
        case css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE :
        case css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE :
            aGUID = CLIENTID_FILEDIALOG_SIMPLE;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION :
        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS :
            aGUID = CLIENTID_FILEDIALOG_OPTIONS;
            break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION :
            aGUID = CLIENTID_FILESAVE;
            break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD :
            aGUID = CLIENTID_FILESAVE_PASSWORD;
            break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION :
            aGUID = CLIENTID_FILESAVE_SELECTION;
            break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE :
            aGUID = CLIENTID_FILESAVE_TEMPLATE;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE :
            aGUID = CLIENTID_FILEOPEN_LINK_TEMPLATE;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_PLAY :
            aGUID = CLIENTID_FILEOPEN_PLAY;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW :
            aGUID = CLIENTID_FILEOPEN_LINK;
            break;
    }
    TFileDialog iDialog = impl_getBaseDialogInterface();
    iDialog->SetClientGuid ( aGUID );

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();

    if ((nFeatures & FEATURE_VERSION) == FEATURE_VERSION)
    {
        iCustom->StartVisualGroup (GROUP_VERSION, L"Version");
        iCustom->AddComboBox      (css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_VERSION);
        iCustom->EndVisualGroup   ();
        iCustom->MakeProminent    (GROUP_VERSION);
    }

    if ((nFeatures & FEATURE_TEMPLATE) == FEATURE_TEMPLATE)
    {
        iCustom->StartVisualGroup (GROUP_TEMPLATE, L"Template");
        iCustom->AddComboBox      (css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_TEMPLATE);
        iCustom->EndVisualGroup   ();
        iCustom->MakeProminent    (GROUP_TEMPLATE);
    }

    if ((nFeatures & FEATURE_IMAGETEMPLATE) == FEATURE_IMAGETEMPLATE)
    {
        iCustom->StartVisualGroup (GROUP_IMAGETEMPLATE, L"Style");
        iCustom->AddComboBox      (css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE);
        iCustom->EndVisualGroup   ();
        iCustom->MakeProminent    (GROUP_IMAGETEMPLATE);
    }

    iCustom->StartVisualGroup (GROUP_CHECKBOXES, L"");

    sal_uInt16 nControlId(0);
    if ((nFeatures & FEATURE_AUTOEXTENSION) == FEATURE_AUTOEXTENSION)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION;
        iCustom->AddCheckButton (nControlId, L"Auto Extension", true);
        setLabelToControl(m_ResProvider, iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_PASSWORD) == FEATURE_PASSWORD)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PASSWORD;
        iCustom->AddCheckButton (nControlId, L"Password", false);
        setLabelToControl(m_ResProvider, iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_READONLY) == FEATURE_READONLY)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_READONLY;
        iCustom->AddCheckButton (nControlId, L"Readonly", false);
        setLabelToControl(m_ResProvider, iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_FILTEROPTIONS) == FEATURE_FILTEROPTIONS)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS;
        iCustom->AddCheckButton (nControlId, L"Filter Options", false);
        setLabelToControl(m_ResProvider, iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_LINK) == FEATURE_LINK)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK;
        iCustom->AddCheckButton (nControlId, L"Link", false);
        setLabelToControl(m_ResProvider, iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_SELECTION) == FEATURE_SELECTION)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION;
        iCustom->AddCheckButton (nControlId, L"Selection", false);
        setLabelToControl(m_ResProvider, iCustom, nControlId);
    }

    /* can be ignored ... new COM dialog supports preview native now  !
    if ((nFeatures & FEATURE_PREVIEW) == FEATURE_PREVIEW)
        iCustom->AddCheckButton (css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, L"Preview", false);
    */

    iCustom->EndVisualGroup();

    if ((nFeatures & FEATURE_PLAY) == FEATURE_PLAY)
        iCustom->AddPushButton (css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY, L"Play");

}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_SetMultiSelectionMode(const RequestRef& rRequest)
{
    const ::sal_Bool bMultiSelection = rRequest->getArgumentOrDefault(PROP_MULTISELECTION_MODE, (::sal_Bool)sal_True);

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    TFileDialog iDialog = impl_getBaseDialogInterface();
    aLock.clear();
    // <- SYNCHRONIZED

    DWORD nFlags = 0;
    m_hLastResult = iDialog->GetOptions ( &nFlags );

    if (bMultiSelection)
        nFlags |=  FOS_ALLOWMULTISELECT;
    else
        nFlags &= ~FOS_ALLOWMULTISELECT;

    iDialog->SetOptions ( nFlags );
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_SetTitle(const RequestRef& rRequest)
{
    ::rtl::OUString sTitle = rRequest->getArgumentOrDefault(PROP_TITLE, ::rtl::OUString());

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    TFileDialog iDialog = impl_getBaseDialogInterface();
    aLock.clear();
    // <- SYNCHRONIZED

    iDialog->SetTitle(reinterpret_cast<LPCTSTR>(sTitle.getStr()));
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_SetFileName(const RequestRef& rRequest)
{
    ::rtl::OUString sFileName = rRequest->getArgumentOrDefault(PROP_FILENAME, ::rtl::OUString());

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    TFileDialog iDialog = impl_getBaseDialogInterface();
    aLock.clear();
    // <- SYNCHRONIZED

    iDialog->SetFileName(reinterpret_cast<LPCTSTR>(sFileName.getStr()));
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_SetDirectory(const RequestRef& rRequest)
{
    ::rtl::OUString sDirectory = rRequest->getArgumentOrDefault(PROP_DIRECTORY, ::rtl::OUString());
    bool            bForce     = rRequest->getArgumentOrDefault(PROP_FORCE, false);

    if( !m_bInExecute)
    {
        // Vista stores last used folders for file dialogs
        // so we don't want the application to change the folder
        // in most cases.
        // Store the requested folder in the mean time and decide later
        // what to do
        m_sDirectory = sDirectory;
    }

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    TFileDialog iDialog = impl_getBaseDialogInterface();
    aLock.clear();
    // <- SYNCHRONIZED

    ComPtr< IShellItem > pFolder;
#ifdef __MINGW32__
    HRESULT hResult = SHCreateItemFromParsingName ( reinterpret_cast<LPCTSTR>(sDirectory.getStr()), NULL, IID_IShellItem, reinterpret_cast<void**>(&pFolder) );
#else
    HRESULT hResult = SHCreateItemFromParsingName ( sDirectory.getStr(), NULL, IID_PPV_ARGS(&pFolder) );
#endif
    if ( FAILED(hResult) )
        return;

    if ( m_bInExecute || bForce )
        iDialog->SetFolder(pFolder);
    else
    {
        // Use set default folder as Microsoft recommends in the IFileDialog documentation.
        iDialog->SetDefaultFolder(pFolder);
    }
}

void VistaFilePickerImpl::impl_sta_GetDirectory(const RequestRef& rRequest)
{
    TFileDialog iDialog = impl_getBaseDialogInterface();
    ComPtr< IShellItem > pFolder;
    HRESULT hResult = iDialog->GetFolder( &pFolder );
    if ( FAILED(hResult) )
        return;
    ::rtl::OUString sFolder = lcl_getURLFromShellItem ( pFolder );
    if( sFolder.getLength())
        rRequest->setArgument( PROP_DIRECTORY, sFolder );
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_SetDefaultName(const RequestRef& rRequest)
{
    ::rtl::OUString sFilename = rRequest->getArgumentOrDefault(PROP_FILENAME, ::rtl::OUString());
    TFileDialog iDialog = impl_getBaseDialogInterface();

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if ( ! iCustom.is())
        return;

    // if we have the autoextension check box set, remove (or change ???) the extension of the filename
    // so that the autoextension mechanism can do its job
    BOOL bValue = FALSE;
    HRESULT hResult = iCustom->GetCheckButtonState( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, &bValue);
    if ( bValue )
    {
        sal_Int32 nSepPos = sFilename.lastIndexOf( '.' );
        if ( -1 != nSepPos )
            sFilename = sFilename.copy(0, nSepPos);
    }

    iDialog->SetFileName ( reinterpret_cast<LPCTSTR>(sFilename.getStr()));
    m_sFilename = sFilename;
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_setFiltersOnDialog()
{
    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    ::std::vector< COMDLG_FILTERSPEC > lFilters       = lcl_buildFilterList(m_lFilters);
    ::rtl::OUString                    sCurrentFilter = m_lFilters.getCurrentFilter();
    sal_Int32                          nCurrentFilter = m_lFilters.getFilterPos(sCurrentFilter);
    TFileDialog                        iDialog        = impl_getBaseDialogInterface();
    TFileDialogCustomize               iCustomize     = impl_getCustomizeInterface();

    aLock.clear();
    // <- SYNCHRONIZED

    if (lFilters.empty())
        return;

    COMDLG_FILTERSPEC   *pFilt = &lFilters[0];
    iDialog->SetFileTypes(lFilters.size(), pFilt/*&lFilters[0]*/);
    iDialog->SetFileTypeIndex(nCurrentFilter + 1);

    BOOL bValue = FALSE;
    HRESULT hResult = iCustomize->GetCheckButtonState( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, &bValue);

    if ( bValue )
    {
        LPCWSTR lpFilterExt = lFilters[0].pszSpec;

        lpFilterExt = wcsrchr( lpFilterExt, '.' );
        if ( lpFilterExt )
            lpFilterExt++;
        iDialog->SetDefaultExtension( lpFilterExt );
    }

}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_getSelectedFiles(const RequestRef& rRequest)
{
    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    TFileOpenDialog iOpen      = m_iDialogOpen;
    TFileSaveDialog iSave      = m_iDialogSave;
    ::sal_Bool      bInExecute = m_bInExecute;

    aLock.clear();
    // <- SYNCHRONIZED

    // ask dialog for results
    // Note        : we must differ between single/multi selection !
    // Note further: we must react different if dialog is in execute or not .-(
    ComPtr< IShellItem >      iItem;
    ComPtr< IShellItemArray > iItems;
    HRESULT                   hResult = E_FAIL;

    if (iOpen.is())
    {
        if (bInExecute)
            hResult = iOpen->GetSelectedItems(&iItems);
        else
        {
            hResult = iOpen->GetResults(&iItems);
            if (FAILED(hResult))
                hResult = iOpen->GetResult(&iItem);
        }
    }
    else
    if (iSave.is())
    {
        if (bInExecute)
            hResult = iSave->GetCurrentSelection(&iItem);
        else
            hResult = iSave->GetResult(&iItem);
    }

    if (FAILED(hResult))
        return;

    // convert and pack results
    TStringList lFiles;
    if (iItem.is())
    {
        const ::rtl::OUString sURL = lcl_getURLFromShellItem(iItem);
        if (sURL.getLength() > 0)
            lFiles.push_back(sURL);
    }

    if (iItems.is())
    {
        DWORD nCount;
        hResult = iItems->GetCount(&nCount);
        if ( SUCCEEDED(hResult) )
        {
            for (DWORD i=0; i<nCount; ++i)
            {
                hResult = iItems->GetItemAt(i, &iItem);
                if ( SUCCEEDED(hResult) )
                {
                    const ::rtl::OUString sURL = lcl_getURLFromShellItem(iItem);
                    if (sURL.getLength() > 0)
                        lFiles.push_back(sURL);
                }
            }
        }
    }

    rRequest->setArgument(PROP_SELECTED_FILES, lFiles.getAsConstList());
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_ShowDialogModal(const RequestRef& rRequest)
{
    impl_sta_setFiltersOnDialog();

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    TFileDialog iDialog = impl_getBaseDialogInterface();
    TFileOpenDialog iOpen = m_iDialogOpen;
    TFileSaveDialog iSave = m_iDialogSave;

    // it's important to know if we are showing the dialog.
    // Some dialog interface methods cant be called then or some
    // tasks must be done differently .-) (e.g. see impl_sta_getSelectedFiles())
    m_bInExecute = sal_True;

    m_bWasExecuted = sal_True;

    aLock.clear();
    // <- SYNCHRONIZED

    // we set the directory only if we have a save dialog and a filename
    // for the other cases, the file dialog remembers its last location
    // according to its client guid.
    if( m_sDirectory.getLength())
    {
        ComPtr< IShellItem > pFolder;
        #ifdef __MINGW32__
            HRESULT hResult = SHCreateItemFromParsingName ( reinterpret_cast<LPCTSTR>(m_sDirectory.getStr()), NULL, IID_IShellItem, reinterpret_cast<void**>(&pFolder) );
        #else
            HRESULT hResult = SHCreateItemFromParsingName ( m_sDirectory.getStr(), NULL, IID_PPV_ARGS(&pFolder) );
        #endif
        if ( SUCCEEDED(hResult) )
        {
            if (m_sFilename.getLength())
            {
                ::rtl::OUString aFileURL(m_sDirectory);
                sal_Int32 nIndex = aFileURL.lastIndexOf('/');
                if (nIndex != aFileURL.getLength()-1)
                    aFileURL += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/"));
                aFileURL += m_sFilename;

                TFileDialogCustomize iCustom = impl_getCustomizeInterface();

                BOOL bValue = FALSE;
                HRESULT hResult = iCustom->GetCheckButtonState( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, &bValue);
                if ( bValue )
                {
                    UINT nFileType;
                    hResult = iDialog->GetFileTypeIndex(&nFileType);
                    if ( SUCCEEDED(hResult) )
                    {
                        ::sal_Int32 nRealIndex = (nFileType-1); // COM dialog base on 1 ... filter container on 0 .-)
                        ::std::vector< COMDLG_FILTERSPEC > lFilters = lcl_buildFilterList(m_lFilters);
                        LPCWSTR lpFilterExt = lFilters[nRealIndex].pszSpec;

                        lpFilterExt = wcsrchr( lpFilterExt, '.' );
                        if ( lpFilterExt )
                            aFileURL += reinterpret_cast<const sal_Unicode*>(lpFilterExt);
                    }
                }

                // Check existence of file. Set folder only for this special case
                ::rtl::OUString aSystemPath;
                osl_getSystemPathFromFileURL( aFileURL.pData, &aSystemPath.pData );

                WIN32_FIND_DATA aFindFileData;
                HANDLE  hFind = FindFirstFile( reinterpret_cast<LPCWSTR>(aSystemPath.getStr()), &aFindFileData );
                if (hFind != INVALID_HANDLE_VALUE)
                    iDialog->SetFolder(pFolder);
                else
                    hResult = iDialog->AddPlace(pFolder, FDAP_TOP);

                FindClose( hFind );
            }
            else
                hResult = iDialog->AddPlace(pFolder, FDAP_TOP);
        }
    }


    HRESULT hResult = E_FAIL;
    try
    {
        // show dialog and wait for user decision
        if (iOpen.is())
            hResult = iOpen->Show( m_hParentWindow ); // parent window needed
        else
        if (iSave.is())
            hResult = iSave->Show( m_hParentWindow ); // parent window needed
    }
    catch(...)
    {}

    // SYNCHRONIZED->
    aLock.reset();
    m_bInExecute = sal_False;
    aLock.clear();
    // <- SYNCHRONIZED

    if ( FAILED(hResult) )
        return;

    impl_sta_getSelectedFiles(rRequest);
    rRequest->setArgument(PROP_DIALOG_SHOW_RESULT, sal_True);
}

//-------------------------------------------------------------------------------
TFileDialog VistaFilePickerImpl::impl_getBaseDialogInterface()
{
    TFileDialog iDialog;

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    if (m_iDialogOpen.is())
#ifdef __MINGW32__
        m_iDialogOpen->QueryInterface(IID_IFileDialog, (void**)(&iDialog));
#else
        m_iDialogOpen.query(&iDialog);
#endif
    if (m_iDialogSave.is())
#ifdef __MINGW32__
        m_iDialogSave->QueryInterface(IID_IFileDialog, (void**)(&iDialog));
#else
        m_iDialogSave.query(&iDialog);
#endif

    return iDialog;
}

//-------------------------------------------------------------------------------
TFileDialogCustomize VistaFilePickerImpl::impl_getCustomizeInterface()
{
    TFileDialogCustomize iCustom;

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    if (m_iDialogOpen.is())
#ifdef __MINGW32__
        m_iDialogOpen->QueryInterface(IID_IFileDialogCustomize, (void**)(&iCustom));
#else
        m_iDialogOpen.query(&iCustom);
#endif
    else
    if (m_iDialogSave.is())
#ifdef __MINGW32__
        m_iDialogSave->QueryInterface(IID_IFileDialogCustomize, (void**)(&iCustom));
#else
        m_iDialogSave.query(&iCustom);
#endif

    return iCustom;
}

//-------------------------------------------------------------------------------
void lcl_removeControlItemsWorkaround(const TFileDialogCustomize& iCustom   ,
                                            ::sal_Int16           nControlId)
{
    ::sal_Int32 i       = 0;
    HRESULT   hResult;

    hResult = iCustom->SetSelectedControlItem(nControlId, 1000);
    hResult = S_OK;
    while ( SUCCEEDED(hResult) )
        hResult = iCustom->RemoveControlItem(nControlId, i++);
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_SetControlValue(const RequestRef& rRequest)
{
    ::sal_Int16   nId     = rRequest->getArgumentOrDefault(PROP_CONTROL_ID    , INVALID_CONTROL_ID    );
    ::sal_Int16   nAction = rRequest->getArgumentOrDefault(PROP_CONTROL_ACTION, INVALID_CONTROL_ACTION);
    css::uno::Any aValue  = rRequest->getArgumentOrDefault(PROP_CONTROL_VALUE , css::uno::Any()       );

    // dont check for right values here ...
    // most parameters are optional !

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if ( ! iCustom.is())
        return;

    switch (nId)
    {
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PASSWORD :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_READONLY :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK :
        //case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW : // can be ignored ... preview is supported native now !
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION :
            {
                ::sal_Bool bValue   = sal_False;
                           aValue >>= bValue;
                iCustom->SetCheckButtonState(nId, bValue);
            }
            break;

        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_VERSION :
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_TEMPLATE :
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE :
            {
                HRESULT hResult;
                switch (nAction)
                {
                    case css::ui::dialogs::ControlActions::DELETE_ITEMS :
                        {
                            hResult = iCustom->RemoveAllControlItems(nId);
                            if ( FAILED(hResult) )
                                lcl_removeControlItemsWorkaround(iCustom, nId);
                        }
                        break;

                    case css::ui::dialogs::ControlActions::ADD_ITEMS :
                        {
                            css::uno::Sequence< ::rtl::OUString > lItems;
                                                       aValue >>= lItems;
                            for (::sal_Int32 i=0; i<lItems.getLength(); ++i)
                            {
                                const ::rtl::OUString& sItem = lItems[i];
                                hResult = iCustom->AddControlItem(nId, i, reinterpret_cast<LPCTSTR>(sItem.getStr()));
                            }
                        }
                        break;

                    case css::ui::dialogs::ControlActions::SET_SELECT_ITEM :
                        {
                            ::sal_Int32 nItem    = 0;
                                        aValue >>= nItem;
                            hResult = iCustom->SetSelectedControlItem(nId, nItem);
                        }
                        break;
                }
            }
            break;

        case css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY :
            {
            }
            break;
        }
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_GetControlValue(const RequestRef& rRequest)
{
    ::sal_Int16 nId     = rRequest->getArgumentOrDefault(PROP_CONTROL_ID    , INVALID_CONTROL_ID    );
    ::sal_Int16 nAction = rRequest->getArgumentOrDefault(PROP_CONTROL_ACTION, INVALID_CONTROL_ACTION);

    // dont check for right values here ...
    // most parameters are optional !

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if ( ! iCustom.is())
        return;

    css::uno::Any aValue;
    if( m_bWasExecuted )
    switch (nId)
    {
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PASSWORD :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_READONLY :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK :
        //case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW : // can be ignored ... preview is supported native now !
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION :
            {
                BOOL    bValue  = FALSE;
                HRESULT hResult = iCustom->GetCheckButtonState(nId, &bValue);
                if ( SUCCEEDED(hResult) )
                    aValue = css::uno::makeAny((sal_Bool)bValue);
            }
            break;
    }

    if (aValue.hasValue())
        rRequest->setArgument(PROP_CONTROL_VALUE, aValue);
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_SetControlLabel(const RequestRef& rRequest)
{
    ::sal_Int16     nId    = rRequest->getArgumentOrDefault(PROP_CONTROL_ID   , INVALID_CONTROL_ID  );
    ::rtl::OUString sLabel = rRequest->getArgumentOrDefault(PROP_CONTROL_LABEL, ::rtl::OUString() );

    // dont check for right values here ...
    // most parameters are optional !

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if ( ! iCustom.is())
        return;
    iCustom->SetControlLabel ( nId, reinterpret_cast<LPCTSTR>(sLabel.getStr()));
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_GetControlLabel(const RequestRef& /*rRequest*/)
{
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_EnableControl(const RequestRef& rRequest)
{
    ::sal_Int16 nId      = rRequest->getArgumentOrDefault(PROP_CONTROL_ID    , INVALID_CONTROL_ID  );
    ::sal_Bool  bEnabled = rRequest->getArgumentOrDefault(PROP_CONTROL_ENABLE, (::sal_Bool)sal_True);

    // dont check for right values here ...
    // most parameters are optional !

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if ( ! iCustom.is())
        return;

    CDCONTROLSTATEF eState = CDCS_VISIBLE;
    if (bEnabled)
        eState |= CDCS_ENABLED;
    else
        eState |= CDCS_INACTIVE;

    iCustom->SetControlState(nId, eState);
}
//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_SetDefaultExtension( const rtl::OUString& currentFilter )
{
   TFileDialog iDialog = impl_getBaseDialogInterface();
   if (currentFilter.getLength())
   {
        rtl::OUString FilterExt;
        m_lFilters.getFilter(currentFilter, FilterExt);

        sal_Int32 posOfPoint = FilterExt.indexOf(L'.');
        const sal_Unicode* pFirstExtStart = FilterExt.getStr() + posOfPoint + 1;

        sal_Int32 posOfSemiColon = FilterExt.indexOf(L';') - 1;
        if (posOfSemiColon < 0)
            posOfSemiColon = FilterExt.getLength() - 1;

        FilterExt = rtl::OUString(pFirstExtStart, posOfSemiColon - posOfPoint);
        iDialog->SetDefaultExtension ( reinterpret_cast<LPCTSTR>(FilterExt.getStr()) );
   }
}

static void impl_refreshFileDialog( TFileDialog iDialog )
{
    if ( SUCCEEDED(iDialog->SetFileName(L"")) &&
         SUCCEEDED(iDialog->SetFileName(L"*.*")) )
    {
        IOleWindow* iOleWindow;
#ifdef __MINGW32__
        if (SUCCEEDED(iDialog->QueryInterface(IID_IOleWindow, reinterpret_cast<void**>(&iOleWindow))))
#else
        if (SUCCEEDED(iDialog->QueryInterface(IID_PPV_ARGS(&iOleWindow))))
#endif
        {
            HWND hwnd;
            if (SUCCEEDED(iOleWindow->GetWindow(&hwnd)))
            {
                PostMessage(hwnd, WM_COMMAND, IDOK, 0);
            }
            iOleWindow->Release();
        }
    }
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::onAutoExtensionChanged (bool bChecked)
{
    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    const ::rtl::OUString sFilter = m_lFilters.getCurrentFilter ();
          ::rtl::OUString sExt    ;
    if ( !m_lFilters.getFilter (sFilter, sExt))
        return;

    TFileDialog iDialog = impl_getBaseDialogInterface();

    aLock.clear();
    // <- SYNCHRONIZED

    LPCWSTR pExt = 0;
    if ( bChecked )
    {
        pExt = reinterpret_cast<LPCTSTR>(sExt.getStr());
        pExt = wcsrchr( pExt, '.' );
        if ( pExt )
            pExt++;
    }
    iDialog->SetDefaultExtension( pExt );
}

bool VistaFilePickerImpl::onFileTypeChanged( UINT /*nTypeIndex*/ )
{
    return true;
}

} // namespace vista
} // namespace win32
} // namespace fpicker

#endif // __IFileDialogCustomize_INTERFACE_DEFINED__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
