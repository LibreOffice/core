/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itransactionmanager.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:36:59 $
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

#ifndef __FRAMEWORK_THREADHELP_ITRANSACTIONMANAGER_H_
#define __FRAMEWORK_THREADHELP_ITRANSACTIONMANAGER_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          Describe different states of a feature of following implementation.
                    During live time of an object different working states occure:
                        initialization - working - closing - closed
                    If you whish to implement thread safe classes you should use these feature to protect
                    your code against calls at wrong time. e.g. you are not full initialized but somewhere
                    call an interface method (initialize phase means startup time from creating object till
                    calling specified first method e.g. XInitialization::initialze()!) then you should refuse
                    this call. The same for closing/disposing the object!
*//*-*************************************************************************************************************/
enum EWorkingMode
{
    E_INIT       ,   // We stand in a init method   -> some calls are accepted - some one are rejected
    E_WORK       ,   // Object is ready for working -> all calls are accepted
    E_BEFORECLOSE,   // We stand in a close method  -> some calls are accepted - some one are rejected
    E_CLOSE          // Object is dead!             -> all calls are rejected!
};

/*-************************************************************************************************************//**
    @descr          If a request was refused by a transaction manager (internal state different E_WORK ...)
                    user can check the reason by using this enum values.
*//*-*************************************************************************************************************/
enum ERejectReason
{
    E_UNINITIALIZED ,
    E_NOREASON      ,
    E_INCLOSE       ,
    E_CLOSED
};

/*-************************************************************************************************************//**
    @descr          A transaction object should support throwing exceptions if user used it at wrong working mode.
                    e.g. We can throw a DisposedException if user try to work and our mode is E_CLOSE!
                    But sometimes he dont need this feature - will handle it by himself.
                    Then we must differ between some exception-modi:
                        E_NOEXCEPTIONS          We never throw any exceptions! User handle it private and looks for ERejectReason.
                        E_HARDEXCEPTIONS        We throw exceptions for all working modes different from E_WORK!
                        E_SOFTEXCEPTIONS        We throw exceptions for all working modes different from E_WORK AND E_INCLOSE!
                                                This mode is useful for impl-methods which should be callable from dispose() method!

                                                e.g.    void dispose()
                                                        {
                                                            m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );
                                                            ...
                                                            impl_setA( 0 );
                                                            ...
                                                            m_aTransactionManager.setWorkingMode( E_CLOSE );
                                                        }

                                                        void impl_setA( int nA )
                                                        {
                                                            ERejectReason       EReason;
                                                            TransactionGuard    aTransactionGuard( m_aTransactionManager, E_SOFTEXCEPTIONS, eReason );

                                                            m_nA = nA;
                                                        }

                                                Normaly (if E_HARDEXCEPTIONS was used!) creation of guard
                                                will throw an exception ... but using of E_SOFTEXCEPTIONS suppress it
                                                and member "A" can be set.
*//*-*************************************************************************************************************/
enum EExceptionMode
{
    E_NOEXCEPTIONS  ,
    E_HARDEXCEPTIONS,
    E_SOFTEXCEPTIONS
};

/*-************************************************************************************************************//**
    @descr          How can you use the transaction manager?
                    Use it in combination with an TransactionGuard, which register your transaction in ctor
                    and release in dtor automaticly! Follow interface class can be used to make using
                    of different manager implmentations possible by using same guard.
*//*-*************************************************************************************************************/
class ITransactionManager
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @descr      These functions must be supported by a derived class!
                            getWorkingMode()        -return current set working mode
                            setWorkingMode()        -change working mode
                                                     (This will block till all current transactions are finished!)
                            isCallRejected()        -test method to check if a call will be rejected by wrong working mode or not
                            registerTransaction()   -start new transaction (increase internal transaction count)
                            unregisterTransaction() -finish transaction    (decrease internal transaction count)
        *//*-*****************************************************************************************************/
        virtual EWorkingMode getWorkingMode         (                                                ) const = 0;
        virtual void         setWorkingMode         ( EWorkingMode   eMode                           )       = 0;
        virtual sal_Bool     isCallRejected         ( ERejectReason& eReason                         ) const = 0;
        virtual void         registerTransaction    ( EExceptionMode eMode  , ERejectReason& eReason ) throw( css::uno::RuntimeException, css::lang::DisposedException ) = 0;
        virtual void         unregisterTransaction  (                                                ) throw( css::uno::RuntimeException, css::lang::DisposedException ) = 0;

};      //  class ITransactionManager

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_ITRANSACTIONMANAGER_H_
