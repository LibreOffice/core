/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cpproduce.hxx"
#include <rtl/string.hxx>
#include "rustfile.hxx"
#include "type_analyzer.hxx"
#include "cpp_include_manager.hxx"
#include <codemaker/typemanager.hxx>
#include <codemaker/unotype.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <iostream>
#include <filesystem>
#include <string_view>
#include <algorithm>

CppProducer::CppProducer(const OString& outputDir, bool verbose, bool dryRun,
                         const rtl::Reference<TypeManager>& typeManager)
    : m_outputDir(outputDir)
    , m_verbose(verbose)
    , m_dryRun(dryRun)
    , m_typeManager(typeManager)
    , m_combinedSourceFile()
    , m_combinedHeaderFile()
{
}

void CppProducer::initializeCombinedFile()
{
    if (m_dryRun)
        return;

    // Create the combined source file (.cxx)
    m_combinedSourceFile = std::make_unique<CppFile>(m_outputDir, "rust_uno_bindings", ".cxx");
    std::filesystem::path sourceFilePath = m_combinedSourceFile->getPath();
    if (std::filesystem::exists(sourceFilePath))
    {
        std::filesystem::remove(sourceFilePath);
    }
    m_combinedSourceFile->openFile();

    // Include the header file
    m_combinedSourceFile->beginLine().append("#include \"rust_uno_bindings.hxx\"").endLine();
    m_combinedSourceFile->beginLine().append("").endLine();

    // Create the combined header file (.hxx)
    m_combinedHeaderFile = std::make_unique<CppFile>(m_outputDir, "rust_uno_bindings", ".hxx");
    std::filesystem::path headerFilePath = m_combinedHeaderFile->getPath();
    if (std::filesystem::exists(headerFilePath))
    {
        std::filesystem::remove(headerFilePath);
    }
    m_combinedHeaderFile->openFile();

    // Add header guard
    m_combinedHeaderFile->beginLine().append("#pragma once").endLine();
    m_combinedHeaderFile->beginLine().append("").endLine();

    // Add only sal/types.h which definitely exists
    m_combinedHeaderFile->beginLine().append("#include <sal/types.h>").endLine();
    m_combinedHeaderFile->beginLine().append("").endLine();
}

void CppProducer::finalizeCombinedFile()
{
    if (m_combinedSourceFile)
    {
        m_combinedSourceFile->closeFile();
        m_combinedSourceFile.reset();
    }

    if (m_combinedHeaderFile)
    {
        m_combinedHeaderFile->closeFile();
        m_combinedHeaderFile.reset();
    }
}

void CppProducer::produceEnum(std::string_view name,
                              const rtl::Reference<unoidl::EnumTypeEntity>& entity)
{
    if (m_verbose)
        std::cout << "[cpp-opaque-enum] " << name << " -> enum bridge functions" << '\n';

    generateEnumBridge(name, entity);
}

void CppProducer::produceStruct(std::string_view name,
                                const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    if (m_verbose)
        std::cout << "[cpp-opaque-struct] " << name
                  << " -> constructor/destructor + getter/setter functions" << '\n';

    generateStructWrapper(name, entity);
    // generateStructBridge(name, entity);
}

void CppProducer::produceInterface(std::string_view name,
                                   const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    if (m_verbose)
        std::cout << "[cpp-opaque-interface] " << name
                  << " -> opaque handle + method wrapper functions" << '\n';

    generateInterfaceHeader(name, entity);
    generateInterfaceSource(name, entity);
}

void CppProducer::produceService(
    std::string_view name, const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity)
{
    if (m_verbose)
        std::cout << "[cpp-opaque-service] " << name << " -> service creation bridge functions"
                  << '\n';

    generateServiceHeader(name, entity);
    generateServiceSource(name, entity);
}

void CppProducer::generateEnumBridge(std::string_view name,
                                     const rtl::Reference<unoidl::EnumTypeEntity>& entity)
{
    generateEnumHeader(name, entity);
    generateEnumSource(name, entity);
}

void CppProducer::generateStructWrapper(std::string_view name,
                                        const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    generateStructHeader(name, entity);
    generateStructSource(name, entity);
}

void CppProducer::generateStructBridge(std::string_view name,
                                       const rtl::Reference<unoidl::PlainStructTypeEntity>&)
{
    CppFile file(m_outputDir, name);

    if (m_dryRun)
        return;

    file.openFile();

    file.beginLine().append("extern \"C\"").endLine().beginBlock();

    // Simple opaque struct bridge functions (like old FFI approach)
    OString className = getInterfaceClassName(name);
    OString functionPrefix = getCFunctionPrefix(name);

    file.beginLine()
        .append("// Opaque struct creation")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT void* ")
        .append(functionPrefix)
        .append("_create()")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("return new ")
        .append(className)
        .append("();")
        .endLine()
        .endBlock();

    file.beginLine().append("").endLine();
    file.endBlock().append(" // extern \"C\"");

    file.closeFile();
}

void CppProducer::generateCBridgeIncludes(CppFile& file, std::string_view name,
                                          const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    // Use existing infrastructure for includes
    TypeAnalyzer analyzer(m_typeManager);
    TypeAnalyzer::TypeInfo typeInfo = analyzer.analyzeInterface(entity);
    CppIncludeManager includeManager(typeInfo);

    includeManager.dumpIncludes(file, name);
}

void CppProducer::generateCBridgeTypeDefinitions(CppFile& file, std::string_view handleTypeName)
{
    // Opaque handle typedef for type safety
    file.beginLine()
        .append("// Opaque handle for type-safe C FFI")
        .endLine()
        .beginLine()
        .append("typedef void* ")
        .append(handleTypeName)
        .append(";")
        .endLine()
        .beginLine()
        .append("")
        .endLine();
}

void CppProducer::generateCBridgeConstructors(CppFile& file, std::string_view className,
                                              std::string_view functionPrefix,
                                              std::string_view handleTypeName)
{
    // Opaque constructor function - creates interface instance from existing Reference
    file.beginLine()
        .append("// Opaque constructor - creates interface wrapper from UNO Reference")
        .endLine()
        .beginLine()
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_create_from_reference(void* xInterface)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("if (!xInterface) return nullptr;")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("// Cast void* back to Reference<")
        .append(className)
        .append(">")
        .endLine()
        .beginLine()
        .append("Reference<")
        .append(className)
        .append(">* pRef = static_cast<Reference<")
        .append(className)
        .append(">*>(xInterface);")
        .endLine()
        .beginLine()
        .append("if (!pRef->is()) return nullptr;")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("// Create wrapper with interface reference")
        .endLine()
        .beginLine()
        .append("return new ")
        .append(className)
        .append("Wrapper(*pRef);")
        .endLine()
        .endBlock()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("// Opaque service creation - attempts to create UNO service")
        .endLine()
        .beginLine()
        .append("// Note: This is a simplified approach - real service creation")
        .endLine()
        .beginLine()
        .append("// would need ComponentContext and service name resolution")
        .endLine()
        .beginLine()
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_create_service(void* xContext, const char* serviceName)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("if (!xContext || !serviceName) return nullptr;")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("try")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("// Cast context")
        .endLine()
        .beginLine()
        .append("Reference<XComponentContext>* pContext =")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("static_cast<Reference<XComponentContext>*>(xContext);")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("// Create service")
        .endLine()
        .beginLine()
        .append("OUString svcName = OUString::createFromAscii(serviceName);")
        .endLine()
        .beginLine()
        .append("Reference<")
        .append(className)
        .append("> xService(")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("pContext->getServiceManager()->createInstanceWithContext(")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("svcName, *pContext), UNO_QUERY);")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("if (!xService.is()) return nullptr;")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("// Return wrapper")
        .endLine()
        .beginLine()
        .append("return new ")
        .append(className)
        .append("Wrapper(xService);")
        .endLine()
        .endBlock()
        .beginLine()
        .append("catch (const Exception& ex)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("// TODO: Log exception details")
        .endLine()
        .beginLine()
        .append("SAL_WARN(\"rustmaker\", \"UNO exception in interface method\");")
        .endLine()
        .beginLine()
        .append("return nullptr;")
        .endLine()
        .endBlock()
        .endBlock();
}

void CppProducer::generateCBridgeDestructor(CppFile& file, std::string_view className,
                                            std::string_view functionPrefix,
                                            std::string_view handleTypeName)
{
    // Opaque destructor function - destroys interface instance
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("// Opaque destructor - destroys interface wrapper")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT void ")
        .append(functionPrefix)
        .append("_destroy(")
        .append(handleTypeName)
        .append(" handle)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("if (handle)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("delete static_cast<")
        .append(className)
        .append("Wrapper*>(handle);")
        .endLine()
        .endBlock();
}

void CppProducer::generateCBridgeValidation(CppFile& file, std::string_view className,
                                            std::string_view functionPrefix,
                                            std::string_view handleTypeName)
{
    // Opaque validity check function
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("// Opaque validity check - tests if interface is valid")
        .endLine()
        .beginLine()
        .append("bool ")
        .append(functionPrefix)
        .append("_is_valid(")
        .append(handleTypeName)
        .append(" handle)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("if (!handle) return false;")
        .endLine()
        .beginLine()
        .append("return static_cast<")
        .append(className)
        .append("Wrapper*>(handle)->isValid();")
        .endLine()
        .endBlock();
}

void CppProducer::generateCBridgeMethods(CppFile& file, std::string_view className,
                                         std::string_view functionPrefix,
                                         std::string_view handleTypeName,
                                         const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    // Opaque method bridge functions - each method gets unique extern "C" function
    for (const auto& method : entity->getDirectMethods())
    {
        file.beginLine()
            .append("")
            .endLine()
            .beginLine()
            .append("// Opaque method bridge for ")
            .append(method.name)
            .endLine()
            .beginLine()
            .append(getMethodReturnType(method.returnType))
            .append(" ")
            .append(functionPrefix)
            .append("_")
            .append(method.name)
            .append("(")
            .append(handleTypeName)
            .append(" handle");

        // Use proper types instead of void* for parameters
        for (const auto& param : method.parameters)
        {
            file.append(", ").append(getCppTypeName(param.type)).append(" ").append(param.name);
        }

        file.append(")")
            .endLine()
            .beginBlock()
            .beginLine()
            .append("if (!handle) return ")
            .append(getMethodDefaultReturn(method.returnType))
            .append(";")
            .endLine()
            .beginLine()
            .append("")
            .endLine()
            .beginLine()
            .append("auto* wrapper = static_cast<")
            .append(className)
            .append("Wrapper*>(handle);")
            .endLine()
            .beginLine()
            .append("return wrapper->")
            .append(method.name)
            .append("(");

        // Pass through all opaque parameters
        bool first = true;
        for (const auto& param : method.parameters)
        {
            if (!first)
                file.append(", ");
            first = false;
            file.append(param.name);
        }

        file.append(");").endLine().endBlock();
    }
}

void CppProducer::generateCBridge(std::string_view name,
                                  const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    CppFile file(m_outputDir, name);

    if (m_dryRun)
        return;

    file.openFile();

    // Generate file setup and includes
    generateCBridgeIncludes(file, name, entity);

    // Generate type definitions
    OString className = getInterfaceClassName(name);
    OString functionPrefix = getCFunctionPrefix(name);
    OString handleTypeName = functionPrefix + "Handle";

    generateCBridgeTypeDefinitions(file, handleTypeName);

    file.beginLine().append("extern \"C\"").endLine().beginBlock();

    // Generate constructor functions
    generateCBridgeConstructors(file, className, functionPrefix, handleTypeName);

    // Generate destructor function
    generateCBridgeDestructor(file, className, functionPrefix, handleTypeName);

    // Generate validation function
    generateCBridgeValidation(file, className, functionPrefix, handleTypeName);

    // Generate method bridge functions
    generateCBridgeMethods(file, className, functionPrefix, handleTypeName, entity);

    file.beginLine().append("").endLine();
    file.endBlock().append(" // extern \"C\"");

    file.closeFile();
}

void CppProducer::generateStructHeader(std::string_view name,
                                       const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    if (m_dryRun || !m_combinedHeaderFile)
        return;

    // Use combined file instead of individual file
    CppFile& file = *m_combinedHeaderFile;
    file.openFileAppend();

    // Generate includes following the pattern used in generateStructSource
    file.beginLine().append("#include <sal/types.h>").endLine();
    file.beginLine().append("#include <com/sun/star/uno/Reference.hxx>").endLine();
    file.beginLine().append("#include <com/sun/star/uno/Any.hxx>").endLine();
    // Include the struct definition
    std::string headerName(name);
    std::replace(headerName.begin(), headerName.end(), '.', '/');
    file.beginLine().append("#include <").append(headerName).append(".hpp>").endLine();
    file.beginLine().append("").endLine();
    file.beginLine().append("using namespace com::sun::star::uno;").endLine();

    file.beginLine().append("extern \"C\"").endLine().beginBlock();
    // Add struct-specific namespace
    std::string cppNamespace(name);
    size_t lastDot = cppNamespace.rfind('.');
    if (lastDot != std::string::npos)
    {
        cppNamespace = cppNamespace.substr(0, lastDot);
        // Convert dots to double colons for C++ namespace syntax
        size_t pos = 0;
        while ((pos = cppNamespace.find('.', pos)) != std::string::npos)
        {
            cppNamespace.replace(pos, 1, "::");
            pos += 2;
        }
    }

    // Simple opaque struct bridge functions
    OString functionPrefix = getCFunctionPrefix(name);
    OString handleTypeName = functionPrefix + "Handle";
    file.beginLine()
        .append("// Opaque handle for type-safe C FFI")
        .endLine()
        .beginLine()
        .append("typedef void* ")
        .append(handleTypeName)
        .append(";")
        .endLine()
        .beginLine()
        .append("")
        .endLine();

    file.beginLine()
        .append("// Opaque struct creation")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_constructor();")
        .endLine();

    file.beginLine()
        .append("// Opaque handle destruction")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT void ")
        .append(functionPrefix)
        .append("_destroy(")
        .append(handleTypeName)
        .append(" handle);")
        .endLine();

    file.beginLine()
        .append("// Struct from_ptr type casting")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_from_ptr(void* ptr);")
        .endLine();

    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("// Getters/setters for direct struct members")
        .endLine();

    // Opaque member access functions - each member gets getter/setter
    for (const auto& member : entity->getDirectMembers())
    {
        file.beginLine()
            .append("// Opaque getter for ")
            .append(member.name)
            .append(" (type: ")
            .append(member.type)
            .append(")")
            .endLine()
            .beginLine()
            .append("SAL_DLLPUBLIC_EXPORT ")
            .append(getStructGetterReturnType(member.type))
            .append(" ")
            .append(functionPrefix)
            .append("_get_")
            .append(member.name)
            .append("(")
            .append(handleTypeName)
            .append(" handle);")
            .endLine();

        file.beginLine()
            .append("// Opaque setter for ")
            .append(member.name)
            .append(" (type: ")
            .append(member.type)
            .append(")")
            .endLine()
            .beginLine()
            .append("SAL_DLLPUBLIC_EXPORT void ")
            .append(functionPrefix)
            .append("_set_")
            .append(member.name)
            .append("(")
            .append(handleTypeName)
            .append(" handle, ")
            .append(getStructSetterParameterType(member.type))
            .append(" value);")
            .endLine();
    }
    file.endBlock().append("// extern \"C\"").endLine();
    file.closeFile();
}

// Unified include generation function to eliminate duplication
void CppProducer::generateCommonIncludes(CppFile& file, std::string_view name, bool needsLogging,
                                         bool needsUnoTypes)
{
    // Always include sal/types.h
    file.beginLine().append("#include <sal/types.h>").endLine();

    // Include logging if needed
    if (needsLogging)
    {
        file.beginLine().append("#include <sal/log.hxx>").endLine();
    }

    // Include UNO types if needed
    if (needsUnoTypes)
    {
        file.beginLine().append("#include <com/sun/star/uno/Reference.hxx>").endLine();
        file.beginLine().append("#include <com/sun/star/uno/Any.hxx>").endLine();
    }

    // Include the type-specific header
    std::string headerName(name);
    std::replace(headerName.begin(), headerName.end(), '.', '/');
    file.beginLine().append("#include <").append(headerName).append(".hpp>").endLine();
    file.beginLine().append("").endLine();
}

void CppProducer::generateStructSourceIncludes(CppFile& file, std::string_view name)
{
    generateCommonIncludes(file, name, false, true);
}

void CppProducer::generateEnumIncludes(CppFile& file, std::string_view name)
{
    generateCommonIncludes(file, name, false, false);
}

void CppProducer::generateEnumSourceIncludes(CppFile& file, std::string_view name)
{
    generateCommonIncludes(file, name, true, false);
}

void CppProducer::generateEnumHeader(std::string_view name,
                                     const rtl::Reference<unoidl::EnumTypeEntity>&)
{
    if (m_dryRun || !m_combinedHeaderFile)
        return;

    // Use combined file instead of individual file
    CppFile& file = *m_combinedHeaderFile;
    file.openFileAppend();

    // Add separator comment for this enum
    file.beginLine().append("").endLine();
    file.beginLine().append("// === ").append(name).append(" ===").endLine();

    // Generate includes in combined file
    generateEnumIncludes(file, name);

    // Generate extern "C" declarations
    file.beginLine().append("extern \"C\"").endLine().beginBlock();

    // Opaque enum conversion functions - treat enums as opaque values
    OString enumName = convertUnoTypeToCpp(OUString::createFromAscii(std::string(name)));
    OString functionPrefix = getCFunctionPrefix(name); // Use consistent double-underscore naming
    file.beginLine()
        .append("    SAL_DLLPUBLIC_EXPORT ")
        .append(enumName)
        .append(" ")
        .append(functionPrefix)
        .append("_from_i32(sal_Int32 value);")
        .endLine();

    file.beginLine()
        .append("    SAL_DLLPUBLIC_EXPORT sal_Int32 ")
        .append(functionPrefix)
        .append("_to_i32(")
        .append(enumName)
        .append(" value);")
        .endLine();

    file.beginLine().append("}").endLine();

    file.closeFile();
}

void CppProducer::generateEnumSource(std::string_view name,
                                     const rtl::Reference<unoidl::EnumTypeEntity>&)
{
    if (m_dryRun || !m_combinedSourceFile)
        return;

    // Use combined file instead of individual file
    CppFile& file = *m_combinedSourceFile;
    file.openFileAppend();

    // Add separator comment for this enum
    file.beginLine().append("").endLine();
    file.beginLine().append("// === ").append(name).append(" ===").endLine();

    // Generate includes for source file
    generateEnumSourceIncludes(file, name);

    // Generate extern "C" implementations
    file.beginLine().append("extern \"C\"").endLine().beginBlock();

    // Opaque enum conversion functions implementations
    OString enumName = convertUnoTypeToCpp(OUString::createFromAscii(std::string(name)));
    OString functionPrefix = getCFunctionPrefix(name); // Use consistent double-underscore naming

    // from_i32 implementation
    file.beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(enumName)
        .append(" ")
        .append(functionPrefix)
        .append("_from_i32(sal_Int32 value)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("return static_cast<")
        .append(enumName)
        .append(">(value);")
        .endLine()
        .endBlock();

    // to_i32 implementation
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT sal_Int32 ")
        .append(functionPrefix)
        .append("_to_i32(")
        .append(enumName)
        .append(" value)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("return static_cast<sal_Int32>(value);")
        .endLine()
        .endBlock();

    file.beginLine().append("}").endLine();

    file.closeFile();
}

// Unified namespace generation function to eliminate duplication
void CppProducer::generateSourceNamespaces(CppFile& file, std::string_view name)
{
    file.beginLine().append("using namespace com::sun::star::uno;").endLine();

    // Add type-specific namespace
    std::string cppNamespace(name);
    size_t lastDot = cppNamespace.rfind('.');
    if (lastDot != std::string::npos)
    {
        cppNamespace = cppNamespace.substr(0, lastDot);
        // Convert dots to double colons for C++ namespace syntax
        size_t pos = 0;
        while ((pos = cppNamespace.find('.', pos)) != std::string::npos)
        {
            cppNamespace.replace(pos, 1, "::");
            pos += 2;
        }
        file.beginLine().append("using namespace ").append(cppNamespace).append(";").endLine();
    }
    file.beginLine().append("").endLine();
}

void CppProducer::generateStructSourceNamespaces(CppFile& file, std::string_view name)
{
    generateSourceNamespaces(file, name);
}

void CppProducer::generateStructSourceBasicFunctions(CppFile& file, std::string_view functionPrefix,
                                                     std::string_view handleTypeName,
                                                     std::string_view className)
{
    // Generate struct creation function
    file.beginLine()
        .append("// Opaque struct creation")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_constructor()")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("return new ")
        .append(className)
        .append("();")
        .endLine()
        .endBlock()
        .beginLine()
        .append("")
        .endLine();

    // Generate struct destroy function
    file.beginLine()
        .append("// Opaque handle destruction")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT void ")
        .append(functionPrefix)
        .append("_destroy(")
        .append(handleTypeName)
        .append(" handle)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("delete static_cast<")
        .append(className)
        .append("*>(handle);")
        .endLine()
        .endBlock()
        .beginLine()
        .append("")
        .endLine();

    // Generate from_ptr function for struct type casting
    file.beginLine()
        .append("// Struct from_ptr type casting")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_from_ptr(void* ptr)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("if (ptr == nullptr) return nullptr;")
        .endLine()
        .beginLine()
        .append("// For structs, we assume the pointer is already the correct type")
        .endLine()
        .beginLine()
        .append("// Create a copy of the struct")
        .endLine()
        .beginLine()
        .append("")
        .append(className)
        .append("* source = static_cast<")
        .append(className)
        .append("*>(ptr);")
        .endLine()
        .beginLine()
        .append("return new ")
        .append(className)
        .append("(*source);")
        .endLine()
        .endBlock()
        .beginLine()
        .append("")
        .endLine();
}

void CppProducer::generateStructSourceBaseMembers(
    CppFile& file, std::string_view functionPrefix, std::string_view handleTypeName,
    std::string_view className, const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    // Generate getters/setters for base type members (if struct has a base)
    if (!entity->getDirectBase().isEmpty())
    {
        file.beginLine()
            .append("// Getters/setters for base type members (")
            .append(entity->getDirectBase())
            .append(")")
            .endLine();

        // Get base type entity to access its members
        rtl::Reference<unoidl::Entity> baseEntity;
        codemaker::UnoType::Sort baseSort
            = m_typeManager->getSort(entity->getDirectBase(), &baseEntity);

        if (baseSort == codemaker::UnoType::Sort::PlainStruct)
        {
            rtl::Reference<unoidl::PlainStructTypeEntity> baseStruct(
                static_cast<unoidl::PlainStructTypeEntity*>(baseEntity.get()));

            for (const auto& baseMember : baseStruct->getDirectMembers())
            {
                generateStructMemberAccessors(file, functionPrefix, handleTypeName, className,
                                              baseMember);
            }
        }
    }
}

void CppProducer::generateStructSourceDirectMembers(
    CppFile& file, std::string_view functionPrefix, std::string_view handleTypeName,
    std::string_view className, const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    // Generate getters/setters for direct struct members
    file.beginLine().append("// Getters/setters for direct struct members").endLine();

    for (const auto& member : entity->getDirectMembers())
    {
        generateStructMemberAccessors(file, functionPrefix, handleTypeName, className, member);
    }
}

void CppProducer::generateStructMemberAccessors(CppFile& file, std::string_view functionPrefix,
                                                std::string_view handleTypeName,
                                                std::string_view className,
                                                const unoidl::PlainStructTypeEntity::Member& member)
{
    // Generate getter implementation
    file.beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(getStructGetterReturnType(member.type))
        .append(" ")
        .append(functionPrefix)
        .append("_get_")
        .append(member.name)
        .append("(")
        .append(handleTypeName)
        .append(" handle)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append(className)
        .append("* obj = static_cast<")
        .append(className)
        .append("*>(handle);")
        .endLine()
        .beginLine();

    // Handle return value conversion based on type
    if (member.type == u"string")
    {
        // For string types, return pointer to the OUString object
        file.append("return &(obj->").append(member.name).append(");");
    }
    else if (member.type == u"any")
    {
        // For any types, return pointer to the Any
        file.append("return &(obj->").append(member.name).append(");");
    }
    else if (member.type == u"boolean" || member.type == u"byte" || member.type == u"short"
             || member.type == u"unsigned short" || member.type == u"long"
             || member.type == u"unsigned long" || member.type == u"hyper"
             || member.type == u"unsigned hyper" || member.type == u"float"
             || member.type == u"double")
    {
        // For primitive types, return pointer to the member for direct access
        file.append("return &(obj->").append(member.name).append(");");
    }
    else
    {
        // For other types, return pointer to member
        file.append("return &(obj->").append(member.name).append(");");
    }

    file.endLine().endBlock();

    // Generate setter implementation
    file.beginLine()
        .append("SAL_DLLPUBLIC_EXPORT void ")
        .append(functionPrefix)
        .append("_set_")
        .append(member.name)
        .append("(")
        .append(handleTypeName)
        .append(" handle, ")
        .append(getStructSetterParameterType(member.type))
        .append(" value)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append(className)
        .append("* obj = static_cast<")
        .append(className)
        .append("*>(handle);")
        .endLine()
        .beginLine()
        .append("obj->")
        .append(member.name);

    // Handle assignment based on parameter type
    OUString unoType = member.type;

    // Check if we're using typed primitive parameters (passed by value)
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        // For primitive types with typed parameters, assign directly (no dereferencing)
        file.append(" = value;");
    }
    else if (unoType == u"string")
    {
        // For string types, dereference the pointer to OUString
        file.append(" = *reinterpret_cast<const OUString*>(value);");
    }
    else if (unoType == u"any")
    {
        // For any types with typed parameters, dereference the uno_Any*
        file.append(" = *reinterpret_cast<const Any*>(value);");
    }
    else
    {
        // For other types (interfaces, structs, enums), use original logic with void*
        // Check if this is a UNO interface type using proper TypeManager classification
        if (isUnoInterface(unoType))
        {
            // For interface types, assign the pointer directly without dereferencing
            file.append(" = reinterpret_cast<")
                .append(convertUnoTypeToCpp(unoType))
                .append("*>(value);");
        }
        else
        {
            // For non-interface types, dereference the pointer
            file.append(" = *reinterpret_cast<const ")
                .append(convertUnoTypeToCpp(unoType))
                .append("*>(value);");
        }
    }
    file.endLine().endBlock().beginLine().append("").endLine();
}

void CppProducer::generateStructSource(std::string_view name,
                                       const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    if (m_dryRun || !m_combinedSourceFile)
        return;

    // Use combined file instead of individual file
    CppFile& file = *m_combinedSourceFile;
    file.openFileAppend();

    // Add separator comment for this struct
    file.beginLine().append("").endLine();
    file.beginLine().append("// === ").append(name).append(" ===").endLine();

    // Start isolated block for this struct to avoid namespace conflicts
    file.beginLine().append("namespace");
    file.beginBlock();

    // Generate includes
    generateStructSourceIncludes(file, name);

    // Generate namespaces (now in isolated scope)
    generateStructSourceNamespaces(file, name);

    OString functionPrefix = getCFunctionPrefix(name);
    OString handleTypeName = functionPrefix + "Handle";
    // Use fully qualified C++ type name to avoid namespace ambiguity
    OString className = convertUnoTypeToCpp(OUString::createFromAscii(std::string(name)));

    file.beginLine().append("extern \"C\"").endLine().beginBlock();

    // Generate basic functions (creation/destruction)
    generateStructSourceBasicFunctions(file, functionPrefix, handleTypeName, className);

    // Generate accessors for base type members
    generateStructSourceBaseMembers(file, functionPrefix, handleTypeName, className, entity);

    // Generate accessors for direct members
    generateStructSourceDirectMembers(file, functionPrefix, handleTypeName, className, entity);

    file.endBlock().append(" // extern \"C\"").endLine();

    // End isolated block for this struct
    file.endBlock().append(" // End isolated block for ").append(name).endLine();

    file.closeFile();
}

// Helper functions
std::string_view CppProducer::splitName(std::string_view name)
{
    size_t split = name.find_last_of(".::");
    if (split != std::string_view::npos)
        return name.substr(split + 1);
    else
        return name;
}

OString CppProducer::getInterfaceClassName(std::string_view unoName)
{
    return OString(splitName(unoName));
}

OString CppProducer::getCFunctionPrefix(std::string_view unoName)
{
    // Convert com.sun.star.lang.XMain to com__sun__star__lang__XMain for function names
    OString temp(unoName);
    return temp.replaceAll("."_ostr, "__"_ostr);
}

OString CppProducer::getMethodReturnType(std::u16string_view returnType) const
{
    // Use the sophisticated type mapping from the old FFI producer
    return getCppTypeName(returnType);
}

OString CppProducer::getMethodDefaultReturn(std::u16string_view returnType) const
{
    // First resolve any typedefs to their underlying types
    OUString resolvedType = resolveTypedef(returnType);

    // Handle void vs non-void return types properly
    if (resolvedType == u"void")
        return ""_ostr; // void methods don't return anything

    // Return appropriate default values for different types
    if (resolvedType == u"double")
        return "0.0"_ostr;
    else if (resolvedType == u"float")
        return "0.0f"_ostr;
    else if (resolvedType == u"sal_Int32" || resolvedType == u"long" || resolvedType == u"sal_Int16"
             || resolvedType == u"short" || resolvedType == u"sal_Int8" || resolvedType == u"byte"
             || resolvedType == u"sal_uInt8" || resolvedType == u"sal_uInt16"
             || resolvedType == u"sal_uInt32" || resolvedType == u"sal_Int64"
             || resolvedType == u"sal_uInt64" || resolvedType == u"sal_Unicode"
             || resolvedType == u"unsigned short" || resolvedType == u"unsigned long"
             || resolvedType == u"unsigned hyper" || resolvedType == u"hyper")
        return "0"_ostr;
    else if (resolvedType == u"sal_Bool" || resolvedType == u"boolean")
        return "false"_ostr;
    else
    {
        // Check if it's an enum type
        if (isUnoType(resolvedType) && isUnoEnum(resolvedType))
        {
            // Return default enum value (first enum value, typically 0)
            OString returnTypeStr = u2b(resolvedType);
            size_t lastDot = returnTypeStr.lastIndexOf('.');
            std::string_view enumName = (lastDot != std::string_view::npos)
                                            ? returnTypeStr.subView(lastDot + 1)
                                            : returnTypeStr;
            return "static_cast<"_ostr + enumName + "_>(0)"_ostr;
        }
        // Check if it's a struct type
        else if (isUnoType(resolvedType) && isUnoStruct(resolvedType))
        {
            // Return empty struct for struct types
            return "{}"_ostr; // Empty struct literal
        }
        else
        {
            return "nullptr"_ostr; // For pointer/interface/any types
        }
    }
}

OString CppProducer::convertUnoTypeToCpp(std::u16string_view unoType) const
{
    std::string result(unoType.begin(), unoType.end());

    // Handle sequences: []type -> Sequence<type>
    int sequenceDepth = 0;
    while (result.starts_with("[]"))
    {
        result = result.substr(2);
        sequenceDepth++;
    }

    // Check if the inner type is an interface and wrap with Reference<> if needed for sequences
    bool needsReferenceWrapper = false;
    if (sequenceDepth > 0 && result.find(".") != std::string::npos)
    {
        // For UNO types with namespaces, check if it's an interface (before namespace conversion)
        rtl::Reference<unoidl::Entity> entity;
        OUString unoTypeName = OUString::createFromAscii(result);
        codemaker::UnoType::Sort sort = m_typeManager->getSort(unoTypeName, &entity);

        if (sort == codemaker::UnoType::Sort::Interface)
        {
            needsReferenceWrapper = true;
        }
    }

    // Handle template arguments recursively (before namespace conversion)
    result = convertTemplateArguments(result);

    // Convert dots to double colons for C++ namespace syntax
    size_t pos = 0;
    while ((pos = result.find('.', pos)) != std::string::npos)
    {
        result.replace(pos, 1, "::");
        pos += 2;
    }

    // Handle primitive types (only for non-template contexts)
    if (result.find('<') == std::string::npos)
    {
        if (result == "boolean")
            result = "sal_Bool";
        else if (result == "byte")
            result = "sal_Int8";
        else if (result == "short")
            result = "sal_Int16";
        else if (result == "unsigned short")
            result = "sal_uInt16";
        else if (result == "long")
            result = "sal_Int32";
        else if (result == "unsigned long")
            result = "sal_uInt32";
        else if (result == "hyper")
            result = "sal_Int64";
        else if (result == "unsigned hyper")
            result = "sal_uInt64";
        else if (result == "float")
            result = "float";
        else if (result == "double")
            result = "double";
        else if (result == "char")
            result = "sal_Unicode";
        else if (result == "string")
            result = "OUString";
        else if (result == "type")
            result = "Type";
        else if (result == "any")
            result = "Any";
        // else: keep as-is for UNO types like com::sun::star::...
    }

    // Apply Reference<> wrapper if needed
    if (needsReferenceWrapper)
    {
        result = "Reference<" + result + ">";
    }

    // Wrap with Sequence<> for each nesting level
    for (int i = 0; i < sequenceDepth; i++)
    {
        result = "Sequence<" + result + ">";
    }

    return OString(result.c_str());
}

std::string CppProducer::convertTemplateArguments(const std::string& unoType) const
{
    std::string result = unoType;

    // Find template arguments within < >
    size_t start = result.find('<');
    if (start == std::string::npos)
        return result; // No template arguments

    size_t end = result.rfind('>');
    if (end == std::string::npos || end <= start)
        return result; // Malformed template

    // Extract and process template arguments
    std::string prefix = result.substr(0, start + 1);
    std::string suffix = result.substr(end);
    std::string args = result.substr(start + 1, end - start - 1);

    // Split arguments by comma (simple split for now)
    std::string processedArgs = "";
    size_t argStart = 0;
    size_t comma = 0;

    while (true)
    {
        comma = args.find(',', argStart);
        std::string arg;
        if (comma == std::string::npos)
        {
            arg = args.substr(argStart);
        }
        else
        {
            arg = args.substr(argStart, comma - argStart);
        }

        // Trim whitespace
        arg.erase(0, arg.find_first_not_of(" \t"));
        arg.erase(arg.find_last_not_of(" \t") + 1);

        // Recursively process nested template arguments first
        arg = convertTemplateArguments(arg);

        // Handle sequence types first: []type -> Sequence<type>
        if (arg.starts_with("[]"))
        {
            std::string innerType = arg.substr(2);
            arg = "Sequence<" + innerType + ">";
        }

        // Convert basic types
        if (arg == "string")
            arg = "OUString";
        else if (arg == "boolean")
            arg = "sal_Bool";
        else if (arg == "byte")
            arg = "sal_Int8";
        else if (arg == "short")
            arg = "sal_Int16";
        else if (arg == "long")
            arg = "sal_Int32";
        else if (arg == "hyper")
            arg = "sal_Int64";
        else if (arg == "float")
            arg = "float";
        else if (arg == "double")
            arg = "double";
        else if (arg == "char")
            arg = "sal_Unicode";
        else if (arg == "type")
            arg = "Type";
        else if (arg == "any")
            arg = "Any";
        // For complex types, convert dots to ::
        else if (arg.find('.') != std::string::npos)
        {
            size_t pos = 0;
            while ((pos = arg.find('.', pos)) != std::string::npos)
            {
                arg.replace(pos, 1, "::");
                pos += 2;
            }
        }

        if (!processedArgs.empty())
            processedArgs += ",";
        processedArgs += arg;

        if (comma == std::string::npos)
            break;
        argStart = comma + 1;
    }

    return prefix + processedArgs + suffix;
}

void CppProducer::generateInterfaceHeader(std::string_view name,
                                          const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    if (m_dryRun || !m_combinedHeaderFile)
        return;

    // Use combined file instead of individual file
    CppFile& file = *m_combinedHeaderFile;
    file.openFileAppend();

    // Add separator comment for this interface
    file.beginLine().append("").endLine();
    file.beginLine().append("// === ").append(name).append(" ===").endLine();

    file.beginLine().append("extern \"C\"").endLine().beginBlock();

    // Generate opaque handle typedef
    OString functionPrefix = getCFunctionPrefix(name);
    OString handleTypeName = functionPrefix + "Handle";

    file.beginLine()
        .append("// Opaque handle for type-safe C FFI")
        .endLine()
        .beginLine()
        .append("typedef void* ")
        .append(handleTypeName)
        .append(";")
        .endLine()
        .beginLine()
        .append("")
        .endLine();

    // Generate create/destroy functions
    file.beginLine()
        .append("// Interface lifecycle")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_constructor();")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT void ")
        .append(functionPrefix)
        .append("_destructor(")
        .append(handleTypeName)
        .append(" handle);")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_from_ptr(void* ptr);")
        .endLine();

    file.beginLine().append("").endLine();

    // Generate method wrappers for all methods
    file.beginLine().append("// Method wrappers").endLine();

    // TODO: Add interface inheritance support in the future
    // For now, only process direct methods from this interface

    // Process only this interface's direct methods (no inheritance)
    for (const auto& method : entity->getDirectMethods())
    {
        // Generate method signature
        file.beginLine();

        // Return type - use void* for all non-void returns (keep it simple)
        OString returnType = getMethodReturnType(method.returnType);
        if (returnType == "void")
        {
            file.append("SAL_DLLPUBLIC_EXPORT void ");
        }
        else
        {
            file.append("SAL_DLLPUBLIC_EXPORT void* ");
        }

        // Function name: com__sun__star__lang__XMain__methodName
        file.append(functionPrefix)
            .append("_")
            .append(method.name)
            .append("(")
            .append(handleTypeName)
            .append(" handle");

        // Add typed parameters instead of void*
        for (const auto& param : method.parameters)
        {
            file.append(", ")
                .append(getTypedParameterType(param.type, param.direction))
                .append(" ")
                .append(param.name);
        }

        file.append(");");
        file.endLine();
    }

    file.endBlock().beginLine().append("// extern \"C\"").endLine();

    file.closeFile();
}

void CppProducer::generateInterfaceSourceIncludes(CppFile& file, std::string_view name)
{
    generateCommonIncludes(file, name, true, true);
}

void CppProducer::generateInterfaceSourceNamespaces(CppFile& file, std::string_view name)
{
    generateSourceNamespaces(file, name);
}

void CppProducer::generateInterfaceSourceBasicFunctions(CppFile& file,
                                                        std::string_view functionPrefix,
                                                        std::string_view className,
                                                        std::string_view handleTypeName)
{
    // Interface creation
    file.beginLine()
        .append("// Interface creation - creates empty Reference (to be populated by Rust)")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_constructor()")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("//Debug: Interface constructor called for ")
        .append(className)
        .endLine()
        .beginLine()
        .append("SAL_WARN(\"rustmaker\", \"Debug: Creating interface ")
        .append(className)
        .append("\");")
        .endLine()
        .beginLine()
        .append("// Create empty Reference - actual interface will be set from Rust side")
        .endLine()
        .beginLine()
        .append("auto res = new Reference<")
        .append(className)
        .append(">();")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("if (!res)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("SAL_WARN(\"rust_uno_ffi\", \"Failed to allocate Reference<")
        .append(className)
        .append("> wrapper\");")
        .endLine()
        .beginLine()
        .append("return nullptr;")
        .endLine()
        .endBlock()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append(
            "// Note: Empty Reference is expected here - is() will be false until set from Rust")
        .endLine()
        .beginLine()
        .append("SAL_INFO(\"rust_uno_ffi\", \"✅ Successfully created empty Reference<")
        .append(className)
        .append("> wrapper\");")
        .endLine()
        .beginLine()
        .append("return res;")
        .endLine()
        .endBlock();

    file.beginLine().append("").endLine();

    // Interface destruction
    file.beginLine()
        .append("// Interface destruction")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT void ")
        .append(functionPrefix)
        .append("_destructor(")
        .append(handleTypeName)
        .append(" handle)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("delete static_cast<Reference<")
        .append(className)
        .append(">*>(handle);")
        .endLine()
        .endBlock();
    file.beginLine().append("").endLine();

    // Interface casting from void* to typed Reference using UNO_QUERY
    file.beginLine()
        .append("// Interface casting - casts void* to typed Reference using UNO_QUERY")
        .endLine()
        .beginLine()
        .append("SAL_DLLPUBLIC_EXPORT ")
        .append(handleTypeName)
        .append(" ")
        .append(functionPrefix)
        .append("_from_ptr(void* ptr)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("if (ptr == nullptr) return nullptr;")
        .endLine()
        .beginLine()
        .append("try")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("// Convert XInterface* to ")
        .append(className)
        .append(" via UNO_QUERY")
        .endLine()
        .beginLine()
        .append("Reference<XInterface>* interfacePtr = static_cast<Reference<XInterface>*>(ptr);")
        .endLine()
        .beginLine()
        .append("Reference<")
        .append(className)
        .append("> queryResult(interfacePtr->get(), UNO_QUERY);")
        .endLine()
        .beginLine()
        .append("if (!queryResult.is()) return nullptr;")
        .endLine()
        .beginLine()
        .append("Reference<")
        .append(className)
        .append(">* refPtr = new Reference<")
        .append(className)
        .append(">(queryResult);")
        .endLine()
        .beginLine()
        .append("if (refPtr && refPtr->is())")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("SAL_INFO(\"rust_uno_ffi\", \"Successfully created Reference<")
        .append(className)
        .append("> in from_ptr\");")
        .endLine()
        .beginLine()
        .append("return refPtr;")
        .endBlock()
        .beginLine()
        .append("SAL_WARN(\"rust_uno_ffi\", \"Reference is invalid in from_ptr for ")
        .append(className)
        .append("\");")
        .endLine()
        .beginLine()
        .append("delete refPtr;")
        .endLine()
        .beginLine()
        .append("return nullptr;")
        .endLine()
        .endBlock()
        .beginLine()
        .append("catch (const Exception& e)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("SAL_WARN(\"rustmaker\", \"UNO exception in ")
        .append(functionPrefix)
        .append("_from_ptr: \" << e.Message);")
        .endLine()
        .beginLine()
        .append("return nullptr;")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .endBlock();

    file.beginLine().append("").endLine();
}

void CppProducer::generateInterfaceSourceMethodImplementations(
    CppFile& file, std::string_view functionPrefix, std::string_view className,
    std::string_view handleTypeName, const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    // Generate method implementations
    file.beginLine().append("// Method implementations").endLine();

    // TODO: Add interface inheritance support in the future
    // For now, only process direct methods from this interface (no inheritance)

    // Process only this interface's direct methods
    for (const auto& method : entity->getDirectMethods())
    {
        generateSingleInterfaceMethod(file, functionPrefix, className, handleTypeName, method);
    }
}

void CppProducer::generateSingleInterfaceMethod(CppFile& file, std::string_view functionPrefix,
                                                std::string_view className,
                                                std::string_view handleTypeName,
                                                const unoidl::InterfaceTypeEntity::Method& method)
{
    file.beginLine();

    // Use void* for all non-void returns (keep it simple)
    OString returnType = getMethodReturnType(method.returnType);
    if (returnType == "void")
    {
        file.append("SAL_DLLPUBLIC_EXPORT void ");
    }
    else
    {
        file.append("SAL_DLLPUBLIC_EXPORT void* ");
    }

    file.append(functionPrefix)
        .append("_")
        .append(method.name)
        .append("(")
        .append(handleTypeName)
        .append(" handle");

    // Add typed parameters instead of void*
    for (const auto& param : method.parameters)
    {
        file.append(", ")
            .append(getTypedParameterType(param.type, param.direction))
            .append(" ")
            .append(param.name);
    }

    file.append(")").endLine().beginBlock();

    // Implementation body
    file.beginLine()
        .append("//Debug: Method ")
        .append(method.name)
        .append(" called")
        .endLine()
        .beginLine()
        .append("SAL_WARN(\"rustmaker\", \"Debug: Method ")
        .append(className)
        .append("::")
        .append(method.name)
        .append(" called\");")
        .endLine()
        .beginLine()
        .append("Reference<")
        .append(className)
        .append(">* ref = static_cast<Reference<")
        .append(className)
        .append(">*>(handle);")
        .endLine()
        .beginLine()
        .append("//Debug: Checking reference validity")
        .endLine()
        .beginLine()
        .append("if (!ref) { SAL_WARN(\"rustmaker\", \"Debug: ref is null\"); ")
        .append(method.returnType == u"void" ? "return;" : "return nullptr;")
        .append(" }")
        .endLine()
        .beginLine()
        .append("if (!ref->is()) { SAL_WARN(\"rustmaker\", \"Debug: ref->is() is false\"); ")
        .append(method.returnType == u"void" ? "return;" : "return nullptr;")
        .append(" }")
        .endLine();

    // Generate try-catch block for UNO exceptions
    file.beginLine().append("try").endLine().beginBlock();

    // Generate actual UNO method call
    generateActualMethodCall(file, method);

    // Note: Return value conversion is now handled inside generateActualMethodCall()

    // Exception handling
    file.endBlock()
        .beginLine()
        .append("catch (const Exception& ex)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("SAL_WARN(\"rustmaker\", \"UNO exception in ")
        .append(className)
        .append("_")
        .append(method.name)
        .append(": \" << ex.Message);")
        .endLine();

    // Return appropriate error value based on return type
    if (method.returnType == u"void")
    {
        file.beginLine().append("return; // void method - no return on exception").endLine();
    }
    else
    {
        file.beginLine().append("return nullptr; // error return for non-void method").endLine();
    }

    file.endBlock().endBlock();

    file.beginLine().append("").endLine();
}

void CppProducer::generateInterfaceSource(std::string_view name,
                                          const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    if (m_dryRun || !m_combinedSourceFile)
        return;

    // Use combined file instead of individual file
    CppFile& file = *m_combinedSourceFile;
    file.openFileAppend();

    // Add separator comment for this interface
    file.beginLine().append("").endLine();
    file.beginLine().append("// === ").append(name).append(" ===").endLine();

    // Generate includes
    generateInterfaceSourceIncludes(file, name);

    // Add namespaces
    generateInterfaceSourceNamespaces(file, name);

    file.beginLine().append("extern \"C\"").endLine().beginBlock();

    // Generate implementation
    OString functionPrefix = getCFunctionPrefix(name);
    OString handleTypeName = functionPrefix + "Handle";
    // Use fully qualified C++ type name to avoid namespace ambiguity
    OString className = convertUnoTypeToCpp(OUString::createFromAscii(std::string(name)));

    // Generate basic functions (creation/destruction)
    generateInterfaceSourceBasicFunctions(file, functionPrefix, className, handleTypeName);

    // Generate method implementations
    generateInterfaceSourceMethodImplementations(file, functionPrefix, className, handleTypeName,
                                                 entity);

    file.endBlock().append(" // extern \"C\"");
    file.beginLine().append("").endLine();

    file.closeFile();
}

void CppProducer::generateActualMethodCall(CppFile& file,
                                           const unoidl::InterfaceTypeEntity::Method& method)
{
    // Generate actual UNO method call based on method signature
    file.beginLine()
        .append("//Debug: About to call UNO method ")
        .append(method.name)
        .endLine()
        .beginLine()
        .append("SAL_WARN(\"rustmaker\", \"Debug: Calling UNO method ")
        .append(method.name)
        .append(" with ")
        .append(OString::number(static_cast<sal_Int32>(method.parameters.size())))
        .append(" parameters\");")
        .endLine();

    if (method.returnType != u"void")
    {
        // Method with return value
        file.beginLine()
            .append("// Call actual UNO method with return value")
            .endLine()
            .beginLine();

        file.append("auto result = (*ref)->").append(method.name).append("(");
    }
    else
    {
        // Void method
        file.beginLine().append("// Call actual UNO method (void return)").endLine().beginLine();

        file.append("(*ref)->").append(method.name).append("(");
    }

    // Add parameters with proper type conversion
    bool first = true;
    for (const auto& param : method.parameters)
    {
        if (!first)
            file.append(", ");
        first = false;

        // Convert void* parameters to proper UNO types based on parameter type
        std::u16string_view paramType = param.type;

        // Determine if parameter is input-only (needs const) or output/inout (needs non-const reference)
        bool isInputOnly
            = (param.direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);

        if (paramType == u"string")
        {
            if (isInputOnly)
            {
                // Input parameter: direct rtl_uString* to OUString conversion (no casting needed)
                file.append("OUString(").append(param.name).append(")");
            }
            else
            {
                // Output parameter: rtl_uString** needs dereferencing to OUString reference
                file.append("*reinterpret_cast<OUString*>(*").append(param.name).append(")");
            }
        }
        else if (paramType == u"any")
        {
            if (isInputOnly)
            {
                // Input parameter: dereference uno_Any* to get Any& (Any inherits from uno_Any)
                file.append("*reinterpret_cast<const Any*>(").append(param.name).append(")");
            }
            else
            {
                // Output parameter: uno_Any** needs dereferencing to Any reference
                file.append("*reinterpret_cast<Any*>(*").append(param.name).append(")");
            }
        }
        // Handle all primitive types with typed parameter support
        else if (paramType == u"boolean" || paramType == u"byte" || paramType == u"short"
                 || paramType == u"unsigned short" || paramType == u"long"
                 || paramType == u"unsigned long" || paramType == u"hyper"
                 || paramType == u"unsigned hyper" || paramType == u"float"
                 || paramType == u"double")
        {
            // Check if this primitive type is using typed parameters
            OString primitiveType = mapUnoPrimitiveToSal(resolveTypedef(paramType));
            if (!primitiveType.isEmpty())
            {
                // Typed parameter approach - direct value for input, dereference pointer for input/output
                if (isInputOnly)
                    file.append(param.name); // Direct value (sal_Bool, sal_Int32, double, etc.)
                else
                    file.append("*").append(
                        param.name); // Dereference pointer (sal_Bool*, sal_Int32*, double*, etc.)
            }
            else
            {
                // Fallback to void* approach (shouldn't happen for primitives, but just in case)
                if (isInputOnly)
                    file.append("*reinterpret_cast<const ")
                        .append(primitiveType)
                        .append("*>(")
                        .append(param.name)
                        .append(")");
                else
                    file.append("*reinterpret_cast<")
                        .append(primitiveType)
                        .append("*>(")
                        .append(param.name)
                        .append(")");
            }
        }
        else if (paramType == u"type")
        {
            if (isInputOnly)
                file.append("*reinterpret_cast<const Type*>(").append(param.name).append(")");
            else
                file.append("*reinterpret_cast<Type*>(").append(param.name).append(")");
        }
        else if (paramType.starts_with(u"[]"))
        {
            // Sequence type - convert with null pointer safety
            OString cppTypeName = convertUnoTypeToCpp(paramType);

            if (isInputOnly)
            {
                // Input parameter - can use temporary empty sequence
                file.append("(")
                    .append(param.name)
                    .append(" ? *reinterpret_cast<const ")
                    .append(cppTypeName)
                    .append("*>(")
                    .append(param.name)
                    .append(") : ")
                    .append(cppTypeName)
                    .append("())");
            }
            else
            {
                // Output/InOut parameter - needs reference, must dereference non-null pointer
                file.append("*reinterpret_cast<")
                    .append(cppTypeName)
                    .append("*>(")
                    .append(param.name)
                    .append(")");
            }
        }
        else
        {
            // For interfaces and other complex types
            // Check if it's an interface type that needs Reference<> wrapper
            rtl::Reference<unoidl::Entity> entity;
            codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(paramType), &entity);

            if (sort == codemaker::UnoType::Sort::Interface)
            {
                // Interface parameters: void* from Rust points to Reference<ActualType>*
                // We need to dereference and cast via UNO_QUERY to the expected interface type
                OString cppTypeName = convertUnoTypeToCpp(paramType);
                if (isInputOnly)
                {
                    // For input interface parameters, we assume the Rust code passes Reference<SomeInterface>*
                    // We dereference it and cast via UNO_QUERY to the expected interface type
                    file.append("Reference<")
                        .append(cppTypeName)
                        .append(">(static_cast<Reference<XInterface>*>(")
                        .append(param.name)
                        .append(")->get(), UNO_QUERY)");
                }
                else
                    file.append("*reinterpret_cast<Reference<")
                        .append(cppTypeName)
                        .append(">*>(")
                        .append(param.name)
                        .append(")");
            }
            else
            {
                // For structs and other complex types, assume proper type cast
                OString cppTypeName = convertUnoTypeToCpp(paramType);
                if (isInputOnly)
                {
                    file.append("(")
                        .append(param.name)
                        .append(" == nullptr ? ")
                        .append(cppTypeName)
                        .append("() : *reinterpret_cast<const ")
                        .append(cppTypeName)
                        .append("*>(")
                        .append(param.name)
                        .append("))");
                }
                else
                {
                    file.append("*reinterpret_cast<")
                        .append(cppTypeName)
                        .append("*>(")
                        .append(param.name)
                        .append(")");
                }
            }
        }
    }

    file.append(");").endLine();

    // Handle return value conversion for non-void methods
    if (method.returnType != u"void")
    {
        // Use helper function to eliminate code duplication
        generateReturnValueConversion(file, method);
    }
}

void CppProducer::generateReturnValueConversion(CppFile& file,
                                                const unoidl::InterfaceTypeEntity::Method& method)
{
    file.beginLine().append("// Convert result to opaque void* return").endLine();

    std::u16string_view returnType = method.returnType;

    if (returnType == u"string")
    {
        file.beginLine().append("return new OUString(result);").endLine();
    }
    else if (returnType == u"any")
    {
        file.beginLine().append("return new Any(result);").endLine();
    }
    else if (returnType == u"boolean")
    {
        file.beginLine().append("return new sal_Bool(result);").endLine();
    }
    else if (returnType == u"byte")
    {
        file.beginLine().append("return new sal_Int8(result);").endLine();
    }
    else if (returnType == u"short")
    {
        file.beginLine().append("return new sal_Int16(result);").endLine();
    }
    else if (returnType == u"long")
    {
        file.beginLine().append("return new sal_Int32(result);").endLine();
    }
    else if (returnType == u"hyper")
    {
        file.beginLine().append("return new sal_Int64(result);").endLine();
    }
    else if (returnType == u"float")
    {
        file.beginLine().append("return new float(result);").endLine();
    }
    else if (returnType == u"double")
    {
        file.beginLine().append("return new double(result);").endLine();
    }
    else if (returnType == u"type")
    {
        file.beginLine().append("return new Type(result);").endLine();
    }
    else if (returnType.starts_with(u"[]"))
    {
        // Sequence return type - return void* to new sequence
        file.beginLine()
            .append("return new ")
            .append(convertUnoTypeToCpp(returnType))
            .append("(result);")
            .endLine();
    }
    else
    {
        // For interfaces - return Reference<> to avoid instantiating abstract types
        rtl::Reference<unoidl::Entity> entity;
        codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(returnType), &entity);

        if (sort == codemaker::UnoType::Sort::Interface)
        {
            // Interface return type - return new Reference<Interface>(result)
            file.beginLine()
                .append("SAL_INFO(\"rust_uno_ffi\", \"Creating Reference wrapper for return type ")
                .append(convertUnoTypeToCpp(returnType))
                .append("\");")
                .endLine()
                .beginLine()
                .append("auto res = new Reference<")
                .append(convertUnoTypeToCpp(returnType))
                .append(">(result);")
                .endLine()
                .beginLine()
                .append("")
                .endLine()
                .beginLine()
                .append("if (!res)")
                .endLine()
                .beginBlock()
                .beginLine()
                .append("SAL_WARN(\"rust_uno_ffi\", \"Failed to create Reference wrapper for ")
                .append(convertUnoTypeToCpp(returnType))
                .append("\");")
                .endLine()
                .beginLine()
                .append("return nullptr;")
                .endLine()
                .endBlock()
                .beginLine()
                .append("")
                .endLine()
                .beginLine()
                .append("if (!res->is())")
                .endLine()
                .beginBlock()
                .beginLine()
                .append(
                    "SAL_WARN(\"rust_uno_ffi\", \"Reference wrapper is invalid (is() = false) for ")
                .append(convertUnoTypeToCpp(returnType))
                .append("\");")
                .endLine()
                .beginLine()
                .append("delete res;")
                .endLine()
                .beginLine()
                .append("return nullptr;")
                .endLine()
                .endBlock()
                .beginLine()
                .append("")
                .endLine()
                .beginLine()
                .append("SAL_INFO(\"rust_uno_ffi\", \"✅ Successfully created valid Reference "
                        "wrapper for ")
                .append(convertUnoTypeToCpp(returnType))
                .append("\");")
                .endLine()
                .beginLine()
                .append("return res;")
                .endLine();
        }
        else
        {
            // For structs, enums and other types - return void* to new object
            file.beginLine()
                .append("return new ")
                .append(convertUnoTypeToCpp(returnType))
                .append("(result);")
                .endLine();
        }
    }
}

// Type mapping functions (from old FFI producer)

OUString CppProducer::resolveTypedef(std::u16string_view unoType) const
{
    // Recursively resolve typedefs to their underlying types
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(unoType), &entity, &cursor);

    if (sort == codemaker::UnoType::Sort::Typedef)
    {
        rtl::Reference<unoidl::TypedefEntity> typedefEntity(
            static_cast<unoidl::TypedefEntity*>(entity.get()));
        if (typedefEntity.is())
        {
            // Recursively resolve in case of nested typedefs
            OUString resolvedType = typedefEntity->getType();
            return resolveTypedef(resolvedType);
        }
    }

    // If not a typedef or resolution failed, return the original type
    return OUString(unoType);
}

OString CppProducer::mapUnoPrimitiveToSal(std::u16string_view unoType)
{
    // Common primitive type mappings from UNO to SAL types
    if (unoType == u"boolean")
        return "sal_Bool"_ostr;
    else if (unoType == u"byte")
        return "sal_Int8"_ostr;
    else if (unoType == u"short")
        return "sal_Int16"_ostr;
    else if (unoType == u"unsigned short")
        return "sal_uInt16"_ostr;
    else if (unoType == u"long")
        return "sal_Int32"_ostr;
    else if (unoType == u"unsigned long")
        return "sal_uInt32"_ostr;
    else if (unoType == u"hyper")
        return "sal_Int64"_ostr;
    else if (unoType == u"unsigned hyper")
        return "sal_uInt64"_ostr;
    else if (unoType == u"float")
        return "float"_ostr;
    else if (unoType == u"double")
        return "double"_ostr;
    else
        return OString(); // Not a primitive type
}

OString CppProducer::getCppTypeName(std::u16string_view unoType) const
{
    // First, resolve any typedefs to their underlying types
    OUString resolvedType = resolveTypedef(unoType);

    // Map UNO types to C++ types for extern "C" functions
    if (resolvedType == u"void")
        return "void"_ostr;
    else if (resolvedType == u"string")
        return "rtl_uString*"_ostr;
    else if (resolvedType == u"any" || resolvedType == u"com.sun.star.uno.Any")
        return "uno_Any*"_ostr;

    // Try primitive type mapping
    OString primitiveType = mapUnoPrimitiveToSal(resolvedType);
    if (!primitiveType.isEmpty())
        return primitiveType;

    // Handle complex types
    if (resolvedType.startsWith(u"[]"))
        return "uno_Sequence*"_ostr;
    else if (isUnoStruct(resolvedType))
    {
        // For structs, return the FFI struct name with underscore (value type, not pointer)
        OString type = u2b(resolvedType);
        size_t lastDot = type.lastIndexOf('.');
        if (lastDot != std::string_view::npos)
            return OString::Concat(type.subView(lastDot + 1)) + "_";
        else
            return type + "_";
    }
    else if (isUnoEnum(resolvedType))
    {
        // For enums, return the enum name with underscore to match generated FFI enums
        OString type = u2b(resolvedType);
        size_t lastDot = type.lastIndexOf('.');
        if (lastDot != std::string_view::npos)
            return OString::Concat(type.subView(lastDot + 1)) + "_";
        else
            return type + "_";
    }
    else if (isUnoConstantGroup(resolvedType))
    {
        // For constant groups, return the FFI name with underscore
        OString type = u2b(resolvedType);
        size_t lastDot = type.lastIndexOf('.');
        if (lastDot != std::string_view::npos)
            return OString::Concat(type.subView(lastDot + 1)) + "_";
        else
            return type + "_";
    }
    else if (isUnoType(resolvedType))
    {
        // For interfaces and other types, return XInterface* pointer
        return "XInterface*"_ostr;
    }
    else
        return "void*"_ostr; // Default to void* for unknown types
}

OString CppProducer::getTypedParameterType(
    std::u16string_view unoType,
    unoidl::InterfaceTypeEntity::Method::Parameter::Direction direction) const
{
    // Handle string types with typed parameters
    if (unoType == u"string")
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input string: pass as rtl_uString* (no const to avoid casting)
            return "rtl_uString*"_ostr;
        }
        else
        {
            // Input/output string: pass as rtl_uString** (modifiable)
            return "rtl_uString**"_ostr;
        }
    }

    // Handle any types with typed parameters
    if (unoType == u"any")
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input any: pass as uno_Any* (no const to avoid casting)
            return "uno_Any*"_ostr;
        }
        else
        {
            // Input/output any: pass as uno_Any** (modifiable)
            return "uno_Any**"_ostr;
        }
    }

    // Only use typed parameters for basic primitive types (not typedefs)
    // This ensures we don't break existing null-checking logic for complex types
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        OString primitiveType = mapUnoPrimitiveToSal(unoType);
        if (!primitiveType.isEmpty())
        {
            bool isInputOnly
                = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
            if (isInputOnly)
            {
                // Input parameters: pass by value (sal_Bool, sal_Int32, etc.)
                return primitiveType;
            }
            else
            {
                // Input/output parameters: pass by pointer (sal_Bool*, sal_Int32*, etc.)
                return primitiveType + "*";
            }
        }
    }

    // For all other types (typedefs, structs, enums, interfaces, sequences), use void*
    // This maintains compatibility with existing null-checking and casting logic
    return "void*"_ostr;
}

OString CppProducer::getStructGetterReturnType(std::u16string_view unoType) const
{
    // Struct getters return a pointer to the member data (like output parameters)
    // Use the same logic as getTypedParameterType with direction = OUT

    // Handle string types
    if (unoType == u"string")
    {
        return "void*"_ostr; // Return pointer to string for compatibility
    }

    // Handle any types
    if (unoType == u"any")
    {
        return "uno_Any*"_ostr; // Return pointer to Any for direct access
    }

    // Handle primitive types
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        OString primitiveType = mapUnoPrimitiveToSal(unoType);
        if (!primitiveType.isEmpty())
        {
            return primitiveType + "*"; // Return pointer to primitive type
        }
    }

    // For all other types (typedefs, structs, enums, interfaces, sequences), use void*
    return "void*"_ostr;
}

OString CppProducer::getStructSetterParameterType(std::u16string_view unoType) const
{
    // Struct setters take the value to set (like input parameters)
    // Use the same logic as getTypedParameterType with direction = IN

    // Handle string types
    if (unoType == u"string")
    {
        return "rtl_uString*"_ostr; // Take string pointer for compatibility
    }

    // Handle any types
    if (unoType == u"any")
    {
        return "uno_Any*"_ostr; // Take Any pointer directly
    }

    // Handle primitive types
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        OString primitiveType = mapUnoPrimitiveToSal(unoType);
        if (!primitiveType.isEmpty())
        {
            return primitiveType; // Take primitive type by value
        }
    }

    // For all other types
    return "void*"_ostr;
}

OString CppProducer::convertBasicType(const OString& typeName)
{
    OString result = typeName;

    // Convert UNO primitive types to C++ types
    if (result == "long")
        result = "sal_Int32"_ostr;
    else if (result == "short")
        result = "sal_Int16"_ostr;
    else if (result == "byte")
        result = "sal_Int8"_ostr;
    else if (result == "boolean")
        result = "sal_Bool"_ostr;
    else if (result == "double")
        result = "double"_ostr;
    else if (result == "float")
        result = "float"_ostr;
    else if (result == "string")
        result = "OUString"_ostr;
    else if (result == "any")
        result = "Any"_ostr;
    else if (result.indexOf('.') != -1)
    {
        // For UNO types with namespace, convert dots to double colons
        result = result.replaceAll("."_ostr, "::"_ostr);
    }

    return result;
}

// Type classification helper methods

bool CppProducer::isUnoType(std::u16string_view typeName) const
{
    // Check if this is a valid UNO type using TypeManager
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;

    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);

    // Return true for any valid UNO type (not just interfaces/structs/enums)
    return sort != codemaker::UnoType::Sort::Void && sort != codemaker::UnoType::Sort::Boolean
           && sort != codemaker::UnoType::Sort::Byte && sort != codemaker::UnoType::Sort::Short
           && sort != codemaker::UnoType::Sort::UnsignedShort
           && sort != codemaker::UnoType::Sort::Long
           && sort != codemaker::UnoType::Sort::UnsignedLong
           && sort != codemaker::UnoType::Sort::Hyper
           && sort != codemaker::UnoType::Sort::UnsignedHyper
           && sort != codemaker::UnoType::Sort::Float && sort != codemaker::UnoType::Sort::Double
           && sort != codemaker::UnoType::Sort::Char && sort != codemaker::UnoType::Sort::String
           && sort != codemaker::UnoType::Sort::Type && sort != codemaker::UnoType::Sort::Any;
}

bool CppProducer::isUnoStruct(std::u16string_view typeName) const
{
    // Check the type sort using TypeManager
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;

    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);

    // Return true if it's a plain struct or polymorphic struct template
    return (sort == codemaker::UnoType::Sort::PlainStruct);
}

bool CppProducer::isUnoEnum(std::u16string_view typeName) const
{
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);
    return sort == codemaker::UnoType::Sort::Enum;
}

bool CppProducer::isUnoPolymorphicStruct(std::u16string_view typeName) const
{
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);
    return sort == codemaker::UnoType::Sort::PolymorphicStructTemplate;
}

bool CppProducer::isUnoConstantGroup(std::u16string_view typeName) const
{
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);
    return sort == codemaker::UnoType::Sort::ConstantGroup;
}

bool CppProducer::isUnoInterface(std::u16string_view typeName) const
{
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);
    return sort == codemaker::UnoType::Sort::Interface;
}

OString CppProducer::convertUnoTypeToHandle(std::u16string_view unoType)
{
    // Convert UNO type name to typedef handle name
    // com.sun.star.lang.XMain -> com__sun__star__lang__XMainHandle
    OString functionPrefix = getCFunctionPrefix(std::string(unoType.begin(), unoType.end()));
    return functionPrefix + "Handle";
}

void CppProducer::generateServiceHeader(
    std::string_view name, const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity)
{
    if (m_dryRun || !m_combinedHeaderFile)
        return;

    CppFile& file = *m_combinedHeaderFile;
    file.openFileAppend();

    // Add pragma once for header guard
    file.beginLine().append("#pragma once").endLine().endLine();

    // Include the service's interface
    OUString interfaceType = entity->getBase();
    std::string includePath(u2b(interfaceType));
    std::replace(includePath.begin(), includePath.end(), '.', '/');
    file.beginLine().append("#include <").append(includePath).append(".hpp>").endLine();
    file.beginLine().append("#include <com/sun/star/uno/XComponentContext.hpp>").endLine();
    file.beginLine().append("#include <com/sun/star/uno/Reference.hxx>").endLine();
    file.endLine();

    // Generate extern "C" declarations for service creation
    OString functionPrefix = getCFunctionPrefix(name); // Use consistent double-underscore naming
    file.beginLine().append("extern \"C\"").endLine().beginBlock();
    file.beginLine().append("    // Service creation functions for ").append(name).endLine();
    file.beginLine()
        .append("    SAL_DLLPUBLIC_EXPORT void* ")
        .append(functionPrefix)
        .append("_create(void* context_handle);")
        .endLine();
    file.beginLine().append("}").endLine();

    file.closeFile();
}

void CppProducer::generateServiceSource(
    std::string_view name, const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity)
{
    if (m_dryRun || !m_combinedSourceFile)
        return;

    CppFile& file = *m_combinedSourceFile;
    file.openFileAppend();

    // Include the header
    std::string headerPath(name);
    std::replace(headerPath.begin(), headerPath.end(), '.', '/');
    file.beginLine().append("#include <com/sun/star/lang/XMultiComponentFactory.hpp>").endLine();
    file.beginLine().append("#include <sal/log.hxx>").endLine();
    file.endLine();

    // Services don't need namespace declarations - use fully qualified names instead

    // Generate service creation function
    OString functionPrefix = getCFunctionPrefix(name); // Use consistent double-underscore naming
    OUString interfaceType = entity->getBase();
    std::string interfaceCppType(u2b(interfaceType));
    std::replace(interfaceCppType.begin(), interfaceCppType.end(), '.', ':');
    // Fix single colons to double colons for C++ namespace syntax
    size_t colonPos = 0;
    while ((colonPos = interfaceCppType.find(":", colonPos)) != std::string::npos)
    {
        if (colonPos == 0 || interfaceCppType[colonPos - 1] != ':')
        {
            interfaceCppType.insert(colonPos, ":");
            colonPos += 2;
        }
        else
        {
            colonPos++;
        }
    }
    if (!interfaceCppType.empty() && interfaceCppType.substr(0, 2) != "::")
    {
        interfaceCppType = "::" + interfaceCppType;
    }

    file.beginLine().append("extern \"C\"").endLine().beginBlock();
    file.beginLine()
        .append("SAL_DLLPUBLIC_EXPORT void* ")
        .append(functionPrefix)
        .append("_create(void* context_handle)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("try")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("Reference<com::sun::star::uno::XComponentContext>* ctx = "
                "static_cast<Reference<com::sun::star::uno::XComponentContext>*>(context_handle);")
        .endLine()
        .beginLine()
        .append("if (!ctx || !ctx->is()) return nullptr;")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("Reference<com::sun::star::lang::XMultiComponentFactory> factory = "
                "(*ctx)->getServiceManager();")
        .endLine()
        .beginLine()
        .append("if (!factory.is()) return nullptr;")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("Reference<")
        .append("com::sun::star::uno::XInterface")
        .append("> service = Reference<")
        .append("com::sun::star::uno::XInterface")
        .append(">(")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("factory->createInstanceWithContext(\"")
        .append(name)
        .append("\", *ctx), UNO_QUERY);")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("if (!service.is()) return nullptr;")
        .endLine()
        .beginLine()
        .append("return new Reference<")
        .append("com::sun::star::uno::XInterface")
        .append(">(service);")
        .endLine()
        .endBlock()
        .beginLine()
        .append("catch (const Exception& ex)")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("SAL_WARN(\"rustmaker\", \"Service creation failed for ")
        .append(name)
        .append(": \" << ex.Message);")
        .endLine()
        .beginLine()
        .append("return nullptr;")
        .endLine()
        .endBlock()
        .endBlock();
    file.beginLine().append("}").endLine();

    file.closeFile();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
