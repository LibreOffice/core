
import drafts.com.sun.star.accessibility.XAccessibleText;
import drafts.com.sun.star.accessibility.AccessibleTextType;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Point;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.IndexOutOfBoundsException;

import java.util.Vector;

class AccessibleTextHandler extends NodeHandler
{
    private XAccessibleText getText( Object aObject )
    {
        XAccessibleText xText =
            (XAccessibleText) UnoRuntime.queryInterface (
                 XAccessibleText.class, aObject);
        return xText;
    }

    public int getChildCount(Object aObject)
    {
        return (getText(aObject) != null) ? 11 : 0;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        Object aRet = null;

        XAccessibleText xText = getText( aObject );
        if( xText != null )
        {
            switch( nIndex )
            {
                case 0:
                    aRet = xText.getText();
                    break;
                case 1:
                    aRet = "# chars: " + xText.getCharacterCount();
                    break;
                case 2:
                    aRet = characters( xText );
                    break;
                case 3:
                    aRet = "selection: " + "[" + xText.getSelectionStart() +
                        "," + xText.getSelectionEnd() + "] \"" +
                        xText.getSelectedText() + "\"";
                    break;
                case 4:
                    aRet = "getCaretPosition: " + xText.getCaretPosition();
                    break;
                case 5:
                    aRet = textAtIndexNode( xText, "Character",
                                            AccessibleTextType.CHARACTER );
                    break;
                case 6:
                    aRet = textAtIndexNode( xText, "Word",
                                            AccessibleTextType.WORD );
                    break;
                case 7:
                    aRet = textAtIndexNode( xText, "Sentence",
                                            AccessibleTextType.SENTENCE );
                    break;
                case 8:
                    aRet = textAtIndexNode( xText, "Paragraph",
                                            AccessibleTextType.PARAGRAPH );
                    break;
                case 9:
                    aRet = textAtIndexNode( xText, "Line",
                                            AccessibleTextType.LINE );
                    break;
                case 10:
                    aRet = bounds( xText );
                    break;
            }
        }
        return aRet;
    }

    /** Create a text node that lists all strings of a particular text type
     */
    private Object textAtIndexNode(
        XAccessibleText xText,
        String sName,
        short nTextType)
    {
        Vector aWords = new Vector();

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
                int nStart = 0;
                for(int i = 1; i < nLength; i++)
                {
                    String sTmp = xText.getTextAtIndex(i, nTextType);
                    if( ! sTmp.equals( sWord ) )
                    {
                        aWords.add( "[" + nStart + "," + (i-1) + "] \"" +
                                       sWord + "\"" );
                        sWord = sTmp;
                        nStart = i;
                    }

                    // don't generate more than 50 children.
                    if( aWords.size() > 50 )
                    {
                        sWord = "...";
                        break;
                    }
                }
                aWords.add( "[" + nStart + "," + nLength + "] \"" +
                           sWord + "\"" );
            }
            catch( IndexOutOfBoundsException e )
            {
                aWords.add( e.toString() );
            }
        }

        // create node, and add default handlers
        AccTreeNode aNode = new AccTreeNode( aWords, sName );
        AccessibilityTreeModel.addDefaultHandlers( aNode );
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
}
