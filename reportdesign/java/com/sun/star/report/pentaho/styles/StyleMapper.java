/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleMapper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:12 $
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


package com.sun.star.report.pentaho.styles;

import java.util.HashMap;

import org.jfree.resourceloader.Resource;
import org.jfree.resourceloader.ResourceException;
import org.jfree.resourceloader.ResourceManager;

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
  private HashMap backend;

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
