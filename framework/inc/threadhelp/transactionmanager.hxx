/*************************************************************************
 *
 *  $RCSfile: transactionmanager.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: as $ $Date: 2002-05-02 11:41:09 $
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

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_
#define __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_INONCOPYABLE_H_
#include <threadhelp/inoncopyable.h>
#endif

#ifndef __FRAMEWORK_THREADHELP_ITRANSACTIONMANAGER_H_
#include <threadhelp/itransactionmanager.h>
#endif

#ifndef __FRAMEWORK_THREADHELP_GATE_HXX_
#include <threadhelp/gate.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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
    @short          implement a transaction manager to support non breakable interface methods
    @descr          Use it to support non breakable interface methods without using any thread
                    synchronization like e.g. mutex, rw-lock!
                    That protect your code against wrong calls at wrong time ... e.g. calls after disposing an object!
                    Use combination of EExceptionMode and ERejectReason to detect rejected requests
                    and react for it. You can enable automaticly throwing of exceptions too.

    @implements     ITransactionManager
    @base           INonCopyable
                    ITransactionManager

    @devstatus      draft
*//*-*************************************************************************************************************/
class TransactionManager : public  ITransactionManager
                         , private INonCopyable
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

                                   TransactionManager           (                                              );
        virtual void               setWorkingMode               ( EWorkingMode eMode                           );
        virtual EWorkingMode       getWorkingMode               (                                              ) const;
        virtual sal_Bool           isCallRejected               ( ERejectReason& eReason                       ) const;
        virtual void               registerTransaction          ( EExceptionMode eMode, ERejectReason& eReason ) throw( css::uno::RuntimeException, css::lang::DisposedException );
        virtual void               unregisterTransaction        (                                              ) throw( css::uno::RuntimeException, css::lang::DisposedException );
        static TransactionManager& getGlobalTransactionManager  (                                              );

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        void impl_throwExceptions( EExceptionMode eMode, ERejectReason eReason ) const throw( css::uno::RuntimeException, css::lang::DisposedException );

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:

        mutable ::osl::Mutex    m_aAccessLock           ;   /// regulate access on internal member of this instance
        Gate                    m_aBarrier              ;   /// used to block transactions requests during change or work mode
        EWorkingMode            m_eWorkingMode          ;   /// current working mode of object which use this manager (used to reject calls at wrong time)
        sal_Int32               m_nTransactionCount     ;   /// every transaction request is registered by this counter

};      //  class TransactionManager

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_
