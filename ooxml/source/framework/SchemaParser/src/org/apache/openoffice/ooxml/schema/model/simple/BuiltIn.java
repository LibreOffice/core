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

package org.apache.openoffice.ooxml.schema.model.simple;

import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Representation of built in types.
 */
public class BuiltIn
    extends SimpleType
{
    public enum Type
    {
        String,
        Token,
        Byte,
        UnsignedByte,
        Short,
        UnsignedShort,
        Int,
        UnsignedInt,
        Integer,
        Long,
        Float,
        Double,
        Boolean,
        Base64Binary,
        DateTime,
        AnyURI
    }
    public static final String XsdNamespace = "http://www.w3.org/2001/XMLSchema";




    public static Vector<BuiltIn> GetTypes ()
    {
        final Vector<BuiltIn> aTypes = new Vector<>();
        for (final Type eType : Type.values())
            aTypes.add(new BuiltIn(eType));
        return aTypes;
    }




    protected BuiltIn (
        final Type eType)
    {
        super(null, GetNameForType(eType), null);
        meType = eType;
    }




    @Override
    public NodeType GetNodeType ()
    {
        return NodeType.BuiltIn;
    }




    public Type GetBuiltInType ()
    {
        return meType;
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    private static QualifiedName GetNameForType (final Type eType)
    {
        final String sTypeName;
        switch (eType)
        {
            case String: sTypeName = "string"; break;
            case Token: sTypeName = "token"; break;
            case Byte: sTypeName = "byte"; break;
            case UnsignedByte: sTypeName = "unsignedByte"; break;
            case Short: sTypeName = "short"; break;
            case UnsignedShort: sTypeName = "unsignedShort"; break;
            case Int: sTypeName = "int"; break;
            case UnsignedInt: sTypeName = "unsignedInt"; break;
            case Integer: sTypeName = "integer"; break;
            case Long: sTypeName = "long"; break;
            case Float: sTypeName = "float"; break;
            case Double: sTypeName = "double"; break;
            case Boolean: sTypeName = "boolean"; break;
            case Base64Binary: sTypeName = "base64Binary"; break;
            case DateTime: sTypeName = "dateTime"; break;
            case AnyURI: sTypeName = "anyURI"; break;
            default:
                throw new RuntimeException();
        }
        return new QualifiedName(XsdNamespace, "xsd", sTypeName);
    }




    @Override
    public String toString ()
    {
        return "builtin "+GetName().GetDisplayName();
    }




    private final Type meType;
}
