
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleEditableText;


class AccessibleEditableTextHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleEditableText xText =
            (XAccessibleEditableText) UnoRuntime.queryInterface (
                XAccessibleEditableText.class, xContext);
        if (xText != null)
            return new AccessibleEditableTextHandler (xText);
        else
            return null;
    }

    public AccessibleEditableTextHandler ()
    {
    }

    public AccessibleEditableTextHandler (XAccessibleEditableText xText)
    {
        if (xText != null)
            maChildList.setSize (1);
    }

    protected static XAccessibleEditableText getEText (AccTreeNode aNode)
    {
        return (XAccessibleEditableText) UnoRuntime.queryInterface (
            XAccessibleEditableText.class, aNode.getContext());
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        return new StringNode ("XAccessibleEditableText is supported", aParent);
    }
}
