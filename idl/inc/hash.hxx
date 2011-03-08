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

#ifndef _HASH_HXX
#define _HASH_HXX

#include <tools/ref.hxx>
#include <tools/string.hxx>
#include <vector>

class SvHashTable
{
    UINT32       nMax;                 // size of hash-tabel
    UINT32       nFill;                // elements in hash-tabel
    UINT32       lAsk;                 // number of requests
    UINT32       lTry;                 // number of tries
protected:
    BOOL        Test_Insert( const void *, BOOL bInsert, UINT32 * pInsertPos );

                            // compare element with entry
    virtual StringCompare   Compare( const void * , UINT32 ) const = 0;
                            // get hash value from subclass
    virtual UINT32          HashFunc( const void * ) const = 0;
public:
                SvHashTable( UINT32 nMaxEntries );
                virtual ~SvHashTable();

    UINT32      GetMax() const { return nMax; }

    virtual BOOL            IsEntry( UINT32 ) const = 0;
};

class SvStringHashTable;
class SvStringHashEntry : public SvRefBase
{
friend class SvStringHashTable;
    ByteString  aName;
    UINT32  nHashId;
    ULONG   nValue;
    BOOL    bHasId;
public:
                    SvStringHashEntry() : bHasId( FALSE ) {;}
                    SvStringHashEntry( const ByteString & rName, UINT32 nIdx )
                        : aName( rName )
                        , nHashId( nIdx )
                        , nValue( 0 )
                        , bHasId( TRUE ) {}
                    ~SvStringHashEntry();

    const ByteString &  GetName() const { return aName; }
    BOOL            HasId() const { return bHasId; }
    UINT32          GetId() const { return nHashId; }

    void            SetValue( ULONG n ) { nValue = n; }
    ULONG           GetValue() const { return nValue; }

    BOOL            operator == ( const SvStringHashEntry & rRef )
                    { return nHashId == rRef.nHashId; }
    BOOL            operator != ( const SvStringHashEntry & rRef )
                    { return ! operator == ( rRef ); }
    SvStringHashEntry & operator = ( const SvStringHashEntry & rRef )
        { SvRefBase::operator=( rRef );
          aName   = rRef.aName;
          nHashId = rRef.nHashId;
          nValue  = rRef.nValue;
          bHasId  = rRef.bHasId;
          return *this;
        }
};

SV_DECL_IMPL_REF(SvStringHashEntry)

typedef ::std::vector< SvStringHashEntry* > SvStringHashList;

class SvStringHashTable : public SvHashTable
{
    SvStringHashEntry*      pEntries;
protected:
    virtual UINT32          HashFunc( const void * pElement ) const;
    virtual StringCompare   Compare( const void * pElement, UINT32 nIndex ) const;
public:
            SvStringHashTable( UINT32 nMaxEntries );   // max size of hash-tabel
            virtual ~SvStringHashTable();

    ByteString          GetNearString( const ByteString & rName ) const;
    virtual BOOL    IsEntry( UINT32 nIndex ) const;

    BOOL    Insert( const ByteString & rStr, UINT32 * pHash ); // insert string
    BOOL    Test( const ByteString & rStr, UINT32 * pHash ) const; // test of insert string
    SvStringHashEntry * Get ( UINT32 nIndex ) const; // return pointer to string
    SvStringHashEntry & operator []( UINT32 nPos ) const
            { return pEntries[ nPos ]; }

    void    FillHashList( SvStringHashList * rList ) const;
};

#endif // _RSCHASH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
