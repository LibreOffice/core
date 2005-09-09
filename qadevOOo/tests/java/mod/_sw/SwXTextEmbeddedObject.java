/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXTextEmbeddedObject.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:54:42 $
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

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextEmbeddedObjectsSupplier;
import com.sun.star.text.XTextFrame;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
 *
 * initial description
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.document.XEmbeddedObjectSupplier
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.text.TextEmbeddedObject
 * @see com.sun.star.text.XTextContent
 * @see com.sun.star.text.XTextEmbeddedObject
 *
 */
public class SwXTextEmbeddedObject extends TestCase {
    XTextDocument xTextDoc;

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
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                        XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
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
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;

        // create testobject here
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) tParam.getMSF());
        try {
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't open document", e);
        }

        XTextCursor xCursor = xTextDoc.getText().createTextCursor();
        try {
            XMultiServiceFactory xMultiServiceFactory = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
            Object o = xMultiServiceFactory.createInstance("com.sun.star.text.TextEmbeddedObject" );
            XTextContent xTextContent = (XTextContent)UnoRuntime.queryInterface(XTextContent.class, o);
            String sChartClassID = "12dcae26-281f-416f-a234-c3086127382e";
            XPropertySet xPropertySet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, xTextContent);
            xPropertySet.setPropertyValue( "CLSID", sChartClassID );

            xTextDoc.getText().insertTextContent( xCursor, xTextContent, false );
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
        }

        XTextEmbeddedObjectsSupplier oTEOS = (XTextEmbeddedObjectsSupplier) UnoRuntime.queryInterface(
                                                     XTextEmbeddedObjectsSupplier.class,
                                                     xTextDoc);

        XNameAccess oEmObj = oTEOS.getEmbeddedObjects();
        XIndexAccess oEmIn = (XIndexAccess) UnoRuntime.queryInterface(
                                     XIndexAccess.class, oEmObj);

        try {
            oObj = (XInterface) AnyConverter.toObject(
                           new Type(XInterface.class), oEmIn.getByIndex(0));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get Object", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("NoAttach", "SwXTextEmbeddedObject");

        XTextFrame aFrame = SOF.createTextFrame(xTextDoc, 500, 500);
        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();
        XTextContent the_content = (XTextContent) UnoRuntime.queryInterface(
                                           XTextContent.class, aFrame);

        try {
            oText.insertTextContent(oCursor, the_content, true);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't insert frame " + e.getMessage());
        }

        tEnv.addObjRelation("TextFrame", aFrame);

        tEnv.addObjRelation("NoSetSize", "SwXTextEmbeddedObject");
        tEnv.addObjRelation("NoPos", "SwXTextEmbeddedObject");

        return tEnv;
    } // finish method getTestEnvironment
} // finish class SwXTextEmbeddedObject
