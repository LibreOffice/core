package integration.forms;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;

/** provides global helpers
*/
public class dbfTools
{
    /* ------------------------------------------------------------------ */
    /** disposes the component given
    */
    static public void disposeComponent( Object xComp ) throws java.lang.RuntimeException
    {
        XComponent xComponent = queryXComponent( xComp );
        if ( null != xComponent )
            xComponent.dispose();
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XPropertySet interface
    */
    static public XPropertySet queryXPropertySet( Object aComp )
    {
        return (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XIndexContainer interface
    */
    static public XIndexContainer queryXIndexContainer( Object aComp )
    {
        return (XIndexContainer)UnoRuntime.queryInterface( XIndexContainer.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XComponent interface
    */
    static public XComponent queryXComponent( Object aComp )
    {
        return (XComponent)UnoRuntime.queryInterface( XComponent.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the parent of the given object
    */
    static Object getParent( Object aComponent, Class aInterfaceClass )
    {
        XChild xAsChild = (XChild)UnoRuntime.queryInterface( XChild.class, aComponent );
        return UnoRuntime.queryInterface( aInterfaceClass, xAsChild.getParent() );
    }
};
