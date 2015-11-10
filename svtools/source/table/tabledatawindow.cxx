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

#include "table/tablecontrol.hxx"

#include "tabledatawindow.hxx"
#include "tablecontrol_impl.hxx"
#include "tablegeometry.hxx"

#include <vcl/help.hxx>
#include <vcl/settings.hxx>

namespace svt { namespace table
{
    using css::uno::Any;

    TableDataWindow::TableDataWindow( TableControl_Impl& _rTableControl )
        :Window( &_rTableControl.getAntiImpl() )
        ,m_rTableControl( _rTableControl )
        ,m_nTipWindowHandle( 0 )
    {
        // by default, use the background as determined by the style settings
        const Color aWindowColor( GetSettings().GetStyleSettings().GetFieldColor() );
        SetBackground( Wallpaper( aWindowColor ) );
        SetFillColor( aWindowColor );
    }

    TableDataWindow::~TableDataWindow()
    {
        disposeOnce();
    }

    void TableDataWindow::dispose()
    {
        impl_hideTipWindow();
        Window::dispose();
    }

    void TableDataWindow::Paint( vcl::RenderContext& rRenderContext, const Rectangle& rUpdateRect )
    {
        m_rTableControl.doPaintContent(rRenderContext, rUpdateRect);
    }

    void TableDataWindow::SetBackground( const Wallpaper& rColor )
    {
        Window::SetBackground( rColor );
    }

    void TableDataWindow::RequestHelp( const HelpEvent& rHEvt )
    {
        HelpEventMode const nHelpMode = rHEvt.GetMode();
        if  (   IsMouseCaptured()
            ||  !( nHelpMode & HelpEventMode::QUICK )
            )
        {
            Window::RequestHelp( rHEvt );
            return;
        }

        OUString sHelpText;
        QuickHelpFlags nHelpStyle = QuickHelpFlags::NONE;

        Point const aMousePos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );
        RowPos const hitRow = m_rTableControl.getRowAtPoint( aMousePos );
        ColPos const hitCol = m_rTableControl.getColAtPoint( aMousePos );

        PTableModel const pTableModel( m_rTableControl.getModel() );
        if ( ( hitCol >= 0 ) && ( hitCol < pTableModel->getColumnCount() ) )
        {
            if ( hitRow == ROW_COL_HEADERS )
            {
                sHelpText = pTableModel->getColumnModel( hitCol )->getHelpText();
            }
            else if ( ( hitRow >= 0 ) && ( hitRow < pTableModel->getRowCount() ) )
            {
                Any aCellToolTip;
                pTableModel->getCellToolTip( hitCol, hitRow, aCellToolTip );
                if ( !aCellToolTip.hasValue() )
                {
                    // use the cell content
                    pTableModel->getCellContent( hitCol, hitRow, aCellToolTip );

                    // use the cell content as tool tip only if it doesn't fit into the cell.
                    Rectangle const aWindowRect( Point( 0, 0 ), GetOutputSizePixel() );
                    TableCellGeometry const aCell( m_rTableControl, aWindowRect, hitCol, hitRow );
                    Rectangle const aCellRect( aCell.getRect() );

                    PTableRenderer const pRenderer = pTableModel->getRenderer();
                    if ( pRenderer->FitsIntoCell( aCellToolTip, *this, aCellRect ) )
                        aCellToolTip.clear();
                }

                pTableModel->getRenderer()->GetFormattedCellString( aCellToolTip, sHelpText );

                if ( sHelpText.indexOf( '\n' ) >= 0 )
                    nHelpStyle = QuickHelpFlags::TipStyleBalloon;
            }
        }

        if ( !sHelpText.isEmpty() )
        {
            // hide the standard (singleton) help window, so we do not have two help windows open at the same time
            Help::HideBalloonAndQuickHelp();

            Rectangle const aControlScreenRect(
                OutputToScreenPixel( Point( 0, 0 ) ),
                GetOutputSizePixel()
            );

            if ( m_nTipWindowHandle )
            {
                Help::UpdateTip( m_nTipWindowHandle, this, aControlScreenRect, sHelpText );
            }
            else
                m_nTipWindowHandle = Help::ShowTip( this, aControlScreenRect, sHelpText, nHelpStyle );
        }
        else
        {
            impl_hideTipWindow();
            Window::RequestHelp( rHEvt );
        }
    }


    void TableDataWindow::impl_hideTipWindow()
    {
        if ( m_nTipWindowHandle != 0 )
        {
            Help::HideTip( m_nTipWindowHandle );
            m_nTipWindowHandle = 0;
        }
    }


    void TableDataWindow::MouseMove( const MouseEvent& rMEvt )
    {
        if ( rMEvt.IsLeaveWindow() )
            impl_hideTipWindow();

        if ( !m_rTableControl.getInputHandler()->MouseMove( m_rTableControl, rMEvt ) )
        {
            Window::MouseMove( rMEvt );
        }
    }

    void TableDataWindow::MouseButtonDown( const MouseEvent& rMEvt )
    {
        impl_hideTipWindow();

        Point const aPoint = rMEvt.GetPosPixel();
        RowPos const hitRow = m_rTableControl.getRowAtPoint( aPoint );
        bool const wasRowSelected = m_rTableControl.isRowSelected( hitRow );
        size_t const nPrevSelRowCount = m_rTableControl.getSelectedRowCount();

        if ( !m_rTableControl.getInputHandler()->MouseButtonDown( m_rTableControl, rMEvt ) )
        {
            Window::MouseButtonDown( rMEvt );
            return;
        }

        bool const isRowSelected = m_rTableControl.isRowSelected( hitRow );
        size_t const nCurSelRowCount = m_rTableControl.getSelectedRowCount();
        if ( isRowSelected != wasRowSelected || nCurSelRowCount != nPrevSelRowCount )
        {
            m_aSelectHdl.Call( nullptr );
        }
    }


    void TableDataWindow::MouseButtonUp( const MouseEvent& rMEvt )
    {
        if ( !m_rTableControl.getInputHandler()->MouseButtonUp( m_rTableControl, rMEvt ) )
            Window::MouseButtonUp( rMEvt );

        m_rTableControl.getAntiImpl().GrabFocus();
    }


    bool TableDataWindow::Notify(NotifyEvent& rNEvt )
    {
        bool bDone = false;
        if ( rNEvt.GetType() == MouseNotifyEvent::COMMAND )
        {
            const CommandEvent& rCEvt = *rNEvt.GetCommandEvent();
            if ( rCEvt.GetCommand() == CommandEventId::Wheel )
            {
                const CommandWheelData* pData = rCEvt.GetWheelData();
                if( !pData->GetModifier() && ( pData->GetMode() == CommandWheelMode::SCROLL ) )
                {
                    bDone = HandleScrollCommand( rCEvt, m_rTableControl.getHorzScrollbar(), m_rTableControl.getVertScrollbar() );
                }
            }
        }
        return bDone || Window::Notify( rNEvt );
    }

}} // namespace svt::table


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
