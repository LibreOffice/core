/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VariablesCollection.java,v $
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


package com.sun.star.report.pentaho.layoutprocessor;

import java.util.ArrayList;

import com.sun.star.report.pentaho.model.FormattedTextElement;
import java.util.List;

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
  private List variables;

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
