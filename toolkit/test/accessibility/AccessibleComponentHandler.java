
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;


class AccessibleComponentHandler
    extends NodeHandler
{

    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleComponent xComponent = getComponent (xContext);
        if (xComponent != null)
            return new AccessibleComponentHandler (xComponent);
        else
            return null;

    }

    public AccessibleComponentHandler ()
    {
    }

    public AccessibleComponentHandler (XAccessibleComponent xComponent)
    {
        if (xComponent != null)
            maChildList.setSize (3);
    }

    private static XAccessibleComponent getComponent(Object aObject)
    {
        return (XAccessibleComponent) UnoRuntime.queryInterface (
            XAccessibleComponent.class, aObject);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        AccessibleTreeNode aChild = null;
        if (aParent instanceof AccTreeNode)
        {
            XAccessibleComponent xComponent = getComponent (
                ((AccTreeNode)aParent).getContext());

            if (xComponent != null)
            {
                switch (nIndex)
                {
                    case 0:
                        aChild = new StringNode ("Location: "+ xComponent.getLocation().X +
                            ", " + xComponent.getLocation().Y, aParent);
                        break;
                    case 1:
                        aChild = new StringNode ("Location on Screen: "
                            + xComponent.getLocationOnScreen().X + ", "
                            + xComponent.getLocationOnScreen().Y,
                            aParent);
                        break;
                    case 2:
                        aChild = new StringNode ("Size: "+ xComponent.getSize().Width + ", "
                            + xComponent.getSize().Height,
                            aParent);
                        break;
                }
            }
        }
        return aChild;
    }
}
