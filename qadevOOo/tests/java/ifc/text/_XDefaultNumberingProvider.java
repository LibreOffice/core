/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XDefaultNumberingProvider.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:19:51 $
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

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.Locale;
import com.sun.star.text.XDefaultNumberingProvider;

/**
* Testing <code>com.sun.star.text.XDefaultNumberingProvider</code>
* interface methods :
* <ul>
*  <li><code> getDefaultOutlineNumberings()</code></li>
*  <li><code> getDefaultContinuousNumberingLevels()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.text.XDefaultNumberingProvider
*/
public class _XDefaultNumberingProvider extends MultiMethodTest {

    public static XDefaultNumberingProvider oObj = null;
    private Locale loc = new Locale("en", "EN", "") ;

    /**
    * Just gets numberings for "en" locale. <p>
    * Has <b>OK</b> status if not <code>null</code> value returned
    * and no runtime exceptions occured.
    */
    public void _getDefaultOutlineNumberings() {
        XIndexAccess xIA[] = oObj.getDefaultOutlineNumberings(loc) ;

        tRes.tested("getDefaultOutlineNumberings()", xIA != null) ;
    }

    /**
    * Just gets numberings for "en" locale. <p>
    * Has <b>OK</b> status if not <code>null</code> value returned
    * and no runtime exceptions occured.
    */
    public void _getDefaultContinuousNumberingLevels() {
        PropertyValue[][] lev = oObj.getDefaultContinuousNumberingLevels(loc) ;

        tRes.tested("getDefaultContinuousNumberingLevels()", lev != null) ;
    }
}

