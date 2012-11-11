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

#include <clang/Basic/SourceManager.h>

namespace loplugin
{

PostfixIncrementFix::PostfixIncrementFix( ASTContext& context, Rewriter& rewriter )
    : RewritePlugin( context, rewriter )
    {
    }

void PostfixIncrementFix::run()
    {
    TraverseDecl( context.getTranslationUnitDecl());
    }

bool PostfixIncrementFix::VisitFunctionDecl( FunctionDecl* declaration )
    {
    // TODO also LO header files? or a subdir?
    if( !context.getSourceManager().isFromMainFile( declaration->getLocStart()))
        return true;
    if( !declaration->doesThisDeclarationHaveABody())
        return true;
    StmtParents parents;
    fixPostfixOperators( declaration->getBody(), parents );
    return true;
    }

void PostfixIncrementFix::fixPostfixOperators( const Stmt* stmt, StmtParents& parents )
    {
    if( const CXXOperatorCallExpr* op = dyn_cast<CXXOperatorCallExpr>( stmt ))
        { // postfix ++ has two arguments (the operand and the hidden extra int)
        if( op->getOperator() == OO_PlusPlus && op->getNumArgs() == 2 )
            fixPostfixOperator( op, parents );
        }
    // For primitive types it would be UnaryOperatorExpr, but probably no good reason to change those.
    parents.push_back( stmt );
    for( ConstStmtIterator it = stmt->child_begin();
         it != stmt->child_end();
         ++it )
        {
        if( *it != NULL ) // some children can be apparently NULL
            fixPostfixOperators( *it, parents );
        }
    assert( parents.back() == stmt );
    parents.pop_back();
    }

void PostfixIncrementFix::fixPostfixOperator( const CXXOperatorCallExpr* op, StmtParents& parents )
    {
    if( !canChangePostfixToPrefix( op, parents, parents.size() - 1 ))
        return;
    if( !shouldDoChange( op->getArg( 0 )))
        return;
    // Adding spaces around the moved ++ should not be necessary
    // (there might a problem with e.g. a+b++ -> a+++b (i.e. a++ +b),
    // but we don't change such expressions).
    if( insertText( op->getLocStart(), "++" )) // insert is intentionally first, in case it fails
        removeText( op->getCallee()->getSourceRange());
    }

bool PostfixIncrementFix::canChangePostfixToPrefix( const CXXOperatorCallExpr* op, StmtParents& parents, int parent_pos )
    {
    // check if foo++ can be safely replaced by ++foo
    switch( parents[ parent_pos ]->getStmtClass())
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
            assert( parent_pos > 0 ); // should not happen
            return canChangePostfixToPrefix( op, parents, parent_pos - 1 );
        case Stmt::ExprWithCleanupsClass:
            // cleanup of a temporary, should be harmless (if the use
            // of the postfix ++ operator here relies on the fact that
            // the dtor for the object will be called, that's pretty insane
            // code). Ignore and go up.
            assert( parent_pos > 0 ); // should not happen
            return canChangePostfixToPrefix( op, parents, parent_pos - 1 );
        case Stmt::ParenExprClass: // parentheses, go up
            assert( parent_pos > 0 );
            return canChangePostfixToPrefix( op, parents, parent_pos - 1 );
        case Stmt::IfStmtClass:
            return canChangeInConditionStatement( op, cast< IfStmt >( parents[ parent_pos ] )->getCond(),
                parents, parent_pos );
        case Stmt::WhileStmtClass:
            return canChangeInConditionStatement( op, cast< WhileStmt >( parents[ parent_pos ] )->getCond(),
                parents, parent_pos );
        case Stmt::DoStmtClass:
            return canChangeInConditionStatement( op, cast< DoStmt >( parents[ parent_pos ] )->getCond(),
                parents, parent_pos );
        case Stmt::ForStmtClass:
            return canChangeInConditionStatement( op, dyn_cast< ForStmt >( parents[ parent_pos ] )->getCond(),
                parents, parent_pos );
        default:
            DiagnosticsEngine& diag = context.getDiagnostics();
            unsigned diagid = diag.getCustomDiagID( DiagnosticsEngine::Fatal,
                "cannot analyze operator++ (plugin needs fixing) [loplugin]" );
            diag.Report( op->getLocStart(), diagid ) << parents[ parent_pos ]->getSourceRange();
//            parents[ parent_pos ]->dump();
//            parents[ std::max( parent_pos - 3, 0 ) ]->dump();
            return false;
        }
    }

bool PostfixIncrementFix::canChangeInConditionStatement( const CXXOperatorCallExpr* op, const Expr* condition,
    const StmtParents& parents, int parent_pos )
    {
    // cannot be changed in condition, can be changed in statements
    if( parent_pos == parents.size() - 1 )
        return op != condition;
    else
        { // indirect child
        assert( parent_pos + 1 < parents.size());
        return parents[ parent_pos + 1 ] != condition;
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
            DiagnosticsEngine& diag = context.getDiagnostics();
            unsigned diagid = diag.getCustomDiagID( DiagnosticsEngine::Fatal,
                "cannot analyze operator++ (plugin needs fixing) [loplugin]" );
            diag.Report( expr->getLocStart(), diagid ) << operand->getSourceRange();
            expr->dump();
            operand->dump();
            return false;
            }
        }
    }

} // namespace
