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
#include "clang/AST/CXXInheritance.h"

namespace {

// Like clang::Stmt::IgnoreImplicit (lib/AST/Stmt.cpp), but also looking through implicit
// UserDefinedConversion's member function call:
Expr const * ignoreAllImplicit(Expr const * expr) {
    if (auto const e = dyn_cast<ExprWithCleanups>(expr)) {
        expr = e->getSubExpr();
    }
    if (auto const e = dyn_cast<MaterializeTemporaryExpr>(expr)) {
        expr = e->GetTemporaryExpr();
    }
    if (auto const e = dyn_cast<CXXBindTemporaryExpr>(expr)) {
        expr = e->getSubExpr();
    }
    while (auto const e = dyn_cast<ImplicitCastExpr>(expr)) {
        expr = e->getSubExpr();
        if (e->getCastKind() == CK_UserDefinedConversion) {
            auto const ce = cast<CXXMemberCallExpr>(expr);
            assert(ce->getNumArgs() == 0);
            expr = ce->getImplicitObjectArgument();
        }
    }
    return expr;
}

Expr const * ignoreParenImpCastAndComma(Expr const * expr) {
    for (;;) {
        expr = expr->IgnoreParenImpCasts();
        auto e = dyn_cast<BinaryOperator>(expr);
        if (e == nullptr || e->getOpcode() != BO_Comma) {
            return expr;
        }
        expr = e->getRHS();
    }
}

Expr const * getSubExprOfLogicalNegation(Expr const * expr) {
    auto e = dyn_cast<UnaryOperator>(ignoreParenImpCastAndComma(expr));
    return e == nullptr || e->getOpcode() != UO_LNot
        ? nullptr : e->getSubExpr();
}

clang::Type const * stripConstRef(clang::Type const * type) {
    auto lvalueType = dyn_cast<LValueReferenceType>(type);
    if (!lvalueType)
        return type;
    return lvalueType->getPointeeType()->getUnqualifiedDesugaredType();
}

bool isCompatibleTypeForOperator(clang::Type const * paramType, CXXRecordDecl const * argRecordDecl) {
    paramType = stripConstRef(paramType);
    auto paramRecordType = dyn_cast<RecordType>(paramType);
    if (!paramRecordType)
        return false;
    CXXRecordDecl const * paramRecordDecl = dyn_cast<CXXRecordDecl>(paramRecordType->getDecl());
    if (!paramRecordDecl)
        return false;
    return argRecordDecl == paramRecordDecl || argRecordDecl->isDerivedFrom(paramRecordDecl);
}

FunctionDecl const * findMemberOperator(CXXRecordDecl const * recordDecl, OverloadedOperatorKind ooOpcode, CXXRecordDecl const * rhs) {
    for (auto it = recordDecl->method_begin(); it != recordDecl->method_end(); ++it) {
        if (it->getOverloadedOperator() == ooOpcode) {
            if (it->getNumParams() == 1 && isCompatibleTypeForOperator(it->getParamDecl(0)->getType().getTypePtr(), rhs))
                return *it;
        }
    }
    return nullptr;
}

// Magic value to indicate we assume this operator exists
static FunctionDecl const * const ASSUME_OPERATOR_EXISTS = reinterpret_cast<FunctionDecl const *>(-1);

// Search for an operator with matching parameter types; while this may miss some operators with
// odd parameter types that would actually be used by the compiler, it is overall better to have too
// many false negatives (i.e., miss valid loplugin:simplifybool warnings) than false positives here:
FunctionDecl const * findOperator(CompilerInstance& compiler, BinaryOperator::Opcode opcode, clang::Type const * lhsType, clang::Type const * rhsType) {
    auto lhsRecordType = dyn_cast<RecordType>(lhsType);
    if (!lhsRecordType)
        return nullptr;
    auto rhsRecordType = dyn_cast<RecordType>(rhsType);
    if (!rhsRecordType)
        return nullptr;
    CXXRecordDecl const * lhsRecordDecl = dyn_cast<CXXRecordDecl>(lhsRecordType->getDecl());
    if (!lhsRecordDecl)
        return nullptr;
    CXXRecordDecl const * rhsRecordDecl = dyn_cast<CXXRecordDecl>(rhsRecordType->getDecl());
    if (!rhsRecordDecl)
        return nullptr;

    auto ctx = lhsRecordDecl->getCanonicalDecl()->getDeclContext();

    /*
        It looks the clang Sema::LookupOverloadedOperatorName is the chunk of functionality I need,
        but I have no idea how to call it from here.
        Actually finding the right standard library operators requires doing conversions and other funky stuff.
        For now, just assume that standard library operators are well-behaved, and have negated operators.
    */
    if (ctx->isStdNamespace())
        return ASSUME_OPERATOR_EXISTS;
    if (auto namespaceDecl = dyn_cast<NamespaceDecl>(ctx)) {
        // because, of course, half the standard library is not "in the standard namespace"
        if (namespaceDecl->getName() == "__gnu_debug")
            return ASSUME_OPERATOR_EXISTS;
    }

    // search for member overloads
    // (using the hard way here because DeclContext::lookup does not work for member operators)
    auto ooOpcode = BinaryOperator::getOverloadedOperator(opcode);
    FunctionDecl const * foundFunction = findMemberOperator(lhsRecordDecl, ooOpcode, rhsRecordDecl);
    if (foundFunction)
        return foundFunction;
    auto ForallBasesCallback = [&](const CXXRecordDecl *baseCXXRecordDecl)
    {
        if (baseCXXRecordDecl->isInvalidDecl())
            return false;
        foundFunction = findMemberOperator(baseCXXRecordDecl, ooOpcode, rhsRecordDecl);
        return false;
    };

    lhsRecordDecl->forallBases(ForallBasesCallback, /*AllowShortCircuit*/true);
    if (foundFunction)
        return foundFunction;

    // search for free function overloads
    if (ctx->getDeclKind() == Decl::LinkageSpec) {
        ctx = ctx->getParent();
    }
    auto operatorDeclName = compiler.getASTContext().DeclarationNames.getCXXOperatorName(ooOpcode);
    auto res = ctx->lookup(operatorDeclName);
    for (auto d = res.begin(); d != res.end(); ++d) {
        FunctionDecl const * f = dyn_cast<FunctionDecl>(*d);
        if (!f || f->getNumParams() != 2)
            continue;
        if (!isCompatibleTypeForOperator(f->getParamDecl(0)->getType().getTypePtr(), lhsRecordDecl))
            continue;
        if (!isCompatibleTypeForOperator(f->getParamDecl(1)->getType().getTypePtr(), rhsRecordDecl))
            continue;
        return f;
    }
    return nullptr;
}

enum class Value { Unknown, False, True };

Value getValue(Expr const * expr) {
    expr = ignoreParenImpCastAndComma(expr);
    if (expr->getType()->isBooleanType()) {
        // Instead going via Expr::isCXX11ConstantExpr would turn up exactly one
        // additional place in svx/source/dialog/framelinkarray.cxx
        //
        //   const bool DIAG_DBL_CLIP_DEFAULT = false;
        //   ...
        //   ... = mxImpl.get() ? mxImpl->mbDiagDblClip : DIAG_DBL_CLIP_DEFAULT;
        //
        // where it is unclear whether it is not actually better to consider
        // DIAG_DBL_CLIP_DEFAULT a tunable parameter (and thus not to simplify):
        auto lit = dyn_cast<CXXBoolLiteralExpr>(expr);
        if (lit != nullptr) {
            return lit->getValue() ? Value::True : Value::False;
        }
    }
    return Value::Unknown;
}

class SimplifyBool:
    public loplugin::FilteringPlugin<SimplifyBool>
{
public:
    explicit SimplifyBool(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    void run() override;

    bool VisitUnaryLNot(UnaryOperator const * expr);

    bool VisitBinLT(BinaryOperator const * expr);

    bool VisitBinGT(BinaryOperator const * expr);

    bool VisitBinLE(BinaryOperator const * expr);

    bool VisitBinGE(BinaryOperator const * expr);

    bool VisitBinEQ(BinaryOperator const * expr);

    bool VisitBinNE(BinaryOperator const * expr);

    bool VisitConditionalOperator(ConditionalOperator const * expr);

    bool TraverseFunctionDecl(FunctionDecl *);

    bool TraverseCXXMethodDecl(CXXMethodDecl *);

private:
    FunctionDecl* m_insideFunctionDecl = nullptr;
};

void SimplifyBool::run() {
    if (compiler.getLangOpts().CPlusPlus) {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
}

bool SimplifyBool::VisitUnaryLNot(UnaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto e = getSubExprOfLogicalNegation(expr->getSubExpr());
    if (e) {
        // Ignore macros, otherwise
        //    OSL_ENSURE(!b, ...);
        // triggers.
        if (compat::getBeginLoc(e).isMacroID())
            return true;
        // double logical not of an int is an idiom to convert to bool
        auto const sub = ignoreAllImplicit(e);
        if (!sub->getType()->isBooleanType())
            return true;
        report(
            DiagnosticsEngine::Warning,
            ("double logical negation expression of the form '!!A' (with A of type"
             " %0) can %select{logically|literally}1 be simplified as 'A'"),
            compat::getBeginLoc(expr))
            << sub->getType()
            << sub->getType()->isBooleanType()
            << expr->getSourceRange();
        return true;
    }
    if (auto binaryOp = dyn_cast<BinaryOperator>(expr->getSubExpr()->IgnoreParenImpCasts())) {
        // Ignore macros, otherwise
        //    OSL_ENSURE(!b, ...);
        // triggers.
        if (compat::getBeginLoc(binaryOp).isMacroID())
            return true;
        if (binaryOp->isComparisonOp())
        {
            auto t = binaryOp->getLHS()->IgnoreImpCasts()->getType()->getUnqualifiedDesugaredType();
            if (t->isTemplateTypeParmType() || t->isDependentType() || t->isRecordType())
                return true;
            // for floating point (with NaN) !(x<y) need not be equivalent to x>=y
            if (t->isFloatingType() ||
                binaryOp->getRHS()->IgnoreImpCasts()->getType()->getUnqualifiedDesugaredType()->isFloatingType())
                return true;
            report(
                DiagnosticsEngine::Warning,
                ("logical negation of comparison operator, can be simplified by inverting operator"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
        }
        else if (binaryOp->isLogicalOp())
        {
            auto containsNegation = [](Expr const * expr) {
                expr = ignoreParenImpCastAndComma(expr);
                if (auto unaryOp = dyn_cast<UnaryOperator>(expr))
                    if (unaryOp->getOpcode() == UO_LNot)
                        return expr;
                if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
                    if (binaryOp->getOpcode() == BO_NE)
                        return expr;
                if (auto cxxOpCall = dyn_cast<CXXOperatorCallExpr>(expr))
                    if (cxxOpCall->getOperator() == OO_ExclaimEqual)
                        return expr;
                return (Expr const*)nullptr;
            };
            auto lhs = containsNegation(binaryOp->getLHS());
            auto rhs = containsNegation(binaryOp->getRHS());
            if (!lhs || !rhs)
                return true;
            if (lhs || rhs)
                report(
                    DiagnosticsEngine::Warning,
                    ("logical negation of logical op containing negation, can be simplified"),
                    compat::getBeginLoc(binaryOp))
                    << binaryOp->getSourceRange();
        }
    }
    if (auto binaryOp = dyn_cast<CXXOperatorCallExpr>(expr->getSubExpr()->IgnoreParenImpCasts())) {
        // Ignore macros, otherwise
        //    OSL_ENSURE(!b, ...);
        // triggers.
        if (compat::getBeginLoc(binaryOp).isMacroID())
            return true;
        auto op = binaryOp->getOperator();
        // Negating things like > and >= would probably not be wise, there is no guarantee the negation holds for operator overloaded types.
        // However, == and != are normally considered ok.
        if (!(op == OO_EqualEqual || op == OO_ExclaimEqual))
            return true;
        BinaryOperator::Opcode negatedOpcode = BinaryOperator::negateComparisonOp(BinaryOperator::getOverloadedOpcode(op));
        auto lhs = binaryOp->getArg(0)->IgnoreImpCasts()->getType()->getUnqualifiedDesugaredType();
        auto rhs = binaryOp->getArg(1)->IgnoreImpCasts()->getType()->getUnqualifiedDesugaredType();
        auto const negOp = findOperator(compiler, negatedOpcode, lhs, rhs);
        if (!negOp)
            return true;
        // if we are inside a similar operator, ignore, eg. operator!= is often defined by calling !operator==
        if (m_insideFunctionDecl && m_insideFunctionDecl->getNumParams() >= 1) {
            auto t = stripConstRef(m_insideFunctionDecl->getParamDecl(0)->getType().getTypePtr());
            if (t == lhs)
                return true;
        }
        // QA code
        StringRef fn(handler.getMainFileName());
        if (loplugin::isSamePathname(fn, SRCDIR "/testtools/source/bridgetest/bridgetest.cxx"))
            return true;
        report(
            DiagnosticsEngine::Warning,
            ("logical negation of comparison operator, can be simplified by inverting operator"),
            compat::getBeginLoc(expr))
            << expr->getSourceRange();
        if (negOp != ASSUME_OPERATOR_EXISTS)
            report(
                DiagnosticsEngine::Note, "the presumed corresponding negated operator is declared here",
                negOp->getLocation())
                << negOp->getSourceRange();
    }
    return true;
}

bool SimplifyBool::VisitBinLT(BinaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (!(expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
          && expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()))
    {
        return true;
    }
    auto v1 = getValue(expr->getLHS());
    auto v2 = getValue(expr->getRHS());
    switch (v1) {
    case Value::Unknown:
        switch (v2) {
        case Value::Unknown:
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'A < false' (with A of type"
                 " %0) can logically be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::True:
            {
                auto e = getSubExprOfLogicalNegation(expr->getLHS());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("less-than expression of the form 'A < true' (with A"
                         " of type %0) can %select{logically|literally}1 be"
                         " simplified as '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getLHS()->IgnoreImpCasts()->getType()
                        << (expr->getLHS()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("less-than expression of the form '!A < true' (with A"
                         " of type %0) can %select{logically|literally}1 be"
                         " simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        }
        break;
    case Value::False:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'false < A' (with A of type"
                 " %0) can %select{logically|literally}1 be simplified as 'A'"),
                compat::getBeginLoc(expr))
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'false < false' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'false < true' can"
                 " literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::True:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'true < A' (with A of type"
                 " %0) can logically be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'true < false' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'true < true' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    }
    return true;
}

bool SimplifyBool::VisitBinGT(BinaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (!(expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
          && expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()))
    {
        return true;
    }
    auto v1 = getValue(expr->getLHS());
    auto v2 = getValue(expr->getRHS());
    switch (v1) {
    case Value::Unknown:
        switch (v2) {
        case Value::Unknown:
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'A > false' (with A of"
                 " type %0) can %select{logically|literally}1 be simplified as"
                 " 'A'"),
                compat::getBeginLoc(expr))
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'A > true' (with A of"
                 " type %0) can logically be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::False:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'false > A' (with A of"
                 " type %0) can logically be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'false > false' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'false > true' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::True:
        switch (v2) {
        case Value::Unknown:
            {
                auto e = getSubExprOfLogicalNegation(expr->getRHS());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("greater-than expression of the form 'true > A' (with"
                         " A of type %0) can %select{logically|literally}1 be"
                         " simplified as '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getRHS()->IgnoreImpCasts()->getType()
                        << (expr->getRHS()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("greater-than expression of the form 'true > !A' (with"
                         " A of type %0) can %select{logically|literally}1 be"
                         " simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'true > false' can"
                 " literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'true > true' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    }
    return true;
}

bool SimplifyBool::VisitBinLE(BinaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (!(expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
          && expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()))
    {
        return true;
    }
    auto v1 = getValue(expr->getLHS());
    auto v2 = getValue(expr->getRHS());
    switch (v1) {
    case Value::Unknown:
        switch (v2) {
        case Value::Unknown:
            break;
        case Value::False:
            {
                auto e = getSubExprOfLogicalNegation(expr->getLHS());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("less-than-or-equal-to expression of the form 'A <="
                         " false' (with A of type %0) can"
                         " %select{logically|literally}1 be simplified as"
                         " '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getLHS()->IgnoreImpCasts()->getType()
                        << (expr->getLHS()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("less-than-or-equal-to expression of the form '!A <="
                         " false' (with A of type %0) can"
                         " %select{logically|literally}1 be simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'A <= true'"
                 " (with A of type %0) can logically be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::False:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'false <= A'"
                 " (with A of type %0) can logically be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'false <= false'"
                 " can literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'false <= true'"
                 " can literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::True:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'true <= A'"
                 " (with A of type %0) can %select{logically|literally}1 be"
                 " simplified as 'A'"),
                compat::getBeginLoc(expr))
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'true <= false'"
                 " can literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'true <= true'"
                 " can literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    }
    return true;
}

bool SimplifyBool::VisitBinGE(BinaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (!(expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
          && expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()))
    {
        return true;
    }
    auto v1 = getValue(expr->getLHS());
    auto v2 = getValue(expr->getRHS());
    switch (v1) {
    case Value::Unknown:
        switch (v2) {
        case Value::Unknown:
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'A >= false'"
                 " (with A of type %0) can logically be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'A >= true'"
                 " (with A of type %0) can %select{logically|literally}1 be"
                 " simplified as 'A'"),
                compat::getBeginLoc(expr))
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::False:
        switch (v2) {
        case Value::Unknown:
            {
                auto e = getSubExprOfLogicalNegation(expr->getRHS());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("greater-than-or-equal-to expression of the form"
                         " 'false >= A' (with A of type %0) can"
                         " %select{logically|literally}1 be simplified as"
                         " '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getRHS()->IgnoreImpCasts()->getType()
                        << (expr->getRHS()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("greater-than-or-equal-to expression of the form"
                         " 'false >= !A' (with A of type %0) can"
                         " %select{logically|literally}1 be simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'false >="
                 " false' can literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'false >="
                 " true' can literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::True:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'true >= A'"
                 " (with A of type %0) can logically be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'true >="
                 " false' can literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'true >="
                 " true' can literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    }
    return true;
}

bool SimplifyBool::VisitBinEQ(BinaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (!(expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
          && expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()))
    {
        return true;
    }
    auto v1 = getValue(expr->getLHS());
    auto v2 = getValue(expr->getRHS());
    switch (v1) {
    case Value::Unknown:
        switch (v2) {
        case Value::Unknown:
            break;
        case Value::False:
            {
                auto e = getSubExprOfLogicalNegation(expr->getLHS());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("equal-to expression of the form 'A == false' (with A"
                         " of type %0) can %select{logically|literally}1 be"
                         " simplified as '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getLHS()->IgnoreImpCasts()->getType()
                        << (expr->getLHS()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("equal-to expression of the form '!A == false' (with A"
                         " of type %0) can %select{logically|literally}1 be"
                         " simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'A == true' (with A of type"
                 " %0) can %select{logically|literally}1 be simplified as 'A'"),
                compat::getBeginLoc(expr))
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::False:
        switch (v2) {
        case Value::Unknown:
            {
                auto e = getSubExprOfLogicalNegation(expr->getRHS());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("equal-to expression of the form 'false == A' (with A"
                         " of type %0) can %select{logically|literally}1 be"
                         " simplified as '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getRHS()->IgnoreImpCasts()->getType()
                        << (expr->getRHS()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("equal-to expression of the form 'false == !A' (with A"
                         " of type %0) can %select{logically|literally}1 be"
                         " simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'false == false' can"
                 " literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'false == true' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::True:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'true == A' (with A of type"
                 " %0) can %select{logically|literally}1 be simplified as 'A'"),
                compat::getBeginLoc(expr))
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'true == false' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'true == true' can"
                 " literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    }
    return true;
}

bool SimplifyBool::VisitBinNE(BinaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (!(expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
          && expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()))
    {
        return true;
    }
    auto v1 = getValue(expr->getLHS());
    auto v2 = getValue(expr->getRHS());
    switch (v1) {
    case Value::Unknown:
        switch (v2) {
        case Value::Unknown:
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'A != false' (with A of"
                 " type %0) can %select{logically|literally}1 be simplified as"
                 " 'A'"),
                compat::getBeginLoc(expr))
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::True:
            {
                auto e = getSubExprOfLogicalNegation(expr->getLHS());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("not-equal-to expression of the form 'A != true' (with"
                         " A of type %0) can %select{logically|literally}1 be"
                         " simplified as '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getLHS()->IgnoreImpCasts()->getType()
                        << (expr->getLHS()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("not-equal-to expression of the form '!A != true'"
                         " (with A of type %0) can"
                         " %select{logically|literally}1 be simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        }
        break;
    case Value::False:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'false != A' (with A of"
                 " type %0) can %select{logically|literally}1 be simplified as"
                 " 'A'"),
                compat::getBeginLoc(expr))
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'false != false' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'false != true' can"
                 " literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    case Value::True:
        switch (v2) {
        case Value::Unknown:
            {
                auto e = getSubExprOfLogicalNegation(expr->getRHS());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("not-equal-to expression of the form 'true != A' (with"
                         " A of type %0) can %select{logically|literally}1 be"
                         " simplified as '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getRHS()->IgnoreImpCasts()->getType()
                        << (expr->getRHS()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("not-equal-to expression of the form 'true != !A'"
                         " (with A of type %0) can"
                         " %select{logically|literally}1 be simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'true != false' can"
                 " literally be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'true != true' can"
                 " literally be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getSourceRange();
            break;
        }
        break;
    }
    return true;
}

bool SimplifyBool::VisitConditionalOperator(ConditionalOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto v1 = getValue(expr->getTrueExpr());
    auto v2 = getValue(expr->getFalseExpr());
    switch (v1) {
    case Value::Unknown:
        switch (v2) {
        case Value::Unknown:
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("conditional expression of the form 'A ? B : false' (with A of"
                 " type %0 and B of type %1) can %select{logically|literally}2"
                 " be simplified as 'A && B'"),
                compat::getBeginLoc(expr))
                << expr->getCond()->IgnoreImpCasts()->getType()
                << expr->getTrueExpr()->IgnoreImpCasts()->getType()
                << ((expr->getCond()->IgnoreImpCasts()->getType()
                     ->isBooleanType())
                    && (expr->getTrueExpr()->IgnoreImpCasts()->getType()
                        ->isBooleanType()))
                << expr->getSourceRange();
            break;
        case Value::True:
            {
                auto e = getSubExprOfLogicalNegation(expr->getCond());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("conditional expression of the form 'A ? B : true'"
                         " (with A of type %0 and B of type %1) can"
                         " %select{logically|literally}2 be simplified as '!A"
                         " || B'"),
                        compat::getBeginLoc(expr))
                        << expr->getCond()->IgnoreImpCasts()->getType()
                        << expr->getTrueExpr()->IgnoreImpCasts()->getType()
                        << ((expr->getCond()->IgnoreImpCasts()->getType()
                             ->isBooleanType())
                            && (expr->getTrueExpr()->IgnoreImpCasts()->getType()
                                ->isBooleanType()))
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("conditional expression of the form '!A ? B : true'"
                         " (with A of type %0 and B of type %1) can"
                         " %select{logically|literally}2 be simplified as 'A ||"
                         " B'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << expr->getTrueExpr()->IgnoreImpCasts()->getType()
                        << (e->IgnoreImpCasts()->getType()->isBooleanType()
                            && (expr->getTrueExpr()->IgnoreImpCasts()
                                ->getType()->isBooleanType()))
                        << expr->getSourceRange();
                }
                break;
            }
        }
        break;
    case Value::False:
        switch (v2) {
        case Value::Unknown:
            {
                auto e = getSubExprOfLogicalNegation(expr->getCond());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("conditional expression of the form 'A ? false : B'"
                         " (with A of type %0 and B of type %1) can"
                         " %select{logically|literally}2 be simplified as '!A"
                         " && B'"),
                        compat::getBeginLoc(expr))
                        << expr->getCond()->IgnoreImpCasts()->getType()
                        << expr->getFalseExpr()->IgnoreImpCasts()->getType()
                        << ((expr->getCond()->IgnoreImpCasts()->getType()
                             ->isBooleanType())
                            && (expr->getFalseExpr()->IgnoreImpCasts()
                                ->getType()->isBooleanType()))
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("conditional expression of the form '!A ? false : B'"
                         " (with A of type %0 and B of type %1) can"
                         " %select{logically|literally}2 be simplified as 'A &&"
                         " B'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << expr->getFalseExpr()->IgnoreImpCasts()->getType()
                        << (e->IgnoreImpCasts()->getType()->isBooleanType()
                            && (expr->getFalseExpr()->IgnoreImpCasts()
                                ->getType()->isBooleanType()))
                        << expr->getSourceRange();
                }
                break;
            }
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("conditional expression of the form 'A ? false : false' (with"
                 " A of type %0) can logically be simplified as 'false'"),
                compat::getBeginLoc(expr))
                << expr->getCond()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::True:
            {
                auto e = getSubExprOfLogicalNegation(expr->getCond());
                if (e == nullptr) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("conditional expression of the form 'A ? false : true'"
                         " (with A of type %0) can"
                         " %select{logically|literally}1 be simplified as"
                         " '!A'"),
                        compat::getBeginLoc(expr))
                        << expr->getCond()->IgnoreImpCasts()->getType()
                        << (expr->getCond()->IgnoreImpCasts()->getType()
                            ->isBooleanType())
                        << expr->getSourceRange();
                } else {
                    report(
                        DiagnosticsEngine::Warning,
                        ("conditional expression of the form '!A ? false :"
                         " true' (with A of type %0) can"
                         " %select{logically|literally}1 be simplified as 'A'"),
                        compat::getBeginLoc(expr))
                        << e->IgnoreImpCasts()->getType()
                        << e->IgnoreImpCasts()->getType()->isBooleanType()
                        << expr->getSourceRange();
                }
                break;
            }
        }
        break;
    case Value::True:
        switch (v2) {
        case Value::Unknown:
            report(
                DiagnosticsEngine::Warning,
                ("conditional expression of the form 'A ? true : B' (with A of"
                 " type %0 and B of type %1) can %select{logically|literally}2"
                 " be simplified as 'A || B'"),
                compat::getBeginLoc(expr))
                << expr->getCond()->IgnoreImpCasts()->getType()
                << expr->getFalseExpr()->IgnoreImpCasts()->getType()
                << ((expr->getCond()->IgnoreImpCasts()->getType()
                     ->isBooleanType())
                    && (expr->getFalseExpr()->IgnoreImpCasts()->getType()
                        ->isBooleanType()))
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("conditional expression of the form 'A ? true : false' (with A"
                 " of type %0) can %select{logically|literally}1 be simplified"
                 " as 'A'"),
                compat::getBeginLoc(expr))
                << expr->getCond()->IgnoreImpCasts()->getType()
                << expr->getCond()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("conditional expression of the form 'A ? true : true' (with A"
                 " of type %0) can logically be simplified as 'true'"),
                compat::getBeginLoc(expr))
                << expr->getCond()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        }
        break;
    }
    return true;
}

bool SimplifyBool::TraverseFunctionDecl(FunctionDecl * functionDecl) {
    auto copy = m_insideFunctionDecl;
    m_insideFunctionDecl = functionDecl;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
    m_insideFunctionDecl = copy;
    return ret;
}

bool SimplifyBool::TraverseCXXMethodDecl(CXXMethodDecl * functionDecl) {
    auto copy = m_insideFunctionDecl;
    m_insideFunctionDecl = functionDecl;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(functionDecl);
    m_insideFunctionDecl = copy;
    return ret;
}

loplugin::Plugin::Registration<SimplifyBool> X("simplifybool");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
