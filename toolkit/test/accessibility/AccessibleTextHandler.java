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

import com.sun.star.accessibility.AccessibleTextType;
import com.sun.star.accessibility.TextSegment;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleEditableText;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Point;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.beans.PropertyValue;

import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.JDialog;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.Icon;
import javax.swing.JTextArea;
import javax.swing.JOptionPane;
import javax.swing.JCheckBox;
import javax.swing.JColorChooser;
import javax.swing.BoxLayout;
import javax.swing.text.JTextComponent;


class AccessibleTextHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleText xText = (XAccessibleText) UnoRuntime.queryInterface (
            XAccessibleText.class, xContext);
        if (xText != null)
            return new AccessibleTextHandler (xText);
        else
            return null;
    }

    public AccessibleTextHandler ()
    {
    }

    public AccessibleTextHandler (XAccessibleText xText)
    {
        if (xText != null)
            maChildList.setSize (8);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        AccessibleTreeNode aChild = null;
        XAccessibleText xText = null;
        if (aParent instanceof AccTreeNode)
            xText = ((AccTreeNode)aParent).getText();

        try
        {
            if( xText != null )
            {
                switch( nIndex )
                {
                    case 0:
                        aChild = new StringNode (xText.getText(), aParent);
                        break;
                    case 1:
                        aChild = new StringNode ("# chars: " + xText.getCharacterCount(), aParent);
                        break;
                    case 2:
                        aChild = new StringNode (characters( xText ), aParent);
                        break;
                    case 3:
                        aChild = new StringNode ("selection: "
                            + "[" + xText.getSelectionStart()
                            + "," + xText.getSelectionEnd()
                            + "] \"" + xText.getSelectedText() + "\"",
                            aParent);
                        break;
                    case 4:
                        aChild = new StringNode ("getCaretPosition: " + xText.getCaretPosition(), aParent);
                        break;
                    case 5:
                    {
                        VectorNode aVec = new VectorNode("portions", aParent);
                        aChild = aVec;
                        aVec.addChild(
                             textAtIndexNode( xText, "Character",
                                              AccessibleTextType.CHARACTER,
                                              aParent ) );
                        aVec.addChild(
                            textAtIndexNode( xText, "Word",
                                             AccessibleTextType.WORD,
                                             aParent ) );
                        aVec.addChild(
                            textAtIndexNode( xText, "Sentence",
                                             AccessibleTextType.SENTENCE,
                                             aParent ) );
                        aVec.addChild(
                            textAtIndexNode( xText, "Paragraph",
                                             AccessibleTextType.PARAGRAPH,
                                             aParent ) );
                        aVec.addChild(
                            textAtIndexNode( xText, "Line",
                                             AccessibleTextType.LINE,
                                             aParent ) );
                        aVec.addChild(
                            textAtIndexNode( xText, "Attribute",
                                             AccessibleTextType.ATTRIBUTE_RUN,
                                             aParent ) );
                        aVec.addChild(
                            textAtIndexNode( xText, "Glyph",
                                             AccessibleTextType.GLYPH,
                                             aParent ) );
                    }
                    break;
                    case 6:
                        aChild = new StringNode (bounds( xText ), aParent);
                        break;
                    case 7:
                        aChild = getAttributes( xText, aParent );
                        break;
                    default:
                        aChild = new StringNode ("unknown child index " + nIndex, aParent);
                }
            }
        }
        catch (Exception e)
        {
            // Return empty child.
        }

        return aChild;
    }


    private String textAtIndexNodeString(
        int nStart, int nEnd,
        String sWord, String sBefore, String sBehind)
    {
        return "[" + nStart + "," + nEnd + "] "
            + "\"" + sWord + "\"     \t"
            + "(" + sBefore + ","
            + "" + sBehind + ")";
    }

    /** Create a text node that lists all strings of a particular text type
     */
    private AccessibleTreeNode textAtIndexNode(
        XAccessibleText xText,
        String sName,
        short nTextType,
        AccessibleTreeNode aParent)
    {
        VectorNode aNode = new VectorNode (sName, aParent);

        // get word at all positions;
        // for nicer display, compare current word to previous one and
        // make a new node for every interval, not for every word
        int nLength = xText.getCharacterCount();
        if( nLength > 0 )
        {
            try
            {
                // sWord + nStart mark the current word
                // make a node as soon as a new one is found; close the last
                // one at the end
                TextSegment sWord = xText.getTextAtIndex(0, nTextType);
                TextSegment sBefore = xText.getTextBeforeIndex(0, nTextType);
                TextSegment sBehind = xText.getTextBehindIndex(0, nTextType);
                int nStart = 0;
                for(int i = 1; i < nLength; i++)
                {
                    TextSegment sTmp = xText.getTextAtIndex(i, nTextType);
                    TextSegment sTBef = xText.getTextBeforeIndex(i, nTextType);
                    TextSegment sTBeh = xText.getTextBehindIndex(i, nTextType);
                    if( ! ( sTmp.equals( sWord ) && sTBef.equals( sBefore ) &&
                            sTBeh.equals( sBehind ) ) )
                    {
                        aNode.addChild (new StringNode (textAtIndexNodeString(
                            nStart, i,
                            sWord.SegmentText, sBefore.SegmentText, sBehind.SegmentText), aNode));
                        sWord = sTmp;
                        sBefore = sTBef;
                        sBehind = sTBeh;
                        nStart = i;
                    }

                    // don't generate more than 50 children.
                    if (aNode.getChildCount() > 50)
                    {
                        sWord.SegmentText = "...";
                        break;
                    }
                }
                aNode.addChild (new StringNode (textAtIndexNodeString(
                    nStart, nLength,
                    sWord.SegmentText, sBefore.SegmentText, sBehind.SegmentText), aNode));
            }
            catch( IndexOutOfBoundsException e )
            {
                aNode.addChild (new StringNode (e.toString(), aNode));
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                aNode.addChild (new StringNode (e.toString(), aNode));
            }
        }

        return aNode;
    }



    /** getCharacter (display as array string) */
    private String characters(XAccessibleText xText)
    {
        // get count (max. 30)
        int nChars = xText.getCharacterCount();
        if( nChars > 30 )
            nChars = 30;

        // build up string
        StringBuffer aChars = new StringBuffer();
        try
        {
            aChars.append( "[" );
            for( int i = 0; i < nChars; i++)
            {
                aChars.append( xText.getCharacter(i) );
                aChars.append( "," );
            }
            if( nChars > 0)
            {
                if( nChars == xText.getCharacterCount() )
                    aChars.deleteCharAt( aChars.length() - 1 );
                else
                    aChars.append( "..." );
            }
            aChars.append( "]" );
        }
        catch( IndexOutOfBoundsException e )
        {
            aChars.append( "   ERROR   " );
        }

        // return result
        return "getCharacters: " + aChars;
    }


    /** iterate over characters, and translate their positions
     * back and forth */
    private String bounds( XAccessibleText xText )
    {
        StringBuffer aBuffer = new StringBuffer( "bounds: " );
        try
        {
            // iterate over characters
            int nCount = xText.getCharacterCount();
            for(int i = 0; i < nCount; i++ )
            {
                // get bounds for this character
                Rectangle aRect = xText.getCharacterBounds( i );

                // get the character by 'clicking' into the middle of
                // the bounds
                Point aMiddle = new Point();
                aMiddle.X = aRect.X + (aRect.Width / 2) - 1;
                aMiddle.Y = aRect.Y + (aRect.Height / 2 ) - 1;
                int nIndex = xText.getIndexAtPoint( aMiddle );

                // get the character, or a '#' for an illegal index
                if( (nIndex >= 0) && (nIndex < xText.getCharacter(i)) )
                    aBuffer.append( xText.getCharacter(nIndex) );
                else
                    aBuffer.append( '#' );
            }
        }
        catch( IndexOutOfBoundsException e )
            { ; } // ignore errors

        return aBuffer.toString();
    }


    private AccessibleTreeNode getAttributes( XAccessibleText xText,
                                  AccessibleTreeNode aParent)
    {
        String[] aAttributeList = new String[] {
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

        AccessibleTreeNode aRet;

        try
        {
            VectorNode aPortions = new VectorNode ("getAttributes", aParent);

            int nIndex = 0;
            int nLength = xText.getCharacterCount();
            while( nIndex < nLength )
            {
                // get attribute run
                String aPortion = null;
                try
                {
                    aPortion = xText.getTextAtIndex(
                        nIndex, AccessibleTextType.ATTRIBUTE_RUN).SegmentText;
                }
                catch(com.sun.star.lang.IllegalArgumentException e)
                {
                    aPortion = new String ("");
                }

                // get attributes and make node with attribute children
                PropertyValue[] aValues = xText.getCharacterAttributes(nIndex, aAttributeList);
                VectorNode aAttrs = new VectorNode (aPortion, aPortions);
                for( int i = 0; i < aValues.length; i++ )
                {
                    new StringNode( aValues[i].Name + ": " + aValues[i].Value,
                                    aAttrs );
                }

                // get next portion, but advance at least one
                nIndex += (aPortion.length() > 0) ? aPortion.length() : 1;
            }

            aRet = aPortions;
        }
        catch( IndexOutOfBoundsException e )
        {
            aRet = new StringNode( "Exception caught:" + e, aParent );
        }

        return aRet;
    }


    static String[] aTextActions =
        new String[] { "select...", "copy..." };
    static String[] aEditableTextActions =
        new String[] { "select...", "copy...",
                       "cut...", "paste...", "edit...", "format..." };

    public String[] getActions (AccessibleTreeNode aNode)
    {
        XAccessibleEditableText xEText = null;
        if (aNode instanceof AccTreeNode)
            xEText = ((AccTreeNode)aNode).getEditText ();

        return (xEText == null) ? aTextActions : aEditableTextActions;
    }

    public void performAction (AccessibleTreeNode aNode, int nIndex)
    {
        if ( ! (aNode instanceof AccTreeNode))
            return;

        AccTreeNode aATNode = (AccTreeNode)aNode;
        TextActionDialog aDialog = null;

        // create proper dialog
        switch( nIndex )
        {
            case 0:
                aDialog = new TextActionDialog( aATNode,
                                                "Select range:",
                                                "select" )
                    {
                        boolean action(
                            JTextComponent aText, AccTreeNode aNode )
                            throws IndexOutOfBoundsException
                        {
                            return aNode.getText().setSelection(
                                getSelectionStart(),
                                getSelectionEnd() );
                        }
                    };
                break;
            case 1:
                aDialog = new TextActionDialog( aATNode,
                                                "Select range and copy:",
                                                "copy" )
                    {
                        boolean action(
                            JTextComponent aText, AccTreeNode aNode )
                            throws IndexOutOfBoundsException
                        {
                            return aNode.getText().copyText(
                                getSelectionStart(),
                                getSelectionEnd() );
                        }
                    };
                break;
            case 2:
                aDialog = new TextActionDialog( aATNode,
                                                "Select range and cut:",
                                                "cut" )
                    {
                        boolean action(
                            JTextComponent aText, AccTreeNode aNode )
                            throws IndexOutOfBoundsException
                        {
                            return aNode.getEditText().cutText(
                                getSelectionStart(),
                                getSelectionEnd() );
                        }
                    };
                break;
            case 3:
                aDialog = new TextActionDialog( aATNode,
                                                "Place Caret and paste:",
                                                "paste" )
                    {
                        boolean action(
                            JTextComponent aText, AccTreeNode aNode )
                            throws IndexOutOfBoundsException
                        {
                            return aNode.getEditText().pasteText(
                                aText.getCaretPosition() );
                        }
                    };
                break;
            case 4:
                aDialog = new TextEditDialog( aATNode, "Edit text:",
                                              "edit" );
                break;
            case 5:
                aDialog = new TextAttributeDialog( aATNode );
                break;
        }

        if( aDialog != null )
            aDialog.show();
    }

}

/**
 * Display a dialog with a text field and a pair of cancel/do-it buttons
 */
class TextActionDialog extends JDialog
    implements ActionListener
{
    AccTreeNode aNode;
    JTextArea aText;
    String sName;
    JCheckBox aIndexToggle;

    public TextActionDialog( AccTreeNode aNd,
                             String sExplanation,
                             String sButtonText )
    {
        super( AccessibilityWorkBench.Instance() );

        aNode = aNd;
        sName = sButtonText;
        init( sExplanation, aNode.getText().getText(), sButtonText );
//        setSize( getPreferredSize() );
        setSize( 350, 225 );
    }

    /** build dialog */
    protected void init( String sExplanation,
                         String sText,
                         String sButtonText )
    {
        setTitle( sName );

        // vertical stacking of the elements
        Container aContent = getContentPane();
        //        aContent.setLayout( new BorderLayout() );

        // label with explanation
        if( sExplanation.length() > 0 )
            aContent.add( new JLabel( sExplanation ), BorderLayout.NORTH );

        // the text field
        aText = new JTextArea();
        aText.setText( sText );
        aText.setColumns( Math.min( Math.max( 40, sText.length() ), 20 ) );
        aText.setRows( sText.length() / 40 + 1 );
        aText.setLineWrap( true );
        aText.setEditable( false );
        aContent.add( aText, BorderLayout.CENTER );

        JPanel aButtons = new JPanel();
        aButtons.setLayout( new FlowLayout() );
        aIndexToggle = new JCheckBox( "reverse selection" );
        aButtons.add( aIndexToggle );
        JButton aActionButton = new JButton( sButtonText );
        aActionButton.setActionCommand( "Action" );
        aActionButton.addActionListener( this );
        aButtons.add( aActionButton );
        JButton aCancelButton = new JButton( "cancel" );
        aCancelButton.setActionCommand( "Cancel" );
        aCancelButton.addActionListener( this );
        aButtons.add( aCancelButton );

        // add Panel with buttons
        aContent.add( aButtons, BorderLayout.SOUTH );
    }

    void cancel()
    {
        hide();
        dispose();
    }

    void action()
    {
        String sError = null;
        try
        {
            boolean bSuccess = action( aText, aNode );
            if( !bSuccess )
                sError = "Can't execute";
        }
        catch( IndexOutOfBoundsException e )
        {
            sError = "Index out of bounds";
        }

        if( sError != null )
            JOptionPane.showMessageDialog( AccessibilityWorkBench.Instance(),
                                           sError, sName,
                                           JOptionPane.ERROR_MESSAGE);

        cancel();
    }

    public void actionPerformed(ActionEvent e)
    {
        String sCommand = e.getActionCommand();

        if( "Cancel".equals( sCommand ) )
            cancel();
        else if( "Action".equals( sCommand ) )
            action();
    }


    int getSelectionStart()     { return getSelection(true); }
    int getSelectionEnd()       { return getSelection(false); }
    int getSelection(boolean bStart)
    {
        return ( bStart ^ aIndexToggle.isSelected() )
            ? aText.getSelectionStart() : aText.getSelectionEnd();
    }



    /** override this for dialog-specific action */
    boolean action( JTextComponent aText, AccTreeNode aNode )
        throws IndexOutOfBoundsException
    {
        return false;
    }
}


class TextEditDialog extends TextActionDialog
{
    public TextEditDialog( AccTreeNode aNode,
                           String sExplanation,
                           String sButtonText )
    {
        super( aNode, sExplanation, sButtonText );
    }

    protected void init( String sExplanation,
                         String sText,
                         String sButtonText )
    {
        super.init( sExplanation, sText, sButtonText );
        aText.setEditable( true );
    }


    /** edit the text */
    boolean action( JTextComponent aText, AccTreeNode aNode )
    {
        // is this text editable? if not, fudge you and return
        XAccessibleEditableText xEdit = aNode.getEditText();
        return ( xEdit == null ) ? false :
            updateText( xEdit, aText.getText() );
    }


    /** update the text */
    boolean updateText( XAccessibleEditableText xEdit, String sNew )
    {
        String sOld = xEdit.getText();

        // false alarm? Early out if no change was done!
        if( sOld.equals( sNew ) )
            return false;

        // get the minimum length of both strings
        int nMinLength = sOld.length();
        if( sNew.length() < nMinLength )
            nMinLength = sNew.length();

        // count equal characters from front and end
        int nFront = 0;
        while( (nFront < nMinLength) &&
               (sNew.charAt(nFront) == sOld.charAt(nFront)) )
            nFront++;
        int nBack = 0;
        while( (nBack < nMinLength) &&
               ( sNew.charAt(sNew.length()-nBack-1) ==
                 sOld.charAt(sOld.length()-nBack-1)    ) )
            nBack++;
        if( nFront + nBack > nMinLength )
            nBack = nMinLength - nFront;

        // so... the first nFront and the last nBack characters
        // are the same. Change the others!
        String sDel = sOld.substring( nFront, sOld.length() - nBack );
        String sIns = sNew.substring( nFront, sNew.length() - nBack );

        System.out.println("edit text: " +
                           sOld.substring(0, nFront) +
                           " [ " + sDel + " -> " + sIns + " ] " +
                           sOld.substring(sOld.length() - nBack) );

        boolean bRet = false;
        try
        {
            // edit the text, and use
            // (set|insert|delete|replace)Text as needed
            if( nFront+nBack == 0 )
                bRet = xEdit.setText( sIns );
            else if( sDel.length() == 0 )
                bRet = xEdit.insertText( sIns, nFront );
            else if( sIns.length() == 0 )
                bRet = xEdit.deleteText( nFront, sOld.length()-nBack );
            else
                bRet = xEdit.replaceText(nFront, sOld.length()-nBack,sIns);
        }
        catch( IndexOutOfBoundsException e )
        {
            bRet = false;
        }

        return bRet;
    }
}


class TextAttributeDialog extends TextActionDialog
{
    public TextAttributeDialog(
        AccTreeNode aNode )
    {
        super( aNode, "Choose attributes, select text, and press 'Set':",
               "set" );
    }

    private JCheckBox aBold, aUnderline, aItalics;
    private Color aForeground, aBackground;

    protected void init( String sExplanation,
                         String sText,
                         String sButtonText )
    {
        super.init( sExplanation, sText, sButtonText );

        aForeground = Color.black;
        aBackground = Color.white;

        JPanel aAttr = new JPanel();
        aAttr.setLayout( new BoxLayout( aAttr, BoxLayout.Y_AXIS ) );

        aBold = new JCheckBox( "bold" );
        aUnderline = new JCheckBox( "underline" );
        aItalics = new JCheckBox( "italics" );

        JButton aForeButton = new JButton("Foreground", new ColorIcon(true));
        aForeButton.addActionListener( new ActionListener() {
                public void actionPerformed(ActionEvent e)
                {
                    aForeground = JColorChooser.showDialog(
                        TextAttributeDialog.this,
                        "Select Foreground Color",
                        aForeground);
                }
            } );

        JButton aBackButton = new JButton("Background", new ColorIcon(false));
        aBackButton.addActionListener( new ActionListener() {
                public void actionPerformed(ActionEvent e)
                {
                    aBackground = JColorChooser.showDialog(
                        TextAttributeDialog.this,
                        "Select Background Color",
                        aBackground);
                }
            } );

        aAttr.add( aBold );
        aAttr.add( aUnderline );
        aAttr.add( aItalics );
        aAttr.add( aForeButton );
        aAttr.add( aBackButton );

        getContentPane().add( aAttr, BorderLayout.WEST );
    }


    class ColorIcon implements Icon
    {
        boolean bForeground;
        static final int nHeight = 16;
        static final int nWidth = 16;

        public ColorIcon(boolean bWhich) { bForeground = bWhich; }
        public int getIconHeight()  { return nHeight; }
        public int getIconWidth() { return nWidth; }
        public void paintIcon(Component c, Graphics g, int x, int y)
        {
            g.setColor( getColor() );
            g.fillRect( x, y, nHeight, nWidth );
            g.setColor( c.getForeground() );
            g.drawRect( x, y, nHeight, nWidth );
        }
        Color getColor()
        {
            return bForeground ? aForeground : aBackground;
        }
    }



    /** edit the text */
    boolean action( JTextComponent aText, AccTreeNode aNode )
        throws IndexOutOfBoundsException
    {
        // is this text editable? if not, fudge you and return
        XAccessibleEditableText xEdit = aNode.getEditText();
        boolean bSuccess = false;
        if( xEdit != null )
        {
            PropertyValue[] aSequence = new PropertyValue[6];
            aSequence[0] = new PropertyValue();
            aSequence[0].Name = "CharWeight";
            aSequence[0].Value = new Integer( aBold.isSelected() ? 150 : 100 );
            aSequence[1] = new PropertyValue();
            aSequence[1].Name = "CharUnderline";
            aSequence[1].Value = new Integer( aUnderline.isSelected() ? 1 : 0 );
            aSequence[2] = new PropertyValue();
            aSequence[2].Name = "CharBackColor";
            aSequence[2].Value = new Integer( aBackground.getRGB() );
            aSequence[3] = new PropertyValue();
            aSequence[3].Name = "CharColor";
            aSequence[3].Value = new Integer( aForeground.getRGB() );
            aSequence[4] = new PropertyValue();
            aSequence[4].Name = "CharPosture";
            aSequence[4].Value = new Integer( aItalics.isSelected() ? 1 : 0 );
            aSequence[5] = new PropertyValue();
            aSequence[5].Name = "CharBackTransparent";
            aSequence[5].Value = new Boolean( false );

            bSuccess = xEdit.setAttributes( getSelectionStart(),
                                            getSelectionEnd(),
                                            aSequence );
        }
        return bSuccess;
    }

}
