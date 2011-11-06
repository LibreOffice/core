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



package org.openoffice.accessibility.awb.view.text;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JColorChooser;
import javax.swing.JPanel;
import javax.swing.text.JTextComponent;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.uno.UnoRuntime;


class TextAttributeDialog
    extends TextActionDialog
{
    public TextAttributeDialog (XAccessibleContext xContext)
    {
        super (xContext,
            "Choose attributes, select text, and press 'Set':",
            "set");
    }

    protected void Layout ()
    {
        super.Layout ();

        maForeground = Color.black;
        maBackground = Color.white;

        JPanel aPanel = new JPanel();
        aPanel.setLayout (new BoxLayout (aPanel, BoxLayout.Y_AXIS));

        maBoldCheckBox = new JCheckBox ("bold");
        maUnderlineCheckBox = new JCheckBox ("underline");
        maItalicsCheckBox = new JCheckBox ("italics");

        JButton aForegroundButton = new JButton ("Foreground",
            new TextAttributeDialog.ColorIcon(true));
        aForegroundButton.addActionListener (new ActionListener()
            {
                public void actionPerformed (ActionEvent aEvent)
                {
                    maForeground = JColorChooser.showDialog (
                        TextAttributeDialog.this,
                        "Select Foreground Color",
                        maForeground);
                }
            } );

        JButton aBackgroundButton = new JButton("Background",
            new TextAttributeDialog.ColorIcon(false));
        aBackgroundButton.addActionListener (new ActionListener()
            {
                public void actionPerformed (ActionEvent eEvent)
                {
                    maBackground = JColorChooser.showDialog(
                        TextAttributeDialog.this,
                        "Select Background Color",
                        maBackground);
                }
            } );

        aPanel.add (maBoldCheckBox);
        aPanel.add (maUnderlineCheckBox);
        aPanel.add (maItalicsCheckBox);
        aPanel.add (aForegroundButton);
        aPanel.add (aBackgroundButton);

        getContentPane().add (aPanel, BorderLayout.WEST);
    }


    /** edit the text */
    boolean EditableTextAction (XAccessibleEditableText xText)
        throws IndexOutOfBoundsException
    {
        PropertyValue[] aSequence = new PropertyValue[6];
        aSequence[0] = new PropertyValue();
        aSequence[0].Name = "CharWeight";
        aSequence[0].Value = new Integer (maBoldCheckBox.isSelected() ? 150 : 100);
        aSequence[1] = new PropertyValue();
        aSequence[1].Name = "CharUnderline";
        aSequence[1].Value = new Integer (maUnderlineCheckBox.isSelected() ? 1 : 0);
        aSequence[2] = new PropertyValue();
        aSequence[2].Name = "CharBackColor";
        aSequence[2].Value = new Integer (maBackground.getRGB());
        aSequence[3] = new PropertyValue();
        aSequence[3].Name = "CharColor";
        aSequence[3].Value = new Integer (maForeground.getRGB());
        aSequence[4] = new PropertyValue();
        aSequence[4].Name = "CharPosture";
        aSequence[4].Value = new Integer (maItalicsCheckBox.isSelected() ? 1 : 0);
        aSequence[5] = new PropertyValue();
        aSequence[5].Name = "CharBackTransparent";
        aSequence[5].Value = new Boolean (false);

        return xText.setAttributes (
            GetSelectionStart(),
            GetSelectionEnd(),
            aSequence);
    }

    class ColorIcon
        implements Icon
    {
        public ColorIcon(boolean bWhich) { bForeground = bWhich; }
        public int getIconHeight()  { return nHeight; }
        public int getIconWidth() { return nWidth; }
        public void paintIcon (Component c, Graphics g, int x, int y)
        {
            g.setColor( getColor() );
            g.fillRect( x, y, nHeight, nWidth );
            g.setColor( c.getForeground() );
            g.drawRect( x, y, nHeight, nWidth );
        }
        Color getColor()
        {
            if (bForeground)
                return maForeground;
            else
                return maBackground;
        }

        private static final int nHeight = 16;
        private static final int nWidth = 16;
        private boolean bForeground;
    }




    private JCheckBox
        maBoldCheckBox,
        maUnderlineCheckBox,
        maItalicsCheckBox;
    private Color
        maForeground,
        maBackground;

}

