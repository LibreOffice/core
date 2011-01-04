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

#include "cellvalueconversion.hxx"
#include "svtools/table/gridtablerenderer.hxx"

#include <com/sun/star/graphic/XGraphic.hpp>

#include <tools/debug.hxx>
#include <vcl/window.hxx>
#include <vcl/image.hxx>

//........................................................................
namespace svt { namespace table
{
//........................................................................

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::graphic::XGraphic;
    using ::com::sun::star::style::HorizontalAlignment;
    using ::com::sun::star::style::HorizontalAlignment_LEFT;
    using ::com::sun::star::style::HorizontalAlignment_CENTER;
    using ::com::sun::star::style::HorizontalAlignment_RIGHT;
    using ::com::sun::star::style::VerticalAlignment;
    using ::com::sun::star::style::VerticalAlignment_TOP;
    using ::com::sun::star::style::VerticalAlignment_MIDDLE;
    using ::com::sun::star::style::VerticalAlignment_BOTTOM;

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
        if ( background != COL_TRANSPARENT )
            _rDevice.SetFillColor( background );
        else
            _rDevice.SetFillColor( _rStyle.GetDialogColor() );
        _rDevice.SetLineColor( _rStyle.GetSeparatorColor() );
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
        ULONG nHorFlag = TEXT_DRAW_LEFT;
        ULONG nVerFlag = TEXT_DRAW_TOP;
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
        const Color lineColor =  m_pImpl->rModel.getLineColor();
        Color aRowBackground2 = m_pImpl->rModel.getEvenRowBackgroundColor();
        const Color fieldColor = _rStyle.GetFieldColor();
        if ( aRowBackground == COL_TRANSPARENT )
            aRowBackground = fieldColor;
        if ( aRowBackground2 == COL_TRANSPARENT )
            aRowBackground2 = fieldColor;
        //if row is selected background color becomes blue, and lines should be also blue
        //if they aren't user defined
        if ( _bSelected )
        {
            const Color aSelected( _rStyle.GetHighlightColor() );
            aRowBackground = aSelected;
            if ( lineColor == COL_TRANSPARENT )
                _rDevice.SetLineColor( aRowBackground );
            else
                _rDevice.SetLineColor( lineColor );
        }
        // if row not selected, check the cases whether user defined backgrounds are set
        // and set line color to be the same
        else
        {
            if ( ( aRowBackground2 != fieldColor ) && ( _nRow % 2 ) )
            {
                aRowBackground = aRowBackground2;
                if ( lineColor == COL_TRANSPARENT )
                    _rDevice.SetLineColor( aRowBackground );
                else
                    _rDevice.SetLineColor( lineColor );
            }
            //fill the rows with alternating background colors if second background color is specified
            else if ( aRowBackground != fieldColor && lineColor == COL_TRANSPARENT )
                _rDevice.SetLineColor( aRowBackground );
            else
            {
                //if Line color is set, then it was user defined and should be visible
                //if it wasn't set, it'll be the same as the default background color, so lines still won't be visible
                _rDevice.SetLineColor( lineColor );
            }
        }
        _rDevice.SetFillColor( aRowBackground );
        _rDevice.DrawRect( _rRowArea );

        // TODO: active?

        _rDevice.Pop();
        (void)_bActive;
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::PaintRowHeader( RowPos const i_rowPos, bool _bActive, bool _bSelected, OutputDevice& _rDevice, const Rectangle& _rArea,
        const StyleSettings& _rStyle )
    {
        _rDevice.Push( PUSH_LINECOLOR | PUSH_TEXTCOLOR );

        _rDevice.SetLineColor( _rStyle.GetSeparatorColor() );
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );

        if ( m_pImpl->rModel.getTextColor() != 0x000000 )
            _rDevice.SetTextColor( m_pImpl->rModel.getTextColor() );
        else
            _rDevice.SetTextColor( _rStyle.GetFieldTextColor() );

        ULONG nHorFlag = TEXT_DRAW_LEFT;
        ULONG nVerFlag = TEXT_DRAW_TOP;
        if ( m_pImpl->rModel.getVerticalAlign() == 1 )
            nVerFlag = TEXT_DRAW_VCENTER;
        else if ( m_pImpl->rModel.getVerticalAlign() == 2 )
            nVerFlag = TEXT_DRAW_BOTTOM;
        if ( m_pImpl->rModel.getColumnModel(0)->getHorizontalAlign() == 1 )
            nHorFlag = TEXT_DRAW_CENTER;
        else if ( m_pImpl->rModel.getColumnModel(0)->getHorizontalAlign() == 2 )
            nHorFlag = TEXT_DRAW_RIGHT;

        Rectangle aRect( _rArea );
        aRect.Left()+=4; aRect.Right()-=4;
        aRect.Bottom()-=2;
        _rDevice.DrawText( aRect, m_pImpl->rModel.getRowHeader( i_rowPos ), nHorFlag | nVerFlag | TEXT_DRAW_CLIP );

        // TODO: active? selected?
        (void)_bActive;
        (void)_bSelected;
        _rDevice.Pop();
    }

    //--------------------------------------------------------------------
    struct GridTableRenderer::CellRenderContext
    {
        OutputDevice&           rDevice;
        Rectangle const         aContentArea;
        StyleSettings const &   rStyle;
        ColPos const            nColumn;
        bool const              bSelected;

        CellRenderContext( OutputDevice& i_device, Rectangle const & i_contentArea,
            StyleSettings const & i_style, ColPos const i_column, bool const i_selected )
            :rDevice( i_device )
            ,aContentArea( i_contentArea )
            ,rStyle( i_style )
            ,nColumn( i_column )
            ,bSelected( i_selected )
        {
        }
    };

    //--------------------------------------------------------------------
    void GridTableRenderer::PaintCell( ColPos const i_column, bool _bSelected, bool _bActive,
        OutputDevice& _rDevice, const Rectangle& _rArea, const StyleSettings& _rStyle )
   {
        _rDevice.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
        Color background1 = m_pImpl->rModel.getOddRowBackgroundColor();
        Color background2 = m_pImpl->rModel.getEvenRowBackgroundColor();
        Color lineColor = m_pImpl->rModel.getLineColor();

        // if row is selected and line color isn't user specified, use the settings' color
        if ( _bSelected )
        {
            if ( lineColor == COL_TRANSPARENT )
                _rDevice.SetLineColor( _rStyle.GetHighlightColor() );
            else
                _rDevice.SetLineColor( lineColor );
        }
        // else set line color to the color of row background
        else
        {
            if ( ( background2 != COL_TRANSPARENT ) && ( m_pImpl->nCurrentRow % 2 ) )
            {
                if ( lineColor == COL_TRANSPARENT )
                    _rDevice.SetLineColor( background2 );
                else
                    _rDevice.SetLineColor( lineColor );
            }
            else if ( ( background1 != COL_TRANSPARENT ) && ( lineColor == COL_TRANSPARENT ) )
                _rDevice.SetLineColor( background1 );
            else
            {
                //if line color is set, then it was user defined and should be visible
                //if it wasn't set, it'll be the same as the default background color, so lines still won't be visible
                _rDevice.SetLineColor( lineColor );
            }
        }
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );

        Rectangle aRect( _rArea );
        ++aRect.Left(); --aRect.Right();

        const CellRenderContext aRenderContext( _rDevice, aRect, _rStyle, i_column, _bSelected );
        impl_paintCellContent( aRenderContext );

        _rDevice.Pop();

        (void)_bActive;
        // no special painting for the active cell at the moment
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::impl_paintCellImage( CellRenderContext const & i_context, Image const & i_image )
    {
        Point imagePos( Point( i_context.aContentArea.Left(), i_context.aContentArea.Top() ) );
        Size imageSize = i_image.GetSizePixel();
        if ( i_context.aContentArea.GetWidth() > imageSize.Width() )
        {
            const HorizontalAlignment eHorzAlign = m_pImpl->rModel.getColumnModel( i_context.nColumn )->getHorizontalAlign();
            switch ( eHorzAlign )
            {
            case HorizontalAlignment_CENTER:
                imagePos.X() += ( i_context.aContentArea.GetWidth() - imageSize.Width() ) / 2;
                break;
            case HorizontalAlignment_RIGHT:
                imagePos.X() = i_context.aContentArea.Right() - imageSize.Width();
                break;
            default:
                break;
            }

        }
        else
            imageSize.Width() = i_context.aContentArea.GetWidth();

        if ( i_context.aContentArea.GetHeight() > imageSize.Height() )
        {
            const VerticalAlignment eVertAlign = m_pImpl->rModel.getVerticalAlign();
            switch ( eVertAlign )
            {
            case VerticalAlignment_MIDDLE:
                imagePos.Y() += ( i_context.aContentArea.GetHeight() - imageSize.Height() ) / 2;
                break;
            case VerticalAlignment_BOTTOM:
                imagePos.Y() = i_context.aContentArea.Bottom() - imageSize.Height();
                break;
            default:
                break;
            }
        }
        else
            imageSize.Height() = i_context.aContentArea.GetHeight() - 1;

        i_context.rDevice.DrawImage( imagePos, imageSize, i_image, 0 );
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::impl_paintCellContent( CellRenderContext const & i_context )
    {
        Any aCellContent;
        m_pImpl->rModel.getCellContent( i_context.nColumn, m_pImpl->nCurrentRow, aCellContent );

        const Reference< XGraphic > xGraphic( aCellContent, UNO_QUERY );
        if ( xGraphic.is() )
        {
            const Image aImage( xGraphic );
            impl_paintCellImage( i_context, aImage );
            return;
        }

        const ::rtl::OUString sText( CellValueConversion::convertToString( aCellContent ) );
        impl_paintCellText( i_context, sText );
    }

    //--------------------------------------------------------------------
    void GridTableRenderer::impl_paintCellText( CellRenderContext const & i_context, ::rtl::OUString const & i_text )
    {
        if ( i_context.bSelected )
            i_context.rDevice.SetTextColor( i_context.rStyle.GetHighlightTextColor() );
        else if ( m_pImpl->rModel.getTextColor() != 0x000000 )
            i_context.rDevice.SetTextColor( m_pImpl->rModel.getTextColor() );
        else
            i_context.rDevice.SetTextColor( i_context.rStyle.GetFieldTextColor() );


        ULONG nVerFlag = TEXT_DRAW_TOP;
        const VerticalAlignment eVertAlign = m_pImpl->rModel.getVerticalAlign();
        switch ( eVertAlign )
        {
        case VerticalAlignment_MIDDLE:  nVerFlag = TEXT_DRAW_VCENTER;   break;
        case VerticalAlignment_BOTTOM:  nVerFlag = TEXT_DRAW_BOTTOM;    break;
        default:
            break;
        }

        ULONG nHorFlag = TEXT_DRAW_LEFT;
        const HorizontalAlignment eHorzAlign = m_pImpl->rModel.getColumnModel( i_context.nColumn )->getHorizontalAlign();
        switch ( eHorzAlign )
        {
        case HorizontalAlignment_CENTER:    nHorFlag = TEXT_DRAW_CENTER;    break;
        case HorizontalAlignment_RIGHT:     nHorFlag = TEXT_DRAW_RIGHT;     break;
        default:
            break;
        }

        Rectangle textRect( i_context.aContentArea );
        textRect.Left() += 2; textRect.Right() -= 2;
        ++textRect.Top(); --textRect.Bottom();

        i_context.rDevice.DrawText( textRect, i_text, nHorFlag | nVerFlag | TEXT_DRAW_CLIP );
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

