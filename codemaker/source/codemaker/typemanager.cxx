/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <cstdlib>
#include <cstring>
#include <vector>

#include <codemaker/global.hxx>
#include <codemaker/typemanager.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <unoidl/unoidl.hxx>

TypeManager::TypeManager(): manager_(new unoidl::Manager) {}

TypeManager::~TypeManager() {}

void TypeManager::loadProvider(OUString const & uri, bool primary) {
    rtl::Reference< unoidl::Provider > prov(manager_->addProvider(uri));
    if (primary) {
        primaryProviders_.push_back(prov);
    }
}

bool TypeManager::foundAtPrimaryProvider(OUString const & name) const {
    if (name.isEmpty()) {
        return !primaryProviders_.empty();
    }
    for (const rtl::Reference< unoidl::Provider >& xProvider : primaryProviders_)
    {
        if (xProvider->findEntity(name).is()) {
            return true;
        }
    }
    if (!manager_->findEntity(name).is()) {
        throw CannotDumpException("Unknown entity '" + name + "'");
    }
    return false;
}

codemaker::UnoType::Sort TypeManager::getSort(
    OUString const & name, rtl::Reference< unoidl::Entity > * entity,
    rtl::Reference< unoidl::MapCursor > * cursor) const
{
    if (name.isEmpty()) {
        if (cursor != nullptr) {
            *cursor = manager_->createCursor("");
        }
        return codemaker::UnoType::Sort::Module;
    }
    if (name == "void") {
        return codemaker::UnoType::Sort::Void;
    }
    if (name == "boolean") {
        return codemaker::UnoType::Sort::Boolean;
    }
    if (name == "byte") {
        return codemaker::UnoType::Sort::Byte;
    }
    if (name == "short") {
        return codemaker::UnoType::Sort::Short;
    }
    if (name == "unsigned short") {
        return codemaker::UnoType::Sort::UnsignedShort;
    }
    if (name == "long") {
        return codemaker::UnoType::Sort::Long;
    }
    if (name == "unsigned long") {
        return codemaker::UnoType::Sort::UnsignedLong;
    }
    if (name == "hyper") {
        return codemaker::UnoType::Sort::Hyper;
    }
    if (name == "unsigned hyper") {
        return codemaker::UnoType::Sort::UnsignedHyper;
    }
    if (name == "float") {
        return codemaker::UnoType::Sort::Float;
    }
    if (name == "double") {
        return codemaker::UnoType::Sort::Double;
    }
    if (name == "char") {
        return codemaker::UnoType::Sort::Char;
    }
    if (name == "string") {
        return codemaker::UnoType::Sort::String;
    }
    if (name == "type") {
        return codemaker::UnoType::Sort::Type;
    }
    if (name == "any") {
        return codemaker::UnoType::Sort::Any;
    }
    if (name.startsWith("[")) {
        return codemaker::UnoType::Sort::Sequence;
    }
    if (name.indexOf('<') != -1) {
        return codemaker::UnoType::Sort::InstantiatedPolymorphicStruct;
    }
    rtl::Reference< unoidl::Entity > ent(manager_->findEntity(name));
    if (!ent.is()) {
        throw CannotDumpException("Unknown entity '" + name + "'");
    }
    if (entity != nullptr) {
        *entity = ent;
    }
    switch (ent->getSort()) {
    case unoidl::Entity::SORT_MODULE:
        if (cursor != nullptr) {
            *cursor = manager_->createCursor(name);
        }
        return codemaker::UnoType::Sort::Module;
    case unoidl::Entity::SORT_ENUM_TYPE:
        return codemaker::UnoType::Sort::Enum;
    case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
        return codemaker::UnoType::Sort::PlainStruct;
    case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        return codemaker::UnoType::Sort::PolymorphicStructTemplate;
    case unoidl::Entity::SORT_EXCEPTION_TYPE:
        return codemaker::UnoType::Sort::Exception;
    case unoidl::Entity::SORT_INTERFACE_TYPE:
        return codemaker::UnoType::Sort::Interface;
    case unoidl::Entity::SORT_TYPEDEF:
        return codemaker::UnoType::Sort::Typedef;
    case unoidl::Entity::SORT_CONSTANT_GROUP:
        return codemaker::UnoType::Sort::ConstantGroup;
    case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
        return codemaker::UnoType::Sort::SingleInterfaceBasedService;
    case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
        return codemaker::UnoType::Sort::AccumulationBasedService;
    case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
        return codemaker::UnoType::Sort::InterfaceBasedSingleton;
    case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
        return codemaker::UnoType::Sort::ServiceBasedSingleton;
    default:
        for (;;) { std::abort(); } // this cannot happen
    }
}

codemaker::UnoType::Sort TypeManager::decompose(
    std::u16string_view name, bool resolveTypedefs, OUString * nucleus,
    sal_Int32 * rank, std::vector< OUString > * arguments,
    rtl::Reference< unoidl::Entity > * entity) const
{
    sal_Int32 k;
    std::vector< OString > args;
    OUString n = b2u(codemaker::UnoType::decompose(u2b(name), &k, &args));
    for (;;) {
        rtl::Reference< unoidl::Entity > ent;
        codemaker::UnoType::Sort s = getSort(n, &ent);
        if (args.empty()
            != (s != codemaker::UnoType::Sort::PolymorphicStructTemplate))
        {
            throw CannotDumpException(
                "template arguments mismatch for \"" + n
                + "\" resolved from \"" + name + "\"");
        }
        switch (s) {
        case codemaker::UnoType::Sort::Typedef:
            if (resolveTypedefs) {
                n = dynamic_cast<unoidl::TypedefEntity&>(*ent).getType();
                while (n.startsWith("[]")) {
                    ++k; //TODO: overflow
                    n = n.copy(std::strlen("[]"));
                }
                break;
            }
            [[fallthrough]];
        case codemaker::UnoType::Sort::Void:
        case codemaker::UnoType::Sort::Boolean:
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
        case codemaker::UnoType::Sort::Float:
        case codemaker::UnoType::Sort::Double:
        case codemaker::UnoType::Sort::Char:
        case codemaker::UnoType::Sort::String:
        case codemaker::UnoType::Sort::Type:
        case codemaker::UnoType::Sort::Any:
        case codemaker::UnoType::Sort::Enum:
        case codemaker::UnoType::Sort::PlainStruct:
        case codemaker::UnoType::Sort::Exception:
        case codemaker::UnoType::Sort::Interface:
            if (nucleus != nullptr) {
                *nucleus = n;
            }
            if (rank != nullptr) {
                *rank = k;
            }
            if (arguments != nullptr) {
                arguments->clear();
            }
            if (entity != nullptr) {
                *entity = ent;
            }
            return s;
        case codemaker::UnoType::Sort::PolymorphicStructTemplate:
            if (args.size()
                != (dynamic_cast<unoidl::PolymorphicStructTypeTemplateEntity&>(*ent).
                    getTypeParameters().size()))
            {
                throw CannotDumpException(
                    "bad number of template arguments for \"" + n
                    + "\" resolved from \"" + name + "\"");
            }
            if (nucleus != nullptr) {
                *nucleus = n;
            }
            if (rank != nullptr) {
                *rank = k;
            }
            if (arguments != nullptr) {
                arguments->clear();
                for (const OString& rArg : args)
                {
                    arguments->push_back(b2u(rArg));
                }
            }
            if (entity != nullptr) {
                *entity = ent;
            }
            return
                codemaker::UnoType::Sort::InstantiatedPolymorphicStruct;
        case codemaker::UnoType::Sort::Sequence:
            for (;;) std::abort(); // this cannot happen
        default:
            throw CannotDumpException(
                "unexpected \"" + n + "\" resolved from \"" + name
                + "\"in call to TypeManager::decompose");
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
