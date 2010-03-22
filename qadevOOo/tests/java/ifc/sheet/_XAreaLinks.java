/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.sheet;

import com.sun.star.sheet.XAreaLinks;
import com.sun.star.table.CellAddress;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.sheet.XAreaLinks</code>
* interface methods :
* <ul>
*  <li><code> insertAtPosition()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.sheet.XAreaLinks
*/
public class _XAreaLinks extends MultiMethodTest {

    public XAreaLinks oObj = null;

    /**
    * Inserts a new link into collection. Checks number of links
    * before and after method call. <p>
    * Has <b>OK</b> status if after method call number of
    * links increased by 1.
    */
    public void _insertAtPosition(){
        boolean bResult = true ;
        int cnt = 0;

          cnt = oObj.getCount() ;
          CellAddress addr = new CellAddress ((short) 1,2,3) ;
          String aSourceArea = util.utils.getFullTestURL("calcshapes.sxc");
          oObj.insertAtPosition (addr, aSourceArea, "a2:b5", "", "") ;

          if (bResult) {
            int new_cnt = oObj.getCount() ;

            if (cnt + 1 != new_cnt) {
              bResult = false ;
              log.println("Number of links before insertAtPosition() call was " + cnt +
                    ", after call is " + new_cnt) ;
            }
        }

        tRes.tested("insertAtPosition()", bResult) ;
   }


    /**
    * Removes a link from collection. Checks number of links
    * before and after method call. <p>
    * Has <b>OK</b> status if after method call number of
    * links decreases by 1. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertAtPosition </code> : to have at least one link. </li>
    * </ul>
    */
    public void _removeByIndex(){
         requiredMethod("insertAtPosition()") ;

         boolean bResult = true ;
      int lcnt = 0;

      lcnt = oObj.getCount() ;
      oObj.removeByIndex(0) ;

      int new_lcnt = oObj.getCount() ;
      if (lcnt - 1 != new_lcnt) {
          bResult = false ;
          log.println(" # Number of links before removeByIndex() call was " +
                lcnt + ", after call is " + new_lcnt) ;
      }
      tRes.tested("removeByIndex()", bResult) ;
    }

} //EOC _XAreaLinks


