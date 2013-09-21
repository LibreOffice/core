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

#ifndef UNUSEDVARIABLECHECK_H
#define UNUSEDVARIABLECHECK_H

#include "plugin.hxx"

namespace loplugin
{

class UnusedVariableCheck
    : public RecursiveASTVisitor< UnusedVariableCheck >
    , public Plugin
    {
    public:
        explicit UnusedVariableCheck( CompilerInstance& compiler );
        virtual void run() override;
        bool VisitVarDecl( const VarDecl* var );
    };

} // namespace

#endif // UNUSEDVARIABLECHECK_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
