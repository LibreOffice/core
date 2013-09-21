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

#ifndef BODYNOTINBLOCK_H
#define BODYNOTINBLOCK_H

#include "plugin.hxx"

namespace loplugin
{

class BodyNotInBlock
    : public RecursiveASTVisitor< BodyNotInBlock >
    , public Plugin
    {
    public:
        explicit BodyNotInBlock( CompilerInstance& compiler );
        virtual void run() override;
        bool VisitIfStmt( const IfStmt* stmt );
        bool VisitWhileStmt( const WhileStmt* stmt );
        bool VisitForStmt( const ForStmt* stmt );
        bool VisitCXXForRangeStmt( const CXXForRangeStmt* stmt );
    private:
        typedef vector< const Stmt* > StmtParents;
        void checkBody( const Stmt* body, SourceLocation stmtLocation, int stmtType, bool dontGoUp = false );
    };

} // namespace

#endif // BODYNOTINBLOCK_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
