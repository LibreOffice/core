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

#ifndef INCLUDED_SVTOOLS_INC_TABLE_GRIDTABLERENDERER_HXX
#define INCLUDED_SVTOOLS_INC_TABLE_GRIDTABLERENDERER_HXX

#include <svtools/table/tablemodel.hxx>
#include <vcl/image.hxx>

#include <memory>


namespace svt { namespace table
{


    struct GridTableRenderer_Impl;


    //= GridTableRenderer

    /** a default implementation for the ->ITableRenderer interface

        This class is able to paint a table grid, table headers, and cell
        backgrounds according to the selected/active state of cells.
    */
    class GridTableRenderer : public ITableRenderer
    {
    private:
        ::std::unique_ptr< GridTableRenderer_Impl >   m_pImpl;

    public:
        /** creates a table renderer associated with the given model

            @param _rModel
                the model which should be rendered. The caller is responsible
                for lifetime control, that is, the model instance must live
                at least as long as the renderer instance lives
        */
        GridTableRenderer( ITableModel& _rModel );
        virtual ~GridTableRenderer() override;

        /** determines whether or not to paint grid lines
        */
        bool    useGridLines() const;

        /** controls whether or not to paint grid lines
        */
        void    useGridLines( bool const i_use );

    public:
        // ITableRenderer overridables
        virtual void    PaintHeaderArea(
                            vcl::RenderContext& _rDevice, const tools::Rectangle& _rArea,
                            bool _bIsColHeaderArea, bool _bIsRowHeaderArea,
                            const StyleSettings& _rStyle ) override;
        virtual void    PaintColumnHeader( ColPos _nCol, bool _bActive,
                            vcl::RenderContext& _rDevice, const tools::Rectangle& _rArea,
                            const StyleSettings& _rStyle ) override;
        virtual void    PrepareRow( RowPos _nRow, bool i_hasControlFocus, bool _bSelected,
                            vcl::RenderContext& _rDevice, const tools::Rectangle& _rRowArea,
                            const StyleSettings& _rStyle ) override;
        virtual void    PaintRowHeader(
                            bool i_hasControlFocus, bool _bSelected,
                            vcl::RenderContext& _rDevice, const tools::Rectangle& _rArea,
                            const StyleSettings& _rStyle ) override;
        virtual void    PaintCell( ColPos const i_col,
                            bool i_hasControlFocus, bool _bSelected,
                            vcl::RenderContext& _rDevice, const tools::Rectangle& _rArea,
                            const StyleSettings& _rStyle ) override;
        virtual void    ShowCellCursor( vcl::Window& _rView, const tools::Rectangle& _rCursorRect) override;
        virtual void    HideCellCursor( vcl::Window& _rView, const tools::Rectangle& _rCursorRect) override;
        virtual bool    FitsIntoCell(
                            css::uno::Any const & i_cellContent,
                            OutputDevice& i_targetDevice, tools::Rectangle const & i_targetArea
                        ) const override;
        virtual bool    GetFormattedCellString(
                            css::uno::Any const & i_cellValue,
                            OUString & o_cellString
                        ) const override;

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
                        OUString const & i_text
                   );
    };

} } // namespace svt::table


#endif // INCLUDED_SVTOOLS_INC_TABLE_GRIDTABLERENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
