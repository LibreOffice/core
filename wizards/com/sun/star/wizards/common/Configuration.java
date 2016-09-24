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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.Locale;

/**
 * This class gives access to the OO configuration API.
 * <p>It contains 4 get and 4 set convenience methods for getting and settings properties
 * in the configuration.</p>
 * <p>For the get methods, two parameters must be given: name and parent, where name is the
 * name of the property, parent is a HierarchyElement (::com::sun::star::configuration::HierarchyElement)</p>
 * <p>The get and set methods support hierarchical property names like "options/gridX". </p>
 * <p>NOTE: not yet supported, but sometime later,
 * If you will omit the "parent" parameter, then the "name" parameter must be in hierarchy form from
 * the root of the registry.</p>
 */
public abstract class Configuration
{

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

    private static String getLocaleString(XMultiServiceFactory xMSF, String root, String key)
    {
        String sLocale = PropertyNames.EMPTY_STRING;
        try
        {
            Object oMasterKey = getConfigurationRoot(xMSF, root, false);
            sLocale = (String) Helper.getUnoObjectbyName(oMasterKey, key);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        if (sLocale.length() == 0 && (!key.equals("Locale") || !root.equals("org.openoffice.System/L10N/")))
        {
            return getLocaleString(xMSF, "org.openoffice.System/L10N/", "Locale");
        }
        return sLocale;
    }

    private static Locale getLocale(XMultiServiceFactory xMSF, String root, String key)
    {
        Locale aLocLocale = new Locale();
        String sLocale = getLocaleString(xMSF, root, key);
        String[] sLocaleList = JavaTools.ArrayoutofString(sLocale, "-");
        aLocLocale.Language = sLocaleList[0];
        if (sLocaleList.length > 1)
        {
            aLocLocale.Country = sLocaleList[1];
        }
        return aLocLocale;
    }

    public static Locale getLocale(XMultiServiceFactory xMSF)
    {
        return getLocale(xMSF, "org.openoffice.Setup/L10N/", "ooSetupSystemLocale");
    }

    public static String[] getNodeDisplayNames(XNameAccess _xNameAccessNode)
    {
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

    public static XNameAccess getChildNodebyIndex(XNameAccess _xNameAccess, int _index) throws com.sun.star.uno.Exception
    {
        String[] snames = _xNameAccess.getElementNames();
        Object oNode = _xNameAccess.getByName(snames[_index]);
        return UnoRuntime.queryInterface(XNameAccess.class, oNode);
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

    public static XNameAccess getChildNodebyDisplayName(XMultiServiceFactory _xMSF, Locale _aLocale, XNameAccess _xNameAccessNode, String _displayname, String _nodename, int _nmaxcharcount)
    {
        String[] snames = null;
        try
        {
            snames = _xNameAccessNode.getElementNames();
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
