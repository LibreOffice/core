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

Tutorial2::Tutorial2( CompilerInstance& compiler )
    : Plugin( compiler )
    {
    }

void Tutorial2::run()
    {
    
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }


bool Tutorial2::VisitIfStmt( const IfStmt* ifstmt )
    {
    if( ignoreLocation( ifstmt ))
        return true;
    
    if( const BinaryOperator* oper = dyn_cast< BinaryOperator >( ifstmt->getCond()))
        {
        
        if( oper->getOpcode() == BO_EQ )
            {
            
            const Stmt* warn = NULL; 
            
            if( isReturnFalse( ifstmt->getThen()))
                warn = ifstmt->getThen();
            
            else if( const CompoundStmt* compound = dyn_cast< CompoundStmt >( ifstmt->getThen()))
                {
                if( compound->size() == 1 ) 
                    if( isReturnFalse( *compound->body_begin())) 
                        warn = *compound->body_begin();
                }
            if( warn != NULL ) 
                {
                report( DiagnosticsEngine::Warning,
                    "returning false after if with equality comparison",
                    cast< ReturnStmt >( warn )->getRetValue()->getLocStart()) 
                    << warn->getSourceRange();
                
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
    
    if( const ReturnStmt* returnstmt = dyn_cast< ReturnStmt >( stmt ))
        {
        
        if( const CXXBoolLiteralExpr* boolliteral = dyn_cast_or_null< CXXBoolLiteralExpr >( returnstmt->getRetValue()))
            {
            if( boolliteral->getValue() == false )
                return true;
            }
        }
    return false;
    }


static Plugin::Registration< Tutorial2 > X( "tutorial2" );

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
