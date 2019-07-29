/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include "check.hxx"
#include "plugin.hxx"

/**
  Look for places we are using map[idx] in a bool context, which allocates an entry in the map, which is sometimes a side-effect we don't want.
*/

namespace
{
class MapIndex : public loplugin::FilteringPlugin<MapIndex>
{
public:
    explicit MapIndex(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitImplicitCastExpr(const ImplicitCastExpr*);
    bool VisitMemberExpr(const MemberExpr*);
};

bool MapIndex::VisitMemberExpr(const MemberExpr* memberExpr)
{
    if (ignoreLocation(memberExpr))
        return true;
    // operator bool conversion
    auto conversionDecl = dyn_cast<CXXConversionDecl>(memberExpr->getMemberDecl());
    if (!conversionDecl || !conversionDecl->getConversionType()->isBooleanType())
        return true;
    auto operatorCall = dyn_cast<CXXOperatorCallExpr>(memberExpr->getBase()->IgnoreCasts());
    if (!operatorCall)
        return true;
    if (operatorCall->getOperator() != OverloadedOperatorKind::OO_Subscript)
        return true;
    auto tc = loplugin::TypeCheck(operatorCall->getArg(0)->getType());
    if (!tc.Class("map") && !tc.Class("unordered_map"))
        return true;
    report(DiagnosticsEngine::Warning,
           "will create an empty entry in the map, you sure about that, rather use count()1",
           operatorCall->getExprLoc());
    return true;
}

bool MapIndex::VisitImplicitCastExpr(const ImplicitCastExpr* implicitCastExpr)
{
    if (ignoreLocation(implicitCastExpr))
    {
        return true;
    }

    // first cast is some kind of "ToBoolean" cast
    auto ck = implicitCastExpr->getCastKind();
    if (ck != CK_MemberPointerToBoolean && ck != CK_PointerToBoolean && ck != CK_IntegralToBoolean
        && ck != CK_FloatingToBoolean
        && ck != CK_FloatingComplexToBoolean && ck != CK_IntegralComplexToBoolean)
        return true;

    // second cast is LValueToRValue
    implicitCastExpr = dyn_cast<ImplicitCastExpr>(implicitCastExpr->getSubExpr());
    if (!implicitCastExpr)
        return true;

    if (implicitCastExpr->getCastKind() != CK_LValueToRValue)
        return true;
    auto operatorCall = dyn_cast<CXXOperatorCallExpr>(implicitCastExpr->getSubExpr());
    if (!operatorCall)
        return true;
    if (operatorCall->getOperator() != OverloadedOperatorKind::OO_Subscript)
        return true;
    auto tc = loplugin::TypeCheck(operatorCall->getArg(0)->getType());
    if (!tc.Class("map") && !tc.Class("unordered_map"))
        return true;
    report(DiagnosticsEngine::Warning,
           "will create an empty entry in the map, you sure about that, rather use count()2",
           implicitCastExpr->getExprLoc());
    return true;
}

loplugin::Plugin::Registration<MapIndex> X("mapindex");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
