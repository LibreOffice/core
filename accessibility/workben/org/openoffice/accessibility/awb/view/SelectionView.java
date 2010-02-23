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

import java.util.Vector;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;

import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JOptionPane;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JToggleButton;
import javax.swing.ListSelectionModel;


import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.accessibility.XAccessibleStateSet;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.IndexOutOfBoundsException;


/** Display a list of children and select/deselect buttons
*/
class SelectionView
    extends ObjectView
    implements ActionListener
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        XAccessibleSelection xSelection = (XAccessibleSelection)UnoRuntime.queryInterface(
                XAccessibleSelection.class, xContext);
        if (xSelection != null)
            return new SelectionView(aContainer);
        else
            return null;
    }

    public SelectionView (ObjectViewContainer aContainer)
    {
        super (aContainer);
        Layout();
    }

    public String GetTitle ()
    {
        return "Selection";
    }

    /** Create and arrange the widgets for this view.
    */
    private void Layout ()
    {
        setLayout (new GridBagLayout());

        GridBagConstraints aConstraints = new GridBagConstraints();

        // Label that shows whether the selection is multi selectable.
        aConstraints.gridx = 0;
        aConstraints.gridy = 0;
        aConstraints.anchor = GridBagConstraints.WEST;
        maTypeLabel = new JLabel ();
        maTypeLabel.setFont (maContainer.GetViewFont());
        add (maTypeLabel, aConstraints);

        // the JListBox
        maChildrenSelector = new JPanel ();
        maChildrenSelector.setPreferredSize (new Dimension (100,100));
        maChildrenSelector.setLayout (
            new BoxLayout (maChildrenSelector, BoxLayout.Y_AXIS));

        aConstraints.gridx = 0;
        aConstraints.gridwidth = 4;
        aConstraints.gridy = 1;
        aConstraints.fill = GridBagConstraints.HORIZONTAL;
        add (new JScrollPane (maChildrenSelector,
                 JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                 JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED),
            aConstraints);

        JButton aButton;
        aButton = new JButton( "Select all" );
        aButton.setFont (maContainer.GetViewFont());
        aButton.setActionCommand( "Select all" );
        aButton.addActionListener( this );
        aConstraints.gridx = 0;
        aConstraints.gridwidth = 1;
        aConstraints.gridy = 2;
        aConstraints.fill = GridBagConstraints.NONE;
        aConstraints.anchor = GridBagConstraints.WEST;
        add (aButton, aConstraints);

        aButton = new JButton( "Clear Selection" );
        aButton.setFont (maContainer.GetViewFont());
        aButton.setActionCommand( "Clear Selection" );
        aButton.addActionListener( this );
        aConstraints.gridx = 1;
        aConstraints.gridy = 2;
        aConstraints.weightx = 1;
        add (aButton, aConstraints);

        setSize (getPreferredSize());
    }


    public void SetObject (XAccessibleContext xContext)
    {
        mxSelection = (XAccessibleSelection)UnoRuntime.queryInterface(
            XAccessibleSelection.class, xContext);
        super.SetObject (xContext);
    }


    public void Update ()
    {
        maChildrenSelector.removeAll ();

        // Determine whether multi selection is possible.
        XAccessibleStateSet aStateSet = mxContext.getAccessibleStateSet();
        boolean bMultiSelectable = false;
        if (aStateSet!=null && aStateSet.contains(
            AccessibleStateType.MULTI_SELECTABLE))
        {
            bMultiSelectable = true;
            maTypeLabel.setText ("multi selectable");
        }
        else
        {
            maTypeLabel.setText ("single selectable");
        }

        if (mxContext.getAccessibleRole() != AccessibleRole.TABLE)
        {
            int nCount = mxContext.getAccessibleChildCount();
            for (int i=0; i<nCount; i++)
            {
                try
                {
                    XAccessible xChild = mxContext.getAccessibleChild(i);
                    XAccessibleContext xChildContext = xChild.getAccessibleContext();

                    String sName = i + " " + xChildContext.getAccessibleName();
                    JToggleButton aChild;
                    aChild = new JCheckBox (sName);
                    aChild.setFont (maContainer.GetViewFont());

                    XAccessibleStateSet aChildStateSet =
                        mxContext.getAccessibleStateSet();
                    aChild.setSelected (aChildStateSet!=null
                        && aChildStateSet.contains(AccessibleStateType.SELECTED));

                    aChild.addActionListener (this);
                    maChildrenSelector.add (aChild);

                }
                catch (IndexOutOfBoundsException e)
                {
                }
            }
        }
    }


    void SelectAll()
    {
        mxSelection.selectAllAccessibleChildren();
    }

    void ClearSelection()
    {
        mxSelection.clearAccessibleSelection();
    }



    /** Call the function associated with the pressed button.
    */
    public void actionPerformed (ActionEvent aEvent)
    {
        String sCommand = aEvent.getActionCommand();

        if (sCommand.equals ("Clear Selection"))
            ClearSelection();
        else if (sCommand.equals ("Select all"))
            SelectAll();
        else
        {
            // Extract the child index from the widget text.
            String[] aWords = sCommand.split (" ");
            int nIndex = Integer.parseInt(aWords[0]);
            try
            {
                if (((JToggleButton)aEvent.getSource()).isSelected())
                    mxSelection.selectAccessibleChild (nIndex);
                else
                    mxSelection.deselectAccessibleChild (nIndex);
            }
            catch (IndexOutOfBoundsException e)
            {
                System.err.println (
                    "caught exception while changing selection: " + e);
            }
        }
    }


    public void notifyEvent (AccessibleEventObject aEvent)
    {
        switch (aEvent.EventId)
        {
            case AccessibleEventId.SELECTION_CHANGED:
            case AccessibleEventId.STATE_CHANGED:
            case AccessibleEventId.CHILD:
                Update ();
        }
    }

    private JPanel maChildrenSelector;
    private XAccessibleSelection mxSelection;
    private JLabel maTypeLabel;
}
