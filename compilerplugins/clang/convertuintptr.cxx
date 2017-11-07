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
#include "plugin.hxx"
#include "check.hxx"

/**
  plugin to help to when converting code from sal_uIntPtr to something more precise.
 */
namespace {

class ConvertUIntPtr:
    public RecursiveASTVisitor<ConvertUIntPtr>, public loplugin::Plugin
{
public:
    explicit ConvertUIntPtr(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        std::string fn( compiler.getSourceManager().getFileEntryForID(
                        compiler.getSourceManager().getMainFileID())->getName() );
        loplugin::normalizeDotDotInFilePath(fn);
        // using sal_uIntPtr as in-between type when converting void* to rtl_TextEncoding
        if (fn == SRCDIR "/sal/osl/unx/thread.cxx")
            return;
        // too much magic
        if (fn == SRCDIR "/sal/rtl/alloc_arena.cxx")
            return;
        if (fn == SRCDIR "/sal/rtl/alloc_cache.cxx")
            return;
        // TODO not sure what is going on here
        if (fn == SRCDIR "/tools/source/generic/bigint.cxx")
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const *);
private:
    bool isIntPtr(QualType qt);
};

bool ConvertUIntPtr::VisitImplicitCastExpr(ImplicitCastExpr const * castExpr)
{
    if (ignoreLocation(castExpr))
        return true;

    if (castExpr->getCastKind() == CK_LValueToRValue)
        return true;
    if (isa<IntegerLiteral>(castExpr->IgnoreCasts()))
        return true;
    // ignore literals like "-123"
    if (isa<UnaryOperator>(castExpr->IgnoreCasts()))
        return true;

    bool isSrcIntPtr = isIntPtr(castExpr->getSubExpr()->getType());
    bool isDestIntPtr = isIntPtr(castExpr->getType());

    if (!isSrcIntPtr && !isDestIntPtr)
        return true;

    // exclude casting between sal_uIntPtr <-> sal_IntPtr
    if (isSrcIntPtr && isDestIntPtr)
        return true;

    if (isSrcIntPtr && loplugin::TypeCheck(castExpr->getType()).AnyBoolean())
        return true;

    report(
        DiagnosticsEngine::Warning,
        "cast from %0 to %1",
        castExpr->getExprLoc())
      << castExpr->getSubExpr()->getType()
      << castExpr->getType()
      << castExpr->getSourceRange();

    return true;
}

bool ConvertUIntPtr::isIntPtr(QualType qt)
{
    auto tc = loplugin::TypeCheck(qt);
    if (!tc.Typedef())
        return false;
    TypedefType const * typedefType = qt->getAs<TypedefType>();
    auto name = typedefType->getDecl()->getName();
    return name == "sal_uIntPtr" || name == "sal_IntPtr";
}

loplugin::Plugin::Registration< ConvertUIntPtr > X("convertuintptr", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
