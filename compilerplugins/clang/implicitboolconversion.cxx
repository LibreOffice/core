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
#include <cstddef>
#include <iterator>
#include <stack>
#include <string>
#include <vector>

#include "compat.hxx"
#include "plugin.hxx"

template<> struct std::iterator_traits<ExprIterator> {
    typedef std::ptrdiff_t difference_type;
    typedef Expr * value_type;
    typedef Expr const ** pointer;
    typedef Expr const & reference;
    typedef std::random_access_iterator_tag iterator_category;
};

namespace {

bool isBool(Expr const * expr, bool allowTypedefs = true) {
    QualType t1 { expr->getType() };
    if (t1->isBooleanType()) {
        return true;
    }
    if (!allowTypedefs) {
        return false;
    }
// css::uno::Sequence<sal_Bool> s(1);s[0]=false /*unotools/source/config/configitem.cxx*/:
if(t1->isSpecificBuiltinType(BuiltinType::UChar))return true;
    TypedefType const * t2 = t1->getAs<TypedefType>();
    if (t2 == nullptr) {
        return false;
    }
    std::string name(t2->getDecl()->getNameAsString());
    return name == "sal_Bool" || name == "BOOL" || name == "FcBool"
        || name == "UBool" || name == "dbus_bool_t" || name == "gboolean"
        || name == "hb_bool_t";
}

bool isBoolExpr(Expr const * expr) {
    if (isBool(expr)) {
        return true;
    }
    ConditionalOperator const * co = dyn_cast<ConditionalOperator>(expr);
    if (co != nullptr) {
        ImplicitCastExpr const * ic1 = dyn_cast<ImplicitCastExpr>(
            co->getTrueExpr()->IgnoreParens());
        ImplicitCastExpr const * ic2 = dyn_cast<ImplicitCastExpr>(
            co->getFalseExpr()->IgnoreParens());
        if (ic1 != nullptr && ic2 != nullptr
            && ic1->getType()->isSpecificBuiltinType(BuiltinType::Int)
            && isBoolExpr(ic1->getSubExpr()->IgnoreParens())
            && ic2->getType()->isSpecificBuiltinType(BuiltinType::Int)
            && isBoolExpr(ic2->getSubExpr()->IgnoreParens()))
        {
            return true;
        }
    }
    return false;
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

class ImplicitBoolConversion:
    public RecursiveASTVisitor<ImplicitBoolConversion>, public loplugin::Plugin
{
public:
    explicit ImplicitBoolConversion(InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseCallExpr(CallExpr * expr);

    bool TraverseCStyleCastExpr(CStyleCastExpr * expr);

    bool TraverseCXXStaticCastExpr(CXXStaticCastExpr * expr);

    bool TraverseCXXFunctionalCastExpr(CXXFunctionalCastExpr * expr);

    bool TraverseConditionalOperator(ConditionalOperator * expr);

    bool TraverseBinLT(BinaryOperator * expr);

    bool TraverseBinLE(BinaryOperator * expr);

    bool TraverseBinGT(BinaryOperator * expr);

    bool TraverseBinGE(BinaryOperator * expr);

    bool TraverseBinEQ(BinaryOperator * expr);

    bool TraverseBinNE(BinaryOperator * expr);

    bool TraverseBinAssign(BinaryOperator * expr);

    bool TraverseBinAndAssign(CompoundAssignOperator * expr);

    bool TraverseBinOrAssign(CompoundAssignOperator * expr);

    bool TraverseBinXorAssign(CompoundAssignOperator * expr);

    bool TraverseReturnStmt(ReturnStmt * stmt);

    bool TraverseFunctionDecl(FunctionDecl * decl);

    bool VisitImplicitCastExpr(ImplicitCastExpr const * expr);

private:
    void reportWarning(ImplicitCastExpr const * expr);

    std::stack<std::vector<ImplicitCastExpr const *>> nested;
    bool externCIntFunctionDefinition = false;
};

bool ImplicitBoolConversion::TraverseCallExpr(CallExpr * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseCallExpr(expr);
    Decl const * d = expr->getCalleeDecl();
    bool ext = false;
    FunctionProtoType const * t = nullptr;
    if (d != nullptr) {
        FunctionDecl const * fd = dyn_cast<FunctionDecl>(d);
        if (fd != nullptr && fd->isExternC()) {
            ext = true;
            PointerType const * pt = dyn_cast<PointerType>(fd->getType());
            t = (pt == nullptr ? fd->getType() : pt->getPointeeType())
                ->getAs<FunctionProtoType>();
        } else {
            VarDecl const * vd = dyn_cast<VarDecl>(d);
            if (vd != nullptr && vd->isExternC())
            {
                ext = true;
                PointerType const * pt = dyn_cast<PointerType>(vd->getType());
                t = (pt == nullptr ? vd->getType() : pt->getPointeeType())
                    ->getAs<FunctionProtoType>();
            }
        }
    }
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (ext) {
            auto j = std::find_if(
                expr->arg_begin(), expr->arg_end(),
                [&i](Expr * e) { return i == e->IgnoreParens(); });
            if (j == expr->arg_end()) {
                reportWarning(i);
            } else {
                std::ptrdiff_t n = j - expr->arg_begin();
                assert(n >= 0);
                assert(
                    static_cast<std::size_t>(n) < compat::getNumParams(*t)
                    || t->isVariadic());
                if (static_cast<std::size_t>(n) < compat::getNumParams(*t)
                    && !(compat::getParamType(*t, n)->isSpecificBuiltinType(
                             BuiltinType::Int)
                         || (compat::getParamType(*t, n)->isSpecificBuiltinType(
                                 BuiltinType::UInt))))
                {
                    reportWarning(i);
                }
            }
        } else {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseCStyleCastExpr(CStyleCastExpr * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseCStyleCastExpr(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getSubExpr()->IgnoreParens()) {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseCXXStaticCastExpr(CXXStaticCastExpr * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseCXXStaticCastExpr(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getSubExpr()->IgnoreParens()) {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseCXXFunctionalCastExpr(
    CXXFunctionalCastExpr * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseCXXFunctionalCastExpr(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getSubExpr()->IgnoreParens()) {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseConditionalOperator(
    ConditionalOperator * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseConditionalOperator(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getTrueExpr()->IgnoreParens()
               && isBoolExpr(expr->getFalseExpr()->IgnoreParenImpCasts()))
              || (i == expr->getFalseExpr()->IgnoreParens()
                  && isBoolExpr(expr->getTrueExpr()->IgnoreParenImpCasts()))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseBinLT(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinLT(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
              || (i == expr->getRHS()->IgnoreParens()
                  && isBool(expr->getLHS()->IgnoreParenImpCasts(), false))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseBinLE(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinLE(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
              || (i == expr->getRHS()->IgnoreParens()
                  && isBool(expr->getLHS()->IgnoreParenImpCasts(), false))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseBinGT(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinGT(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
              || (i == expr->getRHS()->IgnoreParens()
                  && isBool(expr->getLHS()->IgnoreParenImpCasts(), false))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseBinGE(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinGE(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
              || (i == expr->getRHS()->IgnoreParens()
                  && isBool(expr->getLHS()->IgnoreParenImpCasts(), false))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseBinEQ(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinEQ(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
              || (i == expr->getRHS()->IgnoreParens()
                  && isBool(expr->getLHS()->IgnoreParenImpCasts(), false))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseBinNE(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinNE(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
              || (i == expr->getRHS()->IgnoreParens()
                  && isBool(expr->getLHS()->IgnoreParenImpCasts(), false))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

// /usr/include/gtk-2.0/gtk/gtktogglebutton.h: struct _GtkToggleButton:
//  guint GSEAL (active) : 1;
// even though <http://www.gtk.org/api/2.6/gtk/GtkToggleButton.html>:
//  "active"               gboolean              : Read / Write
bool ImplicitBoolConversion::TraverseBinAssign(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinAssign(expr);
    bool ext = false;
    MemberExpr const * me = dyn_cast<MemberExpr>(expr->getLHS());
    if (me != nullptr) {
        FieldDecl const * fd = dyn_cast<FieldDecl>(me->getMemberDecl());
        if (fd != nullptr && fd->isBitField()
            && fd->getBitWidthValue(compiler.getASTContext()) == 1)
        {
            TypedefType const * t = fd->getType()->getAs<TypedefType>();
            ext = t != nullptr && t->getDecl()->getNameAsString() == "guint";
        }
    }
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getRHS()->IgnoreParens() || !ext) {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseBinAndAssign(CompoundAssignOperator * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinAndAssign(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getRHS()->IgnoreParens()
            || !isBool(expr->getLHS()->IgnoreParens(), false))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    if (!ignoreLocation(expr) && isBool(expr->getLHS(), false)
        && !isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
    {
        report(
            DiagnosticsEngine::Warning, "mix of %0 and %1 in operator &=",
            expr->getRHS()->getLocStart())
            << expr->getLHS()->getType()
            << expr->getRHS()->IgnoreParenImpCasts()->getType()
            << expr->getSourceRange();
    }
    return ret;
}

bool ImplicitBoolConversion::TraverseBinOrAssign(CompoundAssignOperator * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinOrAssign(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getRHS()->IgnoreParens()
            || !isBool(expr->getLHS()->IgnoreParens(), false))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    if (!ignoreLocation(expr) && isBool(expr->getLHS(), false)
        && !isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
    {
        report(
            DiagnosticsEngine::Warning, "mix of %0 and %1 in operator |=",
            expr->getRHS()->getLocStart())
            << expr->getLHS()->getType()
            << expr->getRHS()->IgnoreParenImpCasts()->getType()
            << expr->getSourceRange();
    }
    return ret;
}

bool ImplicitBoolConversion::TraverseBinXorAssign(CompoundAssignOperator * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseBinXorAssign(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getRHS()->IgnoreParens()
            || !isBool(expr->getLHS()->IgnoreParens(), false))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    if (!ignoreLocation(expr) && isBool(expr->getLHS(), false)
        && !isBool(expr->getRHS()->IgnoreParenImpCasts(), false))
    {
        report(
            DiagnosticsEngine::Warning, "mix of %0 and %1 in operator ^=",
            expr->getRHS()->getLocStart())
            << expr->getLHS()->getType()
            << expr->getRHS()->IgnoreParenImpCasts()->getType()
            << expr->getSourceRange();
    }
    return ret;
}

bool ImplicitBoolConversion::TraverseReturnStmt(ReturnStmt * stmt) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool ret = RecursiveASTVisitor::TraverseReturnStmt(stmt);
    Expr const * expr = stmt->getRetValue();
    if (expr != nullptr) {
        ExprWithCleanups const * ec = dyn_cast<ExprWithCleanups>(expr);
        if (ec != nullptr) {
            expr = ec->getSubExpr();
        }
        expr = expr->IgnoreParens();
    }
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr || !externCIntFunctionDefinition) {
            reportWarning(i);
        }
    }
    nested.pop();
    return ret;
}

bool ImplicitBoolConversion::TraverseFunctionDecl(FunctionDecl * decl) {
    bool ext = false;
    if (hasCLanguageLinkageType(decl) && decl->isThisDeclarationADefinition()) {
        QualType t { compat::getReturnType(*decl) };
        if (t->isSpecificBuiltinType(BuiltinType::Int)
            || t->isSpecificBuiltinType(BuiltinType::UInt))
        {
            ext = true;
        } else {
            TypedefType const * t2 = t->getAs<TypedefType>();
            // cf. rtl_locale_equals (and sal_Int32 can be long):
            if (t2 != nullptr
                && t2->getDecl()->getNameAsString() == "sal_Int32")
            {
                ext = true;
            }
        }
    }
    if (ext) {
        assert(!externCIntFunctionDefinition);
        externCIntFunctionDefinition = true;
    }
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(decl);
    if (ext) {
        externCIntFunctionDefinition = false;
    }
    return ret;
}

bool ImplicitBoolConversion::VisitImplicitCastExpr(
    ImplicitCastExpr const * expr)
{
    if (ignoreLocation(expr)) {
        return true;
    }
    if (expr->getSubExprAsWritten()->getType()->isBooleanType()
        && !isBool(expr))
    {
        if (nested.empty()) {
            reportWarning(expr);
        } else {
            nested.top().push_back(expr);
        }
    } else {
        ExplicitCastExpr const * sub = dyn_cast<ExplicitCastExpr>(
            expr->getSubExpr()->IgnoreParenImpCasts());
        if (sub != nullptr
            && (sub->getSubExpr()->IgnoreParenImpCasts()->getType().IgnoreParens()
                == expr->getType().IgnoreParens())
            && isBool(sub->getSubExpr()->IgnoreParenImpCasts()))
        {
            report(
                DiagnosticsEngine::Warning,
                "explicit conversion (%0) from %1 to %2 implicitly cast back to %3",
                expr->getLocStart())
                << sub->getCastKindName()
                << sub->getSubExpr()->IgnoreParenImpCasts()->getType()
                << sub->getType() << expr->getType() << expr->getSourceRange();
        }
    }
    return true;
}

void ImplicitBoolConversion::reportWarning(ImplicitCastExpr const * expr) {
    report(
        DiagnosticsEngine::Warning,
        "implicit conversion (%0) from bool to %1", expr->getLocStart())
        << expr->getCastKindName() << expr->getType() << expr->getSourceRange();
}

loplugin::Plugin::Registration<ImplicitBoolConversion> X(
    "implicitboolconversion");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
