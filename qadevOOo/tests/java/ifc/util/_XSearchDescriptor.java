/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSearchDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:44:03 $
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

import com.sun.star.util.XSearchDescriptor;

/**
 * Testing <code>com.sun.star.util.XSearchDescriptor</code>
 * interface methods :
 * <ul>
 *  <li><code> getSearchString()</code></li>
 *  <li><code> setSearchString()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XSearchDescriptor
 */
public class _XSearchDescriptor extends MultiMethodTest {

       // oObj filled by MultiMethodTest
    public XSearchDescriptor oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getSearchString(){
        boolean bResult = false;
        String searchStr = null;

        log.println("test for getSearchString() ");
        searchStr = oObj.getSearchString();

        if (!(searchStr == null)){ bResult = true; }
        tRes.tested("getSearchString()", bResult);
    }

    /**
     * Set a new string and checks the result. <p>
     * Has <b> OK </b> status if the string before setting differs
     * from string after setting. <p>
     */
    public void _setSearchString(){
        boolean bResult = false;
        String oldSearchStr = null;
        String cmpSearchStr = null;
        String newSearchStr = "_XSearchDescriptor";
        log.println("test for setSearchString() ");

        oldSearchStr = oObj.getSearchString();
        oObj.setSearchString(newSearchStr);
        cmpSearchStr = oObj.getSearchString();

        if(!(cmpSearchStr.equals(oldSearchStr))){ bResult = true; }
        //oObj.setSearchString(oldSearchStr);
        tRes.tested("setSearchString()", bResult);
    }

}  // finish class _XSearchDescriptor

