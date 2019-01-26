/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/process.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/main.h>
#include <sal/types.h>
#include <unoidl/unoidl.hxx>

namespace {

void badUsage() {
    std::cerr
        << "Usage:" << std::endl << std::endl
        << "  unoidl-read [--published] [<extra registries>] <registry>"
        << std::endl << std::endl
        << ("where each <registry> is either a new- or legacy-format .rdb file,"
            " a single .idl")
        << std::endl
        << ("file, or a root directory of an .idl file tree.  The complete"
            " content of the")
        << std::endl
        << ("last <registry> is written to stdout; if --published is specified,"
            " only the")
        << std::endl
        << ("published entities (plus any non-published entities referenced"
            " from published")
        << std::endl
        << "via any unpublished optional bases) are written out." << std::endl;
    std::exit(EXIT_FAILURE);
}

OUString getArgumentUri(sal_uInt32 argument) {
    OUString arg;
    rtl_getAppCommandArg(argument, &arg.pData);
    OUString url;
    osl::FileBase::RC e1 = osl::FileBase::getFileURLFromSystemPath(arg, url);
    if (e1 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot convert \"" << arg << "\" to file URL, error code "
            << +e1 << std::endl;
        std::exit(EXIT_FAILURE);
    }
    OUString cwd;
    oslProcessError e2 = osl_getProcessWorkingDir(&cwd.pData);
    if (e2 != osl_Process_E_None) {
        std::cerr
            << "Cannot obtain working directory, error code " << +e2
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
    OUString abs;
    e1 = osl::FileBase::getAbsoluteFileURL(cwd, url, abs);
    if (e1 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot make \"" << url
            << "\" into an absolute file URL, error code " << +e1 << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return abs;
}

OUString decomposeType(
    OUString const & type, std::size_t * rank,
    std::vector<OUString> * typeArguments, bool * entity)
{
    assert(rank != nullptr);
    assert(typeArguments != nullptr);
    assert(entity != nullptr);
    OUString nucl(type);
    *rank = 0;
    typeArguments->clear();
    while (nucl.startsWith("[]", &nucl)) {
        ++*rank;
    }
    sal_Int32 i = nucl.indexOf('<');
    if (i != -1) {
        OUString tmpl(nucl.copy(0, i));
        do {
            ++i; // skip '<' or ','
            sal_Int32 j = i;
            for (sal_Int32 level = 0; j != nucl.getLength(); ++j) {
                sal_Unicode c = nucl[j];
                if (c == ',') {
                    if (level == 0) {
                        break;
                    }
                } else if (c == '<') {
                    ++level;
                } else if (c == '>') {
                    if (level == 0) {
                        break;
                    }
                    --level;
                }
            }
            if (j != nucl.getLength()) {
                typeArguments->push_back(nucl.copy(i, j - i));
            }
            i = j;
        } while (i != nucl.getLength() && nucl[i] != '>');
        assert(i == nucl.getLength() - 1 && nucl[i] == '>');
        assert(!typeArguments->empty());
        nucl = tmpl;
    }
    assert(!nucl.isEmpty());
    *entity = nucl != "void" && nucl != "boolean" && nucl != "byte"
        && nucl != "short" && nucl != "unsigned short" && nucl != "long"
        && nucl != "unsigned long" && nucl != "hyper"
        && nucl != "unsigned hyper" && nucl != "float" && nucl != "double"
        && nucl != "char" && nucl != "string" && nucl != "type"
        && nucl != "any";
    assert(*entity || typeArguments->empty());
    return nucl;
}

struct Entity {
    enum class Sorted { NO, ACTIVE, YES };

    explicit Entity(
        rtl::Reference<unoidl::Entity> const & theEntity, bool theRelevant):
        entity(theEntity), relevant(theRelevant), sorted(Sorted::NO),
        written(false)
    {}

    rtl::Reference<unoidl::Entity> const entity;
    std::set<OUString> dependencies;
    std::set<OUString> interfaceDependencies;
    bool relevant;
    Sorted sorted;
    bool written;
};

void insertEntityDependency(
    rtl::Reference<unoidl::Manager> const & manager,
    std::map<OUString, Entity>::iterator const & iterator,
    OUString const & name, bool weakInterfaceDependency = false)
{
    assert(manager.is());
    if (name != iterator->first) {
        bool ifc = false;
        if (weakInterfaceDependency) {
            rtl::Reference<unoidl::Entity> ent(manager->findEntity(name));
            if (!ent.is()) {
                std::cerr << "Unknown entity " << name << std::endl;
                std::exit(EXIT_FAILURE);
            }
            ifc = ent->getSort() == unoidl::Entity::SORT_INTERFACE_TYPE;
        }
        (ifc
         ? iterator->second.interfaceDependencies
         : iterator->second.dependencies)
            .insert(name);
    }
}

void insertEntityDependencies(
    rtl::Reference<unoidl::Manager> const & manager,
    std::map<OUString, Entity>::iterator const & iterator,
    std::vector<OUString> const & names)
{
    for (auto & i: names) {
        insertEntityDependency(manager, iterator, i);
    }
}

void insertEntityDependencies(
    rtl::Reference<unoidl::Manager> const & manager,
    std::map<OUString, Entity>::iterator const & iterator,
    std::vector<unoidl::AnnotatedReference> const & references)
{
    for (auto & i: references) {
        insertEntityDependency(manager, iterator, i.name);
    }
}

void insertTypeDependency(
    rtl::Reference<unoidl::Manager> const & manager,
    std::map<OUString, Entity>::iterator const & iterator,
    OUString const & type)
{
    std::size_t rank;
    std::vector<OUString> args;
    bool entity;
    OUString nucl(decomposeType(type, &rank, &args, &entity));
    if (entity) {
        insertEntityDependency(manager, iterator, nucl, true);
        for (auto & i: args) {
            insertTypeDependency(manager, iterator, i);
        }
    }
}

void scanMap(
    rtl::Reference<unoidl::Manager> const & manager,
    rtl::Reference<unoidl::MapCursor> const & cursor, bool published,
    OUString const & prefix, std::map<OUString, Entity> & entities)
{
    assert(cursor.is());
    for (;;) {
        OUString id;
        rtl::Reference<unoidl::Entity> ent(cursor->getNext(&id));
        if (!ent.is()) {
            break;
        }
        OUString name(prefix + id);
        if (ent->getSort() == unoidl::Entity::SORT_MODULE) {
            scanMap(
                manager,
                static_cast<unoidl::ModuleEntity *>(ent.get())->createCursor(),
                published, name + ".", entities);
        } else {
            std::map<OUString, Entity>::iterator i(
                entities.insert(
                    std::make_pair(
                        name,
                        Entity(
                            ent,
                            (!published
                             || (static_cast<unoidl::PublishableEntity *>(
                                     ent.get())
                                 ->isPublished())))))
                .first);
            switch (ent->getSort()) {
            case unoidl::Entity::SORT_ENUM_TYPE:
            case unoidl::Entity::SORT_CONSTANT_GROUP:
                break;
            case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
                {
                    rtl::Reference<unoidl::PlainStructTypeEntity> ent2(
                        static_cast<unoidl::PlainStructTypeEntity *>(
                            ent.get()));
                    if (!ent2->getDirectBase().isEmpty()) {
                        insertEntityDependency(
                            manager, i, ent2->getDirectBase());
                    }
                    for (auto & j: ent2->getDirectMembers()) {
                        insertTypeDependency(manager, i, j.type);
                    }
                    break;
                }
            case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
                {
                    rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity>
                        ent2(
                            static_cast<unoidl::PolymorphicStructTypeTemplateEntity *>(
                                ent.get()));
                    for (auto & j: ent2->getMembers()) {
                        if (!j.parameterized) {
                            insertTypeDependency(manager, i, j.type);
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_EXCEPTION_TYPE:
                {
                    rtl::Reference<unoidl::ExceptionTypeEntity> ent2(
                        static_cast<unoidl::ExceptionTypeEntity *>(ent.get()));
                    if (!ent2->getDirectBase().isEmpty()) {
                        insertEntityDependency(
                            manager, i, ent2->getDirectBase());
                    }
                    for (auto & j: ent2->getDirectMembers()) {
                        insertTypeDependency(manager, i, j.type);
                    }
                    break;
                }
            case unoidl::Entity::SORT_INTERFACE_TYPE:
                {
                    rtl::Reference<unoidl::InterfaceTypeEntity> ent2(
                        static_cast<unoidl::InterfaceTypeEntity *>(
                            ent.get()));
                    insertEntityDependencies(
                        manager, i, ent2->getDirectMandatoryBases());
                    insertEntityDependencies(
                        manager, i, ent2->getDirectOptionalBases());
                    for (auto & j: ent2->getDirectAttributes()) {
                        insertTypeDependency(manager, i, j.type);
                    }
                    for (auto & j: ent2->getDirectMethods()) {
                        insertTypeDependency(manager, i, j.returnType);
                        for (auto & k: j.parameters) {
                            insertTypeDependency(manager, i, k.type);
                        }
                        insertEntityDependencies(manager, i, j.exceptions);
                    }
                    break;
                }
            case unoidl::Entity::SORT_TYPEDEF:
                {
                    rtl::Reference<unoidl::TypedefEntity> ent2(
                        static_cast<unoidl::TypedefEntity *>(ent.get()));
                    insertTypeDependency(manager, i, ent2->getType());
                    break;
                }
            case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
                {
                    rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>
                        ent2(
                            static_cast<unoidl::SingleInterfaceBasedServiceEntity *>(
                                ent.get()));
                    insertEntityDependency(manager, i, ent2->getBase());
                    for (auto & j: ent2->getConstructors()) {
                        for (auto & k: j.parameters) {
                            insertTypeDependency(manager, i, k.type);
                        }
                        insertEntityDependencies(manager, i, j.exceptions);
                    }
                    break;
                }
            case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
                {
                    rtl::Reference<unoidl::AccumulationBasedServiceEntity> ent2(
                        static_cast<unoidl::AccumulationBasedServiceEntity *>(
                            ent.get()));
                    insertEntityDependencies(
                        manager, i, ent2->getDirectMandatoryBaseServices());
                    insertEntityDependencies(
                        manager, i, ent2->getDirectOptionalBaseServices());
                    insertEntityDependencies(
                        manager, i, ent2->getDirectMandatoryBaseInterfaces());
                    insertEntityDependencies(
                        manager, i, ent2->getDirectOptionalBaseInterfaces());
                    for (auto & j: ent2->getDirectProperties()) {
                        insertTypeDependency(manager, i, j.type);
                    }
                    break;
                }
            case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
                {
                    rtl::Reference<unoidl::InterfaceBasedSingletonEntity> ent2(
                        static_cast<unoidl::InterfaceBasedSingletonEntity *>(
                            ent.get()));
                    insertEntityDependency(manager, i, ent2->getBase());
                    break;
                }
            case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
                {
                    rtl::Reference<unoidl::ServiceBasedSingletonEntity> ent2(
                        static_cast<unoidl::ServiceBasedSingletonEntity *>(
                            ent.get()));
                    insertEntityDependency(manager, i, ent2->getBase());
                    break;
                }
            case unoidl::Entity::SORT_MODULE:
                assert(false && "this cannot happen");
            }
        }
    }
}

void propagateRelevant(std::map<OUString, Entity> & entities, Entity & entity) {
    if (!entity.relevant) {
        entity.relevant = true;
        if (entity.sorted != Entity::Sorted::YES) {
            for (auto & i: entity.dependencies) {
                std::map<OUString, Entity>::iterator j(entities.find(i));
                if (j != entities.end()) {
                    propagateRelevant(entities, j->second);
                }
            }
        }
    }
}

void visit(
    std::map<OUString, Entity> & entities,
    std::map<OUString, Entity>::iterator const & iterator,
    std::vector<OUString> & result)
{
    switch (iterator->second.sorted) {
    case Entity::Sorted::NO:
        iterator->second.sorted = Entity::Sorted::ACTIVE;
        for (auto & i: iterator->second.dependencies) {
            std::map<OUString, Entity>::iterator j(entities.find(i));
            if (j != entities.end()) {
                if (iterator->second.relevant) {
                    propagateRelevant(entities, j->second);
                }
                visit(entities, j, result);
            }
        }
        iterator->second.sorted = Entity::Sorted::YES;
        result.push_back(iterator->first);
        break;
    case Entity::Sorted::ACTIVE:
        std::cerr
            << "Entity " << iterator->first << " recursively depends on itself"
            << std::endl;
        std::exit(EXIT_FAILURE);
        // fall-through avoids warnings
    default:
        break;
    }
}

std::vector<OUString> sort(std::map<OUString, Entity> & entities) {
    std::vector<OUString> res;
    for (auto i(entities.begin()); i != entities.end(); ++i) {
        visit(entities, i, res);
    }
    return res;
}

void indent(std::vector<OUString> const & modules, unsigned int extra = 0) {
    for (std::vector<OUString>::size_type i = 0; i != modules.size(); ++i) {
        std::cout << ' ';
    }
    for (unsigned int i = 0; i != extra; ++i) {
        std::cout << ' ';
    }
}

void closeModules(
    std::vector<OUString> & modules, std::vector<OUString>::size_type n)
{
    for (std::vector<OUString>::size_type i = 0; i != n; ++i) {
        assert(!modules.empty());
        modules.pop_back();
        indent(modules);
        std::cout << "};\n";
    }
}

OUString openModulesFor(std::vector<OUString> & modules, OUString const & name)
{
    std::vector<OUString>::iterator i(modules.begin());
    for (sal_Int32 j = 0;;) {
        OUString id(name.getToken(0, '.', j));
        if (j == -1) {
            closeModules(
                modules,
                static_cast< std::vector<OUString>::size_type >(
                    modules.end() - i));
            indent(modules);
            return id;
        }
        if (i != modules.end()) {
            if (id == *i) {
                ++i;
                continue;
            }
            closeModules(
                modules,
                static_cast< std::vector<OUString>::size_type >(
                    modules.end() - i));
            i = modules.end();
        }
        indent(modules);
        std::cout << "module " << id << " {\n";
        modules.push_back(id);
        i = modules.end();
    }
}

void writeName(OUString const & name) {
    std::cout << "::" << name.replaceAll(".", "::");
}

void writeAnnotations(std::vector<OUString> const & annotations) {
    if (!annotations.empty()) {
        std::cout << "/**";
        for (auto & i: annotations) {
            //TODO: i.indexOf("*/") == -1
            std::cout << " @" << i;
        }
        std::cout << " */ ";
    }
}

void writePublished(rtl::Reference<unoidl::PublishableEntity> const & entity) {
    assert(entity.is());
    if (entity->isPublished()) {
        std::cout << "published ";
    }
}

void writeAnnotationsPublished(
    rtl::Reference<unoidl::PublishableEntity> const & entity)
{
    assert(entity.is());
    writeAnnotations(entity->getAnnotations());
    writePublished(entity);
}

void writeType(OUString const & type) {
    std::size_t rank;
    std::vector<OUString> args;
    bool entity;
    OUString nucl(decomposeType(type, &rank, &args, &entity));
    for (std::size_t i = 0; i != rank; ++i) {
        std::cout << "sequence< ";
    }
    if (entity) {
        writeName(nucl);
    } else {
        std::cout << nucl;
    }
    if (!args.empty()) {
        std::cout << "< ";
        for (auto i(args.begin()); i != args.end(); ++i) {
            if (i != args.begin()) {
                std::cout << ", ";
            }
            writeType(*i);
        }
        std::cout << " >";
    }
    for (std::size_t i = 0; i != rank; ++i) {
        std::cout << " >";
    }
}

void writeExceptionSpecification(std::vector<OUString> const & exceptions) {
    if (!exceptions.empty()) {
        std::cout << " raises (";
        for (auto i(exceptions.begin()); i != exceptions.end(); ++i) {
            if (i != exceptions.begin()) {
                std::cout << ", ";
            }
            writeName(*i);
        }
        std::cout << ')';
    }
}

void writeEntity(
    std::map<OUString, Entity> & entities, std::vector<OUString> & modules,
    OUString const & name)
{
    std::map<OUString, Entity>::iterator i(entities.find(name));
    if (i != entities.end() && i->second.relevant) {
        assert(!i->second.written);
        i->second.written = true;
        for (auto & j: i->second.interfaceDependencies) {
            std::map<OUString, Entity>::iterator k(entities.find(j));
            if (k != entities.end() && !k->second.written) {
                OUString id(openModulesFor(modules, j));
                if (k->second.entity->getSort()
                    != unoidl::Entity::SORT_INTERFACE_TYPE)
                {
                    std::cerr
                        << "Entity " << j << " should be an interface type"
                        << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                writePublished(
                    static_cast<unoidl::PublishableEntity *>(
                        k->second.entity.get()));
                std::cout << "interface " << id << ";\n";
            }
        }
        OUString id(openModulesFor(modules, name));
        rtl::Reference<unoidl::PublishableEntity> ent(
            static_cast<unoidl::PublishableEntity *>(i->second.entity.get()));
        switch (ent->getSort()) {
        case unoidl::Entity::SORT_ENUM_TYPE:
            {
                rtl::Reference<unoidl::EnumTypeEntity> ent2(
                    static_cast<unoidl::EnumTypeEntity *>(ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "enum " << id << " {\n";
                for (auto j(ent2->getMembers().begin());
                     j != ent2->getMembers().end(); ++j)
                {
                    indent(modules, 1);
                    writeAnnotations(j->annotations);
                    std::cout << j->name << " = " << j->value;
                    if (j + 1 != ent2->getMembers().end()) {
                        std::cout << ',';
                    }
                    std::cout << '\n';
                }
                indent(modules);
                std::cout << "};\n";
                break;
            }
        case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
            {
                rtl::Reference<unoidl::PlainStructTypeEntity> ent2(
                    static_cast<unoidl::PlainStructTypeEntity *>(ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "struct " << id;
                if (!ent2->getDirectBase().isEmpty()) {
                    std::cout << ": ";
                    writeName(ent2->getDirectBase());
                }
                std::cout << " {\n";
                for (auto & j: ent2->getDirectMembers()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    writeType(j.type);
                    std::cout << ' ' << j.name << ";\n";
                }
                indent(modules);
                std::cout << "};\n";
                break;
            }
        case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
            {
                rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity>
                    ent2(
                        static_cast<unoidl::PolymorphicStructTypeTemplateEntity *>(
                            ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "struct " << id << '<';
                for (auto j(ent2->getTypeParameters().begin());
                     j != ent2->getTypeParameters().end(); ++j)
                {
                    if (j != ent2->getTypeParameters().begin()) {
                        std::cout << ", ";
                    }
                    std::cout << *j;
                }
                std::cout << ">  {\n";
                for (auto & j: ent2->getMembers()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    if (j.parameterized) {
                        std::cout << j.type;
                    } else {
                        writeType(j.type);
                    }
                    std::cout << ' ' << j.name << ";\n";
                }
                indent(modules);
                std::cout << "};\n";
                break;
            }
        case unoidl::Entity::SORT_EXCEPTION_TYPE:
            {
                rtl::Reference<unoidl::ExceptionTypeEntity> ent2(
                    static_cast<unoidl::ExceptionTypeEntity *>(ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "exception " << id;
                if (!ent2->getDirectBase().isEmpty()) {
                    std::cout << ": ";
                    writeName(ent2->getDirectBase());
                }
                std::cout << " {\n";
                for (auto & j: ent2->getDirectMembers()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    writeType(j.type);
                    std::cout << ' ' << j.name << ";\n";
                }
                indent(modules);
                std::cout << "};\n";
                break;
            }
        case unoidl::Entity::SORT_INTERFACE_TYPE:
            {
                rtl::Reference<unoidl::InterfaceTypeEntity> ent2(
                    static_cast<unoidl::InterfaceTypeEntity *>(
                        ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "interface " << id << " {\n";
                for (auto & j: ent2->getDirectMandatoryBases()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "interface ";
                    writeName(j.name);
                    std::cout << ";\n";
                }
                for (auto & j: ent2->getDirectOptionalBases()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "[optional] interface ";
                    writeName(j.name);
                    std::cout << ";\n";
                }
                for (auto & j: ent2->getDirectAttributes()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "[attribute";
                    if (j.bound) {
                        std::cout << ", bound";
                    }
                    if (j.readOnly) {
                        std::cout << ", readonly";
                    }
                    std::cout << "] ";
                    writeType(j.type);
                    std::cout << ' ' << j.name;
                    if (!(j.getExceptions.empty() && j.setExceptions.empty())) {
                        std::cout << " {\n";
                        if (!j.getExceptions.empty()) {
                            indent(modules, 2);
                            std::cout << "get";
                            writeExceptionSpecification(j.getExceptions);
                            std::cout << ";\n";
                        }
                        if (!j.setExceptions.empty()) {
                            indent(modules, 2);
                            std::cout << "set";
                            writeExceptionSpecification(j.setExceptions);
                            std::cout << ";\n";
                        }
                        std::cout << " }";
                    }
                    std::cout << ";\n";
                }
                for (auto & j: ent2->getDirectMethods()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    writeType(j.returnType);
                    std::cout << ' ' << j.name << '(';
                    for (auto k(j.parameters.begin()); k != j.parameters.end();
                         ++k)
                    {
                        if (k != j.parameters.begin()) {
                            std::cout << ", ";
                        }
                        switch (k->direction) {
                        case unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN:
                            std::cout << "[in] ";
                            break;
                        case unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_OUT:
                            std::cout << "[out] ";
                            break;
                        case unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN_OUT:
                            std::cout << "[inout] ";
                            break;
                        }
                        writeType(k->type);
                        std::cout << ' ' << k->name;
                    }
                    std::cout << ')';
                    writeExceptionSpecification(j.exceptions);
                    std::cout << ";\n";
                }
                indent(modules);
                std::cout << "};\n";
                break;
            }
        case unoidl::Entity::SORT_TYPEDEF:
            {
                rtl::Reference<unoidl::TypedefEntity> ent2(
                    static_cast<unoidl::TypedefEntity *>(ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "typedef ";
                writeType(ent2->getType());
                std::cout << ' ' << id << ";\n";
                break;
            }
        case unoidl::Entity::SORT_CONSTANT_GROUP:
            {
                rtl::Reference<unoidl::ConstantGroupEntity> ent2(
                    static_cast<unoidl::ConstantGroupEntity *>(ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "constants " << id << " {\n";
                for (auto & j: ent2->getMembers()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "const ";
                    switch (j.value.type) {
                    case unoidl::ConstantValue::TYPE_BOOLEAN:
                        std::cout << "boolean";
                        break;
                    case unoidl::ConstantValue::TYPE_BYTE:
                        std::cout << "byte";
                        break;
                    case unoidl::ConstantValue::TYPE_SHORT:
                        std::cout << "short";
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
                        std::cout << "unsigned short";
                        break;
                    case unoidl::ConstantValue::TYPE_LONG:
                        std::cout << "long";
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
                        std::cout << "unsigned long";
                        break;
                    case unoidl::ConstantValue::TYPE_HYPER:
                        std::cout << "hyper";
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
                        std::cout << "unsigned hyper";
                        break;
                    case unoidl::ConstantValue::TYPE_FLOAT:
                        std::cout << "float";
                        break;
                    case unoidl::ConstantValue::TYPE_DOUBLE:
                        std::cout << "double";
                        break;
                    }
                    std::cout << ' ' << j.name << " = ";
                    switch (j.value.type) {
                    case unoidl::ConstantValue::TYPE_BOOLEAN:
                        std::cout << (j.value.booleanValue ? "TRUE" : "FALSE");
                        break;
                    case unoidl::ConstantValue::TYPE_BYTE:
                        std::cout << int(j.value.byteValue);
                        break;
                    case unoidl::ConstantValue::TYPE_SHORT:
                        std::cout << j.value.shortValue;
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
                        std::cout << j.value.unsignedShortValue;
                        break;
                    case unoidl::ConstantValue::TYPE_LONG:
                        std::cout << j.value.longValue;
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
                        std::cout << j.value.unsignedLongValue;
                        break;
                    case unoidl::ConstantValue::TYPE_HYPER:
                        std::cout << j.value.hyperValue;
                        break;
                    case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
                        std::cout << j.value.unsignedHyperValue;
                        break;
                    case unoidl::ConstantValue::TYPE_FLOAT:
                        std::cout << j.value.floatValue;
                        break;
                    case unoidl::ConstantValue::TYPE_DOUBLE:
                        std::cout << j.value.doubleValue;
                        break;
                    }
                    std::cout << ";\n";
                }
                indent(modules);
                std::cout << "};\n";
                break;
            }
        case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
            {
                rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity> ent2(
                    static_cast<unoidl::SingleInterfaceBasedServiceEntity *>(
                        ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "service " << id << ": ";
                writeName(ent2->getBase());
                if (ent2->getConstructors().size() != 1
                    || !ent2->getConstructors().front().defaultConstructor)
                {
                    std::cout << " {\n";
                    for (auto & j: ent2->getConstructors()) {
                        indent(modules, 1);
                        writeAnnotations(j.annotations);
                        std::cout << j.name << '(';
                        for (auto k(j.parameters.begin());
                             k != j.parameters.end(); ++k)
                        {
                            if (k != j.parameters.begin()) {
                                std::cout << ", ";
                            }
                            std::cout << "[in] ";
                            writeType(k->type);
                            if (k->rest) {
                                std::cout << "...";
                            }
                            std::cout << ' ' << k->name;
                        }
                        std::cout << ')';
                        writeExceptionSpecification(j.exceptions);
                        std::cout << ";\n";
                    }
                    indent(modules);
                    std::cout << '}';
                }
                std::cout << ";\n";
                break;
            }
        case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
            {
                rtl::Reference<unoidl::AccumulationBasedServiceEntity> ent2(
                    static_cast<unoidl::AccumulationBasedServiceEntity *>(
                        ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "service " << id << " {\n";
                for (auto & j: ent2->getDirectMandatoryBaseServices()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "service ";
                    writeName(j.name);
                    std::cout << ";\n";
                }
                for (auto & j: ent2->getDirectOptionalBaseServices()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "[optional] service ";
                    writeName(j.name);
                    std::cout << ";\n";
                }
                for (auto & j: ent2->getDirectMandatoryBaseInterfaces()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "interface ";
                    writeName(j.name);
                    std::cout << ";\n";
                }
                for (auto & j: ent2->getDirectOptionalBaseInterfaces()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "[optional] interface ";
                    writeName(j.name);
                    std::cout << ";\n";
                }
                for (auto & j: ent2->getDirectProperties()) {
                    indent(modules, 1);
                    writeAnnotations(j.annotations);
                    std::cout << "[property";
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_BOUND)
                        != 0)
                    {
                        std::cout << ", bound";
                    }
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_CONSTRAINED)
                        != 0)
                    {
                        std::cout << ", constrained";
                    }
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_MAYBE_AMBIGUOUS)
                        != 0)
                    {
                        std::cout << ", maybeambiguous";
                    }
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_MAYBE_DEFAULT)
                        != 0)
                    {
                        std::cout << ", maybedefault";
                    }
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_MAYBE_VOID)
                        != 0)
                    {
                        std::cout << ", maybevoid";
                    }
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_OPTIONAL)
                        != 0)
                    {
                        std::cout << ", optional";
                    }
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_READ_ONLY)
                        != 0)
                    {
                        std::cout << ", readonly";
                    }
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_REMOVABLE)
                        != 0)
                    {
                        std::cout << ", removable";
                    }
                    if ((j.attributes
                         & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_TRANSIENT)
                        != 0)
                    {
                        std::cout << ", transient";
                    }
                    std::cout << "] ";
                    writeType(j.type);
                    std::cout << ' ' << j.name << ";\n";
                }
                indent(modules);
                std::cout << "};\n";
                break;
            }
        case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
            {
                rtl::Reference<unoidl::InterfaceBasedSingletonEntity> ent2(
                    static_cast<unoidl::InterfaceBasedSingletonEntity *>(
                        ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "singleton " << id << ": ";
                writeName(ent2->getBase());
                std::cout << ";\n";
                break;
            }
        case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
            {
                rtl::Reference<unoidl::ServiceBasedSingletonEntity> ent2(
                    static_cast<unoidl::ServiceBasedSingletonEntity *>(
                        ent.get()));
                writeAnnotationsPublished(ent);
                std::cout << "singleton " << id << " { service ";
                writeName(ent2->getBase());
                std::cout << "; };";
                break;
            }
        case unoidl::Entity::SORT_MODULE:
            assert(false && "this cannot happen");
        }
    }
}

}

SAL_IMPLEMENT_MAIN() {
    try {
        sal_uInt32 args = rtl_getAppCommandArgCount();
        if (args == 0) {
            badUsage();
        }
        OUString arg;
        rtl_getAppCommandArg(0, &arg.pData);
        bool published = arg == "--published";
        if (published && args == 1) {
            badUsage();
        }
        rtl::Reference<unoidl::Manager> mgr(new unoidl::Manager);
        rtl::Reference<unoidl::Provider> prov;
        for (sal_uInt32 i = (published ? 1 : 0); i != args; ++i) {
            OUString uri(getArgumentUri(i));
            try {
                prov = mgr->addProvider(uri);
            } catch (unoidl::NoSuchFileException &) {
                std::cerr
                    << "Input <" << uri << "> does not exist" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
        std::map<OUString, Entity> ents;
        scanMap(mgr, prov->createRootCursor(), published, "", ents);
        std::vector<OUString> sorted(sort(ents));
        std::vector<OUString> mods;
        for (auto & i: sorted) {
            writeEntity(ents, mods, i);
        }
        closeModules(mods, mods.size());
        return EXIT_SUCCESS;
    } catch (unoidl::FileFormatException & e1) {
        std::cerr
            << "Bad input <" << e1.getUri() << ">: " << e1.getDetail()
            << std::endl;
        std::exit(EXIT_FAILURE);
    } catch (std::exception & e1) {
        std::cerr << "Failure: " << e1.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
