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

#include "tutorial3.hxx"

/*
This is a rewriter.

It looks for if statements with a comparison followed by literal return false
and modifies the return statements to 'return maybereturntrue;'
*/

namespace loplugin
{


Tutorial3::Tutorial3( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
    {
    }

void Tutorial3::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool Tutorial3::VisitIfStmt( const IfStmt* ifstmt )
    {
    if( ignoreLocation( ifstmt ))
        return true;
    if( const BinaryOperator* oper = dyn_cast< BinaryOperator >( ifstmt->getCond()))
        {
        if( oper->getOpcode() == BO_EQ )
            {
            
            modifyReturnFalse( ifstmt->getThen());
            
            if( const CompoundStmt* compound = dyn_cast< CompoundStmt >( ifstmt->getThen()))
                {
                if( compound->size() == 1 ) 
                    modifyReturnFalse( *compound->body_begin());
                }
            }
        }
    return true;
    }

void Tutorial3::modifyReturnFalse( const Stmt* stmt )
    {
    
    if( const ReturnStmt* returnstmt = dyn_cast< ReturnStmt >( stmt ))
        {
        
        if( const CXXBoolLiteralExpr* boolliteral = dyn_cast_or_null< CXXBoolLiteralExpr >( returnstmt->getRetValue()))
            {
            if( boolliteral->getValue() == false )
                { 
                replaceText( boolliteral->getSourceRange(), "maybereturntrue" );
                }
            }
        }
    }


static Plugin::Registration< Tutorial3 > X( "tutorial3" );

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
