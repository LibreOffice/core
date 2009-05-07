/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XObjectInspectorModel.java,v $
 * $Revision: 1.5 $
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
 * Test is <b> NOT </b> multithread compilant. <p>
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
     * Call the method <CODE>getPropertyOrderIndex()</CODE> with an invalid propety name.
     * Has <CODE>OK</CODE> status if the returned index is "0".
     */
    public void _getPropertyOrderIndex() {

        int index = 0;
        boolean result = true;

        log.println("try to get index of INvalid property name 'InvalidPropertyName' ...");
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
            log.println("FAILED: value:" + minHelpTextLines + " getted value:" + getMinHelpTextLines);

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
            log.println("FAILED: expected value:" + maxHelpTextLines + " getted value:" + getMaxHelpTextLines);

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
