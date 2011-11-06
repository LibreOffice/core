/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.report.pentaho.styles;

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
 * @author Thomas Morgner
 * @since 11.03.2007
 */
public class StyleMapper
{

    private final Map backend;

    public StyleMapper()
    {
        this.backend = new HashMap();
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
        final StyleMappingRule rule = (StyleMappingRule) backend.get(key);
        return rule != null && rule.isListOfValues();
    }

    public String getStyleFamilyFor(final String elementNamespace,
            final String elementTagName,
            final String attributeNamespace,
            final String attributeName)
    {
        final StyleMapperKey key = new StyleMapperKey(elementNamespace, elementTagName, attributeNamespace, attributeName);
        final StyleMappingRule rule = (StyleMappingRule) backend.get(key);
        if (rule == null)
        {
            return null;
        }
        return rule.getFamily();
    }

    public static StyleMapper loadInstance(final ResourceManager resourceManager)
            throws ResourceException
    {
        final Resource resource = resourceManager.createDirectly("res://com/sun/star/report/pentaho/styles/stylemapper.xml", StyleMapper.class);
        return (StyleMapper) resource.getResource();
    }
}
