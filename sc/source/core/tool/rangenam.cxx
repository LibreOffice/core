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

#include <string.h>
#include <memory>
#include <unotools/collatorwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <osl/diagnose.h>

#include "token.hxx"
#include "tokenarray.hxx"
#include "rangenam.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "rangeutl.hxx"
#include "rechead.hxx"
#include "refupdat.hxx"
#include "document.hxx"
#include "refupdatecontext.hxx"
#include <tokenstringcontext.hxx>

#include <formula/errorcodes.hxx>

using namespace formula;
using ::std::pair;
using ::std::unary_function;

// ScRangeData

ScRangeData::ScRangeData( ScDocument* pDok,
                          const OUString& rName,
                          const OUString& rSymbol,
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
    if (!rSymbol.isEmpty())
        CompileRangeData( rSymbol, pDoc->IsImportingXML());
        // Let the compiler set an error on unknown names for a subsequent
        // CompileUnresolvedXML().
    else
    {
        // #i63513#/#i65690# don't leave pCode as NULL.
        // Copy ctor default-constructs pCode if it was NULL, so it's initialized here, too,
        // to ensure same behavior if unnecessary copying is left out.

        pCode = new ScTokenArray();
        pCode->SetFromRangeName(true);
    }
}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const OUString& rName,
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
    pCode->SetFromRangeName(true);
    InitCode();
}

ScRangeData::ScRangeData( ScDocument* pDok,
                          const OUString& rName,
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
    aRefData.SetFlag3D( true );
    pCode->AddSingleReference( aRefData );
    pCode->SetFromRangeName(true);
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
{
    pCode->SetFromRangeName(true);
}

ScRangeData::~ScRangeData()
{
    delete pCode;
}

void ScRangeData::CompileRangeData( const OUString& rSymbol, bool bSetError )
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
    std::unique_ptr<ScTokenArray> pOldCode( pCode);     // old pCode will be deleted
    pCode = pNewCode;
    pCode->SetFromRangeName(true);
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

void ScRangeData::CompileUnresolvedXML( sc::CompileFormulaContext& rCxt )
{
    if (pCode->GetCodeError() == errNoName)
    {
        // Reconstruct the symbol/formula and then recompile.
        OUString aSymbol;
        rCxt.setGrammar(eTempGrammar);
        ScCompiler aComp(rCxt, aPos, *pCode);
        aComp.CreateStringFromTokenArray( aSymbol);
        // Don't let the compiler set an error for unknown names on final
        // compile, errors are handled by the interpreter thereafter.
        CompileRangeData( aSymbol, false);
    }
}

#if DEBUG_FORMULA_COMPILER
void ScRangeData::Dump() const
{
    cout << "-- ScRangeData" << endl;
    cout << "  name: " << aName << endl;
    cout << "  ref position: (col=" << aPos.Col() << ", row=" << aPos.Row() << ", sheet=" << aPos.Tab() << ")" << endl;

    if (pCode)
        pCode->Dump();
}
#endif

void ScRangeData::GuessPosition()
{
    // set a position that allows "absoluting" of all relative references
    // in CalcAbsIfRel without errors

    OSL_ENSURE(aPos == ScAddress(), "position will go lost now");

    SCsCOL nMinCol = 0;
    SCsROW nMinRow = 0;
    SCsTAB nMinTab = 0;

    formula::FormulaToken* t;
    pCode->Reset();
    while ( ( t = pCode->GetNextReference() ) != NULL )
    {
        ScSingleRefData& rRef1 = *t->GetSingleRef();
        if ( rRef1.IsColRel() && rRef1.Col() < nMinCol )
            nMinCol = rRef1.Col();
        if ( rRef1.IsRowRel() && rRef1.Row() < nMinRow )
            nMinRow = rRef1.Row();
        if ( rRef1.IsTabRel() && rRef1.Tab() < nMinTab )
            nMinTab = rRef1.Tab();

        if ( t->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = t->GetDoubleRef()->Ref2;
            if ( rRef2.IsColRel() && rRef2.Col() < nMinCol )
                nMinCol = rRef2.Col();
            if ( rRef2.IsRowRel() && rRef2.Row() < nMinRow )
                nMinRow = rRef2.Row();
            if ( rRef2.IsTabRel() && rRef2.Tab() < nMinTab )
                nMinTab = rRef2.Tab();
        }
    }

    aPos = ScAddress( (SCCOL)(-nMinCol), (SCROW)(-nMinRow), (SCTAB)(-nMinTab) );
}

void ScRangeData::GetSymbol( OUString& rSymbol, const FormulaGrammar::Grammar eGrammar ) const
{
    ScCompiler aComp(pDoc, aPos, *pCode);
    aComp.SetGrammar(eGrammar);
    aComp.CreateStringFromTokenArray( rSymbol );
}

void ScRangeData::GetSymbol( OUString& rSymbol, const ScAddress& rPos, const FormulaGrammar::Grammar eGrammar ) const
{
    OUString aStr;
    ScCompiler aComp(pDoc, rPos, *pCode);
    aComp.SetGrammar(eGrammar);
    aComp.CreateStringFromTokenArray( aStr );
    rSymbol = aStr;
}

void ScRangeData::UpdateSymbol( OUStringBuffer& rBuffer, const ScAddress& rPos,
                                const FormulaGrammar::Grammar eGrammar )
{
    std::unique_ptr<ScTokenArray> pTemp( pCode->Clone() );
    ScCompiler aComp( pDoc, rPos, *pTemp.get());
    aComp.SetGrammar(eGrammar);
    aComp.MoveRelWrap(GetMaxCol(), GetMaxRow());
    aComp.CreateStringFromTokenArray( rBuffer );
}

void ScRangeData::UpdateReference( sc::RefUpdateContext& rCxt, SCTAB nLocalTab )
{
    sc::RefUpdateResult aRes = pCode->AdjustReferenceInName(rCxt, aPos);
    bModified = aRes.mbReferenceModified;
    if (aRes.mbReferenceModified)
        rCxt.maUpdatedNames.setUpdatedName(nLocalTab, nIndex);
}

void ScRangeData::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest )
{
    bool bChanged = false;

    formula::FormulaToken* t;
    pCode->Reset();

    while ( ( t = pCode->GetNextReference() ) != NULL )
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
                ScRange aAbs = rRef.toAbs(aPos);
                if (ScRefUpdate::UpdateTranspose(pDoc, rSource, rDest, aAbs) != UR_NOTHING)
                {
                    rRef.SetRange(aAbs, aPos);
                    bChanged = true;
                }
            }
        }
    }

    bModified = bChanged;
}

void ScRangeData::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    bool bChanged = false;

    formula::FormulaToken* t;
    pCode->Reset();

    while ( ( t = pCode->GetNextReference() ) != NULL )
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
                ScRange aAbs = rRef.toAbs(aPos);
                if (ScRefUpdate::UpdateGrow(rArea, nGrowX, nGrowY, aAbs) != UR_NOTHING)
                {
                    rRef.SetRange(aAbs, aPos);
                    bChanged = true;
                }
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
        return pCode->IsReference(rRange, aPos);

    return false;
}

bool ScRangeData::IsReference( ScRange& rRange, const ScAddress& rPos ) const
{
    if ( (eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) ) && pCode )
        return pCode->IsReference(rRange, rPos);

    return false;
}

bool ScRangeData::IsValidReference( ScRange& rRange ) const
{
    if ( (eType & ( RT_ABSAREA | RT_REFAREA | RT_ABSPOS ) ) && pCode )
        return pCode->IsValidReference(rRange, aPos);

    return false;
}

void ScRangeData::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt, SCTAB nLocalTab )
{
    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnInsertedTab(rCxt, aPos);
    if (aRes.mbReferenceModified)
        rCxt.maUpdatedNames.setUpdatedName(nLocalTab, nIndex);

    if (rCxt.mnInsertPos <= aPos.Tab())
        aPos.IncTab(rCxt.mnSheets);
}

void ScRangeData::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt, SCTAB nLocalTab )
{
    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnDeletedTab(rCxt, aPos);
    if (aRes.mbReferenceModified)
        rCxt.maUpdatedNames.setUpdatedName(nLocalTab, nIndex);

    if (rCxt.mnDeletePos <= aPos.Tab())
        aPos.IncTab(-rCxt.mnSheets);
}

void ScRangeData::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt, SCTAB nLocalTab )
{
    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnMovedTab(rCxt, aPos);
    if (aRes.mbReferenceModified)
        rCxt.maUpdatedNames.setUpdatedName(nLocalTab, nIndex);

    aPos.SetTab(rCxt.getNewTab(aPos.Tab()));
}

void ScRangeData::MakeValidName( OUString& rName )
{

    // strip leading invalid characters
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rName.getLength();
    while ( nPos < nLen && !ScCompiler::IsCharFlagAllConventions( rName, nPos, SC_COMPILER_C_NAME) )
        ++nPos;
    if ( nPos>0 )
        rName = rName.copy(nPos);

    // if the first character is an invalid start character, precede with '_'
    if ( !rName.isEmpty() && !ScCompiler::IsCharFlagAllConventions( rName, 0, SC_COMPILER_C_CHAR_NAME ) )
        rName = "_" + rName;

    // replace invalid with '_'
    nLen = rName.getLength();
    for (nPos=0; nPos<nLen; nPos++)
    {
        if ( !ScCompiler::IsCharFlagAllConventions( rName, nPos, SC_COMPILER_C_NAME) )
            rName = rName.replaceAt( nPos, 1, "_" );
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
            // Range Parse is partially valid also with invalid sheet name,
            // Address Parse dito, during compile name would generate a #REF!
            if ( rName.indexOf( '.' ) != -1 )
                rName = rName.replaceFirst( ".", "_" );
            else
                rName = "_" + rName;
        }
    }
}

bool ScRangeData::IsNameValid( const OUString& rName, ScDocument* pDoc )
{
    /* XXX If changed, sc/source/filter/ftools/ftools.cxx
     * ScfTools::ConvertToScDefinedName needs to be changed too. */
    sal_Char a('.');
    if (rName.indexOf(a) != -1)
        return false;
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rName.getLength();
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

SCROW ScRangeData::GetMaxRow() const
{
    return mnMaxRow >= 0 ? mnMaxRow : MAXROW;
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
    return pCode->HasReferences();
}

sal_uInt32 ScRangeData::GetUnoType() const
{
    sal_uInt32 nUnoType = 0;
    if ( HasType(RT_CRITERIA) )  nUnoType |= css::sheet::NamedRangeFlag::FILTER_CRITERIA;
    if ( HasType(RT_PRINTAREA) ) nUnoType |= css::sheet::NamedRangeFlag::PRINT_AREA;
    if ( HasType(RT_COLHEADER) ) nUnoType |= css::sheet::NamedRangeFlag::COLUMN_HEADER;
    if ( HasType(RT_ROWHEADER) ) nUnoType |= css::sheet::NamedRangeFlag::ROW_HEADER;
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
    formula::FormulaToken* t;
    pCode->Reset();
    while ( ( t = pCode->GetNextReference() ) != NULL )
    {
        ScSingleRefData& rRef1 = *t->GetSingleRef();
        ScAddress aAbs = rRef1.toAbs(aPos);
        if ( rRef1.IsTabRel() && !rRef1.IsTabDeleted() )
        {
            if (aAbs.Tab() < nMinTab)
                nMinTab = aAbs.Tab();
            if (aAbs.Tab() > nMaxTab)
                nMaxTab = aAbs.Tab();
        }
        if ( t->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = t->GetDoubleRef()->Ref2;
            aAbs = rRef2.toAbs(aPos);
            if ( rRef2.IsTabRel() && !rRef2.IsTabDeleted() )
            {
                if (aAbs.Tab() < nMinTab)
                    nMinTab = aAbs.Tab();
                if (aAbs.Tab() > nMaxTab)
                    nMaxTab = aAbs.Tab();
            }
        }
    }

    SCTAB nTabCount = pDoc->GetTableCount();
    if ( nMaxTab >= nTabCount && nMinTab > 0 )
    {
        //  move position and relative tab refs
        //  The formulas that use the name are not changed by this

        SCTAB nMove = nMinTab;
        ScAddress aOldPos = aPos;
        aPos.SetTab( aPos.Tab() - nMove );

        pCode->Reset();
        while ( ( t = pCode->GetNextReference() ) != NULL )
        {
            switch (t->GetType())
            {
                case svSingleRef:
                {
                    ScSingleRefData& rRef = *t->GetSingleRef();
                    if (!rRef.IsTabDeleted())
                    {
                        ScAddress aAbs = rRef.toAbs(aOldPos);
                        rRef.SetAddress(aAbs, aPos);
                    }
                }
                break;
                case svDoubleRef:
                {
                    ScComplexRefData& rRef = *t->GetDoubleRef();
                    if (!rRef.Ref1.IsTabDeleted())
                    {
                        ScAddress aAbs = rRef.Ref1.toAbs(aOldPos);
                        rRef.Ref1.SetAddress(aAbs, aPos);
                    }
                    if (!rRef.Ref2.IsTabDeleted())
                    {
                        ScAddress aAbs = rRef.Ref2.toAbs(aOldPos);
                        rRef.Ref2.SetAddress(aAbs, aPos);
                    }
                }
                break;
                default:
                    ;
            }
        }
    }
}

void ScRangeData::SetCode( ScTokenArray& rArr )
{
    std::unique_ptr<ScTokenArray> pOldCode( pCode); // old pCode will be deleted
    pCode = new ScTokenArray( rArr );
    pCode->SetFromRangeName(true);
    InitCode();
}

void ScRangeData::InitCode()
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

extern "C"
int SAL_CALL ScRangeData_QsortNameCompare( const void* p1, const void* p2 )
{
    return (int) ScGlobal::GetCollator()->compareString(
            (*static_cast<const ScRangeData* const *>(p1))->GetName(),
            (*static_cast<const ScRangeData* const *>(p2))->GetName() );
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
    explicit MatchByRange(const ScRange& rRange) : mrRange(rRange) {}
    bool operator() ( boost::ptr_container_detail::ref_pair<OUString, const ScRangeData* const> const& r) const
    {
        return r.second->IsRangeAtBlock(mrRange);
    }
};

}

ScRangeName::ScRangeName() {}

ScRangeName::ScRangeName(const ScRangeName& r) :
    maData(r.maData)
{
    // boost::ptr_map clones and deletes, so each collection needs its own
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

void ScRangeName::UpdateReference(sc::RefUpdateContext& rCxt, SCTAB nLocalTab )
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->UpdateReference(rCxt, nLocalTab);
}

void ScRangeName::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt, SCTAB nLocalTab )
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->UpdateInsertTab(rCxt, nLocalTab);
}

void ScRangeName::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt, SCTAB nLocalTab )
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->UpdateDeleteTab(rCxt, nLocalTab);
}

void ScRangeName::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt, SCTAB nLocalTab )
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->UpdateMoveTab(rCxt, nLocalTab);
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

void ScRangeName::CompileUnresolvedXML( sc::CompileFormulaContext& rCxt )
{
    DataType::iterator itr = maData.begin(), itrEnd = maData.end();
    for (; itr != itrEnd; ++itr)
        itr->second->CompileUnresolvedXML(rCxt);
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

    OUString aName(p->GetUpperName());
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

void ScRangeName::erase(const OUString& rName)
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
