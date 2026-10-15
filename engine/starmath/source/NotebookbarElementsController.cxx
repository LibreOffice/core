/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <comphelper/compbase.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/kit/componenthelpers.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <svl/hint.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svl/itemset.hxx>
#include <svl/lstner.hxx>
#include <svl/stritem.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>

#include <ElementsDockingWindow.hxx>
#include <NotebookbarElements.hxx>
#include <document.hxx>
#include <smmod.hxx>
#include <starmath.hrc>
#include <strings.hrc>
#include <view.hxx>

#include <map>
#include <memory>
#include <set>
#include <vector>

using namespace css;
using namespace ::cpo;

namespace
{
std::set<ViewShellId>& ElementsHosts()
{
    static std::set<ViewShellId> aHosts;
    return aHosts;
}

std::map<ViewShellId, int>& LastCategories()
{
    static std::map<ViewShellId, int> aCategories;
    return aCategories;
}

struct CategoryFollower
{
    ViewShellId mnViewId;
    std::function<void(int)> maFollow;
};

std::map<const void*, CategoryFollower>& CategoryFollowers()
{
    static std::map<const void*, CategoryFollower> aFollowers;
    return aFollowers;
}

/** Points the sidebar of the host view at the formula edited in place in it,
    which shows the Elements deck, or back at the host document.

    The sidebar follows the context of every controller it listens to, and a
    listener added to a controller is told its current context straight away. */
void PointSidebarAt(SfxViewShell& rHostView, SmViewShell& rFormulaView, bool bFormula)
{
    sfx2::sidebar::SidebarController* pSidebar
        = sfx2::sidebar::SidebarController::GetSidebarControllerForView(&rHostView);
    if (!pSidebar)
        return;

    const uno::Reference<frame::XController> xFormula = rFormulaView.GetController();
    const uno::Reference<frame::XController> xHost = rHostView.GetController();
    const uno::Reference<ui::XContextChangeEventListener> xListener(pSidebar);
    try
    {
        const uno::Reference<ui::XContextChangeEventMultiplexer> xMultiplexer
            = css::ui::ContextChangeEventMultiplexer::get(comphelper::getProcessComponentContext());
        if (!bFormula)
            xMultiplexer->removeContextChangeEventListener(xListener, xFormula);

        const uno::Reference<frame::XController>& xFocus = bFormula ? xFormula : xHost;
        xMultiplexer->removeContextChangeEventListener(xListener, xFocus);
        xMultiplexer->addContextChangeEventListener(xListener, xFocus);
    }
    catch (const cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("starmath", "cannot move the sidebar to the formula context");
        return;
    }

    SfxViewShell& rFocusView = bFormula ? static_cast<SfxViewShell&>(rFormulaView) : rHostView;
    if (SfxObjectShell* pDocShell = rFocusView.GetObjectShell())
        pSidebar->updateModel(pDocShell->GetModel());

    if (comphelper::COKit::isActive())
        jsdialog::SendSidebarForView(reinterpret_cast<sal_uInt64>(&rHostView));
}

/// The group sections of notebookbarelements_math.ui: enough for the category
/// with the most groups (Operators).
constexpr int nElementGroups = 15;

typedef comphelper::WeakComponentImplHelper<css::lang::XInitialization, css::lang::XServiceInfo,
                                            css::ui::XContextChangeEventListener>
    NotebookbarElementsControllerBase;

/** Drives the element categories box and the element group icon views welded
    into the notebookbar (smath/ui/notebookbarelements_math.ui).

    The selected category is split at its separators, and each group goes into
    an icon view of its own, which the Formula tab shows as a section named
    after the group. The sections a category does not need are hidden.

    This is the counterpart of the Math Elements sidebar panel for a .ui welded
    into the notebookbar rather than into a sidebar panel.

    The notebookbar is welded while the document is still loading, and every
    document gets one whether or not it will ever hold a formula, so nothing
    Math-specific is built there: the elements need a document shell of their
    own and an image rendered per element, and building them that early leaves
    the view half set up - the sidebar never opens. The sections fill themselves
    the first time a formula is edited, which is the first time they are reachable:
    only then does the Formula tab that carries them appear.

    Every formula entered anywhere is reported here, so each is checked to be
    one this view edits before anything is built or listened to. */
class NotebookbarElementsController final : public NotebookbarElementsControllerBase,
                                            public SfxListener
{
public:
    NotebookbarElementsController() {}
    NotebookbarElementsController(const NotebookbarElementsController&) = delete;
    NotebookbarElementsController& operator=(const NotebookbarElementsController&) = delete;

    void initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

    OUString getImplementationName() override
    {
        return u"com.sun.star.comp.starmath.NotebookbarElementsController"_ustr;
    }

    bool supportsService(const OUString& rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override
    {
        return { u"com.sun.star.starmath.NotebookbarElementsController"_ustr };
    }

    // XContextChangeEventListener
    void notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent) override;

    // XEventListener
    void disposing(const css::lang::EventObject&) override {}

    void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

private:
    void disposing(std::unique_lock<std::mutex>& rGuard) override;

    DECL_LINK(FormulaEnteredHdl, void*, void);
    DECL_LINK(InFormulaTabToggledHdl, weld::Toggleable&, void);
    DECL_LINK(CategorySelectedHandle, weld::ComboBox&, void);
    DECL_LINK(ElementClickHandler, const OUString&, void);

    void StopListeningForFormulas();
    void BuildElements();

    /// Fills a group section per group of the category and hides the rest.
    void ShowCategory(int nCategory, bool bForceBuild);

    /// a formula saved as user-defined refreshes the categories.
    SmViewShell* GetView();

    weld::Builder* mpBuilder = nullptr;
    SfxBindings* mpBindings = nullptr;
    ViewShellId mnHostViewId = ViewShellId(-1);
    ImplSVEvent* mpFormulaEnteredEvent = nullptr;
    bool mbListeningForFormulas = false;

    std::unique_ptr<weld::CheckButton> mxInFormulaTab;
    std::unique_ptr<weld::ComboBox> mxCategoryList;
    std::vector<std::unique_ptr<SmElementsControl>> maElementsControls;
    std::vector<std::unique_ptr<weld::Widget>> maGroupSections;
};

void NotebookbarElementsController::initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments)
{
    const comphelper::NamedValueCollection aArguments(rArguments);
    uno::Reference<awt::XWindow> xParentWindow(
        aArguments.getOrDefault(u"ParentWindow"_ustr, uno::Reference<awt::XWindow>()));
    const sal_uInt64 nBindings(aArguments.getOrDefault(u"SfxBindings"_ustr, sal_uInt64(0)));

    if (weld::TransportAsXWindow* pTunnel
        = dynamic_cast<weld::TransportAsXWindow*>(xParentWindow.get()))
        mpBuilder = pTunnel->getBuilder();

    if (!mpBuilder || !nBindings)
        return;

    mpBindings = reinterpret_cast<SfxBindings*>(nBindings);

    if (mpBindings->GetDispatcher() && mpBindings->GetDispatcher()->GetFrame())
    {
        if (const SfxViewShell* pHostView = mpBindings->GetDispatcher()->GetFrame()->GetViewShell())
            mnHostViewId = pHostView->GetViewShellId();
    }

    mxInFormulaTab = mpBuilder->weld_check_button(u"elementsinformulatab"_ustr);
    if (mxInFormulaTab)
    {
        mxInFormulaTab->connect_toggled(
            LINK(this, NotebookbarElementsController, InFormulaTabToggledHdl));
        mxInFormulaTab->set_active(false);
    }

    try
    {
        css::ui::ContextChangeEventMultiplexer::get(comphelper::getProcessComponentContext())
            ->addContextChangeEventListener(this, nullptr);
        mbListeningForFormulas = true;
    }
    catch (const cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("starmath", "cannot listen for the formula context");
    }
}

void NotebookbarElementsController::StopListeningForFormulas()
{
    if (!mbListeningForFormulas)
        return;

    mbListeningForFormulas = false;
    try
    {
        css::ui::ContextChangeEventMultiplexer::get(comphelper::getProcessComponentContext())
            ->removeContextChangeEventListener(this, nullptr);
    }
    catch (const cpo::uno::Exception&)
    {
    }
}

void NotebookbarElementsController::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
{
    StopListeningForFormulas();
    sm::notebookbar::SetHostsElements(mnHostViewId, false);
    mnHostViewId = ViewShellId(-1);

    if (mpFormulaEnteredEvent)
    {
        Application::RemoveUserEvent(mpFormulaEnteredEvent);
        mpFormulaEnteredEvent = nullptr;
    }

    EndListeningAll();
    mxInFormulaTab.reset();
    maElementsControls.clear();
    maGroupSections.clear();
    mxCategoryList.reset();
    mpBindings = nullptr;
    mpBuilder = nullptr;
}

void NotebookbarElementsController::notifyContextChangeEvent(
    const css::ui::ContextChangeEventObject& rEvent)
{
    if (mpFormulaEnteredEvent || !mpBuilder)
        return;

    if (rEvent.ApplicationName
        != vcl::EnumContext::GetApplicationName(vcl::EnumContext::Application::Formula))
        return;

    mpFormulaEnteredEvent
        = Application::PostUserEvent(LINK(this, NotebookbarElementsController, FormulaEnteredHdl));
}

IMPL_LINK(NotebookbarElementsController, InFormulaTabToggledHdl, weld::Toggleable&, rButton, void)
{
    const bool bInFormulaTab = rButton.get_active();
    sm::notebookbar::SetHostsElements(mnHostViewId, bInFormulaTab);

    SmViewShell* pFormulaView = GetView();
    if (!pFormulaView || !mpBindings || !mpBindings->GetDispatcher()
        || !mpBindings->GetDispatcher()->GetFrame())
        return;

    SfxViewShell* pHostView = mpBindings->GetDispatcher()->GetFrame()->GetViewShell();
    if (!pHostView || pHostView == pFormulaView)
        return;

    if (bInFormulaTab && maElementsControls.empty())
        BuildElements();

    PointSidebarAt(*pHostView, *pFormulaView, !bInFormulaTab);
}

IMPL_LINK_NOARG(NotebookbarElementsController, FormulaEnteredHdl, void*, void)
{
    mpFormulaEnteredEvent = nullptr;

    if (!mpBuilder || !GetView())
        return;

    // nothing to show when tab is hidden
    if (maElementsControls.empty() && mxInFormulaTab && mxInFormulaTab->get_active())
        BuildElements();
}

void NotebookbarElementsController::BuildElements()
{
    mxCategoryList = mpBuilder->weld_combo_box(u"categorylist"_ustr);
    for (const auto& rCategoryId : SmElementsControl::categories())
        mxCategoryList->append_text(SmResId(rCategoryId));

    for (int i = 0; i < nElementGroups; ++i)
    {
        const OUString sGroup = "elements_group" + OUString::number(i);
        // only user-defined formulas can be deleted
        auto pControl = std::make_unique<SmElementsControl>(
            mpBuilder->weld_icon_view(sGroup),
            i == 0 ? mpBuilder->weld_menu(u"deletemenu"_ustr) : nullptr);
        pControl->SetSelectHdl(LINK(this, NotebookbarElementsController, ElementClickHandler));
        maElementsControls.push_back(std::move(pControl));
        maGroupSections.push_back(mpBuilder->weld_widget(sGroup + "-group"));
    }

    mxCategoryList->connect_changed(
        LINK(this, NotebookbarElementsController, CategorySelectedHandle));

    // The Formula tab is welded anew on UI change
    const int nCategory = sm::notebookbar::GetCategory(mnHostViewId);
    mxCategoryList->set_active(nCategory);

    for (int i = SmElementsControl::groupCount(nCategory); i < nElementGroups; ++i)
    {
        if (maGroupSections[i])
            maGroupSections[i]->show();
    }
    ShowCategory(nCategory, false);
}

void NotebookbarElementsController::ShowCategory(int nCategory, bool bForceBuild)
{
    SmViewShell* pViewSh = GetView();
    const int nGroups = SmElementsControl::groupCount(nCategory);

    for (int i = 0; i < nElementGroups; ++i)
    {
        weld::Widget* pSection = maGroupSections[i].get();
        if (i >= nGroups)
        {
            if (pSection)
                pSection->hide();
            continue;
        }

        SmElementsControl& rControl = *maElementsControls[i];
        // Online shows it as the name of the section.
        rControl.SetAccessibleName(SmElementsControl::groupName(nCategory, i));
        rControl.SetAllowDelete(SmElementsControl::categories()[nCategory]
                                == RID_CATEGORY_USERDEFINED);
        rControl.setElementSetIndex(nCategory, bForceBuild, i);
        if (pViewSh)
            rControl.setSmSyntaxVersion(pViewSh->GetDoc()->GetSmSyntaxVersion());
        if (pSection)
            pSection->show();
    }
}

void NotebookbarElementsController::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SmNewUserFormula || !mxCategoryList)
        return;

    mxCategoryList->set_active_text(SmResId(RID_CATEGORY_USERDEFINED));
    sm::notebookbar::SetCategory(mnHostViewId, mxCategoryList->get_active());
    ShowCategory(mxCategoryList->get_active(), true);
}

IMPL_LINK(NotebookbarElementsController, CategorySelectedHandle, weld::ComboBox&, rList, void)
{
    const int nActive = rList.get_active();
    if (nActive == -1)
        return;

    sm::notebookbar::SetCategory(mnHostViewId, nActive);
    ShowCategory(nActive, false);
}

IMPL_LINK(NotebookbarElementsController, ElementClickHandler, const OUString&, ElementSource, void)
{
    if (SmViewShell* pViewSh = GetView())
    {
        SfxStringItem aInsertCommand(SID_INSERTCOMMANDTEXT, ElementSource);
        pViewSh->GetViewFrame().GetDispatcher()->ExecuteList(
            SID_INSERTCOMMANDTEXT, SfxCallMode::SLOT, { &aInsertCommand });
    }
}

SmViewShell* NotebookbarElementsController::GetView()
{
    if (!mpBindings || !mpBindings->GetDispatcher() || !mpBindings->GetDispatcher()->GetFrame())
        return nullptr;

    SfxViewShell* pView = mpBindings->GetDispatcher()->GetFrame()->GetViewShell();
    SmViewShell* pSmViewShell = dynamic_cast<SmViewShell*>(pView);
    if (!pSmViewShell && comphelper::COKit::isActive())
    {
        auto* pWindow = static_cast<SmGraphicWindow*>(KitStarMathHelper(pView).GetGraphicWindow());
        if (pWindow)
            pSmViewShell = &pWindow->GetGraphicWidget().GetView();
    }

    if (pSmViewShell && !IsListening(*pSmViewShell))
    {
        EndListeningAll();
        StartListening(*pSmViewShell);
    }

    return pSmViewShell;
}
}

namespace sm::notebookbar
{
bool HostsElements(const SfxViewShell* pView)
{
    return pView && ElementsHosts().count(pView->GetViewShellId());
}

void SetHostsElements(ViewShellId nViewId, bool bHosts)
{
    if (nViewId == ViewShellId(-1))
        return;

    if (bHosts)
        ElementsHosts().insert(nViewId);
    else
        ElementsHosts().erase(nViewId);
}

int GetCategory(ViewShellId nViewId)
{
    const auto it = LastCategories().find(nViewId);
    if (it == LastCategories().end() || it->second < 0
        || o3tl::make_unsigned(it->second) >= SmElementsControl::categories().size())
        return 0;
    return it->second;
}

void SetCategory(ViewShellId nViewId, int nCategory)
{
    if (nViewId == ViewShellId(-1))
        return;

    LastCategories()[nViewId] = nCategory;
    for (const auto& [pOwner, rFollower] : CategoryFollowers())
    {
        if (rFollower.mnViewId == nViewId)
            rFollower.maFollow(nCategory);
    }
}

void FollowCategory(const void* pOwner, ViewShellId nViewId, std::function<void(int)> aFollow)
{
    CategoryFollowers()[pOwner] = { nViewId, std::move(aFollow) };
}

void UnfollowCategory(const void* pOwner) { CategoryFollowers().erase(pOwner); }

SfxViewShell* GetHostView(const cpo::uno::Reference<css::frame::XFrame>& xFrame)
{
    if (!xFrame.is())
        return nullptr;

    const cpo::uno::Reference<css::frame::XFramesSupplier> xCreator = xFrame->getCreator();
    if (!xCreator.is())
        return nullptr;

    return SfxViewShell::Get(xCreator->getController());
}
}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
com_sun_star_comp_starmath_NotebookbarElementsController_get_implementation(
    cpo::uno::XComponentContext*, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new NotebookbarElementsController);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
