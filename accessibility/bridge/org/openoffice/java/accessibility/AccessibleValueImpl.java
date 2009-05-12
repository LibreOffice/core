/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleValueImpl.java,v $
 * $Revision: 1.5 $
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
