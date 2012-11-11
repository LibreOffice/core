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

#include <clang/Basic/SourceManager.h>

/*
This is a rewriter.

Check all lcl_ functions and prepend static if needed.
*/

namespace loplugin
{

LclStaticFix::LclStaticFix( ASTContext& context, Rewriter& rewriter )
    : RewritePlugin( context, rewriter )
    {
    }

void LclStaticFix::run()
    {
    TraverseDecl( context.getTranslationUnitDecl());
    }

bool LclStaticFix::VisitFunctionDecl( FunctionDecl* declaration )
    {
    // TODO also LO header files? or a subdir?
    // Only the .cxx file can be normally edited ... ?
    if( !context.getSourceManager().isFromMainFile( declaration->getLocStart()))
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

} // namespace
