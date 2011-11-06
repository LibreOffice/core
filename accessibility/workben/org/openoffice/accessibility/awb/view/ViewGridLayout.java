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
