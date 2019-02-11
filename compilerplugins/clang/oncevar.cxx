/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

// Original idea from tml.
// Look for variables that are (a) initialised from zero or one constants. (b) only used in one spot.
// In which case, we might as well inline it.

namespace
{

Expr const * lookThroughInitListExpr(Expr const * expr) {
    if (auto const ile = dyn_cast<InitListExpr>(expr->IgnoreParenImpCasts())) {
        if (ile->getNumInits() == 1) {
            return ile->getInit(0);
        }
    }
    return expr;
}

class ConstantValueDependentExpressionVisitor:
    public ConstStmtVisitor<ConstantValueDependentExpressionVisitor, bool>
{
    ASTContext const & context_;

public:
    ConstantValueDependentExpressionVisitor(ASTContext const & context):
        context_(context) {}

    bool Visit(Stmt const * stmt) {
        assert(isa<Expr>(stmt));
        auto const expr = cast<Expr>(stmt);
        if (!expr->isValueDependent()) {
            return expr->isEvaluatable(context_);
        }
        return ConstStmtVisitor::Visit(stmt);
    }

    bool VisitParenExpr(ParenExpr const * expr)
    { return Visit(expr->getSubExpr()); }

    bool VisitCastExpr(CastExpr const * expr) {
        return Visit(expr->getSubExpr());
    }

    bool VisitUnaryOperator(UnaryOperator const * expr)
    { return Visit(expr->getSubExpr()); }

    bool VisitBinaryOperator(BinaryOperator const * expr) {
        return Visit(expr->getLHS()) && Visit(expr->getRHS());
    }

    bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr const *) {
        return true;
    }
};

class OnceVar:
    public loplugin::FilteringPlugin<OnceVar>
{
public:
    explicit OnceVar(loplugin::InstantiationData const & data): FilteringPlugin(data) {}

    virtual void run() override {
        // ignore some files with problematic macros
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // platform-specific stuff
        if (fn == SRCDIR "/sal/osl/unx/thread.cxx"
            || fn == SRCDIR "/sot/source/base/formats.cxx"
            || fn == SRCDIR "/svl/source/config/languageoptions.cxx"
            || fn == SRCDIR "/sfx2/source/appl/appdde.cxx"
            || fn == SRCDIR "/configmgr/source/components.cxx"
            || fn == SRCDIR "/embeddedobj/source/msole/oleembed.cxx")
             return;
        // some of this is necessary
        if (loplugin::hasPathnamePrefix( fn, SRCDIR "/sal/qa/"))
             return;
        if (loplugin::hasPathnamePrefix( fn, SRCDIR "/comphelper/qa/"))
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
        // TODO bug in the plugin
        if (fn == SRCDIR "/vcl/unx/generic/app/saldisp.cxx")
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

    bool VisitMemberExpr(MemberExpr const * expr) {
        // ignore cases like:
        //     const OUString("xxx") xxx;
        //     rtl_something(xxx.pData);
        // where we cannot inline the declaration.
        if (isa<FieldDecl>(expr->getMemberDecl())) {
            recordIgnore(expr);
        }
        return true;
    }

    bool VisitUnaryOperator(UnaryOperator const * expr) {
        // if we take the address of it, or we modify it, ignore it
        UnaryOperator::Opcode op = expr->getOpcode();
        if (op == UO_AddrOf || op == UO_PreInc || op == UO_PostInc
            || op == UO_PreDec || op == UO_PostDec)
        {
            recordIgnore(expr->getSubExpr());
        }
        return true;
    }

    bool VisitBinaryOperator(BinaryOperator const * expr) {
        // if we assign it another value, or modify it, ignore it
        BinaryOperator::Opcode op = expr->getOpcode();
        if (op == BO_Assign || op == BO_PtrMemD || op == BO_PtrMemI || op == BO_MulAssign
            || op == BO_DivAssign || op == BO_RemAssign || op == BO_AddAssign
            || op == BO_SubAssign || op == BO_ShlAssign || op == BO_ShrAssign
            || op == BO_AndAssign || op == BO_XorAssign || op == BO_OrAssign)
        {
            recordIgnore(expr->getLHS());
        }
        return true;
    }

    bool VisitCallExpr(CallExpr const * expr) {
        unsigned firstArg = 0;
        if (auto const cmce = dyn_cast<CXXMemberCallExpr>(expr)) {
            if (auto const e1 = cmce->getMethodDecl()) {
                if (!(e1->isConst() || e1->isStatic())) {
                    recordIgnore(cmce->getImplicitObjectArgument());
                }
            } else if (auto const e2 = dyn_cast<BinaryOperator>(
                           cmce->getCallee()->IgnoreParenImpCasts()))
            {
                switch (e2->getOpcode()) {
                case BO_PtrMemD:
                case BO_PtrMemI:
                    if (!e2->getRHS()->getType()->getAs<MemberPointerType>()
                        ->getPointeeType()->getAs<FunctionProtoType>()
                        ->isConst())
                    {
                        recordIgnore(e2->getLHS());
                    }
                    break;
                default:
                    break;
                }
            }
        } else if (auto const coce = dyn_cast<CXXOperatorCallExpr>(expr)) {
            if (auto const cmd = dyn_cast_or_null<CXXMethodDecl>(
                    coce->getDirectCallee()))
            {
                if (!cmd->isStatic()) {
                    assert(coce->getNumArgs() != 0);
                    if (!cmd->isConst()) {
                        recordIgnore(coce->getArg(0));
                    }
                    firstArg = 1;
                }
            }
        }
        // ignore those ones we are passing by reference
        const FunctionDecl* calleeFunctionDecl = expr->getDirectCallee();
        if (calleeFunctionDecl) {
            for (unsigned i = firstArg; i < expr->getNumArgs(); ++i) {
                if (i < calleeFunctionDecl->getNumParams()) {
                    QualType qt { calleeFunctionDecl->getParamDecl(i)->getType() };
                    if (loplugin::TypeCheck(qt).LvalueReference().NonConst()) {
                        recordIgnore(expr->getArg(i));
                    }
                    if (loplugin::TypeCheck(qt).Pointer().NonConst()) {
                        recordIgnore(expr->getArg(i));
                    }
                }
            }
        }
        return true;
    }

    bool VisitCXXConstructExpr(CXXConstructExpr const * expr) {
        // ignore those ones we are passing by reference
        const CXXConstructorDecl* cxxConstructorDecl = expr->getConstructor();
        for (unsigned i = 0; i < expr->getNumArgs(); ++i) {
            if (i < cxxConstructorDecl->getNumParams()) {
                QualType qt { cxxConstructorDecl->getParamDecl(i)->getType() };
                if (loplugin::TypeCheck(qt).LvalueReference().NonConst()) {
                    recordIgnore(expr->getArg(i));
                }
                if (loplugin::TypeCheck(qt).Pointer().NonConst()) {
                    recordIgnore(expr->getArg(i));
                }
            }
        }
        return true;
    }

    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitVarDecl( const VarDecl* );
    bool TraverseFunctionDecl( FunctionDecl* functionDecl );

private:
    std::unordered_set<VarDecl const *> maVarDeclSet;
    std::unordered_set<VarDecl const *> maVarDeclToIgnoreSet;
    std::unordered_map<VarDecl const *, int> maVarUsesMap;
    std::unordered_map<VarDecl const *, SourceRange> maVarUseSourceRangeMap;

    bool isConstantValueDependentExpression(Expr const * expr) {
        return ConstantValueDependentExpressionVisitor(compiler.getASTContext())
            .Visit(expr);
    }

    void recordIgnore(Expr const * expr) {
        for (;;) {
            expr = expr->IgnoreParenImpCasts();
            if (auto const e = dyn_cast<MemberExpr>(expr)) {
                if (isa<FieldDecl>(e->getMemberDecl())) {
                    expr = e->getBase();
                    continue;
                }
            }
            if (auto const e = dyn_cast<ArraySubscriptExpr>(expr)) {
                expr = e->getBase();
                continue;
            }
            if (auto const e = dyn_cast<BinaryOperator>(expr)) {
                if (e->getOpcode() == BO_PtrMemD) {
                    expr = e->getLHS();
                    continue;
                }
            }
            break;
        }
        auto const dre = dyn_cast<DeclRefExpr>(expr);
        if (dre == nullptr) {
            return;
        }
        auto const var = dyn_cast<VarDecl>(dre->getDecl());
        if (var == nullptr) {
            return;
        }
        maVarDeclToIgnoreSet.insert(var);
    }
};

bool OnceVar::TraverseFunctionDecl( FunctionDecl* functionDecl )
{
    // Ignore functions that contains #ifdef-ery, can be quite tricky
    // to make useful changes when this plugin fires in such functions
    if (containsPreprocessingConditionalInclusion(
            functionDecl->getSourceRange()))
        return true;
    return RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
}

bool OnceVar::VisitVarDecl( const VarDecl* varDecl )
{
    if (ignoreLocation(varDecl)) {
        return true;
    }
    if (auto const init = varDecl->getInit()) {
        recordIgnore(lookThroughInitListExpr(init));
    }
    if (varDecl->isExceptionVariable() || isa<ParmVarDecl>(varDecl)) {
        return true;
    }
    // ignore stuff in header files (which should really not be there, but anyhow)
    if (!compiler.getSourceManager().isInMainFile(varDecl->getLocation())) {
        return true;
    }
    // Ignore macros like FD_ZERO
    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(varDecl))) {
        return true;
    }
    if (varDecl->hasGlobalStorage()) {
        return true;
    }
    auto const tc = loplugin::TypeCheck(varDecl->getType());
    if (!varDecl->getType().isCXX11PODType(compiler.getASTContext())
        && !tc.Class("OString").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
        && !tc.Class("Color").GlobalNamespace()
        && !tc.Class("Pair").GlobalNamespace()
        && !tc.Class("Point").GlobalNamespace()
        && !tc.Class("Size").GlobalNamespace()
        && !tc.Class("Range").GlobalNamespace()
        && !tc.Class("Selection").GlobalNamespace()
        && !tc.Class("Rectangle").Namespace("tools").GlobalNamespace())
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
    if (isa<clang::StringLiteral>(initExpr)) {
        foundStringLiteral = true;
    } else if (auto constructExpr = dyn_cast<CXXConstructExpr>(initExpr)) {
        if (constructExpr->getNumArgs() == 0) {
            foundStringLiteral = true; // i.e., empty string
        } else {
            auto stringLit2 = dyn_cast<clang::StringLiteral>(constructExpr->getArg(0));
            foundStringLiteral = stringLit2 != nullptr;
        }
    }
    if (!foundStringLiteral) {
        auto const init = varDecl->getInit();
        if (!(init->isValueDependent()
              ? isConstantValueDependentExpression(init)
              : init->isConstantInitializer(
                  compiler.getASTContext(), false/*ForRef*/)))
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

    if (maVarUsesMap.find(varDecl) == maVarUsesMap.end()) {
        maVarUsesMap[varDecl] = 1;
        maVarUseSourceRangeMap[varDecl] = declRefExpr->getSourceRange();
    } else {
        maVarUsesMap[varDecl]++;
    }

    return true;
}

loplugin::Plugin::Registration< OnceVar > X("oncevar", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
