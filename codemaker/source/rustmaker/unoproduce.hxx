/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <memory>

#include <codemaker/typemanager.hxx>
#include <unoidl/unoidl.hxx>

#include <rtl/string.hxx>
#include "rustoptions.hxx"
#include "rustfile.hxx"
#include "rustproduce.hxx"
#include "cpproduce.hxx"

// Forward declarations
class CppFile;

/**
 * Main class responsible for generating Rust code from UNO type definitions.
 * Processes UNO IDL types and produces corresponding Rust bindings.
 */
class UnoProducer
{
public:
    UnoProducer()
        : m_manager(new TypeManager())
    {
    }

public:
    // Initialize the producer with command line options
    void initProducer(const RustOptions& options);
    // Generate code for all requested types
    void produceAll();
    // Finalize opaque pointer generation
    void finalizeGeneration();

private:
    // Main UNO type processor - coordinates opaque generation for all types
    void produceType(const OString& name);
    void produceModule(std::string_view name, const rtl::Reference<unoidl::MapCursor>& cursor);

    // Generation for all UNO types - coordinates RustProducer + CppProducer
    void produceEnum(std::string_view name, const rtl::Reference<unoidl::EnumTypeEntity>& entity);
    void produceStruct(std::string_view name,
                       const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);
    void
    producePolyStruct(std::string_view name,
                      const rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity>& entity);
    void produceException(std::string_view name,
                          const rtl::Reference<unoidl::ExceptionTypeEntity>& entity);
    void produceInterface(std::string_view name,
                          const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);
    void produceTypedef(std::string_view name, const rtl::Reference<unoidl::TypedefEntity>& entity);
    void produceConstantGroup(std::string_view name,
                              const rtl::Reference<unoidl::ConstantGroupEntity>& entity);
    void produceService(std::string_view name,
                        const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity);
    void produceSingleton(std::string_view name,
                          const rtl::Reference<unoidl::InterfaceBasedSingletonEntity>& entity);

    // Type name conversion for Rust output
    OString getRustName(std::string_view name, bool istype = false);
    OString getRustName(std::u16string_view name, bool istype = false);

    //
    static OString getBaseUnoName(std::string_view name);
    static OString getBaseUnoName(std::u16string_view name);

    // Core data members
    rtl::Reference<TypeManager> m_manager;

    std::unordered_set<OString> m_startingTypes; // Types requested by user
    std::unordered_set<OString> m_typesProduced; // Prevent duplicate generation
    std::unordered_map<OString, OString> m_typedefs;

    // Runtime configuration
    OString m_rustOutputDir;
    bool m_verbose;
    bool m_dryRun;
    // Producer instances for coordinated opaque generation
    std::unique_ptr<RustProducer> m_rustProducer;
    std::unique_ptr<CppProducer> m_cppProducer;

    // Name processing utilities
    static std::string_view splitName(std::string_view name);
    static OString handleName(std::string_view name, bool istype);
    static OString handleName(std::u16string_view name, bool istype);
    static OString getSafeIdentifier(std::string_view name, bool istype);
    static OString getSafeIdentifier(std::u16string_view name, bool istype);
    static void separatedForeach(const auto& items, auto&& sepFunc, auto&& itemFunc);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
