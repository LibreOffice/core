/*************************************************************************
 *
 *  $RCSfile: _XDataPilotDescriptor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:00:03 $
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
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.sheet.DataPilotFieldOrientation;
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.UnoRuntime;

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
public class _XDataPilotDescriptor extends MultiMethodTest {

    public XDataPilotDescriptor oObj = null;
    CellRangeAddress CRA = new CellRangeAddress((short)1, 1, 1, 5, 5);
    CellRangeAddress oldCRA = null ;
    String sTag = new String ("XDataPilotDescriptor_Tag");
    String fieldsNames[];
    int fieldsAmount = 0;
    int tEnvFieldsAmount = 0;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    protected void before() {
        Integer amount = (Integer)tEnv.getObjRelation("FIELDSAMOUNT");
        if (amount == null) throw new StatusException(Status.failed
            ("Relation 'FIELDSAMOUNT' not found"));
        tEnvFieldsAmount = amount.intValue();
    }

    /**
    * Test calls the method and compares returned value with value that was set
    * in method <code>setSourceRange()</code>. <p>
    * Has <b> OK </b> status if all fields of cell range addresses are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setSourceRange() </code> : to have current source range </li>
    * </ul>
    */
    public void _getSourceRange(){
        requiredMethod("setSourceRange()");
        boolean bResult = true;

        CellRangeAddress objRA = oObj.getSourceRange();
        bResult &= objRA.Sheet == CRA.Sheet;
        bResult &= objRA.StartRow == CRA.StartRow;
        bResult &= objRA.StartColumn == CRA.StartColumn;
        bResult &= objRA.EndRow == CRA.EndRow;
        bResult &= objRA.EndColumn == CRA.EndColumn;

        tRes.tested("getSourceRange()", bResult);
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
    */
    public void _setSourceRange(){
        executeMethod("getColumnFields()") ;
        executeMethod("getRowFields()") ;
        executeMethod("getDataFields()") ;
        executeMethod("getHiddenFields()") ;
        executeMethod("getPageFields()") ;

        oldCRA = oObj.getSourceRange() ;
        oObj.setSourceRange(CRA);

        tRes.tested("setSourceRange()", true);
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
    */
    public void _getTag(){
        requiredMethod("setTag()");
        boolean bResult = true;

        String objTag = oObj.getTag();
        bResult &= objTag.equals(sTag);

        tRes.tested("getTag()", bResult);
    }

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setTag(){
        oObj.setTag(sTag);
        tRes.tested("setTag()", true);
    }

    /**
    * Test calls the method , checks returned value, compares
    * number of fields goten from returned value and obtained by object
    * relation <code>'FIELDSAMOUNT'</code> and set property
    * <code>Orientation</code> to one of DataPilotFieldOrientation values. <p>
    * Has <b> OK </b> status if returned value isn't null, number of fields
    * goten from returned value is less than number of fields obtained by relation
    * and no exceptions were thrown. <p>
    */
    public void _getDataPilotFields(){
        boolean bResult = true;
        XIndexAccess IA = null;

        IA = oObj.getDataPilotFields();
        if (IA == null) {
            log.println("Returned value is null.");
            tRes.tested("getDataPilotFields()", false);
            return;
        } else {log.println("getDataPilotFields returned not Null value -- OK");}

        fieldsAmount = IA.getCount();
        if (fieldsAmount < tEnvFieldsAmount) {
            log.println("Number of fields is less than number goten by relation.");
            tRes.tested("getDataPilotFields()", false);
            return;
        } else {log.println("count of returned fields -- OK");}

        fieldsNames = new String[tEnvFieldsAmount];
        int i = -1 ;
        int cnt = 0 ;
        while (++i < fieldsAmount) {
            Object field;
            try {
                field = IA.getByIndex(i);
            } catch(com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log);
                tRes.tested("getDataPilotFields()", false);
                return;
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                e.printStackTrace(log);
                tRes.tested("getDataPilotFields()", false);
                return;
            }

            XNamed named = (XNamed)
                UnoRuntime.queryInterface(XNamed.class, field);
            String name = named.getName();

            log.print("Field : '" + name + "' ... ") ;

            if (!name.equals("Data")) {

                fieldsNames[cnt] = name ;

                XPropertySet props = (XPropertySet)
                    UnoRuntime.queryInterface(XPropertySet.class, field);

                try {
                  switch (cnt % 5) {
                    case 0 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.COLUMN);
                        log.println("  Column") ;
                        break;
                    case 1 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.ROW);
                        log.println("  Row") ;
                        break;
                    case 2 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.DATA);
                        log.println("  Data") ;
                        break;
                    case 3 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.HIDDEN);
                        log.println("  Hidden") ;
                        break;
                    case 4 :
                        props.setPropertyValue("Orientation",
                            DataPilotFieldOrientation.PAGE);
                        log.println("  Page") ;
                        break;
                } } catch (com.sun.star.lang.WrappedTargetException e) {
                    e.printStackTrace(log);
                    tRes.tested("getDataPilotFields()", false);
                    return;
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    e.printStackTrace(log);
                    tRes.tested("getDataPilotFields()", false);
                    return;
                } catch (com.sun.star.beans.PropertyVetoException e) {
                    e.printStackTrace(log);
                    tRes.tested("getDataPilotFields()", false);
                    return;
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    e.printStackTrace(log);
                    tRes.tested("getDataPilotFields()", false);
                    return;
                }
                if (++cnt > 4) break;
            } else {
                tRes.tested("getDataPilotFields()", false);
                return;
            }
        }

        tRes.tested("getDataPilotFields()", bResult);
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
    */
    public void _getColumnFields(){
        requiredMethod("getDataPilotFields()");
        XIndexAccess IA = oObj.getColumnFields();
        boolean bResult = CheckNames(IA, 0);
        tRes.tested("getColumnFields()", bResult);
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
    */
    public void _getDataFields(){
        requiredMethod("getDataPilotFields()");
        XIndexAccess IA = oObj.getDataFields();
        boolean bResult = CheckNames(IA, 2);
        tRes.tested("getDataFields()", bResult);
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
    */
    public void _getHiddenFields(){
        requiredMethod("getDataPilotFields()");
        XIndexAccess IA = oObj.getHiddenFields();
        boolean bResult = CheckNames(IA, 3);
        tRes.tested("getHiddenFields()", bResult);
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
    */
    public void _getRowFields(){
        requiredMethod("getDataPilotFields()");
        XIndexAccess IA = oObj.getRowFields();
        boolean bResult = CheckNames(IA, 1);
        tRes.tested("getRowFields()", bResult);
    }

    /**
    * setting of PageFields isn't supported by StarOffice Calc
    * Has <b> OK </b> status if the returned IndexAccess
    * isn't NULL. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDataPilotFields() </code> : to have array of field names </li>
    * </ul>
    */
    public void _getPageFields(){
        requiredMethod("getDataPilotFields()");
        XIndexAccess IA = oObj.getPageFields();
        boolean bResult = (IA != null);
        //        boolean bResult = CheckNames(IA, 4);
        tRes.tested("getPageFields()", bResult);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null
    * and no exceptions were thrown. <p>
    */
    public void _getFilterDescriptor(){
        boolean bResult = oObj.getFilterDescriptor() != null;
        tRes.tested("getFilterDescriptor()", bResult);
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
    boolean CheckNames(XIndexAccess IA, int rem) {
        String name = null;

        if (IA == null) {
            log.println("Null retruned.") ;
            return false ;
        }

        if (fieldsNames[rem] == null) {
            log.println("No fields were set to this orientation - cann't check result") ;
            return true ;
        }

        if (IA.getCount() == 0) {
            log.println("No fields found. Must be at least '"
                + fieldsNames[rem] + "'") ;
            return false ;
        }

        try {
            log.print("Fields returned ") ;
            for (int i = 0; i < IA.getCount(); i++) {
                Object field = IA.getByIndex(i);
                XNamed named = (XNamed)UnoRuntime.queryInterface
                    (XNamed.class, field);
                name = named.getName();
                log.print(" " + name) ;
                if (fieldsNames[rem].equals(name)) {
                    log.println(" - OK") ;
                    return true ;
                }
            }
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log) ;
            return false ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log) ;
            return false ;
        }
        log.println(" - FAILED (field " + fieldsNames[rem] + " was not found.") ;
        return false ;
    }

    /**
     * Recreates object(to back old orientations of the fields).
     */
    protected void after() {
        disposeEnvironment();
    }
}

