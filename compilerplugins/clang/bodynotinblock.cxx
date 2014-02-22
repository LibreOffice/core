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

BodyNotInBlock::BodyNotInBlock( const InstantiationData& data )
    : Plugin( data )
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
    
    
    
    
    
    if( stmtLocation.isMacroID())
        return;
    if( dyn_cast< CompoundStmt >( body ))
        return; 
    const Stmt* previousParent = parentStmt( body ); 
    
    for(;;)
        {
        const Stmt* parent = parentStmt( previousParent );
        if( parent == NULL )
            break;
        for( ConstStmtIterator it = parent->child_begin();
             it != parent->child_end();
             )
            {
            if( *it == previousParent ) 
                {
                
                ++it;
                while( it != parent->child_end() && *it == NULL )
                    ++it; 
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
                
                }
            else
                ++it;
            }
        
        
        if( dyn_cast< CompoundStmt >( parent ))
            return;
        
        
        
        if( dontGoUp )
            return;
        previousParent = parent;
        }
    }

static Plugin::Registration< BodyNotInBlock > X( "bodynotinblock" );

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
