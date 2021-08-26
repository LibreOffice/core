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

class UnusedVariableCheck
    : public FilteringPlugin< UnusedVariableCheck >
    {
    public:
        explicit UnusedVariableCheck( const InstantiationData& data );
        virtual void run() override;
        bool VisitVarDecl( const VarDecl* var );
    };

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
