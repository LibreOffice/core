/*************************************************************************
 *
 *  $RCSfile: TypeDetection.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change:$Date: 2004-12-10 17:03:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
import com.sun.star.uno.AnyConverter;
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
        //util.DesktopTools.closeDoc(xTextDoc);
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
            fileURL = fileURL + "/bookmarks.oot";

            XStorable store = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDoc);
            System.out.println(fileURL);
            store.storeToURL(fileURL, new PropertyValue[0]);

        } catch( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn³t create Bookmark", e );
        }

        tEnv.addObjRelation("XContainerQuery.createSubSetEnumerationByProperties",
            querySequenze);
        tEnv.addObjRelation("XTypeDetection.bookmarkDoc", fileURL+"#bookmark");

        return tEnv;
    } // finish method getTestEnvironment


    protected void setPropertyValueValue(PropertyValue[] props, String pName, Object pValue) {
        int i = 0;
        System.out.println(props[i].Name);
        while (i < props.length && !props[i].Name.equals(pName)) {
            i++;
            System.out.println(props[i].Name);
        }
        props[i].Value = pValue;
    }

}

