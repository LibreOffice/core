/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "type_analyzer.hxx"
#include <codemaker/unotype.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <unoidl/unoidl.hxx>
#include <vector>

TypeAnalyzer::TypeAnalyzer(const rtl::Reference<TypeManager>& typeManager)
    : m_typeManager(typeManager)
{
}

TypeAnalyzer::TypeInfo
TypeAnalyzer::analyzeInterface(const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    TypeInfo typeInfo;

    // Always need XInterface for base type operations
    typeInfo.needsUnoInterface = true;
    typeInfo.namespaces.insert("com::sun::star::uno"_ostr);

    // Analyze all methods in the interface
    for (const auto& method : entity->getDirectMethods())
    {
        // Analyze return type
        if (method.returnType != u"void")
        {
            analyzeType(typeInfo, method.returnType);
        }

        // Analyze parameter types
        for (const auto& param : method.parameters)
        {
            analyzeType(typeInfo, param.type);
        }
    }

    return typeInfo;
}

void TypeAnalyzer::analyzeType(TypeInfo& typeInfo, std::u16string_view unoType)
{
    // Use cppumaker's approach: recursively insert dependencies
    insertDependency(typeInfo, unoType);
}

void TypeAnalyzer::insertDependency(TypeInfo& typeInfo, std::u16string_view unoType)
{
    // Decompose sequence/template types like cppumaker does
    sal_Int32 sequenceDepth;
    std::vector<OString> templateArgs;
    OString baseType = codemaker::UnoType::decompose(u2b(unoType), &sequenceDepth, &templateArgs);
    OUString baseTypeU = b2u(baseType);

    if (sequenceDepth != 0)
    {
        typeInfo.needsSequence = true;
    }

    // Get the type sort to determine how to handle it
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(baseTypeU, &entity, &cursor);

    switch (sort)
    {
        case codemaker::UnoType::Sort::Void:
            // No special handling needed
            break;
        case codemaker::UnoType::Sort::Boolean:
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
        case codemaker::UnoType::Sort::Float:
        case codemaker::UnoType::Sort::Double:
        case codemaker::UnoType::Sort::Char:
            typeInfo.needsSalTypes = true;
            break;
        case codemaker::UnoType::Sort::String:
            typeInfo.needsString = true;
            typeInfo.needsRtlUstring = true;
            break;
        case codemaker::UnoType::Sort::Type:
            typeInfo.needsUnoInterface = true; // Type needs XInterface includes
            break;
        case codemaker::UnoType::Sort::Any:
            typeInfo.needsAny = true;
            break;
        case codemaker::UnoType::Sort::PlainStruct:
        {
            // Add this struct to dependencies and recursively analyze its members
            typeInfo.structTypes.insert(baseType);

            rtl::Reference<unoidl::PlainStructTypeEntity> structEntity(
                static_cast<unoidl::PlainStructTypeEntity*>(entity.get()));
            if (structEntity.is())
            {
                // Recursively analyze base struct
                if (!structEntity->getDirectBase().isEmpty())
                {
                    insertDependency(typeInfo, structEntity->getDirectBase());
                }

                // Recursively analyze member types
                for (const auto& member : structEntity->getDirectMembers())
                {
                    insertDependency(typeInfo, member.type);
                }
            }
            break;
        }
        case codemaker::UnoType::Sort::Enum:
            typeInfo.enumTypes.insert(baseType);
            typeInfo.needsEnum = true;
            typeInfo.needsSalTypes = true;
            break;
        case codemaker::UnoType::Sort::Interface:
            typeInfo.interfaceTypes.insert(baseType);
            typeInfo.needsUnoInterface = true;
            typeInfo.namespaces.insert("com::sun::star::uno"_ostr);
            break;
        case codemaker::UnoType::Sort::ConstantGroup:
            typeInfo.constantGroupTypes.insert(baseType);
            typeInfo.needsConstantGroup = true;
            break;
        case codemaker::UnoType::Sort::PolymorphicStructTemplate:
        {
            // Handle template arguments recursively
            for (const OString& arg : templateArgs)
            {
                insertDependency(typeInfo, b2u(arg));
            }
            // Note: Don't add polymorphic struct templates to structTypes
            // since they don't have concrete header files to include
            break;
        }
        default:
            // For unknown types, ensure basic includes
            typeInfo.needsUnoInterface = true;
            break;
    }

    // Extract namespace from UNO type name (for any valid UNO type)
    if (sort != codemaker::UnoType::Sort::Void && sort != codemaker::UnoType::Sort::Boolean
        && sort != codemaker::UnoType::Sort::Byte && sort != codemaker::UnoType::Sort::Short
        && sort != codemaker::UnoType::Sort::UnsignedShort && sort != codemaker::UnoType::Sort::Long
        && sort != codemaker::UnoType::Sort::UnsignedLong && sort != codemaker::UnoType::Sort::Hyper
        && sort != codemaker::UnoType::Sort::UnsignedHyper
        && sort != codemaker::UnoType::Sort::Float && sort != codemaker::UnoType::Sort::Double
        && sort != codemaker::UnoType::Sort::Char && sort != codemaker::UnoType::Sort::String
        && sort != codemaker::UnoType::Sort::Type && sort != codemaker::UnoType::Sort::Any
        && baseType.indexOf('.') != sal_Int32(std::string::npos))
    {
        typeInfo.namespaces.insert(extractNamespace(baseTypeU));
    }
}

OString TypeAnalyzer::extractNamespace(std::u16string_view unoType)
{
    OString typeStr = u2b(unoType);

    // Convert dots to double colons for C++ namespace
    OString cppNamespace = typeStr.replaceAll("."_ostr, "::"_ostr);

    // Remove the last component (the actual type name) to get just the namespace
    sal_Int32 lastColon = cppNamespace.lastIndexOf("::");
    if (lastColon != -1)
    {
        return cppNamespace.copy(0, lastColon);
    }

    return cppNamespace;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
