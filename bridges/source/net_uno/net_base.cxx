/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "net_base.hxx"

#include <string_view>
#include <unordered_map>

#include <o3tl/string_view.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

namespace net_uno
{
namespace
{
const std::unordered_map<std::u16string_view, std::u16string_view> s_typeNames{
    { u"System.Void", u"void" },
    { u"System.Boolean", u"boolean" },
    { u"System.Char", u"char" },
    { u"System.SByte", u"byte" },
    { u"System.Int16", u"short" },
    { u"System.UInt16", u"unsigned short" },
    { u"System.Int32", u"long" },
    { u"System.UInt32", u"unsigned long" },
    { u"System.Int64", u"hyper" },
    { u"System.UInt64", u"unsigned hyper" },
    { u"System.Single", u"float" },
    { u"System.Double", u"double" },
    { u"System.String", u"string" },
    { u"System.Type", u"type" },
    { u"com.sun.star.uno.Any", u"any" },
    { u"com.sun.star.uno.UnoException", u"com.sun.star.uno.Exception" },
    { u"com.sun.star.uno.IQueryInterface", u"com.sun.star.uno.XInterface" },
};

const std::unordered_map<OUString, typelib_TypeClass> s_typeClasses{
    { u"System.Void"_ustr, typelib_TypeClass_VOID },
    { u"System.Boolean"_ustr, typelib_TypeClass_BOOLEAN },
    { u"System.Char"_ustr, typelib_TypeClass_CHAR },
    { u"System.SByte"_ustr, typelib_TypeClass_BYTE },
    { u"System.Int16"_ustr, typelib_TypeClass_SHORT },
    { u"System.UInt16"_ustr, typelib_TypeClass_UNSIGNED_SHORT },
    { u"System.Int32"_ustr, typelib_TypeClass_LONG },
    { u"System.UInt32"_ustr, typelib_TypeClass_UNSIGNED_LONG },
    { u"System.Int64"_ustr, typelib_TypeClass_HYPER },
    { u"System.UInt64"_ustr, typelib_TypeClass_UNSIGNED_HYPER },
    { u"System.Single"_ustr, typelib_TypeClass_FLOAT },
    { u"System.Double"_ustr, typelib_TypeClass_DOUBLE },
    { u"System.String"_ustr, typelib_TypeClass_STRING },
    { u"System.Type"_ustr, typelib_TypeClass_TYPE },
    { u"com.sun.star.uno.Any"_ustr, typelib_TypeClass_ANY },
};

void map_uno_type_to_net(typelib_TypeDescriptionReference* pTDRef, OUStringBuffer& buffer)
{
    switch (pTDRef->eTypeClass)
    {
        case typelib_TypeClass_VOID:
            buffer.append(u"System.Void");
            break;
        case typelib_TypeClass_CHAR:
            buffer.append(u"System.Char");
            break;
        case typelib_TypeClass_BOOLEAN:
            buffer.append(u"System.Boolean");
            break;
        case typelib_TypeClass_BYTE:
            buffer.append(u"System.SByte");
            break;
        case typelib_TypeClass_SHORT:
            buffer.append(u"System.Int16");
            break;
        case typelib_TypeClass_UNSIGNED_SHORT:
            buffer.append(u"System.UInt16");
            break;
        case typelib_TypeClass_LONG:
            buffer.append(u"System.Int32");
            break;
        case typelib_TypeClass_UNSIGNED_LONG:
            buffer.append(u"System.UInt32");
            break;
        case typelib_TypeClass_HYPER:
            buffer.append(u"System.Int64");
            break;
        case typelib_TypeClass_UNSIGNED_HYPER:
            buffer.append(u"System.UInt64");
            break;
        case typelib_TypeClass_FLOAT:
            buffer.append(u"System.Single");
            break;
        case typelib_TypeClass_DOUBLE:
            buffer.append(u"System.Double");
            break;
        case typelib_TypeClass_STRING:
            buffer.append(u"System.String");
            break;
        case typelib_TypeClass_TYPE:
            buffer.append(u"System.Type");
            break;
        case typelib_TypeClass_ANY:
            buffer.append(u"com.sun.star.uno.Any");
            break;

        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_EXCEPTION:
            // These have the same name on both sides
            buffer.append(OUString::unacquired(&pTDRef->pTypeName));
            break;

        case typelib_TypeClass_STRUCT:
            // These have the same name on both sides
            // TODO: What about polymorphic structs? Merge this with above cases if fine
            buffer.append(OUString::unacquired(&pTDRef->pTypeName));
            break;

        case typelib_TypeClass_INTERFACE:
        {
            // These have the same name on both sides
            if (u"com.sun.star.uno.XInterface"_ustr.equals(pTDRef->pTypeName))
                // Except XInterface, which does not exist on the .NET side
                buffer.append(u"com.sun.star.uno.IQueryInterface");
            else
                buffer.append(OUString::unacquired(&pTDRef->pTypeName));
            break;
        }

        case typelib_TypeClass_SEQUENCE:
        {
            TypeDescHolder seqType(pTDRef);
            typelib_TypeDescriptionReference* pElementTDRef
                = reinterpret_cast<typelib_IndirectTypeDescription*>(seqType.get())->pType;
            map_uno_type_to_net(pElementTDRef, buffer);
            buffer.append(u"[]");
            break;
        }

        default:
        {
            throw BridgeRuntimeError(SAL_WHERE, "could not map given type info "
                                                    + OUString::unacquired(&pTDRef->pTypeName));
        }
    }
}

void map_net_type_to_uno(std::u16string_view sTypeName, OUStringBuffer& buffer)
{
    size_t bracketsStart = sTypeName.find_last_not_of(u"[]");
    bool isSequence = bracketsStart != std::u16string_view::npos;
    std::u16string_view sBrackets = isSequence ? sTypeName.substr(bracketsStart + 1) : u"";
    std::u16string_view sFullName = isSequence ? sTypeName.substr(0, bracketsStart + 1) : sTypeName;

    size_t genericsStart = sFullName.find_first_of(u'<');
    size_t genericsEnd = sFullName.find_last_of(u'>');
    bool hasGenerics = genericsStart != std::u16string_view::npos;
    std::u16string_view sGenericParams
        = hasGenerics ? sFullName.substr(genericsStart + 1, genericsEnd - genericsStart - 1) : u"";
    std::u16string_view sBaseName = hasGenerics ? sFullName.substr(0, genericsStart) : sFullName;

    // Sequence brackets go at the beginning of UNO name
    for (size_t i = 0; i < sBrackets.size() / 2; i++)
        buffer.append(u"[]");

    // Builtin types
    auto it = s_typeNames.find(sBaseName);
    if (it != s_typeNames.end())
    {
        buffer.append(it->second);
    }
    else
    {
        buffer.append(sBaseName);
        if (hasGenerics)
        {
            buffer.append(u'<');
            for (size_t i = 0; i != std::string_view::npos;)
            {
                std::u16string_view genericParam(o3tl::getToken(sGenericParams, u',', i));
                map_net_type_to_uno(genericParam, buffer);
                if (i != std::string_view::npos)
                    buffer.append(u',');
            }
            buffer.append(u'>');
        }
    }
}
}

OUString map_uno_type_to_net(typelib_TypeDescriptionReference* pTDRef)
{
    OUStringBuffer buffer;
    map_uno_type_to_net(pTDRef, buffer);
    return buffer.makeStringAndClear();
}

typelib_TypeDescriptionReference* map_net_type_to_uno(const OUString& sTypeName)
{
    typelib_TypeDescriptionReference* retVal = nullptr;

    // Simple types
    auto it = s_typeClasses.find(sTypeName);
    if (it != s_typeClasses.end())
    {
        retVal = *typelib_static_type_getByTypeClass(it->second);
        typelib_typedescriptionreference_acquire(retVal);
        return retVal;
    }

    // Complex types (structs, interfaces, sequences)
    OUStringBuffer buffer;
    map_net_type_to_uno(sTypeName, buffer);
    OUString convertedName = buffer.makeStringAndClear();

    typelib_TypeDescription* pTD = nullptr;
    typelib_typedescription_getByName(&pTD, convertedName.pData);
    if (pTD)
    {
        retVal = pTD->pWeakRef;
        typelib_typedescriptionreference_acquire(retVal);
        typelib_typedescription_release(pTD);
        return retVal;
    }

    throw BridgeRuntimeError(SAL_WHERE, "could not map given type name " + sTypeName);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
