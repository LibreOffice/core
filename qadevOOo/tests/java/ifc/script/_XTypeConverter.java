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

package ifc.script;

import lib.MultiMethodTest;

import com.sun.star.container.XSet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.XTypeConverter;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;

/**
* Testing <code>com.sun.star.script.XTypeConverter</code>
* interface methods :
* <ul>
*  <li><code> convertTo()</code></li>
*  <li><code> convertToSimpleType()</code></li>
* </ul> <p>
* @see com.sun.star.script.XTypeConverter
*/
public class _XTypeConverter extends MultiMethodTest {

    /**
     * oObj filled by MultiMethodTest
     */
    public XTypeConverter oObj = null;

    /**
    * Test creates instance of <code>TypeDescriptionManager</code>,
    * and converts it to <code>XSet</code>. <p>
    * Has <b> OK </b> status if returned value is instance of <code>XSet</code>
    * and no exceptions were thrown. <p>
    * @see com.sun.star.comp.stoc.TypeDescriptionManager
    * @see com.sun.star.container.XSet
    */
    public void _convertTo() {
        XMultiServiceFactory xMSF = tParam.getMSF() ;
        Object value = null ;

        try {
            value = xMSF.createInstance
                ("com.sun.star.comp.stoc.TypeDescriptionManager") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create value to convert") ;
            e.printStackTrace(log) ;
            tRes.tested("convertTo()", false) ;
        }

        try {
            Type destType = new Type(XSet.class) ;

            Object o = oObj.convertTo(value, destType);

            boolean result;
            if (o instanceof Any) {
                result = ((Any)o).getType().equals(destType);
            }
            else {
                result = (o instanceof XSet);
            }

            tRes.tested("convertTo()", result) ;
        } catch (com.sun.star.script.CannotConvertException e) {
            log.println("Exception while converting value.") ;
            e.printStackTrace(log) ;
            tRes.tested("convertTo()", false) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while converting value.") ;
            e.printStackTrace(log) ;
            tRes.tested("convertTo()", false) ;
        }
    }

    /**
    * Test creates instance of <code>Integer</code>,
    * and converts it to <code>String</code>. <p>
    * Has <b> OK </b> status if returned value is instance of <code>String</code>,
    * if returned value is string representation of integer value that
    * was passed to method and no exceptions were thrown. <p>
    */
    public void _convertToSimpleType() {
        Object value = Integer.valueOf(123) ;
        Object destValue = null ;

        try {
            destValue = oObj.convertToSimpleType(value, TypeClass.STRING) ;
        } catch (com.sun.star.script.CannotConvertException e) {
            log.println("Exception while converting value.") ;
            e.printStackTrace(log) ;
            tRes.tested("convertToSimpleType()", false) ;
            return ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while converting value.") ;
            e.printStackTrace(log) ;
            tRes.tested("convertToSimpleType()", false) ;
            return ;
        }

        tRes.tested("convertToSimpleType()", destValue != null &&
            destValue instanceof String &&
            ((String) destValue).equals("123")) ;
    }
}

