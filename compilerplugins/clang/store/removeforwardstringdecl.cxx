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

#include "removeforwardstringdecl.hxx"

/*
This is a rewriter.

Remove all forward declarations of rtl strings. I.e. 'namespace rtl { class OUString; }' etc.
*/

namespace loplugin
{

RemoveForwardStringDecl::RemoveForwardStringDecl( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
    {
    }

void RemoveForwardStringDecl::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool RemoveForwardStringDecl::VisitNamespaceDecl( const NamespaceDecl* declaration )
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

static Plugin::Registration< RemoveForwardStringDecl > X( "removeforwardstringdecl" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
