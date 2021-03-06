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

#pragma once

#include "xiroot.hxx"
#include <mdds/flat_segment_tree.hpp>
#include <o3tl/typed_flags_set.hxx>

enum class ExcColRowFlags : sal_uInt8 {
    NONE         = 0x00,
    Used         = 0x01,
    Default      = 0x02,
    Hidden       = 0x04,
    Man          = 0x08,
};
namespace o3tl {
    template<> struct typed_flags<ExcColRowFlags> : is_typed_flags<ExcColRowFlags, 0x0f> {};
}


class XclImpColRowSettings : protected XclImpRoot
{
public:
    explicit            XclImpColRowSettings( const XclImpRoot& rRoot );
    virtual             ~XclImpColRowSettings() override;

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
    void ApplyColFlag(SCCOL nCol, ExcColRowFlags nNewVal);
    bool GetColFlag(SCCOL nCol, ExcColRowFlags nMask) const;

private:
    typedef ::mdds::flat_segment_tree<SCROW, sal_uInt16>      WidthHeightStoreType;
    typedef ::mdds::flat_segment_tree<SCROW, ExcColRowFlags>  ColRowFlagsType;
    typedef ::mdds::flat_segment_tree<SCROW, bool>            RowHiddenType;

    WidthHeightStoreType maColWidths;
    ColRowFlagsType      maColFlags;
    WidthHeightStoreType maRowHeights;
    ColRowFlagsType      maRowFlags;
    RowHiddenType        maHiddenRows;

    SCROW               mnLastScRow;

    sal_uInt16          mnDefWidth;         /// Default width from DEFCOLWIDTH or STANDARDWIDTH record.
    sal_uInt16          mnDefHeight;        /// Default height from DEFAULTROWHEIGHT record.
    sal_uInt16          mnDefRowFlags;      /// Default row flags from DEFAULTROWHEIGHT record.

    bool                mbHasStdWidthRec;   /// true = Width from STANDARDWIDTH (overrides DEFCOLWIDTH record).
    bool                mbHasDefHeight;     /// true = mnDefHeight and mnDefRowFlags are valid.
    bool                mbDirty;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
