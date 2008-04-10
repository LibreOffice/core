/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbg.java,v $
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
import java.io.OutputStream;
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
    public static void printInterfaces(XInterface xTarget,
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
    public static Type[] getInterfaceTypes(XInterface xTarget) {
        Type[] types = null;
        XTypeProvider xTypeProvider = (XTypeProvider)
                UnoRuntime.queryInterface( XTypeProvider.class, xTarget);
        if( xTypeProvider != null )
            types = xTypeProvider.getTypes();
        return types;
    }

    /**
     * Returns true if a specified target implements the interface with the
     * given name. Note that the comparison is not case sensitive.
     * @param xTarget The implementation which should be analysed.
     * @param ifcName The name of the interface that is tested. The name can
     * be full qualified, such as 'com.sun.star.io.XInputStream', or only
     * consist of the interface name, such as 'XText'.
     * @return True, if xTarget implements the interface named ifcType
     * @see com.sun.star.uno.XInterface
     */
    public static boolean implementsInterface(
                                    XInterface xTarget, String ifcName) {
        Type[] types = getInterfaceTypes(xTarget);
        if( null != types ) {
            int nLen = types.length;
            for( int i = 0; i < nLen ; i++ ) {
                if(types[i].getTypeName().toLowerCase().endsWith(
                                                    ifcName.toLowerCase()))
                    return true;
            }
        }
        return false;
    }

    /**
     * Prints information about an interface type.
     *
     * @param aType The type of the given interface.
     * @see com.sun.star.uno.Type
     */
    public static void printInterfaceInfo(Type aType) {
        try {
            Class zClass = aType.getZClass();
            Method[] methods = zClass.getDeclaredMethods();
            for (int i=0; i<methods.length; i++) {
                System.out.println("\t" + methods[i].getReturnType().getName()
                    + " " + methods[i].getName() + "()");
            }
        }
        catch (Exception ex) {
            System.out.println("Exception occured while printing InterfaceInfo");
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
    public static void printPropertyInfo(XPropertySet PS, String name) {
            printPropertyInfo(PS, name, new PrintWriter(System.out)) ;
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
            Property[] props = PSI.getProperties();
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

            if ((attr & PropertyAttribute.REMOVEABLE) != 0)
                    out.println("\t-REMOVEABLE");

            if ((attr & PropertyAttribute.TRANSIENT) != 0)
                    out.println("\t-TRANSIENT");
        } catch(com.sun.star.uno.Exception e) {
                out.println("Exception!!!!");
            e.printStackTrace(out);
        }
    }

    /**
     * Print the names and the values of a sequnze of <code>PropertyValue</code>
     * to to standard out.
     * @param ps The property which should displayed
     * @see com.sun.star.beans.PropertyValue
     */

    public static void printProperyValueSequenzePairs(PropertyValue[] ps){
        for( int i = 0; i < ps.length; i++){
            printProperyValuePairs(ps[i], new PrintWriter(System.out));
        }
    }

    /**
     * Print the names and the values of a sequenze of <code>PropertyValue</code>
     * to a print writer.
     * @param ps The property which should displayed
     * @param out The print writer which is used as output.
     * @see com.sun.star.beans.PropertyValue
     */
    public static void printProperyValueSequenzePairs(PropertyValue[] ps, PrintWriter out){
        for( int i = 0; i < ps.length; i++){
            printProperyValuePairs(ps[i], out);
        }
    }

    /**
     * Print the name and the value of a <code>PropertyValue</code> to to standard out.
     * @param ps The property which should displayed
     * @see com.sun.star.beans.PropertyValue
     */
    public static void printProperyValuePairs(PropertyValue ps){
        printProperyValuePairs(ps, new PrintWriter(System.out));
    }

    /**
     * Print the name and the value of a <code>PropertyValue</code> to a print writer.
     * @param ps The property which should displayed
     * @param out The print writer which is used as output.
     * @see com.sun.star.beans.PropertyValue
     */
    public static void printProperyValuePairs(PropertyValue ps, PrintWriter out){

        if (ps.Value instanceof String[] ){
            String[] values = (String[]) ps.Value;
            String oneValue = "value is an empty String[]";
            if (values.length > 0){
                oneValue = "['";
                for( int i=0; i < values.length; i++){
                    oneValue += values[i];
                    if (i+1 < values.length) oneValue += "';'";
                }
                oneValue += "']";
            }
            out.println("--------");
            out.println("   Name: '" + ps.Name + "' contains String[]:");
            out.println(oneValue);
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
        XServiceInfo xSI = (XServiceInfo)
                UnoRuntime.queryInterface(XServiceInfo.class,aObject);
        printArray(xSI.getSupportedServiceNames());
        String str="Therein not Supported Service";
        boolean notSupportedServices = false;
        for (int i=0;i<xSI.getSupportedServiceNames().length;i++) {
            if (! xSI.supportsService(xSI.getSupportedServiceNames()[i])) {
                notSupportedServices = true;
                str+="\n" + xSI.getSupportedServiceNames()[i];
            }
        }
        if (notSupportedServices)
            System.out.println(str);
    }

    /**
     * Get the unique implementation id of a UNO object.
     * @param xTarget An implementation of a UNO object.
     * @return The implementation id.
     */
    public static String getImplID( XInterface xTarget ) {
    String str = "";
    XTypeProvider xTypeProvider = (XTypeProvider)
                UnoRuntime.queryInterface( XTypeProvider.class, xTarget);
            if( xTypeProvider != null ) {
        byte[] id = xTypeProvider.getImplementationId();
        str = "ImplementationID: ";
        for (int i=0; i<id.length;i++) {
            Byte b = new Byte(id[i]);
            str += b.intValue();
        }
            } else {
        str = "No Implementation ID available";
    }
    return str;
    }


}