import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.uno.UnoRuntime;


class AccessibleContextHandler
    extends NodeHandler
{
    protected int nChildrenCount;

    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        if (xContext != null)
            return new AccessibleContextHandler (xContext);
        else
            return null;
    }

    public AccessibleContextHandler ()
    {
        super ();
    }

    public AccessibleContextHandler (XAccessibleContext xContext)
    {
        super();
        if (xContext != null)
            maChildList.setSize (3);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        XAccessibleContext xContext = null;
        if (aParent instanceof AccTreeNode)
            xContext = ((AccTreeNode)aParent).getContext();

        String sChild = new String();
        if (xContext != null)
        {
            switch( nIndex )
            {
                case 0:
                    sChild = "Description: " +
                        xContext.getAccessibleDescription();
                    break;
                case 1:
                    sChild = "Role: " + xContext.getAccessibleRole();
                    break;
                case 2:
                    sChild = "Has parent: " +
                        (xContext.getAccessibleParent()!=null ? "yes" : "no");
                    break;
                    /*                case 3:
                    sChild = "Child count: " + xContext.getAccessibleChildCount();
                    break;*/
            }
        }
        return new StringNode (sChild, aParent);
    }
}
