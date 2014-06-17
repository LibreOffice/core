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

package org.apache.openoffice.ooxml.parser;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

public class NamespaceMap
{
    public class NamespaceDescriptor
    {
        NamespaceDescriptor (final String sPrefix, final int nId)
        {
            Prefix = sPrefix;
            Id = nId;
        }
        public final String Prefix;
        public final int Id;
    }
    NamespaceMap (final Vector<String[]> aData)
    {
        maUriToDescriptorMap = new HashMap<>();
        maIdToDescriptorMap = new HashMap<>();

        for (final String[] aLine : aData)
        {
            final int nId = Integer.parseInt(aLine[2]);
            final NamespaceDescriptor aDescriptor = new NamespaceDescriptor(aLine[1], nId);
            maUriToDescriptorMap.put(
                aLine[3],
                aDescriptor);
            maIdToDescriptorMap.put(
                nId,
                aDescriptor);
        }
    }




    public NamespaceDescriptor GetDescriptorForURI (final String sURI)
    {
        if (sURI == null)
            throw new RuntimeException("namespace is null");
        if ( ! maUriToDescriptorMap.containsKey(sURI))
            throw new RuntimeException("namespace '"+sURI+"' is not known");
        return maUriToDescriptorMap.get(sURI);
    }




    public NamespaceDescriptor GetDescriptorForId (final int nId)
    {
        return maIdToDescriptorMap.get(nId);
    }




    public int GetNamespaceCount ()
    {
        return maUriToDescriptorMap.size();
    }




    private final Map<String,NamespaceDescriptor> maUriToDescriptorMap;
    private final Map<Integer,NamespaceDescriptor> maIdToDescriptorMap;
}
