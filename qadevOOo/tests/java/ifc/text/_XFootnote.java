/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XFootnote.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:20:50 $
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

package ifc.text;

import lib.MultiMethodTest;
import util.XInstCreator;

import com.sun.star.text.XFootnote;
import com.sun.star.uno.XInterface;


/**
 * Testing <code>com.sun.star.text.XFootnote</code>
 * interface methods :
 * <ul>
 *  <li><code> getLabel()</code></li>
 *  <li><code> setLabel()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XFootnote
 */
public class _XFootnote extends MultiMethodTest {

    public XFootnote oObj = null;        // oObj filled by MultiMethodTest

    XInstCreator info = null;               // instance creator
    XInterface oInt = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getLabel(){

        boolean result;

        // testing getLabel
        log.println("Testing getLabel() ... ");
        String oldLabel = oObj.getLabel();
        log.println("getLabel: Old Value: " + oldLabel);
        result = (oldLabel != null);
        if (result) {
            log.println(" ... getLabel() - OK");
        }
        else {
            log.println(" ... getLabel() - FAILED");
        }
        tRes.tested("getLabel()", result);

    } // finished getLabel


    /**
     * Sets a new label, then using <code>getLabel</code> method
     * checks if the label was set. <p>
     *
     * Has <b>OK</b> status if set and get values are equal.
     */
    public void _setLabel(){

        boolean result;
        String str = "New XFootnote Label";

        // testing getLabel
        log.println("Testing setLabel() ... ");
        log.println("New label : " + str);

        String oldLabel = oObj.getLabel();
        log.println("Old label was: " + oldLabel);
        oObj.setLabel(str);

        String res = oObj.getLabel();

        log.println("verify setLabel result");
        result = (res.equals(str));
        if (result) {
            log.println(" ... setLabel() - OK");
        }
        else {
            log.println(" ... setLabel() - FAILED");
        }
        tRes.tested("setLabel()", result);

        log.println("restoring the old label value");
        oObj.setLabel(oldLabel);
    } // finished setLabel

}


