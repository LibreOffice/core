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

package org.openoffice.accessibility.awb.view;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import javax.swing.JComponent;
import javax.swing.JLabel;


/** This class is a convenience class for views to use the GridBagLayout.
*/
class ViewGridLayout
{
    public ViewGridLayout (JComponent aComponent)
    {
        maComponent = aComponent;
        maComponent.setLayout (new GridBagLayout());
        maComponent.setMinimumSize (new Dimension (300,30));
        maComponent.setMaximumSize (new Dimension (300,1000));
        mnCurrentLine = 0;
    }

    public JLabel AddLabeledEntry (String sTitle)
    {
        return (JLabel)AddLabeledComponent (sTitle, new JLabel (""));
    }

    public JLabel AddLabeledString (String sTitle)
    {
        JLabel aLabel = AddLabeledEntry (sTitle);
        aLabel.setBackground (new Color(220,220,220));
        aLabel.setOpaque (true);
        return aLabel;
    }

    public JComponent AddLabeledComponent (String sTitle, JComponent aComponent)
    {
        GridBagConstraints constraints = new GridBagConstraints ();
        constraints.gridx = 0;
        constraints.anchor = GridBagConstraints.WEST;
        constraints.fill = GridBagConstraints.NONE;
        constraints.gridy = mnCurrentLine;

        JLabel aLabel = new JLabel(sTitle);
        aLabel.setFont (saFont);
        maComponent.add (aLabel, constraints);
        constraints.gridx = 1;
        constraints.weightx = 1;
        constraints.fill = GridBagConstraints.NONE;
        aComponent.setFont (saFont);
        maComponent.add (aComponent, constraints);

        mnCurrentLine += 1;

        return aComponent;
    }

    public JComponent AddComponent (JComponent aComponent)
    {
        GridBagConstraints constraints = new GridBagConstraints ();
        constraints.gridx = 0;
        constraints.gridwidth = 2;
        constraints.weightx = 1;
        constraints.anchor = GridBagConstraints.WEST;
        constraints.fill = GridBagConstraints.HORIZONTAL;
        constraints.gridy = mnCurrentLine;

        maComponent.add (aComponent, constraints);

        mnCurrentLine += 1;

        return aComponent;
    }

    static public Font GetFont ()
    {
        return saFont;
    }

    static private Font saFont;
    private int mnCurrentLine;
    private JComponent maComponent;

    static
    {
        saFont = new Font ("Dialog", Font.PLAIN, 11);
    }
}
