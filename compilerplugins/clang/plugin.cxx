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

namespace {

Expr const * skipImplicit(Expr const * expr) {
    if (auto const e = dyn_cast<MaterializeTemporaryExpr>(expr)) {
        expr = e->GetTemporaryExpr()->IgnoreImpCasts();
    }
    if (auto const e = dyn_cast<CXXBindTemporaryExpr>(expr)) {
        expr = e->getSubExpr();
    }
    return expr;
}

bool structurallyIdentical(Stmt const * stmt1, Stmt const * stmt2) {
    if (stmt1->getStmtClass() != stmt2->getStmtClass()) {
        return false;
    }
    switch (stmt1->getStmtClass()) {
    case Stmt::CXXConstructExprClass:
        if (cast<CXXConstructExpr>(stmt1)->getConstructor()->getCanonicalDecl()
            != cast<CXXConstructExpr>(stmt2)->getConstructor()->getCanonicalDecl())
        {
            return false;
        }
        break;
    case Stmt::DeclRefExprClass:
        if (cast<DeclRefExpr>(stmt1)->getDecl()->getCanonicalDecl()
            != cast<DeclRefExpr>(stmt2)->getDecl()->getCanonicalDecl())
        {
            return false;
        }
        break;
    case Stmt::ImplicitCastExprClass:
        {
            auto const e1 = cast<ImplicitCastExpr>(stmt1);
            auto const e2 = cast<ImplicitCastExpr>(stmt2);
            if (e1->getCastKind() != e2->getCastKind()
                || e1->getType().getCanonicalType() != e2->getType().getCanonicalType())
            {
                return false;
            }
            break;
        }
    case Stmt::MemberExprClass:
        {
            auto const e1 = cast<MemberExpr>(stmt1);
            auto const e2 = cast<MemberExpr>(stmt2);
            if (e1->isArrow() != e2->isArrow()
                || e1->getType().getCanonicalType() != e2->getType().getCanonicalType())
            {
                return false;
            }
            break;
        }
    case Stmt::CXXMemberCallExprClass:
    case Stmt::CXXOperatorCallExprClass:
        if (cast<Expr>(stmt1)->getType().getCanonicalType()
            != cast<Expr>(stmt2)->getType().getCanonicalType())
        {
            return false;
        }
        break;
    case Stmt::MaterializeTemporaryExprClass:
    case Stmt::ParenExprClass:
        break;
    default:
        // Conservatively assume non-identical for expressions that don't happen for us in practice
        // when compiling the LO code base (and for which the above set of supported classes would
        // need to be extended):
        return false;
    }
    auto i1 = stmt1->child_begin();
    auto e1 = stmt1->child_end();
    auto i2 = stmt2->child_begin();
    auto e2 = stmt2->child_end();
    for (; i1 != e1; ++i1, ++i2) {
        if (i2 == e2 || !structurallyIdentical(*i1, *i2)) {
            return false;
        }
    }
    return i2 == e2;
}

}

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

void Plugin::registerPlugin( Plugin* (*create)( const InstantiationData& ), const char* optionName,
    bool isPPCallback, bool isSharedPlugin, bool byDefault )
{
    PluginHandler::registerPlugin( create, optionName, isPPCallback, isSharedPlugin, byDefault );
}

bool Plugin::evaluate(const Expr* expr, APSInt& x)
{
    if (compat::EvaluateAsInt(expr, x, compiler.getASTContext()))
    {
        return true;
    }
    if (isa<CXXNullPtrLiteralExpr>(expr)) {
        x = 0;
        return true;
    }
    return false;
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

StringRef Plugin::getFileNameOfSpellingLoc(SourceLocation spellingLocation) const
{
    // prevent crashes when running the global-analysis plugins
    if (!spellingLocation.isValid())
        return "";

    static enum { NOINIT, STDIN, GOOD } s_Mode(NOINIT);
    if (s_Mode == GOOD)
    {
        return compiler.getSourceManager().getFilename(spellingLocation);
    }
    else if (s_Mode == STDIN
             || !compiler.getSourceManager().isInMainFile(spellingLocation))
    {
        const char* bufferName = compiler.getSourceManager().getPresumedLoc(spellingLocation).getFilename();
        return bufferName;
    }
    else
    {
        auto const fn(compiler.getSourceManager().getFilename(spellingLocation));
        if (!fn.data()) // wtf? happens in sot/source/sdstor/stg.cxx
        {
            return fn;
        }
#if !defined _WIN32
        assert(fn.startswith("/") || fn == "<stdin>");
#endif
        s_Mode = fn == "<stdin>" ? STDIN : GOOD;
        return getFileNameOfSpellingLoc(spellingLocation);
    }
}

bool Plugin::isInUnoIncludeFile(SourceLocation spellingLocation) const
{
    StringRef name{ getFileNameOfSpellingLoc(spellingLocation) };
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

Plugin::IdenticalDefaultArgumentsResult Plugin::checkIdenticalDefaultArguments(
    Expr const * argument1, Expr const * argument2)
{
    if ((argument1 == nullptr) != (argument2 == nullptr)) {
        return IdenticalDefaultArgumentsResult::No;
    }
    if (argument1 == nullptr) {
        return IdenticalDefaultArgumentsResult::Yes;
    }
    if (argument1->isNullPointerConstant(compiler.getASTContext(), Expr::NPC_NeverValueDependent)
        && argument2->isNullPointerConstant(compiler.getASTContext(), Expr::NPC_NeverValueDependent))
    {
        return IdenticalDefaultArgumentsResult::Yes;
    }
    APSInt x1, x2;
    if (evaluate(argument1, x1) && evaluate(argument2, x2))
    {
        return x1 == x2
            ? IdenticalDefaultArgumentsResult::Yes
            : IdenticalDefaultArgumentsResult::No;
    }
    APFloat f1(0.0f), f2(0.0f);
    if (argument1->EvaluateAsFloat(f1, compiler.getASTContext())
        && argument2->EvaluateAsFloat(f2, compiler.getASTContext()))
    {
        return f1.bitwiseIsEqual(f2)
            ? IdenticalDefaultArgumentsResult::Yes
            : IdenticalDefaultArgumentsResult::No;
    }
    auto const desugared1 = argument1->IgnoreParenImpCasts();
    auto const desugared2 = argument2->IgnoreParenImpCasts();
    if (auto const lit1 = dyn_cast<clang::StringLiteral>(desugared1)) {
        if (auto const lit2 = dyn_cast<clang::StringLiteral>(desugared2)) {
            return lit1->getBytes() == lit2->getBytes()
                ? IdenticalDefaultArgumentsResult::Yes
                : IdenticalDefaultArgumentsResult::No;
        }
    }
    // catch params with defaults like "= OUString()"
    for (Expr const * e1 = desugared1, * e2 = desugared2;;) {
        auto const ce1 = dyn_cast<CXXConstructExpr>(skipImplicit(e1));
        auto const ce2 = dyn_cast<CXXConstructExpr>(skipImplicit(e2));
        if (ce1 == nullptr || ce2 == nullptr) {
            break;
        }
        if (ce1->getConstructor()->getCanonicalDecl() != ce2->getConstructor()->getCanonicalDecl())
        {
            return IdenticalDefaultArgumentsResult::No;
        }
        if (ce1->isElidable() && ce2->isElidable() && ce1->getNumArgs() == 1
            && ce2->getNumArgs() == 1)
        {
            assert(ce1->getConstructor()->isCopyOrMoveConstructor());
            e1 = ce1->getArg(0)->IgnoreImpCasts();
            e2 = ce2->getArg(0)->IgnoreImpCasts();
            continue;
        }
        if (ce1->getNumArgs() == 0 && ce2->getNumArgs() == 0) {
            return IdenticalDefaultArgumentsResult::Yes;
        }
        break;
    }
    // If the EvaluateAsRValue derivatives above failed because the arguments use e.g. (constexpr)
    // function template specializations that happen to not have been instantiated in this TU, try a
    // structural comparison of the arguments:
    if (structurallyIdentical(argument1, argument2)) {
        return IdenticalDefaultArgumentsResult::Yes;
    }
    if (isDebugMode()) {
        report(
            DiagnosticsEngine::Fatal, "TODO: Unexpected 'IdenticalDefaultArgumentsResult::Maybe'",
            argument1->getExprLoc())
            << argument1->getSourceRange();
        report(
            DiagnosticsEngine::Note, "TODO: second argument is here", argument2->getExprLoc())
            << argument2->getSourceRange();
        argument1->dump();
        argument2->dump();
    }
    return IdenticalDefaultArgumentsResult::Maybe;
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
    SourceRange Range(SourceRange(Loc, Loc.getLocWithOffset(Str.size())));
    if( !handler.checkOverlap( Range ) )
    {
        report( DiagnosticsEngine::Warning, "double code removal, possible plugin error", Range.getBegin());
        return false;
    }
    if( rewriter->InsertText( Loc, Str, InsertAfter, indentNewLines ))
        return reportEditFailure( Loc );
    handler.addSourceModification(Range);
    return true;
}

bool RewritePlugin::insertTextAfter( SourceLocation Loc, StringRef Str )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(Loc))
        return false;
    SourceRange Range(SourceRange(Loc, Loc.getLocWithOffset(Str.size())));
    if( !handler.checkOverlap( Range ) )
    {
        report( DiagnosticsEngine::Warning, "double code removal, possible plugin error", Range.getBegin());
        return false;
    }
    if( rewriter->InsertTextAfter( Loc, Str ))
        return reportEditFailure( Loc );
    handler.addSourceModification(Range);
    return true;
}

bool RewritePlugin::insertTextAfterToken( SourceLocation Loc, StringRef Str )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(Loc))
        return false;
    SourceRange Range(SourceRange(Loc, Loc.getLocWithOffset(Str.size())));
    if( !handler.checkOverlap( Range ) )
    {
        report( DiagnosticsEngine::Warning, "double code removal, possible plugin error", Range.getBegin());
        return false;
    }
    if( rewriter->InsertTextAfterToken( Loc, Str ))
        return reportEditFailure( Loc );
    handler.addSourceModification(Range);
    return true;
}

bool RewritePlugin::insertTextBefore( SourceLocation Loc, StringRef Str )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(Loc))
        return false;
    SourceRange Range(SourceRange(Loc, Loc.getLocWithOffset(Str.size())));
    if( !handler.checkOverlap( Range ) )
    {
        report( DiagnosticsEngine::Warning, "double code removal, possible plugin error", Range.getBegin());
        return false;
    }
    if( rewriter->InsertTextBefore( Loc, Str ))
        return reportEditFailure( Loc );
    handler.addSourceModification(Range);
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
    if( !handler.checkOverlap( range.getAsRange() ) )
    {
        report( DiagnosticsEngine::Warning, "double code removal, possible plugin error", range.getBegin());
        return false;
    }
    if( opts.flags & RemoveWholeStatement || opts.flags & RemoveAllWhitespace )
    {
        if( !adjustRangeForOptions( &range, opts ))
            return reportEditFailure( range.getBegin());
    }
    if( rewriter->RemoveText( range, opts ))
        return reportEditFailure( range.getBegin());
    handler.addSourceModification(range.getAsRange());
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
    SourceRange Range(Start, Start.getLocWithOffset(std::max<size_t>(OrigLength, NewStr.size())));
    if( OrigLength != 0 && !handler.checkOverlap( Range ) )
    {
        report( DiagnosticsEngine::Warning, "overlapping code replacement, possible plugin error", Start );
        return false;
    }
    if( rewriter->ReplaceText( Start, OrigLength, NewStr ))
        return reportEditFailure( Start );
    handler.addSourceModification(Range);
    return true;
}

bool RewritePlugin::replaceText( SourceRange range, StringRef NewStr )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(range.getBegin()))
        return false;
    if( rewriter->getRangeSize( range ) == -1 )
        return reportEditFailure( range.getBegin());
    if( !handler.checkOverlap( range ) )
    {
        report( DiagnosticsEngine::Warning, "overlapping code replacement, possible plugin error", range.getBegin());
        return false;
    }
    if( rewriter->ReplaceText( range, NewStr ))
        return reportEditFailure( range.getBegin());
    handler.addSourceModification(range);
    return true;
}

bool RewritePlugin::replaceText( SourceRange range, SourceRange replacementRange )
{
    assert( rewriter );
    if (wouldRewriteWorkdir(range.getBegin()))
        return false;
    if( rewriter->getRangeSize( range ) == -1 )
        return reportEditFailure( range.getBegin());
    if( !handler.checkOverlap( range ) )
    {
        report( DiagnosticsEngine::Warning, "overlapping code replacement, possible plugin error", range.getBegin());
        return false;
    }
    if( rewriter->ReplaceText( range, replacementRange ))
        return reportEditFailure( range.getBegin());
    handler.addSourceModification(range);
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
