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
#include <unordered_set>

#include <clang/AST/CXXInheritance.h>

#include "config_clang.h"

#include "plugin.hxx"
#include "check.hxx"

/**
Look for methods that are taking a lock at the top of the method, but then not
touching any object-local state. In which case the method might not need locking.

TODO

(*) check if the data being returned is never modified, in which case locking is not necessary

*/

namespace
{
class UnnecessaryLocking : public loplugin::FilteringPlugin<UnnecessaryLocking>
{
public:
    explicit UnnecessaryLocking(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        //        StringRef fn(handler.getMainFileName());
        //        if (loplugin::isSamePathname(fn, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx"))
        //            return false;
        return true;
    }
    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool VisitCXXThisExpr(const CXXThisExpr*);

private:
    bool isSolarMutexLockGuardStmt(const Stmt*);
    const Stmt* isOtherMutexLockGuardStmt(const Stmt*);
    std::vector<bool> m_TouchesThis;
    // so we ignore the CxxThisEpxr that references the maMutex in the guard expression
    std::vector<const Stmt*> m_IgnoreThis;
};

bool UnnecessaryLocking::TraverseCXXMethodDecl(CXXMethodDecl* cxxMethodDecl)
{
    if (ignoreLocation(cxxMethodDecl))
        return true;

    if (!cxxMethodDecl->isInstance())
        return true;
    if (!cxxMethodDecl->isThisDeclarationADefinition())
        return true;

    auto compoundStmt = dyn_cast_or_null<CompoundStmt>(cxxMethodDecl->getBody());
    if (!compoundStmt || compoundStmt->size() < 1)
        return true;

    const Stmt* firstStmt = *compoundStmt->body_begin();
    bool solarMutex = isSolarMutexLockGuardStmt(firstStmt);
    const Stmt* ignoreThisStmt = nullptr;
    if (!solarMutex)
        ignoreThisStmt = isOtherMutexLockGuardStmt(firstStmt);
    if (!solarMutex && ignoreThisStmt == nullptr)
        return true;

    m_TouchesThis.push_back(false);
    m_IgnoreThis.push_back(ignoreThisStmt);

    bool rv = FilteringPlugin::TraverseCXXMethodDecl(cxxMethodDecl);

    if (!m_TouchesThis.back())
    {
        StringRef fn = getFilenameOfLocation(
            compiler.getSourceManager().getSpellingLoc(cxxMethodDecl->getBeginLoc()));
        if (
            // template magic
            !loplugin::isSamePathname(fn, SRCDIR "/include/comphelper/unique_disposing_ptr.hxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/inc/unobaseclass.hxx")
            // toolkit needs to lock around access to static methods in vcl
            && !loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/awt/vclxtoolkit.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/toolkit/source/controls/tree/treecontrolpeer.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/awt/vclxcontainer.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/awt/vclxdevice.cxx")
            // touching shared global data
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/framework/source/fwi/classes/protocolhandlercache.cxx")
            // lock around access to static methods in vcl
            && !loplugin::isSamePathname(fn, SRCDIR "/framework/source/services/taskcreatorsrv.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/svx/source/dialog/SafeModeUI.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/svx/source/accessibility/AccessibleFrameSelector.cxx")
            // not sure
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/dialog/filedlghelper.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/appl/appdispatchprovider.cxx")
            // touching shared global data
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unoftn.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unolinebreak.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unoobj.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unorefmk.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unotbl.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/xml/xmltexti.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/uno/dlelstnr.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/access/accdoc.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/access/acccontext.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unocontentcontrol.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unobkm.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/docuno.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/afmtuno.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/appluno.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/unoxml/source/dom/documentbuilder.cxx")
            && !loplugin::isSamePathname(
                   fn, SRCDIR "/sd/source/ui/accessibility/AccessibleDrawDocumentView.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/starmath/source/accessibility.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/starmath/source/AccessibleSmElementsControl.cxx"))
        {
            report(DiagnosticsEngine::Warning, "unnecessary locking", cxxMethodDecl->getBeginLoc())
                << cxxMethodDecl->getSourceRange();
        }
    }

    m_TouchesThis.pop_back();
    m_IgnoreThis.pop_back();

    return rv;
}

bool UnnecessaryLocking::isSolarMutexLockGuardStmt(const Stmt* stmt)
{
    auto declStmt = dyn_cast<DeclStmt>(stmt);
    if (!declStmt)
        return false;
    if (!declStmt->isSingleDecl())
        return false;
    auto varDecl = dyn_cast<VarDecl>(declStmt->getSingleDecl());
    if (!varDecl)
        return false;
    auto tc = loplugin::TypeCheck(varDecl->getType());
    if (!tc.Class("SolarMutexGuard").GlobalNamespace()
        && !tc.Class("SolarMutexClearableGuard").GlobalNamespace()
        && !tc.Class("SolarMutexResettableGuard").GlobalNamespace()
        && !tc.Class("SolarMutexTryAndBuyGuard").GlobalNamespace())
        return false;
    return true;
}

const Stmt* UnnecessaryLocking::isOtherMutexLockGuardStmt(const Stmt* stmt)
{
    auto declStmt = dyn_cast<DeclStmt>(stmt);
    if (!declStmt)
        return nullptr;
    if (!declStmt->isSingleDecl())
        return nullptr;
    auto varDecl = dyn_cast<VarDecl>(declStmt->getSingleDecl());
    if (!varDecl)
        return nullptr;
    auto tc = loplugin::TypeCheck(varDecl->getType());
    if (!tc.Class("unique_lock").StdNamespace() && !tc.Class("scoped_lock").StdNamespace())
        return nullptr;
    auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(varDecl->getInit());
    if (!cxxConstructExpr || cxxConstructExpr->getNumArgs() < 1)
        return nullptr;
    auto memberExpr = dyn_cast<MemberExpr>(cxxConstructExpr->getArg(0));
    if (!memberExpr)
        return nullptr;
    auto thisStmt = memberExpr->getBase();
    return thisStmt;
}

bool UnnecessaryLocking::VisitCXXThisExpr(const CXXThisExpr* cxxThisExpr)
{
    if (ignoreLocation(cxxThisExpr))
        return true;
    // just in case
    if (m_TouchesThis.empty())
        return true;
    // already found something
    if (m_TouchesThis.back())
        return true;
    if (m_IgnoreThis.back() && m_IgnoreThis.back() == cxxThisExpr)
        return true;
    m_TouchesThis.back() = true;
    return true;
}

/** off by default because each warning needs to be carefully inspected */
loplugin::Plugin::Registration<UnnecessaryLocking> unnecessarylocking("unnecessarylocking", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
