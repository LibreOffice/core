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

// Ctor, pass arguments.
Tutorial3::Tutorial3( const InstantiationData& data )
    : FilteringRewritePlugin( data )
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
            // Modify the sub-statement if it is 'return false'.
            modifyReturnFalse( ifstmt->getThen());
            // Modify the sub-statement if it is '{ return false; }'.
            if( const CompoundStmt* compound = dyn_cast< CompoundStmt >( ifstmt->getThen()))
                {
                if( compound->size() == 1 ) // one statement
                    modifyReturnFalse( *compound->body_begin());
                }
            }
        }
    return true;
    }

void Tutorial3::modifyReturnFalse( const Stmt* stmt )
    {
    // Is it return statement?
    if( const ReturnStmt* returnstmt = dyn_cast< ReturnStmt >( stmt ))
        {
        // dyn_cast_or_null<> can also be passed NULL, unlike dyn_cast<>
        if( const CXXBoolLiteralExpr* boolliteral = dyn_cast_or_null< CXXBoolLiteralExpr >( returnstmt->getRetValue()))
            {
            if( boolliteral->getValue() == false )
                { // It is, modify the false to true using LO plugin helper function.
                replaceText( boolliteral->getSourceRange(), "maybereturntrue" );
                }
            }
        }
    }

// Register the plugin action with the LO plugin handling.
static Plugin::Registration< Tutorial3 > tutorial3( "tutorial3" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
