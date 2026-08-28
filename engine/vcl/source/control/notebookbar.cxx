/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <utility>

#include <vcl/layout.hxx>
#include <vcl/notebookbar/notebookbar.hxx>

NotebookBar::NotebookBar(Window* pParent, OUString aUIXMLDescription)
    : Control(pParent)
    , m_pViewShell(nullptr)
    , m_sUIXMLDescription(std::move(aUIXMLDescription))
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    m_xVclContentArea = VclPtr<VclVBox>::Create(this);
    m_xVclContentArea->Show();
    // now access it using GetMainContainer and set dispose callback with SetDisposeCallback

    UpdateBackground();
}

void NotebookBar::SetDisposeCallback(const Link<const SfxViewShell*, void> rDisposeCallback, const SfxViewShell* pViewShell)
{
    m_rDisposeLink = rDisposeCallback;
    m_pViewShell = pViewShell;
}

NotebookBar::~NotebookBar()
{
    disposeOnce();
}

void NotebookBar::dispose()
{
    if (m_rDisposeLink.IsSet())
        m_rDisposeLink.Call(m_pViewShell);

    m_xVclContentArea.disposeAndClear();

    Control::dispose();
}

Size NotebookBar::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));

    return Control::GetOptimalSize();
}

void NotebookBar::Resize()
{
    vcl::Window* pChild = GetWindow(GetWindowType::FirstChild);
    assert(pChild);
    VclContainer::setLayoutAllocation(*pChild, Point(0, 0), GetSizePixel());
    Control::Resize();
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
    SetBackground(rStyleSettings.GetDialogColor());
    UpdateDefaultSettings();
    GetOutDev()->SetSettings( DefaultSettings );

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
    DefaultSettings = std::move(aAllSettings);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
