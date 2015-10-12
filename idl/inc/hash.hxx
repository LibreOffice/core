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

#ifndef INCLUDED_IDL_INC_HASH_HXX
#define INCLUDED_IDL_INC_HASH_HXX

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
    bool                Test_Insert( const OString&, bool bInsert, sal_uInt32 * pInsertPos );

                            // compare element with entry
    virtual bool       equals( const OString& , sal_uInt32 ) const = 0;
                            // get hash value from subclass
    virtual sal_uInt32 HashFunc( const OString& ) const = 0;
public:
                SvHashTable( sal_uInt32 nMaxEntries );
                virtual ~SvHashTable();

    sal_uInt32         GetMax() const { return nMax; }

    virtual bool       IsEntry( sal_uInt32 ) const = 0;
};

class SvStringHashTable;
class SvStringHashEntry : public SvRefBase
{
friend class SvStringHashTable;
    OString     aName;
    sal_uInt32  nHashId;
    sal_uLong   nValue;
    bool        bHasId;
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
    bool            HasId() const { return bHasId; }

    void            SetValue( sal_uLong n ) { nValue = n; }
    sal_uLong       GetValue() const { return nValue; }

    bool            operator == ( const SvStringHashEntry & rRef )
                    { return nHashId == rRef.nHashId; }
    bool            operator != ( const SvStringHashEntry & rRef )
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


class SvStringHashTable : public SvHashTable
{
    SvStringHashEntry*      pEntries;
protected:
    virtual sal_uInt32          HashFunc( const OString& rElement ) const override;
    virtual bool equals( const OString &rElement, sal_uInt32 nIndex ) const override;
public:
            SvStringHashTable( sal_uInt32 nMaxEntries );   // max size of hash-tabel
            virtual ~SvStringHashTable();

    OString GetNearString( const OString& rName ) const;
    virtual bool    IsEntry( sal_uInt32 nIndex ) const override;

    bool    Insert( const OString& rStr, sal_uInt32 * pHash ); // insert string
    bool    Test( const OString& rStr, sal_uInt32 * pHash ) const; // test of insert string
    SvStringHashEntry * Get ( sal_uInt32 nIndex ) const; // return pointer to string
    SvStringHashEntry & operator []( sal_uInt32 nPos ) const
            { return pEntries[ nPos ]; }
};

#endif // INCLUDED_IDL_INC_HASH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
