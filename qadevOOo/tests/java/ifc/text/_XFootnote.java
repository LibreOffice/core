/*************************************************************************
 *
 *  $RCSfile: _XFootnote.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:16:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


