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

#include <clang/Basic/SourceManager.h>

namespace loplugin
{

/*
This is a compile check.

Check for two statements that are both indented to look like a body of if/while/for
but are not inside a compound statement and thus the second one is unrelated.
*/

BodyNotInBlock::BodyNotInBlock( ASTContext& context )
    : Plugin( context )
    {
    }

void BodyNotInBlock::run()
    {
    TraverseDecl( context.getTranslationUnitDecl());
    }

bool BodyNotInBlock::VisitFunctionDecl( FunctionDecl* declaration )
    {
    if( ignoreLocation( declaration ))
        return true;
    if( !declaration->doesThisDeclarationHaveABody())
        return true;
    StmtParents parents;
    traverseStatement( declaration->getBody(), parents );
    return true;
    }

void BodyNotInBlock::traverseStatement( const Stmt* stmt, StmtParents& parents )
    {
    parents.push_back( stmt );
    for( ConstStmtIterator it = stmt->child_begin();
         it != stmt->child_end();
         ++it )
        {
        if( *it != NULL ) // some children can be apparently NULL
            {
            traverseStatement( *it, parents ); // substatements first
            parents.push_back( *it );
            if( const IfStmt* ifstmt = dyn_cast< IfStmt >( *it ))
                {
                checkBody( ifstmt->getThen(), parents, 0, ifstmt->getElse() != NULL );
                checkBody( ifstmt->getElse(), parents, 0 );
                }
            else if( const WhileStmt* whilestmt = dyn_cast< WhileStmt >( *it ))
                checkBody( whilestmt->getBody(), parents, 1 );
            else if( const ForStmt* forstmt = dyn_cast< ForStmt >( *it ))
                checkBody( forstmt->getBody(), parents, 2 );
            else if( const CXXForRangeStmt* forstmt = dyn_cast< CXXForRangeStmt >( *it ))
                checkBody( forstmt->getBody(), parents, 2 );
            parents.pop_back();
            }
        }
    assert( parents.back() == stmt );
    parents.pop_back();
    }

void BodyNotInBlock::checkBody( const Stmt* body, const StmtParents& parents, int stmtType, bool dontGoUp )
    {
    if( body == NULL || parents.size() < 2 )
        return;
    // TODO: If the if/while/for comes from a macro expansion, ignore it completely for
    // now. The code below could assume everything is in the same place (and thus also column)
    // and give a false warning. Moreover some macros are rather lousily written and would
    // result in poor formatting. To be evaluated later, maybe this could be handled
    // including macro expansion.
    if( parents.back()->getLocStart().isMacroID())
        return;
    if( dyn_cast< CompoundStmt >( body ))
        return; // if body is a compound statement, then it is in {}
    // Find the next statement (in source position) after 'body'.
    for( int parent_pos = parents.size() - 2; // start from grandparent
         parent_pos >= 0;
         --parent_pos )
        {
        for( ConstStmtIterator it = parents[ parent_pos ]->child_begin();
             it != parents[ parent_pos ]->child_end();
             )
            {
            if( *it == parents[ parent_pos + 1 ] ) // found grand(grand...)parent
                {
                // get next statement after our (grand...)parent
                ++it;
                while( it != parents[ parent_pos ]->child_end() && *it == NULL )
                    ++it; // skip empty ones (missing 'else' bodies for example)
                if( it != parents[ parent_pos ]->child_end())
                    {
                    bool invalid1, invalid2;
                    unsigned bodyColumn = context.getSourceManager()
                        .getPresumedColumnNumber( body->getLocStart(), &invalid1 );
                    unsigned nextStatementColumn = context.getSourceManager()
                        .getPresumedColumnNumber( (*it)->getLocStart(), &invalid2 );
                    if( invalid1 || invalid2 )
                        return;
                    if( bodyColumn == nextStatementColumn )
                        {
                        report( DiagnosticsEngine::Warning,
                            "statement aligned as second statement in %select{if|while|for}0 body but not in a statement block [loplugin]",
                            (*it)->getLocStart()) << stmtType;
                        report( DiagnosticsEngine::Note,
                            "%select{if|while|for}0 body statement is here [loplugin]",
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
        if( dyn_cast< CompoundStmt >( parents[ parent_pos ] ))
            return;
        // If the body to be checked is a body of an if statement that has also
        // an else part, don't go up, the else is after the body and should make
        // it clear the body does not continue there.
        if( dontGoUp )
            return;
        }
    }

} // namespace
