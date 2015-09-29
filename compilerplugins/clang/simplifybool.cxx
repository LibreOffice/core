/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

namespace {

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
    public RecursiveASTVisitor<SimplifyBool>, public loplugin::Plugin
{
public:
    explicit SimplifyBool(InstantiationData const & data): Plugin(data) {}

    void run() override;

    bool VisitUnaryLNot(UnaryOperator const * expr);

    bool VisitBinLT(BinaryOperator const * expr);

    bool VisitBinGT(BinaryOperator const * expr);

    bool VisitBinLE(BinaryOperator const * expr);

    bool VisitBinGE(BinaryOperator const * expr);

    bool VisitBinEQ(BinaryOperator const * expr);

    bool VisitBinNE(BinaryOperator const * expr);

    bool VisitConditionalOperator(ConditionalOperator const * expr);
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
    if (e == nullptr) {
        return true;
    }
/* hits for OSL_ENSURE(!b, ...);
    report(
        DiagnosticsEngine::Warning,
        ("double logical negation expression of the form '!!A' (with A of type"
         " %0) can %select{logically|literally}1 be simplified as 'A'"),
        expr->getLocStart())
        << e->IgnoreImpCasts()->getType()
        << e->IgnoreImpCasts()->getType()->isBooleanType()
        << expr->getSourceRange();
*/
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
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'false < false' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'false < true' can"
                 " literally be simplified as 'true'"),
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'true < false' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than expression of the form 'true < true' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getLHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'A > true' (with A of"
                 " type %0) can logically be simplified as 'false'"),
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'false > false' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'false > true' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than expression of the form 'true > true' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'false <= false'"
                 " can literally be simplified as 'true'"),
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'false <= true'"
                 " can literally be simplified as 'true'"),
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'true <= false'"
                 " can literally be simplified as 'false'"),
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("less-than-or-equal-to expression of the form 'true <= true'"
                 " can literally be simplified as 'true'"),
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getLHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'A >= true'"
                 " (with A of type %0) can %select{logically|literally}1 be"
                 " simplified as 'A'"),
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'false >="
                 " true' can literally be simplified as 'false'"),
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'true >="
                 " false' can literally be simplified as 'true'"),
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("greater-than-or-equal-to expression of the form 'true >="
                 " true' can literally be simplified as 'true'"),
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'false == true' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'true == false' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("equal-to expression of the form 'true == true' can"
                 " literally be simplified as 'true'"),
                expr->getLocStart())
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
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
                << expr->getRHS()->IgnoreImpCasts()->getType()
                << expr->getRHS()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::False:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'false != false' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'false != true' can"
                 " literally be simplified as 'true'"),
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("not-equal-to expression of the form 'true != true' can"
                 " literally be simplified as 'false'"),
                expr->getLocStart())
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
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
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
                        expr->getLocStart())
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
                        expr->getLocStart())
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
                expr->getLocStart())
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
                expr->getLocStart())
                << expr->getCond()->IgnoreImpCasts()->getType()
                << expr->getCond()->IgnoreImpCasts()->getType()->isBooleanType()
                << expr->getSourceRange();
            break;
        case Value::True:
            report(
                DiagnosticsEngine::Warning,
                ("conditional expression of the form 'A ? true : true' (with A"
                 " of type %0) can logically be simplified as 'true'"),
                expr->getLocStart())
                << expr->getCond()->IgnoreImpCasts()->getType()
                << expr->getSourceRange();
            break;
        }
        break;
    }
    return true;
}

loplugin::Plugin::Registration<SimplifyBool> X("simplifybool");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
