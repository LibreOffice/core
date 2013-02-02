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

#include <clang/AST/ASTContext.h>
#include <clang/Basic/FileManager.h>

#include "pluginhandler.hxx"

/*
Base classes for plugin actions.
*/
namespace loplugin
{

Plugin::Plugin( ASTContext& context )
    : context( context )
    {
    }

DiagnosticBuilder Plugin::report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc )
    {
    return report( level, message, context, loc );
    }

DiagnosticBuilder Plugin::report( DiagnosticsEngine::Level level, StringRef message, ASTContext& context,
    SourceLocation loc )
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
    const char* bufferName = context.getSourceManager().getPresumedLoc( expansionLoc ).getFilename();
    if( bufferName == NULL )
        return true;
    if( strncmp( bufferName, OUTDIR, strlen( OUTDIR )) == 0
        || strncmp( bufferName, WORKDIR, strlen( WORKDIR )) == 0
        || strncmp( bufferName, BUILDDIR, strlen( BUILDDIR )) == 0
        || strncmp( bufferName, SRCDIR, strlen( SRCDIR )) == 0 )
        return false; // ok
    return true;
    }

void Plugin::registerPlugin( Plugin* (*create)( ASTContext&, Rewriter& ), const char* optionName, bool isRewriter )
    {
    PluginHandler::registerPlugin( create, optionName, isRewriter );
    }

/////

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
    report( DiagnosticsEngine::Warning, "cannot perform source modification (macro expansion involved?) [loplugin]", loc );
    return false;
    }

} // namespace
