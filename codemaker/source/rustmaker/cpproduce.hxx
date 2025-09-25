/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <string_view>
#include <unoidl/unoidl.hxx>
#include <codemaker/typemanager.hxx>
#include <rtl/string.hxx>

// Forward declarations
class CppFile;

/**
 * CppProducer generates C++-side opaque pointer implementations.
 *
 * This is the C-side generator in the opaque pointer approach.
 * It creates C++ wrapper classes and extern "C" bridge functions
 * that the Rust-side can call through opaque pointers.
 *
 * Architecture:
 * - C++ wrapper classes that hide UNO complexity
 * - extern "C" bridge functions with unique names
 * - Simple void* pointer casting for opaque compatibility
 * - All UNO complexity isolated in C++ side
 */
class CppProducer
{
public:
    /**
     * Constructor
     * @param outputDir Directory for generated C++ files
     * @param verbose Enable verbose output
     * @param dryRun Don't actually write files
     * @param typeManager UNO type manager for accessing type information
     */
    CppProducer(const OString& outputDir, bool verbose, bool dryRun,
                const rtl::Reference<TypeManager>& typeManager);

    /**
     * Initialize the single combined output file
     */
    void initializeCombinedFile();

    /**
     * Finalize and close the single combined output file
     */
    void finalizeCombinedFile();

    /**
     * Generate C++ opaque enum bridge
     * Creates conversion functions between enum and integers
     */
    void produceEnum(std::string_view name, const rtl::Reference<unoidl::EnumTypeEntity>& entity);

    /**
     * Generate C++ opaque struct bridge
     * Creates wrapper class and field accessor functions
     */
    void produceStruct(std::string_view name,
                       const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);

    /**
     * Generate C++ opaque interface bridge
     * Creates wrapper class and method bridge functions
     */
    void produceInterface(std::string_view name,
                          const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);

    /**
     * Generate C++ opaque service bridge
     * Creates service creation bridge functions
     */
    void produceService(std::string_view name,
                        const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity);

private:
    // Configuration
    OString m_outputDir;
    bool m_verbose;
    bool m_dryRun;
    rtl::Reference<TypeManager> m_typeManager;

    // Combined output files
    std::unique_ptr<CppFile> m_combinedSourceFile; // .cxx file
    std::unique_ptr<CppFile> m_combinedHeaderFile; // .hxx file

    // ========================================================================
    //  CORE TYPE GENERATION METHODS
    // ========================================================================
    void generateStructWrapper(std::string_view name,
                               const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);
    void generateEnumBridge(std::string_view name,
                            const rtl::Reference<unoidl::EnumTypeEntity>& entity);
    void generateStructBridge(std::string_view name,
                              const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);
    void generateCBridge(std::string_view name,
                         const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);

    void generateStructHeader(std::string_view name,
                              const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);
    void generateStructSource(std::string_view name,
                              const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);

    void generateInterfaceHeader(std::string_view name,
                                 const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);
    void generateInterfaceSource(std::string_view name,
                                 const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);

    void generateEnumHeader(std::string_view name,
                            const rtl::Reference<unoidl::EnumTypeEntity>& entity);
    void generateEnumSource(std::string_view name,
                            const rtl::Reference<unoidl::EnumTypeEntity>& entity);

    void
    generateServiceHeader(std::string_view name,
                          const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity);
    void
    generateServiceSource(std::string_view name,
                          const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity);

    // ========================================================================
    //  FILE GENERATION UTILITIES
    // ========================================================================
    // Unified include and namespace generation (eliminates duplication)
    static void generateCommonIncludes(CppFile& file, std::string_view name,
                                       bool needsLogging = false, bool needsUnoTypes = false);
    static void generateSourceNamespaces(CppFile& file, std::string_view name);

    // Legacy functions (maintained for existing calls)
    static void generateStructSourceIncludes(CppFile& file, std::string_view name);
    static void generateStructSourceNamespaces(CppFile& file, std::string_view name);
    static void generateEnumIncludes(CppFile& file, std::string_view name);
    static void generateEnumSourceIncludes(CppFile& file, std::string_view name);
    static void generateInterfaceSourceIncludes(CppFile& file, std::string_view name);
    static void generateInterfaceSourceNamespaces(CppFile& file, std::string_view name);

    // ========================================================================
    //  TYPE SYSTEM & CONVERSION UTILITIES
    // ========================================================================
    // Type conversion and mapping
    OString convertUnoTypeToCpp(std::u16string_view unoType) const;
    std::string convertTemplateArguments(const std::string& unoType) const;
    OString getCppTypeName(std::u16string_view unoType) const;
    OString getTypedParameterType(
        std::u16string_view unoType,
        unoidl::InterfaceTypeEntity::Method::Parameter::Direction direction) const;
    OString getStructGetterReturnType(std::u16string_view unoType) const;
    OString getStructSetterParameterType(std::u16string_view unoType) const;
    OString getRustFFITypeName(std::u16string_view unoType) const;
    static OString mapUnoPrimitiveToSal(std::u16string_view unoType);
    OUString resolveTypedef(std::u16string_view unoType) const;
    static OString convertBasicType(const OString& typeName);
    static OString convertUnoTypeToHandle(std::u16string_view unoType);

    // Type classification
    bool isUnoType(std::u16string_view typeName) const;
    bool isUnoStruct(std::u16string_view typeName) const;
    bool isUnoEnum(std::u16string_view typeName) const;
    bool isUnoInterface(std::u16string_view typeName) const;
    bool isUnoPolymorphicStruct(std::u16string_view typeName) const;
    bool isUnoConstantGroup(std::u16string_view typeName) const;

    // Method return type handling
    OString getMethodReturnType(std::u16string_view returnType) const;
    OString getMethodDefaultReturn(std::u16string_view returnType) const;

    // ========================================================================
    //  CODE GENERATION HELPERS
    // ========================================================================
    // C Bridge generation helpers
    void generateCBridgeIncludes(CppFile& file, std::string_view name,
                                 const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);
    static void generateCBridgeTypeDefinitions(CppFile& file, std::string_view handleTypeName);
    static void generateCBridgeConstructors(CppFile& file, std::string_view className,
                                            std::string_view functionPrefix,
                                            std::string_view handleTypeName);
    static void generateCBridgeDestructor(CppFile& file, std::string_view className,
                                          std::string_view functionPrefix,
                                          std::string_view handleTypeName);
    static void generateCBridgeValidation(CppFile& file, std::string_view className,
                                          std::string_view functionPrefix,
                                          std::string_view handleTypeName);
    void generateCBridgeMethods(CppFile& file, std::string_view className,
                                std::string_view functionPrefix, std::string_view handleTypeName,
                                const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);

    // Interface generation helpers
    static void generateInterfaceSourceBasicFunctions(CppFile& file,
                                                      std::string_view functionPrefix,
                                                      std::string_view className,
                                                      std::string_view handleTypeName);
    void generateInterfaceSourceMethodImplementations(
        CppFile& file, std::string_view functionPrefix, std::string_view className,
        std::string_view handleTypeName, const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);
    void generateSingleInterfaceMethod(CppFile& file, std::string_view functionPrefix,
                                       std::string_view className, std::string_view handleTypeName,
                                       const unoidl::InterfaceTypeEntity::Method& method);
    void generateActualMethodCall(CppFile& file, const unoidl::InterfaceTypeEntity::Method& method);
    void generateReturnValueConversion(CppFile& file,
                                       const unoidl::InterfaceTypeEntity::Method& method);

    // Struct generation helpers
    static void generateStructSourceBasicFunctions(CppFile& file, std::string_view functionPrefix,
                                                   std::string_view handleTypeName,
                                                   std::string_view className);
    void
    generateStructSourceBaseMembers(CppFile& file, std::string_view functionPrefix,
                                    std::string_view handleTypeName, std::string_view className,
                                    const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);
    void
    generateStructSourceDirectMembers(CppFile& file, std::string_view functionPrefix,
                                      std::string_view handleTypeName, std::string_view className,
                                      const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);
    void generateStructMemberAccessors(CppFile& file, std::string_view functionPrefix,
                                       std::string_view handleTypeName, std::string_view className,
                                       const unoidl::PlainStructTypeEntity::Member& member);

    // ========================================================================
    //  UTILITY FUNCTIONS
    // ========================================================================
    static std::string_view splitName(std::string_view name);
    static OString getInterfaceClassName(std::string_view unoName);
    static OString getCFunctionPrefix(std::string_view unoName);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
