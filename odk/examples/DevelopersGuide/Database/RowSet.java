/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.beans.XPropertySet;
import com.sun.star.sdbc.*;
import com.sun.star.sdbcx.Privilege;
import com.sun.star.sdb.XRowSetApproveBroadcaster;

public class RowSet
{
    private static XComponentContext xContext = null;
    private static XMultiComponentFactory xMCF = null;
    public static void main(String argv[]) throws java.lang.Exception
    {
        try {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
            xMCF = xContext.getServiceManager();
        }
        catch( Exception e) {
            System.err.println("ERROR: can't get a component context from a running office ...");
            e.printStackTrace(System.err);
            System.exit(1);
        }

        try{
            showRowSetEvents();
            showRowSetRowCount();
            showRowSetPrivileges();
            useRowSet();
        }
        catch(com.sun.star.uno.Exception e)
        {
            System.err.println(e);
            e.printStackTrace();
        }
        System.exit(0);
    }



    private static void useRowSet() throws com.sun.star.uno.Exception
    {
        // first we create our RowSet object
        XRowSet xRowRes = UnoRuntime.queryInterface(
            XRowSet.class,
            xMCF.createInstanceWithContext("com.sun.star.sdb.RowSet", xContext));

        System.out.println("RowSet created!");
        // set the properties needed to connect to a database
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");
        xProp.setPropertyValue("Command","biblio");
        xProp.setPropertyValue("CommandType",Integer.valueOf(com.sun.star.sdb.CommandType.TABLE));

        xRowRes.execute();
        System.out.println("RowSet executed!");


        XComponent xComp = UnoRuntime.queryInterface(XComponent.class,xRowRes);
        xComp.dispose();
        System.out.println("RowSet destroyed!");
    }

    private static void showRowSetPrivileges() throws com.sun.star.uno.Exception
    {
        // first we create our RowSet object
        XRowSet xRowRes = UnoRuntime.queryInterface(
            XRowSet.class,
            xMCF.createInstanceWithContext("com.sun.star.sdb.RowSet", xContext));

        System.out.println("RowSet created!");
        // set the properties needed to connect to a database
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");
        xProp.setPropertyValue("Command","biblio");
        xProp.setPropertyValue("CommandType",Integer.valueOf(com.sun.star.sdb.CommandType.TABLE));

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
        XComponent xComp = UnoRuntime.queryInterface(XComponent.class,xRowRes);
        xComp.dispose();
        System.out.println("RowSet destroyed!");
    }

    private static void showRowSetRowCount() throws com.sun.star.uno.Exception
    {
        // first we create our RowSet object
        XRowSet xRowRes = UnoRuntime.queryInterface(
            XRowSet.class,
            xMCF.createInstanceWithContext("com.sun.star.sdb.RowSet", xContext));

        System.out.println("RowSet created!");
        // set the properties needed to connect to a database
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");
        xProp.setPropertyValue("Command","biblio");
        xProp.setPropertyValue("CommandType",Integer.valueOf(com.sun.star.sdb.CommandType.TABLE));

        xRowRes.execute();
        System.out.println("RowSet executed!");

        // now look if the RowCount is already final
        System.out.println("The RowCount is final: " + xProp.getPropertyValue("IsRowCountFinal"));

        XResultSet xRes = UnoRuntime.queryInterface(XResultSet.class,xRowRes);
        xRes.last();

        System.out.println("The RowCount is final: " + xProp.getPropertyValue("IsRowCountFinal"));
        System.out.println("There are " + xProp.getPropertyValue("RowCount") + " rows!");

        // now destroy the RowSet
        XComponent xComp = UnoRuntime.queryInterface(XComponent.class,xRowRes);
        xComp.dispose();
        System.out.println("RowSet destroyed!");
    }

    private static void showRowSetEvents() throws com.sun.star.uno.Exception
    {
        // first we create our RowSet object
        XRowSet xRowRes = UnoRuntime.queryInterface(
            XRowSet.class,
            xMCF.createInstanceWithContext("com.sun.star.sdb.RowSet", xContext));

        System.out.println("RowSet created!");
        // add our Listener
        System.out.println("Append our Listener!");
        RowSetEventListener pRow = new RowSetEventListener();
        XRowSetApproveBroadcaster xApBroad = UnoRuntime.queryInterface(XRowSetApproveBroadcaster.class,xRowRes);
        xApBroad.addRowSetApproveListener(pRow);
        xRowRes.addRowSetListener(pRow);

        // set the properties needed to connect to a database
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");
        xProp.setPropertyValue("Command","biblio");
        xProp.setPropertyValue("CommandType",Integer.valueOf(com.sun.star.sdb.CommandType.TABLE));

        xRowRes.execute();
        System.out.println("RowSet executed!");

        // do some movements to check if we got all notifications
        XResultSet xRes = UnoRuntime.queryInterface(XResultSet.class,xRowRes);
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
        XComponent xComp = UnoRuntime.queryInterface(XComponent.class,xRowRes);
        xComp.dispose();
        System.out.println("RowSet destroyed!");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
