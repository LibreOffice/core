
import java.util.Vector;

/**
 * The node type for the AccessibleTreeModel.
 * This implements all the child-handling based on the appropriate
 * NodeHandlers. Trivial nodes can be implemented by any Object
 * type.
 */
class AccTreeNode
{
    private Vector aHandlers;       /// NodeHandlers for this node
    private Object aDataObject;     /// the actual data object
    private Object aDisplayObject;  /// object to be displayed

    public AccTreeNode( Object aData )
    {
        this( aData, aData );
    }

    public AccTreeNode( Object aData, Object aDisplay )
    {
        aHandlers = new Vector();
        aDataObject = aData;
        aDisplayObject = aDisplay;
    }

    public Object getDataObject() { return aDataObject; }
    public Object getDisplayObject() { return aDisplayObject; }

    public void addHandler( NodeHandler aHandler )
    {
        aHandlers.add( aHandler );
    }


    /** iterate over handlers and return child sum */
    public int getChildCount()
    {
        int nRet = 0;
        for(int i = 0; i < aHandlers.size(); i++)
        {
            nRet += ((NodeHandler)aHandlers.get(i)).
                getChildCount( aDataObject );
        }
        return nRet;
    }

    /** iterate over handlers until the child is found */
    public Object getChild(int nIndex)
    {
        if( nIndex >= 0 )
        {
            for(int i = 0; i < aHandlers.size(); i++)
            {
                // check if this handler has the child, and if not
                // search with next handler
                NodeHandler aHandler = (NodeHandler)aHandlers.get(i);
                int nCount = aHandler.getChildCount( aDataObject );
                if( nCount > nIndex )
                    return aHandler.getChild( aDataObject, nIndex );
                else
                    nIndex -= nCount;
            }
        }

        // nothing found?
        return null;
    }

    /** this node is a leaf if have no handlers, or is those
            handlers show no children */
    public boolean isLeaf()
    {
        return (aHandlers.size() == 0) || (getChildCount() == 0);
    }

    public boolean equals(Object aOther)
    {
        return (this == aOther) || aDataObject.equals( aOther );
    }

    public String toString()
    {
        return aDisplayObject.toString();
    }
}
