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

package org.openoffice.java.accessibility;

import com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.uno.AnyConverter;

/** The AccessibleValueImpl mappes the calls to the java AccessibleValue
 *  interface to the corresponding methods of the UNO XAccessibleValue interface
 */
public class AccessibleValueImpl implements javax.accessibility.AccessibleValue {
    protected XAccessibleValue unoObject;

    /** Creates new AccessibleValueImpl */
    public AccessibleValueImpl(XAccessibleValue xAccessibleValue) {
        unoObject = xAccessibleValue;
    }

    public static java.lang.Number toNumber(java.lang.Object any) {
        try {
            if(AnyConverter.isByte(any)) {
                return new Byte(AnyConverter.toByte(any));
            } else if (AnyConverter.isShort(any)) {
                return new Short(AnyConverter.toShort(any));
            } else if (AnyConverter.isInt(any)) {
                return new Integer(AnyConverter.toInt(any));
            } else if (AnyConverter.isLong(any)) {
                return new Long(AnyConverter.toLong(any));
            } else if (AnyConverter.isFloat(any)) {
                return new Float(AnyConverter.toFloat(any));
            } else if (AnyConverter.isDouble(any)) {
                return new Double(AnyConverter.toDouble(any));
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }

        return null;
    }

    public java.lang.Number getMinimumAccessibleValue() {
        try {
            return toNumber(unoObject.getMinimumValue());
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    public java.lang.Number getCurrentAccessibleValue() {
        try {
            return toNumber(unoObject.getCurrentValue());
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    public java.lang.Number getMaximumAccessibleValue() {
        try {
            return toNumber(unoObject.getMaximumValue());
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    public boolean setCurrentAccessibleValue(java.lang.Number number) {
        try {
            return unoObject.setCurrentValue(number);
        } catch (com.sun.star.uno.RuntimeException e) {
            return false;
        }
    }
}
