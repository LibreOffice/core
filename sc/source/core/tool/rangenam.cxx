/*************************************************************************
 *
 *  $RCSfile: rangenam.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2001-02-21 18:33:53 $
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

//------------------------------------------------------------------------

#include <tools/debug.hxx>
#include <tools/intn.hxx>
#include <string.h>

#include "rangenam.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "rangeutl.hxx"
#include "rechead.hxx"
#include "refupdat.hxx"
#include "document.hxx"
#include "indexmap.hxx"


//========================================================================
// ScRangeData
//========================================================================

// Interner ctor fuer das Suchen nach einem Index

ScRangeData::ScRangeData( USHORT n )
           : nIndex( n ), pCode( NULL ), bModified( FALSE )
{}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const String& rName,
                          const String& rSymbol,
                          USHORT nCol,
                          USHORT nRow,
                          USHORT nTab,
                          RangeType nType,
                          BOOL bEnglish ) :
                aName       ( rName ),
                aPos        ( nCol, nRow, nTab ),
                eType       ( nType ),
                pDoc        ( pDok ),
                nIndex      ( 0 ),
                nExportIndex( 0 ),
                pCode       ( NULL ),
                bModified   ( FALSE )
{
    if (rSymbol.Len() > 0)
    {
        ScCompiler aComp( pDoc, aPos );
        aComp.SetCompileEnglish(bEnglish);
        pCode = aComp.CompileString( rSymbol );
        if( !pCode->GetError() )
        {
            pCode->Reset();
            ScToken* p = pCode->GetNextReference();
            if( p )// genau eine Referenz als erstes
            {
                if( p->GetType() == svSingleRef )
                    eType = eType | RT_ABSPOS;
                else
                    eType = eType | RT_ABSAREA;
            }
            // ggf. den Fehlercode wg. unvollstaendiger Formel setzen!
            // Dies ist fuer die manuelle Eingabe
            aComp.CompileTokenArray();
            pCode->DelRPN();
        }
    }
}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const String& rName,
                          const ScTokenArray& rArr,
                          USHORT nCol,
                          USHORT nRow,
                          USHORT nTab,
                          RangeType nType ) :
                aName       ( rName ),
                aPos        ( nCol, nRow, nTab ),
                eType       ( nType ),
                pDoc        ( pDok ),
                nIndex      ( 0 ),
                nExportIndex( 0 ),
                pCode       ( new ScTokenArray( rArr ) ),
                bModified   ( FALSE )
{
    if( !pCode->GetError() )
    {
        pCode->Reset();
        ScToken* p = pCode->GetNextReference();
        if( p )// genau eine Referenz als erstes
        {
            if( p->GetType() == svSingleRef )
                eType = eType | RT_ABSPOS;
            else
                eType = eType | RT_ABSAREA;
        }
        // Die Importfilter haben diesen Test nicht,
        // da die benannten Bereiche z.T. noch unvollstaendig sind.
//      if( !pCode->GetCodeLen() )
//      {
//          // ggf. den Fehlercode wg. unvollstaendiger Formel setzen!
//          ScCompiler aComp( pDok, aPos, *pCode );
//          aComp.CompileTokenArray();
//          pCode->DelRPN();
//      }
    }
}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const String& rName,
                          const ScAddress& rTarget ) :
                aName       ( rName ),
                aPos        ( rTarget ),
                eType       ( RT_NAME ),
                pDoc        ( pDok ),
                nIndex      ( 0 ),
                nExportIndex( 0 ),
                pCode       ( new ScTokenArray ),
                bModified   ( FALSE )
{
    SingleRefData aRefData;
    aRefData.InitAddress( rTarget );
    aRefData.SetFlag3D( TRUE );
    pCode->AddSingleReference( aRefData );
    ScCompiler aComp( pDoc, aPos, *pCode );
    aComp.CompileTokenArray();
    if ( !pCode->GetError() )
        eType |= RT_ABSPOS;
}

ScRangeData::ScRangeData(const ScRangeData& rScRangeData) :
    aName   (rScRangeData.aName),
    aPos        (rScRangeData.aPos),
    eType       (rScRangeData.eType),
    pDoc        (rScRangeData.pDoc),
    nIndex      (rScRangeData.nIndex),
    pCode       (rScRangeData.pCode->Clone()),      // echte Kopie erzeugen (nicht copy-ctor)
    bModified   (rScRangeData.bModified)
{}

ScRangeData::~ScRangeData()
{
    delete pCode;
}

DataObject* ScRangeData::Clone() const
{
    return new ScRangeData(*this);
}

ScRangeData::ScRangeData
    ( SvStream& rStream, ScMultipleReadHeader& rHdr, USHORT nVer )
           : pCode      ( new ScTokenArray ),
             bModified  (FALSE)

{
    rHdr.StartEntry();

    if( nVer >= SC_NEW_TOKEN_ARRAYS )
    {
        UINT32 nPos;
        BYTE nData;
        rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
        rStream >> nPos >> eType >> nIndex >> nData;
        if( nData & 0x0F )
            rStream.SeekRel( nData & 0x0F );
        aPos = ScAddress( nPos );
        pCode->Load( rStream, nVer, aPos );
    }
    else
    {
        UINT16 nTokLen, r, c, t;
        rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
        rStream >> c >> r >> t >> eType >> nIndex >> nTokLen;
        aPos.Set( c, r, t );
        if( nTokLen )
            pCode->Load30( rStream, aPos );
    }

    rHdr.EndEntry();
}

BOOL ScRangeData::Store
    ( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
{
    rHdr.StartEntry();

    rStream.WriteByteString( aName, rStream.GetStreamCharSet() );
    rStream << (UINT32) aPos << eType << nIndex << (BYTE) 0x00;
    pCode->Store( rStream, aPos );

    rHdr.EndEntry();
    return TRUE;
}

BOOL ScRangeData::IsBeyond( USHORT nMaxRow ) const
{
    if ( aPos.Row() > nMaxRow )
        return TRUE;

    ScToken* t;
    pCode->Reset();
    while ( t = pCode->GetNextReference() )
        if ( t->GetSingleRef().nRow > nMaxRow ||
                (t->GetType() == svDoubleRef &&
                t->GetDoubleRef().Ref2.nRow > nMaxRow) )
            return TRUE;

    return FALSE;
}

void ScRangeData::GuessPosition()
{
    //  setzt eine Position, mit der alle relative Referenzen bei CalcAbsIfRel
    //  ohne Fehler verabsolutiert werden koennen

    DBG_ASSERT(aPos == ScAddress(), "die Position geht jetzt verloren");

    INT16 nMinCol = 0;
    INT16 nMinRow = 0;
    INT16 nMinTab = 0;

    ScToken* t;
    pCode->Reset();
    while ( t = pCode->GetNextReference() )
    {
        SingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsColRel() && rRef1.nRelCol < nMinCol )
            nMinCol = rRef1.nRelCol;
        if ( rRef1.IsRowRel() && rRef1.nRelRow < nMinRow )
            nMinRow = rRef1.nRelRow;
        if ( rRef1.IsTabRel() && rRef1.nRelTab < nMinTab )
            nMinTab = rRef1.nRelTab;

        if ( t->GetType() == svDoubleRef )
        {
            SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
            if ( rRef2.IsColRel() && rRef2.nRelCol < nMinCol )
                nMinCol = rRef2.nRelCol;
            if ( rRef2.IsRowRel() && rRef2.nRelRow < nMinRow )
                nMinRow = rRef2.nRelRow;
            if ( rRef2.IsTabRel() && rRef2.nRelTab < nMinTab )
                nMinTab = rRef2.nRelTab;
        }
    }

    aPos = ScAddress( (USHORT)(-nMinCol), (USHORT)(-nMinRow), (USHORT)(-nMinTab) );

    //! Test
//  DBG_ERROR(String("Pos ")+String((USHORT)(-nMinCol))+String("/")+
//          String((USHORT)(-nMinRow))+String("/")+String((USHORT)(-nMinTab)));
}

void ScRangeData::GetSymbol (String& rSymbol) const
{
    ScCompiler aScComp(pDoc, aPos, *pCode);
    aScComp.CreateStringFromTokenArray( rSymbol );
}

void ScRangeData::GetEnglishSymbol  (String& rSymbol, BOOL bCompileXML) const
{
    ScCompiler aScComp(pDoc, aPos, *pCode);
    aScComp.SetCompileEnglish( TRUE );
    aScComp.SetCompileXML( bCompileXML );
    aScComp.CreateStringFromTokenArray( rSymbol );
}

void ScRangeData::UpdateSymbol( String& rSymbol, const ScAddress& rPos,
                                BOOL bEnglish, BOOL bCompileXML )
{
    ScTokenArray* pTemp = pCode->Clone();
    ScCompiler aComp( pDoc, rPos, *pTemp );
    aComp.SetCompileEnglish( bEnglish );
    aComp.SetCompileXML( bCompileXML );
    aComp.MoveRelWrap();
    aComp.CreateStringFromTokenArray( rSymbol );
    delete pTemp;
}

void ScRangeData::UpdateReference(  UpdateRefMode eUpdateRefMode,
                                    const ScRange& r,
                                    short nDx, short nDy, short nDz )
{
    BOOL bChanged = FALSE;

    pCode->Reset();
    if( pCode->GetNextReference() )
    {
        ScCompiler aComp( pDoc, aPos, *pCode );
        BOOL bRelRef = aComp.UpdateNameReference( eUpdateRefMode, r,
                                                    nDx, nDy, nDz,
                                                    bChanged);
        if (eType&RT_SHARED)
        {
            if (bRelRef)
                eType = eType | RT_SHAREDMOD;
            else
                eType = eType & ~RT_SHAREDMOD;
        }
    }

    bModified = bChanged;
}


void ScRangeData::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest )
{
    BOOL bChanged = FALSE;

    ScToken* t;
    pCode->Reset();

    for( t = pCode->GetNextReference(); t; t = pCode->GetNextReference() )
    {
        if( t->GetType() != svIndex )
        {
            SingleDoubleRefModifier& rMod = (t->GetType() == svSingleRef ?
                SingleDoubleRefModifier( t->GetSingleRef() ) :
                SingleDoubleRefModifier( t->GetDoubleRef() ));
            ComplRefData& rRef = rMod.Ref();
            if (!rRef.Ref1.IsColRel() && !rRef.Ref1.IsRowRel() &&
                    (!rRef.Ref1.IsFlag3D() || !rRef.Ref1.IsTabRel()) &&
                ( t->GetType() == svSingleRef ||
                (!rRef.Ref2.IsColRel() && !rRef.Ref2.IsRowRel() &&
                    (!rRef.Ref2.IsFlag3D() || !rRef.Ref2.IsTabRel()))))
            {
                if ( ScRefUpdate::UpdateTranspose( pDoc, rSource, rDest, rRef ) != UR_NOTHING )
                    bChanged = TRUE;
            }
        }
    }

    bModified = bChanged;
}

void ScRangeData::UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY )
{
    BOOL bChanged = FALSE;

    ScToken* t;
    pCode->Reset();

    for( t = pCode->GetNextReference(); t; t = pCode->GetNextReference() )
    {
        if( t->GetType() != svIndex )
        {
            SingleDoubleRefModifier& rMod = (t->GetType() == svSingleRef ?
                SingleDoubleRefModifier( t->GetSingleRef() ) :
                SingleDoubleRefModifier( t->GetDoubleRef() ));
            ComplRefData& rRef = rMod.Ref();
            if (!rRef.Ref1.IsColRel() && !rRef.Ref1.IsRowRel() &&
                    (!rRef.Ref1.IsFlag3D() || !rRef.Ref1.IsTabRel()) &&
                ( t->GetType() == svSingleRef ||
                (!rRef.Ref2.IsColRel() && !rRef.Ref2.IsRowRel() &&
                    (!rRef.Ref2.IsFlag3D() || !rRef.Ref2.IsTabRel()))))
            {
                if ( ScRefUpdate::UpdateGrow( rArea,nGrowX,nGrowY, rRef ) != UR_NOTHING )
                    bChanged = TRUE;
            }
        }
    }

    bModified = bChanged;           // muss direkt hinterher ausgewertet werden
}

BOOL ScRangeData::operator== (const ScRangeData& rData) const       // fuer Undo
{
    if ( nIndex != rData.nIndex ||
         aName  != rData.aName  ||
         aPos   != rData.aPos   ||
         eType  != rData.eType     ) return FALSE;

    USHORT nLen = pCode->GetLen();
    if ( nLen != rData.pCode->GetLen() ) return FALSE;

    ScToken** ppThis = pCode->GetArray();
    ScToken** ppOther = rData.pCode->GetArray();

    for ( USHORT i=0; i<nLen; i++ )
        if ( ppThis[i] != ppOther[i] && !(*ppThis[i] == *ppOther[i]) )
            return FALSE;

    return TRUE;
}

BOOL ScRangeData::IsRangeAtCursor( const ScAddress& rPos, BOOL bStartOnly ) const
{
    BOOL bRet = FALSE;
    ScRange aRange;
    if ( IsReference(aRange) )
    {
        if ( bStartOnly )
            bRet = ( rPos == aRange.aStart );
        else
            bRet = ( aRange.In( rPos ) );
    }
    return bRet;
}

BOOL ScRangeData::IsRangeAtBlock( const ScRange& rBlock ) const
{
    BOOL bRet = FALSE;
    ScRange aRange;
    if ( IsReference(aRange) )
        bRet = ( rBlock == aRange );
    return bRet;
}

BOOL ScRangeData::IsReference( ScRange& rRange ) const
{
    BOOL bIs = FALSE;
    if ( eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) )
        if ( pCode )
            return pCode->IsReference( rRange );

    return bIs;
}

void ScRangeData::UpdateTabRef(USHORT nOldTable, USHORT nFlag, USHORT nNewTable)
{
    pCode->Reset();
    if( pCode->GetNextReference() )
    {
        ScRangeData* pRangeData;        // darf nicht dereferenziert werden!!
        BOOL bChanged;
        ScCompiler aComp( pDoc, aPos, *pCode );
        switch (nFlag)
        {
            case 1:                                     // einfache InsertTab (doc.cxx)
                pRangeData = aComp.UpdateInsertTab(nOldTable, TRUE );   // und CopyTab (doc2.cxx)
                break;
            case 2:                                     // einfaches delete (doc.cxx)
                pRangeData = aComp.UpdateDeleteTab(nOldTable, FALSE, TRUE, bChanged);
                break;
            case 3:                                     // move (doc2.cxx)
            {
                pRangeData = aComp.UpdateMoveTab(nOldTable, nNewTable, TRUE );
            }
                break;
            default:
            {
                DBG_ERROR("ScRangeName::UpdateTabRef: Unknown Flag");
            }
                break;
        }
        if (eType&RT_SHARED)
        {
            if (pRangeData)
                eType = eType | RT_SHAREDMOD;
            else
                eType = eType & ~RT_SHAREDMOD;
        }
    }
}

//  wie beim Uebernehmen von Namen in Excel

void ScRangeData::MakeValidName( String& rName )        // static
{
    if (!ScCompiler::HasCharTable())
        ScCompiler::Init();

    //  ungueltige Zeichen vorne weglassen
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rName.Len();
    while ( nPos < nLen && !ScCompiler::IsWordChar( rName.GetChar(nPos) ) )
        ++nPos;
    if ( nPos>0 )
        rName.Erase(0,nPos);

    //  wenn vorne ein ungueltiges Anfangszeichen steht, '_' davor
    if ( rName.Len() && !ScCompiler::IsCharWordChar( rName.GetChar(0) ) )
        rName.Insert('_',0);

    //  ungueltige durch '_' ersetzen
    nLen = rName.Len();
    for (nPos=0; nPos<nLen; nPos++)
    {
        if ( !ScCompiler::IsWordChar( rName.GetChar(nPos) ) )
            rName.SetChar( nPos, '_' );
    }

    // Name darf keine Referenz beinhalten, wie in IsNameValid
    BOOL bOk;
    do
    {
        bOk = TRUE;
        ScRange aRange;
        if( aRange.Parse( rName, NULL ) )
            bOk = FALSE;
        else
        {
            ScAddress aAddr;
            if ( aAddr.Parse( rName, NULL ) )
                bOk = FALSE;
        }
        if ( !bOk )
        {   //! Range Parse auch bei Bereich mit ungueltigem Tabellennamen gueltig
            //! Address Parse dito, Name erzeugt deswegen bei Compile ein #REF!
            if ( rName.SearchAndReplace( ':', '_' ) == STRING_NOTFOUND
              && rName.SearchAndReplace( '.', '_' ) == STRING_NOTFOUND )
                rName.Insert('_',0);
        }
    } while ( !bOk );
}

BOOL ScRangeData::IsNameValid( const String& rName, ScDocument* pDoc )
{
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rName.Len();
    if ( !nLen || !ScCompiler::IsCharWordChar( rName.GetChar(nPos++) ) )
        return FALSE;
    while ( nPos < nLen )
    {
        if ( !ScCompiler::IsWordChar( rName.GetChar(nPos++) ) )
            return FALSE;
    }
    // Parse nicht auf VALID pruefen, es reicht, wenn irgendein Bestandteil
    // erkannt wurde
    ScRange aRange;
    if( aRange.Parse( rName, pDoc ) )
        return FALSE;
    else
    {
        ScAddress aAddr;
        if ( aAddr.Parse( rName, pDoc ) )
            return FALSE;
    }
    return TRUE;
}


USHORT ScRangeData::GetErrCode()
{
    return pCode ? pCode->GetError() : 0;
}

BOOL ScRangeData::HasReferences() const
{
    pCode->Reset();
    return BOOL( pCode->GetNextReference() != NULL );
}

// bei TransferTab von einem in ein anderes Dokument anpassen,
// um Referenzen auf die eigene Tabelle mitzubekommen

void ScRangeData::TransferTabRef( USHORT nOldTab, USHORT nNewTab )
{
    short nTabDiff = nNewTab - nOldTab;
    short nPosDiff = nNewTab - aPos.Tab();
    aPos.SetTab( nNewTab );
    ScToken* t;
    pCode->Reset();
    while ( t = pCode->GetNextReference() )
    {
        SingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsTabRel() )
            rRef1.nTab += nPosDiff;
        else
            rRef1.nTab += nTabDiff;
        if ( t->GetType() == svDoubleRef )
        {
            SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
            if ( rRef2.IsTabRel() )
                rRef2.nTab += nPosDiff;
            else
                rRef2.nTab += nTabDiff;
        }
    }
}


void ScRangeData::ReplaceRangeNamesInUse( const ScIndexMap& rMap )
{
    BOOL bCompile = FALSE;
    for ( ScToken* p = pCode->First(); p; p = pCode->Next() )
    {
        if ( p->GetOpCode() == ocName )
        {
            USHORT nIndex = p->GetIndex();
            USHORT nNewIndex = rMap.Find( nIndex );
            if ( nIndex != nNewIndex )
            {
                p->SetIndex( nNewIndex );
                bCompile = TRUE;
            }
        }
    }
    if ( bCompile )
    {
        ScCompiler aComp( pDoc, aPos, *pCode );
        aComp.CompileTokenArray();
    }
}


int
#ifdef WNT
__cdecl
#endif
ScRangeData::QsortNameCompare( const void* p1, const void* p2 )
{
    switch ( ScGlobal::pScInternational->Compare(
            (*(const ScRangeData**)p1)->aName,
            (*(const ScRangeData**)p2)->aName, INTN_COMPARE_IGNORECASE ) )
    {
        case COMPARE_LESS:
            return -1;
        break;
        case COMPARE_GREATER:
            return 1;
        break;
        default:
            return 0;
    }
}


//========================================================================
// ScRangeName
//========================================================================

ScRangeName::ScRangeName(const ScRangeName& rScRangeName, ScDocument* pDocument) :
                SortedCollection ( rScRangeName ),
                pDoc ( pDocument ),
                nSharedMaxIndex (rScRangeName.nSharedMaxIndex)
{
    for (USHORT i = 0; i < nCount; i++)
    {
        ((ScRangeData*)At(i))->SetDocument(pDocument);
        ((ScRangeData*)At(i))->SetIndex(((ScRangeData*)rScRangeName.At(i))->GetIndex());
    }
}

short ScRangeName::Compare(DataObject* pKey1, DataObject* pKey2) const
{
    USHORT i1 = ((ScRangeData*)pKey1)->GetIndex();
    USHORT i2 = ((ScRangeData*)pKey2)->GetIndex();
    return (short) i1 - (short) i2;
}

BOOL ScRangeName::SearchName( const String& rName, USHORT& rIndex ) const
{
    USHORT i = 0;
    while (i < nCount)
    {
        String aName;
        ((*this)[i])->GetName( aName );
        if ( ScGlobal::pScInternational->CompareEqual(
                aName, rName, INTN_COMPARE_IGNORECASE ) )
        {
            rIndex = i;
            return TRUE;
        }
        i++;
    }
    return FALSE;
}

BOOL ScRangeName::Load( SvStream& rStream, USHORT nVer )
{
    BOOL bSuccess = TRUE;
    USHORT nNewCount;

    while( nCount > 0 )
        AtFree(0);                  // alles loeschen

    ScMultipleReadHeader aHdr( rStream );

    USHORT nDummy;
    if( nVer >= SC_NEW_TOKEN_ARRAYS )
        rStream >> nSharedMaxIndex >> nNewCount;
    else
        rStream >> nSharedMaxIndex >> nDummy >> nNewCount;
    for (USHORT i=0; i<nNewCount && bSuccess; i++)
    {
        ScRangeData* pData = new ScRangeData( rStream, aHdr, nVer );
        pData->SetDocument(pDoc);
        Insert( pData );
        if( rStream.GetError() != SVSTREAM_OK )
            bSuccess = FALSE;
    }
    return bSuccess;
}

BOOL ScRangeName::Store( SvStream& rStream ) const
{
    ScMultipleWriteHeader aHdr( rStream );

    USHORT i;
    USHORT nSaveCount = nCount;
    USHORT nSaveMaxRow = pDoc->GetSrcMaxRow();
    if ( nSaveMaxRow < MAXROW )
    {
        nSaveCount = 0;
        for (i=0; i<nCount; i++)
            if ( !((const ScRangeData*)At(i))->IsBeyond(nSaveMaxRow) )
                ++nSaveCount;

        if ( nSaveCount < nCount )
            pDoc->SetLostData();            // Warnung ausgeben
    }

    rStream << nSharedMaxIndex << nSaveCount;
    BOOL bSuccess = TRUE;

    for (i=0; i<nCount && bSuccess; i++)
    {
        const ScRangeData* pRangeData = (const ScRangeData*)At(i);
        if ( nSaveMaxRow == MAXROW || !pRangeData->IsBeyond(nSaveMaxRow) )
            bSuccess = pRangeData->Store( rStream, aHdr );
    }

    return bSuccess;
}

void ScRangeName::UpdateReference(  UpdateRefMode eUpdateRefMode,
                                    const ScRange& rRange,
                                    short nDx, short nDy, short nDz )
{
    for (USHORT i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateReference(eUpdateRefMode, rRange,
                                                   nDx, nDy, nDz);
}

void ScRangeName::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest )
{
    for (USHORT i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateTranspose( rSource, rDest );
}

void ScRangeName::UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY )
{
    for (USHORT i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateGrow( rArea, nGrowX, nGrowY );
}

BOOL ScRangeName::IsEqual(DataObject* pKey1, DataObject* pKey2) const
{
    return *(ScRangeData*)pKey1 == *(ScRangeData*)pKey2;
}

BOOL ScRangeName::Insert(DataObject* pDataObject)
{
    if (!((ScRangeData*)pDataObject)->GetIndex())       // schon gesetzt?
    {
        ((ScRangeData*)pDataObject)->SetIndex( GetEntryIndex() );
    }

    return SortedCollection::Insert(pDataObject);
}

// Suche nach einem freien Index

USHORT ScRangeName::GetEntryIndex()
{
    USHORT nLast = 0;
    for ( USHORT i = 0; i < nCount; i++ )
    {
        USHORT nIdx = ((ScRangeData*)pItems[i])->GetIndex();
        if( nIdx > nLast )
        {
            nLast = nIdx;
        }
    }
    return nLast + 1;
}

ScRangeData* ScRangeName::FindIndex( USHORT nIndex )
{
    ScRangeData aDataObj( nIndex );
    USHORT n;
    if( Search( &aDataObj, n ) )
        return (*this)[ n ];
    else
        return NULL;
}

ScRangeData* ScRangeName::GetRangeAtCursor( const ScAddress& rPos, BOOL bStartOnly ) const
{
    if ( pItems )
    {
        for ( USHORT i = 0; i < nCount; i++ )
            if ( ((ScRangeData*)pItems[i])->IsRangeAtCursor( rPos, bStartOnly ) )
                return (ScRangeData*)pItems[i];
    }
    return NULL;
}

ScRangeData* ScRangeName::GetRangeAtBlock( const ScRange& rBlock ) const
{
    if ( pItems )
    {
        for ( USHORT i = 0; i < nCount; i++ )
            if ( ((ScRangeData*)pItems[i])->IsRangeAtBlock( rBlock ) )
                return (ScRangeData*)pItems[i];
    }
    return NULL;
}

void ScRangeName::UpdateTabRef(USHORT nOldTable, USHORT nFlag, USHORT nNewTable)
{
    for (USHORT i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateTabRef(nOldTable, nFlag, nNewTable);
}




