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

#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

namespace loplugin
{

typedef std::vector< const Stmt* > StmtParents;

class BodyNotInBlock
    : public RecursiveASTVisitor< BodyNotInBlock >
    {
    public:
        explicit BodyNotInBlock( ASTContext& context );
        void run();
        bool VisitFunctionDecl( FunctionDecl* declaration );
    private:
        void traverseStatement( const Stmt* stmt, StmtParents& parents );
        void checkBody( const Stmt* body, const StmtParents& parents, int stmtType );
        DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc );
        ASTContext& context;
    };

} // namespace

#endif // BODYNOTINBLOCK_H
