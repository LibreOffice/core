/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XReferenceMarksSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:23:20 $
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

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XReferenceMarksSupplier;

/**
 * Testing <code>com.sun.star.text.XReferenceMarksSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getReferenceMarks()</code></li>
 * </ul> <p>
 *
 * <b>Prerequisites: </b> the reference mark collection
 * must contain an element named 'SwXTextDocument' <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XReferenceMarksSupplier
 */
public class _XReferenceMarksSupplier extends MultiMethodTest {

    public static XReferenceMarksSupplier oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if there is an element
     * named 'SwXTextDocument' in the returned collection.
     */
    public void _getReferenceMarks() {
        boolean res = false;
        XNameAccess the_marks = oObj.getReferenceMarks();
        res = the_marks.hasByName("SwXTextDocument");
        tRes.tested("getReferenceMarks()",res);
    }

}  // finish class _XReferenceMarksSupplier


