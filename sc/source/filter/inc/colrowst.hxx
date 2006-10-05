/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colrowst.hxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-05 16:19:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COLROWST_HXX
#define _COLROWST_HXX

#ifndef EXC_XIROOT_HXX
#include "xiroot.hxx"
#endif

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
    void                HideRow( SCROW nRow );
    void                SetRowSettings( SCROW nRow, sal_uInt16 nHeight, sal_uInt16 nFlags );

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

    SCROW               mnLastScRow;

    sal_uInt16          mnDefWidth;         /// Default width from DEFCOLWIDTH or STANDARDWIDTH record.
    sal_uInt16          mnDefHeight;        /// Default height from DEFAULTROWHEIGHT record.
    sal_uInt16          mnDefRowFlags;      /// Default row flags from DEFAULTROWHEIGHT record.

    bool                mbHasStdWidthRec;   /// true = Width from STANDARDWIDTH (overrides DEFCOLWIDTH record).
    bool                mbHasDefHeight;     /// true = mnDefHeight and mnDefRowFlags are valid.
    bool                mbDirty;
};




#endif

