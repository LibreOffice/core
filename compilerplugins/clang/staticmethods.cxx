/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "compat.hxx"

/*
  Look for member functions that can be static
*/
namespace {

class StaticMethods:
    public RecursiveASTVisitor<StaticMethods>, public loplugin::Plugin
{
private:
    bool bVisitedThis;
public:
    explicit StaticMethods(InstantiationData const & data): Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseCXXMethodDecl(const CXXMethodDecl * decl);

    bool VisitCXXThisExpr(const CXXThisExpr *) { bVisitedThis = true; return true; }
    // these two indicate that we hit something that makes our analysis unreliable
    bool VisitUnresolvedMemberExpr(const UnresolvedMemberExpr *) { bVisitedThis = true; return true; }
    bool VisitCXXDependentScopeMemberExpr(const CXXDependentScopeMemberExpr *) { bVisitedThis = true; return true; }
private:
    std::string getFilename(SourceLocation loc);
};

bool BaseCheckNotTestFixtureSubclass(const CXXRecordDecl *BaseDefinition, void *) {
    if (BaseDefinition->getQualifiedNameAsString().compare("CppUnit::TestFixture") == 0) {
        return false;
    }
    return true;
}

bool isDerivedFromTestFixture(const CXXRecordDecl *decl) {
    if (!decl->hasDefinition())
        return false;
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !decl->forallBases(BaseCheckNotTestFixtureSubclass, nullptr, true)) {
        return true;
    }
    return false;
}

std::string StaticMethods::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    return compiler.getSourceManager().getFilename(spellingLocation);
}


bool StaticMethods::TraverseCXXMethodDecl(const CXXMethodDecl * pCXXMethodDecl) {
    if (ignoreLocation(pCXXMethodDecl)) {
        return true;
    }
    if (!pCXXMethodDecl->isInstance() || pCXXMethodDecl->isVirtual() || !pCXXMethodDecl->hasBody()) {
        return true;
    }
    if (pCXXMethodDecl->getOverloadedOperator() != OverloadedOperatorKind::OO_None || pCXXMethodDecl->hasAttr<OverrideAttr>()) {
        return true;
    }
    if (isa<CXXConstructorDecl>(pCXXMethodDecl) || isa<CXXDestructorDecl>(pCXXMethodDecl) || isa<CXXConversionDecl>(pCXXMethodDecl)) {
        return true;
    }
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(pCXXMethodDecl->getCanonicalDecl()->getLocStart()))) {
        return true;
    }
    if ( pCXXMethodDecl != pCXXMethodDecl->getCanonicalDecl() ) {
        return true;
    }

    // leave these alone for now, it is possible to fix them, but I don't understand how
    SourceLocation canonicalLoc = pCXXMethodDecl->getCanonicalDecl()->getLocStart();
    if (isMacroBodyExpansion(compiler, canonicalLoc) ) {
        StringRef name { Lexer::getImmediateMacroName(
                canonicalLoc, compiler.getSourceManager(), compiler.getLangOpts()) };
        if (name == "DECL_LINK") {
            return true;
        }
    }
    // the CppUnit stuff uses macros and methods that can't be changed
    if (isDerivedFromTestFixture(pCXXMethodDecl->getParent())) {
        return true;
    }
    // don't mess with the backwards compatibility stuff
    if (getFilename(pCXXMethodDecl->getLocStart()) == SRCDIR "/cppuhelper/source/compat.cxx") {
        return true;
    }
    // the DDE has a dummy implementation on Linux and a real one on Windows
    if (getFilename(pCXXMethodDecl->getCanonicalDecl()->getLocStart()) == SRCDIR "/include/svl/svdde.hxx") {
        return true;
    }
    std::string aParentName = pCXXMethodDecl->getParent()->getQualifiedNameAsString();
    // special case having something to do with static initialisation
    // sal/osl/all/utility.cxx
    if (aParentName == "osl::OGlobalTimer") {
        return true;
    }
    // can't change it because in debug mode it can't be static
    // sal/cpprt/operators_new_delete.cxx
    if (aParentName == "(anonymous namespace)::AllocatorTraits") {
        return true;
    }
    // in this case, the code is taking the address of the member function
    // shell/source/unix/sysshell/recently_used_file_handler.cxx
    if (aParentName == "(anonymous namespace)::recently_used_item") {
        return true;
    }

    bVisitedThis = false;
    TraverseStmt(pCXXMethodDecl->getBody());
    if (bVisitedThis) {
        return true;
    }

    report(
        DiagnosticsEngine::Warning,
        "this method can be declared static " + aParentName,
        pCXXMethodDecl->getCanonicalDecl()->getLocStart())
      << pCXXMethodDecl->getCanonicalDecl()->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<StaticMethods> X("staticmethods", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
