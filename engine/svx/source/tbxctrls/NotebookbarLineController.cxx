/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <LineEndStyleBoxes.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/compbase.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <array>
#include <svl/itemset.hxx>
#include <svx/svxids.hrc>
#include <svx/xlineit0.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <vcl/weldutils.hxx>

using namespace css;

namespace
{
typedef comphelper::WeakComponentImplHelper<css::lang::XInitialization, css::lang::XServiceInfo>
    NotebookbarLineControllerBase;
typedef sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface ItemUpdateReceiver;

/** Drives the arrow style boxes of the Line section welded into the
    notebookbar (svx/ui/notebookbarshapeline.ui).

    The boxes themselves are the ones the sidebar Line panel uses; this is the
    counterpart of svx::sidebar::LinePropertyPanel which keeps them up to date
    and applies what is picked in them, for a .ui welded into the notebookbar
    rather than into a sidebar panel. */
class NotebookbarLineController final : public NotebookbarLineControllerBase,
                                        public ItemUpdateReceiver
{
public:
    NotebookbarLineController() {}
    NotebookbarLineController(const NotebookbarLineController&) = delete;
    NotebookbarLineController& operator=(const NotebookbarLineController&) = delete;

    void initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

    OUString getImplementationName() override
    {
        return u"com.sun.star.comp.svx.NotebookbarLineController"_ustr;
    }

    bool supportsService(const OUString& rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override
    {
        return { u"com.sun.star.svx.NotebookbarLineController"_ustr };
    }

    void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                          const SfxPoolItem* pState) override;
    void GetControlState(const sal_uInt16 /*nSId*/,
                         boost::property_tree::ptree& /*rState*/) override
    {
    }

private:
    void disposing(std::unique_lock<std::mutex>& rGuard) override;

    void Execute(sal_uInt16 nSlot, const SfxPoolItem& rItem);

    std::unique_ptr<svx::LineEndStyleBoxes> mxArrowStyles;
    std::array<std::unique_ptr<sfx2::sidebar::ControllerItem>, 4> maControllers;
    SfxBindings* mpBindings = nullptr;
};

void
NotebookbarLineController::initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments)
{
    const comphelper::NamedValueCollection aArguments(rArguments);
    uno::Reference<frame::XFrame> xFrame(
        aArguments.getOrDefault(u"Frame"_ustr, uno::Reference<frame::XFrame>()));
    uno::Reference<awt::XWindow> xParentWindow(
        aArguments.getOrDefault(u"ParentWindow"_ustr, uno::Reference<awt::XWindow>()));
    const sal_uInt64 nBindings(aArguments.getOrDefault(u"SfxBindings"_ustr, sal_uInt64(0)));

    weld::Builder* pBuilder = nullptr;
    if (weld::TransportAsXWindow* pTunnel
        = dynamic_cast<weld::TransportAsXWindow*>(xParentWindow.get()))
        pBuilder = pTunnel->getBuilder();

    if (!pBuilder || !xFrame.is() || !nBindings)
        return;

    mpBindings = reinterpret_cast<SfxBindings*>(nBindings);

    mxArrowStyles.reset(new svx::LineEndStyleBoxes(
        pBuilder->weld_label(u"startarrowlabel"_ustr),
        pBuilder->weld_combo_box(u"startarrowstyle"_ustr),
        pBuilder->weld_label(u"endarrowlabel"_ustr),
        pBuilder->weld_combo_box(u"endarrowstyle"_ustr),
        [this](const XLineStartItem& rItem) { Execute(SID_ATTR_LINE_START, rItem); },
        [this](const XLineEndItem& rItem) { Execute(SID_ATTR_LINE_END, rItem); }, xFrame));

    // the list first, the boxes have nothing to select from without it
    static constexpr sal_uInt16 aSlots[]
        = { SID_LINEEND_LIST, SID_ATTR_LINE_STYLE, SID_ATTR_LINE_START, SID_ATTR_LINE_END };
    static_assert(std::size(aSlots) == std::tuple_size_v<decltype(maControllers)>);

    for (size_t i = 0; i < std::size(aSlots); ++i)
    {
        maControllers[i].reset(new sfx2::sidebar::ControllerItem(aSlots[i], *mpBindings, *this));
        maControllers[i]->RequestUpdate();
    }
}

void NotebookbarLineController::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
{
    for (auto& rController : maControllers)
    {
        if (rController)
            rController->dispose();
        rController.reset();
    }

    mxArrowStyles.reset();
    mpBindings = nullptr;
}

void NotebookbarLineController::Execute(sal_uInt16 nSlot, const SfxPoolItem& rItem)
{
    if (mpBindings && mpBindings->GetDispatcher())
        mpBindings->GetDispatcher()->ExecuteList(nSlot, SfxCallMode::RECORD, { &rItem });
}

void NotebookbarLineController::NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                                 const SfxPoolItem* pState)
{
    if (!mxArrowStyles)
        return;

    if (mxArrowStyles->NotifyItemUpdate(nSId, eState, pState))
        return;

    if (SID_ATTR_LINE_STYLE == nSId)
    {
        const XLineStyleItem* pItem = eState >= SfxItemState::DEFAULT
                                          ? dynamic_cast<const XLineStyleItem*>(pState)
                                          : nullptr;
        mxArrowStyles->set_sensitive(!pItem || pItem->GetValue() != drawing::LineStyle_NONE);
    }
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_NotebookbarLineController_get_implementation(
    css::uno::XComponentContext*, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new NotebookbarLineController);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
