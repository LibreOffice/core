/*************************************************************************
 *
 *  $RCSfile: _XSheetAnnotations.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:03:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


