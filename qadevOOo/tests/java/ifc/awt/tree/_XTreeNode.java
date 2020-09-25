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

package ifc.awt.tree;

import com.sun.star.awt.tree.XTreeNode;
import com.sun.star.uno.AnyConverter;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

/**
 * Testing <code>com.sun.star.awt.tree.XTreeDataModel</code>
 * interface methods :
 * <ul>
 *  <li><code> getChildAt()</code></li>
 *  <li><code> getChildCount()</code></li>
 *  <li><code> getParent()</code></li>
 *  <li><code> getIndex()</code></li>
 *  <li><code> hasChildrenOnDemand()</code></li>
 *  <li><code> getDisplayValue()</code></li>
 *  <li><code> getNodeGraphicURL()</code></li>
 *  <li><code> getExpandedGraphicURL()</code></li>
 *  <li><code> getCollapsedGraphicURL()</code></li> * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 *
 * @see com.sun.star.awt.tree.XTreeDataModel
 */
public class _XTreeNode extends MultiMethodTest {

    public XTreeNode oObj = null;

    private int mCount = 0;

    String msDisplayValue = null;
    String msExpandedGraphicURL = null;
    String msCollapsedGraphicURL = null;
    String msNodeGraphicURL = null;


    @Override
    public void before(){
        msDisplayValue = (String) tEnv.getObjRelation("XTreeNode_DisplayValue");
        if (msDisplayValue == null){
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XTreeNode_DisplayValue'"));
        }

        msExpandedGraphicURL = (String) tEnv.getObjRelation("XTreeNode_ExpandedGraphicURL");
        if (msExpandedGraphicURL == null){
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XTreeNode_ExpandedGraphicURL'"));
        }

        msCollapsedGraphicURL = (String) tEnv.getObjRelation("XTreeNode_CollapsedGraphicURL");
        if (msCollapsedGraphicURL == null){
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XTreeNode_CollapsedGraphicURL'"));
        }

        msNodeGraphicURL = (String) tEnv.getObjRelation("XTreeNode_NodeGraphicURL");
        if(msNodeGraphicURL == null){
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XTreeNode_NodeGraphicURL'"));
        }

    }

    /**
     * Gets the title and compares it to the value set in
     * <code>setTitle</code> method test. <p>
     * Has <b>OK</b> status is set/get values are equal.
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> setTitle </code>  </li>
     * </ul>
     */
    public void _getChildAt(){
        this.requiredMethod("getChildCount()");
        boolean bOK = true;

        for (int i=0; i < mCount ; i++){
            XTreeNode xNode = null;
            try {
                xNode = oObj.getChildAt(i);
            } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
                log.println("ERROR: getChildAt(" + i + "): " + ex.toString());
            }
            if (xNode == null){
                log.println("ERROR: getChildAt(" + i + ") returns null => FAILED");
                bOK = false;
            }
        }

        tRes.tested("getChildAt()", bOK);
    }


    public void _getChildCount(){

        boolean bOK = true;
        mCount = oObj.getChildCount();
        log.println("got count '" + mCount + "' of children");
        if (mCount < 1 ) {
            log.println("ERROR: got a count < 1. The test object must support more than zero children => FAILED");
            bOK = false;
        }
        tRes.tested("getChildCount()", bOK);
    }


    public void _getParent(){
        this.requiredMethod("getChildAt()");

        boolean bOK = true;
        XTreeNode xNode = null;
        try {
            log.println("try to getChildAt(0)");
            xNode = oObj.getChildAt(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            log.println("ERROR: getChildAt(0): " + ex.toString());
        }

        log.println("try to get parent of children");
        if (xNode == null) {
            log.println("missing xNode");
            tRes.tested("getParent()", false);
        }
        else {
            XTreeNode xParrent = xNode.getParent();


            bOK = oObj.equals(xParrent);
            log.println("original object and parent should be the same: " + bOK);
            tRes.tested("getParent()", bOK);
        }
    }


    public void _getIndex(){
        this.requiredMethod("getChildAt()");

        boolean bOK = true;
        XTreeNode xNode = null;
        try {
            log.println("try to getChildAt(0)");
            xNode = oObj.getChildAt(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            log.println("ERROR: getChildAt(0): " + ex.toString());
        }

        log.println("try to get index from child...");
        int index = oObj.getIndex(xNode);

        if (index != 0){
            log.println("ERROR: getIndex() does not return '0' => FAILED");
            bOK = false;
        }

        tRes.tested("getIndex()", bOK);
    }


    public void _hasChildrenOnDemand(){

        boolean bOK = true;

        bOK = oObj.hasChildrenOnDemand();
        tRes.tested("hasChildrenOnDemand()", bOK);
    }


    public void _getDisplayValue(){

        boolean bOK = true;
        String DisplayValue = null;
        Object dispVal = oObj.getDisplayValue();

        try {
            DisplayValue = AnyConverter.toString(dispVal);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: could not convert the returned object of 'getDisplayValue()' " +
                "to String with AnyConverter: " + ex.toString());
        }

        if ( ! this.msDisplayValue.equals(DisplayValue)){
            log.println("ERROR: getNodeGraphicURL() does not return expected value:\n" +
                "\tExpected: " + this.msDisplayValue +"\n" +
                "\tGot: " + DisplayValue);
            bOK = false;
        }

        tRes.tested("getDisplayValue()", bOK);
    }


    public void _getNodeGraphicURL(){

        boolean bOK = true;
        String graphicURL = oObj.getNodeGraphicURL();

        if ( ! this.msNodeGraphicURL.equals(graphicURL)){
            log.println("ERROR: getNodeGraphicURL() does not return expected value:\n" +
                "\tExpected: " + this.msNodeGraphicURL +"\n" +
                "\tGot: " + graphicURL);
            bOK = false;
        }
        tRes.tested("getNodeGraphicURL()", bOK);
    }


    public void _getExpandedGraphicURL(){

        boolean bOK = true;
        String ExpandedGraphicURL = oObj.getExpandedGraphicURL();

        if ( ! this.msExpandedGraphicURL.equals(ExpandedGraphicURL)){
            log.println("ERROR: getExpandedGraphicURL() does not return expected value:\n" +
                "\tExpected: " + this.msExpandedGraphicURL +"\n" +
                "\tGot: " + ExpandedGraphicURL);
            bOK = false;
        }

        tRes.tested("getExpandedGraphicURL()", bOK);
    }


    public void _getCollapsedGraphicURL(){

        boolean bOK = true;

        String CollapsedGraphicURL = oObj.getCollapsedGraphicURL();

        if ( ! this.msCollapsedGraphicURL.equals(CollapsedGraphicURL)){
            log.println("ERROR: getExpandedGraphicURL() does not return expected value:\n" +
                "\tExpected: " + this.msCollapsedGraphicURL +"\n" +
                "\tGot: " + CollapsedGraphicURL);
            bOK = false;
        }

        tRes.tested("getCollapsedGraphicURL()", bOK);
    }

}
