import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.IndexOutOfBoundsException;


/**
 * Map the tree of accessibility objects into their
 * AccessibilityTreeModel counterparts.
 */
class AccessibleTreeHandler
    extends NodeHandler
{
    protected XAccessibleContext mxContext;

    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        if (xContext != null)
            return new AccessibleTreeHandler (xContext);
        else
            return null;
    }

    public AccessibleTreeHandler ()
    {
        super();
        mxContext = null;
    }

    public AccessibleTreeHandler (XAccessibleContext xContext)
    {
        super();
        mxContext = xContext;
        if (mxContext != null)
            // Add one to the number of children to include the string node
            // that tells you how many children there are.
            maChildList.setSize (1 + mxContext.getAccessibleChildCount());
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        AccessibleTreeNode aChild = null;
        if (mxContext != null)
        {
            if (nIndex == 0)
                aChild = new StringNode ("Child count: " + mxContext.getAccessibleChildCount(),
                    aParent);
            else
            {
                // Lower index to skip the string node.
                nIndex -= 1;
                try
                {
                    XAccessible xChild = mxContext.getAccessibleChild (nIndex);
                    aChild = NodeFactory.Instance().createDefaultNode (
                        xChild, aParent);
                }
                catch( IndexOutOfBoundsException e )
                {
                    aChild = new StringNode ("ERROR: no child with index " + nIndex, aParent);
                }
            }
        }
        else
            aChild = new StringNode ("XAccessibleContext interface not supported", aParent);
        return aChild;
    }

    /** Try to add the specified accessible child into the lists of
        children.  The insertion position is determined from the
        getIndexInParent method of the child.
    */
    public AccessibleTreeNode addAccessibleChild (AccessibleTreeNode aParent, XAccessible xChild)
    {
        AccessibleTreeNode aChild = null;

        XAccessibleContext xContext = xChild.getAccessibleContext();
        int nIndex = xContext.getAccessibleIndexInParent() + 1;
        if ((nIndex >= 0) || (nIndex <= maChildList.size()))
        {
            aChild = NodeFactory.Instance().createDefaultNode (xChild, aParent);
            maChildList.insertElementAt (aChild, nIndex);
        }
        return aChild;
    }


    /** Update only the child count node.  Trust on other ways to update the
        accessible children.
    */
    public void update (AccessibleTreeNode aNode)
    {
        maChildList.setElementAt (null, 0);
    }
}
