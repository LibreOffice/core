/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <regex>
#include <string>
#include <set>

#include "compat.hxx"
#include "plugin.hxx"

// Check for some basic naming mismatches which make the code harder to read

namespace {

static const std::regex aMemberRegex("^m[_]?[abnprsx]?[A-Z]");

class StylePolice :
    public RecursiveASTVisitor<StylePolice>, public loplugin::Plugin
{
public:
    explicit StylePolice(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitVarDecl(const VarDecl *);
private:
    StringRef getFilename(SourceLocation loc);
};

StringRef StylePolice::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

bool StylePolice::VisitVarDecl(const VarDecl * varDecl)
{
    if (ignoreLocation(varDecl)) {
        return true;
    }
    if (!varDecl->isLocalVarDecl()) {
        return true;
    }

    std::string name = varDecl->getName();
    if (std::regex_search(name, aMemberRegex))
    {
        report(
            DiagnosticsEngine::Warning,
            "this local variable follows our member field naming convention, which is confusing",
            varDecl->getLocation())
             << varDecl->getType() << varDecl->getSourceRange();
    }
    return true;
}


loplugin::Plugin::Registration< StylePolice > X("stylepolice");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
