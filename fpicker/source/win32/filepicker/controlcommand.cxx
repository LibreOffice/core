/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlcommand.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-11 10:31:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _CONTROLCOMMAND_HXX_
#include "controlcommand.hxx"
#endif

#ifndef _CONTROLCOMMANDREQUEST_HXX_
#include "controlcommandrequest.hxx"
#endif

#ifndef _CONTROLCOMMANDRESULT_HXX_
#include "controlcommandresult.hxx"
#endif

#ifndef _FILEPICKERSTATE_HXX_
#include "filepickerstate.hxx"
#endif

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
