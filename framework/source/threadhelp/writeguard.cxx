/*************************************************************************
 *
 *  $RCSfile: writeguard.cxx,v $
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

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
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
WriteGuard::WriteGuard( IRWLock*        pLock   ,
                        ERefusalReason& eReason )
    :   m_pLock ( pLock     )
    ,   m_eMode ( E_NOLOCK  )
{
    lock( eReason );
}

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
WriteGuard::WriteGuard( IRWLock&        rLock   ,
                        ERefusalReason& eReason )
    :   m_pLock ( &rLock    )
    ,   m_eMode ( E_NOLOCK  )
{
    lock( eReason );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
WriteGuard::~WriteGuard()
{
    unlock();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void WriteGuard::lock( ERefusalReason& eReason )
{
    switch( m_eMode )
    {
        case E_NOLOCK       :   {
                                    // Acquire write access and set return state.
                                    // Mode is set later if it was successful!
                                    m_pLock->acquireWriteAccess( eReason );
                                    if( eReason == E_NOREASON )
                                    {
                                        m_eMode = E_WRITELOCK;
                                    }
                                }
                                break;
        case E_READLOCK     :   {
                                    // User has downgrade to read access before!
                                    // We must release it before we can set a new write access!
                                    m_pLock->releaseReadAccess();
                                    m_pLock->acquireWriteAccess( eReason );
                                    if( eReason == E_NOREASON )
                                    {
                                        m_eMode = E_WRITELOCK;
                                    }
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void WriteGuard::unlock()
{
    switch( m_eMode )
    {
        case E_READLOCK     :   {
                                    // Use has downgraded to a read lock!
                                    m_pLock->releaseReadAccess();
                                    m_eMode = E_NOLOCK;
                                }
                                break;
        case E_WRITELOCK    :   {
                                    m_pLock->releaseWriteAccess();
                                    m_eMode = E_NOLOCK;
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void WriteGuard::downgrade()
{
    if( m_eMode == E_WRITELOCK )
    {
        m_pLock->downgradeWriteAccess();
        m_eMode = E_READLOCK;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
ELockMode WriteGuard::getMode() const
{
    return m_eMode;
}

}   //  namespace framework
