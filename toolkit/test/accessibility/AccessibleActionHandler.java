
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.lang.IndexOutOfBoundsException;

class AccessibleActionHandler
    extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleAction xEComponent =
            (XAccessibleAction) UnoRuntime.queryInterface (
                XAccessibleAction.class, xContext);
        if (xEComponent != null)
            return new AccessibleActionHandler (xEComponent);
        else
            return null;
    }

    public AccessibleActionHandler ()
    {
    }

    public AccessibleActionHandler (XAccessibleAction xAction)
    {
        if (xAction != null)
            maChildList.setSize (1 + xAction.getAccessibleActionCount());
    }

    protected static XAccessibleAction getAction (AccTreeNode aParent)
    {
        return (XAccessibleAction) UnoRuntime.queryInterface (
            XAccessibleAction.class, aParent.getContext());
    }

    public AccessibleTreeNode createChild (
        AccessibleTreeNode aParent,
        int nIndex)
    {
        AccessibleTreeNode aChild = null;

        if (aParent instanceof AccTreeNode)
        {
            XAccessibleAction xAction = getAction ((AccTreeNode)aParent);
            if( xAction != null )
            {
                if (nIndex == 0)
                    aChild = new StringNode ("Number of actions: " + xAction.getAccessibleActionCount(),
                        aParent);
                else
                {
                    nIndex -= 1;
                    try
                    {
                        aChild = new AccessibleActionNode (
                            "Action " + nIndex + " : "
                            + xAction.getAccessibleActionDescription (nIndex),
                            aParent,
                            nIndex);
                    }
                    catch( IndexOutOfBoundsException e )
                    {
                        aChild = new StringNode ("ERROR", aParent);
                    }
                }
            }
        }

        return aChild;
    }
}
