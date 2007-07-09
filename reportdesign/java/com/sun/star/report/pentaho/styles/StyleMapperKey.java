/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleMapperKey.java,v $
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

/**
 * A hash key for the stylemapper.
 *
 * @author Thomas Morgner
 * @since 12.03.2007
 */
public final class StyleMapperKey
{
  private String elementNamespace;
  private String elementName;
  private String attributeNamespace;
  private String attributeName;
  private int hashCode;

  public StyleMapperKey(final String elementNamespace,
                        final String elementName,
                        final String attributeNamespace,
                        final String attributeName)
  {
    if (elementNamespace == null)
    {
      throw new NullPointerException();
    }
    if (elementName == null)
    {
      throw new NullPointerException();
    }

    this.elementNamespace = elementNamespace;
    this.elementName = elementName;
    this.attributeNamespace = attributeNamespace;
    this.attributeName = attributeName;
    this.hashCode = computeHashCode();
  }

  public String getElementNamespace()
  {
    return elementNamespace;
  }

  public String getElementName()
  {
    return elementName;
  }

  public String getAttributeNamespace()
  {
    return attributeNamespace;
  }

  public String getAttributeName()
  {
    return attributeName;
  }

  public boolean equals(final Object o)
  {
    if (this == o)
    {
      return true;
    }
    if (o == null || getClass() != o.getClass())
    {
      return false;
    }

    final StyleMapperKey that = (StyleMapperKey) o;

    if (attributeName != null ? !attributeName.equals(
        that.attributeName) : that.attributeName != null)
    {
      return false;
    }
    if (attributeNamespace != null ? !attributeNamespace.equals(
        that.attributeNamespace) : that.attributeNamespace != null)
    {
      return false;
    }
    if (!elementName.equals(that.elementName))
    {
      return false;
    }
    if (!elementNamespace.equals(that.elementNamespace))
    {
      return false;
    }

    return true;
  }

  private int computeHashCode()
  {
    int result;
    result = elementNamespace.hashCode();
    result = 31 * result + elementName.hashCode();
    result = 31 * result + (attributeNamespace != null ? attributeNamespace.hashCode() : 0);
    result = 31 * result + (attributeName != null ? attributeName.hashCode() : 0);
    return result;
  }

  public int hashCode()
  {
    return hashCode;
  }
}
