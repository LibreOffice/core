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

PostfixIncrementFix::PostfixIncrementFix( const InstantiationData& data )
    : RewritePlugin( data )
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
    
    if( op->getOperator() == OO_PlusPlus && op->getNumArgs() == 2 )
        fixPostfixOperator( op );
    
    return true;
    }

void PostfixIncrementFix::fixPostfixOperator( const CXXOperatorCallExpr* op )
    {
    if( !canChangePostfixToPrefix( op, op ))
        return;
    if( !shouldDoChange( op->getArg( 0 )))
        return;
    
    
    
    if( insertText( op->getLocStart(), "++" )) 
        removeText( op->getCallee()->getSourceRange());
    }

bool PostfixIncrementFix::canChangePostfixToPrefix( const Stmt* stmt , const CXXOperatorCallExpr* op )
    {
    const Stmt* parent = parentStmt( stmt );
    if( parent == NULL )
        return true;
    
    switch( parent->getStmtClass())
        {
        case Stmt::CompoundStmtClass:
            return true;
        
        case Stmt::ImplicitCastExprClass:
        case Stmt::MaterializeTemporaryExprClass:
        case Stmt::BinaryOperatorClass:
        case Stmt::UnaryOperatorClass:
        case Stmt::CallExprClass:
        case Stmt::CXXOperatorCallExprClass:
            return false;
        case Stmt::CXXBindTemporaryExprClass:
            
            
            return canChangePostfixToPrefix( parent, op );
        case Stmt::ExprWithCleanupsClass:
            
            
            
            
            return canChangePostfixToPrefix( parent, op );
        case Stmt::ParenExprClass: 
            return canChangePostfixToPrefix( parent, op );
        case Stmt::IfStmtClass:
            
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
    
    
    const Expr* expr = const_cast< Expr* >( operand )->IgnoreImplicit(); 
    switch( expr->getStmtClass())
        {
        case Stmt::ParenExprClass:
            return true; 
        case Stmt::MemberExprClass:
            return false; 
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
