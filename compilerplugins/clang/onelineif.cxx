/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <regex>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/*
  TODO check "for" statements in a similar way?
*/

namespace
{
class OneLineIf : public loplugin::FilteringPlugin<OneLineIf>
{
public:
    explicit OneLineIf(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitIfStmt(IfStmt const*);
};

bool OneLineIf::VisitIfStmt(IfStmt const* ifStmt)
{
    if (ignoreLocation(ifStmt))
        return true;

    SourceManager& SM = compiler.getSourceManager();

    if (SM.isMacroBodyExpansion(compat::getBeginLoc(ifStmt)))
        return true;
    if (SM.isMacroArgExpansion(compat::getBeginLoc(ifStmt)))
        return true;

    unsigned l1 = SM.getPresumedLineNumber(ifStmt->getCond()->getEndLoc());
    unsigned l2 = SM.getPresumedLineNumber(compat::getBeginLoc(ifStmt->getThen()));
    if (l1 != l2)
        return true;

    char const* p1 = SM.getCharacterData(compat::getBeginLoc(ifStmt->getThen()));
    char const* p2 = SM.getCharacterData(compat::getEndLoc(ifStmt->getThen()));
    if (p2 < p1)
        return true; // macro

    // check to see if the part beginning on the same line as the if condition is just an open brace
    // and/or some comment tex
    do
    {
        auto s = p1;
        while (*s == ' ' || *s == '{')
            ++s;
        if (*s == '/' || *s == '\n')
            return true;
    } while (false);

    report(DiagnosticsEngine::Warning, "then statement on same line as if, hard to read",
           compat::getBeginLoc(ifStmt))
        << ifStmt->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<OneLineIf> X("onelineif", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
