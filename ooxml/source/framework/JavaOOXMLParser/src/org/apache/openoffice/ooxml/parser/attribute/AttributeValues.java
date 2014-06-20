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

package org.apache.openoffice.ooxml.parser.attribute;

import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

/** Container of attribute values of an opening tag.
 */
public class AttributeValues
{
    AttributeValues ()
    {
        maRawAttributeValues = new TreeMap<>();
        maProcessedAttributeValues = new TreeMap<>();
    }




    public void AddAttribute (
        final AttributeDescriptor aAttributeDescriptor,
        final String sRawValue,
        final Object aProcessedValue)
    {
        maRawAttributeValues.put(
            aAttributeDescriptor.GetName(),
            sRawValue);
        maProcessedAttributeValues.put(
            aAttributeDescriptor.GetName(),
            aProcessedValue);
    }




    public Iterable<Entry<String,String>> GetAttributes ()
    {
        return maRawAttributeValues.entrySet();
    }




    public String GetRawAttributeValue (final String sName)
    {
        return maRawAttributeValues.get(sName);
    }




    public Object GetProcessedAttributeValue (final String sName)
    {
        return maProcessedAttributeValues.get(sName);
    }




    public int GetAttributeCount ()
    {
        return maRawAttributeValues.size();
    }




    private Map<String,String> maRawAttributeValues;
    private Map<String,Object> maProcessedAttributeValues;
}
