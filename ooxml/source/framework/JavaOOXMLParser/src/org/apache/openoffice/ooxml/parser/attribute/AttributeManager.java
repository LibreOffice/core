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

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Vector;

import org.apache.openoffice.ooxml.parser.Log;
import org.apache.openoffice.ooxml.parser.NameMap;
import org.apache.openoffice.ooxml.parser.NamespaceMap;
import org.apache.openoffice.ooxml.parser.type.SimpleTypeManager;


/** Match a set of attributes from the document with the attribute
 *  specifications of a state.
 *
 */
public class AttributeManager
{
    /** Create a new AttributeManager for the attribute specifications that
     *  are given in the parse table.
     */
    public AttributeManager (
        final Vector<String[]> aData,
        final NamespaceMap aNamespaceMap,
        final NameMap aNameMap,
        final NameMap aStateNameMap,
        final SimpleTypeManager aSimpleTypeManager)
    {
        maStateIdToAttributesMap = new HashMap<>();
        maNamespaceMap = aNamespaceMap;
        maNameMap = aNameMap;
        maStateNameMap = aStateNameMap;
        maSimpleTypeManager = aSimpleTypeManager;

        ParseData(aData);
    }




    private void ParseData (final Vector<String[]> aData)
    {
        for (final String[] aLine : aData)
        {
            final int nStateId = Integer.parseInt(aLine[1]);
            final int nPrefixId = Integer.parseInt(aLine[2]);
            final boolean bCanBeUnqualified = aLine[3].startsWith("u");
            final int nAttributeId = Integer.parseInt(aLine[4]);
            final int nAttributeTypeId = aLine[5].equals("null") ? -1 : Integer.parseInt(aLine[5]);
            final boolean bIsOptional = aLine[6].startsWith("o");
            final String sDefault = aLine[7];
            // State name.
            final String sAttributeName = aLine[9];
            // Attribute type name.

            Map<Integer,AttributeDescriptor> aAttributesPerState = maStateIdToAttributesMap.get(nStateId);
            if (aAttributesPerState == null)
            {
                aAttributesPerState = new HashMap<>();
                maStateIdToAttributesMap.put(nStateId, aAttributesPerState);
            }

            final AttributeDescriptor aAttributeDescriptor = new AttributeDescriptor(
                nPrefixId,
                nAttributeId,
                bCanBeUnqualified,
                bIsOptional,
                sDefault,
                sAttributeName,
                nAttributeTypeId);

            aAttributesPerState.put(
                (nPrefixId<<16)|nAttributeId,
                aAttributeDescriptor);
            if (bCanBeUnqualified)
                aAttributesPerState.put(
                    nAttributeId,
                    aAttributeDescriptor);
        }
    }




    /** For the state with id nStateId, match the attributes from the document
     *  with the attribute specifications of that state.
     */
    public AttributeValues ParseAttributes (
        final int nStateId,
        final AttributeProvider aDocumentAttributes)
    {
        final AttributeValues aValues = new AttributeValues();

        final Map<Integer,AttributeDescriptor> aAttributesPerState = maStateIdToAttributesMap.get(nStateId);
        if (aAttributesPerState == null)
        {
            if (aDocumentAttributes.HasAttributes())
            {
                Log.Std.printf("state has not attributes defined but document provides %d attributes\n",
                    aDocumentAttributes.GetAttributeCount());
                for (final String[] aEntry : aDocumentAttributes)
                {
                    Log.Dbg.printf("    %s -> %s\n", aEntry[0], aEntry[1]);
                }
                throw new RuntimeException();
            }
        }
        else
        {
            final Set<AttributeDescriptor> aUsedAttributes = new HashSet<>();

            // Process all attributes from the document.
            for (final String[] aEntry : aDocumentAttributes)
            {
                final String sRawValue = aEntry[2];
                final AttributeDescriptor aAttributeDescriptor = ProcessAttribute(
                    aEntry[0],
                    aEntry[1],
                    sRawValue,
                    aAttributesPerState);
                aUsedAttributes.add(aAttributeDescriptor);
                final Object aProcessedValue = maSimpleTypeManager.PreprocessValue(
                    sRawValue,
                    aAttributeDescriptor);
                if (aProcessedValue == null)
                {
                    maSimpleTypeManager.PreprocessValue(
                        sRawValue,
                        aAttributeDescriptor);
                    throw new RuntimeException(
                        String.format("value '%s' of attribute '%s' is not recognized",
                            sRawValue,
                            aAttributeDescriptor.GetName()));
                }
                aValues.AddAttribute(
                    aAttributeDescriptor,
                    sRawValue,
                    aProcessedValue);

                if (Log.Dbg != null)
                {
                    if (aAttributeDescriptor == null)
                        Log.Dbg.printf("attribute %s%s is not known\n",
                            aEntry[0]==null ? "" : ":"+aEntry[0],
                            aEntry[1]);
                    else
                        Log.Dbg.printf("attribute %s:%s(%d:%d) has type %s(%d) and value %s('%s')\n",
                            maNamespaceMap.GetDescriptorForId(aAttributeDescriptor.GetNamespaceId()).Prefix,
                            maNameMap.GetNameForId(aAttributeDescriptor.GetNameId()),
                            aAttributeDescriptor.GetNamespaceId(),
                            aAttributeDescriptor.GetNameId(),
                            maStateNameMap.GetNameForId(aAttributeDescriptor.GetTypeId()),
                            aAttributeDescriptor.GetTypeId(),
                            aProcessedValue,
                            sRawValue);
                }
            }

            // Check if all required attributes where given.
            for (final AttributeDescriptor aAttribute : aAttributesPerState.values())
            {
                if ( ! aUsedAttributes.contains(aAttribute))
                {
                    if ( ! aAttribute.IsOptional())
                        throw new RuntimeException("attribute '"+aAttribute.GetName()+"' is not present but also not optional");
                    else
                    {
                        // Add an entry that gives access to the default value.
                        aValues.AddAttribute(aAttribute, null, null);
                    }
                }
            }
        }

        return aValues;
    }




    private AttributeDescriptor ProcessAttribute (
        final String sNamespace,
        final String sAttributeName,
        final String sAttributeValue,
        final Map<Integer,AttributeDescriptor> aAttributesPerState)
    {
        final AttributeDescriptor aAttributeDescriptor;
        if (sNamespace == null)
        {
            // Attribute name has no namespace.
            final int nAttributeNameId = maNameMap.GetIdForName(sAttributeName);
            aAttributeDescriptor = aAttributesPerState.get(nAttributeNameId);
        }
        else
        {
            // Attribute name has explicit namespace.
            final NamespaceMap.NamespaceDescriptor aDescriptor = maNamespaceMap.GetDescriptorForURI(sNamespace);
            final int nAttributeNameId = maNameMap.GetIdForName(sAttributeName);
            aAttributeDescriptor = aAttributesPerState.get((aDescriptor.Id<<16) | nAttributeNameId);
        }
        return aAttributeDescriptor;
    }




    /** Remove the quotes around the given string.
     *  If it has the special value null (without quotes) then the null reference
     *  is returned.
     */
    private String UnquoteString (final String sValue)
    {
        if (sValue.equals("null"))
            return null;
        else
        {
            assert(sValue.startsWith("\""));
            assert(sValue.endsWith("\""));
            return sValue.substring(1, sValue.length()-1);
        }
    }




    public int GetAttributeCount ()
    {
        int nCount = 0;
        for (final Map<Integer,AttributeDescriptor> aMap : maStateIdToAttributesMap.values())
            nCount += aMap.size();
        return nCount;
    }




    private final Map<Integer,Map<Integer,AttributeDescriptor>> maStateIdToAttributesMap;
    private final NamespaceMap maNamespaceMap;
    private final NameMap maNameMap;
    private final NameMap maStateNameMap;
    private final SimpleTypeManager maSimpleTypeManager;
}
