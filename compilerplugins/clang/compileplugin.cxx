/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "compileplugin.hxx"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Rewrite/Rewriter.h>

#include "unusedvariablecheck.hxx"

using namespace clang;

namespace loplugin
{

/**
 Class that manages all LO modules.
*/
class PluginHandler
    : public ASTConsumer
    {
    public:
        explicit PluginHandler( ASTContext& context )
            : rewriter( context.getSourceManager(), context.getLangOpts())
            , unusedVariableCheck( context )
            {
            }
        virtual void HandleTranslationUnit( ASTContext& context )
            {
            if( context.getDiagnostics().hasErrorOccurred())
                return;
            unusedVariableCheck.run();
            // TODO also LO header files? or a subdir?
           if( const RewriteBuffer* buf = rewriter.getRewriteBufferFor( context.getSourceManager().getMainFileID()))
                buf->write( llvm::outs());
            // TODO else write out the original file?
            }
    private:
        Rewriter rewriter;
        UnusedVariableCheck unusedVariableCheck;
    };

/**
 The Clang plugin class, just forwards to PluginHandler.
*/
class LibreOfficeAction
    : public PluginASTAction
    {
    public:
        virtual ASTConsumer* CreateASTConsumer( CompilerInstance& Compiler, StringRef InFile )
            {
            return new PluginHandler( Compiler.getASTContext());
            }
        virtual bool ParseArgs( const CompilerInstance& CI, const std::vector< std::string >& args )
            {
            return true;
            }
    };

} // namespace

static FrontendPluginRegistry::Add< loplugin::LibreOfficeAction > X( "loplugin", "LibreOffice compile check plugin" );
