package org.openoffice.accessibility.misc;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Vector;


/** Wait for an Office application and connect to it.
*/
public class Connector
    extends TimerTask
{
    final public static long snDelay = 3000;

    public Connector ()
    {
        maTimer = new Timer (true);
        maListeners = new Vector();
        run ();
    }

    public void AddConnectionListener (ActionListener aListener)
    {
        SimpleOffice aOffice = SimpleOffice.Instance();
        if (aOffice!=null && aOffice.IsConnected())
            aListener.actionPerformed (
                new ActionEvent (aOffice,0,"<connected>"));
        maListeners.add (aListener);
    }

    public void run ()
    {
        SimpleOffice aOffice = SimpleOffice.Instance();
        if (aOffice!=null && !aOffice.IsConnected())
            if ( ! aOffice.Connect())
                 maTimer.schedule (this, snDelay);
            else
            {
                ActionEvent aEvent = new ActionEvent (aOffice,0,"<connected>");
                for (int i=0; i<maListeners.size(); i++)
                    ((ActionListener)maListeners.elementAt(i)).actionPerformed(
                        aEvent);
            }
    }

    Timer maTimer;
    Vector maListeners;
}
