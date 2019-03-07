/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

/**
 * Look for places where we are converting from type A through a conversion operator and back to type A,
 * which is redundant. At the moment only look for Color, to aid my ColorData->Color conversion
 */
namespace
{
class DoubleConvert final : public loplugin::FilteringPlugin<DoubleConvert>
{
public:
    explicit DoubleConvert(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }
    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitMaterializeTemporaryExpr(MaterializeTemporaryExpr const*);
};

/**
 The AST looks like:

 CXXOperatorCallExpr 0x8e5b840 'class Color' lvalue
|-ImplicitCastExpr 0x8e5b828 'class Color &(*)(class Color &&) noexcept' <FunctionToPointerDecay>
| `-DeclRefExpr 0x8e5b800 'class Color &(class Color &&) noexcept' lvalue CXXMethod 0x8e59a08 'operator=' 'class Color &(class Color &&) noexcept'
|-DeclRefExpr 0x8e5b678 'class Color' lvalue Var 0x8e5b5d0 'col2' 'class Color'
`-MaterializeTemporaryExpr 0x8e5b7e8 'class Color' xvalue
  `-CXXConstructExpr 0x8e5b7b0 'class Color' 'void (ColorData)'
    `-ImplicitCastExpr 0x8e5b798 'ColorData':'unsigned int' <IntegralCast>
      `-CXXFunctionalCastExpr 0x8e5b770 'sal_Int32':'int' functional cast to sal_Int32 <NoOp>
        `-ImplicitCastExpr 0x8e5b758 'sal_Int32':'int' <UserDefinedConversion>
          `-CXXMemberCallExpr 0x8e5b730 'sal_Int32':'int'
            `-MemberExpr 0x8e5b6f8 '<bound member function type>' .operator int 0x8e51048
              `-ImplicitCastExpr 0x8e5b6e0 'const class Color' lvalue <NoOp>
                `-DeclRefExpr 0x8e5b6b0 'class Color' lvalue Var 0x8e5b518 'col1' 'class Color'
*/
bool DoubleConvert::VisitMaterializeTemporaryExpr(MaterializeTemporaryExpr const* materializetemp)
{
    if (ignoreLocation(materializetemp))
        return true;
    auto cxxConstruct
        = dyn_cast<CXXConstructExpr>(materializetemp->GetTemporaryExpr()->IgnoreParenCasts());
    if (!cxxConstruct)
        return true;
    if (cxxConstruct->getNumArgs() == 0)
        return true;
    auto cxxMemberCallExpr
        = dyn_cast<CXXMemberCallExpr>(cxxConstruct->getArg(0)->IgnoreParenCasts());
    if (!cxxMemberCallExpr)
        return true;
    if (!isa<CXXConversionDecl>(cxxMemberCallExpr->getMethodDecl()))
        return true;
    if (materializetemp->getType().getCanonicalType().getTypePtr()
        != cxxMemberCallExpr->getImplicitObjectArgument()
               ->getType()
               .getCanonicalType()
               .getTypePtr())
        return true;
    if (!loplugin::TypeCheck(materializetemp->getType().getCanonicalType())
             .Class("Color")
             .GlobalNamespace())
        return true;

    report(DiagnosticsEngine::Warning, "redundant double conversion", materializetemp->getExprLoc())
        << materializetemp->getSourceRange();
    return true;
}

static loplugin::Plugin::Registration<DoubleConvert> doubleconvert("doubleconvert");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
