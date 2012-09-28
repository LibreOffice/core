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

#include "VertSplitView.hxx"

#include <tools/debug.hxx>
#include <vcl/split.hxx>

#define SPLITTER_WIDTH  80

using namespace ::dbaui;

//==================================================================
// class OSplitterView
//==================================================================
DBG_NAME(OSplitterView)
OSplitterView::OSplitterView(Window* _pParent,sal_Bool _bVertical) : Window(_pParent,WB_DIALOGCONTROL) // ,WB_BORDER
    ,m_pSplitter( NULL )
    ,m_pLeft(NULL)
    ,m_pRight(NULL)
    ,m_bVertical(_bVertical)
{
    DBG_CTOR(OSplitterView,NULL);
    ImplInitSettings( sal_True, sal_True, sal_True );
}
// -----------------------------------------------------------------------------
OSplitterView::~OSplitterView()
{
    DBG_DTOR(OSplitterView,NULL);
    m_pRight = m_pLeft = NULL;
}
//------------------------------------------------------------------------------
IMPL_LINK( OSplitterView, SplitHdl, Splitter*, /*pSplit*/ )
{
    OSL_ENSURE(m_pSplitter, "Splitter is NULL!");
    if ( m_bVertical )
    {
        long nPosY = m_pSplitter->GetPosPixel().Y();
        m_pSplitter->SetPosPixel( Point( m_pSplitter->GetSplitPosPixel(), nPosY ) );
    }
    else
        m_pSplitter->SetPosPixel( Point( m_pSplitter->GetPosPixel().X(),m_pSplitter->GetSplitPosPixel() ) );

    Resize();
    return 0L;
}
// -----------------------------------------------------------------------------
void OSplitterView::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetPointFont( aFont );
//      Set/*Zoomed*/PointFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetButtonTextColor();
        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetFaceColor() );
    }
}
// -----------------------------------------------------------------------
void OSplitterView::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OSplitterView::GetFocus()
{
    Window::GetFocus();

    // forward the focus to the current cell of the editor control
    if ( m_pLeft )
        m_pLeft->GrabFocus();
    else if ( m_pRight )
        m_pRight->GrabFocus();
}

// -------------------------------------------------------------------------
void OSplitterView::Resize()
{
    Window::Resize();
    OSL_ENSURE( m_pRight, "No init called!");

    Point   aSplitPos;
    Size    aSplitSize;
    Point   aPlaygroundPos( 0,0 );
    Size    aPlaygroundSize( GetOutputSizePixel() );

    if ( m_pLeft && m_pLeft->IsVisible() && m_pSplitter )
    {
        aSplitPos   = m_pSplitter->GetPosPixel();
        aSplitSize  = m_pSplitter->GetOutputSizePixel();
        if ( m_bVertical )
        {
            // calculate the splitter pos and size
            aSplitPos.Y() = aPlaygroundPos.Y();
            aSplitSize.Height() = aPlaygroundSize.Height();

            if( ( aSplitPos.X() + aSplitSize.Width() ) > ( aPlaygroundSize.Width() ))
                aSplitPos.X() = aPlaygroundSize.Width() - aSplitSize.Width();

            if( aSplitPos.X() <= aPlaygroundPos.X() )
                aSplitPos.X() = aPlaygroundPos.X() + sal_Int32(aPlaygroundSize.Width() * 0.3);

            // the tree pos and size
            Point   aTreeViewPos( aPlaygroundPos );
            Size    aTreeViewSize( aSplitPos.X(), aPlaygroundSize.Height() );

            // set the size of treelistbox
            m_pLeft->SetPosSizePixel( aTreeViewPos, aTreeViewSize );

            //set the size of the splitter
            m_pSplitter->SetPosSizePixel( aSplitPos, Size( aSplitSize.Width(), aPlaygroundSize.Height() ) );
            m_pSplitter->SetDragRectPixel( Rectangle(aPlaygroundPos,aPlaygroundSize) );
        }
        else
        {
            aSplitPos.X() = aPlaygroundPos.X();
            aSplitSize.Width() = aPlaygroundSize.Width();

            if( ( aSplitPos.Y() + aSplitSize.Height() ) > ( aPlaygroundSize.Height() ))
                aSplitPos.Y() = aPlaygroundSize.Height() - aSplitSize.Height();

            if( aSplitPos.Y() <= aPlaygroundPos.Y() )
                aSplitPos.Y() = aPlaygroundPos.Y() + sal_Int32(aPlaygroundSize.Height() * 0.3);

            // the tree pos and size
            Point   aTreeViewPos( aPlaygroundPos );
            Size    aTreeViewSize( aPlaygroundSize.Width() ,aSplitPos.Y());

            // set the size of treelistbox
            m_pLeft->SetPosSizePixel( aTreeViewPos, aTreeViewSize );

            //set the size of the splitter
            m_pSplitter->SetPosSizePixel( aSplitPos, Size( aPlaygroundSize.Width(), aSplitSize.Height() ) );
            m_pSplitter->SetDragRectPixel( Rectangle(aPlaygroundPos,aPlaygroundSize) );
        }
    }

    if ( m_pRight )
    {
        if ( m_bVertical )
            m_pRight->setPosSizePixel( aSplitPos.X() + aSplitSize.Width(), aPlaygroundPos.Y(),
                                   aPlaygroundSize.Width() - aSplitSize.Width() - aSplitPos.X(), aPlaygroundSize.Height());
        else
            m_pRight->setPosSizePixel( aSplitPos.X(), aPlaygroundPos.Y() + aSplitPos.Y() + aSplitSize.Height(),
                                   aPlaygroundSize.Width() , aPlaygroundSize.Height() - aSplitSize.Height() - aSplitPos.Y());
    }

}
// -----------------------------------------------------------------------------
void OSplitterView::set(Window* _pRight,Window* _pLeft)
{
    m_pLeft = _pLeft;
    m_pRight = _pRight;
}
// -----------------------------------------------------------------------------
void OSplitterView::setSplitter(Splitter* _pSplitter)
{
    m_pSplitter = _pSplitter;
    if ( m_pSplitter )
    {
        m_pSplitter->SetSplitPosPixel( LogicToPixel( Size( SPLITTER_WIDTH, 0 ), MAP_APPFONT ).Width() );
        m_pSplitter->SetSplitHdl( LINK(this, OSplitterView, SplitHdl) );
        m_pSplitter->Show();
        LINK( this, OSplitterView, SplitHdl ).Call(m_pSplitter);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
