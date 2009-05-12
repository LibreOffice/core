/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TypeDetection.java,v $
 * $Revision: 1.12 $
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
 * @see ifc.util.XFlushable
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

        XNameAccess xNA = (XNameAccess) UnoRuntime.queryInterface
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
        query.Value = "writer_Text";
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

            XStorable store = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDoc);
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

