/*************************************************************************
 *
 *  $RCSfile: _XSheetAnnotationAnchor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:03:09 $
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