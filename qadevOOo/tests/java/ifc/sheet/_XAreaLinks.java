/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAreaLinks.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:41:50 $
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


