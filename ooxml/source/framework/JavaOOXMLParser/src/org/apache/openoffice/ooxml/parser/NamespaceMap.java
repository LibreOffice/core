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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.HashMap;
import java.util.Map;

public class NamespaceMap
{
    NamespaceMap (final File aDataLocation)
    {
        maUriToPrefixMap = new HashMap<>();

        try
        {
            final BufferedReader aReader = new BufferedReader(
                new FileReader(
                    new File(aDataLocation, "namespaces.lst")));

            while (true)
            {
                final String sLine = aReader.readLine();
                if (sLine == null)
                    break;
                final String aParts[] = sLine.split("\\s+");
                maUriToPrefixMap.put(aParts[0], aParts[1]);
            }

            aReader.close();
        }
        catch (final Exception aException)
        {
            throw new RuntimeException(aException);
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
