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

import javax.swing.JFrame;
import javax.swing.JScrollPane;

class EventLogger
{
    public static synchronized EventLogger Instance ()
    {
        if (maInstance == null)
            maInstance = new EventLogger();
        return maInstance;
    }

    private EventLogger ()
    {
        try
        {
            maFrame = new JFrame ();
            maLogger = new TextLogger ();
            maFrame.setContentPane (new JScrollPane (maLogger));

            maFrame.setSize (400,300);
            maFrame.setVisible (true);
        }
        catch (Exception e)
        {}
    }

    private static EventLogger maInstance = null;
    private JFrame maFrame;
    private TextLogger maLogger;
}
