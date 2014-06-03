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
import java.util.Vector;

public class NameMap
{
    NameMap (final File aDataLocation)
    {
        maNameToIdMap = new HashMap<>();
        maIdToNameMap = new Vector<>();

        try
        {
            final BufferedReader aReader = new BufferedReader(
                new FileReader(
                    new File(aDataLocation, "names.lst")));

            while (true)
            {
                final String sLine = aReader.readLine();
                if (sLine == null)
                    break;
                final String aParts[] = sLine.split("\\s+");
                final int nId = Integer.parseInt(aParts[0]);
                maNameToIdMap.put(aParts[1], nId);
                if (maIdToNameMap.size() <= nId)
                    maIdToNameMap.setSize(nId+1);
                maIdToNameMap.set(nId, aParts[1]);
            }

            aReader.close();
        }
        catch (final Exception aException)
        {
            throw new RuntimeException(aException);
        }

        if (Log.Dbg != null)
            Log.Dbg.printf("initialized name map with %d definitions\n", maNameToIdMap.size());
    }




    public int GetIdForName (
        final String sPrefix,
        final String sElementName)
    {
        final String sName;
        if (sPrefix == null)
            sName = sElementName;
        else
            sName = sPrefix+"_"+sElementName;

        if ( ! maNameToIdMap.containsKey(sName))
            throw new RuntimeException("token '"+sName+"' is not known");

        return maNameToIdMap.get(sName);
    }




    public String GetNameForId (final int nId)
    {
        return maIdToNameMap.get(nId);
    }




    private final Map<String,Integer> maNameToIdMap;
    private final Vector<String> maIdToNameMap;
}
