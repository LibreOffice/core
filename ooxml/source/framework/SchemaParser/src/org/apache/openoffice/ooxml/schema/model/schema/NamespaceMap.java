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
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

/** Map between namespace prefixes and URIs.
 *  While namespace URIs can have different prefixes in different schemas,
 *  we will use only one prefix in the OOXML parser.  This class
 *  provides these global prefixes.
 */
public class NamespaceMap
    implements Iterable<Entry<String, String>>
{
    public NamespaceMap ()
    {
        maURIToPrefixMap = new HashMap<>(maPredefinedURIToPrefixMap);

        // Hack to cover the older file format.
        maURIToPrefixMap.put("http://schemas.openxmlformats.org/drawingml/2006/main", "a06");
        maURIToPrefixMap.put("http://purl.oclc.org/ooxml/drawingml/main", "a12");

        maURIToPrefixMap.put("http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing", "wp06");
        maURIToPrefixMap.put("http://purl.oclc.org/ooxml/drawingml/wordprocessingDrawing", "wp12");

        maURIToPrefixMap.put("http://schemas.openxmlformats.org/wordprocessingml/2006/main", "w06");
        maURIToPrefixMap.put("http://purl.oclc.org/ooxml/wordprocessingml/main", "w12");

        maURIToPrefixMap.put("http://schemas.openxmlformats.org/drawingml/2006/picture", "dpct06");
        maURIToPrefixMap.put("http://purl.oclc.org/ooxml/drawingml/picture", "dpct12");

        maURIToPrefixMap.put("http://schemas.openxmlformats.org/officeDocument/2006/math", "m06");
        maURIToPrefixMap.put("http://purl.oclc.org/ooxml/officeDocument/math", "m12");

        maURIToPrefixMap.put("http://purl.oclc.org/ooxml/officeDocument/relationships", "r12");
    }




    public void ProvideNamespace (
        final String sNamespaceURI,
        final String sDefaultPrefix)
    {
        if ( ! maURIToPrefixMap.containsKey(sNamespaceURI))
        {
            final String sPrefix;
            if (sDefaultPrefix != null)
            {
                // Check if we can use the given prefix.
                if (IsPrefixUsed(sDefaultPrefix))
                {
                    // Prefix is already used.  We have to create a new and unique one.
                    sPrefix = new String(new byte[]{(byte)('A'+maURIToPrefixMap.size())});
                }
                else
                {
                    // Use the given prefix.
                    sPrefix = sDefaultPrefix;
                }
            }
            else
            {
                // Namespace is used without prefix.  Don't invent one.
                // It is OK that the resulting null value is not unique.
                sPrefix = null;
            }

            maURIToPrefixMap.put(sNamespaceURI, sPrefix);
        }
    }




    public String GetNamespacePrefix (final String sURI)
    {
        return maURIToPrefixMap.get(sURI);
    }




    public Iterator<Entry<String,String>> iterator ()
    {
        return maURIToPrefixMap.entrySet().iterator();
    }




    public Object GetCount()
    {
        return maURIToPrefixMap.size();
    }




    private boolean IsPrefixUsed (final String sPrefix)
    {
        for (final String sUsedPrefix : maURIToPrefixMap.values())
        {
            if (sUsedPrefix == null)
                continue;
            if (sUsedPrefix.equals(sPrefix))
                return true;
        }
        return false;
    }




    private final Map<String,String> maURIToPrefixMap;
    private final static Map<String,String> maPredefinedURIToPrefixMap;
    static
    {
        maPredefinedURIToPrefixMap =  new HashMap<>();
        maPredefinedURIToPrefixMap.put("http://www.w3.org/2001/XMLSchema", "xsd");
    }
}
