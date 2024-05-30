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

#include <clang/AST/ParentMapContext.h>
#include <clang/Basic/FileManager.h>
#include <clang/Lex/Lexer.h>

#include "config_clang.h"

#include "compat.hxx"
#include "pluginhandler.hxx"
#include "check.hxx"

/*
Base classes for plugin actions.
*/
namespace loplugin
{

namespace {

Expr const * skipImplicit(Expr const * expr) {
    if (auto const e = dyn_cast<MaterializeTemporaryExpr>(expr)) {
        expr = e->getSubExpr()->IgnoreImpCasts();
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
    case Stmt::CXXBindTemporaryExprClass:
    case Stmt::CXXDefaultArgExprClass:
    case Stmt::ParenExprClass:
        break;
    case Stmt::CXXNullPtrLiteralExprClass:
        return true;
    case Stmt::StringLiteralClass:
        return cast<clang::StringLiteral>(stmt1)->getBytes()
            == cast<clang::StringLiteral>(stmt2)->getBytes();
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

bool Plugin::suppressWarningAt(SourceLocation location) const {
    auto const start = compiler.getSourceManager().getSpellingLoc(location);
    auto const startInfo = compiler.getSourceManager().getDecomposedLoc(start);
    auto invalid = false;
    auto const buf = compiler.getSourceManager().getBufferData(startInfo.first, &invalid);
    if (invalid) {
        if (isDebugMode()) {
            report(DiagnosticsEngine::Fatal, "failed to getBufferData", start);
        }
        return false;
    }
    auto const label = std::string("[-loplugin:").append(name).append("]");
    // Look back to the beginning of the previous line:
    auto loc = start;
    auto locInfo = startInfo;
    auto cur = loc;
    enum class State { Normal, Slash, Comment };
    auto state = State::Normal;
    auto newlines = 0;
    for (auto prev = cur;;) {
        auto prevInfo = compiler.getSourceManager().getDecomposedLoc(prev);
        if (prev == compiler.getSourceManager().getLocForStartOfFile(prevInfo.first)) {
            if (state == State::Comment && isDebugMode()) {
                report(
                    DiagnosticsEngine::Fatal,
                    "beginning of file while looking for beginning of comment", prev);
            }
            break;
        }
        Token tok;
        if (Lexer::getRawToken(
                Lexer::GetBeginningOfToken(
                    prev.getLocWithOffset(-1), compiler.getSourceManager(), compiler.getLangOpts()),
                tok, compiler.getSourceManager(), compiler.getLangOpts(), true))
        {
            if (isDebugMode()) {
                report(
                    DiagnosticsEngine::Fatal, "failed to getRawToken",
                    prev.getLocWithOffset(-1));
            }
            break;
        }
        if (tok.getLocation() == cur) {
            // Keep walking back, character by character, through whitespace preceding the current
            // token, which Clang treats as nominally belonging to that token (so the above
            // Lexer::getRawToken/Lexer::GetBeginningOfToken will have produced just the same tok
            // again):
            prev = prev.getLocWithOffset(-1);
            continue;
        }
        cur = tok.getLocation();
        prev = cur;
        if (state == State::Comment) {
            // Lexer::GetBeginningOfToken (at least towards Clang 15, still) only re-scans from the
            // start of the current line, so if we saw the end of a multi-line /*...*/ comment, we
            // saw that as individual '/' and '*' faux-tokens, at which point we must (hopefully?)
            // actually be at the end of such a multi-line comment, so we keep walking back to the
            // first '/*' we encounter (TODO: which still need not be the real start of the comment,
            // if the comment contains embedded '/*', but we could determine that only if we
            // re-scanned from the start of the file):
            if (!tok.is(tok::comment)) {
                continue;
            }
            SmallVector<char, 256> tmp;
            bool invalid = false;
            auto const spell = Lexer::getSpelling(
                prev, tmp, compiler.getSourceManager(), compiler.getLangOpts(), &invalid);
            if (invalid) {
                if (isDebugMode()) {
                    report(DiagnosticsEngine::Fatal, "failed to getSpelling", prev);
                }
            } else if (!compat::starts_with(spell, "/*")) {
                continue;
            }
        }
        prevInfo = compiler.getSourceManager().getDecomposedLoc(prev);
        auto const end = prev.getLocWithOffset(tok.getLength());
        auto const endInfo = compiler.getSourceManager().getDecomposedLoc(end);
        assert(prevInfo.first == endInfo .first);
        assert(prevInfo.second <= endInfo.second);
        assert(endInfo.first == locInfo.first);
        // Whitespace between tokens is found at the end of prev, from end to loc (unless this is a
        // multi-line comment, in which case the whitespace has already been inspected as the
        // whitespace following the comment's final '/' faux-token):
        StringRef ws;
        if (state != State::Comment) {
            assert(endInfo.second <= locInfo.second);
            ws = buf.substr(endInfo.second, locInfo.second - endInfo.second);
        }
        for (std::size_t i = 0;;) {
            auto const j = ws.find('\n', i);
            if (j == StringRef::npos) {
                break;
            }
            ++newlines;
            if (newlines == 2) {
                break;
            }
            i = j + 1;
        }
        if (newlines == 2) {
            break;
        }
        auto str = buf.substr(prevInfo.second, endInfo.second - prevInfo.second);
        if (tok.is(tok::comment) && str.contains(label)) {
            return true;
        }
        for (std::size_t i = 0;;) {
            auto const j = str.find('\n', i);
            if (j == StringRef::npos) {
                break;
            }
            ++newlines;
            if (newlines == 2) {
                break;
            }
            i = j + 1;
        }
        if (newlines == 2) {
            break;
        }
        loc = prev;
        locInfo = prevInfo;
        switch (state) {
        case State::Normal:
            if (tok.is(tok::slash)) {
                state = State::Slash;
            }
            break;
        case State::Slash:
            state = tok.is(tok::star) && ws.empty() ? State::Comment : State::Normal;
                //TODO: check for "ws is only folding whitespace" rather than for `ws.empty()` (but
                // then, we must not count newlines in that whitespace twice, first as part of the
                // whitespace following the comment's semi-final '*' faux-token and then as part of
                // the comment token's content)
            break;
        case State::Comment:
            state = State::Normal;
        }
    }
    // Look forward to the end of the current line:
    loc = start;
    locInfo = startInfo;
    for (;;) {
        Token tok;
        if (Lexer::getRawToken(loc, tok, compiler.getSourceManager(), compiler.getLangOpts(), true))
        {
            if (isDebugMode()) {
                report(DiagnosticsEngine::Fatal, "failed to getRawToken", loc);
            }
            break;
        }
        // Whitespace between tokens is found at the beginning, from loc to beg:
        auto const beg = tok.getLocation();
        auto const begInfo = compiler.getSourceManager().getDecomposedLoc(beg);
        assert(begInfo.first == locInfo.first);
        assert(begInfo.second >= locInfo.second);
        if (buf.substr(locInfo.second, begInfo.second - locInfo.second).contains('\n')) {
            break;
        }
        auto const next = beg.getLocWithOffset(tok.getLength());
        auto const nextInfo = compiler.getSourceManager().getDecomposedLoc(next);
        assert(nextInfo.first == begInfo.first);
        assert(nextInfo.second >= begInfo.second);
        auto const str = buf.substr(begInfo.second, nextInfo.second - begInfo.second);
        if (tok.is(tok::comment) && str.contains(label)) {
            return true;
        }
        if (tok.is(tok::eof) || str.contains('\n')) {
            break;
        }
        loc = next;
        locInfo = nextInfo;
    }
    return false;
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

const Decl* getFunctionDeclContext(ASTContext& context, const Stmt* stmt)
{
    auto const parents = context.getParents(*stmt);
    auto it = parents.begin();

    if (it == parents.end())
          return nullptr;

    const Decl *decl = it->get<Decl>();
    if (decl)
    {
        if (isa<VarDecl>(decl))
            return dyn_cast<FunctionDecl>(decl->getDeclContext());
        return decl;
    }

    stmt = it->get<Stmt>();
    if (stmt)
        return getFunctionDeclContext(context, stmt);

    return nullptr;
}

const FunctionDecl* Plugin::getParentFunctionDecl( const Stmt* stmt )
{
    const Decl *decl = getFunctionDeclContext(compiler.getASTContext(), stmt);
    if (decl)
        return static_cast<const FunctionDecl*>(decl->getNonClosureContext());

    return nullptr;
}

StringRef Plugin::getFilenameOfLocation(SourceLocation spellingLocation) const
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
        char const*const pCXX = getenv("CXX");
        if (pCXX && strstr(pCXX, "sccache"))
        {   // heuristic; sccache passes file with -frewrite-directives by name
            s_Mode = STDIN;
            return getFilenameOfLocation(spellingLocation);
        }
        auto const fn(compiler.getSourceManager().getFilename(spellingLocation));
        if (!fn.data()) // wtf? happens in sot/source/sdstor/stg.cxx
        {
            return fn;
        }
#if !defined _WIN32
        assert(compat::starts_with(fn, "/") || fn == "<stdin>");
#endif
        s_Mode = fn == "<stdin>" ? STDIN : GOOD;
        return getFilenameOfLocation(spellingLocation);
    }
}

bool Plugin::isInUnoIncludeFile(SourceLocation spellingLocation) const
{
    StringRef name{ getFilenameOfLocation(spellingLocation) };
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
           || hasPathnamePrefix(name, SRCDIR "/include/typelib/")
           || hasPathnamePrefix(name, SRCDIR "/include/uno/")
           || hasPathnamePrefix(name, SDKDIR "/include/"));
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
    // Preprocessing directives (other than _Pragma, which is not relevant here) cannot appear in
    // macro expansions, so it is safe to just consider the range of expansion locations:
    auto const begin = compiler.getSourceManager().getExpansionLoc(
        range.getBegin());
    auto const end = compiler.getSourceManager().getExpansionLoc(
        range.getEnd());
    assert(begin.isFileID() && end.isFileID());
    if (!(begin == end
          || compiler.getSourceManager().isBeforeInTranslationUnit(
              begin, end)))
    {
        if (isDebugMode()) {
            report(
                DiagnosticsEngine::Fatal,
                ("unexpected broken range for Plugin::containsPreprocessingConditionalInclusion,"
                 " case 1"),
                range.getBegin())
                << range;
        }
        // Conservatively assume "yes" if lexing fails:
        return true;
    }
    auto hash = false;
    for (auto loc = begin;;) {
        Token tok;
        if (Lexer::getRawToken(
                loc, tok, compiler.getSourceManager(),
                compiler.getLangOpts(), true))
        {
            if (isDebugMode()) {
                report(
                    DiagnosticsEngine::Fatal,
                    ("unexpected broken range for"
                     " Plugin::containsPreprocessingConditionalInclusion, case 2"),
                    loc)
                    << range;
            }
            // Conservatively assume "yes" if lexing fails:
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
        if (loc == end) {
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

bool Plugin::containsComment(SourceRange range)
{
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = range.getBegin();
    SourceLocation endLoc = range.getEnd();
    char const* p1 = SM.getCharacterData(startLoc);
    char const* p2 = SM.getCharacterData(endLoc);
    p2 += Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());

    // when doing 'make solenv.check' we don't want the special comments in the
    // unit test files to trigger this check
    constexpr char const comment0[] = "// expected-error";
    if (std::search(p1, p2, comment0, comment0 + strlen(comment0)) != p2)
        return false;

    // check for comments
    constexpr char const comment1[] = "/*";
    constexpr char const comment2[] = "//";
    if (std::search(p1, p2, comment1, comment1 + strlen(comment1)) != p2)
        return true;
    if (std::search(p1, p2, comment2, comment2 + strlen(comment2)) != p2)
        return true;

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
    if (auto const lit1 = dyn_cast<clang::UserDefinedLiteral>(
            argument1->IgnoreImplicit()->IgnoreParens()))
    {
        if (auto const lit2 = dyn_cast<clang::UserDefinedLiteral>(
                argument2->IgnoreImplicit()->IgnoreParens()))
        {
            return lit1->getLiteralOperatorKind() == clang::UserDefinedLiteral::LOK_Template
                && lit2->getLiteralOperatorKind() == clang::UserDefinedLiteral::LOK_Template
                && lit1->getCalleeDecl()->getCanonicalDecl()
                   == lit2->getCalleeDecl()->getCanonicalDecl()
                ? IdenticalDefaultArgumentsResult::Maybe //TODO: obtain content to compare
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
    return compat::starts_with(
        getFilenameOfLocation(compiler.getSourceManager().getSpellingLoc(loc)), WORKDIR "/");
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
        [](StringRef p, StringRef a) { return compat::starts_with(p, a); });
}

bool isSamePathname(StringRef pathname, StringRef other)
{
    return checkPathname(
        pathname, other, [](StringRef p, StringRef a) { return p == a; });
}

bool isSameUnoIncludePathname(StringRef fullPathname, StringRef includePathname)
{
    llvm::SmallVector<char, 256> buf;
    if (isSamePathname(fullPathname, (SRCDIR "/include/" + includePathname).toStringRef(buf))) {
        return true;
    }
    buf.clear();
    return isSamePathname(fullPathname, (SDKDIR "/include/" + includePathname).toStringRef(buf));
}

bool hasCLanguageLinkageType(FunctionDecl const * decl) {
    assert(decl != nullptr);
    if (decl->isExternC()) {
        return true;
    }
    if (decl->isInExternCContext()) {
        return true;
    }
    return false;
}

static const CXXRecordDecl* stripTypeSugar(QualType qt)
{
    const clang::Type* t = qt.getTypePtr();
    do
    {
        if (auto elaboratedType = dyn_cast<ElaboratedType>(t))
            t = elaboratedType->desugar().getTypePtr();
        else if (auto tsType = dyn_cast<TemplateSpecializationType>(t))
            t = tsType->desugar().getTypePtr();
        else if (auto sttpType = dyn_cast<SubstTemplateTypeParmType>(t))
            t = sttpType->desugar().getTypePtr();
        else if (auto tdType = dyn_cast<TypedefType>(t))
            t = tdType->desugar().getTypePtr();
        else
            break;
    } while(true);
    auto recordType = dyn_cast<RecordType>(t);
    if (!recordType)
        return nullptr;
    return dyn_cast_or_null<CXXRecordDecl>(recordType->getDecl());
}

int derivedFromCount(const CXXRecordDecl* subclassRecordDecl, const CXXRecordDecl* baseclassRecordDecl)
{
    if (!subclassRecordDecl || !baseclassRecordDecl)
        return 0;
    int derivedCount = 0;
    if (subclassRecordDecl == baseclassRecordDecl)
        derivedCount++;
    if (!subclassRecordDecl->hasDefinition())
        return derivedCount;
    for (auto it = subclassRecordDecl->bases_begin(); it != subclassRecordDecl->bases_end(); ++it)
    {
        derivedCount += derivedFromCount(stripTypeSugar(it->getType()), baseclassRecordDecl);
        // short-circuit, we only care about 0,1,2
        if (derivedCount > 1)
            return derivedCount;
    }
    for (auto it = subclassRecordDecl->vbases_begin(); it != subclassRecordDecl->vbases_end(); ++it)
    {
        derivedCount += derivedFromCount(stripTypeSugar(it->getType()), baseclassRecordDecl);
        // short-circuit, we only care about 0,1,2
        if (derivedCount > 1)
            return derivedCount;
    }
    return derivedCount;
}

int derivedFromCount(QualType subclassQt, QualType baseclassQt)
{
    auto baseclassRecordDecl = stripTypeSugar(baseclassQt);
    if (!baseclassRecordDecl)
        return 0;
    auto subclassRecordDecl = stripTypeSugar(subclassQt);
    if (!subclassRecordDecl)
        return 0;

    return derivedFromCount(subclassRecordDecl, baseclassRecordDecl);
}

// It looks like Clang wrongly implements DR 4
// (<http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#4>) and treats
// a variable declared in an 'extern "..." {...}'-style linkage-specification as
// if it contained the 'extern' specifier:
bool hasExternalLinkage(VarDecl const * decl) {
    if (decl->getLinkageAndVisibility().getLinkage() != compat::Linkage::External) {
        return false;
    }
    for (auto ctx = decl->getLexicalDeclContext();
         ctx->getDeclKind() != Decl::TranslationUnit;
         ctx = ctx->getLexicalParent())
    {
        if (auto ls = dyn_cast<LinkageSpecDecl>(ctx)) {
            if (!ls->hasBraces()) {
                return true;
            }
            if (auto prev = decl->getPreviousDecl()) {
                return hasExternalLinkage(prev);
            }
            return !decl->isInAnonymousNamespace();
        }
    }
    return true;
}

bool isSmartPointerType(QualType qt)
{
    // First check whether the object type as written is, or is derived from, std::unique_ptr or
    // std::shared_ptr, in case the get member function is declared at a base class of that std
    // type:
    if (loplugin::isDerivedFrom(
            qt->getAsCXXRecordDecl(),
            [](Decl const * decl) {
                auto const dc = loplugin::DeclCheck(decl);
                return dc.ClassOrStruct("unique_ptr").StdNamespace()
                    || dc.ClassOrStruct("shared_ptr").StdNamespace();
            }))
        return true;

    // Then check the object type coerced to the type of the get member function, in
    // case the type-as-written is derived from one of these types (tools::SvRef is
    // final, but the rest are not):
    auto const tc2 = loplugin::TypeCheck(qt);
    if (tc2.ClassOrStruct("unique_ptr").StdNamespace()
           || tc2.ClassOrStruct("shared_ptr").StdNamespace()
           || tc2.Class("Reference").Namespace("uno").Namespace("star")
                .Namespace("sun").Namespace("com").GlobalNamespace()
           || tc2.Class("Reference").Namespace("rtl").GlobalNamespace()
           || tc2.Class("SvRef").Namespace("tools").GlobalNamespace()
           || tc2.Class("WeakReference").Namespace("tools").GlobalNamespace()
           || tc2.Class("ScopedReadAccess").Namespace("Bitmap").GlobalNamespace()
           || tc2.Class("ScopedVclPtrInstance").GlobalNamespace()
           || tc2.Class("VclPtr").GlobalNamespace()
           || tc2.Class("ScopedVclPtr").GlobalNamespace()
           || tc2.Class("intrusive_ptr").Namespace("boost").GlobalNamespace())
    {
        return true;
    }
    return false;
}

bool isSmartPointerType(const Expr* e)
{
    // First check whether the object type as written is, or is derived from, std::unique_ptr or
    // std::shared_ptr, in case the get member function is declared at a base class of that std
    // type:
    if (loplugin::isDerivedFrom(
            e->IgnoreImpCasts()->getType()->getAsCXXRecordDecl(),
            [](Decl const * decl) {
                auto const dc = loplugin::DeclCheck(decl);
                return dc.ClassOrStruct("unique_ptr").StdNamespace()
                    || dc.ClassOrStruct("shared_ptr").StdNamespace();
            }))
        return true;

    // Then check the object type coerced to the type of the get member function, in
    // case the type-as-written is derived from one of these types (tools::SvRef is
    // final, but the rest are not):
    auto const tc2 = loplugin::TypeCheck(e->getType());
    if (tc2.ClassOrStruct("unique_ptr").StdNamespace()
           || tc2.ClassOrStruct("shared_ptr").StdNamespace()
           || tc2.Class("Reference").Namespace("uno").Namespace("star")
                .Namespace("sun").Namespace("com").GlobalNamespace()
           || tc2.Class("Reference").Namespace("rtl").GlobalNamespace()
           || tc2.Class("SvRef").Namespace("tools").GlobalNamespace()
           || tc2.Class("WeakReference").Namespace("tools").GlobalNamespace()
           || tc2.Class("ScopedReadAccess").Namespace("Bitmap").GlobalNamespace()
           || tc2.Class("ScopedVclPtrInstance").GlobalNamespace()
           || tc2.Class("VclPtr").GlobalNamespace()
           || tc2.Class("ScopedVclPtr").GlobalNamespace()
           || tc2.Class("intrusive_ptr").Namespace("boost").GlobalNamespace())
    {
        return true;
    }
    return false;
}


} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
