/*************************************************************************
 *
 *  $RCSfile: transactionguard.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-05-02 13:00:51 $
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

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
#include <threadhelp/transactionguard.hxx>
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

/*-************************************************************************************************************//**
    @short      ctors
    @descr      Use these ctor methods to initialize the guard right.
                Given reference must be valid - otherwise crashes could occure!

    @attention  It's not neccessary to lock any mutex here! Because a ctor should not be called
                from different threads at the same time ... this class use no refcount mechanism!

    @seealso    -

    @param      "pManager"  pointer to transaction manager for using to register a request
    @param      "rManager"  same as reference
    @param      "eMode"     enable/disable throwing of exceptions for rejected calls
    @param      "eReason"   returns reason for rejected calls if "eMode=E_NOEXCEPTIONS"!
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
TransactionGuard::TransactionGuard( ITransactionManager* pManager, EExceptionMode eMode, ERejectReason* eReason )
    : m_pManager( pManager )
{
    // If exception mode is set to E_HARDEXCETIONS we don't need a buffer to return reason!
    // We handle it private. If a call is rejected, our manager throw some exceptions ... and the reason
    // could be ignorable ...
    if( eReason == NULL )
    {
        ERejectReason eMyReason;
        m_pManager->acquire( eMode, eMyReason );
    }
    else
    {
        m_pManager->acquire( eMode, *eReason );
    }
}

//*****************************************************************************************************************
TransactionGuard::TransactionGuard( ITransactionManager& rManager, EExceptionMode eMode, ERejectReason* eReason )
    : m_pManager( &rManager )
{
    // If exception mode is set to E_HARDEXCETIONS we don't need a buffer to return reason!
    // We handle it private. If a call is rejected, our manager throw some exceptions ... and the reason
    // could be ignorable ...
    if( eReason == NULL )
    {
        ERejectReason eMyReason;
        m_pManager->acquire( eMode, eMyReason );
    }
    else
    {
        m_pManager->acquire( eMode, *eReason );
    }
}

/*-************************************************************************************************************//**
    @short      dtor
    @descr      We must release the transaction manager and can forget his pointer.

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
TransactionGuard::~TransactionGuard()
{
    stop();
}

/*-************************************************************************************************************//**
    @short      stop current transaction
    @descr      We must release the transaction manager and can forget his pointer.

    @attention  We don't support any start() method here - because it is not easy to
                detect if a transaction already started or not!
                (combination of EExceptionMode and ERejectReason)

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void TransactionGuard::stop()
{
    if( m_pManager != NULL )
    {
        m_pManager->release();
        m_pManager = NULL;
    }
}

}   //  namespace framework
