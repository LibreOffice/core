/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include <clang/AST/CXXInheritance.h>
#include "plugin.hxx"
#include "check.hxx"

/**
  When used in "for" loops, css::uno::Sequence objects tend to end up calling the non-const begin()/end(),
  which is considerably more expensive than the const variants because it forces a local copy
  of the internal ref-counted impl object.
*/

namespace
{
class SequenceLoop : public loplugin::FilteringPlugin<SequenceLoop>
{
public:
    explicit SequenceLoop(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXForRangeStmt(CXXForRangeStmt const*);
};

bool SequenceLoop::VisitCXXForRangeStmt(CXXForRangeStmt const* forStmt)
{
    if (ignoreLocation(forStmt))
        return true;

    auto tc = loplugin::TypeCheck(forStmt->getRangeInit()->getType());
    if (tc.Const())
        return true;
    if (!tc.Class("Sequence")
             .Namespace("uno")
             .Namespace("star")
             .Namespace("sun")
             .Namespace("com")
             .GlobalNamespace()
        // B2DPolyPolygon is similar in that accessing the non-const begin()/end() methods
        // might trigger unnecessary copying
        && !tc.Class("B2DPolyPolygon").Namespace("basegfx").GlobalNamespace())
        return true;
    const VarDecl* varDecl = forStmt->getLoopVariable();
    auto tc2 = loplugin::TypeCheck(varDecl->getType());
    if (!tc2.LvalueReference().Const())
        return true;

    report(DiagnosticsEngine::Warning,
           ("use std::as_const, or otherwise make the for-range-initializer expression const, to"
            " avoid creating a copy of the Sequence"),
           compat::getBeginLoc(forStmt->getRangeInit()))
        << forStmt->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<SequenceLoop> sequenceloop("sequenceloop");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
