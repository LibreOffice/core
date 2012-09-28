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

import java.awt.Font;
import java.awt.Color;
import java.awt.Graphics;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JScrollBar;



/** A message area displays text in a scrollable text widget.  It is a
    singleton.  Other objects can access it directly to display messages.
*/
public class MessageArea
    extends JScrollPane
{
    public static synchronized MessageArea Instance ()
    {
        if (saInstance == null)
            saInstance = new MessageArea ();
        return saInstance;
    }




    /** Create a new message area.  This method is private because the class is
        a singleton and may therefore not be instanciated from the outside.
    */
    private MessageArea ()
    {
        maText = new JTextArea();
        maText.setBackground (new Color (255,250,240));
        maText.setFont (new Font ("Helvetica", Font.PLAIN, 9));
        setViewportView (maText);
        setVerticalScrollBarPolicy (JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        setHorizontalScrollBarPolicy (JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);

        printMessage (
            "class path is " + System.getProperty ("java.class.path") + "\n");
    }




    /** Show the given string at the end of the message area and scroll to make
        it visible.
    */
    public static synchronized void print (String aMessage)
    {
        Instance().printMessage(aMessage);
    }




    /** Show the given string at the end of the message area and scroll to make
        it visible.
    */
    public static void println (String aMessage)
    {
        Instance().printMessage (aMessage+"\n");
    }


    public void paintComponent (Graphics g)
    {
        // Synchronize with the graphics object to prevent paint problems.
        // Remember that this is not done by Swing itself.
        synchronized (g)
        {
            JScrollBar sb = getVerticalScrollBar();
            if (sb != null)
            {
                int nScrollBarValue = sb.getMaximum() - sb.getVisibleAmount() - 1;
                sb.setValue (nScrollBarValue);
            }
            super.paintComponent (g);
        }
    }




    /** Append the given string to the end of the text and scroll so that it
        becomes visible.  This is an internal method.  Use one of the static
        and public ones.
    */
    private synchronized void printMessage (String aMessage)
    {
        maText.append (aMessage);
    }




    private static MessageArea saInstance = null;
    private JTextArea maText;
}
