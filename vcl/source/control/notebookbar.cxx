/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/layout.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/notebookbar.hxx>
#include <vcl/taskpanelist.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/vclevent.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
/**
 * split from the main class since it needs different ref-counting mana
 */
class NotebookBarContextChangeEventListener : public ::cppu::WeakImplHelper<css::ui::XContextChangeEventListener>
{
    VclPtr<NotebookBar> mpParent;
public:
    explicit NotebookBarContextChangeEventListener(NotebookBar *p) : mpParent(p) {}

    // XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent) override;

    virtual void SAL_CALL disposing(const ::css::lang::EventObject&) override;
};



NotebookBar::NotebookBar(Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : Control(pParent), m_pEventListener(new NotebookBarContextChangeEventListener(this))
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);
    m_pUIBuilder.reset( new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID, rFrame) );
    mxFrame = rFrame;
    // In the Notebookbar's .ui file must exist control handling context
    // - implementing NotebookbarContextControl interface with id "ContextContainer"
    // or "ContextContainerX" where X is a number >= 1
    NotebookbarContextControl* pContextContainer = nullptr;
    int i = 0;
    do
    {
        OUString aName = "ContextContainer";
        if (i)
            aName += OUString::number(i);

        pContextContainer = dynamic_cast<NotebookbarContextControl*>(m_pUIBuilder->get<Window>(OUStringToOString(aName, RTL_TEXTENCODING_UTF8)));
        if (pContextContainer)
            m_pContextContainers.push_back(pContextContainer);
        i++;
    }
    while( pContextContainer != nullptr );

    UpdateBackground();
}

NotebookBar::~NotebookBar()
{
    disposeOnce();
}

void NotebookBar::dispose()
{
    m_pContextContainers.clear();
    if (m_pSystemWindow && m_pSystemWindow->ImplIsInTaskPaneList(this))
        m_pSystemWindow->GetTaskPaneList()->RemoveWindow(this);
    m_pSystemWindow.clear();
    disposeBuilder();
    m_pEventListener.clear();
    Control::dispose();
}

bool NotebookBar::PreNotify(NotifyEvent& rNEvt)
{
    // capture KeyEvents for taskpane cycling
    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        if (m_pSystemWindow)
            return m_pSystemWindow->PreNotify(rNEvt);
    }
    return Window::PreNotify( rNEvt );
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

    if (bIsLayoutEnabled && pChild->GetType() == WindowType::SCROLLWINDOW)
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

void NotebookBar::Resize()
{
    if(m_pUIBuilder && m_pUIBuilder->get_widget_root())
    {
        vcl::Window* pWindow = m_pUIBuilder->get_widget_root()->GetChild(0);
        if (pWindow)
        {
            Size aSize = pWindow->GetSizePixel();
            aSize.setWidth( GetSizePixel().Width() );
            pWindow->SetSizePixel(aSize);
        }
    }
    Control::Resize();
}

void NotebookBar::SetSystemWindow(SystemWindow* pSystemWindow)
{
    m_pSystemWindow = pSystemWindow;
    if (!m_pSystemWindow->ImplIsInTaskPaneList(this))
        m_pSystemWindow->GetTaskPaneList()->AddWindow(this);
}

void SAL_CALL NotebookBarContextChangeEventListener::notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent)
{
    if (mpParent)
    {
        for (NotebookbarContextControl* pControl : mpParent->m_pContextContainers)
            pControl->SetContext(vcl::EnumContext::GetContextEnum(rEvent.ContextName));
    }
}

void NotebookBar::ControlListener(bool bListen)
{
    if(bListen)
    {
        // remove listeners
        css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (css::ui::ContextChangeEventMultiplexer::get(
                ::comphelper::getProcessComponentContext()));
        xMultiplexer->removeContextChangeEventListener(getContextChangeEventListener(),mxFrame->getController());
    }
    else
    {
        // add listeners
        css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (css::ui::ContextChangeEventMultiplexer::get(
                ::comphelper::getProcessComponentContext()));
        xMultiplexer->addContextChangeEventListener(getContextChangeEventListener(),mxFrame->getController());
    }
}

void SAL_CALL NotebookBarContextChangeEventListener::disposing(const ::css::lang::EventObject&)
{
    mpParent.clear();
}

void NotebookBar::DataChanged(const DataChangedEvent& rDCEvt)
{
    UpdateBackground();
    Control::DataChanged(rDCEvt);
}

void NotebookBar::StateChanged(const  StateChangedType nStateChange )
{
    UpdateBackground();
    Control::StateChanged(nStateChange);
    Invalidate();
}

void NotebookBar::UpdateBackground()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const BitmapEx& aPersona = rStyleSettings.GetPersonaHeader();
    Wallpaper aWallpaper(aPersona);
    aWallpaper.SetStyle(WallpaperStyle::TopRight);
    if (!aPersona.IsEmpty())
        {
            SetBackground(aWallpaper);
            UpdatePersonaSettings();
            SetSettings( PersonaSettings );
        }
    else
        {
            SetBackground(rStyleSettings.GetDialogColor());
            UpdateDefaultSettings();
            SetSettings( DefaultSettings );
        }

    Invalidate(tools::Rectangle(Point(0,0), GetSizePixel()));
}

void NotebookBar::UpdateDefaultSettings()
{
    AllSettings aAllSettings( GetSettings() );
    StyleSettings aStyleSet( aAllSettings.GetStyleSettings() );

    ::Color aTextColor = aStyleSet.GetFieldTextColor();
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetTabTextColor(aTextColor);
    aStyleSet.SetToolTextColor(aTextColor);

    aAllSettings.SetStyleSettings(aStyleSet);
    DefaultSettings = aAllSettings;
}

void NotebookBar::UpdatePersonaSettings()
{
    AllSettings aAllSettings( GetSettings() );
    StyleSettings aStyleSet( aAllSettings.GetStyleSettings() );

    ::Color aTextColor = aStyleSet.GetPersonaMenuBarTextColor().get_value_or(COL_BLACK );
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetTabTextColor(aTextColor);
    aStyleSet.SetToolTextColor(aTextColor);

    aAllSettings.SetStyleSettings(aStyleSet);
    PersonaSettings = aAllSettings;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
