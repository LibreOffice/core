
import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleHypertext;


class AccessibleHypertextHandler extends AccessibleTreeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleHypertext xText =
            (XAccessibleHypertext) UnoRuntime.queryInterface (
                XAccessibleHypertext.class, xContext);
        if (xText != null)
            return new AccessibleHypertextHandler (xText);
        else
            return null;
    }

    public AccessibleHypertextHandler ()
    {
    }

    public AccessibleHypertextHandler (XAccessibleHypertext xText)
    {
        if (xText != null)
            maChildList.setSize (1);
    }

    protected static XAccessibleHypertext getHypertext (AccTreeNode aNode)
    {
        XAccessibleHypertext xHypertext =
            (XAccessibleHypertext) UnoRuntime.queryInterface (
                 XAccessibleHypertext.class, aNode.getContext());
        return xHypertext;
    }

    public AccessibleTreeNode getChild (AccessibleTreeNode aParent, int nIndex)
    {
        return new StringNode ("interface XAccessibleHypertext is supported", aParent);
    }
}
