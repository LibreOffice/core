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

package ifc.sdbc;

import java.util.Vector;

import lib.MultiMethodTest;

import com.sun.star.io.XDataInputStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XTextInputStream;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XRef;
import com.sun.star.sdbc.XRow;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

/**
* Testing <code>com.sun.star.sdbc.XRow</code>
* interface methods :
* <ul>
*  <li><code> wasNull()</code></li>
*  <li><code> getString()</code></li>
*  <li><code> getBoolean()</code></li>
*  <li><code> getByte()</code></li>
*  <li><code> getShort()</code></li>
*  <li><code> getInt()</code></li>
*  <li><code> getLong()</code></li>
*  <li><code> getFloat()</code></li>
*  <li><code> getDouble()</code></li>
*  <li><code> getBytes()</code></li>
*  <li><code> getDate()</code></li>
*  <li><code> getTime()</code></li>
*  <li><code> getTimestamp()</code></li>
*  <li><code> getBinaryStream()</code></li>
*  <li><code> getCharacterStream()</code></li>
*  <li><code> getObject()</code></li>
*  <li><code> getRef()</code></li>
*  <li><code> getBlob()</code></li>
*  <li><code> getClob()</code></li>
*  <li><code> getArray()</code></li>
* </ul> <p>
*
* This interface is full tested in XRowUpdate interface test. Here
* only exceptions checked.
* <p>
*
* Object relations required :
* <ul>
* <li> <code>'CurrentRowData'</code> : (may be used in other
* interface tests) is a <code>java.util.Vector</code> object
* that contains column types and values in current row. Each
* element of vector corresponds to appropriate column (element
* with index 0 to column 1, 1 -> 2, etc.). <p>
* The following <code>XRow</code> methods correspond to classes
* in Vector :
* <ul>
*   <li> <code>getBinaryStream</code> -
*        <code>com.sun.star.io.XDataInputStream</code> class. </li>
*   <li> <code>getCharacterStream</code> -
*        <code>com.sun.star.io.XTextInputStream</code> class. </li>
*   <li> <code>getObject</code> -
*        <code>java.lang.Object[]</code> class, the element with
*         index 0 must be used. </li>
* </ul>
* Other methods uses types they return (i.e. <code>java.lang.String</code>
* for <code>getString</code> method, <code>com.sun.star.sdbc.XRef</code>
* for <code>getRef</code> method).
* </li>
* </ul>
* @see com.sun.star.sdbc.XRaw
* @see ifc.sdbc._XRowUpdate
*/
public class _XRow extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XRow oObj = null ;
    private Vector data = null ;
    private boolean notNullRes = true ;

    /**
    * Retrieves object relation first.
    */
    public void before() {
        data = (Vector) tEnv.getObjRelation("CurrentRowData") ;
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _wasNull() {
        executeMethod("getString()") ;
        executeMethod("getBoolean()") ;
        executeMethod("getByte()") ;
        executeMethod("getShort()") ;
        executeMethod("getInt()") ;
        executeMethod("getLong()") ;
        executeMethod("getFloat()") ;
        executeMethod("getDouble()") ;
        executeMethod("getBytes()") ;
        executeMethod("getDate()") ;
        executeMethod("getTime()") ;
        executeMethod("getTimestamp()") ;
        executeMethod("getBinaryStream()") ;
        executeMethod("getCharacterStream()") ;
        executeMethod("getObject()") ;
        executeMethod("getRef()") ;
        executeMethod("getBlob()") ;
        executeMethod("getClob()") ;
        executeMethod("getArray()") ;

        tRes.tested("wasNull()", notNullRes) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getString() {
        boolean result = true ;
        int col = findColumnOfType(String.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                String getStr = oObj.getString(col) ;
                //result &= ((String)data.get(col - 1)).equals(getStr) ;
                //notNullRes &= !oObj.wasNull() ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getString()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getBoolean() {
        boolean result = true ;
        int col = findColumnOfType(Boolean.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                boolean getVal = oObj.getBoolean(col) ;
                //result &= ((Boolean)data.get(col - 1)).booleanValue() == getVal ;
                //notNullRes &= !oObj.wasNull() ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getBoolean()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getByte() {
        boolean result = true ;
        int col = findColumnOfType(Byte.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                byte getVal = oObj.getByte(col) ;
                //result &= ((Byte)data.get(col - 1)).byteValue() == getVal ;
                //notNullRes &= !oObj.wasNull() ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getByte()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getShort() {
        boolean result = true ;
        int col = findColumnOfType(Short.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                short getVal = oObj.getShort(col) ;
                //result &= ((Short)data.get(col - 1)).shortValue() == getVal ;
                //notNullRes &= !oObj.wasNull() ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getShort()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getInt() {
        boolean result = true ;
        int col = findColumnOfType(Integer.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                int getVal = oObj.getInt(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getInt()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getLong() {
        boolean result = true ;
        int col = findColumnOfType(Long.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                long getVal = oObj.getLong(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getLong()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getFloat() {
        boolean result = true ;
        int col = findColumnOfType(Float.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                float getVal = oObj.getFloat(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getFloat()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getDouble() {
        boolean result = true ;
        int col = findColumnOfType(Double.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                double getVal = oObj.getDouble(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getDouble()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getBytes() {
        boolean result = true ;
        int col = findColumnOfType(byte[].class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                byte[] getVal = oObj.getBytes(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getBytes()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getDate() {
        boolean result = true ;
        int col = findColumnOfType(Date.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                Date getVal = oObj.getDate(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getDate()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getTime() {
        boolean result = true ;
        int col = findColumnOfType(Time.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                Time getVal = oObj.getTime(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getTime()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getTimestamp() {
        boolean result = true ;
        int col = findColumnOfType(DateTime.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                DateTime getVal = oObj.getTimestamp(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getTimestamp()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getBinaryStream() {
        boolean result = true ;
        int col = findColumnOfType(XDataInputStream.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                XInputStream getVal = oObj.getBinaryStream(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getBinaryStream()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getCharacterStream() {
        boolean result = true ;
        int col = findColumnOfType(XTextInputStream.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                XInputStream getVal = oObj.getCharacterStream(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getCharacterStream()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getObject() {
        boolean result = true ;
        int col = findColumnOfType(Object[].class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                Object getVal = oObj.getObject(col, null) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getObject()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getRef() {
        boolean result = true ;
        int col = findColumnOfType(XRef.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                XRef getVal = oObj.getRef(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getRef()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getBlob() {
        boolean result = true ;
        int col = findColumnOfType(XBlob.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                XBlob getVal = oObj.getBlob(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getBlob()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getClob() {
        boolean result = true ;
        int col = findColumnOfType(XClob.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                XClob getVal = oObj.getClob(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getClob()", result) ;
    }

    /**
    * Has <b>OK</b> status if no exceptions occurred in method call.
    */
    public void _getArray() {
        boolean result = true ;
        int col = findColumnOfType(XArray.class) ;
        if (col < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                XArray getVal = oObj.getArray(col) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("getArray()", result) ;
    }

    /**
    * Finds in relation vector index of column of the appropriate
    * type.
    */
    protected int findColumnOfType(Class clz) {

        for (int i = 0; i < data.size(); i++)
            if (clz.isInstance(data.get(i))) return i + 1 ;
        return -1 ;
    }
}  // finish class _XRow


