
import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleHyperlink;


class AccessibleHyperlinkHandler extends AccessibleTreeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleHyperlink xLink =
            (XAccessibleHyperlink) UnoRuntime.queryInterface (
                XAccessibleHyperlink.class, xContext);
        if (xLink != null)
            return new AccessibleHyperlinkHandler (xLink);
        else
            return null;
    }

    public AccessibleHyperlinkHandler ()
    {
    }

    public AccessibleHyperlinkHandler (XAccessibleHyperlink xLink)
    {
        if (xLink != null)
            maChildList.setSize (1);
    }

    protected XAccessibleHyperlink getHyperlink(Object aObject)
    {
        XAccessibleHyperlink xHyperlink =
            (XAccessibleHyperlink) UnoRuntime.queryInterface (
                 XAccessibleHyperlink.class, aObject);
        return xHyperlink;
    }

    public AccessibleTreeNode getChild (AccessibleTreeNode aParent, int nIndex)
    {
        return new StringNode ("interface XAccessibleHyperlink is supported", aParent);
    }
}
