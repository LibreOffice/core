import java.io.*;

import com.sun.star.comp.helper.RegistryServiceFactory;
import com.sun.star.comp.servicemanager.ServiceManager;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XComponent;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.*;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbc.*;
import com.sun.star.sdbcx.Privilege;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XRowSetApproveBroadcaster;

public class RowSet
{
    public static XMultiServiceFactory rSmgr;
    public static void main(String argv[]) throws java.lang.Exception
    {
        try{
            rSmgr = connect("socket,host=localhost,port=8100");
            showRowSetEvents(rSmgr);
            //  showRowSetRowCount(rSmgr);
            //  showRowSetPrivileges(rSmgr);
            //  useRowSet(rSmgr);
        }
        catch(com.sun.star.uno.Exception e)
        {
            System.out.println(e);
            e.printStackTrace();
        }
        System.exit(0);
    }


    public static XMultiServiceFactory connect( String connectStr )
        throws com.sun.star.uno.Exception,
        com.sun.star.uno.RuntimeException, java.lang.Exception
    {
        // initial serviceManager
        XMultiServiceFactory xLocalServiceManager =
            com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();

        // create a connector, so that it can contact the office
        Object  xUrlResolver  = xLocalServiceManager.createInstance( "com.sun.star.bridge.UnoUrlResolver" );
        XUnoUrlResolver urlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface(
            XUnoUrlResolver.class, xUrlResolver );

        Object rInitialObject = urlResolver.resolve( "uno:" + connectStr + ";urp;StarOffice.NamingService" );

        XNamingService rName = (XNamingService)UnoRuntime.queryInterface(
            XNamingService.class, rInitialObject );

        XMultiServiceFactory xMSF = null;
        if( rName != null ) {
            System.err.println( "got the remote naming service !" );
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager" );

            xMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface( XMultiServiceFactory.class, rXsmgr );
        }

        return ( xMSF );
    }

    public static void printDataSources(XMultiServiceFactory _rMSF) throws com.sun.star.uno.Exception
    {
        // create a DatabaseContext and print all DataSource names
        XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class,
                                            _rMSF.createInstance("com.sun.star.sdb.DatabaseContext"));
        String aNames [] = xNameAccess.getElementNames();
        for(int i=0;i<aNames.length;++i)
            System.out.println(aNames[i]);
    }

    public static void useRowSet(XMultiServiceFactory _rMSF) throws com.sun.star.uno.Exception
    {
        // first we create our RowSet object
        XRowSet xRowRes = (XRowSet)UnoRuntime.queryInterface(XRowSet.class,
                                            _rMSF.createInstance("com.sun.star.sdb.RowSet"));

        System.out.println("RowSet created!");
        // set the properties needed to connect to a database
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");
        xProp.setPropertyValue("Command","biblio");
        xProp.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.TABLE));

        xRowRes.execute();
        System.out.println("RowSet executed!");


        XComponent xComp = (XComponent)UnoRuntime.queryInterface(XComponent.class,xRowRes);
        xComp.dispose();
        System.out.println("RowSet destroyed!");
    }

    public static void showRowSetPrivileges(XMultiServiceFactory _rMSF) throws com.sun.star.uno.Exception
    {
        // first we create our RowSet object
        XRowSet xRowRes = (XRowSet)UnoRuntime.queryInterface(XRowSet.class,
                                            _rMSF.createInstance("com.sun.star.sdb.RowSet"));

        System.out.println("RowSet created!");
        // set the properties needed to connect to a database
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");
        xProp.setPropertyValue("Command","biblio");
        xProp.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.TABLE));

        xRowRes.execute();
        System.out.println("RowSet executed!");

        Integer aPriv = (Integer)xProp.getPropertyValue("Privileges");
        int nPriv  = aPriv.intValue();
        if( (nPriv & Privilege.SELECT) == Privilege.SELECT)
            System.out.println("SELECT");
        if( (nPriv & Privilege.INSERT) == Privilege.INSERT)
            System.out.println("INSERT");
        if( (nPriv & Privilege.UPDATE) == Privilege.UPDATE)
            System.out.println("UPDATE");
        if( (nPriv & Privilege.DELETE) == Privilege.DELETE)
            System.out.println("DELETE");

        // now destroy the RowSet
        XComponent xComp = (XComponent)UnoRuntime.queryInterface(XComponent.class,xRowRes);
        xComp.dispose();
        System.out.println("RowSet destroyed!");
    }

    public static void showRowSetRowCount(XMultiServiceFactory _rMSF) throws com.sun.star.uno.Exception
    {
        // first we create our RowSet object
        XRowSet xRowRes = (XRowSet)UnoRuntime.queryInterface(XRowSet.class,
                                            _rMSF.createInstance("com.sun.star.sdb.RowSet"));

        System.out.println("RowSet created!");
        // set the properties needed to connect to a database
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");
        xProp.setPropertyValue("Command","biblio");
        xProp.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.TABLE));

        xRowRes.execute();
        System.out.println("RowSet executed!");

        // now look if the RowCount is already final
        System.out.println("The RowCount is final: " + xProp.getPropertyValue("IsRowCountFinal"));

        XResultSet xRes = (XResultSet)UnoRuntime.queryInterface(XResultSet.class,xRowRes);
        xRes.last();

        System.out.println("The RowCount is final: " + xProp.getPropertyValue("IsRowCountFinal"));
        System.out.println("There are " + xProp.getPropertyValue("RowCount") + " rows!");

        // now destroy the RowSet
        XComponent xComp = (XComponent)UnoRuntime.queryInterface(XComponent.class,xRowRes);
        xComp.dispose();
        System.out.println("RowSet destroyed!");
    }

    public static void showRowSetEvents(XMultiServiceFactory _rMSF) throws com.sun.star.uno.Exception
    {
        // first we create our RowSet object
        XRowSet xRowRes = (XRowSet)UnoRuntime.queryInterface(XRowSet.class,
                                            _rMSF.createInstance("com.sun.star.sdb.RowSet"));

        System.out.println("RowSet created!");
        // add our Listener
        System.out.println("Append our Listener!");
        RowSetEventListener pRow = new RowSetEventListener();
        XRowSetApproveBroadcaster xApBroad = (XRowSetApproveBroadcaster)UnoRuntime.queryInterface(XRowSetApproveBroadcaster.class,xRowRes);
        xApBroad.addRowSetApproveListener(pRow);
        xRowRes.addRowSetListener(pRow);

        // set the properties needed to connect to a database
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");
        xProp.setPropertyValue("Command","biblio");
        xProp.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.TABLE));

        xRowRes.execute();
        System.out.println("RowSet executed!");

        // do some movements to check if we got all notifications
        XResultSet xRes = (XResultSet)UnoRuntime.queryInterface(XResultSet.class,xRowRes);
        System.out.println("beforeFirst");
        xRes.beforeFirst();
        // this should lead to no notifications because
        // we should stand before the first row at the beginning
        System.out.println("We stand before the first row: " + xRes.isBeforeFirst());

        System.out.println("next");
        xRes.next();
        System.out.println("next");
        xRes.next();
        System.out.println("last");
        xRes.last();
        System.out.println("next");
        xRes.next();
        System.out.println("We stand after the last row: " + xRes.isAfterLast());
        System.out.println("first");
        xRes.first();
        System.out.println("previous");
        xRes.previous();
        System.out.println("We stand before the first row: " + xRes.isBeforeFirst());
        System.out.println("afterLast");
        xRes.afterLast();
        System.out.println("We stand after the last row: " + xRes.isAfterLast());

        // now destroy the RowSet
        XComponent xComp = (XComponent)UnoRuntime.queryInterface(XComponent.class,xRowRes);
        xComp.dispose();
        System.out.println("RowSet destroyed!");
    }
}

