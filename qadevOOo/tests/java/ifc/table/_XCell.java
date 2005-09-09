/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XCell.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:06:02 $
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

package ifc.table;

import lib.MultiMethodTest;

import com.sun.star.table.CellContentType;
import com.sun.star.table.XCell;


/**
* Testing <code>com.sun.star.table.XCell</code>
* interface methods :
* <ul>
*  <li><code> getFormula()</code></li>
*  <li><code> setFormula()</code></li>
*  <li><code> getValue()</code></li>
*  <li><code> setValue()</code></li>
*  <li><code> getType()</code></li>
*  <li><code> getError()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.table.XCell
*/
public class _XCell extends MultiMethodTest {
  public XCell oObj = null;

    /**
    * First time errors checked when a proper formula is entered.
    * Second time an incorrect formula entered and errors are checked.<p>
    * Has <b> OK </b> status if in the first case error code 0 returned,
    * and in the second case none-zerro code returned. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFormula() </code> : the method must set proper
    *   formula into cell, so there must be no errors </li>
    * </ul>
    */
    public void _getError() {
        requiredMethod("setFormula()") ;

        boolean result = true;

        if (oObj.getError() != 0) {
            result = false ;
            log.println("getError(): Expected error code is 0, but returned " +
                 oObj.getError()) ;
        }
        oObj.setFormula("=sqrt(-2)") ; // incorrect formula
        if (oObj.getError() == 0) {
            result = false ;
            log.println("getError(): # Non zero error code expected,"+
                " but 0 returned") ;
        }

        tRes.tested("getError()", result);
    } // end getError()

    /**
    * Sets a formula and then gets it. <p>
    * Has <b> OK </b> status if the formula set are the same as get. <p>
    */
    public void _getFormula() {
        boolean result = true;

        String formula = "";
        log.println("getFormula()");
        oObj.setFormula("=2+2");

        formula = (String) oObj.getFormula();

        result &= formula.endsWith("2+2");
        tRes.tested("getFormula()", result);
    } // end getFormula()

    /**
    * Gets the type and check it. <p>
    * Has <b> OK </b> status if the type is one of valid values. <p>
    */
    public void _getType() {
        boolean result = true;
        result = true ;
        log.println("getType() ...");

        if(oObj.getType() == CellContentType.EMPTY) result &= true ;
        else if (oObj.getType() == CellContentType.VALUE) result &= true ;
        else if (oObj.getType() == CellContentType.TEXT) result &= true ;
        else if (oObj.getType() == CellContentType.FORMULA) result &= true ;
        else result = false;

        tRes.tested ("getType()", result) ;
    } // end getType()

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _getValue() {
        boolean result = true;
        double value = 0;
        log.println("getValue() ...");

        value = (double) oObj.getValue();

        tRes.tested("getValue()",result);
    } // end getValue()

    /**
    * Sets a formula and then gets it. <p>
    * Has <b> OK </b> status if the formula set are the same as get. <p>
    */
    public void _setFormula() {
        boolean result = true;
        String formula = "";
        log.println("setFormula() ...");

        oObj.setFormula("=2/6") ;

        formula = (String) oObj.getFormula();

        result &= formula.endsWith("2/6");
        tRes.tested ("setFormula()", result) ;
    } // end setFormula

    /**
    * Sets a value and then gets it. <p>
    * Has <b> OK </b> status if the value set is equal to value get. <p>
    */
    public void _setValue() {
        boolean result = true;
        double cellValue = 0;
        log.println("setValue() ...");

        oObj.setValue(222.333) ;
        cellValue = (double) oObj.getValue() ;

        result &= (cellValue == 222.333);
        tRes.tested("setValue()", result);
       } // end setValue()
}

