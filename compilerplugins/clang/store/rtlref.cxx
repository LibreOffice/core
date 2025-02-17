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

#include "plugin.hxx"
#include "check.hxx"
#include "config_clang.h"
#include <iostream>

/*
This is a compile-time checker.

Check for cases where we have
 - two IDL interfaces A and B,
 - B extends A
 - we are converting a Reference<B> to a Reference<A> using UNO_QUERY

This makes the code simpler and cheaper, because UNO_QUERY can be surprisingly expensive if used a lot.

*/

namespace
{
class RtlRef : public loplugin::FilteringPlugin<RtlRef>
{
public:
    explicit RtlRef(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        if (fn == SRCDIR "/ucb/source/ucp/ext/ucpext_provider.cxx")
            return false;
        if (fn == SRCDIR "/ucb/source/ucp/hierarchy/hierarchyprovider.cxx")
            return false;
        if (fn == SRCDIR "/ucb/source/ucp/cmis/cmis_provider.cxx")
            return false;
        if (fn == SRCDIR "/ucb/source/ucp/webdav-curl/webdavprovider.cxx")
            return false;
        if (fn == SRCDIR "/ucb/source/ucp/webdav-curl/webdavresponseparser.cxx")
            return false;
        if (fn == SRCDIR "/ucb/source/cacher/dynamicresultsetwrapper.cxx")
            return false;
        if (fn == SRCDIR "/sax/source/fastparser/fastparser.cxx")
            return false;
        if (fn == SRCDIR "/chart2/source/controller/main/ObjectHierarchy.cxx")
            return false;
        if (fn == SRCDIR "/chart2/source/controller/main/ShapeController.cxx")
            return false;
        if (fn == SRCDIR "/chart2/source/view/axes/VCartesianAxis.cxx")
            return false;
        if (fn == SRCDIR "/chart2/source/controller/chartapiwrapper/ChartDocumentWrapper.cxx")
            return false;
        if (fn == SRCDIR "/chart2/source/view/main/DataTableView.cxx")
            return false;
        if (fn == SRCDIR "/svx/source/form/datanavi.cxx")
            return false;
        if (fn == SRCDIR "/sc/source/ui/Accessibility/AccessibleSpreadsheet.cxx")
            return false;
        if (fn == SRCDIR "/sd/source/ui/annotations/annotationwindow.cxx")
            return false;
        if (fn == SRCDIR "/sd/source/ui/unoidl/unopage.cxx")
            return false;
        if (fn == SRCDIR "/xmlsecurity/source/helper/xsecsign.cxx")
            return false;
        if (fn == SRCDIR "/xmlsecurity/source/helper/xsecctl.cxx")
            return false;
        if (fn == SRCDIR "/unoxml/source/events/eventdispatcher.cxx")
            return false;
        if (fn == SRCDIR "/unoxml/source/dom/element.cxx")
            return false;
        return true;
    }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitVarDecl(const VarDecl*);
};

bool RtlRef::VisitVarDecl(const VarDecl* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;

    // don't bother processing anything in the Reference.h file. Makes my life easier when debugging this.
    StringRef aFileName
        = getFilenameOfLocation(compiler.getSourceManager().getSpellingLoc(varDecl->getBeginLoc()));
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/com/sun/star/uno/Reference.h"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/com/sun/star/uno/Reference.hxx"))
        return true;

    if (!loplugin::TypeCheck(varDecl->getType()).Class("Reference").Namespace("uno"))
        return true;

    if (varDecl->isStaticLocal())
        return true;

    auto init = varDecl->getInit();
    if (!init)
        return true;
    init = init->IgnoreImpCasts();

    auto memberCall = dyn_cast<CXXMemberCallExpr>(init);
    if (!memberCall)
        return true;
    if (memberCall->getNumArgs() > 1)
        return true;

    auto innerExpr = memberCall->getImplicitObjectArgument()->IgnoreImpCasts();
    if (!loplugin::TypeCheck(innerExpr->getType()).Class("Reference").Namespace("rtl"))
        return true;

    report(DiagnosticsEngine::Warning, "cast from %0", varDecl->getBeginLoc())
        << innerExpr->getType() << varDecl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<RtlRef> rtlref("rtlref");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
