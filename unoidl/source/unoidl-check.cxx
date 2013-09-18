/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/process.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/main.h"
#include "sal/types.h"
#include "unoidl/unoidl.hxx"

namespace unoidl {

bool operator ==(ConstantValue const & lhs, ConstantValue const & rhs) {
    if (lhs.type == rhs.type) {
        switch (lhs.type) {
        case ConstantValue::TYPE_BOOLEAN:
            return lhs.booleanValue == rhs.booleanValue;
        case ConstantValue::TYPE_BYTE:
            return lhs.byteValue == rhs.byteValue;
        case ConstantValue::TYPE_SHORT:
            return lhs.shortValue == rhs.shortValue;
        case ConstantValue::TYPE_UNSIGNED_SHORT:
            return lhs.unsignedShortValue == rhs.unsignedShortValue;
        case ConstantValue::TYPE_LONG:
            return lhs.longValue == rhs.longValue;
        case ConstantValue::TYPE_UNSIGNED_LONG:
            return lhs.unsignedLongValue == rhs.unsignedLongValue;
        case ConstantValue::TYPE_HYPER:
            return lhs.hyperValue == rhs.hyperValue;
        case ConstantValue::TYPE_UNSIGNED_HYPER:
            return lhs.unsignedHyperValue == rhs.unsignedHyperValue;
        case ConstantValue::TYPE_FLOAT:
            return lhs.floatValue == rhs.floatValue;
        case ConstantValue::TYPE_DOUBLE:
            return lhs.doubleValue == rhs.doubleValue;
        }
    }
    return false;
}

bool operator !=(ConstantValue const & lhs, ConstantValue const & rhs) {
    return !(lhs == rhs);
}

bool operator ==(
    SingleInterfaceBasedServiceEntity::Constructor::Parameter const & lhs,
    SingleInterfaceBasedServiceEntity::Constructor::Parameter const & rhs)
{
    return lhs.name == rhs.name && lhs.type == rhs.type && lhs.rest == rhs.rest;
}

}

namespace {

void badUsage() {
    std::cerr
        << "Usage:" << std::endl << std::endl
        << ("  unoidl-check [<extra registries A>] <registry A> -- [<extra"
            " registries B>]")
        << std::endl << "    <registry B>" << std::endl << std::endl
        << ("where each <registry> is either a new- or legacy-format .rdb file,"
            " a single .idl")
        << std::endl
        << ("file, or a root directory of an .idl file tree.  Check that each"
            " entity from")
        << std::endl
        << "<registry A> is also present in <registry B> in a compatible form."
        << std::endl;
    std::exit(EXIT_FAILURE);
}

OUString getArgumentUri(sal_uInt32 argument, bool * delimiter) {
    OUString arg;
    rtl_getAppCommandArg(argument, &arg.pData);
    if (arg == "--") {
        if (delimiter == 0) {
            badUsage();
        }
        *delimiter = true;
        return OUString();
    }
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

OUString showDirection(
    unoidl::InterfaceTypeEntity::Method::Parameter::Direction direction)
{
    switch (direction) {
    case unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN:
        return OUString("[in]");
    case unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_OUT:
        return OUString("[out]");
    case unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN_OUT:
        return OUString("[inout]");
    default:
        assert(false); for (;;) { std::abort(); } // this cannot happen
    }
}

void checkMap(
    rtl::Reference<unoidl::Provider> const & providerB, OUString const & prefix,
    rtl::Reference<unoidl::MapCursor> const & cursor)
{
    assert(providerB.is());
    assert(cursor.is());
    for (;;) {
        OUString id;
        rtl::Reference<unoidl::Entity> entA(cursor->getNext(&id));
        if (!entA.is()) {
            break;
        }
        OUString name(prefix + id);
        if (entA->getSort() == unoidl::Entity::SORT_MODULE) {
            checkMap(
                providerB, name + ".",
                (static_cast<unoidl::ModuleEntity *>(entA.get())
                 ->createCursor()));
        } else {
            rtl::Reference<unoidl::Entity> entB(providerB->findEntity(name));
            if (!entB.is()) {
                std::cerr
                    << "A entity " << name << " is not present in B"
                    << std::endl;
                std::exit(EXIT_FAILURE);
            }
            if (entA->getSort() != entB->getSort()) {
                std::cerr
                    << "A entity " << name << " is of different sort in B"
                    << std::endl;
                std::exit(EXIT_FAILURE);
            }
            if ((dynamic_cast<unoidl::PublishableEntity *>(entA.get())
                 ->isPublished())
                && (!dynamic_cast<unoidl::PublishableEntity *>(entB.get())
                    ->isPublished()))
            {
                std::cerr
                    << "A published entity " << name << " is not published in B"
                    << std::endl;
                std::exit(EXIT_FAILURE);
            }
            switch (entA->getSort()) {
            case unoidl::Entity::SORT_MODULE:
                assert(false); // this cannot happen
            case unoidl::Entity::SORT_ENUM_TYPE:
                {
                    rtl::Reference<unoidl::EnumTypeEntity> ent2A(
                        static_cast<unoidl::EnumTypeEntity *>(entA.get()));
                    rtl::Reference<unoidl::EnumTypeEntity> ent2B(
                        static_cast<unoidl::EnumTypeEntity *>(entB.get()));
                    if (ent2A->getMembers().size()
                        != ent2B->getMembers().size())
                    {
                        std::cerr
                            << "enum type " << name
                            << " number of members changed from "
                            << ent2A->getMembers().size() << " to "
                            << ent2B->getMembers().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::EnumTypeEntity::Member>::const_iterator
                             i(ent2A->getMembers().begin()),
                             j(ent2B->getMembers().begin());
                         i != ent2A->getMembers().end(); ++i, ++j)
                    {
                        if (i->name != j->name || i->value != j->value) {
                            std::cerr
                                << "enum type " << name << " member #"
                                << i - ent2A->getMembers().begin() + 1
                                << " changed from " << i->name << " = "
                                << i->value << " to " << j->name << " = "
                                << j->value << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
                {
                    rtl::Reference<unoidl::PlainStructTypeEntity> ent2A(
                        static_cast<unoidl::PlainStructTypeEntity *>(
                            entA.get()));
                    rtl::Reference<unoidl::PlainStructTypeEntity> ent2B(
                        static_cast<unoidl::PlainStructTypeEntity *>(
                            entB.get()));
                    if (ent2A->getDirectBase() != ent2B->getDirectBase()) {
                        std::cerr
                            << "plain struct type " << name
                            << " direct base changed from "
                            << (ent2A->getDirectBase().isEmpty()
                                ? OUString("none") : ent2A->getDirectBase())
                            << " to "
                            << (ent2B->getDirectBase().isEmpty()
                                ? OUString("none") : ent2B->getDirectBase())
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    if (ent2A->getDirectMembers().size()
                        != ent2B->getDirectMembers().size())
                    {
                        std::cerr
                            << "plain struct type " << name
                            << " number of direct members changed from "
                            << ent2A->getDirectMembers().size() << " to "
                            << ent2B->getDirectMembers().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::PlainStructTypeEntity::Member>::const_iterator
                             i(ent2A->getDirectMembers().begin()),
                             j(ent2B->getDirectMembers().begin());
                         i != ent2A->getDirectMembers().end(); ++i, ++j)
                    {
                        if (i->name != j->name || i->type != j->type) {
                            std::cerr
                                << "plain struct type " << name
                                << " direct member #"
                                << i - ent2A->getDirectMembers().begin() + 1
                                << " changed from " << i->type << " " << i->name
                                << " to " << j->type << " " << j->name
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
                {
                    rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity>
                        ent2A(
                            static_cast<unoidl::PolymorphicStructTypeTemplateEntity *>(
                                entA.get()));
                    rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity>
                        ent2B(
                            static_cast<unoidl::PolymorphicStructTypeTemplateEntity *>(
                                entB.get()));
                    if (ent2A->getTypeParameters().size()
                        != ent2B->getTypeParameters().size())
                    {
                        std::cerr
                            << "polymorphic struct type template " << name
                            << " number of type parameters changed from "
                            << ent2A->getTypeParameters().size() << " to "
                            << ent2B->getTypeParameters().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<OUString>::const_iterator
                             i(ent2A->getTypeParameters().begin()),
                             j(ent2B->getTypeParameters().begin());
                         i != ent2A->getTypeParameters().end(); ++i, ++j)
                    {
                        if (*i != *j) {
                            std::cerr
                                << "polymorphic struct type template " << name
                                << " type parameter #"
                                << i - ent2A->getTypeParameters().begin() + 1
                                << " changed from " << *i << " to " << *j
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    if (ent2A->getMembers().size()
                        != ent2B->getMembers().size())
                    {
                        std::cerr
                            << "polymorphic struct type template " << name
                            << " number of members changed from "
                            << ent2A->getMembers().size() << " to "
                            << ent2B->getMembers().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::PolymorphicStructTypeTemplateEntity::Member>::const_iterator
                             i(ent2A->getMembers().begin()),
                             j(ent2B->getMembers().begin());
                         i != ent2A->getMembers().end(); ++i, ++j)
                    {
                        if (i->name != j->name || i->type != j->type
                            || i->parameterized != j->parameterized)
                        {
                            std::cerr
                                << "polymorphic struct type template " << name
                                << " member #"
                                << i - ent2A->getMembers().begin() + 1
                                << " changed from "
                                << (i->parameterized
                                    ? OUString("parameterized ") : OUString())
                                << i->type << " " << i->name
                                << " to "
                                << (j->parameterized
                                    ? OUString("parameterized ") : OUString())
                                << j->type << " " << j->name
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_EXCEPTION_TYPE:
                {
                    rtl::Reference<unoidl::ExceptionTypeEntity> ent2A(
                        static_cast<unoidl::ExceptionTypeEntity *>(entA.get()));
                    rtl::Reference<unoidl::ExceptionTypeEntity> ent2B(
                        static_cast<unoidl::ExceptionTypeEntity *>(entB.get()));
                    if (ent2A->getDirectBase() != ent2B->getDirectBase()) {
                        std::cerr
                            << "exception type " << name
                            << " direct base changed from "
                            << (ent2A->getDirectBase().isEmpty()
                                ? OUString("none") : ent2A->getDirectBase())
                            << " to "
                            << (ent2B->getDirectBase().isEmpty()
                                ? OUString("none") : ent2B->getDirectBase())
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    if (ent2A->getDirectMembers().size()
                        != ent2B->getDirectMembers().size())
                    {
                        std::cerr
                            << "exception type " << name
                            << " number of direct members changed from "
                            << ent2A->getDirectMembers().size() << " to "
                            << ent2B->getDirectMembers().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::ExceptionTypeEntity::Member>::const_iterator
                             i(ent2A->getDirectMembers().begin()),
                             j(ent2B->getDirectMembers().begin());
                         i != ent2A->getDirectMembers().end(); ++i, ++j)
                    {
                        if (i->name != j->name || i->type != j->type) {
                            std::cerr
                                << "exception type " << name
                                << " direct member #"
                                << i - ent2A->getDirectMembers().begin() + 1
                                << " changed from " << i->type << " " << i->name
                                << " to " << j->type << " " << j->name
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_INTERFACE_TYPE:
                {
                    rtl::Reference<unoidl::InterfaceTypeEntity> ent2A(
                        static_cast<unoidl::InterfaceTypeEntity *>(entA.get()));
                    rtl::Reference<unoidl::InterfaceTypeEntity> ent2B(
                        static_cast<unoidl::InterfaceTypeEntity *>(entB.get()));
                    if (ent2A->getDirectMandatoryBases().size()
                        != ent2B->getDirectMandatoryBases().size())
                    {
                        std::cerr
                            << "interface type " << name
                            << " number of direct mandatory bases changed from "
                            << ent2A->getDirectMandatoryBases().size() << " to "
                            << ent2B->getDirectMandatoryBases().size()
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::AnnotatedReference>::const_iterator
                             i(ent2A->getDirectMandatoryBases().begin()),
                             j(ent2B->getDirectMandatoryBases().begin());
                         i != ent2A->getDirectMandatoryBases().end(); ++i, ++j)
                    {
                        if (i->name != j->name) {
                            std::cerr
                                << "interface type " << name
                                << " direct mandatory base #"
                                << (i - ent2A->getDirectMandatoryBases().begin()
                                    + 1)
                                << " changed from " << i->name << " to "
                                << j->name << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    if (ent2A->getDirectOptionalBases().size()
                        != ent2B->getDirectOptionalBases().size())
                    {
                        std::cerr
                            << "interface type " << name
                            << " number of direct optional bases changed from "
                            << ent2A->getDirectOptionalBases().size() << " to "
                            << ent2B->getDirectOptionalBases().size()
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::AnnotatedReference>::const_iterator
                             i(ent2A->getDirectOptionalBases().begin()),
                             j(ent2B->getDirectOptionalBases().begin());
                         i != ent2A->getDirectOptionalBases().end(); ++i, ++j)
                    {
                        if (i->name != j->name) {
                            std::cerr
                                << "interface type " << name
                                << " direct optional base #"
                                << (i - ent2A->getDirectOptionalBases().begin()
                                    + 1)
                                << " changed from " << i->name << " to "
                                << j->name << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    if (ent2A->getDirectAttributes().size()
                        != ent2B->getDirectAttributes().size())
                    {
                        std::cerr
                            << "interface type " << name
                            << " number of direct attributes changed from "
                            << ent2A->getDirectAttributes().size() << " to "
                            << ent2B->getDirectAttributes().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::InterfaceTypeEntity::Attribute>::const_iterator
                             i(ent2A->getDirectAttributes().begin()),
                             j(ent2B->getDirectAttributes().begin());
                         i != ent2A->getDirectAttributes().end(); ++i, ++j)
                    {
                        if (i->name != j->name || i->type != j->type
                            || i->bound != j->bound
                            || i->readOnly != j->readOnly
                            || i->getExceptions != j->getExceptions
                            || i->setExceptions != j->setExceptions)
                        {
                            std::cerr
                                << "interface type " << name
                                << " direct attribute #"
                                << i - ent2A->getDirectAttributes().begin() + 1
                                << " changed from "
                                << (i->bound ? OUString("bound ") : OUString())
                                << (i->readOnly
                                    ? OUString("read-only ") : OUString())
                                << i->type << " " << i->name //TODO: exceptions
                                << " to "
                                << (j->bound ? OUString("bound ") : OUString())
                                << (j->readOnly
                                    ? OUString("read-only ") : OUString())
                                << j->type << " " << j->name //TODO: exceptions
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    if (ent2A->getDirectMethods().size()
                        != ent2B->getDirectMethods().size())
                    {
                        std::cerr
                            << "interface type " << name
                            << " number of direct methods changed from "
                            << ent2A->getDirectMethods().size() << " to "
                            << ent2B->getDirectMethods().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::InterfaceTypeEntity::Method>::const_iterator
                             i(ent2A->getDirectMethods().begin()),
                             j(ent2B->getDirectMethods().begin());
                         i != ent2A->getDirectMethods().end(); ++i, ++j)
                    {
                        if (i->name != j->name || i->returnType != j->returnType
                            || i->exceptions != j->exceptions)
                        {
                            std::cerr
                                << "interface type " << name
                                << " direct method #"
                                << i - ent2A->getDirectMethods().begin() + 1
                                << " changed from "
                                << i->returnType << " " << i->name //TODO: exceptions
                                << " to " << j->returnType << " " << j->name //TODO: exceptions
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                        if (i->parameters.size() != j->parameters.size()) {
                            std::cerr
                                << "interface type " << name
                                << " direct method " << i->name
                                << " number of parameters changed from "
                                << i->parameters.size() << " to "
                                << j->parameters.size() << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                        for (std::vector<unoidl::InterfaceTypeEntity::Method::Parameter>::const_iterator
                                 k(i->parameters.begin()),
                                 l(j->parameters.begin());
                             k != i->parameters.end(); ++k, ++l)
                        {
                            if (k->type != l->type || k->direction != l->direction)
                            {
                                std::cerr
                                    << "interface type " << name
                                    << " direct method " << i->name
                                    << " parameter #"
                                    << k - i->parameters.begin() + 1
                                    << " changed from "
                                    << showDirection(k->direction) << " "
                                    << k->type << " to "
                                    << showDirection(l->direction) << " "
                                    << l->type << std::endl;
                                std::exit(EXIT_FAILURE);
                            }
                            if (k->name != l->name) {
                                std::cerr
                                    << "Warning: interface type " << name
                                    << " direct method " << i->name
                                    << " parameter #"
                                    << k - i->parameters.begin() + 1
                                    << " changed name from " << k->name
                                    << " to " << l->name << std::endl;
                            }
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_TYPEDEF:
                {
                    rtl::Reference<unoidl::TypedefEntity> ent2A(
                        static_cast<unoidl::TypedefEntity *>(entA.get()));
                    rtl::Reference<unoidl::TypedefEntity> ent2B(
                        static_cast<unoidl::TypedefEntity *>(entB.get()));
                    if (ent2A->getType() != ent2B->getType()) {
                        std::cerr
                            << "typedef " << name << " type changed from "
                            << ent2A->getType() << " to " << ent2B->getType()
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    break;
                }
            case unoidl::Entity::SORT_CONSTANT_GROUP:
                {
                    rtl::Reference<unoidl::ConstantGroupEntity> ent2A(
                        static_cast<unoidl::ConstantGroupEntity *>(entA.get()));
                    rtl::Reference<unoidl::ConstantGroupEntity> ent2B(
                        static_cast<unoidl::ConstantGroupEntity *>(entB.get()));
                    for (std::vector<unoidl::ConstantGroupEntity::Member>::const_iterator
                             i(ent2A->getMembers().begin());
                         i != ent2A->getMembers().end(); ++i)
                    {
                        bool found = false;
                        for (std::vector<unoidl::ConstantGroupEntity::Member>::const_iterator
                                 j(ent2B->getMembers().begin());
                             j != ent2B->getMembers().end(); ++j)
                        {
                            if (i->name == j->name) {
                                if (i->value != j->value) {
                                    std::cerr
                                        << "constant group " << name
                                        << " member " << i->name
                                        << " changed value" << std::endl;
                                    std::exit(EXIT_FAILURE);
                                }
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            std::cerr
                                << "A constant group " << name << " member "
                                << i->name << " is not present in B"
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
                {
                    rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>
                        ent2A(
                            static_cast<unoidl::SingleInterfaceBasedServiceEntity *>(
                                entA.get()));
                    rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>
                        ent2B(
                            static_cast<unoidl::SingleInterfaceBasedServiceEntity *>(
                                entB.get()));
                    if (ent2A->getBase() != ent2B->getBase()) {
                        std::cerr
                            << "single-interface--based servcie " << name
                            << " base changed from " << ent2A->getBase()
                            << " to " << ent2B->getBase()
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    if (ent2A->getConstructors().size()
                        != ent2B->getConstructors().size())
                    {
                        std::cerr
                            << "single-interface--based service " << name
                            << " number of constructors changed from "
                            << ent2A->getConstructors().size() << " to "
                            << ent2B->getConstructors().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::SingleInterfaceBasedServiceEntity::Constructor>::const_iterator
                             i(ent2A->getConstructors().begin()),
                             j(ent2B->getConstructors().begin());
                         i != ent2A->getConstructors().end(); ++i, ++j)
                    {
                        if (i->name != j->name || i->parameters != j->parameters
                            || i->exceptions != j->exceptions
                            || i->defaultConstructor != j->defaultConstructor)
                        {
                            std::cerr
                                << "single-interface--based service " << name
                                << " constructor #"
                                << i - ent2A->getConstructors().begin() + 1
                                << " changed from "
                                << (i->defaultConstructor
                                    ? OUString("default ") : i->name) //TODO: parameters, exceptions
                                << " to "
                                << (j->defaultConstructor
                                    ? OUString("default ") : j->name) //TODO: parameters, exceptions
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
                {
                    rtl::Reference<unoidl::AccumulationBasedServiceEntity>
                        ent2A(
                            static_cast<unoidl::AccumulationBasedServiceEntity *>(
                                entA.get()));
                    rtl::Reference<unoidl::AccumulationBasedServiceEntity>
                        ent2B(
                            static_cast<unoidl::AccumulationBasedServiceEntity *>(
                                entB.get()));
                    if (ent2A->getDirectMandatoryBaseServices().size()
                        != ent2B->getDirectMandatoryBaseServices().size())
                    {
                        std::cerr
                            << "accumulation-based service " << name
                            << (" number of direct mandatory base services"
                                " changed from ")
                            << ent2A->getDirectMandatoryBaseServices().size()
                            << " to "
                            << ent2B->getDirectMandatoryBaseServices().size()
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::AnnotatedReference>::const_iterator
                             i(ent2A->getDirectMandatoryBaseServices().begin()),
                             j(ent2B->getDirectMandatoryBaseServices().begin());
                         i != ent2A->getDirectMandatoryBaseServices().end();
                         ++i, ++j)
                    {
                        if (i->name != j->name) {
                            std::cerr
                                << "accumulation-based service " << name
                                << " direct mandatory base service #"
                                << (i
                                    - (ent2A->getDirectMandatoryBaseServices()
                                       .begin())
                                    + 1)
                                << " changed from " << i->name << " to "
                                << j->name << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    if (ent2A->getDirectOptionalBaseServices().size()
                        != ent2B->getDirectOptionalBaseServices().size())
                    {
                        std::cerr
                            << "accumulation-based service " << name
                            << (" number of direct optional base services"
                                " changed from ")
                            << ent2A->getDirectOptionalBaseServices().size()
                            << " to "
                            << ent2B->getDirectOptionalBaseServices().size()
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::AnnotatedReference>::const_iterator
                             i(ent2A->getDirectOptionalBaseServices().begin()),
                             j(ent2B->getDirectOptionalBaseServices().begin());
                         i != ent2A->getDirectOptionalBaseServices().end();
                         ++i, ++j)
                    {
                        if (i->name != j->name) {
                            std::cerr
                                << "accumulation-based service " << name
                                << " direct optional base service #"
                                << (i
                                    - (ent2A->getDirectOptionalBaseServices()
                                       .begin())
                                    + 1)
                                << " changed from " << i->name << " to "
                                << j->name << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    if (ent2A->getDirectMandatoryBaseInterfaces().size()
                        != ent2B->getDirectMandatoryBaseInterfaces().size())
                    {
                        std::cerr
                            << "accumulation-based service " << name
                            << (" number of direct mandatory base interfaces"
                                " changed from ")
                            << ent2A->getDirectMandatoryBaseInterfaces().size()
                            << " to "
                            << ent2B->getDirectMandatoryBaseInterfaces().size()
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::AnnotatedReference>::const_iterator
                             i(ent2A->getDirectMandatoryBaseInterfaces()
                               .begin()),
                             j(ent2B->getDirectMandatoryBaseInterfaces()
                               .begin());
                         i != ent2A->getDirectMandatoryBaseInterfaces().end();
                         ++i, ++j)
                    {
                        if (i->name != j->name) {
                            std::cerr
                                << "accumulation-based service " << name
                                << " direct mandatory base interface #"
                                << (i
                                    - (ent2A->getDirectMandatoryBaseInterfaces()
                                       .begin())
                                    + 1)
                                << " changed from " << i->name << " to "
                                << j->name << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    if (ent2A->getDirectOptionalBaseInterfaces().size()
                        != ent2B->getDirectOptionalBaseInterfaces().size())
                    {
                        std::cerr
                            << "accumulation-based service " << name
                            << (" number of direct optional base interfaces"
                                " changed from ")
                            << ent2A->getDirectOptionalBaseInterfaces().size()
                            << " to "
                            << ent2B->getDirectOptionalBaseInterfaces().size()
                            << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::AnnotatedReference>::const_iterator
                             i(ent2A->getDirectOptionalBaseInterfaces()
                               .begin()),
                             j(ent2B->getDirectOptionalBaseInterfaces()
                               .begin());
                         i != ent2A->getDirectOptionalBaseInterfaces().end();
                         ++i, ++j)
                    {
                        if (i->name != j->name) {
                            std::cerr
                                << "accumulation-based service " << name
                                << " direct optional base interface #"
                                << (i
                                    - (ent2A->getDirectOptionalBaseInterfaces()
                                       .begin())
                                    + 1)
                                << " changed from " << i->name << " to "
                                << j->name << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    if (ent2A->getDirectProperties().size()
                        > ent2B->getDirectProperties().size())
                    {
                        std::cerr
                            << "accumulation-based service " << name
                            << " number of direct properties changed from "
                            << ent2A->getDirectProperties().size() << " to "
                            << ent2B->getDirectProperties().size() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    for (std::vector<unoidl::AccumulationBasedServiceEntity::Property>::const_iterator
                             i(ent2A->getDirectProperties().begin()),
                             j(ent2B->getDirectProperties().begin());
                         i != ent2A->getDirectProperties().end(); ++i, ++j)
                    {
                        if (i->name != j->name || i->type != j->type
                            || i->attributes != j->attributes)
                        {
                            std::cerr
                                << "accumulation-based service " << name
                                << " direct property #"
                                << i - ent2A->getDirectProperties().begin() + 1
                                << " changed from "
                                << i->type << " " << i->name //TODO: attributes
                                << " to "
                                << j->type << " " << j->name //TODO: attributes
                                << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    for (std::vector<unoidl::AccumulationBasedServiceEntity::Property>::const_iterator
                             i(ent2B->getDirectProperties().begin()
                               + ent2A->getDirectProperties().size());
                         i != ent2B->getDirectProperties().end(); ++i)
                    {
                        if ((i->attributes & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_OPTIONAL) == 0)
                        {
                            std::cerr
                                << "B accumulation-based service " << name
                                << " additional direct property " << i->name
                                << " is not optional" << std::endl;
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    break;
                }
            case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
                {
                    rtl::Reference<unoidl::InterfaceBasedSingletonEntity> ent2A(
                        static_cast<unoidl::InterfaceBasedSingletonEntity *>(
                            entA.get()));
                    rtl::Reference<unoidl::InterfaceBasedSingletonEntity> ent2B(
                        static_cast<unoidl::InterfaceBasedSingletonEntity *>(
                            entB.get()));
                    if (ent2A->getBase() != ent2B->getBase()) {
                        std::cerr
                            << "interface-based singleton " << name
                            << " base changed from " << ent2A->getBase()
                            << " to " << ent2B->getBase() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    break;
                }
            case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
                {
                    rtl::Reference<unoidl::ServiceBasedSingletonEntity> ent2A(
                        static_cast<unoidl::ServiceBasedSingletonEntity *>(
                            entA.get()));
                    rtl::Reference<unoidl::ServiceBasedSingletonEntity> ent2B(
                        static_cast<unoidl::ServiceBasedSingletonEntity *>(
                            entB.get()));
                    if (ent2A->getBase() != ent2B->getBase()) {
                        std::cerr
                            << "service-based singleton " << name
                            << " base changed from " << ent2A->getBase()
                            << " to " << ent2B->getBase() << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    break;
                }
            }
        }
    }
}

}

SAL_IMPLEMENT_MAIN() {
    try {
        sal_uInt32 args = rtl_getAppCommandArgCount();
        rtl::Reference<unoidl::Manager> mgr[2];
        mgr[0] = new unoidl::Manager;
        mgr[1] = new unoidl::Manager;
        rtl::Reference<unoidl::Provider> prov[2];
        int side = 0;
        for (sal_uInt32 i = 0; i != args; ++i) {
            bool delimiter = false;
            OUString uri(getArgumentUri(i, side == 0 ? &delimiter : 0));
            if (delimiter) {
                side = 1;
            } else {
                try {
                    prov[side] = unoidl::loadProvider(mgr[side], uri);
                } catch (unoidl::NoSuchFileException &) {
                    std::cerr
                        << "Input <" << uri << "> does not exist" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                mgr[side]->addProvider(prov[side]);
            }
        }
        if (side == 0 || !(prov[0].is() && prov[1].is())) {
            badUsage();
        }
        checkMap(prov[1], "", prov[0]->createRootCursor());
        return EXIT_SUCCESS;
    } catch (unoidl::FileFormatException & e1) {
        std::cerr
            << "Bad input <" << e1.getUri() << ">: " << e1.getDetail()
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
