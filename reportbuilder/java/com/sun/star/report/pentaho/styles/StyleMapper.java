/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StyleMapper.java,v $
 * $Revision: 1.4 $
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
    final StyleMapperKey key = new StyleMapperKey
        (elementNamespace, elementTagName, attributeNamespace, attributeName);
    final StyleMappingRule rule = (StyleMappingRule) backend.get(key);
    if (rule == null)
    {
      return false;
    }
    return rule.isListOfValues();
  }

  public String getStyleFamilyFor(final String elementNamespace,
                                  final String elementTagName,
                                  final String attributeNamespace,
                                  final String attributeName)
  {
    final StyleMapperKey key = new StyleMapperKey
        (elementNamespace, elementTagName, attributeNamespace, attributeName);
    final StyleMappingRule rule = (StyleMappingRule) backend.get(key);
    if (rule == null)
    {
      return null;
    }
    return rule.getFamily();
  }

  public static StyleMapper loadInstance (final ResourceManager resourceManager)
      throws ResourceException
  {
    final Resource resource = resourceManager.createDirectly
        ("res://com/sun/star/report/pentaho/styles/stylemapper.xml", StyleMapper.class);
    return (StyleMapper) resource.getResource();
  }
}
