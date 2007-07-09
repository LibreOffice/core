/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleMappingRule.java,v $
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
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 12.03.2007
 */
public class StyleMappingRule
{
  private StyleMapperKey key;
  private String family;
  private boolean listOfValues;


  public StyleMappingRule(final StyleMapperKey key, final String family,
                          final boolean listOfValues)
  {
    this.key = key;
    this.family = family;
    this.listOfValues = listOfValues;
  }


  public StyleMapperKey getKey()
  {
    return key;
  }

  public String getFamily()
  {
    return family;
  }

  public boolean isListOfValues()
  {
    return listOfValues;
  }
}
