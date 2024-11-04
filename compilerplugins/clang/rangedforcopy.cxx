
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

#include "check.hxx"
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
    if (isa<DecompositionDecl>(varDecl))
    {
        // Assume that use of a non-reference structured binding is intentional:
        return true;
    }

    const QualType type = varDecl->getType();
    if (!type->isRecordType() || type->isReferenceType() || type->isPointerType())
        return true;

    if (auto exprWithCleanups = dyn_cast<ExprWithCleanups>(varDecl->getInit()))
        if (dyn_cast<CXXBindTemporaryExpr>(exprWithCleanups->getSubExpr()->IgnoreImpCasts()))
            return true;

    if (loplugin::TypeCheck(type).Class("__bit_const_reference").StdNamespace())
    {
        // With libc++ without _LIBCPP_ABI_BITSET_VECTOR_BOOL_CONST_SUBSCRIPT_RETURN_BOOL,
        // iterating over a const std::vector<bool> non-compliantly uses a variable of some
        // internal __bit_const_reference class type, rather than of type bool (see
        // <https://reviews.llvm.org/D123851> "[libc++] Change
        // vector<bool>::const_iterator::reference to bool in ABIv2"):
        return true;
    }

    // trivial class, ignore it
    if (loplugin::TypeCheck(type).Class("TypedWhichId").GlobalNamespace())
        return true;
    if (loplugin::TypeCheck(type).Typedef("OUStringChar").Namespace("rtl"))
        return true;

    std::string name = type.getAsString();
    report(
           DiagnosticsEngine::Warning,
           "Loop variable passed by value, pass by reference instead, e.g. 'const %0&'",
           varDecl->getBeginLoc())
           << name << varDecl->getSourceRange();

    return true;
}


loplugin::Plugin::Registration< RangedForCopy > rangedforcopy("rangedforcopy");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
