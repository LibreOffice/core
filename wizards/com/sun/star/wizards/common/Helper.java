/*************************************************************************
*
*  $RCSfile: Helper.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:36:26 $
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
*/

package com.sun.star.wizards.common;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.UnoRuntime;

public class Helper {

    /** Creates a new instance of Helper */
    public Helper() {
    }

    public static long convertUnoDatetoInteger(com.sun.star.util.Date DateValue) {
        java.util.Calendar oCal = java.util.Calendar.getInstance();
        oCal.set(DateValue.Year, DateValue.Month, DateValue.Day);
        java.util.Date dTime = oCal.getTime();
        long lTime = dTime.getTime();
        long lDate = lTime / (3600 * 24000);
        return lDate;
    }

    public static void setUnoPropertyValue(Object oUnoObject, String PropertyName, Object PropertyValue) {
        try {
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
            if (xPSet.getPropertySetInfo().hasPropertyByName(PropertyName))
                xPSet.setPropertyValue(PropertyName, PropertyValue);
            else{
                Property[] selementnames = xPSet.getPropertySetInfo().getProperties();
                throw new Exception();
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public static Object getUnoObjectbyName(Object oUnoObject, String ElementName) {
        try {
            com.sun.star.container.XNameAccess xName = (com.sun.star.container.XNameAccess) UnoRuntime.queryInterface(com.sun.star.container.XNameAccess.class, oUnoObject);
            if (xName.hasByName(ElementName) == true)
                return xName.getByName(ElementName);
            else
                throw new RuntimeException();
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public static Object getPropertyValue(PropertyValue[] CurPropertyValue, String PropertyName) {
        int MaxCount = CurPropertyValue.length;
        for (int i = 0; i < MaxCount; i++) {
            if (CurPropertyValue[i] != null) {
                if (CurPropertyValue[i].Name == PropertyName) {
                    return CurPropertyValue[i].Value;
                }
            }
        }
        throw new RuntimeException();
    }

    public static Object getUnoPropertyValue(Object oUnoObject, String PropertyName, java.lang.Class xClass) {
        try {
            if (oUnoObject != null) {
                XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
                Object oObject = xPSet.getPropertyValue(PropertyName);
                return com.sun.star.uno.AnyConverter.toObject(new com.sun.star.uno.Type(xClass), oObject);
            }
            return null;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public static Object getPropertyValuefromAny(Object[] CurPropertyValue, String PropertyName) {
        if (CurPropertyValue != null) {
            int MaxCount = CurPropertyValue.length;
            for (int i = 0; i < MaxCount; i++) {
                if (CurPropertyValue[i] != null) {
                    PropertyValue aValue = (PropertyValue) CurPropertyValue[i];
                    if (aValue != null && aValue.Name.equals(PropertyName))
                        return aValue.Value;
                }
            }
        }
        //  System.out.println("Property not found: " + PropertyName);
        return null;
    }

    public static Object getPropertyValuefromAny(Object[] CurPropertyValue, String PropertyName, java.lang.Class xClass) {
        try {
            if (CurPropertyValue != null) {
                int MaxCount = CurPropertyValue.length;
                for (int i = 0; i < MaxCount; i++) {
                    if (CurPropertyValue[i] != null) {
                        PropertyValue aValue = (PropertyValue) CurPropertyValue[i];
                        if (aValue != null && aValue.Name.equals(PropertyName))
                            return com.sun.star.uno.AnyConverter.toObject(new com.sun.star.uno.Type(xClass), aValue.Value);
                    }
                }
            }
            //  System.out.println("Property not found: " + PropertyName);
            return null;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public static Object getUnoPropertyValue(Object oUnoObject, String PropertyName) {
        try {
            if (oUnoObject != null) {
                XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
                Property[] aProps = xPSet.getPropertySetInfo().getProperties();
                Object oObject = xPSet.getPropertyValue(PropertyName);
//              Any aAny = (Any) oObject;
                return oObject;
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
        return null;
    }

    public static Object getUnoArrayPropertyValue(Object oUnoObject, String PropertyName) {
        try {
            if (oUnoObject != null) {
                XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
                Object oObject = xPSet.getPropertyValue(PropertyName);
                if (AnyConverter.isArray(oObject))
                    return getArrayValue(oObject);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
        return null;
    }

    public static Object getUnoStructValue(Object oUnoObject, String PropertyName) {
        try {
            if (oUnoObject != null) {
                XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
                if (xPSet.getPropertySetInfo().hasPropertyByName(PropertyName) == true) {
                    Object oObject = xPSet.getPropertyValue(PropertyName);
                    return oObject;
                }
            }
            return null;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public static void setUnoPropertyValues(Object oUnoObject, String[] PropertyNames, Object[] PropertyValues) {
        try {
            com.sun.star.beans.XMultiPropertySet xMultiPSetLst = (com.sun.star.beans.XMultiPropertySet) UnoRuntime.queryInterface(com.sun.star.beans.XMultiPropertySet.class, oUnoObject);
            xMultiPSetLst.setPropertyValues(PropertyNames, PropertyValues);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    /**
     * @author bc93774
     * checks if the value of an object that represents an array is null.
     * check beforehand if the Object is really an array with "AnyConverter.IsArray(oObject)
     * @param oValue the paramter that has to represent an object
     * @return a null reference if the array is empty
     */
    public static Object getArrayValue(Object oValue) {
        try {
            Object oPropList = com.sun.star.uno.AnyConverter.toArray(oValue);
            int nlen = java.lang.reflect.Array.getLength(oPropList);
            if (nlen == 0)
                return null;
            else
                return oPropList;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

}
