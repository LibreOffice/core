/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SvxUnoTextRange.java,v $
 * $Revision: 1.7.8.1 $
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

import com.sun.star.container.XEnumerationAccess;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class SvxUnoTextRange extends TestCase {

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
            xDrawDoc = DrawTools.createDrawDoc((XMultiServiceFactory)tParam.getMSF());
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
        // create testobject here

        XEnumerationAccess xEA = null ;
        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory)tParam.getMSF()) ;
            XShape oShape = SOF.createShape
                (xDrawDoc,5000,3500,7500,5000,"Text");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;

            XSimpleText text = (XSimpleText) UnoRuntime.queryInterface
                (XSimpleText.class, oShape) ;

            XTextCursor cursor = text.createTextCursor() ;

            text.insertString(cursor, "Paragraph 1", false) ;
            text.insertControlCharacter(cursor,
                ControlCharacter.PARAGRAPH_BREAK, false) ;
            text.insertString(cursor, "Paragraph 2", false) ;
            text.insertControlCharacter(cursor,
                ControlCharacter.PARAGRAPH_BREAK, false) ;
            text.insertString(cursor, "Paragraph 3", false) ;
            text.insertControlCharacter(cursor,
                ControlCharacter.PARAGRAPH_BREAK, false) ;

            xEA = (XEnumerationAccess) UnoRuntime.queryInterface
                (XEnumerationAccess.class, text) ;

            XInterface oTextContent = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),xEA.createEnumeration().nextElement());

            xEA = (XEnumerationAccess) UnoRuntime.queryInterface
                (XEnumerationAccess.class, oTextContent) ;

            oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),xEA.createEnumeration().nextElement());

        } catch (Exception e) {
            log.println("Can't create test object") ;
            e.printStackTrace(log) ;
        }

        // create test environment here
         TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

}

