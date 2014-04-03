/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <set>
#include <string>

#include "clang/AST/Attr.h"

#include "compat.hxx"
#include "plugin.hxx"

namespace {

bool isSalBool(QualType type) {
    TypedefType const * t = type->getAs<TypedefType>();
    return t != nullptr && t->getDecl()->getNameAsString() == "sal_Bool";
}

// It appears that, given a function declaration, there is no way to determine
// the language linkage of the function's type, only of the function's name
// (via FunctionDecl::isExternC); however, in a case like
//
//   extern "C" { static void f(); }
//
// the function's name does not have C language linkage while the function's
// type does (as clarified in C++11 [decl.link]); cf. <http://clang-developers.
// 42468.n3.nabble.com/Language-linkage-of-function-type-tt4037248.html>
// "Language linkage of function type":
bool hasCLanguageLinkageType(FunctionDecl const * decl) {
    assert(decl != nullptr);
    if (decl->isExternC()) {
        return true;
    }
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    if (decl->isInExternCContext()) {
        return true;
    }
#else
    if (decl->getCanonicalDecl()->getDeclContext()->isExternCContext()) {
        return true;
    }
#endif
    return false;
}

bool canOverrideTemplateArgumentMember(CXXMethodDecl const * decl) {
    CXXRecordDecl const * r = dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    assert(r != nullptr);
    for (auto b = r->bases_begin(); b != r->bases_end(); ++b) {
        if (b->getType()->isTemplateTypeParmType()) {
            return true;
        }
    }
    return false;
}

//TODO: current implementation is not at all general, just tests what we
// encounter in practice:
bool hasBoolOverload(FunctionDecl const * decl) {
    if (isa<CXXMethodDecl>(decl)) {
        unsigned n = decl->getNumParams();
        CXXRecordDecl const * r = dyn_cast<CXXRecordDecl>(
            decl->getDeclContext());
        assert(r != nullptr);
        for (auto m = r->method_begin(); m != r->method_end(); ++m) {
            if (m->getDeclName() == decl->getDeclName()
                && m->getNumParams() == n)
            {
                bool hasSB = false;
                for (unsigned i = 0; i != n; ++i) {
                    QualType t1 { decl->getParamDecl(i)->getType() };
                    bool isSB = isSalBool(t1);
                    bool isSBRef = !isSB && t1->isReferenceType()
                        && isSalBool(t1.getNonReferenceType());
                    QualType t2 { m->getParamDecl(i)->getType() };
                    if (!(isSB
                          ? t2->isBooleanType()
                          : isSBRef
                          ? (t2->isReferenceType()
                             && t2.getNonReferenceType()->isBooleanType())
                          : t2 == t1))
                    {
                        goto next;
                    }
                    hasSB |= isSB || isSBRef;
                }
                return hasSB;
                    // cheaply protect against the case where decl would have no
                    // sal_Bool parameters at all and would match itself
            next:;
            }
        }
    }
    return false;
}

class SalBool:
    public RecursiveASTVisitor<SalBool>, public loplugin::RewritePlugin
{
public:
    explicit SalBool(InstantiationData const & data): RewritePlugin(data) {}

    virtual void run() override;

    bool VisitUnaryAddrOf(UnaryOperator const * op);

    bool VisitCStyleCastExpr(CStyleCastExpr * expr);

    bool VisitCXXStaticCastExpr(CXXStaticCastExpr * expr);

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr * expr);

    bool WalkUpFromParmVarDecl(ParmVarDecl const * decl);
    bool VisitParmVarDecl(ParmVarDecl const * decl);

    bool WalkUpFromVarDecl(VarDecl const * decl);
    bool VisitVarDecl(VarDecl const * decl);

    bool WalkUpFromFieldDecl(FieldDecl const * decl);
    bool VisitFieldDecl(FieldDecl const * decl);

    bool WalkUpFromFunctionDecl(FunctionDecl const * decl);
    bool VisitFunctionDecl(FunctionDecl const * decl);

    bool VisitValueDecl(ValueDecl const * decl);

private:
    bool isInUnoIncludeFile(SourceLocation spellingLocation) const;

    bool isInSpecialMainFile(SourceLocation spellingLocation) const;

    bool isInMainFile(SourceLocation spellingLocation) const;

    bool isMacroBodyExpansion(SourceLocation location) const;

    bool rewrite(SourceLocation location);

    std::set<VarDecl const *> varDecls_;
};

void SalBool::run() {
    if (compiler.getLangOpts().CPlusPlus) {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        for (auto decl: varDecls_) {
            SourceLocation loc { decl->getLocStart() };
            TypeSourceInfo * tsi = decl->getTypeSourceInfo();
            if (tsi != nullptr) {
                SourceLocation l {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getBeginLoc()) };
                SourceLocation end {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getEndLoc()) };
                assert(l.isFileID() && end.isFileID());
                if (l == end
                    || compiler.getSourceManager().isBeforeInTranslationUnit(
                        l, end))
                {
                    for (;;) {
                        unsigned n = Lexer::MeasureTokenLength(
                            l, compiler.getSourceManager(),
                            compiler.getLangOpts());
                        std::string s {
                            compiler.getSourceManager().getCharacterData(l),
                            n };
                        if (s == "sal_Bool") {
                            loc = l;
                            break;
                        }
                        if (l == end) {
                            break;
                        }
                        l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                    }
                }
            }
            if (!rewrite(loc)) {
                report(
                    DiagnosticsEngine::Warning,
                    "VarDecl, use \"bool\" instead of \"sal_Bool\"", loc)
                    << decl->getSourceRange();
            }
        }
    }
}

bool SalBool::VisitUnaryAddrOf(UnaryOperator const * op) {
    Expr const * e1 = op->getSubExpr()->IgnoreParenCasts();
    if (isSalBool(e1->getType())) {
        DeclRefExpr const * e2 = dyn_cast<DeclRefExpr>(e1);
        if (e2 != nullptr) {
            VarDecl const * d = dyn_cast<VarDecl>(e2->getDecl());
            if (d != nullptr) {
                varDecls_.erase(d);
            }
        }
    }
    return true;
}

bool SalBool::VisitCStyleCastExpr(CStyleCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isSalBool(expr->getType())) {
        SourceLocation loc { expr->getLocStart() };
        while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        if (isMacroBodyExpansion(loc)) {
            StringRef name { Lexer::getImmediateMacroName(
                loc, compiler.getSourceManager(), compiler.getLangOpts()) };
            if (name == "sal_False" || name == "sal_True") {
                return true;
            }
        }
        report(
            DiagnosticsEngine::Warning,
            "CStyleCastExpr, suspicious cast from %0 to %1",
            expr->getLocStart())
            << expr->getSubExpr()->IgnoreParenImpCasts()->getType()
            << expr->getType() << expr->getSourceRange();
    }
    return true;
}

bool SalBool::VisitCXXStaticCastExpr(CXXStaticCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isSalBool(expr->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "CStyleCastExpr, suspicious cast from %0 to %1",
            expr->getLocStart())
            << expr->getSubExpr()->IgnoreParenImpCasts()->getType()
            << expr->getType() << expr->getSourceRange();
    }
    return true;
}

bool SalBool::VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isSalBool(expr->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "CStyleCastExpr, suspicious cast from %0 to %1",
            expr->getLocStart())
            << expr->getSubExpr()->IgnoreParenImpCasts()->getType()
            << expr->getType() << expr->getSourceRange();
    }
    return true;
}

bool SalBool::WalkUpFromParmVarDecl(ParmVarDecl const * decl) {
    return VisitParmVarDecl(decl);
}

bool SalBool::VisitParmVarDecl(ParmVarDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    if (isSalBool(decl->getType().getNonReferenceType())) {
        FunctionDecl const * f = dyn_cast<FunctionDecl>(decl->getDeclContext());
        if (f != nullptr) { // e.g.: typedef sal_Bool (* FuncPtr )( sal_Bool );
            f = f->getCanonicalDecl();
            if (!(hasCLanguageLinkageType(f)
                  || (isInUnoIncludeFile(
                          compiler.getSourceManager().getSpellingLoc(
                              f->getNameInfo().getLoc()))
                      && (!f->isInlined() || f->hasAttr<DeprecatedAttr>()
                          || decl->getType()->isReferenceType()
                          || hasBoolOverload(f)))
                  || f->isDeleted()))
            {
                CXXMethodDecl const * m = dyn_cast<CXXMethodDecl>(f);
                //XXX: canOverrideTemplateArgumentMember(m) does not require
                // !m->isVirtual()
                if ((m == nullptr || !m->isVirtual()))
                {
                    SourceLocation loc { decl->getLocStart() };
                    TypeSourceInfo * tsi = decl->getTypeSourceInfo();
                    if (tsi != nullptr) {
                        SourceLocation l {
                            compiler.getSourceManager().getExpansionLoc(
                                tsi->getTypeLoc().getBeginLoc()) };
                        SourceLocation end {
                            compiler.getSourceManager().getExpansionLoc(
                                tsi->getTypeLoc().getEndLoc()) };
                        assert(l.isFileID() && end.isFileID());
                        if (l == end
                            || (compiler.getSourceManager()
                                .isBeforeInTranslationUnit(l, end)))
                        {
                            for (;;) {
                                unsigned n = Lexer::MeasureTokenLength(
                                    l, compiler.getSourceManager(),
                                    compiler.getLangOpts());
                                std::string s {
                                    compiler.getSourceManager().getCharacterData(l),
                                        n };
                                if (s == "sal_Bool") {
                                    loc = l;
                                    break;
                                }
                                if (l == end) {
                                    break;
                                }
                                l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                            }
                        }
                    }
                    // Only rewrite declarations in include files if a
                    // definition is also seen, to avoid compilation of a
                    // definition (in a main file only processed later) to fail
                    // with a "mismatch" error before the rewriter had a chance
                    // to act upon the definition (but use the heuristic of
                    // assuming pure virtual functions do not have definitions):
                    if (!((isInMainFile(
                               compiler.getSourceManager().getSpellingLoc(
                                   dyn_cast<FunctionDecl>(
                                       decl->getDeclContext())
                                   ->getNameInfo().getLoc()))
                           || f->isDefined() || f->isPure())
                          && rewrite(loc)))
                    {
                        report(
                            DiagnosticsEngine::Warning,
                            "ParmVarDecl, use \"bool\" instead of \"sal_Bool\"",
                            loc)
                            << decl->getSourceRange();
                    }
                }
            }
        }
    }
    return true;
}

bool SalBool::WalkUpFromVarDecl(VarDecl const * decl) {
    return VisitVarDecl(decl);
}

bool SalBool::VisitVarDecl(VarDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    if (!decl->isExternC() && isSalBool(decl->getType())
        && !isInSpecialMainFile(
            compiler.getSourceManager().getSpellingLoc(decl->getLocStart())))
    {
        varDecls_.insert(decl);
    }
    return true;
}

bool SalBool::WalkUpFromFieldDecl(FieldDecl const * decl) {
    return VisitFieldDecl(decl);
}

bool SalBool::VisitFieldDecl(FieldDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    if (isSalBool(decl->getType())) {
        TagDecl const * td = dyn_cast<TagDecl>(decl->getDeclContext());
        assert(td != nullptr);
        if (!(((td->isStruct() || td->isUnion())
               && compat::isExternCContext(*td))
              || isInUnoIncludeFile(
                  compiler.getSourceManager().getSpellingLoc(
                      decl->getLocation()))))
        {
            SourceLocation loc { decl->getLocStart() };
            TypeSourceInfo * tsi = decl->getTypeSourceInfo();
            if (tsi != nullptr) {
                SourceLocation l {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getBeginLoc()) };
                SourceLocation end {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getEndLoc()) };
                assert(l.isFileID() && end.isFileID());
                if (l == end
                    || compiler.getSourceManager().isBeforeInTranslationUnit(
                        l, end))
                {
                    for (;;) {
                        unsigned n = Lexer::MeasureTokenLength(
                            l, compiler.getSourceManager(),
                            compiler.getLangOpts());
                        std::string s {
                            compiler.getSourceManager().getCharacterData(l),
                                n };
                        if (s == "sal_Bool") {
                            loc = l;
                            break;
                        }
                        if (l == end) {
                            break;
                        }
                        l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                    }
                }
            }
            if (!rewrite(loc)) {
                report(
                    DiagnosticsEngine::Warning,
                    "FieldDecl, use \"bool\" instead of \"sal_Bool\"", loc)
                    << decl->getSourceRange();
            }
        }
    }
    return true;
}

bool SalBool::WalkUpFromFunctionDecl(FunctionDecl const * decl) {
    return VisitFunctionDecl(decl);
}

bool SalBool::VisitFunctionDecl(FunctionDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    if (isSalBool(compat::getReturnType(*decl).getNonReferenceType())) {
        FunctionDecl const * f = decl->getCanonicalDecl();
        CXXMethodDecl const * m = dyn_cast<CXXMethodDecl>(f);
        //XXX: canOverrideTemplateArgumentMember(m) does not require
        // !m->isVirtual()
        if ((m == nullptr || !m->isVirtual()
             || (m->size_overridden_methods() == 0
                 && !canOverrideTemplateArgumentMember(m)))
            && !(hasCLanguageLinkageType(f)
                 || (isInUnoIncludeFile(
                         compiler.getSourceManager().getSpellingLoc(
                             f->getNameInfo().getLoc()))
                     && (!f->isInlined() || f->hasAttr<DeprecatedAttr>()))))
        {
            SourceLocation loc { decl->getLocStart() };
            SourceLocation l { compiler.getSourceManager().getExpansionLoc(
                loc) };
            SourceLocation end { compiler.getSourceManager().getExpansionLoc(
                decl->getNameInfo().getLoc()) };
            assert(l.isFileID() && end.isFileID());
            if (compiler.getSourceManager().isBeforeInTranslationUnit(l, end)) {
                while (l != end) {
                    unsigned n = Lexer::MeasureTokenLength(
                        l, compiler.getSourceManager(), compiler.getLangOpts());
                    std::string s {
                        compiler.getSourceManager().getCharacterData(l), n };
                    if (s == "sal_Bool") {
                        loc = l;
                        break;
                    }
                    l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                }
            }
            // Only rewrite declarations in include files if a definition is
            // also seen, to avoid compilation of a definition (in a main file
            // only processed later) to fail with a "mismatch" error before the
            // rewriter had a chance to act upon the definition (but use the
            // heuristic of assuming pure virtual functions do not have
            // definitions):
            if (!((isInMainFile(
                       compiler.getSourceManager().getSpellingLoc(
                           decl->getNameInfo().getLoc()))
                   || f->isDefined() || f->isPure())
                  && rewrite(loc)))
            {
                report(
                    DiagnosticsEngine::Warning,
                    "use \"bool\" instead of \"sal_Bool\" as return type", loc)
                    << decl->getSourceRange();
            }
        }
    }
    return true;
}

bool SalBool::VisitValueDecl(ValueDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    if (isSalBool(decl->getType()) && !rewrite(decl->getLocStart())) {
        report(
            DiagnosticsEngine::Warning,
            "ValueDecl, use \"bool\" instead of \"sal_Bool\"",
            decl->getLocStart())
            << decl->getSourceRange();
    }
    return true;
}

bool SalBool::isInUnoIncludeFile(SourceLocation spellingLocation) const {
    StringRef name {
        compiler.getSourceManager().getFilename(spellingLocation) };
    return isInMainFile(spellingLocation)
        ? (name == SRCDIR "/cppu/source/cppu/compat.cxx"
           || name == SRCDIR "/cppuhelper/source/compat.cxx"
           || name == SRCDIR "/sal/osl/all/compat.cxx")
        : (name.startswith(SRCDIR "/include/com/")
           || name.startswith(SRCDIR "/include/cppu/")
           || name.startswith(SRCDIR "/include/cppuhelper/")
           || name.startswith(SRCDIR "/include/osl/")
           || name.startswith(SRCDIR "/include/rtl/")
           || name.startswith(SRCDIR "/include/sal/")
           || name.startswith(SRCDIR "/include/salhelper/")
           || name.startswith(SRCDIR "/include/systools/")
           || name.startswith(SRCDIR "/include/typelib/")
           || name.startswith(SRCDIR "/include/uno/"));
}

bool SalBool::isInSpecialMainFile(SourceLocation spellingLocation) const {
    return isInMainFile(spellingLocation)
        && (compiler.getSourceManager().getFilename(spellingLocation)
            == SRCDIR "/cppu/qa/test_any.cxx");
}

bool SalBool::isInMainFile(SourceLocation spellingLocation) const {
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
    return compiler.getSourceManager().isInMainFile(spellingLocation);
#else
    return compiler.getSourceManager().isFromMainFile(spellingLocation);
#endif
}

bool SalBool::isMacroBodyExpansion(SourceLocation location) const {
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    return compiler.getSourceManager().isMacroBodyExpansion(location);
#else
    return location.isMacroID()
        && !compiler.getSourceManager().isMacroArgExpansion(location);
#endif
}

bool SalBool::rewrite(SourceLocation location) {
    if (rewriter != nullptr) {
        //TODO: "sal_Bool" -> "bool", not "::bool"
        SourceLocation loc { compiler.getSourceManager().getExpansionLoc(
                location) };
        unsigned n = Lexer::MeasureTokenLength(
            loc, compiler.getSourceManager(), compiler.getLangOpts());
        if (std::string(compiler.getSourceManager().getCharacterData(loc), n)
            == "sal_Bool")
        {
            return replaceText(loc, n, "bool");
        }
    }
    return false;
}

loplugin::Plugin::Registration<SalBool> X("salbool", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
