
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleHypertext;


class AccessibleHypertextHandler extends AccessibleTreeHandler
{
    protected XAccessibleHypertext getHypertext(Object aObject)
    {
        XAccessibleHypertext xHypertext =
            (XAccessibleHypertext) UnoRuntime.queryInterface (
                 XAccessibleHypertext.class, aObject);
        return xHypertext;
    }

    public int getChildCount(Object aObject)
    {
        return (getHypertext(aObject) == null) ? 0 : 1;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        return "interface XAccessibleHypertext is supported";
    }
}
