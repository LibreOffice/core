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
package org.libreoffice.report.pentaho.styles;

import java.util.HashMap;
import java.util.Map;

import org.pentaho.reporting.libraries.resourceloader.Resource;
import org.pentaho.reporting.libraries.resourceloader.ResourceException;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;


/**
 * The style-mapper holds all information about the OpenOffice style mapping
 * mechanism. OpenOffice references styles by their name and context, a style
 * has a style-family assigned. The style family is determined by the element
 * referencing the style, and there is no easily accessible information
 * available on that.
 * <p/>
 * Therefore this mapper acts as gatekeeper for this information. The style
 * mapping information is read from an external definition file and can be
 * maintained externally.
 *
 * @since 11.03.2007
 */
public class StyleMapper
{

    private final Map<StyleMapperKey,StyleMappingRule> backend;

    public StyleMapper()
    {
        this.backend = new HashMap<StyleMapperKey,StyleMappingRule>();
    }

    public void addMapping(final StyleMappingRule rule)
    {
        backend.put(rule.getKey(), rule);
    }

    public boolean isListOfStyles(final String elementNamespace,
            final String elementTagName,
            final String attributeNamespace,
            final String attributeName)
    {
        final StyleMapperKey key = new StyleMapperKey(elementNamespace, elementTagName, attributeNamespace, attributeName);
        final StyleMappingRule rule = backend.get(key);
        return rule != null && rule.isListOfValues();
    }

    public String getStyleFamilyFor(final String elementNamespace,
            final String elementTagName,
            final String attributeNamespace,
            final String attributeName)
    {
        final StyleMapperKey key = new StyleMapperKey(elementNamespace, elementTagName, attributeNamespace, attributeName);
        final StyleMappingRule rule = backend.get(key);
        if (rule == null)
        {
            return null;
        }
        return rule.getFamily();
    }

    public static StyleMapper loadInstance(final ResourceManager resourceManager)
            throws ResourceException
    {
        final Resource resource = resourceManager.createDirectly("res://org/libreoffice/report/pentaho/styles/stylemapper.xml", StyleMapper.class);
        return (StyleMapper) resource.getResource();
    }
}
