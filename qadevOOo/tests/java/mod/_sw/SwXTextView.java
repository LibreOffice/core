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

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.WrappedTargetException;
import java.io.PrintWriter;
import java.util.Comparator;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.form.XForm;
import com.sun.star.frame.XController;
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
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );

        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
        debug = tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE);
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
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }


    /**
     *  *    creating a TestEnvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *  *    @see PrintWriter
     */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters tParam,
                              PrintWriter log )
        throws com.sun.star.uno.Exception
    {


        // creation of testobject here
        log.println( "creating a test environment" );

        XController xContr = xTextDoc.getCurrentController();

        TestEnvironment tEnv = new TestEnvironment(xContr);

        util.dbg.getSuppServices(xContr);

        SOfficeFactory SOF=SOfficeFactory.getFactory( tParam.getMSF() );
        XTextFrame first =null;
        XTextFrame second =null;

        Object oFrame1 = null;
        Object oFrame2 = null;
        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();
        oFrame1 = SOF.createInstance
            (xTextDoc, "com.sun.star.text.TextFrame" );
        first = UnoRuntime.queryInterface
            ( XTextFrame.class, oFrame1);
        oText.insertTextContent(oCursor,first, false);
        first.getText().setString("Frame 1");
        oFrame2 = SOF.createInstance(xTextDoc, "com.sun.star.text.TextFrame" );
        second = UnoRuntime.queryInterface
            ( XTextFrame.class, oFrame2);
        oText.insertTextContent(oCursor,second, false);
        second.getText().setString("Frame 2");
        oText.insertString( oCursor,
            "SwXTextRanges...SwXTextRanges...SwXTextRanges", false);
        oText.insertControlCharacter( oCursor,
            ControlCharacter.PARAGRAPH_BREAK, false);
        oText.insertString( oCursor,
            "bla...bla...", false);

        XSearchable oSearch = UnoRuntime.queryInterface
            (XSearchable.class, xTextDoc);
        XSearchDescriptor xSDesc = oSearch.createSearchDescriptor();
        xSDesc.setSearchString("SwXTextRanges");
        XIndexAccess textRanges1 = oSearch.findAll(xSDesc);

        xSDesc.setSearchString("bla");
        XIndexAccess textRanges2 = oSearch.findAll(xSDesc);

        tEnv.addObjRelation("Selections", new Object[] {
            oFrame1, oFrame2, textRanges1, textRanges2});
        tEnv.addObjRelation("Comparer", new Comparator<Object>() {
            public int compare(Object o1, Object o2) {
                XServiceInfo serv1 = UnoRuntime.queryInterface(XServiceInfo.class, o1);
                XServiceInfo serv2 = UnoRuntime.queryInterface(XServiceInfo.class, o2);

                String implName1 = serv1.getImplementationName();
                String implName2 = serv2.getImplementationName();
                if (!implName1.equals(implName2)) {
                    return -1;
                }

                XIndexAccess indAc1 = UnoRuntime.queryInterface(XIndexAccess.class, o1);
                XIndexAccess indAc2 = UnoRuntime.queryInterface(XIndexAccess.class, o2);

                if (indAc1 != null && indAc2 != null) {
                    int c1 = indAc1.getCount();
                    int c2 = indAc2.getCount();
                    return c1 == c2 ? 0 : 1;
                }

                XText text1 = UnoRuntime.queryInterface(XText.class, o1);
                XText text2 = UnoRuntime.queryInterface(XText.class, o2);

                if (text1 != null && text2 != null) {
                    return text1.getString().equals(text2.getString()) ? 0 : 1;
                }

                return -1;
            }
          });

        XSelectionSupplier xsel = UnoRuntime.queryInterface(XSelectionSupplier.class,xContr);
        xsel.select(second);

        tEnv.addObjRelation("DOCUMENT",xTextDoc);
        XForm myForm = null;
        String kindOfControl="CommandButton";
        XShape aShape = null;
        log.println("adding control shape '" + kindOfControl + "'");
        aShape = FormTools.createControlShape(xTextDoc, 3000,
                                                        4500, 15000, 10000,
                                                        kindOfControl);


        log.println("adding relation for com.sun.star.view.XFormLayerAccess: XForm");

        WriterTools.getDrawPage(xTextDoc).add(aShape);

        try {

            XDrawPage xDP = WriterTools.getDrawPage(xTextDoc);
            if (xDP == null)
                log.println("ERROR: could not get DrawPage");

            XNameContainer xForms = FormTools.getForms(xDP);
            if (xForms == null)
                log.println("ERROR: could not get Forms");

                log.println("the draw page contains following elements:");
                String[] elements = FormTools.getForms(WriterTools.getDrawPage(xTextDoc)).getElementNames();
                for (int i = 0; i< elements.length; i++){
                    log.println("Element[" + i + "] :" + elements[i]);
                }

            myForm = (XForm) AnyConverter.toObject(new Type(XForm.class), xForms.getByName("Standard"));
                if (myForm == null){
                    log.println("ERROR: could not get 'Standard' from drawpage!");
            if (debug){
                log.println("the draw page contains following elements:");
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
