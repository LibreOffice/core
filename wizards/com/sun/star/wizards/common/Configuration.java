/*************************************************************************
 *
 *  $RCSfile: Configuration.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $  $Date: 2005-07-05 10:16:59 $
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

import com.sun.star.beans.*;
import com.sun.star.container.*;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.Locale;
import com.sun.star.util.XChangesBatch;

/**
 * This class gives access to the OO configuration api.
 * It contains 4 get and 4 set convenience methods for getting and settings properties
 * in the configuration. <br/>
 * For the get methods, two parameters must be given: name and parent, where name is the
 * name of the property, parent is a HierarchyElement (::com::sun::star::configuration::HierarchyElement)<br/>
 * The get and set methods support hieryrchical property names like "options/gridX". <br/>
 * NOTE: not yet supported, but sometime later,
 * If you will ommit the "parent" parameter, then the "name" parameter must be in hierarchy form from
 * the root of the registry.
 * @author  rpiterman
 */
public abstract class Configuration {

    public static int getInt(String name, Object parent) throws Exception {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
            return 0;
        return AnyConverter.toInt(o);
    }

    public static short getShort(String name, Object parent) throws Exception {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
            return (short) 0;
        return AnyConverter.toShort(o);
    }

    public static float getFloat(String name, Object parent) throws Exception {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
            return (float) 0;
        return AnyConverter.toFloat(o);
    }

    public static double getDouble(String name, Object parent) throws Exception {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
            return (double) 0;
        return AnyConverter.toDouble(o);
    }

    public static String getString(String name, Object parent) throws Exception {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
            return "";
        return (String) o;
    }

    public static boolean getBoolean(String name, Object parent) throws Exception {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
            return false;
        return AnyConverter.toBoolean(o);
    }

    public static Object getNode(String name, Object parent) throws Exception {
        return ((XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, parent)).getByName(name);
    }

    public static void set(int value, String name, Object parent) throws Exception {
        set(new Integer(value), name, parent);
    }

    public static void set(short value, String name, Object parent) throws Exception {
        set(new Short(value), name, parent);
    }

    public static void set(String value, String name, Object parent) throws Exception {
        set((Object) value, name, parent);
    }

    public static void set(boolean value, String name, Object parent) throws Exception {
        if (value = true)
            set(Boolean.TRUE, name, parent);
        else
            set(Boolean.FALSE, name, parent);
    }

    public static void set(Object value, String name, Object parent) throws com.sun.star.lang.IllegalArgumentException, PropertyVetoException, UnknownPropertyException, WrappedTargetException {
        ((XHierarchicalPropertySet) UnoRuntime.queryInterface(XHierarchicalPropertySet.class, parent)).setHierarchicalPropertyValue(name, value);
    }

    /** Creates a new instance of RegistryEntry */
    public static Object getConfigurationNode(String name, Object parent) throws Exception {
        return ((XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, parent)).getByName(name);
    }

    public static Object getConfigurationRoot(XMultiServiceFactory xmsf, String sPath, boolean updateable) throws com.sun.star.uno.Exception {

        Object oConfigProvider;
        oConfigProvider = xmsf.createInstance("com.sun.star.configuration.ConfigurationProvider");
        XMultiServiceFactory confMsf = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, oConfigProvider);

        final String sView = updateable ? "com.sun.star.configuration.ConfigurationUpdateAccess" : "com.sun.star.configuration.ConfigurationAccess";

        Object args[] = new Object[updateable ? 2 : 1];

        PropertyValue aPathArgument = new PropertyValue();
        aPathArgument.Name = "nodepath";
        aPathArgument.Value = sPath;

        args[0] = aPathArgument;

        if (updateable) {

            PropertyValue aModeArgument = new PropertyValue();
            aModeArgument.Name = "lazywrite";
            aModeArgument.Value = Boolean.FALSE;

            args[1] = aModeArgument;
        }

        return confMsf.createInstanceWithArguments(sView, args);
    }

    public static String[] getChildrenNames(Object configView) {
        XNameAccess nameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, configView);
        return nameAccess.getElementNames();
    }

    public static String getProductName(XMultiServiceFactory xMSF) {
        try {
            Object oProdNameAccess = getConfigurationRoot(xMSF, "org.openoffice.Setup/Product", false);
            String ProductName = (String) Helper.getUnoObjectbyName(oProdNameAccess, "ooName");
            return ProductName;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public static Locale getOfficeLocale(XMultiServiceFactory xMSF) {
        try {
            Locale aLocLocale = new Locale();
            Object oMasterKey = getConfigurationRoot(xMSF, "org.openoffice.Setup/L10N/", false);
            String sLocale = (String) Helper.getUnoObjectbyName(oMasterKey, "ooLocale");
            String[] sLocaleList = JavaTools.ArrayoutofString(sLocale, "-");
            aLocLocale.Language = sLocaleList[0];
            if (sLocaleList.length > 1)
                aLocLocale.Country = sLocaleList[1];
            return aLocLocale;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public static String getOfficeLinguistic(XMultiServiceFactory xMSF) {
        try {
            Object oMasterKey = getConfigurationRoot(xMSF, "org.openoffice.Setup/L10N/", false);
            String sLinguistic = (String) Helper.getUnoObjectbyName(oMasterKey, "ooLocale");
            return sLinguistic;
        } catch (Exception exception) {
            exception.printStackTrace();
            return null;
        }
    }

    /**
     * This method creates a new configuration node and adds it
     * to the given view. Note that if a node with the given name
     * already exists it will be completely removed from
     * the configuration.
     * @param configView
     * @param name
     * @return the new created configuration node.
     * @throws com.sun.star.lang.WrappedTargetException
     * @throws ElementExistException
     * @throws NoSuchElementException
     * @throws com.sun.star.uno.Exception
     */
    public static Object addConfigNode(Object configView, String name) throws com.sun.star.lang.WrappedTargetException, ElementExistException, NoSuchElementException, com.sun.star.uno.Exception {

        XNameContainer xNameContainer = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, configView);

        if (xNameContainer == null) {
            XNameReplace xNameReplace = (XNameReplace) UnoRuntime.queryInterface(XNameReplace.class, configView);
            return xNameReplace.getByName(name);
        } else {

            /*if (xNameContainer.hasByName(name))
                xNameContainer.removeByName(name);*/

            // create a new detached set element (instance of DataSourceDescription)
            XSingleServiceFactory xElementFactory = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, configView);

            // the new element is the result !
            Object newNode = xElementFactory.createInstance();
            // insert it - this also names the element
            xNameContainer.insertByName(name, newNode);

            return newNode;
        }
    }

    public static void removeNode(Object configView, String name) throws NoSuchElementException, WrappedTargetException {
        XNameContainer xNameContainer = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, configView);

        if (xNameContainer.hasByName(name))
            xNameContainer.removeByName(name);
    }

    public static void commit(Object configView) throws WrappedTargetException {
        XChangesBatch xUpdateControl = (XChangesBatch) UnoRuntime.queryInterface(XChangesBatch.class, configView);
        xUpdateControl.commitChanges();
    }

    public static void updateConfiguration(XMultiServiceFactory xmsf, String path, String name, ConfigNode node, Object param) throws com.sun.star.uno.Exception, com.sun.star.container.ElementExistException, NoSuchElementException, WrappedTargetException {
        Object view = Configuration.getConfigurationRoot(xmsf, path, true);
        addConfigNode(path, name);
        node.writeConfiguration(view, param);
        XChangesBatch xUpdateControl = (XChangesBatch) UnoRuntime.queryInterface(XChangesBatch.class, view);
        xUpdateControl.commitChanges();
    }

    public static void removeNode(XMultiServiceFactory xmsf, String path, String name) throws com.sun.star.uno.Exception, com.sun.star.container.ElementExistException, NoSuchElementException, WrappedTargetException {
        Object view = Configuration.getConfigurationRoot(xmsf, path, true);
        removeNode(view, name);
        XChangesBatch xUpdateControl = (XChangesBatch) UnoRuntime.queryInterface(XChangesBatch.class, view);
        xUpdateControl.commitChanges();
    }


    public static String[] getNodeDisplayNames(XNameAccess _xNameAccessNode){
    String[] snames = null;
    return getNodeChildNames(_xNameAccessNode, "Name");
    }


    public static String[] getNodeChildNames(XNameAccess xNameAccessNode, String _schildname){
    String[] snames = null;
    try {
        snames = xNameAccessNode.getElementNames();
        String[] sdisplaynames = new String[snames.length];
        for (int i = 0; i < snames.length; i++){
            sdisplaynames[i] = (String) Helper.getUnoPropertyValue(xNameAccessNode.getByName(snames[i]), _schildname);
        }
        return sdisplaynames;
    } catch (Exception e) {
        e.printStackTrace(System.out);
        return snames;
    }}


    public static XNameAccess getChildNodebyIndex(Object _oNode, int _index){
        XNameAccess xNameAccessNode = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, _oNode);
        return getChildNodebyIndex(xNameAccessNode, _index);
    }


    public static XNameAccess getChildNodebyIndex(XNameAccess _xNameAccess, int _index){
    try {
        String[] snames = _xNameAccess.getElementNames();
        Object oNode = _xNameAccess.getByName(snames[_index]);
        XNameAccess xNameAccessNode = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oNode);
        return xNameAccessNode;
    } catch (Exception e) {
        e.printStackTrace(System.out);
        return null;
    }}


    public static XNameAccess getChildNodebyName(XNameAccess _xNameAccessNode, String _SubNodeName){
    try {
        if (_xNameAccessNode.hasByName(_SubNodeName))
            return (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, _xNameAccessNode.getByName(_SubNodeName));
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }
    return null;
    }


    public static XNameAccess getChildNodebyDisplayName(XNameAccess _xNameAccessNode, String _displayname){
        String[] snames = null;
        return getChildNodebyDisplayName(_xNameAccessNode, _displayname, "Name");
    }


    public static XNameAccess getChildNodebyDisplayName(XNameAccess _xNameAccessNode, String _displayname, String _nodename){
    String[] snames = null;
    try {
        snames = _xNameAccessNode.getElementNames();
        String[] sdisplaynames = new String[snames.length];
        for (int i = 0; i < snames.length; i++){
            String curdisplayname = (String) Helper.getUnoPropertyValue(_xNameAccessNode.getByName(snames[i]), _nodename);
            if (curdisplayname.equals(_displayname))
                return (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, _xNameAccessNode.getByName(snames[i]));
        }
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }
    return null;
    }




}
