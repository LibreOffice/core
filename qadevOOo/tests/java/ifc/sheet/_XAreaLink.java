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

import com.sun.star.sheet.XAreaLink;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XAreaLink</code>
* interface methods :
* <ul>
*  <li><code> getSourceArea()</code></li>
*  <li><code> setSourceArea()</code></li>
*  <li><code> getDestArea()</code></li>
*  <li><code> setDestArea()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.sheet.XAreaLink
*/
public class _XAreaLink extends MultiMethodTest {

    public XAreaLink oObj = null;
    CellRangeAddress oORAdd, oNRAdd, oCRAdd = null;

    /**
    * Just calls the method and checks the value returned.
    * (More complicated testing done in <code>setDestArea</code>)<p>
    * Has <b>OK</b> status if not null value returned.
    */
    public void _getDestArea(){
        log.println("testing getDestArea()");
        boolean bResult = false;
        oORAdd = oObj.getDestArea();
        if (oORAdd != null){ bResult = true; }
        tRes.tested("getDestArea()", bResult) ;
    }

    /**
    * Just calls the method and checks the value returned.
    * (More complicated testing done in <code>setSourceArea</code>)<p>
    * Has <b>OK</b> status if not null value returned.
    */
    public void _getSourceArea(){
        log.println("testing getSourceArea()");
        boolean bResult = false;
        String src = null;
        src = oObj.getSourceArea() ;
        if (src != null){ bResult = true; }
        tRes.tested("getSourceArea()", bResult) ;
    }

    /**
    * Creates a new destination CellRange address and sets it for
    * the link object. <p>
    * After setting the DestArea, the link is refreshed and the area is
    * adjusted to the size of the source data.
    * Therefore EndCol and EndRow will change after setting. <p>
    * Has <b>OK</b> status if Sheet, Starting Column and Row
    * of the destination range is changed correctly.
    */
    public void _setDestArea(){
        log.println("testing setDestArea()");
        boolean bResult = false;
        int newStartCol = 3, newStartRow = 4, newEndCol = 5, newEndRow = 8 ;
        oORAdd = oObj.getDestArea();
        log.print("Getting: ");
        log.println(getCRA(oORAdd));
        oNRAdd = new CellRangeAddress ((short) 2, newStartCol, newStartRow,
                                                    newEndCol, newEndRow) ;
        oObj.setDestArea(oNRAdd) ;
        log.print("Setting: ");
        log.println(getCRA(oNRAdd));
        oCRAdd = oObj.getDestArea();
        log.print("Getting: ");
        log.println(getCRA(oCRAdd));
        // After setting the DestArea, the link is refreshed and the area is
        // adjusted to the size of the source data.
        // Therefore EndCol and EndRow will change after setting.
        log.println("After setting the DestArea, the link is refreshed "+
            "and the area is");
        log.println("adjusted to the size of the source data.");
        log.println("Therefore only 'Sheet', 'StartCol' and 'StartRow' "+
            "are compared.");
        if ((oCRAdd.StartColumn == oNRAdd.StartColumn) &&
                (oCRAdd.Sheet == oNRAdd.Sheet) &&
                (oCRAdd.StartRow == oNRAdd.StartRow)){
            bResult = true;
            oObj.setDestArea(oORAdd);
        }
        tRes.tested("setDestArea()", bResult) ;
    }

    /**
    * Sets a new source area for the link and then check
    * it using <code>getSourceArea</code>  method. <p>
    * Has <b>OK</b> status if areas set and get are equal.
    */
    public void _setSourceArea(){
        log.println("testing setSourceArea()");
        boolean bResult = false;
        String oSrc = oObj.getSourceArea() ;
        String nSrc = "a1:b2";
        oObj.setSourceArea(nSrc);
        String cSrc = oObj.getSourceArea();
        if( nSrc.equals(cSrc)){
            bResult = true;
            oObj.setSourceArea(oSrc);
        }
        tRes.tested("setSourceArea()", bResult) ;
    }

    /**
    * Prints cell range structure to LOG
    */
    public String getCRA ( CellRangeAddress oCRA ) {
        String res = "( Sheet: ";
        res += oCRA.Sheet;
        res += ";StartColumn: ";
        res += oCRA.StartColumn;
        res += ";StartRow: ";
        res += oCRA.StartRow;
        res += ";EndColumn: ";
        res += oCRA.EndColumn;
        res += ";EndRow: ";
        res += oCRA.EndRow;
        res += " )";
        return res;
    }

} // EOC _XAreaLink


