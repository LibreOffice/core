/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef POSTFIXINCREMENTFIX_H
#define POSTFIXINCREMENTFIX_H

#include "plugin.hxx"

namespace loplugin
{

class PostfixIncrementFix
    : public RecursiveASTVisitor< PostfixIncrementFix >
    , public RewritePlugin
    {
    public:
        explicit PostfixIncrementFix( ASTContext& context, Rewriter& rewriter );
        void run();
        bool VisitFunctionDecl( FunctionDecl* declaration );
    private:
        typedef std::vector< const Stmt* > StmtParents;
        void fixPostfixOperator( const CXXOperatorCallExpr* op, StmtParents& parents );
        void fixPostfixOperators( const Stmt* stmt, StmtParents& parents );
        bool canChangePostfixToPrefix( const CXXOperatorCallExpr* op, StmtParents& parents, int parent_pos );
        bool canChangeInConditionStatement( const CXXOperatorCallExpr* op, const Expr* condition,
            const StmtParents& parents, int parent_pos );
        bool shouldDoChange( const Expr* op );
    };

} // namespace

#endif // POSTFIXINCREMENTFIX_H

