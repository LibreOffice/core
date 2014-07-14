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

import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.parser.XsdNamespace;

public enum BuiltInType
{
    AnyURI,
    Base64Binary,
    Boolean,
    Byte,
    Float,
    DateTime,
    Double,
    HexBinary,
    ID,
    Int,
    Integer,
    Long,
    NcName,
    Short,
    String,
    Token,
    UnsignedByte,
    UnsignedInt,
    UnsignedLong,
    UnsignedShort;

    public static BuiltInType GetForName (final QualifiedName aName)
    {
        assert(aName.GetNamespacePrefix().equals(XsdNamespace.Prefix));
        switch(aName.GetLocalPart())
        {
            case "anyURI": return AnyURI;
            case "base64Binary": return Base64Binary;
            case "boolean": return Boolean;
            case "byte": return Byte;
            case "dateTime": return DateTime;
            case "double": return Double;
            case "float": return Float;
            case "hexBinary": return HexBinary;
            case "ID": return ID;
            case "int" : return Int;
            case "integer": return Integer;
            case "long": return Long;
            case "NCName": return NcName;
            case "short": return Short;
            case "string": return String;
            case "token": return Token;
            case "unsignedByte": return UnsignedByte;
            case "unsignedInt": return UnsignedInt;
            case "unsignedLong": return UnsignedLong;
            case "unsignedShort": return UnsignedShort;
            default: throw new RuntimeException("there is no builtin type named "+aName.GetDisplayName());
        }
    }

    public QualifiedName GetQualifiedName ()
    {
        final String sTypeName;
        switch(this)
        {
            case AnyURI: sTypeName = "anyURI"; break;
            case Base64Binary: sTypeName = "base64Binary"; break;
            case Boolean: sTypeName = "boolean"; break;
            case Byte: sTypeName = "byte"; break;
            case Double: sTypeName = "double"; break;
            case DateTime: sTypeName = "dateTime"; break;
            case Float: sTypeName = "float"; break;
            case HexBinary: sTypeName = "hexBinary"; break;
            case ID: sTypeName = "ID"; break;
            case Int: sTypeName = "int"; break;
            case Integer: sTypeName = "integer"; break;
            case Long: sTypeName = "long"; break;
            case NcName: sTypeName = "NCName"; break;
            case Short: sTypeName = "short"; break;
            case String: sTypeName = "string"; break;
            case Token: sTypeName = "token"; break;
            case UnsignedByte: sTypeName = "unsignedByte"; break;
            case UnsignedInt: sTypeName = "unsignedInt"; break;
            case UnsignedLong: sTypeName = "unsignedLong"; break;
            case UnsignedShort: sTypeName = "unsignedShort"; break;
            default:
                throw new RuntimeException();
        }
        return new QualifiedName(XsdNamespace.URI, XsdNamespace.Prefix, sTypeName);
    }
}
