/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <vector>

#include <codemaker/codemaker.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/strbuf.hxx>
#include <unoidl/unoidl.hxx>

#include "netproduce.hxx"
#include "csharpfile.hxx"

namespace
{
const std::unordered_set<OString> s_reservedKeywords{
    "abstract"_ostr, "as"_ostr,       "base"_ostr,       "bool"_ostr,      "break"_ostr,
    "byte"_ostr,     "case"_ostr,     "catch"_ostr,      "char"_ostr,      "checked"_ostr,
    "class"_ostr,    "const"_ostr,    "continue"_ostr,   "decimal"_ostr,   "default"_ostr,
    "delegate"_ostr, "do"_ostr,       "double"_ostr,     "else"_ostr,      "enum"_ostr,
    "event"_ostr,    "explicit"_ostr, "extern"_ostr,     "false"_ostr,     "finally"_ostr,
    "fixed"_ostr,    "float"_ostr,    "for"_ostr,        "foreach"_ostr,   "goto"_ostr,
    "if"_ostr,       "implicit"_ostr, "in"_ostr,         "int"_ostr,       "interface"_ostr,
    "internal"_ostr, "is"_ostr,       "lock"_ostr,       "long"_ostr,      "namespace"_ostr,
    "new"_ostr,      "null"_ostr,     "object"_ostr,     "operator"_ostr,  "out"_ostr,
    "override"_ostr, "params"_ostr,   "private"_ostr,    "protected"_ostr, "public"_ostr,
    "readonly"_ostr, "ref"_ostr,      "return"_ostr,     "sbyte"_ostr,     "sealed"_ostr,
    "short"_ostr,    "sizeof"_ostr,   "stackalloc"_ostr, "static"_ostr,    "string"_ostr,
    "struct"_ostr,   "switch"_ostr,   "this"_ostr,       "throw"_ostr,     "true"_ostr,
    "try"_ostr,      "typeof"_ostr,   "uint"_ostr,       "ulong"_ostr,     "unchecked"_ostr,
    "unsafe"_ostr,   "ushort"_ostr,   "using"_ostr,      "virtual"_ostr,   "void"_ostr,
    "volatile"_ostr, "while"_ostr,
};

const std::unordered_map<OString, OString> s_baseTypes{
    { "boolean"_ostr, "bool"_ostr },
    { "char"_ostr, "char"_ostr },
    { "byte"_ostr, "sbyte"_ostr },
    { "short"_ostr, "short"_ostr },
    { "unsigned short"_ostr, "ushort"_ostr },
    { "long"_ostr, "int"_ostr },
    { "unsigned long"_ostr, "uint"_ostr },
    { "hyper"_ostr, "long"_ostr },
    { "unsigned hyper"_ostr, "ulong"_ostr },
    { "float"_ostr, "float"_ostr },
    { "double"_ostr, "double"_ostr },
    { "string"_ostr, "string"_ostr },
    { "void"_ostr, "void"_ostr },
    { "type"_ostr, "System.Type"_ostr },
    { "any"_ostr, "com.sun.star.uno.Any"_ostr },
    { "com.sun.star.uno.Exception"_ostr, "com.sun.star.uno.UnoException"_ostr },
    { "com.sun.star.uno.XInterface"_ostr, "com.sun.star.uno.IQueryInterface"_ostr },
};

std::tuple<bool, std::string_view, std::string_view> splitName(std::string_view name)
{
    size_t split = name.find_last_of('.');
    if (split != std::string_view::npos)
        return std::make_tuple(true, name.substr(0, split), name.substr(split + 1));
    else
        return std::make_tuple(false, "", name);
}

OString getBaseUnoName(std::string_view name)
{
    size_t start = name.find_first_not_of("[]");
    if (start == std::string_view::npos)
        start = 0;

    size_t end = name.find_first_of('<');
    if (end == std::string_view::npos)
        end = name.size();

    return OString(name.substr(start, end - start));
}
OString getBaseUnoName(std::u16string_view name) { return getBaseUnoName(u2b(name)); }

OString getSafeIdentifier(std::string_view name)
{
    OString temp(name);
    return s_reservedKeywords.contains(temp) ? "@"_ostr + temp : temp;
}
OString getSafeIdentifier(std::u16string_view name) { return getSafeIdentifier(u2b(name)); }

void separatedForeach(const auto& items, auto&& sepFunc, auto&& itemFunc)
{
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        if (it != items.begin())
            sepFunc();
        itemFunc(*it);
    }
}
}

void NetProducer::initProducer(const NetOptions& options)
{
    m_outputDir = options.getOption("--output-dir"_ostr);
    m_dryRun = options.isValid("--dry-run"_ostr);
    m_verbose = options.isValid("--verbose"_ostr);

    if (options.isValid("--types"_ostr))
    {
        const OString& names(options.getOption("--types"_ostr));
        for (size_t i = 0; i != std::string_view::npos;)
        {
            std::string_view name(o3tl::getToken(names, ';', i));
            if (name == "*")
                m_startingTypes.insert(""_ostr);
            else if (name.ends_with(".*"))
                m_startingTypes.emplace(name.substr(0, name.size() - 2));
            else
                m_startingTypes.emplace(name);
        }
    }
    else
    {
        m_startingTypes.insert(""_ostr);
    }

    for (const OString& file : options.getInputFiles())
        m_manager->loadProvider(convertToFileUrl(file), true);
    for (const OString& file : options.getExtraInputFiles())
        m_manager->loadProvider(convertToFileUrl(file), false);
}

void NetProducer::produceAll()
{
    for (const OString& name : m_startingTypes)
        produceType(name);
}

void NetProducer::produceType(const OString& name)
{
    if (m_typesProduced.contains(name))
        return;

    m_typesProduced.insert(name);

    if (s_baseTypes.contains(name))
        return;

    OUString uname(b2u(name));

    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;

    if (m_manager->foundAtPrimaryProvider(uname))
    {
        switch (m_manager->getSort(uname, &entity, &cursor))
        {
            case codemaker::UnoType::Sort::Module:
                produceModule(name, cursor);
                break;

            case codemaker::UnoType::Sort::Enum:
                produceEnum(name, dynamic_cast<unoidl::EnumTypeEntity*>(entity.get()));
                break;

            case codemaker::UnoType::Sort::PlainStruct:
                producePlainStruct(name,
                                   dynamic_cast<unoidl::PlainStructTypeEntity*>(entity.get()));
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

void NetProducer::produceModule(std::string_view name,
                                const rtl::Reference<unoidl::MapCursor>& cursor)
{
    OUString moduleName;
    while (cursor->getNext(&moduleName).is())
    {
        OString memberName = name.empty() ? u2b(moduleName) : name + "."_ostr + u2b(moduleName);
        produceType(memberName);
    }
}

void NetProducer::produceEnum(std::string_view name,
                              const rtl::Reference<unoidl::EnumTypeEntity>& entity)
{
    CSharpFile file(m_outputDir, name);

    if (m_verbose)
        std::cout << "[enum] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    auto[hasNamespace, namespaceName, typeName] = splitName(name);

    if (hasNamespace)
        file.beginLine().append("namespace ").append(namespaceName).endLine().beginBlock();

    file.beginLine()
        .append("[com.sun.star.uno.UnoGenerated]")
        .endLine()
        .beginLine()
        .append("public enum ")
        .append(getSafeIdentifier(typeName))
        .endLine()
        .beginBlock();
    for (const auto& member : entity->getMembers())
    {
        for (const auto& anno : member.annotations)
            if (anno == "deprecated")
                file.beginLine().append("[System.Obsolete]").endLine();
        file.beginLine()
            .append(getSafeIdentifier(member.name))
            .append(" = ")
            .append(OString::number(member.value))
            .append(",")
            .endLine();
    }
    file.endBlock();

    if (hasNamespace)
        file.endBlock();

    file.closeFile();
}

void NetProducer::producePlainStruct(std::string_view name,
                                     const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    // produce referenced types
    const auto& base = entity->getDirectBase();
    if (!base.isEmpty())
        produceType(getBaseUnoName(base));
    for (const auto& member : entity->getDirectMembers())
        produceType(getBaseUnoName(member.type));

    CSharpFile file(m_outputDir, name);

    // verbose and dry run checks
    if (m_verbose)
        std::cout << "[struct] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    // output namespace block
    auto[hasNamespace, namespaceName, typeName] = splitName(name);

    if (hasNamespace)
        file.beginLine().append("namespace ").append(namespaceName).endLine().beginBlock();

    // generate struct and base structs list
    file.beginLine()
        .append("[com.sun.star.uno.UnoGenerated]")
        .endLine()
        .beginLine()
        .append("public class ")
        .append(getSafeIdentifier(typeName));
    if (!base.isEmpty())
        file.append(" : ").append(getNetName(base));
    file.endLine().beginBlock();

    // generate default constructor
    file.beginLine()
        .append("public ")
        .append(getSafeIdentifier(typeName))
        .append("()")
        .endLine()
        .beginBlock()
        .endBlock();
    file.endLine(); // extra blank line

    // generate full constructor
    std::vector<unoidl::PlainStructTypeEntity::Member> baseFields;
    {
        OUString baseTypeName = base;
        while (!baseTypeName.isEmpty())
        {
            rtl::Reference<unoidl::Entity> baseEntity;
            if (m_manager->getSort(baseTypeName, &baseEntity)
                != codemaker::UnoType::Sort::PlainStruct)
                throw CannotDumpException("'" + b2u(name) + "' base type '" + baseTypeName
                                          + "' is not a plain struct");

            rtl::Reference<unoidl::PlainStructTypeEntity> ref(
                dynamic_cast<unoidl::PlainStructTypeEntity*>(baseEntity.get()));
            baseFields.insert(baseFields.begin(), ref->getDirectMembers().begin(),
                              ref->getDirectMembers().end());

            baseTypeName = ref->getDirectBase();
        }
    }

    std::vector<unoidl::PlainStructTypeEntity::Member> allFields(entity->getDirectMembers());
    allFields.insert(allFields.begin(), baseFields.begin(), baseFields.end());

    file.beginLine().append("public ").append(getSafeIdentifier(typeName)).append("(");
    separatedForeach(
        allFields, [&file]() { file.append(", "); },
        [this, &file](const auto& member) {
            file.append(getNetName(member.type)).append(" ").append(getSafeIdentifier(member.name));
        });
    file.append(")").endLine();
    file.beginLine().extraIndent().append(": base(");
    separatedForeach(baseFields, [&file]() { file.append(", "); },
                     [&file](const auto& member) { file.append(getSafeIdentifier(member.name)); });
    file.append(")").endLine();
    file.beginBlock();
    for (const auto& member : entity->getDirectMembers())
    {
        file.beginLine()
            .append("this.")
            .append(getSafeIdentifier(member.name))
            .append(" = ")
            .append(getSafeIdentifier(member.name))
            .append(";")
            .endLine();
    }
    file.endBlock();
    file.endLine(); // extra blank line

    // generate struct fields
    for (const auto& member : entity->getDirectMembers())
    {
        for (const auto& anno : member.annotations)
            if (anno == "deprecated")
                file.beginLine().append("[System.Obsolete]").endLine();
        file.beginLine()
            .append("public ")
            .append(getNetName(member.type))
            .append(" ")
            .append(getSafeIdentifier(member.name))
            .append(";")
            .endLine();
    }

    file.endBlock();
    if (hasNamespace)
        file.endBlock();
    file.closeFile();
}

void NetProducer::producePolyStruct(
    std::string_view name,
    const rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity>& entity)
{
    // produce referenced types
    for (const auto& member : entity->getMembers())
        if (!member.parameterized)
            produceType(getBaseUnoName(member.type));

    CSharpFile file(m_outputDir, name);

    // verbose and dry run checks
    if (m_verbose)
        std::cout << "[polystruct] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    // output namespace block
    auto[hasNamespace, namespaceName, typeName] = splitName(name);

    if (hasNamespace)
        file.beginLine().append("namespace ").append(namespaceName).endLine().beginBlock();

    // generate struct and type parameters list
    file.beginLine()
        .append("[com.sun.star.uno.UnoGenerated]")
        .endLine()
        .beginLine()
        .append("public class ")
        .append(getSafeIdentifier(typeName))
        .append("<");
    separatedForeach(entity->getTypeParameters(), [&file]() { file.append(", "); },
                     [&file](const auto& param) { file.append(param); });
    file.append(">").endLine().beginBlock();

    // generate default constructor
    file.beginLine()
        .append("public ")
        .append(getSafeIdentifier(typeName))
        .append("()")
        .endLine()
        .beginBlock()
        .endBlock();
    file.endLine(); // extra blank line

    // generate full constructor
    file.beginLine().append("public ").append(getSafeIdentifier(typeName)).append("(");
    separatedForeach(entity->getMembers(), [&file]() { file.append(", "); },
                     [this, &file](const auto& member) {
                         file.append(member.parameterized ? u2b(member.type)
                                                          : getNetName(member.type))
                             .append(" ")
                             .append(getSafeIdentifier(member.name));
                     });
    file.append(")").endLine();
    file.beginBlock();
    for (const auto& member : entity->getMembers())
    {
        file.beginLine()
            .append("this.")
            .append(getSafeIdentifier(member.name))
            .append(" = ")
            .append(getSafeIdentifier(member.name))
            .append(";")
            .endLine();
    }
    file.endBlock();
    file.endLine(); // extra blank line

    // generate struct fields
    for (const auto& member : entity->getMembers())
    {
        for (const auto& anno : member.annotations)
            if (anno == "deprecated")
                file.beginLine().append("[System.Obsolete]").endLine();
        file.beginLine()
            .append("public ")
            .append(member.parameterized ? u2b(member.type) : getNetName(member.type))
            .append(" ")
            .append(getSafeIdentifier(member.name))
            .append(";")
            .endLine();
    }

    file.endBlock();
    if (hasNamespace)
        file.endBlock();
    file.closeFile();
}

void NetProducer::produceException(std::string_view name,
                                   const rtl::Reference<unoidl::ExceptionTypeEntity>& entity)
{
    // produce referenced types
    const auto& base = entity->getDirectBase();
    if (!base.isEmpty())
        produceType(getBaseUnoName(base));
    for (const auto& member : entity->getDirectMembers())
        produceType(getBaseUnoName(member.type));

    CSharpFile file(m_outputDir, name);

    // verbose and dry run checks
    if (m_verbose)
        std::cout << "[exception] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    // output namespace block
    auto[hasNamespace, namespaceName, typeName] = splitName(name);

    if (hasNamespace)
        file.beginLine().append("namespace ").append(namespaceName).endLine().beginBlock();

    // generate exception and base exceptions list
    file.beginLine()
        .append("[com.sun.star.uno.UnoGenerated]")
        .endLine()
        .beginLine()
        .append("public class ")
        .append(getSafeIdentifier(typeName));
    if (!base.isEmpty())
        file.append(" : ").append(getNetName(base));
    file.endLine().beginBlock();

    // generate default constructor
    file.beginLine()
        .append("public ")
        .append(getSafeIdentifier(typeName))
        .append("()")
        .endLine()
        .beginBlock()
        .endBlock();
    file.endLine(); // extra blank line

    // generate full constructor
    std::vector<unoidl::ExceptionTypeEntity::Member> baseFields;
    {
        OUString baseTypeName = base;
        while (!baseTypeName.isEmpty())
        {
            rtl::Reference<unoidl::Entity> baseEntity;
            if (m_manager->getSort(baseTypeName, &baseEntity)
                != codemaker::UnoType::Sort::Exception)
                throw CannotDumpException("'" + b2u(name) + "' base type '" + baseTypeName
                                          + "' is not an exception");

            rtl::Reference<unoidl::ExceptionTypeEntity> ref(
                dynamic_cast<unoidl::ExceptionTypeEntity*>(baseEntity.get()));
            baseFields.insert(baseFields.begin(), ref->getDirectMembers().begin(),
                              ref->getDirectMembers().end());

            baseTypeName = ref->getDirectBase();
        }
    }

    std::vector<unoidl::ExceptionTypeEntity::Member> allFields(entity->getDirectMembers());
    allFields.insert(allFields.begin(), baseFields.begin(), baseFields.end());

    file.beginLine().append("public ").append(getSafeIdentifier(typeName)).append("(");
    separatedForeach(
        allFields, [&file]() { file.append(", "); },
        [this, &file](const auto& member) {
            file.append(getNetName(member.type)).append(" ").append(getSafeIdentifier(member.name));
        });
    file.append(")").endLine();
    file.beginLine().extraIndent().append(": base(");
    separatedForeach(baseFields, [&file]() { file.append(", "); },
                     [&file](const auto& member) { file.append(getSafeIdentifier(member.name)); });
    file.append(")").endLine();
    file.beginBlock();
    for (const auto& member : entity->getDirectMembers())
    {
        file.beginLine()
            .append("this.")
            .append(getSafeIdentifier(member.name))
            .append(" = ")
            .append(getSafeIdentifier(member.name))
            .append(";")
            .endLine();
    }
    file.endBlock();
    file.endLine(); // extra blank line

    // generate exception fields
    for (const auto& member : entity->getDirectMembers())
    {
        for (const auto& anno : member.annotations)
            if (anno == "deprecated")
                file.beginLine().append("[System.Obsolete]").endLine();
        file.beginLine()
            .append("public ")
            .append(getNetName(member.type))
            .append(" ")
            .append(getSafeIdentifier(member.name))
            .append(";")
            .endLine();
    }

    file.endBlock();
    if (hasNamespace)
        file.endBlock();
    file.closeFile();
}

void NetProducer::produceInterface(std::string_view name,
                                   const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    // produce referenced types
    for (const auto& base : entity->getDirectMandatoryBases())
        produceType(getBaseUnoName(base.name));
    for (const auto& member : entity->getDirectAttributes())
    {
        produceType(getBaseUnoName(member.type));
        for (const auto& e : member.getExceptions)
            produceType(getBaseUnoName(e));
        for (const auto& e : member.setExceptions)
            produceType(getBaseUnoName(e));
    }
    for (const auto& member : entity->getDirectMethods())
    {
        produceType(getBaseUnoName(member.returnType));
        for (const auto& e : member.exceptions)
            produceType(getBaseUnoName(e));
        for (const auto& p : member.parameters)
            produceType(getBaseUnoName(p.type));
    }

    CSharpFile file(m_outputDir, name);

    // verbose and dry run checks
    if (m_verbose)
        std::cout << "[interface] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    // output namespace block
    auto[hasNamespace, namespaceName, typeName] = splitName(name);

    if (hasNamespace)
        file.beginLine().append("namespace ").append(namespaceName).endLine().beginBlock();

    // generate interface and base interfaces list
    file.beginLine()
        .append("[com.sun.star.uno.UnoGenerated]")
        .endLine()
        .beginLine()
        .append("public interface ")
        .append(getSafeIdentifier(typeName));
    const auto& bases = entity->getDirectMandatoryBases();
    if (!bases.empty())
    {
        file.append(" : ");
        separatedForeach(bases, [&file]() { file.append(", "); },
                         [this, &file](const auto& b) { file.append(getNetName(b.name)); });
    }
    file.endLine().beginBlock();

    // generate interface properties
    for (const auto& member : entity->getDirectAttributes())
    {
        for (const auto& anno : member.annotations)
            if (anno == "deprecated")
                file.beginLine().append("[System.Obsolete]").endLine();
        if (member.bound)
            file.beginLine().append("[com.sun.star.uno.Bound]").endLine();

        file.beginLine()
            .append(getNetName(member.type))
            .append(" ")
            .append(getSafeIdentifier(member.name))
            .endLine()
            .beginBlock();

        if (!member.getExceptions.empty())
        {
            file.beginLine().append("[com.sun.star.uno.Raises(");
            separatedForeach(member.getExceptions, [&file]() { file.append(", "); },
                             [this, &file](const auto& e) {
                                 file.append("typeof(").append(getNetName(e)).append(")");
                             });
            file.append(")]").endLine();
        }
        file.beginLine().append("get;").endLine();

        if (!member.readOnly)
        {
            if (!member.setExceptions.empty())
            {
                file.beginLine().append("[com.sun.star.uno.Raises(");
                separatedForeach(member.setExceptions, [&file]() { file.append(", "); },
                                 [this, &file](const auto& e) {
                                     file.append("typeof(").append(getNetName(e)).append(")");
                                 });
                file.append(")]").endLine();
            }

            file.beginLine().append("set;").endLine();
        }
        file.endBlock();
    }
    if (!entity->getDirectAttributes().empty())
        file.endLine(); // extra blank line

    // generate interface methods
    for (const auto& member : entity->getDirectMethods())
    {
        for (const auto& anno : member.annotations)
            if (anno == "deprecated")
                file.beginLine().append("[System.Obsolete]").endLine();
        if (!member.exceptions.empty())
        {
            file.beginLine().append("[com.sun.star.uno.Raises(");
            separatedForeach(member.exceptions, [&file]() { file.append(", "); },
                             [this, &file](const auto& e) {
                                 file.append("typeof(").append(getNetName(e)).append(")");
                             });
            file.append(")]").endLine();
        }
        file.beginLine()
            .append(getNetName(member.returnType))
            .append(" ")
            .append(getSafeIdentifier(member.name))
            .append("(");
        separatedForeach(
            member.parameters, [&file]() { file.append(", "); },
            [this, &file](const auto& p) {
                using Dir = unoidl::InterfaceTypeEntity::Method::Parameter::Direction;
                switch (p.direction)
                {
                    case Dir::DIRECTION_IN:
                        break;
                    case Dir::DIRECTION_OUT:
                        file.append("out ");
                        break;
                    case Dir::DIRECTION_IN_OUT:
                        file.append("ref ");
                        break;
                }
                file.append(getNetName(p.type)).append(" ").append(getSafeIdentifier(p.name));
            });
        file.append(");").endLine();
    }

    file.endBlock();
    if (hasNamespace)
        file.endBlock();
    file.closeFile();
}

void NetProducer::produceTypedef(std::string_view name,
                                 const rtl::Reference<unoidl::TypedefEntity>& entity)
{
    OString type = u2b(entity->getType());

    produceType(getBaseUnoName(type));
    m_typedefs.emplace(name, type);

    if (m_verbose)
        std::cout << "[typedef] " << name << " = " << type << '\n';
}

void NetProducer::produceConstantGroup(std::string_view name,
                                       const rtl::Reference<unoidl::ConstantGroupEntity>& entity)
{
    CSharpFile file(m_outputDir, name);

    if (m_verbose)
        std::cout << "[constants] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    auto[hasNamespace, namespaceName, typeName] = splitName(name);

    if (hasNamespace)
        file.beginLine().append("namespace ").append(namespaceName).endLine().beginBlock();

    file.beginLine()
        .append("[com.sun.star.uno.UnoGenerated]")
        .endLine()
        .beginLine()
        .append("public static class ")
        .append(getSafeIdentifier(typeName))
        .endLine()
        .beginBlock();
    for (const auto& member : entity->getMembers())
    {
        for (const auto& anno : member.annotations)
            if (anno == "deprecated")
                file.beginLine().append("[System.Obsolete]").endLine();
        OString type, value;
        switch (member.value.type)
        {
            case unoidl::ConstantValue::TYPE_BOOLEAN:
                type = "bool"_ostr;
                value = member.value.booleanValue ? "true"_ostr : "false"_ostr;
                break;
            case unoidl::ConstantValue::TYPE_BYTE:
                type = "sbyte"_ostr;
                value = OString::number(member.value.byteValue);
                break;
            case unoidl::ConstantValue::TYPE_SHORT:
                type = "short"_ostr;
                value = OString::number(member.value.shortValue);
                break;
            case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
                type = "ushort"_ostr;
                value = OString::number(member.value.unsignedShortValue);
                break;
            case unoidl::ConstantValue::TYPE_LONG:
                type = "int"_ostr;
                value = OString::number(member.value.longValue);
                break;
            case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
                type = "uint"_ostr;
                value = OString::number(member.value.unsignedLongValue);
                break;
            case unoidl::ConstantValue::TYPE_HYPER:
                type = "long"_ostr;
                value = OString::number(member.value.hyperValue);
                break;
            case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
                type = "ulong"_ostr;
                value = OString::number(member.value.unsignedHyperValue);
                break;
            case unoidl::ConstantValue::TYPE_FLOAT:
                type = "float"_ostr;
                value = OString::number(member.value.floatValue);
                break;
            case unoidl::ConstantValue::TYPE_DOUBLE:
                type = "double"_ostr;
                value = OString::number(member.value.doubleValue);
                break;
        }
        file.beginLine()
            .append("public const ")
            .append(type)
            .append(" ")
            .append(getSafeIdentifier(member.name))
            .append(" = ")
            .append(value)
            .append(";")
            .endLine();
    }
    file.endBlock();
    if (hasNamespace)
        file.endBlock();
    file.closeFile();
}

void NetProducer::produceService(
    std::string_view name, const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity)
{
    CSharpFile file(m_outputDir, name);

    if (m_verbose)
        std::cout << "[service] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    auto[hasNamespace, namespaceName, typeName] = splitName(name);

    if (hasNamespace)
        file.beginLine().append("namespace ").append(namespaceName).endLine().beginBlock();

    file.beginLine()
        .append("[com.sun.star.uno.UnoGenerated]")
        .endLine()
        .beginLine()
        .append("public static class ")
        .append(getSafeIdentifier(typeName))
        .endLine()
        .beginBlock();

    for (const auto& ctor : entity->getConstructors())
    {
        for (const auto& anno : ctor.annotations)
            if (anno == "deprecated")
                file.beginLine().append("[System.Obsolete]").endLine();

        std::vector<OUString> exceptions(ctor.exceptions);
        exceptions.emplace(exceptions.begin(), "com.sun.star.uno.DeploymentException");

        file.beginLine().append("[com.sun.star.uno.Raises(");
        separatedForeach(exceptions, [&file]() { file.append(", "); },
                         [this, &file](const auto& e) {
                             file.append("typeof(").append(getNetName(e)).append(")");
                         });
        file.append(")]").endLine();

        if (ctor.defaultConstructor)
        {
            const auto& returnType(getNetName(entity->getBase()));

            file.beginLine()
                .append("public static ")
                .append(returnType)
                .append(" create(com.sun.star.uno.XComponentContext ctx)")
                .endLine()
                .beginBlock();

            file.beginLine()
                .append("try")
                .endLine()
                .beginBlock()
                .beginLine()
                .append("com.sun.star.lang.XMultiComponentFactory mcf = ")
                .append("ctx.getServiceManager();")
                .endLine()
                .beginLine()
                .append(returnType)
                .append(" srv = (")
                .append(returnType)
                .append(")mcf.createInstanceWithContext(\"")
                .append(name)
                .append("\", ctx);")
                .endLine()
                .beginLine()
                .append("return srv;")
                .endLine()
                .endBlock();

            for (const auto& e : ctor.exceptions)
            {
                file.beginLine()
                    .append("catch (")
                    .append(e)
                    .append(")")
                    .endLine()
                    .beginBlock()
                    .beginLine()
                    .append("throw;")
                    .endLine()
                    .endBlock();
            }

            file.beginLine()
                .append("catch")
                .endLine()
                .beginBlock()
                .beginLine()
                .append(
                    "throw new com.sun.star.uno.DeploymentException(\"Could not create service ")
                .append(name)
                .append(" from given XComponentContext\", ctx);")
                .endLine()
                .endBlock();

            file.endBlock();
        }
        else
        {
            const auto& returnType(getNetName(entity->getBase()));
            const auto* restParam = !ctor.parameters.empty() && ctor.parameters.front().rest
                                        ? &ctor.parameters.front()
                                        : nullptr;

            file.beginLine()
                .append("public static ")
                .append(returnType)
                .append(" ")
                .append(getSafeIdentifier(ctor.name))
                .append("(com.sun.star.uno.XComponentContext ctx");
            if (!ctor.parameters.empty())
                file.append(", ");
            separatedForeach(
                ctor.parameters, [&file]() { file.append(", "); },
                [this, &file](const auto& p) {
                    file.append(getNetName(p.type)).append(" ").append(getSafeIdentifier(p.name));
                });
            file.append(")").endLine().beginBlock();

            file.beginLine()
                .append("try")
                .endLine()
                .beginBlock()
                .beginLine()
                .append("com.sun.star.lang.XMultiComponentFactory mcf = ")
                .append("ctx.getServiceManager();")
                .endLine()
                .beginLine()
                .append(returnType)
                .append(" srv = (")
                .append(returnType)
                .append(")mcf.createInstanceWithArgumentsAndContext(\"")
                .append(name)
                .append("\", ");
            if (restParam)
            {
                file.append(getSafeIdentifier(ctor.parameters.front().name));
            }
            else if (ctor.parameters.empty())
            {
                file.append("System.Array.Empty<com.sun.star.uno.Any>()");
            }
            else
            {
                file.append("new com.sun.star.uno.Any[] { ");
                separatedForeach(ctor.parameters, [&file]() { file.append(", "); },
                                 [&file](const auto& p) {
                                     file.append("new com.sun.star.uno.Any(")
                                         .append(getSafeIdentifier(p.name))
                                         .append(")");
                                 });
                file.append(" }");
            }
            file.append(", ctx);").endLine().beginLine().append("return srv;").endLine().endBlock();

            for (const auto& e : ctor.exceptions)
            {
                file.beginLine()
                    .append("catch (")
                    .append(getNetName(e))
                    .append(")")
                    .endLine()
                    .beginBlock()
                    .beginLine()
                    .append("throw;")
                    .endLine()
                    .endBlock();
            }

            file.beginLine()
                .append("catch")
                .endLine()
                .beginBlock()
                .beginLine()
                .append(
                    "throw new com.sun.star.uno.DeploymentException(\"Could not create service ")
                .append(name)
                .append(" from given XComponentContext\", ctx);")
                .endLine()
                .endBlock();

            file.endBlock();
        }
    }

    file.endBlock();
    if (hasNamespace)
        file.endBlock();
    file.closeFile();
}

void NetProducer::produceSingleton(
    std::string_view name, const rtl::Reference<unoidl::InterfaceBasedSingletonEntity>& entity)
{
    CSharpFile file(m_outputDir, name);

    if (m_verbose)
        std::cout << "[singleton] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    auto[hasNamespace, namespaceName, typeName] = splitName(name);

    if (hasNamespace)
        file.beginLine().append("namespace ").append(namespaceName).endLine().beginBlock();

    file.beginLine()
        .append("[com.sun.star.uno.UnoGenerated]")
        .endLine()
        .beginLine()
        .append("public static class ")
        .append(getSafeIdentifier(typeName))
        .endLine()
        .beginBlock();

    file.beginLine()
        .append("[com.sun.star.uno.Raises(typeof(com.sun.star.uno.DeploymentException))]")
        .endLine();
    file.beginLine()
        .append("public static ")
        .append(getNetName(entity->getBase()))
        .append(" get(com.sun.star.uno.XComponentContext ctx)")
        .endLine()
        .beginBlock();

    file.beginLine()
        .append("com.sun.star.uno.Any sgtn = ctx.getValueByName(\"/singletons/")
        .append(name)
        .append("\");")
        .endLine();
    file.beginLine()
        .append("if (!sgtn.hasValue())")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("throw new com.sun.star.uno.DeploymentException(\"Could not get singleton ")
        .append(name)
        .append(" from given XComponentContext\", ctx);")
        .endLine()
        .endBlock();
    file.beginLine()
        .append("return (")
        .append(getNetName(entity->getBase()))
        .append(")sgtn.Value;")
        .endLine();

    file.endBlock().endBlock();

    if (hasNamespace)
        file.endBlock();

    file.closeFile();
}

OString NetProducer::getNetName(std::string_view name)
{
    OString fullName(name);
    OStringBuffer buffer;

    while (true)
    {
        OString baseName = getBaseUnoName(fullName);
        if (m_typedefs.contains(baseName))
            fullName = fullName.replaceFirst(baseName, m_typedefs.at(baseName));
        else
            break;
    }

    std::string_view fullNameView(fullName);

    // if sequence, count dimensions
    int dimensions = 0;
    while (fullNameView.starts_with("[]"))
    {
        ++dimensions;
        fullNameView = fullNameView.substr(2);
    }

    // if polymorphic, process parameters too
    if (fullNameView.ends_with('>'))
    {
        size_t start = fullNameView.find_first_of('<') + 1;
        size_t end = fullNameView.size() - 1;
        buffer.append(fullNameView.substr(0, start));
        OString params(fullNameView.substr(start, end - start));

        bool first = true;
        for (start = 0; start != std::string_view::npos;)
        {
            std::string_view param(o3tl::getToken(params, ',', start));
            if (first)
                first = false;
            else
                buffer.append(", ");
            buffer.append(getNetName(param));
        }
        buffer.append(">");
    }
    else
    {
        // assumes basetypes are not polymorphic for a tiny optimization
        // if this is changed later, move this part out of the else block
        fullName = OString(fullNameView);
        OString baseName = getBaseUnoName(fullName);
        if (s_baseTypes.contains(baseName))
            buffer.append(fullName.replaceFirst(baseName, s_baseTypes.at(baseName)));
        else
            buffer.append(fullName);
    }

    // if sequence, add [] to make array
    while (dimensions--)
        buffer.append("[]");

    return buffer.makeStringAndClear();
}
OString NetProducer::getNetName(std::u16string_view name) { return getNetName(u2b(name)); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
