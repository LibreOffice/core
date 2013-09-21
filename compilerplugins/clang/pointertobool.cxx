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

#include "plugin.hxx"

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>

namespace loplugin
{

/*
This is a compile check.

Check for pointer-to-bool conversions (that are sadly implicit) which are unwanted
and potentially mistakes.

So far the only places that are checked are passing arguments to functions, as those
could easily choose a different overload.

The original idea was that to follow the explicit bool feature from C++11, where
the only conversions that would be considered safe are in conditions (which
in turn means also in ||, && and ! operators) and places where it's considered
unlikely for it to be a problem (or rather, less of a problem
than explicitly avoiding the warning in the code). The code for this is currently
commented out (there are a couple of places such as 'bool foo = returns_pointer();'
that would need modification), possibly enable those later.
*/

class PointerToBool
    : public RecursiveASTVisitor< PointerToBool >
    , public Plugin
    {
    public:
        explicit PointerToBool( CompilerInstance& compiler );
        void run();
        bool VisitImplicitCastExpr( const ImplicitCastExpr* expr );
    private:
        bool ignoreConversion( const Stmt* stmt );
    };

PointerToBool::PointerToBool( CompilerInstance& compiler )
    : Plugin( compiler )
    {
    }

void PointerToBool::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool PointerToBool::VisitImplicitCastExpr( const ImplicitCastExpr* expr )
    {
    if( ignoreLocation( expr ))
        return true;
    // Warning about CK_MemberPointerToBoolean would mean warning about
    // cases there the 'safe bool' idiom is used, so give that such
    // a conversion is otherwise unlikely anyway, it's probably better
    // not to warn here at all (at least as long as the 'explicit bool'
    // from C++11 is not in use).
    if( expr->getCastKind() == CK_PointerToBoolean )
        {
        if( ignoreConversion( expr ))
            return true;
        report( DiagnosticsEngine::Warning,
            "pointer %0 implicitly converted to bool", expr->getLocStart())
            << expr->getSubExpr()->getType() << expr->getSourceRange();
        SourceLocation endOfExpression = locationAfterToken( expr->getLocEnd());
        report( DiagnosticsEngine::Note,
            "explicitly compare to null pointer to silence this warning", endOfExpression )
            << FixItHint::CreateInsertion( endOfExpression, " != NULL" );
        }
    return true;
    }

bool PointerToBool::ignoreConversion( const Stmt* stmt )
    {
#if 1 // less strict version
    const Stmt* parent = parentStmt( stmt );
    if( parent == NULL )
        return true;
    switch( parent->getStmtClass())
        {
        case Stmt::ConditionalOperatorClass:
            if( stmt == cast< ConditionalOperator >( parent )->getCond())
                return true;
            break;
        case Stmt::BinaryOperatorClass:
            {
            const BinaryOperator* binary = cast< BinaryOperator >( parent );
            if(( binary->getOpcode() == BO_LAnd || binary->getOpcode() == BO_LOr )
                && ( stmt == binary->getLHS() || stmt == binary->getRHS()))
                {
                return true;
                }
            break;
            }
        case Stmt::UnaryOperatorClass:
            {
            const UnaryOperator* unary = cast< UnaryOperator >( parent );
            if( unary->getOpcode() == UO_LNot && stmt == unary->getSubExpr())
                return true;
            break;
            }
        default:
            if( const ExplicitCastExpr* castexpr = dyn_cast< ExplicitCastExpr >( parent ))
                if( castexpr->getTypeAsWritten()->isBooleanType() && stmt == castexpr->getSubExpr())
                    return true;
            if( dyn_cast< CallExpr >( parent ))
                return false; // The only place where it's not ignored.
            break;
        }
    return ignoreConversion( parent );
#else // more strict version
    // Warn only if the expression is not used in a conditional context.
    const Stmt* parent = parentStmt( stmt );
    if( parent == NULL ) // Should not happen inside a function, but can happen inside
        return false;    // ctor initializer list.
    switch( parent->getStmtClass())
        {
        case Stmt::IfStmtClass:
            return ( stmt == cast< IfStmt >( parent )->getCond());
        case Stmt::WhileStmtClass:
            return ( stmt == cast< WhileStmt >( parent )->getCond());
        case Stmt::DoStmtClass:
            return ( stmt == cast< DoStmt >( parent )->getCond());
        case Stmt::ForStmtClass:
            return ( stmt == cast< ForStmt >( parent )->getCond());
        case Stmt::ConditionalOperatorClass:
            return ( stmt == cast< ConditionalOperator >( parent )->getCond());
        case Stmt::BinaryOperatorClass:
            {
            const BinaryOperator* binary = cast< BinaryOperator >( parent );
            return (( binary->getOpcode() == BO_LAnd || binary->getOpcode() == BO_LOr )
                && ( stmt == binary->getLHS() || stmt == binary->getRHS()));
            }
        case Stmt::UnaryOperatorClass:
            {
            const UnaryOperator* unary = cast< UnaryOperator >( parent );
            return ( unary->getOpcode() == UO_LNot && stmt == unary->getSubExpr());
            }
        case Stmt::ExprWithCleanupsClass: // Often happens inside if() condition.
            return isInConditionalContext( parent );
        default:
            if( const ExplicitCastExpr* castexpr = dyn_cast< ExplicitCastExpr >( parent ))
                if( castexpr->getTypeAsWritten()->isBooleanType() && stmt == castexpr->getSubExpr())
                    return true;
            break;
        }
    return false;
#endif
    }

static Plugin::Registration< PointerToBool > X( "pointertobool" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
