/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "plugin.hxx"
#include <vector>

/** Look for appending result of adding OUString/OString to OUStringBuffer
 */
namespace
{
class StringBuffer : public loplugin::FilteringPlugin<StringBuffer>
{
public:
    explicit StringBuffer(loplugin::InstantiationData const& rData)
        : FilteringPlugin(rData)
    {
    }

    bool preRun() override
    {
        StringRef fn(handler.getMainFileName());
        return !loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/");
    }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const*);
};

bool StringBuffer::VisitCXXMemberCallExpr(CXXMemberCallExpr const* memberCallExpr)
{
    if (ignoreLocation(memberCallExpr))
        return true;
    if (!loplugin::DeclCheck(memberCallExpr->getRecordDecl())
             .Class("OUStringBuffer")
             .Namespace("rtl")
             .GlobalNamespace())
        return true;
    if (!memberCallExpr->getMethodDecl()->getIdentifier())
        return true;
    if (memberCallExpr->getMethodDecl()->getName() != "append")
        return true;
    auto matTemp = dyn_cast<MaterializeTemporaryExpr>(memberCallExpr->getArg(0));
    if (!matTemp)
        return true;
    if (!isa<CXXOperatorCallExpr>(matTemp->GetTemporaryExpr()))
        return true;
    report(DiagnosticsEngine::Warning,
           "appending added result of OUString to OUStringBuffer, rather do .append(x).append(y)",
           compat::getBeginLoc(memberCallExpr))
        << memberCallExpr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<StringBuffer> stringbuffer("stringbuffer", false);

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
