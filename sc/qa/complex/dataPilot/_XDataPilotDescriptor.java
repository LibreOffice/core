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

package complex.dataPilot;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.sheet.DataPilotFieldOrientation;
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.UnoRuntime;
// import lib.MultiMethodTest;
// import lib.Status;
//import lib.StatusException;
import lib.TestParameters;
// import share.LogWriter;

/**
* Testing <code>com.sun.star.sheet.XDataPilotDescriptor</code>
* interface methods :
* <ul>
*  <li><code> getTag()</code></li>
*  <li><code> setTag()</code></li>
*  <li><code> getSourceRange()</code></li>
*  <li><code> setSourceRange()</code></li>
*  <li><code> getFilterDescriptor()</code></li>
*  <li><code> getDataPilotFields()</code></li>
*  <li><code> getColumnFields()</code></li>
*  <li><code> getRowFields()</code></li>
*  <li><code> getPageFields()</code></li>
*  <li><code> getDataFields()</code></li>
*  <li><code> getHiddenFields()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'FIELDSAMOUNT'</code> (of type <code>Integer</code>):
*   to have number of fields </li>
* <ul> <p>
* @see com.sun.star.sheet.XDataPilotDescriptor
*/
public class _XDataPilotDescriptor {

    public XDataPilotDescriptor oObj = null;
    CellRangeAddress CRA = new CellRangeAddress((short)1, 0, 0, 5, 5);
    CellRangeAddress oldCRA = null ;
    String sTag = new String ("XDataPilotDescriptor_Tag");
    String fieldsNames[];
    int fieldsAmount = 0;
    int tEnvFieldsAmount = 0;

    /**
     * The test parameters
     */
    private TestParameters param = null;

    /**
     * The log writer
     */
//    private LogWriter log = null;

    /**
     * Constructor: gets the object to test, a logger and the test parameters
     * @param xObj The test object
     * @param param The test parameters
     */
    public _XDataPilotDescriptor(XDataPilotDescriptor xObj/*,
                                    LogWriter log*/, TestParameters param) {
        oObj = xObj;
        // this.log = log;
        this.param = param;
    }

    /**
    * Retrieves object relations.

     * @return
     */
    public boolean before() {
        Integer amount = (Integer)param.get("FIELDSAMOUNT");
        if (amount == null) {
            System.out.println("Relation 'FIELDSAMOUNT' not found");
            return false;
        }
        tEnvFieldsAmount = amount.intValue();
        return true;
    }

    /**
    * Test calls the method and compares returned value with value that was set
    * in method <code>setSourceRange()</code>. <p>
    * Has <b> OK </b> status if all fields of cell range addresses are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setSourceRange() </code> : to have current source range </li>
    * </ul>
     * @return
     */
    public boolean _getSourceRange(){
//        requiredMethod("setSourceRange()");
        boolean bResult = true;

        CellRangeAddress objRA = oObj.getSourceRange();
        bResult &= objRA.Sheet == CRA.Sheet;
        bResult &= objRA.StartRow == CRA.StartRow;
        bResult &= objRA.StartColumn == CRA.StartColumn;
        bResult &= objRA.EndRow == CRA.EndRow;
        bResult &= objRA.EndColumn == CRA.EndColumn;

        return bResult;
    }

    /**
    * Test gets the current source range, stores it and sets new source range.<p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> getColumnFields() </code> </li>
    *  <li> <code> getRowFields() </code> </li>
    *  <li> <code> getDataFields() </code> </li>
    *  <li> <code> getHiddenFields() </code> </li>
    *  <li> <code> getPageFields() </code> </li>
    * </ul>
     * @return
     */
    public boolean _setSourceRange(){
/*        executeMethod("getColumnFields()") ;
        executeMethod("getRowFields()") ;
        executeMethod("getDataFields()") ;
        executeMethod("getHiddenFields()") ;
        executeMethod("getPageFields()") ; */

        oldCRA = oObj.getSourceRange() ;
        oObj.setSourceRange(CRA);

        return true;
    }

    /**
    * Test calls the method and checks returned value with value that was set
    * by method <code>setTag()</code>. <p>
    * Has <b> OK </b> status if returned value is equal to value that was set
    * by method <code>setTag()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setTag() </code> : to have current tag </li>
    * </ul>
     * @return
     */
    public boolean _getTag(){
//        requiredMethod("setTag()");
        boolean bResult = true;

        String objTag = oObj.getTag();
        bResult &= objTag.equals(sTag);

        return bResult;
    }

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
     * @return
     */
    public boolean _setTag(){
        oObj.setTag(sTag);
        return true;
    }

    /**
    * Test calls the method , checks returned value, compares
    * number of fields goten from returned value and obtained by object
    * relation <code>'FIELDSAMOUNT'</code> and set property
    * <code>Orientation</code> to one of DataPilotFieldOrientation values. <p>
    * Has <b> OK </b> status if returned value isn't null, number of fields
    * goten from returned value is less than number of fields obtained by relation
    * and no exceptions were thrown. <p>
     * @return
     */
    public boolean _getDataPilotFields(){
        boolean bResult = true;
        XIndexAccess IA = null;

        IA = oObj.getDataPilotFields();
        if (IA == null) {
            System.out.println("Returned value is null.");
            return false;
        } else {System.out.println("getDataPilotFields returned not Null value -- OK");}

        fieldsAmount = IA.getCount();
        if (fieldsAmount < tEnvFieldsAmount) {
            System.out.println("Number of fields is less than number goten by relation.");
            return false;
        } else {System.out.println("count of returned fields -- OK");}

        fieldsNames = new String[tEnvFieldsAmount];
        int i = -1 ;
        int cnt = 0 ;
        while (++i < fieldsAmount) {
            Object field;
            try {
                field = IA.getByIndex(i);
            } catch(com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace();
                return false;
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                e.printStackTrace();
                return false;
            }

            XNamed named = UnoRuntime.queryInterface(XNamed.class, field);
            String name = named.getName();

            System.out.println("Field : '" + name + "' ... ") ;

            if (!name.equals("Data")) {

                fieldsNames[cnt] = name ;

                XPropertySet props =
                    UnoRuntime.queryInterface(XPropertySet.class, field);

                try {
                  switch (cnt % 5) {
                    case 0 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.COLUMN);
                        System.out.println("  Column") ;
                        break;
                    case 1 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.ROW);
                        System.out.println("  Row") ;
                        break;
                    case 2 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.DATA);
                        System.out.println("  Data") ;
                        break;
                    case 3 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.HIDDEN);
                        System.out.println("  Hidden") ;
                        break;
                    case 4 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.PAGE);
                        System.out.println("  Page") ;
                        props.setPropertyValue("CurrentPage", "20");
                        break;
                } } catch (com.sun.star.lang.WrappedTargetException e) {
                    e.printStackTrace();
                    return false;
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    e.printStackTrace();
                    return false;
                } catch (com.sun.star.beans.PropertyVetoException e) {
                    e.printStackTrace();
                    return false;
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    e.printStackTrace();
                    return false;
                }
                if (++cnt > 4)
                {
                    break;
                }
            }
            else
            {
                return false;
            }
        }

        return bResult;
    }

    /**
    * Test calls the method and checks returned collection using the method
    * <code>CheckNames()</code>. <p>
    * Has <b> OK </b> status if the method <code>CheckNames()</code> returns true
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDataPilotFields() </code> : to have array of field names </li>
    * </ul>
     * @return
     */
    public boolean _getColumnFields(){
//        requiredMethod("getDataPilotFields()");
        System.out.println("getColumnFields") ;
        XIndexAccess IA = oObj.getColumnFields();
        return CheckNames(IA, 0);
    }

    /**
    * Test calls the method and checks returned collection using the method
    * <code>CheckNames()</code>. <p>
    * Has <b> OK </b> status if the method <code>CheckNames()</code> returned true
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDataPilotFields() </code> : to have array of field names </li>
    * </ul>
     * @return
     */
    public boolean _getDataFields(){
//        requiredMethod("getDataPilotFields()");
        System.out.println("getDataFields") ;
        XIndexAccess IA = oObj.getDataFields();
        return CheckNames(IA, 2);
    }

    /**
    * Test calls the method and checks returned collection using the method
    * <code>CheckNames()</code>. <p>
    * Has <b> OK </b> status if the method <code>CheckNames()</code> returned true
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDataPilotFields() </code> : to have array of field names </li>
    * </ul>
     * @return
     */
    public boolean _getHiddenFields(){
//        requiredMethod("getDataPilotFields()");
        System.out.println("getHiddenFields") ;
        XIndexAccess IA = oObj.getHiddenFields();
        return CheckNames(IA, 3);
    }

    /**
    * Test calls the method and checks returned collection using the method
    * <code>CheckNames()</code>. <p>
    * Has <b> OK </b> status if the method <code>CheckNames()</code> returned true
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDataPilotFields() </code> : to have array of field names </li>
    * </ul>
     * @return
     */
    public boolean _getRowFields(){
//        requiredMethod("getDataPilotFields()");
        System.out.println("getRowFields") ;
        XIndexAccess IA = oObj.getRowFields();
        boolean bResult = CheckNames(IA, 1);
        return bResult;
    }

    /**
    * setting of PageFields isn't supported by StarOffice Calc
    * Has <b> OK </b> status if the returned IndexAccess
    * isn't NULL. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDataPilotFields() </code> : to have array of field names </li>
    * </ul>
     * @return
     */
    public boolean _getPageFields(){
//        requiredMethod("getDataPilotFields()");
        System.out.println("getPageFields") ;
        XIndexAccess IA = oObj.getPageFields();
        boolean bResult = CheckNames(IA, 4);
        return bResult;
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null
    * and no exceptions were thrown. <p>
     * @return
     */
    public boolean _getFilterDescriptor(){
        boolean bResult = oObj.getFilterDescriptor() != null;
        return bResult;
    }

    /**
    * Method checks that the field with index <code>rem</code> exists
    * in the array <code>IA</code>.
    * @param IA collection of elements that support interface <code>XNamed</code>
    * @param rem index of field in the array of field names that was stored in
    * the method <code>getDataPilotFields()</code>
    * @return true if required field name exists in passed collection;
    * false otherwise
    * @see com.sun.star.container.XNamed
    */
    private boolean CheckNames(XIndexAccess IA, int rem) {
        String name = null;

        if (IA == null) {
            System.out.println("Null retruned.") ;
            return false ;
        }

        if (fieldsNames[rem] == null) {
            System.out.println("No fields were set to this orientation - cann't check result") ;
            return true ;
        }

        if (IA.getCount() == 0) {
            System.out.println("No fields found. Must be at least '"
                + fieldsNames[rem] + "'") ;
            return false ;
        }

        try {
            System.out.println("Fields returned ") ;
            for (int i = 0; i < IA.getCount(); i++) {
                Object field = IA.getByIndex(i);
                XNamed named = UnoRuntime.queryInterface
                    (XNamed.class, field);
                name = named.getName();
                System.out.println(" " + name) ;
                if (fieldsNames[rem].equals(name)) {
                    System.out.println(" - OK") ;
                    return true ;
                }
            }
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            return false ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            return false ;
        }
        System.out.println(" - FAILED (field " + fieldsNames[rem] + " was not found.") ;
        return false ;
    }

    /**
     * Recreates object(to back old orientations of the fields).
     *
    protected void after() {
        disposeEnvironment();
    }*/
}

