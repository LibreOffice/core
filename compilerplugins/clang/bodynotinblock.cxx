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

#include "bodynotinblock.hxx"

namespace loplugin
{

/*
This is a compile check.

Check for two statements that are both indented to look like a body of if/while/for
but are not inside a compound statement and thus the second one is unrelated.

For example:

    if( a != 0 )
        b = 2;
        c = 3;

Here either both statements should be inside {} or the second statement in indented wrong.
*/

BodyNotInBlock::BodyNotInBlock( CompilerInstance& compiler )
    : Plugin( compiler )
    {
    }

void BodyNotInBlock::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool BodyNotInBlock::VisitIfStmt( const IfStmt* stmt )
    {
    if( ignoreLocation( stmt ))
        return true;
    checkBody( stmt->getThen(), stmt->getIfLoc(), 0, stmt->getElse() != NULL );
    checkBody( stmt->getElse(), stmt->getElseLoc(), 0 );
    return true;
    }

bool BodyNotInBlock::VisitWhileStmt( const WhileStmt* stmt )
    {
    if( ignoreLocation( stmt ))
        return true;
    checkBody( stmt->getBody(), stmt->getWhileLoc(), 1 );
    return true;
    }

bool BodyNotInBlock::VisitForStmt( const ForStmt* stmt )
    {
    if( ignoreLocation( stmt ))
        return true;
    checkBody( stmt->getBody(), stmt->getForLoc(), 2 );
    return true;
    }

bool BodyNotInBlock::VisitCXXForRangeStmt( const CXXForRangeStmt* stmt )
    {
    if( ignoreLocation( stmt ))
        return true;
    checkBody( stmt->getBody(), stmt->getForLoc(), 2 );
    return true;
    }

void BodyNotInBlock::checkBody( const Stmt* body, SourceLocation stmtLocation, int stmtType, bool dontGoUp )
    {
    if( body == NULL )
        return;
    // TODO: If the if/else/while/for comes from a macro expansion, ignore it completely for
    // now. The code below could assume everything is in the same place (and thus also column)
    // and give a false warning. Moreover some macros are rather lousily written and would
    // result in poor formatting. To be evaluated later, maybe this could be handled
    // including macro expansion.
    if( stmtLocation.isMacroID())
        return;
    if( dyn_cast< CompoundStmt >( body ))
        return; // if body is a compound statement, then it is in {}
    const Stmt* previousParent = parentStmt( body ); // Here the statement itself.
    // Find the next statement (in source position) after 'body'.
    for(;;)
        {
        const Stmt* parent = parentStmt( previousParent );
        if( parent == NULL )
            break;
        for( ConstStmtIterator it = parent->child_begin();
             it != parent->child_end();
             )
            {
            if( *it == previousParent ) // found grand(grand...)parent
                {
                // get next statement after our (grand...)parent
                ++it;
                while( it != parent->child_end() && *it == NULL )
                    ++it; // skip empty ones (missing 'else' bodies for example)
                if( it != parent->child_end())
                    {
                    bool invalid1, invalid2;
                    unsigned bodyColumn = compiler.getSourceManager()
                        .getPresumedColumnNumber( body->getLocStart(), &invalid1 );
                    unsigned nextStatementColumn = compiler.getSourceManager()
                        .getPresumedColumnNumber( (*it)->getLocStart(), &invalid2 );
                    if( invalid1 || invalid2 )
                        return;
                    if( bodyColumn == nextStatementColumn )
                        {
                        report( DiagnosticsEngine::Warning,
                            "statement aligned as second statement in %select{if|while|for}0 body but not in a statement block",
                            (*it)->getLocStart()) << stmtType;
                        report( DiagnosticsEngine::Note,
                            "%select{if|while|for}0 body statement is here",
                            body->getLocStart()) << stmtType;
                        }
                    return;
                    }
                // else we need to go higher to find the next statement
                }
            else
                ++it;
            }
        // If going up would mean leaving a {} block, stop, because the } should
        // make it visible the two statements are not in the same body.
        if( dyn_cast< CompoundStmt >( parent ))
            return;
        // If the body to be checked is a body of an if statement that has also
        // an else part, don't go up, the else is after the body and should make
        // it clear the body does not continue there.
        if( dontGoUp )
            return;
        previousParent = parent;
        }
    }

static Plugin::Registration< BodyNotInBlock > X( "bodynotinblock" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
