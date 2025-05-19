/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <set>
#include <vector>
#include <codemaker/codemaker.hxx>
#include <codemaker/exceptiontree.hxx>
#include <codemaker/generatedtypeset.hxx>
#include <codemaker/global.hxx>
#include <codemaker/options.hxx>
#include <codemaker/typemanager.hxx>
#include <codemaker/unotype.hxx>
#include <unoidl/unoidl.hxx>
#include <osl/diagnose.h>
#include <algorithm>
#include <iostream>
#include <frozen/unordered_set.h>

namespace codemaker
{
class GeneratedTypeSet;
}
class PythonOptions;
class TypeManager;
class FileStream;

namespace codemaker::pythonmaker
{
// free functions
rtl::OString getSafePythonIdentifier(const rtl::OString& unoIdentifier);
rtl::OString unoNameToPyModulePath(const rtl::OUString& unoName);
rtl::OString mapUnoTypeToPythonHint(std::u16string_view unoTypeName, TypeManager const& typeManager,
                                    rtl::OUString const& currentModuleUnoName,
                                    rtl::OString const& currentClassName,
                                    std::set<rtl::OString>& imports,
                                    std::set<rtl::OUString>& dependentTypes);
void ensureInitPyi(const rtl::OString& baseOutputDir, const rtl::OUString& unoModuleName);
void produce(rtl::OUString const& name, rtl::Reference<TypeManager> const& manager,
             codemaker::GeneratedTypeSet& generated, PythonOptions const& options);

// PythonStubGenerator: declarations only
class PythonStubGenerator
{
public:
    PythonStubGenerator(rtl::OUString const& name, rtl::Reference<TypeManager> const& manager,
                        codemaker::GeneratedTypeSet& generatedSet, PythonOptions const& options);

    void generate();

private:
    void addImportLine(const rtl::OString& importLine);
    std::vector<unoidl::PlainStructTypeEntity::Member>
    getAllStructMembers(unoidl::PlainStructTypeEntity* entity);
    std::vector<unoidl::ExceptionTypeEntity::Member>
    getAllExceptionMembers(unoidl::ExceptionTypeEntity* entity);
    void generateEnum(unoidl::EnumTypeEntity* entity);
    void generateConstantGroup(unoidl::ConstantGroupEntity* entity);
    void generateTypedef(unoidl::TypedefEntity* entity);
    void generateStruct(unoidl::PlainStructTypeEntity* entity);
    void generatePolyStruct(unoidl::PolymorphicStructTypeTemplateEntity* entity);
    void generateException(unoidl::ExceptionTypeEntity* entity);
    void generateInterface(unoidl::InterfaceTypeEntity* entity);
    void generateService(unoidl::SingleInterfaceBasedServiceEntity* entity);
    void generateSingleton(unoidl::InterfaceBasedSingletonEntity* entity);
    void indent();
    void dedent();
    rtl::OString getIndent() const;

    rtl::OUString m_unoName;
    rtl::Reference<TypeManager> m_typeManager;
    codemaker::GeneratedTypeSet& m_generatedSet;
    PythonOptions const& m_options;
    rtl::OString m_baseOutputDir;
    rtl::OString m_filePath;
    rtl::OUString m_moduleName;
    rtl::OString m_pyClassName;
    rtl::OString m_pySafeClassName;
    int m_indentLevel;
    bool m_verbose;
    std::set<rtl::OString> m_imports;
    std::set<rtl::OUString> m_dependentTypes;
    rtl::OStringBuffer m_buffer;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
