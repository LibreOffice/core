
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleTable;


class AccessibleTableHandler extends NodeHandler
{
    protected XAccessibleTable getTable(Object aObject)
    {
        return (XAccessibleTable) UnoRuntime.queryInterface (
            XAccessibleTable.class, aObject);
    }

    public int getChildCount(Object aObject)
    {
        return (getTable(aObject) == null) ? 0 : 1;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        XAccessibleTable xTable = getTable(aObject);
        return "interface XAccessibleTable is supported";
    }
}
