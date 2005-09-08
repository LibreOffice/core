/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XLayerSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:41:12 $
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

package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.drawing.XLayerManager;
import com.sun.star.drawing.XLayerSupplier;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.drawing.XLayerSupplier</code>
* interface methods :
* <ul>
*  <li><code> getLayerManager()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XLayerSupplier
*/
public class _XLayerSupplier extends MultiMethodTest{
    public XLayerSupplier oObj = null;

    /**
    * Gets the manager and it is queried for <code>XLayerManager</code>. <p>
    * Has <b> OK </b> status if returned value is successfully
    * queried for <code>XLayerManager</code>. <p>
    */
    public void _getLayerManager(){
        XNameAccess oNA =null;
        XLayerManager oLM = null;
        boolean result = true;

        log.println("testing getLayerManager() ... ");

        oNA = oObj.getLayerManager();
        oLM = (XLayerManager)
            UnoRuntime.queryInterface ( XLayerManager.class, oNA);
        result = oLM != null;

        tRes.tested("getLayerManager()", result);
    } // finished test

} // end XLayerSupplier

