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

package ifc.inspection;

import com.sun.star.inspection.PropertyCategoryDescriptor;
import com.sun.star.inspection.XObjectInspectorModel;
import lib.MultiMethodTest;



/**
 * Testing <code>com.sun.star.inspection.XObjectInspectorModel</code>
 * interface methods :
 * <ul>
 *  <li><code> describeCategories()</code></li>
 *  <li><code> getHandlerFactories()</code></li>
 *  <li><code> getPropertyOrderIndex()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 *
 */
public class _XObjectInspectorModel extends MultiMethodTest {

    /**
     * the test object
     */
    public XObjectInspectorModel oObj = null;

    /**
     * calls the method <CODE>getHandlerFactories()</CODE>
     * Has <b>OK</b> status if returned value is not null.
     */
    public void _HandlerFactories() {

        Object[] HandlerFactories = oObj.getHandlerFactories();

        tRes.tested("HandlerFactories()", HandlerFactories != null) ;
    }

    /**
     * Call the method <CODE>getPropertyOrderIndex()</CODE> with an invalid property name.
     * Has <CODE>OK</CODE> status if the returned index is "0".
     */
    public void _getPropertyOrderIndex() {

        int index = 0;
        boolean result = true;

        log.println("try to get index of Invalid property name 'InvalidPropertyName' ...");
        index = oObj.getPropertyOrderIndex("InvalidPropertyName");
        log.println("index is: " + index);
        result = (index == 0);

        tRes.tested("getPropertyOrderIndex()", result) ;
    }

    /**
     * Call the method <CODE>describeCategories()</CODE>
     * Has <b>OK</b> status if returned value is not null.
     */
    public void _describeCategories() {

        PropertyCategoryDescriptor[] categories = oObj.describeCategories();

        tRes.tested("describeCategories()", categories != null) ;
    }

    /**
     * Call the method <CODE>getHasHelpSection()</CODE>
     * Has <b>OK</b> status if method returned </CODE>true</CODE>
     */
    public void _HasHelpSection() {

        boolean hasHelpSection = oObj.getHasHelpSection();

        tRes.tested("HasHelpSection()", hasHelpSection) ;
    }

    /**
     * Call the method <CODE>getMinHelpTextLines()</CODE>
     * Has <b>OK</b> status if returned value equals to object relation 'minHelpTextLines'
     */
    public void _MinHelpTextLines() {

        Integer minHelpTextLines = (Integer) tEnv.getObjRelation("minHelpTextLines");

        int getMinHelpTextLines = oObj.getMinHelpTextLines();

        boolean result = (minHelpTextLines.intValue() == getMinHelpTextLines);

        if (!result)
            log.println("FAILED: value:" + minHelpTextLines + " gotten value:" + getMinHelpTextLines);

        tRes.tested("MinHelpTextLines()", result) ;
    }

    /**
     * Call the method <CODE>getMaxHelpTextLines())</CODE>
     * Has <b>OK</b> status if returned value equals to object relation 'maxHelpTextLines'
     */
    public void _MaxHelpTextLines() {

        Integer maxHelpTextLines = (Integer) tEnv.getObjRelation("maxHelpTextLines");

        int getMaxHelpTextLines = oObj.getMaxHelpTextLines();

        boolean result = (maxHelpTextLines.intValue() == getMaxHelpTextLines);

        if (!result)
            log.println("FAILED: expected value:" + maxHelpTextLines + " gotten value:" + getMaxHelpTextLines);

        tRes.tested("MaxHelpTextLines()", result);
    }

    public void _IsReadOnly() {
        boolean readOnly = oObj.getIsReadOnly();

        oObj.setIsReadOnly(!readOnly);

        boolean result = (readOnly != oObj.getIsReadOnly());
        if (!result){
            log.println("FAILED: could not change 'IsReadOnly' to value '" + !readOnly + "'");
        }

        oObj.setIsReadOnly(readOnly);

        result &= (readOnly == oObj.getIsReadOnly());
        if (!result){
            log.println("FAILED: could not change back 'IsReadOnly' to value '" + !readOnly + "'");
        }

        tRes.tested("IsReadOnly()", result);
    }

}
