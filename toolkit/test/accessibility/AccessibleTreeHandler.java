import drafts.com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.IndexOutOfBoundsException;


/**
 * Map the tree of accessibility objects into their
 * AccessibilityTreeModel counterparts.
 */
class AccessibleTreeHandler extends NodeHandler
{
    protected XAccessibleContext getContext(Object aObject)
    {
        XAccessibleContext xContext =
            (XAccessibleContext) UnoRuntime.queryInterface (
                 XAccessibleContext.class, aObject);
        return xContext;
    }

    public int getChildCount(Object aObject)
    {
        XAccessibleContext aContext = getContext(aObject);
        return (aContext == null) ? 0 : aContext.getAccessibleChildCount();
    }

    public Object getChild(Object aObject, int nIndex)
    {
        Object aRet = null;
        XAccessibleContext aContext = getContext(aObject);
        if( aContext != null )
        {
            try
            {
                aRet = AccessibilityTreeModel.
                    createDefaultNode(aContext.getAccessibleChild(nIndex));
            }
            catch( IndexOutOfBoundsException e )
            { } // return null
        }
        return aRet;
    }
}
