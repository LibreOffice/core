
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;


class AccessibleComponentHandler
    extends NodeHandler
{

    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleComponent xComponent =
            (XAccessibleComponent) UnoRuntime.queryInterface (
                XAccessibleComponent.class, xContext);
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
            maChildList.setSize (4);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        AccessibleTreeNode aChild = null;
        if (aParent instanceof AccTreeNode)
        {
            XAccessibleComponent xComponent =
                ((AccTreeNode)aParent).getComponent();

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
                    case 3:
                        boolean bVisible = xComponent.isVisible();
                        boolean bShowing = xComponent.isShowing();
                        String sText;
                        if (bVisible && bShowing)
                            sText = new String ("visible and showing");
                        else if (bVisible)
                            sText = new String ("visible but not showing");
                        else if (bShowing)
                            sText = new String ("showing but not visible");
                        else
                            sText = new String ("neither visible nor showing");
                        aChild = new StringNode (sText, aParent);
                        break;
                }
            }
        }
        return aChild;
    }
}
