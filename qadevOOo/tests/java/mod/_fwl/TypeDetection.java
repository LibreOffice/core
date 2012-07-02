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

package mod._fwl;

import com.sun.star.beans.NamedValue;
import com.sun.star.container.XNameAccess;
import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XStorable;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import util.SOfficeFactory;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.document.TypeDetection</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XContainerQuery</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XNameContainer</code></li>
 *  <li> <code>com::sun::star::container::XNameReplace</code></li>
 *  <li> <code>com::sun::star::document::XTypeDetection</code></li>
 *  <li> <code>com::sun::star::util::XFlushable</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.container.XContainerQuery
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XNameContainer
 * @see com.sun.star.container.XNameReplace
 * @see com.sun.star.document.XTypeDetection
 * @see com.sun.star.util.XFlushable
 * @see ifc.container._XContainerQuery
 * @see ifc.container._XElementAccess
 * @see ifc.container._XNameAccess
 * @see ifc.container._XNameContainer
 * @see ifc.container._XNameReplace
 * @see ifc.document._XTypeDetection
 * @see ifc.util._XFlushable
 */
public class TypeDetection extends TestCase {

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    XTextDocument xTextDoc = null;

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.document.TypeDetection</code>. <p>
    */
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null ;

        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.document.TypeDetection") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get service");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get TypeDetection", e );
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException(Status.failed("Service wasn't created")) ;
        }

        oObj = (XInterface) oInterface ;
        log.println("ImplName: "+utils.getImplName(oObj));

        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        XNameAccess xNA = UnoRuntime.queryInterface
            (XNameAccess.class, oObj);
        String[] elementNames = xNA.getElementNames();
        String elementName = elementNames[0];
        Object[] instance = null;;
        try{
            instance = (Object[]) xNA.getByName(elementName);
        } catch (com.sun.star.container.NoSuchElementException e){
            throw new StatusException(
            Status.failed("Couldn't get elements from object"));
        } catch (com.sun.star.lang.WrappedTargetException e){
            throw new StatusException(
            Status.failed("Couldn't get elements from object"));
        }

        log.println("adding INSTANCE 1 as obj relation to environment");

        setPropertyValueValue((PropertyValue[])instance, "Preferred", "INSTANCE1");
        tEnv.addObjRelation("INSTANCE" +1, instance);

        // com.sun.star.container.XContainerQuery
        NamedValue[] querySequenze = new NamedValue[1];
        NamedValue query = new NamedValue();
        query.Name = "Name";
        query.Value = "generic_Text";
        querySequenze[0] = query;


        log.println("create text document with bookmarks");
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF() );
        String fileURL = null;
        try {
            xTextDoc = SOF.createTextDoc( null );
            XInterface xBookMark = SOF.createBookmark( xTextDoc );
            SOF.insertTextContent( xTextDoc, (XTextContent) xBookMark );

            fileURL = utils.getOfficeTemp((XMultiServiceFactory)Param.getMSF() );
            fileURL = fileURL + "bookmarks.oot";

            XStorable store = UnoRuntime.queryInterface(XStorable.class, xTextDoc);
            System.out.println(fileURL);
            store.storeToURL(fileURL, new PropertyValue[0]);

        } catch( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create Bookmark", e );
        }

        tEnv.addObjRelation("XContainerQuery.createSubSetEnumerationByProperties",
            querySequenze);
        tEnv.addObjRelation("XTypeDetection.bookmarkDoc", fileURL+"#bookmark");

        return tEnv;
    } // finish method getTestEnvironment


    protected void setPropertyValueValue(PropertyValue[] props, String pName, Object pValue) {
        int i = 0;
        while (i < props.length && !props[i].Name.equals(pName)) {
            i++;
        }
        props[i].Value = pValue;
    }

}

