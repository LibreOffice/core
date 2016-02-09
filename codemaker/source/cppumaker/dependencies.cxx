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

#include <cassert>
#include <utility>
#include <vector>

#include "codemaker/global.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "unoidl/unoidl.hxx"

#include "dependencies.hxx"

namespace codemaker { namespace cppumaker {

Dependencies::Dependencies(
    rtl::Reference< TypeManager > const & manager, OUString const & name):
    m_manager(manager), m_voidDependency(false), m_booleanDependency(false),
    m_byteDependency(false), m_shortDependency(false),
    m_unsignedShortDependency(false), m_longDependency(false),
    m_unsignedLongDependency(false), m_hyperDependency(false),
    m_unsignedHyperDependency(false), m_floatDependency(false),
    m_doubleDependency(false), m_charDependency(false),
    m_stringDependency(false), m_typeDependency(false), m_anyDependency(false),
    m_sequenceDependency(false)
{
    assert(manager.is());
    rtl::Reference< unoidl::Entity > ent;
    switch (m_manager->getSort(name, &ent)) {
    case UnoType::SORT_ENUM_TYPE:
        break;
    case UnoType::SORT_PLAIN_STRUCT_TYPE:
        {
            rtl::Reference< unoidl::PlainStructTypeEntity > ent2(
                static_cast< unoidl::PlainStructTypeEntity * >(ent.get()));
            if (!ent2->getDirectBase().isEmpty()) {
                insert(ent2->getDirectBase());
            }
            for (const unoidl::PlainStructTypeEntity::Member& member : ent2->getDirectMembers())
            {
                insert(member.type);
            }
            break;
        }
    case UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        {
            rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > ent2(
                static_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
                    ent.get()));
            for (const unoidl::PolymorphicStructTypeTemplateEntity::Member& member : ent2->getMembers())
            {
                if (!member.parameterized) {
                    insert(member.type);
                }
            }
            break;
        }
    case UnoType::SORT_EXCEPTION_TYPE:
        {
            rtl::Reference< unoidl::ExceptionTypeEntity > ent2(
                static_cast< unoidl::ExceptionTypeEntity * >(ent.get()));
            if (!ent2->getDirectBase().isEmpty()) {
                insert(ent2->getDirectBase());
            }
            for (const unoidl::ExceptionTypeEntity::Member& member : ent2->getDirectMembers())
            {
                insert(member.type);
            }
            break;
        }
    case UnoType::SORT_INTERFACE_TYPE:
        {
            rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
                static_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
            for (const unoidl::AnnotatedReference& ar : ent2->getDirectMandatoryBases())
            {
                insert(ar.name, true);
            }
            if (!(ent2->getDirectAttributes().empty()
                  && ent2->getDirectMethods().empty()))
            {
                insert("com.sun.star.uno.RuntimeException");
            }
            for (const unoidl::InterfaceTypeEntity::Attribute& attr : ent2->getDirectAttributes())
            {
                insert(attr.type);
                for (const OUString& ex : attr.getExceptions)
                {
                    insert(ex);
                }
                for (const OUString& ex : attr.setExceptions)
                {
                    insert(ex);
                }
            }
            for (const unoidl::InterfaceTypeEntity::Method& method : ent2->getDirectMethods())
            {
                insert(method.returnType);
                for (const unoidl::InterfaceTypeEntity::Method::Parameter& param : method.parameters)
                {
                    insert(param.type);
                }
                for (const OUString& ex : method.exceptions)
                {
                    insert(ex);
                }
            }
            break;
        }
    case UnoType::SORT_TYPEDEF:
        insert(static_cast< unoidl::TypedefEntity * >(ent.get())->getType());
        break;
    case UnoType::SORT_CONSTANT_GROUP:
        {
            rtl::Reference< unoidl::ConstantGroupEntity > ent2(
                static_cast< unoidl::ConstantGroupEntity * >(ent.get()));
            for (const unoidl::ConstantGroupEntity::Member& member : ent2->getMembers())
            {
                switch (member.value.type) {
                case unoidl::ConstantValue::TYPE_BOOLEAN:
                    m_booleanDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_BYTE:
                    m_byteDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_SHORT:
                    m_shortDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
                    m_unsignedShortDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_LONG:
                    m_longDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
                    m_unsignedLongDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_HYPER:
                    m_hyperDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
                    m_unsignedHyperDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_FLOAT:
                    m_floatDependency = true;
                    break;
                case unoidl::ConstantValue::TYPE_DOUBLE:
                    m_doubleDependency = true;
                    break;
                }
            }
            break;
        }
    case UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE:
        {
            rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > ent2(
                static_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                    ent.get()));
            if (!ent2->getConstructors().empty()) {
                insert(ent2->getBase());
            }
            for (const unoidl::SingleInterfaceBasedServiceEntity::Constructor& cons : ent2->getConstructors())
            {
                for (const unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter& param
                         : cons.parameters)
                {
                    insert(param.type);
                    if (param.rest) {
                        m_sequenceDependency = true;
                    }
                }
                for (const OUString& ex : cons.exceptions)
                {
                    insert(ex);
                }
            }
            break;
        }
    case UnoType::SORT_INTERFACE_BASED_SINGLETON:
        insert(
            static_cast< unoidl::InterfaceBasedSingletonEntity * >(ent.get())->
            getBase());
        break;
    default:
        assert(false); // this cannot happen
    }
}

Dependencies::~Dependencies() {}

void Dependencies::insert(OUString const & name, bool base) {
    sal_Int32 k;
    std::vector< OString > args;
    OUString n(b2u(UnoType::decompose(u2b(name), &k, &args)));
    if (k != 0) {
        m_sequenceDependency = true;
    }
    switch (m_manager->getSort(n)) {
    case UnoType::SORT_VOID:
        m_voidDependency = true;
        break;
    case UnoType::SORT_BOOLEAN:
        m_booleanDependency = true;
        break;
    case UnoType::SORT_BYTE:
        m_byteDependency = true;
        break;
    case UnoType::SORT_SHORT:
        m_shortDependency = true;
        break;
    case UnoType::SORT_UNSIGNED_SHORT:
        m_unsignedShortDependency = true;
        break;
    case UnoType::SORT_LONG:
        m_longDependency = true;
        break;
    case UnoType::SORT_UNSIGNED_LONG:
        m_unsignedLongDependency = true;
        break;
    case UnoType::SORT_HYPER:
        m_hyperDependency = true;
        break;
    case UnoType::SORT_UNSIGNED_HYPER:
        m_unsignedHyperDependency = true;
        break;
    case UnoType::SORT_FLOAT:
        m_floatDependency = true;
        break;
    case UnoType::SORT_DOUBLE:
        m_doubleDependency = true;
        break;
    case UnoType::SORT_CHAR:
        m_charDependency = true;
        break;
    case UnoType::SORT_STRING:
        m_stringDependency = true;
        break;
    case UnoType::SORT_TYPE:
        m_typeDependency = true;
        break;
    case UnoType::SORT_ANY:
        m_anyDependency = true;
        break;
    case UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        for (const OString& arg : args)
        {
            insert(b2u(arg));
        }
        // fall through
    case UnoType::SORT_SEQUENCE_TYPE:
    case UnoType::SORT_ENUM_TYPE:
    case UnoType::SORT_PLAIN_STRUCT_TYPE:
    case UnoType::SORT_EXCEPTION_TYPE:
    case UnoType::SORT_INTERFACE_TYPE:
    case UnoType::SORT_TYPEDEF:
        {
            std::pair< Map::iterator, bool > i(
                m_map.insert(
                    Map::value_type(n, base ? KIND_BASE : KIND_NO_BASE)));
            if (!i.second && base) {
                i.first->second = KIND_BASE;
            }
            break;
        }
    default:
        throw CannotDumpException(
            "unexpected type \"" + name
            + "\" in call to codemaker::cppumaker::Dependencies::Dependencies");
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
