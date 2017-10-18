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

#include <clang/AST/CXXInheritance.h>
#include "compat.hxx"
#include "plugin.hxx"
#include "check.hxx"

/**
 Look for calls to the ref-counting methods acquire()/release(), which should only be called by classes like rtl::Reference.
*/

namespace {

class ManualRefCount:
    public RecursiveASTVisitor<ManualRefCount>, public loplugin::Plugin
{
public:
    explicit ManualRefCount(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        StringRef fn( compiler.getSourceManager().getFileEntryForID(
                          compiler.getSourceManager().getMainFileID())->getName() );

       // old code, no point in updating
        if (loplugin::isSamePathname(fn, SRCDIR "/store/source/store.cxx"))
             return;

// TODO -----------------------------
        if (loplugin::isSamePathname(fn, SRCDIR "/registry/source/registry.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/registry/source/regimpl.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/registry/source/reflread.cxx"))
             return;
        // TODO MenuAttributes::CreateAttribute
        if (loplugin::isSamePathname(fn, SRCDIR "/framework/source/fwe/xml/menuconfiguration.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/app/apphdl.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/core/dataaccess/ModelImpl.cxx"))
             return;
        // need a better replacement for vcl::EventPoster
        if (loplugin::isSamePathname(fn, SRCDIR "/svtools/source/misc/acceleratorexecute.cxx"))
             return;
        // PostUserEvent stuff
        if (loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/awt/vclxwindow.cxx"))
             return;
        // playing games with pointers passed into combobox entries
        if (loplugin::isSamePathname(fn, SRCDIR "/cui/source/customize/cfgutil.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/cui/source/customize/cfg.cxx"))
             return;
// END TODO -----------------------------

        // can't fix these without breaking stable ABI
        if (fn.startswith(SRCDIR "/sal/"))
             return;
        if (fn.startswith(SRCDIR "/salhelper/"))
             return;
        if (fn.startswith(SRCDIR "/cppu/"))
             return;
        if (fn.startswith(SRCDIR "/cppuhelper/"))
             return;
        if (fn.startswith(SRCDIR "/bridges/"))
             return;

        // lots of magic here
        if (fn.startswith(SRCDIR "/stoc/"))
             return;
        if (fn.startswith(SRCDIR "/testtools/"))
             return;

        // mutex games
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/app/scheduler.cxx"))
             return;
        // opengl games
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/app/svdata.cxx"))
             return;

        // passing the pointer through PostUserEvent
        if (loplugin::isSamePathname(fn, SRCDIR "/avmedia/source/gstreamer/gstplayer.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/form/fmscriptingenv.cxx"))
             return;

        // thread games
        if (loplugin::isSamePathname(fn, SRCDIR "/io/source/stm/opump.cxx"))
             return;

        // ??? no idea what this code is up to
        if (loplugin::isSamePathname(fn, SRCDIR "/extensions/source/scanner/scanunx.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/stoc/source/invocation_adapterfactory/iafactory.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/fpicker/source/office/asyncfilepicker.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/component/FormComponent.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/ucb/source/ucp/file/bc.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/ucb/source/ucp/file/filprp.cxx"))
             return;
        // calling release() ?
        if (loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/helper/accessibilityclient.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svtools/source/misc/svtaccessiblefactory.cxx"))
             return;

        // implementing css::uno::XInterface
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/animations/motionpathtag.cxx"))
             return;
        // UNO factory methods
        if (fn.startswith(SRCDIR "/comphelper/"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/linguistic/source/convdiclist.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/linguistic/source/dlistimp.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/linguistic/source/gciterator.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/linguistic/source/lngsvcmgr.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/linguistic/source/lngopt.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/unx/generic/gdi/gcach_xpeer.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/dlg/dbwizsetup.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/dlg/dbwizsetup.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lingucomponent/source/hyphenator/hyphen/hyphenimp.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lingucomponent/source/spellcheck/spell/sspellimp.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lingucomponent/source/thesaurus/libnth/nthesimp.cxx"))
             return;


        // some kind of complicated listener nonsense
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/framework/tools/FrameworkHelper.cxx"))
             return;
        // more listener nonsense
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/uno/unomailmerge.cxx"))
             return;
        // playing games with it's listener list
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/cellsuno.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/chart2uno.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/dapiuno.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/datauno.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/linkuno.cxx"))
             return;
        // PostUserEvent
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/vba/vbaeventshelper.cxx"))
             return;
        // thread holding itself
        if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/component/EventThread.cxx"))
             return;


        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *);
    bool TraverseCXXRecordDecl(CXXRecordDecl *);
    bool TraverseCXXMethodDecl(CXXMethodDecl *);
    bool TraverseFunctionDecl(FunctionDecl *);
    bool TraverseCXXConstructorDecl(CXXConstructorDecl *);
    bool TraverseCXXDestructorDecl(CXXDestructorDecl *);
    bool TraverseCXXConversionDecl(CXXConversionDecl *);
    bool TraverseClassTemplateSpecializationDecl(ClassTemplateSpecializationDecl *);
    bool TraverseLinkageSpecDecl(LinkageSpecDecl *);
private:
    bool ignoreCallerClass(CXXRecordDecl*);
};

bool ManualRefCount::TraverseCXXMethodDecl(CXXMethodDecl* cxxMethodDecl)
{
    if (ignoreCallerClass(cxxMethodDecl->getParent()))
        return true;
    // disambiguating forwarding methods for XInterface subclasses
    if (cxxMethodDecl->getIdentifier() && (cxxMethodDecl->getName() == "acquire" || cxxMethodDecl->getName() == "release"))
        return true;
    return RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
}

bool ManualRefCount::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    auto tc = loplugin::DeclCheck(functionDecl);
    if (tc.Function("make_shared_from_UNO").Namespace("comphelper").GlobalNamespace())
        return true;
    return RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
}

bool ManualRefCount::TraverseCXXConstructorDecl(CXXConstructorDecl* cxxMethodDecl)
{
    if (ignoreCallerClass(cxxMethodDecl->getParent()))
        return true;
    return RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
}

bool ManualRefCount::TraverseCXXDestructorDecl(CXXDestructorDecl*)
{
    // just ignore destructors, tons of places like to call acquire() on themselves in their destructor
    // supposedly to prevent recursively calling the destructor
    return true;
}
bool ManualRefCount::TraverseCXXConversionDecl(CXXConversionDecl* cxxMethodDecl)
{
    if (ignoreCallerClass(cxxMethodDecl->getParent()))
        return true;
    return RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
}
bool ManualRefCount::TraverseCXXRecordDecl(CXXRecordDecl* cxxRecordDecl)
{
    if (ignoreCallerClass(cxxRecordDecl))
        return true;
    return RecursiveASTVisitor::TraverseCXXRecordDecl(cxxRecordDecl);
}

bool ManualRefCount::TraverseClassTemplateSpecializationDecl(ClassTemplateSpecializationDecl* templateDecl)
{
    if (ignoreCallerClass(templateDecl))
        return true;
    return RecursiveASTVisitor::TraverseClassTemplateSpecializationDecl(templateDecl);
}

bool ManualRefCount::TraverseLinkageSpecDecl(LinkageSpecDecl *)
{
    // ignore methods inside "extern ""C""" blocks, these are normally UNO constructors, which
    // are required to raise the reference count before returning
    return true;
}

bool ManualRefCount::ignoreCallerClass(CXXRecordDecl* cxxRecordDecl)
{
    auto tc = loplugin::TypeCheck(cxxRecordDecl);
    return
           tc.Class("Reference").Namespace("rtl").GlobalNamespace()
        || tc.Class("cow_wrapper").Namespace("o3tl").GlobalNamespace()
        || tc.Class("Reference").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()
        || tc.Class("ShareGuard").Namespace("framework").GlobalNamespace()
        || tc.Class("ControlModelLock").Namespace("frm").GlobalNamespace()
        || tc.Struct("ReleaseFunc").Namespace("detail").Namespace("comphelper").GlobalNamespace()
        // TODO no idea what this is up to
        || tc.Class("SfxModelSubComponent").GlobalNamespace()
        || tc.Class("OSubComponent").Namespace("mysqlc").Namespace("connectivity").GlobalNamespace()
        || tc.Class("OSubComponent").Namespace("connectivity").GlobalNamespace()
        // TODO do we really need this?
        || tc.Class("ShareableMutex").Namespace("framework").GlobalNamespace()
        || tc.Class("ObservableThread").GlobalNamespace()
        ;
}

bool ManualRefCount::VisitCXXMemberCallExpr(const CXXMemberCallExpr* cxxMemberCallExpr)
{
    if (ignoreLocation(cxxMemberCallExpr))
        return true;
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(cxxMemberCallExpr->getLocStart())))
        return true;

    // first, use some heuristics to find the right kind of acquire()/release() calls
    CXXMethodDecl const * calleeMethodDecl = cxxMemberCallExpr->getMethodDecl();
    if (!calleeMethodDecl || !calleeMethodDecl->getIdentifier())
        return true;
    if (calleeMethodDecl->getName() != "acquire" && calleeMethodDecl->getName() != "release")
        return true;
    if (calleeMethodDecl->getNumParams() != 0)
        return true;
    // std::unique_ptr::release() and similar methods
    if (calleeMethodDecl->getName() == "release" && loplugin::TypeCheck(calleeMethodDecl->getReturnType()).Pointer())
        return true;

    // these are OK
    auto calleeRecordTC = loplugin::TypeCheck(calleeMethodDecl->getParent());
    if (calleeRecordTC.Struct("ResourceHolder").Namespace("store").GlobalNamespace())
        return true;
    if (calleeRecordTC.Class("Module").Namespace("osl").GlobalNamespace())
        return true;
    if (calleeRecordTC.Class("Mutex").Namespace("osl").GlobalNamespace())
        return true;
    if (calleeRecordTC.Class("multi_type_vector").Namespace("mdds").GlobalNamespace())
        return true;

//    while (calleeMethodDecl->size_overridden_methods() > 0)
//        calleeMethodDecl = *calleeMethodDecl->begin_overridden_methods();
//    auto tc2 = loplugin::TypeCheck(calleeMethodDecl->getParent());
//    if (tc2.Class("XInterface").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace())
//        return true;

std::cout << calleeMethodDecl->getParent()->getQualifiedNameAsString() << std::endl;
    report(
        DiagnosticsEngine::Warning, "call to acquire/release",
        cxxMemberCallExpr->getLocStart())
        << cxxMemberCallExpr->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< ManualRefCount > X("manualrefcount", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
