/*************************************************************************
 *
 *  $RCSfile: SwXTextView.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:54:58 $
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
import java.util.Comparator;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XSearchDescriptor;
import com.sun.star.util.XSearchable;
import com.sun.star.view.XSelectionSupplier;

/**
 *
 * initial description
 * @see com.sun.star.text.XTextViewCursorSupplier
 * @see com.sun.star.view.XControlAccess
 * @see com.sun.star.view.XSelectionSupplier
 * @see com.sun.star.view.XViewSettingsSupplier
 *
 */
public class SwXTextView extends TestCase {

    XTextDocument xTextDoc;

    /**
     * in general this method creates a testdocument
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *  *    @see PrintWriter
     *
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn³t create document", e );
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
     *  *    @see PrintWriter
     *
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
    }


    /**
     *  *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *  *    @see PrintWriter
     */
    public TestEnvironment createTestEnvironment( TestParameters tParam,
                              PrintWriter log )throws StatusException {


        // creation of testobject here
        log.println( "creating a test environment" );

        XController xContr = xTextDoc.getCurrentController();

        TestEnvironment tEnv = new TestEnvironment(xContr);

        util.dbg.getSuppServices(xContr);

        SOfficeFactory SOF=SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        XTextFrame first =null;
        XTextFrame second =null;

        Object oFrame1 = null;
        Object oFrame2 = null;
        try {
            XText oText = xTextDoc.getText();
            XTextCursor oCursor = oText.createTextCursor();
            oFrame1 = SOF.createInstance
                (xTextDoc, "com.sun.star.text.TextFrame" );
            first = (XTextFrame)UnoRuntime.queryInterface
                ( XTextFrame.class, oFrame1);
            oText.insertTextContent(oCursor,first, false);
            first.getText().setString("Frame 1");
            oFrame2 = SOF.createInstance(xTextDoc, "com.sun.star.text.TextFrame" );
            second = (XTextFrame)UnoRuntime.queryInterface
                ( XTextFrame.class, oFrame2);
            oText.insertTextContent(oCursor,second, false);
            second.getText().setString("Frame 2");
            oText.insertString( oCursor,
                "SwXTextRanges...SwXTextRanges...SwXTextRanges", false);
            oText.insertControlCharacter( oCursor,
                ControlCharacter.PARAGRAPH_BREAK, false);
            oText.insertString( oCursor,
                "bla...bla...", false);
        } catch (Exception Ex ) {
            Ex.printStackTrace(log);
            throw new StatusException("Couldn't insert text table ", Ex);
        }

        XSearchable oSearch = (XSearchable)UnoRuntime.queryInterface
            (XSearchable.class, xTextDoc);
        XSearchDescriptor xSDesc = oSearch.createSearchDescriptor();
        xSDesc.setSearchString("SwXTextRanges");
        XIndexAccess textRanges1 = oSearch.findAll(xSDesc);

        xSDesc.setSearchString("bla");
        XIndexAccess textRanges2 = oSearch.findAll(xSDesc);

        tEnv.addObjRelation("Selections", new Object[] {
            oFrame1, oFrame2, textRanges1, textRanges2});
        tEnv.addObjRelation("Comparer", new Comparator() {
            public int compare(Object o1, Object o2) {
                XServiceInfo serv1 = (XServiceInfo)
                    UnoRuntime.queryInterface(XServiceInfo.class, o1);
                XServiceInfo serv2 = (XServiceInfo)
                    UnoRuntime.queryInterface(XServiceInfo.class, o2);

                String implName1 = serv1.getImplementationName();
                String implName2 = serv2.getImplementationName();
                if (!implName1.equals(implName2)) {
                    return -1;
                }

                XIndexAccess indAc1 = (XIndexAccess)
                    UnoRuntime.queryInterface(XIndexAccess.class, o1);
                XIndexAccess indAc2 = (XIndexAccess)
                    UnoRuntime.queryInterface(XIndexAccess.class, o2);

                if (indAc1 != null && indAc2 != null) {
                    int c1 = indAc1.getCount();
                    int c2 = indAc2.getCount();
                    return c1 == c2 ? 0 : 1;
                }

                XText text1 = (XText)
                    UnoRuntime.queryInterface(XText.class, o1);
                XText text2 = (XText)
                    UnoRuntime.queryInterface(XText.class, o2);

                if (text1 != null && text2 != null) {
                    return text1.getString().equals(text2.getString()) ? 0 : 1;
                }

                return -1;
            }
            public boolean equals(Object obj) {
                return compare(this, obj) == 0;
            } });

        XSelectionSupplier xsel = (XSelectionSupplier)
            UnoRuntime.queryInterface(XSelectionSupplier.class,xContr);
        try {
            xsel.select(second);
        } catch (Exception e) {
            log.println("Couldn't select");
            throw new StatusException( "Couldn't select", e );
        }

        tEnv.addObjRelation("DOCUMENT",xTextDoc);

        return tEnv;

    } // finish method getTestEnvironment
}    // finish class SwXTextView
