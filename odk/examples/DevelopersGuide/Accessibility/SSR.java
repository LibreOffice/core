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

import java.awt.event.ActionListener;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import javax.swing.*;


/** The simple screen reader (SSR) registers at the toolkit as focus listener
    and displays information about the currently focused object.
*/
public class SSR
    implements ActionListener
{
    /** Just pass the control to the SSR class.
    */
    public static void main (String args[])
    {
        new SSR ();
    }




    /** Create a new instance of the simple screen reader.
    */
    public SSR ()
    {
        Layout ();

        // Create the event handler and tell it where to display information
        // about the currently focused accessible object.
        maEventHandler = new EventHandler ();
        maEventHandler.addObjectDisplay (maTextualDisplay);
        maEventHandler.addObjectDisplay (maGraphicalDisplay);
    }




    /** Setup the GUI.  It is divided into three areas.  The lower half is
        ocupied by a message area that logs all the events received from
        accessibility objects.  The upper half is shared by two different 
        displays of the currently focused object.  On left there is a textual
        representation.  On the right there is a graphical view of the 
        objects's outline.
    */
    private void Layout ()
    {
        GridBagConstraints constraints;

        JPanel aPanel = new JPanel (true);
        aPanel.setLayout (new GridBagLayout());
        aPanel.setOpaque (true);

        mFrame = new JFrame ("Simple Screen Reader 0.3");
        mFrame.setContentPane(aPanel);
        mFrame.setSize (600,400);

        
        addComponent (new JLabel ("Focused Object:"),
            0,0, 1,1, 0,0, GridBagConstraints.WEST, GridBagConstraints.NONE);


        maTextualDisplay = new TextualDisplay ();
        addComponent (maTextualDisplay,
            0,1, 1,1, 1,1, GridBagConstraints.CENTER, GridBagConstraints.BOTH);

        maGraphicalDisplay = new GraphicalDisplay ();
        addComponent (maGraphicalDisplay,
            1,0, 1,2, 1,1, GridBagConstraints.CENTER, GridBagConstraints.BOTH);

        addComponent (new JLabel ("Messages:"),
            0,2, 1,1, 0,0, GridBagConstraints.WEST, GridBagConstraints.NONE);

        addComponent (MessageArea.Instance(),
            0,3, 2,1, 1,1, GridBagConstraints.CENTER, GridBagConstraints.BOTH);


        JButton aButton = new JButton ("Quit SSR");
        addComponent (aButton,
            0,4, 1,1, 0,0, GridBagConstraints.WEST,GridBagConstraints.NONE);
        aButton.addActionListener (this);

        mFrame.show();
    }
    



    /** Add a GUI element with the given constraints to the main window.
    */
    private JComponent addComponent (JComponent aComponent,
        int x, int y, int width, int height, double weightx, double weighty,
        int anchor, int fill)
    {
        aComponent.setDoubleBuffered (false);
        GridBagConstraints aConstraints = new GridBagConstraints();
        aConstraints.gridx = x;
        aConstraints.gridy = y;
        aConstraints.gridwidth = width;
        aConstraints.gridheight = height;
        aConstraints.weightx = weightx;
        aConstraints.weighty = weighty;
        aConstraints.anchor = anchor;
        aConstraints.fill = fill;
        
        mFrame.getContentPane().add (aComponent, aConstraints);

        return aComponent;
    }




    /** This call-back handles button presses.
    */
    public void actionPerformed (java.awt.event.ActionEvent e)
    {
        if (e.getActionCommand().equals ("Quit SSR"))
        {
            maEventHandler.finalize ();
            System.exit(0);
        }
    }


    /// The main frame that contains all other GUI elements.
    private JFrame mFrame;

    /// A textutal representation of the currently focused object.
    private TextualDisplay maTextualDisplay;

    /// A graphical representation of the currently focused object.
    private GraphicalDisplay maGraphicalDisplay;

    /// The event handler that reacts to all the accessibility events.
    private EventHandler maEventHandler;
}
