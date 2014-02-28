/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <string>

#include "plugin.hxx"

namespace {

bool isStdException(QualType type) {
    //TODO:
    std::string name { type.getAsString() };
    return name == "std::exception" || name == "::std::exception";
}

class StdException:
    public RecursiveASTVisitor<StdException>, public loplugin::RewritePlugin
{
public:
    explicit StdException(InstantiationData const & data): RewritePlugin(data)
    {}

    virtual void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl(CXXMethodDecl const * decl);

private:
    bool isInMainFile(SourceLocation spellingLocation) const;
};

bool StdException::VisitCXXMethodDecl(CXXMethodDecl const * decl) {
    if (ignoreLocation(decl)
        || decl->begin_overridden_methods() == decl->end_overridden_methods())
    {
        return true;
    }
    CXXMethodDecl const * over = nullptr;
    for (auto i = decl->begin_overridden_methods();
         i != decl->end_overridden_methods(); ++i)
    {
        FunctionProtoType const * t
            = (*i)->getType()->getAs<FunctionProtoType>();
        switch (t->getExceptionSpecType()) {
        case EST_None:
            continue;
        case EST_DynamicNone:
        case EST_BasicNoexcept:
            return true;
        case EST_Dynamic:
            {
                unsigned n = t->getNumExceptions();
                for (unsigned j = 0; j != n; ++j) {
                    if (isStdException(t->getExceptionType(j))) {
                        over = *i;
                        goto found;
                    }
                }
                return true;
            }
        case EST_ComputedNoexcept:
            switch (t->getNoexceptSpec(compiler.getASTContext())) {
            case FunctionProtoType::NR_NoNoexcept:
            case FunctionProtoType::NR_BadNoexcept:
                assert(false);
                // fall through
            case FunctionProtoType::NR_Dependent:
                break;
            case FunctionProtoType::NR_Throw:
                continue;
            case FunctionProtoType::NR_Nothrow:
                return true;
            }
        case EST_MSAny:
        case EST_Unevaluated:
        case EST_Uninstantiated:
            continue; //TODO???
        }
    }
    return true;
found:
    FunctionProtoType const * t = decl->getType()->getAs<FunctionProtoType>();
    if (!t->hasDynamicExceptionSpec()) {
        report(
            DiagnosticsEngine::Warning,
            "override does not have dynamic exception specification",
            decl->getLocStart())
            << decl->getSourceRange();
        report(
            DiagnosticsEngine::Note,
            ("overridden declaration with dynamic exception specification"
             " including std::exception is here"),
            over->getLocStart());
        return true;
    }
    unsigned n = t->getNumExceptions();
    for (unsigned i = 0; i != n; ++i) {
        if (isStdException(t->getExceptionType(i))) {
            return true;
        }
    }
    SourceRange r { decl->getSourceRange() };
    SourceLocation l {
        compiler.getSourceManager().getExpansionLoc(r.getBegin()) };
    SourceLocation end {
        compiler.getSourceManager().getExpansionLoc(r.getEnd()) };
    assert(
        l == end
        || compiler.getSourceManager().isBeforeInTranslationUnit(l, end));
    bool seenThrow = false;
    unsigned parens = 0;
    SourceLocation openParen;
    SourceLocation loc;
    for (;;) {
        unsigned n = Lexer::MeasureTokenLength(
            l, compiler.getSourceManager(), compiler.getLangOpts());
        std::string s { compiler.getSourceManager().getCharacterData(l), n };
        if (s == "{" || s == ";") {
            break;
        }
        if (!seenThrow) {
            if (s == "throw") {
                seenThrow = true;
            }
        } else if (s == "(") {
            assert(parens < std::numeric_limits<unsigned>::max());
            ++parens;
            if (parens == 1) {
                openParen = l;
            }
            loc = l;
        } else if (s == ")") {
            assert(parens != 0);
            --parens;
            if (parens == 0) {
                assert(loc.isValid());
                // Only rewrite declarations in include files if a definition is
                // also seen, to avoid compilation of a definition (in a main
                // file only processed later) to fail with a "mismatch" error
                // before the rewriter had a chance to act upon the definition
                // (but use the heuristic of assuming pure virtual functions do
                // not have definitions):
                if (rewriter != nullptr
                    && (isInMainFile(
                            compiler.getSourceManager().getSpellingLoc(loc))
                        || decl->isDefined() || decl->isPure())
                    && insertTextAfterToken(
                        loc,
                        (loc == openParen
                         ? "std::exception" : ", std::exception")))
                {
                    return true;
                }
                break;
            }
            loc = l;
        } else if (!s.empty() && s.compare(0, 2, "/*") != 0
                   && s.compare(0, 2, "//") != 0)
        {
            loc = l;
        }
        if (l == end) {
            break;
        }
        l = l.getLocWithOffset(std::max<unsigned>(n, 1));
    }
    report(
        DiagnosticsEngine::Warning,
        "override dropped std::exception from dynamic exception specification",
        openParen.isValid() ? openParen : decl->getLocStart())
        << decl->getSourceRange();
    report(
        DiagnosticsEngine::Note, "overridden declaration is here",
        over->getLocStart());
    return true;
}

bool StdException::isInMainFile(SourceLocation spellingLocation) const {
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
    return compiler.getSourceManager().isInMainFile(spellingLocation);
#else
    return compiler.getSourceManager().isFromMainFile(spellingLocation);
#endif
}

loplugin::Plugin::Registration<StdException> X("stdexception", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
