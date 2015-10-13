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
#include "customcontrolfactory.hxx"
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
        sal_Bool bFileOpenDialog = sal_True,
        sal_uInt32 dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        sal_uInt32 dwTemplateId = 0,
        HINSTANCE hInstance = 0 );

    virtual ~CWinFileOpenImpl( );

    // XExecutableDialog

    virtual sal_Int16 SAL_CALL execute(  ) throw( css::uno::RuntimeException );

    // XFilePicker

    virtual void SAL_CALL setDefaultName( const OUString& aName )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual css::uno::Sequence< OUString > SAL_CALL getFiles(  )
        throw(css::uno::RuntimeException );

    virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual OUString SAL_CALL getDisplayDirectory( ) throw ( css::uno::RuntimeException );

    // XFilterManager

    virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual void SAL_CALL setCurrentFilter( const OUString& aTitle )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual OUString  SAL_CALL getCurrentFilter( )
        throw( css::uno::RuntimeException );

    // XFilterGroupManager

    virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    // XFilePickerControlAccess

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const css::uno::Any& aValue )
        throw( css::uno::RuntimeException );

    virtual css::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const OUString& aLabel )
        throw (css::uno::RuntimeException);

    virtual OUString SAL_CALL getLabel( sal_Int16 aControlId )
        throw ( css::uno::RuntimeException);

    // XFilePreview

    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  )
        throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getTargetColorDepth( )
        throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableWidth( )
        throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableHeight( )
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const css::uno::Any& aImage )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState )
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getShowState( )
        throw (css::uno::RuntimeException);

    // XCancelable

    virtual void SAL_CALL cancel( );

    // Implementation details

protected:
    sal_Int16 SAL_CALL getFocused( );

    virtual bool SAL_CALL preModal( );
    virtual void SAL_CALL postModal( sal_Int16 nDialogResult );

    virtual sal_uInt32 SAL_CALL onFileOk();
    virtual void SAL_CALL onSelChanged( HWND hwndListBox );

    // only called back if OFN_EXPLORER is set
    virtual void SAL_CALL onInitDone();
    virtual void SAL_CALL onFolderChanged();
    virtual void SAL_CALL onTypeChanged( sal_uInt32 nFilterIndex );

    // call base class method first when overriding
    virtual void SAL_CALL onInitDialog( HWND hwndDlg );

    virtual sal_uInt32 SAL_CALL onCtrlCommand( HWND hwndDlg, sal_uInt16 ctrlId, sal_uInt16 notifyCode );

    void onWMSize();
    void onWMShow(sal_Bool bShow);
    void onWMWindowPosChanged();
    void onCustomControlHelpRequest(LPHELPINFO lphi);

private:
    inline void SAL_CALL appendFilterGroupSeparator( );

    inline sal_Bool SAL_CALL IsCustomControlHelpRequested(LPHELPINFO lphi) const;

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
    std::unique_ptr<CCustomControlFactory>  m_CustomControlFactory;
    std::unique_ptr<CCustomControl>         m_CustomControls;
    CFilePicker*                            m_FilePicker;
    WNDPROC                                 m_pfnOldDlgProc;
    OUString                                m_defaultName;
    sal_Bool                                m_bInitialSelChanged;
    CHelpPopupWindow                        m_HelpPopupWindow;
    CFilePickerState*                       m_FilePickerState;
    CExecuteFilePickerState*                m_ExecuteFilePickerState;
    CNonExecuteFilePickerState*             m_NonExecuteFilePickerState;
    CResourceProvider                       m_ResProvider;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
