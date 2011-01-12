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
#include "svtools/colorcfg.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/graphic/XGraphic.hpp>
/** === end UNO includes === **/

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>
#include <vcl/image.hxx>

//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    /** === begin UNO using === **/
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
    /** === end UNO using === **/

    struct GridTableRenderer_Impl
    {
        ITableModel&    rModel;
        RowPos          nCurrentRow;
        bool            bUseGridLines;

        GridTableRenderer_Impl( ITableModel& _rModel )
            :rModel( _rModel )
            ,nCurrentRow( ROW_INVALID )
            ,bUseGridLines( true )
        {
        }
    };

    //==================================================================================================================
    //= GridTableRenderer
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    GridTableRenderer::GridTableRenderer( ITableModel& _rModel )
        :m_pImpl( new GridTableRenderer_Impl( _rModel ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    GridTableRenderer::~GridTableRenderer()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    RowPos GridTableRenderer::getCurrentRow() const
    {
        return m_pImpl->nCurrentRow;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool GridTableRenderer::useGridLines() const
    {
        return m_pImpl->bUseGridLines;
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::useGridLines( bool const i_use )
    {
        m_pImpl->bUseGridLines = i_use;
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        Color lcl_getEffectiveColor(
            ::boost::optional< ::Color > const & i_modelColor,
            StyleSettings const & i_styleSettings,
            ::Color const & ( StyleSettings::*i_getDefaultColor ) () const
        )
        {
            if ( !!i_modelColor )
                return *i_modelColor;
            return ( i_styleSettings.*i_getDefaultColor )();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::PaintHeaderArea(
        OutputDevice& _rDevice, const Rectangle& _rArea, bool _bIsColHeaderArea, bool _bIsRowHeaderArea,
        const StyleSettings& _rStyle )
    {
        OSL_PRECOND( _bIsColHeaderArea || _bIsRowHeaderArea,
            "GridTableRenderer::PaintHeaderArea: invalid area flags!" );

        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR);

        Color const background = lcl_getEffectiveColor( m_pImpl->rModel.getHeaderBackgroundColor(), _rStyle, &StyleSettings::GetDialogColor );
        _rDevice.SetFillColor( background );

        m_pImpl->bUseGridLines ? _rDevice.SetLineColor( _rStyle.GetSeparatorColor() ) : _rDevice.SetLineColor();
        _rDevice.DrawRect( _rArea );
        // delimiter lines at bottom/right
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );

        _rDevice.Pop();
        (void)_bIsColHeaderArea;
        (void)_bIsRowHeaderArea;
    }

    //------------------------------------------------------------------------------------------------------------------
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

        ::Color const textColor = lcl_getEffectiveColor( m_pImpl->rModel.getTextColor(), _rStyle, &StyleSettings::GetFieldTextColor );
        _rDevice.SetTextColor( textColor );

        ULONG nHorFlag = TEXT_DRAW_LEFT;
        ULONG nVerFlag = TEXT_DRAW_TOP;
        if ( m_pImpl->rModel.getVerticalAlign() == 1 )
            nVerFlag = TEXT_DRAW_VCENTER;
        else if ( m_pImpl->rModel.getVerticalAlign() == 2 )
            nVerFlag = TEXT_DRAW_BOTTOM;
        if ( m_pImpl->rModel.getColumnModel(_nCol)->getHorizontalAlign() == 1 )
            nHorFlag = TEXT_DRAW_CENTER;
        else if ( m_pImpl->rModel.getColumnModel(_nCol)->getHorizontalAlign() == 2 )
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
        // selection for column header not yet implemented
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::PrepareRow( RowPos _nRow, bool _bActive, bool _bSelected,
        OutputDevice& _rDevice, const Rectangle& _rRowArea, const StyleSettings& _rStyle )
    {
        // remember the row for subsequent calls to the other ->ITableRenderer methods
        m_pImpl->nCurrentRow = _nRow;

        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR);

        ::Color backgroundColor = _rStyle.GetFieldColor();

        ::boost::optional< ::Color > aLineColor( m_pImpl->rModel.getLineColor() );
        ::Color lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;

        if ( _bSelected )
        {
            // selected rows use the background color from the style
            backgroundColor = _rStyle.GetHighlightColor();
            if ( !aLineColor )
                lineColor = backgroundColor;
        }
        else
        {
            ::boost::optional< ::std::vector< ::Color > > aRowColors = m_pImpl->rModel.getRowBackgroundColors();
            if ( !aRowColors )
            {
                // use alternating default colors
                if ( ( m_pImpl->nCurrentRow % 2 ) == 0 )
                {
                    backgroundColor = _rStyle.GetFieldColor();
                }
                else
                {
                    Color hilightColor = _rStyle.GetHighlightColor();
                    USHORT const luminance = hilightColor.GetLuminance();
                    hilightColor.SetRed( 9 * ( 255 - hilightColor.GetRed() ) / 10 + hilightColor.GetRed() );
                    hilightColor.SetGreen( 9 * ( 255 - hilightColor.GetGreen() ) / 10 + hilightColor.GetGreen() );
                    hilightColor.SetBlue( 9 * ( 255 - hilightColor.GetBlue() ) / 10 + hilightColor.GetBlue() );
                    backgroundColor = hilightColor;
                }
            }
            else
            {
                if ( aRowColors->empty() )
                {
                    // all colors have the same background color
                    backgroundColor = _rStyle.GetFieldColor();
                }
                else
                {
                    backgroundColor = aRowColors->at( m_pImpl->nCurrentRow % aRowColors->size() );
                }
            }
        }

        m_pImpl->bUseGridLines ? _rDevice.SetLineColor( lineColor ) : _rDevice.SetLineColor();
        _rDevice.SetFillColor( backgroundColor );
        _rDevice.DrawRect( _rRowArea );

        _rDevice.Pop();

        (void)_bActive;
        // row containing the active cell not rendered any special at the moment
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::PaintRowHeader( bool _bActive, bool _bSelected, OutputDevice& _rDevice, const Rectangle& _rArea,
        const StyleSettings& _rStyle )
    {
        _rDevice.Push( PUSH_LINECOLOR | PUSH_TEXTCOLOR );

        ::boost::optional< ::Color > const aLineColor( m_pImpl->rModel.getLineColor() );
        ::Color const lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );

        ::Color const textColor = lcl_getEffectiveColor( m_pImpl->rModel.getHeaderTextColor(), _rStyle, &StyleSettings::GetFieldTextColor );
        _rDevice.SetTextColor( textColor );

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
        _rDevice.DrawText( aRect, m_pImpl->rModel.getRowHeader( m_pImpl->nCurrentRow ), nHorFlag | nVerFlag | TEXT_DRAW_CLIP );

        // TODO: active? selected?
        (void)_bActive;
        (void)_bSelected;
        _rDevice.Pop();
    }

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        /** transforms a rectangle denoting a cell area so that afterwards, it denotes the area we
            can use for rendering the cell's content.
        */
        static void lcl_convertCellToContentArea( Rectangle & io_area )
        {
            ++io_area.Left(); --io_area.Right();
        }

        static void lcl_convertContentToTextRenderingArea( Rectangle & io_area )
        {
            io_area.Left() += 2; io_area.Right() -= 2;
            ++io_area.Top(); --io_area.Bottom();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::PaintCell( ColPos const i_column, bool _bSelected, bool _bActive,
        OutputDevice& _rDevice, const Rectangle& _rArea, const StyleSettings& _rStyle )
    {
        _rDevice.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );

        ::boost::optional< ::Color > aLineColor( m_pImpl->rModel.getLineColor() );
        ::Color lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;

        if ( _bSelected )
        {
            // if no line color is specified by the model, use the usual selection color for lines
            if ( !aLineColor )
                lineColor = _rStyle.GetHighlightColor();
        }

        m_pImpl->bUseGridLines ? _rDevice.SetLineColor( lineColor ) : _rDevice.SetLineColor();
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );

        Rectangle aRect( _rArea );
        lcl_convertCellToContentArea( aRect );

        const CellRenderContext aRenderContext( _rDevice, aRect, _rStyle, i_column, _bSelected );
        impl_paintCellContent( aRenderContext );

        _rDevice.Pop();

        (void)_bActive;
        // no special painting for the active cell at the moment
    }

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::impl_paintCellText( CellRenderContext const & i_context, ::rtl::OUString const & i_text )
    {
        if ( i_context.bSelected )
            i_context.rDevice.SetTextColor( i_context.rStyle.GetHighlightTextColor() );
        else
        {
            ::Color const textColor = lcl_getEffectiveColor( m_pImpl->rModel.getTextColor(), i_context.rStyle, &StyleSettings::GetFieldTextColor );
            i_context.rDevice.SetTextColor( textColor );
        }


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
        lcl_convertContentToTextRenderingArea( textRect );

        i_context.rDevice.DrawText( textRect, i_text, nHorFlag | nVerFlag | TEXT_DRAW_CLIP );
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::ShowCellCursor( Window& _rView, const Rectangle& _rCursorRect)
    {
        _rView.ShowFocus( _rCursorRect );
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::HideCellCursor( Window& _rView, const Rectangle& _rCursorRect)
    {
        (void)_rCursorRect;
        _rView.HideFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    bool GridTableRenderer::FitsIntoCell( Any const & i_cellContent, ColPos const i_colPos, RowPos const i_rowPos,
        bool const i_active, bool const i_selected, OutputDevice& i_targetDevice, Rectangle const & i_targetArea )
    {
        if ( !i_cellContent.hasValue() )
            return true;

        Reference< XGraphic > const xGraphic( i_cellContent, UNO_QUERY );
        if ( xGraphic.is() )
            // for the moment, assume it fits. We can always scale it down during painting ...
            return true;

        ::rtl::OUString const sText( CellValueConversion::convertToString( i_cellContent ) );
        if ( sText.getLength() == 0 )
            return true;

        Rectangle aTargetArea( i_targetArea );
        lcl_convertCellToContentArea( aTargetArea );
        lcl_convertContentToTextRenderingArea( aTargetArea );

        long const nTextHeight = i_targetDevice.GetTextHeight();
        if ( nTextHeight > aTargetArea.GetHeight() )
            return false;

        long const nTextWidth = i_targetDevice.GetTextWidth( sText );
        if ( nTextWidth > aTargetArea.GetWidth() )
            return false;

        OSL_UNUSED( i_active );
        OSL_UNUSED( i_selected );
        return true;
    }

//......................................................................................................................
} } // namespace svt::table
//......................................................................................................................

