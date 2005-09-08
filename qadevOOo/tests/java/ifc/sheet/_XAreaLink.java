/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAreaLink.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:41:35 $
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
* Test is <b> NOT </b> multithread compilant. <p>
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
        if (!(oORAdd == null)){ bResult = true; }
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
        if (!(src == null)){ bResult = true; }
        tRes.tested("getSourceArea()", bResult) ;
    }

    /**
    * Creates a new desination CellRange address and sets it for
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


