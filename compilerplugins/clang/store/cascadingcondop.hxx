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

#ifndef CASCADINGCONDOP_H
#define CASCADINGCONDOP_H

#include "plugin.hxx"

namespace loplugin
{

struct WalkCounter;

// The class implementing the plugin action.
class CascadingCondOp
    // Inherits from the Clang class that will allow examing the Clang AST tree (i.e. syntax tree).
    : public RecursiveASTVisitor< CascadingCondOp >
    // And the base class for LO Clang plugins.
    , public Plugin
    {
    public:
        CascadingCondOp( const InstantiationData& data );
        virtual void run() override;
        void Walk( const Stmt* stmt, WalkCounter& c );
        bool VisitStmt( const Stmt* stmt );
    };

}

#endif // CASCADINGCONDOP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
