/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/zforlist.hxx>
#include <vcl/builderpage.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Button.hxx>
#include <vcl/weld/FormattedSpinButton.hxx>
#include <vcl/weld/TreeView.hxx>
#include <vcl/weld/weldutils.hxx>

BuilderPage::BuilderPage(weld::Widget* pParent, weld::DialogController* pController,
                         const OUString& rUIXMLDescription, const OUString& rID)
    : m_pDialogController(pController)
    , m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , m_xContainer(m_xBuilder->weld_container(rID))
{
}

void BuilderPage::Activate() {}

void BuilderPage::Deactivate() {}

BuilderPage::~BuilderPage() {}

namespace weld
{
size_t GetAbsPos(const weld::TreeView& rTreeView, const weld::TreeIter& rIter)
{
    size_t nAbsPos = 0;

    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator(&rIter));
    if (!rTreeView.get_iter_first(*xEntry))
        xEntry.reset();

    while (xEntry && rTreeView.iter_compare(*xEntry, rIter) != 0)
    {
        if (!rTreeView.iter_next(*xEntry))
            xEntry.reset();
        nAbsPos++;
    }

    return nAbsPos;
}

bool IsEntryVisible(const weld::TreeView& rTreeView, const weld::TreeIter& rIter)
{
    // short circuit for the common case
    if (rTreeView.get_iter_depth(rIter) == 0)
        return true;

    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator(&rIter));
    bool bRetVal = false;
    do
    {
        if (rTreeView.get_iter_depth(*xEntry) == 0)
        {
            bRetVal = true;
            break;
        }
    } while (rTreeView.iter_parent(*xEntry) && rTreeView.get_row_expanded(*xEntry));
    return bRetVal;
}

void RemoveParentKeepChildren(weld::TreeView& rTreeView, const weld::TreeIter& rParent)
{
    if (rTreeView.iter_has_child(rParent))
    {
        std::unique_ptr<weld::TreeIter> xNewParent(rTreeView.make_iterator(&rParent));
        if (!rTreeView.iter_parent(*xNewParent))
            xNewParent.reset();

        while (true)
        {
            std::unique_ptr<weld::TreeIter> xChild(rTreeView.make_iterator(&rParent));
            if (!rTreeView.iter_children(*xChild))
                break;
            rTreeView.move_subtree(*xChild, xNewParent.get(), -1);
        }
    }
    rTreeView.remove(rParent);
}

WidgetStatusListener::WidgetStatusListener(weld::Widget* widget, const OUString& aCommand)
    : mWidget(widget)
{
    const css::uno::Reference<css::uno::XComponentContext>& xContext
        = ::comphelper::getProcessComponentContext();
    css::uno::Reference<css::frame::XDesktop2> xDesktop = css::frame::Desktop::create(xContext);

    css::uno::Reference<css::frame::XFrame> xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        xFrame = xDesktop;

    mxFrame = std::move(xFrame);

    maCommandURL.Complete = aCommand;
    css::uno::Reference<css::util::XURLTransformer> xParser
        = css::util::URLTransformer::create(xContext);
    xParser->parseStrict(maCommandURL);
}

void WidgetStatusListener::startListening()
{
    if (mxDispatch.is())
        mxDispatch->removeStatusListener(this, maCommandURL);

    css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(mxFrame,
                                                                         css::uno::UNO_QUERY);
    if (!xDispatchProvider.is())
        return;

    mxDispatch = xDispatchProvider->queryDispatch(maCommandURL, u""_ustr, 0);
    if (mxDispatch.is())
        mxDispatch->addStatusListener(this, maCommandURL);
}

void WidgetStatusListener::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    mWidget->set_sensitive(rEvent.IsEnabled);
}

void WidgetStatusListener::disposing(const css::lang::EventObject& /*Source*/)
{
    mxDispatch.clear();
}

void WidgetStatusListener::dispose()
{
    if (mxDispatch.is())
    {
        mxDispatch->removeStatusListener(this, maCommandURL);
        mxDispatch.clear();
    }
    mxFrame.clear();
    mWidget = nullptr;
}

ButtonPressRepeater::ButtonPressRepeater(weld::Button& rButton, const Link<Button&, void>& rLink,
                                         const Link<const CommandEvent&, void>& rContextLink)
    : m_rButton(rButton)
    , m_aRepeat("vcl ButtonPressRepeater m_aRepeat")
    , m_aLink(rLink)
    , m_aContextLink(rContextLink)
    , m_bModKey(false)
{
    // instead of connect_clicked because we want a button held down to
    // repeat the next/prev
    m_rButton.connect_mouse_press(LINK(this, ButtonPressRepeater, MousePressHdl));
    m_rButton.connect_mouse_release(LINK(this, ButtonPressRepeater, MouseReleaseHdl));

    m_aRepeat.SetInvokeHandler(LINK(this, ButtonPressRepeater, RepeatTimerHdl));
}

IMPL_LINK(ButtonPressRepeater, MousePressHdl, const MouseEvent&, rMouseEvent, bool)
{
    if (rMouseEvent.IsRight())
    {
        m_aContextLink.Call(
            CommandEvent(rMouseEvent.GetPosPixel(), CommandEventId::ContextMenu, true));
        return false;
    }
    m_bModKey = rMouseEvent.IsMod1();
    if (!m_rButton.get_sensitive())
        return false;
    auto self = weak_from_this();
    RepeatTimerHdl(nullptr);
    if (!self.lock())
        return false;
    if (!m_rButton.get_sensitive())
        return false;
    m_aRepeat.SetTimeout(MouseSettings::GetButtonStartRepeat());
    m_aRepeat.Start();
    return true;
}

IMPL_LINK_NOARG(ButtonPressRepeater, MouseReleaseHdl, const MouseEvent&, bool)
{
    m_bModKey = false;
    m_aRepeat.Stop();
    return true;
}

IMPL_LINK_NOARG(ButtonPressRepeater, RepeatTimerHdl, Timer*, void)
{
    m_aRepeat.SetTimeout(Application::GetSettings().GetMouseSettings().GetButtonRepeat());
    m_aLink.Call(m_rButton);
}

weld::Window* GetPopupParent(vcl::Window& rOutWin, tools::Rectangle& rRect)
{
    rRect.SetPos(rOutWin.OutputToScreenPixel(rRect.TopLeft()));
    AbsoluteScreenPixelRectangle aRectAbs = FloatingWindow::ImplConvertToAbsPos(&rOutWin, rRect);

    vcl::Window* pWin = rOutWin.GetFrameWindow();
    // resolve from a possible BorderWindow to the ClientWindow (returns itself if not)
    pWin = pWin->ImplGetWindow();

    rRect = FloatingWindow::ImplConvertToRelPos(pWin, aRectAbs);
    rRect.SetPos(pWin->ScreenToOutputPixel(rRect.TopLeft()));

    return rOutWin.GetFrameWeld();
}

void SetPointFont(OutputDevice& rDevice, const vcl::Font& rFont, bool bUseDeviceDPI)
{
    auto pDefaultDevice = Application::GetDefaultDevice();
    if (pDefaultDevice)
        if (vcl::Window* pDefaultWindow = pDefaultDevice->GetOwnerWindow())
            pDefaultWindow->SetPointFont(rDevice, rFont, bUseDeviceDPI);
}

ReorderingDropTarget::ReorderingDropTarget(weld::TreeView& rTreeView)
    : DropTargetHelper(rTreeView.get_drop_target())
    , m_rTreeView(rTreeView)
{
}

sal_Int8 ReorderingDropTarget::AcceptDrop(const AcceptDropEvent& rEvt)
{
    // to enable the autoscroll when we're close to the edges
    m_rTreeView.get_dest_row_at_pos(rEvt.maPosPixel, true);
    return DND_ACTION_MOVE;
}

sal_Int8 ReorderingDropTarget::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    weld::TreeView* pSource = m_rTreeView.get_drag_source();
    // only dragging within the same widget allowed
    if (!pSource || pSource != &m_rTreeView)
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xSource = m_rTreeView.get_selected();
    if (!xSource)
        return DND_ACTION_NONE;

    int nTargetPos = -1;
    if (std::unique_ptr<weld::TreeIter> xTarget
        = m_rTreeView.get_dest_row_at_pos(rEvt.maPosPixel, true))
        nTargetPos = m_rTreeView.get_iter_index_in_parent(*xTarget);
    m_rTreeView.move_subtree(*xSource, nullptr, nTargetPos);

    return DND_ACTION_NONE;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
