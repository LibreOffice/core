
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleEditableText;


class AccessibleEditableTextHandler extends NodeHandler
{
    protected XAccessibleEditableText getEText(Object aObject)
    {
        return (XAccessibleEditableText) UnoRuntime.queryInterface (
            XAccessibleEditableText.class, aObject);
    }

    public int getChildCount(Object aObject)
    {
        return (getEText(aObject) == null) ? 0 : 1;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        XAccessibleEditableText xContext = getEText(aObject);
        return "XAccessibleEditableText is supported";
    }
}
