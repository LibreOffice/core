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

#include "plugin.hxx"

#include <cassert>

#include <clang/Basic/FileManager.h>
#include <clang/Lex/Lexer.h>

#include "pluginhandler.hxx"
#include "compat.hxx"

/*
Base classes for plugin actions.
*/
namespace loplugin
{

Plugin::Plugin( const InstantiationData& data )
    : compiler( data.compiler ), handler( data.handler ), name( data.name )
    {
    }

DiagnosticBuilder Plugin::report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc ) const
    {
    return handler.report( level, name, message, compiler, loc );
    }

bool Plugin::ignoreLocation( SourceLocation loc )
    {
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( loc );
    if( compiler.getSourceManager().isInSystemHeader( expansionLoc ))
        return true;
    const char* bufferName = compiler.getSourceManager().getPresumedLoc( expansionLoc ).getFilename();
    if( bufferName == NULL
        || strncmp( bufferName, WORKDIR, strlen( WORKDIR )) == 0 )
        return true;
    if( strncmp( bufferName, BUILDDIR, strlen( BUILDDIR )) == 0
        || strncmp( bufferName, SRCDIR, strlen( SRCDIR )) == 0 )
        return false; // ok
    return true;
    }

void Plugin::registerPlugin( Plugin* (*create)( const InstantiationData& ), const char* optionName, bool isPPCallback, bool byDefault )
    {
    PluginHandler::registerPlugin( create, optionName, isPPCallback, byDefault );
    }

unordered_map< const Stmt*, const Stmt* > Plugin::parents;

const Stmt* Plugin::parentStmt( const Stmt* stmt )
    {
    if( parents.empty())
        buildParents( compiler );
if(parents.count(stmt)!=1)stmt->dump();
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


bool Plugin::isInUnoIncludeFile(SourceLocation spellingLocation) const {
    StringRef name {
        compiler.getSourceManager().getFilename(spellingLocation) };
    return compat::isInMainFile(compiler.getSourceManager(), spellingLocation)
        ? (name == SRCDIR "/cppu/source/cppu/compat.cxx"
           || name == SRCDIR "/cppuhelper/source/compat.cxx"
           || name == SRCDIR "/sal/osl/all/compat.cxx")
        : (name.startswith(SRCDIR "/include/com/")
           || name.startswith(SRCDIR "/include/cppu/")
           || name.startswith(SRCDIR "/include/cppuhelper/")
           || name.startswith(SRCDIR "/include/osl/")
           || name.startswith(SRCDIR "/include/rtl/")
           || name.startswith(SRCDIR "/include/sal/")
           || name.startswith(SRCDIR "/include/salhelper/")
           || name.startswith(SRCDIR "/include/systools/")
           || name.startswith(SRCDIR "/include/typelib/")
           || name.startswith(SRCDIR "/include/uno/")
           || name.startswith(WORKDIR "/"));
}

namespace
{
class ParentBuilder
    : public RecursiveASTVisitor< ParentBuilder >
    {
    public:
        bool VisitFunctionDecl( const FunctionDecl* function );
        bool VisitObjCMethodDecl( const ObjCMethodDecl* method );
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

bool ParentBuilder::VisitObjCMethodDecl( const ObjCMethodDecl* method )
    {
//    if( ignoreLocation( declaration ))
//        return true; ???
    if( method->hasBody())
        {
        const Stmt* body = method->getBody();
        (*parents)[ body ] = NULL; // no parent
        walk( body );
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

SourceLocation Plugin::locationAfterToken( SourceLocation location )
    {
    return Lexer::getLocForEndOfToken( location, 0, compiler.getSourceManager(), compiler.getLangOpts());
    }

RewritePlugin::RewritePlugin( const InstantiationData& data )
    : Plugin( data )
    , rewriter( data.rewriter )
    {
    }

bool RewritePlugin::insertText( SourceLocation Loc, StringRef Str, bool InsertAfter, bool indentNewLines )
    {
    assert( rewriter );
    if( rewriter->InsertText( Loc, Str, InsertAfter, indentNewLines ))
        return reportEditFailure( Loc );
    return true;
    }

bool RewritePlugin::insertTextAfter( SourceLocation Loc, StringRef Str )
    {
    assert( rewriter );
    if( rewriter->InsertTextAfter( Loc, Str ))
        return reportEditFailure( Loc );
    return true;
    }

bool RewritePlugin::insertTextAfterToken( SourceLocation Loc, StringRef Str )
    {
    assert( rewriter );
    if( rewriter->InsertTextAfterToken( Loc, Str ))
        return reportEditFailure( Loc );
    return true;
    }

bool RewritePlugin::insertTextBefore( SourceLocation Loc, StringRef Str )
    {
    assert( rewriter );
    if( rewriter->InsertTextBefore( Loc, Str ))
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
    assert( rewriter );
    if( rewriter->getRangeSize( range, opts ) == -1 )
        return reportEditFailure( range.getBegin());
    if( !handler.addRemoval( range.getBegin() ) )
        {
        report( DiagnosticsEngine::Warning, "double code removal, possible plugin error", range.getBegin());
        return true;
        }
    if( opts.flags & RemoveWholeStatement || opts.flags & RemoveAllWhitespace )
        {
        if( !adjustRangeForOptions( &range, opts ))
            return reportEditFailure( range.getBegin());
        }
    if( rewriter->RemoveText( range, opts ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::adjustRangeForOptions( CharSourceRange* range, RewriteOptions opts )
    {
    assert( rewriter );
    SourceManager& SM = rewriter->getSourceMgr();
    SourceLocation fileStartLoc = SM.getLocForStartOfFile( SM.getFileID( range->getBegin()));
    if( fileStartLoc.isInvalid())
        return false;
    bool isInvalid = false;
    const char* fileBuf = SM.getCharacterData( fileStartLoc, &isInvalid );
    if( isInvalid )
        return false;
    const char* startBuf = SM.getCharacterData( range->getBegin(), &isInvalid );
    if( isInvalid )
        return false;
    SourceLocation locationEnd = range->getEnd();
    if( range->isTokenRange())
        locationEnd = locationAfterToken( locationEnd );
    const char* endBuf = SM.getCharacterData( locationEnd, &isInvalid );
    if( isInvalid )
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
    assert( rewriter );
    if( OrigLength != 0 && !handler.addRemoval( Start ) )
        {
        report( DiagnosticsEngine::Warning, "double code replacement, possible plugin error", Start );
        return true;
        }
    if( rewriter->ReplaceText( Start, OrigLength, NewStr ))
        return reportEditFailure( Start );
    return true;
    }

bool RewritePlugin::replaceText( SourceRange range, StringRef NewStr )
    {
    assert( rewriter );
    if( rewriter->getRangeSize( range ) == -1 )
        return reportEditFailure( range.getBegin());
    if( !handler.addRemoval( range.getBegin() ) )
        {
        report( DiagnosticsEngine::Warning, "double code replacement, possible plugin error", range.getBegin());
        return true;
        }
    if( rewriter->ReplaceText( range, NewStr ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::replaceText( SourceRange range, SourceRange replacementRange )
    {
    assert( rewriter );
    if( rewriter->getRangeSize( range ) == -1 )
        return reportEditFailure( range.getBegin());
    if( !handler.addRemoval( range.getBegin() ) )
        {
        report( DiagnosticsEngine::Warning, "double code replacement, possible plugin error", range.getBegin());
        return true;
        }
    if( rewriter->ReplaceText( range, replacementRange ))
        return reportEditFailure( range.getBegin());
    return true;
    }

bool RewritePlugin::reportEditFailure( SourceLocation loc )
    {
    report( DiagnosticsEngine::Warning, "cannot perform source modification (macro expansion involved?)", loc );
    return false;
    }

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
