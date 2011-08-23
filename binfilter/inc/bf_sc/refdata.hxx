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

#ifndef SC_REFDATA_HXX
#define SC_REFDATA_HXX

#include "global.hxx"
namespace binfilter {


// Ref-Flags for old (until release 3.1) documents

struct OldSingleRefBools
{
    BYTE    bRelCol;	// Flag values (see further down), 2 bits each in file format
    BYTE    bRelRow;
    BYTE    bRelTab;
    BYTE	bOldFlag3D;	// two BOOL flags (see further down)
};

#define	SR_ABSOLUTE	0		// Absolute value
#define	SR_RELABS	1		// Relative value as absolute value (until release 3.1)
#define	SR_RELATIVE	2		// Relative value as delta value (after release 3.1)
#define	SR_DELETED	3		// Deleted col/row/tab

#define SRF_3D		0x01	// 3D reference, was the BOOL (before build 304a)
#define SRF_RELNAME 0x02	// Reference derived from RangeName with relative values
#define SRF_BITS	0x03	// Mask of possible bits


struct SingleRefData		// Single reference (one address) into the sheet
{
    INT16	nCol;		// Absolute values
    INT16	nRow;
    INT16	nTab;
    INT16	nRelCol;	// Values relative to the position
    INT16	nRelRow;
    INT16	nRelTab;

    union
    {
        BOOL bFlags;
        struct
        {
            BOOL	bColRel		:1;
            BOOL	bColDeleted	:1;
            BOOL	bRowRel		:1;
            BOOL	bRowDeleted	:1;
            BOOL	bTabRel		:1;
            BOOL	bTabDeleted	:1;
            BOOL	bFlag3D		:1;		// 3D-Ref
            BOOL	bRelName	:1;		// Reference derived from RangeName with relative values
        }Flags;
    };

    // No default ctor, because used in ScRawToken union, set InitFlags!
    inline	void InitFlags() { bFlags = 0; }	// all FALSE
    // InitAddress: InitFlags and set address
    inline	void InitAddress( const ScAddress& rAdr );
    // InitAddressRel: InitFlags and set address, everything relative to rPos
    inline	void InitAddressRel( const ScAddress& rAdr, const ScAddress& rPos );
    inline	void SetColRel( BOOL bVal )	{ Flags.bColRel = (bVal ? TRUE : FALSE ); }
    inline	BOOL IsColRel() const		{ return Flags.bColRel; }
    inline	void SetRowRel( BOOL bVal )	{ Flags.bRowRel = (bVal ? TRUE : FALSE ); }
    inline	BOOL IsRowRel() const		{ return Flags.bRowRel; }
    inline	void SetTabRel( BOOL bVal )	{ Flags.bTabRel = (bVal ? TRUE : FALSE ); }
    inline	BOOL IsTabRel() const		{ return Flags.bTabRel; }

    inline	void SetColDeleted( BOOL bVal )	{ Flags.bColDeleted = (bVal ? TRUE : FALSE ); }
    inline	BOOL IsColDeleted() const		{ return Flags.bColDeleted; }
    inline	void SetRowDeleted( BOOL bVal ) { Flags.bRowDeleted = (bVal ? TRUE : FALSE ); }
    inline	BOOL IsRowDeleted() const		{ return Flags.bRowDeleted; }
    inline	void SetTabDeleted( BOOL bVal ) { Flags.bTabDeleted = (bVal ? TRUE : FALSE ); }
    inline	BOOL IsTabDeleted() const		{ return Flags.bTabDeleted; }
    inline  BOOL IsDeleted() const          { return IsColDeleted() || IsRowDeleted() || IsTabDeleted(); }

    inline	void SetFlag3D( BOOL bVal )	{ Flags.bFlag3D = (bVal ? TRUE : FALSE ); }
    inline	BOOL IsFlag3D() const		{ return Flags.bFlag3D; }
    inline	void SetRelName( BOOL bVal )	{ Flags.bRelName = (bVal ? TRUE : FALSE ); }
    inline	BOOL IsRelName() const			{ return Flags.bRelName; }

    inline	BOOL Valid() const;

            void SmartRelAbs( const ScAddress& rPos );
            void CalcRelFromAbs( const ScAddress& rPos );
            void CalcAbsIfRel( const ScAddress& rPos );
            void OldBoolsToNewFlags( const OldSingleRefBools& );
            BYTE CreateStoreByteFromFlags() const;
            void CreateFlagsFromLoadByte( BYTE );
             BOOL operator==( const SingleRefData& ) const;
};

inline void SingleRefData::InitAddress( const ScAddress& rAdr )
{
    InitFlags();
    nCol = rAdr.Col();
    nRow = rAdr.Row();
    nTab = rAdr.Tab();
}

inline void SingleRefData::InitAddressRel( const ScAddress& rAdr,
                                            const ScAddress& rPos )
{
    InitFlags();
    nCol = rAdr.Col();
    nRow = rAdr.Row();
    nTab = rAdr.Tab();
    SetColRel( TRUE );
    SetRowRel( TRUE );
    SetTabRel( TRUE );
    CalcRelFromAbs( rPos );
}

inline BOOL SingleRefData::Valid() const
{
    return	nCol >= 0 && nCol <= MAXCOL &&
            nRow >= 0 && nRow <= MAXROW &&
            nTab >= 0 && nTab <= MAXTAB;
}


struct ComplRefData			// Complex reference (a range) into the sheet
{
    SingleRefData Ref1;
    SingleRefData Ref2;

    inline	void InitFlags()
        { Ref1.InitFlags(); Ref2.InitFlags(); }
    inline	void InitRange( const ScRange& rRange )
        {
            Ref1.InitAddress( rRange.aStart );
            Ref2.InitAddress( rRange.aEnd );
        }
    inline	void InitRangeRel( const ScRange& rRange, const ScAddress& rPos )
        {
            Ref1.InitAddressRel( rRange.aStart, rPos );
            Ref2.InitAddressRel( rRange.aEnd, rPos );
        }
    inline	void SmartRelAbs( const ScAddress& rPos )
        { Ref1.SmartRelAbs( rPos ); Ref2.SmartRelAbs( rPos ); }
    inline	void CalcRelFromAbs( const ScAddress& rPos )
        { Ref1.CalcRelFromAbs( rPos ); Ref2.CalcRelFromAbs( rPos ); }
    inline	void CalcAbsIfRel( const ScAddress& rPos )
        { Ref1.CalcAbsIfRel( rPos ); Ref2.CalcAbsIfRel( rPos ); }
    inline	BOOL Valid() const
        { return Ref1.Valid() && Ref2.Valid(); }
            void PutInOrder();
     inline	BOOL operator==( const ComplRefData& r ) const
         { return Ref1 == r.Ref1 && Ref2 == r.Ref2; }
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
