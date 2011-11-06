/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_COLROWST_HXX
#define SC_COLROWST_HXX

#include "xiroot.hxx"

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
    ScfUInt16Vec        maHeights;          /// Row heights in twips.
    ScfUInt8Vec         maRowFlags;         /// Flags for all rows.

    const SCCOL         mnMaxCol;
    const SCROW         mnMaxRow;

    SCROW               mnLastScRow;

    sal_uInt16          mnDefWidth;         /// Default width from DEFCOLWIDTH or STANDARDWIDTH record.
    sal_uInt16          mnDefHeight;        /// Default height from DEFAULTROWHEIGHT record.
    sal_uInt16          mnDefRowFlags;      /// Default row flags from DEFAULTROWHEIGHT record.

    bool                mbHasStdWidthRec;   /// true = Width from STANDARDWIDTH (overrides DEFCOLWIDTH record).
    bool                mbHasDefHeight;     /// true = mnDefHeight and mnDefRowFlags are valid.
    bool                mbDirty;
};




#endif

