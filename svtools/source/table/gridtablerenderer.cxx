/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "svtools/table/gridtablerenderer.hxx"

#include <tools/debug.hxx>
#include <vcl/window.hxx>

//........................................................................
namespace svt { namespace table
{
//........................................................................

    struct GridTableRenderer_Impl
    {
        ITableModel&    rModel;
        RowPos          nCurrentRow;

        GridTableRenderer_Impl( ITableModel& _rModel )
            :rModel( _rModel )
            ,nCurrentRow( ROW_INVALID )
        {
        }
    };

    //====================================================================
    //= GridTableRenderer
    //====================================================================
    //--------------------------------------------------------------------
    GridTableRenderer::GridTableRenderer( ITableModel& _rModel )
        :m_pImpl( new GridTableRenderer_Impl( _rModel ) )
    {
    }

    //--------------------------------------------------------------------
    GridTableRenderer::~GridTableRenderer()
    {
        DELETEZ( m_pImpl );
    }

    //--------------------------------------------------------------------
    RowPos GridTableRenderer::getCurrentRow()
    {
        return m_pImpl->nCurrentRow;
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PaintHeaderArea(
        OutputDevice& _rDevice, const Rectangle& _rArea, bool _bIsColHeaderArea, bool _bIsRowHeaderArea,
        const StyleSettings& _rStyle )
    {
        OSL_PRECOND( _bIsColHeaderArea || _bIsRowHeaderArea,
            "GridTableRenderer::PaintHeaderArea: invalid area flags!" );

        // fill the rows with alternating background colors
        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR );

        _rDevice.SetLineColor();
        _rDevice.SetFillColor( _rStyle.GetDialogColor() );
        _rDevice.DrawRect( _rArea );

        // delimiter lines at bottom/right
        _rDevice.SetLineColor( _rStyle.GetDialogTextColor() );
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );

        _rDevice.Pop();
        (void)_bIsColHeaderArea;
        (void)_bIsRowHeaderArea;
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PaintColumnHeader( ColPos _nCol, bool _bActive, bool _bSelected,
        OutputDevice& _rDevice, const Rectangle& _rArea, const StyleSettings& _rStyle )
    {
        _rDevice.Push( PUSH_LINECOLOR );

        _rDevice.SetLineColor( _rStyle.GetDialogTextColor() );
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );

        String sHeaderText;

        PColumnModel pColumn = m_pImpl->rModel.getColumnModel( _nCol );
        DBG_ASSERT( !!pColumn, "GridTableRenderer::PaintColumnHeader: invalid column model object!" );
        if ( !!pColumn )
            sHeaderText = pColumn->getName();
        Color aRowBackground = _rStyle.GetFieldColor();
        if ( _bSelected )
        {
            aRowBackground = COL_BLUE;
        }
        _rDevice.DrawText( _rArea, sHeaderText, TEXT_DRAW_LEFT | TEXT_DRAW_TOP );
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );
        _rDevice.Pop();

        (void)_bActive;
        // no special painting for the active column at the moment

        //(void)_bSelected;
        // TODO: selection not yet implemented
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PrepareRow( RowPos _nRow, bool _bActive, bool _bSelected,
        OutputDevice& _rDevice, const Rectangle& _rRowArea, const StyleSettings& _rStyle )
    {
        // remember the row for subsequent calls to the other ->ITableRenderer methods
        m_pImpl->nCurrentRow = _nRow;

        // fill the rows with alternating background colors
        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR );

        _rDevice.SetLineColor();

        Color aRowBackground = _rStyle.GetFieldColor();

        _rDevice.SetFillColor( aRowBackground );

        _rDevice.DrawRect( _rRowArea );

        // TODO: active? selected?

        _rDevice.Pop();
        (void) _bSelected;
        (void)_bActive;

        // no special painting for the active row at the moment

        //(void)_bSelected;
        // TODO: selection not yet implemented
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PaintRowHeader( bool _bActive, bool _bSelected, OutputDevice& _rDevice, const Rectangle& _rArea,
        const StyleSettings& _rStyle, rtl::OUString& _rText )
    {
        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR );

        _rDevice.SetLineColor( _rStyle.GetDialogTextColor() );
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );
        _rDevice.DrawText( _rArea, _rText, TEXT_DRAW_LEFT);
        // TODO: active? selected?
        (void)_bActive;
        (void)_bSelected;

        _rDevice.Pop();
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PaintCell( ColPos _nColumn, bool _bSelected, bool _bActive,
        OutputDevice& _rDevice, const Rectangle& _rArea, const StyleSettings& _rStyle, rtl::OUString& _rText )
   {
        _rDevice.Push( PUSH_LINECOLOR );

        // draw the grid
        _rDevice.SetLineColor( COL_LIGHTGRAY );
        // TODO: the LIGHTGRAY should probably be a property/setting
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );

        {
            // TODO: remove those temporary place holders
            Rectangle aRect( _rArea );
            ++aRect.Left(); --aRect.Right();
            ++aRect.Top(); --aRect.Bottom();

            String sText;
            if(_bSelected)
            {
                Color aRed(COL_BLUE);
                _rDevice.SetFillColor( aRed );
                _rDevice.SetTextColor(COL_WHITE);
            }
            _rDevice.DrawRect( _rArea );
            (void)_nColumn;
            _rDevice.DrawText( aRect, _rText, TEXT_DRAW_LEFT | TEXT_DRAW_TOP);
        }
        if(_bSelected)
        {
            _rDevice.SetFillColor( _rStyle.GetFieldColor() );
            _rDevice.SetTextColor(COL_BLACK);
        }

        _rDevice.Pop();

        (void)_bActive;
//        // no special painting for the active cell at the moment
       (void)_rStyle;
//        // TODO: do we need this?
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::ShowCellCursor( Window& _rView, const Rectangle& _rCursorRect)
    {
        _rView.ShowFocus( _rCursorRect );
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::HideCellCursor( Window& _rView, const Rectangle& _rCursorRect)
    {
        (void)_rCursorRect;
        _rView.HideFocus();

    }

//........................................................................
} } // namespace svt::table
//........................................................................

