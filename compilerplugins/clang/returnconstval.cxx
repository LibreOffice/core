/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <string>
#include <set>

#include "plugin.hxx"

/**
  Find code where we are return a const value type from a function.
  Which makes no sense.
  Either we should return by non-const value, or by const ref.
  e.g.
    struct S2 {
        OUString mv1;
        const OUString get13() { return mv1; }
    }

  Specifically, this code pattern will prevent copy/move optimisations.
*/

namespace
{
class ReturnConstVal : public loplugin::FilteringPlugin<ReturnConstVal>
{
public:
    explicit ReturnConstVal(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(const FunctionDecl* decl);
};

bool ReturnConstVal::VisitFunctionDecl(const FunctionDecl* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;
    if (!functionDecl->hasBody())
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl))
        return true;
    QualType t1{ functionDecl->getReturnType() };
    if (!t1.isConstQualified())
        return true;
    if (t1->isReferenceType())
        return true;
    report(DiagnosticsEngine::Warning, "either return non-const, or by const ref",
           functionDecl->getSourceRange().getBegin())
        << functionDecl->getSourceRange();

    // display the location of the class member declaration so I don't have to search for it by hand
    auto canonicalDecl = functionDecl->getCanonicalDecl();
    if (canonicalDecl != functionDecl)
    {
        report(DiagnosticsEngine::Note, "either return non-const, or by const ref",
               canonicalDecl->getSourceRange().getBegin())
            << canonicalDecl->getSourceRange();
    }

    return true;
}

loplugin::Plugin::Registration<ReturnConstVal> returnconstval("returnconstval");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
