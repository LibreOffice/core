/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package mod._sw;

import com.sun.star.beans.XPropertySet;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextEmbeddedObjectsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


public class SwXTextEmbeddedObjects extends TestCase {

    XTextDocument oDoc;

    /**
     * in general this method creates a testdocument
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
     *
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            oDoc = SOF.createTextDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occured.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn?t create document", e );
        }
    }

    /**
     * in general this method disposes the testenvironment and document
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
     *
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(oDoc);
    }


    /**
     *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    public TestEnvironment createTestEnvironment( TestParameters tParam,
                  PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        // create testobject here
        XTextCursor xCursor = oDoc.getText().createTextCursor();
        try {
            XMultiServiceFactory xMultiServiceFactory = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, oDoc);
            Object o = xMultiServiceFactory.createInstance("com.sun.star.text.TextEmbeddedObject" );
            XTextContent xTextContent = (XTextContent)UnoRuntime.queryInterface(XTextContent.class, o);
            String sChartClassID = "12dcae26-281f-416f-a234-c3086127382e";
            XPropertySet xPropertySet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, xTextContent);
            xPropertySet.setPropertyValue( "CLSID", sChartClassID );

            oDoc.getText().insertTextContent( xCursor, xTextContent, false );
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
        }

        XTextEmbeddedObjectsSupplier oTEOS = (XTextEmbeddedObjectsSupplier)
            UnoRuntime.queryInterface(XTextEmbeddedObjectsSupplier.class, oDoc);

        oObj = oTEOS.getEmbeddedObjects();

          TestEnvironment tEnv = new TestEnvironment( oObj );
        return tEnv;

        } // finish method getTestEnvironment

}    // finish class SwXTextEmbeddedObjects

