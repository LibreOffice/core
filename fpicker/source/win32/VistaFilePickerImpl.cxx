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

#include <sal/config.h>

#include <memory>

#include "VistaFilePickerImpl.hxx"

#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <comphelper/sequence.hxx>
#include <fpicker/strings.hrc>
#include <fpicker/fpsofficeResMgr.hxx>
#include <osl/file.hxx>
#include <rtl/process.h>
#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/string_view.hxx>
#include <vcl/svapp.hxx>
#include "WinImplHelper.hxx"

#include <shlguid.h>
#include <shlobj.h>

static bool is_current_process_window(HWND hwnd)
{
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    return (pid == GetCurrentProcessId());
}

static HWND choose_parent_window()
{
    HWND hwnd_parent = GetForegroundWindow();
    if (!is_current_process_window(hwnd_parent))
       hwnd_parent = GetDesktopWindow();
    return hwnd_parent;
}

namespace {

bool createFolderItem(OUString const& url, sal::systools::COMReference<IShellItem>& folder)
{
    OUString path;
    if (osl::FileBase::getSystemPathFromFileURL(url, path)
        != osl::FileBase::E_None)
    {
        return false;
    }
    HRESULT res = SHCreateItemFromParsingName(
        o3tl::toW(path.getStr()), nullptr,
        IID_PPV_ARGS(&folder));
    return SUCCEEDED(res);
}

}

namespace fpicker{
namespace win32{
namespace vista{


// types, const etcpp.


const ::sal_Int16 INVALID_CONTROL_ID     = -1;
const ::sal_Int16 INVALID_CONTROL_ACTION = -1;

// Guids used for IFileDialog::SetClientGuid
const GUID CLIENTID_FILEDIALOG_SIMPLE        = {0xB8628FD3, 0xA3F5, 0x4845, 0x9B, 0x62, 0xD5, 0x1E, 0xDF, 0x97, 0xC4, 0x83};
const GUID CLIENTID_FILEDIALOG_OPTIONS       = {0x93ED486F, 0x0D04, 0x4807, 0x8C, 0x44, 0xAC, 0x26, 0xCB, 0x6C, 0x5D, 0x36};
const GUID CLIENTID_FILESAVE_PASSWORD        = {0xC12D4F4C, 0x4D41, 0x4D4F, 0x97, 0xEF, 0x87, 0xF9, 0x8D, 0xB6, 0x1E, 0xA6};
const GUID CLIENTID_FILESAVE_SELECTION       = {0x5B2482B3, 0x0358, 0x4E09, 0xAA, 0x64, 0x2B, 0x76, 0xB2, 0xA0, 0xDD, 0xFE};
const GUID CLIENTID_FILESAVE_TEMPLATE        = {0x9996D877, 0x20D5, 0x424B, 0x9C, 0x2E, 0xD3, 0xB6, 0x31, 0xEC, 0xF7, 0xCE};
const GUID CLIENTID_FILEOPEN_LINK_TEMPLATE   = {0x32237796, 0x1509, 0x49D1, 0xBB, 0x7E, 0x63, 0xAD, 0x36, 0xAE, 0x86, 0x8C};
const GUID CLIENTID_FILEOPEN_LINK_ANCHOR     = {0xBE3188CB, 0x399A, 0x45AE, 0x8F, 0x78, 0x75, 0x17, 0xAF, 0x26, 0x81, 0xEA};
const GUID CLIENTID_FILEOPEN_PLAY            = {0x32CFB147, 0xF5AE, 0x4F90, 0xA1, 0xF1, 0x81, 0x20, 0x72, 0xBB, 0x2F, 0xC5};
const GUID CLIENTID_FILEOPEN_LINK            = {0x39AC4BAE, 0x7D2D, 0x46BC, 0xBE, 0x2E, 0xF8, 0x8C, 0xB5, 0x65, 0x5E, 0x6A};


class TDialogImplBase
{
public:
    TDialogImplBase(IFileDialog* iDialog)
        : m_iDialog(iDialog)
    {
    }

    virtual ~TDialogImplBase() = default;

    TFileDialog getComPtr() { return m_iDialog; }
    virtual sal::systools::COMReference<IShellItemArray> getResult(bool bInExecute)
    {
        sal::systools::COMReference<IShellItem> iItem;
        if (m_iDialog.is())
        {
            if (bInExecute)
                m_iDialog->GetCurrentSelection(&iItem);
            else
                m_iDialog->GetResult(&iItem);
        }
        void* iItems = nullptr;
        if (iItem.is())
            SHCreateShellItemArrayFromShellItem(iItem.get(), IID_IShellItemArray, &iItems);
        return static_cast<IShellItemArray*>(iItems);
    }

private:
    TFileDialog m_iDialog;
};

namespace {

template <class ComPtrDialog, REFCLSID CLSID> class TDialogImpl : public TDialogImplBase
{
public:
    TDialogImpl()
        : TDialogImplBase(ComPtrDialog(CLSID).get())
    {
    }
};

class TOpenDialogImpl : public TDialogImpl<TFileOpenDialog, CLSID_FileOpenDialog>
{
public:
    sal::systools::COMReference<IShellItemArray> getResult(bool bInExecute) override
    {
        sal::systools::COMReference<IShellItemArray> iItems;
        TFileOpenDialog iDialog(getComPtr(), sal::systools::COM_QUERY_THROW);
        bool bGetResult = false;
        if (!iDialog.is())
            bGetResult = true;
        else if (FAILED(bInExecute ? iDialog->GetSelectedItems(&iItems) : iDialog->GetResults(&iItems)))
            bGetResult = true;

        if (bGetResult)
            iItems = TDialogImplBase::getResult(bInExecute);

        return iItems;
    }
};

}

using TSaveDialogImpl = TDialogImpl<TFileSaveDialog, CLSID_FileSaveDialog>;
using TFolderPickerDialogImpl = TDialogImpl<TFileOpenDialog, CLSID_FileOpenDialog>;


static OUString lcl_getURLFromShellItem (IShellItem* pItem)
{
    LPWSTR pStr = nullptr;
    OUString sURL;
    HRESULT hr;

    hr = pItem->GetDisplayName ( SIGDN_FILESYSPATH, &pStr );
    if (SUCCEEDED(hr))
    {
        ::osl::FileBase::getFileURLFromSystemPath( OUString(o3tl::toU(pStr)), sURL );
        goto cleanup;
    }

    hr = pItem->GetDisplayName ( SIGDN_URL, &pStr );
    if (SUCCEEDED(hr))
    {
        sURL = o3tl::toU(pStr);
        goto cleanup;
    }

    hr = pItem->GetDisplayName ( SIGDN_PARENTRELATIVEPARSING, &pStr );
    if (SUCCEEDED(hr))
    {
        GUID known_folder_id;
        std::wstring aStr = pStr;
        CoTaskMemFree (pStr);

        if (0 == aStr.compare(0, 3, L"::{"))
            aStr = aStr.substr(2);
        hr = IIDFromString(aStr.c_str(), &known_folder_id);
        if (SUCCEEDED(hr))
        {
            hr = SHGetKnownFolderPath(known_folder_id, 0, nullptr, &pStr);
            if (SUCCEEDED(hr))
            {
                ::osl::FileBase::getFileURLFromSystemPath(OUString(o3tl::toU(pStr)), sURL);
                goto cleanup;
            }
        }
    }

    // Default fallback
    hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pStr);
    if (SUCCEEDED(hr))
        ::osl::FileBase::getFileURLFromSystemPath(OUString(o3tl::toU(pStr)), sURL);
    else // shouldn't happen...
        goto bailout;

cleanup:
    CoTaskMemFree (pStr);
bailout:
    return sURL;
}

// Vista file picker shows the filter mask next to filter name in the list; so we need to remove the
// mask from the filter name to avoid duplicating masks
static OUString lcl_AdjustFilterName(const OUString& sName)
{
    const sal_Int32 idx = sName.indexOf("(.");
    return (idx > 0) ? OUString(o3tl::trim(sName.subView(0, idx))) : sName;
}

// rvStrings holds the OUStrings, pointers to which data are stored in returned COMDLG_FILTERSPEC
static ::std::vector<COMDLG_FILTERSPEC> lcl_buildFilterList(CFilterContainer& rContainer,
                                                            std::vector<OUString>& rvStrings)
{
    ::std::vector< COMDLG_FILTERSPEC > lList  ;
    CFilterContainer::FILTER_ENTRY_T   aFilter;

    rContainer.beginEnumFilter( );
    while( rContainer.getNextFilter(aFilter) )
    {
        COMDLG_FILTERSPEC aSpec;

        rvStrings.push_back(lcl_AdjustFilterName(aFilter.first)); // to avoid dangling pointer
        aSpec.pszName = o3tl::toW(rvStrings.back().getStr());
        aSpec.pszSpec = o3tl::toW(aFilter.second.getStr());

        lList.push_back(aSpec);
    }

    return lList;
}


VistaFilePickerImpl::VistaFilePickerImpl()
    : m_lFilters     ()
    , m_iEventHandler(new VistaFilePickerEventHandler(this))
    , m_bInExecute   (false)
    , m_bWasExecuted (false)
    , m_hParentWindow(nullptr)
    , m_sDirectory   ()
    , m_sFilename    ()
{
}


VistaFilePickerImpl::~VistaFilePickerImpl()
{
}


void VistaFilePickerImpl::doRequest(Request& rRequest)
{
    try
    {
        switch(rRequest.getRequest())
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

            case E_CREATE_FOLDER_PICKER:
                    impl_sta_CreateFolderPicker(rRequest);
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


void VistaFilePickerImpl::impl_sta_addFilePickerListener(Request& rRequest)
{
    const css::uno::Reference< css::ui::dialogs::XFilePickerListener > xListener = rRequest.getArgumentOrDefault(PROP_PICKER_LISTENER, css::uno::Reference< css::ui::dialogs::XFilePickerListener >());
    if ( ! xListener.is())
        return;

    if (m_iEventHandler.is())
    {
        auto* pHandlerImpl = static_cast<VistaFilePickerEventHandler*>(m_iEventHandler.get());
        pHandlerImpl->addFilePickerListener(xListener);
    }
}


void VistaFilePickerImpl::impl_sta_removeFilePickerListener(Request& rRequest)
{
    const css::uno::Reference< css::ui::dialogs::XFilePickerListener > xListener = rRequest.getArgumentOrDefault(PROP_PICKER_LISTENER, css::uno::Reference< css::ui::dialogs::XFilePickerListener >());
    if ( ! xListener.is())
        return;

    if (m_iEventHandler.is())
    {
        auto* pHandlerImpl = static_cast<VistaFilePickerEventHandler*>(m_iEventHandler.get());
        pHandlerImpl->removeFilePickerListener(xListener);
    }
}


void VistaFilePickerImpl::impl_sta_appendFilter(Request& rRequest)
{
    const OUString sTitle  = rRequest.getArgumentOrDefault(PROP_FILTER_TITLE, OUString());
    const OUString sFilter = rRequest.getArgumentOrDefault(PROP_FILTER_VALUE, OUString());

    m_lFilters.addFilter(sTitle, sFilter);
}


void VistaFilePickerImpl::impl_sta_appendFilterGroup(Request& rRequest)
{
    const css::uno::Sequence< css::beans::StringPair > aFilterGroup  =
        rRequest.getArgumentOrDefault(PROP_FILTER_GROUP, css::uno::Sequence< css::beans::StringPair >());

    if ( m_lFilters.numFilter() > 0 && aFilterGroup.getLength() > 0 )
        m_lFilters.addFilter( STRING_SEPARATOR, "", true );

    ::sal_Int32 c = aFilterGroup.getLength();
    ::sal_Int32 i = 0;
    for (i=0; i<c; ++i)
    {
        const css::beans::StringPair& rFilter = aFilterGroup[i];
        m_lFilters.addFilter(rFilter.First, rFilter.Second);
    }
}


void VistaFilePickerImpl::impl_sta_setCurrentFilter(Request& rRequest)
{
    const OUString sTitle  = rRequest.getArgumentOrDefault(PROP_FILTER_TITLE, OUString());

    m_lFilters.setCurrentFilter(sTitle);
}


void VistaFilePickerImpl::impl_sta_getCurrentFilter(Request& rRequest)
{
    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    UINT        nIndex  = UINT_MAX;
    HRESULT     hResult = iDialog->GetFileTypeIndex(&nIndex);
    if (
        ( FAILED(hResult)    ) ||
        ( nIndex == UINT_MAX )      // COM dialog sometimes return S_OK for empty filter lists .-(
       )
        return;

    OUString sTitle;
    ::sal_Int32     nRealIndex = nIndex-1; // COM dialog base on 1 ... filter container on 0 .-)
    if (
        (nRealIndex >= 0                         ) &&
        (m_lFilters.getFilterNameByIndex(nRealIndex, sTitle))
       )
        rRequest.setArgument(PROP_FILTER_TITLE, sTitle);
    else if ( nRealIndex == -1 ) // Dialog not visible yet
    {
        sTitle = m_lFilters.getCurrentFilter();
        rRequest.setArgument(PROP_FILTER_TITLE, sTitle);
    }
}


template <class TDialogImplClass> void VistaFilePickerImpl::impl_sta_CreateDialog()
{
    m_pDialog = std::make_shared<TDialogImplClass>();
}


void VistaFilePickerImpl::impl_sta_InitDialog(Request& rRequest, DWORD nOrFlags)
{
    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    DWORD nFlags = 0;
    iDialog->GetOptions ( &nFlags );

    nFlags &= ~FOS_FORCESHOWHIDDEN;
    nFlags |=  FOS_PATHMUSTEXIST;
    nFlags |=  FOS_DONTADDTORECENT;
    nFlags |= nOrFlags;

    iDialog->SetOptions ( nFlags );

    css::uno::Reference<css::awt::XWindow> xWindow = rRequest.getArgumentOrDefault(PROP_PARENT_WINDOW, css::uno::Reference<css::awt::XWindow>());
    if(xWindow.is())
    {
        css::uno::Reference<css::awt::XSystemDependentWindowPeer> xSysDepWin(xWindow,css::uno::UNO_QUERY);
        if(xSysDepWin.is()) {
            css::uno::Sequence<sal_Int8> aProcessIdent(16);
            rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8*>(aProcessIdent.getArray()));
            css::uno::Any aAny = xSysDepWin->getWindowHandle(aProcessIdent,css::lang::SystemDependent::SYSTEM_WIN32);
            sal_Int64 tmp = 0;
            aAny >>= tmp;
            if(tmp != 0)
            {
                m_hParentWindow = reinterpret_cast<HWND>(tmp);
            }
        }
    }

    ::sal_Int32 nFeatures = rRequest.getArgumentOrDefault(PROP_FEATURES, ::sal_Int32(0));
    ::sal_Int32 nTemplate = rRequest.getArgumentOrDefault(PROP_TEMPLATE_DESCR, ::sal_Int32(0));
    impl_sta_enableFeatures(nFeatures, nTemplate);

    if (m_iEventHandler.is())
    {
        auto* pHandlerImpl = static_cast<VistaFilePickerEventHandler*>(m_iEventHandler.get());
        pHandlerImpl->startListening(iDialog);
    }
}


void VistaFilePickerImpl::impl_sta_CreateOpenDialog(Request& rRequest)
{
    impl_sta_CreateDialog<TOpenDialogImpl>();
    impl_sta_InitDialog(rRequest, FOS_FILEMUSTEXIST | FOS_OVERWRITEPROMPT);
}


void VistaFilePickerImpl::impl_sta_CreateSaveDialog(Request& rRequest)
{
    impl_sta_CreateDialog<TSaveDialogImpl>();
    impl_sta_InitDialog(rRequest, FOS_FILEMUSTEXIST | FOS_OVERWRITEPROMPT);
}


void VistaFilePickerImpl::impl_sta_CreateFolderPicker(Request& rRequest)
{
    impl_sta_CreateDialog<TFolderPickerDialogImpl>();
    impl_sta_InitDialog(rRequest, FOS_PICKFOLDERS);
}


const ::sal_Int32 GROUP_VERSION         =   1;
const ::sal_Int32 GROUP_TEMPLATE        =   2;
const ::sal_Int32 GROUP_IMAGETEMPLATE   =   3;
const ::sal_Int32 GROUP_CHECKBOXES      =   4;
const ::sal_Int32 GROUP_IMAGEANCHOR     =   5;


static void setLabelToControl(TFileDialogCustomize iCustom, sal_uInt16 nControlId)
{
    OUString aLabel = CResourceProvider::getResString(nControlId);
    aLabel = SOfficeToWindowsLabel(aLabel);
    iCustom->SetControlLabel(nControlId, o3tl::toW(aLabel.getStr()) );
}


void VistaFilePickerImpl::impl_sta_enableFeatures(::sal_Int32 nFeatures, ::sal_Int32 nTemplate)
{
    GUID aGUID = {};
    switch (nTemplate)
    {
        case css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE :
        case css::ui::dialogs::TemplateDescription::FILEOPEN_PREVIEW :
        case css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE :
            aGUID = CLIENTID_FILEDIALOG_SIMPLE;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION :
        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS :
            aGUID = CLIENTID_FILEDIALOG_OPTIONS;
            break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD :
            aGUID = CLIENTID_FILESAVE_PASSWORD;
            break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION :
        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION :
            aGUID = CLIENTID_FILESAVE_SELECTION;
            break;

        case css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE :
            aGUID = CLIENTID_FILESAVE_TEMPLATE;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE :
            aGUID = CLIENTID_FILEOPEN_LINK_TEMPLATE;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR :
            aGUID = CLIENTID_FILEOPEN_LINK_ANCHOR;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_PLAY :
        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PLAY :
            aGUID = CLIENTID_FILEOPEN_PLAY;
            break;

        case css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW :
            aGUID = CLIENTID_FILEOPEN_LINK;
            break;
    }
    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (iDialog.is())
        iDialog->SetClientGuid ( aGUID );

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if (!iCustom.is())
        return;

    if ((nFeatures & FEATURE_VERSION) == FEATURE_VERSION)
    {
        iCustom->StartVisualGroup (GROUP_VERSION, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_VERSION).replaceFirst("~","").getStr()));
        iCustom->AddComboBox      (css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_VERSION);
        iCustom->EndVisualGroup   ();
        iCustom->MakeProminent    (GROUP_VERSION);
    }

    if ((nFeatures & FEATURE_TEMPLATE) == FEATURE_TEMPLATE)
    {
        iCustom->StartVisualGroup (GROUP_TEMPLATE, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_TEMPLATES).replaceFirst("~","").getStr()));
        iCustom->AddComboBox      (css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_TEMPLATE);
        iCustom->EndVisualGroup   ();
        iCustom->MakeProminent    (GROUP_TEMPLATE);
    }

    if ((nFeatures & FEATURE_IMAGETEMPLATE) == FEATURE_IMAGETEMPLATE)
    {
        iCustom->StartVisualGroup (GROUP_IMAGETEMPLATE, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_IMAGE_TEMPLATE).replaceFirst("~","").getStr()));
        iCustom->AddComboBox      (css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE);
        iCustom->EndVisualGroup   ();
        iCustom->MakeProminent    (GROUP_IMAGETEMPLATE);
    }

    if ((nFeatures & FEATURE_IMAGEANCHOR) == FEATURE_IMAGEANCHOR)
    {
        iCustom->StartVisualGroup (GROUP_IMAGEANCHOR, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_IMAGE_ANCHOR).replaceFirst("~","").getStr()));
        iCustom->AddComboBox      (css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR);
        iCustom->EndVisualGroup   ();
        iCustom->MakeProminent    (GROUP_IMAGEANCHOR);
    }

    iCustom->StartVisualGroup (GROUP_CHECKBOXES, L"");

    sal_uInt16 nControlId(0);
    if ((nFeatures & FEATURE_AUTOEXTENSION) == FEATURE_AUTOEXTENSION)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION;
        iCustom->AddCheckButton (nControlId, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_AUTO_EXTENSION).replaceFirst("~","").getStr()), true);
        setLabelToControl(iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_PASSWORD) == FEATURE_PASSWORD)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PASSWORD;
        iCustom->AddCheckButton (nControlId, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_PASSWORD).replaceFirst("~","").getStr()), false);
        setLabelToControl(iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_GPGPASSWORD) == FEATURE_GPGPASSWORD)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_GPGENCRYPTION;
        iCustom->AddCheckButton (nControlId, L"GpgPassword", false);
        setLabelToControl(iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_READONLY) == FEATURE_READONLY)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_READONLY;
        iCustom->AddCheckButton (nControlId, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_READONLY).replaceFirst("~","").getStr()), false);
        setLabelToControl(iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_FILTEROPTIONS) == FEATURE_FILTEROPTIONS)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS;
        iCustom->AddCheckButton (nControlId, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_FILTER_OPTIONS).replaceFirst("~","").getStr()), false);
        setLabelToControl(iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_LINK) == FEATURE_LINK)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK;
        iCustom->AddCheckButton (nControlId, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_INSERT_AS_LINK).replaceFirst("~","").getStr()), false);
        setLabelToControl(iCustom, nControlId);
    }

    if ((nFeatures & FEATURE_SELECTION) == FEATURE_SELECTION)
    {
        nControlId = css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION;
        iCustom->AddCheckButton (nControlId, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_SELECTION).replaceFirst("~","").getStr()), false);
        setLabelToControl(iCustom, nControlId);
    }

    /* can be ignored ... new COM dialog supports preview native now  !
    if ((nFeatures & FEATURE_PREVIEW) == FEATURE_PREVIEW)
        iCustom->AddCheckButton (css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, L"Preview", false);
    */

    iCustom->EndVisualGroup();

    if ((nFeatures & FEATURE_PLAY) == FEATURE_PLAY)
        iCustom->AddPushButton (css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY, o3tl::toW(FpsResId(STR_SVT_FILEPICKER_PLAY).replaceFirst("~","").getStr()));

}


void VistaFilePickerImpl::impl_sta_SetMultiSelectionMode(Request& rRequest)
{
    const bool bMultiSelection = rRequest.getArgumentOrDefault(PROP_MULTISELECTION_MODE, true);

    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    DWORD nFlags = 0;
    iDialog->GetOptions(&nFlags);

    if (bMultiSelection)
        nFlags |=  FOS_ALLOWMULTISELECT;
    else
        nFlags &= ~FOS_ALLOWMULTISELECT;

    iDialog->SetOptions ( nFlags );
}


void VistaFilePickerImpl::impl_sta_SetTitle(Request& rRequest)
{
    OUString sTitle = rRequest.getArgumentOrDefault(PROP_TITLE, OUString());

    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    iDialog->SetTitle(o3tl::toW(sTitle.getStr()));
}


void VistaFilePickerImpl::impl_sta_SetFileName(Request& rRequest)
{
    OUString sFileName = rRequest.getArgumentOrDefault(PROP_FILENAME, OUString());

    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    iDialog->SetFileName(o3tl::toW(sFileName.getStr()));
}


void VistaFilePickerImpl::impl_sta_SetDirectory(Request& rRequest)
{
    OUString sDirectory = rRequest.getArgumentOrDefault(PROP_DIRECTORY, OUString());

    if( !m_bInExecute)
    {
        // Vista stores last used folders for file dialogs
        // so we don't want the application to change the folder
        // in most cases.
        // Store the requested folder in the meantime and decide later
        // what to do
        m_sDirectory = sDirectory;
    }

    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    sal::systools::COMReference<IShellItem> pFolder;
    if ( !createFolderItem(sDirectory, pFolder) )
        return;

    iDialog->SetFolder(pFolder.get());
}

OUString VistaFilePickerImpl::GetDirectory()
{
    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return OUString();
    sal::systools::COMReference<IShellItem> pFolder;
    HRESULT hResult = iDialog->GetFolder( &pFolder );
    if ( FAILED(hResult) )
        return OUString();
    return lcl_getURLFromShellItem(pFolder.get());
}

void VistaFilePickerImpl::impl_sta_GetDirectory(Request& rRequest)
{
    const OUString sFolder = m_sDirectory.isEmpty() ? GetDirectory() : m_sDirectory;
    if (!sFolder.isEmpty())
        rRequest.setArgument(PROP_DIRECTORY, sFolder);
}

void VistaFilePickerImpl::impl_sta_SetDefaultName(Request& rRequest)
{
    OUString sFilename = rRequest.getArgumentOrDefault(PROP_FILENAME, OUString());
    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if ( ! iCustom.is())
        return;

    // if we have the autoextension check box set, remove (or change ???) the extension of the filename
    // so that the autoextension mechanism can do its job
    BOOL bValue = FALSE;
    HRESULT hResult = iCustom->GetCheckButtonState( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, &bValue);
    if ( FAILED(hResult) )
        return;
    if ( bValue )
    {
        sal_Int32 nSepPos = sFilename.lastIndexOf( '.' );
        if ( -1 != nSepPos )
            sFilename = sFilename.copy(0, nSepPos);
    }

    iDialog->SetFileName (o3tl::toW(sFilename.getStr()));
    m_sFilename = sFilename;
}


void VistaFilePickerImpl::impl_sta_setFiltersOnDialog()
{
    std::vector<OUString> vStrings; // to hold the adjusted filter names, pointers to which will be
                                    // stored in lFilters
    ::std::vector< COMDLG_FILTERSPEC > lFilters       = lcl_buildFilterList(m_lFilters, vStrings);
    OUString                    sCurrentFilter = m_lFilters.getCurrentFilter();
    sal_Int32                          nCurrentFilter = m_lFilters.getFilterPos(sCurrentFilter);
    TFileDialog                        iDialog        = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;
    TFileDialogCustomize               iCustomize     = impl_getCustomizeInterface();
    if (!iCustomize.is())
        return;

    if (lFilters.empty())
        return;

    COMDLG_FILTERSPEC   *pFilt = lFilters.data();
    iDialog->SetFileTypes(lFilters.size(), pFilt/*&lFilters[0]*/);
    iDialog->SetFileTypeIndex(nCurrentFilter + 1);

    BOOL bValue = FALSE;
    HRESULT hResult = iCustomize->GetCheckButtonState( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, &bValue);
    if ( FAILED(hResult) )
        return;

    if ( bValue )
    {
        PCWSTR lpFilterExt = lFilters[0].pszSpec;

        lpFilterExt = wcsrchr( lpFilterExt, '.' );
        if ( lpFilterExt )
            lpFilterExt++;
        iDialog->SetDefaultExtension( lpFilterExt );
    }

}


void VistaFilePickerImpl::impl_sta_getSelectedFiles(Request& rRequest)
{
    if (m_pDialog == nullptr)
        return;

    // ask dialog for results
    // we must react different if dialog is in execute or not .-(
    sal::systools::COMReference<IShellItemArray> iItems = m_pDialog->getResult(m_bInExecute);
    if (!iItems.is())
        return;

    // convert and pack results
    std::vector< OUString > lFiles;
    if (DWORD nCount; SUCCEEDED(iItems->GetCount(&nCount)))
    {
        for (DWORD i = 0; i < nCount; ++i)
        {
            if (sal::systools::COMReference<IShellItem> iItem;
                SUCCEEDED(iItems->GetItemAt(i, &iItem)))
            {
                if (const OUString sURL = lcl_getURLFromShellItem(iItem.get()); !sURL.isEmpty())
                    lFiles.push_back(sURL);
            }
        }
    }

    rRequest.setArgument(PROP_SELECTED_FILES, comphelper::containerToSequence(lFiles));
}


void VistaFilePickerImpl::impl_sta_ShowDialogModal(Request& rRequest)
{
    impl_sta_setFiltersOnDialog();

    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    // it's important to know if we are showing the dialog.
    // Some dialog interface methods can't be called then or some
    // tasks must be done differently .-) (e.g. see impl_sta_getSelectedFiles())
    m_bInExecute = true;

    m_bWasExecuted = true;

    // we set the directory only if we have a save dialog and a filename
    // for the other cases, the file dialog remembers its last location
    // according to its client guid.
    if( m_sDirectory.getLength())
    {
        sal::systools::COMReference<IShellItem> pFolder;
        if ( createFolderItem(m_sDirectory, pFolder) )
        {
            if (m_sFilename.getLength())
            {
                OUString aFileURL(m_sDirectory);
                sal_Int32 nIndex = aFileURL.lastIndexOf('/');
                if (nIndex != aFileURL.getLength()-1)
                    aFileURL += "/";
                aFileURL += m_sFilename;

                TFileDialogCustomize iCustom = impl_getCustomizeInterface();
                if (!iCustom.is())
                    return;

                BOOL bValue = FALSE;
                HRESULT hResult = iCustom->GetCheckButtonState( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, &bValue);
                if ( bValue )
                {
                    UINT nFileType;
                    hResult = iDialog->GetFileTypeIndex(&nFileType);
                    if ( SUCCEEDED(hResult) && nFileType > 0 )
                    {
                        // COM dialog base on 1 ... filter container on 0 .-)
                        ::size_t nRealIndex = nFileType-1;
                        OUString sFilter;
                        if (m_lFilters.getFilterByIndex(nRealIndex, sFilter))
                        {
                            const sal_Int32 idx = sFilter.indexOf('.');
                            if (idx >= 0)
                                aFileURL += sFilter.subView(idx);
                        }
                    }
                }

                // Check existence of file. Set folder only for this special case
                OUString aSystemPath;
                osl_getSystemPathFromFileURL( aFileURL.pData, &aSystemPath.pData );

                WIN32_FIND_DATAW aFindFileData;
                HANDLE  hFind = FindFirstFileW( o3tl::toW(aSystemPath.getStr()), &aFindFileData );
                if (hFind != INVALID_HANDLE_VALUE)
                    iDialog->SetFolder(pFolder.get());
                else
                    hResult = iDialog->AddPlace(pFolder.get(), FDAP_TOP);

                FindClose( hFind );
            }
            else
                iDialog->AddPlace(pFolder.get(), FDAP_TOP);
        }
    }

    HRESULT hResult = E_FAIL;
    try
    {
        // tdf#146007: Make sure we don't hold solar mutex: COM may need to forward
        // the execution to the main thread, and holding solar mutex could deadlock
        SolarMutexReleaser r;
        // show dialog and wait for user decision
        hResult = iDialog->Show(m_hParentWindow ? m_hParentWindow
                                                : choose_parent_window()); // parent window needed
    }
    catch(...)
    {}

    m_bInExecute = false;

    if (m_iEventHandler.is())
    {
        auto* pHandlerImpl = static_cast<VistaFilePickerEventHandler*>(m_iEventHandler.get());
        pHandlerImpl->stopListening();
    }

    if ( FAILED(hResult) )
        return;

    impl_sta_getSelectedFiles(rRequest);
    rRequest.setArgument(PROP_DIALOG_SHOW_RESULT, true);
}


TFileDialog VistaFilePickerImpl::impl_getBaseDialogInterface()
{
    TFileDialog iDialog;

    if (m_pDialog != nullptr)
        iDialog = m_pDialog->getComPtr();

    return iDialog;
}


TFileDialogCustomize VistaFilePickerImpl::impl_getCustomizeInterface()
{
    if (m_pDialog != nullptr)
        return { m_pDialog->getComPtr(), sal::systools::COM_QUERY_THROW };

    return {};
}


static void lcl_removeControlItemsWorkaround(const TFileDialogCustomize& iCustom   ,
                                            ::sal_Int16           nControlId)
{
    (void)iCustom->SetSelectedControlItem(nControlId, 1000); // Don't care if this fails (useless?)
    DWORD i = 0;
    HRESULT hResult = S_OK;
    while ( SUCCEEDED(hResult) )
        hResult = iCustom->RemoveControlItem(nControlId, i++);
}


void VistaFilePickerImpl::impl_sta_SetControlValue(Request& rRequest)
{
    ::sal_Int16   nId     = rRequest.getArgumentOrDefault(PROP_CONTROL_ID    , INVALID_CONTROL_ID    );
    ::sal_Int16   nAction = rRequest.getArgumentOrDefault(PROP_CONTROL_ACTION, INVALID_CONTROL_ACTION);
    css::uno::Any aValue  = rRequest.getValue(PROP_CONTROL_VALUE);

    // don't check for right values here ...
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
                bool bValue   = false;
                aValue >>= bValue;
                iCustom->SetCheckButtonState(nId, bValue);
            }
            break;

        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_VERSION :
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_TEMPLATE :
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE :
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR :
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
                            aValue >>= m_lItems;
                            for (::sal_Int32 i=0; i<m_lItems.getLength(); ++i)
                            {
                                const OUString& sItem = m_lItems[i];
                                hResult = iCustom->AddControlItem(nId, i, o3tl::toW(sItem.getStr()));
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


void VistaFilePickerImpl::impl_sta_GetControlValue(Request& rRequest)
{
    ::sal_Int16 nId     = rRequest.getArgumentOrDefault(PROP_CONTROL_ID    , INVALID_CONTROL_ID    );

    // don't check for right values here ...
    // most parameters are optional !

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if ( ! iCustom.is())
        return;

    css::uno::Any aValue;
    if( m_bWasExecuted )
        switch (nId)
        {
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PASSWORD :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_GPGENCRYPTION :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_READONLY :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS :
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK :
        //case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW : // can be ignored ... preview is supported native now !
        case css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION :
            {
                BOOL    bValue  = FALSE;
                HRESULT hResult = iCustom->GetCheckButtonState(nId, &bValue);
                if ( SUCCEEDED(hResult) )
                    aValue <<= bool(bValue);
            }
            break;
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_VERSION:
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
        case css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR:
            {
                DWORD    bValue = 0;
                HRESULT hResult = iCustom->GetSelectedControlItem(nId, &bValue);
                if ( SUCCEEDED(hResult) )
                {
                    const OUString& sItem = m_lItems[bValue];
                    aValue <<= OUString(sItem.getStr());
                }
            }
            break;
        }

    if (aValue.hasValue())
        rRequest.setArgument(PROP_CONTROL_VALUE, aValue);
}


void VistaFilePickerImpl::impl_sta_SetControlLabel(Request& rRequest)
{
    ::sal_Int16     nId    = rRequest.getArgumentOrDefault(PROP_CONTROL_ID   , INVALID_CONTROL_ID  );
    OUString sLabel = rRequest.getArgumentOrDefault(PROP_CONTROL_LABEL, OUString() );

    // don't check for right values here ...
    // most parameters are optional !

    TFileDialogCustomize iCustom = impl_getCustomizeInterface();
    if ( ! iCustom.is())
        return;
    iCustom->SetControlLabel (nId, o3tl::toW(sLabel.getStr()));
}


void VistaFilePickerImpl::impl_sta_GetControlLabel(Request& /*rRequest*/)
{
}


void VistaFilePickerImpl::impl_sta_EnableControl(Request& rRequest)
{
    ::sal_Int16 nId      = rRequest.getArgumentOrDefault(PROP_CONTROL_ID    , INVALID_CONTROL_ID  );
    bool bEnabled = rRequest.getArgumentOrDefault(PROP_CONTROL_ENABLE, true);

    // don't check for right values here ...
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

void VistaFilePickerImpl::impl_SetDefaultExtension( const OUString& currentFilter )
{
   TFileDialog iDialog = impl_getBaseDialogInterface();
   if (!iDialog.is())
        return;

   if (currentFilter.getLength())
   {
        OUString FilterExt;
        m_lFilters.getFilterByName(currentFilter, FilterExt);

        sal_Int32 posOfPoint = FilterExt.indexOf(L'.');
        const sal_Unicode* pFirstExtStart = FilterExt.getStr() + posOfPoint + 1;

        sal_Int32 posOfSemiColon = FilterExt.indexOf(L';') - 1;
        if (posOfSemiColon < 0)
            posOfSemiColon = FilterExt.getLength() - 1;

        FilterExt = OUString(pFirstExtStart, posOfSemiColon - posOfPoint);
        iDialog->SetDefaultExtension ( o3tl::toW(FilterExt.getStr()) );
   }
}

void VistaFilePickerImpl::onAutoExtensionChanged (bool bChecked)
{
    const OUString sFilter = m_lFilters.getCurrentFilter ();
    OUString sExt    ;
    if (!m_lFilters.getFilterByName(sFilter, sExt))
        return;

    TFileDialog iDialog = impl_getBaseDialogInterface();
    if (!iDialog.is())
        return;

    PCWSTR pExt = nullptr;
    if ( bChecked )
    {
        pExt = o3tl::toW(sExt.getStr());
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

void VistaFilePickerImpl::onDirectoryChanged()
{
    m_sDirectory = GetDirectory();
}

} // namespace vista
} // namespace win32
} // namespace fpicker

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
