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
#include <fstream>
#include <set>
#include "plugin.hxx"

namespace {

class DodgySwitch:
    public RecursiveASTVisitor<DodgySwitch>, public loplugin::Plugin
{
public:
    explicit DodgySwitch(loplugin::InstantiationData const & data): Plugin(data)
    {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitDefaultStmt(DefaultStmt const * );
    bool VisitCaseStmt(CaseStmt const * );
private:
    bool IsParentSwitch(Stmt const * );
};

bool DodgySwitch::VisitDefaultStmt(DefaultStmt const * defaultStmt)
{
    if (ignoreLocation(defaultStmt))
        return true;
    if (!IsParentSwitch(defaultStmt))
        report(
            DiagnosticsEngine::Warning, "default statement not directly under switch",
            defaultStmt->getLocStart())
          << defaultStmt->getSourceRange();
    return true;
}

bool DodgySwitch::VisitCaseStmt(CaseStmt const * caseStmt)
{
    if (ignoreLocation(caseStmt))
        return true;
    if (!IsParentSwitch(caseStmt))
    {
        //parentStmt(parentStmt(caseStmt))->dump();
        report(
            DiagnosticsEngine::Warning, "case statement not directly under switch",
            caseStmt->getLocStart())
          << caseStmt->getSourceRange();
    }
    return true;
}

bool DodgySwitch::IsParentSwitch(Stmt const * stmt)
{
    auto parent = getParentStmt(stmt);
    if (isa<CaseStmt>(parent) || isa<DefaultStmt>(parent)) // daisy chain
        return true;
    auto compoundStmt = dyn_cast<CompoundStmt>(parent);
    if (!compoundStmt)
        return false;
    return isa<SwitchStmt>(getParentStmt(compoundStmt));
}

loplugin::Plugin::Registration< DodgySwitch > X("dodgyswitch", false);

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
