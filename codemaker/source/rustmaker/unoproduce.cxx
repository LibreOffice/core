/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <memory>

#include <o3tl/string_view.hxx>
#include <string_view>

#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_set.h>
#include <frozen/unordered_map.h>

#include "unoproduce.hxx"
#include "rustproduce.hxx"
#include "cpproduce.hxx"
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "rustfile.hxx"
#include <unoidl/unoidl.hxx>
#include <codemaker/unotype.hxx>

namespace
{
// Rust keywords that need special handling to avoid naming conflicts
constexpr auto aReservedKeywords
    = frozen::make_unordered_set<std::string_view>(
        { "as",     "break", "const",  "continue", "crate", "else",  "enum",   "extern",
          "false",  "fn",    "for",    "if",       "impl",  "in",    "let",    "loop",
          "match",  "mod",   "move",   "mut",      "pub",   "ref",   "return", "Result",
          "self",   "Self",  "static", "struct",   "super", "trait", "true",   "type",
          "unsafe", "use",   "where",  "while",    "async", "await", "dyn",    "try" });

// Maps UNO basic types to their Rust equivalents
// TODO: need to edit the core types Implemented manually
const auto aBaseTypes = frozen::make_unordered_map<std::string_view, OString>(
    { { "boolean", "bool"_ostr },
      { "char", "char"_ostr },
      { "byte", "i8"_ostr },
      { "short", "i16"_ostr },
      { "unsigned short", "u16"_ostr },
      { "long", "i32"_ostr },
      { "unsigned long", "u32"_ostr },
      { "hyper", "i64"_ostr },
      { "unsigned hyper", "u64"_ostr },
      { "float", "f32"_ostr },
      { "double", "f64"_ostr },
      { "string", "String"_ostr },
      { "void", "()"_ostr },
      { "type", "uno::type"_ostr }, // TODO:
      { "any", "uno::any"_ostr }, // TODO:
      // TODO: These exception types need proper implementation
      { "com.sun.star.uno.Exception", "com::sun::star::uno::Exception"_ostr } });
// Note: XInterface removed from baseTypes so it can be generated like other interfaces
}

std::string_view UnoProducer::splitName(std::string_view name)
{
    size_t split = name.find_last_of(".::");
    if (split != std::string_view::npos)
        return name.substr(split + 1);
    else
        return name;
}

OString UnoProducer::handleName(std::string_view name, bool istype)
{
    // Convert UNO dotted names to Rust double-colon syntax
    OString temp(name);
    OString result = temp.replaceAll("."_ostr, "::"_ostr);
    temp = result;
    if (istype)
        result = "crate::"_ostr + temp + "::" + splitName(name);
    return result;
}

OString UnoProducer::handleName(std::u16string_view name, bool istype)
{
    return handleName(u2b(name), istype);
}

OString UnoProducer::getBaseUnoName(std::string_view name)
{
    // Extract base type name by removing array brackets and template parameters
    size_t start = name.find_first_not_of("[]");
    if (start == std::string_view::npos)
        start = 0;

    size_t end = name.find_first_of('<');
    if (end == std::string_view::npos)
        end = name.size();

    return OString(name.substr(start, end - start));
}
OString UnoProducer::getBaseUnoName(std::u16string_view name) { return getBaseUnoName(u2b(name)); }

OString UnoProducer::getSafeIdentifier(std::string_view name, bool istype = false)
{
    // Add underscore suffix to avoid Rust keyword conflicts
    OString temp = handleName(name, istype);
    return aReservedKeywords.contains(temp) ? temp + "_"_ostr : temp;
}

OString UnoProducer::getSafeIdentifier(std::u16string_view name, bool istype = false)
{
    return getSafeIdentifier(u2b(name), istype);
}

void UnoProducer::separatedForeach(const auto& items, auto&& sepFunc, auto&& itemFunc)
{
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        if (it != items.begin())
            sepFunc();
        itemFunc(*it);
    }
}

void UnoProducer::initProducer(const RustOptions& options)
{
    m_manager = rtl::Reference<TypeManager>(new TypeManager());

    // Load type providers from input files (following old approach pattern)
    for (const OString& file : options.getInputFiles())
        m_manager->loadProvider(convertToFileUrl(file), true);
    for (const OString& file : options.getExtraInputFiles())
        m_manager->loadProvider(convertToFileUrl(file), false);

    // Get separate output directories for C++ and Rust files
    OString cppOutputDir = options.getOption("--cpp-output-dir"_ostr);
    m_rustOutputDir = options.getOption("--rust-output-dir"_ostr);
    // Enable dry-run mode (don't actually write files)
    m_dryRun = options.isValid("--dry-run"_ostr);
    // Enable verbose output for debugging
    m_verbose = options.isValid("--verbose"_ostr);

    // Initialize both producers with separate output directories
    m_rustProducer
        = std::make_unique<RustProducer>(m_rustOutputDir, m_verbose, m_dryRun, m_manager);
    m_cppProducer = std::make_unique<CppProducer>(cppOutputDir, m_verbose, m_dryRun, m_manager);

    // Initialize combined C++ output file
    m_cppProducer->initializeCombinedFile();

    // Parse type selection criteria (following old approach exactly)
    if (options.isValid("--types"_ostr))
    {
        const OString& names(options.getOption("--types"_ostr));
        // Process semicolon-separated list of type names/patterns
        for (size_t i = 0; i != std::string_view::npos;)
        {
            std::string_view name(o3tl::getToken(names, ';', i));
            if (name == "*")
                // "*" means generate all types
                m_startingTypes.insert(""_ostr);
            else if (name.ends_with(".*"))
                // "namespace.*" means generate all types in namespace
                m_startingTypes.emplace(name.substr(0, name.size() - 2));
            else
                // Specific type name
                m_startingTypes.emplace(name);
        }
    }
    else
    {
        // Default: generate all types
        m_startingTypes.insert(""_ostr);
    }
}

void UnoProducer::produceAll()
{
    for (const OString& name : m_startingTypes)
        produceType(name);
}

void UnoProducer::finalizeGeneration()
{
    // Finalize combined C++ output file
    if (m_cppProducer)
    {
        m_cppProducer->finalizeCombinedFile();
    }

    // Always finalize mod.rs files in opaque mode
    if (!m_dryRun)
    {
        if (m_verbose)
            std::cout << "Finalizing mod.rs files with complete module information...\n";

        RustFile::finalizeModFiles(std::filesystem::path(m_rustOutputDir.getStr()));

        if (m_verbose)
            std::cout << "Module finalization complete\n";
    }
}

void UnoProducer::produceType(const OString& name)
{
    // Skip already processed types to avoid duplicates
    if (m_typesProduced.contains(name))
        return;

    m_typesProduced.insert(name);

    // Skip built-in types that don't need code generation
    if (aBaseTypes.contains(name))
        return;

    OUString uname(b2u(name));

    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;

    // Only generate code for types from primary providers (not dependencies)
    if (m_manager->foundAtPrimaryProvider(uname))
    {
        // Dispatch to appropriate generator based on UNO type kind
        switch (m_manager->getSort(uname, &entity, &cursor))
        {
            case codemaker::UnoType::Sort::Module:
                produceModule(name, cursor);
                break;

            case codemaker::UnoType::Sort::Enum:
                produceEnum(name, dynamic_cast<unoidl::EnumTypeEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::PlainStruct:
                produceStruct(name, dynamic_cast<unoidl::PlainStructTypeEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::PolymorphicStructTemplate:
                producePolyStruct(
                    name, dynamic_cast<unoidl::PolymorphicStructTypeTemplateEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::Exception:
                produceException(name, dynamic_cast<unoidl::ExceptionTypeEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::Interface:
                produceInterface(name, dynamic_cast<unoidl::InterfaceTypeEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::Typedef:
                produceTypedef(name, dynamic_cast<unoidl::TypedefEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::ConstantGroup:
                produceConstantGroup(name,
                                     dynamic_cast<unoidl::ConstantGroupEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::SingleInterfaceBasedService:
                produceService(
                    name, dynamic_cast<unoidl::SingleInterfaceBasedServiceEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::InterfaceBasedSingleton:
                produceSingleton(
                    name, dynamic_cast<unoidl::InterfaceBasedSingletonEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::AccumulationBasedService:
            case codemaker::UnoType::Sort::ServiceBasedSingleton:
                // old-style services and singletons not supported
                break;

            default:
                throw CannotDumpException(u"entity '"_ustr + uname + u"' has unexpected type"_ustr);
        }
    }
    else
    {
        // type from --extra-types
        switch (m_manager->getSort(uname, &entity, &cursor))
        {
            case codemaker::UnoType::Sort::Typedef:
                produceTypedef(name, dynamic_cast<unoidl::TypedefEntity*>(entity.get()));
                break;

            default:
                break;
        }
    }
}

void UnoProducer::produceModule(std::string_view name,
                                const rtl::Reference<unoidl::MapCursor>& cursor)
{
    OUString moduleName;
    while (cursor->getNext(&moduleName).is())
    {
        OString memberName = name.empty() ? u2b(moduleName) : name + "."_ostr + u2b(moduleName);
        produceType(memberName);
    }
}
void UnoProducer::produceEnum(std::string_view name,
                              const rtl::Reference<unoidl::EnumTypeEntity>& entity)
{
    // Coordinate both producers for complete opaque enum generation
    m_cppProducer->produceEnum(name, entity); // C++ side: bridge functions
    m_rustProducer->produceEnum(name, entity); // Rust side: wrapper with extern declarations
}

void UnoProducer::produceStruct(std::string_view name,
                                const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    // Coordinate both producers for complete struct generation
    m_cppProducer->produceStruct(name, entity); // C++ side: wrapper class + bridge functions
    m_rustProducer->produceStruct(name, entity); // Rust side: wrapper with extern declarations
}

void UnoProducer::producePolyStruct(
    std::string_view name, const rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity>&)
{
    // TODO: Implement polymorphic struct support
    if (m_verbose)
        std::cout << "[poly-struct] " << name << " -> skipping polymorphic structs for now\n";
}

void UnoProducer::produceException(std::string_view name,
                                   const rtl::Reference<unoidl::ExceptionTypeEntity>&)
{
    // TODO: Handle exceptions as special structs
    if (m_verbose)
        std::cout << "[exception] " << name << " -> treating as struct\n";
}

void UnoProducer::produceInterface(std::string_view name,
                                   const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    // Coordinate both producers for complete interface generation
    m_cppProducer->produceInterface(name, entity); // C++ side: wrapper class + bridge functions
    m_rustProducer->produceInterface(name, entity); // Rust side: wrapper with extern declarations
}

void UnoProducer::produceTypedef(std::string_view name,
                                 const rtl::Reference<unoidl::TypedefEntity>&)
{
    // TODO: Handle typedefs properly
    if (m_verbose)
        std::cout << "[typedef] " << name << " -> skipping typedef resolution\n";
}

void UnoProducer::produceConstantGroup(std::string_view name,
                                       const rtl::Reference<unoidl::ConstantGroupEntity>&)
{
    // TODO: Handle constant groups
    if (m_verbose)
        std::cout << "[constant-group] " << name << " -> skipping constants for now\n";
}

void UnoProducer::produceService(
    std::string_view name, const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity)
{
    // Generate service creation wrappers that access services through their interfaces
    if (m_verbose)
        std::cout << "[service] " << name << " -> generating service creation methods\n";

    // Coordinate both producers for complete service generation
    m_cppProducer->produceService(name, entity); // C++ side: service creation bridge functions
    m_rustProducer->produceService(name,
                                   entity); // Rust side: service wrapper with creation methods
}

void UnoProducer::produceSingleton(std::string_view name,
                                   const rtl::Reference<unoidl::InterfaceBasedSingletonEntity>&)
{
    // Singletons are accessed through their interfaces
    if (m_verbose)
        std::cout << "[singleton] " << name << " -> use underlying interface instead\n";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
