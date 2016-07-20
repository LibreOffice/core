/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/layout.hxx>
#include <vcl/notebookbar.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/implbase.hxx>

/**
 * split from the main class since it needs different ref-counting mana
 */
class NotebookBarContextChangeEventListener : public ::cppu::WeakImplHelper<css::ui::XContextChangeEventListener>
{
    VclPtr<NotebookBar> mpParent;
public:
    NotebookBarContextChangeEventListener(NotebookBar *p) : mpParent(p) {}
    virtual ~NotebookBarContextChangeEventListener() {}

    // XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL disposing(const ::css::lang::EventObject&)
        throw (::css::uno::RuntimeException, ::std::exception) override;
};



NotebookBar::NotebookBar(Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : Control(pParent), m_pEventListener(new NotebookBarContextChangeEventListener(this))
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID, rFrame);
    get(m_pTabControl, "notebook1");
}

NotebookBar::~NotebookBar()
{
    disposeOnce();
}

void NotebookBar::dispose()
{
    disposeBuilder();
    m_pEventListener.clear();
    m_pTabControl.clear();
    Control::dispose();
}

Size NotebookBar::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));

    return Control::GetOptimalSize();
}

void NotebookBar::setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags)
{
    bool bCanHandleSmallerWidth = false;
    bool bCanHandleSmallerHeight = false;

    bool bIsLayoutEnabled = isLayoutEnabled(this);
    Window *pChild = GetWindow(GetWindowType::FirstChild);

    if (bIsLayoutEnabled && pChild->GetType() == WINDOW_SCROLLWINDOW)
    {
        WinBits nStyle = pChild->GetStyle();
        if (nStyle & (WB_AUTOHSCROLL | WB_HSCROLL))
            bCanHandleSmallerWidth = true;
        if (nStyle & (WB_AUTOVSCROLL | WB_VSCROLL))
            bCanHandleSmallerHeight = true;
    }

    Size aSize(GetOptimalSize());
    if (!bCanHandleSmallerWidth)
        nWidth = std::max(nWidth, aSize.Width());
    if (!bCanHandleSmallerHeight)
        nHeight = std::max(nHeight, aSize.Height());

    Control::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);

    if (bIsLayoutEnabled && (nFlags & PosSizeFlags::Size))
        VclContainer::setLayoutAllocation(*pChild, Point(0, 0), Size(nWidth, nHeight));
}

void NotebookBar::SetIconClickHdl(Link<NotebookBar*, void> aHdl)
{
    m_pTabControl->SetIconClickHdl(aHdl);
}

void NotebookBar::StateChanged(StateChangedType nType)
{
    if (nType == StateChangedType::Visible)
    {
        // visibility changed, update the container
        GetParent()->Resize();
    }

    Control::StateChanged(nType);
}

void SAL_CALL NotebookBarContextChangeEventListener::notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception)
{
    if (mpParent && mpParent->m_pTabControl)
        mpParent->m_pTabControl->SetContext(vcl::EnumContext::GetContextEnum(rEvent.ContextName));
}


void SAL_CALL NotebookBarContextChangeEventListener::disposing(const ::css::lang::EventObject&)
    throw (::css::uno::RuntimeException, ::std::exception)
{
    mpParent.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
