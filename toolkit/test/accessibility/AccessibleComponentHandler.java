
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleComponent;


class AccessibleComponentHandler extends AccessibleTreeHandler
{
    private XAccessibleComponent getComponent(Object aObject)
    {
        return (XAccessibleComponent) UnoRuntime.queryInterface (
            XAccessibleComponent.class, aObject);
    }

    public int getChildCount(Object aObject)
    {
        return (getComponent(aObject) == null) ? 0 : 3;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        XAccessibleComponent xComponent = getComponent(aObject);

        Object aRet = null;
        if( xComponent != null )
        {
            switch( nIndex )
            {
                case 0:
                    aRet = "Location: "+ xComponent.getLocation().X +
                        ", " + xComponent.getLocation().Y;
                    break;
                case 1:
                    aRet = "Location on Screen: "+
                        xComponent.getLocationOnScreen().X + ", " +
                        xComponent.getLocationOnScreen().Y;
                    break;
                case 2:
                    aRet =  "Size: "+ xComponent.getSize().Width + ", " +
                        xComponent.getSize().Height;
                    break;
            }
        }
        return aRet;
    }
}
