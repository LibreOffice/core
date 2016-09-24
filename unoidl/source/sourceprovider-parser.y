/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*TODO: check Exception, RuntimeException, XInterface defns */

%locations
%pure-parser

%{

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#include <utility>
#include <vector>

#include <sourceprovider-parser-requires.hxx>

%}

%union {
    sal_uInt64 ival;
    double fval;
    OString * sval;

    bool bval;
    std::vector<OUString> * excns;
    unoidl::detail::SourceProviderAccessDecls decls;
    unoidl::InterfaceTypeEntity::Method::Parameter::Direction dir;
    unoidl::detail::SourceProviderFlags flags;
    unoidl::detail::SourceProviderExpr expr;
    unoidl::detail::SourceProviderType * type;
    std::vector<unoidl::detail::SourceProviderType> * types;
}

/* TODO: %destructor { delete $$; } <sval> <excns> <type> <types> */

%lex-param {yyscan_t yyscanner}
%parse-param {yyscan_t yyscanner}

%{

#include "osl/file.h"
#include "osl/thread.h"
#include "sal/log.hxx"

#include "sourceprovider-scanner.hxx"

#define YYLLOC_DEFAULT(Current, Rhs, N) \
    do { (Current) = YYRHSLOC((Rhs), (N) ? 1 : 0); } while (0)

void yyerror(YYLTYPE * locp, yyscan_t yyscanner, char const * msg) {
    assert(locp != nullptr);
    unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
    data->errorLine = *locp;
    data->parserError = OString(msg);
}

namespace {

void error(YYLTYPE location, yyscan_t yyscanner, OUString const & message) {
    unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
    data->errorLine = location;
    data->errorMessage = message;
}

OUString flagName(unoidl::detail::SourceProviderFlags flag) {
    switch (flag) {
    case unoidl::detail::FLAG_ATTRIBUTE:
        return OUString("attribute");
    case unoidl::detail::FLAG_BOUND:
        return OUString("bound");
    case unoidl::detail::FLAG_CONSTRAINED:
        return OUString("constrained");
    case unoidl::detail::FLAG_MAYBEAMBIGUOUS:
        return OUString("maybeambiguous");
    case unoidl::detail::FLAG_MAYBEDEFAULT:
        return OUString("maybedefault");
    case unoidl::detail::FLAG_MAYBEVOID:
        return OUString("maybevoid");
    case unoidl::detail::FLAG_OPTIONAL:
        return OUString("optional");
    case unoidl::detail::FLAG_PROPERTY:
        return OUString("property");
    case unoidl::detail::FLAG_READONLY:
        return OUString("readonly");
    case unoidl::detail::FLAG_REMOVABLE:
        return OUString("removable");
    case unoidl::detail::FLAG_TRANSIENT:
        return OUString("transient");
    default:
        assert(false && "this cannot happen"); for (;;) { std::abort(); }
    }
}

OUString convertName(OString const * name) {
    assert(name != nullptr);
    OUString s(OStringToOUString(*name, RTL_TEXTENCODING_ASCII_US));
    delete name;
    return s;
}

OUString convertToFullName(
    unoidl::detail::SourceProviderScannerData const * data,
    OString const * identifier)
{
    assert(data != nullptr);
    OUString pref;
    if (!data->modules.empty()) {
        pref = data->modules.back() + ".";
    }
    return pref + convertName(identifier);
}

void convertToCurrentName(
    unoidl::detail::SourceProviderScannerData * data,
    OString const * identifier)
{
    assert(data != nullptr);
    assert(data->currentName.isEmpty());
    data->currentName = convertToFullName(data, identifier);
    assert(!data->currentName.isEmpty());
}

void clearCurrentState(unoidl::detail::SourceProviderScannerData * data) {
    assert(data != nullptr);
    data->currentName.clear();
    data->publishedContext = false;
}

unoidl::detail::SourceProviderEntity * getCurrentEntity(
    unoidl::detail::SourceProviderScannerData * data)
{
    assert(data != nullptr);
    assert(!data->currentName.isEmpty());
    std::map<OUString, unoidl::detail::SourceProviderEntity>::iterator i(
        data->entities.find(data->currentName));
    assert(i != data->entities.end());
    assert(i->second.kind == unoidl::detail::SourceProviderEntity::KIND_LOCAL);
    assert(i->second.pad.is());
    return &i->second;
}

template<typename T> rtl::Reference<T> getCurrentPad(
    unoidl::detail::SourceProviderScannerData * data)
{
    rtl::Reference<T> pad(dynamic_cast<T *>(getCurrentEntity(data)->pad.get()));
    assert(pad.is());
    return pad;
}

bool nameHasSameIdentifierAs(OUString const & name, OUString const & identifier)
{
    sal_Int32 i = name.lastIndexOf('.') + 1;
    return identifier.getLength() == name.getLength() - i
        && name.match(identifier, i);
}

bool coerce(
    YYLTYPE location, yyscan_t yyscanner,
    unoidl::detail::SourceProviderExpr * lhs,
    unoidl::detail::SourceProviderExpr * rhs)
{
    assert(lhs != nullptr);
    assert(rhs != nullptr);
    bool ok = bool(); // avoid warnings
    switch (lhs->type) {
    case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
        ok = rhs->type != unoidl::detail::SourceProviderExpr::TYPE_BOOL;
        break;
    case unoidl::detail::SourceProviderExpr::TYPE_INT:
        switch (rhs->type) {
        case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
            ok = false;
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_INT:
            ok = true;
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_UINT:
            if (lhs->ival >= 0) {
                lhs->type = unoidl::detail::SourceProviderExpr::TYPE_UINT;
                ok = true;
            } else if (rhs->uval <= SAL_MAX_INT64) {
                rhs->type = unoidl::detail::SourceProviderExpr::TYPE_INT;
                ok = true;
            } else {
                ok = false;
            }
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
            lhs->fval = lhs->ival;
            ok = true;
            break;
        }
        break;
    case unoidl::detail::SourceProviderExpr::TYPE_UINT:
        switch (rhs->type) {
        case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
            ok = false;
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_INT:
            if (rhs->ival >= 0) {
                rhs->type = unoidl::detail::SourceProviderExpr::TYPE_UINT;
                ok = true;
            } else if (lhs->uval <= SAL_MAX_INT64) {
                lhs->type = unoidl::detail::SourceProviderExpr::TYPE_INT;
                ok = true;
            } else {
                ok = false;
            }
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_UINT:
            ok = true;
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
            lhs->fval = lhs->uval;
            ok = true;
            break;
        }
        break;
    case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
        switch (rhs->type) {
        case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
            ok = false;
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_INT:
            rhs->fval = rhs->ival;
            ok = true;
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_UINT:
            rhs->fval = rhs->uval;
            ok = true;
            break;
        case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
            ok = true;
            break;
        }
        break;
    }
    if (!ok) {
        error(location, yyscanner, "cannot coerce binary expression arguments");
    }
    return ok;
}

unoidl::detail::SourceProviderEntity * findEntity_(
    unoidl::detail::SourceProviderScannerData * data, OUString * name)
{
    assert(data != nullptr);
    assert(name != nullptr);
    OUString n;
    if (!name->startsWith(".", &n)) {
        for (auto i(data->modules.rbegin()); i != data->modules.rend(); ++i) {
            n = *i + "." + *name;
            std::map<OUString, unoidl::detail::SourceProviderEntity>::iterator j(
                data->entities.find(n));
            if (j != data->entities.end()) {
                *name = n;
                return &j->second;
            }
            rtl::Reference<unoidl::Entity> ent(data->manager->findEntity(n));
            if (ent.is()) {
                std::map<OUString, unoidl::detail::SourceProviderEntity>::iterator
                    k(data->entities.insert(
                          std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                              n,
                              unoidl::detail::SourceProviderEntity(
                                  unoidl::detail::SourceProviderEntity::KIND_EXTERNAL,
                                  ent))).
                      first);
                *name = n;
                return &k->second;
            }
        }
        n = *name;
    }
    std::map<OUString, unoidl::detail::SourceProviderEntity>::iterator i(
        data->entities.find(n));
    if (i != data->entities.end()) {
        *name = n;
        return &i->second;
    }
    rtl::Reference<unoidl::Entity> ent(data->manager->findEntity(n));
    if (ent.is()) {
        std::map<OUString, unoidl::detail::SourceProviderEntity>::iterator
            j(data->entities.insert(
                  std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                      n,
                      unoidl::detail::SourceProviderEntity(
                          unoidl::detail::SourceProviderEntity::KIND_EXTERNAL,
                          ent))).
              first);
        *name = n;
        return &j->second;
    }
    return nullptr;
}

enum Found { FOUND_ERROR, FOUND_TYPE, FOUND_ENTITY };

Found findEntity(
    YYLTYPE location, yyscan_t yyscanner,
    unoidl::detail::SourceProviderScannerData * data,
    bool resolveInterfaceDefinitions, OUString * name,
    unoidl::detail::SourceProviderEntity const ** entity, bool * typedefed,
    unoidl::detail::SourceProviderType * typedefedType)
{
    //TODO: avoid recursion
    assert(data != nullptr);
    assert(name != nullptr);
    assert(entity != nullptr);
    unoidl::detail::SourceProviderEntity * e = findEntity_(data, name);
    OUString n(*name);
    OUString typeNucleus;
    std::size_t rank = 0;
    std::vector<unoidl::detail::SourceProviderType> args;
    for (;;) {
        if (e != nullptr) {
            switch (e->kind) {
            case unoidl::detail::SourceProviderEntity::KIND_LOCAL:
                if (e->pad.is()) {
                    break;
                }
                assert(e->entity.is());
                // fall through
            case unoidl::detail::SourceProviderEntity::KIND_EXTERNAL:
                if (e->entity->getSort() == unoidl::Entity::SORT_TYPEDEF) {
                    if (typedefed != nullptr) {
                        *typedefed = true;
                    }
                    if (data->publishedContext
                        && !static_cast<unoidl::TypedefEntity *>(
                            e->entity.get())->isPublished())
                    {
                        error(
                            location, yyscanner,
                            ("type " + *name + " based on unpublished typedef "
                             + n + " used in published context"));
                        return FOUND_ERROR;
                    }
                    OUString t(
                        static_cast<unoidl::TypedefEntity *>(e->entity.get())
                        ->getType());
                    typeNucleus = t;
                    while (typeNucleus.startsWith("[]", &typeNucleus)) {
                        if (!args.empty()) {
                            error(
                                location, yyscanner,
                                ("inconsistent type manager: bad type " + *name
                                 + (" based on instantiated polymorphic struct"
                                    " type based on sequence type named ")
                                 + t));
                            return FOUND_ERROR;
                        }
                        if (rank == std::numeric_limits<std::size_t>::max()) {
                            error(
                                location, yyscanner,
                                ("bad type " + *name
                                 + " based on sequence type of too high rank"));
                            return FOUND_ERROR;
                        }
                        ++rank;
                    }
                    sal_Int32 i = typeNucleus.indexOf('<');
                    if (i != -1) {
                        if (!args.empty()) {
                            error(
                                location, yyscanner,
                                ("inconsistent type manager: bad type " + *name
                                 + (" based on instantiated polymorphic struct"
                                    " type based on instantiated polymorphic"
                                    " struct type named ")
                                 + t));
                            return FOUND_ERROR;
                        }
                        OUString tmpl(typeNucleus.copy(0, i));
                        do {
                            ++i; // skip '<' or ','
                            sal_Int32 j = i;
                            for (sal_Int32 level = 0;
                                 j != typeNucleus.getLength(); ++j)
                            {
                                sal_Unicode c = typeNucleus[j];
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
                            if (j != typeNucleus.getLength()) {
                                OUString argName(typeNucleus.copy(i, j - i));
                                unoidl::detail::SourceProviderEntity const *
                                    argEnt;
                                unoidl::detail::SourceProviderType argType;
                                switch (
                                    findEntity(
                                        location, yyscanner, data, false,
                                        &argName, &argEnt, nullptr, &argType))
                                {
                                case FOUND_ERROR:
                                    return FOUND_ERROR;
                                case FOUND_TYPE:
                                    break;
                                case FOUND_ENTITY:
                                    if (argEnt == nullptr) {
                                        error(
                                            location, yyscanner,
                                            (("inconsistent type manager: bad"
                                              " instantiated polymorphic struct"
                                              " type template type argument ")
                                             + argName));
                                        return FOUND_ERROR;
                                    } else {
                                        unoidl::detail::SourceProviderType::Type
                                            argT
                                            = unoidl::detail::SourceProviderType::Type();
                                            // avoid warnings
                                        switch (argEnt->kind) {
                                        case unoidl::detail::SourceProviderEntity::KIND_LOCAL:
                                            if (e->pad.is()) {
                                                error(
                                                    location, yyscanner,
                                                    (("inconsistent type"
                                                      " manager: bad"
                                                      " instantiated"
                                                      " polymorphic struct type"
                                                      " template type"
                                                      " argument ")
                                                     + argName));
                                                return FOUND_ERROR;
                                            }
                                            assert(e->entity.is());
                                            // fall through
                                        case unoidl::detail::SourceProviderEntity::KIND_EXTERNAL:
                                            switch (e->entity->getSort()) {
                                            case unoidl::Entity::SORT_ENUM_TYPE:
                                                argT = unoidl::detail::SourceProviderType::TYPE_ENUM;
                                                break;
                                            case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
                                                argT = unoidl::detail::SourceProviderType::TYPE_PLAIN_STRUCT;
                                                break;
                                            case unoidl::Entity::SORT_INTERFACE_TYPE:
                                                argT = unoidl::detail::SourceProviderType::TYPE_INTERFACE;
                                                break;
                                            default:
                                                error(
                                                    location, yyscanner,
                                                    (("inconsistent type"
                                                      "manager: bad"
                                                      " instantiated"
                                                      " polymorphic struct type"
                                                      " template type"
                                                      " argument ")
                                                     + argName));
                                                return FOUND_ERROR;
                                            }
                                            break;
                                        case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
                                        case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
                                            argT = unoidl::detail::SourceProviderType::TYPE_INTERFACE;
                                            break;
                                        case unoidl::detail::SourceProviderEntity::KIND_MODULE:
                                            assert(false && "this cannot happen");
                                        }
                                        argType
                                            = unoidl::detail::SourceProviderType(
                                                argT, argName, argEnt);
                                    }
                                    break;
                                }
                                args.push_back(argType);
                            }
                            i = j;
                        } while (i != typeNucleus.getLength()
                                 && typeNucleus[i] != '>');
                        if (i != typeNucleus.getLength() - 1
                            || typeNucleus[i] != '>')
                        {
                            error(
                                location, yyscanner,
                                ("inconsistent type manager: bad type name \""
                                 + t + "\""));
                            return FOUND_ERROR;
                        }
                        assert(!args.empty());
                        typeNucleus = tmpl;
                    }
                    if (typeNucleus.isEmpty()) {
                        error(
                            location, yyscanner,
                            ("inconsistent type manager: bad type name \"" + t
                             + "\""));
                        return FOUND_ERROR;
                    }
                    if (typeNucleus == "void") {
                        error(
                            location, yyscanner,
                            ("inconsistent type manager: bad type " + *name
                             + " based on void"));
                        return FOUND_ERROR;
                    }
                    if (typeNucleus == "boolean" || typeNucleus == "byte"
                        || typeNucleus == "short"
                        || typeNucleus == "unsigned short"
                        || typeNucleus == "long"
                        || typeNucleus == "unsigned long"
                        || typeNucleus == "hyper"
                        || typeNucleus == "unsigned hyper"
                        || typeNucleus == "float" || typeNucleus == "double"
                        || typeNucleus == "char" || typeNucleus == "string"
                        || typeNucleus == "type" || typeNucleus == "any")
                    {
                        if (!args.empty()) {
                            error(
                                location, yyscanner,
                                ("inconsistent type manager: bad type " + *name
                                 + (" based on instantiated polymorphic struct"
                                    " type based on ")
                                 + typeNucleus));
                            return FOUND_ERROR;
                        }
                        break;
                    }
                    n = "." + typeNucleus;
                    typeNucleus.clear();
                    e = findEntity_(data, &n);
                    continue;
                }
                break;
            case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
            case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
                if (resolveInterfaceDefinitions) {
                    rtl::Reference<unoidl::Entity> ent(
                        data->manager->findEntity(n));
                    // Do not allow ent to be of SORT_TYPEDEF:
                    if (!ent.is()
                        || (ent->getSort()
                            != unoidl::Entity::SORT_INTERFACE_TYPE))
                    {
                        error(
                            location, yyscanner,
                            (*name + " is based on interface declaration " + n
                             + " that is not an interface type entity"));
                        return FOUND_ERROR;
                    }
                    e->kind
                        = unoidl::detail::SourceProviderEntity::KIND_EXTERNAL;
                    e->entity = ent;
                }
                break;
            case unoidl::detail::SourceProviderEntity::KIND_MODULE:
                error(
                    location, yyscanner,
                    *name + " is based on module entity " + n);
                return FOUND_ERROR;
            }
        }
        if (!typeNucleus.isEmpty() || rank != 0 || !args.empty()) {
            if (typeNucleus.isEmpty() && e == nullptr) {
                // Found a type name based on an unknown entity:
                *entity = nullptr;
                return FOUND_ENTITY;
            }
            unoidl::detail::SourceProviderType t;
            if (args.empty()) {
                if (typeNucleus == "boolean") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_BOOLEAN);
                } else if (typeNucleus == "byte") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_BYTE);
                } else if (typeNucleus == "short") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_SHORT);
                } else if (typeNucleus == "unsigned short") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_UNSIGNED_SHORT);
                } else if (typeNucleus == "long") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_LONG);
                } else if (typeNucleus == "unsigned long") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_UNSIGNED_LONG);
                } else if (typeNucleus == "hyper") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_HYPER);
                } else if (typeNucleus == "unsigned hyper") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_UNSIGNED_HYPER);
                } else if (typeNucleus == "float") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_FLOAT);
                } else if (typeNucleus == "double") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_DOUBLE);
                } else if (typeNucleus == "char") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_CHAR);
                } else if (typeNucleus == "string") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_STRING);
                } else if (typeNucleus == "type") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_TYPE);
                } else if (typeNucleus == "any") {
                    t = unoidl::detail::SourceProviderType(
                        unoidl::detail::SourceProviderType::TYPE_ANY);
                } else {
                    assert(typeNucleus.isEmpty());
                    assert(e != nullptr);
                    switch (e->kind) {
                    case unoidl::detail::SourceProviderEntity::KIND_LOCAL:
                        if (e->pad.is()) {
                            if (dynamic_cast<unoidl::detail::SourceProviderEnumTypeEntityPad *>(
                                    e->pad.get())
                                != nullptr)
                            {
                                t = unoidl::detail::SourceProviderType(
                                    unoidl::detail::SourceProviderType::TYPE_ENUM,
                                    n, e);
                            } else if (dynamic_cast<unoidl::detail::SourceProviderPlainStructTypeEntityPad *>(
                                           e->pad.get())
                                       != nullptr)
                            {
                                t = unoidl::detail::SourceProviderType(
                                    unoidl::detail::SourceProviderType::TYPE_PLAIN_STRUCT,
                                    n, e);
                            } else if (dynamic_cast<unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *>(
                                           e->pad.get())
                                       != nullptr)
                            {
                                error(
                                    location, yyscanner,
                                    ("bad type " + *name
                                     + (" based on recursive reference to"
                                        " polymorphic struct type template ")
                                     + n));
                                return FOUND_ERROR;
                            } else if (dynamic_cast<unoidl::detail::SourceProviderExceptionTypeEntityPad *>(
                                           e->pad.get())
                                       != nullptr)
                            {
                                t = unoidl::detail::SourceProviderType(
                                    unoidl::detail::SourceProviderType::TYPE_EXCEPTION,
                                    n, e);
                            } else if (dynamic_cast<unoidl::detail::SourceProviderInterfaceTypeEntityPad *>(
                                           e->pad.get())
                                       != nullptr)
                            {
                                t = unoidl::detail::SourceProviderType(
                                    unoidl::detail::SourceProviderType::TYPE_INTERFACE,
                                    n, e);
                            } else {
                                error(
                                    location, yyscanner,
                                    ("bad type " + *name
                                     + " based on non-type entity " + n));
                                return FOUND_ERROR;
                            }
                            break;
                        }
                        assert(e->entity.is());
                        // fall through
                    case unoidl::detail::SourceProviderEntity::KIND_EXTERNAL:
                        switch (e->entity->getSort()) {
                        case unoidl::Entity::SORT_ENUM_TYPE:
                            t = unoidl::detail::SourceProviderType(
                                unoidl::detail::SourceProviderType::TYPE_ENUM,
                                n, e);
                            break;
                        case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
                            t = unoidl::detail::SourceProviderType(
                                unoidl::detail::SourceProviderType::TYPE_PLAIN_STRUCT,
                                n, e);
                            break;
                        case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
                            error(
                                location, yyscanner,
                                ("bad type " + *name
                                 + " based on polymorphic struct type template "
                                 + n + " without type arguments"));
                            return FOUND_ERROR;
                        case unoidl::Entity::SORT_EXCEPTION_TYPE:
                            t = unoidl::detail::SourceProviderType(
                                unoidl::detail::SourceProviderType::TYPE_EXCEPTION,
                                n, e);
                            break;
                        case unoidl::Entity::SORT_INTERFACE_TYPE:
                            t = unoidl::detail::SourceProviderType(
                                unoidl::detail::SourceProviderType::TYPE_INTERFACE,
                                n, e);
                            break;
                        default:
                            error(
                                location, yyscanner,
                                ("bad type " + *name
                                 + " based on non-type entity " + n));
                            return FOUND_ERROR;
                        }
                        break;
                    case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
                    case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
                        t = unoidl::detail::SourceProviderType(
                            unoidl::detail::SourceProviderType::TYPE_INTERFACE,
                            n, e);
                        break;
                    case unoidl::detail::SourceProviderEntity::KIND_MODULE:
                        assert(false && "this cannot happen");
                    }
                }
            } else {
                assert(typeNucleus.isEmpty());
                assert(e != nullptr);
                switch (e->kind) {
                case unoidl::detail::SourceProviderEntity::KIND_LOCAL:
                    if (e->pad.is()) {
                        error(
                            location, yyscanner,
                            ("bad type " + *name
                             + (" based on instantiated polymorphic struct type"
                                " based on ")
                             + n
                             + (" that is either not a polymorphic struct type"
                                " template or a recursive reference to a"
                                " polymorphic struct type template")));
                        return FOUND_ERROR;
                    }
                    assert(e->entity.is());
                    // fall through
                case unoidl::detail::SourceProviderEntity::KIND_EXTERNAL:
                    if (e->entity->getSort()
                        == unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE)
                    {
                        if (args.size()
                            != (static_cast<
                                    unoidl::PolymorphicStructTypeTemplateEntity *>(
                                        e->entity.get())
                                ->getTypeParameters().size()))
                        {
                            error(
                                location, yyscanner,
                                ("bad type " + *name
                                 + (" based on instantiated polymorphic struct"
                                    " type with ")
                                 + OUString::number(args.size())
                                 + (" type arguments based on polymorphic"
                                    " struct type template ")
                                 + n + " with "
                                 + OUString::number(
                                     static_cast<
                                         unoidl::PolymorphicStructTypeTemplateEntity *>(
                                             e->entity.get())
                                     ->getTypeParameters().size())
                                 + " type paramters"));
                            return FOUND_ERROR;
                        }
                        t = unoidl::detail::SourceProviderType(n, e, args);
                        break;
                    }
                    // fall through
                case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
                case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
                    error(
                        location, yyscanner,
                        ("bad type " + *name
                         + (" based on instantiated polymorphic struct type"
                            " based on ")
                         + n
                         + " that is not a polymorphic struct type template"));
                    return FOUND_ERROR;
                case unoidl::detail::SourceProviderEntity::KIND_MODULE:
                    assert(false && "this cannot happen");
                }
            }
            if (typedefedType != nullptr) {
                for (std::size_t i = 0; i != rank; ++i) {
                    t = unoidl::detail::SourceProviderType(&t);
                }
                *typedefedType = t;
                typedefedType->typedefName = *name;
            }
            *entity = nullptr;
            return FOUND_TYPE;
        }
        *entity = e;
        return FOUND_ENTITY;
    }
}


bool checkTypeArgument(
    YYLTYPE location, yyscan_t yyscanner,
    unoidl::detail::SourceProviderType const & type)
{
    switch (type.type) {
    case unoidl::detail::SourceProviderType::TYPE_VOID:
    case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_SHORT:
    case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_LONG:
    case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_HYPER:
    case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
    case unoidl::detail::SourceProviderType::TYPE_PARAMETER: //TODO?
        error(
            location, yyscanner,
            "bad instantiated polymorphic struct type argument");
        return false;
    case unoidl::detail::SourceProviderType::TYPE_SEQUENCE:
        return checkTypeArgument(location, yyscanner, type.subtypes.front());
    default:
        return true;
    }
}

bool checkInstantiatedPolymorphicStructTypeArgument(
    unoidl::detail::SourceProviderType const & type, OUString const & name)
{
    if (type.type
        == unoidl::detail::SourceProviderType::TYPE_INSTANTIATED_POLYMORPHIC_STRUCT)
    {
        for (auto & i: type.subtypes) {
            if (checkInstantiatedPolymorphicStructTypeArgument(i, name)
                || i.getName() == name) // no need to worry about typedef
            {
                return true;
            }
        }
    }
    return false;
}

std::vector<OUString> annotations(bool deprecated) {
    std::vector<OUString> ann;
    if (deprecated) {
        ann.push_back("deprecated");
    }
    return ann;
}

}

%}

%token TOK_ELLIPSIS
%token TOK_COLONS
%token TOK_LEFTSHIFT
%token TOK_RIGHTSHIFT

%token TOK_FALSE
%token TOK_TRUE
%token TOK_ANY
%token TOK_ATTRIBUTE
%token TOK_BOOLEAN
%token TOK_BOUND
%token TOK_BYTE
%token TOK_CHAR
%token TOK_CONST
%token TOK_CONSTANTS
%token TOK_CONSTRAINED
%token TOK_DOUBLE
%token TOK_ENUM
%token TOK_EXCEPTION
%token TOK_FLOAT
%token TOK_GET
%token TOK_HYPER
%token TOK_IN
%token TOK_INOUT
%token TOK_INTERFACE
%token TOK_LONG
%token TOK_MAYBEAMBIGUOUS
%token TOK_MAYBEDEFAULT
%token TOK_MAYBEVOID
%token TOK_MODULE
%token TOK_OPTIONAL
%token TOK_OUT
%token TOK_PROPERTY
%token TOK_PUBLISHED
%token TOK_RAISES
%token TOK_READONLY
%token TOK_REMOVABLE
%token TOK_SEQUENCE
%token TOK_SERVICE
%token TOK_SET
%token TOK_SHORT
%token TOK_SINGLETON
%token TOK_STRING
%token TOK_STRUCT
%token TOK_TRANSIENT
%token TOK_TYPE
%token TOK_TYPEDEF
%token TOK_UNSIGNED
%token TOK_VOID

%token<sval> TOK_IDENTIFIER
%token<ival> TOK_INTEGER
%token<fval> TOK_FLOATING

%token TOK_DEPRECATED

%token TOK_ERROR

%type<sval> identifier name singleInheritance singleInheritance_opt
%type<bval> ctors_opt deprecated_opt ellipsis_opt published_opt
%type<decls> attributeAccessDecl attributeAccessDecls
%type<dir> direction
%type<excns> exceptionSpec exceptionSpec_opt exceptions
%type<flags> flag flagSection flagSection_opt flags
%type<expr> addExpr andExpr expr multExpr orExpr primaryExpr shiftExpr unaryExpr
  xorExpr
%type<type> type
%type<types> typeArguments

%initial-action { yylloc = 1; }

%%

definitions:
  definitions definition
| /* empty */
;

definition:
  moduleDecl
| enumDefn
| plainStructDefn
| polymorphicStructTemplateDefn
| exceptionDefn
| interfaceDefn
| typedefDefn
| constantGroupDefn
| singleInterfaceBasedServiceDefn
| accumulationBasedServiceDefn
| interfaceBasedSingletonDefn
| serviceBasedSingletonDefn
| interfaceDecl
;

moduleDecl:
  TOK_MODULE identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString name(convertToFullName(data, $2));
      data->modules.push_back(name);
      std::pair<std::map<OUString, unoidl::detail::SourceProviderEntity>::iterator, bool> p(
          data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  name,
                  unoidl::detail::SourceProviderEntity(
                      unoidl::detail::SourceProviderEntity::KIND_MODULE))));
      if (!p.second
          && (p.first->second.kind
              != unoidl::detail::SourceProviderEntity::KIND_MODULE))
      {
          error(@2, yyscanner, "multiple entities named " + name);
          YYERROR;
      }
  }
  '{' definitions '}' ';' { yyget_extra(yyscanner)->modules.pop_back(); }
;

enumDefn:
  deprecated_opt published_opt TOK_ENUM identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      convertToCurrentName(data, $4);
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  data->currentName,
                  unoidl::detail::SourceProviderEntity(
                      new unoidl::detail::SourceProviderEnumTypeEntityPad(
                          $2)))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + data->currentName);
          YYERROR;
      }
  }
  '{' enumMembers '}' ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderEnumTypeEntityPad * pad =
          dynamic_cast<unoidl::detail::SourceProviderEnumTypeEntityPad *>(
              ent->pad.get());
      assert(pad != nullptr);
      ent->entity = new unoidl::EnumTypeEntity(
          pad->isPublished(), pad->members, annotations($1));
      ent->pad.clear();
      clearCurrentState(data);
  }
;

enumMembers:
| enumMembers ',' enumMember
| enumMember
;

enumMember:
  deprecated_opt identifier
  {
      OUString id(convertName($2));
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      rtl::Reference<unoidl::detail::SourceProviderEnumTypeEntityPad> pad(
          getCurrentPad<unoidl::detail::SourceProviderEnumTypeEntityPad>(data));
      sal_Int32 v;
      if (pad->members.empty()) {
          v = 0;
      } else {
          v = pad->members.back().value;
          if (v == SAL_MAX_INT32) {
              error(
                  @2, yyscanner,
                  ("enum " + data->currentName + " member " + id
                   + " would have out-of-range value 2^31"));
              YYERROR;
          }
          ++v;
      }
      pad->members.emplace_back(id, v, annotations($1));
  }
| deprecated_opt identifier '=' expr
  {
      OUString id(convertName($2));
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      rtl::Reference<unoidl::detail::SourceProviderEnumTypeEntityPad> pad(
          getCurrentPad<unoidl::detail::SourceProviderEnumTypeEntityPad>(data));
      sal_Int32 v;
      switch ($4.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          if ($4.ival < SAL_MIN_INT32 || $4.ival > SAL_MAX_INT32) {
              error(
                  @4, yyscanner,
                  ("out-of-range enum " + data->currentName + " member " + id
                   + " value " + OUString::number($4.ival)));
              YYERROR;
          }
          v = static_cast<sal_Int32>($4.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          if ($4.uval > SAL_MAX_INT32) {
              error(
                  @4, yyscanner,
                  ("out-of-range enum " + data->currentName + " member " + id
                   + " value " + OUString::number($4.uval)));
              YYERROR;
          }
          v = static_cast<sal_Int32>($4.uval);
          break;
      default:
          error(
              @4, yyscanner,
              ("non-integer enum " + data->currentName + " member " + id
               + " value"));
          YYERROR;
          break;
      }
      pad->members.emplace_back(id, v, annotations($1));
  }
;

plainStructDefn:
  deprecated_opt published_opt TOK_STRUCT identifier singleInheritance_opt
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      convertToCurrentName(data, $4);
      OUString baseName;
      rtl::Reference<unoidl::PlainStructTypeEntity> baseEnt;
      if ($5 != nullptr) {
          baseName = convertName($5);
          unoidl::detail::SourceProviderEntity const * p;
          if (findEntity(
                  @5, yyscanner, data, false, &baseName, &p, nullptr, nullptr)
              == FOUND_ERROR)
          {
              YYERROR;
          }
          if (p == nullptr || !p->entity.is()
              || p->entity->getSort() != unoidl::Entity::SORT_PLAIN_STRUCT_TYPE)
          {
              error(
                  @5, yyscanner,
                  ("plain struct type " + data->currentName + " base "
                   + baseName
                   + " does not resolve to an existing plain struct type"));
              YYERROR;
          }
          baseEnt = static_cast<unoidl::PlainStructTypeEntity *>(
              p->entity.get());
          if ($2 && !baseEnt->isPublished()) {
              error(
                  @5, yyscanner,
                  ("published plain struct type " + data->currentName + " base "
                   + baseName + " is unpublished"));
              YYERROR;
          }
      }
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  data->currentName,
                  unoidl::detail::SourceProviderEntity(
                      new unoidl::detail::SourceProviderPlainStructTypeEntityPad(
                          $2, baseName, baseEnt)))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + data->currentName);
          YYERROR;
      }
  }
  '{' structMembers '}' ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderPlainStructTypeEntityPad * pad =
          dynamic_cast<
              unoidl::detail::SourceProviderPlainStructTypeEntityPad *>(
                  ent->pad.get());
      assert(pad != nullptr);
      ent->entity = new unoidl::PlainStructTypeEntity(
          pad->isPublished(), pad->baseName, pad->members, annotations($1));
      ent->pad.clear();
      clearCurrentState(data);
  }
;

polymorphicStructTemplateDefn:
  deprecated_opt published_opt TOK_STRUCT identifier '<'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      convertToCurrentName(data, $4);
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  data->currentName,
                  unoidl::detail::SourceProviderEntity(
                      new unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad(
                          $2)))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + data->currentName);
          YYERROR;
      }
  }
  typeParameters '>' '{' structMembers '}' ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *
          pad = dynamic_cast<
              unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *>(
                  ent->pad.get());
      assert(pad != nullptr);
      ent->entity = new unoidl::PolymorphicStructTypeTemplateEntity(
          pad->isPublished(), pad->typeParameters, pad->members,
          annotations($1));
      ent->pad.clear();
      clearCurrentState(data);
  }
;

typeParameters:
  typeParameters ',' identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      rtl::Reference<unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad>(
                  data));
      OUString id(convertName($3));
      if (std::find(pad->typeParameters.begin(), pad->typeParameters.end(), id)
          != pad->typeParameters.end())
      {
          error(
              @3, yyscanner,
              ("polymorphic struct type template " + data->currentName
               + " type parameter " + id
               + " has same identifier as another type parameter"));
          YYERROR;
      }
      pad->typeParameters.push_back(id);
  }
| identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      rtl::Reference<unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad>(
                  data));
      OUString id(convertName($1));
      assert(pad->typeParameters.empty());
      pad->typeParameters.push_back(id);
  }
;

exceptionDefn:
  deprecated_opt published_opt TOK_EXCEPTION identifier singleInheritance_opt
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      convertToCurrentName(data, $4);
      OUString baseName;
      rtl::Reference<unoidl::ExceptionTypeEntity> baseEnt;
      if ($5 != nullptr) {
          baseName = convertName($5);
          unoidl::detail::SourceProviderEntity const * p;
          if (findEntity(
                  @5, yyscanner, data, false, &baseName, &p, nullptr, nullptr)
              == FOUND_ERROR)
          {
              YYERROR;
          }
          if (p == nullptr || !p->entity.is()
              || p->entity->getSort() != unoidl::Entity::SORT_EXCEPTION_TYPE)
          {
              error(
                  @5, yyscanner,
                  ("exception type " + data->currentName + " base " + baseName
                   + " does not resolve to an existing exception type"));
              YYERROR;
          }
          baseEnt = static_cast<unoidl::ExceptionTypeEntity *>(
              p->entity.get());
          if ($2 && !baseEnt->isPublished()) {
              error(
                  @5, yyscanner,
                  ("published exception type " + data->currentName + " base "
                   + baseName + " is unpublished"));
              YYERROR;
          }
      }
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  data->currentName,
                  unoidl::detail::SourceProviderEntity(
                      new unoidl::detail::SourceProviderExceptionTypeEntityPad(
                          $2, baseName, baseEnt)))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + data->currentName);
          YYERROR;
      }
  }
 '{' structMembers '}' ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderExceptionTypeEntityPad * pad =
          dynamic_cast<unoidl::detail::SourceProviderExceptionTypeEntityPad *>(
              ent->pad.get());
      assert(pad != nullptr);
      ent->entity = new unoidl::ExceptionTypeEntity(
          pad->isPublished(), pad->baseName, pad->members, annotations($1));
      ent->pad.clear();
      clearCurrentState(data);
  }
;

structMembers:
  structMembers structMember
| /* empty */
;

structMember:
  deprecated_opt type identifier ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderType t(*$2);
      delete $2;
      OUString id(convertName($3));
      switch (t.type) {
      case unoidl::detail::SourceProviderType::TYPE_VOID:
      case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
          error(
              @2, yyscanner,
              ("illegal struct/exception type " + data->currentName
               + " direct member " + id + " type"));
          YYERROR;
          break;
      default:
          break;
      }
      if (t.type != unoidl::detail::SourceProviderType::TYPE_PARAMETER
          && t.getName() == data->currentName) // no need to worry about typedef
      {
          error(
              @2, yyscanner,
              ("struct/exception type " + data->currentName + " direct member "
               + id + " has same type as the type itself"));
          YYERROR;
      }
      if (checkInstantiatedPolymorphicStructTypeArgument(t, data->currentName))
      {
          error(
              @2, yyscanner,
              ("struct/exception type " + data->currentName + " direct member "
               + id
               + (" has instantiated polymorphic struct type that uses the type"
                  " itself as an argument")));
          YYERROR;
      }
      if (nameHasSameIdentifierAs(data->currentName, id)) {
          error(
              @3, yyscanner,
              ("struct/exception type " + data->currentName + " direct member "
               + id + " has same unqualified identifer as the type itself"));
          YYERROR;
      }
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderPlainStructTypeEntityPad * p1 =
          dynamic_cast<unoidl::detail::SourceProviderPlainStructTypeEntityPad *>(
              ent->pad.get());
      if (p1 != nullptr) {
          for (auto & i: p1->members) {
              if (id == i.name) {
                  error(
                      @3, yyscanner,
                      ("plain struct type " + data->currentName
                       + " direct member " + id
                       + " has same identifier as another direct member"));
                  YYERROR;
              }
          }
          if (p1->baseEntity.is()) {
              OUString baseName(p1->baseName);
              for (auto baseEnt(p1->baseEntity);;) {
                  if (nameHasSameIdentifierAs(baseName, id)) {
                      error(
                          @3, yyscanner,
                          ("plain struct type " + data->currentName
                           + " direct member " + id
                           + " has same unqalified identifier as base "
                           + baseName));
                      YYERROR;
                  }
                  for (auto & i: baseEnt->getDirectMembers()) {
                      if (id == i.name) {
                          error(
                              @3, yyscanner,
                              ("plain struct type " + data->currentName
                               + " direct member " + id
                               + " has same identifier as a member of base "
                               + baseName));
                          YYERROR;
                      }
                  }
                  baseName = baseEnt->getDirectBase();
                  if (baseName.isEmpty()) {
                      break;
                  }
                  unoidl::detail::SourceProviderEntity const * p;
                  if (findEntity(
                          @2, yyscanner, data, false, &baseName, &p, nullptr,
                          nullptr)
                      == FOUND_ERROR)
                  {
                      YYERROR;
                  }
                  if (p == nullptr || !p->entity.is()
                      || (p->entity->getSort()
                          != unoidl::Entity::SORT_PLAIN_STRUCT_TYPE))
                  {
                      error(
                          @2, yyscanner,
                          ("inconsistent type manager: plain struct type "
                           + data->currentName + " base " + baseName
                           + (" does not resolve to an existing plain struct"
                              " type")));
                      YYERROR;
                  }
                  baseEnt = static_cast<unoidl::PlainStructTypeEntity *>(
                      p->entity.get());
              }
          }
          p1->members.emplace_back(id, t.getName(), annotations($1));
      } else {
          unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *
              p2 = dynamic_cast<unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *>(
                  ent->pad.get());
          if (p2 != nullptr) {
              for (auto & i: p2->members) {
                  if (id == i.name) {
                      error(
                          @3, yyscanner,
                          ("polymorphic struct type template "
                           + data->currentName + " direct member " + id
                           + " has same identifier as another direct member"));
                      YYERROR;
                  }
              }
              p2->members.emplace_back(
                  id, t.getName(),
                  t.type == unoidl::detail::SourceProviderType::TYPE_PARAMETER,
                  annotations($1));
          } else {
              unoidl::detail::SourceProviderExceptionTypeEntityPad * p3
                  = dynamic_cast<unoidl::detail::SourceProviderExceptionTypeEntityPad *>(
                      ent->pad.get());
              assert(p3 != nullptr);
              for (auto & i: p3->members) {
                  if (id == i.name) {
                      error(
                          @3, yyscanner,
                          ("exception type " + data->currentName
                           + " direct member " + id
                           + " has same identifier as another direct member"));
                      YYERROR;
                  }
              }
              if (p3->baseEntity.is()) {
                  OUString baseName(p3->baseName);
                  for (auto baseEnt(p3->baseEntity);;) {
                      if (nameHasSameIdentifierAs(baseName, id)) {
                          error(
                              @3, yyscanner,
                              ("exception type " + data->currentName
                               + " direct member " + id
                               + " has same unqalified identifier as base "
                               + baseName));
                          YYERROR;
                      }
                      for (auto & i: baseEnt->getDirectMembers()) {
                          if (id == i.name) {
                              error(
                                  @3, yyscanner,
                                  ("exception type " + data->currentName
                                   + " direct member " + id
                                   + " has same identifier as a member of base "
                                   + baseName));
                              YYERROR;
                          }
                      }
                      baseName = baseEnt->getDirectBase();
                      if (baseName.isEmpty()) {
                          break;
                      }
                      unoidl::detail::SourceProviderEntity const * p;
                      if (findEntity(
                              @2, yyscanner, data, false, &baseName, &p,
                              nullptr, nullptr)
                          == FOUND_ERROR)
                      {
                          YYERROR;
                      }
                      if (p == nullptr || !p->entity.is()
                          || (p->entity->getSort()
                              != unoidl::Entity::SORT_EXCEPTION_TYPE))
                      {
                          error(
                              @2, yyscanner,
                              ("inconsistent type manager: exception type "
                               + data->currentName + " base " + baseName
                               + (" does not resolve to an existing exception"
                                  " type")));
                          YYERROR;
                      }
                      baseEnt = static_cast<unoidl::ExceptionTypeEntity *>(
                          p->entity.get());
                  }
              }
              p3->members.emplace_back(id, t.getName(), annotations($1));
          }
      }
  }
;

interfaceDefn:
  deprecated_opt published_opt TOK_INTERFACE identifier singleInheritance_opt
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      convertToCurrentName(data, $4);
      OUString baseName;
      rtl::Reference<unoidl::InterfaceTypeEntity> baseEnt;
      if ($5 != nullptr) {
          baseName = convertName($5);
          unoidl::detail::SourceProviderEntity const * p;
          if (findEntity(
                  @5, yyscanner, data, true, &baseName, &p, nullptr, nullptr)
              == FOUND_ERROR)
          {
              YYERROR;
          }
          if (p == nullptr || !p->entity.is()
              || p->entity->getSort() != unoidl::Entity::SORT_INTERFACE_TYPE)
          {
              error(
                  @5, yyscanner,
                  ("interface type " + data->currentName + " direct base "
                   + baseName
                   + " does not resolve to an existing interface type"));
              YYERROR;
          }
          baseEnt = static_cast<unoidl::InterfaceTypeEntity *>(p->entity.get());
          if ($2 && !baseEnt->isPublished()) {
              error(
                  @5, yyscanner,
                  ("published interface type " + data->currentName
                   + " direct base " + baseName + " is unpublished"));
              YYERROR;
          }
      }
      std::map<OUString, unoidl::detail::SourceProviderEntity>::iterator i(
          data->entities.find(data->currentName));
      if (i != data->entities.end()) {
          switch (i->second.kind) {
          case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
              break;
          case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
              if (!$2) {
                  error(
                      @4, yyscanner,
                      ("unpublished interface type " + data->currentName
                       + " has been declared published"));
                  YYERROR;
              }
              break;
          default:
              error(
                  @4, yyscanner,
                  "multiple entities named " + data->currentName);
              YYERROR;
              break;
          }
      }
      rtl::Reference<unoidl::detail::SourceProviderInterfaceTypeEntityPad> pad(
          new unoidl::detail::SourceProviderInterfaceTypeEntityPad(
              $2, baseEnt.is()));
      if (baseEnt.is()
          && !pad->addDirectBase(
              @4, yyscanner, data,
              unoidl::detail::SourceProviderInterfaceTypeEntityPad::DirectBase(
                  baseName, baseEnt, std::vector<OUString>()),
              false))
      {
          YYERROR;
      }
      data->entities[data->currentName] = unoidl::detail::SourceProviderEntity(
          pad.get());
  }
  '{' interfaceMembers '}' ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderInterfaceTypeEntityPad * pad =
          dynamic_cast<unoidl::detail::SourceProviderInterfaceTypeEntityPad *>(
              ent->pad.get());
      assert(pad != nullptr);
      if (pad->directMandatoryBases.empty()
          && data->currentName != "com.sun.star.uno.XInterface")
      {
          OUString base(".com.sun.star.uno.XInterface");
          unoidl::detail::SourceProviderEntity const * p;
          if (findEntity(@4, yyscanner, data, true, &base, &p, nullptr, nullptr)
              == FOUND_ERROR)
          {
              YYERROR;
          }
          if (p == nullptr || !p->entity.is()
              || p->entity->getSort() != unoidl::Entity::SORT_INTERFACE_TYPE)
          {
              error(
                  @3, yyscanner,
                  ("interface type " + data->currentName
                   + " implicit direct base " + base
                   + " does not resolve to an existing interface type"));
              YYERROR;
          }
          if (!pad->addDirectBase(
                  @3, yyscanner, data,
                  unoidl::detail::SourceProviderInterfaceTypeEntityPad::DirectBase(
                      base,
                      static_cast<unoidl::InterfaceTypeEntity *>(
                          p->entity.get()),
                      std::vector<OUString>()),
                  false))
          {
              YYERROR;
          }
      }
      std::vector<unoidl::AnnotatedReference> mbases;
      for (auto & i: pad->directMandatoryBases) {
          mbases.emplace_back(i.name, i.annotations);
      }
      std::vector<unoidl::AnnotatedReference> obases;
      for (auto & i: pad->directOptionalBases) {
          obases.emplace_back(i.name, i.annotations);
      }
      ent->entity = new unoidl::InterfaceTypeEntity(
          pad->isPublished(), mbases, obases, pad->directAttributes,
          pad->directMethods, annotations($1));
      ent->pad.clear();
      clearCurrentState(data);
  }
;

interfaceMembers:
 interfaceMembers interfaceMember
| /* empty */
;

interfaceMember:
  interfaceBase
| interfaceAttribute
| interfaceMethod
;

interfaceBase:
  deprecated_opt flagSection_opt TOK_INTERFACE name ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString name(convertName($4));
      rtl::Reference<unoidl::detail::SourceProviderInterfaceTypeEntityPad> pad(
          getCurrentPad<unoidl::detail::SourceProviderInterfaceTypeEntityPad>(
              data));
      if (pad->singleBase) {
          error(
              @3, yyscanner,
              "single-inheritance interface cannot have additional bases");
          YYERROR;
      }
      if (($2 & ~unoidl::detail::FLAG_OPTIONAL) != 0) {
          error(
              @2, yyscanner,
              "interface base can only be flagged as [optional]");
          YYERROR;
      }
      bool opt = ($2 & unoidl::detail::FLAG_OPTIONAL) != 0;
      OUString orgName(name);
      unoidl::detail::SourceProviderEntity const * p;
      bool typedefed = false;
      if (findEntity(@4, yyscanner, data, true, &name, &p, &typedefed, nullptr)
          == FOUND_ERROR)
      {
          YYERROR;
      }
      if (p == nullptr || !p->entity.is()
          || p->entity->getSort() != unoidl::Entity::SORT_INTERFACE_TYPE)
      {
          error(
              @4, yyscanner,
              ("interface type " + data->currentName + " direct base " + name
               + " does not resolve to an existing interface type"));
          YYERROR;
      }
      if (typedefed) {
          error(
              @4, yyscanner,
              ("interface type " + data->currentName + " direct base " + orgName
               + " is a typedef"));
          YYERROR;
      }
      rtl::Reference<unoidl::InterfaceTypeEntity> ent(
          static_cast<unoidl::InterfaceTypeEntity *>(p->entity.get()));
      if (data->publishedContext && !ent->isPublished()) {
          error(
              @4, yyscanner,
              ("published interface type " + data->currentName + " direct base "
               + name + " is unpublished"));
          YYERROR;
      }
      if (!pad->addDirectBase(
              @4, yyscanner, data,
              unoidl::detail::SourceProviderInterfaceTypeEntityPad::DirectBase(
                  name, ent, annotations($1)),
              opt))
      {
          YYERROR;
      }
  }
;

interfaceAttribute:
  deprecated_opt flagSection type identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderType t(*$3);
      delete $3;
      OUString id(convertName($4));
      if (($2 & unoidl::detail::FLAG_ATTRIBUTE) == 0) {
          error(
              @2, yyscanner,
              "interface attribute must be flagged as [attribute]");
          YYERROR;
      }
      if (($2
           & ~(unoidl::detail::FLAG_ATTRIBUTE | unoidl::detail::FLAG_BOUND
               | unoidl::detail::FLAG_READONLY))
          != 0)
      {
          error(
              @2, yyscanner,
              ("interface attribute can only be flagged as [attribute,"
               " optional]"));
          YYERROR;
      }
      switch (t.type) {
      case unoidl::detail::SourceProviderType::TYPE_VOID:
      case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
          error(
              @3, yyscanner,
              ("illegal interface type " + data->currentName
               + " direct attribute " + id + " type"));
          YYERROR;
          break;
      default:
          break;
      }
      rtl::Reference<unoidl::detail::SourceProviderInterfaceTypeEntityPad> pad(
          getCurrentPad<unoidl::detail::SourceProviderInterfaceTypeEntityPad>(
              data));
      if (!pad->addDirectMember(@4, yyscanner, data, id)) {
          YYERROR;
      }
      pad->directAttributes.emplace_back(
          id, t.getName(), ($2 & unoidl::detail::FLAG_BOUND) != 0,
          ($2 & unoidl::detail::FLAG_READONLY) != 0,
          std::vector<OUString>(), std::vector<OUString>(), annotations($1));
  }
  attributeAccessDecls_opt ';'
;

attributeAccessDecls_opt:
  '{' attributeAccessDecls '}'
| /* empty */
;

attributeAccessDecls:
  attributeAccessDecls attributeAccessDecl
  {
      if (($1 & $2) != 0) {
          error(
              @2, yyscanner, "duplicate get/set attribute access declaration");
          YYERROR;
      }
      $$ = unoidl::detail::SourceProviderAccessDecls($1 | $2);
  }
| /* empty */ { $$ = unoidl::detail::SourceProviderAccessDecls(0); }
;

attributeAccessDecl:
  TOK_GET exceptionSpec ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      rtl::Reference<unoidl::detail::SourceProviderInterfaceTypeEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderInterfaceTypeEntityPad>(
              data));
      assert(!pad->directAttributes.empty());
      pad->directAttributes.back().getExceptions = *$2;
      delete $2;
      $$ = unoidl::detail::ACCESS_DECL_GET;
  }
| TOK_SET exceptionSpec ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      rtl::Reference<unoidl::detail::SourceProviderInterfaceTypeEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderInterfaceTypeEntityPad>(
              data));
      assert(!pad->directAttributes.empty());
      pad->directAttributes.back().setExceptions = *$2;
      delete $2;
      if (pad->directAttributes.back().readOnly) {
          error(
              @1, yyscanner,
              ("interface type " + data->currentName
               + " direct read-only attribute "
               + pad->directAttributes.back().name
               + " cannot have set access declaration"));
          YYERROR;
      }
      $$ = unoidl::detail::ACCESS_DECL_SET;
  }
;

interfaceMethod:
  deprecated_opt type identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderType t(*$2);
      delete $2;
      OUString id(convertName($3));
      if (t.type == unoidl::detail::SourceProviderType::TYPE_EXCEPTION) {
          error(
              @3, yyscanner,
              ("illegal interface type " + data->currentName
               + " direct method " + id + " return type"));
          YYERROR;
      }
      rtl::Reference<unoidl::detail::SourceProviderInterfaceTypeEntityPad> pad(
          getCurrentPad<unoidl::detail::SourceProviderInterfaceTypeEntityPad>(
              data));
      if (!pad->addDirectMember(@3, yyscanner, data, id)) {
          YYERROR;
      }
      pad->directMethods.emplace_back(
          id, t.getName(),
          std::vector<unoidl::InterfaceTypeEntity::Method::Parameter>(),
          std::vector<OUString>(), annotations($1));
  }
  '(' methodParams_opt ')' exceptionSpec_opt ';'
  {
      if ($8 != nullptr) {
          unoidl::detail::SourceProviderScannerData * data
              = yyget_extra(yyscanner);
          rtl::Reference<unoidl::detail::SourceProviderInterfaceTypeEntityPad>
              pad(getCurrentPad<unoidl::detail::SourceProviderInterfaceTypeEntityPad>(
                  data));
          assert(!pad->directMethods.empty());
          pad->directMethods.back().exceptions = *$8;
          delete $8;
      }
  }
;

methodParams_opt:
  methodParams
| /* empty */
;

methodParams:
  methodParams ',' methodParam
| methodParam
;

methodParam:
  '[' direction ']' type identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderType t(*$4);
      delete $4;
      OUString id(convertName($5));
      rtl::Reference<unoidl::detail::SourceProviderInterfaceTypeEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderInterfaceTypeEntityPad>(
              data));
      assert(!pad->directMethods.empty());
      switch (t.type) {
      case unoidl::detail::SourceProviderType::TYPE_VOID:
      case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
          error(
              @4, yyscanner,
              ("illegal interface type " + data->currentName
               + " direct method " + pad->directMethods.back().name
               + " parameter " + id + " type"));
          YYERROR;
          break;
      default:
          break;
      }
      for (auto & i: pad->directMethods.back().parameters) {
          if (id == i.name) {
              error(
                  @5, yyscanner,
                  ("interface type " + data->currentName + " direct method "
                   + pad->directMethods.back().name + " parameter " + id
                   + " has same identifier as another parameter"));
              YYERROR;
          }
      }
      pad->directMethods.back().parameters.emplace_back(id, t.getName(), $2);
  }
;

direction:
  TOK_IN { $$ = unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN; }
| TOK_OUT
  { $$ = unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_OUT; }
| TOK_INOUT
  { $$ = unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN_OUT; }
;

typedefDefn:
  deprecated_opt published_opt TOK_TYPEDEF type identifier ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      unoidl::detail::SourceProviderType t(*$4);
      delete $4;
      OUString name(convertToFullName(data, $5));
      // There is no good reason to forbid typedefs to VOID, to instantiated
      // polymorphic struct types, and to exception types, but some old client
      // code of registry data expects this typedef restriction (like the
      // assert(false) default in handleTypedef in
      // codemaker/source/javamaker/javatype.cxx), so forbid them for now:
      switch (t.type) {
      case unoidl::detail::SourceProviderType::TYPE_VOID:
      case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
      case unoidl::detail::SourceProviderType::TYPE_INSTANTIATED_POLYMORPHIC_STRUCT:
          error(@4, yyscanner, "bad typedef type");
          YYERROR;
          break;
      case unoidl::detail::SourceProviderType::TYPE_ENUM:
      case unoidl::detail::SourceProviderType::TYPE_PLAIN_STRUCT:
      case unoidl::detail::SourceProviderType::TYPE_INTERFACE:
          if ($2) {
              bool unpub = false;
              switch (t.entity->kind) {
              case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
                  unpub = true;
                  break;
              case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
                  break;
              case unoidl::detail::SourceProviderEntity::KIND_MODULE:
                  assert(false && "this cannot happen");
              default:
                  assert(t.entity->entity.is() || t.entity->pad.is());
                  unpub
                      = !(t.entity->entity.is()
                          ? static_cast<unoidl::PublishableEntity *>(
                              t.entity->entity.get())->isPublished()
                          : t.entity->pad->isPublished());
                  break;
              }
              if (unpub) {
                  error(
                      @4, yyscanner,
                      "published typedef " + name + " type is unpublished");
                  YYERROR;
              }
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_PARAMETER:
          assert(false && "this cannot happen");
      default:
          break;
      }
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  name,
                  unoidl::detail::SourceProviderEntity(
                      unoidl::detail::SourceProviderEntity::KIND_LOCAL,
                      new unoidl::TypedefEntity(
                          $2, t.getName(), annotations($1))))).
          second)
      {
          error(@5, yyscanner, "multiple entities named " + name);
          YYERROR;
      }
      clearCurrentState(data);
  }
;

constantGroupDefn:
  deprecated_opt published_opt TOK_CONSTANTS identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      convertToCurrentName(data, $4);
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  data->currentName,
                  unoidl::detail::SourceProviderEntity(
                      new unoidl::detail::SourceProviderConstantGroupEntityPad(
                          $2)))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + data->currentName);
          YYERROR;
      }
  }
  '{' constants '}' ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderConstantGroupEntityPad * pad =
          dynamic_cast<unoidl::detail::SourceProviderConstantGroupEntityPad *>(
              ent->pad.get());
      assert(pad != nullptr);
      ent->entity = new unoidl::ConstantGroupEntity(
          pad->isPublished(), pad->members, annotations($1));
      ent->pad.clear();
      clearCurrentState(data);
  }
;

constants:
  constants constant
| /* empty */
;

constant:
  deprecated_opt TOK_CONST type identifier '=' expr ';'
  {
      OUString id(convertName($4));
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      rtl::Reference<unoidl::detail::SourceProviderConstantGroupEntityPad> pad(
          getCurrentPad<unoidl::detail::SourceProviderConstantGroupEntityPad>(
              data));
      unoidl::detail::SourceProviderType t(*$3);
      delete $3;
      unoidl::ConstantValue v(false); // dummy value
      switch (t.type) {
      case unoidl::detail::SourceProviderType::TYPE_BOOLEAN:
          if ($6.type != unoidl::detail::SourceProviderExpr::TYPE_BOOL) {
              error(
                  @6, yyscanner,
                  ("bad value of boolean-typed constant " + data->currentName
                   + "." + id));
              YYERROR;
          }
          v = unoidl::ConstantValue($6.bval);
          break;
      case unoidl::detail::SourceProviderType::TYPE_BYTE:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              if ($6.ival < SAL_MIN_INT8 || $6.ival > SAL_MAX_INT8) {
                  error(
                      @6, yyscanner,
                      ("out-of-range byte-typed constant " + data->currentName
                       + "." + id + " value " + OUString::number($6.ival)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_Int8>($6.ival));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              if ($6.uval > SAL_MAX_INT8) {
                  error(
                      @6, yyscanner,
                      ("out-of-range byte-typed constant " + data->currentName
                       + "." + id + " value " + OUString::number($6.uval)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_Int8>($6.uval));
              break;
          default:
              error(
                  @6, yyscanner,
                  ("bad value of byte-typed constant " + data->currentName + "."
                   + id));
              YYERROR;
              break;
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_SHORT:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              if ($6.ival < SAL_MIN_INT16 || $6.ival > SAL_MAX_INT16) {
                  error(
                      @6, yyscanner,
                      ("out-of-range short-typed constant " + data->currentName
                       + "." + id + " value " + OUString::number($6.ival)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_Int16>($6.ival));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              if ($6.uval > SAL_MAX_INT16) {
                  error(
                      @6, yyscanner,
                      ("out-of-range short-typed constant " + data->currentName
                       + "." + id + " value " + OUString::number($6.uval)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_Int16>($6.uval));
              break;
          default:
              error(
                  @6, yyscanner,
                  ("bad value of short-typed constant " + data->currentName
                   + "." + id));
              YYERROR;
              break;
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_SHORT:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              if ($6.ival < 0 || $6.ival > SAL_MAX_UINT16) {
                  error(
                      @6, yyscanner,
                      ("out-of-range unsigned-short-typed constant "
                       + data->currentName + "." + id + " value "
                       + OUString::number($6.ival)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_uInt16>($6.ival));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              if ($6.uval > SAL_MAX_UINT16) {
                  error(
                      @6, yyscanner,
                      ("out-of-range unsigned-short-typed constant "
                       + data->currentName + "." + id + " value "
                       + OUString::number($6.uval)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_uInt16>($6.uval));
              break;
          default:
              error(
                  @6, yyscanner,
                  ("bad value of unsigned-short-typed constant "
                   + data->currentName + "." + id));
              YYERROR;
              break;
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_LONG:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              if ($6.ival < SAL_MIN_INT32 || $6.ival > SAL_MAX_INT32) {
                  error(
                      @6, yyscanner,
                      ("out-of-range long-typed constant " + data->currentName
                       + "." + id + " value " + OUString::number($6.ival)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_Int32>($6.ival));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              if ($6.uval > SAL_MAX_INT32) {
                  error(
                      @6, yyscanner,
                      ("out-of-range long-typed constant " + data->currentName
                       + "." + id + " value " + OUString::number($6.uval)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_Int32>($6.uval));
              break;
          default:
              error(
                  @6, yyscanner,
                  ("bad value of long-typed constant " + data->currentName
                   + "." + id));
              YYERROR;
              break;
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_LONG:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              if ($6.ival < 0 || $6.ival > SAL_MAX_UINT32) {
                  error(
                      @6, yyscanner,
                      ("out-of-range unsigned-long-typed constant "
                       + data->currentName + "." + id + " value "
                       + OUString::number($6.ival)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_uInt32>($6.ival));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              if ($6.uval > SAL_MAX_UINT32) {
                  error(
                      @6, yyscanner,
                      ("out-of-range unsigned-long-typed constant "
                       + data->currentName + "." + id + " value "
                       + OUString::number($6.uval)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_uInt32>($6.uval));
              break;
          default:
              error(
                  @6, yyscanner,
                  ("bad value of unsigned-long-typed constant "
                   + data->currentName + "." + id));
              YYERROR;
              break;
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_HYPER:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              v = unoidl::ConstantValue($6.ival);
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              if ($6.uval > SAL_MAX_INT64) {
                  error(
                      @6, yyscanner,
                      ("out-of-range hyper-typed constant " + data->currentName
                       + "." + id + " value " + OUString::number($6.uval)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_Int64>($6.uval));
              break;
          default:
              error(
                  @6, yyscanner,
                  ("bad value of hyper-typed constant " + data->currentName
                   + "." + id));
              YYERROR;
              break;
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_HYPER:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              if ($6.ival < 0) {
                  error(
                      @6, yyscanner,
                      ("out-of-range unsigned-hyper-typed constant "
                       + data->currentName + "." + id + " value "
                       + OUString::number($6.ival)));
                  YYERROR;
              }
              v = unoidl::ConstantValue(static_cast<sal_uInt64>($6.ival));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              v = unoidl::ConstantValue($6.uval);
              break;
          default:
              error(
                  @6, yyscanner,
                  ("bad value of unsigned-hyper-typed constant "
                   + data->currentName + "." + id));
              YYERROR;
              break;
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_FLOAT:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
              error(
                  @6, yyscanner,
                  ("bad boolean value of float-typed constant "
                   + data->currentName + "." + id));
              YYERROR;
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              v = unoidl::ConstantValue(static_cast<float>($6.ival));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              v = unoidl::ConstantValue(static_cast<float>($6.uval));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
              v = unoidl::ConstantValue(static_cast<float>($6.fval));
              break;
          }
          break;
      case unoidl::detail::SourceProviderType::TYPE_DOUBLE:
          switch ($6.type) {
          case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
              error(
                  @6, yyscanner,
                  ("bad boolean value of double-typed constant "
                   + data->currentName + "." + id));
              YYERROR;
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_INT:
              v = unoidl::ConstantValue(static_cast<double>($6.ival));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_UINT:
              v = unoidl::ConstantValue(static_cast<double>($6.uval));
              break;
          case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
              v = unoidl::ConstantValue($6.fval);
              break;
          }
          break;
      default:
          error(
              @3, yyscanner,
              "bad type for constant " + data->currentName + "." + id);
          YYERROR;
          break;
      }
      pad->members.emplace_back(id, v, annotations($1));
  }
;

singleInterfaceBasedServiceDefn:
  deprecated_opt published_opt TOK_SERVICE identifier singleInheritance
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      convertToCurrentName(data, $4);
      OUString base(convertName($5));
      unoidl::detail::SourceProviderEntity const * p;
      if (findEntity(@5, yyscanner, data, false, &base, &p, nullptr, nullptr)
          == FOUND_ERROR)
      {
          YYERROR;
      }
      bool ifcBase = false;
      bool pubBase = false;
      if (p != nullptr) {
          switch (p->kind) {
          case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
              ifcBase = true;
              pubBase = false;
              break;
          case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
              ifcBase = true;
              pubBase = true;
              break;
          default:
              if (p->entity.is()
                  && (p->entity->getSort()
                      == unoidl::Entity::SORT_INTERFACE_TYPE))
              {
                  ifcBase = true;
                  pubBase = static_cast<unoidl::InterfaceTypeEntity *>(
                      p->entity.get())->isPublished();
              }
              break;
          }
      }
      if (!ifcBase) {
          error(
              @5, yyscanner,
              ("single-interface--based service " + data->currentName + " base "
               + base + " does not resolve to an interface type"));
          YYERROR;
      }
      if ($2 && !pubBase) {
          error(
              @5, yyscanner,
              ("published single-interface--based service " + data->currentName
               + " base " + base + " is unpublished"));
          YYERROR;
      }
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  data->currentName,
                  unoidl::detail::SourceProviderEntity(
                      new unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad(
                          $2, base)))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + data->currentName);
          YYERROR;
      }
  }
  ctors_opt ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad * pad =
          dynamic_cast<unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad *>(
              ent->pad.get());
      assert(pad != nullptr);
      std::vector<unoidl::SingleInterfaceBasedServiceEntity::Constructor> ctors;
      if ($7) {
          for (auto & i: pad->constructors) {
              std::vector<unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter> parms;
              for (auto & j: i.parameters) {
                  parms.emplace_back(j.name, j.type.getName(), j.rest);
              }
              ctors.push_back(
                  unoidl::SingleInterfaceBasedServiceEntity::Constructor(
                      i.name, parms, i.exceptions, i.annotations));
          }
      } else {
          assert(pad->constructors.empty());
          ctors.push_back(
              unoidl::SingleInterfaceBasedServiceEntity::Constructor());
      }
      ent->entity = new unoidl::SingleInterfaceBasedServiceEntity(
          pad->isPublished(), pad->base, ctors, annotations($1));
      ent->pad.clear();
      clearCurrentState(data);
  }
;

ctors_opt:
  '{' ctors '}' { $$ = true; }
| /* empty */ { $$ = false; }
;

ctors:
  ctors ctor
| /* empty */
;

ctor:
  deprecated_opt identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString id(convertName($2));
      rtl::Reference<unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad>(
                  data));
      for (auto & i: pad->constructors) {
          if (id == i.name) {
              error(
                  @2, yyscanner,
                  ("single-interface--based service " + data->currentName
                   + " constructor " + id
                   + " has same identifier as another constructor"));
              YYERROR;
          }
      }
      pad->constructors.push_back(
          unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad::Constructor(
              id, annotations($1)));
  }
  '(' ctorParams_opt ')' exceptionSpec_opt ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      rtl::Reference<unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad>(
                  data));
      assert(!pad->constructors.empty());
      if ($7 != nullptr) {
          pad->constructors.back().exceptions = *$7;
          delete $7;
      }
      for (auto i(pad->constructors.begin()); i != pad->constructors.end() - 1;
           ++i)
      {
          if (i->parameters.size()
              == pad->constructors.back().parameters.size())
          {
              bool same = true;
              for (auto
                       j(i->parameters.begin()),
                       k(pad->constructors.back().parameters.begin());
                   j != i->parameters.end(); ++j, ++k)
              {
                  if (!j->type.equals(k->type) || j->rest != k->rest) {
                      same = false;
                      break;
                  }
              }
              if (same) {
                  error(
                      @2, yyscanner,
                      ("single-interface--based service " + data->currentName
                       + " constructor " + pad->constructors.back().name
                       + " has similar paramete list to constructor "
                       + i->name));
                  YYERROR;
              }
          }
      }
  }
;

ctorParams_opt:
  ctorParams
| /* empty */
;

ctorParams:
  ctorParams ',' ctorParam
| ctorParam
;

ctorParam:
  '[' direction ']' type ellipsis_opt identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderType t(*$4);
      delete $4;
      OUString id(convertName($6));
      rtl::Reference<unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad>(
              data));
      assert(!pad->constructors.empty());
      if ($2 != unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN) {
          error(
              @4, yyscanner,
              ("single-interface--based service " + data->currentName
               + " constructor " + pad->constructors.back().name + " parameter "
               + id + " direction must be [in]"));
          YYERROR;
      }
      switch (t.type) {
      case unoidl::detail::SourceProviderType::TYPE_VOID:
      case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
          error(
              @4, yyscanner,
              ("illegal single-interface--based service " + data->currentName
               + " constructor " + pad->constructors.back().name + " parameter "
               + id + " type"));
          YYERROR;
          break;
      default:
          break;
      }
      if ($5) {
          if (t.type != unoidl::detail::SourceProviderType::TYPE_ANY) {
              error(
                  @4, yyscanner,
                  ("illegal single-interface--based service "
                   + data->currentName + " constructor "
                   + pad->constructors.back().name + " rest parameter " + id
                   + " non-any type"));
              YYERROR;
          }
          if (!pad->constructors.back().parameters.empty()) {
              error(
                  @5, yyscanner,
                  ("single-interface--based service " + data->currentName
                   + " constructor " + pad->constructors.back().name
                   + " rest parameter " + id + " must be first parameter"));
              YYERROR;
          }
      } else if (!pad->constructors.back().parameters.empty()
                 && pad->constructors.back().parameters.back().rest)
      {
          error(
              @1, yyscanner,
              ("single-interface--based service " + data->currentName
               + " constructor " + pad->constructors.back().name
               + " rest parameter must be last parameter"));
          YYERROR;
      }
      for (auto & i: pad->constructors.back().parameters) {
          if (id == i.name) {
              error(
                  @6, yyscanner,
                  ("single-interface--based service " + data->currentName
                   + " constructor " + pad->constructors.back().name
                   + " parameter " + id
                   + " has same identifier as another parameter"));
              YYERROR;
          }
      }
      pad->constructors.back().parameters.push_back(
          unoidl::detail::SourceProviderSingleInterfaceBasedServiceEntityPad::Constructor::Parameter(
              id, t, $5));
  }
;

ellipsis_opt:
  TOK_ELLIPSIS { $$ = true; }
| /* empty */ { $$ = false; }

accumulationBasedServiceDefn:
  deprecated_opt published_opt TOK_SERVICE identifier
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      convertToCurrentName(data, $4);
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  data->currentName,
                  unoidl::detail::SourceProviderEntity(
                      new unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad(
                          $2)))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + data->currentName);
          YYERROR;
      }
  }
  '{' serviceMembers '}' ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
      unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad * pad =
          dynamic_cast<unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad *>(
              ent->pad.get());
      assert(pad != nullptr);
      ent->entity = new unoidl::AccumulationBasedServiceEntity(
          pad->isPublished(), pad->directMandatoryBaseServices,
          pad->directOptionalBaseServices, pad->directMandatoryBaseInterfaces,
          pad->directOptionalBaseInterfaces, pad->directProperties,
          annotations($1));
      ent->pad.clear();
      clearCurrentState(data);
  }
;

serviceMembers:
  serviceMembers serviceMember
| /* empty */
;

serviceMember:
  serviceBase
| serviceInterfaceBase
| serviceProperty
;

serviceBase:
  deprecated_opt flagSection_opt TOK_SERVICE name ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString name(convertName($4));
      rtl::Reference<unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad> pad(
          getCurrentPad<unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad>(
              data));
      if (($2 & ~unoidl::detail::FLAG_OPTIONAL) != 0) {
          error(
              @2, yyscanner,
              "service base can only be flagged as [optional]");
          YYERROR;
      }
      bool opt = ($2 & unoidl::detail::FLAG_OPTIONAL) != 0;
      unoidl::detail::SourceProviderEntity const * p;
      if (findEntity(@4, yyscanner, data, false, &name, &p, nullptr, nullptr)
          == FOUND_ERROR)
      {
          YYERROR;
      }
      if (p == nullptr || !p->entity.is()
          || (p->entity->getSort()
              != unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE))
      {
          error(
              @4, yyscanner,
              ("accumulation-based service " + data->currentName
               + " direct base service " + name
               + " does not resolve to an accumulation-based service"));
          YYERROR;
      }
      if (data->publishedContext
          && !static_cast<unoidl::AccumulationBasedServiceEntity *>(
              p->entity.get())->isPublished())
      {
          error(
              @4, yyscanner,
              ("published accumulation-based service " + data->currentName
               + " direct base service " + name + " is unpublished"));
          YYERROR;
      }
      std::vector<unoidl::AnnotatedReference> & v(
          opt
          ? pad->directOptionalBaseServices : pad->directMandatoryBaseServices);
      for (auto & i: v) {
          if (name == i.name) {
              error(
                  @4, yyscanner,
                  ("accumulation-based service " + data->currentName
                   + " duplicate direct base service " + name));
              YYERROR;
          }
      }
      v.emplace_back(name, annotations($1));
  }
;

serviceInterfaceBase:
  deprecated_opt flagSection_opt TOK_INTERFACE name ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString name(convertName($4));
      rtl::Reference<unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad> pad(
          getCurrentPad<unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad>(
              data));
      if (($2 & ~unoidl::detail::FLAG_OPTIONAL) != 0) {
          error(
              @2, yyscanner,
              "interface base can only be flagged as [optional]");
          YYERROR;
      }
      bool opt = ($2 & unoidl::detail::FLAG_OPTIONAL) != 0;
      unoidl::detail::SourceProviderEntity const * p;
      if (findEntity(@4, yyscanner, data, false, &name, &p, nullptr, nullptr)
          == FOUND_ERROR)
      {
          YYERROR;
      }
      bool ifcBase = false;
      bool pubBase = false;
      if (p != nullptr) {
          switch (p->kind) {
          case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
              ifcBase = true;
              pubBase = false;
              break;
          case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
              ifcBase = true;
              pubBase = true;
              break;
          default:
              if (p->entity.is()
                  && (p->entity->getSort()
                      == unoidl::Entity::SORT_INTERFACE_TYPE))
              {
                  ifcBase = true;
                  pubBase = static_cast<unoidl::InterfaceTypeEntity *>(
                      p->entity.get())->isPublished();
              }
              break;
          }
      }
      if (!ifcBase) {
          error(
              @4, yyscanner,
              ("accumulation-based service " + data->currentName
               + " direct base interface " + name
               + " does not resolve to an interface type"));
          YYERROR;
      }
      if (data->publishedContext && !opt && !pubBase) {
          error(
              @4, yyscanner,
              ("published accumulation-based service " + data->currentName
               + " direct base interface " + name + " is unpublished"));
          YYERROR;
      }
      std::vector<unoidl::AnnotatedReference> & v(
          opt
          ? pad->directOptionalBaseInterfaces
          : pad->directMandatoryBaseInterfaces);
      for (auto & i: v) {
          if (name == i.name) {
              error(
                  @4, yyscanner,
                  ("accumulation-based service " + data->currentName
                   + " duplicate direct base interface " + name));
              YYERROR;
          }
      }
      v.emplace_back(name, annotations($1));
  }
;

serviceProperty:
  deprecated_opt flagSection type identifier ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::detail::SourceProviderType t(*$3);
      delete $3;
      OUString id(convertName($4));
      if (($2 & unoidl::detail::FLAG_PROPERTY) == 0) {
          error(
              @2, yyscanner,
              ("accumulation-based service property must be flagged as"
               " [property]"));
          YYERROR;
      }
      if (($2
           & ~(unoidl::detail::FLAG_BOUND | unoidl::detail::FLAG_CONSTRAINED
               | unoidl::detail::FLAG_MAYBEAMBIGUOUS
               | unoidl::detail::FLAG_MAYBEDEFAULT
               | unoidl::detail::FLAG_MAYBEVOID | unoidl::detail::FLAG_OPTIONAL
               | unoidl::detail::FLAG_PROPERTY | unoidl::detail::FLAG_READONLY
               | unoidl::detail::FLAG_REMOVABLE
               | unoidl::detail::FLAG_TRANSIENT))
          != 0)
      {
          error(
              @2, yyscanner,
              ("accumulation-based service property can only be flagged as"
               " [property, bound, constrained, maybeambiguous, maybedefault,"
               " maybevoid, optional, readonly, removable, transient]"));
          YYERROR;
      }
      int att = 0;
      if (($2 & unoidl::detail::FLAG_BOUND) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_BOUND;
      }
      if (($2 & unoidl::detail::FLAG_CONSTRAINED) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_CONSTRAINED;
      }
      if (($2 & unoidl::detail::FLAG_MAYBEAMBIGUOUS) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_MAYBE_AMBIGUOUS;
      }
      if (($2 & unoidl::detail::FLAG_MAYBEDEFAULT) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_MAYBE_DEFAULT;
      }
      if (($2 & unoidl::detail::FLAG_MAYBEVOID) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_MAYBE_VOID;
      }
      if (($2 & unoidl::detail::FLAG_OPTIONAL) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_OPTIONAL;
      }
      if (($2 & unoidl::detail::FLAG_READONLY) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_READ_ONLY;
      }
      if (($2 & unoidl::detail::FLAG_REMOVABLE) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_REMOVABLE;
      }
      if (($2 & unoidl::detail::FLAG_TRANSIENT) != 0) {
          att |= unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_TRANSIENT;
      }
      switch (t.type) {
      case unoidl::detail::SourceProviderType::TYPE_VOID:
      case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
          error(
              @3, yyscanner,
              ("illegal accumulation-based service " + data->currentName
               + " direct property " + id + " type"));
          YYERROR;
          break;
      default:
          break;
      }
      rtl::Reference<unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad>
          pad(getCurrentPad<unoidl::detail::SourceProviderAccumulationBasedServiceEntityPad>(
                  data));
      for (auto & i: pad->directProperties) {
          if (id == i.name) {
              error(
                  @4, yyscanner,
                  ("accumulation-based service " + data->currentName
                   + " duplicate direct property " + id));
              YYERROR;
          }
      }
      pad->directProperties.emplace_back(
          id, t.getName(),
          unoidl::AccumulationBasedServiceEntity::Property::Attributes(att),
          annotations($1));
  }
;

interfaceBasedSingletonDefn:
  deprecated_opt published_opt TOK_SINGLETON identifier singleInheritance ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      OUString name(convertToFullName(data, $4));
      OUString base(convertName($5));
      unoidl::detail::SourceProviderEntity const * p;
      if (findEntity(@5, yyscanner, data, false, &base, &p, nullptr, nullptr)
          == FOUND_ERROR)
      {
          YYERROR;
      }
      bool ifcBase = false;
      bool pubBase = false;
      if (p != nullptr) {
          switch (p->kind) {
          case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
              ifcBase = true;
              pubBase = false;
              break;
          case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
              ifcBase = true;
              pubBase = true;
              break;
          default:
              if (p->entity.is()
                  && (p->entity->getSort()
                      == unoidl::Entity::SORT_INTERFACE_TYPE))
              {
                  ifcBase = true;
                  pubBase = static_cast<unoidl::InterfaceTypeEntity *>(
                      p->entity.get())->isPublished();
              }
              break;
          }
      }
      if (!ifcBase) {
          error(
              @5, yyscanner,
              ("interface-based singleton " + name + " base " + base
               + " does not resolve to an interface type"));
          YYERROR;
      }
      if ($2 && !pubBase) {
          error(
              @5, yyscanner,
              ("published interface-based singleton " + name + " base " + base
               + " is unpublished"));
          YYERROR;
      }
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  name,
                  unoidl::detail::SourceProviderEntity(
                      unoidl::detail::SourceProviderEntity::KIND_LOCAL,
                      new unoidl::InterfaceBasedSingletonEntity(
                          $2, base, annotations($1))))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + name);
          YYERROR;
      }
      clearCurrentState(data);
  }
;

serviceBasedSingletonDefn:
  deprecated_opt published_opt TOK_SINGLETON identifier '{' TOK_SERVICE name ';'
  '}' ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      OUString name(convertToFullName(data, $4));
      OUString base(convertName($7));
      unoidl::detail::SourceProviderEntity const * p;
      if (findEntity(@7, yyscanner, data, false, &base, &p, nullptr, nullptr)
          == FOUND_ERROR)
      {
          YYERROR;
      }
      if (p == nullptr
          || !p->entity.is()
          || (p->entity->getSort()
              != unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE))
      {
          error(
              @7, yyscanner,
              ("service-based singleton " + name + " base " + base
               + " does not resolve to an accumulation-based service"));
          YYERROR;
      }
      if ($2
          && !static_cast<unoidl::AccumulationBasedServiceEntity *>(
              p->entity.get())->isPublished())
      {
          error(
              @7, yyscanner,
              ("published service-based singleton " + name + " base " + base
               + " is unpublished"));
          YYERROR;
      }
      if (!data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  name,
                  unoidl::detail::SourceProviderEntity(
                      unoidl::detail::SourceProviderEntity::KIND_LOCAL,
                      new unoidl::ServiceBasedSingletonEntity(
                          $2, base, annotations($1))))).
          second)
      {
          error(@4, yyscanner, "multiple entities named " + name);
          YYERROR;
      }
      clearCurrentState(data);
  }
;

singleInheritance_opt:
  singleInheritance
| /* empty */ { $$ = nullptr; }
;

singleInheritance: ':' name { $$ = $2; }
;

exceptionSpec_opt:
  exceptionSpec
| /* empty */ { $$ = nullptr; }
;

exceptionSpec: TOK_RAISES '(' exceptions ')' { $$ = $3; }
;

exceptions:
  exceptions ',' name
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString name(convertName($3));
      unoidl::detail::SourceProviderEntity const * p;
      if (findEntity(@3, yyscanner, data, false, &name, &p, nullptr, nullptr)
          == FOUND_ERROR)
      {
          delete $1; /* see commented-out %destructor above */
          YYERROR;
      }
      if (p == nullptr
          || !p->entity.is()
          || (p->entity->getSort() != unoidl::Entity::SORT_EXCEPTION_TYPE))
      {
          delete $1; /* see commented-out %destructor above */
          error(
              @3, yyscanner,
              ("exception " + name + " does not resolve to an exception type"));
          YYERROR;
      }
      if (data->publishedContext
          && !(static_cast<unoidl::ExceptionTypeEntity *>(p->entity.get())
               ->isPublished()))
      {
          delete $1; /* see commented-out %destructor above */
          error(
              @3, yyscanner,
              ("unpublished exception " + name + " used in published context"));
          YYERROR;
      }
      if (std::find($1->begin(), $1->end(), name) != $1->end()) {
          delete $1; /* see commented-out %destructor above */
          error(
              @3, yyscanner, ("exception " + name + " listed more than once"));
          YYERROR;
      }
      $1->push_back(name);
      $$ = $1;
  }
| name
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString name(convertName($1));
      unoidl::detail::SourceProviderEntity const * p;
      if (findEntity(@1, yyscanner, data, false, &name, &p, nullptr, nullptr)
          == FOUND_ERROR)
      {
          YYERROR;
      }
      if (p == nullptr
          || !p->entity.is()
          || (p->entity->getSort() != unoidl::Entity::SORT_EXCEPTION_TYPE))
      {
          error(
              @1, yyscanner,
              ("exception " + name + " does not resolve to an exception type"));
          YYERROR;
      }
      if (data->publishedContext
          && !(static_cast<unoidl::ExceptionTypeEntity *>(p->entity.get())
               ->isPublished()))
      {
          error(
              @1, yyscanner,
              ("unpublished exception " + name + " used in published context"));
          YYERROR;
      }
      $$ = new std::vector<OUString>; $$->push_back(name);
  }
;

interfaceDecl:
  deprecated_opt/*ignored*/ published_opt TOK_INTERFACE identifier ';'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      data->publishedContext = $2;
      OUString name(convertToFullName(data, $4));
      std::pair<std::map<OUString, unoidl::detail::SourceProviderEntity>::iterator, bool> p(
          data->entities.insert(
              std::map<OUString, unoidl::detail::SourceProviderEntity>::value_type(
                  name,
                  unoidl::detail::SourceProviderEntity(
                      $2
                      ? unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL
                      : unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL))));
      if (!p.second) {
          switch (p.first->second.kind) {
          case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
              if ($2) {
                  p.first->second.kind
                      = unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL;
              }
              break;
          case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
              break;
          default:
              assert(p.first->second.entity.is());
              if (p.first->second.entity->getSort()
                  != unoidl::Entity::SORT_INTERFACE_TYPE)
              {
                  error(
                      @4, yyscanner,
                      "multiple entities named " + data->currentName);
                  YYERROR;
              }
              if ($2
                  && !static_cast<unoidl::InterfaceTypeEntity *>(
                      p.first->second.entity.get())->isPublished())
              {
                  error(
                      @4, yyscanner,
                      ("published interface type declaration "
                       + data->currentName + " has been defined unpublished"));
                  YYERROR;
              }
          }
      }
      clearCurrentState(data);
  }
;

published_opt:
  TOK_PUBLISHED { $$ = true; }
| /* empty */ { $$ = false; }
;

flagSection_opt:
  flagSection
| /* empty */ { $$ = unoidl::detail::SourceProviderFlags(0); }
;

flagSection: '[' flags ']' { $$ = $2; }
;

flags:
  flags ',' flag
  {
      if (($1 & $3) != 0) {
          error(@3, yyscanner, "duplicate flag " + flagName($3));
          YYERROR;
      }
      $$ = unoidl::detail::SourceProviderFlags($1 | $3);
  }
| flag
;

flag:
  TOK_ATTRIBUTE { $$ = unoidl::detail::FLAG_ATTRIBUTE; }
| TOK_BOUND { $$ = unoidl::detail::FLAG_BOUND; }
| TOK_CONSTRAINED { $$ = unoidl::detail::FLAG_CONSTRAINED; }
| TOK_MAYBEAMBIGUOUS { $$ = unoidl::detail::FLAG_MAYBEAMBIGUOUS; }
| TOK_MAYBEDEFAULT { $$ = unoidl::detail::FLAG_MAYBEDEFAULT; }
| TOK_MAYBEVOID { $$ = unoidl::detail::FLAG_MAYBEVOID; }
| TOK_OPTIONAL { $$ = unoidl::detail::FLAG_OPTIONAL; }
| TOK_PROPERTY { $$ = unoidl::detail::FLAG_PROPERTY; }
| TOK_READONLY { $$ = unoidl::detail::FLAG_READONLY; }
| TOK_REMOVABLE { $$ = unoidl::detail::FLAG_REMOVABLE; }
| TOK_TRANSIENT { $$ = unoidl::detail::FLAG_TRANSIENT; }
;

expr: orExpr
;

orExpr:
  orExpr '|' xorExpr
  {
      if (!coerce(@1, yyscanner, &$1, &$3)) {
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival | $3.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval | $3.uval);
          break;
      default:
          error(@1, yyscanner, "arguments of non-integer type to \"|\"");
          YYERROR;
          break;
      }
  }
| xorExpr
;

xorExpr:
  xorExpr '^' andExpr
  {
      if (!coerce(@1, yyscanner, &$1, &$3)) {
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival ^ $3.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval ^ $3.uval);
          break;
      default:
          error(@1, yyscanner, "arguments of non-integer type to \"^\"");
          YYERROR;
          break;
      }
  }
| andExpr
;

andExpr:
  andExpr '&' shiftExpr
  {
      if (!coerce(@1, yyscanner, &$1, &$3)) {
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival & $3.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval & $3.uval);
          break;
      default:
          error(@1, yyscanner, "arguments of non-integer type to \"&\"");
          YYERROR;
          break;
      }
  }
| shiftExpr
;

shiftExpr:
  shiftExpr TOK_LEFTSHIFT addExpr
  {
      int n;
      switch ($3.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          if ($3.ival < 0 || $3.ival > 63) {
              error(
                  @3, yyscanner,
                  ("out-of-range shift argument " + OUString::number($3.ival)
                   + " to \"<<\" "));
              YYERROR;
          }
          n = static_cast<int>($3.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          if ($3.uval > 63) {
              error(
                  @3, yyscanner,
                  ("out-of-range shift argument " + OUString::number($3.uval)
                   + " to \"<<\" "));
              YYERROR;
          }
          n = static_cast<int>($3.uval);
          break;
      default:
          error(@3, yyscanner, "right argument of non-integer type to \"<<\"");
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          if ($1.ival < 0) {
              error(
                  @1, yyscanner,
                  ("cannot left-shift negative argument "
                   + OUString::number($1.ival)));
              YYERROR;
          }
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival << n);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval << n);
          break;
      default:
          error(@1, yyscanner, "left argument of non-integer type to \"<<\"");
          YYERROR;
          break;
      }
  }
| shiftExpr TOK_RIGHTSHIFT addExpr
  {
      int n;
      switch ($3.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          if ($3.ival < 0 || $3.ival > 63) {
              error(
                  @3, yyscanner,
                  ("out-of-range shift argument " + OUString::number($3.ival)
                   + " to \">>\" "));
              YYERROR;
          }
          n = static_cast<int>($3.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          if ($3.uval > 63) {
              error(
                  @3, yyscanner,
                  ("out-of-range shift argument " + OUString::number($3.uval)
                   + " to \">>\" "));
              YYERROR;
          }
          n = static_cast<int>($3.uval);
          break;
      default:
          error(@3, yyscanner, "right argument of non-integer type to \">>\"");
          YYERROR;
          break;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival >> n);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval >> n);
          break;
      default:
          error(@1, yyscanner, "left argument of non-integer type to \">>\"");
          YYERROR;
          break;
      }
  }
| addExpr
;

addExpr:
  addExpr '+' multExpr
  {
      if (!coerce(@1, yyscanner, &$1, &$3)) {
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
          error(@1, yyscanner, "arguments of boolean type to binary \"+\"");
          YYERROR;
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival + $3.ival); //TODO: overflow
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval + $3.uval); //TODO: overflow
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
          $$ = unoidl::detail::SourceProviderExpr::Float($1.fval + $3.fval);
          break;
      }
  }
| addExpr '-' multExpr
  {
      if (!coerce(@1, yyscanner, &$1, &$3)) {
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
          error(@1, yyscanner, "arguments of boolean type to binary \"-\"");
          YYERROR;
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival - $3.ival); //TODO: overflow
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval - $3.uval); //TODO: overflow
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
          $$ = unoidl::detail::SourceProviderExpr::Float($1.fval - $3.fval);
          break;
      }
  }
| multExpr
;

multExpr:
  multExpr '*' unaryExpr
  {
      if (!coerce(@1, yyscanner, &$1, &$3)) {
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
          error(@1, yyscanner, "arguments of boolean type to \"*\"");
          YYERROR;
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival * $3.ival); //TODO: overflow
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval * $3.uval); //TODO: overflow
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
          $$ = unoidl::detail::SourceProviderExpr::Float($1.fval * $3.fval);
          break;
      }
  }
| multExpr '/' unaryExpr
  {
      if (!coerce(@1, yyscanner, &$1, &$3)) {
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
          error(@1, yyscanner, "arguments of boolean type to \"/\"");
          YYERROR;
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          if ($3.ival == 0) {
              error(@3, yyscanner, "cannot divide by zero");
              YYERROR;
          }
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival / $3.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          if ($3.uval == 0) {
              error(@3, yyscanner, "cannot divide by zero");
              YYERROR;
          }
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval / $3.uval);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
          if ($3.fval == 0) {
              error(@3, yyscanner, "cannot divide by zero");
              YYERROR;
          }
          $$ = unoidl::detail::SourceProviderExpr::Float($1.fval - $3.fval);
          break;
      }
  }
| multExpr '%' unaryExpr
  {
      if (!coerce(@1, yyscanner, &$1, &$3)) {
          YYERROR;
      }
      switch ($1.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          if ($3.ival == 0) {
              error(@3, yyscanner, "cannot divide by zero");
              YYERROR;
          }
          $$ = unoidl::detail::SourceProviderExpr::Int($1.ival % $3.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          if ($3.uval == 0) {
              error(@3, yyscanner, "cannot divide by zero");
              YYERROR;
          }
          $$ = unoidl::detail::SourceProviderExpr::Uint($1.uval % $3.uval);
          break;
      default:
          error(@1, yyscanner, "arguments of non-integer type to \"%\"");
          YYERROR;
          break;
      }
  }
| unaryExpr
;

unaryExpr:
  '+' primaryExpr
  {
      if ($2.type == unoidl::detail::SourceProviderExpr::TYPE_BOOL) {
          error(@2, yyscanner, "argument of boolean type to unary \"+\"");
          YYERROR;
      }
      $$ = $2;
  }
| '-' primaryExpr
  {
      switch ($2.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_BOOL:
          error(@2, yyscanner, "argument of boolean type to unary \"-\"");
          YYERROR;
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          if ($2.ival == SAL_MIN_INT64) {
              error(@2, yyscanner, "cannot negate -2^63");
              YYERROR;
          }
          $$ = unoidl::detail::SourceProviderExpr::Int(-$2.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          if ($2.uval == SAL_CONST_UINT64(0x8000000000000000)) {
              $$ = unoidl::detail::SourceProviderExpr::Int(SAL_MIN_INT64);
          } else {
              if ($2.uval > SAL_MAX_INT64) {
                  error(
                      @2, yyscanner,
                      ("cannot negate out-of-range value "
                       + OUString::number($2.uval)));
                  YYERROR;
              }
              $$ = unoidl::detail::SourceProviderExpr::Int(
                  -static_cast<sal_Int64>($2.uval));
          }
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_FLOAT:
          $$ = unoidl::detail::SourceProviderExpr::Float(-$2.fval);
          break;
      }
  }
| '~' primaryExpr
  {
      switch ($2.type) {
      case unoidl::detail::SourceProviderExpr::TYPE_INT:
          $$ = unoidl::detail::SourceProviderExpr::Int(~$2.ival);
          break;
      case unoidl::detail::SourceProviderExpr::TYPE_UINT:
          $$ = unoidl::detail::SourceProviderExpr::Uint(~$2.uval);
          break;
      default:
          error(@2, yyscanner, "argument of non-integer type to \"~\"");
          YYERROR;
          break;
      }
  }
| primaryExpr
;

primaryExpr:
  '(' expr ')' { $$ = $2; }
| TOK_FALSE { $$ = unoidl::detail::SourceProviderExpr::Bool(false); }
| TOK_TRUE { $$ = unoidl::detail::SourceProviderExpr::Bool(true); }
| TOK_INTEGER { $$ = unoidl::detail::SourceProviderExpr::Uint($1); }
| TOK_FLOATING { $$ = unoidl::detail::SourceProviderExpr::Float($1); }
| name
  {
      OUString name(convertName($1));
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      unoidl::ConstantValue v(false); // dummy value
      bool found = false;
      bool unpub = false;
      sal_Int32 i = name.lastIndexOf('.');
      if (i == -1) {
          rtl::Reference<unoidl::detail::SourceProviderEntityPad> pad(
              getCurrentEntity(data)->pad);
          unoidl::detail::SourceProviderEnumTypeEntityPad * p1 = dynamic_cast<
              unoidl::detail::SourceProviderEnumTypeEntityPad *>(pad.get());
          if (p1 != nullptr) {
              for (auto & j: p1->members) {
                  if (j.name == name) {
                      v = unoidl::ConstantValue(j.value);
                      found = true;
                      break;
                  }
              }
          } else {
              unoidl::detail::SourceProviderConstantGroupEntityPad * p2
                  = dynamic_cast<
                      unoidl::detail::SourceProviderConstantGroupEntityPad *>(
                          pad.get());
              if (p2 != nullptr) {
                  for (auto & j: p2->members) {
                      if (j.name == name) {
                          v = j.value;
                          found = true;
                          break;
                      }
                  }
              }
          }
      } else {
          OUString scope(name.copy(0, i));
          unoidl::detail::SourceProviderEntity const * ent;
          if (findEntity(
                  @1, yyscanner, data, false, &scope, &ent, nullptr, nullptr)
              == FOUND_ERROR)
          {
              YYERROR;
          }
          if (ent != nullptr) {
              OUString id(name.copy(i + 1));
              // No need to check for enum members here, as they cannot be
              // referenced in expressions by qualified name (TODO: is that true?):
              if (ent->entity.is()) {
                  if (ent->entity->getSort()
                      == unoidl::Entity::SORT_CONSTANT_GROUP)
                  {
                      std::vector<unoidl::ConstantGroupEntity::Member> const &
                          mems(
                              static_cast<unoidl::ConstantGroupEntity *>(
                                  ent->entity.get())->
                              getMembers());
                      for (auto & j: mems) {
                          if (j.name == id) {
                              v = j.value;
                              found = true;
                              unpub
                                  = !static_cast<unoidl::ConstantGroupEntity *>(
                                      ent->entity.get())->isPublished();
                              break;
                          }
                      }
                  }
              } else if (ent->pad.is()) {
                  unoidl::detail::SourceProviderConstantGroupEntityPad * pad
                      = dynamic_cast<
                          unoidl::detail::SourceProviderConstantGroupEntityPad *>(
                              ent->pad.get());
                  if (pad != nullptr) {
                      for (auto & j: pad->members) {
                          if (j.name == id) {
                              v = j.value;
                              found = true;
                              unpub = !ent->pad->isPublished();
                              break;
                          }
                      }
                  }
              }
          }
      }
      if (!found) {
          error(
              @1, yyscanner,
              (name
               + (" does not resolve to neither a constant nor an unqualified"
                  " enum member")));
          YYERROR;
      }
      if (data->publishedContext && unpub) {
          error(
              @1, yyscanner,
              "unpublished value " + name + " used in published context");
          YYERROR;
      }
      switch (v.type) {
      case unoidl::ConstantValue::TYPE_BOOLEAN:
          $$ = unoidl::detail::SourceProviderExpr::Bool(v.booleanValue);
          break;
      case unoidl::ConstantValue::TYPE_BYTE:
          $$ = unoidl::detail::SourceProviderExpr::Int(v.byteValue);
          break;
      case unoidl::ConstantValue::TYPE_SHORT:
          $$ = unoidl::detail::SourceProviderExpr::Int(v.shortValue);
          break;
      case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
          $$ = unoidl::detail::SourceProviderExpr::Uint(v.unsignedShortValue);
          break;
      case unoidl::ConstantValue::TYPE_LONG:
          $$ = unoidl::detail::SourceProviderExpr::Int(v.longValue);
          break;
      case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
          $$ = unoidl::detail::SourceProviderExpr::Uint(v.unsignedLongValue);
          break;
      case unoidl::ConstantValue::TYPE_HYPER:
          $$ = unoidl::detail::SourceProviderExpr::Int(v.hyperValue);
          break;
      case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
          $$ = unoidl::detail::SourceProviderExpr::Uint(v.unsignedHyperValue);
          break;
      case unoidl::ConstantValue::TYPE_FLOAT:
          $$ = unoidl::detail::SourceProviderExpr::Float(v.floatValue);
          break;
      case unoidl::ConstantValue::TYPE_DOUBLE:
          $$ = unoidl::detail::SourceProviderExpr::Float(v.doubleValue);
          break;
      }
  }
;

typeArguments:
  typeArguments ',' type
  {
      unoidl::detail::SourceProviderType t(*$3);
      delete $3;
      if (!checkTypeArgument(@3, yyscanner, t)) {
          delete $1; /* see commented-out %destructor above */
          YYERROR;
      }
      $1->push_back(t);
      $$ = $1;
  }
| type
  {
      unoidl::detail::SourceProviderType t(*$1);
      delete $1;
      if (!checkTypeArgument(@1, yyscanner, t)) {
          YYERROR;
      }
      $$ = new std::vector<unoidl::detail::SourceProviderType>;
      $$->push_back(t);
  }
;

type:
  TOK_VOID
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_VOID);
  }
| TOK_BOOLEAN
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_BOOLEAN);
  }
| TOK_BYTE
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_BYTE);
  }
| TOK_SHORT
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_SHORT);
  }
| TOK_UNSIGNED TOK_SHORT
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_UNSIGNED_SHORT);
  }
| TOK_LONG
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_LONG);
  }
| TOK_UNSIGNED TOK_LONG
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_UNSIGNED_LONG);
  }
| TOK_HYPER
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_HYPER);
  }
| TOK_UNSIGNED TOK_HYPER
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_UNSIGNED_HYPER);
  }
| TOK_FLOAT
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_FLOAT);
  }
| TOK_DOUBLE
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_DOUBLE);
  }
| TOK_CHAR
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_CHAR);
  }
| TOK_STRING
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_STRING);
  }
| TOK_TYPE
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_TYPE);
  }
| TOK_ANY
  {
      $$ = new unoidl::detail::SourceProviderType(
          unoidl::detail::SourceProviderType::TYPE_ANY);
  }
| TOK_SEQUENCE '<' type '>'
  {
      switch ($3->type) {
      case unoidl::detail::SourceProviderType::TYPE_VOID:
      case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
      case unoidl::detail::SourceProviderType::TYPE_PARAMETER: //TODO?
          error(@3, yyscanner, "illegal sequence type component type");
          YYERROR;
          break;
      default:
          break;
      }
      $$ = new unoidl::detail::SourceProviderType($3);
      delete $3;
  }
| name
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString name(convertName($1));
      bool done = false;
      if (name.indexOf('.') == -1 && !data->currentName.isEmpty()) {
          unoidl::detail::SourceProviderEntity * ent = getCurrentEntity(data);
          unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *
              pad = dynamic_cast<
                  unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *>(
                      ent->pad.get());
          if (pad != nullptr
              && (std::find(
                      pad->typeParameters.begin(), pad->typeParameters.end(),
                      name)
                  != pad->typeParameters.end()))
          {
              $$ = new unoidl::detail::SourceProviderType(name);
              done = true;
          }
      }
      if (!done) {
          unoidl::detail::SourceProviderEntity const * ent;
          unoidl::detail::SourceProviderType t;
          switch (findEntity(
                      @1, yyscanner, data, false, &name, &ent, nullptr, &t))
          {
          case FOUND_ERROR:
              YYERROR;
              break;
          case FOUND_TYPE:
              $$ = new unoidl::detail::SourceProviderType(t);
              break;
          case FOUND_ENTITY:
              if (ent == nullptr) {
                  error(@1, yyscanner, "unknown entity " + name);
                  YYERROR;
              }
              bool ok = false;
              switch (ent->kind) {
              case unoidl::detail::SourceProviderEntity::KIND_LOCAL:
                  if (ent->pad.is()) {
                      if (data->publishedContext && !ent->pad->isPublished()) {
                          error(
                              @1, yyscanner,
                              ("unpublished entity " + name
                               + " used in published context"));
                          YYERROR;
                      }
                      if (dynamic_cast<unoidl::detail::SourceProviderEnumTypeEntityPad *>(
                              ent->pad.get())
                          != nullptr)
                      {
                          $$ = new unoidl::detail::SourceProviderType(
                              unoidl::detail::SourceProviderType::TYPE_ENUM,
                              name, ent);
                          ok = true;
                      } else if (dynamic_cast<unoidl::detail::SourceProviderPlainStructTypeEntityPad *>(
                                     ent->pad.get())
                                 != nullptr)
                      {
                          $$ = new unoidl::detail::SourceProviderType(
                              unoidl::detail::SourceProviderType::TYPE_PLAIN_STRUCT,
                              name, ent);
                          ok = true;
                      } else if (dynamic_cast<unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *>(
                                     ent->pad.get())
                                 != nullptr)
                      {
                          error(
                              @1, yyscanner,
                              (("recursive reference to polymorphic struct type"
                                " template ")
                               + name));
                          YYERROR;
                      } else if (dynamic_cast<unoidl::detail::SourceProviderExceptionTypeEntityPad *>(
                                     ent->pad.get())
                                 != nullptr)
                      {
                          $$ = new unoidl::detail::SourceProviderType(
                              unoidl::detail::SourceProviderType::TYPE_EXCEPTION,
                              name, ent);
                          ok = true;
                      } else if (dynamic_cast<unoidl::detail::SourceProviderInterfaceTypeEntityPad *>(
                                     ent->pad.get())
                                 != nullptr)
                      {
                          $$ = new unoidl::detail::SourceProviderType(
                              unoidl::detail::SourceProviderType::TYPE_INTERFACE,
                              name, ent);
                          ok = true;
                      }
                      break;
                  }
                  assert(ent->entity.is());
                  // fall through
              case unoidl::detail::SourceProviderEntity::KIND_EXTERNAL:
                  if (data->publishedContext
                      && ent->entity->getSort() != unoidl::Entity::SORT_MODULE
                      && !static_cast<unoidl::PublishableEntity *>(
                          ent->entity.get())->isPublished())
                  {
                      error(
                          @1, yyscanner,
                          ("unpublished entity " + name
                           + " used in published context"));
                      YYERROR;
                  }
                  switch (ent->entity->getSort()) {
                  case unoidl::Entity::SORT_ENUM_TYPE:
                      $$ = new unoidl::detail::SourceProviderType(
                          unoidl::detail::SourceProviderType::TYPE_ENUM, name,
                          ent);
                      ok = true;
                      break;
                  case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
                      $$ = new unoidl::detail::SourceProviderType(
                          unoidl::detail::SourceProviderType::TYPE_PLAIN_STRUCT,
                          name, ent);
                      ok = true;
                      break;
                  case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
                      error(
                          @1, yyscanner,
                          ("polymorphic struct type template " + name
                           + " without type arguments"));
                      YYERROR;
                      break;
                  case unoidl::Entity::SORT_EXCEPTION_TYPE:
                      $$ = new unoidl::detail::SourceProviderType(
                          unoidl::detail::SourceProviderType::TYPE_EXCEPTION,
                          name, ent);
                      ok = true;
                      break;
                  case unoidl::Entity::SORT_INTERFACE_TYPE:
                      $$ = new unoidl::detail::SourceProviderType(
                          unoidl::detail::SourceProviderType::TYPE_INTERFACE,
                          name, ent);
                      ok = true;
                      break;
                  case unoidl::Entity::SORT_TYPEDEF:
                      assert(false && "this cannot happen");
                      // fall through
                  default:
                      break;
                  }
                  break;
              case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
                  if (data->publishedContext) {
                      error(
                          @1, yyscanner,
                          ("unpublished entity " + name
                           + " used in published context"));
                      YYERROR;
                  }
                  // fall through
              case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
                  $$ = new unoidl::detail::SourceProviderType(
                      unoidl::detail::SourceProviderType::TYPE_INTERFACE, name,
                      ent);
                  ok = true;
                  break;
              case unoidl::detail::SourceProviderEntity::KIND_MODULE:
                  assert(false && "this cannot happen");
              }
              if (!ok) {
                  error(@1, yyscanner, "non-type entity " + name);
                  YYERROR;
              }
              break;
          }
      }
  }
| name '<' typeArguments '>'
  {
      unoidl::detail::SourceProviderScannerData * data = yyget_extra(yyscanner);
      OUString name(convertName($1));
      std::vector<unoidl::detail::SourceProviderType> args(*$3);
      delete $3;
      unoidl::detail::SourceProviderEntity const * ent;
      if (findEntity(@1, yyscanner, data, false, &name, &ent, nullptr, nullptr)
          == FOUND_ERROR)
      {
          YYERROR;
      }
      if (ent == nullptr) {
          error(@1, yyscanner, "unknown entity " + name);
          YYERROR;
      }
      bool ok = false;
      switch (ent->kind) {
      case unoidl::detail::SourceProviderEntity::KIND_LOCAL:
          if (ent->pad.is()) {
              if (dynamic_cast<unoidl::detail::SourceProviderPolymorphicStructTypeTemplateEntityPad *>(
                      ent->pad.get())
                  != nullptr)
              {
                  error(
                      @1, yyscanner,
                      (("recursive reference to polymorphic struct type"
                        " template ")
                       + name));
                  YYERROR;
              }
              break;
          }
          assert(ent->entity.is());
          // fall through
      case unoidl::detail::SourceProviderEntity::KIND_EXTERNAL:
          if (ent->entity->getSort()
              == unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE)
          {
              rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity> e(
                  static_cast<unoidl::PolymorphicStructTypeTemplateEntity *>(
                      ent->entity.get()));
              if (args.size() != e->getTypeParameters().size()) {
                  error(
                      @1, yyscanner,
                      ("bad number of polymorphic struct type template " + name
                       + " type arguments"));
                  YYERROR;
              }
              if (data->publishedContext && !e->isPublished()) {
                  error(
                      @1, yyscanner,
                      ("unpublished polymorphic struct type template " + name
                       + " used in published context"));
                  YYERROR;
              }
              $$ = new unoidl::detail::SourceProviderType(name, ent, args);
              ok = true;
          }
          break;
      case unoidl::detail::SourceProviderEntity::KIND_INTERFACE_DECL:
      case unoidl::detail::SourceProviderEntity::KIND_PUBLISHED_INTERFACE_DECL:
          break;
      case unoidl::detail::SourceProviderEntity::KIND_MODULE:
          assert(false && "this cannot happen");
      }
      if (!ok) {
          error(@1, yyscanner, "non-type entity " + name);
          YYERROR;
      }
  }
;

name:
  name TOK_COLONS identifier { *$1 += "." + *$3; delete $3; $$ = $1; }
| TOK_COLONS identifier { *$2 = "." + *$2; $$ = $2; }
| identifier
;

identifier:
  TOK_IDENTIFIER
| TOK_GET { $$ = new OString("get"); }
| TOK_PUBLISHED { $$ = new OString("published"); }
| TOK_SET { $$ = new OString("set"); }
;

deprecated_opt:
  TOK_DEPRECATED { $$ = true; }
| /* empty */ { $$ = false; }
;

%%

namespace unoidl { namespace detail {

OUString SourceProviderType::getName() const {
    if (!typedefName.isEmpty()) {
        return typedefName;
    }
    switch (type) {
    case unoidl::detail::SourceProviderType::TYPE_VOID:
        return OUString("void");
    case unoidl::detail::SourceProviderType::TYPE_BOOLEAN:
        return OUString("boolean");
    case unoidl::detail::SourceProviderType::TYPE_BYTE:
        return OUString("byte");
    case unoidl::detail::SourceProviderType::TYPE_SHORT:
        return OUString("short");
    case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_SHORT:
        return OUString("unsigned short");
    case unoidl::detail::SourceProviderType::TYPE_LONG:
        return OUString("long");
    case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_LONG:
        return OUString("unsigned long");
    case unoidl::detail::SourceProviderType::TYPE_HYPER:
        return OUString("hyper");
    case unoidl::detail::SourceProviderType::TYPE_UNSIGNED_HYPER:
        return OUString("unsigned hyper");
    case unoidl::detail::SourceProviderType::TYPE_FLOAT:
        return OUString("float");
    case unoidl::detail::SourceProviderType::TYPE_DOUBLE:
        return OUString("double");
    case unoidl::detail::SourceProviderType::TYPE_CHAR:
        return OUString("char");
    case unoidl::detail::SourceProviderType::TYPE_STRING:
        return OUString("string");
    case unoidl::detail::SourceProviderType::TYPE_TYPE:
        return OUString("type");
    case unoidl::detail::SourceProviderType::TYPE_ANY:
        return OUString("any");
    case unoidl::detail::SourceProviderType::TYPE_SEQUENCE:
        assert(subtypes.size() == 1);
        return "[]" + subtypes.front().getName();
    case unoidl::detail::SourceProviderType::TYPE_ENUM:
    case unoidl::detail::SourceProviderType::TYPE_PLAIN_STRUCT:
    case unoidl::detail::SourceProviderType::TYPE_EXCEPTION:
    case unoidl::detail::SourceProviderType::TYPE_INTERFACE:
    case unoidl::detail::SourceProviderType::TYPE_PARAMETER:
        return name;
    case unoidl::detail::SourceProviderType::TYPE_INSTANTIATED_POLYMORPHIC_STRUCT:
        {
            OUString n(name + "<");
            for (auto i(subtypes.begin()); i != subtypes.end(); ++i) {
                if (i != subtypes.begin()) {
                    n += ",";
                }
                n += i->getName();
            }
            return n + ">";
        }
    default:
        assert(false && "this cannot happen"); for (;;) { std::abort(); }
    }
}

bool SourceProviderType::equals(SourceProviderType const & other) const {
    if (type != other.type || name != other.name
        || subtypes.size() != other.subtypes.size())
    {
        return false;
    }
    for (auto i(subtypes.begin()), j(other.subtypes.begin());
         i != subtypes.end(); ++i, ++j)
    {
        if (!i->equals(*j)) {
            return false;
        }
    }
    return true;
}

bool SourceProviderInterfaceTypeEntityPad::addDirectBase(
    YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
    DirectBase const & base, bool optional)
{
    std::set<OUString> seen;
    if (!(checkBaseClashes(
              location, yyscanner, data, base.name, base.entity, true, optional,
              optional, &seen)
          && addBase(
              location, yyscanner, data, base.name, base.name, base.entity,
              true, optional)))
    {
        return false;
    }
    if (optional) {
        addOptionalBaseMembers(
            location, yyscanner, data, base.name, base.entity);
    }
    (optional ? directOptionalBases : directMandatoryBases).push_back(base);
    return true;
}

bool SourceProviderInterfaceTypeEntityPad::addDirectMember(
    YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
    OUString const & name)
{
    assert(data != nullptr);
    if (!checkMemberClashes(location, yyscanner, data, "", name, true)) {
        return false;
    }
    allMembers.insert(
        std::map<OUString, Member>::value_type(
            name, Member(data->currentName)));
    return true;
}

bool SourceProviderInterfaceTypeEntityPad::checkBaseClashes(
    YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
    OUString const & name,
    rtl::Reference<unoidl::InterfaceTypeEntity> const & entity, bool direct,
    bool optional, bool outerOptional, std::set<OUString> * seen) const
{
    assert(data != nullptr);
    assert(entity.is());
    assert(seen != nullptr);
    if (direct || optional || seen->insert(name).second) {
        std::map<OUString, BaseKind>::const_iterator i(allBases.find(name));
        if (i != allBases.end()) {
            switch (i->second) {
            case BASE_INDIRECT_OPTIONAL:
                if (direct && optional) {
                    error(
                        location, yyscanner,
                        ("interface type " + data->currentName
                         + " duplicate base " + name));
                    return false;
                }
                break;
            case BASE_DIRECT_OPTIONAL:
                if (direct || !outerOptional) {
                    error(
                        location, yyscanner,
                        ("interface type " + data->currentName
                         + " duplicate base " + name));
                    return false;
                }
                return true;
            case BASE_INDIRECT_MANDATORY:
                if (direct) {
                    error(
                        location, yyscanner,
                        ("interface type " + data->currentName
                         + " duplicate base " + name));
                    return false;
                }
                return true;
            case BASE_DIRECT_MANDATORY:
                if (direct || (!optional && !outerOptional)) {
                    error(
                        location, yyscanner,
                        ("interface type " + data->currentName
                         + " duplicate base " + name));
                    return false;
                }
                return true;
            }
        }
        if (direct || !optional) {
            for (auto & j: entity->getDirectMandatoryBases()) {
                OUString n("." + j.name);
                unoidl::detail::SourceProviderEntity const * p;
                if (findEntity(
                        location, yyscanner, data, true, &n, &p, nullptr,
                        nullptr)
                    == FOUND_ERROR)
                {
                    return false;
                }
                if (p == nullptr || !p->entity.is()
                    || (p->entity->getSort()
                        != unoidl::Entity::SORT_INTERFACE_TYPE))
                {
                    error(
                        location, yyscanner,
                        ("inconsistent type manager: interface type "
                         + data->currentName + " base " + n
                         + " does not resolve to an existing interface type"));
                    return false;
                }
                if (!checkBaseClashes(
                        location, yyscanner, data, n,
                        static_cast<unoidl::InterfaceTypeEntity *>(
                            p->entity.get()),
                        false, false, outerOptional, seen))
                {
                    return false;
                }
            }
            for (auto & j: entity->getDirectOptionalBases()) {
                OUString n("." + j.name);
                unoidl::detail::SourceProviderEntity const * p;
                if (findEntity(
                        location, yyscanner, data, true, &n, &p, nullptr,
                        nullptr)
                    == FOUND_ERROR)
                {
                    return false;
                }
                if (p == nullptr || !p->entity.is()
                    || (p->entity->getSort()
                        != unoidl::Entity::SORT_INTERFACE_TYPE))
                {
                    error(
                        location, yyscanner,
                        ("inconsistent type manager: interface type "
                         + data->currentName + " base " + n
                         + " does not resolve to an existing interface type"));
                    return false;
                }
                if (!checkBaseClashes(
                        location, yyscanner, data, n,
                        static_cast<unoidl::InterfaceTypeEntity *>(
                            p->entity.get()),
                        false, true, outerOptional, seen))
                {
                    return false;
                }
            }
            for (auto & j: entity->getDirectAttributes()) {
                if (!checkMemberClashes(
                        location, yyscanner, data, name, j.name,
                        !outerOptional))
                {
                    return false;
                }
            }
            for (auto & j: entity->getDirectMethods()) {
                if (!checkMemberClashes(
                        location, yyscanner, data, name, j.name,
                        !outerOptional))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool SourceProviderInterfaceTypeEntityPad::checkMemberClashes(
    YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
    OUString const & interfaceName, OUString const & memberName,
    bool checkOptional) const
{
    std::map<OUString, Member>::const_iterator i(allMembers.find(memberName));
    if (i != allMembers.end()) {
        if (!i->second.mandatory.isEmpty()) {
            // For a direct member, interfaceName will be empty, so this will
            // catch two direct members with the same name:
            if (i->second.mandatory != interfaceName) {
                error(
                    location, yyscanner,
                    ("interface type " + data->currentName
                     + " duplicate member " + memberName));
                return false;
            }
        } else if (checkOptional) {
            for (auto & j: i->second.optional) {
                if (j != interfaceName) {
                    error(
                        location, yyscanner,
                        ("interface type " + data->currentName
                         + " duplicate member " + memberName));
                    return false;
                }
            }
        }
    }
    return true;
}

bool SourceProviderInterfaceTypeEntityPad::addBase(
    YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
    OUString const & directBaseName, OUString const & name,
    rtl::Reference<unoidl::InterfaceTypeEntity> const & entity, bool direct,
    bool optional)
{
    assert(data != nullptr);
    assert(entity.is());
    BaseKind kind = optional
        ? direct ? BASE_DIRECT_OPTIONAL : BASE_INDIRECT_OPTIONAL
        : direct ? BASE_DIRECT_MANDATORY : BASE_INDIRECT_MANDATORY;
    std::pair<std::map<OUString, BaseKind>::iterator, bool> p(
        allBases.insert(
            std::map<OUString, BaseKind>::value_type(name, kind)));
    bool seen = !p.second && p.first->second >= BASE_INDIRECT_MANDATORY;
    if (!p.second && kind > p.first->second) {
        p.first->second = kind;
    }
    if (!optional && !seen) {
        for (auto & i: entity->getDirectMandatoryBases()) {
            OUString n("." + i.name);
            unoidl::detail::SourceProviderEntity const * q;
            if (findEntity(
                    location, yyscanner, data, true, &n, &q, nullptr, nullptr)
                == FOUND_ERROR)
            {
                return false;
            }
            if (q == nullptr || !q->entity.is()
                || q->entity->getSort() != unoidl::Entity::SORT_INTERFACE_TYPE)
            {
                error(
                    location, yyscanner,
                    ("inconsistent type manager: interface type "
                     + data->currentName + " base " + n
                     + " does not resolve to an existing interface type"));
                return false;
            }
            if (!addBase(
                    location, yyscanner, data, directBaseName, n,
                    static_cast<unoidl::InterfaceTypeEntity *>(q->entity.get()),
                    false, false))
            {
                return false;
            }
        }
        for (auto & i: entity->getDirectOptionalBases())
        {
            OUString n("." + i.name);
            unoidl::detail::SourceProviderEntity const * q;
            if (findEntity(
                    location, yyscanner, data, true, &n, &q, nullptr, nullptr)
                == FOUND_ERROR)
            {
                return false;
            }
            if (q == nullptr || !q->entity.is()
                || q->entity->getSort() != unoidl::Entity::SORT_INTERFACE_TYPE)
            {
                error(
                    location, yyscanner,
                    ("inconsistent type manager: interface type "
                     + data->currentName + " base " + n
                     + " does not resolve to an existing interface type"));
                return false;
            }
            if (!addBase(
                    location, yyscanner, data, directBaseName, n,
                    static_cast<unoidl::InterfaceTypeEntity *>(q->entity.get()),
                    false, true))
            {
                return false;
            }
        }
        for (auto & i: entity->getDirectAttributes()) {
            allMembers.insert(
                std::map<OUString, Member>::value_type(i.name, Member(name)));
        }
        for (auto & i: entity->getDirectMethods()) {
            allMembers.insert(
                std::map<OUString, Member>::value_type(i.name, Member(name)));
        }
    }
    return true;
}

bool SourceProviderInterfaceTypeEntityPad::addOptionalBaseMembers(
    YYLTYPE location, yyscan_t yyscanner, SourceProviderScannerData * data,
    OUString const & name,
    rtl::Reference<unoidl::InterfaceTypeEntity> const & entity)
{
    assert(entity.is());
    for (auto & i: entity->getDirectMandatoryBases()) {
        OUString n("." + i.name);
        unoidl::detail::SourceProviderEntity const * p;
        if (findEntity(
                location, yyscanner, data, true, &n, &p, nullptr, nullptr)
            == FOUND_ERROR)
        {
            return false;
        }
        if (p == nullptr || !p->entity.is()
            || p->entity->getSort() != unoidl::Entity::SORT_INTERFACE_TYPE)
        {
            error(
                location, yyscanner,
                ("inconsistent type manager: interface type "
                 + data->currentName + " base " + n
                 + " does not resolve to an existing interface type"));
            return false;
        }
        if (!addOptionalBaseMembers(
                location, yyscanner, data, n,
                static_cast<unoidl::InterfaceTypeEntity *>(p->entity.get())))
        {
            return false;
        }
    }
    for (auto & i: entity->getDirectAttributes()) {
        Member & m(
            allMembers.insert(
                std::map<OUString, Member>::value_type(i.name, Member("")))
            .first->second);
        if (m.mandatory.isEmpty()) {
            m.optional.insert(name);
        }
    }
    for (auto & i: entity->getDirectMethods()) {
        Member & m(
            allMembers.insert(
                std::map<OUString, Member>::value_type(i.name, Member("")))
            .first->second);
        if (m.mandatory.isEmpty()) {
            m.optional.insert(name);
        }
    }
    return true;
}

bool parse(OUString const & uri, SourceProviderScannerData * data) {
    assert(data != nullptr);
    oslFileHandle handle;
    oslFileError e = osl_openFile(uri.pData, &handle, osl_File_OpenFlag_Read);
    switch (e) {
    case osl_File_E_None:
        break;
    case osl_File_E_NOENT:
        return false;
    default:
        throw FileFormatException(uri, "cannot open: " + OUString::number(e));
    }
    sal_uInt64 size;
    e = osl_getFileSize(handle, &size);
    if (e != osl_File_E_None) {
        oslFileError e2 = osl_closeFile(handle);
        SAL_WARN_IF(
            e2 != osl_File_E_None, "unoidl",
            "cannot close " << uri << ": " << +e2);
        throw FileFormatException(
            uri, "cannot get size: " + OUString::number(e));
    }
    void * address;
    e = osl_mapFile(handle, &address, size, 0, osl_File_MapFlag_RandomAccess);
    if (e != osl_File_E_None) {
        oslFileError e2 = osl_closeFile(handle);
        SAL_WARN_IF(
            e2 != osl_File_E_None, "unoidl",
            "cannot close " << uri << ": " << +e2);
        throw FileFormatException(uri, "cannot mmap: " + OUString::number(e));
    }
    try {
        data->setSource(address, size);
        yyscan_t yyscanner;
        if (yylex_init_extra(data, &yyscanner) != 0) {
            // Checking errno for the specific EINVAL, ENOMEM documented for
            // yylex_init_extra would not work as those values are not defined
            // by the C++ Standard:
            int e2 = errno;
            throw FileFormatException(
                uri,
                "yylex_init_extra failed with errno " + OUString::number(e2));
        }
        int e2 = yyparse(yyscanner);
        yylex_destroy(yyscanner);
        switch (e2) {
        case 0:
            break;
        default:
            assert(false);
            // fall through
        case 1:
            throw FileFormatException(
                uri,
                ("cannot parse"
                 + (data->errorLine == 0
                    ? OUString() : " line " + OUString::number(data->errorLine))
                 + (data->parserError.isEmpty()
                    ? OUString()
                    : (", "
                       + OStringToOUString(
                           data->parserError, osl_getThreadTextEncoding())))
                 + (data->errorMessage.isEmpty()
                    ? OUString() : ": \"" + data->errorMessage + "\"")));
        case 2:
            throw std::bad_alloc();
        }
    } catch (...) {
        e = osl_unmapMappedFile(handle, address, size);
        SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot unmap: " << +e);
        e = osl_closeFile(handle);
        SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot close: " << +e);
        throw;
    }
    e = osl_unmapMappedFile(handle, address, size);
    SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot unmap: " << +e);
    e = osl_closeFile(handle);
    SAL_WARN_IF(e != osl_File_E_None, "unoidl", "cannot close: " << +e);
    return true;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
