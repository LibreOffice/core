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

#include "bodynotinblock.hxx"
#include "unusedvariablecheck.hxx"

using namespace clang;

namespace loplugin
{

Plugin::Plugin( ASTContext& context )
    : context( context )
    {
    }

DiagnosticBuilder Plugin::report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc )
    {
    DiagnosticsEngine& diag = context.getDiagnostics();
#if 0
    // Do some mappings (e.g. for -Werror) that clang does not do for custom messages for some reason.
    if( level == DiagnosticsEngine::Warning && diag.getWarningsAsErrors())
        level = DiagnosticsEngine::Error;
    if( level == DiagnosticsEngine::Error && diag.getErrorsAsFatal())
        level = DiagnosticsEngine::Fatal;
#endif
    return diag.Report( loc, diag.getCustomDiagID( level, message ));
    }

bool Plugin::ignoreLocation( SourceLocation loc )
    {
    return context.getSourceManager().isInSystemHeader( context.getSourceManager().getExpansionLoc( loc ));
    }

/**
 Class that manages all LO modules.
*/
class PluginHandler
    : public ASTConsumer
    {
    public:
        explicit PluginHandler( ASTContext& context )
            : rewriter( context.getSourceManager(), context.getLangOpts())
            , bodyNotInBlock( context )
            , unusedVariableCheck( context )
            {
            }
        virtual void HandleTranslationUnit( ASTContext& context )
            {
            if( context.getDiagnostics().hasErrorOccurred())
                return;
            bodyNotInBlock.run();
            unusedVariableCheck.run();
            // TODO also LO header files? or a subdir?
           if( const RewriteBuffer* buf = rewriter.getRewriteBufferFor( context.getSourceManager().getMainFileID()))
                buf->write( llvm::outs());
            // TODO else write out the original file?
            }
    private:
        Rewriter rewriter;
        BodyNotInBlock bodyNotInBlock;
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
