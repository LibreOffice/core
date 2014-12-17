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

import javax.swing.JEditorPane;
import javax.swing.event.HyperlinkListener;
import javax.swing.event.HyperlinkEvent;

class TextLogger
    extends JEditorPane
{
    public TextLogger ()
        throws java.io.IOException
    {
        super ("http://localhost");

        setEditable (false);
        final JEditorPane finalPane = this;
        addHyperlinkListener (new HyperlinkListener()
            {
                public void hyperlinkUpdate (HyperlinkEvent e)
                {
                    try
                    {
                        if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
                            finalPane.setPage (e.getURL());
                    }
                    catch (java.io.IOException ex)
                    {
                        ex.printStackTrace(System.err);
                    }
                }
            });
    }

    public void appendText (String sText)
    {
        try
        {
            getDocument().insertString (getDocument().getLength(), sText, null);
        }
        catch (javax.swing.text.BadLocationException e)
        {}
    }
}
