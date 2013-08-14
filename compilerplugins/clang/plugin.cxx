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

#include <clang/Basic/FileManager.h>
#include <clang/Lex/Lexer.h>

#include "pluginhandler.hxx"

/*
Base classes for plugin actions.
*/
namespace loplugin
{

Plugin::Plugin( CompilerInstance& compiler )
    : compiler( compiler )
    {
    }

DiagnosticBuilder Plugin::report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc )
    {
    return report( level, message, compiler, loc );
    }

DiagnosticBuilder Plugin::report( DiagnosticsEngine::Level level, StringRef message, CompilerInstance& compiler,
    SourceLocation loc )
    {
    DiagnosticsEngine& diag = compiler.getDiagnostics();
    // Do some mappings (e.g. for -Werror) that clang does not do for custom messages for some reason.
    if( level == DiagnosticsEngine::Warning && diag.getWarningsAsErrors())
        level = DiagnosticsEngine::Error;
    if( level == DiagnosticsEngine::Error && diag.getErrorsAsFatal())
        level = DiagnosticsEngine::Fatal;
    string fullMessage = ( message + " [loplugin]" ).str();
    if( loc.isValid())
        return diag.Report( loc, diag.getCustomDiagID( level, fullMessage ));
    else
        return diag.Report( diag.getCustomDiagID( level, fullMessage ));
    }

bool Plugin::ignoreLocation( SourceLocation loc )
    {
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( loc );
    if( compiler.getSourceManager().isInSystemHeader( expansionLoc ))
        return true;
    const char* bufferName = compiler.getSourceManager().getPresumedLoc( expansionLoc ).getFilename();
    if( bufferName == NULL )
        return true;
    if( strncmp( bufferName, OUTDIR, strlen( OUTDIR )) == 0
        || strncmp( bufferName, WORKDIR, strlen( WORKDIR )) == 0
        || strncmp( bufferName, BUILDDIR, strlen( BUILDDIR )) == 0
        || strncmp( bufferName, SRCDIR, strlen( SRCDIR )) == 0 )
        return false; // ok
    return true;
    }

void Plugin::registerPlugin( Plugin* (*create)( CompilerInstance&, Rewriter& ), const char* optionName, bool isRewriter, bool isPPCallback )
    {
    PluginHandler::registerPlugin( create, optionName, isRewriter, isPPCallback );
    }

unordered_map< const Stmt*, const Stmt* > Plugin::parents;

const Stmt* Plugin::parentStmt( const Stmt* stmt )
    {
    if( parents.empty())
        buildParents( compiler );
    assert( parents.count( stmt ) == 1 );
    return parents[ stmt ];
    }

Stmt* Plugin::parentStmt( Stmt* stmt )
    {
    if( parents.empty())
        buildParents( compiler );
    assert( parents.count( stmt ) == 1 );
    return const_cast< Stmt* >( parents[ stmt ] );
    }

namespace
{
class ParentBuilder
    : public RecursiveASTVisitor< ParentBuilder >
    {
    public:
        bool VisitFunctionDecl( const FunctionDecl* function );
        void walk( const Stmt* stmt );
        unordered_map< const Stmt*, const Stmt* >* parents;
    };

bool ParentBuilder::VisitFunctionDecl( const FunctionDecl* function )
    {
//    if( ignoreLocation( declaration ))
//        return true; ???
    if( function->doesThisDeclarationHaveABody())
        {
        const Stmt* body = function->getBody();
        (*parents)[ body ] = NULL; // no parent
        walk( body );
        }
    if( const CXXConstructorDecl* ctor = dyn_cast< CXXConstructorDecl >( function ))
        {
        for( CXXConstructorDecl::init_const_iterator it = ctor->init_begin();
             it != ctor->init_end();
             ++it )
            {
            const Expr* init_expression = (*it)->getInit();
            (*parents)[ init_expression ] = NULL;
            walk( init_expression );
            }
        }
    return true;
    }

void ParentBuilder::walk( const Stmt* stmt )
    {
    for( ConstStmtIterator it = stmt->child_begin();
         it != stmt->child_end();
         ++it )
        {
        if( *it != NULL )
            {
            (*parents)[ *it ] = stmt;
            walk( *it );
            }
        }
    }

} // namespace

void Plugin::buildParents( CompilerInstance& compiler )
    {
    assert( parents.empty());
    ParentBuilder builder;
    builder.parents = &parents;
    builder.TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

/////

RewritePlugin::RewritePlugin( CompilerInstance& compiler, Rewriter& rewriter )
    : Plugin( compiler )
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
    CharSourceRange range( SourceRange( Start, Start.getLocWithOffset( Length )), false );
    return removeText( range, opts );
    }

bool RewritePlugin::removeText( SourceRange range, RewriteOptions opts )
    {
    return removeText( CharSourceRange( range, true ), opts );
    }

bool RewritePlugin::removeText( CharSourceRange range, RewriteOptions opts )
    {
    if( removals.find( range.getBegin()) != removals.end())
        report( DiagnosticsEngine::Warning, "double code removal, possible plugin error", range.getBegin());
    removals.insert( range.getBegin());
    if( opts.flags & RemoveWholeStatement || opts.flags & RemoveAllWhitespace )
        {
        if( !adjustRangeForOptions( &range, opts ))
            return reportEditFailure( range.getBegin());
        }
    if( rewriter.RemoveText( range, opts ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::adjustRangeForOptions( CharSourceRange* range, RewriteOptions opts )
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
    SourceLocation locationEnd = range->getEnd();
    if( range->isTokenRange())
        locationEnd = Lexer::getLocForEndOfToken( locationEnd, 0, compiler.getSourceManager(), compiler.getLangOpts());
    const char* endBuf = SM.getCharacterData( locationEnd, &invalid );
    if( invalid )
        return false;
    const char* startPos = startBuf;
    --startPos;
    while( startPos >= fileBuf && ( *startPos == ' ' || *startPos == '\t' ))
        --startPos;
    if( startPos >= fileBuf && *startPos == '\n' )
        startPos = startBuf - 1; // do not remove indentation whitespace (RemoveLineIfEmpty can do that)
    const char* endPos = endBuf;
    while( *endPos == ' ' || *endPos == '\t' )
        ++endPos;
    if( opts.flags & RemoveWholeStatement )
        {
        if( *endPos == ';' )
            ++endPos;
        else
            return false;
        }
    *range = CharSourceRange( SourceRange( range->getBegin().getLocWithOffset( startPos - startBuf + 1 ),
        locationEnd.getLocWithOffset( endPos - endBuf )), false );
    return true;
    }

bool RewritePlugin::replaceText( SourceLocation Start, unsigned OrigLength, StringRef NewStr )
    {
    if( OrigLength != 0 && removals.find( Start ) != removals.end())
        report( DiagnosticsEngine::Warning, "double code replacement, possible plugin error", Start );
    removals.insert( Start );
    if( rewriter.ReplaceText( Start, OrigLength, NewStr ))
        return reportEditFailure( Start );
    return true;
    }

bool RewritePlugin::replaceText( SourceRange range, StringRef NewStr )
    {
    if( removals.find( range.getBegin()) != removals.end())
        report( DiagnosticsEngine::Warning, "double code replacement, possible plugin error", range.getBegin());
    removals.insert( range.getBegin());
    if( rewriter.ReplaceText( range, NewStr ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::replaceText( SourceRange range, SourceRange replacementRange )
    {
    if( removals.find( range.getBegin()) != removals.end())
        report( DiagnosticsEngine::Warning, "double code replacement, possible plugin error", range.getBegin());
    removals.insert( range.getBegin());
    if( rewriter.ReplaceText( range, replacementRange ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::reportEditFailure( SourceLocation loc )
    {
    report( DiagnosticsEngine::Warning, "cannot perform source modification (macro expansion involved?)", loc );
    return false;
    }

} // namespace
