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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


//------------------------------------------------------------------------

#include <tools/debug.hxx>
#include <string.h>
#include <memory>
#include <unotools/collatorwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "token.hxx"
#include "tokenarray.hxx"
#include "rangenam.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "rangeutl.hxx"
#include "rechead.hxx"
#include "refupdat.hxx"
#include "document.hxx"

using namespace formula;

//========================================================================
// ScRangeData
//========================================================================

// Interner ctor fuer das Suchen nach einem Index

ScRangeData::ScRangeData( sal_uInt16 n )
           : pCode( NULL ), nIndex( n ), bModified( sal_False ), mnMaxRow(-1), mnMaxCol(-1)
{}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const String& rName,
                          const String& rSymbol,
                          const ScAddress& rAddress,
                          RangeType nType,
                          const FormulaGrammar::Grammar eGrammar ) :
                aName       ( rName ),
                aUpperName  ( ScGlobal::pCharClass->upper( rName ) ),
                pCode       ( NULL ),
                aPos        ( rAddress ),
                eType       ( nType ),
                pDoc        ( pDok ),
                nIndex      ( 0 ),
                bModified   ( sal_False ),
                mnMaxRow    (-1),
                mnMaxCol    (-1)
{
    if (rSymbol.Len() > 0)
    {
        ScCompiler aComp( pDoc, aPos );
        aComp.SetGrammar(eGrammar);
        pCode = aComp.CompileString( rSymbol );
        if( !pCode->GetCodeError() )
        {
            pCode->Reset();
            FormulaToken* p = pCode->GetNextReference();
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

        pCode = new ScTokenArray();
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
                bModified   ( sal_False ),
                mnMaxRow    (-1),
                mnMaxCol    (-1)
{
    if( !pCode->GetCodeError() )
    {
        pCode->Reset();
        FormulaToken* p = pCode->GetNextReference();
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
                pCode       ( new ScTokenArray() ),
                aPos        ( rTarget ),
                eType       ( RT_NAME ),
                pDoc        ( pDok ),
                nIndex      ( 0 ),
                bModified   ( sal_False ),
                mnMaxRow    (-1),
                mnMaxCol    (-1)
{
    ScSingleRefData aRefData;
    aRefData.InitAddress( rTarget );
    aRefData.SetFlag3D( sal_True );
    pCode->AddSingleReference( aRefData );
    ScCompiler aComp( pDoc, aPos, *pCode );
    aComp.SetGrammar(pDoc->GetGrammar());
    aComp.CompileTokenArray();
    if ( !pCode->GetCodeError() )
        eType |= RT_ABSPOS;
}

ScRangeData::ScRangeData(const ScRangeData& rScRangeData) :
    ScDataObject(),
    aName   (rScRangeData.aName),
    aUpperName  (rScRangeData.aUpperName),
    pCode       (rScRangeData.pCode ? rScRangeData.pCode->Clone() : new ScTokenArray()),        // echte Kopie erzeugen (nicht copy-ctor)
    aPos        (rScRangeData.aPos),
    eType       (rScRangeData.eType),
    pDoc        (rScRangeData.pDoc),
    nIndex      (rScRangeData.nIndex),
    bModified   (rScRangeData.bModified),
    mnMaxRow    (rScRangeData.mnMaxRow),
    mnMaxCol    (rScRangeData.mnMaxCol)
{}

ScRangeData::~ScRangeData()
{
    delete pCode;
}

ScDataObject* ScRangeData::Clone() const
{
    return new ScRangeData(*this);
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
    while ( ( t = static_cast<ScToken*>(pCode->GetNextReference()) ) != NULL )
    {
        ScSingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsColRel() && rRef1.nRelCol < nMinCol )
            nMinCol = rRef1.nRelCol;
        if ( rRef1.IsRowRel() && rRef1.nRelRow < nMinRow )
            nMinRow = rRef1.nRelRow;
        if ( rRef1.IsTabRel() && rRef1.nRelTab < nMinTab )
            nMinTab = rRef1.nRelTab;

        if ( t->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = t->GetDoubleRef().Ref2;
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

void ScRangeData::GetSymbol( String& rSymbol, const FormulaGrammar::Grammar eGrammar ) const
{
    ScCompiler aComp(pDoc, aPos, *pCode);
    aComp.SetGrammar(eGrammar);
    aComp.CreateStringFromTokenArray( rSymbol );
}

void ScRangeData::UpdateSymbol( rtl::OUStringBuffer& rBuffer, const ScAddress& rPos,
                                const FormulaGrammar::Grammar eGrammar )
{
    ::std::auto_ptr<ScTokenArray> pTemp( pCode->Clone() );
    ScCompiler aComp( pDoc, rPos, *pTemp.get());
    aComp.SetGrammar(eGrammar);
    aComp.MoveRelWrap(GetMaxCol(), GetMaxRow());
    aComp.CreateStringFromTokenArray( rBuffer );
}

void ScRangeData::UpdateReference(  UpdateRefMode eUpdateRefMode,
                                    const ScRange& r,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    sal_Bool bChanged = sal_False;

    pCode->Reset();
    if( pCode->GetNextReference() )
    {
        sal_Bool bSharedFormula = ((eType & RT_SHARED) == RT_SHARED);
        ScCompiler aComp( pDoc, aPos, *pCode );
        aComp.SetGrammar(pDoc->GetGrammar());
        const sal_Bool bRelRef = aComp.UpdateNameReference( eUpdateRefMode, r,
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
    sal_Bool bChanged = sal_False;

    ScToken* t;
    pCode->Reset();

    while ( ( t = static_cast<ScToken*>(pCode->GetNextReference()) ) != NULL )
    {
        if( t->GetType() != svIndex )
        {
            SingleDoubleRefModifier aMod( *t );
            ScComplexRefData& rRef = aMod.Ref();
            if (!rRef.Ref1.IsColRel() && !rRef.Ref1.IsRowRel() &&
                    (!rRef.Ref1.IsFlag3D() || !rRef.Ref1.IsTabRel()) &&
                ( t->GetType() == svSingleRef ||
                (!rRef.Ref2.IsColRel() && !rRef.Ref2.IsRowRel() &&
                    (!rRef.Ref2.IsFlag3D() || !rRef.Ref2.IsTabRel()))))
            {
                if ( ScRefUpdate::UpdateTranspose( pDoc, rSource, rDest, rRef ) != UR_NOTHING )
                    bChanged = sal_True;
            }
        }
    }

    bModified = bChanged;
}

void ScRangeData::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    sal_Bool bChanged = sal_False;

    ScToken* t;
    pCode->Reset();

    while ( ( t = static_cast<ScToken*>(pCode->GetNextReference()) ) != NULL )
    {
        if( t->GetType() != svIndex )
        {
            SingleDoubleRefModifier aMod( *t );
            ScComplexRefData& rRef = aMod.Ref();
            if (!rRef.Ref1.IsColRel() && !rRef.Ref1.IsRowRel() &&
                    (!rRef.Ref1.IsFlag3D() || !rRef.Ref1.IsTabRel()) &&
                ( t->GetType() == svSingleRef ||
                (!rRef.Ref2.IsColRel() && !rRef.Ref2.IsRowRel() &&
                    (!rRef.Ref2.IsFlag3D() || !rRef.Ref2.IsTabRel()))))
            {
                if ( ScRefUpdate::UpdateGrow( rArea,nGrowX,nGrowY, rRef ) != UR_NOTHING )
                    bChanged = sal_True;
            }
        }
    }

    bModified = bChanged;           // muss direkt hinterher ausgewertet werden
}

sal_Bool ScRangeData::operator== (const ScRangeData& rData) const       // fuer Undo
{
    if ( nIndex != rData.nIndex ||
         aName  != rData.aName  ||
         aPos   != rData.aPos   ||
         eType  != rData.eType     ) return sal_False;

    sal_uInt16 nLen = pCode->GetLen();
    if ( nLen != rData.pCode->GetLen() ) return sal_False;

    FormulaToken** ppThis = pCode->GetArray();
    FormulaToken** ppOther = rData.pCode->GetArray();

    for ( sal_uInt16 i=0; i<nLen; i++ )
        if ( ppThis[i] != ppOther[i] && !(*ppThis[i] == *ppOther[i]) )
            return sal_False;

    return sal_True;
}

//UNUSED2009-05 sal_Bool ScRangeData::IsRangeAtCursor( const ScAddress& rPos, sal_Bool bStartOnly ) const
//UNUSED2009-05 {
//UNUSED2009-05     sal_Bool bRet = sal_False;
//UNUSED2009-05     ScRange aRange;
//UNUSED2009-05     if ( IsReference(aRange) )
//UNUSED2009-05     {
//UNUSED2009-05         if ( bStartOnly )
//UNUSED2009-05             bRet = ( rPos == aRange.aStart );
//UNUSED2009-05         else
//UNUSED2009-05             bRet = ( aRange.In( rPos ) );
//UNUSED2009-05     }
//UNUSED2009-05     return bRet;
//UNUSED2009-05 }

sal_Bool ScRangeData::IsRangeAtBlock( const ScRange& rBlock ) const
{
    sal_Bool bRet = sal_False;
    ScRange aRange;
    if ( IsReference(aRange) )
        bRet = ( rBlock == aRange );
    return bRet;
}

sal_Bool ScRangeData::IsReference( ScRange& rRange ) const
{
    if ( (eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS )) && pCode )
        return pCode->IsReference( rRange );

    return sal_False;
}

sal_Bool ScRangeData::IsReference( ScRange& rRange, const ScAddress& rPos ) const
{
    if ( (eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) ) && pCode )
    {
        ::std::auto_ptr<ScTokenArray> pTemp( pCode->Clone() );
        ScCompiler aComp( pDoc, rPos, *pTemp);
        aComp.SetGrammar(pDoc->GetGrammar());
        aComp.MoveRelWrap(MAXCOL, MAXROW);
        return pTemp->IsReference( rRange );
    }

    return sal_False;
}

sal_Bool ScRangeData::IsValidReference( ScRange& rRange ) const
{
    if ( (eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) ) && pCode )
        return pCode->IsValidReference( rRange );

    return sal_False;
}

void ScRangeData::UpdateTabRef(SCTAB nOldTable, sal_uInt16 nFlag, SCTAB nNewTable)
{
    pCode->Reset();
    if( pCode->GetNextReference() )
    {
        ScRangeData* pRangeData = NULL;     // must not be dereferenced
        sal_Bool bChanged;
        ScCompiler aComp( pDoc, aPos, *pCode);
        aComp.SetGrammar(pDoc->GetGrammar());
        switch (nFlag)
        {
            case 1:                                     // einfache InsertTab (doc.cxx)
                pRangeData = aComp.UpdateInsertTab(nOldTable, sal_True );   // und CopyTab (doc2.cxx)
                break;
            case 2:                                     // einfaches delete (doc.cxx)
                pRangeData = aComp.UpdateDeleteTab(nOldTable, sal_False, sal_True, bChanged);
                break;
            case 3:                                     // move (doc2.cxx)
            {
                pRangeData = aComp.UpdateMoveTab(nOldTable, nNewTable, sal_True );
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


void ScRangeData::MakeValidName( String& rName )        // static
{
    //ScCompiler::InitSymbolsNative();

    // strip leading invalid characters
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rName.Len();
    while ( nPos < nLen && !ScCompiler::IsCharFlagAllConventions( rName, nPos, SC_COMPILER_C_NAME) )
        ++nPos;
    if ( nPos>0 )
        rName.Erase(0,nPos);

    // if the first character is an invalid start character, precede with '_'
    if ( rName.Len() && !ScCompiler::IsCharFlagAllConventions( rName, 0, SC_COMPILER_C_CHAR_NAME ) )
        rName.Insert('_',0);

    // replace invalid with '_'
    nLen = rName.Len();
    for (nPos=0; nPos<nLen; nPos++)
    {
        if ( !ScCompiler::IsCharFlagAllConventions( rName, nPos, SC_COMPILER_C_NAME) )
            rName.SetChar( nPos, '_' );
    }

    // Ensure that the proposed name is not a reference under any convention,
    // same as in IsNameValid()
    ScAddress aAddr;
    ScRange aRange;
    for (int nConv = FormulaGrammar::CONV_UNSPECIFIED; ++nConv < FormulaGrammar::CONV_LAST; )
    {
        ScAddress::Details details( static_cast<FormulaGrammar::AddressConvention>( nConv ) );
        // Don't check Parse on VALID, any partial only VALID may result in
        // #REF! during compile later!
        while (aRange.Parse( rName, NULL, details) || aAddr.Parse( rName, NULL, details))
        {
            //! Range Parse is partially valid also with invalid sheet name,
            //! Address Parse dito, during compile name would generate a #REF!
            if ( rName.SearchAndReplace( '.', '_' ) == STRING_NOTFOUND )
                rName.Insert('_',0);
        }
    }
}

sal_Bool ScRangeData::IsNameValid( const String& rName, ScDocument* pDoc )
{
    /* XXX If changed, sc/source/filter/ftools/ftools.cxx
     * ScfTools::ConvertToScDefinedName needs to be changed too. */
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rName.Len();
    if ( !nLen || !ScCompiler::IsCharFlagAllConventions( rName, nPos++, SC_COMPILER_C_CHAR_NAME ) )
        return sal_False;
    while ( nPos < nLen )
    {
        if ( !ScCompiler::IsCharFlagAllConventions( rName, nPos++, SC_COMPILER_C_NAME ) )
            return sal_False;
    }
    ScAddress aAddr;
    ScRange aRange;
    for (int nConv = FormulaGrammar::CONV_UNSPECIFIED; ++nConv < FormulaGrammar::CONV_LAST; )
    {
        ScAddress::Details details( static_cast<FormulaGrammar::AddressConvention>( nConv ) );
        // Don't check Parse on VALID, any partial only VALID may result in
        // #REF! during compile later!
        if (aRange.Parse( rName, pDoc, details) || aAddr.Parse( rName, pDoc, details))
            return sal_False;
    }
    return sal_True;
}

void ScRangeData::SetMaxRow(SCROW nRow)
{
    mnMaxRow = nRow;
}

SCROW ScRangeData::GetMaxRow() const
{
    return mnMaxRow >= 0 ? mnMaxRow : MAXROW;
}

void ScRangeData::SetMaxCol(SCCOL nCol)
{
    mnMaxCol = nCol;
}

SCCOL ScRangeData::GetMaxCol() const
{
    return mnMaxCol >= 0 ? mnMaxCol : MAXCOL;
}


sal_uInt16 ScRangeData::GetErrCode()
{
    return pCode ? pCode->GetCodeError() : 0;
}

sal_Bool ScRangeData::HasReferences() const
{
    pCode->Reset();
    return sal_Bool( pCode->GetNextReference() != NULL );
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
    while ( ( t = static_cast<ScToken*>(pCode->GetNextReference()) ) != NULL )
    {
        ScSingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsTabRel() )
            rRef1.nTab = sal::static_int_cast<SCsTAB>( rRef1.nTab + nPosDiff );
        else
            rRef1.nTab = sal::static_int_cast<SCsTAB>( rRef1.nTab + nTabDiff );
        if ( t->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = t->GetDoubleRef().Ref2;
            if ( rRef2.IsTabRel() )
                rRef2.nTab = sal::static_int_cast<SCsTAB>( rRef2.nTab + nPosDiff );
            else
                rRef2.nTab = sal::static_int_cast<SCsTAB>( rRef2.nTab + nTabDiff );
        }
    }
}

void ScRangeData::ReplaceRangeNamesInUse( const IndexMap& rMap )
{
    bool bCompile = false;
    for ( FormulaToken* p = pCode->First(); p; p = pCode->Next() )
    {
        if ( p->GetOpCode() == ocName )
        {
            const sal_uInt16 nOldIndex = p->GetIndex();
            IndexMap::const_iterator itr = rMap.find(nOldIndex);
            const sal_uInt16 nNewIndex = itr == rMap.end() ? nOldIndex : itr->second;
            if ( nOldIndex != nNewIndex )
            {
                p->SetIndex( nNewIndex );
                bCompile = true;
            }
        }
    }
    if ( bCompile )
    {
        ScCompiler aComp( pDoc, aPos, *pCode);
        aComp.SetGrammar(pDoc->GetGrammar());
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
    while ( ( t = static_cast<ScToken*>(pCode->GetNextReference()) ) != NULL )
    {
        ScSingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsTabRel() && !rRef1.IsTabDeleted() )
        {
            if ( rRef1.nTab < nMinTab )
                nMinTab = rRef1.nTab;
            if ( rRef1.nTab > nMaxTab )
                nMaxTab = rRef1.nTab;
        }
        if ( t->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = t->GetDoubleRef().Ref2;
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
        while ( ( t = static_cast<ScToken*>(pCode->GetNextReference()) ) != NULL )
        {
            ScSingleRefData& rRef1 = t->GetSingleRef();
            if ( rRef1.IsTabRel() && !rRef1.IsTabDeleted() )
                rRef1.nTab = sal::static_int_cast<SCsTAB>( rRef1.nTab - nMove );
            if ( t->GetType() == svDoubleRef )
            {
                ScSingleRefData& rRef2 = t->GetDoubleRef().Ref2;
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
    return (int) ScGlobal::GetCollator()->compareString(
            (*(const ScRangeData**)p1)->GetName(),
            (*(const ScRangeData**)p2)->GetName() );
}


//========================================================================
// ScRangeName
//========================================================================

ScRangeName::ScRangeName(const ScRangeName& rScRangeName, ScDocument* pDocument) :
                ScSortedCollection ( rScRangeName ),
                pDoc ( pDocument ),
                nSharedMaxIndex (rScRangeName.nSharedMaxIndex)
{
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        ((ScRangeData*)At(i))->SetDocument(pDocument);
        ((ScRangeData*)At(i))->SetIndex(((ScRangeData*)rScRangeName.At(i))->GetIndex());
    }
}

short ScRangeName::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    sal_uInt16 i1 = ((ScRangeData*)pKey1)->GetIndex();
    sal_uInt16 i2 = ((ScRangeData*)pKey2)->GetIndex();
    return (short) i1 - (short) i2;
}

sal_Bool ScRangeName::SearchNameUpper( const String& rUpperName, sal_uInt16& rIndex ) const
{
    // SearchNameUpper must be called with an upper-case search string

    sal_uInt16 i = 0;
    while (i < nCount)
    {
        if ( ((*this)[i])->GetUpperName() == rUpperName )
        {
            rIndex = i;
            return sal_True;
        }
        i++;
    }
    return sal_False;
}

sal_Bool ScRangeName::SearchName( const String& rName, sal_uInt16& rIndex ) const
{
    if ( nCount > 0 )
        return SearchNameUpper( ScGlobal::pCharClass->upper( rName ), rIndex );
    else
        return sal_False;
}

void ScRangeName::UpdateReference(  UpdateRefMode eUpdateRefMode,
                                    const ScRange& rRange,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    for (sal_uInt16 i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateReference(eUpdateRefMode, rRange,
                                                   nDx, nDy, nDz);
}

void ScRangeName::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest )
{
    for (sal_uInt16 i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateTranspose( rSource, rDest );
}

void ScRangeName::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    for (sal_uInt16 i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateGrow( rArea, nGrowX, nGrowY );
}

sal_Bool ScRangeName::IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return *(ScRangeData*)pKey1 == *(ScRangeData*)pKey2;
}

sal_Bool ScRangeName::Insert(ScDataObject* pScDataObject)
{
    if (!((ScRangeData*)pScDataObject)->GetIndex())     // schon gesetzt?
    {
        ((ScRangeData*)pScDataObject)->SetIndex( GetEntryIndex() );
    }

    return ScSortedCollection::Insert(pScDataObject);
}

// Suche nach einem freien Index

sal_uInt16 ScRangeName::GetEntryIndex()
{
    sal_uInt16 nLast = 0;
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        sal_uInt16 nIdx = ((ScRangeData*)pItems[i])->GetIndex();
        if( nIdx > nLast )
        {
            nLast = nIdx;
        }
    }
    return nLast + 1;
}

ScRangeData* ScRangeName::FindIndex( sal_uInt16 nIndex )
{
    ScRangeData aDataObj( nIndex );
    sal_uInt16 n;
    if( Search( &aDataObj, n ) )
        return (*this)[ n ];
    else
        return NULL;
}

//UNUSED2009-05 ScRangeData* ScRangeName::GetRangeAtCursor( const ScAddress& rPos, sal_Bool bStartOnly ) const
//UNUSED2009-05 {
//UNUSED2009-05     if ( pItems )
//UNUSED2009-05     {
//UNUSED2009-05         for ( sal_uInt16 i = 0; i < nCount; i++ )
//UNUSED2009-05             if ( ((ScRangeData*)pItems[i])->IsRangeAtCursor( rPos, bStartOnly ) )
//UNUSED2009-05                 return (ScRangeData*)pItems[i];
//UNUSED2009-05     }
//UNUSED2009-05     return NULL;
//UNUSED2009-05 }

ScRangeData* ScRangeName::GetRangeAtBlock( const ScRange& rBlock ) const
{
    if ( pItems )
    {
        for ( sal_uInt16 i = 0; i < nCount; i++ )
            if ( ((ScRangeData*)pItems[i])->IsRangeAtBlock( rBlock ) )
                return (ScRangeData*)pItems[i];
    }
    return NULL;
}

void ScRangeName::UpdateTabRef(SCTAB nOldTable, sal_uInt16 nFlag, SCTAB nNewTable)
{
    for (sal_uInt16 i=0; i<nCount; i++)
        ((ScRangeData*)pItems[i])->UpdateTabRef(nOldTable, nFlag, nNewTable);
}




