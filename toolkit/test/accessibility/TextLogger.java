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

import javax.swing.JEditorPane;
import javax.swing.event.HyperlinkListener;
import javax.swing.event.HyperlinkEvent;
import javax.swing.text.Document;
import java.net.URL;

class TextLogger
    extends JEditorPane
{
    public TextLogger ()
        throws java.io.IOException
    {
//        maDocument = getEditorKit().createDefaultDocument();
            super ("http://localhost");
        try
        {
            //            setPage (new URL ("http://www.spiegel.de"));
        }
        catch (Exception e)
        {}

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
            maDocument.insertString (maDocument.getLength(), sText, null);
        }
        catch (javax.swing.text.BadLocationException e)
        {}
    }

    private Document maDocument;
}
