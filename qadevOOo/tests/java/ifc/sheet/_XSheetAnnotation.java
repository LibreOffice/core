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

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.ValueComparer;

import com.sun.star.sheet.XSheetAnnotation;
import com.sun.star.table.CellAddress;
import com.sun.star.text.XSimpleText;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XSheetAnnotation</code>
* interface methods :
* <ul>
*  <li><code> getPosition()</code></li>
*  <li><code> getAuthor()</code></li>
*  <li><code> getDate()</code></li>
*  <li><code> getIsVisible()</code></li>
*  <li><code> setIsVisible()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CELLPOS'</code> (of type
*   <code>com.sun.star.table.CellAddress</code>):
*   The position of cell with annotation. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.sheet.XSheetAnnotation
*/
public class _XSheetAnnotation extends MultiMethodTest {

    public XSheetAnnotation oObj = null;

        /**
        * Gets the author of annotation. <p>
        * Has <b>OK</b> status if not null value returned.
        */
        public void _getAuthor() {
          String author = oObj.getAuthor();
          tRes.tested("getAuthor()", author != null);
        }

        /**
        * Gets the modification date of annotation. <p>
        * Has <b>OK</b> status if not null value returned.
        */
        public void _getDate() {
          String date = oObj.getDate();
          tRes.tested("getDate()", date != null);
        }

        /**
        * Sets the string of annotation, then makes it visible and
        * checks the value returned by <code>getIsVisible</code> method. <p>
        * Has <b>OK</b> status if the method returns <code>true</code>.
        */
        public void _getIsVisible() {
          XSimpleText oText  = UnoRuntime.queryInterface(XSimpleText.class, oObj);
          oText.setString("XSheetAnnotation");
          oObj.setIsVisible(true);
          boolean bVis = oObj.getIsVisible();
          tRes.tested("getIsVisible()", bVis);
        }

        /**
        * Gets the position of annotated cell and compares it to
        * the position passed as relation. <p>
        * Has <b>OK</b> status if these positions are equal and not
        * null.
        */
        public void _getPosition() {
          boolean bResult = false;
          CellAddress sCAddr = (CellAddress) tEnv.getObjRelation("CELLPOS") ;
          if (sCAddr == null) throw new StatusException(Status.failed
            ("Relation 'CELLPOS' not found"));

          CellAddress oCAddr = oObj.getPosition();

          bResult = (oCAddr != null) &&
            ValueComparer.equalValue(oCAddr, sCAddr) ;

          tRes.tested("getPosition()", bResult);
        }

        /**
        * Sets the string of annotation, makes it hidden and then
        * visible. Visibility is checked in both cases. <p>
        * Has <b>OK</b> status if the <code>getIsVisible</code> method
        * returns <code>flase</code> in the first case and <code>true</code>
        * in the second.
        */
        public void _setIsVisible() {
          boolean bResult = true;
          XSimpleText oText  = UnoRuntime.queryInterface(XSimpleText.class, oObj);
          oText.setString("XSheetAnnotation");
          oObj.setIsVisible(false);
          boolean bVis = oObj.getIsVisible();
          if (!bVis) {
              oObj.setIsVisible(true);
              bVis = oObj.getIsVisible();
              if (bVis) {
                  bResult = true;
              }
          }

          tRes.tested("setIsVisible()", bResult);
        }

} // EOC _XSheetAnnotation

