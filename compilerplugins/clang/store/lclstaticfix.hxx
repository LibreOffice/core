/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef LCLSTATICFIX_H
#define LCLSTATICFIX_H

#include "plugin.hxx"

namespace loplugin
{

class LclStaticFix
    : public RecursiveASTVisitor< LclStaticFix >
    , public RewritePlugin
    {
    public:
        explicit LclStaticFix( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run() override;
        bool VisitFunctionDecl( const FunctionDecl* declaration );
    };

} // namespace

#endif // POSTFIXINCREMENTFIX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
