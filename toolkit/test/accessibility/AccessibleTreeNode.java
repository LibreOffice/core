import java.util.Vector;
import com.sun.star.lang.IndexOutOfBoundsException;

/**
    Base class for all tree nodes.
 */
class AccessibleTreeNode
{
    /// The parent node.  It is null for the root node.
    protected AccessibleTreeNode maParent;

    /// The object to be displayed.
    private Object maDisplayObject;

    public AccessibleTreeNode (Object aDisplayObject, AccessibleTreeNode aParent)
    {
        maDisplayObject = aDisplayObject;
        maParent = aParent;
    }

    public void update ()
    {
        // Empty
    }

    public AccessibleTreeNode getParent ()
    {
        return maParent;
    }

    public Object getDisplayObject ()
    {
        return maDisplayObject;
    }

    public int getChildCount ()
    {
        return 0;
    }

    public AccessibleTreeNode getChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        throw new IndexOutOfBoundsException();
    }

    public AccessibleTreeNode getChildNoCreate (int nIndex)
        throws IndexOutOfBoundsException
    {
        throw new IndexOutOfBoundsException();
    }

    public boolean removeChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        throw new IndexOutOfBoundsException();
    }

    public int indexOf (AccessibleTreeNode aNode)
    {
        return -1;
    }

    /** Create a path to this node by first asking the parent for its path
        and then appending this object.
    */
    public void createPath (java.util.Vector aPath)
    {
        if (maParent != null)
            maParent.createPath (aPath);
        aPath.add (this);
    }

    public Object[] createPath ()
    {
        Vector aPath = new Vector (1);
        createPath (aPath);
        return aPath.toArray();
    }

    public boolean isLeaf()
    {
        return true;
    }

    public String toString()
    {
        return maDisplayObject.toString();
    }

    /** get names of suported actions */
    public String[] getActions ()
    {
        return new String[] {};
    }

    /** perform action */
    public void performAction (int nIndex)
    {
    }
}
