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

class PostfixIncrementFix
    : public loplugin::FilteringRewritePlugin< PostfixIncrementFix >
    {
    public:
        explicit PostfixIncrementFix( const InstantiationData& data );
        virtual void run() override;
        bool VisitCXXOperatorCallExpr( const CXXOperatorCallExpr* op );
    private:
        void fixPostfixOperator( const CXXOperatorCallExpr* op );
        void fixPostfixOperators( const Stmt* stmt );
        bool canChangePostfixToPrefix( const Stmt* stmt, const CXXOperatorCallExpr* op );
        bool shouldDoChange( const Expr* op );
    };

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
