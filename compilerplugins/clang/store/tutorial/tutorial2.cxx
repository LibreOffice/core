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

#include "tutorial2.hxx"

/*
This is a compile check.

Warns about if statements with a comparison followed by literal return false:
if( a == 1 )
    return false;
*/

namespace loplugin
{

Tutorial2::Tutorial2( const InstantiationData& data )
    : FilteringPlugin( data )
    {
    }

void Tutorial2::run()
    {
    // The Clang AST helper class will call VisitIfStmt for every if statement.
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

// This function is called for every if statement.
bool Tutorial2::VisitIfStmt( const IfStmt* ifstmt )
    {
    if( ignoreLocation( ifstmt ))
        return true;
    // Check if the condition of the if statement is a binary operator.
    if( const BinaryOperator* oper = dyn_cast< BinaryOperator >( ifstmt->getCond()))
        {
        // And if it's operator==.
        if( oper->getOpcode() == BO_EQ )
            {
            // Now check if the sub-statement is 'return false'.
            const Stmt* warn = NULL; // The return statement (for the warning message).
            // Check if the sub-statement is directly 'return false;'.
            if( isReturnFalse( ifstmt->getThen()))
                warn = ifstmt->getThen();
            // Check if the sub-statement is '{ return false; }'
            else if( const CompoundStmt* compound = dyn_cast< CompoundStmt >( ifstmt->getThen()))
                {
                if( compound->size() == 1 ) // one statement
                    if( isReturnFalse( *compound->body_begin())) // check the one sub-statement
                        warn = *compound->body_begin();
                }
            if( warn != NULL ) // there is a return statement to warn about.
                {
                report( DiagnosticsEngine::Warning,
                    "returning false after if with equality comparison",
                    cast< ReturnStmt >( warn )->getRetValue()->getLocStart()) // the 'false' in the return
                    << warn->getSourceRange();
                // Also add a note showing the if statement.
                report( DiagnosticsEngine::Note,
                    "the if statement is here",
                    ifstmt->getLocStart());
                }
            }
        }
    return true;
    }

bool Tutorial2::isReturnFalse( const Stmt* stmt )
    {
    // Is it return statement?
    if( const ReturnStmt* returnstmt = dyn_cast< ReturnStmt >( stmt ))
        {
        // dyn_cast_or_null<> can also be passed NULL, unlike dyn_cast<>
        if( const CXXBoolLiteralExpr* boolliteral = dyn_cast_or_null< CXXBoolLiteralExpr >( returnstmt->getRetValue()))
            {
            if( boolliteral->getValue() == false )
                return true;
            }
        }
    return false;
    }

// Register the plugin action with the LO plugin handling.
static Plugin::Registration< Tutorial2 > tutorial2( "tutorial2" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
