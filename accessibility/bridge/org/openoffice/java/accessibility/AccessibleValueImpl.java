/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleValueImpl.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:34:55 $
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
