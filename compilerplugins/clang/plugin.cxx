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
#include <cstddef>
#include <string>

#include <clang/Basic/FileManager.h>
#include <clang/Lex/Lexer.h>

#include "pluginhandler.hxx"

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

void normalizeDotDotInFilePath( std::string & s )
{
    for (std::string::size_type i = 0;;)
    {
        i = s.find("/.", i);
        if (i == std::string::npos) {
            break;
        }
        if (i + 2 == s.length() || s[i + 2] == '/') {
            s.erase(i, 2); // [AAA]/.[/CCC] -> [AAA][/CCC]
        } else if (s[i + 2] == '.'
                   && (i + 3 == s.length() || s[i + 3] == '/'))
        {
            if (i == 0) { // /..[/CCC] -> /..[/CCC]
                break;
            }
            auto j = s.rfind('/', i - 1);
            if (j == std::string::npos)
            {
                // BBB/..[/CCC] -> BBB/..[/CCC] (instead of BBB/../CCC ->
                // CCC, to avoid wrong ../../CCC -> CCC; relative paths
                // shouldn't happen anyway, and even if they did, wouldn't
                // match against WORKDIR anyway, as WORKDIR should be
                // absolute):
                break;
            }
            s.erase(j, i + 3 - j); // AAA/BBB/..[/CCC] -> AAA[/CCC]
            i = j;
        } else {
            i += 2;
        }
    }
}

void Plugin::registerPlugin( Plugin* (*create)( const InstantiationData& ), const char* optionName, bool isPPCallback, bool byDefault )
{
    PluginHandler::registerPlugin( create, optionName, isPPCallback, byDefault );
}

const Stmt* Plugin::getParentStmt( const Stmt* stmt )
{
    auto parentsRange = compiler.getASTContext().getParents(*stmt);
    if ( parentsRange.begin() == parentsRange.end())
        return nullptr;
    return parentsRange.begin()->get<Stmt>();
}

Stmt* Plugin::getParentStmt( Stmt* stmt )
{
    auto parentsRange = compiler.getASTContext().getParents(*stmt);
    if ( parentsRange.begin() == parentsRange.end())
        return nullptr;
    return const_cast<Stmt*>(parentsRange.begin()->get<Stmt>());
}

static const Decl* getDeclContext(ASTContext& context, const Stmt* stmt)
{
    auto it = context.getParents(*stmt).begin();

    if (it == context.getParents(*stmt).end())
          return nullptr;

    const Decl *aDecl = it->get<Decl>();
    if (aDecl)
          return aDecl;

    const Stmt *aStmt = it->get<Stmt>();
    if (aStmt)
        return getDeclContext(context, aStmt);

    return nullptr;
}

const FunctionDecl* Plugin::getParentFunctionDecl( const Stmt* stmt )
{
    const Decl *decl = getDeclContext(compiler.getASTContext(), stmt);
    if (decl)
        return static_cast<const FunctionDecl*>(decl->getNonClosureContext());

    return nullptr;
}


bool Plugin::isInUnoIncludeFile(SourceLocation spellingLocation) const
{
    StringRef name {
        compiler.getSourceManager().getFilename(spellingLocation) };
    return compiler.getSourceManager().isInMainFile(spellingLocation)
        ? (isSamePathname(name, SRCDIR "/cppu/source/cppu/compat.cxx")
           || isSamePathname(name, SRCDIR "/cppuhelper/source/compat.cxx")
           || isSamePathname(name, SRCDIR "/sal/osl/all/compat.cxx"))
        : (hasPathnamePrefix(name, SRCDIR "/include/com/")
           || hasPathnamePrefix(name, SRCDIR "/include/cppu/")
           || hasPathnamePrefix(name, SRCDIR "/include/cppuhelper/")
           || hasPathnamePrefix(name, SRCDIR "/include/osl/")
           || hasPathnamePrefix(name, SRCDIR "/include/rtl/")
           || hasPathnamePrefix(name, SRCDIR "/include/sal/")
           || hasPathnamePrefix(name, SRCDIR "/include/salhelper/")
           || hasPathnamePrefix(name, SRCDIR "/include/systools/")
           || hasPathnamePrefix(name, SRCDIR "/include/typelib/")
           || hasPathnamePrefix(name, SRCDIR "/include/uno/"));
}

bool Plugin::isInUnoIncludeFile(const FunctionDecl* functionDecl) const
{
    return isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
             functionDecl->getCanonicalDecl()->getNameInfo().getLoc()));
}

SourceLocation Plugin::locationAfterToken( SourceLocation location )
{
    return Lexer::getLocForEndOfToken( location, 0, compiler.getSourceManager(), compiler.getLangOpts());
}

bool Plugin::isUnitTestMode()
{
    return PluginHandler::isUnitTestMode();
}

bool Plugin::containsPreprocessingConditionalInclusion(SourceRange range)
{
    auto const begin = compiler.getSourceManager().getExpansionLoc(
        range.getBegin());
    auto const end = compiler.getSourceManager().getExpansionLoc(
        range.getEnd());
    assert(begin.isFileID() && end.isFileID());
    if (!(begin == end
          || compiler.getSourceManager().isBeforeInTranslationUnit(
              begin, end)))
    {
        // Conservatively assume "yes" if lexing fails (e.g., due to
        // macros):
        return true;
    }
    auto hash = false;
    for (auto loc = begin;;) {
        Token tok;
        if (Lexer::getRawToken(
                loc, tok, compiler.getSourceManager(),
                compiler.getLangOpts(), true))
        {
            // Conservatively assume "yes" if lexing fails (e.g., due to
            // macros):
            return true;
        }
        if (hash && tok.is(tok::raw_identifier)) {
            auto const id = tok.getRawIdentifier();
            if (id == "if" || id == "ifdef" || id == "ifndef"
                || id == "elif" || id == "else" || id == "endif")
            {
                return true;
            }
        }
        if (loc == range.getEnd()) {
            break;
        }
        hash = tok.is(tok::hash) && tok.isAtStartOfLine();
        loc = loc.getLocWithOffset(
            std::max<unsigned>(
                Lexer::MeasureTokenLength(
                    loc, compiler.getSourceManager(),
                    compiler.getLangOpts()),
                1));
    }
    return false;
}

RewritePlugin::RewritePlugin( const InstantiationData& data )
    : Plugin( data )
    , rewriter( data.rewriter )
{
}

bool RewritePlugin::insertText( SourceLocation Loc, StringRef Str, bool InsertAfter, bool indentNewLines )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(Loc))
        return false;
    if( rewriter->InsertText( Loc, Str, InsertAfter, indentNewLines ))
        return reportEditFailure( Loc );
    return true;
}

bool RewritePlugin::insertTextAfter( SourceLocation Loc, StringRef Str )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(Loc))
        return false;
    if( rewriter->InsertTextAfter( Loc, Str ))
        return reportEditFailure( Loc );
    return true;
}

bool RewritePlugin::insertTextAfterToken( SourceLocation Loc, StringRef Str )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(Loc))
        return false;
    if( rewriter->InsertTextAfterToken( Loc, Str ))
        return reportEditFailure( Loc );
    return true;
}

bool RewritePlugin::insertTextBefore( SourceLocation Loc, StringRef Str )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(Loc))
        return false;
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
    if (wouldRewriteWorkdir(range.getBegin()))
        return false;
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
    if (wouldRewriteWorkdir(Start))
        return false;
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
    if (wouldRewriteWorkdir(range.getBegin()))
        return false;
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
    if (wouldRewriteWorkdir(range.getBegin()))
        return false;
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

bool RewritePlugin::wouldRewriteWorkdir(SourceLocation loc)
{
    if (loc.isInvalid() || loc.isMacroID()) {
        return false;
    }
    return
        compiler.getSourceManager().getFilename(
            compiler.getSourceManager().getSpellingLoc(loc))
        .startswith(WORKDIR "/");
}

bool RewritePlugin::reportEditFailure( SourceLocation loc )
{
    report( DiagnosticsEngine::Warning, "cannot perform source modification (macro expansion involved?)", loc );
    return false;
}

namespace {

template<typename Fn> bool checkPathname(
    StringRef pathname, StringRef against, Fn check)
{
    if (check(pathname, against)) {
        return true;
    }
#if defined _WIN32
    for (std::size_t n = 0;;)
    {
        std::size_t n1 = pathname.find('\\', n);
        std::size_t n2 = against.find('\\', n);
        if (n1 <= n2) {
            if (n1 >= against.size()) {
                return check(pathname.substr(n), against.substr(n));
            }
            if ((against[n1] != '/' && against[n1] != '\\')
                || pathname.substr(n, n1 - n) != against.substr(n, n1 - n))
            {
                break;
            }
            n = n1 + 1;
        } else {
            if (n2 >= pathname.size()) {
                return check(pathname.substr(n), against.substr(n));
            }
            if (pathname[n2] != '/'
                || pathname.substr(n, n2 - n) != against.substr(n, n2 - n))
            {
                break;
            }
            n = n2 + 1;
        }
    }
#endif
    return false;
}

}

bool hasPathnamePrefix(StringRef pathname, StringRef prefix)
{
    return checkPathname(
        pathname, prefix,
        [](StringRef p, StringRef a) { return p.startswith(a); });
}

bool isSamePathname(StringRef pathname, StringRef other)
{
    return checkPathname(
        pathname, other, [](StringRef p, StringRef a) { return p == a; });
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
