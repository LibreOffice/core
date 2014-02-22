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

#include <boost/scoped_ptr.hpp>

#include "filepickerstate.hxx"
#include <osl/diagnose.h>
#include "controlaccess.hxx"
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include "controlcommandrequest.hxx"
#include "controlcommandresult.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <osl/file.hxx>
#include "FileOpenDlg.hxx"

#include "../misc/WinImplHelper.hxx"

//


using com::sun::star::uno::Any;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;

using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::ListboxControlActions;


//


const sal_Int32 MAX_LABEL = 256;
const sal_Int16 LISTBOX_LABEL_OFFSET = 100;





CFilePickerState::~CFilePickerState( )
{
}


//


CNonExecuteFilePickerState::CNonExecuteFilePickerState( ) :
    m_FirstControlCommand( NULL )
{
}


//


CNonExecuteFilePickerState::~CNonExecuteFilePickerState( )
{
    reset( );
}


//


void SAL_CALL CNonExecuteFilePickerState::setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const Any& aValue )
{
    CValueControlCommand* value_command = new CValueControlCommand(
        aControlId, aControlAction, aValue );

    addControlCommand( value_command );
}


//


Any SAL_CALL CNonExecuteFilePickerState::getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
{
    CValueControlCommandRequest value_request( aControlId, aControlAction );
    Any aAny;

    if (m_FirstControlCommand)
    {
        
        boost::scoped_ptr< CControlCommandResult > result( m_FirstControlCommand->handleRequest( &value_request ) );

        OSL_ENSURE( result.get(), "invalid getValue request" );

        if ( result.get() )
        {
            
            
            
            
            
            
            
            
            
            

            if ( result->hasResult( ) )
            {
                CValueCommandResult* value_result = dynamic_cast< CValueCommandResult* >( result.get( ) );
                OSL_ENSURE( value_result, "should have be a CValueCommandResult" );

                aAny = value_result->getValue( );
                OSL_ENSURE( aAny.hasValue( ), "empty any" );
            }
        }
    }

    return aAny;
}


//


void SAL_CALL CNonExecuteFilePickerState::enableControl( sal_Int16 aControlId, sal_Bool bEnable )
{
    CEnableControlCommand* enable_command = new CEnableControlCommand(
        aControlId, bEnable );

    addControlCommand( enable_command );
}


//


void SAL_CALL CNonExecuteFilePickerState::setLabel( sal_Int16 aControlId, const OUString& aLabel )
{
    CLabelControlCommand* label_command = new CLabelControlCommand(
        aControlId, aLabel );

    addControlCommand( label_command );
}


//


OUString SAL_CALL CNonExecuteFilePickerState::getLabel( sal_Int16 aControlId )
{
    CControlCommandRequest label_request( aControlId );

    
    boost::scoped_ptr< CControlCommandResult > result( m_FirstControlCommand->handleRequest( &label_request ) );

    OSL_ENSURE( result->hasResult( ), "invalid getValue request" );

    OUString aLabel;

    if ( result->hasResult( ) )
    {
        CLabelCommandResult* label_result = dynamic_cast< CLabelCommandResult* >( result.get( ) );
        OSL_ENSURE( label_result, "should have be a CLabelCommandResult" );

        aLabel = label_result->getLabel( );
    }

    return aLabel;
}

/*  #i26224#
    When typing file names with drive letter but without '\'
    in the "File name" box of the FileOpen dialog the FileOpen
    dialog makes strange paths out of them e.g. "d:.\test.sxw".
    Such file names will not be accepted by sal so we fix these
    somehow broken paths here. */
OUString MatchFixBrokenPath(const OUString& path)
{
    OSL_ASSERT(path.getLength() >= 4);

    if (path[1] == ':' && path[2] == '.' && path[3] == '\\')
    {
        
        return OUString(path.getStr(), 2) + path.copy(3, path.getLength() - 3);
    }
    return path;
}


//

static OUString trimTrailingSpaces(const OUString& rString)
{
    OUString aResult(rString);

    sal_Int32 nIndex = rString.lastIndexOf(' ');
    if (nIndex == rString.getLength()-1)
    {
        while (nIndex >= 0 && rString[nIndex] == ' ')
            nIndex--;
        if (nIndex >= 0)
            aResult = rString.copy(0,nIndex+1);
        else
            aResult = OUString();
    }
    return aResult;
}

Sequence< OUString > SAL_CALL CNonExecuteFilePickerState::getFiles( CFileOpenDialog* aFileOpenDialog )
{
    OSL_PRECOND( aFileOpenDialog, "invalid parameter" );

    Sequence< OUString > aFilePathList;
    OUString aFilePathURL;
    OUString aFilePath;
    ::osl::FileBase::RC rc;

    aFilePath = aFileOpenDialog->getFullFileName( );

    if ( aFilePath.getLength( ) )
    {
        
        
        const sal_Unicode* pTemp = aFilePath.getStr();
        const sal_Unicode* pStrEnd = pTemp + aFilePath.getLength();
        sal_uInt32 lSubStr;

        while (pTemp < pStrEnd)
        {
            
            lSubStr = rtl_ustr_getLength(pTemp);

            aFilePathList.realloc(aFilePathList.getLength() + 1);

            aFilePathList[aFilePathList.getLength() - 1] =
                MatchFixBrokenPath(OUString(pTemp, lSubStr));

            pTemp += (lSubStr + 1);
        }

        
        sal_Int32 lenFileList = aFilePathList.getLength( );
        OSL_ASSERT( lenFileList >= 1 );

        for ( sal_Int32 i = 0; i < lenFileList; i++ )
        {
            aFilePath = trimTrailingSpaces(aFilePathList[i]);
            rc = ::osl::FileBase::getFileURLFromSystemPath(
                aFilePath, aFilePathURL );

            
            
            
            if ( rc != ::osl::FileBase::E_None )
            {
                aFilePathList.realloc( 0 );
                break;
            }

            aFilePathList[i] = aFilePathURL;
        }
    }

    return aFilePathList;
}


//


OUString SAL_CALL CNonExecuteFilePickerState::getDisplayDirectory( CFileOpenDialog* aFileOpenDialog )
{
    OSL_PRECOND( aFileOpenDialog, "invalid parameter" );

    OUString pathURL;
    OUString displayDir;

    displayDir = aFileOpenDialog->getLastDisplayDirectory( );

    if ( displayDir.getLength( ) )
        ::osl::FileBase::getFileURLFromSystemPath( displayDir, pathURL );

    return pathURL;
}


//


void SAL_CALL CNonExecuteFilePickerState::reset( )
{
    CControlCommand* nextCommand;
    CControlCommand* currentCommand = m_FirstControlCommand;

    while( currentCommand )
    {
        nextCommand = currentCommand->getNextCommand( );
        delete currentCommand;
        currentCommand = nextCommand;
    }

    m_FirstControlCommand = NULL;
}


//


CControlCommand* SAL_CALL CNonExecuteFilePickerState::getControlCommand( ) const
{
    return m_FirstControlCommand;
}







void SAL_CALL CNonExecuteFilePickerState::addControlCommand( CControlCommand* aControlCommand )
{
    OSL_ASSERT( aControlCommand );

    if ( NULL == m_FirstControlCommand )
    {
        m_FirstControlCommand = aControlCommand;
    }
    else
    {
        CControlCommand* pNextControlCommand = m_FirstControlCommand;

        while ( pNextControlCommand->getNextCommand( ) != NULL )
            pNextControlCommand = pNextControlCommand->getNextCommand( );

        pNextControlCommand->setNextCommand( aControlCommand );
    }
}




//


CExecuteFilePickerState::CExecuteFilePickerState( HWND hwndDlg ) :
    m_hwndDlg( hwndDlg )
{
}


//


void SAL_CALL CExecuteFilePickerState::setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const Any& aValue )
{
    
    if ( com::sun::star::ui::dialogs::ControlActions::SET_HELP_URL == aControlAction )
        return;

    HWND hwndCtrl = GetHwndDlgItem( aControlId );

    
    
    if ( !hwndCtrl || (aControlId == LISTBOX_FILTER) )
    {
        OSL_FAIL( "invalid control id" );
        return;
    }

    CTRL_CLASS aCtrlClass = GetCtrlClass( hwndCtrl );
    if ( UNKNOWN == aCtrlClass )
    {
        OSL_FAIL( "unsupported control class" );
        return;
    }

    CTRL_SETVALUE_FUNCTION_T lpfnSetValue =
        GetCtrlSetValueFunction( aCtrlClass, aControlAction );

    if ( !lpfnSetValue )
    {
        OSL_FAIL( "unsupported control action" );
        return;
    }

    
    
    
    lpfnSetValue( hwndCtrl, aValue, Reference< XInterface >( ), 3 );
}


//


Any SAL_CALL CExecuteFilePickerState::getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
{
    
    if ( com::sun::star::ui::dialogs::ControlActions::GET_HELP_URL == aControlAction )
        return Any( );

    HWND hwndCtrl = GetHwndDlgItem( aControlId );

    
    
    if ( !hwndCtrl || (aControlId == LISTBOX_FILTER) )
    {
        OSL_FAIL( "invalid control id" );
        return Any( );
    }

    CTRL_CLASS aCtrlClass = GetCtrlClass( hwndCtrl );
    if ( UNKNOWN == aCtrlClass )
    {
        OSL_FAIL( "unsupported control class" );
        return Any( );
    }

    CTRL_GETVALUE_FUNCTION_T lpfnGetValue =
        GetCtrlGetValueFunction( aCtrlClass, aControlAction );

    if ( !lpfnGetValue )
    {
        OSL_FAIL( "unsupported control action" );
        return Any( );
    }

    return lpfnGetValue( hwndCtrl );
}


//


void SAL_CALL CExecuteFilePickerState::enableControl( sal_Int16 aControlId, sal_Bool bEnable )
{
    HWND hwndCtrl = GetHwndDlgItem( aControlId );

    OSL_ENSURE( IsWindow( hwndCtrl ), "invalid element id");

    EnableWindow( hwndCtrl, bEnable );
}


//


void SAL_CALL CExecuteFilePickerState::setLabel( sal_Int16 aControlId, const OUString& aLabel )
{
    HWND hwndCtrl = GetHwndDlgItem( aControlId );

    OSL_ENSURE( IsWindow( hwndCtrl ), "invalid element id");

    if ( IsListboxControl( hwndCtrl ) )
        hwndCtrl = GetListboxLabelItem( aControlId );

    OUString aWinLabel = SOfficeToWindowsLabel( aLabel );

    
    
    SetWindowText( hwndCtrl, reinterpret_cast<LPCTSTR>(aWinLabel.getStr( )) );
}


//


OUString SAL_CALL CExecuteFilePickerState::getLabel( sal_Int16 aControlId )
{
    HWND hwndCtrl = GetHwndDlgItem( aControlId );

    OSL_ENSURE( IsWindow( hwndCtrl ), "invalid element id");

    if ( IsListboxControl( hwndCtrl ) )
        hwndCtrl = GetListboxLabelItem( aControlId );

    sal_Unicode aLabel[MAX_LABEL];
    int nRet = GetWindowText( hwndCtrl, reinterpret_cast<LPTSTR>(aLabel), MAX_LABEL );

    OUString ctrlLabel;
    if ( nRet )
    {
        ctrlLabel = WindowsToSOfficeLabel( aLabel );
    }

    return ctrlLabel;
}


//


Sequence< OUString > SAL_CALL CExecuteFilePickerState::getFiles( CFileOpenDialog* aFileOpenDialog )
{
    OSL_POSTCOND( aFileOpenDialog, "invalid parameter" );

    Sequence< OUString > aFilePathList;
    OUString aFilePathURL;
    OUString aFilePath;
    ::osl::FileBase::RC rc;

    
    

    
    
    aFilePath = aFileOpenDialog->getCurrentFilePath( );

    
    
    
    
    rc = ::osl::FileBase::getFileURLFromSystemPath(
        aFilePath, aFilePathURL );

    if ( ::osl::FileBase::E_None == rc )
    {
        aFilePathList.realloc( 1 );
        aFilePathList[0] = aFilePathURL;
    }

    return aFilePathList;
}


//


OUString SAL_CALL CExecuteFilePickerState::getDisplayDirectory( CFileOpenDialog* aFileOpenDialog )
{
    OSL_POSTCOND( aFileOpenDialog, "invalid parameter" );

    OUString pathURL;
    OUString displayDir;

    displayDir = aFileOpenDialog->getCurrentFolderPath( );

    if ( displayDir.getLength( ) )
        ::osl::FileBase::getFileURLFromSystemPath( displayDir, pathURL );

    return pathURL;
}


//


void SAL_CALL CExecuteFilePickerState::initFilePickerControls( CControlCommand* firstControlCommand )
{
    CControlCommand* aControlCommand = firstControlCommand;

    while ( aControlCommand )
    {
        aControlCommand->exec( this );
        aControlCommand = aControlCommand->getNextCommand( );
    }
}


//


void SAL_CALL CExecuteFilePickerState::cacheControlState( HWND hwndControl, CFilePickerState* aNonExecFilePickerState )
{
    OSL_ENSURE( hwndControl && aNonExecFilePickerState, "invalid parameters" );

    CTRL_CLASS aCtrlClass = GetCtrlClass( hwndControl );

    sal_Int16 aControlAction;
    CTRL_GETVALUE_FUNCTION_T lpfnGetValue;

    if ( CHECKBOX == aCtrlClass )
    {
        aControlAction = 0;

        lpfnGetValue = GetCtrlGetValueFunction( aCtrlClass, aControlAction );

        OSL_ASSERT( lpfnGetValue );

        aNonExecFilePickerState->setValue(
            sal::static_int_cast< sal_Int16 >( GetDlgCtrlID( hwndControl ) ),
            aControlAction,
            lpfnGetValue( hwndControl ) );

        aNonExecFilePickerState->setLabel(
            sal::static_int_cast< sal_Int16 >( GetDlgCtrlID( hwndControl ) ),
            getLabel(
                sal::static_int_cast< sal_Int16 >(
                    GetDlgCtrlID( hwndControl ) ) ) );
    }
    else if ( LISTBOX == aCtrlClass )
    {
        
        
        

        aControlAction = GET_SELECTED_ITEM;

        lpfnGetValue = GetCtrlGetValueFunction( aCtrlClass, aControlAction );

        aNonExecFilePickerState->setValue(
            sal::static_int_cast< sal_Int16 >( GetDlgCtrlID( hwndControl ) ),
            aControlAction,
            lpfnGetValue( hwndControl ) );

        aControlAction = ::com::sun::star::ui::dialogs::ControlActions::GET_SELECTED_ITEM_INDEX;

        lpfnGetValue = GetCtrlGetValueFunction( aCtrlClass, aControlAction );

        aNonExecFilePickerState->setValue(
            sal::static_int_cast< sal_Int16 >( GetDlgCtrlID( hwndControl ) ),
            aControlAction,
            lpfnGetValue( hwndControl ) );
    }
}


//


void SAL_CALL CExecuteFilePickerState::setHwnd( HWND hwndDlg )
{
    m_hwndDlg = hwndDlg;
}


//


inline sal_Bool SAL_CALL CExecuteFilePickerState::IsListboxControl( HWND hwndControl ) const
{
    OSL_PRECOND( IsWindow( hwndControl ), "invalid parameter" );

    CTRL_CLASS aCtrlClass = GetCtrlClass( hwndControl );
    return ( LISTBOX == aCtrlClass );
}









inline sal_Int16 SAL_CALL CExecuteFilePickerState::ListboxIdToListboxLabelId( sal_Int16 aListboxId ) const
{
    return ( aListboxId + LISTBOX_LABEL_OFFSET );
}


//


inline HWND SAL_CALL CExecuteFilePickerState::GetListboxLabelItem( sal_Int16 aControlId ) const
{
    sal_Int16 aLabelId = ListboxIdToListboxLabelId( aControlId );
    HWND hwndCtrl = GetHwndDlgItem( aLabelId );

    OSL_ASSERT( IsWindow( hwndCtrl ) );

    return hwndCtrl;
}


//


HWND SAL_CALL CExecuteFilePickerState::GetHwndDlgItem( sal_Int16 aControlId, sal_Bool bIncludeStdCtrls ) const
{
    OSL_ENSURE( IsWindow( m_hwndDlg ), "no valid parent window set before" );

    HWND hwndCtrl = GetDlgItem( m_hwndDlg, aControlId );

    
    
    if ( !hwndCtrl && bIncludeStdCtrls )
    {
        hwndCtrl = GetDlgItem(
            GetParent( m_hwndDlg ),
            CommonFilePickerCtrlIdToWinFileOpenCtrlId( aControlId ) );
    }

    return hwndCtrl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
