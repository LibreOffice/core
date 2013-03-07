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
package org.libreoffice.report.pentaho.parser;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.jfree.layouting.namespace.NamespaceDefinition;
import org.jfree.layouting.namespace.Namespaces;
import org.jfree.report.JFreeReportBoot;

import org.pentaho.reporting.libraries.base.config.DefaultConfiguration;
import org.pentaho.reporting.libraries.resourceloader.Resource;
import org.pentaho.reporting.libraries.resourceloader.ResourceException;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;


public class OfficeParserUtil
{

    private static final Log LOGGER = LogFactory.getLog(OfficeParserUtil.class);
    private static OfficeParserUtil instance;
    private static final String NAMESPACES_PREFIX = "namespaces.";

    public static synchronized OfficeParserUtil getInstance()
    {
        if (instance == null)
        {
            instance = new OfficeParserUtil();
        }
        return instance;
    }
    private final DefaultConfiguration props;
    private final NamespaceDefinition[] namespaces;

    private OfficeParserUtil()
    {
        props = new DefaultConfiguration();

        final ResourceManager resourceManager = new ResourceManager();
        resourceManager.registerDefaults();
        try
        {
            final Resource res = resourceManager.createDirectly("res://org/libreoffice/report/pentaho/parser/selectors.properties", Properties.class);
            final Properties resProps = (Properties) res.getResource();
            props.putAll(resProps);
        }
        catch (ResourceException e)
        {
            LOGGER.warn("Unable to load mapping rules. Parsing services may not be available.", e);
        }

        namespaces = Namespaces.createFromConfig(JFreeReportBoot.getInstance().getGlobalConfig(),
                "org.jfree.report.namespaces.", resourceManager);
    }

    public NamespaceDefinition getNamespaceDeclaration(final String uri)
    {
        if (uri == null)
        {
            throw new NullPointerException("URI must not be null");
        }

        for (int i = 0; i < namespaces.length; i++)
        {
            final NamespaceDefinition definition = namespaces[i];
            if (uri.equals(definition.getURI()))
            {
                return definition;
            }
        }
        return null;
    }

    public String getGenericFont(final String officeFont)
    {
        return props.getProperty("font-family." + officeFont.toLowerCase(), officeFont);
    }

    public String getNamespaceURI(final String namespacePrefix)
    {
        return props.getProperty(NAMESPACES_PREFIX + namespacePrefix);
    }

    public String getNamespaceForStyleFamily(final String styleFamily)
    {
        return props.getProperty("style-family." + styleFamily);
    }

    public Map<String,String> getNamespaces()
    {
        final Map<String,String> map = new HashMap<String,String>();
        final Iterator keys = props.findPropertyKeys(NAMESPACES_PREFIX);
        while (keys.hasNext())
        {
            final String key = (String) keys.next();
            final String value = props.getConfigProperty(key);
            map.put(key.substring(NAMESPACES_PREFIX.length()), value);
        }
        return map;
    }

    public String getNamespacePrefix(final String namespaceURI)
    {
        final Iterator keys = props.findPropertyKeys(NAMESPACES_PREFIX);
        while (keys.hasNext())
        {
            final String key = (String) keys.next();
            final String value = props.getConfigProperty(key);
            if (namespaceURI.equals(value))
            {
                return key.substring(NAMESPACES_PREFIX.length());
            }
        }
        return null;
    }

    public AttributeSpecification parseStyleAttrDefinition(final String key, final String prefix, final String tagname)
    {
        final String configPrefix = "attr." + prefix + "." + tagname + ".";
        final String configSuffix = key.substring(configPrefix.length());
        final int dotPosition = configSuffix.indexOf('.');
        if (dotPosition == -1)
        {
            return null;
        }
        final String namespaceUri = getNamespaceURI(configSuffix.substring(0, dotPosition));
        final String attrName = configSuffix.substring(dotPosition + 1);
        final String value = props.getProperty(key);
        return new AttributeSpecification(namespaceUri, attrName, value);
    }

    public Iterator findStylesForElement(final String prefix,
            final String tagname)
    {
        final String configPrefix = "attr." + prefix + "." + tagname + ".";
        return props.findPropertyKeys(configPrefix);
    }

    public boolean isValidStyleElement(final String uri, final String tagName)
    {
        final String prefix = getNamespacePrefix(uri);
        if (prefix == null)
        {
            return false;
        }

        final Iterator stylesForElement = findStylesForElement(prefix, tagName);
        return stylesForElement.hasNext();
    }

    public String getSelectorPattern()
    {
        return props.getConfigProperty("style-selector.pattern");
    }

    public static void main(final String[] args)
    {
        JFreeReportBoot.getInstance().start();
        System.out.print(OfficeParserUtil.getInstance().getNamespaces());
    }
}
