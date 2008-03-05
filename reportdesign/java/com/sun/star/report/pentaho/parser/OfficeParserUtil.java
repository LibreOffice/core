/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeParserUtil.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:39:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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
package com.sun.star.report.pentaho.parser;

import java.util.Iterator;
import java.util.Properties;
import java.util.Map;
import java.util.HashMap;

import org.jfree.resourceloader.Resource;
import org.jfree.resourceloader.ResourceException;
import org.jfree.resourceloader.ResourceManager;
import org.jfree.util.DefaultConfiguration;
import org.jfree.util.Log;
import org.jfree.report.JFreeReportBoot;
import org.jfree.layouting.namespace.NamespaceDefinition;
import org.jfree.layouting.namespace.Namespaces;

public class OfficeParserUtil
{

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
    private DefaultConfiguration props;
    private NamespaceDefinition[] namespaces;

    private OfficeParserUtil()
    {
        props = new DefaultConfiguration();

        ResourceManager resourceManager = new ResourceManager();
        resourceManager.registerDefaults();
        try
        {
            final Resource res = resourceManager.createDirectly("res://com/sun/star/report/pentaho/parser/selectors.properties", Properties.class);
            final Properties resProps = (Properties) res.getResource();
            props.putAll(resProps);
        }
        catch (ResourceException e)
        {
            Log.warn("Unable to load mapping rules. Parsing services may not be available.", e);
        }

        namespaces = Namespaces.createFromConfig(JFreeReportBoot.getInstance().getGlobalConfig(),
                "org.jfree.report.namespaces.", resourceManager);
    }

    public NamespaceDefinition getNamespaceDeclaration(String uri)
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

    public String getGenericFont(String officeFont)
    {
        return props.getProperty("font-family." + officeFont.toLowerCase(), officeFont);
    }

    public String getNamespaceURI(String namespacePrefix)
    {
        return props.getProperty(NAMESPACES_PREFIX + namespacePrefix);
    }

    public String getNamespaceForStyleFamily(String styleFamily)
    {
        return props.getProperty("style-family." + styleFamily);
    }

    public Map getNamespaces()
    {
        Map map = new HashMap();
        Iterator keys = props.findPropertyKeys(NAMESPACES_PREFIX);
        while (keys.hasNext())
        {
            String key = (String) keys.next();
            String value = props.getConfigProperty(key);
            map.put(key.substring(NAMESPACES_PREFIX.length()), value);
        }
        return map;
    }

    public String getNamespacePrefix(String namespaceURI)
    {
        Iterator keys = props.findPropertyKeys(NAMESPACES_PREFIX);
        while (keys.hasNext())
        {
            String key = (String) keys.next();
            String value = props.getConfigProperty(key);
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

    public boolean isValidStyleElement(String uri, String tagName)
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

    public static void main(String[] args)
    {
        JFreeReportBoot.getInstance().start();
        System.out.print(OfficeParserUtil.getInstance().getNamespaces());
    }
}
