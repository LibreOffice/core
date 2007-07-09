/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VariablesCollection.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:06 $
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


package com.sun.star.report.pentaho.layoutprocessor;

import java.util.ArrayList;

import com.sun.star.report.pentaho.model.FormattedTextElement;

/**
 * A variables collection is used to collect all FormattedTextElement objects
 * of a repeated header or footer. Later, for each of these elements a variable
 * setter is inserted into a hidden (in fact just very small) paragraph. These
 * variables can later be read using the 'variable-get' construct.
 *
 * From the idea, this is equal to the 'strings' declaration of CSS3, although
 * this code is explicit instead of declarative.
 *
 * @author Thomas Morgner
 * @since 22.03.2007
 */
public class VariablesCollection
{
  private VariablesCollection parent;
  private String namePrefix;
  private ArrayList variables;

  public VariablesCollection(final String namePrefix)
  {
    this(namePrefix, null);
  }

  public VariablesCollection(final String namePrefix, final VariablesCollection parent)
  {
    if (namePrefix == null)
    {
      throw new NullPointerException("NamePrefix cannot be null");
    }

    this.namePrefix = namePrefix;
    this.parent = parent;
    this.variables = new ArrayList();
  }

  public VariablesCollection getParent()
  {
    return parent;
  }

  public String getNamePrefix()
  {
    return namePrefix;
  }

  public String addVariable (final FormattedTextElement element)
  {
    variables.add(element);
    final int size = variables.size();
    return namePrefix + size;
  }

  public FormattedTextElement[] getVariables ()
  {
    return (FormattedTextElement[])
        variables.toArray(new FormattedTextElement[variables.size()]);
  }

  public int getVariablesCount ()
  {
    return variables.size();
  }


}
