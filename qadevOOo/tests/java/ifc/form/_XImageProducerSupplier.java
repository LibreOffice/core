/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XImageProducerSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:47:11 $
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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.awt.XImageProducer;
import com.sun.star.form.XImageProducerSupplier;

/**
* Testing <code>com.sun.star.form.XImageProducerSupplier</code>
* interface methods :
* <ul>
*  <li><code> getImageProducer()</code></li>
* </ul>
* @see com.sun.star.form.XImageProducerSupplier
*/
public class _XImageProducerSupplier extends MultiMethodTest {

    public XImageProducerSupplier oObj = null;

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method returns non null value
    * and no exceptions were thrown. <p>
    */
    public void _getImageProducer() {
        boolean bResult = false;
        XImageProducer ip = oObj.getImageProducer();
        bResult = ip!=null;
        tRes.tested("getImageProducer()", bResult);
    }
}


