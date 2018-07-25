/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include "plugin.hxx"

namespace {

bool isStatic(ValueDecl const * decl, bool * memberEnumerator) {
    assert(memberEnumerator != nullptr);
    // clang::MemberExpr::getMemberDecl is documented to return either a
    // FieldDecl or a CXXMethodDecl, but can apparently also return a VarDecl
    // (as C++ static data members are modeled by VarDecl, not FieldDecl) or an
    // EnumConstantDecl (struct { enum {E}; } s; s.E;), see
    // <https://reviews.llvm.org/D23907> "Fix documentation of
    // MemberExpr::getMemberDecl":
    auto fd = dyn_cast<FieldDecl>(decl);
    if (fd != nullptr) {
        *memberEnumerator = false;
        return false;
    }
    auto vd = dyn_cast<VarDecl>(decl);
    if (vd != nullptr) {
        *memberEnumerator = false;
        assert(vd->isStaticDataMember());
        return true;
    }
    auto md = dyn_cast<CXXMethodDecl>(decl);
    if (md != nullptr) {
        *memberEnumerator = false;
        return md->isStatic();
    }
    assert(dyn_cast<EnumConstantDecl>(decl) != nullptr);
    *memberEnumerator = true;
    return true;
}

class StaticAccess:
    public RecursiveASTVisitor<StaticAccess>, public loplugin::Plugin
{
public:
    explicit StaticAccess(loplugin::InstantiationData const & data):
        Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitMemberExpr(MemberExpr const * expr);
};

bool StaticAccess::VisitMemberExpr(MemberExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto decl = expr->getMemberDecl();
    bool me;
    if (!isStatic(decl, &me)) {
        return true;
    }
    auto const loc = expr->getExprLoc();
    if (compiler.getSourceManager().isMacroBodyExpansion(loc)) {
        auto const name = Lexer::getImmediateMacroName(
            loc, compiler.getSourceManager(), compiler.getLangOpts());
        if (name == "BEGIN_COM_MAP" || name == "DEFAULT_REFLECTION_HANDLER") {
            // .../VC/Tools/MSVC/14.14.26428/atlmfc/include\atlcom.h(2226,10):  note: expanded from
            //   macro 'BEGIN_COM_MAP'
            //     return this->InternalQueryInterface(this, _GetEntries(), iid, ppvObject);
            //            ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // .../VC/Tools/MSVC/14.14.26428/atlmfc/include\atlwin.h(2890,5):  note: expanded from
            //   macro 'DEFAULT_REFLECTION_HANDLER'
            //     if(this->DefaultReflectionHandler(hWnd, uMsg, wParam, lParam, lResult))
            //        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            return true;
        }
    }
    report(
        DiagnosticsEngine::Warning,
        ("accessing %select{static class member|member enumerator}0 through"
         " class member access syntax, use a qualified name like '%1' instead"),
        expr->getLocStart())
        << me << decl->getQualifiedNameAsString() << expr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<StaticAccess> X("staticaccess");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
