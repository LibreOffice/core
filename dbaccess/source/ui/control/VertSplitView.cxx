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

#include <VertSplitView.hxx>

#include <vcl/split.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#include <osl/diagnose.h>

#define SPLITTER_WIDTH  80

using namespace ::dbaui;

// class OSplitterView
OSplitterView::OSplitterView(vcl::Window* _pParent) : Window(_pParent,WB_DIALOGCONTROL) // ,WB_BORDER
    ,m_pSplitter( nullptr )
    ,m_pLeft(nullptr)
    ,m_pRight(nullptr)
    ,m_pResizeId(nullptr)
{
    ImplInitSettings();
}

OSplitterView::~OSplitterView()
{
    disposeOnce();
}

void OSplitterView::dispose()
{
    if (m_pResizeId)
    {
        RemoveUserEvent(m_pResizeId);
        m_pResizeId = nullptr;
    }
    m_pSplitter.clear();
    m_pLeft.clear();
    m_pRight.clear();
    vcl::Window::dispose();
}

IMPL_LINK_NOARG( OSplitterView, SplitHdl, Splitter*, void )
{
    OSL_ENSURE(m_pSplitter, "Splitter is NULL!");
    m_pSplitter->SetPosPixel( Point( m_pSplitter->GetPosPixel().X(),m_pSplitter->GetSplitPosPixel() ) );

    Resize();
}

void OSplitterView::ImplInitSettings()
{
    // FIXME RenderContext
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    vcl::Font aFont = rStyleSettings.GetAppFont();
    if ( IsControlFont() )
        aFont.Merge( GetControlFont() );
    SetPointFont(*this, aFont);
//  Set/*Zoomed*/PointFont( aFont );

    Color aTextColor = rStyleSettings.GetButtonTextColor();
    if ( IsControlForeground() )
         aTextColor = GetControlForeground();
    SetTextColor( aTextColor );

    if( IsControlBackground() )
        SetBackground( GetControlBackground() );
    else
        SetBackground( rStyleSettings.GetFaceColor() );
}

void OSplitterView::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OSplitterView::GetFocus()
{
    Window::GetFocus();

    // forward the focus to the current cell of the editor control
    if ( m_pLeft )
        m_pLeft->GrabFocus();
    else if ( m_pRight )
        m_pRight->GrabFocus();
}

IMPL_LINK_NOARG(OSplitterView, ResizeHdl, void*, void)
{
    m_pResizeId = nullptr;

    OSL_ENSURE( m_pRight, "No init called!");

    Point   aSplitPos;
    Size    aSplitSize;
    Point   aPlaygroundPos( 0,0 );
    Size    aPlaygroundSize( GetOutputSizePixel() );

    if ( m_pLeft && m_pLeft->IsVisible() && m_pSplitter )
    {
        aSplitPos   = m_pSplitter->GetPosPixel();
        aSplitSize  = m_pSplitter->GetOutputSizePixel();
        aSplitPos.setX( aPlaygroundPos.X() );
        aSplitSize.setWidth( aPlaygroundSize.Width() );

        if( ( aSplitPos.Y() + aSplitSize.Height() ) > ( aPlaygroundSize.Height() ))
            aSplitPos.setY( aPlaygroundSize.Height() - aSplitSize.Height() );

        if( aSplitPos.Y() <= aPlaygroundPos.Y() )
            aSplitPos.setY( aPlaygroundPos.Y() + sal_Int32(aPlaygroundSize.Height() * 0.3) );

        // the tree pos and size
        Point   aTreeViewPos( aPlaygroundPos );
        Size    aTreeViewSize( aPlaygroundSize.Width() ,aSplitPos.Y());

        // set the size of treelistbox
        m_pLeft->SetPosSizePixel( aTreeViewPos, aTreeViewSize );

        //set the size of the splitter
        m_pSplitter->SetPosSizePixel( aSplitPos, Size( aPlaygroundSize.Width(), aSplitSize.Height() ) );
        m_pSplitter->SetDragRectPixel( tools::Rectangle(aPlaygroundPos,aPlaygroundSize) );
    }

    if ( m_pRight )
    {
        m_pRight->setPosSizePixel( aSplitPos.X(), aPlaygroundPos.Y() + aSplitPos.Y() + aSplitSize.Height(),
                               aPlaygroundSize.Width() , aPlaygroundSize.Height() - aSplitSize.Height() - aSplitPos.Y());
    }
}

void OSplitterView::Resize()
{
    Window::Resize();
    if (m_pResizeId)
        RemoveUserEvent(m_pResizeId);
    m_pResizeId = PostUserEvent(LINK(this, OSplitterView, ResizeHdl), this, true);
}

void OSplitterView::set(vcl::Window* _pRight,Window* _pLeft)
{
    m_pLeft = _pLeft;
    m_pRight = _pRight;
}

void OSplitterView::setSplitter(Splitter* _pSplitter)
{
    m_pSplitter = _pSplitter;
    if ( m_pSplitter )
    {
        m_pSplitter->SetSplitPosPixel(LogicToPixel(Size(SPLITTER_WIDTH, 0), MapMode(MapUnit::MapAppFont)).Width());
        m_pSplitter->SetSplitHdl( LINK(this, OSplitterView, SplitHdl) );
        m_pSplitter->Show();
        LINK( this, OSplitterView, SplitHdl ).Call(m_pSplitter);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
