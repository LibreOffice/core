
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleTable;


class AccessibleTableHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleTable xTable =
            (XAccessibleTable) UnoRuntime.queryInterface (
                XAccessibleTable.class, xContext);
        if (xTable != null)
            return new AccessibleTableHandler (xTable);
        else
            return null;
    }

    public AccessibleTableHandler ()
    {
    }

    public AccessibleTableHandler (XAccessibleTable xTable)
    {
        if (xTable != null)
            maChildList.setSize (1);
    }

    protected static XAccessibleTable getTable(Object aObject)
    {
        return (XAccessibleTable) UnoRuntime.queryInterface (
            XAccessibleTable.class, aObject);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        return new StringNode ("interface XAccessibleTable is supported", aParent);
    }
}
