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

#include "compileplugin.hxx"

namespace loplugin
{

class BodyNotInBlock
    : public RecursiveASTVisitor< BodyNotInBlock >
    , public Plugin
    {
    public:
        explicit BodyNotInBlock( ASTContext& context );
        void run();
        bool VisitFunctionDecl( FunctionDecl* declaration );
    private:
        typedef std::vector< const Stmt* > StmtParents;
        void traverseStatement( const Stmt* stmt, StmtParents& parents );
        void checkBody( const Stmt* body, const StmtParents& parents, int stmtType, bool dontGoUp = false );
    };

} // namespace

#endif // BODYNOTINBLOCK_H
