/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XLayoutConstrains.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:59:00 $
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

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.Size;
import com.sun.star.awt.XLayoutConstrains;

/**
* Testing <code>com.sun.star.awt.XLayoutConstrains</code>
* interface methods:
* <ul>
*   <li><code> getMinimumSize() </code></li>
*   <li><code> getPreferredSize() </code></li>
*   <li><code> calcAdjustedSize() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XLayoutConstrains
*/
public class _XLayoutConstrains extends MultiMethodTest {
    public XLayoutConstrains oObj = null;

    /**
    * Test calls the method, then check if returned value is not null.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _getMinimumSize() {
        Size aSize = oObj.getMinimumSize();
        tRes.tested("getMinimumSize()", aSize != null);
    }

    /**
    * Test calls the method, then check if returned value is not null.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _getPreferredSize() {
        Size aSize = oObj.getPreferredSize();
        tRes.tested("getPreferredSize()", aSize != null);
    }

    /**
    * Test calls the method with the new size as a parameter.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _calcAdjustedSize() {
        Size aSize = oObj.calcAdjustedSize(new Size(50,50));
        tRes.tested("calcAdjustedSize()", aSize != null);
    }

}

