/*************************************************************************
 *
 *  $RCSfile: _XGrid.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:34:36 $
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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XGrid;

/**
* Testing <code>com.sun.star.form.XGrid</code>
* interface methods :
* <ul>
*  <li><code> getCurrentColumnPosition()</code></li>
*  <li><code> setCurrentColumnPosition(short nPos)</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XGrid
*/
public class _XGrid extends MultiMethodTest {
    public XGrid oObj = null;
    public short positionValue;

    /**
    * The method called. Then CurrentColumnPosition is obtained and
    * compared with previously changed value.
    * Has <b> OK </b> status if the method successfully returns
    * and the returned value is correct.
    */
    public void _getCurrentColumnPosition() {
        boolean result;

        requiredMethod("setCurrentColumnPosition()");
        log.println("Testing getCurrentColumnPosition()...");
        if (oObj.getCurrentColumnPosition() == positionValue) {
            result = true;
        } else {
            result = false;
        }
        tRes.tested("getCurrentColumnPosition()", result);
    }

    /**
    * The method called. Then CurrentColumnPosition is obtained, changed
    * and saved.
    * Has <b> OK </b> status if no exceptions was occured.
    */
    public void _setCurrentColumnPosition() {
        log.println("Testing setCurrentColumnPosition()...");
        if (oObj.getCurrentColumnPosition() != 0) {
            positionValue = 0;
        } else {
            positionValue = 1;
        }
        oObj.setCurrentColumnPosition(positionValue);
        tRes.tested("setCurrentColumnPosition()", true);
    }

}
