/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DriverTest.java,v $
 * $Revision: 1.4 $
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
package qa.drivers.hsqldb;

import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.sdbc.*;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.frame.XStorable;

import com.sun.star.lang.*;
import com.sun.star.document.XDocumentSubStorageSupplier;
import complexlib.ComplexTestCase;

import java.io.PrintWriter;

import util.utils;
import java.util.*;
import java.io.*;
import org.hsqldb.jdbcDriver;
import qa.drivers.hsqldb.DatabaseMetaData;
import org.hsqldb.lib.StopWatch;
import com.sun.star.sdbc.*;
import com.sun.star.container.XNameAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.embed.XStorage;

public class DriverTest extends ComplexTestCase {


    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public String getTestObjectName() {
        return "DriverTest";
    }

    public void assurePublic(String sMessage,boolean check){
        addResult(sMessage,check);
    }

    public void test(){
        mThreadTimeOut = 10000000;
        XDataSource ds = null;
        System.gc();
        try {
            XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class,((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.sdb.DatabaseContext"));
            ds = (XDataSource)UnoRuntime.queryInterface(XDataSource.class,xNameAccess.getByName("file:///g:/test.odb"));
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
        mThreadTimeOut = 10000000;
        System.gc();

        com.sun.star.beans.PropertyValue[] info = null;
        XDriver drv = null;
        try{
            info = new com.sun.star.beans.PropertyValue[]{
                new com.sun.star.beans.PropertyValue("JavaDriverClass",0,"org.hsqldb.jdbcDriver",PropertyState.DIRECT_VALUE)
                ,new com.sun.star.beans.PropertyValue("ParameterNameSubstitution",0,new Boolean(false),PropertyState.DIRECT_VALUE)
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
