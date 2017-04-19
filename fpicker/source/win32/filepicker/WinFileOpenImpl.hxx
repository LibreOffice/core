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


#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_WINFILEOPENIMPL_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_WINFILEOPENIMPL_HXX

#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include "FilterContainer.hxx"
#include "FileOpenDlg.hxx"
#include "previewadapter.hxx"
#include "helppopupwindow.hxx"
#include "customcontrol.hxx"
#include "../misc/resourceprovider.hxx"

#include <utility>
#include <memory>
#include <vector>
#include <osl/conditn.hxx>

class CFilePicker;
class CFilePickerState;
class CExecuteFilePickerState;
class CNonExecuteFilePickerState;

class CWinFileOpenImpl : public CFileOpenDialog
{
public:
    CWinFileOpenImpl(
        CFilePicker* aFilePicker,
        bool bFileOpenDialog = true,
        sal_uInt32 dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        sal_uInt32 dwTemplateId = 0,
        HINSTANCE hInstance = nullptr );

    virtual ~CWinFileOpenImpl( ) override;

    // XExecutableDialog

    /// @throws css::uno::RuntimeException
    virtual sal_Int16 SAL_CALL execute(  );

    // XFilePicker

    virtual void SAL_CALL setDefaultName( const OUString& aName ) override;

    /// @throws css::uno::RuntimeException
    virtual css::uno::Sequence< OUString > SAL_CALL getFiles(  );

    virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory ) override;

    /// @throws css::uno::RuntimeException
    virtual OUString SAL_CALL getDisplayDirectory( );

    // XFilterManager

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter );

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setCurrentFilter( const OUString& aTitle );

    /// @throws css::uno::RuntimeException
    virtual OUString  SAL_CALL getCurrentFilter( );

    // XFilterGroupManager

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters );

    // XFilePickerControlAccess

    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const css::uno::Any& aValue );

    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction );

    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL enableControl( sal_Int16 aControlId, bool bEnable );

    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const OUString& aLabel );

    /// @throws css::uno::RuntimeException
    virtual OUString SAL_CALL getLabel( sal_Int16 aControlId );

    // XFilePreview

    /// @throws css::uno::RuntimeException
    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  );

    /// @throws css::uno::RuntimeException
    virtual sal_Int32 SAL_CALL getTargetColorDepth( );

    /// @throws css::uno::RuntimeException
    virtual sal_Int32 SAL_CALL getAvailableWidth( );

    /// @throws css::uno::RuntimeException
    virtual sal_Int32 SAL_CALL getAvailableHeight( );

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const css::uno::Any& aImage );

    /// @throws css::uno::RuntimeException
    virtual bool SAL_CALL setShowState( bool bShowState );

    /// @throws css::uno::RuntimeException
    virtual bool SAL_CALL getShowState( );

    // XCancelable

    virtual void SAL_CALL cancel( );

    // Implementation details

protected:
    static sal_Int16 SAL_CALL getFocused( );

    virtual bool SAL_CALL preModal( ) override;
    virtual void SAL_CALL postModal( sal_Int16 nDialogResult ) override;

    virtual sal_uInt32 SAL_CALL onFileOk() override;
    virtual void SAL_CALL onSelChanged( HWND hwndListBox ) override;

    // only called back if OFN_EXPLORER is set
    virtual void SAL_CALL onInitDone() override;
    virtual void SAL_CALL onFolderChanged() override;
    virtual void SAL_CALL onTypeChanged( sal_uInt32 nFilterIndex ) override;

    // call base class method first when overriding
    virtual void SAL_CALL onInitDialog( HWND hwndDlg ) override;

    virtual sal_uInt32 SAL_CALL onCtrlCommand( HWND hwndDlg, sal_uInt16 ctrlId, sal_uInt16 notifyCode ) override;

    void onWMSize();
    void onWMShow(bool bShow);
    void onWMWindowPosChanged();
    void onCustomControlHelpRequest(LPHELPINFO lphi);

private:
    inline void SAL_CALL appendFilterGroupSeparator( );

    static inline bool SAL_CALL IsCustomControlHelpRequested(LPHELPINFO lphi);

    void EnlargeStdControlLabels() const;

    // initialize all controls from cache
    void SAL_CALL InitControlLabel( HWND hWnd );
    void SAL_CALL InitCustomControlContainer(HWND hCustomControl);

    // save the control state
    void SAL_CALL CacheControlState(HWND hWnd);

    void SAL_CALL SetDefaultExtension();
    void SAL_CALL InitialSetDefaultName();

    static LRESULT CALLBACK SubClassFunc(HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam);

    static BOOL CALLBACK EnumChildWndProc( HWND hWnd, LPARAM lParam );

private:
    std::unique_ptr<CFilterContainer>       m_filterContainer;
    std::unique_ptr<CPreviewAdapter>        m_Preview;
    std::unique_ptr<CCustomControl>         m_CustomControls;
    CFilePicker*                            m_FilePicker;
    WNDPROC                                 m_pfnOldDlgProc;
    OUString                                m_defaultName;
    bool                                    m_bInitialSelChanged;
    CHelpPopupWindow                        m_HelpPopupWindow;
    CFilePickerState*                       m_FilePickerState;
    CExecuteFilePickerState*                m_ExecuteFilePickerState;
    CNonExecuteFilePickerState*             m_NonExecuteFilePickerState;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
