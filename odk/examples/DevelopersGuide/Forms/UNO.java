import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.awt.*;
import com.sun.star.container.*;
import com.sun.star.form.*;
import com.sun.star.lang.*;
import com.sun.star.sdb.*;
import com.sun.star.sdbcx.*;

public class UNO
{
    public static XPropertySet queryPropertySet( Object aObject )
    {
        return (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, aObject );
    }
    public static XControlModel queryControlModel( Object aObject )
    {
        return (XControlModel)UnoRuntime.queryInterface( XControlModel.class, aObject );
    }
    public static XIndexContainer queryIndexContainer( Object aObject )
    {
        return (XIndexContainer)UnoRuntime.queryInterface( XIndexContainer.class, aObject );
    }
    public static XReset queryReset( Object aObject )
    {
        return (XReset)UnoRuntime.queryInterface( XReset.class, aObject );
    }
    public static XServiceInfo queryServiceInfo( Object aObject )
    {
        return (XServiceInfo)UnoRuntime.queryInterface( XServiceInfo.class, aObject );
    }
    public static XColumn queryColumn( Object aObject )
    {
        return (XColumn)UnoRuntime.queryInterface( XColumn.class, aObject );
    }

    public static XComponent queryComponent( Object aObject )
    {
        return (XComponent)UnoRuntime.queryInterface( XComponent.class, aObject );
    }

    public static XTablesSupplier queryTablesSupplier( Object aObject )
    {
        return (XTablesSupplier)UnoRuntime.queryInterface( XTablesSupplier.class, aObject );
    }

/*  replace Foo with the identifier of your choice.

    Why does Java not have templates?

    public static XFoo queryFoo( Object aObject )
    {
        return (XFoo)UnoRuntime.queryInterface( XFoo.class, aObject );
    }
*/
};
