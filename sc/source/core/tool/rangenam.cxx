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
#include <memory>
#include <unotools/collatorwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

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
using ::std::pair;
using ::std::unary_function;
using ::rtl::OUString;

//========================================================================
// ScRangeData
//========================================================================

ScRangeData::ScRangeData( ScDocument* pDok,
                          const rtl::OUString& rName,
                          const String& rSymbol,
                          const ScAddress& rAddress,
                          RangeType nType,
                          const FormulaGrammar::Grammar eGrammar ) :
                aName       ( rName ),
                aUpperName  ( ScGlobal::pCharClass->uppercase( rName ) ),
                pCode       ( NULL ),
                aPos        ( rAddress ),
                eType       ( nType ),
                pDoc        ( pDok ),
                eTempGrammar( eGrammar ),
                nIndex      ( 0 ),
                bModified   ( false ),
                mnMaxRow    (-1),
                mnMaxCol    (-1)
{
    if (rSymbol.Len() > 0)
        CompileRangeData( rSymbol, pDoc->IsImportingXML());
        // Let the compiler set an error on unknown names for a subsequent
        // CompileUnresolvedXML().
    else
    {
        // #i63513#/#i65690# don't leave pCode as NULL.
        // Copy ctor default-constructs pCode if it was NULL, so it's initialized here, too,
        // to ensure same behavior if unnecessary copying is left out.

        pCode = new ScTokenArray();
    }
}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const rtl::OUString& rName,
                          const ScTokenArray& rArr,
                          const ScAddress& rAddress,
                          RangeType nType ) :
                aName       ( rName ),
                aUpperName  ( ScGlobal::pCharClass->uppercase( rName ) ),
                pCode       ( new ScTokenArray( rArr ) ),
                aPos        ( rAddress ),
                eType       ( nType ),
                pDoc        ( pDok ),
                eTempGrammar( FormulaGrammar::GRAM_UNSPECIFIED ),
                nIndex      ( 0 ),
                bModified   ( false ),
                mnMaxRow    (-1),
                mnMaxCol    (-1)
{
    if( !pCode->GetCodeError() )
    {
        pCode->Reset();
        FormulaToken* p = pCode->GetNextReference();
        if( p )   // exact one reference at first
        {
            if( p->GetType() == svSingleRef )
                eType = eType | RT_ABSPOS;
            else
                eType = eType | RT_ABSAREA;
        }
    }
}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const rtl::OUString& rName,
                          const ScAddress& rTarget ) :
                aName       ( rName ),
                aUpperName  ( ScGlobal::pCharClass->uppercase( rName ) ),
                pCode       ( new ScTokenArray() ),
                aPos        ( rTarget ),
                eType       ( RT_NAME ),
                pDoc        ( pDok ),
                eTempGrammar( FormulaGrammar::GRAM_UNSPECIFIED ),
                nIndex      ( 0 ),
                bModified   ( false ),
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

ScRangeData::ScRangeData(const ScRangeData& rScRangeData, ScDocument* pDocument) :
    aName   (rScRangeData.aName),
    aUpperName  (rScRangeData.aUpperName),
    pCode       (rScRangeData.pCode ? rScRangeData.pCode->Clone() : new ScTokenArray()),   // make real copy (not copy-ctor)
    aPos        (rScRangeData.aPos),
    eType       (rScRangeData.eType),
    pDoc        (pDocument ? pDocument : rScRangeData.pDoc),
    eTempGrammar(rScRangeData.eTempGrammar),
    nIndex      (rScRangeData.nIndex),
    bModified   (rScRangeData.bModified),
    mnMaxRow    (rScRangeData.mnMaxRow),
    mnMaxCol    (rScRangeData.mnMaxCol)
{}

ScRangeData::~ScRangeData()
{
    delete pCode;
}

void ScRangeData::CompileRangeData( const String& rSymbol, bool bSetError )
{
    if (eTempGrammar == FormulaGrammar::GRAM_UNSPECIFIED)
    {
        OSL_FAIL( "ScRangeData::CompileRangeData: unspecified grammar");
        // Anything is almost as bad as this, but we might have the best choice
        // if not loading documents.
        eTempGrammar = FormulaGrammar::GRAM_NATIVE;
    }

    ScCompiler aComp( pDoc, aPos );
    aComp.SetGrammar( eTempGrammar);
    if (bSetError)
        aComp.SetExtendedErrorDetection( ScCompiler::EXTENDED_ERROR_DETECTION_NAME_NO_BREAK);
    ScTokenArray* pNewCode = aComp.CompileString( rSymbol );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScTokenArray> pOldCode( pCode);     // old pCode will be deleted
    SAL_WNODEPRECATED_DECLARATIONS_POP
    pCode = pNewCode;
    if( !pCode->GetCodeError() )
    {
        pCode->Reset();
        FormulaToken* p = pCode->GetNextReference();
        if( p )
        {
            // first token is a reference
            /* FIXME: wouldn't that need a check if it's exactly one reference? */
            if( p->GetType() == svSingleRef )
                eType = eType | RT_ABSPOS;
            else
                eType = eType | RT_ABSAREA;
        }
        // For manual input set an error for an incomplete formula.
        if (!pDoc->IsImportingXML())
        {
            aComp.CompileTokenArray();
            pCode->DelRPN();
        }
    }
}

void ScRangeData::CompileUnresolvedXML()
{
    if (pCode->GetCodeError() == errNoName)
    {
        // Reconstruct the symbol/formula and then recompile.
        String aSymbol;
        ScCompiler aComp( pDoc, aPos, *pCode);
        aComp.SetGrammar( eTempGrammar);
        aComp.CreateStringFromTokenArray( aSymbol);
        // Don't let the compiler set an error for unknown names on final
        // compile, errors are handled by the interpreter thereafter.
        CompileRangeData( aSymbol, false);
    }
}

void ScRangeData::GuessPosition()
{
    // set a position that allows "absoluting" of all relative references
    // in CalcAbsIfRel without errors

    OSL_ENSURE(aPos == ScAddress(), "die Position geht jetzt verloren");

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
}

void ScRangeData::GetSymbol( String& rSymbol, const FormulaGrammar::Grammar eGrammar ) const
{
    ScCompiler aComp(pDoc, aPos, *pCode);
    aComp.SetGrammar(eGrammar);
    aComp.CreateStringFromTokenArray( rSymbol );
}

void ScRangeData::GetSymbol( OUString& rSymbol, const FormulaGrammar::Grammar eGrammar ) const
{
    String aStr;
    GetSymbol(aStr, eGrammar);
    rSymbol = aStr;
}

void ScRangeData::GetSymbol( OUString& rSymbol, const ScAddress& rPos, const FormulaGrammar::Grammar eGrammar ) const
{
    String aStr;
    ScCompiler aComp(pDoc, rPos, *pCode);
    aComp.SetGrammar(eGrammar);
    aComp.CreateStringFromTokenArray( aStr );
    rSymbol = aStr;
}

void ScRangeData::UpdateSymbol( rtl::OUStringBuffer& rBuffer, const ScAddress& rPos,
                                const FormulaGrammar::Grammar eGrammar )
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScTokenArray> pTemp( pCode->Clone() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    ScCompiler aComp( pDoc, rPos, *pTemp.get());
    aComp.SetGrammar(eGrammar);
    aComp.MoveRelWrap(GetMaxCol(), GetMaxRow());
    aComp.CreateStringFromTokenArray( rBuffer );
}

void ScRangeData::UpdateReference(  UpdateRefMode eUpdateRefMode,
                                    const ScRange& r,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bLocal )
{
    bool bChanged = false;

    pCode->Reset();
    if( pCode->GetNextReference() )
    {
        bool bSharedFormula = ((eType & RT_SHARED) == RT_SHARED);
        ScCompiler aComp( pDoc, aPos, *pCode );
        aComp.SetGrammar(pDoc->GetGrammar());
        const bool bRelRef = aComp.UpdateNameReference( eUpdateRefMode, r,
                                                    nDx, nDy, nDz,
                                                    bChanged, bSharedFormula, bLocal);
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
    bool bChanged = false;

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
                    bChanged = true;
            }
        }
    }

    bModified = bChanged;
}

void ScRangeData::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    bool bChanged = false;

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
                    bChanged = true;
            }
        }
    }

    bModified = bChanged;           // has to be evaluated immediately afterwards
}

bool ScRangeData::operator== (const ScRangeData& rData) const       // for Undo
{
    if ( nIndex != rData.nIndex ||
         aName  != rData.aName  ||
         aPos   != rData.aPos   ||
         eType  != rData.eType     ) return false;

    sal_uInt16 nLen = pCode->GetLen();
    if ( nLen != rData.pCode->GetLen() ) return false;

    FormulaToken** ppThis = pCode->GetArray();
    FormulaToken** ppOther = rData.pCode->GetArray();

    for ( sal_uInt16 i=0; i<nLen; i++ )
        if ( ppThis[i] != ppOther[i] && !(*ppThis[i] == *ppOther[i]) )
            return false;

    return true;
}


bool ScRangeData::IsRangeAtBlock( const ScRange& rBlock ) const
{
    bool bRet = false;
    ScRange aRange;
    if ( IsReference(aRange) )
        bRet = ( rBlock == aRange );
    return bRet;
}

bool ScRangeData::IsReference( ScRange& rRange ) const
{
    if ( (eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS )) && pCode )
        return pCode->IsReference( rRange );

    return false;
}

bool ScRangeData::IsReference( ScRange& rRange, const ScAddress& rPos ) const
{
    if ( (eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) ) && pCode )
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScTokenArray> pTemp( pCode->Clone() );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        ScCompiler aComp( pDoc, rPos, *pTemp);
        aComp.SetGrammar(pDoc->GetGrammar());
        aComp.MoveRelWrap(MAXCOL, MAXROW);
        return pTemp->IsReference( rRange );
    }

    return false;
}

bool ScRangeData::IsValidReference( ScRange& rRange ) const
{
    if ( (eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) ) && pCode )
        return pCode->IsValidReference( rRange );

    return false;
}

void ScRangeData::UpdateTabRef(SCTAB nOldTable, sal_uInt16 nFlag, SCTAB nNewTable, SCTAB nNewSheets)
{
    pCode->Reset();
    if( pCode->GetNextReference() )
    {
        ScRangeData* pRangeData = NULL;     // must not be dereferenced
        bool bChanged;
        ScCompiler aComp( pDoc, aPos, *pCode);
        aComp.SetGrammar(pDoc->GetGrammar());
        switch (nFlag)
        {
            case 1:                                     // simple InsertTab (doc.cxx)
                pRangeData = aComp.UpdateInsertTab(nOldTable, true, nNewSheets );   // und CopyTab (doc2.cxx)
                break;
            case 2:                                     // simple delete (doc.cxx)
                pRangeData = aComp.UpdateDeleteTab(nOldTable, false, true, bChanged);
                break;
            case 3:                                     // move (doc2.cxx)
            {
                pRangeData = aComp.UpdateMoveTab(nOldTable, nNewTable, true );
            }
                break;
            default:
            {
                OSL_FAIL("ScRangeName::UpdateTabRef: Unknown Flag");
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


void ScRangeData::MakeValidName( String& rName )
{

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

bool ScRangeData::IsNameValid( const String& rName, ScDocument* pDoc )
{
    /* XXX If changed, sc/source/filter/ftools/ftools.cxx
     * ScfTools::ConvertToScDefinedName needs to be changed too. */
    sal_Char a('.');
    if (rName.Search(a, 0) != STRING_NOTFOUND)
        return false;
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rName.Len();
    if ( !nLen || !ScCompiler::IsCharFlagAllConventions( rName, nPos++, SC_COMPILER_C_CHAR_NAME ) )
        return false;
    while ( nPos < nLen )
    {
        if ( !ScCompiler::IsCharFlagAllConventions( rName, nPos++, SC_COMPILER_C_NAME ) )
            return false;
    }
    ScAddress aAddr;
    ScRange aRange;
    for (int nConv = FormulaGrammar::CONV_UNSPECIFIED; ++nConv < FormulaGrammar::CONV_LAST; )
    {
        ScAddress::Details details( static_cast<FormulaGrammar::AddressConvention>( nConv ) );
        // Don't check Parse on VALID, any partial only VALID may result in
        // #REF! during compile later!
        if (aRange.Parse( rName, pDoc, details) || aAddr.Parse( rName, pDoc, details))
            return false;
    }
    return true;
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


sal_uInt16 ScRangeData::GetErrCode() const
{
    return pCode ? pCode->GetCodeError() : 0;
}

bool ScRangeData::HasReferences() const
{
    pCode->Reset();
    return pCode->GetNextReference() != NULL;
}

sal_uInt32 ScRangeData::GetUnoType() const
{
    sal_uInt32 nUnoType = 0;
    if ( HasType(RT_CRITERIA) )  nUnoType |= com::sun::star::sheet::NamedRangeFlag::FILTER_CRITERIA;
    if ( HasType(RT_PRINTAREA) ) nUnoType |= com::sun::star::sheet::NamedRangeFlag::PRINT_AREA;
    if ( HasType(RT_COLHEADER) ) nUnoType |= com::sun::star::sheet::NamedRangeFlag::COLUMN_HEADER;
    if ( HasType(RT_ROWHEADER) ) nUnoType |= com::sun::star::sheet::NamedRangeFlag::ROW_HEADER;
    return nUnoType;
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


extern "C"
int SAL_CALL ScRangeData_QsortNameCompare( const void* p1, const void* p2 )
{
    return (int) ScGlobal::GetCollator()->compareString(
            (*(const ScRangeData**)p1)->GetName(),
            (*(const ScRangeData**)p2)->GetName() );
}

bool operator<(const ScRangeData& left, const ScRangeData& right)
{
    return left.GetName() < right.GetName();
}

namespace {

/**
 * Predicate to check if the name references the specified range.
 */
class MatchByRange : public unary_function<ScRangeData, bool>
{
    const ScRange& mrRange;
public:
    MatchByRange(const ScRange& rRange) : mrRange(rRange) {}
    template < typename Pair >
    bool operator() ( Pair const& r) const
    {
        return r.second->IsRangeAtBlock(mrRange);
    }
};

class MatchByIndex : public unary_function<ScRangeData, bool>
{
    sal_uInt16 mnIndex;
public:
    MatchByIndex(sal_uInt16 nIndex) : mnIndex(nIndex) {}
    bool operator() (const ScRangeData& r) const
    {
        return mnIndex == r.GetIndex();
    }
};

}

ScRangeName::ScRangeName() {}

ScRangeName::ScRangeName(const ScRangeName& r) :
    maData(r.maData)
{
    // boost::ptr_set clones and deletes, so each collection needs its own
    // index to data.
    maIndexToData.resize( r.maIndexToData.size(), NULL);
    DataType::const_iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
    {
        size_t nPos = itr->second->GetIndex() - 1;
        if (nPos >= maIndexToData.size())
        {
            OSL_FAIL( "ScRangeName copy-ctor: maIndexToData size doesn't fit");
            maIndexToData.resize(nPos+1, NULL);
        }
        maIndexToData[nPos] = const_cast<ScRangeData*>(itr->second);
    }
}

const ScRangeData* ScRangeName::findByRange(const ScRange& rRange) const
{
    DataType::const_iterator itr = std::find_if(
        maData.begin(), maData.end(), MatchByRange(rRange));
    return itr == maData.end() ? NULL : itr->second;
}

ScRangeData* ScRangeName::findByUpperName(const OUString& rName)
{
    DataType::iterator itr = maData.find(rName);
    return itr == maData.end() ? NULL : itr->second;
}

const ScRangeData* ScRangeName::findByUpperName(const OUString& rName) const
{
    DataType::const_iterator itr = maData.find(rName);
    return itr == maData.end() ? NULL : itr->second;
}

ScRangeData* ScRangeName::findByIndex(sal_uInt16 i) const
{
    if (!i)
        // index should never be zero.
        return NULL;

    size_t nPos = i - 1;
    return nPos < maIndexToData.size() ? maIndexToData[nPos] : NULL;
}

void ScRangeName::UpdateReference(
    UpdateRefMode eUpdateRefMode, const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bLocal)
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->UpdateReference(eUpdateRefMode, rRange, nDx, nDy, nDz, bLocal);
}

void ScRangeName::UpdateTabRef(SCTAB nTable, sal_uInt16 nFlag, SCTAB nNewTable, SCTAB nNewSheets)
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->UpdateTabRef(nTable, nFlag, nNewTable, nNewSheets);
}

void ScRangeName::UpdateTranspose(const ScRange& rSource, const ScAddress& rDest)
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->UpdateTranspose(rSource, rDest);
}

void ScRangeName::UpdateGrow(const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY)
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->UpdateGrow(rArea, nGrowX, nGrowY);
}

void ScRangeName::CompileUnresolvedXML()
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->CompileUnresolvedXML();
}

ScRangeName::const_iterator ScRangeName::begin() const
{
    return maData.begin();
}

ScRangeName::const_iterator ScRangeName::end() const
{
    return maData.end();
}

ScRangeName::iterator ScRangeName::begin()
{
    return maData.begin();
}

ScRangeName::iterator ScRangeName::end()
{
    return maData.end();
}

size_t ScRangeName::size() const
{
    return maData.size();
}

bool ScRangeName::empty() const
{
    return maData.empty();
}

bool ScRangeName::insert(ScRangeData* p)
{
    if (!p)
        return false;

    if (!p->GetIndex())
    {
        // Assign a new index.  An index must be unique and is never 0.
        IndexDataType::iterator itr = std::find(
            maIndexToData.begin(), maIndexToData.end(), static_cast<ScRangeData*>(NULL));
        if (itr != maIndexToData.end())
        {
            // Empty slot exists.  Re-use it.
            size_t nPos = std::distance(maIndexToData.begin(), itr);
            p->SetIndex(nPos + 1);
        }
        else
            // No empty slot.  Append it to the end.
            p->SetIndex(maIndexToData.size() + 1);
    }

    rtl::OUString aName(p->GetUpperName());
    erase(aName); // ptr_map won't insert it if a duplicate name exists.
    pair<DataType::iterator, bool> r = maData.insert(aName, p);
    if (r.second)
    {
        // Data inserted.  Store its index for mapping.
        size_t nPos = p->GetIndex() - 1;
        if (nPos >= maIndexToData.size())
            maIndexToData.resize(nPos+1, NULL);
        maIndexToData[nPos] = p;
    }
    return r.second;
}

void ScRangeName::erase(const ScRangeData& r)
{
    erase(r.GetUpperName());
}

void ScRangeName::erase(const rtl::OUString& rName)
{
    DataType::iterator itr = maData.find(rName);
    if (itr != maData.end())
        erase(itr);
}

void ScRangeName::erase(const iterator& itr)
{
    sal_uInt16 nIndex = itr->second->GetIndex();
    maData.erase(itr);
    OSL_ENSURE( 0 < nIndex && nIndex <= maIndexToData.size(), "ScRangeName::erase: bad index");
    if (0 < nIndex && nIndex <= maIndexToData.size())
        maIndexToData[nIndex-1] = NULL;
}

void ScRangeName::clear()
{
    maData.clear();
    maIndexToData.clear();
}

bool ScRangeName::operator== (const ScRangeName& r) const
{
    return maData == r.maData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
