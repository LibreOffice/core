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

#ifndef SVTOOLS_INC_TABLE_GRIDTABLERENDERER_HXX
#define SVTOOLS_INC_TABLE_GRIDTABLERENDERER_HXX

#include <svtools/table/tablemodel.hxx>

#include <boost/scoped_ptr.hpp>

//........................................................................
namespace svt { namespace table
{
//........................................................................

    struct GridTableRenderer_Impl;

    //====================================================================
    //= GridTableRenderer
    //====================================================================
    /** a default implementation for the ->ITableRenderer interface

        This class is able to paint a table grid, table headers, and cell
        backgrounds according to the selected/active state of cells.
    */
    class GridTableRenderer : public ITableRenderer
    {
    private:
        ::boost::scoped_ptr< GridTableRenderer_Impl >   m_pImpl;

    public:
        /** creates a table renderer associated with the given model

            @param _rModel
                the model which should be rendered. The caller is responsible
                for lifetime control, that is, the model instance must live
                at least as long as the renderer instance lives
        */
        GridTableRenderer( ITableModel& _rModel );
        ~GridTableRenderer();

        /** determines whether or not to paint grid lines
        */
        bool    useGridLines() const;

        /** controls whether or not to paint grid lines
        */
        void    useGridLines( bool const i_use );

    public:
        // ITableRenderer overridables
        virtual void    PaintHeaderArea(
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            bool _bIsColHeaderArea, bool _bIsRowHeaderArea,
                            const StyleSettings& _rStyle );
        virtual void    PaintColumnHeader( ColPos _nCol, bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            const StyleSettings& _rStyle );
        virtual void    PrepareRow( RowPos _nRow, bool i_hasControlFocus, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rRowArea,
                            const StyleSettings& _rStyle );
        virtual void    PaintRowHeader(
                            bool i_hasControlFocus, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            const StyleSettings& _rStyle );
        virtual void    PaintCell( ColPos const i_col,
                            bool i_hasControlFocus, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            const StyleSettings& _rStyle );
        virtual void    ShowCellCursor( Window& _rView, const Rectangle& _rCursorRect);
        virtual void    HideCellCursor( Window& _rView, const Rectangle& _rCursorRect);
        virtual bool    FitsIntoCell(
                            ::com::sun::star::uno::Any const & i_cellContent,
                            ColPos const i_colPos, RowPos const i_rowPos,
                            bool const i_active, bool const i_selected,
                            OutputDevice& i_targetDevice, Rectangle const & i_targetArea
                        ) const;
        virtual bool    GetFormattedCellString(
                            ::com::sun::star::uno::Any const & i_cellValue,
                            ColPos const i_colPos, RowPos const i_rowPos,
                            ::rtl::OUString & o_cellString
                        ) const;

    private:
        struct CellRenderContext;

        void    impl_paintCellContent(
                        CellRenderContext const & i_context
                   );
        void    impl_paintCellImage(
                        CellRenderContext const & i_context,
                        Image const & i_image
                   );
        void    impl_paintCellText(
                        CellRenderContext const & i_context,
                        ::rtl::OUString const & i_text
                   );
    };
//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_GRIDTABLERENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
