/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConsoleWait.java,v $
 * $Revision: 1.3 $
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
        XComponent component = (XComponent)UnoRuntime.queryInterface( XComponent.class, _disposable );
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