import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleExtendedComponent;
import drafts.com.sun.star.accessibility.XAccessibleAction;
import drafts.com.sun.star.accessibility.XAccessibleImage;
import drafts.com.sun.star.accessibility.XAccessibleRelationSet;
import drafts.com.sun.star.accessibility.XAccessibleStateSet;
import drafts.com.sun.star.accessibility.XAccessibleText;
import drafts.com.sun.star.accessibility.XAccessibleEditableText;
import drafts.com.sun.star.accessibility.AccessibleTextType;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.uno.UnoRuntime;

import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;



/** This class is a start to collect the handling of a JTree and a DefaultTreeModel.
*/
public class AccessibilityTree extends JTree
{
//     public static void main (String args[])
//     {
//         // doesn't work any more; constructor has changed
//         new AccessibilityWorkBench ();
//     }




    /** Local class used to store the accessible object with every tree node.
    */
    protected class TreeNode extends DefaultMutableTreeNode
    {
        public AccessibleObject aAccessibleObject;
        public TreeNode (String sName)
        {
            super (sName);
            aAccessibleObject = null;
        }
    }

    /** Create a new accessibility tree.  Use the specified message display
        for displaying messages and the specified canvas to draw the
        graphical representations of accessible objects on.
    */
    public AccessibilityTree (
        MessageInterface aMessageDisplay,
        Canvas aCanvas)
    {
        maMessageDisplay = aMessageDisplay;
        maCanvas = aCanvas;

        maRoot = new DefaultMutableTreeNode ("Accessibility Tree");
        maTreeModel = new DefaultTreeModel (maRoot);
        setModel (maTreeModel);

        setEditable (true);

        MouseListener ml = new MouseAdapter() {
                public void mousePressed(MouseEvent e) {
                    int selRow = getRowForLocation(e.getX(), e.getY());
                    TreePath selPath = getPathForLocation(e.getX(), e.getY());
                    if(selRow != -1) {
                        if (e.getClickCount() == 2)
                        {
                            System.out.println ("double click : " + selRow + " , " + selPath);
                            Object aObject = selPath.getLastPathComponent();
                            if (TreeNode.class.isInstance (aObject))
                            {
                                TreeNode aNode = (TreeNode)aObject;
                                createTree (aNode.aAccessibleObject.getAccessible(),
                                    selPath);
                                expandShapes ();
                            }
                        }
                    }
                }
            };
        addMouseListener (ml);

        // allow editing of XAccessibleText interfaces
        maTreeModel.addTreeModelListener( new TextUpdateListener() );
    }




    public void createTree (XAccessible xRoot, TreePath aPath)
    {
        System.out.println ("creating accessibility tree for root " + xRoot + " under path " +
            aPath);
        TreeNode aTree = createAccessibilityTree (
            xRoot,
            0,
            new java.awt.Point(0,0),
            aPath);
        DefaultMutableTreeNode aRoot = (DefaultMutableTreeNode)aPath.getLastPathComponent();
        aRoot.insert (aTree, aRoot.getChildCount());
        maTreeModel.reload ();
    }



    public void createTree (XAccessible xRoot)
    {
        createTree (xRoot, new TreePath (maRoot));
    }




    public DefaultMutableTreeNode getRoot ()
    {
        return maRoot;
    }




    protected TreeNode createAccessibilityTree (
        XAccessible xRoot, int depth, java.awt.Point aOrigin, TreePath aPath)
    {
        TreeNode aRoot = null;

        try
        {
            aRoot = newTreeNode (xRoot, depth);
            TreePath aNewPath = aPath.pathByAddingChild (aRoot);

            if (depth >= 0)
            {
                AccessibleObject aObject = new AccessibleObject (xRoot, aNewPath);
                aRoot.aAccessibleObject = aObject;
                message ("creating accessibility tree at depth " + depth + ": "
                    + aObject.toString());

                if (maCanvas != null)
                {
                    if (aObject.getOrigin().x != 0 || aObject.getOrigin().y != 0)
                    {
                        maCanvas.addAccessible (aObject);
                        System.out.println ("adding object " + aObject.toString() + " to canvas");
                    }
                }
                else
                    System.out.println ("no canvas");

                aOrigin = aObject.getOrigin ();
            }

            //  Iterate over children and show them.
            XAccessibleContext xContext = xRoot.getAccessibleContext();
            if (xContext != null)
            {
                int n = xContext.getAccessibleChildCount();
                for (int i=0; i<n; i++)
                {
                    DefaultMutableTreeNode aNode =
                        createAccessibilityTree (xContext.getAccessibleChild(i),
                            depth+1, aOrigin, aNewPath);
                    aRoot.insert (aNode, aRoot.getChildCount());
                }
            }
            else
            {
                //                println ("object is not accessible");
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while creating accessibility tree: " + e);
        }

        return aRoot;
    }




    /** Create a new node for the JTree which represents the specified accessible object.
        The node displays selected attributes of the accessible object.
    */
    protected TreeNode newTreeNode (XAccessible xAccessible, int depth)
    {
        TreeNode aNode = null;
        try
        {
            if (xAccessible == null)
            {
                aNode = new TreeNode ("not accessible");
            }
            else
            {
                XAccessibleContext xContext = xAccessible.getAccessibleContext();

                if (xContext == null)
                {
                    aNode = new TreeNode ("no context");
                }
                else
                {
                    aNode = new TreeNode (
                        xContext.getAccessibleName());
                    aNode.add (new DefaultMutableTreeNode (
                                   "Description: "+ xContext.getAccessibleDescription()));
                    aNode.add (new DefaultMutableTreeNode (
                                   "Role: "+ xContext.getAccessibleRole()));
                    aNode.add (new DefaultMutableTreeNode (
                                   "Has parent: "+ (xContext.getAccessibleParent()!=null?
                                       "yes" : "no")));
                    aNode.add (new DefaultMutableTreeNode (
                                   "Child count: "+ xContext.getAccessibleChildCount()));

                    XAccessibleComponent xComponent =
                        (XAccessibleComponent) UnoRuntime.queryInterface (
                            XAccessibleComponent.class, xContext);
                    if (xComponent != null)
                    {
                        aNode.add (new DefaultMutableTreeNode (
                                       "Location: "+ xComponent.getLocation().X + ", "
                                       + xComponent.getLocation().Y));
                        aNode.add (new DefaultMutableTreeNode (
                                       "Location on Screen: "+ xComponent.getLocationOnScreen().X
                                       + ", "+ xComponent.getLocationOnScreen().Y));
                        aNode.add (new DefaultMutableTreeNode (
                                       "Size: "+ xComponent.getSize().Width + ", "
                                       + xComponent.getSize().Height));

                    }
                    else
                        aNode.add (new DefaultMutableTreeNode (
                                       "XAccessibleComponent not supported"));

                    XAccessibleExtendedComponent xEComponent =
                        (XAccessibleExtendedComponent) UnoRuntime.queryInterface (
                            XAccessibleExtendedComponent.class, xContext);
                    if (xEComponent != null)
                    {
                        int nColor = xEComponent.getForeground();
                        aNode.add (new DefaultMutableTreeNode (
                                       "Foreground color: R"
                                       + (nColor>>16&0xff)
                                       +"G"+ (nColor>>8&0xff)
                                       +"B"+ (nColor>>0&0xff)));
                        nColor = xEComponent.getBackground();
                        aNode.add (new DefaultMutableTreeNode (
                                       "Background color: R"
                                       + (nColor>>16&0xff)
                                       +"G"+ (nColor>>8&0xff)
                                       +"B"+ (nColor>>0&0xff)));
                    }
                    else
                        aNode.add (new DefaultMutableTreeNode (
                                       "XAccessibleExtendedComponent not supported"));

                    XAccessibleAction xAction =
                        (XAccessibleAction) UnoRuntime.queryInterface (
                            XAccessibleAction.class, xContext);
                    if (xAction != null)
                    {
                        int nActions = xAction.getAccessibleActionCount();
                        aNode.add (
                            new DefaultMutableTreeNode (
                                "Actions: " + nActions));
                        for (int i=0; i<nActions; i++)
                            aNode.add (
                                new DefaultMutableTreeNode (
                                    "Action " + i + " : " + xAction.getAccessibleActionDescription(i)));
                    }

                    XAccessibleImage xImage =
                        (XAccessibleImage) UnoRuntime.queryInterface (
                            XAccessibleImage.class, xContext);
                    if (xImage != null)
                    {
                        aNode.add (
                            new DefaultMutableTreeNode (
                                "Image description: " + xImage.getAccessibleImageDescription()));
                    }

                    XAccessibleText xText =
                        (XAccessibleText) UnoRuntime.queryInterface (
                            XAccessibleText.class, xContext);
                    if (xText != null)
                    {
                        aNode.add( newTextTreeNode( xText ) );
                    }
                }
            }

            // Use interface XServiceInfo to retrieve information about
            // supported services.
            XServiceInfo xSI = (XServiceInfo) UnoRuntime.queryInterface (
                XServiceInfo.class, xAccessible);
            if (xSI == null)
                aNode.add (new DefaultMutableTreeNode (
                               "XServiceInfo  not supported"));
            else
                aNode.add (new DefaultMutableTreeNode (
                               "Service name: " + xSI.getImplementationName ()));
        }
        catch (Exception e)
        {
            System.out.println ("caught exception in newTreeNode: " + e);
        }

        return aNode;
    }

    /** Create a node for an XAccessibleText interface. */
    private DefaultMutableTreeNode newTextTreeNode( XAccessibleText xText )
    {
        // construct XAccessibleText node with (editable) text
        DefaultMutableTreeNode aNode = new DefaultMutableTreeNode( xText );
        aNode.add( new DefaultMutableTreeNode( xText.getText() ) );

        // other methods
        int nChars = xText.getCharacterCount();
        aNode.add( new DefaultMutableTreeNode( "# chars: " + nChars ) );

        // getCharacter (as array)
        if( nChars > 30 )
            nChars = 30;
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
        aNode.add( new DefaultMutableTreeNode("getCharacters: " + aChars));

        // selection + caret
        aNode.add( new DefaultMutableTreeNode(
            "selection: " + "[" + xText.getSelectionStart() + "," +
            xText.getSelectionStart() + "] \"" + xText.getSelectedText() +
            "\"" ) );

        aNode.add( new DefaultMutableTreeNode(
            "getCaretPosition: " + xText.getCaretPosition() ) );

        // now do all the TextTypes
        aNode.add( newTextAtIndexTreeNode(
            xText, "Character", AccessibleTextType.CHARACTER ) );
        aNode.add( newTextAtIndexTreeNode(
            xText, "Word", AccessibleTextType.WORD ) );
        aNode.add( newTextAtIndexTreeNode(
            xText, "Sentence", AccessibleTextType.SENTENCE ) );
        aNode.add( newTextAtIndexTreeNode(
            xText, "Paragraph", AccessibleTextType.PARAGRAPH ) );
        aNode.add( newTextAtIndexTreeNode(
            xText, "Line", AccessibleTextType.LINE ) );

        return aNode;
    }

    /** Create a text node that lists all strings of a particular text type
     */
    private DefaultMutableTreeNode newTextAtIndexTreeNode(
        XAccessibleText xText, String sName, short nTextType)
    {
        DefaultMutableTreeNode aNode =
            new DefaultMutableTreeNode( "TextType: " + sName );

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
                        aNode.add( new DefaultMutableTreeNode(
                            "[" + nStart + "," + (i-1) + "] \"" +
                            sWord + "\"" ) );
                        sWord = sTmp;
                        nStart = i;
                    }

                    // don't generate more than 50 children.
                    if( aNode.getChildCount() > 50 )
                    {
                        sWord = "...";
                        break;
                    }
                }
                aNode.add( new DefaultMutableTreeNode(
                    "[" + nStart + "," + nLength + "] \"" + sWord + "\"" ) );
            }
            catch( IndexOutOfBoundsException e )
            {
                aNode.add( new DefaultMutableTreeNode( e.toString() ) );
            }
        }

        return aNode;
    }


    /** Expand all nodes and their subtrees that represent shapes.  Call
        this method from the outside.
    */
    public void expandShapes ()
    {
        mbFirstShapeSeen = false;
        expandShapes (maRoot);

        addTreeSelectionListener( new TreeSelectionListener() {
                public void valueChanged(TreeSelectionEvent e) {
                    System.out.println ("TreeSelectionEvent = " + e);
                    }
                }
            );

    }



    /** Expand all nodes and their subtrees that represent shapes.
    */
    protected void expandShapes (Object aRoot)
    {
        message ("Expanding shapes.");

        try
        {
            int nChildCount = maTreeModel.getChildCount(aRoot);
            // Ignore leafs.
            if (nChildCount > 0)
            {
                for (int i=0; i<nChildCount; i++)
                {
                    Object aChild = maTreeModel.getChild (aRoot, i);
                    if (TreeNode.class.isInstance (aChild))
                    {
                        TreeNode aNode = (TreeNode)aChild;
                        AccessibleObject aAccessibleObject = (AccessibleObject)aNode.aAccessibleObject;
                        if (aAccessibleObject != null)
                            // Expand every node that has one of the new OO roles.
                            if (aAccessibleObject.getRole() >= 100)
                            {
                                TreePath aPath = new TreePath (aNode.getPath());
                                expandPath (aPath);
                                if ( ! mbFirstShapeSeen)
                                {
                                    mbFirstShapeSeen = true;
                                    makeVisible (aPath);
                                }
                            }
                    }
                    // Descent into tree strucuture.
                    expandShapes (aChild);
                }
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while expanding shape nodes: " + e);
        }
    }




    public void addDocument (XAccessible xAccessible, String sURL)
    {
        if (maTreeModel != null && maRoot != null)
        {
            System.out.println ("Trying to add node at position " + maRoot.getChildCount()
                + " into root node");
            if (sURL.length() == 0)
                sURL = "<unnamed>";
            TreeNode aNode = new TreeNode ("Document: " + sURL);
            aNode.aAccessibleObject = new AccessibleObject (xAccessible,
                new TreePath (maRoot).pathByAddingChild(aNode));
            maRoot.insert (aNode, maRoot.getChildCount());
        }
        maTreeModel.reload ();
    }


    public void clear ()
    {
        maRoot.removeAllChildren();
    }


    protected void message (String message)
    {
        maMessageDisplay.message (message);
    }


    protected MessageInterface
        maMessageDisplay;

    private DefaultMutableTreeNode
        maRoot;
    private DefaultTreeModel
        maTreeModel;
    private Canvas
        maCanvas;
    private boolean
        mbFirstShapeSeen;




    /** listen to tree model changes in order to update XAccessibleText objects
     */
    class TextUpdateListener implements TreeModelListener
    {
        public void treeNodesChanged(TreeModelEvent e)
        {
            // if the change is to the first child of a DefaultMutableTreeNode
            // with an XAccessibleText child, then we call updateText
            int[] aIndices = e.getChildIndices();
            if( (aIndices != null) &&
                (aIndices.length > 0) &&
                (aIndices[0] == 0) )
            {
                // so there is a first child, check for XAccessibleText then
                DefaultMutableTreeNode aParent = (DefaultMutableTreeNode)
                    (e.getTreePath().getLastPathComponent());
                DefaultMutableTreeNode aNode = (DefaultMutableTreeNode)
                    (aParent.getChildAt(0));

                if( aParent.getUserObject() instanceof XAccessibleText)
                {
                    // joy! Call updateText with the data
                    XAccessibleText xText =
                        (XAccessibleText)aParent.getUserObject();
                    updateText( xText, aNode.toString() );

//                     // and update the tree (by deleting the old children)
//                     // (this is a work-around for the currently missing
//                     //  notifications)
//                     updateNode( xText, aParent );
                }
            }
        }

        // don't care:
        public void treeNodesInserted(TreeModelEvent e) { ; }
        public void treeNodesRemoved(TreeModelEvent e) { ; }
        public void treeStructureChanged(TreeModelEvent e) { ; }

        /** update the text */
        boolean updateText( XAccessibleText xText, String sNew )
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
}
