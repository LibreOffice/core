import com.sun.star.accessibility.XAccessibleContext;

interface IAccessibleObjectDisplay
{

    /** Set the accessible object to display.  Call this method e.g. when a
        new object has been focused.
    */
    public void setAccessibleObject (XAccessibleContext xContext);

    public void updateAccessibleObject (XAccessibleContext xContext);
}
