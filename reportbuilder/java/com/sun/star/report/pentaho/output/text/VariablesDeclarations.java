/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VariablesDeclarations.java,v $
 * $Revision: 1.5 $
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

    private final AttributeNameGenerator nameGenerator;
    private final Map variables;

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
            holder.put(null, name);
            holder.put("time", name);
            holder.put("date", name);
            holder.put("datetime", name);
            holder.put("float", name);
            holder.put("string", name);
            holder.put("boolean", name);
        }
        holder.put(type, name);
        return result;
    }

    public Map getDefinedMappings()
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
                final String type = (String) entry.getKey();
                if (type != null)
                {
                    final String varName = (String) entry.getValue();
                    mappings.put(varName, type);
                }
            }
        }
        return mappings;
    }
}
