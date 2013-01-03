/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "removeforwardstringdecl.hxx"

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

/*
This is a rewriter.

Remove all forward declarations of rtl strings. I.e. 'namespace rtl { class OUString; }' etc.
*/

namespace loplugin
{

RemoveForwardStringDecl::RemoveForwardStringDecl( ASTContext& context, Rewriter& rewriter )
    : RewritePlugin( context, rewriter )
    {
    }

void RemoveForwardStringDecl::run()
    {
    TraverseDecl( context.getTranslationUnitDecl());
    }

bool RemoveForwardStringDecl::VisitNamespaceDecl( NamespaceDecl* declaration )
    {
    if( ignoreLocation( declaration ))
        return true;
    if( declaration->getQualifiedNameAsString() != "rtl" )
        return true;
    bool canRemove = true;
    for( NamespaceDecl::decl_iterator it = declaration->decls_begin();
         it != declaration->decls_end();
         ++it )
        {
        if( *it != NULL )
            {
            if( !tryRemoveStringForwardDecl( *it ))
                canRemove = false;
            }
        }
    if( canRemove ) // contained only forward decls that we removed
        removeText( declaration->getSourceRange(), RemoveLineIfEmpty );
    return true;
    }

bool RemoveForwardStringDecl::tryRemoveStringForwardDecl( const Decl* decl )
    {
    const CXXRecordDecl* classdecl = dyn_cast< CXXRecordDecl >( decl );
    if( classdecl == NULL )
        return false;
    if( !classdecl->isFreeStanding() || classdecl->isCompleteDefinition())
        return false; // not a simple forward declaration
    if( classdecl->getName() == "OString" || classdecl->getName() == "OUString"
        || classdecl->getName() == "OStringBuffer" || classdecl->getName() == "OUStringBuffer"
        || classdecl->getName() == "OStringHash" || classdecl->getName() == "OUStringHash"
        || classdecl->getName() == "OStringLiteral" || classdecl->getName() == "OUStringLiteral" )
        {
        removeText( SourceRange( classdecl->getOuterLocStart(), classdecl->getLocEnd()),
            RemoveLineIfEmpty | RemoveWholeStatement );
        return true;
        }
    return false;
    }

} // namespace
