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

        TODO update the documentation when it's decided whether this renderer
        also does value handling
    */
    class GridTableRenderer : public ITableRenderer
    {
    private:
        GridTableRenderer_Impl*     m_pImpl;

    public:
        /** creates a table renderer associated with the given model

            @param _rModel
                the model which should be rendered. The caller is responsible
                for lifetime control, that is, the model instance must live
                at least as long as the renderer instance lives
        */
        GridTableRenderer( ITableModel& _rModel );
        ~GridTableRenderer();

        /** returns the index of the row currently being painted

            According to the ->ITableRenderer interface, one call is made
            to the renderer with a row to prepare (->PrepareRow()), and subsequent
            calls do not carry the row index anymore, but are relative to the
            row which has previously been prepared.

            This method returns the index of the last row which has been prepared
        */
        RowPos  getCurrentRow();

    protected:
        // ITableRenderer overridables
        virtual void    PaintHeaderArea(
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            bool _bIsColHeaderArea, bool _bIsRowHeaderArea,
                            const StyleSettings& _rStyle );
        virtual void    PaintColumnHeader( ColPos _nCol, bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            const StyleSettings& _rStyle );
        virtual void    PrepareRow( RowPos _nRow, bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rRowArea,
                            const StyleSettings& _rStyle );
        virtual void    PaintRowHeader(
                            bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            const StyleSettings& _rStyle, rtl::OUString& _rText );
        virtual void    PaintCellImage( ColPos _nColumn,
                            bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                const StyleSettings& _rStyle, Image* _pCellData );
    virtual void    PaintCellString( ColPos _nColumn,
                            bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                const StyleSettings& _rStyle, rtl::OUString& _rText );
        virtual void    ShowCellCursor( Window& _rView, const Rectangle& _rCursorRect);
        virtual void    HideCellCursor( Window& _rView, const Rectangle& _rCursorRect);
    };
//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_GRIDTABLERENDERER_HXX
