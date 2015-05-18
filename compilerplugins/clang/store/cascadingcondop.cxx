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

#include "cascadingcondop.hxx"

/*
This is a compile check.

It checks for complex statements with conditional operators in conditional
operators, which are errorprone, e.g.
 Thing foo = IsBar() ? ( IsBaz() ? b1 : b2 ) : b3;

However, it finds 556 cases in sw/source alone, thus likely needs some more
restricting, e.g. by checking for multiline conditional operator statements or
a certain length in characters (but that needs the Context/SourceManager, which
I havent played with yet).
*/

// the value is rather arbitrary, but code above this number of stmts begins to
// be smelly
static const int stmtlimit = 50;

namespace loplugin
{

struct WalkCounter
{
    int  stmtcount;
    bool cascading;
};

// Ctor, nothing special, pass the argument(s).
CascadingCondOp::CascadingCondOp( const InstantiationData& data )
    : Plugin( data )
{
}

// Perform the actual action.
void CascadingCondOp::run()
{
    // Traverse the whole AST of the translation unit (i.e. examine the whole source file).
    // The Clang AST helper class will call VisitReturnStmt for every return statement.
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
}

void CascadingCondOp::Walk( const Stmt* stmt, WalkCounter& c )
{
    for(Stmt::const_child_iterator it = stmt->child_begin(); it != stmt->child_end(); ++it)
    {
        ++c.stmtcount;
        if ( dyn_cast< ConditionalOperator >( *it ))
            c.cascading = true;
        Walk(*it, c);
    }
}

bool CascadingCondOp::VisitStmt( const Stmt* stmt )
{
    if ( const ConditionalOperator* condop = dyn_cast< ConditionalOperator >( stmt ))
    {
        WalkCounter c = { 0, false };
        Walk(condop, c);
        if(c.cascading && c.stmtcount >= stmtlimit)
        {
            std::string msg("cascading conditional operator, complexity: ");
            msg.append(std::to_string(c.stmtcount));
            report( DiagnosticsEngine::Warning, msg, condop->getLocStart());
        }
    }
    return true;
}

// Register the plugin action with the LO plugin handling.
static Plugin::Registration< CascadingCondOp > X( "cascadingcondop" );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
