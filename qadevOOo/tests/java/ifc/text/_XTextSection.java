/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextSection.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:27:44 $
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

import com.sun.star.text.XTextSection;

/**
 * Testing <code>com.sun.star.text.XTextSection</code>
 * interface methods :
 * <ul>
 *  <li><code> getParentSection()</code></li>
 *  <li><code> getChildSections()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextSection
 */
public class _XTextSection extends MultiMethodTest {

    public static XTextSection oObj = null;
    public XTextSection child = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns non zero legth array.
     */
    public void _getChildSections(){
        boolean bResult = false;
        XTextSection oSect[] = oObj.getChildSections();
        if (oSect.length > 0){
            bResult = true;
            child = oSect[0];
        }
        tRes.tested("getChildSections()",  bResult);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getParentSection(){
        requiredMethod("getChildSections()");
        boolean bResult = false;
        bResult = (child.getParentSection() != null);
        tRes.tested("getParentSection()",  bResult);
    }


}  // finish class _XTextSection

