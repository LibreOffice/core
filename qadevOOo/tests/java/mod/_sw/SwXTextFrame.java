/*************************************************************************
 *
 *  $RCSfile: SwXTextFrame.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:51:54 $
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
import com.sun.star.text.XTextFrame;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 *
 * initial description
 * @see com.sun.star.text.XText
 *
 */
public class SwXTextFrame extends TestCase {
    XTextDocument xTextDoc;

    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn³t create document", e);
        }
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        xTextDoc.dispose();
    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    public synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                              PrintWriter log) {
        XInterface oObj = null;
        XTextFrame oFrame1 = null;
        XTextFrame oFrame2 = null;
        XPropertySet oPropSet = null;
        XText oText = null;
        XTextCursor oCursor = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) Param.getMSF());


        // creating Frames
        log.println("creating Frames");


        Object instance = null;

        try {
            oFrame1 = SOF.createTextFrame(xTextDoc, 500, 500);
            oFrame2 = SOF.createTextFrame(xTextDoc, 1500, 1500);
            oPropSet = (XPropertySet) UnoRuntime.queryInterface(
                               XPropertySet.class, oFrame1);


            // AnchorTypes: 0 = paragraph, 1 = as char, 2 = page,
            // 3 = frame/paragraph 4= at char
            oPropSet.setPropertyValue("AnchorType",
                                      TextContentAnchorType.AS_CHARACTER);
            oText = xTextDoc.getText();
            oCursor = oText.createTextCursor();

            log.println("inserting Frame1");

            XTextContent the_content = (XTextContent) UnoRuntime.queryInterface(
                                               XTextContent.class, oFrame1);
            oText.insertTextContent(oCursor, the_content, true);

            log.println("inserting Frame2");
            the_content = (XTextContent) UnoRuntime.queryInterface(
                                  XTextContent.class, oFrame2);
            oText.insertTextContent(oCursor, the_content, true);

            XText oFrameText = oFrame1.getText();
            oFrameText.insertString(oFrameText.getStart(), "The FrameText",
                                    true);

            instance = SOF.createInstance(xTextDoc,
                                          "com.sun.star.text.TextFrame");
        } catch (Exception Ex) {
            Ex.printStackTrace(log);
            throw new StatusException("Couldn't insert TextFrame ", Ex);
        }

        oObj = oFrame1;

        log.println("creating a new environment for TextFrame object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("CONTENT",
                            (XTextContent) UnoRuntime.queryInterface(
                                    XTextContent.class, instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        log.println("adding ObjRelation for XShape " +
                    "(get/setPosition won't work there)");
        tEnv.addObjRelation("NoPos", "SwXTextFrame");
        tEnv.addObjRelation("NoSetSize", "SwXTextFrame");

        tEnv.addObjRelation("TextFrame", oFrame2);

        return tEnv;
    } // finish method getTestEnvironment
} // finish class SwXTextFrame
