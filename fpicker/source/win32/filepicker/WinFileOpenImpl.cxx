/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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





using namespace com::sun::star;

using com::sun::star::ui::dialogs::FilePickerEvent;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::ui::dialogs::XFilePicker2;

using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::ListboxControlActions;





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





CWinFileOpenImpl::~CWinFileOpenImpl()
{
    delete m_ExecuteFilePickerState;
    delete m_NonExecuteFilePickerState;
}





void CWinFileOpenImpl::setDisplayDirectory(const OUString& aDirectory)
    throw( IllegalArgumentException, uno::RuntimeException )
{
    OUString aSysDirectory;
    if( aDirectory.getLength() > 0)
    {
        if ( ::osl::FileBase::E_None !=
             ::osl::FileBase::getSystemPathFromFileURL(aDirectory,aSysDirectory))
            throw IllegalArgumentException(
                OUString("Invalid directory"),
                static_cast<XFilePicker2*>(m_FilePicker), 1);

        
        
        
        
        
        
        if ( aSysDirectory.lastIndexOf(BACKSLASH) != (aSysDirectory.getLength() - 1))
            aSysDirectory += BACKSLASH;
    }

    
    CFileOpenDialog::setDisplayDirectory(aSysDirectory);
}





OUString CWinFileOpenImpl::getDisplayDirectory() throw(uno::RuntimeException)
{
    return m_FilePickerState->getDisplayDirectory(this);
}


//


void SAL_CALL CWinFileOpenImpl::setDefaultName(const OUString& aName)
    throw( IllegalArgumentException, uno::RuntimeException )
{
    
    
    
    
    
    
    
    m_defaultName = aName;
}









uno::Sequence<OUString> SAL_CALL CWinFileOpenImpl::getFiles()
    throw(uno::RuntimeException)
{
    return m_FilePickerState->getFiles(this);
}





sal_Int16 SAL_CALL CWinFileOpenImpl::execute(  ) throw(uno::RuntimeException)
{
    sal_Int16 rc = CFileOpenDialog::doModal();

    if (1 == rc)
        rc = ::com::sun::star::ui::dialogs::ExecutableDialogResults::OK;
    else if (0 == rc)
        rc = ::com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;
    else
        throw uno::RuntimeException(
            OUString("Error executing dialog"),
            static_cast<XFilePicker2*>(m_FilePicker));

    return rc;
}







void SAL_CALL CWinFileOpenImpl::appendFilter(const OUString& aTitle, const OUString& aFilter)
    throw(IllegalArgumentException, uno::RuntimeException)
{
    sal_Bool bRet = m_filterContainer->addFilter(aTitle, aFilter);

    if (!bRet)
        throw IllegalArgumentException(
            OUString("filter already exists"),
            static_cast<XFilePicker2*>(m_FilePicker), 1);

    
    
    
    
    
    
    
    if (0 == getSelectedFilterIndex())
        CFileOpenDialog::setFilterIndex(1);
}





void SAL_CALL CWinFileOpenImpl::setCurrentFilter(const OUString& aTitle)
    throw( IllegalArgumentException, uno::RuntimeException)
{
    sal_Int32 filterPos = m_filterContainer->getFilterPos(aTitle);

    if (filterPos < 0)
        throw IllegalArgumentException(
            OUString("filter doesn't exist"),
            static_cast<XFilePicker2*>(m_FilePicker), 1);

    
    CFileOpenDialog::setFilterIndex(filterPos + 1);
}





OUString SAL_CALL CWinFileOpenImpl::getCurrentFilter() throw(uno::RuntimeException)
{
    sal_uInt32 nIndex = getSelectedFilterIndex();

    OUString currentFilter;
    if (nIndex > 0)
    {
        
        if (!m_filterContainer->getFilter(nIndex - 1, currentFilter)) {
            OSL_ASSERT(false);
        }
    }

    return currentFilter;
}


//


inline void SAL_CALL CWinFileOpenImpl::appendFilterGroupSeparator()
{
    m_filterContainer->addFilter(FILTER_SEPARATOR, ALL_FILES_WILDCARD, ALLOW_DUPLICATES);
}





void SAL_CALL CWinFileOpenImpl::appendFilterGroup(const OUString& sGroupTitle, const uno::Sequence<beans::StringPair>& aFilters)
    throw (IllegalArgumentException, uno::RuntimeException)
{
    (void) sGroupTitle; 
    OSL_ENSURE(0 == sGroupTitle.getLength(), "appendFilterGroup: Parameter 'GroupTitle' currently ignored");

    sal_Int32 nFilters = aFilters.getLength();

    OSL_PRECOND(nFilters > 0, "Empty filter list");

    if (nFilters > 0)
    {
        
        
        if (m_filterContainer->numFilter() > 0)
            appendFilterGroupSeparator();

        for (int i = 0; i < nFilters; i++)
            appendFilter(aFilters[i].First, aFilters[i].Second);
    }
}










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






uno::Any SAL_CALL CWinFileOpenImpl::getValue(sal_Int16 aControlId, sal_Int16 aControlAction)
    throw(uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( aControlId ))
        return m_FilePickerState->getValue(aControlId, aControlAction);
    else
        return uno::Any();
}





void SAL_CALL CWinFileOpenImpl::enableControl(sal_Int16 ControlID, sal_Bool bEnable)
    throw(uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( ControlID ))
        m_FilePickerState->enableControl(ControlID, bEnable);
}


//


void SAL_CALL CWinFileOpenImpl::setLabel( sal_Int16 aControlId, const OUString& aLabel )
    throw (uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( aControlId ))
        m_FilePickerState->setLabel(aControlId, aLabel);
}


//


OUString SAL_CALL CWinFileOpenImpl::getLabel( sal_Int16 aControlId )
        throw (uno::RuntimeException)
{
    OSL_ASSERT(m_FilePickerState);
    if ( !filterControlCommand( aControlId ))
        return m_FilePickerState->getLabel(aControlId);
    else
        return OUString();
}


//


uno::Sequence<sal_Int16> SAL_CALL CWinFileOpenImpl::getSupportedImageFormats()
    throw (uno::RuntimeException)
{
    return m_Preview->getSupportedImageFormats();
}


//


sal_Int32 SAL_CALL CWinFileOpenImpl::getTargetColorDepth()
    throw (uno::RuntimeException)
{
    return m_Preview->getTargetColorDepth();
}


//


sal_Int32 SAL_CALL CWinFileOpenImpl::getAvailableWidth()
        throw (uno::RuntimeException)
{
    return m_Preview->getAvailableWidth();
}


//


sal_Int32 SAL_CALL CWinFileOpenImpl::getAvailableHeight()
    throw (uno::RuntimeException)
{
    return m_Preview->getAvailableHeight();
}


//


void SAL_CALL CWinFileOpenImpl::setImage(sal_Int16 aImageFormat, const uno::Any& aImage)
    throw (IllegalArgumentException, uno::RuntimeException)
{
    m_Preview->setImage(aImageFormat,aImage);
}


//


sal_Bool SAL_CALL CWinFileOpenImpl::setShowState(sal_Bool bShowState)
        throw (uno::RuntimeException)
{
    return m_Preview->setShowState(bShowState);
}


//


sal_Bool SAL_CALL CWinFileOpenImpl::getShowState()
    throw (uno::RuntimeException)
{
    return m_Preview->getShowState();
}


//


void SAL_CALL CWinFileOpenImpl::cancel()
{
    if (IsWindow(m_hwndFileOpenDlg))
    {
        
        
        PostMessage(
            m_hwndFileOpenDlg,
            WM_COMMAND,
            MAKEWPARAM(IDCANCEL,BN_CLICKED),
            (LPARAM)GetDlgItem(m_hwndFileOpenDlg, IDCANCEL));
    }
}





sal_Int16 SAL_CALL CWinFileOpenImpl::getFocused()
{
    int nID = GetDlgCtrlID(GetFocus());

    
    
    
    if (nID >= ctlFirst)
        nID = 0;

    return sal::static_int_cast< sal_Int16 >(nID);
}


//


inline sal_Bool SAL_CALL CWinFileOpenImpl::IsCustomControlHelpRequested(LPHELPINFO lphi) const
{
    return ((lphi->iCtrlId != IDOK) && (lphi->iCtrlId != IDCANCEL) && (lphi->iCtrlId < ctlFirst));
}







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

    } 

    return lResult;
}


//


void SAL_CALL CWinFileOpenImpl::InitControlLabel(HWND hWnd)
{
    
    
    

    sal_Int16 aCtrlId = sal::static_int_cast< sal_Int16 >(GetDlgCtrlID(hWnd));
    OUString aLabel = m_ResProvider.getResString(aCtrlId);
    if (aLabel.getLength())
        setLabel(aCtrlId, aLabel);
}















void SAL_CALL CWinFileOpenImpl::InitCustomControlContainer(HWND hCustomControl)
{
    m_CustomControls->AddControl(
        m_CustomControlFactory->CreateCustomControl(hCustomControl,m_hwndFileOpenDlg));
}


//


void SAL_CALL CWinFileOpenImpl::CacheControlState(HWND hWnd)
{
    OSL_ASSERT(m_FilePickerState && m_NonExecuteFilePickerState);
    m_ExecuteFilePickerState->cacheControlState(hWnd, m_NonExecuteFilePickerState);
}


//


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
        
        OSL_ASSERT(sal_False);
    }

    return bRet;
}


//


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


//


void SAL_CALL CWinFileOpenImpl::onSelChanged(HWND)
{
    
    
    
    
    
    
    
    
    
    
    
    
    InitialSetDefaultName();

    FilePickerEvent evt;
    m_FilePicker->fileSelectionChanged(evt);
}






void CWinFileOpenImpl::EnlargeStdControlLabels() const
{
    HWND hFilterBoxLabel = GetDlgItem(m_hwndFileOpenDlg, stc2);
    HWND hFileNameBoxLabel = GetDlgItem(m_hwndFileOpenDlg, stc3);
    HWND hFileNameBox = GetDlgItem(m_hwndFileOpenDlg, cmb13);

    HWND hFilterBox = GetDlgItem(m_hwndFileOpenDlg, cmb1);
    HWND hOkButton = GetDlgItem(m_hwndFileOpenDlg, IDOK);

    
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

    
    m_HelpPopupWindow.setParent(m_hwndFileOpenDlg);

    EnlargeStdControlLabels();

    
    
    
    
    m_ExecuteFilePickerState->setHwnd(m_hwndFileOpenDlgChild);

    m_FilePickerState = m_ExecuteFilePickerState;

    

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

    
    
    m_FilePicker->resumeEventNotification();

    
    PostMessage(
        HWND_BROADCAST,
        RegisterWindowMessage(TEXT("SYSTEM_WINDOW_ACTIVATED")),
        0,
        0);

    
    
    CFileOpenDialog::onInitDone();
}


//


void SAL_CALL CWinFileOpenImpl::onFolderChanged()
{
    FilePickerEvent evt;
    m_FilePicker->directoryChanged(evt);
}


//


void SAL_CALL CWinFileOpenImpl::onTypeChanged(sal_uInt32)
{
    SetDefaultExtension();

    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    m_FilePicker->controlStateChanged(evt);
}





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


//


void CWinFileOpenImpl::onWMSize()
{
    m_Preview->notifyParentSizeChanged();
    m_CustomControls->Align();
    m_FilePicker->dialogSizeChanged();
}


//


void CWinFileOpenImpl::onWMShow(sal_Bool bShow)
{
    m_Preview->notifyParentShow(bShow);
}


//


void CWinFileOpenImpl::onWMWindowPosChanged()
{
    m_Preview->notifyParentWindowPosChanged();
}


//


void CWinFileOpenImpl::onCustomControlHelpRequest(LPHELPINFO lphi)
{
    FilePickerEvent evt;
    evt.ElementId = sal::static_int_cast< sal_Int16 >(lphi->iCtrlId);

    OUString aPopupHelpText = m_FilePicker->helpRequested(evt);

    if (aPopupHelpText.getLength())
    {
        m_HelpPopupWindow.setText(aPopupHelpText);

        DWORD dwMsgPos = GetMessagePos();
        m_HelpPopupWindow.show(LOWORD(dwMsgPos),HIWORD(dwMsgPos));
    }
}


//


void SAL_CALL CWinFileOpenImpl::onInitDialog(HWND hwndDlg)
{
    
    m_pfnOldDlgProc =
        reinterpret_cast<WNDPROC>(
            SetWindowLongPtr( hwndDlg, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(SubClassFunc)));
}





bool SAL_CALL CWinFileOpenImpl::preModal()
{
    CFileOpenDialog::setFilter(
        makeWinFilterBuffer(*m_filterContainer.get()));

    return true;
}





void CWinFileOpenImpl::postModal(sal_Int16 nDialogResult)
{
    CFileOpenDialog::postModal(nDialogResult);

    
    
    
    m_CustomControls->RemoveAllControls();

    m_FilePickerState = m_NonExecuteFilePickerState;
}


//


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

            OUString currentFilter;
            if (nIndex > 0)
            {
                
                m_filterContainer->getFilter(nIndex - 1, currentFilter);

                if (currentFilter.getLength())
                {
                    OUString FilterExt;
                    m_filterContainer->getFilter(currentFilter, FilterExt);

                    sal_Int32 posOfPoint = FilterExt.indexOf(L'.');
                    const sal_Unicode* pFirstExtStart = FilterExt.getStr() + posOfPoint + 1;

                    sal_Int32 posOfSemiColon = FilterExt.indexOf(L';') - 1;
                    if (posOfSemiColon < 0)
                        posOfSemiColon = FilterExt.getLength() - 1;

                    FilterExt = OUString(pFirstExtStart, posOfSemiColon - posOfPoint);

                    SendMessage(m_hwndFileOpenDlg, CDM_SETDEFEXT, 0, reinterpret_cast<LPARAM>(FilterExt.getStr()));
                 }
            }
        }
        else
        {
            SendMessage(m_hwndFileOpenDlg, CDM_SETDEFEXT, 0, reinterpret_cast<LPARAM>(TEXT("")));
        }
    }

    
}


//


void SAL_CALL CWinFileOpenImpl::InitialSetDefaultName()
{
    
    
    
    if (m_bInitialSelChanged && m_defaultName.getLength())
    {
        
        
        
        
        HWND hwndEdt1 = GetDlgItem(m_hwndFileOpenDlg, cmb13);
        SetWindowText(hwndEdt1, reinterpret_cast<LPCTSTR>(m_defaultName.getStr()));
    }

    m_bInitialSelChanged = sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
