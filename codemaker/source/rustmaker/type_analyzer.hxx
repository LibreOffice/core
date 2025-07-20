/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <set>
#include <string>
#include <unoidl/unoidl.hxx>
#include <codemaker/typemanager.hxx>
#include <rtl/string.hxx>

/**
 * Analyzes UNO interface entities to identify type dependencies for dynamic include generation.
 */
class TypeAnalyzer
{
public:
    TypeAnalyzer(const rtl::Reference<TypeManager>& typeManager);

private:
    rtl::Reference<TypeManager> m_typeManager;

public:
    /**
     * Container for type analysis results.
     */
    struct TypeInfo
    {
        std::set<OString> interfaceTypes; // UNO interface types (e.g., "com.sun.star.text.XText")
        std::set<OString> structTypes; // UNO struct types (e.g., "com.sun.star.beans.Property")
        std::set<OString> enumTypes; // UNO enum types (e.g., "com.sun.star.uno.TypeClass")
        std::set<OString>
            constantGroupTypes; // UNO constant groups (e.g., "com.sun.star.awt.FontWeight")
        std::set<OString> namespaces; // Required namespaces (e.g., "com::sun::star::text")

        // Core UNO types - implemented first: Any
        bool needsAny = false;

        // Core UNO types - TODO: implement in future
        bool needsString = false; // TODO: implement string type handling
        bool needsSequence = false; // TODO: implement sequence type handling
        bool needsPropertyValue = false; // TODO: implement PropertyValue handling

        // Basic types
        bool needsSalTypes = false; // For sal_Int32, sal_Bool, etc.
        bool needsRtlUstring = false; // For rtl_uString*
        bool needsUnoInterface = false; // For XInterface base type
        bool needsEnum = false; // For enum type handling
        bool needsConstantGroup = false; // For constant group handling
    };

    /**
     * Analyzes the given interface entity and returns type dependency information.
     *
     * @param entity The interface entity to analyze.
     * @return TypeInfo containing all identified dependencies.
     */
    TypeInfo analyzeInterface(const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);

    /**
     * Analyzes a single UNO type string and updates the TypeInfo accordingly.
     *
     * @param typeInfo The TypeInfo to update.
     * @param unoType The UNO type string to analyze (e.g., "com.sun.star.uno.Any").
     */
    void analyzeType(TypeInfo& typeInfo, std::u16string_view unoType);

private:
    /**
     * Recursively inserts type dependencies following cppumaker's pattern.
     * This method handles decomposition of sequences and templates, and recursively
     * analyzes struct members and base types.
     *
     * @param typeInfo The TypeInfo to update with dependencies.
     * @param unoType The UNO type to analyze recursively.
     */
    void insertDependency(TypeInfo& typeInfo, std::u16string_view unoType);

    /**
     * Extracts namespace from a UNO type name.
     *
     * @param unoType The UNO type name (e.g., "com.sun.star.text.XText").
     * @return The corresponding C++ namespace (e.g., "com::sun::star::text").
     */
    static OString extractNamespace(std::u16string_view unoType);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
