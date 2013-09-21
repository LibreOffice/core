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

/*
This is technically a rewriter, but it actually only generates data about code.

This is incomplete.

Checks for all function declarations for whether they are used or not. This information
should be output to files and in a second pass it should be checked (by another tool)
which functions are never used.
*/

#include "plugin.hxx"

namespace loplugin
{

class UnusedCode
    : public RecursiveASTVisitor< UnusedCode >
    , public RewritePlugin
    {
    public:
        explicit UnusedCode( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run() override;
        bool VisitFunctionDecl( const FunctionDecl* declaration );
    };

UnusedCode::UnusedCode( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
    {
    }

void UnusedCode::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool UnusedCode::VisitFunctionDecl( const FunctionDecl* declaration )
    {
    if( ignoreLocation( declaration ))
        return true;
    bool isUsed = declaration->isUsed();
    if( const CXXMethodDecl* cxxmethod = dyn_cast< CXXMethodDecl >( declaration ))
        {
        if( !isUsed && cxxmethod->isVirtual())
            { // Virtual methods are used also if a method they override is used.
            for( CXXMethodDecl::method_iterator it = cxxmethod->begin_overridden_methods();
                 it != cxxmethod->end_overridden_methods();
                 ++it )
                {
                if( (*it)->isUsed())
                    {
                    isUsed = true;
                    break;
                    }
                }
            }
        }
    // Fully qualified name: declaration->getQualifiedNameAsString()
    // Is used: isUsed
    // The main source file compiled: compiler.getSourceManager().getFileEntryForID( compiler.getSourceManager().getMainFileID())->getName()
    return true;
    }

static Plugin::Registration< UnusedCode > X( "unusedcode" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
