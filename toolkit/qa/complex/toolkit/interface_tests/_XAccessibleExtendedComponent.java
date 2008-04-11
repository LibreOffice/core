/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XAccessibleExtendedComponent.java,v $
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

package complex.toolkit.interface_tests;

import lib.MultiMethodTest;
import com.sun.star.accessibility.XAccessibleExtendedComponent;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.awt.XFont;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

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

    private LogWriter log;
    private static final String className =
        "com.sun.star.accessibility.XAccessibleExtendedComponent" ;

    public XAccessibleExtendedComponent oObj = null;

    // temporary while accessibility package is in com.sun.star
    protected String getTestedClassName() {
        return className;
    }

    public _XAccessibleExtendedComponent(XInterface object, LogWriter log) {
        oObj = (XAccessibleExtendedComponent)UnoRuntime.queryInterface(
                                    XAccessibleExtendedComponent.class, object);
        this.log = log;
    }

    /**
     * Just calls the method.
     */
    public boolean _getFont() {
        XFont font = oObj.getFont();
        log.println("getFont(): " + font);
        return true;
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value isn't null.
     */
    public boolean _getTitledBorderText() {
        String titleBorderText = oObj.getTitledBorderText();
        log.println("getTitledBorderText(): '" + titleBorderText + "'");
        return titleBorderText != null;
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value isn't null.
     */
    public boolean _getToolTipText() {
        String toolTipText = oObj.getToolTipText();
        log.println("getToolTipText(): '" + toolTipText + "'");
        return toolTipText != null;
    }
}
