/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "plugin.hxx"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/FileManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <stdio.h>
#include <unistd.h>

#include "bodynotinblock.hxx"
#include "lclstaticfix.hxx"
#include "postfixincrementfix.hxx"
#include "removeforwardstringdecl.hxx"
#include "sallogareas.hxx"
#include "unusedvariablecheck.hxx"

#include <config_clang.h>

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
    if( loc.isValid())
        return diag.Report( loc, diag.getCustomDiagID( level, message ));
    else
        return diag.Report( diag.getCustomDiagID( level, message ));
    }

bool Plugin::ignoreLocation( SourceLocation loc )
    {
    SourceLocation expansionLoc = context.getSourceManager().getExpansionLoc( loc );
    if( context.getSourceManager().isInSystemHeader( expansionLoc ))
        return true;
    bool invalid;
    const char* bufferName = context.getSourceManager().getBufferName( expansionLoc, &invalid );
    if( invalid )
        return true;
    if( strncmp( bufferName, OUTDIR, strlen( OUTDIR )) == 0
        || strncmp( bufferName, WORKDIR, strlen( WORKDIR )) == 0
        || strncmp( bufferName, BUILDDIR, strlen( BUILDDIR )) == 0
        || strncmp( bufferName, SRCDIR, strlen( SRCDIR )) == 0 )
        return false; // ok
    return true;
    }


RewritePlugin::RewritePlugin( ASTContext& context, Rewriter& rewriter )
    : Plugin( context )
    , rewriter( rewriter )
    {
    }

bool RewritePlugin::insertText( SourceLocation Loc, StringRef Str, bool InsertAfter, bool indentNewLines )
    {
    if( rewriter.InsertText( Loc, Str, InsertAfter, indentNewLines ))
        return reportEditFailure( Loc );
    return true;
    }

bool RewritePlugin::insertTextAfter( SourceLocation Loc, StringRef Str )
    {
    if( rewriter.InsertTextAfter( Loc, Str ))
        return reportEditFailure( Loc );
    return true;
    }

bool RewritePlugin::insertTextAfterToken( SourceLocation Loc, StringRef Str )
    {
    if( rewriter.InsertTextAfterToken( Loc, Str ))
        return reportEditFailure( Loc );
    return true;
    }

bool RewritePlugin::insertTextBefore( SourceLocation Loc, StringRef Str )
    {
    if( rewriter.InsertTextBefore( Loc, Str ))
        return reportEditFailure( Loc );
    return true;
    }

bool RewritePlugin::removeText( SourceLocation Start, unsigned Length, RewriteOptions opts )
    {
    return removeText( SourceRange( Start, Start.getLocWithOffset( Length )), opts );
    }

bool RewritePlugin::removeText( SourceRange range, RewriteOptions opts )
    {
    if( opts.RemoveWholeStatement )
        {
        if( !adjustForWholeStatement( &range ))
            return reportEditFailure( range.getBegin());
        }
    if( rewriter.RemoveText( range, opts ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::adjustForWholeStatement( SourceRange* range )
    {
    SourceManager& SM = rewriter.getSourceMgr();
    SourceLocation fileStartLoc = SM.getLocForStartOfFile( SM.getFileID( range->getBegin()));
    if( fileStartLoc.isInvalid())
        return false;
    bool invalid = false;
    const char* fileBuf = SM.getCharacterData( fileStartLoc, &invalid );
    if( invalid )
        return false;
    const char* startBuf = SM.getCharacterData( range->getBegin(), &invalid );
    if( invalid )
        return false;
    const char* endBuf = SM.getCharacterData( range->getEnd(), &invalid );
    if( invalid )
        return false;
    const char* startSpacePos = startBuf;
    // do not skip \n here, RemoveLineIfEmpty can take care of that
    --startSpacePos;
    while( startSpacePos >= fileBuf && ( *startSpacePos == ' ' || *startSpacePos == '\t' ))
        --startSpacePos;
    const char* semiPos = strchr( endBuf, ';' );
    if( semiPos == NULL )
        return false;
    *range = SourceRange( range->getBegin().getLocWithOffset( startSpacePos - startBuf + 1 ),
        range->getEnd().getLocWithOffset( semiPos - endBuf + 1 ));
    return true;
    }

bool RewritePlugin::replaceText( SourceLocation Start, unsigned OrigLength, StringRef NewStr )
    {
    if( rewriter.ReplaceText( Start, OrigLength, NewStr ))
        return reportEditFailure( Start );
    return true;
    }

bool RewritePlugin::replaceText( SourceRange range, StringRef NewStr )
    {
    if( rewriter.ReplaceText( range, NewStr ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::replaceText( SourceRange range, SourceRange replacementRange )
    {
    if( rewriter.ReplaceText( range, replacementRange ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::reportEditFailure( SourceLocation loc )
    {
    DiagnosticsEngine& diag = context.getDiagnostics();
    diag.Report( loc, diag.getCustomDiagID( DiagnosticsEngine::Warning,
        "cannot perform source modification (macro expansion involved?) [loplugin]" ));
    return false;
    }


/**
 Class that manages all LO modules.
*/
class PluginHandler
    : public ASTConsumer
    {
    public:
        explicit PluginHandler( ASTContext& context, const vector< string >& args )
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
        virtual void HandleTranslationUnit( ASTContext& context )
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
                        int moduleEnd = filename.find( '/', modulePos );
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
                else if( strncmp( e->getName(), BUILDDIR, strlen( BUILDDIR )) == 0 )
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
    private:
        bool isArg( const char* arg ) const
            {
            return find( args.begin(), args.end(), arg ) != args.end();
            }
        Rewriter rewriter;
        vector< string > args;
        BodyNotInBlock bodyNotInBlock;
        LclStaticFix lclStaticFix;
        PostfixIncrementFix postfixIncrementFix;
        RemoveForwardStringDecl removeForwardStringDecl;
        SalLogAreas salLogAreas;
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
            return new PluginHandler( Compiler.getASTContext(), _args );
            }
        virtual bool ParseArgs( const CompilerInstance& CI, const vector< string >& args )
            {
            _args = args;
            return true;
            }
    private:
        vector< string > _args;
    };

} // namespace

static FrontendPluginRegistry::Add< loplugin::LibreOfficeAction > X( "loplugin", "LibreOffice compile check plugin" );
