/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

