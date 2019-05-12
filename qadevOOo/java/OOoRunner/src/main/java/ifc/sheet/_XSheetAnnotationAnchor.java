/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XSheetAnnotation;
import com.sun.star.sheet.XSheetAnnotationAnchor;
import com.sun.star.table.CellAddress;
import com.sun.star.text.XSimpleText;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XSheetAnnotationAnchor</code>
* interface methods :
* <ul>
*  <li><code> getAnnotation()</code></li>
* </ul>
* @see com.sun.star.sheet.XSheetAnnotationAnchor
*/
public class _XSheetAnnotationAnchor extends MultiMethodTest {

    public XSheetAnnotationAnchor oObj = null;
    protected XSheetAnnotation anno = null;

    public void _getAnnotation() {
        anno = oObj.getAnnotation();
        tRes.tested("getAnnotation()",checkAnnotation());
    }

    protected boolean checkAnnotation() {
        boolean res = true;
        res &= check_getAuthor();
        res &= check_getDate();
        res &= check_getIsVisible();
        res &= check_getPosition();
        res &= check_setIsVisible();
        return res;
    }

    /**
    * Gets the author of annotation. <p>
    * Returns <b>true</b> if not null value returned.
    */
    protected boolean check_getAuthor() {
      String author = anno.getAuthor();
      return (author != null);
    }

    /**
    * Gets the modification date of annotation. <p>
    * Returns <b>true</b> if not null value returned.
    */
    protected boolean check_getDate() {
      String date = anno.getDate();
      return (date != null);
    }

    /**
    * Sets the string of annotation, then makes it visible and
    * checks the value returned by <code>getIsVisible</code> method. <p>
    * Returns <b>true</b> if the method returns <code>true</code>.
    */
    protected boolean check_getIsVisible() {
      XSimpleText oText  = (XSimpleText)
        UnoRuntime.queryInterface(XSimpleText.class, anno);
      oText.setString("XSheetAnnotation");
      anno.setIsVisible(true);
      boolean bVis = anno.getIsVisible();
      return bVis;
    }

    /**
    * Gets the position of annotated cell
    * Returns <b>true</b> if this position is not null.
    */
    protected boolean check_getPosition() {
      CellAddress oCAddr = anno.getPosition();
      return  (oCAddr != null);
    }

    /**
    * Sets the string of annotation, makes it hidden and then
    * visible. Visibility is checked in both cases. <p>
    * Returns <b>true</b> if the <code>getIsVisible</code> method
    * returns <code>false</code> in the first case and <code>true</code>
    * in the second.
    */
    protected boolean check_setIsVisible() {
      boolean bResult = true;
      XSimpleText oText  = (XSimpleText)
          UnoRuntime.queryInterface(XSimpleText.class, anno);
      oText.setString("XSheetAnnotation");
      anno.setIsVisible(false);
      boolean bVis = anno.getIsVisible();
      if (!bVis) {
          anno.setIsVisible(true);
          bVis = anno.getIsVisible();
          if (bVis) {
              bResult = true;
          }
      }

      return bResult;
    }

}