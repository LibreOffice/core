/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>

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

TypeCheck TypeCheck::ConstNonVolatile() const {
    return
        (!type_.isNull() && type_.isConstQualified()
         && !type_.isVolatileQualified())
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
        || n == "gboolean" || n == "hb_bool_t" || n == "jboolean" || n == "my_bool");
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

TypeCheck TypeCheck::RvalueReference() const {
    if (!type_.isNull()) {
        auto const t = type_->getAs<clang::RValueReferenceType>();
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

TypeCheck TypeCheck::MemberPointerOf() const {
    if (!type_.isNull()) {
        auto const t = type_->getAs<clang::MemberPointerType>();
        if (t != nullptr) {
            return TypeCheck(compat::getClass(t));
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

DeclCheck TypeCheck::TemplateSpecializationClass() const {
    if (!type_.isNull()) {
        if (auto const t = type_->getAs<clang::TemplateSpecializationType>()) {
            if (!t->isTypeAlias()) {
                if (auto const d = llvm::dyn_cast_or_null<clang::ClassTemplateDecl>(
                        t->getTemplateName().getAsTemplateDecl()))
                {
                    return DeclCheck(d->getTemplatedDecl());
                }
            }
        }
    }
    return DeclCheck();
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

namespace {

bool isGlobalNamespace(clang::DeclContext const * context) {
    assert(context != nullptr);
    return context->getEnclosingNamespaceContext()->isTranslationUnit();
}

}

TerminalCheck ContextCheck::GlobalNamespace() const {
    return TerminalCheck(context_ != nullptr && isGlobalNamespace(context_));
}

TerminalCheck ContextCheck::StdNamespace() const {
    return TerminalCheck(
        context_ != nullptr && lookThroughLinkageSpec()->isStdNamespace());
}

namespace {

bool isStdOrNestedNamespace(clang::DeclContext const * context) {
    assert(context != nullptr);
    if (!context->isNamespace()) {
        return false;
    }
    if (isGlobalNamespace(context)) {
        return false;
    }
    if (context->isStdNamespace()) {
        return true;
    }
    return isStdOrNestedNamespace(context->getParent());
}

}

TerminalCheck ContextCheck::StdOrNestedNamespace() const {
    return TerminalCheck(context_ != nullptr && isStdOrNestedNamespace(lookThroughLinkageSpec()));
}

ContextCheck ContextCheck::AnonymousNamespace() const {
    auto n = llvm::dyn_cast_or_null<clang::NamespaceDecl>(lookThroughLinkageSpec());
    return ContextCheck(
        n != nullptr && n->isAnonymousNamespace() ? n->getParent() : nullptr);
}

clang::DeclContext const * ContextCheck::lookThroughLinkageSpec() const {
    if (context_ != nullptr && context_->getDeclKind() == clang::Decl::LinkageSpec) {
        return context_->getParent();
    }
    return context_;
}

namespace {

bool BaseCheckNotSomethingInterestingSubclass(const clang::CXXRecordDecl *BaseDefinition) {
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
        !decl->forallBases(BaseCheckNotSomethingInterestingSubclass)) {
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
    auto const tc = TypeCheck(rec);
    // Check some common non-LO types:
    if (tc.Class("basic_string").StdNamespace()
        || tc.Class("deque").StdNamespace()
        || tc.Class("list").StdNamespace()
        || tc.Class("map").StdNamespace()
        || tc.Class("pair").StdNamespace()
        || tc.Class("queue").StdNamespace()
        || tc.Class("set").StdNamespace()
        || tc.Class("stack").StdNamespace()
        || tc.Class("unordered_map").StdNamespace()
        || tc.Class("unordered_set").StdNamespace()
        || tc.Class("vector").StdNamespace())
    {
        return true;
    }
    return isDerivedFromSomethingInteresting(rec);
}

namespace {

// Make sure Foo and ::Foo are considered equal:
bool areSameSugaredType(clang::QualType type1, clang::QualType type2) {
    auto t1 = type1.getLocalUnqualifiedType();
    if (auto const et = llvm::dyn_cast<clang::ElaboratedType>(t1)) {
        t1 = et->getNamedType();
    }
    auto t2 = type2.getLocalUnqualifiedType();
    if (auto const et = llvm::dyn_cast<clang::ElaboratedType>(t2)) {
        t2 = et->getNamedType();
    }
    return t1 == t2;
}

bool isArithmeticOp(clang::Expr const * expr) {
    expr = expr->IgnoreParenImpCasts();
    if (auto const e = llvm::dyn_cast<clang::BinaryOperator>(expr)) {
        switch (e->getOpcode()) {
        case clang::BO_Mul:
        case clang::BO_Div:
        case clang::BO_Rem:
        case clang::BO_Add:
        case clang::BO_Sub:
        case clang::BO_Shl:
        case clang::BO_Shr:
        case clang::BO_And:
        case clang::BO_Xor:
        case clang::BO_Or:
            return true;
        case clang::BO_Comma:
            return isArithmeticOp(e->getRHS());
        default:
            return false;
        }
    }
    return llvm::isa<clang::UnaryOperator>(expr)
        || llvm::isa<clang::AbstractConditionalOperator>(expr);
}

}

bool isOkToRemoveArithmeticCast(
    clang::ASTContext & context, clang::QualType t1, clang::QualType t2, const clang::Expr* subExpr)
{
    // Don't warn if the types are arithmetic (in the C++ meaning), and: either
    // at least one is a typedef or decltype (and if both are, they're different),
    // or the sub-expression involves some operation that is likely to change
    // types through promotion, or the sub-expression is an integer literal (so
    // its type generally depends on its value and suffix if any---even with a
    // suffix like L it could still be either long or long long):
    if ((t1->isIntegralType(context)
         || t1->isRealFloatingType())
        && ((!areSameSugaredType(t1, t2)
             && (loplugin::TypeCheck(t1).Typedef()
                 || loplugin::TypeCheck(t2).Typedef()
                 || llvm::isa<clang::DecltypeType>(t1) || llvm::isa<clang::DecltypeType>(t2)))
            || isArithmeticOp(subExpr)
            || llvm::isa<clang::IntegerLiteral>(subExpr->IgnoreParenImpCasts())))
    {
        return false;
    }
    return true;
}


static bool BaseCheckSubclass(const clang::CXXRecordDecl *BaseDefinition, void *p) {
    assert(BaseDefinition != nullptr);
    auto const & base = *static_cast<const DeclChecker *>(p);
    if (base(BaseDefinition)) {
        return true;
    }
    return false;
}

bool forAnyBase(
    clang::CXXRecordDecl const * decl, clang::CXXRecordDecl::ForallBasesCallback matches)
{
    // Based on the implementation of clang::CXXRecordDecl::forallBases in LLVM's
    // clang/lib/AST/CXXInheritance.cpp:
    for (auto const & i: decl->bases()) {
        auto const t = i.getType()->getAs<clang::RecordType>();
        if (t == nullptr) {
            return false;
        }
        auto const b = llvm::cast_or_null<clang::CXXRecordDecl>(t->getDecl()->getDefinition());
        if (b == nullptr || (b->isDependentContext() && !b->isCurrentInstantiation(decl))) {
            return false;
        }
        if (matches(b) || forAnyBase(b, matches)) {
            return true;
        }
    }
    return false;
}

bool isDerivedFrom(const clang::CXXRecordDecl *decl, DeclChecker base, bool checkSelf) {
    if (!decl)
        return false;
    if (checkSelf && base(decl))
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (forAnyBase(decl,
            [&base](const clang::CXXRecordDecl *BaseDefinition) -> bool
                { return BaseCheckSubclass(BaseDefinition, &base); }))
    {
        return true;
    }
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
