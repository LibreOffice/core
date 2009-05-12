/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SvxUnoTextField.java,v $
 * $Revision: 1.6.8.1 $
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

package mod._svx;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.SOfficeFactory;

import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see ifc._XComponent
 * @see ifc._TextContent
 * @see ifc._XTextContent
 * @see ifc._XTextField
 *
 */
public class SvxUnoTextField extends TestCase {

    static XComponent xDrawDoc;

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

        try {
            log.println( "creating a drawdoc" );
            xDrawDoc = DrawTools.createDrawDoc( (XMultiServiceFactory) tParam.getMSF());
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
        util.DesktopTools.closeDoc(xDrawDoc);
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
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        try {

            SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)  tParam.getMSF());
              oShape = SOF.createShape(xDrawDoc,5000,3500,7500,5000,"Rectangle");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);
        }
        catch (Exception e) {
            log.println("Couldn't create Shape");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create Shape ",e);
        }

        XTextCursor the_Cursor = null;

          // create testobject here
        try {

            XText the_Text = (XText) UnoRuntime.queryInterface(XText.class,oShape);
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface( XMultiServiceFactory.class, xDrawDoc );
            the_Cursor = the_Text.createTextCursor();
            oObj = (XInterface)
                oDocMSF.createInstance( "com.sun.star.text.TextField.DateTime" );
            XTextContent the_Field = (XTextContent)
                             UnoRuntime.queryInterface(XTextContent.class,oObj);


            the_Text.insertTextContent(the_Cursor,the_Field,false);
        }
        catch (Exception ex) {
            log.println("Couldn't create Textfield");
            ex.printStackTrace(log);
            throw new StatusException("Couldn't create TextField ",ex);
        }

        log.println( "creating a new environment for FieldMaster object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );
        tEnv.addObjRelation("RANGE", the_Cursor);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxUnoTextField

