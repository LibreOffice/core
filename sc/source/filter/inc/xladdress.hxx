/*************************************************************************
 *
 *  $RCSfile: xladdress.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-29 11:49:34 $
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
#ifndef SC_XLADDRESS_HXX
#define SC_XLADDRESS_HXX

#include <vector>

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

class ScRangeList;
class XclImpStream;
class XclExpStream;

// ============================================================================

/** A 2D cell address struct with Excel column and row indexes. */
struct XclAddress
{
    sal_uInt16          mnCol;
    sal_uInt16          mnRow;

    inline explicit     XclAddress( ScAddress::Uninitialized ) {}
    inline explicit     XclAddress() : mnCol( 0 ), mnRow( 0 ) {}
    inline explicit     XclAddress( sal_uInt16 nCol, sal_uInt16 nRow ) : mnCol( nCol ), mnRow( nRow ) {}

    inline void         Set( sal_uInt16 nCol, sal_uInt16 nRow ) { mnCol = nCol; mnRow = nRow; }

    void                Read( XclImpStream& rStrm, bool bCol16Bit = true );
    void                Write( XclExpStream& rStrm, bool bCol16Bit = true ) const;
};

inline bool operator==( const XclAddress& rL, const XclAddress& rR )
{
    return (rL.mnCol == rR.mnCol) && (rL.mnRow == rR.mnRow);
}

inline bool operator<( const XclAddress& rL, const XclAddress& rR )
{
    return (rL.mnCol < rR.mnCol) || ((rL.mnCol == rR.mnCol) && (rL.mnRow < rR.mnRow));
}

inline XclImpStream& operator>>( XclImpStream& rStrm, XclAddress& rXclPos )
{
    rXclPos.Read( rStrm );
    return rStrm;
}

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclAddress& rXclPos )
{
    rXclPos.Write( rStrm );
    return rStrm;
}

// ----------------------------------------------------------------------------

/** A 2D cell range address struct with Excel column and row indexes. */
struct XclRange
{
    XclAddress          maFirst;
    XclAddress          maLast;

    inline explicit     XclRange( ScAddress::Uninitialized e ) : maFirst( e ), maLast( e ) {}
    inline explicit     XclRange() {}
    inline explicit     XclRange( const XclAddress& rPos ) : maFirst( rPos ), maLast( rPos ) {}
    inline explicit     XclRange( const XclAddress& rFirst, const XclAddress& rLast ) : maFirst( rFirst ), maLast( rLast ) {}
    inline explicit     XclRange( sal_uInt16 nCol1, sal_uInt16 nRow1, sal_uInt16 nCol2, sal_uInt16 nRow2 ) :
                            maFirst( nCol1, nRow1 ), maLast( nCol2, nRow2 ) {}

    inline void         Set( const XclAddress& rFirst, const XclAddress& rLast )
                            { maFirst = rFirst; maLast = rLast; }
    inline void         Set( sal_uInt16 nCol1, sal_uInt16 nRow1, sal_uInt16 nCol2, sal_uInt16 nRow2 )
                            { maFirst.Set( nCol1, nRow1 ); maLast.Set( nCol2, nRow2 ); }

    inline sal_uInt16   GetColCount() const { return maLast.mnCol - maFirst.mnCol + 1; }
    inline sal_uInt16   GetRowCount() const { return maLast.mnRow - maFirst.mnRow + 1; }
    bool                Contains( const XclAddress& rPos ) const;

    void                Read( XclImpStream& rStrm, bool bCol16Bit = true );
    void                Write( XclExpStream& rStrm, bool bCol16Bit = true ) const;
};

inline bool operator==( const XclRange& rL, const XclRange& rR )
{
    return (rL.maFirst == rR.maFirst) && (rL.maLast == rR.maLast);
}

inline bool operator<( const XclRange& rL, const XclRange& rR )
{
    return (rL.maFirst < rR.maFirst) || ((rL.maFirst == rR.maFirst) && (rL.maLast < rR.maLast));
}

inline XclImpStream& operator>>( XclImpStream& rStrm, XclRange& rXclRange )
{
    rXclRange.Read( rStrm );
    return rStrm;
}

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclRange& rXclRange )
{
    rXclRange.Write( rStrm );
    return rStrm;
}

// ----------------------------------------------------------------------------

/** A 2D cell range address list with Excel column and row indexes. */
class XclRangeList : public ::std::vector< XclRange >
{
public:
    inline explicit     XclRangeList() {}

    XclRange            GetEnclosingRange() const;

    void                Read( XclImpStream& rStrm, bool bCol16Bit = true );
    void                Write( XclExpStream& rStrm, bool bCol16Bit = true ) const;
    void                WriteSubList( XclExpStream& rStrm,
                            size_t nBegin, size_t nCount, bool bCol16Bit = true ) const;
};

inline XclImpStream& operator>>( XclImpStream& rStrm, XclRangeList& rXclRanges )
{
    rXclRanges.Read( rStrm );
    return rStrm;
}

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclRangeList& rXclRanges )
{
    rXclRanges.Write( rStrm );
    return rStrm;
}

// ============================================================================

class XclTracer;

/** Base class for import/export address converters. */
class XclAddressConverterBase
{
public:
    explicit            XclAddressConverterBase( XclTracer& rTracer, const ScAddress& rMaxPos );
    virtual             ~XclAddressConverterBase();

    /** Returns whether the "some columns have been cut" warning box should be shown. */
    inline bool         IsColTruncated() const { return mbColTrunc; }
    /** Returns whether the "some rows have been cut" warning box should be shown. */
    inline bool         IsRowTruncated() const { return mbRowTrunc; }
    /** Returns whether the "some sheets have been cut" warning box should be shown. */
    inline bool         IsTabTruncated() const { return mbTabTrunc; }

    // ------------------------------------------------------------------------

    /** Checks if the passed sheet index is valid.
        @param nScTab  The sheet index to check.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the sheet index is not valid.
        @return  true = Sheet index in nScTab is valid. */
    bool                CheckScTab( SCTAB nScTab, bool bWarn );

    // ------------------------------------------------------------------------
protected:
    XclTracer&          mrTracer;       /// Tracer for invalid addresses.
    ScAddress           maMaxPos;       /// Default maximum position.
    sal_uInt16          mnMaxCol;       /// Maximum column index, as 16-bit value.
    sal_uInt16          mnMaxRow;       /// Maximum row index, as 16-bit value.
    bool                mbColTrunc;     /// Flag for "columns truncated" warning box.
    bool                mbRowTrunc;     /// Flag for "rows truncated" warning box.
    bool                mbTabTrunc;     /// Flag for "tables truncated" warning box.
};

// ============================================================================

#endif

