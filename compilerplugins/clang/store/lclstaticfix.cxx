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

#include "lclstaticfix.hxx"

/*
This is a rewriter.

Check all lcl_ functions and prepend static if needed.
*/

namespace loplugin
{

LclStaticFix::LclStaticFix( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
    {
    }

void LclStaticFix::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool LclStaticFix::VisitFunctionDecl( const FunctionDecl* declaration )
    {
    if( ignoreLocation( declaration ))
        return true;
    if( declaration->isCXXClassMember())
        return true;
    if( declaration->getStorageClass() == SC_Static )
        return true;
    string name = declaration->getQualifiedNameAsString();
    if( name.find( "::" ) != string::npos )
        return true;
    if( name.compare( 0, 4, "lcl_" ) != 0 )
        return true;
    insertText( declaration->getLocStart(), "static " );
    return true;
    }

static Plugin::Registration< LclStaticFix > X( "lclstaticfix" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
