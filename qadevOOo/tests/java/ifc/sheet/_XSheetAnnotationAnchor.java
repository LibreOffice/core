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