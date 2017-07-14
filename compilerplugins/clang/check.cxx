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

TypeCheck TypeCheck::NonConst() const {
    return !type_.isNull() && !type_.isConstQualified()
        ? *this : TypeCheck();
        // returning TypeCheck(type_.getUnqualifiedType()) instead of *this
        // may look tempting, but could remove sugar we might be interested in
        // checking for
}

TypeCheck TypeCheck::NonConstVolatile() const {
    return
        (!type_.isNull() && !type_.isConstQualified()
         && !type_.isVolatileQualified())
        ? *this : TypeCheck();
        // returning TypeCheck(type_.getUnqualifiedType()) instead of *this
        // may look tempting, but could remove sugar we might be interested in
        // checking for
}

TypeCheck TypeCheck::Const() const {
    return
        (!type_.isNull() && type_.isConstQualified()
         && !type_.isVolatileQualified())
        ? *this : TypeCheck();
        // returning TypeCheck(type_.getUnqualifiedType()) instead of *this
        // may look tempting, but could remove sugar we might be interested in
        // checking for
}

TypeCheck TypeCheck::Volatile() const {
    return
        (!type_.isNull() && !type_.isConstQualified()
         && type_.isVolatileQualified())
        ? *this : TypeCheck();
        // returning TypeCheck(type_.getUnqualifiedType()) instead of *this
        // may look tempting, but could remove sugar we might be interested in
        // checking for
}

TypeCheck TypeCheck::ConstVolatile() const {
    return
        (!type_.isNull() && type_.isConstQualified()
         && type_.isVolatileQualified())
        ? *this : TypeCheck();
        // returning TypeCheck(type_.getUnqualifiedType()) instead of *this
        // may look tempting, but could remove sugar we might be interested in
        // checking for
}

TerminalCheck TypeCheck::Void() const {
    return TerminalCheck(
        !type_.isNull()
        && type_->isSpecificBuiltinType(clang::BuiltinType::Void));
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
        || n == "FcBool" || n == "GLboolean" || n == "NPBool" || n == "TW_BOOL"
        || n == "UBool" || n == "boolean" || n == "dbus_bool_t"
        || n == "gboolean" || n == "hb_bool_t" || n == "jboolean");
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

TerminalCheck TypeCheck::Enum() const {
    if (!type_.isNull()) {
        auto const t = type_->getAs<clang::EnumType>();
        if (t != nullptr) {
            return TerminalCheck(true);
        }
    }
    return TerminalCheck(false);
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

namespace {

bool BaseCheckNotSomethingInterestingSubclass(
    const clang::CXXRecordDecl *BaseDefinition
#if CLANG_VERSION < 30800
    , void *
#endif
    )
{
    if (BaseDefinition) {
        auto tc = TypeCheck(BaseDefinition);
        if (tc.Class("Dialog").GlobalNamespace() || tc.Class("SfxPoolItem").GlobalNamespace()) {
            return false;
        }
    }
    return true;
}

bool isDerivedFromSomethingInteresting(const clang::CXXRecordDecl *decl) {
    if (!decl)
        return false;
    auto tc = TypeCheck(decl);
    if (tc.Class("Dialog"))
        return true;
    if (tc.Class("SfxPoolItem"))
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !compat::forallBases(*decl, BaseCheckNotSomethingInterestingSubclass, nullptr, true)) {
        return true;
    }
    return false;
}

}

bool isExtraWarnUnusedType(clang::QualType type) {
    auto const rec = type->getAsCXXRecordDecl();
    if (rec == nullptr) {
        return false;
    }
    if (rec->hasAttrs()) {
        // Clang currently has no support for custom attributes, but the
        // annotate attribute comes close, so check for
        // __attribute__((annotate("lo_warn_unused"))):
        for (auto i = rec->specific_attr_begin<clang::AnnotateAttr>(),
                 e = rec->specific_attr_end<clang::AnnotateAttr>();
             i != e; ++i) {
            if ((*i)->getAnnotation() == "lo_warn_unused") {
                return true;
            }
        }
    }
    auto const tc = TypeCheck(rec);
    // Check some common non-LO types:
    if (tc.Class("string").Namespace("std").GlobalNamespace()
        || tc.Class("basic_string").Namespace("std").GlobalNamespace()
        || tc.Class("list").Namespace("std").GlobalNamespace()
        || (tc.Class("list").Namespace("__debug").Namespace("std")
            .GlobalNamespace())
        || tc.Class("vector").Namespace("std").GlobalNamespace()
        || (tc.Class("vector" ).Namespace("__debug").Namespace("std")
            .GlobalNamespace()))
    {
        return true;
    }
    return isDerivedFromSomethingInteresting(rec);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
