/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextGraphicObjectsSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:26:50 $
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
import com.sun.star.text.XTextGraphicObjectsSupplier;

/**
 * Testing <code>com.sun.star.text.XTextGraphicObjectsSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getGraphicObjects()</code></li>
 * </ul> <p>
 *
 * The component <b>must have</b> the graphic object with
 * name  'SwXTextDocument'. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextGraphicObjectsSupplier
 */
public class _XTextGraphicObjectsSupplier extends MultiMethodTest {

    public static XTextGraphicObjectsSupplier oObj = null;

    /**
     * Gets graphic objects collection from the component, and checks
     * if the object with name 'SwXTextDocument' exists. <p>
     * Has <b>OK</b> status if the object exists.
     */
    public void _getGraphicObjects() {
        boolean res = false;

        XNameAccess the_graphics = oObj.getGraphicObjects();
        res = the_graphics.hasByName("SwXTextDocument");

        tRes.tested("getGraphicObjects()",res);
    }

}  // finish class _XTextGraphicObjectsSupplier

