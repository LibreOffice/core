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

package util;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import java.io.PrintWriter;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Method;

/**
 * This class accumulates all kinds of methods for accessing debug information
 * from UNO implementations.
 */
public class dbg {

    /**
     * Prints information about the supported interfaces of an implementation
     * to standard out.
     * @param xTarget The implementation which should be analysed.
     * @see com.sun.star.uno.XInterface
     */
    public static void printInterfaces(XInterface xTarget) {
        printInterfaces(xTarget, false);
    }

    /**
     * Prints information about the supported interfaces of an implementation
     * to standard out. Extended information can be printed.
     * @param xTarget The implementation which should be analysed.
     * @param extendedInfo Should extended information be printed?
     * @see com.sun.star.uno.XInterface
     */
    private static void printInterfaces(XInterface xTarget,
                                                    boolean extendedInfo){
        Type[] types = getInterfaceTypes(xTarget);
        if( null != types ) {
            int nLen = types.length;
            for( int i = 0; i < nLen ; i++ ) {
                System.out.println(types[i].getTypeName());
                if (extendedInfo) {
                    printInterfaceInfo(types[i]);
                    System.out.println();
                }
            }
        }
    }

    /**
     * Returns all interface types of an implementation as a type array.
     * @param xTarget The implementation which should be analyzed.
     * @return An array with all interface types; null if there are none.
     * @see com.sun.star.uno.XInterface
     */
    private static Type[] getInterfaceTypes(XInterface xTarget) {
        Type[] types = null;
        XTypeProvider xTypeProvider = UnoRuntime.queryInterface( XTypeProvider.class, xTarget);
        if( xTypeProvider != null )
            types = xTypeProvider.getTypes();
        return types;
    }



    /**
     * Prints information about an interface type.
     *
     * @param aType The type of the given interface.
     * @see com.sun.star.uno.Type
     */
    private static void printInterfaceInfo(Type aType) {
        try {
            Class<?> zClass = aType.getZClass();
            Method[] methods = zClass.getDeclaredMethods();
            for (int i=0; i<methods.length; i++) {
                System.out.println("\t" + methods[i].getReturnType().getName()
                    + " " + methods[i].getName() + "()");
            }
        }
        catch (Exception ex) {
            System.out.println("Exception occurred while printing InterfaceInfo");
            ex.printStackTrace();
        }
    }

    /**
     * Prints a string array to standard out.
     *
     * @param entries : The array to be printed.
     */
    public static void printArray( String [] entries ) {
            for ( int i=0; i< entries.length;i++ ) {
                    System.out.println(entries[i]);
            }
    }

    /**
     * Print all information about the property <code>name</code> from
     * the property set <code>PS</code> to standard out.
     * @param PS The property set which should contain a property called
     *           <code>name</code>.
     * @param name The name of the property.
     * @see com.sun.star.beans.XPropertySet
     */
    public static void printPropertyInfo(XPropertySet PS, String name) throws UnsupportedEncodingException {
            printPropertyInfo(PS, name, new PrintWriter(new OutputStreamWriter(System.out, "UTF-8")));
    }

    /**
     * Print all information about the property <code>name</code> from
     * the property set <code>PS</code> to a print writer.
     * @param PS The property set which should contain a property called
     *           <code>name</code>.
     * @param name The name of the property.
     * @param out The print writer which is used as output.
     * @see com.sun.star.beans.XPropertySet
     */
    public static void printPropertyInfo(XPropertySet PS, String name,
                                                        PrintWriter out) {
        try {
            XPropertySetInfo PSI = PS.getPropertySetInfo();
            PSI.getProperties();
            Property prop = PSI.getPropertyByName(name);
            out.println("Property name is " + prop.Name);
            out.println("Property handle is " + prop.Handle);
            out.println("Property type is " + prop.Type.getTypeName());
            out.println("Property current value is " +
                                                    PS.getPropertyValue(name));
            out.println("Attributes :");
            short attr = prop.Attributes;

            if ((attr & PropertyAttribute.BOUND) != 0)
                    out.println("\t-BOUND");

            if ((attr & PropertyAttribute.CONSTRAINED) != 0)
                    out.println("\t-CONSTRAINED");

            if ((attr & PropertyAttribute.MAYBEAMBIGUOUS) != 0)
                    out.println("\t-MAYBEAMBIGUOUS");

            if ((attr & PropertyAttribute.MAYBEDEFAULT) != 0)
                    out.println("\t-MAYBEDEFAULT");

            if ((attr & PropertyAttribute.MAYBEVOID) != 0)
                    out.println("\t-MAYBEVOID");

            if ((attr & PropertyAttribute.READONLY) != 0)
                    out.println("\t-READONLY");

            if ((attr & PropertyAttribute.REMOVABLE) != 0)
                    out.println("\t-REMOVABLE");

            if ((attr & PropertyAttribute.TRANSIENT) != 0)
                    out.println("\t-TRANSIENT");
        } catch(com.sun.star.uno.Exception e) {
                out.println("Exception!!!!");
            e.printStackTrace(out);
        }
    }



    /**
     * Print the names and the values of a sequenze of <code>PropertyValue</code>
     * to a print writer.
     * @param ps The property which should displayed
     * @param out The print writer which is used as output.
     * @see com.sun.star.beans.PropertyValue
     */
    private static void printProperyValueSequenzePairs(PropertyValue[] ps, PrintWriter out){
        for( int i = 0; i < ps.length; i++){
            printProperyValuePairs(ps[i], out);
        }
    }



    /**
     * Print the name and the value of a <code>PropertyValue</code> to a print writer.
     * @param ps The property which should displayed
     * @param out The print writer which is used as output.
     * @see com.sun.star.beans.PropertyValue
     */
    private static void printProperyValuePairs(PropertyValue ps, PrintWriter out){

        if (ps.Value instanceof String[] ){
            String[] values = (String[]) ps.Value;
            StringBuilder oneValue = new StringBuilder("value is an empty String[]");
            if (values.length > 0){
                oneValue.append("['");
                for( int i=0; i < values.length; i++){
                    oneValue.append(values[i]);
                    if (i+1 < values.length) oneValue.append("';'");
                }
                oneValue.append("']");
            }
            out.println("--------");
            out.println("   Name: '" + ps.Name + "' contains String[]:");
            out.println(oneValue.toString());
            out.println("--------");

        } else if (ps.Value instanceof PropertyValue){
            out.println("--------");
            out.println("   Name: '" + ps.Name + "' contains PropertyValue:");
            printProperyValuePairs((PropertyValue)ps.Value, out);
            out.println("--------");

        } else if (ps.Value instanceof PropertyValue[]){
            out.println("--------");
            out.println("   Name: '" + ps.Name + "' contains PropertyValue[]:");
            printProperyValueSequenzePairs((PropertyValue[])ps.Value, out);
            out.println("--------");

        } else {
            out.println("Name: '" + ps.Name + "' Value: '" + ps.Value.toString() + "'");
        }
    }

    /**
     * Print the names of all properties inside this property set
     * @param ps The property set which is printed.
     * @see com.sun.star.beans.XPropertySet
     */
    public static void printPropertiesNames(XPropertySet ps) {
            XPropertySetInfo psi = ps.getPropertySetInfo();
            Property[] props = psi.getProperties();
            for (int i = 0; i < props.length; i++)
                    System.out.println(i + ".  " + props[i].Name);
    }

    /**
     * Print the supported services of a UNO object.
     * @param aObject A UNO object.
     */
    public static void getSuppServices (Object aObject) {
        XServiceInfo xSI = UnoRuntime.queryInterface(XServiceInfo.class,aObject);
        printArray(xSI.getSupportedServiceNames());
        StringBuilder str = new StringBuilder("Therein not Supported Service");
        boolean notSupportedServices = false;
        for (int i=0;i<xSI.getSupportedServiceNames().length;i++) {
            if (! xSI.supportsService(xSI.getSupportedServiceNames()[i])) {
                notSupportedServices = true;
                str.append("\n").append(xSI.getSupportedServiceNames()[i]);
            }
        }
        if (notSupportedServices)
            System.out.println(str.toString());
    }
}
