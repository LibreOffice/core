/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "pluginhandler.hxx"

#include <clang/AST/ASTContext.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <stdio.h>
#include <unistd.h>

namespace loplugin
{

PluginHandler::PluginHandler( ASTContext& context, const vector< string >& args )
    : rewriter( context.getSourceManager(), context.getLangOpts())
    , args( args )
    , bodyNotInBlock( context )
    , lclStaticFix( context, rewriter )
    , postfixIncrementFix( context, rewriter )
    , removeForwardStringDecl( context, rewriter )
    , salLogAreas( context )
    , unusedVariableCheck( context )
    {
    }

void PluginHandler::HandleTranslationUnit( ASTContext& context )
    {
    if( context.getDiagnostics().hasErrorOccurred())
        return;
    if( isArg( "lclstaticfix" ))
        lclStaticFix.run();
    else if( isArg( "postfixincrementfix" ))
        postfixIncrementFix.run();
    else if( isArg( "removeforwardstringdecl" ))
        removeForwardStringDecl.run();
    else if( args.empty())
        {
        bodyNotInBlock.run();
        salLogAreas.run();
        unusedVariableCheck.run();
        }
    else
        {
        DiagnosticsEngine& diag = context.getDiagnostics();
        diag.Report( diag.getCustomDiagID( DiagnosticsEngine::Fatal,
            "unknown plugin tool %0 [loplugin]" )) << args.front();
        }
    for( Rewriter::buffer_iterator it = rewriter.buffer_begin();
         it != rewriter.buffer_end();
         ++it )
        {
        const FileEntry* e = context.getSourceManager().getFileEntryForID( it->first );
        DiagnosticsEngine& diag = context.getDiagnostics();
        /* Check where the file actually is, and warn about cases where modification
           most probably doesn't matter (generated files in workdir).
           The order here is important, as OUTDIR and WORKDIR are often in SRCDIR/BUILDDIR,
           and BUILDDIR is sometimes in SRCDIR. */
        string modifyFile;
        if( strncmp( e->getName(), OUTDIR, strlen( OUTDIR )) == 0 )
            {
            /* Try to find a matching file for a file in solver/ (include files
               are usually included from there rather than from the source dir) if possible. */
            if( strncmp( e->getName(), OUTDIR "/inc/", strlen( OUTDIR ) + strlen( "/inc/" )) == 0 )
                {
                string filename( e->getName());
                int modulePos = strlen( OUTDIR ) + strlen( "/inc/" );
                size_t moduleEnd = filename.find( '/', modulePos );
                if( moduleEnd != string::npos )
                    {
                    modifyFile = SRCDIR "/" + filename.substr( modulePos, moduleEnd - modulePos )
                        + "/inc/" + filename.substr( modulePos );
                    }
                }
            if( modifyFile.empty())
                diag.Report( diag.getCustomDiagID( DiagnosticsEngine::Warning,
                    "modified source in solver/ : %0 [loplugin]" )) << e->getName();
            }
        else if( strncmp( e->getName(), WORKDIR, strlen( WORKDIR )) == 0 )
            diag.Report( diag.getCustomDiagID( DiagnosticsEngine::Warning,
                "modified source in workdir/ : %0 [loplugin]" )) << e->getName();
        else if( strcmp( SRCDIR, BUILDDIR ) != 0 && strncmp( e->getName(), BUILDDIR, strlen( BUILDDIR )) == 0 )
            diag.Report( diag.getCustomDiagID( DiagnosticsEngine::Warning,
                "modified source in build dir : %0 [loplugin]" )) << e->getName();
        else if( strncmp( e->getName(), SRCDIR, strlen( SRCDIR )) == 0 )
            ; // ok
        else
            {
            diag.Report( diag.getCustomDiagID( DiagnosticsEngine::Warning,
                "modified source in unknown location, not modifying : %0 [loplugin]" )) << e->getName();
            continue; // --->
            }
        if( modifyFile.empty())
            modifyFile = e->getName();
        char* filename = new char[ modifyFile.length() + 100 ];
        sprintf( filename, "%s.new.%d", modifyFile.c_str(), getpid());
        string error;
        bool ok = false;
        raw_fd_ostream ostream( filename, error );
        if( error.empty())
            {
            it->second.write( ostream );
            ostream.close();
            if( !ostream.has_error() && rename( filename, modifyFile.c_str()) == 0 )
                ok = true;
            }
        ostream.clear_error();
        unlink( filename );
        if( !ok )
            diag.Report( diag.getCustomDiagID( DiagnosticsEngine::Error,
                "cannot write modified source to %0 (%1) [loplugin]" )) << modifyFile << error;
        delete[] filename;
        }
    }

ASTConsumer* LibreOfficeAction::CreateASTConsumer( CompilerInstance& Compiler, StringRef InFile )
    {
    return new PluginHandler( Compiler.getASTContext(), _args );
    }

bool LibreOfficeAction::ParseArgs( const CompilerInstance& CI, const vector< string >& args )
    {
    _args = args;
    return true;
    }


static FrontendPluginRegistry::Add< loplugin::LibreOfficeAction > X( "loplugin", "LibreOffice compile check plugin" );

} // namespace
