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

package integration.extensions;

import com.sun.star.uno.*;
import com.sun.star.lang.XComponent;

public class ConsoleWait implements com.sun.star.lang.XEventListener
{
    private Object  m_disposable;

    /** a helper class which waits for a console ENTER key event in a dedicated thread,
        and notifies a ConsoleWait object if this event happened
     */
    private class WaitForEnter extends java.lang.Thread
    {
        private ConsoleWait m_toNotify;
        private boolean     m_done;

        public WaitForEnter( ConsoleWait _toNotify )
        {
            m_toNotify = _toNotify;
            m_done = false;
        }

        public boolean isDone()
        {
            return m_done;
        }

        public void run()
        {
            try
            {
                System.out.println( "\npress enter to exit" );
                System.in.read();

                m_done = true;
                // notify that the user pressed the key
                synchronized ( m_toNotify )
                {
                    m_toNotify.notify();
                }
            }
            catch( java.lang.Exception e )
            {
                // not really interested in
                System.err.println( e );
            }
        }
    };

    /** creates a ConsoleWait instance
     *  @param _disposable
     *      a component whose disposal should be monitored. When this component dies,
     *      the ConsoleWait also returns from an waitForConsole call, even if the user
     *      did not yet press the enter key
     */
    public ConsoleWait( Object _disposable )
    {
        m_disposable = _disposable;
        XComponent component = UnoRuntime.queryInterface( XComponent.class, _disposable );
        if ( component != null )
            component.addEventListener( this );
    }

    /** waits for the user to press the ENTER key (on the console where she started the java program)
        or the disposable component to be closed by the user.
        @return
            TRUE if the user pressed a key on the console, FALSE if she closed the document
    */
    public boolean waitForUserInput() throws java.lang.Exception
    {
        synchronized (this)
        {
            WaitForEnter keyWaiter = new WaitForEnter( this );
            keyWaiter.start();
            wait();

            // if the waiter thread is done, the user pressed enter
            boolean bKeyPressed = keyWaiter.isDone();
            if ( !bKeyPressed )
                keyWaiter.interrupt();

            return bKeyPressed;
        }
    }

    /* ------------------------------------------------------------------ */
    /* XEventListener overridables                                        */
    /* ------------------------------------------------------------------ */
    public void disposing( com.sun.star.lang.EventObject eventObject )
    {
        if ( eventObject.Source.equals( m_disposable ) )
        {
            // notify ourself that we can stop waiting for user input
            synchronized (this)
            {
                notify();
            }
        }
    }
}