
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleImage;


class AccessibleImageHandler extends NodeHandler
{
    protected XAccessibleImage getImage(Object aObject)
    {
        return (XAccessibleImage) UnoRuntime.queryInterface (
            XAccessibleImage.class, aObject);
    }

    public int getChildCount(Object aObject)
    {
        return (getImage(aObject) == null) ? 0 : 1;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        XAccessibleImage xImage = getImage(aObject);
        return (xImage == null) ? "" :
            "Image description: " + xImage.getAccessibleImageDescription();
    }
}
