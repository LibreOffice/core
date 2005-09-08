/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAccessible.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:46:18 $
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

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;

/**
 * Testing <code>com.sun.star.accessibility.XAccessible</code>
 * interface methods :
 * <ul>
 *  <li><code> getAccessibleContext()</code></li>
 * </ul> <p>
 * @see com.sun.star.accessibility.XAccessible
 */
public class _XAccessible extends MultiMethodTest {


    public XAccessible oObj = null;

    /**
     * Just calls the method. <p>
     * Has <b> OK </b> status if not null value returned.
     */
    public void _getAccessibleContext() {
        XAccessibleContext ac = oObj.getAccessibleContext();
        tRes.tested("getAccessibleContext()",ac != null);
    }

}
