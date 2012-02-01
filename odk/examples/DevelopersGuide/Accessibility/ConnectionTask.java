/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



import java.awt.event.ActionListener;
import javax.swing.*;
import java.awt.*;
import java.util.*;

import com.sun.star.awt.XFocusListener;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.bridge.XUnoUrlResolver;
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
