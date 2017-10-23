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

#include <memory>
#include <tokenuno.hxx>

#include <sal/macros.h>

#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/ExternalReference.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/AddressConvention.hpp>
#include <com/sun/star/sheet/NameToken.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <svl/itemprop.hxx>
#include <vcl/svapp.hxx>

#include <miscuno.hxx>
#include <convuno.hxx>
#include <unonames.hxx>
#include <token.hxx>
#include <compiler.hxx>
#include <tokenarray.hxx>
#include <docsh.hxx>
#include <rangeseq.hxx>
#include <externalrefmgr.hxx>

using namespace ::formula;
using namespace ::com::sun::star;

static const SfxItemPropertyMapEntry* lcl_GetFormulaParserMap()
{
    static const SfxItemPropertyMapEntry aFormulaParserMap_Impl[] =
    {
        {OUString(SC_UNO_COMPILEFAP),           0,  cppu::UnoType<bool>::get(),                   0, 0 },
        {OUString(SC_UNO_COMPILEENGLISH),       0,  cppu::UnoType<bool>::get(),                   0, 0 },
        {OUString(SC_UNO_IGNORELEADING),        0,  cppu::UnoType<bool>::get(),                   0, 0 },
        {OUString(SC_UNO_FORMULACONVENTION),    0,  cppu::UnoType<decltype(sheet::AddressConvention::UNSPECIFIED)>::get(), 0, 0 },
        {OUString(SC_UNO_OPCODEMAP),            0,  cppu::UnoType<uno::Sequence< sheet::FormulaOpCodeMapEntry >>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aFormulaParserMap_Impl;
}

SC_SIMPLE_SERVICE_INFO( ScFormulaParserObj, "ScFormulaParserObj", SC_SERVICENAME_FORMULAPARS )

ScFormulaParserObj::ScFormulaParserObj(ScDocShell* pDocSh) :
    mpDocShell( pDocSh ),
    mnConv( sheet::AddressConvention::UNSPECIFIED ),
    mbEnglish( false ),
    mbIgnoreSpaces( true ),
    mbCompileFAP( false )
{
    mpDocShell->GetDocument().AddUnoObject(*this);
}

ScFormulaParserObj::~ScFormulaParserObj()
{
    SolarMutexGuard g;

    if (mpDocShell)
        mpDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScFormulaParserObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
        mpDocShell = nullptr;
}

// XFormulaParser

void ScFormulaParserObj::SetCompilerFlags( ScCompiler& rCompiler ) const
{
    static const formula::FormulaGrammar::AddressConvention aConvMap[] = {
        formula::FormulaGrammar::CONV_OOO,        // <- AddressConvention::OOO
        formula::FormulaGrammar::CONV_XL_A1,      // <- AddressConvention::XL_A1
        formula::FormulaGrammar::CONV_XL_R1C1,    // <- AddressConvention::XL_R1C1
        formula::FormulaGrammar::CONV_XL_OOX,     // <- AddressConvention::XL_OOX
        formula::FormulaGrammar::CONV_LOTUS_A1    // <- AddressConvention::LOTUS_A1
    };
    static const sal_Int16 nConvMapCount = SAL_N_ELEMENTS(aConvMap);

    // If mxOpCodeMap is not empty it overrides mbEnglish, and vice versa. We
    // don't need to initialize things twice.
    if (mxOpCodeMap.get())
        rCompiler.SetFormulaLanguage( mxOpCodeMap );
    else
    {
        sal_Int32 nFormulaLanguage = mbEnglish ?
            sheet::FormulaLanguage::ENGLISH :
            sheet::FormulaLanguage::NATIVE;
        ScCompiler::OpCodeMapPtr xMap = rCompiler.GetOpCodeMap( nFormulaLanguage);
        rCompiler.SetFormulaLanguage( xMap);
    }

    formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_UNSPECIFIED;
    if (mnConv >= 0 && mnConv < nConvMapCount)
        eConv = aConvMap[mnConv];

    rCompiler.SetRefConvention( eConv );
    rCompiler.EnableJumpCommandReorder(!mbCompileFAP);
    rCompiler.EnableStopOnError(!mbCompileFAP);

    rCompiler.SetExternalLinks( maExternalLinks);
}

uno::Sequence<sheet::FormulaToken> SAL_CALL ScFormulaParserObj::parseFormula(
    const OUString& aFormula, const table::CellAddress& rReferencePos )
{
    SolarMutexGuard aGuard;
    uno::Sequence<sheet::FormulaToken> aRet;

    if (mpDocShell)
    {
        ScDocument& rDoc = mpDocShell->GetDocument();
        ScExternalRefManager::ApiGuard aExtRefGuard(&rDoc);

        ScAddress aRefPos( ScAddress::UNINITIALIZED );
        ScUnoConversion::FillScAddress( aRefPos, rReferencePos );
        ScCompiler aCompiler( &rDoc, aRefPos, rDoc.GetGrammar());
        SetCompilerFlags( aCompiler );

        ScTokenArray* pCode = aCompiler.CompileString( aFormula );
        (void)ScTokenConversion::ConvertToTokenSequence( rDoc, aRet, *pCode );
        delete pCode;
    }

    return aRet;
}

OUString SAL_CALL ScFormulaParserObj::printFormula(
        const uno::Sequence<sheet::FormulaToken>& aTokens, const table::CellAddress& rReferencePos )
{
    SolarMutexGuard aGuard;
    OUString aRet;

    if (mpDocShell)
    {
        ScDocument& rDoc = mpDocShell->GetDocument();
        ScTokenArray aCode;
        (void)ScTokenConversion::ConvertToTokenArray( rDoc, aCode, aTokens );
        ScAddress aRefPos( ScAddress::UNINITIALIZED );
        ScUnoConversion::FillScAddress( aRefPos, rReferencePos );
        ScCompiler aCompiler( &rDoc, aRefPos, aCode, rDoc.GetGrammar());
        SetCompilerFlags( aCompiler );

        OUStringBuffer aBuffer;
        aCompiler.CreateStringFromTokenArray( aBuffer );
        aRet = aBuffer.makeStringAndClear();
    }

    return aRet;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScFormulaParserObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef(new SfxItemPropertySetInfo( lcl_GetFormulaParserMap() ));
    return aRef;
}

void SAL_CALL ScFormulaParserObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;
    if ( aPropertyName == SC_UNO_COMPILEFAP )
    {
        aValue >>= mbCompileFAP;
    }
    else if ( aPropertyName == SC_UNO_COMPILEENGLISH )
    {
        bool bOldEnglish = mbEnglish;
        if (!(aValue >>= mbEnglish))
            throw lang::IllegalArgumentException();

        // Need to recreate the symbol map to change English property
        // because the map is const. So for performance reasons set
        // CompileEnglish _before_ OpCodeMap!
        if (mxOpCodeMap.get() && mbEnglish != bOldEnglish)
        {
            ScDocument& rDoc = mpDocShell->GetDocument();
            ScCompiler aCompiler( &rDoc, ScAddress(), rDoc.GetGrammar());
            mxOpCodeMap = formula::FormulaCompiler::CreateOpCodeMap( maOpCodeMapping, mbEnglish);
        }

    }
    else if ( aPropertyName == SC_UNO_FORMULACONVENTION )
    {
        aValue >>= mnConv;
    }
    else if ( aPropertyName == SC_UNO_IGNORELEADING )
    {
        aValue >>= mbIgnoreSpaces;
    }
    else if ( aPropertyName == SC_UNO_OPCODEMAP )
    {
        if (!(aValue >>= maOpCodeMapping))
            throw lang::IllegalArgumentException();

        ScDocument& rDoc = mpDocShell->GetDocument();
        ScCompiler aCompiler( &rDoc, ScAddress(), rDoc.GetGrammar());
        mxOpCodeMap = formula::FormulaCompiler::CreateOpCodeMap( maOpCodeMapping, mbEnglish);

    }
    else if ( aPropertyName == SC_UNO_EXTERNALLINKS )
    {
        if (!(aValue >>= maExternalLinks))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScFormulaParserObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_COMPILEFAP )
    {
        aRet <<= mbCompileFAP;
    }
    else if ( aPropertyName == SC_UNO_COMPILEENGLISH )
    {
        aRet <<= mbEnglish;
    }
    else if ( aPropertyName == SC_UNO_FORMULACONVENTION )
    {
        aRet <<= mnConv;
    }
    else if ( aPropertyName == SC_UNO_IGNORELEADING )
    {
        aRet <<= mbIgnoreSpaces;
    }
    else if ( aPropertyName == SC_UNO_OPCODEMAP )
    {
        aRet <<= maOpCodeMapping;
    }
    else if ( aPropertyName == SC_UNO_EXTERNALLINKS )
    {
        aRet <<= maExternalLinks;
    }
    else
        throw beans::UnknownPropertyException();
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScFormulaParserObj )

static void lcl_ExternalRefToApi( sheet::SingleReference& rAPI, const ScSingleRefData& rRef )
{
    rAPI.Column         = 0;
    rAPI.Row            = 0;
    rAPI.Sheet          = 0;
    rAPI.RelativeColumn = 0;
    rAPI.RelativeRow    = 0;
    rAPI.RelativeSheet  = 0;

    sal_Int32 nFlags = 0;
    if ( rRef.IsColRel() )
    {
        nFlags |= sheet::ReferenceFlags::COLUMN_RELATIVE;
        rAPI.RelativeColumn = rRef.Col();
    }
    else
        rAPI.Column = rRef.Col();

    if ( rRef.IsRowRel() )
    {
        nFlags |= sheet::ReferenceFlags::ROW_RELATIVE;
        rAPI.RelativeRow = rRef.Row();
    }
    else
        rAPI.Row = rRef.Row();

    if ( rRef.IsColDeleted() ) nFlags |= sheet::ReferenceFlags::COLUMN_DELETED;
    if ( rRef.IsRowDeleted() ) nFlags |= sheet::ReferenceFlags::ROW_DELETED;
    if ( rRef.IsFlag3D() )     nFlags |= sheet::ReferenceFlags::SHEET_3D;
    if ( rRef.IsRelName() )    nFlags |= sheet::ReferenceFlags::RELATIVE_NAME;
    rAPI.Flags = nFlags;
}

static void lcl_SingleRefToApi( sheet::SingleReference& rAPI, const ScSingleRefData& rRef )
{
    sal_Int32 nFlags = 0;
    if ( rRef.IsColRel() )
    {
        nFlags |= sheet::ReferenceFlags::COLUMN_RELATIVE;
        rAPI.RelativeColumn = rRef.Col();
        rAPI.Column = 0;
    }
    else
    {
        rAPI.RelativeColumn = 0;
        rAPI.Column = rRef.Col();
    }

    if ( rRef.IsRowRel() )
    {
        nFlags |= sheet::ReferenceFlags::ROW_RELATIVE;
        rAPI.RelativeRow = rRef.Row();
        rAPI.Row = 0;
    }
    else
    {
        rAPI.RelativeRow = 0;
        rAPI.Row = rRef.Row();
    }

    if ( rRef.IsTabRel() )
    {
        nFlags |= sheet::ReferenceFlags::SHEET_RELATIVE;
        rAPI.RelativeSheet = rRef.Tab();
        rAPI.Sheet = 0;
    }
    else
    {
        rAPI.RelativeSheet = 0;
        rAPI.Sheet = rRef.Tab();
    }

    if ( rRef.IsColDeleted() ) nFlags |= sheet::ReferenceFlags::COLUMN_DELETED;
    if ( rRef.IsRowDeleted() ) nFlags |= sheet::ReferenceFlags::ROW_DELETED;
    if ( rRef.IsTabDeleted() ) nFlags |= sheet::ReferenceFlags::SHEET_DELETED;
    if ( rRef.IsFlag3D() )     nFlags |= sheet::ReferenceFlags::SHEET_3D;
    if ( rRef.IsRelName() )    nFlags |= sheet::ReferenceFlags::RELATIVE_NAME;
    rAPI.Flags = nFlags;
}

bool ScTokenConversion::ConvertToTokenArray( ScDocument& rDoc,
        ScTokenArray& rTokenArray, const uno::Sequence<sheet::FormulaToken>& rSequence )
{
    return !rTokenArray.Fill(rSequence, rDoc.GetSharedStringPool(), rDoc.GetExternalRefManager());
}

bool ScTokenConversion::ConvertToTokenSequence( const ScDocument& rDoc,
        uno::Sequence<sheet::FormulaToken>& rSequence, const ScTokenArray& rTokenArray )
{
    sal_Int32 nLen = static_cast<sal_Int32>(rTokenArray.GetLen());
    formula::FormulaToken** pTokens = rTokenArray.GetArray();
    if ( pTokens )
    {
        rSequence.realloc(nLen);
        for (sal_Int32 nPos=0; nPos<nLen; nPos++)
        {
            const formula::FormulaToken& rToken = *pTokens[nPos];
            sheet::FormulaToken& rAPI = rSequence[nPos];

            OpCode eOpCode = rToken.GetOpCode();
            // eOpCode may be changed in the following switch/case
            switch ( rToken.GetType() )
            {
                case svByte:
                    // Only the count of spaces is stored as "long". Parameter count is ignored.
                    if ( eOpCode == ocSpaces )
                        rAPI.Data <<= (sal_Int32) rToken.GetByte();
                    else
                        rAPI.Data.clear();      // no data
                    break;
                case formula::svDouble:
                    rAPI.Data <<= rToken.GetDouble();
                    break;
                case formula::svString:
                    rAPI.Data <<= rToken.GetString().getString();
                    break;
                case svExternal:
                    // Function name is stored as string.
                    // Byte (parameter count) is ignored.
                    rAPI.Data <<= rToken.GetExternal();
                    break;
                case svSingleRef:
                    {
                        sheet::SingleReference aSingleRef;
                        lcl_SingleRefToApi( aSingleRef, *rToken.GetSingleRef() );
                        rAPI.Data <<= aSingleRef;
                    }
                    break;
                case formula::svDoubleRef:
                    {
                        sheet::ComplexReference aCompRef;
                        lcl_SingleRefToApi( aCompRef.Reference1, *rToken.GetSingleRef() );
                        lcl_SingleRefToApi( aCompRef.Reference2, *rToken.GetSingleRef2() );
                        rAPI.Data <<= aCompRef;
                    }
                    break;
                case svIndex:
                    {
                        sheet::NameToken aNameToken;
                        aNameToken.Index = static_cast<sal_Int32>( rToken.GetIndex() );
                        aNameToken.Sheet = rToken.GetSheet();
                        rAPI.Data <<= aNameToken;
                    }
                    break;
                case svMatrix:
                    if (!ScRangeToSequence::FillMixedArray( rAPI.Data, rToken.GetMatrix(), true))
                        rAPI.Data.clear();
                    break;
                case svExternalSingleRef:
                    {
                        sheet::SingleReference aSingleRef;
                        lcl_ExternalRefToApi( aSingleRef, *rToken.GetSingleRef() );
                        size_t nCacheId;
                        rDoc.GetExternalRefManager()->getCacheTable(
                            rToken.GetIndex(), rToken.GetString().getString(), false, &nCacheId);
                        aSingleRef.Sheet = static_cast< sal_Int32 >( nCacheId );
                        sheet::ExternalReference aExtRef;
                        aExtRef.Index = rToken.GetIndex();
                        aExtRef.Reference <<= aSingleRef;
                        rAPI.Data <<= aExtRef;
                        eOpCode = ocPush;
                    }
                    break;
                case svExternalDoubleRef:
                    {
                        sheet::ComplexReference aComplRef;
                        lcl_ExternalRefToApi( aComplRef.Reference1, *rToken.GetSingleRef() );
                        lcl_ExternalRefToApi( aComplRef.Reference2, *rToken.GetSingleRef2() );
                        size_t nCacheId;
                        rDoc.GetExternalRefManager()->getCacheTable(
                            rToken.GetIndex(), rToken.GetString().getString(), false, &nCacheId);
                        aComplRef.Reference1.Sheet = static_cast< sal_Int32 >( nCacheId );
                        // NOTE: This assumes that cached sheets are in consecutive order!
                        aComplRef.Reference2.Sheet =
                            aComplRef.Reference1.Sheet +
                            (rToken.GetSingleRef2()->Tab() - rToken.GetSingleRef()->Tab());
                        sheet::ExternalReference aExtRef;
                        aExtRef.Index = rToken.GetIndex();
                        aExtRef.Reference <<= aComplRef;
                        rAPI.Data <<= aExtRef;
                        eOpCode = ocPush;
                    }
                    break;
                case svExternalName:
                    {
                        sheet::ExternalReference aExtRef;
                        aExtRef.Index = rToken.GetIndex();
                        aExtRef.Reference <<= rToken.GetString().getString();
                        rAPI.Data <<= aExtRef;
                        eOpCode = ocPush;
                    }
                    break;
                default:
                    SAL_WARN("sc",  "ScTokenConversion::ConvertToTokenSequence: unhandled token type " << StackVarEnumToString(rToken.GetType()));
                    SAL_FALLTHROUGH;
                case svJump:    // occurs with ocIf, ocChoose
                case svError:   // seems to be fairly common, and probably not exceptional and not worth a warning?
                case svMissing: // occurs with ocMissing
                case svSep:     // occurs with ocSep, ocOpen, ocClose, ocArray*
                    rAPI.Data.clear();      // no data
            }
            rAPI.OpCode = static_cast<sal_Int32>(eOpCode);      //! assuming equal values for the moment
        }
    }
    else
        rSequence.realloc(0);

    return true;
}

ScFormulaOpCodeMapperObj::ScFormulaOpCodeMapperObj(::std::unique_ptr<formula::FormulaCompiler> && _pCompiler)
: formula::FormulaOpCodeMapperObj(std::move(_pCompiler))
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
