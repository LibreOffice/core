/*************************************************************************
 *
 *  $RCSfile: WinFileOpenImpl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: tra $ $Date: 2001-08-03 14:07:53 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _WINFILEOPENIMPL_HXX_
#include "WinFileOpenImpl.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_FILEPICKEREVENT_HPP_
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_FILEPREVIEWIMAGEFORMATS_HPP_
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_LISTBOXCONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ListBoxControlActions.hpp>
#endif

#ifndef _WINIMPLHELPER_HXX_
#include "..\misc\WinImplHelper.hxx"
#endif

#ifndef _FILEPICKER_HXX_
#include "filepicker.hxx"
#endif

#ifndef _CONTROLACCESS_HXX_
#include "controlaccess.hxx"
#endif

#ifndef _DIBPREVIEW_HXX_
#include "dibpreview.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <systools/win32/comdlg9x.h>
#include <systools/win32/user9x.h>

#ifndef _RESOURCEPROVIDER_HXX_
#include "..\misc\resourceprovider.hxx"
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace std;
using rtl::OUString;
using rtl::OUStringBuffer;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::ui::dialogs::FilePickerEvent;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::ui::dialogs::XFilePicker;

using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::ListboxControlActions;

//-------------------------------------------------------------------------
// to distinguish what to do in the enum child window callback function
//-------------------------------------------------------------------------

enum ECW_ACTION_T
{
    CHECK_PREVIEW = 0,
    INIT_CONTROL_VALUES,
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
//
//-------------------------------------------------------------------------

const sal_Int32 MAX_LABEL = 256;
const OUString BACKSLASH = OUString::createFromAscii( "\\" );

//-------------------------------------------------------------------------
// ctor
//-------------------------------------------------------------------------

CWinFileOpenImpl::CWinFileOpenImpl(
    CFilePicker* aFilePicker,
    sal_Bool bFileOpenDialog,
    sal_uInt32 dwFlags,
    sal_uInt32 dwTemplateId,
    HINSTANCE hInstance ) :
    CFileOpenDialog( bFileOpenDialog, dwFlags, dwTemplateId, hInstance ),
    m_filterContainer( new CFilterContainer( ) ),
    m_FilePicker( aFilePicker ),
    m_bPreviewExists( sal_False ),
    m_bInExecuteMode( sal_False ),
    m_bInitialSelChanged( sal_True ),
    m_HelpPopupWindow( hInstance, m_hwndFileOpenDlg )
{
}

//------------------------------------------------------------------------
// dtor
//------------------------------------------------------------------------

CWinFileOpenImpl::~CWinFileOpenImpl( )
{
}

//------------------------------------------------------------------------
// we expect the directory in URL format
//------------------------------------------------------------------------

void CWinFileOpenImpl::setDisplayDirectory( const OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    OUString aSysDirectory;
    if( aDirectory.getLength( ) > 0 )
    {
        if ( ::osl::FileBase::E_None !=
             ::osl::FileBase::getSystemPathFromFileURL( aDirectory, aSysDirectory )  )
            throw IllegalArgumentException(
                OUString::createFromAscii( "Invalid directory" ),
                static_cast< XFilePicker* >( m_FilePicker ),
                1 );

        // we ensure that there is a trailing '/' at the end of
        // he given file url, because the windows functions only
        // works correctly when providing "c:\" or an environment
        // variable like "=c:=c:\.." etc. is set, else the
        // FolderPicker would stand in the root of the shell
        // hierarchie which is the desktop folder
        if ( aSysDirectory.lastIndexOf( BACKSLASH ) != (aSysDirectory.getLength( ) - 1) )
            aSysDirectory += BACKSLASH;
    }

    // call base class method
    CFileOpenDialog::setDisplayDirectory( aSysDirectory );
}

//------------------------------------------------------------------------
// we return the directory in URL format
//------------------------------------------------------------------------

OUString CWinFileOpenImpl::getDisplayDirectory( ) throw( RuntimeException )
{
    OUString pathURL;
    OUString displayDir;

    if ( m_bInExecuteMode )
    {
        displayDir = getCurrentFolderPath( );
    }
    else
    {
        displayDir = CFileOpenDialog::getDisplayDirectory( );
    }

    if ( displayDir.getLength( ) )
        ::osl::FileBase::getFileURLFromSystemPath( displayDir, pathURL );

    return pathURL;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::setDefaultName( const OUString& aName )
    throw( IllegalArgumentException, RuntimeException )
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

Sequence< OUString > SAL_CALL CWinFileOpenImpl::getFiles(  ) throw(RuntimeException)
{
    Sequence< OUString > aFilePathList;
    OUString aFilePathURL;
    OUString aFilePath;
    ::osl::FileBase::RC rc;

    // in execution mode getFullFileName doesn't
    // return anything, so we must use another way
    if ( m_bInExecuteMode )
    {
        // returns the currently selected file(s)
        // including path information
        aFilePath = getCurrentFilePath( );

        // if multiple files are selected or the user
        // typed anything that doesn't seem to be a valid
        // file name getFileURLFromSystemPath fails
        // and we return an empty file list
        rc = ::osl::FileBase::getFileURLFromSystemPath(
            aFilePath, aFilePathURL );

        if ( ::osl::FileBase::E_None == rc )
        {
            aFilePathList.realloc( 1 );
            aFilePathList[0] = aFilePathURL;
        }
    }
    else // not in execution mode
    {
        aFilePath = getFullFileName( );

        if ( aFilePath.getLength( ) )
        {
            // tokenize the returned string and copy the
            // sub-strings separately into a sequence
            const sal_Unicode* pTemp = aFilePath.getStr( );
            const sal_Unicode* pStrEnd = pTemp + aFilePath.getLength( );
            sal_uInt32 lSubStr;

            while ( pTemp < pStrEnd )
            {
                // detect the length of the next
                // sub string
                lSubStr = wcslen( pTemp );

                aFilePathList.realloc(
                    aFilePathList.getLength( ) + 1 );

                aFilePathList[aFilePathList.getLength( ) - 1] =
                    OUString( pTemp, lSubStr );

                pTemp += (lSubStr + 1);
            }

            // change all entries to file URLs
            sal_Int32 lenFileList = aFilePathList.getLength( );
            OSL_ASSERT( lenFileList >= 1 );

            for ( sal_Int32 i = 0; i < lenFileList; i++ )
            {
                rc = ::osl::FileBase::getFileURLFromSystemPath(
                    aFilePathList[i], aFilePathURL );

                // we do return all or nothing, that means
                // in case of failures we destroy the sequence
                // and return an empty sequence
                if ( rc != ::osl::FileBase::E_None )
                {
                    aFilePathList.realloc( 0 );
                    break;
                }

                aFilePathList[i] = aFilePathURL;
            }
        }
    }

    return aFilePathList;
}

//-----------------------------------------------------------------------------------------
// shows the FileOpen/FileSave dialog
//-----------------------------------------------------------------------------------------

sal_Int16 SAL_CALL CWinFileOpenImpl::execute(  ) throw(RuntimeException)
{
    sal_Int16 rc = CFileOpenDialog::doModal( );

    if ( 1 == rc )
        rc = ::com::sun::star::ui::dialogs::ExecutableDialogResults::OK;
    else if ( 0 == rc )
        rc = ::com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;
    else
        throw RuntimeException(
            OUString::createFromAscii( "Error executing dialog" ),
            static_cast< XFilePicker* >( m_FilePicker ) );

    return rc;
}

//-----------------------------------------------------------------------------------------
// appends a new filter
// returns false if the title (aTitle) was already added or the title or the filter are
// empty
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::appendFilter( const OUString& aTitle, const OUString& aFilter )
    throw(IllegalArgumentException, RuntimeException)
{
    sal_Bool bRet = m_filterContainer->addFilter( aTitle, aFilter );
    if ( !bRet )
        throw IllegalArgumentException(
            OUString::createFromAscii("filter already exists"),
            static_cast< XFilePicker* >( m_FilePicker ),
            1 );
}

//-----------------------------------------------------------------------------------------
// sets a current filter
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::setCurrentFilter( const OUString& aTitle )
    throw( IllegalArgumentException, RuntimeException)
{
    sal_Int32 filterPos = m_filterContainer->getFilterPos( aTitle );

    if ( filterPos < 0 )
        throw IllegalArgumentException(
            OUString::createFromAscii( "filter doesn't exist"),
            static_cast< XFilePicker* >( m_FilePicker ),
            1 );

    // filter index of the base class starts with 1
    CFileOpenDialog::setFilterIndex( filterPos + 1 );
}

//-----------------------------------------------------------------------------------------
// returns the currently selected filter
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CWinFileOpenImpl::getCurrentFilter(  ) throw(RuntimeException)
{
    sal_uInt32 nIndex = getSelectedFilterIndex( );

    OUString currentFilter;
    if ( nIndex > 0 )
    {
        // filter index of the base class starts with 1
        sal_Bool bRet = m_filterContainer->getFilter( nIndex - 1, currentFilter );
        OSL_ASSERT( bRet );
    }

    return currentFilter;
}

//=================================================================================================================
// XExtendedFilePicker
//=================================================================================================================

void SAL_CALL CWinFileOpenImpl::setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const Any& aValue )
    throw(RuntimeException)
{
    if ( m_bInExecuteMode )
    {
        HWND hwndCtrl = GetHwndDlgItem( aControlId );

        // the filter listbox can be manipulated via this
        // method the caller should use XFilterManager
        if ( !hwndCtrl || (aControlId == LISTBOX_FILTER) )
        {
            OSL_ENSURE( sal_False, "invalid control id" );
            return;
        }

        CTRL_CLASS aCtrlClass = GetCtrlClass( hwndCtrl );
        if ( UNKNOWN == aCtrlClass )
        {
            OSL_ENSURE( sal_False, "unsupported control class" );
            return;
        }

        CTRL_SETVALUE_FUNCTION_T lpfnSetValue =
            GetCtrlSetValueFunction( aCtrlClass, aControlAction );

        if ( !lpfnSetValue )
        {
            OSL_ENSURE( sal_False, "unsupported control action" );
            return;
        }

        // the function that we call should throw an IllegalArgumentException if
        // the given value is invalid or empty, that's why we provide a Reference
        // to an XInterface and a argument position
        lpfnSetValue( hwndCtrl, aValue, static_cast< XFilePicker* >( m_FilePicker ), 3 );
    }
    else
    {
        // the last one wins if somebody adds an entry for
        // a checkbox for instance multiple times
        m_ControlCache.push_back(
            ControlCacheEntry( aControlId, aControlAction, aValue ) );
    }
}

//-----------------------------------------------------------------------------------------
// returns the value of an custom template element
// we assume that there are only checkboxes or comboboxes
//-----------------------------------------------------------------------------------------

Any SAL_CALL CWinFileOpenImpl::getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
    throw(RuntimeException)
{
    Any aAny;

    if ( m_bInExecuteMode )
    {
        HWND hwndCtrl = GetHwndDlgItem( aControlId );

        // the filter listbox can be manipulated via this
        // method the caller should use XFilterManager
        if ( !hwndCtrl || (aControlId == LISTBOX_FILTER) )
        {
            OSL_ENSURE( sal_False, "invalid control id" );
            return aAny;
        }

        CTRL_CLASS aCtrlClass = GetCtrlClass( hwndCtrl );
        if ( UNKNOWN == aCtrlClass )
        {
            OSL_ENSURE( sal_False, "unsupported control class" );
            return aAny;
        }

        CTRL_GETVALUE_FUNCTION_T lpfnGetValue =
            GetCtrlGetValueFunction( aCtrlClass, aControlAction );

        if ( !lpfnGetValue )
        {
            OSL_ENSURE( sal_False, "unsupported control action" );
            return aAny;
        }

        aAny = lpfnGetValue( hwndCtrl );
    }
    else
    {
        ControlCache_T::iterator iter_end = m_ControlCache.end( );
        ControlCache_T::iterator iter = m_ControlCache.begin( );

        for ( /* empty */; iter != iter_end; ++iter )
        {
            if ( (iter->m_controlId == aControlId) &&
                 (iter->m_controlAction == aControlAction) )
            {
                aAny = iter->m_Value;
                break;
            }
        }
    }

    return aAny;
}

//-----------------------------------------------------------------------------------------
// enables a custom template element
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::enableControl( sal_Int16 ControlID, sal_Bool bEnable )
    throw(RuntimeException)
{
    HWND hwndCtrl = GetHwndDlgItem( ControlID );

    if ( !hwndCtrl )
    {

#pragma message( "################################" )
#pragma message( " fix this" )
#pragma message( "################################" )
        /*
        will be enabled again when enableControl in offline
        mode is ready

        OSL_ENSURE( sal_False, "invalid element id");
        */

        return;
    }

    EnableWindow( hwndCtrl, bEnable );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel )
    throw (RuntimeException)
{
    HWND hwndCtrl = GetHwndDlgItem( aControlId );

    if ( !hwndCtrl )
    {
#pragma message( "################################" )
#pragma message( " fix this" )
#pragma message( "################################" )
        /*
        will be enabled again when setLabel in offline
        mode is ready

        OSL_ENSURE( sal_False, "invalid element id");
        */
        return;
    }

    // somewhat risky because we don't know if OUString
    // has a terminating '\0'
    SetWindowTextW( hwndCtrl, aLabel.getStr( ) );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CWinFileOpenImpl::getLabel( sal_Int16 aControlId )
        throw (RuntimeException)
{
    HWND hwndCtrl = GetHwndDlgItem( aControlId );

    if ( !hwndCtrl )
    {
#pragma message( "################################" )
#pragma message( " fix this" )
#pragma message( "################################" )
        /*
        will be enabled again when setLabel in offline
        mode is ready
        OSL_ENSURE( sal_False, "invalid element id");
        */

        return OUString( );
    }

    sal_Unicode aLabel[MAX_LABEL];
    int nRet = GetWindowTextW( hwndCtrl, aLabel, MAX_LABEL );

    OUString ctrlLabel;
    if ( nRet )
        ctrlLabel = OUString( aLabel, wcslen( aLabel ) );

    return ctrlLabel;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

Sequence< sal_Int16 > SAL_CALL CWinFileOpenImpl::getSupportedImageFormats(  )
    throw (RuntimeException)
{
    Sequence< sal_Int16 > imgFormatList( 1 );
    imgFormatList[0] = ::com::sun::star::ui::dialogs::FilePreviewImageFormats::BITMAP;
    return imgFormatList;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CWinFileOpenImpl::getTargetColorDepth( )
    throw (RuntimeException)
{
    if ( m_DIBPreview.get( ) )
        return m_DIBPreview->getColorDepth( );

    return 0;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CWinFileOpenImpl::getAvailableWidth( )
        throw (RuntimeException)
{
    if ( m_DIBPreview.get( ) )
        return m_DIBPreview->getWidth( );

    return 0;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CWinFileOpenImpl::getAvailableHeight( )
    throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_DIBPreview.get( ) )
        return m_DIBPreview->getHeight( );

    return 0;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::setImage( sal_Int16 aImageFormat, const Any& aImage )
    throw (IllegalArgumentException, RuntimeException)
{
    if ( m_DIBPreview.get( ) )
    {
        if ( aImageFormat != ::com::sun::star::ui::dialogs::FilePreviewImageFormats::BITMAP )
            throw IllegalArgumentException(
                OUString::createFromAscii("unsupported image format"),
                static_cast< XFilePicker* >( m_FilePicker ),
                1 );

        if ( aImage.hasValue( ) &&
             (aImage.getValueType( ) != getCppuType( (Sequence< sal_Int8 >*) 0 ) ) )
            throw IllegalArgumentException(
                OUString::createFromAscii("invalid image data"),
                static_cast< XFilePicker* >( m_FilePicker ),
                2 );

        Sequence< sal_Int8 > ImageData;

        // if the any has no value we have an
        // empty Sequence which clears the
        // preview window
        if ( aImage.hasValue( ) )
            aImage >>= ImageData;

        m_DIBPreview->setImage( ImageData );
    }
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CWinFileOpenImpl::setShowState( sal_Bool bShowState )
        throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_DIBPreview.get( ) )
        return m_DIBPreview->show( bShowState );

    return sal_False;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CWinFileOpenImpl::getShowState( )
    throw (RuntimeException)
{
    if ( m_DIBPreview.get( ) )
        return m_DIBPreview->isVisible( );

    return sal_False;
}

//-----------------------------------------------------------------------------------------
// returns the id of a custom template element
//-----------------------------------------------------------------------------------------

sal_Int16 SAL_CALL CWinFileOpenImpl::getFocused( )
{
    sal_Int32 nID = GetDlgCtrlID( GetFocus( ) );

    // we don't forward id's of standard file open
    // dialog elements (ctlFirst is defined in dlgs.h
    // in MS Platform SDK)
    if ( nID >= ctlFirst )
        nID = 0;

    return nID;
}

//-----------------------------------------------------------------------------------------
// our own DlgProc because we do subclass the dialog
// we catch the WM_NCDESTROY message in order to erase an entry in our static map
// if one instance dies
//-----------------------------------------------------------------------------------------

unsigned int CALLBACK CWinFileOpenImpl::SubClassFunc(
    HWND hWnd, WORD wMessage, WPARAM wParam, LPARAM lParam )
{
    unsigned int lResult = 0;

    CWinFileOpenImpl* pImpl =
        dynamic_cast< CWinFileOpenImpl* >(
            getCurrentInstance( hWnd ) );
    OSL_ASSERT( pImpl );

    switch( wMessage )
    {
    case WM_HELP:
    {
        LPHELPINFO lphi = reinterpret_cast< LPHELPINFO >( lParam );

        // we handle only our own elements ourself
        if ( (lphi->iCtrlId != IDOK) && (lphi->iCtrlId != IDCANCEL) && (lphi->iCtrlId < ctlFirst) )
        {
            FilePickerEvent evt;
            evt.ElementId = lphi->iCtrlId;

            OUString aPopupHelpText = pImpl->m_FilePicker->helpRequested( evt );

            if ( aPopupHelpText.getLength( ) )
            {
                pImpl->m_HelpPopupWindow.setText( aPopupHelpText );

                DWORD dwMsgPos = GetMessagePos( );
                pImpl->m_HelpPopupWindow.show( LOWORD( dwMsgPos ), HIWORD( dwMsgPos ) );
            }
        }
        else // call the standard help
            lResult = CallWindowProcA(
                reinterpret_cast< WNDPROC >( pImpl->m_pfnOldDlgProc ),
                hWnd,
                wMessage,
                wParam,
                lParam );
    }
    break;

    default:
        // !!! we use CallWindowProcA
        lResult = CallWindowProcA(
            reinterpret_cast< WNDPROC >( pImpl->m_pfnOldDlgProc ),
            hWnd,
            wMessage,
            wParam,
            lParam );

    break;

    } // switch

    return lResult;
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::InitControlState( HWND hWnd )
{
    CResourceProvider aResProvider;

    ControlCache_T::iterator iter_end = m_ControlCache.end( );
    ControlCache_T::iterator iter = m_ControlCache.begin( );

    for ( /* empty */; iter != iter_end; ++iter )
    {
        if ( GetDlgCtrlID( hWnd ) == iter->m_controlId )
            setValue(
                iter->m_controlId,
                iter->m_controlAction,
                iter->m_Value );
    }

    //-----------------------------------------
    // set the labels for all extendet controls
    //-----------------------------------------

    sal_Int32 aCtrlId = GetDlgCtrlID( hWnd );
    OUString aLabel = aResProvider.getResString( aCtrlId );
    if ( aLabel.getLength( ) )
        setLabel( aCtrlId, aLabel );
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::CacheControlState( HWND hWnd )
{
    CTRL_CLASS aCtrlClass = GetCtrlClass( hWnd );

    if ( UNKNOWN == aCtrlClass )
        return;

    sal_Int16 aControlAction;

    if ( CHECKBOX == aCtrlClass )
    {
        aControlAction = 0;
    }
    else if ( LISTBOX == aCtrlClass )
    {
        // for listboxes we save only the
        // last selected item
        aControlAction = GET_SELECTED_ITEM;
    }

    CTRL_GETVALUE_FUNCTION_T lpfnGetValue =
        GetCtrlGetValueFunction( aCtrlClass, aControlAction );

    OSL_ASSERT( lpfnGetValue );

    m_ControlCache.push_back(
        ControlCacheEntry(
        GetDlgCtrlID( hWnd ),
        aControlAction,
        lpfnGetValue( hWnd ) ) );
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

sal_Bool SAL_CALL CWinFileOpenImpl::HasPreview( HWND hWnd )
{
    if ( CHECKBOX_PREVIEW == GetDlgCtrlID( hWnd ) )
        m_bPreviewExists = sal_True;

    return m_bPreviewExists;
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

BOOL CALLBACK CWinFileOpenImpl::EnumChildWndProc( HWND hWnd, LPARAM lParam )
{
    EnumParam* enumParam    = (EnumParam*)lParam;
    CWinFileOpenImpl* pImpl = enumParam->m_instance;

    OSL_ASSERT( pImpl );

    BOOL bRet = TRUE;

    switch( enumParam->m_action )
    {
    case CHECK_PREVIEW:
        if ( pImpl->HasPreview( hWnd ) )
            bRet = FALSE;
    break;

    case INIT_CONTROL_VALUES:
        pImpl->InitControlState( hWnd );
    break;

    case CACHE_CONTROL_VALUES:
        pImpl->CacheControlState( hWnd );
    break;

    default:
        // should not end here
        OSL_ASSERT( sal_False );
    }

    return bRet;
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

sal_uInt32 SAL_CALL CWinFileOpenImpl::onFileOk()
{
    // fill the control value cache
    m_ControlCache.clear( );

    EnumParam enumParam( CACHE_CONTROL_VALUES, this );

    EnumChildWindows(
        m_hwndFileOpenDlgChild,
        CWinFileOpenImpl::EnumChildWndProc,
        (LPARAM)&enumParam );

    return 0;
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onSelChanged( HWND hwndListBox )
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
    InitialSetDefaultName( );

    FilePickerEvent evt;
    evt.Source = static_cast< XFilePicker* >( m_FilePicker );
    m_FilePicker->fileSelectionChanged( evt );
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onHelp( )
{
    FilePickerEvent evt;
    evt.ElementId = getFocused( );
    evt.Source = static_cast< XFilePicker* >( m_FilePicker );
    m_FilePicker->helpRequested( evt );
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onInitDone()
{
    // we check if the checkbox is present and if so
    // create a preview window
    EnumParam enumParam( CHECK_PREVIEW, this );

    EnumChildWindows(
        m_hwndFileOpenDlgChild,
        CWinFileOpenImpl::EnumChildWndProc,
        (LPARAM)&enumParam );

    // create and display the preview control

    if ( m_bPreviewExists )
    {
        // lst1 is the file listbox and is defined by MS in dlgs.h
        HWND hwndFileListbox = GetDlgItem( m_hwndFileOpenDlg, lst1 );
        OSL_ASSERT( IsWindow( hwndFileListbox ) );

        // save the original size of the file listbox
        RECT rcFileListbox;
        GetWindowRect( hwndFileListbox, &rcFileListbox );

        m_SizeFileListBoxOriginal = std::make_pair(
            rcFileListbox.right - rcFileListbox.left,
            rcFileListbox.bottom - rcFileListbox.top );

        // shrink the width of the fileopen file listbox
        sal_uInt32 newWidthFileListbox =
            static_cast< sal_uInt32 >(
            (rcFileListbox.right - rcFileListbox.left) / 2);

        // resize the fileopen file listbox
        SetWindowPos( hwndFileListbox,
            NULL, 0, 0, newWidthFileListbox,
            m_SizeFileListBoxOriginal.second,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

        // get the new size of the file listbox
        GetWindowRect( hwndFileListbox, &rcFileListbox );

        POINT ulCorner;
        ulCorner.x = rcFileListbox.right;
        ulCorner.y = rcFileListbox.top;

        ScreenToClient( m_hwndFileOpenDlg, &ulCorner );

        DIMENSION_T sizePreviewWnd = std::make_pair(
            (rcFileListbox.right - rcFileListbox.left) - 3,
            (rcFileListbox.bottom - rcFileListbox.top) );

        // we try to get a window handle to a control with
        // the id 1119 which is defined as stc32 in the
        // Platform SDK header files and has a special
        // meaning (see MSDN under the title:
        // "Explorer-Style Custom Templates")
        // if this control is available, all standard controls,
        // will be placed on this control, so we have to use
        // it as parent for our preview window else the stc32
        // control may paint over the client area of our
        // preview control
        // if there is no stc32 control, all standard controls
        // are childs of the dialog box itself
        HWND hwndParent;
        HWND hwndStc32 = GetDlgItem( m_hwndFileOpenDlgChild, 1119 );
        if ( IsWindow( hwndStc32 ) )
            hwndParent = hwndStc32;
        else
            hwndParent = m_hwndFileOpenDlg;

        // as parent we use the
        CDIBPreview* pDIBPreview = new CDIBPreview(
            ulCorner.x,
            ulCorner.y,
            sizePreviewWnd.first,
            sizePreviewWnd.second,
            hwndParent,
            m_ofn.hInstance );

        m_DIBPreview.reset( pDIBPreview );

        // restore the origional size of the file listbox on failure
        if ( m_DIBPreview.get( ) &&
             !(m_DIBPreview->getWidth( ) && m_DIBPreview->getHeight( )) )
        {
            SetWindowPos( hwndFileListbox,
                NULL, 0, 0,
                m_SizeFileListBoxOriginal.first,
                m_SizeFileListBoxOriginal.second,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
        }
    }

    // initialize controls from cache
    enumParam.m_action   = INIT_CONTROL_VALUES;
    enumParam.m_instance = this;

    EnumChildWindows(
        m_hwndFileOpenDlgChild,
        CWinFileOpenImpl::EnumChildWndProc,
        (LPARAM)&enumParam );

    SetDefaultExtension( );

    // but now we have a valid parent handle
    m_HelpPopupWindow.setParent( m_hwndFileOpenDlg );
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onFolderChanged()
{
    FilePickerEvent evt;
    evt.Source = static_cast< XFilePicker* >( m_FilePicker );
    m_FilePicker->directoryChanged( evt );
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onTypeChanged( sal_uInt32 nFilterIndex )
{
    SetDefaultExtension( );

    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    evt.Source = static_cast< XFilePicker* >( m_FilePicker );
    m_FilePicker->controlStateChanged( evt );
}

//-----------------------------------------------------------------------------------------
// onMessageCommand handler
//-----------------------------------------------------------------------------------------

sal_uInt32 SAL_CALL CWinFileOpenImpl::onCtrlCommand(
    HWND hwndDlg, sal_uInt16 ctrlId, sal_uInt16 notifyCode )
{
    SetDefaultExtension( );

    if ( ctrlId < ctlFirst )
    {
        FilePickerEvent evt;

        evt.ElementId = ctrlId;
        evt.Source = static_cast< XFilePicker* >( m_FilePicker );
        m_FilePicker->controlStateChanged( evt );
    }

    return 0;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::onInitDialog( HWND hwndDlg, HWND hwndChild )
{
    // subclass the dialog window
    m_pfnOldDlgProc =
        reinterpret_cast< DLGPROC >(
            SetWindowLong( hwndDlg,
            DWL_DLGPROC,
            reinterpret_cast< DWORD >( SubClassFunc ) ) );
}

//-----------------------------------------------------------------------------------------
// processing before showing the dialog
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CWinFileOpenImpl::preModal( )
{
    CFileOpenDialog::setFilter(
        makeWinFilterBuffer( *m_filterContainer.get( ) ) );

    m_bInExecuteMode = sal_True;

    return sal_True;
}

//-----------------------------------------------------------------------------------------
// processing after showing the dialog
//-----------------------------------------------------------------------------------------

void CWinFileOpenImpl::postModal( sal_Int16 nDialogResult )
{
    CFileOpenDialog::postModal( nDialogResult );

    m_bInExecuteMode = sal_False;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

HWND SAL_CALL CWinFileOpenImpl::GetHwndDlgItem( sal_Int16 ctrlId, sal_Bool bIncludeStdCtrls )
{
    HWND hwndCtrl = GetDlgItem( m_hwndFileOpenDlgChild, ctrlId );

    // maybe it's a control of the dialog itself for instance
    // the ok and cancel button
    if ( !hwndCtrl && bIncludeStdCtrls )
    {
        ctrlId = CommonFilePickerCtrlIdToWinFileOpenCtrlId( ctrlId );
        hwndCtrl = GetDlgItem( m_hwndFileOpenDlg, ctrlId );
    }

    return hwndCtrl;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::SetDefaultExtension( )
{
    // !!! HACK !!!

    OSVERSIONINFOA  OSVerInfo;

    OSVerInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFOA );
    GetVersionExA( &OSVerInfo );

    // if windows 95/98
    sal_Bool bIsWin9x = ( VER_PLATFORM_WIN32_WINDOWS == OSVerInfo.dwPlatformId );

    HWND hwndChkSaveWithExt = GetDlgItem( m_hwndFileOpenDlgChild, 100 );

    if ( hwndChkSaveWithExt )
    {
        Any aAny = CheckboxGetState( hwndChkSaveWithExt );
        sal_Bool bChecked = *reinterpret_cast< const sal_Bool* >( aAny.getValue( ) );

        if ( bChecked )
        {
            sal_uInt32 nIndex = getSelectedFilterIndex( );

            OUString currentFilter;
            if ( nIndex > 0 )
            {
                // filter index of the base class starts with 1
                sal_Bool bRet = m_filterContainer->getFilter( nIndex - 1, currentFilter );

                if ( currentFilter.getLength( ) )
                {
                    OUString FilterExt;
                    m_filterContainer->getFilter( currentFilter, FilterExt );

                    sal_Int32 posOfPoint = FilterExt.indexOf( L'.' );
                    const sal_Unicode* pFirstExtStart = FilterExt.getStr( ) + posOfPoint + 1;

                    sal_Int32 posOfSemiColon = FilterExt.indexOf( L';' ) - 1;
                    if ( posOfSemiColon < 0 )
                        posOfSemiColon = FilterExt.getLength( ) - 1;

                    FilterExt = OUString( pFirstExtStart, posOfSemiColon - posOfPoint );

                    if ( bIsWin9x )
                    {
                        rtl::OString tmp = rtl::OUStringToOString( FilterExt, osl_getThreadTextEncoding( ) );

                        SendMessageA( m_hwndFileOpenDlg, CDM_SETDEFEXT, 0, (LPARAM)( tmp.getStr( ) ) );
                    }
                    else
                    {
                        SendMessageW( m_hwndFileOpenDlg, CDM_SETDEFEXT, 0, (LPARAM)( FilterExt.getStr( ) ) );
                    }
                }
            }
        }
        else
        {
            if ( bIsWin9x )
                SendMessageA( m_hwndFileOpenDlg, CDM_SETDEFEXT, 0, (LPARAM)"" );
            else
                SendMessageW( m_hwndFileOpenDlg, CDM_SETDEFEXT, 0, (LPARAM)L"");
        }
    }

    // !!! HACK !!!
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CWinFileOpenImpl::InitialSetDefaultName( )
{
    // manually setting the file name that appears
    // initially in the file-name-box of the file
    // open dialog (reason: see above setDefaultName)
    if ( m_bInitialSelChanged && m_defaultName.getLength( ) )
    {
        sal_Int32 edt1Id = edt1;

        // under W2k the there is a combobox instead
        // of an edit field for the file name edit field
        // the control id of this box is cmb13 and not
        // edt1 as before so we must use this id
        if ( IsWin2000( ) )
            edt1Id = cmb13;

        HWND hwndEdt1 = GetDlgItem( m_hwndFileOpenDlg, edt1Id );
        SetWindowTextW( hwndEdt1, m_defaultName.getStr( ) );
    }

    m_bInitialSelChanged = sal_False;
}