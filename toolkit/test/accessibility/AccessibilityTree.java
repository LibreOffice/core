import AccessibleObject;
import MessageInterface;

import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleExtendedComponent;
import drafts.com.sun.star.accessibility.XAccessibleAction;
import drafts.com.sun.star.accessibility.XAccessibleImage;
import drafts.com.sun.star.accessibility.XAccessibleRelationSet;
import drafts.com.sun.star.accessibility.XAccessibleStateSet;

import com.sun.star.lang.XServiceInfo;
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
    public static void main (String args[])
    {
        new AccessibilityWorkBench ();
    }




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
}
