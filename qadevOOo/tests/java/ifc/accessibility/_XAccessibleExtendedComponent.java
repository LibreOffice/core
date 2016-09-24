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