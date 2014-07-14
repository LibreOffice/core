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

import org.apache.openoffice.ooxml.parser.attribute.AttributeValues;

/** Context that has the same life time (by default) as the element it represents.
 *  Gives access to the attribute values and the parent context.
 */
public class ElementContext
{
    ElementContext (
        final String sElementName,
        final String sTypeName,
        final boolean bIsSkipping,
        final AttributeValues aValues,
        final ElementContext aParentContext)
    {
        msElementName = sElementName;
        msTypeName = sTypeName;
        mbIsSkipping = bIsSkipping;
        maAttributeValues = aValues;
        maParentContext = aParentContext;
    }




    public String GetElementName ()
    {
        return msElementName;
    }




    public String GetTypeName ()
    {
        return msTypeName;
    }




    public AttributeValues GetAttributes ()
    {
        return maAttributeValues;
    }




    /** Return the context of the parent element.
     *  Can be null when there is no parent element.
     */
    public ElementContext GetParentContext ()
    {
        return maParentContext;
    }




    private final String msElementName;
    private final String msTypeName;
    private final boolean mbIsSkipping;
    private final AttributeValues maAttributeValues;
    private final ElementContext maParentContext;
}
