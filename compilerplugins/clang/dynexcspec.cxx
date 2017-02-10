/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <functional>

#include "clang/AST/Comment.h"

#include "plugin.hxx"

// Remove dynamic exception specifications.  See the mail thread starting at
// <https://lists.freedesktop.org/archives/libreoffice/2017-January/076665.html>
// "Dynamic Exception Specifications" for details.

namespace {

bool isOverriding(FunctionDecl const * decl) {
    if (decl->hasAttr<OverrideAttr>()) {
        return true;
    }
    auto m = dyn_cast<CXXMethodDecl>(decl);
    return m != nullptr
        && m->begin_overridden_methods() != m->end_overridden_methods();
}

bool isDtorOrDealloc(FunctionDecl const * decl) {
    if (isa<CXXDestructorDecl>(decl)) {
        return true;
    }
    switch (decl->getOverloadedOperator()) {
    case OO_Delete:
    case OO_Array_Delete:
        return true;
    default:
        return false;
    }
}

class DynExcSpec:
    public RecursiveASTVisitor<DynExcSpec>, public loplugin::RewritePlugin
{
public:
    explicit DynExcSpec(InstantiationData const & data): RewritePlugin(data) {}

    void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitFunctionDecl(FunctionDecl const * decl) {
        if (ignoreLocation(decl)) {
            return true;
        }
        auto proto = decl->getType()->getAs<FunctionProtoType>();
        if (proto == nullptr || proto->getExceptionSpecType() != EST_Dynamic) {
            return true;
        }
        if (decl->isCanonicalDecl() && !isOverriding(decl)
            && !anyRedeclHasThrowsDocumentation(decl))
        {
            report(
                DiagnosticsEngine::Warning,
                ("function declaration has dynamic exception specification but"
                 " no corresponding documentation comment"),
                decl->getLocation())
                << decl->getSourceRange();
        }
        if (rewriter != nullptr) {
            if (!(decl->isDefined() || decl->isPure())) {
                return true;
            }
            if (auto m = dyn_cast<CXXMethodDecl>(decl)) {
                for (auto i = m->begin_overridden_methods();
                     i != m->end_overridden_methods(); ++i)
                {
                    auto proto2 = (*i)->getType()->getAs<FunctionProtoType>();
                    assert(proto2 != nullptr);
                    if (proto2->getExceptionSpecType() == EST_Dynamic) {
                        return true;
                    }
                }
            }
        }
        bool dtorOrDealloc = isDtorOrDealloc(decl);
        SourceRange source;
#if CLANG_VERSION >= 40000
        source = decl->getExceptionSpecSourceRange();
#endif
        if (rewriter != nullptr && source.isValid()) {
            if (dtorOrDealloc) {
                if (replaceText(source, "noexcept(false)")) {
                    return true;
                }
            } else {
                auto beg = source.getBegin();
                if (beg.isFileID()) {
                    for (;;) {
                        auto prev = Lexer::GetBeginningOfToken(
                            beg.getLocWithOffset(-1),
                            compiler.getSourceManager(),
                            compiler.getLangOpts());
                        auto n = Lexer::MeasureTokenLength(
                            prev, compiler.getSourceManager(),
                            compiler.getLangOpts());
                        auto s = StringRef(
                            compiler.getSourceManager().getCharacterData(prev),
                            n);
                        while (s.startswith("\\\n")) {
                            s = s.drop_front(2);
                            while (!s.empty()
                                   && (s.front() == ' ' || s.front() == '\t'
                                       || s.front() == '\n' || s.front() == '\v'
                                       || s.front() == '\f'))
                            {
                                s = s.drop_front(1);
                            }
                        }
                        if (!s.empty() && s != "\\") {
                            if (s.startswith("//")) {
                                beg = source.getBegin();
                            }
                            break;
                        }
                        beg = prev;
                    }
                }
                if (removeText(SourceRange(beg, source.getEnd()))) {
                    return true;
                }
            }
        }
        report(
            DiagnosticsEngine::Warning,
            (dtorOrDealloc
             ? "replace dynamic exception specification with 'noexcept(false)'"
             : "remove dynamic exception specification"),
            source.isValid() ? source.getBegin() : decl->getLocation())
            << (source.isValid() ? source : decl->getSourceRange());
        return true;
    }

private:
    bool hasThrowsDocumentation(FunctionDecl const * decl) {
        if (auto cmt = compiler.getASTContext().getCommentForDecl(
            decl, &compiler.getPreprocessor()))
        {
            for (auto i = cmt->child_begin(); i != cmt->child_end(); ++i) {
                if (auto bcc = dyn_cast<comments::BlockCommandComment>(*i)) {
                    if (compiler.getASTContext().getCommentCommandTraits()
                        .getCommandInfo(bcc->getCommandID())->IsThrowsCommand)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool anyRedeclHasThrowsDocumentation(FunctionDecl const * decl) {
        return std::any_of(
            decl->redecls_begin(), decl->redecls_end(),
            [this](FunctionDecl * d) { return hasThrowsDocumentation(d); });
            // std::bind(
            //     &DynExcSpec::hasThrowsDocumentation, this,
            //     std::placeholders::_1));
    }
};

loplugin::Plugin::Registration<DynExcSpec> X("dynexcspec", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
