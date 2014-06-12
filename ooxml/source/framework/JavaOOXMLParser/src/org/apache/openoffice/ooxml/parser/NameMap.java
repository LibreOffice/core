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

public class NameMap
{
    NameMap (final Vector<String[]> aData)
    {
        maNameToIdMap = new HashMap<>();
        maIdToNameMap = new Vector<>();

        for (final String[] aLine : aData)
        {
            final int nId = Integer.parseInt(aLine[1]);

            maNameToIdMap.put(aLine[2], nId);

            if (maIdToNameMap.size() <= nId)
                maIdToNameMap.setSize(nId+1);
            maIdToNameMap.set(nId, aLine[2]);
        }
    }




    public int GetIdForName (
        final String sName)
    {
        if ( ! maNameToIdMap.containsKey(sName))
            throw new RuntimeException("token '"+sName+"' is not known");

        return maNameToIdMap.get(sName);
    }




    public String GetNameForId (final int nId)
    {
        if (nId == -1)
            return "<none>";
        else
            return maIdToNameMap.get(nId);
    }




    public int GetNameCount ()
    {
        return maIdToNameMap.size();
    }




    private final Map<String,Integer> maNameToIdMap;
    private final Vector<String> maIdToNameMap;
}
