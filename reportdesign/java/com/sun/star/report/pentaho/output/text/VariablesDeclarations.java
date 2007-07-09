/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VariablesDeclarations.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:08 $
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


package com.sun.star.report.pentaho.output.text;

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;

import org.jfree.report.util.AttributeNameGenerator;

/**
 * A collection that holds all used variables. A variable is primarily keyed by
 * its original name. If a variable contains more than one type, it is also
 * keyed by the type.
 *
 * @author Thomas Morgner
 * @since 26.03.2007
 */
public class VariablesDeclarations
{
  private AttributeNameGenerator nameGenerator;
  private HashMap variables;

  public VariablesDeclarations()
  {
    variables = new HashMap();
    nameGenerator = new AttributeNameGenerator();
  }

  public String produceVariable(final String name,
                                final String type)
  {
    HashMap holder = (HashMap) variables.get(name);
    if (holder == null)
    {
      holder = new HashMap();
      variables.put(name, holder);
    }

    final String mapping = (String) holder.get(type);
    if (mapping != null)
    {
      return mapping;
    }
    final String result = nameGenerator.generateName(name);
    if (holder.isEmpty())
    {
      // create the default mapping as well..
      holder.put (null, name);
    }
    holder.put (type, name);
    return result;
  }

  public Map getDefinedMappings ()
  {
    final HashMap mappings = new HashMap();
    final Iterator vars = variables.values().iterator();
    while (vars.hasNext())
    {
      final HashMap types = (HashMap) vars.next();
      final Iterator varsByType = types.entrySet().iterator();
      while (varsByType.hasNext())
      {
        final Map.Entry entry = (Map.Entry) varsByType.next();
        final String varName = (String) entry.getValue();
        final String type = (String) entry.getKey();
        if (type != null)
        {
          mappings.put (varName, type);
        }
      }
    }
    return mappings;
  }
}
