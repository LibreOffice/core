
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleHyperlink;


class AccessibleHyperlinkHandler extends AccessibleTreeHandler
{
    protected XAccessibleHyperlink getHyperlink(Object aObject)
    {
        XAccessibleHyperlink xHyperlink =
            (XAccessibleHyperlink) UnoRuntime.queryInterface (
                 XAccessibleHyperlink.class, aObject);
        return xHyperlink;
    }

    public int getChildCount(Object aObject)
    {
        return (getHyperlink(aObject) == null) ? 0 : 1;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        return "interface XAccessibleHyperlink is supported";
    }
}
