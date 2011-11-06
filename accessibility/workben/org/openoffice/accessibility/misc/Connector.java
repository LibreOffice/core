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
