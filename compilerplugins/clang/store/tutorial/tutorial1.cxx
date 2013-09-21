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

#include "tutorial1.hxx"

/*
This is a compile check.

Checks all return statements and warns if they return literal false (i.e. 'return false').
*/

namespace loplugin
{

// Ctor, nothing special, pass the argument(s).
Tutorial1::Tutorial1( CompilerInstance& compiler )
    : Plugin( compiler )
    {
    }

// Perform the actual action.
void Tutorial1::run()
    {
    // Traverse the whole AST of the translation unit (i.e. examine the whole source file).
    // The Clang AST helper class will call VisitReturnStmt for every return statement.
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

// This function is called for every return statement.
// Returning true means to continue with examining the AST, false means to stop (just always return true).
bool Tutorial1::VisitReturnStmt( const ReturnStmt* returnstmt )
    {
    // Helper function from the LO base plugin class, call at the very beginning to ignore sources
    // that should not be processed (e.g. system headers).
    if( ignoreLocation( returnstmt ))
        return true;
    // Get the expression in the return statement (see ReturnStmt API docs).
    const Expr* expression = returnstmt->getRetValue();
    if( expression == NULL )
        return true; // plain 'return;' without expression
    // Check if the expression is a bool literal (Clang uses dyn_cast<> instead of dynamic_cast<>).
    if( const CXXBoolLiteralExpr* boolliteral = dyn_cast< CXXBoolLiteralExpr >( expression ))
        { // It is.
        if( boolliteral->getValue() == false ) // Is it 'return false;' ? (See CXXBoolLiteralExpr API docs)
            { // Ok, warn, use LO plugin helper function.
            report( DiagnosticsEngine::Warning, // It's just a warning.
                "returning false",  // the message
                boolliteral->getLocStart()) // and the exact position where the message should point
                    << returnstmt->getSourceRange(); // and the full return statement to highlight (optional)
            }
        }
    return true;
    }

// Register the plugin action with the LO plugin handling.
static Plugin::Registration< Tutorial1 > X( "tutorial1" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
