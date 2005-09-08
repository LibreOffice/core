/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XHeaderFooterContent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:48:59 $
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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.text.XText;

/**
 * Testing <code>com.sun.star.sheet.XHeaderFooterContent</code>
 * interface methods :
 * <ul>
 *  <li><code> getLeftText()</code></li>
 *  <li><code> getCenterText()</code></li>
 *  <li><code> getRightText()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.sheet.XHeaderFooterContent
 */
public class _XHeaderFooterContent extends MultiMethodTest {

    public XHeaderFooterContent oObj = null;

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"LEFT"</code>.
    * <p>
    */
    public void _getLeftText() {
        XText left = oObj.getLeftText();
        tRes.tested("getLeftText()",left.getString().equals("LEFT"));
    }

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"RIGHT"</code>.
    * <p>
    */
    public void _getRightText() {
        XText right = oObj.getRightText();
        tRes.tested("getRightText()",right.getString().equals("RIGHT"));
    }

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"CENTER"</code>.
    * <p>
    */
    public void _getCenterText() {
        XText center = oObj.getCenterText();
        tRes.tested("getCenterText()",center.getString().equals("CENTER"));
    }

}  // finish class _XHeaderFooterContent


