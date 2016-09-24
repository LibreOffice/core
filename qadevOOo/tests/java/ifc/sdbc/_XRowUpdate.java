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

package ifc.sdbc;

import java.util.List;

import lib.MultiMethodTest;
import lib.Status;
import util.utils;
import util.ValueComparer;

import com.sun.star.io.XDataInputStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XTextInputStream;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

/**
* Testing <code>com.sun.star.sdbc.XRowUpdate</code>
* interface methods :
* <ul>
*  <li><code> updateNull()</code></li>
*  <li><code> updateBoolean()</code></li>
*  <li><code> updateByte()</code></li>
*  <li><code> updateShort()</code></li>
*  <li><code> updateInt()</code></li>
*  <li><code> updateLong()</code></li>
*  <li><code> updateFloat()</code></li>
*  <li><code> updateDouble()</code></li>
*  <li><code> updateString()</code></li>
*  <li><code> updateBytes()</code></li>
*  <li><code> updateDate()</code></li>
*  <li><code> updateTime()</code></li>
*  <li><code> updateTimestamp()</code></li>
*  <li><code> updateBinaryStream()</code></li>
*  <li><code> updateCharacterStream()</code></li>
*  <li><code> updateObject()</code></li>
*  <li><code> updateNumericObject()</code></li>
* </ul> <p>
* Object relations required :
* <ul>
* <li> <code>'CurrentRowData'</code> : (may be used in other
*   interface tests) is a <code>java.util.Vector</code> object
*   that contains column types and values in current row. Each
*   element of vector corresponds to appropriate column (element
*   with index 0 to column 1, 1 -> 2, etc.). <p>
*   The following <code>XRowUpdate</code> methods correspond to classes
*   in Vector :
*   <ul>
*   <li> <code>setBinaryStream</code> -
*        <code>com.sun.star.io.XDataInputStream</code> class. </li>
*   <li> <code>setCharacterStream</code> -
*        <code>com.sun.star.io.XTextInputStream</code> class. </li>
*   <li> <code>setObject</code> -
*        <code>java.lang.Object[]</code> class, the element with
*         index 0 must be used. </li>
*   </ul>
*   Other methods uses types they return (i.e. <code>String</code>
*   for <code>setString</code> method, <code>com.sun.star.sdbc.XRef</code>
*   for <code>setRef</code> method).
* </li>
* <li> <code>'XRowUpdate.XRow'</code> : implementation of <code>
*   com.sun.star.sdbc.XRow</code> interface for checking updated data.
* </li>
* </ul> <p>
* The test <b>damages</b> the object, so it is recreated finally.
* @see com.sun.star.sdbc.XRowUpdate
* @see com.sun.star.sdbc.XRow
*/
public class _XRowUpdate extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XRowUpdate oObj = null ;

    private List<Object> rowData = null ;
    private XRow row = null ;

    /**
    * Gets relations.
    */
    @Override
    public void before() {
        rowData = (List<Object>) tEnv.getObjRelation("CurrentRowData") ;
        if (rowData == null) {
            log.println("!!! 'CurrentRowData' relation not found !!!") ;
        }
        row = (XRow) tEnv.getObjRelation("XRowUpdate.XRow") ;
        if (rowData == null) {
            log.println("!!! 'XRowUpdate.XRow' relation not found !!!") ;
        }
    }

    /**
    * Try to set NULL value for each column. Then using <code>XRow</code>
    * relation check if NULL was really set. <p>
    * Has OK status if for every column NULL value was successfully set.
    * @see com.sun.star.sdbc.XRow
    */
    public void _updateNull() {
        boolean result = true ;
        for (int i = 0; i < rowData.size(); i++) {
            if (rowData.get(i) == null) continue ;
            log.print("  Setting NULL at column #" + (i+1) + " ...") ;
            try {
                oObj.updateNull(i + 1) ;

                if (rowData.get(i) instanceof String) row.getString(i + 1) ;
                if (rowData.get(i) instanceof Boolean) row.getBoolean(i + 1) ;
                if (rowData.get(i) instanceof Byte) row.getByte(i + 1) ;
                if (rowData.get(i) instanceof Short) row.getShort(i + 1) ;
                if (rowData.get(i) instanceof Integer) row.getInt(i + 1) ;
                if (rowData.get(i) instanceof Long) row.getLong(i + 1) ;
                if (rowData.get(i) instanceof Float) row.getFloat(i + 1) ;
                if (rowData.get(i) instanceof Double) row.getDouble(i + 1) ;
                if (rowData.get(i) instanceof byte[]) row.getBytes(i + 1) ;
                if (rowData.get(i) instanceof Date) row.getDate(i + 1) ;
                if (rowData.get(i) instanceof Time) row.getTime(i + 1) ;
                if (rowData.get(i) instanceof DateTime)
                    row.getTimestamp(i + 1) ;
                if (rowData.get(i) instanceof XDataInputStream)
                    row.getBinaryStream(i + 1) ;
                if (rowData.get(i) instanceof XTextInputStream)
                    row.getCharacterStream(i + 1) ;

                if (!row.wasNull()) {
                    log.println("FAILED") ;
                    log.println("Not NULL was returned !!!") ;
                    result = false ;
                } else {
                    log.println("OK") ;
                }
            } catch (SQLException e) {
                log.println("FAILED") ;
                e.printStackTrace(log) ;
                result = false ;
            }
        }

        tRes.tested("updateNull()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateBoolean() {
        boolean result = true ;
        int idx = findColumnOfType(Boolean.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateBoolean()", Status.skipped(true)) ;
            return ;
        }

        try {
            boolean newVal = !row.getBoolean(idx) ;
            oObj.updateBoolean(idx, newVal) ;
            boolean getVal = row.getBoolean(idx) ;
            result = newVal == getVal ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateBoolean()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateByte() {
        boolean result = true ;
        int idx = findColumnOfType(Byte.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateByte()", Status.skipped(true)) ;
            return ;
        }

        try {
            byte newVal = (byte) (row.getByte(idx) + 1) ;
            oObj.updateByte(idx, newVal) ;
            byte getVal = row.getByte(idx) ;
            result = newVal == getVal ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateByte()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateShort() {
        boolean result = true ;
        int idx = findColumnOfType(Short.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateShort()", Status.skipped(true)) ;
            return ;
        }

        try {
            short newVal = (short) (row.getShort(idx) + 1) ;
            oObj.updateShort(idx, newVal) ;
            short getVal = row.getShort(idx) ;
            result = newVal == getVal ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateShort()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateInt() {
        boolean result = true ;
        int idx = findColumnOfType(Integer.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateInt()", Status.skipped(true)) ;
            return ;
        }

        try {
            int newVal = 1 + row.getInt(idx)  ;
            oObj.updateInt(idx, newVal) ;
            int getVal = row.getInt(idx) ;
            result = newVal == getVal ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateInt()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateLong() {
        boolean result = true ;
        int idx = findColumnOfType(Long.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateLong()", Status.skipped(true)) ;
            return ;
        }

        try {
            long newVal = 1 + row.getLong(idx) ;
            oObj.updateLong(idx, newVal) ;
            long getVal = row.getLong(idx) ;
            result = newVal == getVal ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateLong()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateFloat() {
        boolean result = true ;
        int idx = findColumnOfType(Float.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateFloat()", Status.skipped(true)) ;
            return ;
        }

        try {
            float newVal = (float) (1.1 + row.getFloat(idx));
            oObj.updateFloat(idx, newVal) ;
            float getVal = row.getFloat(idx) ;
            result = newVal == getVal ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateFloat()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateDouble() {
        boolean result = true ;
        int idx = findColumnOfType(Double.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateDouble()", Status.skipped(true)) ;
            return ;
        }

        try {
            double newVal = 1.1 + row.getDouble(idx) ;
            oObj.updateDouble(idx, newVal) ;
            double getVal = row.getDouble(idx) ;
            result = utils.approxEqual(newVal, getVal);
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateDouble()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateString() {
        boolean result = true ;
        int idx = findColumnOfType(String.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateString()", Status.skipped(true)) ;
            return ;
        }

        try {
            String newVal = "_" + row.getString(idx) ;
            oObj.updateString(idx, newVal) ;
            String getVal = row.getString(idx) ;
            result = newVal.equals(getVal) ;
            log.println("New value = '" + newVal + "', get value = '"
                + getVal + "'") ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateString()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateBytes() {
        boolean result = true ;
        int idx = findColumnOfType(byte[].class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateBytes()", Status.skipped(true)) ;
            return ;
        }

        try {
            byte[] newVal = row.getBytes(idx) ;
            if (newVal == null || newVal.length == 0) {
                newVal = new byte[] {34, 111, 98} ;
            } else {
                newVal = new byte[] {(byte) (newVal[0] + 1), 111, 98} ;
            }
            oObj.updateBytes(idx, newVal) ;
            byte[] getVal = row.getBytes(idx) ;
            result = ValueComparer.equalValue(newVal, getVal) ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateBytes()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateDate() {
        boolean result = true ;
        int idx = findColumnOfType(Date.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateDate()", Status.skipped(true)) ;
            return ;
        }

        try {
            Date newVal = row.getDate(idx) ;
            newVal.Year ++ ;
            oObj.updateDate(idx, newVal) ;
            Date getVal = row.getDate(idx) ;
            result = ValueComparer.equalValue(newVal, getVal) ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateDate()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateTime() {
        boolean result = true ;
        int idx = findColumnOfType(Time.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateTime()", Status.skipped(true)) ;
            return ;
        }

        try {
            Time newVal = row.getTime(idx) ;
            newVal.Seconds ++ ;
            oObj.updateTime(idx, newVal) ;
            Time getVal = row.getTime(idx) ;
            result = ValueComparer.equalValue(newVal, getVal) ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateTime()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateTimestamp() {
        boolean result = true ;
        int idx = findColumnOfType(DateTime.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateTimestamp()", Status.skipped(true)) ;
            return ;
        }

        try {
            DateTime newVal = row.getTimestamp(idx) ;
            newVal.Year ++ ;
            oObj.updateTimestamp(idx, newVal) ;
            DateTime getVal = row.getTimestamp(idx) ;
            result = ValueComparer.equalValue(newVal, getVal) ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateTimestamp()", result) ;
    }


    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateBinaryStream() {
        boolean result = true ;
        int idx = findColumnOfType(XDataInputStream.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateBinaryStream()", Status.skipped(true)) ;
            return ;
        }

        try {
            Object oStream = tParam.getMSF().
                createInstance("com.sun.star.io.DataInputStream") ;
            XInputStream newVal = UnoRuntime.queryInterface
                (XInputStream.class, oStream);

            oObj.updateBinaryStream(idx, newVal, 0) ;
            XInputStream getVal = row.getBinaryStream(idx) ;
            result = UnoRuntime.areSame(newVal, getVal) ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Unexpected exception:") ;
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateBinaryStream()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateCharacterStream() {
        boolean result = true ;
        int idx = findColumnOfType(XTextInputStream.class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateCharacterStream()", Status.skipped(true)) ;
            return ;
        }

        try {
            Object oStream = tParam.getMSF().
                createInstance("com.sun.star.io.TextInputStream") ;
            XInputStream newVal = UnoRuntime.queryInterface
                (XInputStream.class, oStream);

            oObj.updateCharacterStream(idx, newVal, 0) ;
            XInputStream getVal = row.getCharacterStream(idx) ;
            result = UnoRuntime.areSame(newVal, getVal) ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Unexpected exception:") ;
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateCharacterStream()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateObject() {
        boolean result = true ;
        int idx = findColumnOfType(Object[].class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateObject()", Status.skipped(true)) ;
            return ;
        }

        try {
            Object newVal = tParam.getMSF().
                createInstance("com.sun.star.io.Pipe") ;

            oObj.updateObject(idx, newVal) ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Unexpected exception:") ;
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateObject()", result) ;
    }

    /**
    * Updates column with the appropriate type (if exists) and then
    * checks result with interface <code>XRow</code>.<p>
    * Has OK status if column successfully updated, ahd the same
    * result returned.
    */
    public void _updateNumericObject() {
        boolean result = true ;
        int idx = findColumnOfType(Object[].class) ;

        if (idx < 0) {
            log.println("Required type not found") ;
            tRes.tested("updateNumericObject()", Status.skipped(true)) ;
            return ;
        }

        try {
            Object newVal = tParam.getMSF().
                                createInstance("com.sun.star.io.Pipe") ;

            oObj.updateNumericObject(idx, newVal, 0) ;
        } catch (SQLException e) {
            e.printStackTrace(log) ;
            result = false ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Unexpected exception:") ;
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("updateNumericObject()", result) ;
    }

    /**
    * Finds in relation vector index of column of the appropriate
    * type.
    */
    protected int findColumnOfType(Class<?> clz) {

        for (int i = 0; i < rowData.size(); i++)
            if (clz.isInstance(rowData.get(i))) return i + 1 ;
        return -1 ;
    }

    /**
    * Disposes environment.
    */
    @Override
    public void after() {
        disposeEnvironment() ;
    }

}  // finish class _XRow


