/*************************************************************************
 *
 *  $RCSfile: collect.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 18:10:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <string.h>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#include <unotools/collatorwrapper.hxx>

#include "rechead.hxx"
#include "collect.hxx"
#include "document.hxx"         // fuer TypedStrData Konstruktor

// -----------------------------------------------------------------------

DataObject::~DataObject()
{
}

//------------------------------------------------------------------------
// Collection
//------------------------------------------------------------------------

void lcl_DeleteDataObjects( DataObject** p, USHORT nCount )
{
    if ( p )
    {
        for (USHORT i = 0; i < nCount; i++) delete p[i];
        delete[] p;
        p = NULL;
    }
}

Collection::Collection(USHORT nLim, USHORT nDel) :
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
    pItems = new DataObject*[nLimit];
}

Collection::Collection(const Collection& rCollection)
    :   nCount ( 0 ),
        nLimit ( 0 ),
        nDelta ( 0 ),
        pItems ( NULL )
{
    *this = rCollection;
}

//------------------------------------------------------------------------

Collection::~Collection()
{
    lcl_DeleteDataObjects( pItems, nCount );
}

//------------------------------------------------------------------------

void Collection::AtFree(USHORT nIndex)
{
    if ((pItems) && (nIndex < nCount))
    {
        delete pItems[nIndex];
        memmove ( &pItems[nIndex], &pItems[nIndex + 1], (nCount - nIndex) * sizeof(DataObject*));
        --nCount;
        pItems[nCount] = NULL;
    }
}

//------------------------------------------------------------------------

void Collection::Free(DataObject* pDataObject)
{
    AtFree(IndexOf(pDataObject));
}

//------------------------------------------------------------------------

void Collection::FreeAll()
{
    lcl_DeleteDataObjects( pItems, nCount );
    nCount = 0;
    pItems = new DataObject*[nLimit];
}

//------------------------------------------------------------------------

BOOL Collection::AtInsert(USHORT nIndex, DataObject* pDataObject)
{
    if ((nCount < MAXCOLLECTIONSIZE) && (nIndex <= nCount) && pItems)
    {
        if (nCount == nLimit)
        {
            DataObject** pNewItems = new DataObject*[nLimit + nDelta];
            if (!pNewItems)
                return FALSE;
            nLimit += nDelta;
            memmove(pNewItems, pItems, nCount * sizeof(DataObject*));
            delete[] pItems;
            pItems = pNewItems;
        }
        if (nCount > nIndex)
            memmove(&pItems[nIndex + 1], &pItems[nIndex], (nCount - nIndex) * sizeof(DataObject*));
        pItems[nIndex] = pDataObject;
        nCount++;
        return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------------

BOOL Collection::Insert(DataObject* pDataObject)
{
    return AtInsert(nCount, pDataObject);
}

//------------------------------------------------------------------------

DataObject* Collection::At(USHORT nIndex) const
{
    if (nIndex < nCount)
        return pItems[nIndex];
    else
        return NULL;
}

//------------------------------------------------------------------------

USHORT Collection::IndexOf(DataObject* pDataObject) const
{
    USHORT nIndex = 0xffff;
    for (USHORT i = 0; ((i < nCount) && (nIndex == 0xffff)); i++)
    {
        if (pItems[i] == pDataObject) nIndex = i;
    }
    return nIndex;
}

//------------------------------------------------------------------------

Collection& Collection::operator=( const Collection& r )
{
    lcl_DeleteDataObjects( pItems, nCount );

    nCount = r.nCount;
    nLimit = r.nLimit;
    nDelta = r.nDelta;
    pItems = new DataObject*[nLimit];
    for ( USHORT i=0; i<nCount; i++ )
        pItems[i] = r.pItems[i]->Clone();

    return *this;
}

//------------------------------------------------------------------------

DataObject* Collection::Clone() const
{
    return new Collection(*this);
}

//------------------------------------------------------------------------
// SortedCollection
//------------------------------------------------------------------------

SortedCollection::SortedCollection(USHORT nLim, USHORT nDel, BOOL bDup) :
    Collection (nLim, nDel),
    bDuplicates ( bDup)
{
}

//------------------------------------------------------------------------

USHORT SortedCollection::IndexOf(DataObject* pDataObject) const
{
    USHORT nIndex;
    if (Search(pDataObject, nIndex))
        return nIndex;
    else
        return 0xffff;
}

//------------------------------------------------------------------------

BOOL SortedCollection::Search(DataObject* pDataObject, USHORT& rIndex) const
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
        nCompare = Compare(pItems[nIndex], pDataObject);
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

BOOL SortedCollection::Insert(DataObject* pDataObject)
{
    USHORT nIndex;
    BOOL bFound = Search(pDataObject, nIndex);
    if (bFound)
    {
        if (bDuplicates)
            return AtInsert(nIndex, pDataObject);
        else
            return FALSE;
    }
    else
        return AtInsert(nIndex, pDataObject);
}

//------------------------------------------------------------------------

BOOL SortedCollection::InsertPos(DataObject* pDataObject, USHORT& nIndex)
{
    BOOL bFound = Search(pDataObject, nIndex);
    if (bFound)
    {
        if (bDuplicates)
            return AtInsert(nIndex, pDataObject);
        else
            return FALSE;
    }
    else
        return AtInsert(nIndex, pDataObject);
}

//------------------------------------------------------------------------

BOOL SortedCollection::operator==(const SortedCollection& rCmp) const
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

BOOL SortedCollection::IsEqual(DataObject* pKey1, DataObject* pKey2) const
{
    return ( Compare(pKey1, pKey2) == 0 );      // Default: nur Index vergleichen
}

//------------------------------------------------------------------------

DataObject* StrData::Clone() const
{
    return new StrData(*this);
}

//------------------------------------------------------------------------

short StrCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
{
    StringCompare eComp = ((StrData*)pKey2)->aStr.CompareTo(((StrData*)pKey1)->aStr);
    if (eComp == COMPARE_EQUAL)
        return 0;
    else if (eComp == COMPARE_LESS)
        return -1;
    else
        return 1;
}

//------------------------------------------------------------------------

DataObject* StrCollection::Clone() const
{
    return new StrCollection(*this);
}

//------------------------------------------------------------------------

void StrCollection::Load( SvStream& rStream )
{
    ScReadHeader aHdr( rStream );
    lcl_DeleteDataObjects( pItems, nCount );
    BOOL bDups;
    rStream >> bDups;
    SetDups( bDups );
    rStream >> nCount >> nLimit >> nDelta;
    pItems = new DataObject*[nLimit];
    String aStr;
    rtl_TextEncoding eSet = rStream.GetStreamCharSet();
    for ( USHORT i=0; i<nCount; i++ )
    {
        rStream.ReadByteString( aStr, eSet );
        pItems[i] = new StrData( aStr );
    }
}

void StrCollection::Store( SvStream& rStream ) const
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
// TypedStrCollection
//------------------------------------------------------------------------

TypedStrData::TypedStrData( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab,
                                BOOL bAllStrings )
{
    if ( pDoc->HasValueData( nCol, nRow, nTab ) )
    {
        pDoc->GetValue( nCol, nRow, nTab, nValue );
        if (bAllStrings)
            pDoc->GetString( nCol, nRow, nTab, aStrValue );
        nStrType = 0;
    }
    else
    {
        pDoc->GetString( nCol, nRow, nTab, aStrValue );
        nValue = 0.0;
        nStrType = 1;       //! Typ uebergeben ?
    }
}

DataObject* TypedStrData::Clone() const
{
    return new TypedStrData(*this);
}

DataObject* TypedStrCollection::Clone() const
{
    return new TypedStrCollection(*this);
}

short TypedStrCollection::Compare( DataObject* pKey1, DataObject* pKey2 ) const
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
                nResult = (short) ScGlobal::pCaseCollator->compareString(
                    rData1.aStrValue, rData2.aStrValue );
            else
                nResult = (short) ScGlobal::pCollator->compareString(
                    rData1.aStrValue, rData2.aStrValue );
        }
    }

    return nResult;
}

BOOL TypedStrCollection::FindText( const String& rStart, String& rResult,
                                    USHORT& rPos, BOOL bBack ) const
{
    //  Die Collection ist nach String-Vergleichen sortiert, darum muss hier
    //  alles durchsucht werden

    xub_StrLen nCmpLen = rStart.Len();
    BOOL bFound = FALSE;

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
                String aCmp = pData->aStrValue.Copy(0,nCmpLen);
                if ( ScGlobal::pCollator->compareString( aCmp, rStart )
                        == COMPARE_EQUAL )
                {
                    rResult = pData->aStrValue;
                    rPos = i;
                    bFound = TRUE;
                    break;
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
                String aCmp = pData->aStrValue.Copy(0,nCmpLen);
                if ( ScGlobal::pCollator->compareString( aCmp, rStart )
                        == COMPARE_EQUAL )
                {
                    rResult = pData->aStrValue;
                    rPos = i;
                    bFound = TRUE;
                    break;
                }
            }
        }
    }

    return bFound;
}

        // Gross-/Kleinschreibung anpassen

BOOL TypedStrCollection::GetExactMatch( String& rString ) const
{
    for (USHORT i=0; i<nCount; i++)
    {
        TypedStrData* pData = (TypedStrData*) pItems[i];
        if ( pData->nStrType &&
             ScGlobal::pCollator->compareString( pData->aStrValue, rString )
                 == COMPARE_EQUAL )
        {
            rString = pData->aStrValue;                         // String anpassen
            return TRUE;
        }
    }

    return FALSE;
}



