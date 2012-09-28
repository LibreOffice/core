/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import java.util.*;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.accessibility.*;
import com.sun.star.awt.XExtendedToolkit;


/** This timer task tries to connect to a running Office application in regular
    intervals until a connection can be successfully established.
*/
class ConnectionTask
    extends TimerTask
{
    public ConnectionTask (EventListenerProxy xListener)
    {
        Init (xListener);
    }

    private void Init (EventListenerProxy xListener)
    {
        mxListener = xListener;
        mbInitialized = false;

        maTimer = new java.util.Timer ();
        maTimer.schedule (this, 0, mnPeriod);
    }


    /** This method is run every time the task is executed.  It tries to
        connect to and register the listener at an Office application.  If it
        can establish a connection it terminates the timer task.  Otherwise it
        waits until the next activation.
    */
    public void run ()
    {
        if (registerListeners())
        {
            // Focus listener was successfully registered so we can cancel this task.
            MessageArea.println ("\nconnected successfully to office");
            cancel ();
            maTimer = null;
        }
    }




    /** Try to register the listener.
    */
    private boolean registerListeners ()
    {
        // Get toolkit.
        XExtendedToolkit xToolkit = getToolkit ();

        // Register at toolkit as focus event listener.
        if (xToolkit != null)
        {
            xToolkit.addTopWindowListener (mxListener);
            int nTopWindowCount = xToolkit.getTopWindowCount();
            try
            {
                com.sun.star.lang.EventObject aEvent = new com.sun.star.lang.EventObject();
                for (int i=0; i<nTopWindowCount; i++)
                {
                    XAccessible xAccessible = (XAccessible) UnoRuntime.queryInterface(
                        XAccessible.class,
                        xToolkit.getTopWindow(i));
                    XAccessibleContext xContext = xAccessible.getAccessibleContext();
                    if (xContext.getAccessibleName().length() > 0)
                    {
                        // Simulate an event that leads to registration the
                        // listener at the window.
                        aEvent.Source = xToolkit.getTopWindow(i);
                        mxListener.windowOpened (aEvent);
                    }
                }
            }

            catch (com.sun.star.lang.IndexOutOfBoundsException aException)
            {
                // This exception signals that the number of top windows has
                // changed since our last call to getTopWindowCount().
            }
            return true;
        }
        else
            return false;
    }




    /** Get the toolkit from an Office which can then be used to register
        the listener.
    */
    private XExtendedToolkit getToolkit ()
    {
        XMultiServiceFactory xFactory = connectToOffice();

        // Get toolkit.
        XExtendedToolkit xToolkit = null;
        try
        {
            if (xFactory != null)
            {
                xToolkit = (XExtendedToolkit) UnoRuntime.queryInterface(
                    XExtendedToolkit.class,
                    xFactory.createInstance ("stardiv.Toolkit.VCLXToolkit"));
            }
        }
        catch (com.sun.star.uno.Exception aException)
        {
            MessageArea.println ("caught exception while creating extended toolkit");
            // Ignored.
        }

        return xToolkit;
    }




    /** Connect to a running (Star|Open)Office application
    */
    private XMultiServiceFactory connectToOffice ()
    {
        // connect to a running office and get the ServiceManager
        try
        {
            com.sun.star.uno.XComponentContext xCmpContext = null;

            // get the remote office component context
            xCmpContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if( xCmpContext != null )
                System.out.println("Connected to a running office ...");

            // get the remote office service manager
            com.sun.star.lang.XMultiComponentFactory xMCF =
                xCmpContext.getServiceManager();

            return (XMultiServiceFactory) UnoRuntime.queryInterface (
                XMultiServiceFactory.class, xMCF);
        }

        catch (Exception e)
        {
            if ( ! mbInitialized)
                MessageArea.println ("Could not connect to office");
            else
                MessageArea.print (".");
        }
        mbInitialized = true;
        return null;
    }

    /** Time in milliseconds between two attempts to connect to an Office
         application.
    */
    private int mnPeriod = 1000;

    private EventListenerProxy mxListener;
    private boolean mbInitialized;

    /** This timer is used for the registration loop to retry to connect to
        the Office until a connection can be established.
    */
    private java.util.Timer maTimer;
}
