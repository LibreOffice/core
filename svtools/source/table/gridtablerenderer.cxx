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


#include "cellvalueconversion.hxx"
#include <table/gridtablerenderer.hxx>
#include <svtools/table/tablesort.hxx>
#include <svtools/colorcfg.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>


namespace svt { namespace table
{
    using ::css::uno::Any;
    using ::css::uno::Reference;
    using ::css::uno::UNO_QUERY;
    using ::css::uno::XInterface;
    using ::css::uno::TypeClass_INTERFACE;
    using ::css::graphic::XGraphic;
    using ::css::style::HorizontalAlignment;
    using ::css::style::HorizontalAlignment_CENTER;
    using ::css::style::HorizontalAlignment_RIGHT;
    using ::css::style::VerticalAlignment;
    using ::css::style::VerticalAlignment_MIDDLE;
    using ::css::style::VerticalAlignment_BOTTOM;


    //= CachedSortIndicator

    class CachedSortIndicator
    {
    public:
        CachedSortIndicator()
            : m_lastHeaderHeight( 0 )
            , m_lastArrowColor( COL_TRANSPARENT )
        {
        }

        BitmapEx const & getBitmapFor(vcl::RenderContext const & i_device, long const i_headerHeight,
                                      StyleSettings const & i_style, bool const i_sortAscending);

    private:
        long m_lastHeaderHeight;
        Color m_lastArrowColor;
        BitmapEx m_sortAscending;
        BitmapEx m_sortDescending;
    };

    BitmapEx const & CachedSortIndicator::getBitmapFor(vcl::RenderContext const& i_device, long const i_headerHeight,
        StyleSettings const & i_style, bool const i_sortAscending )
    {
        BitmapEx& rBitmap(i_sortAscending ? m_sortAscending : m_sortDescending);
        if (!rBitmap || (i_headerHeight != m_lastHeaderHeight) || (i_style.GetActiveColor() != m_lastArrowColor))
        {
            long const nSortIndicatorWidth = 2 * i_headerHeight / 3;
            long const nSortIndicatorHeight = 2 * nSortIndicatorWidth / 3;

            Point const aBitmapPos( 0, 0 );
            Size const aBitmapSize( nSortIndicatorWidth, nSortIndicatorHeight );
            ScopedVclPtrInstance< VirtualDevice > aDevice(i_device, DeviceFormat::DEFAULT,
                                                          DeviceFormat::DEFAULT);
            aDevice->SetOutputSizePixel( aBitmapSize );

            DecorationView aDecoView(aDevice.get());
            aDecoView.DrawSymbol(tools::Rectangle(aBitmapPos, aBitmapSize),
                                 i_sortAscending ? SymbolType::SPIN_UP : SymbolType::SPIN_DOWN,
                                 i_style.GetActiveColor());

            rBitmap = aDevice->GetBitmapEx(aBitmapPos, aBitmapSize);
            m_lastHeaderHeight = i_headerHeight;
            m_lastArrowColor = i_style.GetActiveColor();
        }
        return rBitmap;
    }


    //= GridTableRenderer_Impl

    struct GridTableRenderer_Impl
    {
        ITableModel&        rModel;
        RowPos              nCurrentRow;
        bool                bUseGridLines;
        CachedSortIndicator aSortIndicator;
        CellValueConversion aStringConverter;

        explicit GridTableRenderer_Impl( ITableModel& _rModel )
            : rModel( _rModel )
            , nCurrentRow( ROW_INVALID )
            , bUseGridLines( true )
            , aSortIndicator( )
            , aStringConverter()
        {
        }
    };


    //= helper

    namespace
    {
        tools::Rectangle lcl_getContentArea( GridTableRenderer_Impl const & i_impl, tools::Rectangle const & i_cellArea )
        {
            tools::Rectangle aContentArea( i_cellArea );
            if ( i_impl.bUseGridLines )
            {
                aContentArea.AdjustRight( -1 );
                aContentArea.AdjustBottom( -1 );
            }
            return aContentArea;
        }
        tools::Rectangle lcl_getTextRenderingArea( tools::Rectangle const & i_contentArea )
        {
            tools::Rectangle aTextArea( i_contentArea );
            aTextArea.AdjustLeft(2 ); aTextArea.AdjustRight( -2 );
            aTextArea.AdjustTop( 1 ); aTextArea.AdjustBottom( -1 );
            return aTextArea;
        }

        DrawTextFlags lcl_getAlignmentTextDrawFlags( GridTableRenderer_Impl const & i_impl, ColPos const i_columnPos )
        {
            DrawTextFlags nVertFlag = DrawTextFlags::Top;
            VerticalAlignment const eVertAlign = i_impl.rModel.getVerticalAlign();
            switch ( eVertAlign )
            {
            case VerticalAlignment_MIDDLE:  nVertFlag = DrawTextFlags::VCenter;  break;
            case VerticalAlignment_BOTTOM:  nVertFlag = DrawTextFlags::Bottom;   break;
            default:
                break;
            }

            DrawTextFlags nHorzFlag = DrawTextFlags::Left;
            HorizontalAlignment const eHorzAlign = i_impl.rModel.getColumnCount() > 0
                                                ?  i_impl.rModel.getColumnModel( i_columnPos )->getHorizontalAlign()
                                                :  HorizontalAlignment_CENTER;
            switch ( eHorzAlign )
            {
            case HorizontalAlignment_CENTER:    nHorzFlag = DrawTextFlags::Center;   break;
            case HorizontalAlignment_RIGHT:     nHorzFlag = DrawTextFlags::Right;    break;
            default:
                break;
            }

            return nVertFlag | nHorzFlag;
        }

    }


    //= GridTableRenderer


    GridTableRenderer::GridTableRenderer( ITableModel& _rModel )
        :m_pImpl( new GridTableRenderer_Impl( _rModel ) )
    {
    }


    GridTableRenderer::~GridTableRenderer()
    {
    }


    bool GridTableRenderer::useGridLines() const
    {
        return m_pImpl->bUseGridLines;
    }


    void GridTableRenderer::useGridLines( bool const i_use )
    {
        m_pImpl->bUseGridLines = i_use;
    }


    namespace
    {
        Color lcl_getEffectiveColor(boost::optional<Color> const& i_modelColor,
                                    StyleSettings const& i_styleSettings,
                                    Color const& (StyleSettings::*i_getDefaultColor) () const)
        {
            if (!!i_modelColor)
                return *i_modelColor;
            return (i_styleSettings.*i_getDefaultColor)();
        }
    }


    void GridTableRenderer::PaintHeaderArea(vcl::RenderContext& rRenderContext, const tools::Rectangle& _rArea,
                                            bool _bIsColHeaderArea, bool _bIsRowHeaderArea, const StyleSettings& _rStyle)
    {
        OSL_PRECOND(_bIsColHeaderArea || _bIsRowHeaderArea, "GridTableRenderer::PaintHeaderArea: invalid area flags!");

        rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);

        Color const background = lcl_getEffectiveColor(m_pImpl->rModel.getHeaderBackgroundColor(),
                                                       _rStyle, &StyleSettings::GetDialogColor);
        rRenderContext.SetFillColor(background);

        rRenderContext.SetLineColor();
        rRenderContext.DrawRect(_rArea);

        // delimiter lines at bottom/right
        boost::optional<Color> aLineColor(m_pImpl->rModel.getLineColor());
        Color const lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;
        rRenderContext.SetLineColor(lineColor);
        rRenderContext.DrawLine(_rArea.BottomLeft(), _rArea.BottomRight());
        rRenderContext.DrawLine(_rArea.BottomRight(), _rArea.TopRight());

        rRenderContext.Pop();
    }


    void GridTableRenderer::PaintColumnHeader(
        ColPos _nCol,
        bool, // _bActive: no special painting for the active column at the moment
        vcl::RenderContext& rRenderContext,
        const tools::Rectangle& _rArea, const StyleSettings& _rStyle)
    {
        rRenderContext.Push(PushFlags::LINECOLOR);

        OUString sHeaderText;
        PColumnModel const pColumn = m_pImpl->rModel.getColumnModel( _nCol );
        DBG_ASSERT( pColumn, "GridTableRenderer::PaintColumnHeader: invalid column model object!" );
        if ( pColumn )
            sHeaderText = pColumn->getName();

        Color const textColor = lcl_getEffectiveColor( m_pImpl->rModel.getTextColor(), _rStyle, &StyleSettings::GetFieldTextColor );
        rRenderContext.SetTextColor(textColor);

        tools::Rectangle const aTextRect( lcl_getTextRenderingArea( lcl_getContentArea( *m_pImpl, _rArea ) ) );
        DrawTextFlags nDrawTextFlags = lcl_getAlignmentTextDrawFlags( *m_pImpl, _nCol ) | DrawTextFlags::Clip;
        if (!m_pImpl->rModel.isEnabled())
            nDrawTextFlags |= DrawTextFlags::Disable;
        rRenderContext.DrawText( aTextRect, sHeaderText, nDrawTextFlags );

        boost::optional<Color> const aLineColor( m_pImpl->rModel.getLineColor() );
        Color const lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;
        rRenderContext.SetLineColor( lineColor );
        rRenderContext.DrawLine( _rArea.BottomRight(), _rArea.TopRight());
        rRenderContext.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );

        // draw sort indicator if the model data is sorted by the given column
        ITableDataSort const * pSortAdapter = m_pImpl->rModel.getSortAdapter();
        ColumnSort aCurrentSortOrder;
        if ( pSortAdapter != nullptr )
            aCurrentSortOrder = pSortAdapter->getCurrentSortOrder();
        if ( aCurrentSortOrder.nColumnPos == _nCol )
        {
            long const nHeaderHeight( _rArea.GetHeight() );
            BitmapEx const aIndicatorBitmap = m_pImpl->aSortIndicator.getBitmapFor(rRenderContext, nHeaderHeight, _rStyle,
                                                                                   aCurrentSortOrder.eSortDirection == ColumnSortAscending);
            Size const aBitmapSize( aIndicatorBitmap.GetSizePixel() );
            long const nSortIndicatorPaddingX = 2;
            long const nSortIndicatorPaddingY = ( nHeaderHeight - aBitmapSize.Height() ) / 2;

            if ( nDrawTextFlags & DrawTextFlags::Right )
            {
                // text is right aligned => draw the sort indicator at the left hand side
                rRenderContext.DrawBitmapEx(Point(_rArea.Left() + nSortIndicatorPaddingX, _rArea.Top() + nSortIndicatorPaddingY),
                                            aIndicatorBitmap);
            }
            else
            {
                // text is left-aligned or centered => draw the sort indicator at the right hand side
                rRenderContext.DrawBitmapEx(Point(_rArea.Right() - nSortIndicatorPaddingX - aBitmapSize.Width(), nSortIndicatorPaddingY),
                                            aIndicatorBitmap);
            }
        }

        rRenderContext.Pop();
    }


    void GridTableRenderer::PrepareRow(RowPos _nRow, bool i_hasControlFocus, bool _bSelected, vcl::RenderContext& rRenderContext,
                                       const tools::Rectangle& _rRowArea, const StyleSettings& _rStyle)
    {
        // remember the row for subsequent calls to the other ->ITableRenderer methods
        m_pImpl->nCurrentRow = _nRow;

        rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);

        Color backgroundColor = _rStyle.GetFieldColor();

        boost::optional<Color> const aLineColor( m_pImpl->rModel.getLineColor() );
        Color lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;

        Color const activeSelectionBackColor = lcl_getEffectiveColor(m_pImpl->rModel.getActiveSelectionBackColor(),
                                                                     _rStyle, &StyleSettings::GetHighlightColor);
        if (_bSelected)
        {
            // selected rows use the background color from the style
            backgroundColor = i_hasControlFocus
                ? activeSelectionBackColor
                : lcl_getEffectiveColor(m_pImpl->rModel.getInactiveSelectionBackColor(), _rStyle, &StyleSettings::GetDeactiveColor);
            if (!aLineColor)
                lineColor = backgroundColor;
        }
        else
        {
            boost::optional< std::vector<Color> > aRowColors = m_pImpl->rModel.getRowBackgroundColors();
            if (!aRowColors)
            {
                // use alternating default colors
                Color const fieldColor = _rStyle.GetFieldColor();
                if (_rStyle.GetHighContrastMode() || ((m_pImpl->nCurrentRow % 2) == 0))
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
                if (aRowColors->empty())
                {
                    // all colors have the same background color
                    backgroundColor = _rStyle.GetFieldColor();
                }
                else
                {
                    backgroundColor = aRowColors->at(m_pImpl->nCurrentRow % aRowColors->size());
                }
            }
        }

        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(backgroundColor);
        rRenderContext.DrawRect(_rRowArea);

        rRenderContext.Pop();
    }


    void GridTableRenderer::PaintRowHeader(bool /*i_hasControlFocus*/, bool /*_bSelected*/, vcl::RenderContext& rRenderContext,
                                           const tools::Rectangle& _rArea, const StyleSettings& _rStyle)
    {
        rRenderContext.Push( PushFlags::LINECOLOR | PushFlags::TEXTCOLOR );

        boost::optional<Color> const aLineColor( m_pImpl->rModel.getLineColor() );
        Color const lineColor = !aLineColor ? _rStyle.GetSeparatorColor() : *aLineColor;
        rRenderContext.SetLineColor(lineColor);
        rRenderContext.DrawLine(_rArea.BottomLeft(), _rArea.BottomRight());

        Any const rowHeading( m_pImpl->rModel.getRowHeading( m_pImpl->nCurrentRow ) );
        OUString const rowTitle( m_pImpl->aStringConverter.convertToString( rowHeading ) );
        if (!rowTitle.isEmpty())
        {
            Color const textColor = lcl_getEffectiveColor(m_pImpl->rModel.getHeaderTextColor(),
                                                          _rStyle, &StyleSettings::GetFieldTextColor);
            rRenderContext.SetTextColor(textColor);

            tools::Rectangle const aTextRect(lcl_getTextRenderingArea(lcl_getContentArea(*m_pImpl, _rArea)));
            DrawTextFlags nDrawTextFlags = lcl_getAlignmentTextDrawFlags(*m_pImpl, 0) | DrawTextFlags::Clip;
            if (!m_pImpl->rModel.isEnabled())
                nDrawTextFlags |= DrawTextFlags::Disable;
                // TODO: is using the horizontal alignment of the 0'th column a good idea here? This is pretty ... arbitrary ..
            rRenderContext.DrawText(aTextRect, rowTitle, nDrawTextFlags);
        }

        rRenderContext.Pop();
    }


    struct GridTableRenderer::CellRenderContext
    {
        OutputDevice&           rDevice;
        tools::Rectangle const         aContentArea;
        StyleSettings const &   rStyle;
        ColPos const            nColumn;
        bool const              bSelected;
        bool const              bHasControlFocus;

        CellRenderContext( OutputDevice& i_device, tools::Rectangle const & i_contentArea,
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


    void GridTableRenderer::PaintCell(ColPos const i_column, bool _bSelected, bool i_hasControlFocus,
                                      vcl::RenderContext& rRenderContext, const tools::Rectangle& _rArea, const StyleSettings& _rStyle)
    {
        rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);

        tools::Rectangle const aContentArea(lcl_getContentArea(*m_pImpl, _rArea));
        CellRenderContext const aCellRenderContext(rRenderContext, aContentArea, _rStyle, i_column, _bSelected, i_hasControlFocus);
        impl_paintCellContent(aCellRenderContext);

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

            rRenderContext.SetLineColor( lineColor );
            rRenderContext.DrawLine( _rArea.BottomLeft(), _rArea.BottomRight() );
            rRenderContext.DrawLine( _rArea.BottomRight(), _rArea.TopRight() );
        }

        rRenderContext.Pop();
    }


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
                imagePos.AdjustX(( i_context.aContentArea.GetWidth() - imageSize.Width() ) / 2 );
                break;
            case HorizontalAlignment_RIGHT:
                imagePos.setX( i_context.aContentArea.Right() - imageSize.Width() );
                break;
            default:
                break;
            }

        }
        else
            imageSize.setWidth( i_context.aContentArea.GetWidth() );

        if ( i_context.aContentArea.GetHeight() > imageSize.Height() )
        {
            const VerticalAlignment eVertAlign = m_pImpl->rModel.getVerticalAlign();
            switch ( eVertAlign )
            {
            case VerticalAlignment_MIDDLE:
                imagePos.AdjustY(( i_context.aContentArea.GetHeight() - imageSize.Height() ) / 2 );
                break;
            case VerticalAlignment_BOTTOM:
                imagePos.setY( i_context.aContentArea.Bottom() - imageSize.Height() );
                break;
            default:
                break;
            }
        }
        else
            imageSize.setHeight( i_context.aContentArea.GetHeight() - 1 );
        DrawImageFlags const nStyle = m_pImpl->rModel.isEnabled() ? DrawImageFlags::NONE : DrawImageFlags::Disable;
        i_context.rDevice.DrawImage( imagePos, imageSize, i_image, nStyle );
    }


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

        const OUString sText( m_pImpl->aStringConverter.convertToString( aCellContent ) );
        impl_paintCellText( i_context, sText );
    }


    void GridTableRenderer::impl_paintCellText( CellRenderContext const & i_context, OUString const & i_text )
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

        tools::Rectangle const textRect( lcl_getTextRenderingArea( i_context.aContentArea ) );
        DrawTextFlags nDrawTextFlags = lcl_getAlignmentTextDrawFlags( *m_pImpl, i_context.nColumn ) | DrawTextFlags::Clip;
        if ( !m_pImpl->rModel.isEnabled() )
            nDrawTextFlags |= DrawTextFlags::Disable;
        i_context.rDevice.DrawText( textRect, i_text, nDrawTextFlags );
    }


    void GridTableRenderer::ShowCellCursor( vcl::Window& _rView, const tools::Rectangle& _rCursorRect)
    {
        _rView.ShowFocus( _rCursorRect );
    }


    void GridTableRenderer::HideCellCursor( vcl::Window& _rView, const tools::Rectangle&)
    {
        _rView.HideFocus();
    }


    bool GridTableRenderer::FitsIntoCell( Any const & i_cellContent,
        OutputDevice& i_targetDevice, tools::Rectangle const & i_targetArea ) const
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

        OUString const sText( m_pImpl->aStringConverter.convertToString( i_cellContent ) );
        if ( sText.isEmpty() )
            return true;

        tools::Rectangle const aTargetArea( lcl_getTextRenderingArea( lcl_getContentArea( *m_pImpl, i_targetArea ) ) );

        long const nTextHeight = i_targetDevice.GetTextHeight();
        if ( nTextHeight > aTargetArea.GetHeight() )
            return false;

        long const nTextWidth = i_targetDevice.GetTextWidth( sText );
        return nTextWidth <= aTargetArea.GetWidth();
    }


    bool GridTableRenderer::GetFormattedCellString( Any const & i_cellValue, OUString & o_cellString ) const
    {
        o_cellString = m_pImpl->aStringConverter.convertToString( i_cellValue );

        return true;
    }


} } // namespace svt::table


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
