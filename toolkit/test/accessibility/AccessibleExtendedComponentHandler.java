
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessibleExtendedComponent;


class AccessibleExtendedComponentHandler extends AccessibleTreeHandler
{
    private XAccessibleExtendedComponent getComponent(Object aObject)
    {
        return (XAccessibleExtendedComponent) UnoRuntime.queryInterface (
            XAccessibleExtendedComponent.class, aObject);
    }


    public int getChildCount(Object aObject)
    {
        return (getComponent(aObject) == null) ? 0 : 2;
    }

    public Object getChild(Object aObject, int nIndex)
    {
        XAccessibleExtendedComponent xEComponent = getComponent(aObject);

        Object aRet = null;
        if( xEComponent != null )
        {
            int nColor;
            switch( nIndex )
            {
                case 0:
                    nColor = xEComponent.getForeground();
                    aRet = "Foreground color: R"
                        +       (nColor>>16&0xff)
                        + "G" + (nColor>>8&0xff)
                        + "B" + (nColor>>0&0xff);
                    break;
                case 1:
                    nColor = xEComponent.getBackground();
                    aRet = "Background color: R"
                        +       (nColor>>16&0xff)
                        + "G" + (nColor>>8&0xff)
                        + "B" + (nColor>>0&0xff);
                    break;
            }
        }
        return aRet;
    }
}
