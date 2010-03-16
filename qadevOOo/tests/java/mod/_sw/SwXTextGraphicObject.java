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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.TextContentAnchorType;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class SwXTextGraphicObject extends TestCase {

    XTextDocument xTextDoc;

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
            log.println( "creating a textdoc" );
            xTextDoc = SOF.createTextDoc( null );;
        } catch ( Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
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
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
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
    protected TestEnvironment createTestEnvironment
                (TestParameters tParam, PrintWriter log) {

            XInterface oObj = null;
            Object oGObject = null;
            Object xTextFrame = null;
            SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

            Object instance = null;

            try {
                oGObject = SOF.createInstance
                    (xTextDoc,"com.sun.star.text.GraphicObject");
                instance = SOF.createInstance
                    (xTextDoc,"com.sun.star.text.GraphicObject");
                xTextFrame = SOF.createTextFrame(xTextDoc, 500, 500);
            }
            catch (Exception ex) {
                log.println("Couldn't create instance");
                ex.printStackTrace(log);
                throw new StatusException("Couldn't create instance", ex );
            }

            oObj = (XInterface) oGObject;

            XText the_text = xTextDoc.getText();
            XTextCursor the_cursor = the_text.createTextCursor();
            XTextContent the_content = (XTextContent)
                UnoRuntime.queryInterface(XTextContent.class,oObj);

            log.println("inserting Frame");
            try{
                XTextContent Framecontent = (XTextContent) UnoRuntime.queryInterface(
                                                   XTextContent.class, xTextFrame);
                the_text.insertTextContent(the_cursor, Framecontent, true);
            } catch (Exception e) {
                System.out.println("Couldn't insert text frame");
                e.printStackTrace();
                throw new StatusException("Couldn't insert text frame", e );
            }


           log.println( "inserting graphic" );
            try {
                the_text.insertTextContent(the_cursor,the_content,true);
            } catch (Exception e) {
                System.out.println("Couldn't insert Content");
                e.printStackTrace();
                throw new StatusException("Couldn't insert Content", e );
            }

            log.println( "adding graphic" );
            XPropertySet oProps = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class,oObj);
            try {
                String wat = util.utils.getFullTestURL("space-metal.jpg");
                oProps.setPropertyValue("AnchorType",
                    TextContentAnchorType.AT_PARAGRAPH);
                oProps.setPropertyValue("GraphicURL",wat);
                oProps.setPropertyValue("HoriOrientPosition",new Integer(5500));
                oProps.setPropertyValue("VertOrientPosition",new Integer(4200));
                oProps.setPropertyValue("Width",new Integer(4400));
                oProps.setPropertyValue("Height",new Integer(4000));
            } catch (Exception e) {
                System.out.println("Couldn't set property 'GraphicURL'");
                e.printStackTrace();
                throw new StatusException
                    ("Couldn't set property 'GraphicURL'", e );
            }

                    TestEnvironment tEnv = new TestEnvironment( oObj );

                    log.println( "adding ObjRelation for XShape "
                        +"(get/setPosition won't work there)" );
                    tEnv.addObjRelation("NoPos", "SwXTextGraphicObject");
                    tEnv.addObjRelation("NoSetSize","SwXTextGraphicObject");
            Object ImgMap = null;
            //creating ObjectRelation for the property
            // 'ImageMap' of 'TextGraphicObject'
            try {
                XMultiServiceFactory xDocMSF = (XMultiServiceFactory)
                    UnoRuntime.queryInterface
                    (XMultiServiceFactory.class,xTextDoc);
                ImgMap = xDocMSF.createInstance
                    ("com.sun.star.image.ImageMapRectangleObject");
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }

            tEnv.addObjRelation("IMGMAP",ImgMap);

            tEnv.addObjRelation("CONTENT", (XTextContent)
                        UnoRuntime.queryInterface(XTextContent.class,instance));
            tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

            //object relation for text.BaseFrameProperties
            tEnv.addObjRelation("TextFrame", xTextFrame);

            return tEnv;

    } // finish method getTestEnvironment

}    // finish class SwXTextGraphicObject

