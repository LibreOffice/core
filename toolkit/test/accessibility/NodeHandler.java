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


    //
    // The following methods support editing of children and actions.
    // They have default implementations for no actions and read-only.
    //

    /** May this child be changed? */
    public boolean isChildEditable(Object aObject, int nIndex)
    {
        return false;
    }

    /** change this child's value */
    public void setChild(Object aObject, int nIndex) { }


    /** get names of suported actions */
    public String[] getActions(Object aObject)
    {
        return new String[] {};
    }

    /** perform action */
    public void performAction(Object aObject, int nIndex) { }
}
