package org.openoffice.accessibility.misc;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.PrintStream;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Vector;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.comp.helper.Bootstrap;

/** This class establishes a connection to a StarOffice application.
 */
public class OfficeConnection
    extends TimerTask
{
    final public static long snDelay = 3000;

    public static synchronized OfficeConnection Instance ()
    {
        if (saInstance == null)
            saInstance = new OfficeConnection ();
        return saInstance;
    }




    static public void SetPipeName (String sPipeName)
    {
        ssDefaultPipeName = sPipeName;
    }




    public void AddConnectionListener (ActionListener aListener)
    {
        SimpleOffice aOffice = SimpleOffice.Instance();
        if (IsValid())
            aListener.actionPerformed (
                new ActionEvent (aOffice,0,"<connected>"));
        maListeners.add (aListener);
    }



    /** @descr Return the service manager that represents the connected
                StarOffice application
    */
    public XMultiServiceFactory GetServiceManager ()
    {
        return maServiceManager;
    }




    /** Return a flag that indicates if the constructor has been able to
        establish a valid connection.
    */
    public boolean IsValid ()
    {
        return (maServiceManager != null);
    }




    /** Connect to a already running StarOffice application that has
        been started with a command line argument like
        "-accept=pipe,name=<username>;urp;"
    */
    private boolean Connect ()
    {
        mbInitialized = true;
        //  Set up connection string.
        String sConnectString = "uno:pipe,name=" + msPipeName
                + ";urp;StarOffice.ServiceManager";

        // connect to a running office and get the ServiceManager
        try
        {
            //  Create a URL Resolver.
            XMultiServiceFactory aLocalServiceManager =
                Bootstrap.createSimpleServiceManager();
            XUnoUrlResolver aURLResolver =
                (XUnoUrlResolver) UnoRuntime.queryInterface (
                    XUnoUrlResolver.class,
                    aLocalServiceManager.createInstance (
                        "com.sun.star.bridge.UnoUrlResolver")
                    );

            maServiceManager =
                (XMultiServiceFactory) UnoRuntime.queryInterface (
                    XMultiServiceFactory.class,
                    aURLResolver.resolve (sConnectString)
                    );
        }

        catch (Exception e)
        {
            if (maOut != null)
            {
                maOut.println ("Could not connect with "
                    + sConnectString + " : " + e);
                maOut.println ("Please start OpenOffice/StarOffice with "
                    + "\"-accept=pipe,name=" + msPipeName + ";urp;\"");
            }
        }

        return maServiceManager != null;
    }


    public void run ()
    {
        if ( ! IsValid())
        {
            MessageArea.println ("trying to connect");
            if (Connect())
            {
                // Stop the timer.
                cancel ();

                ActionEvent aEvent = new ActionEvent (this,0,"<connected>");
                for (int i=0; i<maListeners.size(); i++)
                    ((ActionListener)maListeners.elementAt(i)).actionPerformed(aEvent);
            }
        }
    }

    private OfficeConnection ()
    {
        this (null);
    }


    private OfficeConnection (PrintStream aOut)
    {
        msPipeName = ssDefaultPipeName;
        maOut = aOut;
        maListeners = new Vector();
        maServiceManager = null;

        maTimer = new Timer (true);
        maTimer.schedule (this, 0, snDelay);
    }


    private static OfficeConnection saInstance = null;
    private static String ssDefaultPipeName = System.getenv( "USER" );

    private XMultiServiceFactory maServiceManager;
    String msPipeName;

    /** A value of true just indicates that it has been tried to establish a connection,
        not that that has been successfull.
    */
    private boolean mbInitialized = false;

    /// Stream used to print messages.
    private PrintStream maOut;
    private Timer maTimer;
    private Vector maListeners;
}
