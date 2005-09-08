/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAccessibleImage.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:47:59 $
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

import com.sun.star.accessibility.XAccessibleImage;

/**
 * Testing <code>com.sun.star.accessibility.XAccessibleImage</code>
 * interface methods :
 * <ul>
 *  <li><code>getAccessibleImageDescription()</code></li>
 *  <li><code>getAccessibleImageHeight()</code></li>
 *  <li><code>getAccessibleImageWidth()</code></li>
 * </ul> <p>
 * @see com.sun.star.accessibility.XAccessibleImage
 */
public class _XAccessibleImage extends MultiMethodTest {

    public XAccessibleImage oObj = null;

    /**
     * Just calls the method.
     */
    public void _getAccessibleImageDescription() {
        String descr = oObj.getAccessibleImageDescription();
        log.println("getAccessibleImageDescription(): '" + descr + "'");
        tRes.tested("getAccessibleImageDescription()", descr != null);
    }

    /**
     * Just calls the method.
     */
    public void _getAccessibleImageHeight() {
        int height = oObj.getAccessibleImageHeight();
        log.println("getAccessibleImageHeight(): " + height);
        tRes.tested("getAccessibleImageHeight()", true);
    }

    /**
     * Just calls the method.
     */
    public void _getAccessibleImageWidth() {
        int width = oObj.getAccessibleImageWidth();
        log.println("getAccessibleImageWidth(): " + width);
        tRes.tested("getAccessibleImageWidth()", true);
    }
}