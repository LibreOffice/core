/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.lang.IndexOutOfBoundsException;

import javax.swing.*;
import java.awt.*;
import java.util.Vector;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

/**
 * Display a dialog with a list-box of children and select/deselect buttons
 */
class SelectionDialog extends JDialog
    implements ActionListener
{
    public SelectionDialog (AccTreeNode aNode)
    {
        super (AccessibilityWorkBench.Instance());

        maNode = aNode;

        Layout();
    }

    /** build dialog */
    protected void Layout ()
    {
        setTitle( "Select" );

        // vertical stacking of the elements
        Container aContent = getContentPane();

        // label with explanation
        aContent.add( new JLabel( "Select/Deselect child elements" ),
                      BorderLayout.NORTH );

        // the JListBox
        maChildrenSelector = new JList (GetChildrenList());
        maChildrenSelector.setPreferredSize (new Dimension (500,300));
        aContent.add (maChildrenSelector, BorderLayout.CENTER);
        maChildrenSelector.setSelectionMode (ListSelectionModel.SINGLE_SELECTION);

        JPanel aButtons = new JPanel();
        aButtons.setLayout( new FlowLayout() );

        JButton aButton;

        aButton = new JButton( "Select" );
        aButton.setActionCommand( "Select" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        aButton = new JButton( "Deselect" );
        aButton.setActionCommand( "Deselect" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        aButton = new JButton( "Select all" );
        aButton.setActionCommand( "Select all" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        aButton = new JButton( "Clear Selection" );
        aButton.setActionCommand( "Clear Selection" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        aButton = new JButton( "Close" );
        aButton.setActionCommand( "Close" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        // add Panel with buttons
        aContent.add( aButtons, BorderLayout.SOUTH );

        setSize( getPreferredSize() );
    }

    /** Get a list of all children
    */
    private Vector GetChildrenList ()
    {
        mxSelection = maNode.getSelection();

        XAccessibleContext xContext = maNode.getContext();
        int nCount = xContext.getAccessibleChildCount();
        Vector aChildVector = new Vector();
        for(int i = 0; i < nCount; i++)
        {
            try
            {
                XAccessible xChild = xContext.getAccessibleChild(i);
                XAccessibleContext xChildContext = xChild.getAccessibleContext();
                aChildVector.add( i + " " + xChildContext.getAccessibleName());
            }
            catch( IndexOutOfBoundsException e )
            {
                aChildVector.add( "ERROR: IndexOutOfBoundsException" );
            }
        }
        return aChildVector;
    }


    void close ()
    {
        hide();
        dispose();
    }

    void select()
    {
        try
        {
            mxSelection.selectAccessibleChild (maChildrenSelector.getSelectedIndex());
        }
        catch( IndexOutOfBoundsException e )
        {
            JOptionPane.showMessageDialog( AccessibilityWorkBench.Instance(),
                                           "Can't select: IndexOutofBounds",
                                           "Error in selectAccessibleChild",
                                           JOptionPane.ERROR_MESSAGE);
        }
    }

    void deselect()
    {
        try
        {
            mxSelection.deselectAccessibleChild(
                maChildrenSelector.getSelectedIndex());
        }
        catch( IndexOutOfBoundsException e )
        {
            JOptionPane.showMessageDialog( AccessibilityWorkBench.Instance(),
                                           "Can't deselect: IndexOutofBounds",
                                           "Error in deselectAccessibleChild",
                                           JOptionPane.ERROR_MESSAGE);
        }
    }

    void selectAll()
    {
        mxSelection.selectAllAccessibleChildren();
    }

    void clearSelection()
    {
        mxSelection.clearAccessibleSelection();
    }



    public void actionPerformed(ActionEvent e)
    {
        String sCommand = e.getActionCommand();

        if( "Close".equals( sCommand ) )
            close();
        else if ( "Select".equals( sCommand ) )
            select();
        else if ( "Deselect".equals( sCommand ) )
            deselect();
        else if ( "Clear Selection".equals( sCommand ) )
            clearSelection();
        else if ( "Select all".equals( sCommand ) )
            selectAll();
    }

    private JList maChildrenSelector;
    private XAccessibleSelection mxSelection;
    private AccTreeNode maNode;
}
