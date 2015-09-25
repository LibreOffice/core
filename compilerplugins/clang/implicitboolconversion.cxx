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

#if __clang_major__ == 3 && __clang_minor__ < 7

template<> struct std::iterator_traits<ExprIterator> {
    typedef std::ptrdiff_t difference_type;
    typedef Expr * value_type;
    typedef Expr const ** pointer;
    typedef Expr const & reference;
    typedef std::random_access_iterator_tag iterator_category;
};

template<> struct std::iterator_traits<ConstExprIterator> {
    typedef std::ptrdiff_t difference_type;
    typedef Expr const * value_type;
    typedef Expr const ** pointer;
    typedef Expr const & reference;
    typedef std::random_access_iterator_tag iterator_category;
};

#endif

namespace {

Expr const * ignoreParenAndTemporaryMaterialization(Expr const * expr) {
    for (;;) {
        expr = expr->IgnoreParens();
        auto e = dyn_cast<MaterializeTemporaryExpr>(expr);
        if (e == nullptr) {
            return expr;
        }
        expr = e->GetTemporaryExpr();
    }
}

Expr const * ignoreParenImpCastAndComma(Expr const * expr) {
    for (;;) {
        expr = expr->IgnoreParenImpCasts();
        BinaryOperator const * op = dyn_cast<BinaryOperator>(expr);
        if (op == nullptr || op->getOpcode() != BO_Comma) {
            return expr;
        }
        expr = op->getRHS();
    }
}

SubstTemplateTypeParmType const * getAsSubstTemplateTypeParmType(QualType type)
{
    //TODO: unwrap all kinds of (non-SubstTemplateTypeParmType) sugar, not only
    // TypedefType sugar:
    for (;;) {
        TypedefType const * t = type->getAs<TypedefType>();
        if (t == nullptr) {
            return dyn_cast<SubstTemplateTypeParmType>(type);
        }
        type = t->desugar();
    }
}

bool areSameTypedef(QualType type1, QualType type2) {
    // type1.getTypePtr() == typ2.getTypePtr() fails for e.g. ::sal_Bool vs.
    // sal_Bool:
    auto t1 = type1->getAs<TypedefType>();
    auto t2 = type2->getAs<TypedefType>();
    return t1 != nullptr && t2 != nullptr && t1->getDecl() == t2->getDecl();
}

bool isBool(QualType type, bool allowTypedefs = true) {
    if (type->isBooleanType()) {
        return true;
    }
    if (!allowTypedefs) {
        return false;
    }
    TypedefType const * t2 = type->getAs<TypedefType>();
    if (t2 == nullptr) {
        return false;
    }
    std::string name(t2->getDecl()->getNameAsString());
    return name == "sal_Bool" || name == "BOOL" || name == "Boolean"
        || name == "FT_Bool" || name == "FcBool" || name == "GLboolean"
        || name == "NPBool" || name == "UBool" || name == "dbus_bool_t"
        || name == "gboolean" || name == "hb_bool_t" || name == "jboolean";
}

bool isBool(Expr const * expr, bool allowTypedefs = true) {
    return isBool(expr->getType(), allowTypedefs);
}

bool isMatchingBool(Expr const * expr, Expr const * comparisonExpr) {
    return isBool(expr, false)
        || areSameTypedef(expr->getType(), comparisonExpr->getType());
}

bool isBoolExpr(Expr const * expr) {
    if (isBool(expr)) {
        return true;
    }
    expr = ignoreParenImpCastAndComma(expr);
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
    std::stack<Expr const *> stack;
    Expr const * e = expr;
    for (;;) {
        e = ignoreParenImpCastAndComma(e);
        MemberExpr const * me = dyn_cast<MemberExpr>(e);
        if (me == nullptr) {
            break;
        }
        stack.push(e);
        e = me->getBase();
    }
    for (;;) {
        e = ignoreParenImpCastAndComma(e);
        CXXOperatorCallExpr const * op = dyn_cast<CXXOperatorCallExpr>(e);
        if (op == nullptr || op->getOperator() != OO_Subscript) {
            break;
        }
        stack.push(e);
        e = op->getArg(0);
    }
    if (!stack.empty()) {
        TemplateSpecializationType const * t
            = e->getType()->getAs<TemplateSpecializationType>();
        for (;;) {
            if (t == nullptr) {
                break;
            }
            QualType ty;
            MemberExpr const * me = dyn_cast<MemberExpr>(stack.top());
            if (me != nullptr) {
                TemplateDecl const * td
                    = t->getTemplateName().getAsTemplateDecl();
                if (td == nullptr) {
                    break;
                }
                TemplateParameterList const * ps = td->getTemplateParameters();
                SubstTemplateTypeParmType const * t2
                    = getAsSubstTemplateTypeParmType(
                        me->getMemberDecl()->getType());
                if (t2 == nullptr) {
                    break;
                }
                auto i = std::find(
                    ps->begin(), ps->end(),
                    t2->getReplacedParameter()->getDecl());
                if (i == ps->end()) {
                    break;
                }
                if (ps->size() != t->getNumArgs()) { //TODO
                    break;
                }
                TemplateArgument const & arg = t->getArg(i - ps->begin());
                if (arg.getKind() != TemplateArgument::Type) {
                    break;
                }
                ty = arg.getAsType();
            } else {
                CXXOperatorCallExpr const * op
                    = dyn_cast<CXXOperatorCallExpr>(stack.top());
                assert(op != nullptr);
                TemplateDecl const * d
                    = t->getTemplateName().getAsTemplateDecl();
                if (d == nullptr
                    || (d->getQualifiedNameAsString()
                        != "com::sun::star::uno::Sequence")
                    || t->getNumArgs() != 1
                    || t->getArg(0).getKind() != TemplateArgument::Type)
                {
                    break;
                }
                ty = t->getArg(0).getAsType();
            }
            stack.pop();
            if (stack.empty()) {
                if (isBool(ty)) {
                    return true;
                }
                break;
            }
            t = ty->getAs<TemplateSpecializationType>();
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

    bool TraverseCXXMemberCallExpr(CXXMemberCallExpr * expr);

    bool TraverseCXXConstructExpr(CXXConstructExpr * expr);

    bool TraverseCXXTemporaryObjectExpr(CXXTemporaryObjectExpr * expr);

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
    bool isExternCFunctionCall(
        CallExpr const * expr, FunctionProtoType const ** functionType);

    bool isExternCFunctionCallReturningInt(Expr const * expr);

    void checkCXXConstructExpr(CXXConstructExpr const * expr);

    void reportWarning(ImplicitCastExpr const * expr);

    std::stack<std::vector<ImplicitCastExpr const *>> nested;
    std::stack<CallExpr const *> calls;
    bool bExternCIntFunctionDefinition = false;
};

bool ImplicitBoolConversion::TraverseCallExpr(CallExpr * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    calls.push(expr);
    bool bRet = RecursiveASTVisitor::TraverseCallExpr(expr);
    FunctionProtoType const * t;
    bool bExt = isExternCFunctionCall(expr, &t);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        auto j = std::find_if(
            expr->arg_begin(), expr->arg_end(),
            [&i](Expr * e) {
                return i == ignoreParenAndTemporaryMaterialization(e);
            });
        if (j == expr->arg_end()) {
            reportWarning(i);
        } else {
            std::ptrdiff_t n = j - expr->arg_begin();
            assert(n >= 0);
            if (t != nullptr
                && static_cast<std::size_t>(n) >= compat::getNumParams(*t))
            {
                assert(t->isVariadic());
                // ignore bool to int promotions of variadic arguments
            } else if (bExt) {
                if (t != nullptr) {
                    assert(
                        static_cast<std::size_t>(n) < compat::getNumParams(*t));
                    if (!(compat::getParamType(*t, n)->isSpecificBuiltinType(
                              BuiltinType::Int)
                          || compat::getParamType(*t, n)->isSpecificBuiltinType(
                              BuiltinType::UInt)
                          || compat::getParamType(*t, n)->isSpecificBuiltinType(
                              BuiltinType::Long)))
                    {
                        reportWarning(i);
                    }
                } else {
                    reportWarning(i);
                }
            } else {
                // Filter out
                //
                //   template<typename T> void f(T);
                //   f<sal_Bool>(true);
                //
                DeclRefExpr const * dr = dyn_cast<DeclRefExpr>(
                    expr->getCallee()->IgnoreParenImpCasts());
                if (dr != nullptr && dr->hasExplicitTemplateArgs()) {
                    FunctionDecl const * fd
                        = dyn_cast<FunctionDecl>(dr->getDecl());
                    if (fd != nullptr
                        && static_cast<std::size_t>(n) < fd->getNumParams())
                    {
                        SubstTemplateTypeParmType const * t2
                            = getAsSubstTemplateTypeParmType(
                                fd->getParamDecl(n)->getType()
                                .getNonReferenceType());
                        if (t2 != nullptr) {
                            //TODO: fix this superficial nonsense check:
                            ASTTemplateArgumentListInfo const & ai
                                = dr->getExplicitTemplateArgs();
                            if (ai.NumTemplateArgs == 1
                                && (ai[0].getArgument().getKind()
                                    == TemplateArgument::Type)
                                && isBool(ai[0].getTypeSourceInfo()->getType()))
                            {
                                continue;
                            }
                        }
                    }
                }
                reportWarning(i);
            }
        }
    }
    calls.pop();
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseCXXMemberCallExpr(CXXMemberCallExpr * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseCXXMemberCallExpr(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        auto j = std::find_if(
            expr->arg_begin(), expr->arg_end(),
            [&i](Expr * e) {
                return i == ignoreParenAndTemporaryMaterialization(e);
            });
        if (j != expr->arg_end()) {
            // Filter out
            //
            //  template<typename T> struct S { void f(T); };
            //  S<sal_Bool> s;
            //  s.f(true);
            //
            std::ptrdiff_t n = j - expr->arg_begin();
            assert(n >= 0);
            CXXMethodDecl const * d = expr->getMethodDecl();
            if (static_cast<std::size_t>(n) >= d->getNumParams()) {
                // Ignore bool to int promotions of variadic arguments:
                assert(d->isVariadic());
                continue;
            }
            QualType ty
                = ignoreParenImpCastAndComma(expr->getImplicitObjectArgument())
                ->getType();
            if (dyn_cast<MemberExpr>(expr->getCallee())->isArrow()) {
                ty = ty->getAs<PointerType>()->getPointeeType();
            }
            TemplateSpecializationType const * ct
                = ty->getAs<TemplateSpecializationType>();
            if (ct != nullptr) {
                SubstTemplateTypeParmType const * pt
                    = getAsSubstTemplateTypeParmType(
                        d->getParamDecl(n)->getType().getNonReferenceType());
                if (pt != nullptr) {
                    TemplateDecl const * td
                        = ct->getTemplateName().getAsTemplateDecl();
                    if (td != nullptr) {
                        //TODO: fix this superficial nonsense check:
                        if (ct->getNumArgs() >= 1
                            && ct->getArg(0).getKind() == TemplateArgument::Type
                            && isBool(ct->getArg(0).getAsType()))
                        {
                            continue;
                        }
                    }
                }
            }
        }
        reportWarning(i);
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseCXXConstructExpr(CXXConstructExpr * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseCXXConstructExpr(expr);
    checkCXXConstructExpr(expr);
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseCXXTemporaryObjectExpr(
    CXXTemporaryObjectExpr * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseCXXTemporaryObjectExpr(expr);
    checkCXXConstructExpr(expr);
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseCStyleCastExpr(CStyleCastExpr * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseCStyleCastExpr(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getSubExpr()->IgnoreParens()) {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseCXXStaticCastExpr(CXXStaticCastExpr * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseCXXStaticCastExpr(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getSubExpr()->IgnoreParens()) {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseCXXFunctionalCastExpr(
    CXXFunctionalCastExpr * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseCXXFunctionalCastExpr(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getSubExpr()->IgnoreParens()) {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseConditionalOperator(
    ConditionalOperator * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseConditionalOperator(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getTrueExpr()->IgnoreParens()
               && (isBoolExpr(expr->getFalseExpr()->IgnoreParenImpCasts())
                   || isExternCFunctionCallReturningInt(expr->getFalseExpr())))
              || (i == expr->getFalseExpr()->IgnoreParens()
                  && (isBoolExpr(expr->getTrueExpr()->IgnoreParenImpCasts())
                      || isExternCFunctionCallReturningInt(
                          expr->getTrueExpr())))
              || (!compiler.getLangOpts().CPlusPlus
                  && i == expr->getCond()->IgnoreParens())))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinLT(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinLT(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isMatchingBool(
                   expr->getRHS()->IgnoreImpCasts(), i->getSubExprAsWritten()))
              || (i == expr->getRHS()->IgnoreParens()
                  && isMatchingBool(
                      expr->getLHS()->IgnoreImpCasts(),
                      i->getSubExprAsWritten()))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinLE(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinLE(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isMatchingBool(
                   expr->getRHS()->IgnoreImpCasts(), i->getSubExprAsWritten()))
              || (i == expr->getRHS()->IgnoreParens()
                  && isMatchingBool(
                      expr->getLHS()->IgnoreImpCasts(),
                      i->getSubExprAsWritten()))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinGT(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinGT(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isMatchingBool(
                   expr->getRHS()->IgnoreImpCasts(), i->getSubExprAsWritten()))
              || (i == expr->getRHS()->IgnoreParens()
                  && isMatchingBool(
                      expr->getLHS()->IgnoreImpCasts(),
                      i->getSubExprAsWritten()))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinGE(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinGE(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isMatchingBool(
                   expr->getRHS()->IgnoreImpCasts(), i->getSubExprAsWritten()))
              || (i == expr->getRHS()->IgnoreParens()
                  && isMatchingBool(
                      expr->getLHS()->IgnoreImpCasts(),
                      i->getSubExprAsWritten()))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinEQ(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinEQ(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isMatchingBool(
                   expr->getRHS()->IgnoreImpCasts(), i->getSubExprAsWritten()))
              || (i == expr->getRHS()->IgnoreParens()
                  && isMatchingBool(
                      expr->getLHS()->IgnoreImpCasts(),
                      i->getSubExprAsWritten()))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinNE(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinNE(expr);
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (!((i == expr->getLHS()->IgnoreParens()
               && isMatchingBool(
                   expr->getRHS()->IgnoreImpCasts(), i->getSubExprAsWritten()))
              || (i == expr->getRHS()->IgnoreParens()
                  && isMatchingBool(
                      expr->getLHS()->IgnoreImpCasts(),
                      i->getSubExprAsWritten()))))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinAssign(BinaryOperator * expr) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinAssign(expr);
    // /usr/include/gtk-2.0/gtk/gtktogglebutton.h: struct _GtkToggleButton:
    //  guint GSEAL (active) : 1;
    // even though <http://www.gtk.org/api/2.6/gtk/GtkToggleButton.html>:
    //  "active"               gboolean              : Read / Write
    bool bExt = false;
    MemberExpr const * me = dyn_cast<MemberExpr>(expr->getLHS());
    if (me != nullptr) {
        FieldDecl const * fd = dyn_cast<FieldDecl>(me->getMemberDecl());
        if (fd != nullptr && fd->isBitField()
            && fd->getBitWidthValue(compiler.getASTContext()) == 1)
        {
            TypedefType const * t = fd->getType()->getAs<TypedefType>();
            bExt = t != nullptr && t->getDecl()->getNameAsString() == "guint";
        }
    }
    assert(!nested.empty());
    for (auto i: nested.top()) {
        if (i != expr->getRHS()->IgnoreParens()
            || !(bExt || isBoolExpr(expr->getLHS())))
        {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinAndAssign(CompoundAssignOperator * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinAndAssign(expr);
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
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinOrAssign(CompoundAssignOperator * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinOrAssign(expr);
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
    return bRet;
}

bool ImplicitBoolConversion::TraverseBinXorAssign(CompoundAssignOperator * expr)
{
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseBinXorAssign(expr);
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
    return bRet;
}

bool ImplicitBoolConversion::TraverseReturnStmt(ReturnStmt * stmt) {
    nested.push(std::vector<ImplicitCastExpr const *>());
    bool bRet = RecursiveASTVisitor::TraverseReturnStmt(stmt);
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
        if (i != expr || !bExternCIntFunctionDefinition) {
            reportWarning(i);
        }
    }
    nested.pop();
    return bRet;
}

bool ImplicitBoolConversion::TraverseFunctionDecl(FunctionDecl * decl) {
    bool bExt = false;
    if (hasCLanguageLinkageType(decl) && decl->isThisDeclarationADefinition()) {
        QualType t { compat::getReturnType(*decl) };
        if (t->isSpecificBuiltinType(BuiltinType::Int)
            || t->isSpecificBuiltinType(BuiltinType::UInt))
        {
            bExt = true;
        } else {
            TypedefType const * t2 = t->getAs<TypedefType>();
            // cf. rtl_locale_equals (and sal_Int32 can be long):
            if (t2 != nullptr
                && t2->getDecl()->getNameAsString() == "sal_Int32")
            {
                bExt = true;
            }
        }
    }
    if (bExt) {
        assert(!bExternCIntFunctionDefinition);
        bExternCIntFunctionDefinition = true;
    }
    bool bRet = RecursiveASTVisitor::TraverseFunctionDecl(decl);
    if (bExt) {
        bExternCIntFunctionDefinition = false;
    }
    return bRet;
}

bool ImplicitBoolConversion::VisitImplicitCastExpr(
    ImplicitCastExpr const * expr)
{
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isBool(expr->getSubExprAsWritten()) && !isBool(expr)) {
        // Ignore NoOp from 'sal_Bool' (aka 'unsigned char') to 'const unsigned
        // char' in makeAny(b) with b of type sal_Bool:
        if (expr->getCastKind() != CK_NoOp) {
            if (nested.empty()) {
                reportWarning(expr);
            } else {
                nested.top().push_back(expr);
            }
        }
        return true;
    }
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
        return true;
    }
    if (expr->getType()->isBooleanType() && !isBoolExpr(expr->getSubExpr())
        && !calls.empty())
    {
        CallExpr const * call = calls.top();
        if (std::find_if(
                call->arg_begin(), call->arg_end(),
                [expr](Expr const * e) { return expr == e->IgnoreParens(); })
            != call->arg_end())
        {
            report(
                DiagnosticsEngine::Warning,
                "implicit conversion (%0) of call argument from %1 to %2",
                expr->getLocStart())
                << expr->getCastKindName() << expr->getSubExpr()->getType()
                << expr->getType() << expr->getSourceRange();
            return true;
        }
    }
    return true;
}

bool ImplicitBoolConversion::isExternCFunctionCall(
    CallExpr const * expr, FunctionProtoType const ** functionType)
{
    assert(functionType != nullptr);
    *functionType = nullptr;
    Decl const * d = expr->getCalleeDecl();
    if (d != nullptr) {
        FunctionDecl const * fd = dyn_cast<FunctionDecl>(d);
        if (fd != nullptr) {
            PointerType const * pt = fd->getType()->getAs<PointerType>();
            QualType t2(pt == nullptr ? fd->getType() : pt->getPointeeType());
            *functionType = t2->getAs<FunctionProtoType>();
            assert(
                *functionType != nullptr || !compiler.getLangOpts().CPlusPlus
                || (fd->getBuiltinID() != Builtin::NotBuiltin
                    && isa<FunctionNoProtoType>(t2)));
                // __builtin_*s have no proto type?
            return fd->isExternC()
                || compiler.getSourceManager().isInExternCSystemHeader(
                    fd->getLocation());
        }
        VarDecl const * vd = dyn_cast<VarDecl>(d);
        if (vd != nullptr) {
            PointerType const * pt = vd->getType()->getAs<PointerType>();
            *functionType
                = ((pt == nullptr ? vd->getType() : pt->getPointeeType())
                   ->getAs<FunctionProtoType>());
            return vd->isExternC();
        }
    }
    return false;
}

bool ImplicitBoolConversion::isExternCFunctionCallReturningInt(
    Expr const * expr)
{
    CallExpr const * e = dyn_cast<CallExpr>(expr->IgnoreParenImpCasts());
    FunctionProtoType const * t;
    return e != nullptr && e->getType()->isSpecificBuiltinType(BuiltinType::Int)
        && isExternCFunctionCall(e, &t);
}

void ImplicitBoolConversion::checkCXXConstructExpr(
    CXXConstructExpr const * expr)
{
    assert(!nested.empty());
    for (auto i: nested.top()) {
        auto j = std::find_if(
            expr->arg_begin(), expr->arg_end(),
            [&i](Expr const * e) {
                return i == ignoreParenAndTemporaryMaterialization(e);
            });
        if (j != expr->arg_end()) {
            TemplateSpecializationType const * t1 = expr->getType()->
                getAs<TemplateSpecializationType>();
            SubstTemplateTypeParmType const * t2 = nullptr;
            CXXConstructorDecl const * d = expr->getConstructor();
            if (d->getNumParams() == expr->getNumArgs()) { //TODO: better check
                t2 = getAsSubstTemplateTypeParmType(
                    d->getParamDecl(j - expr->arg_begin())->getType()
                    .getNonReferenceType());
            }
            if (t1 != nullptr && t2 != nullptr) {
                TemplateDecl const * td
                    = t1->getTemplateName().getAsTemplateDecl();
                if (td != nullptr) {
                    TemplateParameterList const * ps
                        = td->getTemplateParameters();
                    auto i = std::find(
                        ps->begin(), ps->end(),
                        t2->getReplacedParameter()->getDecl());
                    if (i != ps->end()) {
                        if (ps->size() == t1->getNumArgs()) { //TODO
                            TemplateArgument const & arg = t1->getArg(
                                i - ps->begin());
                            if (arg.getKind() == TemplateArgument::Type
                                && isBool(arg.getAsType()))
                            {
                                continue;
                            }
                        }
                    }
                }
            }
        }
        reportWarning(i);
    }
}

void ImplicitBoolConversion::reportWarning(ImplicitCastExpr const * expr) {
    if (!compiler.getLangOpts().ObjC2 || compiler.getLangOpts().CPlusPlus) {
        report(
            DiagnosticsEngine::Warning,
            "implicit conversion (%0) from %1 to %2", expr->getLocStart())
            << expr->getCastKindName() << expr->getSubExprAsWritten()->getType()
            << expr->getType() << expr->getSourceRange();
    }
}

loplugin::Plugin::Registration<ImplicitBoolConversion> X(
    "implicitboolconversion");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
