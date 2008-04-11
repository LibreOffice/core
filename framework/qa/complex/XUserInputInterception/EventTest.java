/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EventTest.java,v $
 * $Revision: 1.4 $
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

package complex.XUserInputInterception;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.KeyEvent;
import com.sun.star.awt.MouseEvent;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XKeyHandler;
import com.sun.star.awt.XMouseClickHandler;
import com.sun.star.awt.XUserInputInterception;
import com.sun.star.awt.XWindow;
import com.sun.star.chart.XChartDocument;
import com.sun.star.frame.*;
import com.sun.star.lang.*;
import com.sun.star.lang.EventObject;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;
import com.sun.star.util.*;
import com.sun.star.uno.*;
import java.awt.Robot;
import java.awt.event.InputEvent;

import complexlib.ComplexTestCase;
import util.AccessibilityTools;
import util.SOfficeFactory;

//-----------------------------------------------
/**
 * This <CODE>ComplexTest</CODE> checks the interface
 * <CODE>XUserInputInterception</CODE>. Therefore it creates a document,
 * adds a mouse and a key listener onto the interface and fire the
 * correspond events. If all listener works as expected the test resluts in
 * <CODE>OK</CODE> status.
 * @short Check the interface XUserInputIntercaption
 * @descr checks is a simple way the interface XUserInputInteraction
 */
public class EventTest extends ComplexTestCase {
    //-------------------------------------------
    // some const

    //-------------------------------------------
    // member

    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xMSF = null;

    /** indicates if the mousePressed event was called*/
    private boolean m_mousePressed = false;
    /** indicates if the mouseReleased event was called*/
    private boolean m_mouseReleased = false;

    /** indicates if the mousePressed event was called*/
    private boolean m_keyPressed = false;
    /** indicates if the mouseReleased event was called*/
    private boolean m_keyReleased = false;

    /** points to a global StarOffice factory */
    private SOfficeFactory m_SOF = null;

    /**
     * define the miliseconds to wait until a <CODE>EventTrigger</CODE> thread should
     * be finished with its work
     */
    final int m_threadWait = 3000;

    //-------------------------------------------
    // test environment

    //-------------------------------------------
    /**
     * The test methods are:
     * <ul>
     *    <li><CODE>checkTextDocument</CODE></LI>
     *    <li><CODE>checkCalcDocument</CODE></LI>
     *    <li><CODE>checkDrawDocument</CODE></LI>
     *    <li><CODE>checkImpressDocument</CODE></LI>
     *    <li><CODE>checkChartDocument</CODE></LI>
     *    <li><CODE>checkMathDocument</CODE></li>
     * </ul>
     * @short A function to tell the framework,
     * which test functions are available.
     * @return All test methods.
     * @todo Think about selection of tests from outside ...
     */
    public String[] getTestMethodNames() {
        return new String[]
        { "checkTextDocument",
          "checkCalcDocument",
          "checkDrawDocument",
          "checkImpressDocument",
          "checkChartDocument",
          "checkMathDocument",
        };
    }

    //-------------------------------------------
    /**
     * creates the mebmer <CODE>m_xMSF</CODE> and <CODE>m_SOF</CODE>
     * @short Create the environment for following tests.
     * @descr create an empty test frame, where we can load
     * different components inside.
     */
    public void before() {
        // get uno service manager from global test environment
        m_xMSF = (XMultiServiceFactory)param.getMSF();

        // create frame instance
        try {
            // get a soffice factory object
            m_SOF = SOfficeFactory.getFactory((XMultiServiceFactory) param.getMSF());

        } catch(java.lang.Throwable ex) {
            failed("Could not create the XUserInputInterception instance.");
        }
    }

    //-------------------------------------------
    /**
     * closes the document
     * @short close the document.
     * @param xDoc the document to close
     */
    public void closeDoc(XInterface xDoc) {
        XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(
                XCloseable.class, xDoc);
        try {
            xClose.close(false);
        } catch(com.sun.star.util.CloseVetoException exVeto) {
            log.println("document couldn't be closed successfully.");
        }
    }

    /**
     * creates a text document and check the <CODE>XMouseClickHandler</CODE> and
     * <CODE>XKeyHandler</CODE>
     * @see com.sun.star.awt.XKeyHandler
     * @see com.sun.star.awt.XMouseClickHandler
     */
    public void checkTextDocument(){

        XTextDocument xDoc = null;

        try{
            xDoc = m_SOF.createTextDoc("WriterTest");
        } catch (com.sun.star.uno.Exception e){
            failed("Could not create a text document: " +e.toString());
        }

        checkListener(xDoc);

        closeDoc(xDoc);
    }

    /**
     * creates an impress document and check the <CODE>XMouseClickHandler</CODE> and
     * <CODE>XKeyHandler</CODE>
     * @see com.sun.star.awt.XKeyHandler
     * @see com.sun.star.awt.XMouseClickHandler
     */
    public void checkImpressDocument(){

        XComponent xDoc = null;

        try{
            xDoc = m_SOF.createImpressDoc("ImpressTest");
        } catch (com.sun.star.uno.Exception e){
            failed("Could not create an impress document: " +e.toString());
        }

        checkListener(xDoc);

        closeDoc(xDoc);
    }

    /**
     * creates a chart document and check the <CODE>XMouseClickHandler</CODE> and
     * <CODE>XKeyHandler</CODE>
     * @see com.sun.star.awt.XKeyHandler
     * @see com.sun.star.awt.XMouseClickHandler
     */
    public void checkChartDocument(){

        XChartDocument xDoc = null;

        try{
            xDoc = m_SOF.createChartDoc("ChartTest");
        } catch (com.sun.star.uno.Exception e){
            failed("Could not create a chart document: " +e.toString());
        }

        checkListener(xDoc);

        closeDoc(xDoc);
    }

    /**
     * creates a math document and check the <CODE>XMouseClickHandler</CODE> and
     * <CODE>XKeyHandler</CODE>
     * @see com.sun.star.awt.XKeyHandler
     * @see com.sun.star.awt.XMouseClickHandler
     */
    public void checkMathDocument(){

        XComponent xDoc = null;

        try{
            xDoc = m_SOF.createMathDoc("MathTest");
        } catch (com.sun.star.uno.Exception e){
            failed("Could not create a math document: " +e.toString());
        }

        checkListener(xDoc);

        closeDoc(xDoc);
    }

    /**
     * creates a draw document and check the <CODE>XMouseClickHandler</CODE> and
     * <CODE>XKeyHandler</CODE>
     * @see com.sun.star.awt.XKeyHandler
     * @see com.sun.star.awt.XMouseClickHandler
     */
    public void checkDrawDocument(){

        XComponent xDoc = null;

        try{
            xDoc = m_SOF.createDrawDoc("DrawTest");
        } catch (com.sun.star.uno.Exception e){
            failed("Could not create a draw document: " +e.toString());
        }

        checkListener(xDoc);

        closeDoc(xDoc);
    }

    /**
     * creates a calc document and check the <CODE>XMouseClickHandler</CODE> and
     * <CODE>XKeyHandler</CODE>
     * @see com.sun.star.awt.XKeyHandler
     * @see com.sun.star.awt.XMouseClickHandler
     */
    public void checkCalcDocument(){

        XSpreadsheetDocument xDoc = null;

        try{
            xDoc = m_SOF.createCalcDoc("CalcTest");
        } catch (com.sun.star.uno.Exception e){
            failed("Could not create a calc document: " +e.toString());
        }

        checkListener(xDoc);
        closeDoc(xDoc);
    }

    /**
     * This is the central test method. It is called by ceck[DOCTYPE]Document. It
     * creates the <CODE>XUserInputInterception</CODE> from the document and call the
     * <CODE>checkMouseListener</CODE> test and the <CODE>checkKeyListener</CODE> test
     * @param xDoc the document to test
     */
    private void checkListener(XInterface xDoc){

        XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xDoc);

        XUserInputInterception xUII = getUII(xModel);

        checkMouseListener(xUII, xModel);
        checkKeyListener(xUII, xModel);
    }

    /**
     * Creates a <CODE>MyKeyHandler</CODE> and adds it to the
     * <CODE>XUserInputInterception</CODE>. Then an <CODE>EventTrigger</CODE> thread
     * was created and started.
     * Has <CODE>OK</CODE> if the members <CODE>m_keyPressed</CODE> and
     * <CODE>m_keyReleased</CODE> are <CODE>TRUE</CODE>
     * @param xUII the XUserInputInterception
     * @param xModel the XModel of a document
     * @see EventTest.MyKeyHander
     * @see EventTest.EventTrigger
     */
    private void checkKeyListener(XUserInputInterception xUII, XModel xModel) {
        m_keyPressed = false;
        m_keyReleased = false;

        MyKeyHandler keyListener = new MyKeyHandler();

        xUII.addKeyHandler(keyListener);

        log.println("starting thread to check the key listener...");
        EventTrigger et = new EventTrigger(xModel, EventTriggerType.KEY_TEXT_INTO_DOC);

        et.run();

        util.utils.shortWait(m_threadWait);
        log.println("key listener thread should be finished.");

        assure("key event does not work!", m_keyPressed && m_keyReleased);
        xUII.removeKeyHandler(keyListener);

    }

    /**
     * Creates a <CODE>MyMouseClickHandler</CODE> and adds it to the
     * <CODE>XUserInputInterception</CODE>. Then an <CODE>EventTrigger</CODE> thread
     * was created and started.
     * Has <CODE>OK</CODE> if the members <CODE>m_mousePressed</CODE> and
     * <CODE>m_mouseReleased</CODE> are <CODE>TRUE</CODE>
     * @param xUII the XUserInputInterception
     * @param xModel the XModel of a document
     * @see EventTest.MyMouseClickHander
     * @see EventTest.EventTrigger
     */

    private void checkMouseListener(XUserInputInterception xUII, XModel xModel) {

        m_mousePressed = false;
        m_mouseReleased = false;

        MyMouseClickHandler mouseListener = new MyMouseClickHandler();

        xUII.addMouseClickHandler(mouseListener);

        log.println("starting thread to check the mouse listener...");
        EventTrigger et = new EventTrigger(xModel, EventTriggerType.MOUSE_KLICK_INTO_DOC);

        et.run();

        util.utils.shortWait(m_threadWait);
        log.println("mouse listener thread should be finished.");

        assure("mouse event does not work!", m_mousePressed && m_mouseReleased);
        xUII.removeMouseClickHandler(mouseListener);
    }

    /**
     * returns the <CODE>XUserInputInterception</CODE> from the <CODE>XMdoel</CODE>
     * @param xModel the XModel of a document
     * @return the <CODE>XUserInputInterception</CODE> of the document
     */
    private XUserInputInterception getUII(XModel xModel){

        XController xController = xModel.getCurrentController();

        XUserInputInterception xUII = (XUserInputInterception) UnoRuntime.queryInterface(
                XUserInputInterception.class, xController);
        if (xUII == null) {
            failed("could not get XUserInputInterception from XContoller", true);
        }
         return xUII;
    }

    /**
     * Listener which added and its method must be called
     * on <code>keyPressed</code> and <code>keyReleased</code> call.
     */
    public class MyKeyHandler implements XKeyHandler {
        /**
         * This event sets the member <code>m_keyPressed</coed> to
         *  <code>true</code>
         * @param oEvent The key event informs about the pressed key.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean keyPressed( KeyEvent oEvent ){
            log.println("XKeyHandler: keyPressed-Event");
            m_keyPressed = true;
            return true;
        }
        /**
         * This event sets the member <code>m_keyReleased</coed> to
         *  <code>true</code>
         * @param oEvent The key event informs about the pressed key.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean keyReleased( KeyEvent oEvent ){
            log.println("XKeyHandler: keyReleased-Event");
            m_keyReleased = true;
            return true;
        }
        /**
         * This event does nothing usefull
         * @param oEvent refers to the object that fired the event.
         */
        public void disposing( EventObject oEvent ){
            log.println("XKeyHandler: disposing-Event");
        }
    }

    /**
     * Listener which added and its method must be called
     * on <code>mousePressed</code> and <code>mouseReleased</code> call.
     */
    public class MyMouseClickHandler implements XMouseClickHandler {
        /**
         * This event sets the member <code>m_mousePressed</coed> to
         *  <code>true</code>
         * @param oEvent The mouse event informs about the kind of mouse event.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean mousePressed( MouseEvent oEvent ){
            log.println("XMouseClickHandler: mousePressed-Event");
            m_mousePressed = true;
            return true;
        }
        /**
         * This event sets the member <code>m_mouseReleased</coed> to
         *  <code>true</code>
         * @param oEvent The mouse event informs about the kind of mouse event.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean mouseReleased( MouseEvent oEvent ){
            log.println("XMouseClickHandler: mouseReleased-Event");
            m_mouseReleased = true;
            return true;
        }
        /**
         * This event does nothing usefull
         * @param oEvent refers to the object that fired the event.
         */
        public void disposing( EventObject oEvent ){
            log.println("XMouseClickHandler: disposing-Event");
        }
    };

    /**
     * To check the events this class is a thread which click a mouse button and
     * press a key with the <CODE>Robot</CODE> class
     * @see java.awt.Robot
     */
    private class EventTrigger extends Thread{

        /**
         * represents a <CODE>AccessibilityTools</CODE>
         */
        private final AccessibilityTools at = new AccessibilityTools();
        /**
         * represents an <CODE>EventType</CODE>
         * @see EventTest.EventTriggerType
         */
        private int eventType = 0;
        /**
         * represents a <CODE>XModel</CODE> of a document
         */
        private XModel xModel = null;

        /**
         * Creates an instacne of this class. The parameter <CODE>eType</CODE> represents
         * the kind of event wich will be triggert at <CODE>run()</CODE>
         * @param model the model of a document
         * @param eType the kind of event which should be trigger
         */
        public EventTrigger(XModel model, int eType)
        {
            this.xModel = model;
            this.eventType = eType;
        }

        /**
         * Triggers the event wich is represented by <CODE>eventType</CODE>
         * The scenarios are:
         * <ul>
         *    <li>EventTest.EventTriggerType.MOUSE_KLICK_INTO_DOC
         *        which calls
         *        <li><CODE>clickIntoDoc</CODE></LI>
         *        </LI>
         *    <li>EventTest.EventTriggerType.KEY_TEXT_INTO_DOC
         *        which calls
         *            <li><CODE>clickIntodoc</CODE></LI>
         *            <li><CODE>keyIntoDoc</CODE></LI>
         *    </LI>
         * </UL>
         */
        public void run(){

            switch (this.eventType){

                case EventTriggerType.MOUSE_KLICK_INTO_DOC:
                    clickIntoDoc();
                    break;
                case EventTriggerType.KEY_TEXT_INTO_DOC:
                    clickIntoDoc();
                    keyIntoDoc();
                    break;

            }
        }
        /**
         * This method cklicks into the middel of a document. It uses Accessibility
         * to get the document and query for its position and its range to calculate
         * the middle. This values was used for <CODE>Robot</CODE> Class. This
         * Robot class is able to move the mouse and to cklick a mouse button
         * @see java.awt.Robot
        */
        private void clickIntoDoc(){
            try{
                // get the position and the range of a scroll bar

                XWindow xWindow = at.getCurrentWindow(
                                          (XMultiServiceFactory) param.getMSF(),
                                          xModel);

                XAccessible xRoot = at.getAccessibleObject(xWindow);



                XAccessibleContext xPanel = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);
                XAccessibleComponent xPanelCont = (XAccessibleComponent) UnoRuntime.queryInterface(XAccessibleComponent.class, xPanel);

                // the position of the panel
                Point point = xPanelCont.getLocationOnScreen();

                // the range of the panel
                Rectangle rect = xPanelCont.getBounds();

                try {
                    Robot rob = new Robot();
                    int x = point.X + (rect.Width / 2);
                    int y = point.Y + (rect.Height / 2);
                    log.println("try to klick into the middle of the document");
                    rob.mouseMove(x, y);
                    rob.mousePress(InputEvent.BUTTON1_MASK);
                    rob.mouseRelease(InputEvent.BUTTON1_MASK);
                } catch (java.awt.AWTException e) {
                    log.println("couldn't press mouse button");
                }
            } catch (java.lang.Exception e){
                log.println("could not click into the scroll bar: " + e.toString());
            }
        }

        /**
         * This method press the "A" key. Therefore it uses the <CODE>Robot</CODE>
         * class.
         * @see java.awt.Robot
        */
        private void keyIntoDoc(){
            try {
                Robot rob = new Robot();
                log.println("try to press 'A'");
                rob.keyPress(java.awt.event.KeyEvent.VK_A);
                rob.keyRelease(java.awt.event.KeyEvent.VK_A);
            } catch (java.awt.AWTException e) {
                log.println("couldn't press key");
            }

        }
    }

    /** This interface represents all possible actions which could be used
     * in the <CODE>EventTrigger</CODE> class.
     * @see EventTest.EventTrigger
    */
    private interface EventTriggerType{

        /** klick the mouse into the scroll bar*/
        final public static int MOUSE_KLICK_INTO_DOC = 1;

        /** write some text into a spread sheet*/
        final public static int KEY_TEXT_INTO_DOC = 2;
    }
}