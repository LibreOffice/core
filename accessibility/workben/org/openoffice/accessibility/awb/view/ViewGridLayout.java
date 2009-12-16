/*************************************************************************
 *
 *  $RCSfile: ViewGridLayout.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:38 $
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
