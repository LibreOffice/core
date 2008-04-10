/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RegistryTools.java,v $
 * $Revision: 1.3 $
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

// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import java.io.PrintWriter ;

import com.sun.star.registry.XRegistryKey ;
import com.sun.star.registry.XSimpleRegistry ;
import com.sun.star.registry.RegistryKeyType ;
import com.sun.star.registry.RegistryValueType ;
import com.sun.star.registry.InvalidRegistryException ;
import com.sun.star.lang.XMultiServiceFactory ;
import com.sun.star.uno.Exception;

public class RegistryTools {

    /**
    * Creates 'com.sun.star.registry.SimpleRegistry'
    * service.
    * @param xMSF Multiservice factory.
    * @return Service created.
    */
    public static XSimpleRegistry createRegistryService
        (XMultiServiceFactory xMSF) throws com.sun.star.uno.Exception {

        Object oInterface = xMSF.createInstance
            ("com.sun.star.registry.SimpleRegistry");
        return (XSimpleRegistry) UnoRuntime.queryInterface (
            XSimpleRegistry.class, oInterface) ;
    }

    /**
    * Opens registry file for reading/writing. If file doesn't
    * exist a new one created.
    * @param file Registry file name.
    * @param xMSF Multiservice factory.
    * @return Opened registry.
    */
    public static XSimpleRegistry openRegistry
        (String file, XMultiServiceFactory xMSF)
        throws com.sun.star.uno.Exception {

        XSimpleRegistry reg = createRegistryService(xMSF) ;

        reg.open(file, false, true) ;

        return reg ;
    }

    /**
    * Compares two registry keys, their names, value
    * types and values.
    * return <code>true</code> if key names, value types
    * and values are equal, else returns <code>false</code>.
    */
    public static boolean compareKeys
        (XRegistryKey key1, XRegistryKey key2) {

        if (key1 == null || key2 == null ||
            !key1.isValid() || !key2.isValid())

            return false ;

        String keyName1 = getShortKeyName(key1.getKeyName()) ;
        String keyName2 = getShortKeyName(key2.getKeyName()) ;

        if (!keyName1.equals(keyName2)) return false ;

        try {
            if (key1.getValueType() != key2.getValueType()) return false ;
        } catch (InvalidRegistryException e) {
            return false ;
        }

        RegistryValueType type ;
        try {
            type = key1.getValueType() ;

            if (type.equals(RegistryValueType.ASCII)) {
                if (!key1.getAsciiValue().equals(key2.getAsciiValue()))
                    return false ;
            } else
            if (type.equals(RegistryValueType.STRING)) {
                if (!key1.getStringValue().equals(key2.getStringValue()))
                    return false ;
            } else
            if (type.equals(RegistryValueType.LONG)) {
                if (key1.getLongValue() != key2.getLongValue())
                    return false ;
            } else
            if (type.equals(RegistryValueType.BINARY)) {
                byte[] bin1 = key1.getBinaryValue() ;
                byte[] bin2 = key2.getBinaryValue() ;
                if (bin1.length != bin2.length)
                    return false ;
                for (int i = 0; i < bin1.length; i++)
                    if (bin1[i] != bin2[i]) return false ;
            } else
            if (type.equals(RegistryValueType.ASCIILIST)) {
                String[] list1 = key1.getAsciiListValue() ;
                String[] list2 = key2.getAsciiListValue() ;
                if (list1.length != list2.length)
                    return false ;
                for (int i = 0; i < list1.length; i++)
                    if (!list1[i].equals(list2[i])) return false ;
            } else
            if (type.equals(RegistryValueType.STRINGLIST)) {
                String[] list1 = key1.getStringListValue() ;
                String[] list2 = key2.getStringListValue() ;
                if (list1.length != list2.length)
                    return false ;
                for (int i = 0; i < list1.length; i++)
                    if (!list1[i].equals(list2[i])) return false ;
            } else
            if (type.equals(RegistryValueType.LONGLIST)) {
                int[] list1 = key1.getLongListValue() ;
                int[] list2 = key2.getLongListValue() ;
                if (list1.length != list2.length)
                    return false ;
                for (int i = 0; i < list1.length; i++)
                    if (list1[i] != list2[i]) return false ;
            }
        } catch (Exception e) {
            return false ;
        }

        return true ;
    }

    /**
    * Gets name of the key relative to its parent.
    * For example if full name of key is '/key1/subkey'
    * short key name is 'subkey'
    * @param keyName Full key name.
    * @return Short key name.
    */
    public static String getShortKeyName(String keyName) {
        if (keyName == null) return null ;
        int idx = keyName.lastIndexOf("/") ;
        if (idx < 0) return keyName ;
        else return keyName.substring(idx + 1) ;
    }

    /**
    * Compare all child keys.
    * @param compareRoot If <code>true</code> method also
    * compare root keys, if <code>false</code> it begins recursive
    * comparing from children of root keys.
    * @return <code>true</code> if keys and their sub keys are equal.
    */
    protected static boolean compareKeyTrees
        (XRegistryKey tree1, XRegistryKey tree2, boolean compareRoot) {

        if (compareRoot && !compareKeys(tree1, tree2)) return false ;

        try {
            String[] keyNames1 = tree1.getKeyNames() ;
            String[] keyNames2 = tree2.getKeyNames() ;

            if (keyNames1 == null && keyNames2 == null) return true ;

            if (keyNames1 == null || keyNames2 == null ||
                keyNames2.length != keyNames1.length)
                return false ;

            for (int i = 0; i < keyNames1.length; i++) {

                String keyName = getShortKeyName(keyNames1[i]) ;
                XRegistryKey key2 = tree2.openKey(keyName) ;

                if (key2 == null)
                // key with the same name doesn't exist in the second tree
                    return false ;

                if (!tree1.getKeyType(keyName).equals(
                     tree2.getKeyType(keyName)))
                    return false ;

                if (tree1.getKeyType(keyName).equals(
                    RegistryKeyType.LINK)) {

                    if (!getShortKeyName(tree1.getLinkTarget(keyName)).equals(
                        getShortKeyName(tree2.getLinkTarget(keyName))))

                        return false ;
                } else {

                    if (compareKeyTrees(tree1.openKey(keyName),
                          tree2.openKey(keyName), true) == false) return false ;
                }
            }
        } catch (InvalidRegistryException e) {
            return false ;
        }

        return true ;
    }

    /**
    * Compare keys specified and all their child keys.
    * @return <code>true</code> if keys and their sub keys are equal.
    */
    public static boolean compareKeyTrees
        (XRegistryKey tree1, XRegistryKey tree2) {

        return compareKeyTrees(tree1, tree2, false) ;
    }

    /**
    * Prints to a specified output about all keys and subkeys information
    * (key name, type, value, link target, attributes) recursively.
    * @param reg Registry for which information is needed.
    * @param out Output stream.
    */
    public static void printRegistryInfo(XSimpleRegistry reg, PrintWriter out) {
        try {
            printRegistryInfo(reg.getRootKey(), out) ;
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            out.println("!!! Can't open root registry key for info printing") ;
        }
    }

    /**
    * Prints to a specified output about all keys and subkeys information
    * (key name, type, value, link target, attributes) recursively.
    * @param root Key for which subkeys (and further) information is required.
    * @param out Output stream.
    */
    public static void printRegistryInfo(XRegistryKey root, PrintWriter out) {
        if (root == null) {
            out.println("/(null)") ;
            return ;
        }

        out.println("/") ;
        try {
            printTreeInfo(root, out, "  ") ;
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            out.println("Exception accessing registry :") ;
            e.printStackTrace(out) ;
        }
    }

    private static void printTreeInfo(XRegistryKey key,
        PrintWriter out, String margin)
        throws com.sun.star.registry.InvalidRegistryException {

        String[] subKeys = key.getKeyNames() ;

        if (subKeys == null || subKeys.length == 0) return ;

        for (int i = 0; i < subKeys.length; i++) {
            printKeyInfo(key, subKeys[i], out, margin) ;
            XRegistryKey subKey = key.openKey
                (getShortKeyName(subKeys[i])) ;
            printTreeInfo(subKey, out, margin + "  ") ;
            subKey.closeKey() ;
        }
    }

    private static void printKeyInfo(XRegistryKey parentKey,
        String keyName, PrintWriter out, String margin)
        throws com.sun.star.registry.InvalidRegistryException {

        out.print(margin) ;
        keyName = getShortKeyName(keyName) ;
        XRegistryKey key = parentKey.openKey(keyName) ;
        if (key != null)
            out.print("/" + getShortKeyName(key.getKeyName()) + " ") ;
        else {
            out.println("(null)") ;
            return ;
        }

        if (!key.isValid()) {
            out.println("(not valid)") ;
            return ;
        }

        if (key.isReadOnly()) {
            out.print("(read only) ") ;
        }

        if (parentKey.getKeyType(keyName) == RegistryKeyType.LINK) {
            out.println("(link to " + parentKey.getLinkTarget(keyName) + ")") ;
            return ;
        }

        RegistryValueType type ;
        try {
            type = key.getValueType() ;

            if (type.equals(RegistryValueType.ASCII)) {
                out.println("[ASCII] = '" + key.getAsciiValue() + "'") ;
            } else
            if (type.equals(RegistryValueType.STRING)) {
                out.println("[STRING] = '" + key.getStringValue() + "'") ;
            } else
            if (type.equals(RegistryValueType.LONG)) {
                out.println("[LONG] = " + key.getLongValue()) ;
            } else
            if (type.equals(RegistryValueType.BINARY)) {
                out.print("[BINARY] = {") ;
                byte[] bin = key.getBinaryValue() ;
                for (int i = 0; i < bin.length; i++)
                    out.print("" + bin[i] + ",") ;
                out.println("}") ;
            } else
            if (type.equals(RegistryValueType.ASCIILIST)) {
                out.print("[ASCIILIST] = {") ;
                String[] list = key.getAsciiListValue() ;
                for (int i = 0; i < list.length; i++)
                    out.print("'" + list[i] + "',") ;
                out.println("}") ;
            } else
            if (type.equals(RegistryValueType.STRINGLIST)) {
                out.print("[STRINGLIST] = {") ;
                String[] list = key.getStringListValue() ;
                for (int i = 0; i < list.length; i++)
                    out.print("'" + list[i] + "',") ;
                out.println("}") ;
            } else
            if (type.equals(RegistryValueType.LONGLIST)) {
                out.print("[LONGLIST] = {") ;
                int[] list = key.getLongListValue() ;
                for (int i = 0; i < list.length; i++)
                    out.print("" + list[i] + ",") ;
                out.println("}") ;
            } else {
                out.println("") ;
            }
        } catch (com.sun.star.uno.Exception e) {
            out.println("Exception occured : ") ;
            e.printStackTrace(out) ;
        } finally {
            key.closeKey() ;
        }
    }


//  public static void compareKeyTrees

}