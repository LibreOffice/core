/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
*/

#ifndef LO_CLANG_SHARED_PLUGINS

#include <algorithm>
#include <cassert>
#include <list>
#include <map>

#include "plugin.hxx"
//#include "clang/AST/CXXInheritance.h"

// Idea from bubli. Check that the index variable in a for loop is able to cover the range
// revealed by the terminating condition.
// If not, we might end up in an endless loop, or just not process certain parts.

namespace
{

class LoopVarTooSmall:
    public loplugin::FilteringPlugin<LoopVarTooSmall>
{
public:
    explicit LoopVarTooSmall(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitForStmt( const ForStmt* stmt ) {
        checkExpr(stmt->getCond());
        return true;
    }

    bool VisitWhileStmt(WhileStmt const * stmt) {
        checkExpr(stmt->getCond());
        return true;
    }

    bool VisitDoStmt(DoStmt const * stmt) {
        checkExpr(stmt->getCond());
        return true;
    }

private:
    unsigned getIntValueWidth(QualType type) const;

    void checkSubExpr(Expr const * expr, bool positive);

    void checkExpr(Expr const * expr);

    struct Comparison {
        BinaryOperator const * op;
        unsigned rhsWidth;
    };

    struct Comparisons {
        std::list<Comparison> comparisons;
        unsigned lhsWidth;
    };

    std::map<Decl const *, Comparisons> comparisons_;
};

unsigned LoopVarTooSmall::getIntValueWidth(QualType type) const {
    if (auto const et = type->getAs<EnumType>()) {
        auto const ed = et->getDecl();
        if (!ed->isFixed()) {
            unsigned pos = ed->getNumPositiveBits();
            unsigned neg = ed->getNumNegativeBits();
            return neg == 0 ? std::max(pos, 1U) : std::max(pos + 1, neg);
        }
    }
    return compiler.getASTContext().getIntWidth(type);
}

void LoopVarTooSmall::checkSubExpr(Expr const * expr, bool positive) {
    auto const e = expr->IgnoreImplicit()->IgnoreParenImpCasts();
    if (auto const uo = dyn_cast<UnaryOperator>(e)) {
        if (uo->getOpcode() == UO_LNot) {
            checkSubExpr(uo->getSubExpr(), !positive);
        }
        return;
    }
    const BinaryOperator* binOp = dyn_cast<BinaryOperator>(e);
    if (!binOp)
        return;
    bool less;
    if (positive) {
        switch (binOp->getOpcode()) {
        case BO_LAnd:
            checkSubExpr(binOp->getLHS(), true);
            checkSubExpr(binOp->getRHS(), true);
            return;
        case BO_LT:
        case BO_NE:
            less = true;
            break;
        case BO_LE:
            less = false;
            break;
        default:
            return;
        }
    } else {
        switch (binOp->getOpcode()) {
        case BO_LOr:
            checkSubExpr(binOp->getLHS(), false);
            checkSubExpr(binOp->getRHS(), false);
            return;
        case BO_GE:
        case BO_EQ:
            less = true;
            break;
        case BO_GT:
            less = false;
            break;
        default:
            return;
        }
    }
    auto lhs = dyn_cast<DeclRefExpr>(binOp->getLHS()->IgnoreParenImpCasts());
    if (!lhs)
        return;
    QualType qt = lhs->getType();
    if (!qt->isIntegralOrEnumerationType())
        return;
    unsigned qt1BitWidth = getIntValueWidth(qt);
    auto lhsDecl = lhs->getDecl();
    if (!isa<VarDecl>(lhsDecl)) {
        if (auto fd = dyn_cast<FieldDecl>(lhsDecl)) {
            if (fd->isBitField()) {
                qt1BitWidth = std::max(
                    qt1BitWidth,
                    fd->getBitWidthValue(compiler.getASTContext()));
            }
        } else {
            return;
        }
    }
    const Expr* binOpRHS = binOp->getRHS()->IgnoreParenImpCasts();
    QualType qt2 = binOpRHS->getType();
    if (!qt2->isIntegralType(compiler.getASTContext()))
        return;
    unsigned qt2BitWidth;
    llvm::APSInt aIntResult;
    // Work around missing Clang 3.9 fix <https://reviews.llvm.org/rL271762>
    // "Sema: do not attempt to sizeof a dependent type", causing Clang 3.8 to
    // crash during EvaluateAsInt() on expressions of the form
    //
    //   sizeof (T)
    //
    // with dependent type T:
    if (!binOpRHS->isValueDependent()
        && compat::EvaluateAsInt(binOpRHS, aIntResult, compiler.getASTContext()))
    {
        if (less && aIntResult.isStrictlyPositive()) {
            --aIntResult;
        }
        qt2BitWidth = aIntResult.isUnsigned() || !aIntResult.isNegative()
            ? std::max(aIntResult.getActiveBits(), 1U)
            : aIntResult.getBitWidth() - aIntResult.countLeadingOnes() + 1;
    } else {
        // Ignore complex expressions for now, promotion rules on conditions
        // like "i < (size()+1)" make it hard to guess at a correct type:
        if (isa<BinaryOperator>(binOpRHS) || isa<ConditionalOperator>(binOpRHS))
        {
            return;
        }
        qt2BitWidth = getIntValueWidth(qt2);
        if (auto dre = dyn_cast<DeclRefExpr>(binOpRHS)) {
            if (auto fd = dyn_cast<FieldDecl>(dre->getDecl())) {
                if (fd->isBitField()) {
                    qt2BitWidth = std::max(
                        qt2BitWidth,
                        fd->getBitWidthValue(compiler.getASTContext()));
                }
            }
        }
    }

    auto i = comparisons_.find(lhsDecl);
    if (i == comparisons_.end()) {
        i = (comparisons_.insert(
                 decltype(comparisons_)::value_type(lhsDecl, {{}, qt1BitWidth}))
             .first);
    } else {
        assert(i->second.lhsWidth == qt1BitWidth);
    }
    bool ins = true;
    for (auto j = i->second.comparisons.begin();
         j != i->second.comparisons.end();)
    {
        if (qt2BitWidth > j->rhsWidth) {
            ins = false;
            break;
        } else if (qt2BitWidth < j->rhsWidth) {
            j = i->second.comparisons.erase(j);
        } else {
            ++j;
        }
    }
    if (ins) {
        i->second.comparisons.push_back({binOp, qt2BitWidth});
    }
}

void LoopVarTooSmall::checkExpr(Expr const * expr) {
    if (expr != nullptr && !ignoreLocation(expr)) {
        assert(comparisons_.empty());
        checkSubExpr(expr, true);
        for (auto const & i: comparisons_) {
            for (auto const & j: i.second.comparisons) {
                if (i.second.lhsWidth < j.rhsWidth) {
                    report(
                        DiagnosticsEngine::Warning,
                        "loop index type %0 is narrower than length type %1",
                        j.op->getExprLoc())
                        << j.op->getLHS()->IgnoreImpCasts()->getType()
                        << j.op->getRHS()->IgnoreImpCasts()->getType()
                        << j.op->getSourceRange();
                }
            }
        }
        comparisons_.clear();
    }
}

loplugin::Plugin::Registration< LoopVarTooSmall > loopvartoosmall("loopvartoosmall");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
