
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
                        com.sun.star.awt.Point aLocation = xComponent.getLocation();
                        aChild = new StringNode (
                            "Location: " + aLocation.X + ", " + aLocation.Y,
                            aParent);
                        break;
                    case 1:
                        com.sun.star.awt.Point aScreenLocation = xComponent.getLocationOnScreen();
                        aChild = new StringNode (
                            "Location on Screen: " + aScreenLocation.X + ", " + aScreenLocation.Y,
                            aParent);
                        break;
                    case 2:
                        com.sun.star.awt.Size aSize = xComponent.getSize();
                        aChild = new StringNode (
                            "Size: "+ aSize.Width + ", " + aSize.Height,
                            aParent);
                        break;
                    case 3:
                        com.sun.star.awt.Rectangle aBBox = xComponent.getBounds();
                        aChild = new StringNode (
                            "Bounding Box: "+ aBBox.X + ", " + aBBox.Y + ","
                            + aBBox.Width + ", " + aBBox.Height,
                            aParent);
                        break;
                }
            }
        }
        return aChild;
    }

    public void update (AccessibleTreeNode aNode)
    {
        maChildList.clear();
        if (aNode instanceof AccTreeNode)
            if (((AccTreeNode)aNode).getComponent() != null)
                maChildList.setSize (4);
    }
}
