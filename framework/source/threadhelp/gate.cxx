/*************************************************************************
 *
 *  $RCSfile: gate.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:16 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_GATE_HXX_
#include <threadhelp/gate.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

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
