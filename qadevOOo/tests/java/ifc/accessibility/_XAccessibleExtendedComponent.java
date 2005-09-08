/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAccessibleExtendedComponent.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:47:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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