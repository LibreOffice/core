/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
import com.sun.star.embed.XStorage;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XDriver;
import connectivity.tools.HsqlDatabase;

public class HsqlDriverTest extends ComplexTestCase {


    @Override
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
            HsqlDatabase database = new HsqlDatabase( param.getMSF() );
            ds = database.getDataSource().getXDataSource();
        } catch(Exception ex) {
            throw new RuntimeException("factory: unable to construct data source", ex );
        }

        try{
            XDocumentSubStorageSupplier doc = UnoRuntime.queryInterface(XDocumentSubStorageSupplier.class,ds);
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
            XStorable mod = UnoRuntime.queryInterface(XStorable.class,ds);
            mod.store();
            XComponent xComp = UnoRuntime.queryInterface(XComponent.class,stor);
            if ( xComp != null )
                xComp.dispose();
        } catch(Exception e){}

        try{
            XDocumentSubStorageSupplier doc = UnoRuntime.queryInterface(XDocumentSubStorageSupplier.class,ds);
            XModel mod = UnoRuntime.queryInterface(XModel.class,ds);
            XStorage stor = doc.getDocumentSubStorage("database",4);
            com.sun.star.beans.PropertyValue[] info = new com.sun.star.beans.PropertyValue[]{
                new com.sun.star.beans.PropertyValue("Storage",0,stor,PropertyState.DIRECT_VALUE)
                ,new com.sun.star.beans.PropertyValue("URL",0,mod.getURL(),PropertyState.DIRECT_VALUE)
            };
            XDriver drv = UnoRuntime.queryInterface(XDriver.class,param.getMSF().createInstance("com.sun.star.sdbcx.comp.hsqldb.Driver"));


            TestCacheSize test = new TestCacheSize(info,drv);

            StopWatch     sw   = new StopWatch();

            try{
                test.setUp();
                test.testFillUp();
                test.checkResults();
                test.tearDown();
                System.out.println("Total Test Time: " + sw.elapsedTime());
            } catch(Exception e){}

            try{
                XStorable mod2 = UnoRuntime.queryInterface(XStorable.class,ds);
                mod2.store();
            } catch(Exception e){}
        }catch(Exception e){}
    }

    public void test2(){
        System.gc();

        try{
            com.sun.star.beans.PropertyValue[] info = new com.sun.star.beans.PropertyValue[]{
                new com.sun.star.beans.PropertyValue("JavaDriverClass",0,"org.hsqldb.jdbcDriver",PropertyState.DIRECT_VALUE)
                ,new com.sun.star.beans.PropertyValue("ParameterNameSubstitution",0, false,PropertyState.DIRECT_VALUE)
            };
            XDriver drv = UnoRuntime.queryInterface(XDriver.class,param.getMSF().createInstance("com.sun.star.comp.sdbc.JDBCDriver"));
            TestCacheSize test = new TestCacheSize(info,drv);
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
