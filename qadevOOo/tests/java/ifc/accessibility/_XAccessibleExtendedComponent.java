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

package ifc.accessibility;

import lib.MultiMethodTest;

import com.sun.star.accessibility.XAccessibleExtendedComponent;
import com.sun.star.awt.XFont;

/**
 * Testing <code>com.sun.star.accessibility.XAccessibleExtendedComponent</code>
 * interface methods :
 * <ul>
 *  <li><code> getForeground()</code></li>
 *  <li><code> getBackground()</code></li>
 *  <li><code> getFont()</code></li>
 *  <li><code> isEnabled()</code></li>
 *  <li><code> getTitledBorderText()</code></li>
 *  <li><code> getToolTipText()</code></li>
 * </ul> <p>
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 */
public class _XAccessibleExtendedComponent extends MultiMethodTest {

    public XAccessibleExtendedComponent oObj = null;

    /**
     * Just calls the method.
     * deprecated from version srx644g 29.10.02 on
     *
    public void _getForeground() {
        int forColor = oObj.getForeground();
        log.println("getForeground(): " + forColor);
        tRes.tested("getForeground()", true);
    }

    /**
     * Just calls the method.
     * deprecated from version srx644g 29.10.02 on
     *
    public void _getBackground() {
        int backColor = oObj.getBackground();
        log.println("getBackground(): " + backColor);
        tRes.tested("getBackground()", true);
    }

    /**
     * Just calls the method.
     */
    public void _getFont() {
        XFont font = oObj.getFont();
        log.println("getFont(): " + font);
        tRes.tested("getFont()", true);
    }

    /**
     * Calls the method and compares returned value with value that was
     * returned by the method
     * <code>XAccessibleStateSet.contains(AccessibleStateType.ENABLED)</code>.
     * Has OK status if returned values are equal.
     * deprecated from version srx644g 29.10.02 on
     *
    public void _isEnabled() {
        boolean isEnabled = oObj.isEnabled();
        log.println("isEnabled(): " + isEnabled);

        boolean res = true;

        XAccessibleStateSet accStateSet = (XAccessibleStateSet)
            UnoRuntime.queryInterface(XAccessibleStateSet.class, oObj);

        if (accStateSet != null) {
            res = accStateSet.contains(AccessibleStateType.ENABLED)==isEnabled;
        }

        tRes.tested("isEnabled()", res);
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value isn't null.
     */
    public void _getTitledBorderText() {
        String titleBorderText = oObj.getTitledBorderText();
        log.println("getTitledBorderText(): '" + titleBorderText + "'");
        tRes.tested("getTitledBorderText()", titleBorderText != null);
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value isn't null.
     */
    public void _getToolTipText() {
        String toolTipText = oObj.getToolTipText();
        log.println("getToolTipText(): '" + toolTipText + "'");
        tRes.tested("getToolTipText()", toolTipText != null);
    }
}