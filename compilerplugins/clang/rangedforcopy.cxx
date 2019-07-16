
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
#include <iostream>

#include "plugin.hxx"
#include "clang/AST/CXXInheritance.h"

// Check that we're not unnecessarily copying variables in a range based for loop
// e.g. "for (OUString a: aList)" results in a copy of each string being made,
// whereas "for (const OUString& a: aList)" does not.

namespace
{

class RangedForCopy:
    public loplugin::FilteringPlugin<RangedForCopy>
{
public:
    explicit RangedForCopy(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    virtual void run() override {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXForRangeStmt( const CXXForRangeStmt* stmt );
};

bool RangedForCopy::VisitCXXForRangeStmt( const CXXForRangeStmt* stmt )
{
    if (ignoreLocation( stmt ))
        return true;

    const VarDecl* varDecl = stmt->getLoopVariable();
    if (!varDecl)
      return true;

    const QualType type = varDecl->getType();
    if (type->isRecordType() && !type->isReferenceType() && !type->isPointerType())
    {
        std::string name = type.getAsString();
        report(
               DiagnosticsEngine::Warning,
               "Loop variable passed by value, pass by reference instead, e.g. 'const %0&'",
               compat::getBeginLoc(varDecl))
               << name << varDecl->getSourceRange();
    }

    return true;
}


loplugin::Plugin::Registration< RangedForCopy > rangedforcopy("rangedforcopy");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
