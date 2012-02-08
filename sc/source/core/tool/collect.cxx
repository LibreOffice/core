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




#include <string.h>
#include <tools/stream.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "rechead.hxx"
#include "collect.hxx"
#include "document.hxx"         // fuer TypedStrData Konstruktor

// -----------------------------------------------------------------------

ScDataObject::~ScDataObject()
{
}

//------------------------------------------------------------------------
// Collection
//------------------------------------------------------------------------

void lcl_DeleteScDataObjects( ScDataObject** p, sal_uInt16 nCount )
{
    if ( p )
    {
        for (sal_uInt16 i = 0; i < nCount; i++) delete p[i];
        delete[] p;
        p = NULL;
    }
}

ScCollection::ScCollection(sal_uInt16 nLim, sal_uInt16 nDel) :
    nCount ( 0 ),
    nLimit ( nLim ),
    nDelta ( nDel ),
    pItems ( NULL )
{
    if (nDelta > MAXDELTA)
        nDelta = MAXDELTA;
    else if (nDelta == 0)
        nDelta = 1;
    if (nLimit > MAXCOLLECTIONSIZE)
        nLimit = MAXCOLLECTIONSIZE;
    else if (nLimit < nDelta)
        nLimit = nDelta;
    pItems = new ScDataObject*[nLimit];
}

ScCollection::ScCollection(const ScCollection& rCollection)
    :   ScDataObject(),
        nCount ( 0 ),
        nLimit ( 0 ),
        nDelta ( 0 ),
        pItems ( NULL )
{
    *this = rCollection;
}

//------------------------------------------------------------------------

ScCollection::~ScCollection()
{
    lcl_DeleteScDataObjects( pItems, nCount );
}

//------------------------------------------------------------------------
sal_uInt16 ScCollection::GetCount() const { return nCount; }
void ScCollection::AtFree(sal_uInt16 nIndex)
{
    if ((pItems) && (nIndex < nCount))
    {
        delete pItems[nIndex];
        --nCount;               // before memmove
        memmove ( &pItems[nIndex], &pItems[nIndex + 1], (nCount - nIndex) * sizeof(ScDataObject*));
        pItems[nCount] = NULL;
    }
}

//------------------------------------------------------------------------

void ScCollection::Free(ScDataObject* pScDataObject)
{
    AtFree(IndexOf(pScDataObject));
}

//------------------------------------------------------------------------

void ScCollection::FreeAll()
{
    lcl_DeleteScDataObjects( pItems, nCount );
    nCount = 0;
    pItems = new ScDataObject*[nLimit];
}

//------------------------------------------------------------------------

sal_Bool ScCollection::AtInsert(sal_uInt16 nIndex, ScDataObject* pScDataObject)
{
    if ((nCount < MAXCOLLECTIONSIZE) && (nIndex <= nCount) && pItems)
    {
        if (nCount == nLimit)
        {
            ScDataObject** pNewItems = new ScDataObject*[nLimit + nDelta];
            if (!pNewItems)
                return false;
            nLimit = sal::static_int_cast<sal_uInt16>( nLimit + nDelta );
            memmove(pNewItems, pItems, nCount * sizeof(ScDataObject*));
            delete[] pItems;
            pItems = pNewItems;
        }
        if (nCount > nIndex)
            memmove(&pItems[nIndex + 1], &pItems[nIndex], (nCount - nIndex) * sizeof(ScDataObject*));
        pItems[nIndex] = pScDataObject;
        nCount++;
        return sal_True;
    }
    return false;
}

//------------------------------------------------------------------------

sal_Bool ScCollection::Insert(ScDataObject* pScDataObject)
{
    return AtInsert(nCount, pScDataObject);
}

//------------------------------------------------------------------------

ScDataObject* ScCollection::At(sal_uInt16 nIndex) const
{
    if (nIndex < nCount)
        return pItems[nIndex];
    else
        return NULL;
}

//------------------------------------------------------------------------

sal_uInt16 ScCollection::IndexOf(ScDataObject* pScDataObject) const
{
    sal_uInt16 nIndex = 0xffff;
    for (sal_uInt16 i = 0; ((i < nCount) && (nIndex == 0xffff)); i++)
    {
        if (pItems[i] == pScDataObject) nIndex = i;
    }
    return nIndex;
}

//------------------------------------------------------------------------

ScCollection& ScCollection::operator=( const ScCollection& r )
{
    lcl_DeleteScDataObjects( pItems, nCount );

    nCount = r.nCount;
    nLimit = r.nLimit;
    nDelta = r.nDelta;
    pItems = new ScDataObject*[nLimit];
    for ( sal_uInt16 i=0; i<nCount; i++ )
        pItems[i] = r.pItems[i]->Clone();

    return *this;
}

//------------------------------------------------------------------------

ScDataObject*   ScCollection::Clone() const
{
    return new ScCollection(*this);
}

//------------------------------------------------------------------------
// ScSortedCollection
//------------------------------------------------------------------------

ScSortedCollection::ScSortedCollection(sal_uInt16 nLim, sal_uInt16 nDel, sal_Bool bDup) :
    ScCollection (nLim, nDel),
    bDuplicates ( bDup)
{
}

//------------------------------------------------------------------------

sal_uInt16 ScSortedCollection::IndexOf(ScDataObject* pScDataObject) const
{
    sal_uInt16 nIndex;
    if (Search(pScDataObject, nIndex))
        return nIndex;
    else
        return 0xffff;
}

//------------------------------------------------------------------------

sal_Bool ScSortedCollection::Search(ScDataObject* pScDataObject, sal_uInt16& rIndex) const
{
    rIndex = nCount;
    sal_Bool bFound = false;
    short nLo = 0;
    short nHi = nCount - 1;
    short nIndex;
    short nCompare;
    while (nLo <= nHi)
    {
        nIndex = (nLo + nHi) / 2;
        nCompare = Compare(pItems[nIndex], pScDataObject);
        if (nCompare < 0)
            nLo = nIndex + 1;
        else
        {
            nHi = nIndex - 1;
            if (nCompare == 0)
            {
                bFound = sal_True;
                nLo = nIndex;
            }
        }
    }
    rIndex = nLo;
    return bFound;
}

//------------------------------------------------------------------------

sal_Bool ScSortedCollection::Insert(ScDataObject* pScDataObject)
{
    sal_uInt16 nIndex;
    sal_Bool bFound = Search(pScDataObject, nIndex);
    if (bFound)
    {
        if (bDuplicates)
            return AtInsert(nIndex, pScDataObject);
        else
            return false;
    }
    else
        return AtInsert(nIndex, pScDataObject);
}

//------------------------------------------------------------------------

sal_Bool ScSortedCollection::InsertPos(ScDataObject* pScDataObject, sal_uInt16& nIndex)
{
    sal_Bool bFound = Search(pScDataObject, nIndex);
    if (bFound)
    {
        if (bDuplicates)
            return AtInsert(nIndex, pScDataObject);
        else
            return false;
    }
    else
        return AtInsert(nIndex, pScDataObject);
}

//------------------------------------------------------------------------

sal_Bool ScSortedCollection::operator==(const ScSortedCollection& rCmp) const
{
    if ( nCount != rCmp.nCount )
        return false;
    for (sal_uInt16 i=0; i<nCount; i++)
        if ( !IsEqual(pItems[i],rCmp.pItems[i]) )
            return false;
    return sal_True;
}

//------------------------------------------------------------------------

//  IsEqual - komplette Inhalte vergleichen

sal_Bool ScSortedCollection::IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return ( Compare(pKey1, pKey2) == 0 );      // Default: nur Index vergleichen
}

//------------------------------------------------------------------------
// TypedScStrCollection
//------------------------------------------------------------------------

bool TypedStrData::LessCaseSensitive::operator() (const TypedStrData& left, const TypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
        return left.mfValue < right.mfValue;

    return ScGlobal::GetCaseTransliteration()->compareString(
        left.maStrValue, right.maStrValue) < 0;
}

bool TypedStrData::LessCaseInsensitive::operator() (const TypedStrData& left, const TypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
        return left.mfValue < right.mfValue;

    return ScGlobal::GetpTransliteration()->compareString(
        left.maStrValue, right.maStrValue) < 0;
}

bool TypedStrData::EqualCaseSensitive::operator() (const TypedStrData& left, const TypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return false;

    if (left.meStrType == Value && left.mfValue != right.mfValue)
        return false;

    return ScGlobal::GetCaseTransliteration()->compareString(
        left.maStrValue, right.maStrValue) == 0;
}

bool TypedStrData::EqualCaseInsensitive::operator() (const TypedStrData& left, const TypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return false;

    if (left.meStrType == Value && left.mfValue != right.mfValue)
        return false;

    return ScGlobal::GetpTransliteration()->compareString(
        left.maStrValue, right.maStrValue) == 0;
}

bool TypedStrData::operator== (const TypedStrData& r) const
{
    // Case insensitive comparison by default.
    EqualCaseInsensitive aHdl;
    return aHdl(*this, r);
}

bool TypedStrData::operator< (const TypedStrData& r) const
{
    // Case insensitive comparison by default.
    LessCaseInsensitive aHdl;
    return aHdl(*this, r);
}

TypedStrData::TypedStrData(
    const rtl::OUString& rStr, double nVal, StringType nType ) :
    maStrValue(rStr),
    mfValue(nVal),
    meStrType(nType) {}

TypedStrData::TypedStrData( const TypedStrData& rCpy ) :
    maStrValue(rCpy.maStrValue),
    mfValue(rCpy.mfValue),
    meStrType(rCpy.meStrType) {}

bool TypedStrData::IsStrData() const
{
    return meStrType != Value;
}

const rtl::OUString& TypedStrData::GetString() const
{
    return maStrValue;
}

double TypedStrData::GetValue() const
{
    return mfValue;
}

TypedStrData::StringType TypedStrData::GetStringType() const
{
    return meStrType;
}

FindTypedStrData::FindTypedStrData(const TypedStrData& rVal, bool bCaseSens) :
    maVal(rVal), mbCaseSens(bCaseSens) {}

bool FindTypedStrData::operator() (const TypedStrData& r) const
{
    if (mbCaseSens)
    {
        TypedStrData::EqualCaseSensitive aHdl;
        return aHdl(maVal, r);
    }
    else
    {
        TypedStrData::EqualCaseInsensitive aHdl;
        return aHdl(maVal, r);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
