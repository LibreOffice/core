/*************************************************************************
 *
 *  $RCSfile: TextView.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2008/05/14 13:21:35 $
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
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.JTree;
import javax.swing.tree.TreeNode;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.MutableTreeNode;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleTextType;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.TextSegment;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleMultiLineText;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.uno.UnoRuntime;

import org.openoffice.accessibility.awb.view.text.CaretSpinnerModel;
import org.openoffice.accessibility.awb.view.text.TextDialogFactory;


public class TextView
    extends ObjectView
    implements ActionListener
{

    /** Create a TextView when the given object supports the
        XAccessibleText interface.
    */
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        XAccessibleText xText = (XAccessibleText)UnoRuntime.queryInterface(
                XAccessibleText.class, xContext);
        if (xText != null)
            return new TextView (aContainer);
        else
            return null;
    }


    public TextView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        ViewGridLayout aLayout = new ViewGridLayout (this);

        maTextLabel = aLayout.AddLabeledString ("Text: ");
        maCharacterArrayLabel = aLayout.AddLabeledEntry ("Characters: ");
        maCharacterCountLabel = aLayout.AddLabeledEntry ("Character Count: ");
        maSelectionLabel = aLayout.AddLabeledEntry ("Selection: ");
        maBoundsLabel = aLayout.AddLabeledEntry ("Bounds Test: ");
        maCaretPositionSpinner = (JSpinner)aLayout.AddLabeledComponent (
            "Caret position:", new JSpinner());
        Dimension aSize = maCaretPositionSpinner.getSize();
        maCaretPositionSpinner.setPreferredSize (new Dimension (100,20));
        maCaretLineNoLabel = aLayout.AddLabeledEntry ("Line number at caret: ");
        maCaretLineTextLabel = aLayout.AddLabeledEntry ("Text of line at caret: ");
        maLineNoFromCaretPosLabel = aLayout.AddLabeledEntry ("Line number at index of caret: ");
        maLineTextFromCaretPosLabel = aLayout.AddLabeledEntry ("Text of line at index of caret: ");

        JPanel aButtonPanel = new JPanel ();
        aLayout.AddComponent (aButtonPanel);

        JButton aButton = new JButton ("select...");
        aButton.setFont (aLayout.GetFont());
        aButton.addActionListener (this);
        aButtonPanel.add (aButton);

        aButton = new JButton ("copy...");
        aButton.setFont (aLayout.GetFont());
        aButton.addActionListener (this);
        aButtonPanel.add (aButton);

        // A tree that holds the text broken down into various segments.
        maTree = new JTree ();
        aLayout.AddComponent (new JScrollPane (
            maTree,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));
    }


    /** Additionally to the context store a reference to the
        XAccessibleText interface.
    */
    public void SetObject (XAccessibleContext xObject)
    {
        mxText = (XAccessibleText)UnoRuntime.queryInterface(
            XAccessibleText.class, xObject);
        maCaretSpinnerModel = new CaretSpinnerModel(mxText);
        maCaretPositionSpinner.setModel (maCaretSpinnerModel);
        super.SetObject (xObject);
    }

    synchronized public void Destroy ()
    {
        mxText = null;
        super.Destroy();
    }

    synchronized public void Update ()
    {
        maCaretPositionSpinner.setEnabled (mxText != null);
        DefaultMutableTreeNode aRoot = new DefaultMutableTreeNode ("Text Segments");
        if (mxText == null)
        {
            maTextLabel.setText ("<null object>");
            maCharacterArrayLabel.setText ("<null object>");
            maCharacterCountLabel.setText ("<null object>");
            maSelectionLabel.setText ("<null object>");
            maBoundsLabel.setText ("<null object>");
            maCaretLineNoLabel.setText ("<null object>");
            maCaretLineTextLabel.setText ("<null object>");
            maLineNoFromCaretPosLabel.setText ("<null object>");
            maLineTextFromCaretPosLabel.setText ("<null object>");
        }
        else
        {
            maTextLabel.setText (mxText.getText());
            maCharacterArrayLabel.setText (GetCharacterArray());
            maCharacterCountLabel.setText (
                Integer.toString(mxText.getCharacterCount()));
            // Selection.
            maSelectionLabel.setText (
                "[" + mxText.getSelectionStart()
                + "," + mxText.getSelectionEnd()
                + "] \"" + mxText.getSelectedText() + "\"");

            // Character bounds.
            maBoundsLabel.setText (GetTextBoundsString());

            // Caret position.
            maCaretPositionSpinner.setValue (new Integer (mxText.getCaretPosition()));

            // Multi line methods.
            XAccessibleMultiLineText xMultiText = (XAccessibleMultiLineText)
                UnoRuntime.queryInterface( XAccessibleMultiLineText.class, mxText );

            if( null != xMultiText ) {
                try {
                  maCaretLineNoLabel.setText ( Integer.toString( xMultiText.getNumberOfLineWithCaret() ) );
                  TextSegment ts = xMultiText.getTextAtLineWithCaret();
                  maCaretLineTextLabel.setText ( "[" + ts.SegmentStart
                     + "," + ts.SegmentEnd
                     + "] \"" + ts.SegmentText + "\"");
                  maLineNoFromCaretPosLabel.setText ( Integer.toString( xMultiText.getLineNumberAtIndex( mxText.getCaretPosition() ) ) );
                  ts = xMultiText.getTextAtLineNumber(xMultiText.getLineNumberAtIndex( mxText.getCaretPosition() ) );
                  maLineTextFromCaretPosLabel.setText  ( "[" + ts.SegmentStart
                    + "," + ts.SegmentEnd
                    + "] \"" + ts.SegmentText + "\"");
                } catch( IndexOutOfBoundsException e) {
                }
            }

            // Text segments.
            aRoot.add (CreateNode ("Character", AccessibleTextType.CHARACTER));
            aRoot.add (CreateNode ("Word", AccessibleTextType.WORD));
            aRoot.add (CreateNode ("Sentence", AccessibleTextType.SENTENCE));
            aRoot.add (CreateNode ("Paragraph", AccessibleTextType.PARAGRAPH));
            aRoot.add (CreateNode ("Line", AccessibleTextType.LINE));
            aRoot.add (CreateNode ("Attribute", AccessibleTextType.ATTRIBUTE_RUN));
            aRoot.add (CreateNode ("Glyph", AccessibleTextType.GLYPH));
        }
        ((DefaultTreeModel)maTree.getModel()).setRoot (aRoot);
    }

    public String GetTitle ()
    {
        return ("Text");
    }

    public void notifyEvent (AccessibleEventObject aEvent)
    {
        System.out.println (aEvent);
        switch (aEvent.EventId)
        {
            case AccessibleEventId.CARET_CHANGED :
                maCaretSpinnerModel.Update();
                Update ();
                break;

            case AccessibleEventId.TEXT_CHANGED :
            case AccessibleEventId.TEXT_SELECTION_CHANGED:
                Update ();
                break;
        }
    }

    public void actionPerformed (ActionEvent aEvent)
    {
        String sCommand = aEvent.getActionCommand();
        if (sCommand.equals ("select..."))
            TextDialogFactory.CreateSelectionDialog (mxContext);
        else if (sCommand.equals ("copy..."))
            TextDialogFactory.CreateCopyDialog (mxContext);
    }



    /** Create a string that is a list of all characters returned by the
        getCharacter() method.
    */
    private String GetCharacterArray ()
    {
        // Do not show more than 30 characters.
        int nCharacterCount = mxText.getCharacterCount();
        int nMaxDisplayCount = 30;

        // build up string
        StringBuffer aCharacterArray = new StringBuffer();
        int nIndex = 0;
        try
        {
            while (nIndex<nCharacterCount && nIndex<nMaxDisplayCount)
            {
                aCharacterArray.append (mxText.getCharacter (nIndex));
                if (nIndex < nCharacterCount-1)
                    aCharacterArray.append (",");
                nIndex ++;
            }
            if (nMaxDisplayCount < nCharacterCount)
                aCharacterArray.append (", ...");
        }
        catch (IndexOutOfBoundsException e)
        {
            aCharacterArray.append ("; Index Out Of Bounds at index " + nIndex);
        }

        return aCharacterArray.toString();
    }



    /** Iterate over all characters and translate their positions
        back and forth.
        */
    private String GetTextBoundsString ()
    {
        StringBuffer aBuffer = new StringBuffer ();
        try
        {
            // Iterate over all characters in the text.
            int nCount = mxText.getCharacterCount();
            for (int i=0; i<nCount; i++)
            {
                // Get bounds for this character.
                Rectangle aBBox = mxText.getCharacterBounds (i);

                // get the character by 'clicking' into the middle of
                // the bounds
                Point aMiddle = new Point();
                aMiddle.X = aBBox.X + (aBBox.Width / 2) - 1;
                aMiddle.Y = aBBox.Y + (aBBox.Height / 2) - 1;
                int nIndex = mxText.getIndexAtPoint (aMiddle);

                // get the character, or a '#' for an illegal index
                if ((nIndex >= 0) && (nIndex < mxText.getCharacter(i)))
                    aBuffer.append (mxText.getCharacter(nIndex));
                else
                    aBuffer.append ('#');
            }
        }
        catch (IndexOutOfBoundsException aEvent)
        {
            // Ignore errors.
        }

        return aBuffer.toString();
    }




    private final static int BEFORE = -1;
    private final static int AT = 0;
    private final static int BEHIND = +1;

    private MutableTreeNode CreateNode (String sTitle, short nTextType)
    {
        DefaultMutableTreeNode aNode = new DefaultMutableTreeNode (sTitle);

        aNode.add (CreateSegmentNode ("Before", nTextType, BEFORE));
        aNode.add (CreateSegmentNode ("At", nTextType, AT));
        aNode.add (CreateSegmentNode ("Behind", nTextType, BEHIND));

        return aNode;
    }

    private MutableTreeNode CreateSegmentNode (String sTitle, short nTextType, int nWhere)
    {
        TextSegment aSegment;
        int nTextLength = mxText.getCharacterCount();
        DefaultMutableTreeNode aNode = new DefaultMutableTreeNode (sTitle);
        for (int nIndex=0; nIndex<=nTextLength; /* empty */)
        {
            aSegment = GetTextSegment (nIndex, nTextType, nWhere);
            DefaultMutableTreeNode aSegmentNode = new DefaultMutableTreeNode (
                new StringBuffer (
                    Integer.toString (nIndex) + " -> "
                    + Integer.toString (aSegment.SegmentStart) + " - "
                    + Integer.toString (aSegment.SegmentEnd) + " : "
                    + aSegment.SegmentText.toString()));
            aNode.add (aSegmentNode);
            if (nTextType ==  AccessibleTextType.ATTRIBUTE_RUN)
                AddAttributeNodes (aSegmentNode, aSegment);
            if (aSegment.SegmentEnd > nIndex)
                nIndex = aSegment.SegmentEnd;
            else
                nIndex ++;
        }

        return aNode;
    }


    private TextSegment GetTextSegment (int nIndex, short nTextType, int nWhere)
    {
        TextSegment aSegment;

        try
        {
            switch (nWhere)
            {
                case BEFORE:
                    aSegment = mxText.getTextBeforeIndex (nIndex, nTextType);
                    break;

                case AT:
                    aSegment = mxText.getTextAtIndex (nIndex, nTextType);
                    break;

                case BEHIND:
                    aSegment = mxText.getTextBehindIndex (nIndex, nTextType);
                    break;

                default:
                    aSegment = new TextSegment();
                    aSegment.SegmentText = new String ("unknown position "   + nWhere);
                    aSegment.SegmentStart = nIndex;
                    aSegment.SegmentStart = nIndex+1;
                    break;
            }
        }
        catch (IndexOutOfBoundsException aException)
        {
            aSegment = new TextSegment ();
            aSegment.SegmentText = new String ("Invalid index at ") + nIndex + " : "
                + aException.toString();
            aSegment.SegmentStart = nIndex;
            aSegment.SegmentEnd = nIndex+1;
        }
        catch (IllegalArgumentException aException)
        {
            aSegment = new TextSegment ();
            aSegment.SegmentText = new String ("Illegal argument at ") + nIndex + " : "
                + aException.toString();
            aSegment.SegmentStart = nIndex;
            aSegment.SegmentEnd = nIndex+1;
        }

        return aSegment;
    }


    /** Add to the given node one node for every attribute of the given segment.
    */
    private void AddAttributeNodes (
        DefaultMutableTreeNode aNode,
        TextSegment aSegment)
    {
        try
        {
            PropertyValue[] aValues = mxText.getCharacterAttributes (
                aSegment.SegmentStart, aAttributeList);
            for (int i=0; i<aValues.length; i++)
                aNode.add (new DefaultMutableTreeNode (
                    aValues[i].Name + ": " + aValues[i].Value));
        }
        catch (IndexOutOfBoundsException aException)
        {
            aNode.add (new DefaultMutableTreeNode (
                "caught IndexOutOfBoundsException while retrieveing attributes"));
        }
    }

    private XAccessibleText mxText;
    private JLabel
        maTextLabel,
        maCharacterArrayLabel,
        maCharacterCountLabel,
        maSelectionLabel,
        maBoundsLabel,
        maCaretLineNoLabel,
        maCaretLineTextLabel,
        maLineNoFromCaretPosLabel,
        maLineTextFromCaretPosLabel;

    private JSpinner maCaretPositionSpinner;
    private JTree maTree;
    private CaretSpinnerModel maCaretSpinnerModel;

    private static String[] aAttributeList = new String[] {
        "CharBackColor",
        "CharColor",
        "CharEscapement",
        "CharHeight",
        "CharPosture",
        "CharStrikeout",
        "CharUnderline",
        "CharWeight",
        "ParaAdjust",
        "ParaBottomMargin",
        "ParaFirstLineIndent",
        "ParaLeftMargin",
        "ParaLineSpacing",
        "ParaRightMargin",
        "ParaTabStops"};
}
