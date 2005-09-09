/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXTextEmbeddedObjects.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:55:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package mod._sw;

import com.sun.star.beans.XPropertySet;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.utils;

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
            // Some exception occures.FAILED
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

