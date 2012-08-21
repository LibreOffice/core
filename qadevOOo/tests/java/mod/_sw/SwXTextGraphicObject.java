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
            xTextDoc = SOF.createTextDoc( null );
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
            XTextContent the_content = UnoRuntime.queryInterface(XTextContent.class,oObj);

            log.println("inserting Frame");
            try{
                XTextContent Framecontent = UnoRuntime.queryInterface(
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
            XPropertySet oProps = UnoRuntime.queryInterface(XPropertySet.class,oObj);
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
                XMultiServiceFactory xDocMSF = UnoRuntime.queryInterface
                (XMultiServiceFactory.class,xTextDoc);
                ImgMap = xDocMSF.createInstance
                    ("com.sun.star.image.ImageMapRectangleObject");
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }

            tEnv.addObjRelation("IMGMAP",ImgMap);

            tEnv.addObjRelation("CONTENT", UnoRuntime.queryInterface(XTextContent.class,instance));
            tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

            //object relation for text.BaseFrameProperties
            tEnv.addObjRelation("TextFrame", xTextFrame);

            return tEnv;

    } // finish method getTestEnvironment

}    // finish class SwXTextGraphicObject

