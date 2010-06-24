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

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.WrappedTargetException;
import java.io.PrintWriter;
import java.util.Comparator;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.form.XForm;
import com.sun.star.frame.XController;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XSearchDescriptor;
import com.sun.star.util.XSearchable;
import com.sun.star.view.XSelectionSupplier;
import util.FormTools;
import util.WriterTools;

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

    boolean debug = false;

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
            debug = tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE);

        } catch ( com.sun.star.uno.Exception e ) {
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
     *  *    @see PrintWriter
     *
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
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
        XForm myForm = null;
        String kindOfControl="CommandButton";
        XShape aShape = null;
        try{
            log.println("adding contol shape '" + kindOfControl + "'");
            aShape = FormTools.createControlShape(xTextDoc, 3000,
                                                            4500, 15000, 10000,
                                                            kindOfControl);
        } catch (Exception e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't create following control shape : '" +
                                        kindOfControl + "': ", e);

        }


        log.println("adding relation for com.sun.star.view.XFormLayerAccess: XForm");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        try {

            XDrawPage xDP = WriterTools.getDrawPage(xTextDoc);
            if (xDP == null)
                log.println("ERROR: could not get DrawPage");

            XNameContainer xForms = FormTools.getForms(xDP);
            if (xForms == null)
                log.println("ERROR: could not get Forms");

                log.println("the draw page contains folowing elemtens:");
                String[] elements = FormTools.getForms(WriterTools.getDrawPage(xTextDoc)).getElementNames();
                for (int i = 0; i< elements.length; i++){
                    log.println("Element[" + i + "] :" + elements[i]);
                }

            myForm = (XForm) AnyConverter.toObject(new Type(XForm.class), xForms.getByName("Standard"));
                if (myForm == null){
                    log.println("ERROR: could not get 'Standard' from drawpage!");
            if (debug){
                log.println("the draw page contains folowing elemtens:");
//                String[] elements = FormTools.getForms(WriterTools.getDrawPage(xTextDoc)).getElementNames();
//                for (int i = 0; i< elements.length; i++){
//                    log.println("Element[" + i + "] :" + elements[i]);
//                }
            }
            }
            else
                tEnv.addObjRelation("XFormLayerAccess.XForm", myForm);
        } catch (WrappedTargetException ex) {
            log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
        } catch (NoSuchElementException ex) {
            log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
        }


        return tEnv;

    } // finish method getTestEnvironment
}    // finish class SwXTextView
