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

#ifndef _HASH_HXX
#define _HASH_HXX

#include <rtl/ustring.hxx>
#include <tools/ref.hxx>
#include <tools/solar.h>
#include <vector>

class SvHashTable
{
    sal_uInt32       nMax;                 // size of hash-tabel
    sal_uInt32       nFill;                // elements in hash-tabel
    sal_uInt32       lAsk;                 // number of requests
    sal_uInt32       lTry;                 // number of tries
protected:
    sal_Bool        Test_Insert( const OString&, sal_Bool bInsert, sal_uInt32 * pInsertPos );

                            // compare element with entry
    virtual bool equals( const OString& , sal_uInt32 ) const = 0;
                            // get hash value from subclass
    virtual sal_uInt32          HashFunc( const OString& ) const = 0;
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
    OString aName;
    sal_uInt32  nHashId;
    sal_uLong   nValue;
    sal_Bool    bHasId;
public:
    SvStringHashEntry()
        : nHashId(0)
        , nValue(0)
        , bHasId(false)
    {
    }
    SvStringHashEntry( const OString& rName, sal_uInt32 nIdx )
        : aName(rName)
        , nHashId(nIdx)
        , nValue(0)
        , bHasId(true)
    {
    }
    virtual ~SvStringHashEntry();

    const OString&  GetName() const { return aName; }
    sal_Bool        HasId() const { return bHasId; }
    sal_uInt32      GetId() const { return nHashId; }

    void            SetValue( sal_uLong n ) { nValue = n; }
    sal_uLong           GetValue() const { return nValue; }

    sal_Bool        operator == ( const SvStringHashEntry & rRef )
                    { return nHashId == rRef.nHashId; }
    sal_Bool        operator != ( const SvStringHashEntry & rRef )
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

typedef tools::SvRef<SvStringHashEntry> SvStringHashEntryRef;

typedef ::std::vector< SvStringHashEntry* > SvStringHashList;

class SvStringHashTable : public SvHashTable
{
    SvStringHashEntry*      pEntries;
protected:
    virtual sal_uInt32          HashFunc( const OString& rElement ) const SAL_OVERRIDE;
    virtual bool equals( const OString &rElement, sal_uInt32 nIndex ) const SAL_OVERRIDE;
public:
            SvStringHashTable( sal_uInt32 nMaxEntries );   // max size of hash-tabel
            virtual ~SvStringHashTable();

    OString GetNearString( const OString& rName ) const;
    virtual sal_Bool    IsEntry( sal_uInt32 nIndex ) const SAL_OVERRIDE;

    sal_Bool    Insert( const OString& rStr, sal_uInt32 * pHash ); // insert string
    sal_Bool    Test( const OString& rStr, sal_uInt32 * pHash ) const; // test of insert string
    SvStringHashEntry * Get ( sal_uInt32 nIndex ) const; // return pointer to string
    SvStringHashEntry & operator []( sal_uInt32 nPos ) const
            { return pEntries[ nPos ]; }

    void    FillHashList( SvStringHashList * rList ) const;
};

#endif // _RSCHASH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
