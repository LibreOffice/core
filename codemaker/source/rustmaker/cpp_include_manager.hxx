/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "type_analyzer.hxx"
#include <rtl/string.hxx>

// Forward declaration
class CppFile;

/**
 * Manages dynamic generation of C++ includes and namespace declarations
 * based on type analysis results.
 */
class CppIncludeManager
{
public:
    /**
     * Constructor that takes type analysis results.
     *
     * @param typeInfo The type analysis results from TypeAnalyzer.
     */
    explicit CppIncludeManager(const TypeAnalyzer::TypeInfo& typeInfo);

    /**
     * Writes the necessary #include directives to the C++ file.
     *
     * @param file The CppFile to write to.
     * @param interfaceName The name of the interface being processed (for self-include).
     */
    void dumpIncludes(CppFile& file, std::string_view interfaceName);

    /**
     * Writes the necessary using namespace declarations to the C++ file.
     *
     * @param file The CppFile to write to.
     */
    void dumpNamespaces(CppFile& file);

private:
    const TypeAnalyzer::TypeInfo& m_typeInfo;

    /**
     * Writes standard system includes that are always needed.
     *
     * @param file The CppFile to write to.
     */
    void writeStandardIncludes(CppFile& file);

    /**
     * Writes UNO-specific includes based on type analysis.
     *
     * @param file The CppFile to write to.
     */
    void writeUnoIncludes(CppFile& file);

    /**
     * Writes interface-specific includes.
     *
     * @param file The CppFile to write to.
     * @param interfaceName The name of the interface being processed.
     */
    void writeInterfaceIncludes(CppFile& file, std::string_view interfaceName);

    /**
     * Calculates relative include paths like cppumaker does.
     * This generates the proper relative path from the current file being generated
     * to the target dependency file.
     *
     * @param file The current file being generated (provides context for relative path)
     * @param targetType The UNO type name (e.g., "com.sun.star.beans.Property")
     * @param extension The file extension to use (e.g., ".hxx", ".hpp")
     * @return Properly formatted include statement with quotes
     */
    static OString calculateRelativeIncludePath(const CppFile& file, const OString& targetType,
                                                const std::string& extension);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
