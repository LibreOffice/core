/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"

// Original idea from tml.
// Look for variables that are (a) initialised from zero or one constants. (b) only used in one spot.
// In which case, we might as well inline it.

namespace
{

bool startsWith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

class OnceVar:
    public RecursiveASTVisitor<OnceVar>, public loplugin::Plugin
{
public:
    explicit OnceVar(InstantiationData const & data): Plugin(data) {}

    virtual void run() override {
        // ignore some files with problematic macros
        std::string fn( compiler.getSourceManager().getFileEntryForID(
                        compiler.getSourceManager().getMainFileID())->getName() );
        normalizeDotDotInFilePath(fn);
        // platform-specific stuff
        if (fn == SRCDIR "/sal/osl/unx/thread.cxx"
            || fn == SRCDIR "/sot/source/base/formats.cxx"
            || fn == SRCDIR "/svl/source/config/languageoptions.cxx"
            || fn == SRCDIR "/sfx2/source/appl/appdde.cxx"
            || fn == SRCDIR "/configmgr/source/components.cxx"
            || fn == SRCDIR "/embeddedobj/source/msole/oleembed.cxx")
             return;
        // some of this is necessary
        if (startsWith( fn, SRCDIR "/sal/qa/"))
             return;
        if (startsWith( fn, SRCDIR "/comphelper/qa/"))
             return;
        // TODO need to check calls via function pointer
        if (fn == SRCDIR "/i18npool/source/textconversion/textconversion_zh.cxx"
            || fn == SRCDIR "/i18npool/source/localedata/localedata.cxx")
             return;
        // debugging stuff
        if (fn == SRCDIR "/sc/source/core/data/dpcache.cxx"
            || fn == SRCDIR "/sw/source/core/layout/dbg_lay.cxx"
            || fn == SRCDIR "/sw/source/core/layout/ftnfrm.cxx")
             return;
        // TODO taking local reference to variable
        if (fn == SRCDIR "/sc/source/filter/excel/xechart.cxx")
             return;
        // macros managing to generate to a valid warning
        if (fn == SRCDIR "/solenv/bin/concat-deps.c")
             return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        for (auto const & varDecl : maVarDeclSet)
        {
            if (maVarDeclToIgnoreSet.find(varDecl) != maVarDeclToIgnoreSet.end())
                continue;
            int noUses = 0;
            auto it = maVarUsesMap.find(varDecl);
            if (it != maVarUsesMap.end())
                noUses = it->second;
            if (noUses > 1)
                continue;
            report(DiagnosticsEngine::Warning,
                   "var used only once, should be inlined or declared const",
                   varDecl->getLocation())
                << varDecl->getSourceRange();
            if (it != maVarUsesMap.end())
                report(DiagnosticsEngine::Note,
                       "used here",
                       maVarUseSourceRangeMap[varDecl].getBegin())
                    << maVarUseSourceRangeMap[varDecl];
        }
    }

    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitVarDecl( const VarDecl* );

private:
    std::unordered_set<VarDecl const *> maVarDeclSet;
    std::unordered_set<VarDecl const *> maVarDeclToIgnoreSet;
    std::unordered_map<VarDecl const *, int> maVarUsesMap;
    std::unordered_map<VarDecl const *, SourceRange> maVarUseSourceRangeMap;
};

bool OnceVar::VisitVarDecl( const VarDecl* varDecl )
{
    if (ignoreLocation(varDecl)) {
        return true;
    }
    if (varDecl->isExceptionVariable() || isa<ParmVarDecl>(varDecl)) {
        return true;
    }
    // ignore stuff in header files (which should really not be there, but anyhow)
    if (!compiler.getSourceManager().isInMainFile(varDecl->getLocation())) {
        return true;
    }
    // Ignore macros like FD_ZERO
    if (compiler.getSourceManager().isMacroBodyExpansion(varDecl->getLocStart())) {
        return true;
    }
    if (varDecl->hasGlobalStorage()) {
        return true;
    }
    auto const tc = loplugin::TypeCheck(varDecl->getType());
    if (!varDecl->getType()->isScalarType()
        && !varDecl->getType()->isBooleanType()
        && !varDecl->getType()->isEnumeralType()
        && !tc.Class("OString").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace())
    {
        return true;
    }
    if (varDecl->getType()->isPointerType())
        return true;
    // if it's declared const, ignore it, it's there to make the code easier to read
    if (tc.Const())
        return true;

    if (!varDecl->hasInit())
        return true;

    // check for string or scalar literals
    bool foundStringLiteral = false;
    const Expr * initExpr = varDecl->getInit();
    if (auto e = dyn_cast<ExprWithCleanups>(initExpr)) {
        initExpr = e->getSubExpr();
    }
    if (auto stringLit = dyn_cast<clang::StringLiteral>(initExpr)) {
        foundStringLiteral = true;
        // ignore long literals, helps to make the code more legible
        if (stringLit->getLength() > 40) {
            return true;
        }
    } else if (auto constructExpr = dyn_cast<CXXConstructExpr>(initExpr)) {
        if (constructExpr->getNumArgs() > 0) {
            auto stringLit2 = dyn_cast<clang::StringLiteral>(constructExpr->getArg(0));
            foundStringLiteral = stringLit2 != nullptr;
            // ignore long literals, helps to make the code more legible
            if (stringLit2 && stringLit2->getLength() > 40) {
                return true;
            }
        }
    }
    if (!foundStringLiteral) {
        auto const init = varDecl->getInit();
#if CLANG_VERSION < 30900
        // Work around missing Clang 3.9 fix <https://reviews.llvm.org/rL271762>
        // "Sema: do not attempt to sizeof a dependent type" (while an
        // initializer expression of the form
        //
        //   sizeof (T)
        //
        // with dependent type T /is/ constant, keep consistent here with the
        // (arguably broken) behavior of isConstantInitializer returning false
        // in Clang >= 3.9):
        if (init->isValueDependent()) {
            return true;
        }
#endif
        if (!init->isConstantInitializer(compiler.getASTContext(), false/*ForRef*/))
        {
            return true;
        }
    }

    maVarDeclSet.insert(varDecl);

    return true;
}

bool OnceVar::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    if (ignoreLocation(declRefExpr)) {
        return true;
    }
    const Decl* decl = declRefExpr->getDecl();
    if (!isa<VarDecl>(decl) || isa<ParmVarDecl>(decl)) {
        return true;
    }
    const VarDecl * varDecl = dyn_cast<VarDecl>(decl)->getCanonicalDecl();
    // ignore stuff in header files (which should really not be there, but anyhow)
    if (!compiler.getSourceManager().isInMainFile(varDecl->getLocation())) {
        return true;
    }

    Stmt const * parent = parentStmt(declRefExpr);
    // ignore cases like:
    //     const OUString("xxx") xxx;
    //     rtl_something(xxx.pData);
    // and
    //      foo(&xxx);
    // where we cannot inline the declaration.
    auto const tc = loplugin::TypeCheck(varDecl->getType());
    if (tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        && parent && (isa<MemberExpr>(parent) || isa<UnaryOperator>(parent)))
    {
        maVarDeclToIgnoreSet.insert(varDecl);
        return true;
    }

    // if we take the address of it, or we modify it, ignore it
    if (auto unaryOp = dyn_cast_or_null<UnaryOperator>(parent)) {
        UnaryOperator::Opcode op = unaryOp->getOpcode();
        if (op == UO_AddrOf || op == UO_PreInc || op == UO_PostInc
            || op == UO_PreDec || op == UO_PostDec)
        {
            maVarDeclToIgnoreSet.insert(varDecl);
            return true;
        }
    }

    // if we assign it another value, or modify it, ignore it
    if (auto binaryOp = dyn_cast_or_null<BinaryOperator>(parent)) {
        if (binaryOp->getLHS() == declRefExpr)
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            if (op == BO_Assign || op == BO_PtrMemD || op == BO_PtrMemI || op == BO_MulAssign
                || op == BO_DivAssign || op == BO_RemAssign || op == BO_AddAssign
                || op == BO_SubAssign || op == BO_ShlAssign || op == BO_ShrAssign
                || op == BO_AndAssign || op == BO_XorAssign || op == BO_OrAssign)
            {
                maVarDeclToIgnoreSet.insert(varDecl);
                return true;
            }
        }
    }

    // ignore those ones we are passing by reference
    if (auto callExpr = dyn_cast_or_null<CallExpr>(parent)) {
        const FunctionDecl* calleeFunctionDecl = callExpr->getDirectCallee();
        if (calleeFunctionDecl) {
            for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
                if (callExpr->getArg(i) == declRefExpr) {
                    if (i < calleeFunctionDecl->getNumParams()) {
                        QualType qt { calleeFunctionDecl->getParamDecl(i)->getType() };
                        if (loplugin::TypeCheck(qt).LvalueReference()) {
                            maVarDeclToIgnoreSet.insert(varDecl);
                            return true;
                        }
                    }
                    break;
                }
            }
        }
    }
    // ignore those ones we are passing by reference
    if (auto cxxConstructExpr = dyn_cast_or_null<CXXConstructExpr>(parent)) {
        const CXXConstructorDecl* cxxConstructorDecl = cxxConstructExpr->getConstructor();
        for (unsigned i = 0; i < cxxConstructExpr->getNumArgs(); ++i) {
            if (cxxConstructExpr->getArg(i) == declRefExpr) {
                if (i < cxxConstructorDecl->getNumParams()) {
                    QualType qt { cxxConstructorDecl->getParamDecl(i)->getType() };
                    if (loplugin::TypeCheck(qt).LvalueReference()) {
                        maVarDeclToIgnoreSet.insert(varDecl);
                        return true;
                    }
                }
                break;
            }
        }
        return true;
    }

    if (maVarUsesMap.find(varDecl) == maVarUsesMap.end()) {
        maVarUsesMap[varDecl] = 1;
        maVarUseSourceRangeMap[varDecl] = declRefExpr->getSourceRange();
    } else {
        maVarUsesMap[varDecl]++;
    }

    return true;
}

loplugin::Plugin::Registration< OnceVar > X("oncevar", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
