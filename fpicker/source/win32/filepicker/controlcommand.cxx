/*************************************************************************
 *
 *  $RCSfile: controlcommand.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-08-10 12:08:54 $
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

CControlCommandResult* SAL_CALL CControlCommand::handleRequest( CControlCommandRequest* aRequest )
{
    return new CControlCommandResult( );
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
    else if ( (nextCommand = getNextCommand( )) )
    {
        result = nextCommand->handleRequest( aRequest );
    }
    else
    {
        result = new CControlCommandResult( );
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
    else if ( (nextCommand = getNextCommand( )) )
    {
        result = nextCommand->handleRequest( aRequest );
    }
    else
    {
        result = new CControlCommandResult( );
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
