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

class RemoveForwardStringDecl
    : public loplugin::FilteringRewritePlugin< RemoveForwardStringDecl >
    {
    public:
        explicit RemoveForwardStringDecl( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run() override;
        bool VisitNamespaceDecl( const NamespaceDecl* declaration );
    private:
        bool tryRemoveStringForwardDecl( const Decl* decl );
    };

} // namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
