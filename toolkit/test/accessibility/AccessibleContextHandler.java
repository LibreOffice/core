import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XIndexAccess;
import java.util.HashMap;

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
            maChildList.setSize (4);
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
                    int nRole = xContext.getAccessibleRole();
                    sChild = "Role: " + nRole + " (" + NameProvider.getRoleName(nRole) + ")";
                    break;
                case 2:
                    XAccessible xParent = xContext.getAccessibleParent();
                    sChild = "Has parent: " + (xParent!=null ? "yes" : "no");
                    /*                    if (xParent != ((AccTreeNode)aParent).getAccessible())
                    {
                        sChild += " but that is inconsistent"
                            + "#" + xParent + " # " + ((AccTreeNode)aParent).getAccessible();
                    }
                    */
                    break;
                case 3:
                    sChild = "";
                    XAccessibleStateSet xStateSet =
                        xContext.getAccessibleStateSet();
                    if (xStateSet != null)
                    {
                        for (short i=0; i<=30; i++)
                        {
                            if (xStateSet.contains (i))
                            {
                                if (sChild.compareTo ("") != 0)
                                    sChild += ", ";
                                sChild += NameProvider.getStateName(i);
                            }
                        }
                    }
                    else
                        sChild += "no state set";
                    sChild = "State set: " + sChild;

                    /*                case 3:
                    sChild = "Child count: " + xContext.getAccessibleChildCount();
                    break;*/
            }
        }
        return new StringNode (sChild, aParent);
    }
}
