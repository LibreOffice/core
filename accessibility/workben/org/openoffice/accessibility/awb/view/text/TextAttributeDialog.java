/*************************************************************************
 *
 *  $RCSfile: TextAttributeDialog.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

