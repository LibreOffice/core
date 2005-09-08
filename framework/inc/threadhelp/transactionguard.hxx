/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transactionguard.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:38:26 $
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

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
#define __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_INONCOPYABLE_H_
#include <threadhelp/inoncopyable.h>
#endif

#ifndef __FRAMEWORK_THREADHELP_ITRANSACTIONMANAGER_H_
#include <threadhelp/itransactionmanager.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          implement a guard to support non breakable transactions
    @descr          If you whish to support non breakable method calls without lockingf any mutex, rw-lock or
                    something like that - you should use this guard implementation.
                    Initialize it at first in your method and don't release it till end of your function!
                    Your "transaction" is registered in ctor and automaticly released in dtor.
                    Use set/get of working mode to enable/disable further transactions.
                    It's possible too, to enable automaticly throwing of some exceptions for illegal
                    transaction requests ... e.g. interface call for already disposed objects.

    @attention      To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!

    @implements     -
    @base           INonCopyable

    @devstatus      draft
*//*-*************************************************************************************************************/
class TransactionGuard : private INonCopyable
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @short      ctors
            @descr      Use these ctor methods to initialize the guard right.
                        Given reference must be valid - otherwise crashes could occure!

            @attention  It's not neccessary to lock any mutex here! Because a ctor should not be called
                        from different threads at the same time ... this class use no refcount mechanism!

            @seealso    -

            @param      "rManager"  reference to transaction manager for using to register a request
            @param      "eMode"     enable/disable throwing of exceptions for rejected calls
            @param      "eReason"   returns reason for rejected calls if "eMode=E_NOEXCEPTIONS"!
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline TransactionGuard( ITransactionManager& rManager, EExceptionMode eMode, ERejectReason* eReason = NULL )
            : m_pManager( &rManager )
        {
            // If exception mode is set to E_HARDEXCETIONS we don't need a buffer to return reason!
            // We handle it private. If a call is rejected, our manager throw some exceptions ... and the reason
            // could be ignorable ...
            if( eReason == NULL )
            {
                ERejectReason eMyReason;
                m_pManager->registerTransaction( eMode, eMyReason );
            }
            else
            {
                m_pManager->registerTransaction( eMode, *eReason );
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
        inline ~TransactionGuard()
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
        inline void stop()
        {
            if( m_pManager != NULL )
            {
                m_pManager->unregisterTransaction();
                m_pManager = NULL;
            }
        }

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        /*-****************************************************************************************************//**
            @short      disable using of these functions!
            @descr      It's not allowed to use this methods. Different problem can occure otherwise.
                        Thats why we disable it by make it private.

            @seealso    other ctor

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        TransactionGuard();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:

        ITransactionManager*   m_pManager   ;   /// pointer to safed transaction manager

};      //  class TransactionGuard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
