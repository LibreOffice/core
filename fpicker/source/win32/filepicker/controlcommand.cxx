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

#include "controlcommand.hxx"
#include "controlcommandrequest.hxx"
#include "controlcommandresult.hxx"
#include "filepickerstate.hxx"

//---------------------------------------------
//
//---------------------------------------------

CControlCommand::CControlCommand( sal_Int16 aControlId ) :
    m_NextCommand( NULL ),
    m_aControlId( aControlId )
{
}

//---------------------------------------------
//
//---------------------------------------------

CControlCommand::~CControlCommand( )
{
}

//---------------------------------------------
//
//---------------------------------------------

CControlCommandResult* SAL_CALL CControlCommand::handleRequest( CControlCommandRequest* pRequest )
{
    // if the command does not support handleRequest, it should at least
    // redirect the request to the next element
    // so the base class implementation has to do it

    OSL_ENSURE( pRequest, "inavlid parameter" );

    CControlCommandResult* result;
    CControlCommand* nextCommand;

    nextCommand = getNextCommand( );
    if ( nextCommand )
    {
        result = nextCommand->handleRequest( pRequest );
    }
    else
    {
        result = new CControlCommandResult();
    }

    return result;
}

//---------------------------------------------
//
//---------------------------------------------

CControlCommand* SAL_CALL CControlCommand::getNextCommand( ) const
{
    return m_NextCommand;
}

//---------------------------------------------
//
//---------------------------------------------

void SAL_CALL CControlCommand::setNextCommand( CControlCommand* nextCommand )
{
    m_NextCommand = nextCommand;
}

//---------------------------------------------
//
//---------------------------------------------

sal_Int16 SAL_CALL CControlCommand::getControlId( ) const
{
    return m_aControlId;
}


//---------------------------------------------
//
//---------------------------------------------

CValueControlCommand::CValueControlCommand(
        sal_Int16 aControlId,
        sal_Int16 aControlAction,
        const ::com::sun::star::uno::Any& aValue ) :
    CControlCommand( aControlId ),
    m_aControlAction( aControlAction ),
    m_aValue( aValue )
{
}

//---------------------------------------------
//
//---------------------------------------------

void SAL_CALL CValueControlCommand::exec( CFilePickerState* aFilePickerState )
{
    OSL_ENSURE( aFilePickerState, "empty reference" );

    aFilePickerState->setValue(
        getControlId( ),
        m_aControlAction,
        m_aValue );
}

//---------------------------------------------
//
//---------------------------------------------

CControlCommandResult* SAL_CALL CValueControlCommand::handleRequest( CControlCommandRequest* aRequest )
{
    CValueControlCommandRequest* value_request =
        dynamic_cast< CValueControlCommandRequest* >( aRequest );

    CControlCommandResult* result;
    CControlCommand* nextCommand;

    if ( value_request &&
         (value_request->getControlId( ) == getControlId( )) &&
         (value_request->getControlAction( ) == m_aControlAction) )
    {
        result = new CValueCommandResult( sal_True, m_aValue );
    }
    else
    {
        nextCommand = getNextCommand( );
        if ( nextCommand )
        {
            result = nextCommand->handleRequest( aRequest );
        }
        else
        {
            result = new CControlCommandResult( );
        }
    }

    return result;
}

//---------------------------------------------
//
//---------------------------------------------

sal_Int16 SAL_CALL CValueControlCommand::getControlAction( ) const
{
    return m_aControlAction;
}

//---------------------------------------------
//
//---------------------------------------------

::com::sun::star::uno::Any SAL_CALL CValueControlCommand::getValue( ) const
{
    return m_aValue;
}


//---------------------------------------------
//
//---------------------------------------------

CLabelControlCommand::CLabelControlCommand(
        sal_Int16 aControlId,
        const rtl::OUString& aLabel ) :
    CControlCommand( aControlId ),
    m_aLabel( aLabel )
{
}

//---------------------------------------------
//
//---------------------------------------------

void SAL_CALL CLabelControlCommand::exec( CFilePickerState* aFilePickerState )
{
    OSL_ENSURE( aFilePickerState, "empty reference" );

    aFilePickerState->setLabel( getControlId( ), m_aLabel );
}

//---------------------------------------------
//
//---------------------------------------------

CControlCommandResult* SAL_CALL CLabelControlCommand::handleRequest( CControlCommandRequest* aRequest )
{
    OSL_ENSURE( aRequest, "inavlid parameter" );

    CControlCommandResult* result;
    CControlCommand* nextCommand;

    CValueControlCommandRequest* value_request =
        dynamic_cast< CValueControlCommandRequest* >( aRequest );

    if ( !value_request &&
         (aRequest->getControlId( ) == getControlId( )) )
    {
        result = new CLabelCommandResult( sal_True, m_aLabel );
    }
    else
    {
        nextCommand = getNextCommand( );
        if ( nextCommand )
        {
            result = nextCommand->handleRequest( aRequest );
        }
        else
        {
            result = new CControlCommandResult( );
        }
    }

    return result;
}

//---------------------------------------------
//
//---------------------------------------------

rtl::OUString SAL_CALL CLabelControlCommand::getLabel( ) const
{
    return m_aLabel;
}

//---------------------------------------------
//
//---------------------------------------------

CEnableControlCommand::CEnableControlCommand(
        sal_Int16 aControlId,
        sal_Bool bEnable ) :
    CControlCommand( aControlId ),
    m_bEnable( bEnable )
{
}

//---------------------------------------------
//
//---------------------------------------------

void SAL_CALL CEnableControlCommand::exec( CFilePickerState* aFilePickerState )
{
    OSL_ENSURE( aFilePickerState, "empty reference" );

    aFilePickerState->enableControl( getControlId( ), m_bEnable );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
