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

    bool VisitCompoundStmt(const CompoundStmt*);
    bool VisitCXXThisExpr(const CXXThisExpr*);
    bool VisitCallExpr(const CallExpr*);

private:
    bool isSolarMutexLockGuardStmt(const Stmt*);
    const CXXThisExpr* isOtherMutexLockGuardStmt(const Stmt*);
    std::vector<bool> m_TouchesThis;
    // so we ignore the CxxThisEpxr that references the maMutex in the guard expression
    std::vector<const CXXThisExpr*> m_IgnoreThis;
};

bool UnnecessaryLocking::VisitCompoundStmt(const CompoundStmt* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;
    if (compoundStmt->size() < 1)
        return true;

    const Stmt* firstStmt = *compoundStmt->body_begin();
    bool solarMutex = isSolarMutexLockGuardStmt(firstStmt);
    const CXXThisExpr* ignoreThisStmt = nullptr;
    if (!solarMutex)
        ignoreThisStmt = isOtherMutexLockGuardStmt(firstStmt);
    if (!solarMutex && ignoreThisStmt == nullptr)
        return true;

    m_TouchesThis.push_back(false);
    m_IgnoreThis.push_back(ignoreThisStmt);

    for (const Stmt* stmt : compoundStmt->body())
        FilteringPlugin::TraverseStmt(const_cast<Stmt*>(stmt));

    if (!m_TouchesThis.back())
    {
        StringRef fn = getFilenameOfLocation(
            compiler.getSourceManager().getSpellingLoc(compoundStmt->getBeginLoc()));
        if (
            // template magic
            !loplugin::isSamePathname(fn, SRCDIR "/include/comphelper/unique_disposing_ptr.hxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/inc/unobaseclass.hxx")

            // false+
            && !loplugin::isSamePathname(fn, SRCDIR "/cppuhelper/source/component_context.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/controls/tree/treecontrol.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/toolkit/source/helper/listenermultiplexer.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/include/toolkit/helper/macros.hxx")
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/chart2/source/controller/main/CommandDispatch.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/chart2/source/controller/main/ChartView.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/chart2/source/controller/main/SelectionHelper.cxx")
            && !loplugin::isSamePathname(
                   fn, SRCDIR "/chart2/source/controller/accessibility/AccessibleChartView.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/desktop/source/offacc/acceptor.cxx")
            && !loplugin::isSamePathname(
                   fn, SRCDIR "/desktop/source/deployment/registry/component/dp_component.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/desktop/source/deployment/gui/dp_gui_dialog2.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/desktop/source/lib/init.cxx")

            // needs to lock around access to methods in vcl
            && !loplugin::isSamePathname(fn, SRCDIR "/basctl/source/basicide/unomodel.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/cui/source/dialogs/AdditionsDialog.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/cui/source/dialogs/cuigaldlg.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/browser/unodatbr.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/uno/dbinteraction.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/dlg/DbAdminImpl.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/misc/UITools.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/desktop/source/lib/lokclipboard.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/editeng/source/misc/unolingu.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/framework/source/uielement/popuptoolbarcontroller.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/framework/source/uielement/newmenucontroller.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/framework/source/uielement/menubarwrapper.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/framework/source/services/desktop.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/framework/source/layoutmanager/layoutmanager.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/framework/source/layoutmanager/toolbarlayoutmanager.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/framework/source/fwe/helper/actiontriggerhelper.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/unodoc.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/filtuno.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/funcuno.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/vba/vbaapplication.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/unoidl/unodoc.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/unoidl/unomodule.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/remotecontrol/Receiver.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/sd/source/ui/slidesorter/controller/SlsClipboard.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/appl/fwkhelper.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/appl/appinit.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/appl/shutdownicon.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/dialog/dockwin.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/statbar/stbitem.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/toolbox/tbxitem.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/svtools/source/java/javainteractionhandler.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/svx/source/accessibility/ShapeTypeHandler.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/svx/source/tbxctrls/tbunosearchcontrollers.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/svx/source/form/fmscriptingenv.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/svx/source/fmcomp/fmgridif.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/awt/vclxspinbutton.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/awt/vclxtoolkit.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/toolkit/source/controls/tree/treecontrolpeer.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/ucb/source/ucp/image/ucpimage.cxx")
            && !loplugin::hasPathnamePrefix(fn, SRCDIR "/vcl/")

            // not sure
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/dbaccess/source/ui/browser/AsynchronousLink.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/framework/source/services/autorecovery.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/dialog/filedlghelper.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/appl/appdispatchprovider.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/ucb/source/ucp/tdoc/tdoc_storage.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/core/data/poolhelp.cxx")

            // touching shared global data
            && !loplugin::isSamePathname(fn, SRCDIR
                                         "/framework/source/fwi/classes/protocolhandlercache.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/basic/source/basmgr/basicmanagerrepository.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/basic/source/classes/sb.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/docuno.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/afmtuno.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/appluno.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/vba/vbaapplication.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/access/accdoc.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/access/acccontext.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/sw/source/core/bastyp/proofreadingiterator.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unoftn.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unolinebreak.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unoobj.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unorefmk.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unotbl.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unocontentcontrol.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unobkm.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unocoll.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/unocore/unostyle.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/xml/xmltexti.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/uno/dlelstnr.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/uno/unoatxt.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/uno/unodoc.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/uno/unomodule.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/uno/SwXFilterOptions.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/shells/translatehelper.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/sw/source/ui/vba/vbaapplication.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/unoxml/source/dom/documentbuilder.cxx")
            && !loplugin::isSamePathname(
                   fn, SRCDIR "/sd/source/ui/accessibility/AccessibleDrawDocumentView.cxx")
            && !loplugin::isSamePathname(fn, SRCDIR "/starmath/source/accessibility.cxx")
            && !loplugin::isSamePathname(fn,
                                         SRCDIR "/starmath/source/AccessibleSmElementsControl.cxx"))
        {
            report(DiagnosticsEngine::Warning, "unnecessary locking", compoundStmt->getBeginLoc())
                << compoundStmt->getSourceRange();
        }
    }

    m_TouchesThis.pop_back();
    m_IgnoreThis.pop_back();

    return true;
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

const CXXThisExpr* UnnecessaryLocking::isOtherMutexLockGuardStmt(const Stmt* stmt)
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
    if (!tc.Class("unique_lock").StdNamespace() && !tc.Class("scoped_lock").StdNamespace()
        && !tc.Class("Guard") && !tc.Class("ClearableGuard") && !tc.Class("ResettableGuard"))
        return nullptr;
    auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(varDecl->getInit());
    if (!cxxConstructExpr || cxxConstructExpr->getNumArgs() < 1)
        return nullptr;
    auto arg0 = cxxConstructExpr->getArg(0);
    if (auto memberExpr = dyn_cast<MemberExpr>(arg0))
    {
        const CXXThisExpr* thisStmt
            = dyn_cast<CXXThisExpr>(memberExpr->getBase()->IgnoreImplicit());
        return thisStmt;
    }
    else if (auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(arg0))
    {
        return dyn_cast_or_null<CXXThisExpr>(
            memberCallExpr->getImplicitObjectArgument()->IgnoreImplicit());
    }
    return nullptr;
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

bool UnnecessaryLocking::VisitCallExpr(const CallExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    // just in case
    if (m_TouchesThis.empty())
        return true;
    // already found something
    if (m_TouchesThis.back())
        return true;
    const CXXMethodDecl* callee = dyn_cast_or_null<CXXMethodDecl>(callExpr->getDirectCallee());
    if (!callee)
        return true;
    auto dc = loplugin::DeclCheck(callee->getParent());
    if (dc.Class("VCLUnoHelper") || dc.Class("Application"))
        m_TouchesThis.back() = true;
    return true;
}

/** off by default because each warning needs to be carefully inspected */
loplugin::Plugin::Registration<UnnecessaryLocking> unnecessarylocking("unnecessarylocking", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
