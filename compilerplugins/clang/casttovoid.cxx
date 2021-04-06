/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <map>
#include <stack>

#include "clang/AST/Attr.h"

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace {

bool isWarnUnusedType(QualType type) {
    if (auto const t = type->getAs<TypedefType>()) {
        if (t->getDecl()->hasAttr<WarnUnusedAttr>()) {
            return true;
        }
    }
    if (auto const t = type->getAs<RecordType>()) {
        if (t->getDecl()->hasAttr<WarnUnusedAttr>()) {
            return true;
        }
    }
    return loplugin::isExtraWarnUnusedType(type);
}

Expr const * lookThroughInitListExpr(Expr const * expr) {
    if (auto const ile = dyn_cast<InitListExpr>(expr->IgnoreParenImpCasts())) {
        if (ile->getNumInits() == 1) {
            return ile->getInit(0);
        }
    }
    return expr;
}

class CastToVoid final:
    public loplugin::FilteringPlugin<CastToVoid>
{
public:
    explicit CastToVoid(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    bool TraverseCStyleCastExpr(CStyleCastExpr * expr) {
        auto const dre = checkCast(expr);
        if (dre != nullptr) {
            castToVoid_.push({expr, dre});
        }
        auto const ret = RecursiveASTVisitor::TraverseCStyleCastExpr(expr);
        if (dre != nullptr) {
            assert(!castToVoid_.empty());
            assert(castToVoid_.top().cast == expr);
            assert(castToVoid_.top().sub == dre);
            castToVoid_.pop();
        }
        return ret;
    }

    bool TraverseCXXStaticCastExpr(CXXStaticCastExpr * expr) {
        auto const dre = checkCast(expr);
        if (dre != nullptr) {
            castToVoid_.push({expr, dre});
        }
        auto const ret = RecursiveASTVisitor::TraverseCXXStaticCastExpr(expr);
        if (dre != nullptr) {
            assert(!castToVoid_.empty());
            assert(castToVoid_.top().cast == expr);
            assert(castToVoid_.top().sub == dre);
            castToVoid_.pop();
        }
        return ret;
    }

    bool TraverseCXXFunctionalCastExpr(CXXFunctionalCastExpr * expr) {
        auto const dre = checkCast(expr);
        if (dre != nullptr) {
            castToVoid_.push({expr, dre});
        }
        auto const ret = RecursiveASTVisitor::TraverseCXXFunctionalCastExpr(
            expr);
        if (dre != nullptr) {
            assert(!castToVoid_.empty());
            assert(castToVoid_.top().cast == expr);
            assert(castToVoid_.top().sub == dre);
            castToVoid_.pop();
        }
        return ret;
    }

    bool TraverseFunctionDecl(FunctionDecl * decl) {
        returnTypes_.push(decl->getReturnType());
        auto const ret = RecursiveASTVisitor::TraverseFunctionDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == decl->getReturnType());
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXDeductionGuideDecl(CXXDeductionGuideDecl * decl) {
        returnTypes_.push(decl->getReturnType());
        auto const ret = RecursiveASTVisitor::TraverseCXXDeductionGuideDecl(
            decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == decl->getReturnType());
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXMethodDecl(CXXMethodDecl * decl) {
        returnTypes_.push(decl->getReturnType());
        auto const ret = RecursiveASTVisitor::TraverseCXXMethodDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == decl->getReturnType());
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXConstructorDecl(CXXConstructorDecl * decl) {
        returnTypes_.push(decl->getReturnType());
        auto const ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == decl->getReturnType());
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXDestructorDecl(CXXDestructorDecl * decl) {
        returnTypes_.push(decl->getReturnType());
        auto const ret = RecursiveASTVisitor::TraverseCXXDestructorDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == decl->getReturnType());
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXConversionDecl(CXXConversionDecl * decl) {
        returnTypes_.push(decl->getReturnType());
        auto const ret = RecursiveASTVisitor::TraverseCXXConversionDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == decl->getReturnType());
        returnTypes_.pop();
        return ret;
    }

    bool TraverseObjCMethodDecl(ObjCMethodDecl * decl) {
        returnTypes_.push(decl->getReturnType());
        auto const ret = RecursiveASTVisitor::TraverseObjCMethodDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == decl->getReturnType());
        returnTypes_.pop();
        return ret;
    }

    bool TraverseConstructorInitializer(CXXCtorInitializer * init) {
        if (auto const field = init->getAnyMember()) {
            if (loplugin::TypeCheck(field->getType()).LvalueReference()) {
                recordConsumption(lookThroughInitListExpr(init->getInit()));
            }
        }
        return RecursiveASTVisitor::TraverseConstructorInitializer(init);
    }

    bool TraverseLambdaExpr(LambdaExpr * expr, DataRecursionQueue * queue = nullptr) {
        if (!shouldTraversePostOrder()) {
            if (!WalkUpFromLambdaExpr(expr)) {
                return false;
            }
        }
        auto const n = expr->capture_size();
        for (unsigned i = 0; i != n; ++i) {
            auto const c = expr->capture_begin() + i;
            if (c->isExplicit() || shouldVisitImplicitCode()) {
                if (!TraverseLambdaCapture(expr, c, expr->capture_init_begin()[i])) {
                    return false;
                }
            }
        }
        if (!TraverseCXXRecordDecl(expr->getLambdaClass())) {
            return false;
        }
        if (!queue && shouldTraversePostOrder()) {
            if (!WalkUpFromLambdaExpr(expr)) {
                return false;
            }
        }
        return true;
    }

    bool VisitDeclRefExpr(DeclRefExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        auto const var = dyn_cast<VarDecl>(expr->getDecl());
        if (var == nullptr) {
            return true;
        }
        auto & usage = vars_[var->getCanonicalDecl()];
        if (!castToVoid_.empty() && castToVoid_.top().sub == expr) {
            usage.castToVoid.push_back(castToVoid_.top().cast);
        } else {
            usage.mentioned = true;
        }
        return true;
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        if (expr->getCastKind() != CK_LValueToRValue) {
            return true;
        }
        recordConsumption(expr->getSubExpr());
        return true;
    }

    bool VisitCallExpr(CallExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        unsigned firstArg = 0;
        if (auto const cmce = dyn_cast<CXXMemberCallExpr>(expr)) {
            if (auto const e1 = cmce->getMethodDecl()) {
                if (e1->isConst() || e1->isStatic()) {
                    recordConsumption(cmce->getImplicitObjectArgument());
                }
            } else if (auto const e2 = dyn_cast<BinaryOperator>(
                           cmce->getCallee()->IgnoreParenImpCasts()))
            {
                switch (e2->getOpcode()) {
                case BO_PtrMemD:
                case BO_PtrMemI:
                    if (e2->getRHS()->getType()->getAs<MemberPointerType>()
                        ->getPointeeType()->getAs<FunctionProtoType>()
                        ->isConst())
                    {
                        recordConsumption(e2->getLHS());
                    }
                    break;
                default:
                    break;
                }
            }
        } else if (isa<CXXOperatorCallExpr>(expr)) {
            if (auto const cmd = dyn_cast_or_null<CXXMethodDecl>(
                    expr->getDirectCallee()))
            {
                if (!cmd->isStatic()) {
                    assert(expr->getNumArgs() != 0);
                    if (cmd->isConst()) {
                        recordConsumption(expr->getArg(0));
                    }
                    firstArg = 1;
                }
            }
        }
        auto fun = expr->getDirectCallee();
        if (fun == nullptr) {
            return true;
        }
        unsigned const n = std::min(fun->getNumParams(), expr->getNumArgs());
        for (unsigned i = firstArg; i < n; ++i) {
            if (!loplugin::TypeCheck(fun->getParamDecl(i)->getType())
                .LvalueReference().Const())
            {
                continue;
            }
            recordConsumption(lookThroughInitListExpr(expr->getArg(i)));
        }
        return true;
    }

    bool VisitCXXConstructExpr(CXXConstructExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        auto const ctor = expr->getConstructor();
        unsigned const n = std::min(ctor->getNumParams(), expr->getNumArgs());
        for (unsigned i = 0; i != n; ++i) {
            if (!loplugin::TypeCheck(ctor->getParamDecl(i)->getType())
                .LvalueReference().Const())
            {
                continue;
            }
            recordConsumption(lookThroughInitListExpr(expr->getArg(i)));
        }
        return true;
    }

    bool VisitReturnStmt(ReturnStmt const * stmt) {
        if (ignoreLocation(stmt)) {
            return true;
        }
        assert(!returnTypes_.empty());
        if (!loplugin::TypeCheck(returnTypes_.top()).LvalueReference().Const())
        {
            return true;
        }
        auto const ret = stmt->getRetValue();
        if (ret == nullptr) {
            return true;
        }
        recordConsumption(lookThroughInitListExpr(ret));
        return true;
    }

    bool VisitVarDecl(VarDecl const * decl) {
        if (ignoreLocation(decl)) {
            return true;
        }
        if (!loplugin::TypeCheck(decl->getType()).LvalueReference()) {
            return true;
        }
        auto const init = decl->getInit();
        if (init == nullptr) {
            return true;
        }
        recordConsumption(lookThroughInitListExpr(init));
        return true;
    }

    bool VisitFieldDecl(FieldDecl const * decl) {
        if (ignoreLocation(decl)) {
            return true;
        }
        if (!loplugin::TypeCheck(decl->getType()).LvalueReference()) {
            return true;
        }
        auto const init = decl->getInClassInitializer();
        if (init == nullptr) {
            return true;
        }
        recordConsumption(lookThroughInitListExpr(init));
        return true;
    }

private:
    struct Usage {
        std::vector<ExplicitCastExpr const *> castToVoid;
        bool mentioned = false;
        DeclRefExpr const * firstConsumption = nullptr;
    };

    struct Cast {
        ExplicitCastExpr const * cast;
        DeclRefExpr const * sub;
    };

    std::map<VarDecl const *, Usage> vars_;
    std::stack<Cast> castToVoid_;
    std::stack<QualType> returnTypes_;

    void run() override {
        if (compiler.getPreprocessor().getIdentifierInfo("NDEBUG")->hasMacroDefinition()) {
            return;
        }
        if (!TraverseDecl(compiler.getASTContext().getTranslationUnitDecl())) {
            return;
        }
        for (auto const & i: vars_) {
            if (i.second.firstConsumption == nullptr) {
                if (i.second.mentioned) {
                    continue;
                }
                if (isa<ParmVarDecl>(i.first)) {
                    if (!compiler.getLangOpts().CPlusPlus
                        || isSharedCAndCppCode(i.first))
                    {
                        continue;
                    }
                    auto const ctxt = i.first->getDeclContext();
                    if (dyn_cast_or_null<ObjCMethodDecl>(ctxt) != nullptr) {
                        continue;
                    }
                    auto const fun = dyn_cast_or_null<FunctionDecl>(ctxt);
                    assert(fun != nullptr);
                    if (containsPreprocessingConditionalInclusion(
                            fun->getSourceRange()))
                    {
                        continue;
                    }
                    auto const meth = dyn_cast<CXXMethodDecl>(fun);
                    report(
                        DiagnosticsEngine::Warning,
                        "unused%select{| virtual function}0 parameter name",
                        i.first->getLocation())
                        << (meth != nullptr && meth->isVirtual())
                        << i.first->getSourceRange();
                    for (auto const j: i.second.castToVoid) {
                        report(
                            DiagnosticsEngine::Note, "cast to void here",
                            j->getExprLoc())
                            << j->getSourceRange();
                    }
                } else if (!i.second.castToVoid.empty()
                           && !isWarnUnusedType(i.first->getType()))
                {
                    auto const fun = dyn_cast_or_null<FunctionDecl>(i.first->getDeclContext());
                    assert(fun != nullptr);
                    if (containsPreprocessingConditionalInclusion(fun->getSourceRange())) {
                        continue;
                    }
                    report(
                        DiagnosticsEngine::Warning,
                        "unused variable %select{declaration|name}0",
                        i.first->getLocation())
                        << i.first->isExceptionVariable()
                        << i.first->getSourceRange();
                    for (auto const j: i.second.castToVoid) {
                        report(
                            DiagnosticsEngine::Note, "cast to void here",
                            j->getExprLoc())
                            << j->getSourceRange();
                    }
                }
            } else {
                for (auto const j: i.second.castToVoid) {
                    report(
                        DiagnosticsEngine::Warning, "unnecessary cast to void",
                        j->getExprLoc())
                        << j->getSourceRange();
                    report(
                        DiagnosticsEngine::Note, "first consumption is here",
                        i.second.firstConsumption->getExprLoc())
                        << i.second.firstConsumption->getSourceRange();
                }
            }
        }
    }

    bool isFromCIncludeFile(SourceLocation spellingLocation) const {
        return !compiler.getSourceManager().isInMainFile(spellingLocation)
            && (StringRef(
                    compiler.getSourceManager().getPresumedLoc(spellingLocation)
                    .getFilename())
                .endswith(".h"));
    }

    bool isSharedCAndCppCode(VarDecl const * decl) const {
        auto loc = decl->getLocation();
        while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        // Assume that code is intended to be shared between C and C++ if it
        // comes from an include file ending in .h, and is either in an extern
        // "C" context or the body of a macro definition:
        return
            isFromCIncludeFile(compiler.getSourceManager().getSpellingLoc(loc))
            && (decl->isInExternCContext()
                || compiler.getSourceManager().isMacroBodyExpansion(loc));
    }

    DeclRefExpr const * checkCast(ExplicitCastExpr const * expr) {
        if (!loplugin::TypeCheck(expr->getTypeAsWritten()).Void()) {
            return nullptr;
        }
        if (compiler.getSourceManager().isMacroBodyExpansion(
                compat::getBeginLoc(expr)))
        {
            return nullptr;
        }
        return dyn_cast<DeclRefExpr>(expr->getSubExpr()->IgnoreParenImpCasts());
    }

    void recordConsumption(Expr const * expr) {
        for (;;) {
            expr = expr->IgnoreParenImpCasts();
            if (auto const e = dyn_cast<MemberExpr>(expr)) {
                expr = e->getBase();
                continue;
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
        // In C (but not in C++)
        //
        //   (void) x
        //
        // contains an implicit lvalue-to-rvalue cast, so VisitImplicitCastExpr
        // would record that as a consumption if we didn't filter it out here:
        if (!castToVoid_.empty() && castToVoid_.top().sub == dre) {
            return;
        }
        auto const var = dyn_cast<VarDecl>(dre->getDecl());
        if (var == nullptr) {
            return;
        }
        auto & usage = vars_[var->getCanonicalDecl()];
        if (usage.firstConsumption != nullptr) {
            return;
        }
        auto const loc = compat::getBeginLoc(dre);
        if (compiler.getSourceManager().isMacroArgExpansion(loc)
            && (Lexer::getImmediateMacroNameForDiagnostics(
                    loc, compiler.getSourceManager(), compiler.getLangOpts())
                == "assert"))
        {
            return;
        }
        usage.firstConsumption = dre;
    }
};

static loplugin::Plugin::Registration<CastToVoid> reg("casttovoid");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
