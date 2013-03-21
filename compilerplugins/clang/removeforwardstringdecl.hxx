/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef REMOVEFORWARDSTRINGDECL_H
#define REMOVEFORWARDSTRINGDECL_H

#include "plugin.hxx"

namespace loplugin
{

class RemoveForwardStringDecl
    : public RecursiveASTVisitor< RemoveForwardStringDecl >
    , public RewritePlugin
    {
    public:
        explicit RemoveForwardStringDecl( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run();
        bool VisitNamespaceDecl( NamespaceDecl* declaration );
    private:
        bool tryRemoveStringForwardDecl( const Decl* decl );
    };

} // namespace

#endif // REMOVEFORWARDSTRINGDECL_H

