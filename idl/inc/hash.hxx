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
    sal_uInt32       nMax;                 // size of hash-tabel
    sal_uInt32       nFill;                // elements in hash-tabel
    sal_uInt32       lAsk;                 // number of requests
    sal_uInt32       lTry;                 // number of tries
protected:
    sal_Bool        Test_Insert( const rtl::OString&, sal_Bool bInsert, sal_uInt32 * pInsertPos );

                            // compare element with entry
    virtual bool equals( const rtl::OString& , sal_uInt32 ) const = 0;
                            // get hash value from subclass
    virtual sal_uInt32          HashFunc( const rtl::OString& ) const = 0;
public:
                SvHashTable( sal_uInt32 nMaxEntries );
                virtual ~SvHashTable();

    sal_uInt32      GetMax() const { return nMax; }

    virtual sal_Bool            IsEntry( sal_uInt32 ) const = 0;
};

class SvStringHashTable;
class SvStringHashEntry : public SvRefBase
{
friend class SvStringHashTable;
    rtl::OString aName;
    sal_uInt32  nHashId;
    sal_uLong   nValue;
    sal_Bool    bHasId;
public:
                    SvStringHashEntry() : bHasId( sal_False ) {;}
                    SvStringHashEntry( const rtl::OString& rName, sal_uInt32 nIdx )
                        : aName( rName )
                        , nHashId( nIdx )
                        , nValue( 0 )
                        , bHasId( sal_True ) {}
                    ~SvStringHashEntry();

    const rtl::OString& GetName() const { return aName; }
    sal_Bool            HasId() const { return bHasId; }
    sal_uInt32          GetId() const { return nHashId; }

    void            SetValue( sal_uLong n ) { nValue = n; }
    sal_uLong           GetValue() const { return nValue; }

    sal_Bool            operator == ( const SvStringHashEntry & rRef )
                    { return nHashId == rRef.nHashId; }
    sal_Bool            operator != ( const SvStringHashEntry & rRef )
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
    virtual sal_uInt32          HashFunc( const rtl::OString& rElement ) const;
    virtual bool equals( const rtl::OString &rElement, sal_uInt32 nIndex ) const;
public:
            SvStringHashTable( sal_uInt32 nMaxEntries );   // max size of hash-tabel
            virtual ~SvStringHashTable();

    rtl::OString GetNearString( const rtl::OString& rName ) const;
    virtual sal_Bool    IsEntry( sal_uInt32 nIndex ) const;

    sal_Bool    Insert( const rtl::OString& rStr, sal_uInt32 * pHash ); // insert string
    sal_Bool    Test( const rtl::OString& rStr, sal_uInt32 * pHash ) const; // test of insert string
    SvStringHashEntry * Get ( sal_uInt32 nIndex ) const; // return pointer to string
    SvStringHashEntry & operator []( sal_uInt32 nPos ) const
            { return pEntries[ nPos ]; }

    void    FillHashList( SvStringHashList * rList ) const;
};

#endif // _RSCHASH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
