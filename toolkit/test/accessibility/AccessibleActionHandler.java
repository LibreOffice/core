
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.lang.IndexOutOfBoundsException;

class AccessibleActionHandler extends NodeHandler
{
    protected XAccessibleAction getAction(Object aObject)
    {
        return (XAccessibleAction) UnoRuntime.queryInterface (
            XAccessibleAction.class, aObject);
    }

    public int getChildCount(Object aObject)
    {
        XAccessibleAction xAction = getAction(aObject);
        return (xAction == null) ? 0 : 1 + xAction.getAccessibleActionCount();
    }

    public Object getChild(Object aObject, int nIndex)
    {
        Object aRet = null;

        XAccessibleAction xAction = getAction(aObject);
        if( xAction != null )
        {
            if( nIndex == 1 )
                aRet = "Actions: " + xAction.getAccessibleActionCount();
            else
            {
                try
                {
                    aRet = "Action " + (nIndex-1) + " : " +
                        xAction.getAccessibleActionDescription(nIndex-1);
                }
                catch( IndexOutOfBoundsException e )
                {
                    aRet = "ERROR";
                }
            }
        }

        return aRet;
    }
}
