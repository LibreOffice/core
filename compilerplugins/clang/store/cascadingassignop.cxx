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

#include "cascadingassignop.hxx"

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
static const int stmtlimit = 20;

namespace loplugin
{

struct WalkCounter
{
    int  stmtcount;
    bool cascading;
    bool conditionals;
};

// Ctor, nothing special, pass the argument(s).
CascadingAssignOp::CascadingAssignOp( const InstantiationData& data )
    : Plugin( data )
{
}

// Perform the actual action.
void CascadingAssignOp::run()
{
    // Traverse the whole AST of the translation unit (i.e. examine the whole source file).
    // The Clang AST helper class will call VisitReturnStmt for every return statement.
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
}

void CascadingAssignOp::Walk( const Stmt* stmt, WalkCounter& c )
{
    for(Stmt::const_child_iterator it = stmt->child_begin(); it != stmt->child_end(); ++it)
    {
        ++c.stmtcount;
        const BinaryOperator* binop = dyn_cast< BinaryOperator >( *it );
        if ( binop )
        {
            if ( (binop->isAssignmentOp() || binop->isCompoundAssignmentOp()))
                c.cascading = true;
            if ( dyn_cast< AbstractConditionalOperator >( binop ) || binop->isLogicalOp())
                c.conditionals = true;
        }
        Walk(*it, c);
    }
}

bool CascadingAssignOp::VisitStmt( const Stmt* stmt )
{
    const BinaryOperator* binop = dyn_cast< BinaryOperator >( stmt );
    if ( binop && (binop->isAssignmentOp() || binop->isCompoundAssignmentOp()))
    {
        WalkCounter c = { 0, false, false };
        Walk(binop, c);
        if(c.cascading && c.conditionals && c.stmtcount >= stmtlimit)
        {
            std::string msg("cascading assign operator mixing in conditionals, complexity: ");
            msg.append(std::to_string(c.stmtcount));
            report( DiagnosticsEngine::Warning, msg, binop->getLocStart());
        }
    }
    return true;
}

// Register the plugin action with the LO plugin handling.
static Plugin::Registration< CascadingAssignOp > X( "cascadingassignop" );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
