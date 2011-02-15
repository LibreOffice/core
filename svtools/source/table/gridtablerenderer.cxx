/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "svtools/table/gridtablerenderer.hxx"

#include <tools/debug.hxx>
#include <vcl/window.hxx>
#include <vcl/image.hxx>

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

        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR);
        Color background = m_pImpl->rModel.getHeaderBackgroundColor();
        if( background != 0xFFFFFF)
            _rDevice.SetFillColor(background);
        else
            _rDevice.SetFillColor(_rStyle.GetDialogColor());
        _rDevice.SetLineColor(_rStyle.GetSeparatorColor());
        _rDevice.DrawRect( _rArea );
        // delimiter lines at bottom/right
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
        _rDevice.Push( PUSH_LINECOLOR);
        _rDevice.SetLineColor(_rStyle.GetSeparatorColor());
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight());

        String sHeaderText;
        PColumnModel pColumn = m_pImpl->rModel.getColumnModel( _nCol );
        DBG_ASSERT( !!pColumn, "GridTableRenderer::PaintColumnHeader: invalid column model object!" );
        if ( !!pColumn )
            sHeaderText = pColumn->getName();
        if(m_pImpl->rModel.getTextColor() != 0x000000)
            _rDevice.SetTextColor(m_pImpl->rModel.getTextColor());
        else
            _rDevice.SetTextColor(_rStyle.GetFieldTextColor());
        sal_uLong nHorFlag = TEXT_DRAW_LEFT;
        sal_uLong nVerFlag = TEXT_DRAW_TOP;
        if(m_pImpl->rModel.getVerticalAlign() == 1)
            nVerFlag = TEXT_DRAW_VCENTER;
        else if(m_pImpl->rModel.getVerticalAlign() == 2)
            nVerFlag = TEXT_DRAW_BOTTOM;
        if(m_pImpl->rModel.getColumnModel(_nCol)->getHorizontalAlign() == 1)
            nHorFlag = TEXT_DRAW_CENTER;
        else if(m_pImpl->rModel.getColumnModel(_nCol)->getHorizontalAlign() == 2)
            nHorFlag = TEXT_DRAW_RIGHT;
        Rectangle aRect(_rArea);
        aRect.Left()+=4; aRect.Right()-=4;
        aRect.Bottom()-=2;
            _rDevice.DrawText( aRect, sHeaderText, nHorFlag | nVerFlag | TEXT_DRAW_CLIP);
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );
        _rDevice.Pop();

        (void)_bActive;
        // no special painting for the active column at the moment

        (void)_bSelected;
        //selection for column header not yet implemented
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PrepareRow( RowPos _nRow, bool _bActive, bool _bSelected,
        OutputDevice& _rDevice, const Rectangle& _rRowArea, const StyleSettings& _rStyle )
    {
        // remember the row for subsequent calls to the other ->ITableRenderer methods
        m_pImpl->nCurrentRow = _nRow;

        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR);

        Color aRowBackground = m_pImpl->rModel.getOddRowBackgroundColor();
        Color line =  m_pImpl->rModel.getLineColor();
        Color aRowBackground2 = m_pImpl->rModel.getEvenRowBackgroundColor();
        Color fieldColor = _rStyle.GetFieldColor();
        if(aRowBackground == 0xFFFFFF)
            aRowBackground = fieldColor;
        if(aRowBackground2 == 0xFFFFFF)
            aRowBackground2 = fieldColor;
        //if row is selected background color becomes blue, and lines should be also blue
        //if they aren't user defined
        if(_bSelected)
        {
            Color aSelected(_rStyle.GetHighlightColor());
            aRowBackground = aSelected;
            if(line == 0xFFFFFF)
                _rDevice.SetLineColor(aRowBackground);
            else
                _rDevice.SetLineColor(line);
        }
        //if row not selected, check the cases whether user defined backgrounds are set
        //and set line color to be the same
        else
        {
            if(aRowBackground2 != fieldColor && _nRow%2)
            {
                aRowBackground = aRowBackground2;
                if(line == 0xFFFFFF)
                    _rDevice.SetLineColor(aRowBackground);
                else
                    _rDevice.SetLineColor(line);
            }
            //fill the rows with alternating background colors if second background color is specified
            else if(aRowBackground != fieldColor && line == 0xFFFFFF)
                _rDevice.SetLineColor(aRowBackground);
            else
            {
                //if Line color is set, then it was user defined and should be visible
                //if it wasn't set, it'll be the same as the default background color, so lines still won't be visible
                _rDevice.SetLineColor(line);
            }
        }
        _rDevice.SetFillColor( aRowBackground );
        _rDevice.DrawRect( _rRowArea );

        // TODO: active?

        _rDevice.Pop();
        (void)_bActive;
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PaintRowHeader( bool _bActive, bool _bSelected, OutputDevice& _rDevice, const Rectangle& _rArea,
        const StyleSettings& _rStyle, rtl::OUString& _rText )
    {
        _rDevice.Push( PUSH_LINECOLOR);
        _rDevice.SetLineColor(_rStyle.GetSeparatorColor());
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );
        if(m_pImpl->rModel.getTextColor() != 0x000000)
            _rDevice.SetTextColor(m_pImpl->rModel.getTextColor());
        else
            _rDevice.SetTextColor(_rStyle.GetFieldTextColor());
        sal_uLong nHorFlag = TEXT_DRAW_LEFT;
        sal_uLong nVerFlag = TEXT_DRAW_TOP;
        if(m_pImpl->rModel.getVerticalAlign() == 1)
            nVerFlag = TEXT_DRAW_VCENTER;
        else if(m_pImpl->rModel.getVerticalAlign() == 2)
            nVerFlag = TEXT_DRAW_BOTTOM;
        if(m_pImpl->rModel.getColumnModel(0)->getHorizontalAlign() == 1)
            nHorFlag = TEXT_DRAW_CENTER;
        else if(m_pImpl->rModel.getColumnModel(0)->getHorizontalAlign() == 2)
            nHorFlag = TEXT_DRAW_RIGHT;
        Rectangle aRect(_rArea);
        aRect.Left()+=4; aRect.Right()-=4;
        aRect.Bottom()-=2;
        _rDevice.DrawText( aRect, _rText, nHorFlag | nVerFlag | TEXT_DRAW_CLIP);
        // TODO: active? selected?
        (void)_bActive;
        (void)_bSelected;
        //at the moment no special paint for selected row header
        _rDevice.Pop();
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PaintCellImage( ColPos _nColumn, bool _bSelected, bool _bActive,
        OutputDevice& _rDevice, const Rectangle& _rArea, const StyleSettings& _rStyle, Image* _pCellData )
   {
        _rDevice.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR);
        Color background1 = m_pImpl->rModel.getOddRowBackgroundColor();
        Color background2 = m_pImpl->rModel.getEvenRowBackgroundColor();
        Color line = m_pImpl->rModel.getLineColor();
        //if row is selected and line color isn't user specified, set it blue
        if(_bSelected)
        {
            if(line == 0xFFFFFF)
                _rDevice.SetLineColor(_rStyle.GetHighlightColor());
            else
                _rDevice.SetLineColor(line);
        }
        //else set line color to the color of row background
        else
        {
            if(background2 != 0xFFFFFF && m_pImpl->nCurrentRow%2)
            {
                if(line == 0xFFFFFF)
                    _rDevice.SetLineColor(background2);
                else
                    _rDevice.SetLineColor(line);
            }
            else if(background1 != 0xFFFFFF && line == 0xFFFFFF)
                _rDevice.SetLineColor(background1);
            else
            {
                //if line color is set, then it was user defined and should be visible
                //if it wasn't set, it'll be the same as the default background color, so lines still won't be visible
                _rDevice.SetLineColor(line);
            }
        }
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );

        Rectangle aRect( _rArea );
        ++aRect.Left(); --aRect.Right();
        aRect.Top(); aRect.Bottom();
        Point imagePos(Point(aRect.Left(), aRect.Top()));
        Size imageSize = _pCellData->GetSizePixel();
        if(aRect.GetWidth() > imageSize.Width())
        {
            if(m_pImpl->rModel.getColumnModel(_nColumn)->getHorizontalAlign() == 1)
                imagePos.X() = aRect.Left()+((double)(aRect.GetWidth() - imageSize.Width()))/2;
            else if(m_pImpl->rModel.getColumnModel(_nColumn)->getHorizontalAlign() == 2)
                imagePos.X() = aRect.Right() - imageSize.Width();
        }
        else
            imageSize.Width() = aRect.GetWidth();
        if(aRect.GetHeight() > imageSize.Height())
        {
            if(m_pImpl->rModel.getVerticalAlign() == 1)
                imagePos.Y() = aRect.Top()+((double)(aRect.GetHeight() - imageSize.Height()))/2;
            else if(m_pImpl->rModel.getVerticalAlign() == 2)
                imagePos.Y() = aRect.Bottom() - imageSize.Height();
        }
        else
            imageSize.Height() = aRect.GetHeight()-1;
        Image& image (*_pCellData);
        _rDevice.DrawImage(imagePos, imageSize, image, 0);
        _rDevice.Pop();

        (void)_bActive;
        // no special painting for the active cell at the moment
    }

    //--------------------------------------------------------------------
   void GridTableRenderer::PaintCellString( ColPos _nColumn, bool _bSelected, bool _bActive,
        OutputDevice& _rDevice, const Rectangle& _rArea, const StyleSettings& _rStyle, rtl::OUString& _rText )
   {
        _rDevice.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
        Color background1 = m_pImpl->rModel.getOddRowBackgroundColor();
        Color background2 = m_pImpl->rModel.getEvenRowBackgroundColor();
        Color line = m_pImpl->rModel.getLineColor();
        //if row is selected and line color isn't user specified, set it blue
        if(_bSelected)
        {
            if(line == 0xFFFFFF)
                _rDevice.SetLineColor(_rStyle.GetHighlightColor());
            else
                _rDevice.SetLineColor(line);
        }
        //else set line color to the color of row background
        else
        {
            if(background2 != 0xFFFFFF && m_pImpl->nCurrentRow%2)
            {
                if(line == 0xFFFFFF)
                    _rDevice.SetLineColor(background2);
                else
                    _rDevice.SetLineColor(line);
            }
            else if(background1 != 0xFFFFFF && line == 0xFFFFFF)
                _rDevice.SetLineColor(background1);
            else
            {
                //if Line color is set, then it was user defined and should be visible
                //if it wasn't set, it'll be the same as the default background color, so lines still won't be visible
                _rDevice.SetLineColor(line);
            }
        }
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );

        Rectangle aRect( _rArea );
        ++aRect.Left(); --aRect.Right();
        aRect.Top(); aRect.Bottom();
        if(_bSelected)
            _rDevice.SetTextColor(_rStyle.GetHighlightTextColor());
        else if(m_pImpl->rModel.getTextColor() != 0x000000)
            _rDevice.SetTextColor(m_pImpl->rModel.getTextColor());
        else
            _rDevice.SetTextColor(_rStyle.GetFieldTextColor());
        sal_uLong nHorFlag = TEXT_DRAW_LEFT;
        sal_uLong nVerFlag = TEXT_DRAW_TOP;
        if(m_pImpl->rModel.getVerticalAlign() == 1)
            nVerFlag = TEXT_DRAW_VCENTER;
        else if(m_pImpl->rModel.getVerticalAlign() == 2)
            nVerFlag = TEXT_DRAW_BOTTOM;
        if(m_pImpl->rModel.getColumnModel(_nColumn)->getHorizontalAlign() == 1)
            nHorFlag = TEXT_DRAW_CENTER;
        else if(m_pImpl->rModel.getColumnModel(_nColumn)->getHorizontalAlign() == 2)
            nHorFlag = TEXT_DRAW_RIGHT;
        Rectangle textRect(_rArea);
        textRect.Left()+=4; textRect.Right()-=4;
        textRect.Bottom()-=2;
        _rDevice.DrawText( textRect, _rText, nHorFlag | nVerFlag | TEXT_DRAW_CLIP);

        _rDevice.Pop();
        (void)_bActive;
        // no special painting for the active cell at the moment
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

