
import drafts.com.sun.star.accessibility.XAccessibleContext;


class AccessibleContextHandler extends AccessibleTreeHandler
{
    public int getChildCount(Object aObject)
    {
        return (getContext(aObject) == null) ? 0 : 4;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        XAccessibleContext xContext = getContext(aObject);

        Object aRet = null;
        if( xContext != null )
        {
            switch( nIndex )
            {
                case 0:
                    aRet = "Description: " +
                        xContext.getAccessibleDescription();
                    break;
                case 1:
                    aRet = "Role: " + xContext.getAccessibleRole();
                    break;
                case 2:
                    aRet = "Has parent: " +
                        (xContext.getAccessibleParent()!=null ? "yes" : "no");
                    break;
                case 3:
                    aRet = "Child count: " + xContext.getAccessibleChildCount();
                    break;
            }
        }
        return aRet;
    }
}
