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

package ifc.io;

import lib.MultiMethodTest;
import util.ValueComparer;
import util.dbg;
import util.utils;

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XObjectInputStream;
import com.sun.star.io.XObjectOutputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XPersistObject;
import com.sun.star.uno.UnoRuntime;


/**
* Testing <code>com.sun.star.io.XPersistObject</code>
* interface methods :
* <ul>
*  <li><code> getServiceName()</code></li>
*  <li><code> write()</code></li>
*  <li><code> read()</code></li>
* </ul> <p>
* This test need the following object relations :
* <ul>
*  <li> <code>'OBJNAME'</code> : <code>String</code> value that
*   contains service name which object represents.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XPersistObject
* @see com.sun.star.io.XObjectInputStream
* @see com.sun.star.io.XObjectOutputStream
*/
public class _XPersistObject extends MultiMethodTest {

    public XPersistObject    oObj = null;
    XObjectInputStream iStream = null;
    XObjectOutputStream oStream = null;
    String sname = null;

    boolean    result = true;


    /**
    * Test calls the method and checks return value. <p>
    * Has <b> OK </b> status if the method returns proper service names
    * which is equal to <code>'OBJNAME'</code> relation value. <p>
    */
    public void _getServiceName() {
        result = true;
        sname = oObj.getServiceName();
        log.println("Method returned '" + sname + "'") ;
        String objName = (String)tEnv.getObjRelation("OBJNAME");
        if (objName == null) {
            log.println("No OBJNAME relation!");
            result = false;
        } else {
            result &= sname.equals(objName);
            if (!result)
                log.println("Name of object must be '" + objName +
                            "' but returned name is '" + sname +"'");
        }

        tRes.tested("getServiceName()", result);
    }

    /**
    * Creates service get by <code>getServiceName</code> method and tries
    * to read object written to stream by <code>write</code> method test.
    * Then properties of object written and object read are compared. <p>
    * Has <b>OK</b> status if all properties of two objects are equal
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getServiceName() </code> : to have service name
    *   which has to be created </li>
    *  <li> <code> write() </code> : to write object tested into stream</li>
    * </ul>
    */
    public void _read() {
        requiredMethod("getServiceName()");
        requiredMethod("write()") ;

        boolean bResult = true;

        try {
            Object noPS = tEnv.getObjRelation("noPS");
            if ( noPS == null) {
                XPropertySet objps = UnoRuntime.queryInterface(
                    XPropertySet.class, oObj);
                XPropertySetInfo objpsi = objps.getPropertySetInfo();
                Property[] objprops = objpsi.getProperties();

                Object oCopy = tParam.getMSF().createInstance(sname);

                XPersistObject persCopy = UnoRuntime.queryInterface(XPersistObject.class, oCopy);

                persCopy.read(iStream);

                XPropertySet copyps = UnoRuntime.queryInterface(
                                                    XPropertySet.class, oCopy);

                XPropertySetInfo copypsi = copyps.getPropertySetInfo();
                Property[] copyprops = copypsi.getProperties();

                for (int i = 0; i < copyprops.length; i++) {
                    Object cps = copyps.getPropertyValue(copyprops[i].Name);
                    Object ops = objps.getPropertyValue(objprops[i].Name);
                    boolean locRes = ( (ValueComparer.equalValue(cps,ops)) ||
                                    (utils.isVoid(cps) && utils.isVoid(ops)) );

                    //transient properties aren't stored
                    if (isTransient(objprops[i])) locRes = true;

                    Object pseudo = tEnv.getObjRelation("PSEUDOPERSISTENT");
                    if ( (pseudo != null) && !locRes) {
                        String str = copyprops[i].Name;
                        locRes = ( (str.equals("Time")) || (str.equals("Date"))
                                            || (str.equals("FormatsSupplier"))
                                            || (str.equals("Text"))
                                            || (str.equals("Value"))
                                            || (str.indexOf("UserDefined")>0)
                                            );
                    }
                    if (!locRes) {
                        log.println("Property '" + copyprops[i].Name
                            + "' failed");
                        dbg.printPropertyInfo(objps, objprops[i].Name, log);
                        dbg.printPropertyInfo(copyps, copyprops[i].Name, log);
                    }
                    bResult &= locRes;
                }
            } else {
                Object oCopy = tParam.getMSF().createInstance(sname);
                XPersistObject persCopy = UnoRuntime.queryInterface(XPersistObject.class, oCopy);

                persCopy.read(iStream);

                bResult = persCopy.getServiceName().equals(sname);

            }

        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occurred : ");
            e.printStackTrace(log) ;
            bResult = false;
        }

        tRes.tested("read()", bResult);
    }

    /**
    * Test calls the method and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _write() {
        boolean bResult = true;
        try {
            initPipe();
            oObj.write(oStream);
        } catch (com.sun.star.io.IOException e) {
            log.println("Exception occurred while test. " + e);
            bResult = false;
        }
        tRes.tested("write()", bResult);
    }


    /**
    * Creates the following stream scheme <code>
    * ObjectOutputStream -> Pipe -> ObjectInputStream </code> for writing/reading
    * object.
    */
    protected void initPipe() {
        try {
            Object aPipe = tParam.getMSF().createInstance
                ("com.sun.star.io.Pipe");
            Object istream = tParam.getMSF().createInstance
                ("com.sun.star.io.ObjectInputStream");
            Object ostream = tParam.getMSF().createInstance
                ("com.sun.star.io.ObjectOutputStream");

            // Now the objects that aren't described anywhere
            Object mistream = tParam.getMSF().createInstance
                ("com.sun.star.io.MarkableInputStream");
            Object mostream = tParam.getMSF().createInstance
                ("com.sun.star.io.MarkableOutputStream");

            XActiveDataSink xdSi = UnoRuntime.queryInterface(XActiveDataSink.class, istream);
            XActiveDataSource xdSo = UnoRuntime.queryInterface(XActiveDataSource.class, ostream);
            XActiveDataSink xdSmi = UnoRuntime.queryInterface(XActiveDataSink.class, mistream);
            XActiveDataSource xdSmo = UnoRuntime.queryInterface(XActiveDataSource.class, mostream);

            XInputStream miStream = UnoRuntime.queryInterface(XInputStream.class, mistream);
            XOutputStream moStream = UnoRuntime.queryInterface(XOutputStream.class, mostream);
            XInputStream PipeIn = UnoRuntime.queryInterface(XInputStream.class, aPipe);
            XOutputStream PipeOut = UnoRuntime.queryInterface(XOutputStream.class,aPipe);

            xdSi.setInputStream(miStream);
            xdSo.setOutputStream(moStream);
            xdSmi.setInputStream(PipeIn);
            xdSmo.setOutputStream(PipeOut);

            iStream = UnoRuntime.queryInterface(XObjectInputStream.class, istream);
            oStream = UnoRuntime.queryInterface(XObjectOutputStream.class, ostream);


        } catch (com.sun.star.uno.Exception e) {
            System.out.println("exc " + e);
        }

    }

    public static boolean isTransient(Property prop) {
        short attr = prop.Attributes;
        return ((attr & com.sun.star.beans.PropertyAttribute.TRANSIENT) != 0);
    }


}


