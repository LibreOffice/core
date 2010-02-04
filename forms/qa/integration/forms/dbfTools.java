package integration.forms;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;

/** provides global helpers
*/
class dbfTools
{
    /* ------------------------------------------------------------------ */
    /** disposes the component given
    */
    static public void disposeComponent( Object xComp ) throws java.lang.RuntimeException
    {
        XComponent xComponent = queryComponent( xComp );
        if ( null != xComponent )
            xComponent.dispose();
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XPropertySet interface
    */
    static public XPropertySet queryPropertySet( Object aComp )
    {
        return UnoRuntime.queryInterface( XPropertySet.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XIndexContainer interface
    */
    static public XIndexContainer queryIndexContainer( Object aComp )
    {
        return UnoRuntime.queryInterface( XIndexContainer.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XComponent interface
    */
    static public XComponent queryComponent( Object aComp )
    {
        return UnoRuntime.queryInterface( XComponent.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the parent of the given object
    */
    @SuppressWarnings("unchecked")
    static Object getParent( Object aComponent, Class aInterfaceClass )
    {
        XChild xAsChild = UnoRuntime.queryInterface( XChild.class, aComponent );
        return UnoRuntime.queryInterface( aInterfaceClass, xAsChild.getParent() );
    }
};
