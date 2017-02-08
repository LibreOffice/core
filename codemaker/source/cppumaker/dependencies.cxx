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
    case UnoType::Sort::Enum:
        break;
    case UnoType::Sort::PlainStruct:
        {
            rtl::Reference< unoidl::PlainStructTypeEntity > ent2(
                static_cast< unoidl::PlainStructTypeEntity * >(ent.get()));
            if (!ent2->getDirectBase().isEmpty()) {
                insert(ent2->getDirectBase(), KIND_NORMAL);
            }
            for (const unoidl::PlainStructTypeEntity::Member& member : ent2->getDirectMembers())
            {
                insert(member.type, KIND_NORMAL);
            }
            break;
        }
    case UnoType::Sort::PolymorphicStructTemplate:
        {
            rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > ent2(
                static_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
                    ent.get()));
            for (const unoidl::PolymorphicStructTypeTemplateEntity::Member& member : ent2->getMembers())
            {
                if (!member.parameterized) {
                    insert(member.type, KIND_NORMAL);
                }
            }
            break;
        }
    case UnoType::Sort::Exception:
        {
            rtl::Reference< unoidl::ExceptionTypeEntity > ent2(
                static_cast< unoidl::ExceptionTypeEntity * >(ent.get()));
            if (!ent2->getDirectBase().isEmpty()) {
                insert(ent2->getDirectBase(), KIND_NORMAL);
            }
            for (const unoidl::ExceptionTypeEntity::Member& member : ent2->getDirectMembers())
            {
                insert(member.type, KIND_NORMAL);
            }
            break;
        }
    case UnoType::Sort::Interface:
        {
            rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
                static_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
            for (const unoidl::AnnotatedReference& ar : ent2->getDirectMandatoryBases())
            {
                insert(ar.name, KIND_BASE);
            }
            if (!(ent2->getDirectAttributes().empty()
                  && ent2->getDirectMethods().empty()))
            {
                insert("com.sun.star.uno.RuntimeException", KIND_EXCEPTION);
            }
            for (const unoidl::InterfaceTypeEntity::Attribute& attr : ent2->getDirectAttributes())
            {
                insert(attr.type, KIND_NORMAL);
                for (const OUString& ex : attr.getExceptions)
                {
                    insert(ex, KIND_EXCEPTION);
                }
                for (const OUString& ex : attr.setExceptions)
                {
                    insert(ex, KIND_EXCEPTION);
                }
            }
            for (const unoidl::InterfaceTypeEntity::Method& method : ent2->getDirectMethods())
            {
                insert(method.returnType, KIND_NORMAL);
                for (const unoidl::InterfaceTypeEntity::Method::Parameter& param : method.parameters)
                {
                    insert(param.type, KIND_NORMAL);
                }
                for (const OUString& ex : method.exceptions)
                {
                    insert(ex, KIND_EXCEPTION);
                }
            }
            break;
        }
    case UnoType::Sort::Typedef:
        insert(
            static_cast< unoidl::TypedefEntity * >(ent.get())->getType(),
            KIND_NORMAL);
        break;
    case UnoType::Sort::ConstantGroup:
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
    case UnoType::Sort::SingleInterfaceBasedService:
        {
            rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > ent2(
                static_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                    ent.get()));
            if (!ent2->getConstructors().empty()) {
                insert(ent2->getBase(), KIND_NORMAL);
            }
            for (const unoidl::SingleInterfaceBasedServiceEntity::Constructor& cons : ent2->getConstructors())
            {
                for (const unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter& param
                         : cons.parameters)
                {
                    insert(param.type, KIND_NORMAL);
                    if (param.rest) {
                        m_sequenceDependency = true;
                    }
                }
                for (const OUString& ex : cons.exceptions)
                {
                    insert(ex, KIND_EXCEPTION);
                }
            }
            break;
        }
    case UnoType::Sort::InterfaceBasedSingleton:
        insert(
            static_cast< unoidl::InterfaceBasedSingletonEntity * >(ent.get())->
                getBase(),
            KIND_NORMAL);
        break;
    default:
        assert(false); // this cannot happen
    }
}

Dependencies::~Dependencies() {}

void Dependencies::insert(OUString const & name, Kind kind) {
    sal_Int32 k;
    std::vector< OString > args;
    OUString n(b2u(UnoType::decompose(u2b(name), &k, &args)));
    if (k != 0) {
        m_sequenceDependency = true;
    }
    switch (m_manager->getSort(n)) {
    case UnoType::Sort::Void:
        m_voidDependency = true;
        break;
    case UnoType::Sort::Boolean:
        m_booleanDependency = true;
        break;
    case UnoType::Sort::Byte:
        m_byteDependency = true;
        break;
    case UnoType::Sort::Short:
        m_shortDependency = true;
        break;
    case UnoType::Sort::UnsignedShort:
        m_unsignedShortDependency = true;
        break;
    case UnoType::Sort::Long:
        m_longDependency = true;
        break;
    case UnoType::Sort::UnsignedLong:
        m_unsignedLongDependency = true;
        break;
    case UnoType::Sort::Hyper:
        m_hyperDependency = true;
        break;
    case UnoType::Sort::UnsignedHyper:
        m_unsignedHyperDependency = true;
        break;
    case UnoType::Sort::Float:
        m_floatDependency = true;
        break;
    case UnoType::Sort::Double:
        m_doubleDependency = true;
        break;
    case UnoType::Sort::Char:
        m_charDependency = true;
        break;
    case UnoType::Sort::String:
        m_stringDependency = true;
        break;
    case UnoType::Sort::Type:
        m_typeDependency = true;
        break;
    case UnoType::Sort::Any:
        m_anyDependency = true;
        break;
    case UnoType::Sort::PolymorphicStructTemplate:
        for (const OString& arg : args)
        {
            insert(b2u(arg), KIND_NORMAL);
        }
        SAL_FALLTHROUGH;
    case UnoType::Sort::Sequence:
    case UnoType::Sort::Enum:
    case UnoType::Sort::PlainStruct:
    case UnoType::Sort::Exception:
    case UnoType::Sort::Interface:
    case UnoType::Sort::Typedef:
        {
            std::pair< Map::iterator, bool > i(
                m_map.insert(
                    Map::value_type(n, kind)));
            if (!i.second && kind == KIND_BASE) {
                assert(i.first->second != KIND_EXCEPTION);
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
