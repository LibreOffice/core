/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPILERPLUGINS_CLANG_TYPECHECK_HXX
#define INCLUDED_COMPILERPLUGINS_CLANG_TYPECHECK_HXX

#include <cstddef>

#include <clang/AST/DeclBase.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>

namespace loplugin {

class NamespaceCheck;
class TerminalCheck;

class TypeCheck {
public:
    explicit TypeCheck(clang::QualType type): type_(type) {}

    explicit operator bool() const { return !type_.isNull(); }

    TypeCheck Const() const;

    TerminalCheck Char() const;

    TypeCheck Pointer() const;

    TypeCheck LvalueReference() const;

    template<std::size_t N> inline NamespaceCheck Class(char const (& id)[N])
        const;

    TypeCheck NotSubstTemplateTypeParmType() const;

private:
    TypeCheck() = default;

    clang::QualType const type_;
};

class NamespaceCheck {
public:
    explicit operator bool() const { return context_ != nullptr; }

    TerminalCheck GlobalNamespace() const;

    template<std::size_t N> inline NamespaceCheck Namespace(
        char const (& id)[N]) const;

    TerminalCheck StdNamespace() const;

private:
    friend class TypeCheck;

    explicit NamespaceCheck(clang::DeclContext const * context = nullptr):
        context_(context) {}

    clang::DeclContext const * const context_;
};

class TerminalCheck {
public:
    explicit operator bool() const { return satisfied_; }

private:
    friend TypeCheck;
    friend NamespaceCheck;

    explicit TerminalCheck(bool satisfied): satisfied_(satisfied) {}

    bool const satisfied_;
};

template<std::size_t N> NamespaceCheck TypeCheck::Class(char const (& id)[N])
    const
{
    if (!type_.isNull()) {
        auto const t = type_->getAs<clang::RecordType>();
        if (t != nullptr) {
            auto const d = t->getDecl();
            if (d->isClass()) {
                auto const i = d->getIdentifier();
                if (i != nullptr && i->isStr(id)) {
                    return NamespaceCheck(d->getDeclContext());
                }
            }
        }
    }
    return NamespaceCheck();
}

template<std::size_t N> NamespaceCheck NamespaceCheck::Namespace(
    char const (& id)[N]) const
{
    if (context_) {
        auto n = llvm::dyn_cast<clang::NamespaceDecl>(context_);
        if (n != nullptr) {
            auto const i = n->getIdentifier();
            if (i != nullptr && i->isStr(id)) {
                return NamespaceCheck(n->getParent());
            }
        }
    }
    return NamespaceCheck();
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
