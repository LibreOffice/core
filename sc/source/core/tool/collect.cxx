/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: collect.cxx,v $
 * $Revision: 1.14.32.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



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

void lcl_DeleteScDataObjects( ScDataObject** p, USHORT nCount )
{
    if ( p )
    {
        for (USHORT i = 0; i < nCount; i++) delete p[i];
        delete[] p;
        p = NULL;
    }
}

ScCollection::ScCollection(USHORT nLim, USHORT nDel) :
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

void ScCollection::AtFree(USHORT nIndex)
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

BOOL ScCollection::AtInsert(USHORT nIndex, ScDataObject* pScDataObject)
{
    if ((nCount < MAXCOLLECTIONSIZE) && (nIndex <= nCount) && pItems)
    {
        if (nCount == nLimit)
        {
            ScDataObject** pNewItems = new ScDataObject*[nLimit + nDelta];
            if (!pNewItems)
                return FALSE;
            nLimit = sal::static_int_cast<USHORT>( nLimit + nDelta );
            memmove(pNewItems, pItems, nCount * sizeof(ScDataObject*));
            delete[] pItems;
            pItems = pNewItems;
        }
        if (nCount > nIndex)
            memmove(&pItems[nIndex + 1], &pItems[nIndex], (nCount - nIndex) * sizeof(ScDataObject*));
        pItems[nIndex] = pScDataObject;
        nCount++;
        return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------------

BOOL ScCollection::Insert(ScDataObject* pScDataObject)
{
    return AtInsert(nCount, pScDataObject);
}

//------------------------------------------------------------------------

ScDataObject* ScCollection::At(USHORT nIndex) const
{
    if (nIndex < nCount)
        return pItems[nIndex];
    else
        return NULL;
}

//------------------------------------------------------------------------

USHORT ScCollection::IndexOf(ScDataObject* pScDataObject) const
{
    USHORT nIndex = 0xffff;
    for (USHORT i = 0; ((i < nCount) && (nIndex == 0xffff)); i++)
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
    for ( USHORT i=0; i<nCount; i++ )
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

ScSortedCollection::ScSortedCollection(USHORT nLim, USHORT nDel, BOOL bDup) :
    ScCollection (nLim, nDel),
    bDuplicates ( bDup)
{
}

//------------------------------------------------------------------------

USHORT ScSortedCollection::IndexOf(ScDataObject* pScDataObject) const
{
    USHORT nIndex;
    if (Search(pScDataObject, nIndex))
        return nIndex;
    else
        return 0xffff;
}

//------------------------------------------------------------------------

BOOL ScSortedCollection::Search(ScDataObject* pScDataObject, USHORT& rIndex) const
{
    rIndex = nCount;
    BOOL bFound = FALSE;
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
                bFound = TRUE;
                nLo = nIndex;
            }
        }
    }
    rIndex = nLo;
    return bFound;
}

//------------------------------------------------------------------------

BOOL ScSortedCollection::Insert(ScDataObject* pScDataObject)
{
    USHORT nIndex;
    BOOL bFound = Search(pScDataObject, nIndex);
    if (bFound)
    {
        if (bDuplicates)
            return AtInsert(nIndex, pScDataObject);
        else
            return FALSE;
    }
    else
        return AtInsert(nIndex, pScDataObject);
}

//------------------------------------------------------------------------

BOOL ScSortedCollection::InsertPos(ScDataObject* pScDataObject, USHORT& nIndex)
{
    BOOL bFound = Search(pScDataObject, nIndex);
    if (bFound)
    {
        if (bDuplicates)
            return AtInsert(nIndex, pScDataObject);
        else
            return FALSE;
    }
    else
        return AtInsert(nIndex, pScDataObject);
}

//------------------------------------------------------------------------

BOOL ScSortedCollection::operator==(const ScSortedCollection& rCmp) const
{
    if ( nCount != rCmp.nCount )
        return FALSE;
    for (USHORT i=0; i<nCount; i++)
        if ( !IsEqual(pItems[i],rCmp.pItems[i]) )
            return FALSE;
    return TRUE;
}

//------------------------------------------------------------------------

//  IsEqual - komplette Inhalte vergleichen

BOOL ScSortedCollection::IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return ( Compare(pKey1, pKey2) == 0 );      // Default: nur Index vergleichen
}

//------------------------------------------------------------------------

ScDataObject*   StrData::Clone() const
{
    return new StrData(*this);
}

//------------------------------------------------------------------------

short ScStrCollection::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    StringCompare eComp = ((StrData*)pKey1)->aStr.CompareTo(((StrData*)pKey2)->aStr);
    if (eComp == COMPARE_EQUAL)
        return 0;
    else if (eComp == COMPARE_LESS)
        return -1;
    else
        return 1;
}

//------------------------------------------------------------------------

ScDataObject*   ScStrCollection::Clone() const
{
    return new ScStrCollection(*this);
}

//------------------------------------------------------------------------

void ScStrCollection::Load( SvStream& rStream )
{
    ScReadHeader aHdr( rStream );
    lcl_DeleteScDataObjects( pItems, nCount );
    BOOL bDups;
    rStream >> bDups;
    SetDups( bDups );
    rStream >> nCount >> nLimit >> nDelta;
    pItems = new ScDataObject*[nLimit];
    String aStr;
    rtl_TextEncoding eSet = rStream.GetStreamCharSet();
    for ( USHORT i=0; i<nCount; i++ )
    {
        rStream.ReadByteString( aStr, eSet );
        pItems[i] = new StrData( aStr );
    }
}

void ScStrCollection::Store( SvStream& rStream ) const
{
    ScWriteHeader aHdr( rStream );
    BOOL bDups = IsDups();
    rStream << bDups << nCount << nLimit << nDelta;
    rtl_TextEncoding eSet = rStream.GetStreamCharSet();
    for ( USHORT i=0; i<nCount; i++ )
    {
        rStream.WriteByteString( ((StrData*)pItems[i])->GetString(), eSet );
    }
}

//------------------------------------------------------------------------
// TypedScStrCollection
//------------------------------------------------------------------------

//UNUSED2008-05  TypedStrData::TypedStrData( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
//UNUSED2008-05                                  BOOL bAllStrings )
//UNUSED2008-05  {
//UNUSED2008-05      if ( pDoc->HasValueData( nCol, nRow, nTab ) )
//UNUSED2008-05      {
//UNUSED2008-05          pDoc->GetValue( nCol, nRow, nTab, nValue );
//UNUSED2008-05          if (bAllStrings)
//UNUSED2008-05              pDoc->GetString( nCol, nRow, nTab, aStrValue );
//UNUSED2008-05          nStrType = 0;
//UNUSED2008-05      }
//UNUSED2008-05      else
//UNUSED2008-05      {
//UNUSED2008-05          pDoc->GetString( nCol, nRow, nTab, aStrValue );
//UNUSED2008-05          nValue = 0.0;
//UNUSED2008-05          nStrType = 1;       //! Typ uebergeben ?
//UNUSED2008-05      }
//UNUSED2008-05  }

ScDataObject*   TypedStrData::Clone() const
{
    return new TypedStrData(*this);
}

ScDataObject* TypedScStrCollection::Clone() const
{
    return new TypedScStrCollection(*this);
}

short TypedScStrCollection::Compare( ScDataObject* pKey1, ScDataObject* pKey2 ) const
{
    short nResult = 0;

    if ( pKey1 && pKey2 )
    {
        TypedStrData& rData1 = (TypedStrData&)*pKey1;
        TypedStrData& rData2 = (TypedStrData&)*pKey2;

        if ( rData1.nStrType > rData2.nStrType )
            nResult = 1;
        else if ( rData1.nStrType < rData2.nStrType )
            nResult = -1;
        else if ( !rData1.nStrType /* && !rData2.nStrType */ )
        {
            //--------------------
            // Zahlen vergleichen:
            //--------------------
            if ( rData1.nValue == rData2.nValue )
                nResult = 0;
            else if ( rData1.nValue < rData2.nValue )
                nResult = -1;
            else
                nResult = 1;
        }
        else /* if ( rData1.nStrType && rData2.nStrType ) */
        {
            //---------------------
            // Strings vergleichen:
            //---------------------
            if ( bCaseSensitive )
                nResult = (short) ScGlobal::pCaseTransliteration->compareString(
                    rData1.aStrValue, rData2.aStrValue );
            else
                nResult = (short) ScGlobal::pTransliteration->compareString(
                    rData1.aStrValue, rData2.aStrValue );
        }
    }

    return nResult;
}

BOOL TypedScStrCollection::FindText( const String& rStart, String& rResult,
                                    USHORT& rPos, BOOL bBack ) const
{
    //  Die Collection ist nach String-Vergleichen sortiert, darum muss hier
    //  alles durchsucht werden

    BOOL bFound = FALSE;

    String aOldResult;
    if ( rPos != SCPOS_INVALID && rPos < nCount )
    {
        TypedStrData* pData = (TypedStrData*) pItems[rPos];
        if (pData->nStrType)
            aOldResult = pData->aStrValue;
    }

    if ( bBack )                                    // rueckwaerts
    {
        USHORT nStartPos = nCount;
        if ( rPos != SCPOS_INVALID )
            nStartPos = rPos;                       // weitersuchen...

        for ( USHORT i=nStartPos; i>0; )
        {
            --i;
            TypedStrData* pData = (TypedStrData*) pItems[i];
            if (pData->nStrType)
            {
                if ( ScGlobal::pTransliteration->isMatch( rStart, pData->aStrValue ) )
                {
                    //  If the collection is case sensitive, it may contain several entries
                    //  that are equal when compared case-insensitive. They are skipped here.
                    if ( !bCaseSensitive || !aOldResult.Len() ||
                            !ScGlobal::pTransliteration->isEqual(
                            pData->aStrValue, aOldResult ) )
                    {
                        rResult = pData->aStrValue;
                        rPos = i;
                        bFound = TRUE;
                        break;
                    }
                }
            }
        }
    }
    else                                            // vorwaerts
    {
        USHORT nStartPos = 0;
        if ( rPos != SCPOS_INVALID )
            nStartPos = rPos + 1;                   // weitersuchen...

        for ( USHORT i=nStartPos; i<nCount; i++ )
        {
            TypedStrData* pData = (TypedStrData*) pItems[i];
            if (pData->nStrType)
            {
                if ( ScGlobal::pTransliteration->isMatch( rStart, pData->aStrValue ) )
                {
                    //  If the collection is case sensitive, it may contain several entries
                    //  that are equal when compared case-insensitive. They are skipped here.
                    if ( !bCaseSensitive || !aOldResult.Len() ||
                            !ScGlobal::pTransliteration->isEqual(
                            pData->aStrValue, aOldResult ) )
                    {
                        rResult = pData->aStrValue;
                        rPos = i;
                        bFound = TRUE;
                        break;
                    }
                }
            }
        }
    }

    return bFound;
}

        // Gross-/Kleinschreibung anpassen

BOOL TypedScStrCollection::GetExactMatch( String& rString ) const
{
    for (USHORT i=0; i<nCount; i++)
    {
        TypedStrData* pData = (TypedStrData*) pItems[i];
        if ( pData->nStrType && ScGlobal::pTransliteration->isEqual(
                pData->aStrValue, rString ) )
        {
            rString = pData->aStrValue;                         // String anpassen
            return TRUE;
        }
    }

    return FALSE;
}



