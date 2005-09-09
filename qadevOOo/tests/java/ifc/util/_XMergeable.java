/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XMergeable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:41:38 $
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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XMergeable;


/**
 * Testing <code>com.sun.star.util.XMergeable</code>
 * interface methods :
 * <ul>
 *  <li><code> merge()</code></li>
 *  <li><code> getIsMerged()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XMergeable
 */
public class _XMergeable extends MultiMethodTest {

    public XMergeable oObj = null;                // oObj filled by MultiMethodTest

    /**
     * Checks the property <code>IsMerged</code> and sets it
     * to the opposite value. <p>
     * Has <b> OK </b> status if the property value has changed. <p>
     */
    public void _getIsMerged () {

        boolean isMerged = oObj.getIsMerged();
        oObj.merge(!isMerged);
        tRes.tested("getIsMerged()", isMerged != oObj.getIsMerged());
    }

    /**
     * Checks the property <code>IsMerged</code> and sets it
     * to the opposite value. <p>
     * Has <b> OK </b> status if the property value has changed. <p>
     */
    public void _merge () {

        boolean isMerged = oObj.getIsMerged();
        oObj.merge(!isMerged);
        tRes.tested("merge()", isMerged != oObj.getIsMerged());
    }
} // finisch class _XMergeable


