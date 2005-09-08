/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSheetAnnotations.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:53:09 $
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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XSheetAnnotations;
import com.sun.star.table.CellAddress;


/**
* Testing <code>com.sun.star.sheet.XSheetAnnotations</code>
* interface methods :
* <ul>
*  <li><code> insertNew()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.sheet.XSheetAnnotations
*/
public class _XSheetAnnotations extends MultiMethodTest {

    public XSheetAnnotations oObj = null;

    /**
    * Adds two new annotations into collection. <p>
    * Has <b>OK</b> status if the number of elements in collection
    * increased by 2 after method call.
    */
    public void _insertNew(){
        boolean bResult = false;

        int initialAmount = oObj.getCount();
        String sAnno = oObj.toString();

        oObj.insertNew(new CellAddress((short)1, 2, 5), sAnno + "1");
        oObj.insertNew(new CellAddress((short)1, 1, 1), sAnno + "2");

        bResult = (oObj.getCount() == 2 + initialAmount);
        tRes.tested("insertNew()", bResult);
    }

    /**
    * Removes one annotation from collection. <p>
    * Has <b>OK</b> status if the number of elements in collection
    * decreased after method call. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertNew </code> : to be sure at least two elements
    *   exist in the collection.</li>
    * </ul>
    */
    public void _removeByIndex(){
        requiredMethod("insertNew()");
        int tmpCnt = oObj.getCount();

        oObj.removeByIndex(1);
        int newCnt = oObj.getCount();

        tRes.tested("removeByIndex()", newCnt < tmpCnt);
    }

} // EOC _XSheetAnnotations


