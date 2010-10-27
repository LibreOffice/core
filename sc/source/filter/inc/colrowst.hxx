/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef SC_COLROWST_HXX
#define SC_COLROWST_HXX

#include "xiroot.hxx"
#include <mdds/flat_segment_tree.hpp>

#define XLS_USE_NEW_ROW_CONT 1


class XclImpStream;

// ============================================================================

class XclImpColRowSettings : protected XclImpRoot
{
public:
    explicit            XclImpColRowSettings( const XclImpRoot& rRoot );
    virtual             ~XclImpColRowSettings();

    void                SetDefWidth( sal_uInt16 nDefWidth, bool bStdWidthRec = false );
    void                SetWidthRange( SCCOL nCol1, SCCOL nCol2, sal_uInt16 nWidth );
    void                HideCol( SCCOL nCol );
    void                HideColRange( SCCOL nCol1, SCCOL nCol2 );

    void                SetDefHeight( sal_uInt16 nDefHeight, sal_uInt16 nFlags );
    void                SetHeight( SCROW nRow, sal_uInt16 nHeight );
    void                SetRowSettings( SCROW nRow, sal_uInt16 nHeight, sal_uInt16 nFlags );
    void                SetManualRowHeight( SCROW nScRow );

    void                SetDefaultXF( SCCOL nScCol1, SCCOL nScCol2, sal_uInt16 nXFIndex );
    /** Inserts all column and row settings of the specified sheet, except the hidden flags. */
    void                Convert( SCTAB nScTab );
    /** Sets the HIDDEN flags at all hidden columns and rows in the specified sheet. */
    void                ConvertHiddenFlags( SCTAB nScTab );

private:
    ScfUInt16Vec        maWidths;           /// Column widths in twips.
    ScfUInt8Vec         maColFlags;         /// Flags for all columns.
                                            ///
    typedef ::mdds::flat_segment_tree<SCROW, sal_uInt16> RowHeightsType;
    typedef ::mdds::flat_segment_tree<SCROW, sal_uInt8>  RowFlagsType;
    typedef ::mdds::flat_segment_tree<SCROW, bool>       RowHiddenType;
    RowHeightsType      maRowHeights;
    RowFlagsType        maRowFlags;
    RowHiddenType       maHiddenRows;

    SCROW               mnLastScRow;

    sal_uInt16          mnDefWidth;         /// Default width from DEFCOLWIDTH or STANDARDWIDTH record.
    sal_uInt16          mnDefHeight;        /// Default height from DEFAULTROWHEIGHT record.
    sal_uInt16          mnDefRowFlags;      /// Default row flags from DEFAULTROWHEIGHT record.

    bool                mbHasStdWidthRec;   /// true = Width from STANDARDWIDTH (overrides DEFCOLWIDTH record).
    bool                mbHasDefHeight;     /// true = mnDefHeight and mnDefRowFlags are valid.
    bool                mbDirty;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
