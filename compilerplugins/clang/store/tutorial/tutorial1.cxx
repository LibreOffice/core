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


Tutorial1::Tutorial1( CompilerInstance& compiler )
    : Plugin( compiler )
    {
    }


void Tutorial1::run()
    {
    
    
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }



bool Tutorial1::VisitReturnStmt( const ReturnStmt* returnstmt )
    {
    
    
    if( ignoreLocation( returnstmt ))
        return true;
    
    const Expr* expression = returnstmt->getRetValue();
    if( expression == NULL )
        return true; 
    
    if( const CXXBoolLiteralExpr* boolliteral = dyn_cast< CXXBoolLiteralExpr >( expression ))
        { 
        if( boolliteral->getValue() == false ) 
            { 
            report( DiagnosticsEngine::Warning, 
                "returning false",  
                boolliteral->getLocStart()) 
                    << returnstmt->getSourceRange(); 
            }
        }
    return true;
    }


static Plugin::Registration< Tutorial1 > X( "tutorial1" );

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
