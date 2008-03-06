/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rangenam.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:36:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


//------------------------------------------------------------------------

#include <tools/debug.hxx>
#include <string.h>
#include <memory>
#include <unotools/collatorwrapper.hxx>
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

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
           : pCode( NULL ), nIndex( n ), bModified( FALSE )
{}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const String& rName,
                          const String& rSymbol,
                          const ScAddress& rAddress,
                          RangeType nType,
                          const ScGrammar::Grammar eGrammar ) :
                aName       ( rName ),
                aUpperName  ( ScGlobal::pCharClass->upper( rName ) ),
                pCode       ( NULL ),
                aPos        ( rAddress ),
                eType       ( nType ),
                pDoc        ( pDok ),
                nIndex      ( 0 ),
                bModified   ( FALSE )
{
    if (rSymbol.Len() > 0)
    {
        ScCompiler aComp( pDoc, aPos, eGrammar );
        pCode = aComp.CompileString( rSymbol );
        if( !pCode->GetCodeError() )
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
    else
    {
        // #i63513#/#i65690# don't leave pCode as NULL.
        // Copy ctor default-constructs pCode if it was NULL, so it's initialized here, too,
        // to ensure same behavior if unnecessary copying is left out.

        pCode = new ScTokenArray;
    }
}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const String& rName,
                          const ScTokenArray& rArr,
                          const ScAddress& rAddress,
                          RangeType nType ) :
                aName       ( rName ),
                aUpperName  ( ScGlobal::pCharClass->upper( rName ) ),
                pCode       ( new ScTokenArray( rArr ) ),
                aPos        ( rAddress ),
                eType       ( nType ),
                pDoc        ( pDok ),
                nIndex      ( 0 ),
                bModified   ( FALSE )
{
    if( !pCode->GetCodeError() )
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
                aUpperName  ( ScGlobal::pCharClass->upper( rName ) ),
                pCode       ( new ScTokenArray ),
                aPos        ( rTarget ),
                eType       ( RT_NAME ),
                pDoc        ( pDok ),
                nIndex      ( 0 ),
                bModified   ( FALSE )
{
    SingleRefData aRefData;
    aRefData.InitAddress( rTarget );
    aRefData.SetFlag3D( TRUE );
    pCode->AddSingleReference( aRefData );
    ScCompiler aComp( pDoc, aPos, *pCode );
    aComp.CompileTokenArray();
    if ( !pCode->GetCodeError() )
        eType |= RT_ABSPOS;
}

ScRangeData::ScRangeData(const ScRangeData& rScRangeData) :
    DataObject(),
    aName   (rScRangeData.aName),
    aUpperName  (rScRangeData.aUpperName),
    pCode       (rScRangeData.pCode ? rScRangeData.pCode->Clone() : new ScTokenArray),      // echte Kopie erzeugen (nicht copy-ctor)
    aPos        (rScRangeData.aPos),
    eType       (rScRangeData.eType),
    pDoc        (rScRangeData.pDoc),
    nIndex      (rScRangeData.nIndex),
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

BOOL ScRangeData::IsBeyond( SCROW nMaxRow ) const
{
    if ( aPos.Row() > nMaxRow )
        return TRUE;

    ScToken* t;
    pCode->Reset();
    while ( ( t = pCode->GetNextReference() ) != NULL )
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

    SCsCOL nMinCol = 0;
    SCsROW nMinRow = 0;
    SCsTAB nMinTab = 0;

    ScToken* t;
    pCode->Reset();
    while ( ( t = pCode->GetNextReference() ) != NULL )
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

    aPos = ScAddress( (SCCOL)(-nMinCol), (SCROW)(-nMinRow), (SCTAB)(-nMinTab) );

    //! Test
//  DBG_ERROR(String("Pos ")+String((SCCOL)(-nMinCol))+String("/")+
//          String((SCROW)(-nMinRow))+String("/")+String((SCTAB)(-nMinTab)));
}

void ScRangeData::GetSymbol( String& rSymbol, const ScGrammar::Grammar eGrammar ) const
{
    ScCompiler aScComp(pDoc, aPos, *pCode, eGrammar);
    aScComp.CreateStringFromTokenArray( rSymbol );
}

void ScRangeData::UpdateSymbol( String& rSymbol, const ScAddress& rPos,
                                const ScGrammar::Grammar eGrammar )
{
    ScTokenArray* pTemp = pCode->Clone();
    ScCompiler aComp( pDoc, rPos, *pTemp, eGrammar );
    aComp.MoveRelWrap();
    aComp.CreateStringFromTokenArray( rSymbol );
    delete pTemp;
}

void ScRangeData::UpdateSymbol( rtl::OUStringBuffer& rBuffer, const ScAddress& rPos,
                                const ScGrammar::Grammar eGrammar )
{
    ScTokenArray* pTemp = pCode->Clone();
    ScCompiler aComp( pDoc, rPos, *pTemp, eGrammar );
    aComp.MoveRelWrap();
    aComp.CreateStringFromTokenArray( rBuffer );
    delete pTemp;
}

void ScRangeData::UpdateReference(  UpdateRefMode eUpdateRefMode,
                                    const ScRange& r,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    BOOL bChanged = FALSE;

    pCode->Reset();
    if( pCode->GetNextReference() )
    {
        BOOL bSharedFormula = ((eType & RT_SHARED) == RT_SHARED);
        ScCompiler aComp( pDoc, aPos, *pCode );
        BOOL bRelRef = aComp.UpdateNameReference( eUpdateRefMode, r,
                                                    nDx, nDy, nDz,
                                                    bChanged, bSharedFormula);
        if (bSharedFormula)
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
            SingleDoubleRefModifier aMod( *t );
            ComplRefData& rRef = aMod.Ref();
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

void ScRangeData::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    BOOL bChanged = FALSE;

    ScToken* t;
    pCode->Reset();

    for( t = pCode->GetNextReference(); t; t = pCode->GetNextReference() )
    {
        if( t->GetType() != svIndex )
        {
            SingleDoubleRefModifier aMod( *t );
            ComplRefData& rRef = aMod.Ref();
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
    if ( eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) )
        if ( pCode )
            return pCode->IsReference( rRange );

    return FALSE;
}

BOOL ScRangeData::IsReference( ScRange& rRange, const ScAddress& rPos ) const
{
    if ( eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) )
        if ( pCode )
        {
            ::std::auto_ptr<ScTokenArray> pTemp( pCode->Clone() );
            ScCompiler aComp( pDoc, rPos, *pTemp );
            aComp.MoveRelWrap();
            return pTemp->IsReference( rRange );
        }

    return FALSE;
}

BOOL ScRangeData::IsValidReference( ScRange& rRange ) const
{
    if ( eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) )
        if ( pCode )
            return pCode->IsValidReference( rRange );

    return FALSE;
}

void ScRangeData::UpdateTabRef(SCTAB nOldTable, USHORT nFlag, SCTAB nNewTable)
{
    pCode->Reset();
    if( pCode->GetNextReference() )
    {
        ScRangeData* pRangeData = NULL;     // must not be dereferenced
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
    ScCompiler::InitSymbolsNative();

    //  ungueltige Zeichen vorne weglassen
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rName.Len();
    while ( nPos < nLen && !ScCompiler::IsWordChar( rName, nPos) )
        ++nPos;
    if ( nPos>0 )
        rName.Erase(0,nPos);

    //  wenn vorne ein ungueltiges Anfangszeichen steht, '_' davor
    if ( rName.Len() && !ScCompiler::IsCharWordChar( rName, 0 ) )
        rName.Insert('_',0);

    //  ungueltige durch '_' ersetzen
    nLen = rName.Len();
    for (nPos=0; nPos<nLen; nPos++)
    {
        if ( !ScCompiler::IsWordChar( rName, nPos) )
            rName.SetChar( nPos, '_' );
    }

    // Name darf keine Referenz beinhalten, wie in IsNameValid
    ScAddress aAddr;
        ScRange aRange;
    int nConv = ScAddress::CONV_UNSPECIFIED; // use int so that op++ works

    // Ensure that the proposed name is not an address under any convention
    while ( ++nConv != ScAddress::CONV_LAST )
        {
        ScAddress::Details details( static_cast<ScAddress::Convention>( nConv ) );
        while( aRange.Parse( rName, NULL, details )
               || aAddr.Parse( rName, NULL, details ) )
        {
            //! Range Parse auch bei Bereich mit ungueltigem Tabellennamen gueltig
            //! Address Parse dito, Name erzeugt deswegen bei Compile ein #REF!
            if ( rName.SearchAndReplace( ':', '_' ) == STRING_NOTFOUND
              && rName.SearchAndReplace( '.', '_' ) == STRING_NOTFOUND )
                rName.Insert('_',0);
        }
    }
}

BOOL ScRangeData::IsNameValid( const String& rName, ScDocument* pDoc )
{
    /*  If changed, ScfTools::ConvertToScDefinedName (sc/source/filter/ftools/ftools.cxx)
        needs to be changed too. */
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rName.Len();
    if ( !nLen || !ScCompiler::IsCharWordChar( rName, nPos++ ) )
        return FALSE;
    while ( nPos < nLen )
    {
        if ( !ScCompiler::IsWordChar( rName, nPos++ ) )
            return FALSE;
    }
    // Parse nicht auf VALID pruefen, es reicht, wenn irgendein Bestandteil
    // erkannt wurde
    ScRange aRange;
    if( aRange.Parse( rName, pDoc ) )   // THIS IS WRONG
        return FALSE;
    else
    {
        ScAddress aAddr;
        if ( aAddr.Parse( rName, pDoc ) )   // THIS IS WRONG
            return FALSE;
    }
    return TRUE;
}


USHORT ScRangeData::GetErrCode()
{
    return pCode ? pCode->GetCodeError() : 0;
}

BOOL ScRangeData::HasReferences() const
{
    pCode->Reset();
    return BOOL( pCode->GetNextReference() != NULL );
}

// bei TransferTab von einem in ein anderes Dokument anpassen,
// um Referenzen auf die eigene Tabelle mitzubekommen

void ScRangeData::TransferTabRef( SCTAB nOldTab, SCTAB nNewTab )
{
    long nTabDiff = (long)nNewTab - nOldTab;
    long nPosDiff = (long)nNewTab - aPos.Tab();
    aPos.SetTab( nNewTab );
    ScToken* t;
    pCode->Reset();
    while ( ( t = pCode->GetNextReference() ) != NULL )
    {
        SingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsTabRel() )
            rRef1.nTab = sal::static_int_cast<SCsTAB>( rRef1.nTab + nPosDiff );
        else
            rRef1.nTab = sal::static_int_cast<SCsTAB>( rRef1.nTab + nTabDiff );
        if ( t->GetType() == svDoubleRef )
        {
            SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
            if ( rRef2.IsTabRel() )
                rRef2.nTab = sal::static_int_cast<SCsTAB>( rRef2.nTab + nPosDiff );
            else
                rRef2.nTab = sal::static_int_cast<SCsTAB>( rRef2.nTab + nTabDiff );
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
            USHORT nOldIndex = p->GetIndex();
            USHORT nNewIndex = rMap.Find( nOldIndex );
            if ( nOldIndex != nNewIndex )
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


void ScRangeData::ValidateTabRefs()
{
    //  try to make sure all relative references and the reference position
    //  are within existing tables, so they can be represented as text
    //  (if the range of used tables is more than the existing tables,
    //  the result may still contain invalid tables, because the relative
    //  references aren't changed so formulas stay the same)

    //  find range of used tables

    SCTAB nMinTab = aPos.Tab();
    SCTAB nMaxTab = nMinTab;
    ScToken* t;
    pCode->Reset();
    while ( ( t = pCode->GetNextReference() ) != NULL )
    {
        SingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsTabRel() && !rRef1.IsTabDeleted() )
        {
            if ( rRef1.nTab < nMinTab )
                nMinTab = rRef1.nTab;
            if ( rRef1.nTab > nMaxTab )
                nMaxTab = rRef1.nTab;
        }
        if ( t->GetType() == svDoubleRef )
        {
            SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
            if ( rRef2.IsTabRel() && !rRef2.IsTabDeleted() )
            {
                if ( rRef2.nTab < nMinTab )
                    nMinTab = rRef2.nTab;
                if ( rRef2.nTab > nMaxTab )
                    nMaxTab = rRef2.nTab;
            }
        }
    }

    SCTAB nTabCount = pDoc->GetTableCount();
    if ( nMaxTab >= nTabCount && nMinTab > 0 )
    {
        //  move position and relative tab refs
        //  The formulas that use the name are not changed by this

        SCTAB nMove = nMinTab;
        aPos.SetTab( aPos.Tab() - nMove );

        pCode->Reset();
        while ( ( t = pCode->GetNextReference() ) != NULL )
        {
            SingleRefData& rRef1 = t->GetSingleRef();
            if ( rRef1.IsTabRel() && !rRef1.IsTabDeleted() )
                rRef1.nTab = sal::static_int_cast<SCsTAB>( rRef1.nTab - nMove );
            if ( t->GetType() == svDoubleRef )
            {
                SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
                if ( rRef2.IsTabRel() && !rRef2.IsTabDeleted() )
                    rRef2.nTab = sal::static_int_cast<SCsTAB>( rRef2.nTab - nMove );
            }
        }
    }
}


extern "C" int
#ifdef WNT
__cdecl
#endif
ScRangeData_QsortNameCompare( const void* p1, const void* p2 )
{
    return (int) ScGlobal::pCollator->compareString(
            (*(const ScRangeData**)p1)->GetName(),
            (*(const ScRangeData**)p2)->GetName() );
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

BOOL ScRangeName::SearchNameUpper( const String& rUpperName, USHORT& rIndex ) const
{
    // SearchNameUpper must be called with an upper-case search string

    USHORT i = 0;
    while (i < nCount)
    {
        if ( ((*this)[i])->GetUpperName() == rUpperName )
        {
            rIndex = i;
            return TRUE;
        }
        i++;
    }
    return FALSE;
}

BOOL ScRangeName::SearchName( const String& rName, USHORT& rIndex ) const
{
    if ( nCount > 0 )
        return SearchNameUpper( ScGlobal::pCharClass->upper( rName ), rIndex );
    else
        return FALSE;
}

void ScRangeName::UpdateReference(  UpdateRefMode eUpdateRefMode,
                                    const ScRange& rRange,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
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

void ScRangeName::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
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

void ScRangeName::UpdateTabRef(SCTAB nOldTable, USHORT nFlag, SCTAB nNewTable)
{
    for (USHORT i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateTabRef(nOldTable, nFlag, nNewTable);
}




