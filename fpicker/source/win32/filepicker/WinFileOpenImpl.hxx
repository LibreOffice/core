/*************************************************************************
 *
 *  $RCSfile: WinFileOpenImpl.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:04:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _WINFILEOPENIMPL_HXX_
#define _WINFILEOPENIMPL_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXTENDEDFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_FILEPICKEREVENT_HPP_
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERGROUPMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#endif

#ifndef _FILTER_CONTAINER_HXX_
#include "FilterContainer.hxx"
#endif

#ifndef _FILEOPENDLG_HXX_
#include "FileOpenDlg.hxx"
#endif

#ifndef _PREVIEWADAPTER_HXX_
#include "previewadapter.hxx"
#endif

#ifndef _HELPPOPUPWINDOW_HXX_
#include "helppopupwindow.hxx"
#endif

#ifndef _CUSTOMCONTROL_HXX_
#include "customcontrol.hxx"
#endif

#ifndef _CUSTOMCONTROLFACTORY_HXX_
#include "customcontrolfactory.hxx"
#endif

#include <utility>
#include <memory>
#include <vector>

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

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
    virtual void SAL_CALL onInitDialog( HWND hwndDlg, HWND hwndChild );

    virtual sal_uInt32 SAL_CALL onCtrlCommand( HWND hwndDlg, sal_uInt16 ctrlId, sal_uInt16 notifyCode );


    LRESULT SAL_CALL onWMSize(HWND hwnd, WPARAM type, WORD width, WORD height );
    LRESULT SAL_CALL onWMShow(HWND hwnd, BOOL bShow, int fState);
    LRESULT SAL_CALL onWMWindowPosChanged(HWND hwnd);
    LRESULT SAL_CALL onCustomControlHelpRequest(LPHELPINFO lphi);

private:
    inline void SAL_CALL appendFilterGroupSeparator( );

    inline sal_Bool SAL_CALL IsCustomControlHelpRequested(LPHELPINFO lphi) const;

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
};


#endif
