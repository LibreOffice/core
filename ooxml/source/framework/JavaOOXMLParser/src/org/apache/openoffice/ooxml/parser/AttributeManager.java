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

import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

public class AttributeManager
{
    public AttributeManager (final File aDataLocation)
    {
        maStateIdToAttributesMap = new HashMap<>();
    }




    public void ParseAttributes (
        final int nStateId,
        final AttributeProvider aAttributeProvider)
    {
        final Map<String,String> aAttributeDefinitions = maStateIdToAttributesMap.get(nStateId);
        if (aAttributeDefinitions == null)
        {
           // if (aAttributeProvider.HasAttributes())
                //throw new RuntimeException();
        }
        else
        {
            for (final Entry<String,String> aEntry : aAttributeProvider)
            {
                ParseAttributeValue(
                    aEntry.getKey(),
                    aEntry.getValue(),
                    aAttributeDefinitions.get(aEntry.getKey()));
            }
        }
    }



    private void ParseAttributeValue (
        final String sName,
        final String sValue,
        final String sSimpleTypeName)
    {
        Log.Dbg.printf("attribute %s has type %s and value %s\n",
            sName,
            sSimpleTypeName,
            sValue);
    }




    private final Map<String,Map<String,String>> maStateIdToAttributesMap;
}
