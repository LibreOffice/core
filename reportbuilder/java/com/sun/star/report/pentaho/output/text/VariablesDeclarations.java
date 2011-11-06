/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.report.pentaho.output.text;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

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
