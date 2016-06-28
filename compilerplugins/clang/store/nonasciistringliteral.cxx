/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include "plugin.hxx"

namespace {

class NonAsciiStringLiteral:
    public clang::RecursiveASTVisitor<NonAsciiStringLiteral>,
    public loplugin::Plugin
{
public:
    explicit NonAsciiStringLiteral(InstantiationData const & data): Plugin(data)
    {}

    void run() override;

    bool VisitStringLiteral(clang::StringLiteral const * expr);
};

void NonAsciiStringLiteral::run() {
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

bool NonAsciiStringLiteral::VisitStringLiteral(
    clang::StringLiteral const * expr)
{
    if (ignoreLocation(expr) || !expr->isAscii()) {
        return true;
    }
    auto const s = expr->getString();
    for (unsigned i = 0; i != s.size(); ++i) {
        if (!clang::isASCII(s[i])) {
            auto const loc = expr->getLocationOfByte(
                i, compiler.getSourceManager(), compiler.getLangOpts(),
                compiler.getTarget());
            auto const sloc = compiler.getSourceManager().getSpellingLoc(loc);
            auto const sloc0 = clang::Lexer::GetBeginningOfToken(
                sloc, compiler.getSourceManager(), compiler.getLangOpts());
            auto p0 = compiler.getSourceManager().getCharacterData(sloc0);
            assert(p0 != nullptr);
            auto n = Lexer::MeasureTokenLength(
                sloc0, compiler.getSourceManager(), compiler.getLangOpts());
            //TODO: handle line splicing
if(!(n >= 2)){
                report(
                    clang::DiagnosticsEngine::Fatal,
                    "FATAL B %0",
                    loc)
<<n
                    << expr->getSourceRange();
return true;
}
            assert(n >= 2);
            assert(p0[0] == '"');
if(!(p0[n - 1] == '"')){
                report(
                    clang::DiagnosticsEngine::Fatal,
                    "FATAL A %0 %1 %2",
                    loc)
<<n<<int(p0[n-1])<<StringRef(p0, n)
                    << expr->getSourceRange();
return true;
}
            assert(p0[n - 1] == '"');
            ++p0;
            n -= 2;
            auto const p = compiler.getSourceManager().getCharacterData(sloc);
            assert(p != nullptr);
if(!(p >= p0 && p < p0 + n)){
                report(
                    clang::DiagnosticsEngine::Fatal,
                    "FATAL code %0 at index %1 in ordinary string literal (%2, %3: %4 %5)",
                    loc)
                    << unsigned(static_cast<unsigned char>(s[i])) << i
<<int(p-p0)<<n<<int(*p0)<<int(*p)
                    << expr->getSourceRange();
return true;
}
            assert(p >= p0 && p < p0 + n);
            if (!(p[0] == '\\'
                  || (compiler.getLangOpts().Trigraphs
                      && p[0] == '?' && p + 2 - p0 < n && p[1] == '?'
                      && p[2] == '/')))
            {
                report(
                    clang::DiagnosticsEngine::Warning,
                    "non-ASCII character with code %0 at index %1 in ordinary"
                        " string literal",
                    loc)
                    << unsigned(static_cast<unsigned char>(s[i])) << i
                    << expr->getSourceRange();
            }
        }
    }
    return true;
}

loplugin::Plugin::Registration<NonAsciiStringLiteral> X(
    "nonasciistringliteral");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
