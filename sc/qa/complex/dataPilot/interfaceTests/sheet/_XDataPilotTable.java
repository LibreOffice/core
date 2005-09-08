/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XDataPilotTable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:07:31 $
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

package complex.dataPilot.interfaceTests.sheet;

import com.sun.star.sheet.XDataPilotTable;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import lib.Status;
//import lib.StatusException;
import lib.TestParameters;
import share.LogWriter;

/**
* Testing <code>com.sun.star.sheet.XDataPilotTable</code>
* interface methods :
* <ul>
*  <li><code> getOutputRange()</code></li>
*  <li><code> refresh()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'OUTPUTRANGE'</code> (of type <code>CellAddress</code>):
*   to check value returned by method <code>getOutputRange()</code> </li>
*  <li> <code>'CELLFORCHANGE'</code> (of type <code>XCell</code>):
*   to check the method refresh(value of this cell will be changed)</li>
*  <li> <code>'CELLFORCHECK'</code> (of type <code>XCell</code>):
* to check the method refresh (value of this cell must be changed after refresh
* call) </li><ul> <p>
* @see com.sun.star.sheet.XDataPilotTable
* @see com.sun.star.table.CellAddress
*/
public class _XDataPilotTable {

    public XDataPilotTable oObj = null;
    XCell xCellForChange = null;
    XCell xCellForCheck = null;
    CellAddress OutputRange = null;
    int changeValue = 0;

    /**
     * The test parameters
     */
    private TestParameters param = null;

    /**
     * The log writer
     */
    private LogWriter log = null;

    /**
     * Constructor: gets the object to test, a logger and the test parameters
     * @param xObj The test object
     * @param log A log writer
     * @param param The test parameters
     */
    public _XDataPilotTable(XDataPilotTable xObj,
                                    LogWriter log, TestParameters param) {
        oObj = xObj;
        this.log = log;
        this.param = param;
    }

    public boolean before() {
        xCellForChange = (XCell)param.get("CELLFORCHANGE");
        xCellForCheck = (XCell)param.get("CELLFORCHECK");
        OutputRange = (CellAddress)param.get("OUTPUTRANGE");
        changeValue = ((Integer)param.get("CHANGEVALUE")).intValue();

        if (xCellForChange == null || OutputRange == null ||
                xCellForCheck == null) {
            log.println("Relation not found");
            return false;
        }
        return true;
    }
    /**
    * Test calls the method and checks returned value using value obtained by
    * object relation <code>'OUTPUTRANGE'</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    */
    public boolean _getOutputRange(){
        boolean bResult = true;
        CellRangeAddress objRange = oObj.getOutputRange();
        bResult &= OutputRange.Sheet == objRange.Sheet;
        bResult &= OutputRange.Row == objRange.StartRow;
        bResult &= OutputRange.Column == objRange.StartColumn;
        return bResult;
    }

    /**
    * Test sets new value of the cell obtained by object relation
    * 'CELLFORCHANGE', and checks value of the cell obtained by object
    * relation 'CELLFORCHECK'.<p>
    * Has <b>OK</b> status if value of the cell obtained by object relation
    * 'CELLFORCHECK' is changed. <p>
    */
    public boolean _refresh(){
        xCellForChange.setValue(changeValue);
        double oldData = xCellForCheck.getValue();
        oObj.refresh();
        double newData = xCellForCheck.getValue();
        log.println("Old data:" + oldData + "; new data:" + newData);

        return oldData != newData;
    }
}

