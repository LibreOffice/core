/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gate.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/gate.hxx>
#include <threadhelp/resetableguard.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
Gate::Gate()
    :   m_bClosed   ( sal_False )
    ,   m_bGapOpen  ( sal_False )
{
    open();
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
Gate::~Gate()
{
    open();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void Gate::open()
{
    // We must safe access to our internal member!
    ResetableGuard aLock( m_aAccessLock );
    // Set condition -> wait don't block any longer -> gate is open
    m_aPassage.set();
    // Check if operation was successful!
    // Check returns false if condition isn't set => m_bClosed will be true then => we must return false; opening failed
    m_bClosed = ( m_aPassage.check() == sal_False );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void Gate::close()
{
    // We must safe access to our internal member!
    ResetableGuard aLock( m_aAccessLock );
    // Reset condition -> wait blocks now -> gate is closed
    m_aPassage.reset();
    // Check if operation was successful!
    // Check returns false if condition was reseted => m_bClosed will be true then => we can return true; closing ok
    m_bClosed = ( m_aPassage.check() == sal_False );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void Gate::openGap()
{
    // We must safe access to our internal member!
    ResetableGuard aLock( m_aAccessLock );
    // Open passage for current waiting threads.
    m_aPassage.set();
    // Check state of condition.
    // If condition is set check() returns true => m_bGapOpen will be true too => we can use it as return value.
    m_bGapOpen = ( m_aPassage.check() == sal_True );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool Gate::wait( const TimeValue* pTimeOut )
{
    // We must safe access to our internal member!
    ResetableGuard aLock( m_aAccessLock );
    // If gate not closed - caller can pass it.
    sal_Bool bSuccessful = sal_True;
    if( m_bClosed == sal_True )
    {
        // Otherwise first new thread must close an open gap!
        if( m_bGapOpen == sal_True )
        {
            m_bGapOpen = sal_False;
            m_aPassage.reset();
        }
        // Then we must release used access lock -
        // because next call will block ...
        // and if we hold the access lock nobody else can use this object without a dadlock!
        aLock.unlock();
        // Wait for opening gate ...
        bSuccessful = ( m_aPassage.wait( pTimeOut ) == osl_cond_result_ok );
    }

    return bSuccessful;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool Gate::isOpen() const
{
    // I think we never need the access lock for pure reading of this bool member ...
    return !m_bClosed;
}

}   //  namespace framework
