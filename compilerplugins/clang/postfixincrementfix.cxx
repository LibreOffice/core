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

#include "postfixincrementfix.hxx"

/*
This is a rewriter.

Change all postfix ++ operators of non-trivial types to prefix if possible.
*/

namespace loplugin
{

PostfixIncrementFix::PostfixIncrementFix( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
    {
    }

void PostfixIncrementFix::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool PostfixIncrementFix::VisitCXXOperatorCallExpr( const CXXOperatorCallExpr* op )
    {
    if( ignoreLocation( op ))
        return true;
    // postfix ++ has two arguments (the operand and the hidden extra int)
    if( op->getOperator() == OO_PlusPlus && op->getNumArgs() == 2 )
        fixPostfixOperator( op );
    // For primitive types it would be UnaryOperatorExpr, but probably no good reason to change those.
    return true;
    }

void PostfixIncrementFix::fixPostfixOperator( const CXXOperatorCallExpr* op )
    {
    if( !canChangePostfixToPrefix( op, op ))
        return;
    if( !shouldDoChange( op->getArg( 0 )))
        return;
    // Adding spaces around the moved ++ should not be necessary
    // (there might a problem with e.g. a+b++ -> a+++b (i.e. a++ +b),
    // but we don't change such expressions).
    if( insertText( op->getLocStart(), "++" )) // insert is intentionally first, in case it fails
        removeText( op->getCallee()->getSourceRange());
    }

bool PostfixIncrementFix::canChangePostfixToPrefix( const Stmt* stmt , const CXXOperatorCallExpr* op )
    {
    const Stmt* parent = parentStmt( stmt );
    if( parent == NULL )
        return true;
    // check if foo++ can be safely replaced by ++foo
    switch( parent->getStmtClass())
        {
        case Stmt::CompoundStmtClass:
            return true;
        // these mean somebody is going to use it
        case Stmt::ImplicitCastExprClass:
        case Stmt::MaterializeTemporaryExprClass:
        case Stmt::BinaryOperatorClass:
        case Stmt::UnaryOperatorClass:
        case Stmt::CallExprClass:
        case Stmt::CXXOperatorCallExprClass:
            return false;
        case Stmt::CXXBindTemporaryExprClass:
            // tricky, it may just mean the temporary will be cleaned up
            // (by ExprWithCleanups), ignore and go up
            return canChangePostfixToPrefix( parent, op );
        case Stmt::ExprWithCleanupsClass:
            // cleanup of a temporary, should be harmless (if the use
            // of the postfix ++ operator here relies on the fact that
            // the dtor for the object will be called, that's pretty insane
            // code). Ignore and go up.
            return canChangePostfixToPrefix( parent, op );
        case Stmt::ParenExprClass: // parentheses, go up
            return canChangePostfixToPrefix( parent, op );
        case Stmt::IfStmtClass:
            // cannot be changed in condition, can be changed in statements
            return cast< IfStmt >( parent )->getCond() != stmt;
        case Stmt::WhileStmtClass:
            return cast< WhileStmt >( parent )->getCond() != stmt;
        case Stmt::DoStmtClass:
            return cast< DoStmt >( parent )->getCond() != stmt;
        case Stmt::ForStmtClass:
            return cast< ForStmt >( parent )->getCond() != stmt;
        default:
            report( DiagnosticsEngine::Fatal, "cannot analyze operator++ (plugin needs fixing)",
                op->getLocStart()) << parent->getSourceRange();
            parent->dump();
            return false;
        }
    }

bool PostfixIncrementFix::shouldDoChange( const Expr* operand )
    {
    // TODO Changing 'a->b++' to '++a->b' is technically the same, but the latter probably looks confusing,
    // so either avoid that, or add parentheses. Avoid for now.
    const Expr* expr = const_cast< Expr* >( operand )->IgnoreImplicit(); // does not have const version
    switch( expr->getStmtClass())
        {
        case Stmt::ParenExprClass:
            return true; // there are already parentheses, ok to move the ++
        case Stmt::MemberExprClass:
            return false; // ++a->b , avoid
        case Stmt::DeclRefExprClass:
            return true;
        default:
            {
            report( DiagnosticsEngine::Fatal, "cannot analyze operator++ (plugin needs fixing)",
                expr->getLocStart()) << operand->getSourceRange();
            expr->dump();
            operand->dump();
            return false;
            }
        }
    }

static Plugin::Registration< PostfixIncrementFix > X( "postfixincrementfix" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
