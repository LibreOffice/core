/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include "check.hxx"
#include "compat.hxx"

namespace loplugin {

TypeCheck TypeCheck::Const() const {
    return
        (!type_.isNull() && type_.isConstQualified()
         && !type_.isVolatileQualified())
        ? *this : TypeCheck();
        // returning TypeCheck(type_.getUnqualifiedType()) instead of *this
        // may look tempting, but could remove sugar we might be interested in
        // checking for
}

TerminalCheck TypeCheck::Char() const {
    return TerminalCheck(
        !type_.isNull()
        && (type_->isSpecificBuiltinType(clang::BuiltinType::Char_S)
            || type_->isSpecificBuiltinType(clang::BuiltinType::Char_U)));
}

TerminalCheck TypeCheck::AnyBoolean() const {
    if (type_->isBooleanType()) {
        return TerminalCheck(true);
    }
    auto t = type_->getAs<clang::TypedefType>();
    if (t == nullptr) {
        return TerminalCheck(false);
    }
    auto n =t->getDecl()->getName();
    return TerminalCheck(
        n == "sal_Bool" || n == "BOOL" || n == "Boolean" || n == "FT_Bool"
        || n == "FcBool" || n == "GLboolean" || n == "NPBool" || n == "UBool"
        || n == "dbus_bool_t" || n == "gboolean" || n == "hb_bool_t"
        || n == "jboolean");
}

TypeCheck TypeCheck::LvalueReference() const {
    if (!type_.isNull()) {
        auto const t = type_->getAs<clang::LValueReferenceType>();
        if (t != nullptr) {
            return TypeCheck(t->getPointeeType());
        }
    }
    return TypeCheck();
}

TypeCheck TypeCheck::Pointer() const {
    if (!type_.isNull()) {
        auto const t = type_->getAs<clang::PointerType>();
        if (t != nullptr) {
            return TypeCheck(t->getPointeeType());
        }
    }
    return TypeCheck();
}

TypeCheck TypeCheck::Typedef() const {
    if (!type_.isNull()) {
        if (auto const t = type_->getAs<clang::TypedefType>()) {
            return TypeCheck(t->desugar());
        }
    }
    return TypeCheck();
}

TypeCheck TypeCheck::NotSubstTemplateTypeParmType() const {
    return
        (!type_.isNull()
         && type_->getAs<clang::SubstTemplateTypeParmType>() == nullptr)
        ? *this : TypeCheck();
}

ContextCheck DeclCheck::Operator(clang::OverloadedOperatorKind op) const {
    assert(op != clang::OO_None);
    auto f = llvm::dyn_cast_or_null<clang::FunctionDecl>(decl_);
    return ContextCheck(
        f != nullptr && f->getOverloadedOperator() == op
        ? f->getDeclContext() : nullptr);
}

ContextCheck DeclCheck::MemberFunction() const {
    auto m = llvm::dyn_cast_or_null<clang::CXXMethodDecl>(decl_);
    return ContextCheck(m == nullptr ? nullptr : m->getParent());
}

TerminalCheck ContextCheck::GlobalNamespace() const {
    return TerminalCheck(
        context_ != nullptr
        && ((compat::isLookupContext(*context_)
             ? context_ : context_->getLookupParent())
            ->isTranslationUnit()));
}

TerminalCheck ContextCheck::StdNamespace() const {
    return TerminalCheck(
        context_ != nullptr && compat::isStdNamespace(*context_));
}

ContextCheck ContextCheck::AnonymousNamespace() const {
    auto n = llvm::dyn_cast_or_null<clang::NamespaceDecl>(context_);
    return ContextCheck(
        n != nullptr && n->isAnonymousNamespace() ? n->getParent() : nullptr);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
