/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VistaFilePickerImpl.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "VistaFilePickerImpl.hxx"

#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>

#include <comphelper/sequenceasvector.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>

//-----------------------------------------------------------------------------
// namespace
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
::rtl::OUString lcl_getURLFromShellItem (IShellItem* pItem)
{
    LPOLESTR pStr = NULL;

    SIGDN   eConversion = SIGDN_URL;
    HRESULT hr          = pItem->GetDisplayName ( eConversion, &pStr );

    if ( FAILED(hr) )
        return ::rtl::OUString();

    ::rtl::OUString sURL = ::rtl::OUString(pStr);
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

        aSpec.pszName = aFilter.first ;
        aSpec.pszSpec = aFilter.second;

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
    , m_iEventHandler(new VistaFilePickerEventHandler())
    , m_bInExecute   (sal_False)
{
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

            case E_SET_DIRECTORY :
                    impl_sta_SetDirectory(rRequest);
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
    m_iDialogOpen.query(&iDialog);

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
    nFlags |=  FOS_ALLOWMULTISELECT;

    iDialog->SetOptions ( nFlags );

    ::sal_Int32 nFeatures = rRequest->getArgumentOrDefault(PROP_FEATURES, (::sal_Int32)0);
    impl_sta_enableFeatures(nFeatures);

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
    m_iDialogSave.query(&iDialog);

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
    impl_sta_enableFeatures(nFeatures);

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
void VistaFilePickerImpl::impl_sta_enableFeatures(::sal_Int32 nFeatures)
{
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

    if ((nFeatures & FEATURE_AUTOEXTENSION) == FEATURE_AUTOEXTENSION)
        iCustom->AddCheckButton (css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, L"Auto Extension", true);

    if ((nFeatures & FEATURE_PASSWORD) == FEATURE_PASSWORD)
        iCustom->AddCheckButton (css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, L"Password", true);

    if ((nFeatures & FEATURE_READONLY) == FEATURE_READONLY)
        iCustom->AddCheckButton (css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_READONLY, L"Readonly", false);

    if ((nFeatures & FEATURE_FILTEROPTIONS) == FEATURE_FILTEROPTIONS)
        iCustom->AddCheckButton (css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS, L"Filter Options", false);

    if ((nFeatures & FEATURE_LINK) == FEATURE_LINK)
        iCustom->AddCheckButton (css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, L"Link", false);

    if ((nFeatures & FEATURE_SELECTION) == FEATURE_SELECTION)
        iCustom->AddCheckButton (css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, L"Selection", false);

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

    iDialog->SetTitle(sTitle);
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_SetDirectory(const RequestRef& rRequest)
{
    ::rtl::OUString sDirectory = rRequest->getArgumentOrDefault(PROP_DIRECTORY, ::rtl::OUString());

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    TFileDialog iDialog = impl_getBaseDialogInterface();
    aLock.clear();
    // <- SYNCHRONIZED

    ComPtr< IShellItem > pFolder;
    HRESULT hResult = SHCreateItemFromParsingName ( sDirectory, NULL, IID_PPV_ARGS(&pFolder) );
    if ( FAILED(hResult) )
        return;

    iDialog->SetFolder(pFolder);
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

    aLock.clear();
    // <- SYNCHRONIZED

    iDialog->SetFileTypes(lFilters.size(), &lFilters[0]);
    iDialog->SetFileTypeIndex(nCurrentFilter);
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
    HRESULT                   hResult;

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

    aLock.clear();
    // <- SYNCHRONIZED

    HRESULT hResult;
    try
    {
        // show dialog and wait for user decision
        if (iOpen.is())
            hResult = iOpen->Show(0);
        else
        if (iSave.is())
            hResult = iSave->Show(0);
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
    rRequest->setArgument(PROP_DIALOG_SHOW_RESULT, (::sal_Bool)sal_True);
}

//-------------------------------------------------------------------------------
TFileDialog VistaFilePickerImpl::impl_getBaseDialogInterface()
{
    TFileDialog iDialog;

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    if (m_iDialogOpen.is())
        m_iDialogOpen.query(&iDialog);
    else
    if (m_iDialogSave.is())
        m_iDialogSave.query(&iDialog);

    return iDialog;
}

//-------------------------------------------------------------------------------
TFileDialogCustomize VistaFilePickerImpl::impl_getCustomizeInterface()
{
    TFileDialogCustomize iCustom;

    // SYNCHRONIZED->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

    if (m_iDialogOpen.is())
        m_iDialogOpen.query(&iCustom);
    else
    if (m_iDialogSave.is())
        m_iDialogSave.query(&iCustom);

    return iCustom;
}

//-------------------------------------------------------------------------------
void lcl_removeControlItemsWorkaround(const TFileDialogCustomize& iCustom   ,
                                            ::sal_Int16           nControlId)
{
    ::sal_Int32 i       = 0;
      HRESULT   hResult = S_OK;

    hResult = iCustom->SetSelectedControlItem(nControlId, 1000);
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
                                hResult = iCustom->AddControlItem(nId, i, sItem);
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
                BOOL    bValue  = sal_False;
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
}

//-------------------------------------------------------------------------------
void VistaFilePickerImpl::impl_sta_GetControlLabel(const RequestRef& rRequest)
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

} // namespace vista
} // namespace win32
} // namespace fpicker
