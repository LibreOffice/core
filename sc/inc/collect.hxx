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
    sal_uInt16          nCount;
    sal_uInt16          nLimit;
    sal_uInt16          nDelta;
    ScDataObject**  pItems;
public:
    ScCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4);
    ScCollection(const ScCollection& rCollection);
    virtual             ~ScCollection();

    virtual ScDataObject*   Clone() const;

    void        AtFree(sal_uInt16 nIndex);
    void        Free(ScDataObject* pScDataObject);
    void        FreeAll();

    sal_Bool        AtInsert(sal_uInt16 nIndex, ScDataObject* pScDataObject);
    virtual sal_Bool        Insert(ScDataObject* pScDataObject);

    ScDataObject*   At(sal_uInt16 nIndex) const;
    virtual sal_uInt16      IndexOf(ScDataObject* pScDataObject) const;
    sal_uInt16 GetCount() const;

            ScDataObject* operator[]( const sal_uInt16 nIndex) const {return At(nIndex);}
            ScCollection&   operator=( const ScCollection& rCol );
};


class SC_DLLPUBLIC  ScSortedCollection : public ScCollection
{
private:
    sal_Bool    bDuplicates;
protected:
                        // for ScStrCollection load/store
            void        SetDups( sal_Bool bVal ) { bDuplicates = bVal; }
            sal_Bool        IsDups() const { return bDuplicates; }
public:
    ScSortedCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = false);
    ScSortedCollection(const ScSortedCollection& rScSortedCollection) :
                            ScCollection(rScSortedCollection),
                            bDuplicates(rScSortedCollection.bDuplicates) {}

    virtual sal_uInt16      IndexOf(ScDataObject* pScDataObject) const;
    virtual short       Compare(ScDataObject* pKey1, ScDataObject* pKey2) const = 0;
    virtual sal_Bool        IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const;
    sal_Bool        Search(ScDataObject* pScDataObject, sal_uInt16& rIndex) const;
    virtual sal_Bool        Insert(ScDataObject* pScDataObject);
    virtual sal_Bool        InsertPos(ScDataObject* pScDataObject, sal_uInt16& nIndex);

    sal_Bool        operator==(const ScSortedCollection& rCmp) const;
};

//------------------------------------------------------------------------
// TypedScStrCollection: wie ScStrCollection, nur, dass Zahlen vor Strings
//                     sortiert werden

class TypedStrData : public ScDataObject
{
    friend class TypedScStrCollection;
public:
    TypedStrData( const rtl::OUString& rStr, double nVal = 0.0,
                  sal_uInt16 nType = SC_STRTYPE_STANDARD );

    TypedStrData( const TypedStrData& rCpy );

    virtual ScDataObject*   Clone() const;

    bool IsStrData() const;
    SC_DLLPUBLIC const rtl::OUString& GetString() const;
    double GetValue() const;

private:
    rtl::OUString aStrValue;
    double nValue;
    sal_uInt16 nStrType;           // 0 = Value
};

class SC_DLLPUBLIC TypedScStrCollection : public ScSortedCollection
{
private:
    sal_Bool    bCaseSensitive;

public:
    TypedScStrCollection( sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = false );

    TypedScStrCollection( const TypedScStrCollection& rCpy )
        : ScSortedCollection( rCpy ) { bCaseSensitive = rCpy.bCaseSensitive; }
    ~TypedScStrCollection();

    virtual ScDataObject*       Clone() const;
    virtual short           Compare( ScDataObject* pKey1, ScDataObject* pKey2 ) const;

    TypedStrData*   operator[]( const sal_uInt16 nIndex) const;

    void    SetCaseSensitive( sal_Bool bSet );

    sal_Bool    FindText( const String& rStart, String& rResult, sal_uInt16& rPos, sal_Bool bBack ) const;
    sal_Bool    GetExactMatch( String& rString ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
