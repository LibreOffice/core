/*************************************************************************
 *
 *  $RCSfile: colrowst.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-29 13:42:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COLROWST_HXX
#define _COLROWST_HXX

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _ROOT_HXX
#include "root.hxx"
#endif

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

// ----- forwards --------------------------------------------------------

class XclImpStream;

// ----------------------------------------------- class XclImpColRowSettings --

#define ROWFLAG_USED        0x01
#define ROWFLAG_DEFAULT     0x02
#define ROWFLAG_HIDDEN      0x04
#define ROWFLAG_MAN         0x08


class XclImpColRowSettings : public ExcRoot
{
private:
    // ACHTUNG: Col-/Row-Angaben in TWIPS
    INT32*              pWidth;
    BOOL*               pColHidden;
    UINT16*             pHeight;
    INT8*               pRowFlags;

    INT32               nMaxRow;    // < 0 -> no row used!
    UINT16              nDefWidth;
    UINT16              nDefHeight;
    BOOL                bDirty;     // noch nicht rausgehauen?
    BOOL                bSetByStandard;     // StandardWidth hat Vorrang vor DefColWidth!

    void                _SetRowSettings( const SCROW nRow, const UINT16 nExcelHeight, const UINT16 nGrbit );

public:
                        XclImpColRowSettings( RootData& rRootData );
                        ~XclImpColRowSettings();
    void                Reset( void );

    void                SetDefaults( UINT16 nWidth, UINT16 nHeight );
    inline void         SetDefWidth( const UINT16 nNew, const BOOL bStandardWidth = FALSE );
    inline void         SetDefHeight( const UINT16 nNew );

    inline void         Used( const SCCOL nCol, const SCROW nRow );

    inline void         HideCol( const SCCOL nCol );
    void                HideColRange( SCCOL nColFirst, SCCOL nColLast );
    void                SetWidthRange( SCCOL nF, SCCOL nL, UINT16 nNew );
    void                SetDefaultXF( SCCOL nColFirst, SCCOL nColLast, UINT16 nXF );
    inline void         SetWidth( const SCCOL nCol, const INT32 nNew );

    inline void         SetHeight( const SCROW nRow, const UINT16 nNew );
    inline void         HideRow( const SCROW nRow );
    inline void         SetRowSettings( const SCROW nRow, const UINT16 nExcelHeight, const UINT16 nGrbit );
                                    // Auswertung/Umrechung von nExcelHeight und Auswertung nGrbit

    /** Inserts all column and row settings of the specified sheet, except the hidden flags. */
    void                Apply( SCTAB nScTab );
    /** Sets the HIDDEN flags at all hidden columns and rows in the specified sheet. */
    void                SetHiddenFlags( SCTAB nScTab );
};




inline void XclImpColRowSettings::SetDefWidth( const UINT16 n, const BOOL b )
{
    if( b )
    {
        nDefWidth = n;
        bSetByStandard = TRUE;
    }
    else if( !bSetByStandard )
        nDefWidth = n;
}


inline void XclImpColRowSettings::SetDefHeight( const UINT16 n )
{
    nDefHeight = n;
}


inline void XclImpColRowSettings::SetWidth( const SCCOL nCol, const INT32 nNew )
{
    if( ValidCol(nCol) )
        pWidth[ nCol ] = nNew;
}


inline void XclImpColRowSettings::SetHeight( const SCROW nRow, const UINT16 n )
{
    if( ValidRow(nRow) )
    {
        pHeight[ nRow ] = n & 0x7FFF;

        INT8    nFlags = pRowFlags[ nRow ];
        nFlags |= ROWFLAG_USED;

        if( n & 0x8000 )
            nFlags|= ROWFLAG_DEFAULT;

        if( nRow > nMaxRow )
            nMaxRow = nRow;

        pRowFlags[ nRow ] = nFlags;
    }
}


inline void XclImpColRowSettings::HideCol( const SCCOL nCol )
{
    if( ValidCol(nCol) )
        pColHidden[ nCol ] = TRUE;
}


inline void XclImpColRowSettings::HideRow( const SCROW nRow )
{
    if( ValidRow(nRow) )
    {
        pRowFlags[ nRow ] |= ( ROWFLAG_HIDDEN | ROWFLAG_USED );

        if( nRow > nMaxRow )
            nMaxRow = nRow;
    }
}


inline void XclImpColRowSettings::Used( const SCCOL nCol, const SCROW nRow )
{
    if( ValidCol(nCol) && ValidRow(nRow) )
    {
        pRowFlags[ nRow ] |= ROWFLAG_USED;

        if( nRow > nMaxRow )
            nMaxRow = nRow;
    }
}


inline void XclImpColRowSettings::SetRowSettings( const SCROW nRow, const UINT16 nExcelHeight, const UINT16 nGrbit )
{
    if( ValidRow(nRow) )
        _SetRowSettings( nRow, nExcelHeight, nGrbit );
}




#endif

