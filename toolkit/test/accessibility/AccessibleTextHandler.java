
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleText;
import drafts.com.sun.star.accessibility.XAccessibleEditableText;
import drafts.com.sun.star.accessibility.AccessibleTextType;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Point;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.IndexOutOfBoundsException;

import java.util.Vector;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.BorderLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.JDialog;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JTextArea;
import javax.swing.JOptionPane;
import javax.swing.text.JTextComponent;


class AccessibleTextHandler extends NodeHandler
{
    private XAccessibleText mxText;

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
        mxText = xText;
        if (mxText != null)
            maChildList.setSize (12);
    }

    protected XAccessibleText getText( Object aObject )
    {
        return mxText;
    }

    protected XAccessibleEditableText getEditText( Object aObject )
    {
        XAccessibleEditableText xText =
            (XAccessibleEditableText) UnoRuntime.queryInterface (
                 XAccessibleEditableText.class, aObject);
        return xText;
    }



    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        AccessibleTreeNode aChild = null;
        XAccessibleText xText = null;
        if (aParent instanceof AccTreeNode)
            xText = getText (((AccTreeNode)aParent).getContext());

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
                        aChild = textAtIndexNode( xText, "Character",
                            AccessibleTextType.CHARACTER,
                            aParent);
                        break;
                    case 6:
                        aChild = textAtIndexNode( xText, "Word",
                            AccessibleTextType.WORD,
                            aParent);
                        break;
                    case 7:
                        aChild = textAtIndexNode( xText, "Sentence",
                            AccessibleTextType.SENTENCE,
                            aParent);
                        break;
                    case 8:
                        aChild = textAtIndexNode( xText, "Paragraph",
                            AccessibleTextType.PARAGRAPH,
                            aParent);
                        break;
                    case 9:
                        aChild = textAtIndexNode( xText, "Line",
                            AccessibleTextType.LINE,
                            aParent);
                        break;
                    case 10:
                        aChild = textAtIndexNode( xText, "Attribute",
                            (short)6/*AccessibleTextType.ATTRIBUTE*/,
                            aParent);
                        break;
                    case 11:
                        aChild = new StringNode (bounds( xText ), aParent);
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
                String sWord = xText.getTextAtIndex(0, nTextType);
                String sBefore = xText.getTextBeforeIndex(0, nTextType);
                String sBehind = xText.getTextBehindIndex(0, nTextType);
                int nStart = 0;
                for(int i = 1; i < nLength; i++)
                {
                    String sTmp = xText.getTextAtIndex(i, nTextType);
                    String sTBef = xText.getTextBeforeIndex(i, nTextType);
                    String sTBeh = xText.getTextBehindIndex(i, nTextType);
                    if( ! ( sTmp.equals( sWord ) && sTBef.equals( sBefore ) &&
                            sTBeh.equals( sBehind ) ) )
                    {
                        aNode.addChild (new StringNode (textAtIndexNodeString(
                            nStart, i, sWord, sBefore, sBehind), aNode));
                        sWord = sTmp;
                        sBefore = sTBef;
                        sBehind = sTBeh;
                        nStart = i;
                    }

                    // don't generate more than 50 children.
                    if (aNode.getChildCount() > 50)
                    {
                        sWord = "...";
                        break;
                    }
                }
                aNode.addChild (new StringNode (textAtIndexNodeString(
                    nStart, nLength, sWord, sBefore, sBehind), aNode));
            }
            catch( IndexOutOfBoundsException e )
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


    static String[] aTextActions =
        new String[] { "select...", "copy..." };
    static String[] aEditableTextActions =
        new String[] { "select...", "copy...",
                       "cut...", "paste...", "edit..." };

    public String[] getActions(Object aObject)
    {
        XAccessibleEditableText xEText = getEditText( aObject );

        return (xEText == null) ? aTextActions : aEditableTextActions;
    }

    public void performAction(Object aObject, int nIndex)
    {
        TextActionDialog aDialog = null;

        XAccessibleText xText = getText( aObject );

        // create proper dialog
        switch( nIndex )
        {
            case 0:
                aDialog = new TextActionDialog( aObject,
                                                "Select range:",
                                                xText.getText(),
                                                "select" )
                    {
                        void action( JTextComponent aText, Object aObject )
                                throws IndexOutOfBoundsException
                        {
                            getText( aObject ).setSelection(
                                aText.getSelectionStart(),
                                aText.getSelectionEnd() );
                        }
                    };
                break;
            case 1:
                aDialog = new TextActionDialog( aObject,
                                                "Select range and copy:",
                                                xText.getText(),
                                                "copy" )
                    {
                        void action( JTextComponent aText, Object aObject )
                                throws IndexOutOfBoundsException
                        {
                            getText( aObject ).copyText(
                                aText.getSelectionStart(),
                                aText.getSelectionEnd() );
                        }
                    };
                break;
            case 2:
                aDialog = new TextActionDialog( aObject,
                                                "Select range and cut:",
                                                xText.getText(),
                                                "cut" )
                    {
                        void action( JTextComponent aText, Object aObject )
                                throws IndexOutOfBoundsException
                        {
                            getEditText( aObject ).cutText(
                                aText.getSelectionStart(),
                                aText.getSelectionEnd() );
                        }
                    };
                break;
            case 3:
                aDialog = new TextActionDialog( aObject,
                                                "Place Caret and paste:",
                                                xText.getText(),
                                                "paste" )
                    {
                        void action( JTextComponent aText, Object aObject )
                                throws IndexOutOfBoundsException
                        {
                            getEditText( aObject ).pasteText(
                                aText.getCaretPosition() );
                        }
                    };
                break;
            case 4:
                aDialog = new TextEditDialog( aObject, "Edit text:",
                                              xText.getText(), "edit" );
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
    JTextArea aText;
    Object aObject;
    String sName;

    public TextActionDialog( Object aObj,
                             String sExplanation,
                             String sText,
                             String sButtonText )
    {
        super( AccessibilityWorkBench.get() );

        aObject = aObj;
        sName = sButtonText;
        init( sExplanation, sText, sButtonText );
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
        aText.setColumns( 40 );
        aText.setRows( Math.max( sText.length() / 40, 5 ) );
        aText.setLineWrap( true );
        aText.setEditable( false );
        aContent.add( aText, BorderLayout.CENTER );

        JPanel aButtons = new JPanel();
        aButtons.setLayout( new FlowLayout() );
        JButton aActionButton = new JButton( sButtonText );
        aActionButton.setActionCommand( "Action" );
        aActionButton.addActionListener( this );
        aButtons.add( aActionButton );
        JButton aCancelButton = new JButton( "Cancel" );
        aCancelButton.setActionCommand( "Cancel" );
        aCancelButton.addActionListener( this );
        aButtons.add( aCancelButton );

        // add Panel with buttons
        aContent.add( aButtons, BorderLayout.SOUTH );

        setSize( aContent.getPreferredSize() );
    }

    void cancel()
    {
        hide();
        dispose();
    }

    void action()
    {
        try
        {
            action( aText, aObject );
        }
        catch( IndexOutOfBoundsException e )
        {
            JOptionPane.showMessageDialog(
                AccessibilityWorkBench.get(), "Index out of bounds",
                sName, JOptionPane.ERROR_MESSAGE);
        }
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


    /** override this for dialog-specific action */
    void action( JTextComponent aText, Object aObject )
        throws IndexOutOfBoundsException
    {
    }
}


class TextEditDialog extends TextActionDialog
{
    public TextEditDialog( Object aObj,
                           String sExplanation,
                           String sText,
                           String sButtonText )
    {
        super( aObj, sExplanation, sText, sButtonText );
    }

    protected void init( String sExplanation,
                         String sText,
                         String sButtonText )
    {
        super.init( sExplanation, sText, sButtonText );
        aText.setEditable( true );
    }


    /** edit the text */
    void action( JTextComponent aText, Object aObject )
    {
        // is this text editable? if not, fudge you and return
        XAccessibleEditableText xEdit =
            (XAccessibleEditableText) UnoRuntime.queryInterface (
                  XAccessibleEditableText.class, aObject);
        if( xEdit != null )
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

