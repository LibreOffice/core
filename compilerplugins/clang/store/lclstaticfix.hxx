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

#pragma once

#include "plugin.hxx"

namespace loplugin
{

class LclStaticFix
    : public loplugin::FilteringRewritePlugin< LclStaticFix >
    {
    public:
        explicit LclStaticFix( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run() override;
        bool VisitFunctionDecl( const FunctionDecl* declaration );
    };

} // namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
