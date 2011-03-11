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

#ifndef SC_COLLECT_HXX
#define SC_COLLECT_HXX

#include "address.hxx"
#include <tools/string.hxx>

#ifndef INCLUDED_LIMITS_H
#include <limits.h>
#define INCLUDED_LIMITS_H
#endif
#include "scdllapi.h"

#define MAXCOLLECTIONSIZE       16384
#define MAXDELTA                1024
#define SCPOS_INVALID           USHRT_MAX

#define SC_STRTYPE_VALUE        0
#define SC_STRTYPE_STANDARD     1

class ScDocument;

class SC_DLLPUBLIC ScDataObject
{
public:
                            ScDataObject() {}
    virtual    ~ScDataObject();
    virtual    ScDataObject*       Clone() const = 0;
};

class SC_DLLPUBLIC ScCollection : public ScDataObject
{
protected:
    USHORT          nCount;
    USHORT          nLimit;
    USHORT          nDelta;
    ScDataObject**  pItems;
public:
    ScCollection(USHORT nLim = 4, USHORT nDel = 4);
    ScCollection(const ScCollection& rCollection);
    virtual             ~ScCollection();

    virtual ScDataObject*   Clone() const;

    void        AtFree(USHORT nIndex);
    void        Free(ScDataObject* pScDataObject);
    void        FreeAll();

    BOOL        AtInsert(USHORT nIndex, ScDataObject* pScDataObject);
    virtual BOOL        Insert(ScDataObject* pScDataObject);

    ScDataObject*   At(USHORT nIndex) const;
    virtual USHORT      IndexOf(ScDataObject* pScDataObject) const;
    USHORT GetCount() const;

            ScDataObject* operator[]( const USHORT nIndex) const {return At(nIndex);}
            ScCollection&   operator=( const ScCollection& rCol );
};


class SC_DLLPUBLIC  ScSortedCollection : public ScCollection
{
private:
    BOOL    bDuplicates;
protected:
                        // for ScStrCollection load/store
            void        SetDups( BOOL bVal ) { bDuplicates = bVal; }
            BOOL        IsDups() const { return bDuplicates; }
public:
    ScSortedCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE);
    ScSortedCollection(const ScSortedCollection& rScSortedCollection) :
                            ScCollection(rScSortedCollection),
                            bDuplicates(rScSortedCollection.bDuplicates) {}

    virtual USHORT      IndexOf(ScDataObject* pScDataObject) const;
    virtual short       Compare(ScDataObject* pKey1, ScDataObject* pKey2) const = 0;
    virtual BOOL        IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const;
    BOOL        Search(ScDataObject* pScDataObject, USHORT& rIndex) const;
    virtual BOOL        Insert(ScDataObject* pScDataObject);
    virtual BOOL        InsertPos(ScDataObject* pScDataObject, USHORT& nIndex);

    BOOL        operator==(const ScSortedCollection& rCmp) const;
};



//------------------------------------------------------------------------
class StrData : public ScDataObject
{
friend class ScStrCollection;
    String aStr;
public:
                        StrData(const String& rStr) : aStr(rStr) {}
                        StrData(const StrData& rData) : ScDataObject(), aStr(rData.aStr) {}
    virtual ScDataObject*   Clone() const;
    const String&       GetString() const { return aStr; }
    // SetString only, if StrData is not in ScStrCollection! for example
    // for Searcher
    void                SetString( const String& rNew ) { aStr = rNew; }
};

class SvStream;

class SC_DLLPUBLIC ScStrCollection : public ScSortedCollection
{
public:
    ScStrCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) :
                        ScSortedCollection  ( nLim, nDel, bDup ) {}
    ScStrCollection(const ScStrCollection& rScStrCollection) :
                        ScSortedCollection  ( rScStrCollection ) {}

    virtual ScDataObject*   Clone() const;
            StrData*    operator[]( const USHORT nIndex) const {return (StrData*)At(nIndex);}
    virtual short       Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;
};

//------------------------------------------------------------------------
// TypedScStrCollection: wie ScStrCollection, nur, dass Zahlen vor Strings
//                     sortiert werden

class TypedStrData : public ScDataObject
{
public:
            TypedStrData( const String& rStr, double nVal = 0.0,
                          USHORT nType = SC_STRTYPE_STANDARD )
                : aStrValue(rStr),
                  nValue(nVal),
                  nStrType(nType) {}

            TypedStrData( const TypedStrData& rCpy )
                : ScDataObject(),
                  aStrValue(rCpy.aStrValue),
                  nValue(rCpy.nValue),
                  nStrType(rCpy.nStrType) {}

    virtual ScDataObject*   Clone() const;

    BOOL                IsStrData() const { return nStrType != 0; }
    const String&       GetString() const { return aStrValue; }
    double              GetValue () const { return nValue; }

private:
    friend class TypedScStrCollection;

    String  aStrValue;
    double  nValue;
    USHORT  nStrType;           // 0 = Value
};

class SC_DLLPUBLIC TypedScStrCollection : public ScSortedCollection
{
private:
    BOOL    bCaseSensitive;

public:
    TypedScStrCollection( USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE );

    TypedScStrCollection( const TypedScStrCollection& rCpy )
        : ScSortedCollection( rCpy ) { bCaseSensitive = rCpy.bCaseSensitive; }
    ~TypedScStrCollection();

    virtual ScDataObject*       Clone() const;
    virtual short           Compare( ScDataObject* pKey1, ScDataObject* pKey2 ) const;

    TypedStrData*   operator[]( const USHORT nIndex) const;

    void    SetCaseSensitive( BOOL bSet );

    BOOL    FindText( const String& rStart, String& rResult, USHORT& rPos, BOOL bBack ) const;
    BOOL    GetExactMatch( String& rString ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
