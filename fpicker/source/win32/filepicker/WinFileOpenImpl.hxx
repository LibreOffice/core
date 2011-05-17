/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#ifndef _WINFILEOPENIMPL_HXX_
#define _WINFILEOPENIMPL_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include "FilterContainer.hxx"
#include "FileOpenDlg.hxx"
#include "previewadapter.hxx"
#include "helppopupwindow.hxx"
#include "customcontrol.hxx"
#include "customcontrolfactory.hxx"
#include "..\misc\resourceprovider.hxx"

#include <utility>
#include <memory>
#include <vector>
#include <osl/conditn.hxx>

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

// forward declaration
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

    //-----------------------------------------------------------------------------------------
    // XExecutableDialog
    //-----------------------------------------------------------------------------------------

    virtual sal_Int16 SAL_CALL execute(  ) throw( ::com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------------------------------
    // XFilePicker
    //-----------------------------------------------------------------------------------------

    virtual void SAL_CALL setDefaultName( const ::rtl::OUString& aName )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles(  )
        throw(::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDisplayDirectory( const ::rtl::OUString& aDirectory )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getDisplayDirectory( ) throw ( ::com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------------------------------
    // XFilterManager
    //-----------------------------------------------------------------------------------------

    virtual void SAL_CALL appendFilter( const ::rtl::OUString& aTitle, const ::rtl::OUString& aFilter )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setCurrentFilter( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString  SAL_CALL getCurrentFilter( )
        throw( ::com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------------------------------
    // XFilterGroupManager
    //-----------------------------------------------------------------------------------------

    virtual void SAL_CALL appendFilterGroup( const ::rtl::OUString& sGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aFilters )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------------------------------
    // XFilePickerControlAccess
    //-----------------------------------------------------------------------------------------

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const ::com::sun::star::uno::Any& aValue )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 aControlId )
        throw ( ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XFilePreview
    //------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getTargetColorDepth( )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableWidth( )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableHeight( )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getShowState( )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XCancelable
    //------------------------------------------------

    virtual void SAL_CALL cancel( );

    //------------------------------------------------
    // Implementation details
    //------------------------------------------------

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

    // call base class method first when overloading
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
    std::auto_ptr<CFilterContainer>         m_filterContainer;
    std::auto_ptr<CPreviewAdapter>          m_Preview;
    std::auto_ptr<CCustomControlFactory>    m_CustomControlFactory;
    std::auto_ptr<CCustomControl>           m_CustomControls;
    CFilePicker*                            m_FilePicker;
    WNDPROC                                 m_pfnOldDlgProc;
    rtl::OUString                           m_defaultName;
    sal_Bool                                m_bInitialSelChanged;
    CHelpPopupWindow                        m_HelpPopupWindow;
    CFilePickerState*                       m_FilePickerState;
    CExecuteFilePickerState*                m_ExecuteFilePickerState;
    CNonExecuteFilePickerState*             m_NonExecuteFilePickerState;
    CResourceProvider                       m_ResProvider;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
