/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
* Test is <b> NOT </b> multithread compliant. <p>
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


