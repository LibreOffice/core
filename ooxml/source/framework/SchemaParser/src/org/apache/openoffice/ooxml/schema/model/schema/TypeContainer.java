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

package org.apache.openoffice.ooxml.schema.model.schema;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;

import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

public class TypeContainer<T extends Node>
{
    TypeContainer ()
    {
        maTypes = new HashMap<>();
    }




    public void Add (final T aType)
    {
        maTypes.put(aType.GetName().GetDisplayName(), aType);
    }




    public T Get (final QualifiedName aName)
    {
        return maTypes.get(aName.GetDisplayName());
    }




    public T Get (final String sPrefixedName)
    {
        return maTypes.get(sPrefixedName);
    }




    public boolean Contains (final String sPrefixedName)
    {
        return maTypes.containsKey(sPrefixedName);
    }




    public int GetCount ()
    {
        return maTypes.size();
    }




    public Iterable<T> GetUnsorted ()
    {
        return maTypes.values();
    }




    public Iterable<T> GetSorted ()
    {
        final Set<T> aSortedItems = new TreeSet<T>(maTypes.values());
        return aSortedItems;
    }




    Map<String,T> maTypes;
}
