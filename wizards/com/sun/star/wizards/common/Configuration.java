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
 */
public abstract class Configuration
{

    public static int getInt(String name, Object parent) throws Exception
    {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
        {
            return 0;
        }
        return AnyConverter.toInt(o);
    }

    public static short getShort(String name, Object parent) throws Exception
    {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
        {
            return (short) 0;
        }
        return AnyConverter.toShort(o);
    }

    public static float getFloat(String name, Object parent) throws Exception
    {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
        {
            return 0;
        }
        return AnyConverter.toFloat(o);
    }

    public static double getDouble(String name, Object parent) throws Exception
    {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
        {
            return 0;
        }
        return AnyConverter.toDouble(o);
    }

    public static String getString(String name, Object parent) throws Exception
    {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
        {
            return PropertyNames.EMPTY_STRING;
        }
        return (String) o;
    }

    public static boolean getBoolean(String name, Object parent) throws Exception
    {
        Object o = getNode(name, parent);
        if (AnyConverter.isVoid(o))
        {
            return false;
        }
        return AnyConverter.toBoolean(o);
    }

    public static Object getNode(String name, Object parent) throws Exception
    {
        return UnoRuntime.queryInterface(XNameAccess.class, parent).getByName(name);
    }

    public static void set(int value, String name, Object parent) throws Exception
    {
        set(new Integer(value), name, parent);
    }

    public static void set(short value, String name, Object parent) throws Exception
    {
        set(new Short(value), name, parent);
    }

    public static void set(String value, String name, Object parent) throws Exception
    {
        set((Object) value, name, parent);
    }

    public static void set(boolean value, String name, Object parent) throws Exception
    {
        if (value)
        {
            set(Boolean.TRUE, name, parent);
        }
        else
        {
            set(Boolean.FALSE, name, parent);
        }
    }

    public static void set(Object value, String name, Object parent) throws com.sun.star.lang.IllegalArgumentException, PropertyVetoException, UnknownPropertyException, WrappedTargetException
    {
        UnoRuntime.queryInterface(XHierarchicalPropertySet.class, parent).setHierarchicalPropertyValue(name, value);
    }

    /** Creates a new instance of RegistryEntry
     * @param name
     * @param parent
     * @return
     * @throws Exception
     */
    public static Object getConfigurationNode(String name, Object parent) throws Exception
    {
        return UnoRuntime.queryInterface(XNameAccess.class, parent).getByName(name);
    }

    public static Object getConfigurationRoot(XMultiServiceFactory xmsf, String sPath, boolean updateable) throws com.sun.star.uno.Exception
    {

        Object oConfigProvider;
        oConfigProvider = xmsf.createInstance("com.sun.star.configuration.ConfigurationProvider");
        XMultiServiceFactory confMsf = UnoRuntime.queryInterface(XMultiServiceFactory.class, oConfigProvider);

        final String sView = updateable ? "com.sun.star.configuration.ConfigurationUpdateAccess" : "com.sun.star.configuration.ConfigurationAccess";

        Object args[] = new Object[updateable ? 2 : 1];

        PropertyValue aPathArgument = new PropertyValue();
        aPathArgument.Name = "nodepath";
        aPathArgument.Value = sPath;

        args[0] = aPathArgument;

        if (updateable)
        {

            PropertyValue aModeArgument = new PropertyValue();
            aModeArgument.Name = "lazywrite";
            aModeArgument.Value = Boolean.FALSE;

            args[1] = aModeArgument;
        }

        return confMsf.createInstanceWithArguments(sView, args);
    }

    public static String[] getChildrenNames(Object configView)
    {
        XNameAccess nameAccess = UnoRuntime.queryInterface(XNameAccess.class, configView);
        return nameAccess.getElementNames();
    }

    public static String getProductName(XMultiServiceFactory xMSF)
    {
        try
        {
            Object oProdNameAccess = getConfigurationRoot(xMSF, "org.openoffice.Setup/Product", false);
            return (String) Helper.getUnoObjectbyName(oProdNameAccess, "ooName");
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public static String getOfficeLocaleString(XMultiServiceFactory xMSF)
    {
        String sLocale = PropertyNames.EMPTY_STRING;
        try
        {
            Locale aLocLocale = new Locale();
            Object oMasterKey = getConfigurationRoot(xMSF, "org.openoffice.Setup/L10N/", false);
            sLocale = (String) Helper.getUnoObjectbyName(oMasterKey, "ooLocale");
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return sLocale;
    }

    public static Locale getOfficeLocale(XMultiServiceFactory xMSF)
    {
        Locale aLocLocale = new Locale();
        String sLocale = getOfficeLocaleString(xMSF);
        String[] sLocaleList = JavaTools.ArrayoutofString(sLocale, "-");
        aLocLocale.Language = sLocaleList[0];
        if (sLocaleList.length > 1)
        {
            aLocLocale.Country = sLocaleList[1];
        }
        return aLocLocale;
    }

    public static String getOfficeLinguistic(XMultiServiceFactory xMSF)
    {
        try
        {
            Object oMasterKey = getConfigurationRoot(xMSF, "org.openoffice.Setup/L10N/", false);
            return (String) Helper.getUnoObjectbyName(oMasterKey, "ooLocale");
        }
        catch (Exception exception)
        {
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
    public static Object addConfigNode(Object configView, String name) throws com.sun.star.lang.WrappedTargetException, ElementExistException, NoSuchElementException, com.sun.star.uno.Exception
    {

        XNameContainer xNameContainer = UnoRuntime.queryInterface(XNameContainer.class, configView);

        if (xNameContainer == null)
        {
            XNameReplace xNameReplace = UnoRuntime.queryInterface(XNameReplace.class, configView);
            return xNameReplace.getByName(name);
        }
        else
        {

            // create a new detached set element (instance of DataSourceDescription)
            XSingleServiceFactory xElementFactory = UnoRuntime.queryInterface(XSingleServiceFactory.class, configView);

            // the new element is the result !
            Object newNode = xElementFactory.createInstance();
            // insert it - this also names the element
            xNameContainer.insertByName(name, newNode);

            return newNode;
        }
    }

    public static void removeNode(Object configView, String name) throws NoSuchElementException, WrappedTargetException
    {
        XNameContainer xNameContainer = UnoRuntime.queryInterface(XNameContainer.class, configView);

        if (xNameContainer.hasByName(name))
        {
            xNameContainer.removeByName(name);
        }
    }

    public static void commit(Object configView) throws WrappedTargetException
    {
        XChangesBatch xUpdateControl = UnoRuntime.queryInterface(XChangesBatch.class, configView);
        xUpdateControl.commitChanges();
    }

    public static void updateConfiguration(XMultiServiceFactory xmsf, String path, String name, ConfigNode node, Object param) throws com.sun.star.uno.Exception, com.sun.star.container.ElementExistException, NoSuchElementException, WrappedTargetException
    {
        Object view = Configuration.getConfigurationRoot(xmsf, path, true);
        addConfigNode(path, name);
        node.writeConfiguration(view, param);
        XChangesBatch xUpdateControl = UnoRuntime.queryInterface(XChangesBatch.class, view);
        xUpdateControl.commitChanges();
    }

    public static void removeNode(XMultiServiceFactory xmsf, String path, String name) throws com.sun.star.uno.Exception, com.sun.star.container.ElementExistException, NoSuchElementException, WrappedTargetException
    {
        Object view = Configuration.getConfigurationRoot(xmsf, path, true);
        removeNode(view, name);
        XChangesBatch xUpdateControl = UnoRuntime.queryInterface(XChangesBatch.class, view);
        xUpdateControl.commitChanges();
    }

    public static String[] getNodeDisplayNames(XNameAccess _xNameAccessNode)
    {
        String[] snames = null;
        return getNodeChildNames(_xNameAccessNode, PropertyNames.PROPERTY_NAME);
    }

    public static String[] getNodeChildNames(XNameAccess xNameAccessNode, String _schildname)
    {
        String[] snames = null;
        try
        {
            snames = xNameAccessNode.getElementNames();
            String[] sdisplaynames = new String[snames.length];
            for (int i = 0; i < snames.length; i++)
            {
                Object oContent = Helper.getUnoPropertyValue(xNameAccessNode.getByName(snames[i]), _schildname);
                if (!AnyConverter.isVoid(oContent))
                {
                    sdisplaynames[i] = (String) Helper.getUnoPropertyValue(xNameAccessNode.getByName(snames[i]), _schildname);
                }
                else
                {
                    sdisplaynames[i] = snames[i];
                }
            }
            return sdisplaynames;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return snames;
        }
    }

    public static XNameAccess getChildNodebyIndex(XNameAccess _xNameAccess, int _index)
    {
        try
        {
            String[] snames = _xNameAccess.getElementNames();
            Object oNode = _xNameAccess.getByName(snames[_index]);
            return UnoRuntime.queryInterface(XNameAccess.class, oNode);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return null;
        }
    }

    public static XNameAccess getChildNodebyName(XNameAccess _xNameAccessNode, String _SubNodeName)
    {
        try
        {
            if (_xNameAccessNode.hasByName(_SubNodeName))
            {
                return UnoRuntime.queryInterface(XNameAccess.class, _xNameAccessNode.getByName(_SubNodeName));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return null;
    }

    public static XNameAccess getChildNodebyDisplayName(XNameAccess _xNameAccessNode, String _displayname)
    {
        String[] snames = null;
        return getChildNodebyDisplayName(_xNameAccessNode, _displayname, PropertyNames.PROPERTY_NAME);
    }

    public static XNameAccess getChildNodebyDisplayName(XNameAccess _xNameAccessNode, String _displayname, String _nodename)
    {
        String[] snames = null;
        try
        {
            snames = _xNameAccessNode.getElementNames();
            String[] sdisplaynames = new String[snames.length];
            for (int i = 0; i < snames.length; i++)
            {
                String curdisplayname = (String) Helper.getUnoPropertyValue(_xNameAccessNode.getByName(snames[i]), _nodename);
                if (curdisplayname.equals(_displayname))
                {
                    return UnoRuntime.queryInterface(XNameAccess.class, _xNameAccessNode.getByName(snames[i]));
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return null;
    }

    public static XNameAccess getChildNodebyDisplayName(XMultiServiceFactory _xMSF, Locale _aLocale, XNameAccess _xNameAccessNode, String _displayname, String _nodename, int _nmaxcharcount)
    {
        String[] snames = null;
        try
        {
            snames = _xNameAccessNode.getElementNames();
            String[] sdisplaynames = new String[snames.length];
            for (int i = 0; i < snames.length; i++)
            {
                String curdisplayname = (String) Helper.getUnoPropertyValue(_xNameAccessNode.getByName(snames[i]), _nodename);
                if ((_nmaxcharcount > 0) && (_nmaxcharcount < curdisplayname.length()))
                {
                    curdisplayname = curdisplayname.substring(0, _nmaxcharcount);
                }
                curdisplayname = Desktop.removeSpecialCharacters(_xMSF, _aLocale, curdisplayname);

                if (curdisplayname.equals(_displayname))
                {
                    return UnoRuntime.queryInterface(XNameAccess.class, _xNameAccessNode.getByName(snames[i]));
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return null;
    }
}
