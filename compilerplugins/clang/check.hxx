/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPILERPLUGINS_CLANG_CHECK_HXX
#define INCLUDED_COMPILERPLUGINS_CLANG_CHECK_HXX

#include <cstddef>

#include <clang/AST/DeclBase.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/Basic/OperatorKinds.h>

namespace loplugin {

class ContextCheck;
class TerminalCheck;

namespace detail {

inline ContextCheck checkRecordDecl(
    clang::Decl const * decl, clang::TagTypeKind tag, char const * id);

}

class TypeCheck {
public:
    explicit TypeCheck(clang::QualType type): type_(type) {}

    explicit TypeCheck(clang::TypeDecl const * decl): type_(decl->getTypeForDecl(), 0) {}

    explicit operator bool() const { return !type_.isNull(); }

    TypeCheck NonConst() const;

    TypeCheck NonConstVolatile() const;

    TypeCheck Const() const;

    TypeCheck Volatile() const;

    TypeCheck ConstVolatile() const;

    TerminalCheck Void() const;

    TerminalCheck Char() const;

    TerminalCheck AnyBoolean() const;

    TypeCheck Pointer() const;

    TerminalCheck Enum() const;

    TypeCheck LvalueReference() const;

    inline ContextCheck Class(char const * id) const;

    template<std::size_t N> inline ContextCheck Struct(char const (& id)[N])
        const;

    TypeCheck Typedef() const;

    inline ContextCheck Typedef(char const * id) const;

    TypeCheck NotSubstTemplateTypeParmType() const;

private:
    TypeCheck() = default;

    clang::QualType const type_{};
};

class DeclCheck {
public:
    explicit DeclCheck(clang::Decl const * decl): decl_(decl) {}

    explicit operator bool() const { return decl_ != nullptr; }

    inline ContextCheck Class(char const * id) const;

    template<std::size_t N> inline ContextCheck Struct(char const (& id)[N])
        const;

    template<std::size_t N> inline ContextCheck Union(char const (& id)[N])
        const;

    template<std::size_t N> inline ContextCheck Function(char const (& id)[N])
        const;

    ContextCheck Operator(clang::OverloadedOperatorKind op) const;

    template<std::size_t N> inline ContextCheck Var(char const (& id)[N])
        const;

    ContextCheck MemberFunction() const;

private:
    clang::Decl const * const decl_;
};

class ContextCheck {
public:
    explicit operator bool() const { return context_ != nullptr; }

    TerminalCheck GlobalNamespace() const;

    template<std::size_t N> inline ContextCheck Namespace(
        char const (& id)[N]) const;

    TerminalCheck StdNamespace() const;

    ContextCheck AnonymousNamespace() const;

    inline ContextCheck Class(char const * id) const;

    template<std::size_t N> inline ContextCheck Struct(char const (& id)[N])
        const;

private:
    friend DeclCheck;
    friend TypeCheck;
    friend ContextCheck detail::checkRecordDecl(
        clang::Decl const * decl, clang::TagTypeKind tag, char const * id);

    explicit ContextCheck(clang::DeclContext const * context = nullptr):
        context_(context) {}

    clang::DeclContext const * const context_;
};

class TerminalCheck {
public:
    explicit operator bool() const { return satisfied_; }

private:
    friend ContextCheck;
    friend TypeCheck;

    explicit TerminalCheck(bool satisfied): satisfied_(satisfied) {}

    bool const satisfied_;
};

namespace detail {

ContextCheck checkRecordDecl(
    clang::Decl const * decl, clang::TagTypeKind tag, char const * id)
{
    auto r = llvm::dyn_cast_or_null<clang::RecordDecl>(decl);
    if (r != nullptr && r->getTagKind() == tag) {
        auto const i = r->getIdentifier();
        if (i != nullptr && i->isStr(id)) {
            return ContextCheck(r->getDeclContext());
        }
    }
    return ContextCheck();
}

}

ContextCheck TypeCheck::Class(char const * id)
    const
{
    if (!type_.isNull()) {
        auto const t = type_->getAs<clang::RecordType>();
        if (t != nullptr) {
            return detail::checkRecordDecl(t->getDecl(), clang::TTK_Class, id);
        }
    }
    return ContextCheck();
}

template<std::size_t N> ContextCheck TypeCheck::Struct(char const (& id)[N])
    const
{
    if (!type_.isNull()) {
        auto const t = type_->getAs<clang::RecordType>();
        if (t != nullptr) {
            return detail::checkRecordDecl(t->getDecl(), clang::TTK_Struct, id);
        }
    }
    return ContextCheck();
}

ContextCheck TypeCheck::Typedef(char const * id)
    const
{
    if (!type_.isNull()) {
        if (auto const t = type_->getAs<clang::TypedefType>()) {
            auto const d = t->getDecl();
            auto const i = d->getIdentifier();
            assert(i != nullptr);
            if (i->isStr(id)) {
                return ContextCheck(d->getDeclContext());
            }
        }
    }
    return ContextCheck();
}

ContextCheck DeclCheck::Class(char const * id)
    const
{
    return detail::checkRecordDecl(decl_, clang::TTK_Class, id);
}

template<std::size_t N> ContextCheck DeclCheck::Struct(char const (& id)[N])
    const
{
    return detail::checkRecordDecl(decl_, clang::TTK_Struct, id);
}

template<std::size_t N> ContextCheck DeclCheck::Union(char const (& id)[N])
    const
{
    return detail::checkRecordDecl(decl_, clang::TTK_Union, id);
}

template<std::size_t N> ContextCheck DeclCheck::Function(char const (& id)[N])
    const
{
    auto f = llvm::dyn_cast_or_null<clang::FunctionDecl>(decl_);
    if (f != nullptr) {
        auto const i = f->getIdentifier();
        if (i != nullptr && i->isStr(id)) {
            return ContextCheck(f->getDeclContext());
        }
    }
    return ContextCheck();
}

template<std::size_t N> ContextCheck DeclCheck::Var(char const (& id)[N])
    const
{
    auto f = llvm::dyn_cast_or_null<clang::VarDecl>(decl_);
    if (f != nullptr) {
        auto const i = f->getIdentifier();
        if (i != nullptr && i->isStr(id)) {
            return ContextCheck(f->getDeclContext());
        }
    }
    return ContextCheck();
}

template<std::size_t N> ContextCheck ContextCheck::Namespace(
    char const (& id)[N]) const
{
    if (context_) {
        auto n = llvm::dyn_cast<clang::NamespaceDecl>(context_);
        if (n != nullptr) {
            auto const i = n->getIdentifier();
            if (i != nullptr && i->isStr(id)) {
                return ContextCheck(n->getParent());
            }
        }
    }
    return ContextCheck();
}

ContextCheck ContextCheck::Class(char const * id)
    const
{
    return detail::checkRecordDecl(
        llvm::dyn_cast_or_null<clang::Decl>(context_), clang::TTK_Class, id);
}

template<std::size_t N> ContextCheck ContextCheck::Struct(char const (& id)[N])
    const
{
    return detail::checkRecordDecl(
        llvm::dyn_cast_or_null<clang::Decl>(context_), clang::TTK_Struct, id);
}

bool isExtraWarnUnusedType(clang::QualType type);

bool isOkToRemoveArithmeticCast(
    clang::ASTContext & context, clang::QualType t1, clang::QualType t2,
    const clang::Expr* subExpr);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
