/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#pragma once

#include <vcl/window.hxx>

namespace chart
{

class ChartController;

/** The ChartWindow collects events from the window and forwards them the to the controller
thus the controller can perform appropriate actions
*/

class ChartWindow : public vcl::Window
{
public:
    ChartWindow( ChartController* pController, vcl::Window* pParent, WinBits nStyle );
    virtual ~ChartWindow() override;
    virtual void dispose() override;

    //from base class Window:
    virtual void PrePaint(vcl::RenderContext& rRenderContext) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void Tracking( const TrackingEvent& rTEvt ) override;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void Resize() override;
    virtual void Activate() override;
    virtual void Deactivate() override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual void Command( const CommandEvent& rCEvt ) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void RequestHelp( const HelpEvent& rHEvt ) override;

    /// For LibreOfficeKit, we need to route these to the mouse events.
    virtual void LogicMouseButtonDown(const MouseEvent&) override;
    virtual void LogicMouseButtonUp(const MouseEvent&) override;
    virtual void LogicMouseMove(const MouseEvent&) override;

    void ForceInvalidate();
    virtual void Invalidate( InvalidateFlags nFlags = InvalidateFlags::NONE ) override;
    virtual void Invalidate( const tools::Rectangle& rRect, InvalidateFlags nFlags = InvalidateFlags::NONE ) override;
    virtual void Invalidate( const vcl::Region& rRegion, InvalidateFlags nFlags = InvalidateFlags::NONE ) override;
    /// Notify the LOK client about an invalidated area.
    virtual void LogicInvalidate( const tools::Rectangle* pRectangle ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    virtual FactoryFunction GetUITestFactory() const override;

    ChartController* GetController();

    virtual bool IsChart() const override { return true; }
    vcl::Window* GetParentEditWin();

private:
    // returns the chart bounding box in twips
    tools::Rectangle GetBoundingBox();

private:
    ChartController* m_pWindowController;
    bool m_bInPaint;
    VclPtr<vcl::Window> m_pViewShellWindow;

    void adjustHighContrastMode();
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
