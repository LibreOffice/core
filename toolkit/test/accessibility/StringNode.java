import com.sun.star.lang.IndexOutOfBoundsException;

/**
    Base class for all tree nodes.
 */
class StringNode
    extends AccessibleTreeNode
{
    public StringNode (String aDisplayObject, AccessibleTreeNode aParent)
    {
        super (aDisplayObject, aParent);
    }
}
