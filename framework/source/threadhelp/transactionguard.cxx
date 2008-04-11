/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: transactionguard.cxx,v $
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
#include <threadhelp/transactionguard.hxx>

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
