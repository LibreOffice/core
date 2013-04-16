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

#include "sal/config.h"

#include <cstdlib>
#include <cstring>
#include <vector>

#include "codemaker/global.hxx"
#include "codemaker/typemanager.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "unoidl/unoidl.hxx"

TypeManager::TypeManager(): manager_(new unoidl::Manager) {}

TypeManager::~TypeManager() {}

void TypeManager::loadProvider(OUString const & uri, bool primary) {
    rtl::Reference< unoidl::Provider > prov(
        unoidl::loadProvider(manager_, uri));
    manager_->addProvider(prov);
    if (primary) {
        primaryProviders_.push_back(prov);
    }
}

bool TypeManager::foundAtPrimaryProvider(OUString const & name) const {
    if (name.isEmpty()) {
        return !primaryProviders_.empty();
    }
    for (std::vector< rtl::Reference< unoidl::Provider > >::const_iterator i(
             primaryProviders_.begin());
         i != primaryProviders_.end(); ++i)
    {
        if ((*i)->findEntity(name).is()) {
            return true;
        }
    }
    return false;
}

codemaker::UnoType::Sort TypeManager::getSort(
    OUString const & name, rtl::Reference< unoidl::Entity > * entity,
    rtl::Reference< unoidl::MapCursor > * cursor) const
{
    if (name.isEmpty()) {
        if (cursor != 0) {
            *cursor = manager_->createCursor("");
        }
        return codemaker::UnoType::SORT_MODULE;
    }
    if (name == "void") {
        return codemaker::UnoType::SORT_VOID;
    }
    if (name == "boolean") {
        return codemaker::UnoType::SORT_BOOLEAN;
    }
    if (name == "byte") {
        return codemaker::UnoType::SORT_BYTE;
    }
    if (name == "short") {
        return codemaker::UnoType::SORT_SHORT;
    }
    if (name == "unsigned short") {
        return codemaker::UnoType::SORT_UNSIGNED_SHORT;
    }
    if (name == "long") {
        return codemaker::UnoType::SORT_LONG;
    }
    if (name == "unsigned long") {
        return codemaker::UnoType::SORT_UNSIGNED_LONG;
    }
    if (name == "hyper") {
        return codemaker::UnoType::SORT_HYPER;
    }
    if (name == "unsigned hyper") {
        return codemaker::UnoType::SORT_UNSIGNED_HYPER;
    }
    if (name == "float") {
        return codemaker::UnoType::SORT_FLOAT;
    }
    if (name == "double") {
        return codemaker::UnoType::SORT_DOUBLE;
    }
    if (name == "char") {
        return codemaker::UnoType::SORT_CHAR;
    }
    if (name == "string") {
        return codemaker::UnoType::SORT_STRING;
    }
    if (name == "type") {
        return codemaker::UnoType::SORT_TYPE;
    }
    if (name == "any") {
        return codemaker::UnoType::SORT_ANY;
    }
    if (name.startsWith("[")) {
        return codemaker::UnoType::SORT_SEQUENCE_TYPE;
    }
    if (name.indexOf('<') != -1) {
        return codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE;
    }
    rtl::Reference< unoidl::Entity > ent(manager_->findEntity(name));
    if (!ent.is()) {
        throw CannotDumpException("Unknown entity '" + name + "'");
    }
    if (entity != 0) {
        *entity = ent;
    }
    switch (ent->getSort()) {
    case unoidl::Entity::SORT_MODULE:
        if (cursor != 0) {
            *cursor = manager_->createCursor(name);
        }
        return codemaker::UnoType::SORT_MODULE;
    case unoidl::Entity::SORT_ENUM_TYPE:
        return codemaker::UnoType::SORT_ENUM_TYPE;
    case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
        return codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE;
    case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        return codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE;
    case unoidl::Entity::SORT_EXCEPTION_TYPE:
        return codemaker::UnoType::SORT_EXCEPTION_TYPE;
    case unoidl::Entity::SORT_INTERFACE_TYPE:
        return codemaker::UnoType::SORT_INTERFACE_TYPE;
    case unoidl::Entity::SORT_TYPEDEF:
        return codemaker::UnoType::SORT_TYPEDEF;
    case unoidl::Entity::SORT_CONSTANT_GROUP:
        return codemaker::UnoType::SORT_CONSTANT_GROUP;
    case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
        return codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE;
    case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
        return codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE;
    case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
        return codemaker::UnoType::SORT_INTERFACE_BASED_SINGLETON;
    case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
        return codemaker::UnoType::SORT_SERVICE_BASED_SINGLETON;
    default:
        for (;;) { std::abort(); } // this cannot happen
    }
}

codemaker::UnoType::Sort TypeManager::decompose(
    OUString const & name, bool resolveTypedefs, OUString * nucleus,
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
            != (s != codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE))
        {
            throw CannotDumpException(
                "template arguments mismatch for \"" + n
                + "\" resolved from \"" + name + "\"");
        }
        switch (s) {
        case codemaker::UnoType::SORT_TYPEDEF:
            if (resolveTypedefs) {
                n = dynamic_cast< unoidl::TypedefEntity * >(ent.get())->
                    getType();
                while (n.startsWith("[]")) {
                    ++k; //TODO: overflow
                    n = n.copy(std::strlen("[]"));
                }
                break;
            }
            // fall through
        case codemaker::UnoType::SORT_VOID:
        case codemaker::UnoType::SORT_BOOLEAN:
        case codemaker::UnoType::SORT_BYTE:
        case codemaker::UnoType::SORT_SHORT:
        case codemaker::UnoType::SORT_UNSIGNED_SHORT:
        case codemaker::UnoType::SORT_LONG:
        case codemaker::UnoType::SORT_UNSIGNED_LONG:
        case codemaker::UnoType::SORT_HYPER:
        case codemaker::UnoType::SORT_UNSIGNED_HYPER:
        case codemaker::UnoType::SORT_FLOAT:
        case codemaker::UnoType::SORT_DOUBLE:
        case codemaker::UnoType::SORT_CHAR:
        case codemaker::UnoType::SORT_STRING:
        case codemaker::UnoType::SORT_TYPE:
        case codemaker::UnoType::SORT_ANY:
        case codemaker::UnoType::SORT_ENUM_TYPE:
        case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
        case codemaker::UnoType::SORT_EXCEPTION_TYPE:
        case codemaker::UnoType::SORT_INTERFACE_TYPE:
            if (nucleus != 0) {
                *nucleus = n;
            }
            if (rank != 0) {
                *rank = k;
            }
            if (arguments != 0) {
                arguments->clear();
            }
            if (entity != 0) {
                *entity = ent;
            }
            return s;
        case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
            if (args.size()
                != (dynamic_cast<
                    unoidl::PolymorphicStructTypeTemplateEntity * >(ent.get())->
                    getTypeParameters().size()))
            {
                throw CannotDumpException(
                    "bad number of template arguments for \"" + n
                    + "\" resolved from \"" + name + "\"");
            }
            if (nucleus != 0) {
                *nucleus = n;
            }
            if (rank != 0) {
                *rank = k;
            }
            if (arguments != 0) {
                arguments->clear();
                for (std::vector< OString >::iterator i(args.begin());
                     i != args.end(); ++i)
                {
                    arguments->push_back(b2u(*i));
                }
            }
            if (entity != 0) {
                *entity = ent;
            }
            return
                codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE;
        case codemaker::UnoType::SORT_SEQUENCE_TYPE:
            assert(false); // this cannot happen
            // fall through
        default:
            throw CannotDumpException(
                "unexpected \"" + n + "\" resolved from \"" + name
                + ("\"in call to TypeManager::decompose"));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
