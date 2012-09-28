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

package ifc.awt;

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
import com.sun.star.frame.XModel;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import java.awt.Robot;
import java.awt.event.InputEvent;
import lib.MultiMethodTest;
import util.AccessibilityTools;

/**
* Testing <code>com.sun.star.awt.XUserInputInterception</code>
* interface methods:
* <ul>
*  <li><code> addKeyHandler() </code></li>
*  <li><code> removeKeyHandler() </code></li>
*  <li><code> addMouseClickHandler() </code></li>
*  <li><code> removeMouseClickHandler() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XUserInputInterception.XModel'</code> (of type <code>XModel</code>):
*   used as model where a mouse click or a key press could be done </li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XUserInputInterception
*/
public class _XUserInputInterception extends MultiMethodTest {
    public XUserInputInterception oObj = null;

    private XModel m_XModel = null;

    /** the listener 1 for the mouse click test */
    private MyMouseClickHandler1 m_MouseListener1 = null;
    /** the listener 2 for the mouse click test */
    private MyMouseClickHandler2 m_MouseListener2 = null;

    /** the listener 1 for the key event test */
    private MyKeyHandler1 m_KeyListener1 = null;
    /** the listener 2 for the key event test */
    private MyKeyHandler2 m_KeyListener2 = null;

    /** indicates if the mousePressed event was called*/
    private boolean m_mousePressed1 = false;
    /** indicates if the mouseReleased event was called*/
    private boolean m_mouseReleased1 = false;

    /** indicates if the mousePressed event was called*/
    private boolean m_mousePressed2 = false;
    /** indicates if the mouseReleased event was called*/
    private boolean m_mouseReleased2 = false;

    /** indicates if the mousePressed event was called*/
    private boolean m_keyPressed1 = false;
    /** indicates if the mouseReleased event was called*/
    private boolean m_keyReleased1 = false;

    /** indicates if the mousePressed event was called*/
    private boolean m_keyPressed2 = false;
    /** indicates if the mouseReleased event was called*/
    private boolean m_keyReleased2 = false;

    /** get the object rlation XUserInputInterception.XModel from the
     * test environment
     */
    protected void before() {
        log.print("try to get object relation 'XUserInputInterception.XModel': ");
        m_XModel = (XModel)tEnv.getObjRelation("XUserInputInterception.XModel");
        if (m_XModel == null) log.println("failed => null");
        else log.println("OK");

    }

    /**
    * This test adds two different key listener to the object. <p>
    *
    * Has <b> OK </b> if no exception is thrown.
    */
    public void _addKeyHandler() {

        log.println("creating key listener 1");
        m_KeyListener1 = new MyKeyHandler1();

        log.println("creating key listener 2");
        m_KeyListener2 = new MyKeyHandler2();


        log.println("adding key listener 1");
        oObj.addKeyHandler(m_KeyListener1);


        log.println("adding key listener 2");
        oObj.addKeyHandler(m_KeyListener2);

        tRes.tested("addKeyHandler()", true);
    }

    /**
     * The test requires <CODE>addKeyHandler()</CODE> which adds two key listener.
     * Then one of them will be removed. In a second thread a key event is released
     * by the <CODE>robot</CODE> class.<p>
     * Has <b> OK </b> status if only one of the listener are triggered. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> addKeyHandler() </code> : adds two key listener </li>
     * </ul>
     */
    public void _removeKeyHandler() {
        requiredMethod("addKeyHandler()");

        log.println("remove key listener 2");

        oObj.removeKeyHandler(m_KeyListener2);

        log.println("starting thread to check the key listener...");
        EventTrigger et = new EventTrigger(m_XModel, EventTriggerType.KEY_TEXT_INTO_DOC);

        et.run();

        util.utils.shortWait(tParam.getInt(util.PropertyName.SHORT_WAIT) * 2);
        log.println("key listener thread should be finished.");


        boolean bOK = m_keyPressed1 & m_keyReleased1 &
                      ! m_keyPressed2 & ! m_keyReleased2;

        if (! bOK){
            log.println("The key listener has not the expectd status:");
            log.println("listener\texpected\tgot");
            log.println("keyPressed1\ttrue\t"+m_keyPressed1);
            log.println("keyReleased1\ttrue\t"+m_keyReleased1);
            log.println("keyPressed2\tfalse\t"+m_keyPressed2);
            log.println("keyReleased2\tfalse\t"+m_keyReleased2);
        }

        log.println("remove Key listener 1");
        oObj.removeKeyHandler(m_KeyListener1);

        tRes.tested("removeKeyHandler()", bOK);
    }

    /**
     * This test adds two different mouse klick listener to the object. <p>
     *
     * Has <b> OK </b> if no exception is thrown.
     */
    public void _addMouseClickHandler() {
        log.println("creating mouse listener 1");
        m_MouseListener1 = new MyMouseClickHandler1();
        log.println("creating mouse listener 2");
        m_MouseListener2 = new MyMouseClickHandler2();

        log.println("adding mouse listener 1");
        oObj.addMouseClickHandler(m_MouseListener1);
        log.println("adding mouse listener 2");
        oObj.addMouseClickHandler(m_MouseListener2);

        tRes.tested("addMouseClickHandler()", true);
    }

    /**
     *  The test requires <CODE>addMouseClickHandler()</CODE> which adds two key listener.
     * Then one of them will be removed. In a second thread a mouse klick event is released
     * by the <CODE>robot</CODE> class.<p>
     * Has <b> OK </b> status if only one of the listener are triggered. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> addMouseKlickHandler() </code> : adds two key listener </li>
     * </ul>
     */
    public void _removeMouseClickHandler() {
        requiredMethod("addMouseClickHandler");

        log.println("remove mouse listener 2");

        oObj.removeMouseClickHandler(m_MouseListener2);

        log.println("starting thread to check the mouse listener...");
        EventTrigger et = new EventTrigger(m_XModel, EventTriggerType.MOUSE_KLICK_INTO_DOC);

        et.run();

        util.utils.shortWait(tParam.getInt(util.PropertyName.SHORT_WAIT) * 2);
        log.println("mouse listener thread should be finished.");

        boolean bOK = m_mousePressed1 & m_mouseReleased1 &
                      ! m_mousePressed2 & ! m_mouseReleased2;

        if (! bOK){
            log.println("The mouse listener has not the expectd status:");
            log.println("listener\t\texpected\tgot");
            log.println("mousePressed1\ttrue\t\t"+m_mousePressed1);
            log.println("mouseReleased1\ttrue\t\t"+m_mouseReleased1);
            log.println("mousePressed2\tfalse\t\t"+m_mousePressed2);
            log.println("mouseReleased2\tfalse\t\t"+m_mouseReleased2);
        }

        log.println("remove mouse listener 1");
        oObj.removeMouseClickHandler(m_MouseListener1);

        tRes.tested("removeMouseClickHandler()", bOK);
    }


    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

    /**
     * Listener which added and its method must be called
     * on <code>keyPressed</code> and <code>keyReleased</code> call.
     */
    public class MyKeyHandler1 implements XKeyHandler {
        /**
         * This event sets the member <code>m_keyPressed</coed> to
         *  <code>true</code>
         * @param oEvent The key event informs about the pressed key.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean keyPressed( KeyEvent oEvent ){
            log.println("XKeyHandler 1: keyPressed-Event");
            m_keyPressed1 = true;
            return true;
        }
        /**
         * This event sets the member <code>m_keyReleased</coed> to
         *  <code>true</code>
         * @param oEvent The key event informs about the pressed key.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean keyReleased( KeyEvent oEvent ){
            log.println("XKeyHandler 1: keyReleased-Event");
            m_keyReleased1 = true;
            return true;
        }
        /**
         * This event does nothing usefull
         * @param oEvent refers to the object that fired the event.
         */
        public void disposing( EventObject oEvent ){
            log.println("XKeyHandler 1: disposing-Event");
        }
    }
    /**
     * Listener which added and its method must be called
     * on <code>keyPressed</code> and <code>keyReleased</code> call.
     */
    public class MyKeyHandler2 implements XKeyHandler {
        /**
         * This event sets the member <code>m_keyPressed</coed> to
         *  <code>true</code>
         * @param oEvent The key event informs about the pressed key.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean keyPressed( KeyEvent oEvent ){
            log.println("XKeyHandler 2: keyPressed-Event: " +
                        "This should not be happen because listener is removed!");
            m_keyPressed2 = true;
            return true;
        }
        /**
         * This event sets the member <code>m_keyReleased</coed> to
         *  <code>true</code>
         * @param oEvent The key event informs about the pressed key.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean keyReleased( KeyEvent oEvent ){
            log.println("XKeyHandler 2: keyReleased-Event: " +
                    "This should not be happen because listener is removed!");
            m_keyReleased2 = true;
            return true;
        }
        /**
         * This event does nothing usefull
         * @param oEvent refers to the object that fired the event.
         */
        public void disposing( EventObject oEvent ){
            log.println("XKeyHandler 2: disposing-Event: " +
                        "This should not be happen because listener is removed!");
        }
    }

    /**
     * Listener which added and its method must be called
     * on <code>mousePressed</code> and <code>mouseReleased</code> call.
     */
    public class MyMouseClickHandler1 implements XMouseClickHandler {
        /**
         * This event sets the member <code>m_mousePressed</coed> to
         *  <code>true</code>
         * @param oEvent The mouse event informs about the kind of mouse event.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean mousePressed( MouseEvent oEvent ){
            log.println("XMouseClickHandler 1: mousePressed-Event");
            m_mousePressed1 = true;
            return true;
        }
        /**
         * This event sets the member <code>m_mouseReleased</coed> to
         *  <code>true</code>
         * @param oEvent The mouse event informs about the kind of mouse event.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean mouseReleased( MouseEvent oEvent ){
            log.println("XMouseClickHandler 1: mouseReleased-Event");
            m_mouseReleased1 = true;
            return true;
        }
        /**
         * This event does nothing usefull
         * @param oEvent refers to the object that fired the event.
         */
        public void disposing( EventObject oEvent ){
            log.println("XMouseClickHandler 1: disposing-Event");
        }
    }

    /**
     * Listener which added and removed. Its method must NOT be called
     * on <code>mousePressed</code> and <code>mouseReleased</code> call.
     */
    public class MyMouseClickHandler2 implements XMouseClickHandler {
        /**
         * This event sets the member <code>m_mousePressed</coed> to
         *  <code>true</code>
         * @param oEvent The mouse event informs about the kind of mouse event.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean mousePressed( MouseEvent oEvent ){
            log.println("XMouseClickHandler 2: mousePressed-Event: " +
                        "This should not be happen because listener is removed!");
            m_mousePressed2 = true;
            return true;
        }
        /**
         * This event sets the member <code>m_mouseReleased</coed> to
         *  <code>true</code>
         * @param oEvent The mouse event informs about the kind of mouse event.
         * @return returns <CODE>TRUE</CODE> in erery case
         */
        public boolean mouseReleased( MouseEvent oEvent ){
            log.println("XMouseClickHandler 2: mouseReleased-Event: " +
                        "This should not be happen because listener is removed!");
            m_mouseReleased2 = true;
            return true;
        }
        /**
         * This event does nothing usefull
         * @param oEvent refers to the object that fired the event.
         */
        public void disposing( EventObject oEvent ){
            log.println("XMouseClickHandler 2: disposing-Event: " +
                    "   This should not be happen because listener is removed!");
        }
    }

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

                util.DesktopTools.bringWindowToFront(xModel);

                XWindow xWindow = at.getCurrentWindow(
                                          (XMultiServiceFactory) tParam.getMSF(),
                                          xModel);

                XAccessible xRoot = at.getAccessibleObject(xWindow);



                XAccessibleContext xPanel = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);
                XAccessibleComponent xPanelCont = UnoRuntime.queryInterface(XAccessibleComponent.class, xPanel);

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

