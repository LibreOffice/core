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

#include <vector>
#include <address.hxx>

class XclImpStream;
class XclExpStream;

/** A 2D cell address struct with Excel column and row indexes. */
struct XclAddress
{
    sal_uInt16          mnCol;
    sal_uInt32          mnRow;

    explicit     XclAddress( ScAddress::Uninitialized ) {}
    explicit     XclAddress() : mnCol( 0 ), mnRow( 0 ) {}
    explicit     XclAddress( sal_uInt16 nCol, sal_uInt32 nRow ) : mnCol( nCol ), mnRow( nRow ) {}

    void         Set( sal_uInt16 nCol, sal_uInt32 nRow ) { mnCol = nCol; mnRow = nRow; }

    void                Read( XclImpStream& rStrm );
    void                Write( XclExpStream& rStrm ) const;
};

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

/** A 2D cell range address struct with Excel column and row indexes. */
struct XclRange
{
    XclAddress          maFirst;
    XclAddress          maLast;

    explicit     XclRange( ScAddress::Uninitialized e ) : maFirst( e ), maLast( e ) {}
    explicit     XclRange() {}
    explicit     XclRange( const XclAddress& rPos ) : maFirst( rPos ), maLast( rPos ) {}
    explicit     XclRange( sal_uInt16 nCol1, sal_uInt32 nRow1, sal_uInt16 nCol2, sal_uInt32 nRow2 ) :
                            maFirst( nCol1, nRow1 ), maLast( nCol2, nRow2 ) {}

    void         Set( sal_uInt16 nCol1, sal_uInt32 nRow1, sal_uInt16 nCol2, sal_uInt32 nRow2 )
                            { maFirst.Set( nCol1, nRow1 ); maLast.Set( nCol2, nRow2 ); }

    sal_uInt16   GetColCount() const {
        return maFirst.mnCol <= maLast.mnCol && maFirst.mnRow <= maLast.mnRow
            ? maLast.mnCol - maFirst.mnCol + 1 : 0;
    }
    sal_uInt32   GetRowCount() const {
        return maFirst.mnCol <= maLast.mnCol && maFirst.mnRow <= maLast.mnRow
            ? maLast.mnRow - maFirst.mnRow + 1 : 0;
    }
    bool                Contains( const XclAddress& rPos ) const;

    void                Read( XclImpStream& rStrm, bool bCol16Bit = true );
    void                Write( XclExpStream& rStrm, bool bCol16Bit = true ) const;
};

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

typedef ::std::vector< XclRange > XclRangeVector;

/** A 2D cell range address list with Excel column and row indexes. */
class XclRangeList
{
private:
    XclRangeVector mRanges;

public:
    explicit     XclRangeList() : mRanges() {}

    size_t size() const { return mRanges.size(); }
    bool empty() const { return mRanges.empty(); }
    XclRangeVector::const_iterator begin() const { return mRanges.begin(); }
    XclRangeVector::const_iterator end() const { return mRanges.end(); }
    void clear() { mRanges.clear(); }
    void push_back(const XclRange &rRange) { mRanges.push_back(rRange); }

    XclRange            GetEnclosingRange() const;

    void                Read( XclImpStream& rStrm, bool bCol16Bit = true, sal_uInt16 nCountInStream = 0 );
    void                Write( XclExpStream& rStrm, bool bCol16Bit = true, sal_uInt16 nCountInStream = 0 ) const;
    void                WriteSubList( XclExpStream& rStrm,
                            size_t nBegin, size_t nCount, bool bCol16Bit = true, sal_uInt16 nCountInStream = 0 ) const;
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

class XclTracer;

/** Base class for import/export address converters. */
class XclAddressConverterBase
{
public:
    explicit            XclAddressConverterBase( XclTracer& rTracer, const ScAddress& rMaxPos );
    virtual             ~XclAddressConverterBase();

    /** Returns whether the "some columns have been cut" warning box should be shown. */
    bool         IsColTruncated() const { return mbColTrunc; }
    /** Returns whether the "some rows have been cut" warning box should be shown. */
    bool         IsRowTruncated() const { return mbRowTrunc; }
    /** Returns whether the "some sheets have been cut" warning box should be shown. */
    bool         IsTabTruncated() const { return mbTabTrunc; }

    /** Checks if the passed sheet index is valid.
        @param nScTab  The sheet index to check.
        Sets the internal flag that produces a warning box
            after loading/saving the file, if the sheet index is not valid.
    */
    void                CheckScTab( SCTAB nScTab );

protected:
    XclTracer&          mrTracer;       /// Tracer for invalid addresses.
    ScAddress           maMaxPos;       /// Default maximum position.
    sal_uInt16          mnMaxCol;       /// Maximum column index, as 16-bit value.
    sal_uInt32          mnMaxRow;       /// Maximum row index.
    bool                mbColTrunc;     /// Flag for "columns truncated" warning box.
    bool                mbRowTrunc;     /// Flag for "rows truncated" warning box.
    bool                mbTabTrunc;     /// Flag for "tables truncated" warning box.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
