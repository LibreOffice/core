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

#include "shared.hxx"
#include "WinFileOpenImpl.hxx"
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include "../misc/WinImplHelper.hxx"

#include "FilePicker.hxx"
#include "controlaccess.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>
#include <osl/thread.hxx>
#include "filepickerstate.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star;

using com::sun::star::ui::dialogs::FilePickerEvent;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::ui::dialogs::XFilePicker2;

using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::ListboxControlActions;

//-------------------------------------------------------------------------
// to distinguish what to do in the enum child window callback function
//-------------------------------------------------------------------------

enum ECW_ACTION_T
{
    INIT_CUSTOM_CONTROLS,
    CACHE_CONTROL_VALUES
};

struct EnumParam
{
    ECW_ACTION_T        m_action;
    CWinFileOpenImpl*   m_instance;

    EnumParam( ECW_ACTION_T action, CWinFileOpenImpl* instance ):
        m_action( action ),
        m_instance( instance )
    {}
};

//-------------------------------------------------------------------------
// ctor
//-------------------------------------------------------------------------

CWinFileOpenImpl::CWinFileOpenImpl(
    CFilePicker* aFilePicker,
    sal_Bool bFileOpenDialog,
    sal_uInt32 dwFlags,
    sal_uInt32 dwTemplateId,
    HINSTANCE hInstance) :
    CFileOpenDialog(bFileOpenDialog, dwFlags, dwTemplateId, hInstance),
    m_filterContainer(new CFilterContainer()),
    m_Preview(new CPreviewAdapter(hInstance)),
    m_CustomControlFactory(new CCustomControlFactory()),
    m_CustomControls(m_CustomControlFactory->CreateCustomControlContainer()),
    m_FilePicker(aFilePicker),
    m_bInitialSelChanged(sal_True),
    m_HelpPopupWindow(hInstance, m_hwndFileOpenDlg),
    m_ExecuteFilePickerState(new CExecuteFilePickerState()),
    m_NonExecuteFilePickerState(new CNonExecuteFilePickerState())
{
    m_FilePickerState = m_NonExecuteFilePickerState;
}

//------------------------------------------------------------------------
// dtor
//------------------------------------------------------------------------

CWinFileOpenImpl::~CWinFileOpenImpl()
{
    delete m_ExecuteFilePickerState;
    delete m_NonExecuteFilePickerState;
}

//------------------------------------------------------------------------
// we expect the directory in URL format
//------------------------------------------------------------------------

void CWinFileOpenImpl::setDisplayDirectory(const rtl::OUString& aDirectory)
    throw( IllegalArgumentException, uno::RuntimeException )
{
    rtl::OUString aSysDirectory;
    if( aDirectory.getLength() > 0)
    {
        if ( ::osl::FileBase::E_None !=
             ::osl::FileBase::getSystemPathFromFileURL(aDirectory,aSysDirectory))
            throw IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid directory")),
                static_cast<XFilePicker2*>(m_FilePicker), 1);

        // we ensure that there is a trailing '/' at the end of
        // he given file url, because the windows functions only
        // works correctly when providing "c:\" or an environment
        // variable like "=c:=c:\.." etc. is set, else the
        // FolderPicker would stand in the root of the shell
        // hierarchie which is the desktop folder
        if ( aSysDirectory.lastIndexOf(BACKSLASH) != (aSysDirectory.getLength() - 1))
            aSysDirectory += BACKSLASH;
    }

    // call base class method
    CFileOpenDialog::setDisplayDirectory(aSysDirectory);
}

//------------------------------------------------------------------------
// we return the directory in URL format
//------------------------------------------------------------------------

rtl::OUString CWinFileOpenImpl::getDisplayDirectory() throw(uno::RuntimeException)
{
    return m_FilePickerState->getDisplayDirectory(this);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::setDefaultName(const rtl::OUString& aName)
    throw( IllegalArgumentException, uno::RuntimeException )
{
    // we don't set the default name directly
    // because this influences how the file open
    // dialog sets the initial path when it is about
    // to open (see MSDN: OPENFILENAME)
    // so we save the default name which should
    // appear in the file-name-box and set
    // this name when processing onInitDone
    m_defaultName = aName;
}

//-----------------------------------------------------------------------------------------
// return format: URL
// if multiselection is allowed there are two different cases
// 1. one file selected: the sequence contains one entry path\filename.ext
// 2. multiple files selected: the sequence contains multiple entries
//    the first entry is the path url, all other entries are file names
//-----------------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL CWinFileOpenImpl::getFiles()
    throw(uno::RuntimeException)
{
    return m_FilePickerState->getFiles(this);
}

//-----------------------------------------------------------------------------------------
// shows the FileOpen/FileSave dialog
//-----------------------------------------------------------------------------------------

sal_Int16 SAL_CALL CWinFileOpenImpl::execute(  ) throw(uno::RuntimeException)
{
    sal_Int16 rc = CFileOpenDialog::doModal();

    if (1 == rc)
        rc = ::com::sun::star::ui::dialogs::ExecutableDialogResults::OK;
    else if (0 == rc)
        rc = ::com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;
    else
        throw uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Error executing dialog")),
            static_cast<XFilePicker2*>(m_FilePicker));

    return rc;
}

//-----------------------------------------------------------------------------------------
// appends a new filter
// returns false if the title (aTitle) was already added or the title or the filter are
// empty
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::appendFilter(const rtl::OUString& aTitle, const rtl::OUString& aFilter)
    throw(IllegalArgumentException, uno::RuntimeException)
{
    sal_Bool bRet = m_filterContainer->addFilter(aTitle, aFilter);

    if (!bRet)
        throw IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("filter already exists")),
            static_cast<XFilePicker2*>(m_FilePicker), 1);

    // #95345# see MSDN OPENFILENAME
    // If nFilterIndex is zero and lpstrCustomFilter is NULL,
    // the system uses the first filter in the lpstrFilter buffer.
    // to reflect this we must set the filter index so that calls
    // to getSelectedFilterIndex without explicitly calling
    // setFilterIndex before does not return 0 which leads to a
    // false state
    if (0 == getSelectedFilterIndex())
        CFileOpenDialog::setFilterIndex(1);
}

//-----------------------------------------------------------------------------------------
// sets a current filter
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::setCurrentFilter(const rtl::OUString& aTitle)
    throw( IllegalArgumentException, uno::RuntimeException)
{
    sal_Int32 filterPos = m_filterContainer->getFilterPos(aTitle);

    if (filterPos < 0)
        throw IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("filter doesn't exist")),
            static_cast<XFilePicker2*>(m_FilePicker), 1);

    // filter index of the base class starts with 1
    CFileOpenDialog::setFilterIndex(filterPos + 1);
}

//-----------------------------------------------------------------------------------------
// returns the currently selected filter
//-----------------------------------------------------------------------------------------

rtl::OUString SAL_CALL CWinFileOpenImpl::getCurrentFilter() throw(uno::RuntimeException)
{
    sal_uInt32 nIndex = getSelectedFilterIndex();

    rtl::OUString currentFilter;
    if (nIndex > 0)
    {
        // filter index of the base class starts with 1
        if (!m_filterContainer->getFilter(nIndex - 1, currentFilter)) {
            OSL_ASSERT(false);
        }
    }

    return currentFilter;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

inline void SAL_CALL CWinFileOpenImpl::appendFilterGroupSeparator()
{
    m_filterContainer->addFilter(FILTER_SEPARATOR, ALL_FILES_WILDCARD, ALLOW_DUPLICATES);
}

//-----------------------------------------------------------------------------------------
// XFilterGroupManager
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::appendFilterGroup(const rtl::OUString& sGroupTitle, const uno::Sequence<beans::StringPair>& aFilters)
    throw (IllegalArgumentException, uno::RuntimeException)
{
    (void) sGroupTitle; // avoid warning
    OSL_ENSURE(0 == sGroupTitle.getLength(), "appendFilterGroup: Parameter 'GroupTitle' currently ignored");

    sal_Int32 nFilters = aFilters.getLength();

    OSL_PRECOND(nFilters > 0, "Empty filter list");

    if (nFilters > 0)
    {
        // append a separator before the next group if
        // there is already a group of filters
        if (m_filterContainer->numFilter() > 0)
            appendFilterGroupSeparator();

        for (int i = 0; i < nFilters; i++)
            appendFilter(aFilters[i].First, aFilters[i].Second);
    }
}

//=================================================================================================================
// XExtendedFilePicker
//=================================================================================================================

// #i90917: Due to a different feature set for the system-dependent file pickers
// it's possible that generic code (e.g. sfx2) provides control ids
// (see ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR) which are NOT
// available on all platforms. This filter function should filter out control ids
// which are only available on KDE/GTK file pickers.
static bool filterControlCommand( sal_Int16 nControlId )
{
    if ( nControlId == LISTBOX_FILTER_SELECTOR )
        return true;
    return false;
}

void SAL_CALL CWinFileOpenImpl::setValue(sal_Int16 aControlId, sal_Int16 aControlAction, const uno::Any& aValue)
    throw(uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( aControlId ))
        m_FilePickerState->setValue(aControlId, aControlAction, aValue);
}

//-----------------------------------------------------------------------------------------
// returns the value of an custom template element
// we assume that there are only checkboxes or comboboxes
//-----------------------------------------------------------------------------------------

uno::Any SAL_CALL CWinFileOpenImpl::getValue(sal_Int16 aControlId, sal_Int16 aControlAction)
    throw(uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( aControlId ))
        return m_FilePickerState->getValue(aControlId, aControlAction);
    else
        return uno::Any();
}

//-----------------------------------------------------------------------------------------
// enables a custom template element
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::enableControl(sal_Int16 ControlID, sal_Bool bEnable)
    throw(uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( ControlID ))
        m_FilePickerState->enableControl(ControlID, bEnable);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::setLabel( sal_Int16 aControlId, const rtl::OUString& aLabel )
    throw (uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( aControlId ))
        m_FilePickerState->setLabel(aControlId, aLabel);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

rtl::OUString SAL_CALL CWinFileOpenImpl::getLabel( sal_Int16 aControlId )
        throw (uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( aControlId ))
        return m_FilePickerState->getLabel(aControlId);
    else
        return rtl::OUString();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

uno::Sequence<sal_Int16> SAL_CALL CWinFileOpenImpl::getSupportedImageFormats()
    throw (uno::RuntimeException)
{
    return m_Preview->getSupportedImageFormats();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CWinFileOpenImpl::getTargetColorDepth()
    throw (uno::RuntimeException)
{
    return m_Preview->getTargetColorDepth();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CWinFileOpenImpl::getAvailableWidth()
        throw (uno::RuntimeException)
{
    return m_Preview->getAvailableWidth();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CWinFileOpenImpl::getAvailableHeight()
    throw (uno::RuntimeException)
{
    return m_Preview->getAvailableHeight();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::setImage(sal_Int16 aImageFormat, const uno::Any& aImage)
    throw (IllegalArgumentException, uno::RuntimeException)
{
    m_Preview->setImage(aImageFormat,aImage);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CWinFileOpenImpl::setShowState(sal_Bool bShowState)
        throw (uno::RuntimeException)
{
    return m_Preview->setShowState(bShowState);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CWinFileOpenImpl::getShowState()
    throw (uno::RuntimeException)
{
    return m_Preview->getShowState();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::cancel()
{
    if (IsWindow(m_hwndFileOpenDlg))
    {
        // simulate a mouse click to the
        // cancel button
        PostMessage(
            m_hwndFileOpenDlg,
            WM_COMMAND,
            MAKEWPARAM(IDCANCEL,BN_CLICKED),
            (LPARAM)GetDlgItem(m_hwndFileOpenDlg, IDCANCEL));
    }
}

//-----------------------------------------------------------------------------------------
// returns the id of a custom template element
//-----------------------------------------------------------------------------------------

sal_Int16 SAL_CALL CWinFileOpenImpl::getFocused()
{
    int nID = GetDlgCtrlID(GetFocus());

    // we don't forward id's of standard file open
    // dialog elements (ctlFirst is defined in dlgs.h
    // in MS Platform SDK)
    if (nID >= ctlFirst)
        nID = 0;

    return sal::static_int_cast< sal_Int16 >(nID);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

inline sal_Bool SAL_CALL CWinFileOpenImpl::IsCustomControlHelpRequested(LPHELPINFO lphi) const
{
    return ((lphi->iCtrlId != IDOK) && (lphi->iCtrlId != IDCANCEL) && (lphi->iCtrlId < ctlFirst));
}

//-----------------------------------------------------------------------------------------
// our own DlgProc because we do subclass the dialog
// we catch the WM_NCDESTROY message in order to erase an entry in our static map
// if one instance dies
//-----------------------------------------------------------------------------------------

LRESULT CALLBACK CWinFileOpenImpl::SubClassFunc(
    HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    unsigned int lResult = 0;

    CWinFileOpenImpl* pImpl = dynamic_cast<CWinFileOpenImpl*>(getCurrentInstance(hWnd));

    switch(wMessage)
    {
    case WM_HELP:
    {
        LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

        if (pImpl->IsCustomControlHelpRequested(lphi))
            pImpl->onCustomControlHelpRequest(lphi);
        else
            lResult = CallWindowProc(
                reinterpret_cast<WNDPROC>(pImpl->m_pfnOldDlgProc),
                hWnd,wMessage,wParam,lParam);
    }
    break;

    case WM_SIZE:
        lResult = CallWindowProc(
            reinterpret_cast<WNDPROC>(pImpl->m_pfnOldDlgProc),
            hWnd,wMessage,wParam,lParam);

        pImpl->onWMSize();
        break;

    case WM_WINDOWPOSCHANGED:
        lResult = CallWindowProc(
            reinterpret_cast<WNDPROC>(pImpl->m_pfnOldDlgProc),
            hWnd,wMessage,wParam,lParam);

        pImpl->onWMWindowPosChanged();
        break;

    case WM_SHOWWINDOW:
        lResult = CallWindowProc(
            reinterpret_cast<WNDPROC>(pImpl->m_pfnOldDlgProc),
            hWnd,wMessage,wParam,lParam);

        pImpl->onWMShow((sal_Bool)wParam);
        break;

    case WM_NCDESTROY:
        // restore the old window proc
        SetWindowLongPtr(hWnd, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(pImpl->m_pfnOldDlgProc));

        lResult = CallWindowProc(
            reinterpret_cast<WNDPROC>(pImpl->m_pfnOldDlgProc),
            hWnd,wMessage,wParam,lParam);
        break;

    default:
        lResult = CallWindowProc(
            reinterpret_cast<WNDPROC>(pImpl->m_pfnOldDlgProc),
            hWnd,wMessage,wParam,lParam);
    break;

    } // switch

    return lResult;
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::InitControlLabel(HWND hWnd)
{
    //-----------------------------------------
    // set the labels for all extendet controls
    //-----------------------------------------

    sal_Int16 aCtrlId = sal::static_int_cast< sal_Int16 >(GetDlgCtrlID(hWnd));
    rtl::OUString aLabel = m_ResProvider.getResString(aCtrlId);
    if (aLabel.getLength())
        setLabel(aCtrlId, aLabel);
}

//-----------------------------------------------------------------
// There may be problems with the layout of our custom controls,
// so that they are not aligned with the standard controls of the
// FileOpen dialog.
// We use a simple algorithm to move the custom controls to their
// proper position and resize them.
// Our approach is to align all static text controls with the
// static text control "File name" of the FileOpen dialog,
// all checkboxes and all list/comboboxes will be left aligned with
// the standard combobox edt1 (defined in MS platform sdk dlgs.h)
// and all push buttons will be left aligned with the standard
// "OK" button
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::InitCustomControlContainer(HWND hCustomControl)
{
    m_CustomControls->AddControl(
        m_CustomControlFactory->CreateCustomControl(hCustomControl,m_hwndFileOpenDlg));
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::CacheControlState(HWND hWnd)
{
    OSL_ASSERT(m_FilePickerState && m_NonExecuteFilePickerState);
    m_ExecuteFilePickerState->cacheControlState(hWnd, m_NonExecuteFilePickerState);
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

BOOL CALLBACK CWinFileOpenImpl::EnumChildWndProc(HWND hWnd, LPARAM lParam)
{
    EnumParam* enumParam    = (EnumParam*)lParam;
    CWinFileOpenImpl* pImpl = enumParam->m_instance;

    OSL_ASSERT(pImpl);

    sal_Bool bRet = sal_True;

    switch(enumParam->m_action)
    {
    case INIT_CUSTOM_CONTROLS:
        pImpl->InitControlLabel(hWnd);
        pImpl->InitCustomControlContainer(hWnd);
        break;

    case CACHE_CONTROL_VALUES:
        pImpl->CacheControlState(hWnd);
    break;

    default:
        // should not end here
        OSL_ASSERT(sal_False);
    }

    return bRet;
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

sal_uInt32 SAL_CALL CWinFileOpenImpl::onFileOk()
{
    m_NonExecuteFilePickerState->reset();

    EnumParam enumParam(CACHE_CONTROL_VALUES,this);

    EnumChildWindows(
        m_hwndFileOpenDlgChild,
        CWinFileOpenImpl::EnumChildWndProc,
        (LPARAM)&enumParam);

    return 0;
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onSelChanged(HWND)
{
    // the windows file open dialog sends an initial
    // SelChanged message after the InitDone message
    // when the dialog is about to be opened
    // if the lpstrFile buffer of the OPENFILENAME is
    // empty (zero length string) the windows file open
    // dialog sends a WM_SETTEXT message with an empty
    // string to the file name edit line
    // this would overwritte our text when we would set
    // the default name in onInitDone, so we have to
    // remeber that this is the first SelChanged message
    // and set the default name here to overwrite the
    // windows setting
    InitialSetDefaultName();

    FilePickerEvent evt;
    m_FilePicker->fileSelectionChanged(evt);
}

// #i40865# The size of the standard labels 'File name'
// and 'File type' is to short in some cases when the
// label will be changed (e.g. in the Brazil version).
// We just make sure that the labels are using the maximum
// available space.
void CWinFileOpenImpl::EnlargeStdControlLabels() const
{
    HWND hFilterBoxLabel = GetDlgItem(m_hwndFileOpenDlg, stc2);
    HWND hFileNameBoxLabel = GetDlgItem(m_hwndFileOpenDlg, stc3);
    HWND hFileNameBox = GetDlgItem(m_hwndFileOpenDlg, cmb13);
    if (!hFileNameBox)
        hFileNameBox = GetDlgItem(m_hwndFileOpenDlg, edt1); // under Win98 it is edt1 instead of cmb13

    HWND hFilterBox = GetDlgItem(m_hwndFileOpenDlg, cmb1);
    HWND hOkButton = GetDlgItem(m_hwndFileOpenDlg, IDOK);

    // Move filter and file name box nearer to OK and Cancel button
    RECT rcOkButton;
    GetWindowRect(hOkButton, &rcOkButton);

    const int MAX_GAP = 10;
    const int OFFSET = 0;

    RECT rcFileNameBox;
    GetWindowRect(hFileNameBox, &rcFileNameBox);
    int w = rcFileNameBox.right - rcFileNameBox.left;
    int h = rcFileNameBox.bottom - rcFileNameBox.top;

    int gap = rcOkButton.left - rcFileNameBox.right;
    gap = (gap > MAX_GAP) ? gap - MAX_GAP : gap;

    ScreenToClient(m_hwndFileOpenDlg, (LPPOINT)&rcFileNameBox);
    MoveWindow(hFileNameBox, rcFileNameBox.left + gap + OFFSET, rcFileNameBox.top, w - OFFSET, h, true);

    RECT rcFilterBox;
    GetWindowRect(hFilterBox, &rcFilterBox);
    w = rcFilterBox.right - rcFilterBox.left;
    h = rcFilterBox.bottom - rcFilterBox.top;
    ScreenToClient(m_hwndFileOpenDlg, (LPPOINT)&rcFilterBox);
    MoveWindow(hFilterBox, rcFilterBox.left + gap + OFFSET, rcFilterBox.top, w - OFFSET, h, true);

    // get the new window rect
    GetWindowRect(hFileNameBox, &rcFileNameBox);

    RECT rcFilterBoxLabel;
    GetWindowRect(hFilterBoxLabel, &rcFilterBoxLabel);
    int offset = rcFileNameBox.left - rcFilterBoxLabel.right - 1;

    w = rcFilterBoxLabel.right - rcFilterBoxLabel.left + offset;
    h = rcFilterBoxLabel.bottom - rcFilterBoxLabel.top;
    ScreenToClient(m_hwndFileOpenDlg, (LPPOINT)&rcFilterBoxLabel);
    MoveWindow(hFilterBoxLabel, rcFilterBoxLabel.left, rcFilterBoxLabel.top, w, h, true);

    RECT rcFileNameBoxLabel;
    GetWindowRect(hFileNameBoxLabel, &rcFileNameBoxLabel);
    w = rcFileNameBoxLabel.right - rcFileNameBoxLabel.left + offset;
    h = rcFileNameBoxLabel.bottom - rcFileNameBoxLabel.top;
    ScreenToClient(m_hwndFileOpenDlg, (LPPOINT)&rcFileNameBoxLabel);
    MoveWindow(hFileNameBoxLabel, rcFileNameBoxLabel.left, rcFileNameBoxLabel.top, w, h, true);
}

void SAL_CALL CWinFileOpenImpl::onInitDone()
{
    m_Preview->setParent(m_hwndFileOpenDlg);

    // but now we have a valid parent handle
    m_HelpPopupWindow.setParent(m_hwndFileOpenDlg);

    EnlargeStdControlLabels();

    // #99826
    // Set the online filepicker state before initializing
    // the control labels from the resource else we are
    // overriding the offline settings
    m_ExecuteFilePickerState->setHwnd(m_hwndFileOpenDlgChild);

    m_FilePickerState = m_ExecuteFilePickerState;

    // initialize controls from cache

    EnumParam enumParam(INIT_CUSTOM_CONTROLS,this);

    EnumChildWindows(
        m_hwndFileOpenDlgChild,
        CWinFileOpenImpl::EnumChildWndProc,
        (LPARAM)&enumParam);

    m_ExecuteFilePickerState->initFilePickerControls(
        m_NonExecuteFilePickerState->getControlCommand());

    SetDefaultExtension();

    m_CustomControls->Align();

    m_CustomControls->SetFont(
        reinterpret_cast<HFONT>(SendMessage(m_hwndFileOpenDlg, WM_GETFONT, 0, 0)));

    // resume event notification that was
    // defered in onInitDialog
    m_FilePicker->resumeEventNotification();

    //#105996 let vcl know that now a system window is active
    PostMessage(
        HWND_BROADCAST,
        RegisterWindowMessage(TEXT("SYSTEM_WINDOW_ACTIVATED")),
        0,
        0);

    // call the parent function to center the
    // dialog to it's parent
    CFileOpenDialog::onInitDone();
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onFolderChanged()
{
    FilePickerEvent evt;
    m_FilePicker->directoryChanged(evt);
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onTypeChanged(sal_uInt32)
{
    SetDefaultExtension();

    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    m_FilePicker->controlStateChanged(evt);
}

//-----------------------------------------------------------------------------------------
// onMessageCommand handler
//-----------------------------------------------------------------------------------------

sal_uInt32 SAL_CALL CWinFileOpenImpl::onCtrlCommand(
    HWND, sal_uInt16 ctrlId, sal_uInt16)
{
    SetDefaultExtension();

    if (ctrlId < ctlFirst)
    {
        FilePickerEvent evt;
        evt.ElementId = ctrlId;
        m_FilePicker->controlStateChanged(evt);
    }

    return 0;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void CWinFileOpenImpl::onWMSize()
{
    m_Preview->notifyParentSizeChanged();
    m_CustomControls->Align();
    m_FilePicker->dialogSizeChanged();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void CWinFileOpenImpl::onWMShow(sal_Bool bShow)
{
    m_Preview->notifyParentShow(bShow);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void CWinFileOpenImpl::onWMWindowPosChanged()
{
    m_Preview->notifyParentWindowPosChanged();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void CWinFileOpenImpl::onCustomControlHelpRequest(LPHELPINFO lphi)
{
    FilePickerEvent evt;
    evt.ElementId = sal::static_int_cast< sal_Int16 >(lphi->iCtrlId);

    rtl::OUString aPopupHelpText = m_FilePicker->helpRequested(evt);

    if (aPopupHelpText.getLength())
    {
        m_HelpPopupWindow.setText(aPopupHelpText);

        DWORD dwMsgPos = GetMessagePos();
        m_HelpPopupWindow.show(LOWORD(dwMsgPos),HIWORD(dwMsgPos));
    }
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onInitDialog(HWND hwndDlg)
{
    // subclass the dialog window
    m_pfnOldDlgProc =
        reinterpret_cast<WNDPROC>(
            SetWindowLongPtr( hwndDlg, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(SubClassFunc)));
}

//-----------------------------------------------------------------------------------------
// processing before showing the dialog
//-----------------------------------------------------------------------------------------

bool SAL_CALL CWinFileOpenImpl::preModal()
{
    CFileOpenDialog::setFilter(
        makeWinFilterBuffer(*m_filterContainer.get()));

    return true;
}

//-----------------------------------------------------------------------------------------
// processing after showing the dialog
//-----------------------------------------------------------------------------------------

void CWinFileOpenImpl::postModal(sal_Int16 nDialogResult)
{
    CFileOpenDialog::postModal(nDialogResult);

    // empty the container in order to get rid off
    // invalid controls in case someone calls execute
    // twice in sequence with the same instance
    m_CustomControls->RemoveAllControls();

    m_FilePickerState = m_NonExecuteFilePickerState;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::SetDefaultExtension()
{
    HWND hwndChkSaveWithExt = GetDlgItem(m_hwndFileOpenDlgChild, 100);

    if (hwndChkSaveWithExt)
    {
        uno::Any aAny = CheckboxGetState(hwndChkSaveWithExt);
        sal_Bool bChecked = *reinterpret_cast<const sal_Bool*>(aAny.getValue());

        if (bChecked)
        {
            sal_uInt32 nIndex = getSelectedFilterIndex();

            rtl::OUString currentFilter;
            if (nIndex > 0)
            {
                // filter index of the base class starts with 1
                m_filterContainer->getFilter(nIndex - 1, currentFilter);

                if (currentFilter.getLength())
                {
                    rtl::OUString FilterExt;
                    m_filterContainer->getFilter(currentFilter, FilterExt);

                    sal_Int32 posOfPoint = FilterExt.indexOf(L'.');
                    const sal_Unicode* pFirstExtStart = FilterExt.getStr() + posOfPoint + 1;

                    sal_Int32 posOfSemiColon = FilterExt.indexOf(L';') - 1;
                    if (posOfSemiColon < 0)
                        posOfSemiColon = FilterExt.getLength() - 1;

                    FilterExt = rtl::OUString(pFirstExtStart, posOfSemiColon - posOfPoint);

                    SendMessage(m_hwndFileOpenDlg, CDM_SETDEFEXT, 0, reinterpret_cast<LPARAM>(FilterExt.getStr()));
                 }
            }
        }
        else
        {
            SendMessage(m_hwndFileOpenDlg, CDM_SETDEFEXT, 0, reinterpret_cast<LPARAM>(TEXT("")));
        }
    }

    // !!! HACK !!!
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::InitialSetDefaultName()
{
    // manually setting the file name that appears
    // initially in the file-name-box of the file
    // open dialog (reason: see above setDefaultName)
    if (m_bInitialSelChanged && m_defaultName.getLength())
    {
        sal_Int32 edt1Id = edt1;

        // under W2k the there is a combobox instead
        // of an edit field for the file name edit field
        // the control id of this box is cmb13 and not
        // edt1 as before so we must use this id
        edt1Id = cmb13;

        HWND hwndEdt1 = GetDlgItem(m_hwndFileOpenDlg, edt1Id);
        SetWindowText(hwndEdt1, reinterpret_cast<LPCTSTR>(m_defaultName.getStr()));
    }

    m_bInitialSelChanged = sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
