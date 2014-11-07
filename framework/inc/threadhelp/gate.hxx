/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_GATE_HXX
#define INCLUDED_FRAMEWORK_INC_THREADHELP_GATE_HXX

#include <threadhelp/igate.h>

#include <boost/noncopyable.hpp>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          implement a gate to block multiple threads at same time or unblock all
    @descr          A gate can be used as a negative-condition! You can open a "door" - wait() will not block ...
                    or you can close it - wait() blocks till open() is called again.
                    As a special feature you can open the gate a little bit by sing openGap().
                    Then all currently waiting threads are running immediately - but new ones are blocked!

    @attention      To prevent us against wrong using, the default ctor, copy ctor and the =operator are marked private!

    @implements     IGate
    @base           IGate

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class Gate : public  IGate
           , private boost::noncopyable
{

    //  public methods

    public:

        /*-****************************************************************************************************
            @short      ctor
            @descr      These initialize the object right as an open gate.
        *//*-*****************************************************************************************************/
        inline Gate()
            :   m_bClosed   ( false )
            ,   m_bGapOpen  ( false )
        {
            open();
        }

        /*-****************************************************************************************************
            @short      dtor
            @descr      Is user forget it - we open the gate ...
                        blocked threads can running ... but I don't know
                        if it's right - we are destroyed yet!?
        *//*-*****************************************************************************************************/
        inline virtual ~Gate()
        {
            open();
        }

        /*-****************************************************************************************************
            @interface  IGate
            @short      open the gate
            @descr      A wait() call will not block then.

            @seealso    method close()
        *//*-*****************************************************************************************************/
        virtual void open() SAL_OVERRIDE
        {
            // We must safe access to our internal member!
            ::osl::MutexGuard aLock( m_aAccessLock );
            // Set condition -> wait don't block any longer -> gate is open
            m_aPassage.set();
            // Check if operation was successful!
            // Check returns false if condition isn't set => m_bClosed will be true then => we must return false; opening failed
            m_bClosed = !m_aPassage.check();
        }

        /*-****************************************************************************************************
            @interface  IGate
            @short      close the gate
            @descr      A wait() call will block then.

            @seealso    method open()
        *//*-*****************************************************************************************************/
        virtual void close() SAL_OVERRIDE
        {
            // We must safe access to our internal member!
            ::osl::MutexGuard aLock( m_aAccessLock );
            // Reset condition -> wait blocks now -> gate is closed
            m_aPassage.reset();
            // Check if operation was successful!
            // Check returns false if condition was reseted => m_bClosed will be true then => we can return true; closing ok
            m_bClosed = !m_aPassage.check();
        }

        /*-****************************************************************************************************
            @interface  IGate
            @short      open gate for current waiting threads
            @descr      All current waiting threads stand in wait() at line "m_aPassage.wait()" ...
                        With this call you can open the passage for these waiting ones.
                        The "gap" is closed by any new thread which call wait() automatically!

            @seealso    method wait()
            @seealso    method open()
        *//*-*****************************************************************************************************/
        virtual void openGap() SAL_OVERRIDE
        {
            // We must safe access to our internal member!
            ::osl::MutexGuard aLock( m_aAccessLock );
            // Open passage for current waiting threads.
            m_aPassage.set();
            // Check state of condition.
            // If condition is set check() returns true => m_bGapOpen will be true too => we can use it as return value.
            m_bGapOpen = m_aPassage.check();
        }

        /*-****************************************************************************************************
            @interface  IGate
            @short      must be called to pass the gate
            @descr      If gate "open"   => wait() will not block.
                        If gate "closed" => wait() will block till somewhere open it again.
                        If gap  "open"   => currently waiting threads unblocked, new ones blocked

            @seealso    method wait()
            @seealso    method open()

            @param      "pTimeOut", optional parameter to wait a certain time
            @return     true, if wait was successful (gate was opened)
                        false, if condition has an error or timeout was reached!

            @onerror    We return false.
        *//*-*****************************************************************************************************/
        virtual bool wait( const TimeValue* pTimeOut = NULL ) SAL_OVERRIDE
        {
            // We must safe access to our internal member!
            ::osl::ClearableMutexGuard aLock( m_aAccessLock );
            // If gate not closed - caller can pass it.
            bool bSuccessful = true;
            if( m_bClosed )
            {
                // Otherwise first new thread must close an open gap!
                if( m_bGapOpen )
                {
                    m_bGapOpen = false;
                    m_aPassage.reset();
                }
                // Then we must release used access lock -
                // because next call will block ...
                // and if we hold the access lock nobody else can use this object without a dadlock!
                aLock.clear();
                // Wait for opening gate ...
                bSuccessful = ( m_aPassage.wait( pTimeOut ) == ::osl::Condition::result_ok );
            }

            return bSuccessful;
        }

    //  private member

    private:

        ::osl::Mutex        m_aAccessLock;
        ::osl::Condition    m_aPassage;
        bool                m_bClosed;
        bool                m_bGapOpen;

};      //  class Gate

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_GATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
