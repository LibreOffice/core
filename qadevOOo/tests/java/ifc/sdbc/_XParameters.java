/*************************************************************************
 *
 *  $RCSfile: _XParameters.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:53:56 $
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

package ifc.sdbc;

import java.util.Vector;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.io.XDataInputStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XTextInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XParameters;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

/**
/**
* Testing <code>com.sun.star.sdbc.XParameters</code>
* interface methods :
* <ul>
*  <li><code> setNull()</code></li>
*  <li><code> setObjectNull()</code></li>
*  <li><code> setBoolean()</code></li>
*  <li><code> setByte()</code></li>
*  <li><code> setShort()</code></li>
*  <li><code> setInt()</code></li>
*  <li><code> setLong()</code></li>
*  <li><code> setFloat()</code></li>
*  <li><code> setDouble()</code></li>
*  <li><code> setString()</code></li>
*  <li><code> setBytes()</code></li>
*  <li><code> setDate()</code></li>
*  <li><code> setTime()</code></li>
*  <li><code> setTimestamp()</code></li>
*  <li><code> setBinaryStream()</code></li>
*  <li><code> setCharacterStream()</code></li>
*  <li><code> setObject()</code></li>
*  <li><code> setObjectWithInfo()</code></li>
*  <li><code> setRef()</code></li>
*  <li><code> setBlob()</code></li>
*  <li><code> setClob()</code></li>
*  <li><code> setArray()</code></li>
*  <li><code> clearParameters()</code></li>
* </ul> <p>
* Object relations required :
* <ul>
* <li> <code>'XParameters.ParamValues'</code> :  is a
* <code>java.util.Vector</code> object
* that contains parameter types and values of the statement. Each
* element of vector corresponds to appropriate parameter (element
* with index 0 to parameter #1, 1 -> #2, etc.). <p>
* The following <code>XParameters</code> methods correspond to classes
* in Vector :
* <ul>
*   <li> <code>setBinaryStream</code> -
*        <code>com.sun.star.io.XDataInputStream</code> class. </li>
*   <li> <code>setCharacterStream</code> -
*        <code>com.sun.star.io.XTextInputStream</code> class. </li>
*   <li> <code>setObject</code> -
*        <code>java.lang.Object[]</code> class, the element with
*         index 0 must be used. </li>
* </ul>
* Other methods uses types of their arguments (i.e.
* <code>java.lang.String</code>
* for <code>setString</code> method, <code>com.sun.star.sdbc.XRef</code>
* for <code>setRef</code> method).
* </li>
* </ul>
* @see com.sun.star.sdbc.XParameters
*/
public class _XParameters extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XParameters oObj = null ;

    private Vector data = null ;

    /**
    * Gets object relation
    */
    public void before() {
        data = (Vector) tEnv.getObjRelation("XParameters.ParamValues") ;
        if (data == null) {
            log.println("!!! Relation not found !!!") ;
        }
    }

    /**
    * Sets String parameter (if exists) to SQL NULL value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setNull() {
        boolean result = true ;
        int idx = findParamOfType(String.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setNull(idx, DataType.VARCHAR) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setNull()", result) ;
    }

    public void _setObjectNull() {
        /*
            !!! TO DO !!!
        */
        tRes.tested("setObjectNull()", Status.skipped(true)) ;
    }

    /**
    * Sets String parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setString() {
        boolean result = true ;
        int idx = findParamOfType(String.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setString(idx, "XParameters") ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setString()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setBoolean() {
        boolean result = true ;
        int idx = findParamOfType(Boolean.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setBoolean(idx, true) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setBoolean()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setByte() {
        boolean result = true ;
        int idx = findParamOfType(Byte.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setByte(idx, (byte)122) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setByte()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setShort() {
        boolean result = true ;
        int idx = findParamOfType(Short.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setShort(idx, (short)133) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setShort()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setInt() {
        boolean result = true ;
        int idx = findParamOfType(Integer.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setInt(idx, 13300) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setInt()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setLong() {
        boolean result = true ;
        int idx = findParamOfType(Long.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setLong(idx, 13362453) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setLong()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setFloat() {
        boolean result = true ;
        int idx = findParamOfType(Float.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setFloat(idx, (float)133.55) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setFloat()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setDouble() {
        boolean result = true ;
        int idx = findParamOfType(Double.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setDouble(idx, 133) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setDouble()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setBytes() {
        boolean result = true ;
        int idx = findParamOfType(byte[].class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setBytes(idx, new byte[] {5}) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setBytes()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setDate() {
        boolean result = true ;
        int idx = findParamOfType(Date.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setDate(
                    idx, new Date ((short)19, (short)01, (short)1979)) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setDate()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setTime() {
        boolean result = true ;
        int idx = findParamOfType(Time.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setTime(
                    idx, new Time((short)1,(short)2,(short)3,(short)44)) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setTime()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setTimestamp() {
        boolean result = true ;
        int idx = findParamOfType(DateTime.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                oObj.setTimestamp(idx, new DateTime((short)1,(short)2,(short)3,
                    (short)4, (short)19, (short)01, (short)1979)) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setTimestamp()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setBinaryStream() {
        boolean result = true ;
        int idx = findParamOfType(XDataInputStream.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                Object oStream = ((XMultiServiceFactory)tParam.getMSF()).
                        createInstance("com.sun.star.io.DataInputStream") ;
                XInputStream xStream = (XInputStream)UnoRuntime.queryInterface
                    (XInputStream.class, oStream);

                oObj.setBinaryStream(idx, xStream, 2) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            } catch (com.sun.star.uno.Exception e) {
                log.println("Unexpected exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setBinaryStream()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setCharacterStream() {
        boolean result = true ;
        int idx = findParamOfType(XTextInputStream.class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                Object oStream = ((XMultiServiceFactory)tParam.getMSF())
                        .createInstance("com.sun.star.io.TextInputStream") ;
                XInputStream xStream = (XInputStream)UnoRuntime.queryInterface
                    (XInputStream.class, oStream);

                oObj.setCharacterStream(idx, xStream, 2) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            } catch (com.sun.star.uno.Exception e) {
                log.println("Unexpected exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setCharacterStream()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setObject() {
        boolean result = true ;
        int idx = findParamOfType(Object[].class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                Object obj = ((XMultiServiceFactory)tParam.getMSF()).
                                createInstance("com.sun.star.io.Pipe") ;

                oObj.setObject(idx, obj) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            } catch (com.sun.star.uno.Exception e) {
                log.println("Unexpected exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setObject()", result) ;
    }

    /**
    * Sets parameter (if exists) to new value. <p>
    * Has OK status if no exceptions occured.
    */
    public void _setObjectWithInfo() {
        boolean result = true ;
        int idx = findParamOfType(Object[].class) ;
        if (idx < 0) log.println("Type not found in relation: not tested");
        else {
            try {
                Object obj = ((XMultiServiceFactory)tParam.getMSF()).
                    createInstance("com.sun.star.io.Pipe") ;

                oObj.setObjectWithInfo(idx, obj, DataType.OBJECT, 0) ;
            } catch (SQLException e) {
                log.println("Unexpected SQL exception:") ;
                log.println(e) ;
                result = false ;
            } catch (com.sun.star.uno.Exception e) {
                log.println("Unexpected exception:") ;
                log.println(e) ;
                result = false ;
            }
        }

        tRes.tested("setObjectWithInfo()", result) ;
    }

    public void _setRef() {
        /*
            !!! TO DO !!!
        */
        tRes.tested("setRef()", Status.skipped(true)) ;
    }
    public void _setBlob() {
        /*
            !!! TO DO !!!
        */
        tRes.tested("setBlob()", Status.skipped(true)) ;
    }
    public void _setClob() {
        /*
            !!! TO DO !!!
        */
        tRes.tested("setClob()", Status.skipped(true)) ;
    }
    public void _setArray() {
        /*
            !!! TO DO !!!
        */
        tRes.tested("setArray()", Status.skipped(true)) ;
    }

    /**
    * Calls method. <p>
    * Has OK status if no exceptions occured.
    */
    public void _clearParameters() {
        boolean result = true ;
        try {
            oObj.clearParameters() ;
        } catch (SQLException e) {
            log.println("Unexpected SQL exception:") ;
            log.println(e) ;
            result = false ;
        }

        tRes.tested("clearParameters()", result) ;
    }


    /**
    * Finds in relation vector index of parameter of the appropriate
    * type.
    */
    private int findParamOfType(Class clz) {

        for (int i = 0; i < data.size(); i++)
            if (clz.isInstance(data.get(i))) return i + 1 ;
        return -1 ;
    }

}  // finish class _XParameters


