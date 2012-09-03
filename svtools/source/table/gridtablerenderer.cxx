/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "cellvalueconversion.hxx"
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/colorcfg.hxx"

#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/decoview.hxx>

//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::TypeClass_INTERFACE;
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

    //==================================================================================================================
    //= CachedSortIndicator
    //==================================================================================================================
    class CachedSortIndicator
    {
    public:
        CachedSortIndicator()
            :m_lastHeaderHeight( 0 )
            ,m_lastArrowColor( COL_TRANSPARENT )
        {
        }

        BitmapEx const & getBitmapFor( OutputDevice const & i_device, long const i_headerHeight, StyleSettings const & i_style, bool const i_sortAscending );

    private:
        long        m_lastHeaderHeight;
        Color       m_lastArrowColor;
        BitmapEx    m_sortAscending;
        BitmapEx    m_sortDescending;
    };

    //------------------------------------------------------------------------------------------------------------------
    BitmapEx const & CachedSortIndicator::getBitmapFor( OutputDevice const & i_device, long const i_headerHeight,
        StyleSettings const & i_style, bool const i_sortAscending )
    {
        BitmapEx & rBitmap( i_sortAscending ? m_sortAscending : m_sortDescending );
        if ( !rBitmap || ( i_headerHeight != m_lastHeaderHeight ) || ( i_style.GetActiveColor() != m_lastArrowColor ) )
        {
            long const nSortIndicatorWidth = 2 * i_headerHeight / 3;
            long const nSortIndicatorHeight = 2 * nSortIndicatorWidth / 3;

            Point const aBitmapPos( 0, 0 );
            Size const aBitmapSize( nSortIndicatorWidth, nSortIndicatorHeight );
            VirtualDevice aDevice( i_device, 0, 0 );
            aDevice.SetOutputSizePixel( aBitmapSize );

            DecorationView aDecoView( &aDevice );
            aDecoView.DrawSymbol(
                Rectangle( aBitmapPos, aBitmapSize ),
                i_sortAscending ? SYMBOL_SPIN_UP : SYMBOL_SPIN_DOWN,
                i_style.GetActiveColor()
            );

            rBitmap = aDevice.GetBitmapEx( aBitmapPos, aBitmapSize );
            m_lastHeaderHeight = i_headerHeight;
            m_lastArrowColor = i_style.GetActiveColor();
        }
        return rBitmap;
    }

    //==================================================================================================================
    //= GridTableRenderer_Impl
    //==================================================================================================================
    struct GridTableRenderer_Impl
    {
        ITableModel&        rModel;
        RowPos              nCurrentRow;
        bool                bUseGridLines;
        CachedSortIndicator aSortIndicator;
        CellValueConversion aStringConverter;

        GridTableRenderer_Impl( ITableModel& _rModel )
            :rModel( _rModel )
            ,nCurrentRow( ROW_INVALID )
            ,bUseGridLines( true )
            ,aSortIndicator( )
            ,aStringConverter( ::comphelper::ComponentContext( ::comphelper::getProcessServiceFactory() ) )
        {
        }
    };

    //==================================================================================================================
    //= helper
    //==================================================================================================================
    namespace
    {
        static Rectangle lcl_getContentArea( GridTableRenderer_Impl const & i_impl, Rectangle const & i_cellArea )
        {
            Rectangle aContentArea( i_cellArea );
            if ( i_impl.bUseGridLines )
            {
                --aContentArea.Right();
                --aContentArea.Bottom();
            }
            return aContentArea;
        }
        static Rectangle lcl_getTextRenderingArea( Rectangle const & i_contentArea )
        {
            Rectangle aTextArea( i_contentArea );
            aTextArea.Left() += 2; aTextArea.Right() -= 2;
            ++aTextArea.Top(); --aTextArea.Bottom();
            return aTextArea;
        }

        static sal_uLong lcl_getAlignmentTextDrawFlags( GridTableRenderer_Impl const & i_impl, ColPos const i_columnPos )
        {
            sal_uLong nVertFlag = TEXT_DRAW_TOP;
            VerticalAlignment const eVertAlign = i_impl.rModel.getVerticalAlign();
            switch ( eVertAlign )
            {
            case VerticalAlignment_MIDDLE:  nVertFlag = TEXT_DRAW_VCENTER;  break;
            case VerticalAlignment_BOTTOM:  nVertFlag = TEXT_DRAW_BOTTOM;   break;
            default:
                break;
            }

            sal_uLong nHorzFlag = TEXT_DRAW_LEFT;
            HorizontalAlignment const eHorzAlign = i_impl.rModel.getColumnCount() > 0
                                                ?  i_impl.rModel.getColumnModel( i_columnPos )->getHorizontalAlign()
                                                :  HorizontalAlignment_CENTER;
            switch ( eHorzAlign )
            {
            case HorizontalAlignment_CENTER:    nHorzFlag = TEXT_DRAW_CENTER;   break;
            case HorizontalAlignment_RIGHT:     nHorzFlag = TEXT_DRAW_RIGHT;    break;
            default:
                break;
            }

            return nVertFlag | nHorzFlag;
        }

    }

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

        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR );

        Color const background = lcl_getEffectiveColor( m_pImpl->rModel.getHeaderBackgroundColor(), _rStyle, &StyleSettings::GetDialogColor );
        _rDevice.SetFillColor( background );

        _rDevice.SetLineColor();
        _rDevice.DrawRect( _rArea );

        // delimiter lines at bottom/right
        ::boost::optional< ::Color > aLineColor( m_pImpl->rModel.getLineColor() );
        ::Color const lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;
        _rDevice.SetLineColor( lineColor );
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

        String sHeaderText;
        PColumnModel const pColumn = m_pImpl->rModel.getColumnModel( _nCol );
        DBG_ASSERT( !!pColumn, "GridTableRenderer::PaintColumnHeader: invalid column model object!" );
        if ( !!pColumn )
            sHeaderText = pColumn->getName();

        ::Color const textColor = lcl_getEffectiveColor( m_pImpl->rModel.getTextColor(), _rStyle, &StyleSettings::GetFieldTextColor );
        _rDevice.SetTextColor( textColor );

        Rectangle const aTextRect( lcl_getTextRenderingArea( lcl_getContentArea( *m_pImpl, _rArea ) ) );
        sal_uLong const nDrawTextFlags = lcl_getAlignmentTextDrawFlags( *m_pImpl, _nCol ) | TEXT_DRAW_CLIP;
        _rDevice.DrawText( aTextRect, sHeaderText, nDrawTextFlags );

        ::boost::optional< ::Color > const aLineColor( m_pImpl->rModel.getLineColor() );
        ::Color const lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;
        _rDevice.SetLineColor( lineColor );
        _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight());
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );

        // draw sort indicator if the model data is sorted by the given column
        ITableDataSort const * pSortAdapter = m_pImpl->rModel.getSortAdapter();
        ColumnSort aCurrentSortOrder;
        if ( pSortAdapter != NULL )
            aCurrentSortOrder = pSortAdapter->getCurrentSortOrder();
        if ( aCurrentSortOrder.nColumnPos == _nCol )
        {
            long const nHeaderHeight( _rArea.GetHeight() );
            BitmapEx const aIndicatorBitmap = m_pImpl->aSortIndicator.getBitmapFor( _rDevice, nHeaderHeight, _rStyle,
                aCurrentSortOrder.eSortDirection == ColumnSortAscending );
            Size const aBitmapSize( aIndicatorBitmap.GetSizePixel() );
            long const nSortIndicatorPaddingX = 2;
            long const nSortIndicatorPaddingY = ( nHeaderHeight - aBitmapSize.Height() ) / 2;

            if ( ( nDrawTextFlags & TEXT_DRAW_RIGHT ) != 0 )
            {
                // text is right aligned => draw the sort indicator at the left hand side
                _rDevice.DrawBitmapEx(
                    Point( _rArea.Left() + nSortIndicatorPaddingX, _rArea.Top() + nSortIndicatorPaddingY ),
                    aIndicatorBitmap
                );
            }
            else
            {
                // text is left-aligned or centered => draw the sort indicator at the right hand side
                _rDevice.DrawBitmapEx(
                    Point( _rArea.Right() - nSortIndicatorPaddingX - aBitmapSize.Width(), nSortIndicatorPaddingY ),
                    aIndicatorBitmap
                );
            }
        }

        _rDevice.Pop();

        (void)_bActive;
        // no special painting for the active column at the moment

        (void)_bSelected;
        // selection for column header not yet implemented
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::PrepareRow( RowPos _nRow, bool i_hasControlFocus, bool _bSelected,
        OutputDevice& _rDevice, const Rectangle& _rRowArea, const StyleSettings& _rStyle )
    {
        // remember the row for subsequent calls to the other ->ITableRenderer methods
        m_pImpl->nCurrentRow = _nRow;

        _rDevice.Push( PUSH_FILLCOLOR | PUSH_LINECOLOR);

        ::Color backgroundColor = _rStyle.GetFieldColor();

        ::boost::optional< ::Color > const aLineColor( m_pImpl->rModel.getLineColor() );
        ::Color lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;

        ::Color const activeSelectionBackColor =
            lcl_getEffectiveColor( m_pImpl->rModel.getActiveSelectionBackColor(), _rStyle, &StyleSettings::GetHighlightColor );
        if ( _bSelected )
        {
            // selected rows use the background color from the style
            backgroundColor = i_hasControlFocus
                ?   activeSelectionBackColor
                :   lcl_getEffectiveColor( m_pImpl->rModel.getInactiveSelectionBackColor(), _rStyle, &StyleSettings::GetDeactiveColor );
            if ( !aLineColor )
                lineColor = backgroundColor;
        }
        else
        {
            ::boost::optional< ::std::vector< ::Color > > aRowColors = m_pImpl->rModel.getRowBackgroundColors();
            if ( !aRowColors )
            {
                // use alternating default colors
                Color const fieldColor = _rStyle.GetFieldColor();
                if ( _rStyle.GetHighContrastMode() || ( ( m_pImpl->nCurrentRow % 2 ) == 0 ) )
                {
                    backgroundColor = fieldColor;
                }
                else
                {
                    Color hilightColor = activeSelectionBackColor;
                    hilightColor.SetRed( 9 * ( fieldColor.GetRed() - hilightColor.GetRed() ) / 10 + hilightColor.GetRed() );
                    hilightColor.SetGreen( 9 * ( fieldColor.GetGreen() - hilightColor.GetGreen() ) / 10 + hilightColor.GetGreen() );
                    hilightColor.SetBlue( 9 * ( fieldColor.GetBlue() - hilightColor.GetBlue() ) / 10 + hilightColor.GetBlue() );
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

        //m_pImpl->bUseGridLines ? _rDevice.SetLineColor( lineColor ) : _rDevice.SetLineColor();
        _rDevice.SetLineColor();
        _rDevice.SetFillColor( backgroundColor );
        _rDevice.DrawRect( _rRowArea );

        _rDevice.Pop();
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::PaintRowHeader( bool i_hasControlFocus, bool _bSelected, OutputDevice& _rDevice, const Rectangle& _rArea,
        const StyleSettings& _rStyle )
    {
        _rDevice.Push( PUSH_LINECOLOR | PUSH_TEXTCOLOR );

        ::boost::optional< ::Color > const aLineColor( m_pImpl->rModel.getLineColor() );
        ::Color const lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;
        _rDevice.SetLineColor( lineColor );
        _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );

        Any const rowHeading( m_pImpl->rModel.getRowHeading( m_pImpl->nCurrentRow ) );
        ::rtl::OUString const rowTitle( m_pImpl->aStringConverter.convertToString( rowHeading ) );
        if ( !rowTitle.isEmpty() )
        {
            ::Color const textColor = lcl_getEffectiveColor( m_pImpl->rModel.getHeaderTextColor(), _rStyle, &StyleSettings::GetFieldTextColor );
            _rDevice.SetTextColor( textColor );

            Rectangle const aTextRect( lcl_getTextRenderingArea( lcl_getContentArea( *m_pImpl, _rArea ) ) );
            sal_uLong const nDrawTextFlags = lcl_getAlignmentTextDrawFlags( *m_pImpl, 0 ) | TEXT_DRAW_CLIP;
                // TODO: is using the horizontal alignment of the 0'th column a good idea here? This is pretty ... arbitray ..
            _rDevice.DrawText( aTextRect, rowTitle, nDrawTextFlags );
        }

        (void)i_hasControlFocus;
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
        bool const              bHasControlFocus;

        CellRenderContext( OutputDevice& i_device, Rectangle const & i_contentArea,
            StyleSettings const & i_style, ColPos const i_column, bool const i_selected, bool const i_hasControlFocus )
            :rDevice( i_device )
            ,aContentArea( i_contentArea )
            ,rStyle( i_style )
            ,nColumn( i_column )
            ,bSelected( i_selected )
            ,bHasControlFocus( i_hasControlFocus )
        {
        }
    };

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::PaintCell( ColPos const i_column, bool _bSelected, bool i_hasControlFocus,
        OutputDevice& _rDevice, const Rectangle& _rArea, const StyleSettings& _rStyle )
    {
        _rDevice.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );

        Rectangle const aContentArea( lcl_getContentArea( *m_pImpl, _rArea ) );
        CellRenderContext const aRenderContext( _rDevice, aContentArea, _rStyle, i_column, _bSelected, i_hasControlFocus );
        impl_paintCellContent( aRenderContext );

        if ( m_pImpl->bUseGridLines )
        {
            ::boost::optional< ::Color > aLineColor( m_pImpl->rModel.getLineColor() );
            ::Color lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;

            if ( _bSelected && !aLineColor )
            {
                // if no line color is specified by the model, use the usual selection color for lines in selected cells
                lineColor = i_hasControlFocus
                    ?   lcl_getEffectiveColor( m_pImpl->rModel.getActiveSelectionBackColor(), _rStyle, &StyleSettings::GetHighlightColor )
                    :   lcl_getEffectiveColor( m_pImpl->rModel.getInactiveSelectionBackColor(), _rStyle, &StyleSettings::GetDeactiveColor );
            }

            _rDevice.SetLineColor( lineColor );
            _rDevice.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );
            _rDevice.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );
        }

        _rDevice.Pop();
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

        if ( aCellContent.getValueTypeClass() == TypeClass_INTERFACE )
        {
            Reference< XInterface > const xContentInterface( aCellContent, UNO_QUERY );
            if ( !xContentInterface.is() )
                // allowed. kind of.
                return;

            Reference< XGraphic > const xGraphic( aCellContent, UNO_QUERY );
            ENSURE_OR_RETURN_VOID( xGraphic.is(), "GridTableRenderer::impl_paintCellContent: only XGraphic interfaces (or NULL) are supported for painting." );

            const Image aImage( xGraphic );
            impl_paintCellImage( i_context, aImage );
            return;
        }

        const ::rtl::OUString sText( m_pImpl->aStringConverter.convertToString( aCellContent ) );
        impl_paintCellText( i_context, sText );
    }

    //------------------------------------------------------------------------------------------------------------------
    void GridTableRenderer::impl_paintCellText( CellRenderContext const & i_context, ::rtl::OUString const & i_text )
    {
        if ( i_context.bSelected )
        {
            ::Color const textColor = i_context.bHasControlFocus
                ?   lcl_getEffectiveColor( m_pImpl->rModel.getActiveSelectionTextColor(), i_context.rStyle, &StyleSettings::GetHighlightTextColor )
                :   lcl_getEffectiveColor( m_pImpl->rModel.getInactiveSelectionTextColor(), i_context.rStyle, &StyleSettings::GetDeactiveTextColor );
            i_context.rDevice.SetTextColor( textColor );
        }
        else
        {
            ::Color const textColor = lcl_getEffectiveColor( m_pImpl->rModel.getTextColor(), i_context.rStyle, &StyleSettings::GetFieldTextColor );
            i_context.rDevice.SetTextColor( textColor );
        }

        Rectangle const textRect( lcl_getTextRenderingArea( i_context.aContentArea ) );
        sal_uLong const nDrawTextFlags = lcl_getAlignmentTextDrawFlags( *m_pImpl, i_context.nColumn ) | TEXT_DRAW_CLIP;
        i_context.rDevice.DrawText( textRect, i_text, nDrawTextFlags );
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
        bool const i_active, bool const i_selected, OutputDevice& i_targetDevice, Rectangle const & i_targetArea ) const
    {
        if ( !i_cellContent.hasValue() )
            return true;

        if ( i_cellContent.getValueTypeClass() == TypeClass_INTERFACE )
        {
            Reference< XInterface > const xContentInterface( i_cellContent, UNO_QUERY );
            if ( !xContentInterface.is() )
                return true;

            Reference< XGraphic > const xGraphic( i_cellContent, UNO_QUERY );
            if ( xGraphic.is() )
                // for the moment, assume it fits. We can always scale it down during painting ...
                return true;

            OSL_ENSURE( false, "GridTableRenderer::FitsIntoCell: only XGraphic interfaces (or NULL) are supported for painting." );
            return true;
        }

        ::rtl::OUString const sText( m_pImpl->aStringConverter.convertToString( i_cellContent ) );
        if ( sText.isEmpty() )
            return true;

        Rectangle const aTargetArea( lcl_getTextRenderingArea( lcl_getContentArea( *m_pImpl, i_targetArea ) ) );

        long const nTextHeight = i_targetDevice.GetTextHeight();
        if ( nTextHeight > aTargetArea.GetHeight() )
            return false;

        long const nTextWidth = i_targetDevice.GetTextWidth( sText );
        if ( nTextWidth > aTargetArea.GetWidth() )
            return false;

        OSL_UNUSED( i_active );
        OSL_UNUSED( i_selected );
        OSL_UNUSED( i_rowPos );
        OSL_UNUSED( i_colPos );
        return true;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool GridTableRenderer::GetFormattedCellString( Any const & i_cellValue, ColPos const i_colPos, RowPos const i_rowPos, ::rtl::OUString & o_cellString ) const
    {
        o_cellString = m_pImpl->aStringConverter.convertToString( i_cellValue );

        OSL_UNUSED( i_colPos );
        OSL_UNUSED( i_rowPos );
        return true;
    }

//......................................................................................................................
} } // namespace svt::table
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
