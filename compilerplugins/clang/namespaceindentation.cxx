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
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <locale>
#include <fstream>
#include <set>
#include "plugin.hxx"

/*
*/

namespace
{
class NamespaceIndentation : public loplugin::FilteringPlugin<NamespaceIndentation>
{
public:
    explicit NamespaceIndentation(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitNamespaceDecl(NamespaceDecl const*);

private:
    std::string GetFullNamespace(const NamespaceDecl* nsDecl);
};

void trim(std::string& str)
{
    // right trim
    auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(it1.base(), str.end());
    // left trim
    auto it2 = std::find_if(str.begin(), str.end(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(str.begin(), it2);
}

bool NamespaceIndentation::VisitNamespaceDecl(NamespaceDecl const* nsDecl)
{
    if (ignoreLocation(nsDecl))
        return true;
    if (nsDecl->isAnonymousNamespace())
        return true;
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(nsDecl->getLocation())))
        return true;

    // right now, just fixing up the fallout from clang-tidy-modernize-namespaces, which
    // does not touch header files
    if (!compiler.getSourceManager().isInMainFile(nsDecl->getLocation()))
        return true;

    auto& SM = compiler.getSourceManager();

    // if we have a combined ns (.e.g namespace aaa::bbb), this appears in the AST
    // as two nested namespace sharing the same source locations, so ignore the outer decls
    if (!nsDecl->decls_empty())
    {
        auto child = dyn_cast_or_null<NamespaceDecl>(*nsDecl->decls_begin());
        if (child)
        {
            bool invalid1 = false;
            bool invalid2 = false;
            unsigned line1 = SM.getPresumedLineNumber(nsDecl->getBeginLoc(), &invalid1);
            unsigned line2 = SM.getPresumedLineNumber(child->getBeginLoc(), &invalid2);
            if (line1 == line2)
                return true;
        }
    }

    // Truly hacky way to find the actual beginning of an xxx::yyy namespace declaration
    // if we are inside the yyy NameSpaceDecl of
    //      namespace xxx::yyy
    // the beginLoc is just between the "xxx" and the "::"
    auto nsDeclBeginLoc = nsDecl->getBeginLoc();
    bool foundMultiple = false;
    {
        constexpr int BACKSCAN = 32;
        auto beginLoc = nsDecl->getBeginLoc().getLocWithOffset(-BACKSCAN);
        auto endLoc = nsDecl->getBeginLoc().getLocWithOffset(3);
        const char* p1 = SM.getCharacterData(beginLoc);
        const char* p2 = SM.getCharacterData(endLoc);
        unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
        if (p2 < p1 || n > 128 || (p2 - p1 + n) > 2048)
            return true;
        auto s = std::string(p1, p2 - p1);
        auto idx1 = s.rfind(" "); // find the space preceding the namespace token
        if (idx1 != std::string::npos)
        {
            auto namespaceToken = s.substr(idx1);
            if (namespaceToken.find("::") != std::string::npos)
            {
                auto idx = s.rfind("\n");
                nsDeclBeginLoc = nsDecl->getBeginLoc().getLocWithOffset(idx - BACKSCAN + 1);
                foundMultiple = true;
            }
        }
    }

    // for now, I am only interested in fixing the fallout from clang-tidy-modernize-namespace, not
    // anything else
    if (!foundMultiple)
        return true;

    bool invalid1 = false;
    bool invalid2 = false;
    unsigned col1 = SM.getPresumedColumnNumber(nsDeclBeginLoc, &invalid1);
    unsigned col2 = SM.getPresumedColumnNumber(nsDecl->getRBraceLoc(), &invalid2);
    unsigned line1 = SM.getPresumedLineNumber(nsDeclBeginLoc, &invalid1);
    unsigned line2 = SM.getPresumedLineNumber(nsDecl->getRBraceLoc(), &invalid2);
    if (invalid1 || invalid2)
        return true;
    if (line1 == line2) // single line declaration
        return true;
    if (col1 != col2)
        report(DiagnosticsEngine::Warning, "statement right brace mis-aligned",
               nsDecl->getRBraceLoc());

    // no easy way to get the position of the left brace
    auto endLoc = nsDecl->getBeginLoc().getLocWithOffset(256);
    const char* p1 = SM.getCharacterData(SM.getExpansionLoc(nsDecl->getBeginLoc()));
    const char* p2 = SM.getCharacterData(SM.getExpansionLoc(endLoc));
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    if (p2 < p1 || n > 128 || (p2 - p1 + n) > 2048)
        return true;
    auto s = std::string(p1, p2 - p1 + n);
    auto idx1 = s.find("\n");
    auto idx2 = s.find("{");
    if (idx1 != std::string::npos && idx2 != std::string::npos)
        if (idx1 < idx2)
        {
            auto col3 = idx2 - idx1;
            if (col1 != col3)
                report(DiagnosticsEngine::Warning, "statement left brace mis-aligned",
                       nsDecl->getBeginLoc());
        }

    // extract the comment following the end brace
    auto beginLoc = nsDecl->getRBraceLoc();
    endLoc = beginLoc.getLocWithOffset(128);
    p1 = SM.getCharacterData(SM.getExpansionLoc(beginLoc));
    p2 = SM.getCharacterData(SM.getExpansionLoc(endLoc));
    n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    if (p2 < p1 || n > 128 || (p2 - p1 + n) > 2048)
        return true;
    s = std::string(p1, p2 - p1 + n);
    idx1 = s.find("//");
    idx2 = s.find("\n");
    if (idx1 != std::string::npos && idx2 != std::string::npos && idx1 < idx2)
    {
        idx1 += 2;
        s = s.substr(idx1, idx2 - idx1);
        trim(s);
        std::string fullNamespace = GetFullNamespace(nsDecl);
        if (!(s == fullNamespace || s == (fullNamespace + " namespace") || s == "namespace"
              || s == ("namespace " + fullNamespace) || s == ("namespace ::" + fullNamespace)
              || s == ("end " + fullNamespace) || s == "end namespace"
              || s == ("end namespace " + fullNamespace)
              || s == ("end " + fullNamespace + " namespace") || s == "end of namespace"
              || s == ("end of namespace " + fullNamespace)
              || s == ("end of namespace ::" + fullNamespace)
              || s == ("eof of namespace " + fullNamespace)))
        {
            report(DiagnosticsEngine::Warning, "incorrect comment at end of namespace %0",
                   nsDecl->getRBraceLoc())
                << fullNamespace;
        }
    }

    return true;
}

std::string NamespaceIndentation::GetFullNamespace(const NamespaceDecl* nsDecl)
{
    std::vector<llvm::StringRef> names;
    auto ns = nsDecl;
    while (ns)
    {
        names.push_back(ns->getName());
        ns = dyn_cast<NamespaceDecl>(ns->getParent());
    }
    std::string fullNamespace;
    for (auto it = names.rbegin(); it != names.rend(); ++it)
        fullNamespace += "::" + it->str();
    fullNamespace = fullNamespace.substr(2);
    return fullNamespace;
}

// leave this off by default, so as not to annoy people
loplugin::Plugin::Registration<NamespaceIndentation> namespaceindentation("namespaceindentation",
                                                                          false);

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
