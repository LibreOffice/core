
import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleImage;


class AccessibleImageHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleImage xImage =
            (XAccessibleImage) UnoRuntime.queryInterface (
                XAccessibleImage.class, xContext);
        if (xImage != null)
            return new AccessibleImageHandler (xImage);
        else
            return null;
    }

    public AccessibleImageHandler ()
    {
    }

    public AccessibleImageHandler (XAccessibleImage xImage)
    {
        if (xImage != null)
            maChildList.setSize (1);
    }

    protected static XAccessibleImage getImage (AccTreeNode aNode)
    {
        return (XAccessibleImage) UnoRuntime.queryInterface (
            XAccessibleImage.class, aNode.getContext());
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        if (aParent instanceof AccTreeNode)
        {
            XAccessibleImage xImage = getImage ((AccTreeNode)aParent);
            if (xImage != null)
                return new StringNode (
                    "Image: " +
                    xImage.getAccessibleImageDescription() + " (" +
                    xImage.getAccessibleImageWidth() + "x" +
                    xImage.getAccessibleImageHeight() + ")",
                    aParent);
        }
        return null;
    }
}
