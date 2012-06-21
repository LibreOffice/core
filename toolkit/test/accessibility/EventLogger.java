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
