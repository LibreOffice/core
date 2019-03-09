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

#include <vector>

#include <svtools/controldims.hxx>

#include "RemoteDialogClientBox.hxx"
#include <RemoteServer.hxx>

#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

#include <sdresid.hxx>
#include <strings.hrc>

using namespace std;

using namespace ::com::sun::star;

namespace sd {

//                          struct ClientBoxEntry

ClientBoxEntry::ClientBoxEntry(const std::shared_ptr<ClientInfo>& pClientInfo)
    : m_bActive(false)
    , m_pClientInfo(pClientInfo)
{
}

ClientBoxEntry::~ClientBoxEntry()
{}

// ClientBox

ClientBox::ClientBox( vcl::Window* pParent, WinBits nStyle ) :
    Control( pParent, nStyle ),
    m_bHasScrollBar( false ),
    m_bHasActive( false ),
    m_bNeedsRecalc( true ),
    m_bAdjustActive( false ),
    m_nActive( 0 ),
    m_nTopIndex( 0 ),
    m_nActiveHeight( 0 ),
    m_aPinBox( VclPtr<NumericBox>::Create( this, 0 ) ),
    m_aDeauthoriseButton( VclPtr<PushButton>::Create( this ) ),
    m_aScrollBar( VclPtr<ScrollBar>::Create( this, WB_VERT ) )
{
    m_aScrollBar->SetScrollHdl( LINK( this, ClientBox, ScrollHdl ) );
    m_aScrollBar->EnableDrag();

    m_aPinBox->SetUseThousandSep(false);
    m_aDeauthoriseButton->SetText( SdResId(STR_DEAUTHORISE_CLIENT) );
    m_aDeauthoriseButton->SetClickHdl( LINK( this, ClientBox, DeauthoriseHdl ) );

    SetPosPixel( Point( RSC_SP_DLG_INNERBORDER_LEFT, RSC_SP_DLG_INNERBORDER_TOP ) );
    long nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        m_nStdHeight = nTitleHeight;
    else
        m_nStdHeight = nIconHeight;
    m_nStdHeight += GetTextHeight() + TOP_OFFSET;

    m_nActiveHeight = m_nStdHeight;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( IsControlBackground() )
        SetBackground( GetControlBackground() );
    else
        SetBackground( rStyleSettings.GetFieldColor() );

    populateEntries();

    Show();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(ClientBox, WB_TABSTOP)

Size ClientBox::GetOptimalSize() const
{
    return LogicToPixel(Size(200, 140), MapMode(MapUnit::MapAppFont));
}

ClientBox::~ClientBox()
{
    disposeOnce();
}

void ClientBox::dispose()
{
    m_vEntries.clear();

    m_aPinBox.disposeAndClear();
    m_aDeauthoriseButton.disposeAndClear();
    m_aScrollBar.disposeAndClear();
    Control::dispose();
}

// Title + description
void ClientBox::CalcActiveHeight()
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    // get title height
    long aTextHeight;
    long nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        aTextHeight = nTitleHeight;
    else
        aTextHeight = nIconHeight;

    // Text entry height
    Size aSize = GetOutputSizePixel();
    if ( m_bHasScrollBar )
        aSize.AdjustWidth( -(m_aScrollBar->GetSizePixel().Width()) );

    aSize.AdjustWidth( -ICON_OFFSET );

    aSize = LogicToPixel( Size( RSC_CD_PUSHBUTTON_WIDTH, RSC_CD_PUSHBUTTON_HEIGHT ),
                               MapMode( MapUnit::MapAppFont ) );
    aTextHeight += aSize.Height();

    if ( aTextHeight < m_nStdHeight )
        aTextHeight = m_nStdHeight;

    m_nActiveHeight = aTextHeight + 2;
}

::tools::Rectangle ClientBox::GetEntryRect( const long nPos ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    Size aSize( GetOutputSizePixel() );

    if ( m_bHasScrollBar )
        aSize.AdjustWidth( -(m_aScrollBar->GetSizePixel().Width()) );

    if ( m_vEntries[ nPos ]->m_bActive )
        aSize.setHeight( m_nActiveHeight );
    else
        aSize.setHeight( m_nStdHeight );

    Point aPos( 0, -m_nTopIndex + nPos * m_nStdHeight );
    if ( m_bHasActive && ( nPos < m_nActive ) )
        aPos.AdjustY(m_nActiveHeight - m_nStdHeight );

    return ::tools::Rectangle( aPos, aSize );
}

long ClientBox::GetActiveEntryIndex()
{
    if ( m_bHasActive )
        return m_nActive;
    else
        return -1;
}

//This function may be called with nPos < 0
void ClientBox::selectEntry( const long nPos )
{
    //ToDo we should not use the guard at such a big scope here.
    //Currently it is used to guard m_vEntries and m_nActive. m_nActive will be
    //modified in this function.
    //It would be probably best to always use a copy of m_vEntries
    //and some other state variables from ClientBox for
    //the whole painting operation. See issue i86993
    ::osl::ClearableMutexGuard guard(m_entriesMutex);

    if ( m_bHasActive )
    {
        if ( nPos == m_nActive )
            return;

        m_bHasActive = false;
        m_vEntries[ m_nActive ]->m_bActive = false;
    }

    if ( ( nPos >= 0 ) && ( nPos < static_cast<long>(m_vEntries.size()) ) )
    {
        m_bHasActive = true;
        m_nActive = nPos;
        m_vEntries[ nPos ]->m_bActive = true;

        if ( IsReallyVisible() )
        {
            m_bAdjustActive = true;
        }
    }

    // We empty the pin box now too, just in case the user previously
    // entered a pin, but then changed their selected device.
    m_aPinBox->SetText( "" );
    if ( m_bHasActive )
    {
        bool bAlreadyAuthorised =
            m_vEntries[ m_nActive ]->m_pClientInfo->mbIsAlreadyAuthorised;

        if ( bAlreadyAuthorised )
        {
            m_aDeauthoriseButton->GetFocus();
        }
        else
        {
            m_aPinBox->GetFocus();
        }
    }

    if ( IsReallyVisible() )
    {
        m_bNeedsRecalc = true;
        Invalidate();
    }

    guard.clear();
}

void ClientBox::DrawRow(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect, const TClientBoxEntry& rEntry)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (rEntry->m_bActive)
        SetTextColor(rStyleSettings.GetHighlightTextColor());
    else
        SetTextColor(rStyleSettings.GetFieldTextColor());

    if (rEntry->m_bActive)
    {
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetHighlightColor());
        rRenderContext.DrawRect(rRect);
    }
    else
    {
        if(IsControlBackground())
            SetBackground(GetControlBackground());
        else
            SetBackground(rStyleSettings.GetFieldColor());

        rRenderContext.SetTextFillColor();
        rRenderContext.Erase(rRect);
    }

    // FIXME: draw bluetooth or wifi icon
    Point aPos(rRect.TopLeft());

    // Setup fonts
    vcl::Font aStdFont(rRenderContext.GetFont());
    vcl::Font aBoldFont(aStdFont);
    aBoldFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aBoldFont);
    long aTextHeight = rRenderContext.GetTextHeight();

    // Get max title width
    long nMaxTitleWidth = rRect.GetWidth() - ICON_OFFSET;
    nMaxTitleWidth -= ( 2 * SMALL_ICON_SIZE ) + ( 4 * SPACE_BETWEEN );

    long aTitleWidth = rRenderContext.GetTextWidth(rEntry->m_pClientInfo->mName) + (aTextHeight / 3);

    aPos = rRect.TopLeft() + Point(ICON_OFFSET, TOP_OFFSET);

    if (aTitleWidth > nMaxTitleWidth)
    {
        aTitleWidth = nMaxTitleWidth - (aTextHeight / 3);
        OUString aShortTitle = rRenderContext.GetEllipsisString(rEntry->m_pClientInfo->mName, aTitleWidth );
        rRenderContext.DrawText(aPos, aShortTitle);
    }
    else
        rRenderContext.DrawText(aPos, rEntry->m_pClientInfo->mName);

    SetFont(aStdFont);

    aPos.AdjustY(aTextHeight );
    if (rEntry->m_bActive)
    {
      OUString sPinText(SdResId(STR_ENTER_PIN));
      DrawText(m_sPinTextRect, sPinText);
    }

    rRenderContext.SetLineColor(COL_LIGHTGRAY);
    rRenderContext.DrawLine(rRect.BottomLeft(), rRect.BottomRight());
}

void ClientBox::RecalcAll()
{
    if ( m_bHasActive )
        CalcActiveHeight();

    SetupScrollBar();

    Size aPBSize = LogicToPixel(
                      Size( RSC_CD_PUSHBUTTON_WIDTH, RSC_CD_PUSHBUTTON_HEIGHT ),
                      MapMode( MapUnit::MapAppFont ) );
    m_aPinBox->SetSizePixel( aPBSize );
    m_aDeauthoriseButton->SetSizePixel( m_aDeauthoriseButton->GetOptimalSize() );

    if ( !m_bHasActive )
    {
        m_aPinBox->Show( false );
        m_aDeauthoriseButton->Show( false );
    }
    else
    {
        ::tools::Rectangle aEntryRect = GetEntryRect( m_nActive );

        Size  aPinBoxSize( m_aPinBox->GetSizePixel() );
        Point aPos( aEntryRect.Left(),
                    aEntryRect.Bottom() - TOP_OFFSET - aPinBoxSize.Height() );

        bool bAlreadyAuthorised = m_vEntries[ m_nActive ]->m_pClientInfo->mbIsAlreadyAuthorised;

        if ( !bAlreadyAuthorised )
        {
            m_sPinTextRect = ::tools::Rectangle( aPos.X(), aPos.Y(),
                                        aEntryRect.Right(),
                                        aEntryRect.Bottom() - TOP_OFFSET);

            OUString sPinText(SdResId(STR_ENTER_PIN));

            aPos = Point( aEntryRect.Left() + GetTextWidth( sPinText ),
                          aEntryRect.Bottom() - TOP_OFFSET - aPinBoxSize.Height() );
            m_aPinBox->SetPosPixel( aPos );
            // The text would have it's TOP aligned with the top of
            // the pin box -- hence we push it down to align baselines.
            m_sPinTextRect += Point( 0, 4 );
        }
        else
        {
            aPos += Point( 20, 0 );
            m_aDeauthoriseButton->SetPosPixel( aPos );
        }

        m_aPinBox->Show( !bAlreadyAuthorised );
        m_aDeauthoriseButton->Show( bAlreadyAuthorised );

        if ( m_bAdjustActive )
        {
            m_bAdjustActive = false;

            // If the top of the selected entry isn't visible, make it visible
            if ( aEntryRect.Top() < 0 )
            {
                m_nTopIndex += aEntryRect.Top();
                aEntryRect.Move( 0, -aEntryRect.Top() );
            }

            // If the bottom of the selected entry isn't visible, make it visible even if now the top
            // isn't visible any longer ( the buttons are more important )
            Size aOutputSize = GetOutputSizePixel();
            if ( aEntryRect.Bottom() > aOutputSize.Height() )
            {
                m_nTopIndex += ( aEntryRect.Bottom() - aOutputSize.Height() );
                aEntryRect.Move( 0, -( aEntryRect.Bottom() - aOutputSize.Height() ) );
            }

            // If there is unused space below the last entry but all entries don't fit into the box,
            // move the content down to use the whole space
            const long nTotalHeight = GetTotalHeight();
            if ( m_bHasScrollBar && ( aOutputSize.Height() + m_nTopIndex > nTotalHeight ) )
            {
                long nOffset = m_nTopIndex;
                m_nTopIndex = nTotalHeight - aOutputSize.Height();
                nOffset -= m_nTopIndex;
                aEntryRect.Move( 0, nOffset );
            }

            if ( m_bHasScrollBar )
                m_aScrollBar->SetThumbPos( m_nTopIndex );
        }
    }

    m_bNeedsRecalc = false;
}

bool ClientBox::HandleCursorKey( sal_uInt16 nKeyCode )
{
    if ( m_vEntries.empty() )
        return true;

    long nSelect = 0;

    if ( m_bHasActive )
    {
        long nPageSize = GetOutputSizePixel().Height() / m_nStdHeight;
        if ( nPageSize < 2 )
            nPageSize = 2;

        if ( ( nKeyCode == KEY_DOWN ) || ( nKeyCode == KEY_RIGHT ) )
            nSelect = m_nActive + 1;
        else if ( ( nKeyCode == KEY_UP ) || ( nKeyCode == KEY_LEFT ) )
            nSelect = m_nActive - 1;
        else if ( nKeyCode == KEY_HOME )
            nSelect = 0;
        else if ( nKeyCode == KEY_END )
            nSelect = m_vEntries.size() - 1;
        else if ( nKeyCode == KEY_PAGEUP )
            nSelect = m_nActive - nPageSize + 1;
        else if ( nKeyCode == KEY_PAGEDOWN )
            nSelect = m_nActive + nPageSize - 1;
    }
    else // when there is no selected entry, we will select the first or the last.
    {
        if ( ( nKeyCode == KEY_DOWN ) || ( nKeyCode == KEY_PAGEDOWN ) || ( nKeyCode == KEY_HOME ) )
            nSelect = 0;
        else if ( ( nKeyCode == KEY_UP ) || ( nKeyCode == KEY_PAGEUP ) || ( nKeyCode == KEY_END ) )
            nSelect = m_vEntries.size() - 1;
    }

    if ( nSelect < 0 )
        nSelect = 0;
    if ( nSelect >= static_cast<long>(m_vEntries.size()) )
        nSelect = m_vEntries.size() - 1;

    selectEntry( nSelect );

    return true;
}

void ClientBox::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle &/*rPaintRect*/)
{
    if (m_bNeedsRecalc)
        RecalcAll();

    Point aStart(0, -m_nTopIndex);
    Size aSize(GetOutputSizePixel());

    if (m_bHasScrollBar)
        aSize.AdjustWidth( -(m_aScrollBar->GetSizePixel().Width()) );

    const ::osl::MutexGuard aGuard(m_entriesMutex);

    for (auto& vEntry : m_vEntries)
    {
        aSize.setHeight( vEntry->m_bActive ? m_nActiveHeight : m_nStdHeight );
        ::tools::Rectangle aEntryRect(aStart, aSize);
        DrawRow(rRenderContext, aEntryRect, vEntry);
        aStart.AdjustY(aSize.Height() );
    }
}

long ClientBox::GetTotalHeight() const
{
    long nHeight = m_vEntries.size() * m_nStdHeight;

    if ( m_bHasActive )
    {
        nHeight += m_nActiveHeight - m_nStdHeight;
    }

    return nHeight;
}

void ClientBox::SetupScrollBar()
{
    const Size aSize = GetOutputSizePixel();
    const long nScrBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    const long nTotalHeight = GetTotalHeight();
    const bool bNeedsScrollBar = ( nTotalHeight > aSize.Height() );

    if ( bNeedsScrollBar )
    {
        if ( m_nTopIndex + aSize.Height() > nTotalHeight )
            m_nTopIndex = nTotalHeight - aSize.Height();

        m_aScrollBar->SetPosSizePixel( Point( aSize.Width() - nScrBarSize, 0 ),
                                       Size( nScrBarSize, aSize.Height() ) );
        m_aScrollBar->SetRangeMax( nTotalHeight );
        m_aScrollBar->SetVisibleSize( aSize.Height() );
        m_aScrollBar->SetPageSize( ( aSize.Height() * 4 ) / 5 );
        m_aScrollBar->SetLineSize( m_nStdHeight );
        m_aScrollBar->SetThumbPos( m_nTopIndex );

        if ( !m_bHasScrollBar )
            m_aScrollBar->Show();
    }
    else if ( m_bHasScrollBar )
    {
        m_aScrollBar->Hide();
        m_nTopIndex = 0;
    }

    m_bHasScrollBar = bNeedsScrollBar;
}

void ClientBox::Resize()
{
    RecalcAll();
}

long ClientBox::PointToPos( const Point& rPos )
{
    long nPos = ( rPos.Y() + m_nTopIndex ) / m_nStdHeight;

    if ( m_bHasActive && ( nPos > m_nActive ) )
    {
        if ( rPos.Y() + m_nTopIndex <= m_nActive*m_nStdHeight + m_nActiveHeight )
            nPos = m_nActive;
        else
            nPos = ( rPos.Y() + m_nTopIndex - (m_nActiveHeight - m_nStdHeight) ) / m_nStdHeight;
    }

    return nPos;
}

OUString ClientBox::getPin()
{
    return OUString::number( m_aPinBox->GetValue() );
}

void ClientBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    long nPos = PointToPos( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.IsMod1() && m_bHasActive )
            selectEntry( m_vEntries.size() );   // Selecting an not existing entry will deselect the current one
        else
            selectEntry( nPos );
    }
}

bool ClientBox::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        vcl::KeyCode    aKeyCode = pKEvt->GetKeyCode();
        sal_uInt16      nKeyCode = aKeyCode.GetCode();

        if ( aKeyCode.GetGroup() == KEYGROUP_CURSOR )
            bHandled = HandleCursorKey( nKeyCode );
    }

    if ( rNEvt.GetType() == MouseNotifyEvent::COMMAND )
    {
        if ( m_bHasScrollBar &&
             ( rNEvt.GetCommandEvent()->GetCommand() == CommandEventId::Wheel ) )
        {
            const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
            if ( pData->GetMode() == CommandWheelMode::SCROLL )
            {
                long nThumbPos = m_aScrollBar->GetThumbPos();
                if ( pData->GetDelta() < 0 )
                    m_aScrollBar->DoScroll( nThumbPos + m_nStdHeight );
                else
                    m_aScrollBar->DoScroll( nThumbPos - m_nStdHeight );
                bHandled = true;
            }
        }
    }

    if ( !bHandled )
        return Control::EventNotify(rNEvt);
    else
        return true;
}

void ClientBox::addEntry( const std::shared_ptr<ClientInfo>& pClientInfo )
{
    long         nPos = 0;

    TClientBoxEntry xEntry( new ClientBoxEntry( pClientInfo ) );

    ::osl::ClearableMutexGuard guard(m_entriesMutex);
    if ( m_vEntries.empty() )
    {
        m_vEntries.push_back( xEntry );
    }
    else
    {
        m_vEntries.insert( m_vEntries.begin()+nPos, xEntry );
    }

    //access to m_nActive must be guarded
    if ( m_bHasActive && ( m_nActive >= nPos ) )
        m_nActive += 1;

    guard.clear();

    if ( IsReallyVisible() )
        Invalidate();

    m_bNeedsRecalc = true;
}

void ClientBox::clearEntries()
{
    selectEntry( -1 );
    m_bHasActive = false;

    const ::osl::MutexGuard aGuard( m_entriesMutex );

    m_vEntries.clear();
    if ( IsReallyVisible() )
        Invalidate();
    m_bNeedsRecalc = true;
}

void ClientBox::populateEntries()
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    clearEntries();

#ifdef ENABLE_SDREMOTE
    RemoteServer::ensureDiscoverable();

    vector< std::shared_ptr< ClientInfo > > aClients( RemoteServer::getClients() );

    for ( const auto& rxClient : aClients )
    {
        addEntry( rxClient );
    }
#endif

    if ( IsReallyVisible() )
        Invalidate();
    m_bNeedsRecalc = true;
}

void ClientBox::DoScroll( long nDelta )
{
    m_nTopIndex += nDelta;
    Point aNewSBPt( m_aScrollBar->GetPosPixel() );

    ::tools::Rectangle aScrRect( Point(), GetOutputSizePixel() );
    aScrRect.AdjustRight( -(m_aScrollBar->GetSizePixel().Width()) );
    Scroll( 0, -nDelta, aScrRect );

    m_aScrollBar->SetPosPixel( aNewSBPt );
}

IMPL_LINK( ClientBox, ScrollHdl, ScrollBar*, pScrBar, void )
{
    DoScroll( pScrBar->GetDelta() );
}

IMPL_LINK_NOARG( ClientBox, DeauthoriseHdl, Button*, void )
{
    long aSelected = GetActiveEntryIndex();
    if ( aSelected < 0 )
        return;
    TClientBoxEntry aEntry = GetEntryData(aSelected);

#ifdef ENABLE_SDREMOTE
    RemoteServer::deauthoriseClient( aEntry->m_pClientInfo );
#endif
    populateEntries();
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
