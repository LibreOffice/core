/**
 * Map an arbitrary object into parts of a tree node.
 */
abstract class NodeHandler
{
    /** return the number of children this object has */
    public abstract int getChildCount(Object aObject);

    /**
     * return a child object.
     * You can use any object type for trivial nodes. Complex
     * children have to be AccTreeNode instances.
     * @see AccTreeNode
     */
    public abstract Object getChild(Object aObject, int nIndex);
}
