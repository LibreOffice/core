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

#include <csvcontrol.hxx>
#include <vcl/settings.hxx>
#include <AccessibleCsvControl.hxx>

ScCsvLayoutData::ScCsvLayoutData() :
    mnPosCount( 1 ),
    mnPosOffset( 0 ),
    mnWinWidth( 1 ),
    mnHdrWidth( 0 ),
    mnCharWidth( 1 ),
    mnLineCount( 1 ),
    mnLineOffset( 0 ),
    mnWinHeight( 1 ),
    mnHdrHeight( 0 ),
    mnLineHeight( 1 ),
    mnPosCursor( CSV_POS_INVALID ),
    mnColCursor( 0 ),
    mnNoRepaint( 0 ),
    mbAppRTL( AllSettings::GetLayoutRTL() )
{
}

ScCsvDiff ScCsvLayoutData::GetDiff( const ScCsvLayoutData& rData ) const
{
    ScCsvDiff nRet = ScCsvDiff::Equal;
    if( mnPosCount != rData.mnPosCount )        nRet |= ScCsvDiff::PosCount;
    if( mnPosOffset != rData.mnPosOffset )      nRet |= ScCsvDiff::PosOffset;
    if( mnHdrWidth != rData.mnHdrWidth )        nRet |= ScCsvDiff::HeaderWidth;
    if( mnCharWidth != rData.mnCharWidth )      nRet |= ScCsvDiff::CharWidth;
    if( mnLineCount != rData.mnLineCount )      nRet |= ScCsvDiff::LineCount;
    if( mnLineOffset != rData.mnLineOffset )    nRet |= ScCsvDiff::LineOffset;
    if( mnHdrHeight != rData.mnHdrHeight )      nRet |= ScCsvDiff::HeaderHeight;
    if( mnLineHeight != rData.mnLineHeight )    nRet |= ScCsvDiff::LineHeight;
    if( mnPosCursor != rData.mnPosCursor )      nRet |= ScCsvDiff::RulerCursor;
    if( mnColCursor != rData.mnColCursor )      nRet |= ScCsvDiff::GridCursor;
    return nRet;
}

ScCsvControl::ScCsvControl( ScCsvControl& rParent ) :
    VclReferenceBase(),
    Control( &rParent, WB_TABSTOP | WB_NODIALOGCONTROL ),
    mrData( rParent.GetLayoutData() ),
    mbValidGfx( false )
{
}

ScCsvControl::ScCsvControl( vcl::Window* pParent, const ScCsvLayoutData& rData, WinBits nBits ) :
    Control( pParent, nBits ),
    mrData( rData ),
    mbValidGfx( false )
{
}

ScCsvControl::~ScCsvControl()
{
    disposeOnce();
}

void ScCsvControl::dispose()
{
    if( mxAccessible.is() )
        mxAccessible->dispose();
    mxAccessible = nullptr; // lp#1566050: prevent cyclic reference zombies
    Control::dispose();
}

// event handling -------------------------------------------------------------

void ScCsvControl::GetFocus()
{
    Control::GetFocus();
    AccSendFocusEvent( true );
}

void ScCsvControl::LoseFocus()
{
    Control::LoseFocus();
    AccSendFocusEvent( false );
}

void ScCsvControl::AccSendFocusEvent( bool bFocused )
{
    if( mxAccessible.is() )
        mxAccessible->SendFocusEvent( bFocused );
}

void ScCsvControl::AccSendCaretEvent()
{
    if( mxAccessible.is() )
        mxAccessible->SendCaretEvent();
}

void ScCsvControl::AccSendVisibleEvent()
{
    if( mxAccessible.is() )
        mxAccessible->SendVisibleEvent();
}

void ScCsvControl::AccSendSelectionEvent()
{
    if( mxAccessible.is() )
        mxAccessible->SendSelectionEvent();
}

void ScCsvControl::AccSendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows )
{
    if( mxAccessible.is() )
        mxAccessible->SendTableUpdateEvent( nFirstColumn, nLastColumn, bAllRows );
}

void ScCsvControl::AccSendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    if( mxAccessible.is() )
        mxAccessible->SendInsertColumnEvent( nFirstColumn, nLastColumn );
}

void ScCsvControl::AccSendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    if( mxAccessible.is() )
        mxAccessible->SendRemoveColumnEvent( nFirstColumn, nLastColumn );
}

// repaint helpers ------------------------------------------------------------

void ScCsvControl::Repaint( bool bInvalidate )
{
    if( bInvalidate )
        InvalidateGfx();
    if( !IsNoRepaint() )
        Execute( CSVCMD_REPAINT );
}

void ScCsvControl::DisableRepaint()
{
    ++mrData.mnNoRepaint;
}

void ScCsvControl::EnableRepaint()
{
    OSL_ENSURE( IsNoRepaint(), "ScCsvControl::EnableRepaint - invalid call" );
    --mrData.mnNoRepaint;
    Repaint();
}

// command handling -----------------------------------------------------------

void ScCsvControl::Execute( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 )
{
    maCmd.Set( eType, nParam1, nParam2 );
    maCmdHdl.Call( *this );
}

// layout helpers -------------------------------------------------------------

sal_Int32 ScCsvControl::GetVisPosCount() const
{
    return (mrData.mnWinWidth - GetHdrWidth()) / GetCharWidth();
}

sal_Int32 ScCsvControl::GetMaxPosOffset() const
{
    return std::max<sal_Int32>( GetPosCount() - GetVisPosCount() + 2, 0 );
}

bool ScCsvControl::IsValidSplitPos( sal_Int32 nPos ) const
{
    return (0 < nPos) && (nPos < GetPosCount() );
}

bool ScCsvControl::IsVisibleSplitPos( sal_Int32 nPos ) const
{
    return IsValidSplitPos( nPos ) && (GetFirstVisPos() <= nPos) && (nPos <= GetLastVisPos());
}

sal_Int32 ScCsvControl::GetHdrX() const
{
    return IsRTL() ? (mrData.mnWinWidth - GetHdrWidth()) : 0;
}

sal_Int32 ScCsvControl::GetFirstX() const
{
    return IsRTL() ? 0 : GetHdrWidth();
}

sal_Int32 ScCsvControl::GetLastX() const
{
    return mrData.mnWinWidth - (IsRTL() ? GetHdrWidth() : 0) - 1;
}

sal_Int32 ScCsvControl::GetX( sal_Int32 nPos ) const
{
    return GetFirstX() + (nPos - GetFirstVisPos()) * GetCharWidth();
}

sal_Int32 ScCsvControl::GetPosFromX( sal_Int32 nX ) const
{
    return (nX - GetFirstX() + GetCharWidth() / 2) / GetCharWidth() + GetFirstVisPos();
}

sal_Int32 ScCsvControl::GetVisLineCount() const
{
    return (mrData.mnWinHeight - GetHdrHeight() - 2) / GetLineHeight() + 1;
}

sal_Int32 ScCsvControl::GetLastVisLine() const
{
    return std::min( GetFirstVisLine() + GetVisLineCount(), GetLineCount() ) - 1;
}

sal_Int32 ScCsvControl::GetMaxLineOffset() const
{
    return std::max<sal_Int32>( GetLineCount() - GetVisLineCount() + 1, 0 );
}

bool ScCsvControl::IsValidLine( sal_Int32 nLine ) const
{
    return (0 <= nLine) && (nLine < GetLineCount());
}

bool ScCsvControl::IsVisibleLine( sal_Int32 nLine ) const
{
    return IsValidLine( nLine ) && (GetFirstVisLine() <= nLine) && (nLine <= GetLastVisLine());
}

sal_Int32 ScCsvControl::GetY( sal_Int32 nLine ) const
{
    return GetHdrHeight() + (nLine - GetFirstVisLine()) * GetLineHeight();
}

sal_Int32 ScCsvControl::GetLineFromY( sal_Int32 nY ) const
{
    return (nY - GetHdrHeight()) / GetLineHeight() + GetFirstVisLine();
}

// static helpers -------------------------------------------------------------

void ScCsvControl::ImplInvertRect( OutputDevice& rOutDev, const tools::Rectangle& rRect )
{
    rOutDev.Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR | PushFlags::RASTEROP );
    rOutDev.SetLineColor( COL_BLACK );
    rOutDev.SetFillColor( COL_BLACK );
    rOutDev.SetRasterOp( RasterOp::Invert );
    rOutDev.DrawRect( rRect );
    rOutDev.Pop();
}

ScMoveMode ScCsvControl::GetHorzDirection( sal_uInt16 nCode, bool bHomeEnd )
{
    switch( nCode )
    {
        case KEY_LEFT:  return MOVE_PREV;
        case KEY_RIGHT: return MOVE_NEXT;
    }
    if( bHomeEnd ) switch( nCode )
    {
        case KEY_HOME:  return MOVE_FIRST;
        case KEY_END:   return MOVE_LAST;
    }
    return MOVE_NONE;
}

ScMoveMode ScCsvControl::GetVertDirection( sal_uInt16 nCode, bool bHomeEnd )
{
    switch( nCode )
    {
        case KEY_UP:        return MOVE_PREV;
        case KEY_DOWN:      return MOVE_NEXT;
        case KEY_PAGEUP:    return MOVE_PREVPAGE;
        case KEY_PAGEDOWN:  return MOVE_NEXTPAGE;
    }
    if( bHomeEnd ) switch( nCode )
    {
        case KEY_HOME:      return MOVE_FIRST;
        case KEY_END:       return MOVE_LAST;
    }
    return MOVE_NONE;
}

// accessibility --------------------------------------------------------------

css::uno::Reference< css::accessibility::XAccessible > ScCsvControl::CreateAccessible()
{
    mxAccessible = ImplCreateAccessible().get();
    return css::uno::Reference< css::accessibility::XAccessible >(mxAccessible.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
