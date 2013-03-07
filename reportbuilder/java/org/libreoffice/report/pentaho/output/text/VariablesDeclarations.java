/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report.pentaho.output.text;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.jfree.report.util.AttributeNameGenerator;


/**
 * A collection that holds all used variables. A variable is primarily keyed by
 * its original name. If a variable contains more than one type, it is also
 * keyed by the type.
 *
 * @since 26.03.2007
 */
public class VariablesDeclarations
{

    private final AttributeNameGenerator nameGenerator;
    private final Map<String, HashMap<String,String> > variables;

    public VariablesDeclarations()
    {
        variables = new HashMap<String, HashMap<String,String> >();
        nameGenerator = new AttributeNameGenerator();
    }

    public String produceVariable(final String name,
            final String type)
    {
        HashMap<String,String> holder = variables.get(name);
        if (holder == null)
        {
            holder = new HashMap<String,String>();
            variables.put(name, holder);
        }

        final String mapping = holder.get(type);
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

    public Map<String,String> getDefinedMappings()
    {
        final HashMap<String,String> mappings = new HashMap<String,String>();
        final Iterator<HashMap<String,String>> vars = variables.values().iterator();
        while (vars.hasNext())
        {
            final HashMap<String,String> types = vars.next();
            final Iterator<Map.Entry<String,String>> varsByType = types.entrySet().iterator();
            while (varsByType.hasNext())
            {
                final Map.Entry<String,String> entry = varsByType.next();
                final String type = entry.getKey();
                if (type != null)
                {
                    final String varName = entry.getValue();
                    mappings.put(varName, type);
                }
            }
        }
        return mappings;
    }
}
