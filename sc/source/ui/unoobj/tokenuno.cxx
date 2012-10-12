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


#include "tokenuno.hxx"

#include <sal/macros.h>

#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/ExternalReference.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/AddressConvention.hpp>
#include <com/sun/star/sheet/NameToken.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <svl/itemprop.hxx>
#include <vcl/svapp.hxx>

#include "miscuno.hxx"
#include "convuno.hxx"
#include "unonames.hxx"
#include "token.hxx"
#include "compiler.hxx"
#include "tokenarray.hxx"
#include "docsh.hxx"
#include "rangeseq.hxx"
#include "externalrefmgr.hxx"

using namespace ::formula;
using namespace ::com::sun::star;

// ============================================================================

static const SfxItemPropertyMapEntry* lcl_GetFormulaParserMap()
{
    static SfxItemPropertyMapEntry aFormulaParserMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_COMPILEFAP),           0,  &getBooleanCppuType(),                   0, 0 },
        {MAP_CHAR_LEN(SC_UNO_COMPILEENGLISH),       0,  &getBooleanCppuType(),                   0, 0 },
        {MAP_CHAR_LEN(SC_UNO_IGNORELEADING),        0,  &getBooleanCppuType(),                   0, 0 },
        {MAP_CHAR_LEN(SC_UNO_FORMULACONVENTION),    0,  &getCppuType(&sheet::AddressConvention::UNSPECIFIED), 0, 0 },
        {MAP_CHAR_LEN(SC_UNO_OPCODEMAP),            0,  &getCppuType((uno::Sequence< sheet::FormulaOpCodeMapEntry >*)0), 0, 0 },
        {0,0,0,0,0,0}
    };
    return aFormulaParserMap_Impl;
}

SC_SIMPLE_SERVICE_INFO( ScFormulaParserObj, "ScFormulaParserObj", SC_SERVICENAME_FORMULAPARS )

// ============================================================================

ScFormulaParserObj::ScFormulaParserObj(ScDocShell* pDocSh) :
    mpDocShell( pDocSh ),
    mnConv( sheet::AddressConvention::UNSPECIFIED ),
    mbEnglish( false ),
    mbIgnoreSpaces( true ),
    mbCompileFAP( false )
{
    mpDocShell->GetDocument()->AddUnoObject(*this);
}

ScFormulaParserObj::~ScFormulaParserObj()
{
    if (mpDocShell)
        mpDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScFormulaParserObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        mpDocShell = NULL;
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
    static const sal_Int16 nConvMapCount = sizeof(aConvMap)/sizeof(aConvMap[0]);

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

    rCompiler.SetCompileForFAP(mbCompileFAP);

    rCompiler.SetExternalLinks( maExternalLinks);
}

uno::Sequence<sheet::FormulaToken> SAL_CALL ScFormulaParserObj::parseFormula(
        const rtl::OUString& aFormula, const table::CellAddress& rReferencePos )
                                throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence<sheet::FormulaToken> aRet;

    if (mpDocShell)
    {
        ScDocument* pDoc = mpDocShell->GetDocument();
        ScExternalRefManager::ApiGuard aExtRefGuard(pDoc);

        ScAddress aRefPos( ScAddress::UNINITIALIZED );
        ScUnoConversion::FillScAddress( aRefPos, rReferencePos );
        ScCompiler aCompiler( pDoc, aRefPos);
        aCompiler.SetGrammar(pDoc->GetGrammar());
        SetCompilerFlags( aCompiler );

        ScTokenArray* pCode = aCompiler.CompileString( aFormula );
        (void)ScTokenConversion::ConvertToTokenSequence( *pDoc, aRet, *pCode );
        delete pCode;
    }

    return aRet;
}

rtl::OUString SAL_CALL ScFormulaParserObj::printFormula(
        const uno::Sequence<sheet::FormulaToken>& aTokens, const table::CellAddress& rReferencePos )
                                throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    rtl::OUString aRet;

    if (mpDocShell)
    {
        ScDocument* pDoc = mpDocShell->GetDocument();
        ScTokenArray aCode;
        (void)ScTokenConversion::ConvertToTokenArray( *pDoc, aCode, aTokens );
        ScAddress aRefPos( ScAddress::UNINITIALIZED );
        ScUnoConversion::FillScAddress( aRefPos, rReferencePos );
        ScCompiler aCompiler( pDoc, aRefPos, aCode);
        aCompiler.SetGrammar(pDoc->GetGrammar());
        SetCompilerFlags( aCompiler );

        rtl::OUStringBuffer aBuffer;
        aCompiler.CreateStringFromTokenArray( aBuffer );
        aRet = aBuffer.makeStringAndClear();
    }

    return aRet;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScFormulaParserObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef(new SfxItemPropertySetInfo( lcl_GetFormulaParserMap() ));
    return aRef;
}

void SAL_CALL ScFormulaParserObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aString(aPropertyName);
    if ( aString.EqualsAscii( SC_UNO_COMPILEFAP ) )
    {
        aValue >>= mbCompileFAP;
    }
    else if ( aString.EqualsAscii( SC_UNO_COMPILEENGLISH ) )
    {
        bool bOldEnglish = mbEnglish;
        if (aValue >>= mbEnglish)
        {
            // Need to recreate the symbol map to change English property
            // because the map is const. So for performance reasons set
            // CompileEnglish _before_ OpCodeMap!
            if (mxOpCodeMap.get() && mbEnglish != bOldEnglish)
            {
                ScDocument* pDoc = mpDocShell->GetDocument();
                ScCompiler aCompiler( pDoc, ScAddress());
                aCompiler.SetGrammar(pDoc->GetGrammar());
                mxOpCodeMap = aCompiler.CreateOpCodeMap( maOpCodeMapping, mbEnglish);
            }
        }
        else
            throw lang::IllegalArgumentException();
    }
    else if ( aString.EqualsAscii( SC_UNO_FORMULACONVENTION ) )
    {
        aValue >>= mnConv;
    }
    else if ( aString.EqualsAscii( SC_UNO_IGNORELEADING ) )
    {
        aValue >>= mbIgnoreSpaces;
    }
    else if ( aString.EqualsAscii( SC_UNO_OPCODEMAP ) )
    {
        if (aValue >>= maOpCodeMapping)
        {
            ScDocument* pDoc = mpDocShell->GetDocument();
            ScCompiler aCompiler( pDoc, ScAddress());
            aCompiler.SetGrammar(pDoc->GetGrammar());
            mxOpCodeMap = aCompiler.CreateOpCodeMap( maOpCodeMapping, mbEnglish);
        }
        else
            throw lang::IllegalArgumentException();
    }
    else if ( aString.EqualsAscii( SC_UNO_EXTERNALLINKS ) )
    {
        if (!(aValue >>= maExternalLinks))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScFormulaParserObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    String aString(aPropertyName);
    if ( aString.EqualsAscii( SC_UNO_COMPILEFAP ) )
    {
        aRet <<= mbCompileFAP;
    }
    else if ( aString.EqualsAscii( SC_UNO_COMPILEENGLISH ) )
    {
        aRet <<= mbEnglish;
    }
    else if ( aString.EqualsAscii( SC_UNO_FORMULACONVENTION ) )
    {
        aRet <<= mnConv;
    }
    else if ( aString.EqualsAscii( SC_UNO_IGNORELEADING ) )
    {
        aRet <<= mbIgnoreSpaces;
    }
    else if ( aString.EqualsAscii( SC_UNO_OPCODEMAP ) )
    {
        aRet <<= maOpCodeMapping;
    }
    else if ( aString.EqualsAscii( SC_UNO_EXTERNALLINKS ) )
    {
        aRet <<= maExternalLinks;
    }
    else
        throw beans::UnknownPropertyException();
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScFormulaParserObj )

// ============================================================================

static void lcl_ExternalRefToApi( sheet::SingleReference& rAPI, const ScSingleRefData& rRef )
{
    rAPI.Column         = rRef.nCol;
    rAPI.Row            = rRef.nRow;
    rAPI.Sheet          = 0;
    rAPI.RelativeColumn = rRef.nRelCol;
    rAPI.RelativeRow    = rRef.nRelRow;
    rAPI.RelativeSheet  = 0;

    sal_Int32 nFlags = 0;
    if ( rRef.IsColRel() )     nFlags |= sheet::ReferenceFlags::COLUMN_RELATIVE;
    if ( rRef.IsRowRel() )     nFlags |= sheet::ReferenceFlags::ROW_RELATIVE;
    if ( rRef.IsColDeleted() ) nFlags |= sheet::ReferenceFlags::COLUMN_DELETED;
    if ( rRef.IsRowDeleted() ) nFlags |= sheet::ReferenceFlags::ROW_DELETED;
    if ( rRef.IsFlag3D() )     nFlags |= sheet::ReferenceFlags::SHEET_3D;
    if ( rRef.IsRelName() )    nFlags |= sheet::ReferenceFlags::RELATIVE_NAME;
    rAPI.Flags = nFlags;
}

static void lcl_SingleRefToApi( sheet::SingleReference& rAPI, const ScSingleRefData& rRef )
{
    rAPI.Column         = rRef.nCol;
    rAPI.Row            = rRef.nRow;
    rAPI.Sheet          = rRef.nTab;
    rAPI.RelativeColumn = rRef.nRelCol;
    rAPI.RelativeRow    = rRef.nRelRow;
    rAPI.RelativeSheet  = rRef.nRelTab;

    sal_Int32 nFlags = 0;
    if ( rRef.IsColRel() )     nFlags |= sheet::ReferenceFlags::COLUMN_RELATIVE;
    if ( rRef.IsRowRel() )     nFlags |= sheet::ReferenceFlags::ROW_RELATIVE;
    if ( rRef.IsTabRel() )     nFlags |= sheet::ReferenceFlags::SHEET_RELATIVE;
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
    return !rTokenArray.Fill(rSequence,rDoc.GetExternalRefManager());
}

bool ScTokenConversion::ConvertToTokenSequence( ScDocument& rDoc,
        uno::Sequence<sheet::FormulaToken>& rSequence, const ScTokenArray& rTokenArray )
{
    bool bError = false;

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
                    rAPI.Data <<= rtl::OUString( rToken.GetString() );
                    break;
                case svExternal:
                    // Function name is stored as string.
                    // Byte (parameter count) is ignored.
                    rAPI.Data <<= rtl::OUString( rToken.GetExternal() );
                    break;
                case svSingleRef:
                    {
                        sheet::SingleReference aSingleRef;
                        lcl_SingleRefToApi( aSingleRef, static_cast<const ScToken&>(rToken).GetSingleRef() );
                        rAPI.Data <<= aSingleRef;
                    }
                    break;
                case formula::svDoubleRef:
                    {
                        sheet::ComplexReference aCompRef;
                        lcl_SingleRefToApi( aCompRef.Reference1, static_cast<const ScToken&>(rToken).GetSingleRef() );
                        lcl_SingleRefToApi( aCompRef.Reference2, static_cast<const ScToken&>(rToken).GetSingleRef2() );
                        rAPI.Data <<= aCompRef;
                    }
                    break;
                case svIndex:
                    {
                        sheet::NameToken aNameToken;
                        aNameToken.Index = static_cast<sal_Int32>( rToken.GetIndex() );
                        aNameToken.Global = static_cast<sal_Bool>( rToken.IsGlobal() );
                        rAPI.Data <<= aNameToken;
                    }
                    break;
                case svMatrix:
                    if (!ScRangeToSequence::FillMixedArray( rAPI.Data, static_cast<const ScToken&>(rToken).GetMatrix(), true))
                        rAPI.Data.clear();
                    break;
                case svExternalSingleRef:
                    {
                        sheet::SingleReference aSingleRef;
                        lcl_ExternalRefToApi( aSingleRef, static_cast<const ScToken&>(rToken).GetSingleRef() );
                        size_t nCacheId;
                        rDoc.GetExternalRefManager()->getCacheTable( rToken.GetIndex(), rToken.GetString(), false, &nCacheId );
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
                        lcl_ExternalRefToApi( aComplRef.Reference1, static_cast<const ScToken&>(rToken).GetSingleRef() );
                        lcl_ExternalRefToApi( aComplRef.Reference2, static_cast<const ScToken&>(rToken).GetSingleRef2() );
                        size_t nCacheId;
                        rDoc.GetExternalRefManager()->getCacheTable( rToken.GetIndex(), rToken.GetString(), false, &nCacheId );
                        aComplRef.Reference1.Sheet = static_cast< sal_Int32 >( nCacheId );
                        // NOTE: This assumes that cached sheets are in consecutive order!
                        aComplRef.Reference2.Sheet = aComplRef.Reference1.Sheet + (static_cast<const ScToken&>(rToken).GetSingleRef2().nTab - static_cast<const ScToken&>(rToken).GetSingleRef().nTab);
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
                        aExtRef.Reference <<= ::rtl::OUString( rToken.GetString() );
                        rAPI.Data <<= aExtRef;
                        eOpCode = ocPush;
                    }
                    break;
                default:
                    OSL_TRACE( "ScTokenConversion::ConvertToTokenSequence: unhandled token type SvStackVar %d", rToken.GetType());
                case svSep:     // occurs with ocSep, ocOpen, ocClose, ocArray*
                case svJump:    // occurs with ocIf, ocChose
                case svMissing: // occurs with ocMissing
                    rAPI.Data.clear();      // no data
            }
            rAPI.OpCode = static_cast<sal_Int32>(eOpCode);      //! assuming equal values for the moment
        }
    }
    else
        rSequence.realloc(0);

    return !bError;
}

// ============================================================================

SAL_WNODEPRECATED_DECLARATIONS_PUSH
ScFormulaOpCodeMapperObj::ScFormulaOpCodeMapperObj(::std::auto_ptr<formula::FormulaCompiler> _pCompiler)
: formula::FormulaOpCodeMapperObj(_pCompiler)
{
}
SAL_WNODEPRECATED_DECLARATIONS_POP

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
