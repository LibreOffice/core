import java.util.Vector;


/**
 * Map an arbitrary object into parts of a tree node.
 */
abstract class NodeHandler
{
    /** This vector is used as cache for the child objects.
    */
    protected Vector maChildList;


    public abstract NodeHandler createHandler (
        com.sun.star.accessibility.XAccessibleContext xContext);

    public NodeHandler ()
    {
        maChildList = new Vector ();
    }

    /** Clear the cache of child objects.
    */
    public void clear ()
    {
        synchronized (maChildList)
        {
            maChildList = new Vector ();
        }
    }

    /** This factory method creates an individual handler for the specified
        object that may hold information to accelerate the access to its children.
    */
    //    public abstract NodeHandler createHandler (Object aObject);

    /** return the number of children this object has */
    public int getChildCount(Object aObject)
    {
        synchronized (maChildList)
        {
            return maChildList.size();
        }
    }

    /**
     * return a child object. Complex
     * children have to be AccTreeNode instances.
     * @see AccTreeNode
     */
    public AccessibleTreeNode getChild (AccessibleTreeNode aParent, int nIndex)
    {
        synchronized (maChildList)
        {
            AccessibleTreeNode aChild = (AccessibleTreeNode)maChildList.get(nIndex);
            if (aChild == null)
            {
                aChild = createChild (aParent, nIndex);
                if (aChild == null)
                    aChild = new StringNode ("could not create child", aParent);
                maChildList.setElementAt (aChild, nIndex);
            }
            return aChild;
        }
    }

    public AccessibleTreeNode getChildNoCreate (AccessibleTreeNode aParent, int nIndex)
    {
        synchronized (maChildList)
        {
            return (AccessibleTreeNode)maChildList.get(nIndex);
        }
    }

    /** Remove the specified child from the list of children.
    */
    public boolean removeChild (AccessibleTreeNode aNode, int nIndex)
    {
        try
        {
            synchronized (maChildList)
            {
                System.out.println ("    removing child at position " + nIndex + ": "
                    + maChildList.elementAt (nIndex));
                maChildList.remove (nIndex);
            }
        }
        catch (Exception e)
        {
            return false;
        }
        return true;
    }

    public int indexOf (AccessibleTreeNode aNode)
    {
        synchronized (maChildList)
        {
            return maChildList.indexOf (aNode);
        }
    }

    /** Create a child object for the specified data.  This method is called
        usually from getChild and put there into the cache.
    */
    public abstract AccessibleTreeNode createChild (
        AccessibleTreeNode aParent, int nIndex);

    //
    // The following methods support editing of children and actions.
    // They have default implementations for no actions and read-only.
    //

    /** May this child be changed? */
    public boolean isChildEditable (AccessibleTreeNode aNode, int nIndex)
    {
        return false;
    }

    /** change this child's value */
    //    public void setChild(Object aObject, int nIndex) { }


    /** get names of suported actions */
    public String[] getActions (AccessibleTreeNode aNode)
    {
        return new String[] {};
    }

    /** perform action */
    public void performAction (AccessibleTreeNode aNode, int nIndex)
    {
    }

    /** Update all children.
    */
    public void update (AccessibleTreeNode aNode)
    {
    }
}
