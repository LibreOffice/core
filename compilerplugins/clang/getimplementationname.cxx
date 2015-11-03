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

namespace {

clang::Expr const * ignoreParenImplicitComma(clang::Expr const * expr) {
    for (;;) {
        auto const e1 = expr->IgnoreParens()->IgnoreImplicit();
        if (e1 != expr) {
            expr = e1;
            continue;
        }
        // auto const e1 = dyn_cast<clang::ExprWithCleanups>(expr);
        // if (e1 != nullptr) {
        //     expr = e1->getSubExpr();
        //     continue;
        // }
        auto const e2 = dyn_cast<clang::BinaryOperator>(expr);
        if (e2 != nullptr && e2->getOpcode() == clang::BO_Comma) {
            expr = e2->getRHS();
            continue;
        }
        return expr;
    }
}

bool isPlainChar(clang::QualType type) {
    return type->isSpecificBuiltinType(clang::BuiltinType::Char_S)
        || type->isSpecificBuiltinType(clang::BuiltinType::Char_U);
}

bool overridesXServiceInfo(clang::CXXMethodDecl const * decl) {
    for (auto i = decl->begin_overridden_methods();
         i != decl->end_overridden_methods(); ++i)
    {
        if (((*i)->getParent()->getQualifiedNameAsString()
             == "com::sun::star::lang::XServiceInfo")
            || overridesXServiceInfo(*i))
        {
            return true;
        }
    }
    return false;
}

class GetImplementationName:
    public clang::RecursiveASTVisitor<GetImplementationName>,
    public loplugin::Plugin
{
public:
    explicit GetImplementationName(InstantiationData const & data): Plugin(data)
    {}

    void run() override;

    bool VisitCXXMethodDecl(clang::CXXMethodDecl const * decl);

private:
    bool isStringConstant(Expr const * expr, clang::StringRef * string);

    bool returnsStringConstant(
        FunctionDecl const * decl, clang::StringRef * string);
};

void GetImplementationName::run() {
    if (compiler.getLangOpts().CPlusPlus) {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
}

bool GetImplementationName::VisitCXXMethodDecl(
    clang::CXXMethodDecl const * decl)
{
    if (ignoreLocation(decl) || !decl->doesThisDeclarationHaveABody()
        || !decl->isVirtual())
    {
        return true;
    }
    auto const id = decl->getIdentifier();
    if (id == nullptr || id->getName() != "getImplementationName"
        || !overridesXServiceInfo(decl))
    {
        return true;
    }
    clang::StringRef str;
    if (!returnsStringConstant(decl, &str)) {
        report(
            clang::DiagnosticsEngine::Warning,
            "cannot determine returned string", decl->getLocation())
            << decl->getSourceRange();
        return true;
    }
    report(
        clang::DiagnosticsEngine::Warning, "\"%0\" implemented by %1",
        decl->getLocation())
        << str << decl->getParent()->getQualifiedNameAsString()
        << decl->getSourceRange();
    return true;
}

bool GetImplementationName::isStringConstant(
    Expr const * expr, clang::StringRef * string)
{
    QualType t = expr->getType();
    if (!(t->isConstantArrayType() && t.isConstQualified()
          && isPlainChar(t->getAsArrayTypeUnsafe()->getElementType())))
    {
        return false;
    }
    DeclRefExpr const * dre = dyn_cast<DeclRefExpr>(expr);
    if (dre != nullptr) {
        VarDecl const * var = dyn_cast<VarDecl>(dre->getDecl());
        if (var != nullptr) {
            Expr const * init = var->getAnyInitializer();
            if (init != nullptr) {
                expr = ignoreParenImplicitComma(init);
            }
        }
    }
    StringLiteral const * lit = dyn_cast<StringLiteral>(expr);
    if (lit != nullptr) {
        if (!lit->isAscii()) {
            return false;
        }
        *string = lit->getString();
        return true;
    }
    APValue v;
    if (!expr->isCXX11ConstantExpr(compiler.getASTContext(), &v)) {
        return false;
    }
    switch (v.getKind()) {
    case APValue::LValue:
        return false; //TODO
    case APValue::Array:
        {
            if (v.hasArrayFiller()) { //TODO: handle final NUL filler?
                return false;
            }
            unsigned n = v.getArraySize();
            assert(n != 0);
            for (unsigned i = 0; i != n; ++i) {
                APValue e(v.getArrayInitializedElt(i));
                if (!e.isInt()) { //TODO: assert?
                    return false;
                }
                APSInt iv = e.getInt();
                if (iv == 0) {
                    if (i == n -1) {
                        continue;
                    }
                    return false;
                } else if (iv.uge(0x80)) {
                    return false;
                }
                //TODO
            }
            return false;//TODO
        }
    default:
        assert(false); //TODO???
        return "BAD11";
    }
}

bool GetImplementationName::returnsStringConstant(
    FunctionDecl const * decl, clang::StringRef * string)
{
    auto s1 = decl->getBody();
    if (s1 == nullptr) {
        return false;
    }
    for (;;) {
        auto s2 = dyn_cast<clang::CompoundStmt>(s1);
        if (s2 == nullptr) {
            auto const s3 = dyn_cast<clang::CXXTryStmt>(s1);
            if (s3 == nullptr) {
                break;
            }
            s2 = s3->getTryBlock();
        }
        if (s2->size() != 1) {
            break;
        }
        s1 = s2->body_front();
    }
    auto const s4 = dyn_cast<clang::ReturnStmt>(s1);
    if (s4 == nullptr) {
        return false;
    }
    for (auto e1 = ignoreParenImplicitComma(s4->getRetValue());;) {
        auto const e2 = dyn_cast<clang::CallExpr>(e1);
        if (e2 != nullptr) {
            auto const d = e2->getDirectCallee();
            return d != nullptr && returnsStringConstant(d, string);
        }
        auto const e3 = dyn_cast<clang::CXXFunctionalCastExpr>(e1);
        if (e3 != nullptr) {
            e1 = ignoreParenImplicitComma(e3->getSubExpr());
            continue;
        }
        auto const e4 = dyn_cast<clang::CXXConstructExpr>(e1);
        if (e4 != nullptr)  {
            if (e4->getNumArgs() < 1) {
                return false;
            }
            e1 = ignoreParenImplicitComma(e4->getArg(0));
            continue;
        }
        return isStringConstant(e1, string);
    }
}

loplugin::Plugin::Registration<GetImplementationName> X(
    "getimplementationname");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
