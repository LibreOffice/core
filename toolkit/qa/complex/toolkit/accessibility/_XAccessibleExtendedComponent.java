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

package complex.toolkit.accessibility;

import com.sun.star.accessibility.XAccessibleExtendedComponent;
import com.sun.star.awt.XFont;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

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
public class _XAccessibleExtendedComponent {

    private final XAccessibleExtendedComponent oObj;

    public _XAccessibleExtendedComponent(XInterface object/*, LogWriter log*/) {
        oObj = UnoRuntime.queryInterface(XAccessibleExtendedComponent.class, object);
    }

    /**
     * Just calls the method.
     */
    public boolean _getFont() {
        XFont font = oObj.getFont();
        System.out.println("getFont(): " + font);
        return true;
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value isn't null.
     */
    public boolean _getTitledBorderText() {
        String titleBorderText = oObj.getTitledBorderText();
        System.out.println("getTitledBorderText(): '" + titleBorderText + "'");
        return titleBorderText != null;
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value isn't null.
     */
    public boolean _getToolTipText() {
        String toolTipText = oObj.getToolTipText();
        System.out.println("getToolTipText(): '" + toolTipText + "'");
        return toolTipText != null;
    }
}
