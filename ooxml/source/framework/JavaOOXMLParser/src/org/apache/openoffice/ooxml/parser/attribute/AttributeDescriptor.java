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

/** Store information about a single attribute (per state) that was read
 *  from the parse table.
 *
 *  Note that an attribute that is defined for more than one state has one
 *  AttributeDescriptor object per state.
 *
 */
public class AttributeDescriptor
{
    public AttributeDescriptor (
        final int nPrefixId,
        final int nAttributeId,
        final boolean bCanBeUnqualified,
        final boolean bIsOptional,
        final String sDefaultValue,
        final String sAttributeName,
        final int nAttributeTypeId)
    {
        mnNamespaceId = nPrefixId;
        mnAttributeId = nAttributeId;
        mbCanBeUnqualified = bCanBeUnqualified;
        mbIsOptional = bIsOptional;
        msDefaultValue = sDefaultValue;
        msAttributeName = sAttributeName;
        mnAttributeTypeId = nAttributeTypeId;
    }




    public int GetTypeId()
    {
        return mnAttributeTypeId;
    }




    public int GetNamespaceId ()
    {
        return mnNamespaceId;
    }




    public int GetNameId ()
    {
        return mnAttributeId;
    }




    public boolean CanBeUnqualified ()
    {
        return mbCanBeUnqualified;
    }




    public boolean IsOptional ()
    {
        return mbIsOptional;
    }




    public String GetDefaultValue ()
    {
        return msDefaultValue;
    }




    public String GetName ()
    {
        return msAttributeName;
    }




    @Override
    public String toString ()
    {
        return String.format(
            "attribute %s(%d) of type %d",
            msAttributeName,
            mnAttributeId,
            mnAttributeTypeId);
    }




    private final int mnNamespaceId;
    private final int mnAttributeId;
    private final boolean mbCanBeUnqualified;
    private final boolean mbIsOptional;
    private final String msDefaultValue;
    private final String msAttributeName;
    private final int mnAttributeTypeId;
}
