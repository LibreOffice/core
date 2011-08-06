/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package complex.connectivity;

import complex.connectivity.hsqldb.TestCacheSize;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;

import com.sun.star.lang.*;
import com.sun.star.document.XDocumentSubStorageSupplier;
import complexlib.ComplexTestCase;


import org.hsqldb.lib.StopWatch;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.PropertyState;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.embed.XStorage;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XDriver;
import connectivity.tools.HsqlDatabase;

public class HsqlDriverTest extends ComplexTestCase {


    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    @Override
    public String getTestObjectName() {
        return "DriverTest";
    }

    public void assurePublic(String sMessage,boolean check){
        super.assure(sMessage,check);
    }

    public void test(){
        XDataSource ds = null;
        System.gc();
        try {
            HsqlDatabase database = new HsqlDatabase( (XMultiServiceFactory)param.getMSF() );
            ds = database.getDataSource().getXDataSource();
        } catch(Exception ex) {
            throw new RuntimeException("factory: unable to construct data source" );
        }

        try{
            XDocumentSubStorageSupplier doc = (XDocumentSubStorageSupplier)UnoRuntime.queryInterface(XDocumentSubStorageSupplier.class,ds);
            XStorage stor = doc.getDocumentSubStorage("database",4);
            try{
            if ( stor.isStreamElement("db.log") )
                stor.removeElement("db.log");
            } catch(Exception e){}
            try{
            if ( stor.isStreamElement("db.properties") )
                stor.removeElement("db.properties");
            } catch(Exception e){}
            try{
            if ( stor.isStreamElement("db.script") )
                stor.removeElement("db.script");
            } catch(Exception e){}
            try{
            if ( stor.isStreamElement("db.script.new") )
                stor.removeElement("db.script.new");
            } catch(Exception e){}
            XStorable mod = (XStorable)UnoRuntime.queryInterface(XStorable.class,ds);
            mod.store();
            XComponent xComp = (XComponent)UnoRuntime.queryInterface(XComponent.class,stor);
            if ( xComp != null )
                xComp.dispose();
        } catch(Exception e){}

        com.sun.star.beans.PropertyValue[] info = null;
        XDriver drv = null;
        try{
            XDocumentSubStorageSupplier doc = (XDocumentSubStorageSupplier)UnoRuntime.queryInterface(XDocumentSubStorageSupplier.class,ds);
            XModel mod = (XModel)UnoRuntime.queryInterface(XModel.class,ds);
            XStorage stor = doc.getDocumentSubStorage("database",4);
            info = new com.sun.star.beans.PropertyValue[]{
                new com.sun.star.beans.PropertyValue("Storage",0,stor,PropertyState.DIRECT_VALUE)
                ,new com.sun.star.beans.PropertyValue("URL",0,mod.getURL(),PropertyState.DIRECT_VALUE)
            };
            drv = (XDriver)UnoRuntime.queryInterface(XDriver.class,((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.sdbcx.comp.hsqldb.Driver"));


            TestCacheSize test = new TestCacheSize(((XMultiServiceFactory)param.getMSF()),info,drv);

            StopWatch     sw   = new StopWatch();

            try{
                test.setUp();
                test.testFillUp();
                test.checkResults();
                test.tearDown();
                System.out.println("Total Test Time: " + sw.elapsedTime());
            } catch(Exception e){}

            try{
                XStorable mod2 = (XStorable)UnoRuntime.queryInterface(XStorable.class,ds);
                mod2.store();
            } catch(Exception e){}
        }catch(Exception e){}
    }
    public void test2(){
        System.gc();

        com.sun.star.beans.PropertyValue[] info = null;
        XDriver drv = null;
        try{
            info = new com.sun.star.beans.PropertyValue[]{
                new com.sun.star.beans.PropertyValue("JavaDriverClass",0,"org.hsqldb.jdbcDriver",PropertyState.DIRECT_VALUE)
                ,new com.sun.star.beans.PropertyValue("ParameterNameSubstitution",0, false,PropertyState.DIRECT_VALUE)
            };
            drv = (XDriver)UnoRuntime.queryInterface(XDriver.class,((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.comp.sdbc.JDBCDriver"));
            TestCacheSize test = new TestCacheSize(((XMultiServiceFactory)param.getMSF()),info,drv);
            test.setURL("jdbc:hsqldb:g:\\hsql\\db");


            StopWatch     sw   = new StopWatch();

            try{
                test.setUp();
                test.testFillUp();
                test.checkResults();
                test.tearDown();
                System.out.println("Total Test Time: " + sw.elapsedTime());
            } catch(Exception e){}
        }catch(Exception e){}
    }
}
