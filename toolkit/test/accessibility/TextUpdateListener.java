import drafts.com.sun.star.accessibility.*;
import com.sun.star.uno.UnoRuntime;

import javax.swing.tree.*;
import javax.swing.event.*;



/** listen to tree model changes in order to update XAccessibleText objects
*/
class TextUpdateListener implements TreeModelListener
{
    public void treeNodesChanged(TreeModelEvent e)
    {
        try {
        // if the change is to the first child of a DefaultMutableTreeNode
        // with an XAccessibleText child, then we call updateText
        int[] aIndices = e.getChildIndices();
        if( (aIndices != null) &&
            (aIndices.length > 0) )
        {
            // we have a parent... lets check for XAccessibleText then
            DefaultMutableTreeNode aParent = (DefaultMutableTreeNode)
                (e.getTreePath().getLastPathComponent());
            DefaultMutableTreeNode aNode = (DefaultMutableTreeNode)
                (aParent.getChildAt(aIndices[0]));
            if( aParent.getUserObject() instanceof XAccessibleText)
            {
                // aha! we have an xText. So we can now check for
                // the various cases we support
                XAccessibleText xText =
                    (XAccessibleText)aParent.getUserObject();

                if( aIndices[0] == 0 )
                {
                    // first child! Then we call updateText
                    updateText( xText, aNode.toString() );
                }
                else
                {
                    // JDK 1.4:
                    //                        // check for pattern "Selection:"
                    //                         Matcher m = Pattern.compile(
                    //                             "selection: \\[(-?[0-9]+),(-?[0-9]+)\\] \".*" ).
                    //                             matcher( aNode.toString() );
                    //                         if( m.matches() )
                    //                         {
                    //                             try
                    //                             {
                    //                                 // aha! Selection:
                    //                                 setSelection( xText,
                    //                                               Integer.parseInt(m.group(1)),
                    //                                               Integer.parseInt(m.group(2)) );
                    //                             }
                    //                             catch( NumberFormatException f )
                    //                             {
                    //                                 // ignore
                    //                             }
                    //                         }
                }
            }
        }
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException aException)
        {}
    }

    // don't care:
    public void treeNodesInserted(TreeModelEvent e) { ; }
    public void treeNodesRemoved(TreeModelEvent e) { ; }
    public void treeStructureChanged(TreeModelEvent e) { ; }

    /** update the text */
    boolean updateText( XAccessibleText xText, String sNew )
        throws com.sun.star.lang.IndexOutOfBoundsException
    {
        // is this text editable? if not, fudge you and return
        XAccessibleEditableText xEdit =
            (XAccessibleEditableText) UnoRuntime.queryInterface (
                XAccessibleEditableText.class, xText);
        if (xEdit == null)
            return false;

        String sOld = xText.getText();

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

    boolean setSelection( XAccessibleText xText, int p1, int p2 )
    {
        try
        {
            return xText.setSelection( p1, p2 );
        }
        catch( com.sun.star.lang.IndexOutOfBoundsException f )
        {
            return false;
        }
    }

    //         /** replace the given node with a new xText node */
    //         void updateNode( XAccessibleText xText,
    //                          DefaultMutableTreeNode aNode )
    //         {
    //             // create a new node
    //             DefaultMutableTreeNode aNew = newTextTreeNode( xText );
    //
    //             // get parent (must be DefaultMutableTreeNode)
    //             DefaultMutableTreeNode aParent =
    //                 (DefaultMutableTreeNode)aNode.getParent();
    //             if( aParent != null )
    //             {
    //                 // remove old sub-tree, and insert new one
    //                 int nIndex = aParent.getIndex( aNode );
    //                 aParent.remove( nIndex );
    //                 aParent.insert( aNew, nIndex );
    //             }
    //         }
}
