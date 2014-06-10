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
    NamespaceMap (final Vector<String[]> aData)
    {
        maUriToPrefixMap = new HashMap<>();

        for (final String[] aLine : aData)
        {
            maUriToPrefixMap.put(aLine[2], aLine[1]);
        }

        if (Log.Dbg != null)
            Log.Dbg.printf("initialized namespace map with %d definitions\n", maUriToPrefixMap.size());
    }




    public String GetPrefixForURI (final String sURI)
    {
        if ( ! maUriToPrefixMap.containsKey(sURI))
            throw new RuntimeException("namespace '"+sURI+"' is not known");
        return maUriToPrefixMap.get(sURI);
    }




    private final Map<String,String> maUriToPrefixMap;
}
