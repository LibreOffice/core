/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OneOfConstantsMapper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:10 $
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


package com.sun.star.report.pentaho.parser.stylemapper;

import java.util.HashMap;

import com.sun.star.report.pentaho.parser.StyleMapper;
import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.StyleKey;
import org.jfree.layouting.input.style.values.CSSValue;

public abstract class OneOfConstantsMapper implements StyleMapper
{
  private StyleKey styleKey;
  private HashMap mappings;

  protected OneOfConstantsMapper (StyleKey styleKey)
  {
    this.styleKey = styleKey;
    this.mappings = new HashMap();
  }

  public void addMapping (String value, CSSValue target)
  {
    mappings.put(value, target);
  }

  public void updateStyle (String uri,
                           String attrName,
                           String attrValue,
                           CSSDeclarationRule targetRule)
  {
    final CSSValue value = lookupMapping(attrValue);
    if (value != null)
    {
      targetRule.setPropertyValue(styleKey, value);
    }
  }

  public StyleKey getStyleKey ()
  {
    return styleKey;
  }

  protected CSSValue lookupMapping (String attrValue)
  {
    return (CSSValue) mappings.get(attrValue);
  }
}
