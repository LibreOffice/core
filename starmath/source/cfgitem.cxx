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

#include <cassert>
#include <memory>
#include <vector>

#include <svl/itemset.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svl/eitem.hxx>
#include <svl/languageoptions.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <i18nlangtag/languagetag.hxx>

#include <officecfg/Office/Math.hxx>
#include "cfgitem.hxx"

#include <starmath.hrc>
#include <smmod.hxx>
#include <symbol.hxx>
#include <format.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

#define SYMBOL_LIST         "SymbolList"
#define FONT_FORMAT_LIST    "FontFormatList"

static Sequence< OUString > lcl_GetFontPropertyNames()
{
    return Sequence< OUString > {
                        "Name",
                        "CharSet",
                        "Family",
                        "Pitch",
                        "Weight",
                        "Italic"
                    };
}

static Sequence< OUString > lcl_GetSymbolPropertyNames()
{
    return Sequence< OUString > {
                        "Char",
                        "Set",
                        "Predefined",
                        "FontFormatId"
                    };
}

static Sequence< OUString > lcl_GetFormatPropertyNames()
{
    //! Beware of order according to *_BEGIN *_END defines in format.hxx !
    //! see respective load/save routines here
    return Sequence< OUString > {
                        "StandardFormat/Textmode",
                        "StandardFormat/GreekCharStyle",
                        "StandardFormat/ScaleNormalBracket",
                        "StandardFormat/HorizontalAlignment",
                        "StandardFormat/BaseSize",
                        "StandardFormat/TextSize",
                        "StandardFormat/IndexSize",
                        "StandardFormat/FunctionSize",
                        "StandardFormat/OperatorSize",
                        "StandardFormat/LimitsSize",
                        "StandardFormat/Distance/Horizontal",
                        "StandardFormat/Distance/Vertical",
                        "StandardFormat/Distance/Root",
                        "StandardFormat/Distance/SuperScript",
                        "StandardFormat/Distance/SubScript",
                        "StandardFormat/Distance/Numerator",
                        "StandardFormat/Distance/Denominator",
                        "StandardFormat/Distance/Fraction",
                        "StandardFormat/Distance/StrokeWidth",
                        "StandardFormat/Distance/UpperLimit",
                        "StandardFormat/Distance/LowerLimit",
                        "StandardFormat/Distance/BracketSize",
                        "StandardFormat/Distance/BracketSpace",
                        "StandardFormat/Distance/MatrixRow",
                        "StandardFormat/Distance/MatrixColumn",
                        "StandardFormat/Distance/OrnamentSize",
                        "StandardFormat/Distance/OrnamentSpace",
                        "StandardFormat/Distance/OperatorSize",
                        "StandardFormat/Distance/OperatorSpace",
                        "StandardFormat/Distance/LeftSpace",
                        "StandardFormat/Distance/RightSpace",
                        "StandardFormat/Distance/TopSpace",
                        "StandardFormat/Distance/BottomSpace",
                        "StandardFormat/Distance/NormalBracketSize",
                        "StandardFormat/VariableFont",
                        "StandardFormat/FunctionFont",
                        "StandardFormat/NumberFont",
                        "StandardFormat/TextFont",
                        "StandardFormat/SerifFont",
                        "StandardFormat/SansFont",
                        "StandardFormat/FixedFont"
                    };
}

struct SmCfgOther
{
    SmPrintSize     ePrintSize;
    sal_uInt16      nPrintZoomFactor;
    bool            bPrintTitle;
    bool            bPrintFormulaText;
    bool            bPrintFrame;
    bool            bIsSaveOnlyUsedSymbols;
    bool            bIsAutoCloseBrackets;
    bool            bIgnoreSpacesRight;
    bool            bToolboxVisible;
    bool            bAutoRedraw;
    bool            bFormulaCursor;

    SmCfgOther();
};


SmCfgOther::SmCfgOther()
    : ePrintSize(PRINT_SIZE_NORMAL)
    , nPrintZoomFactor(100)
    , bPrintTitle(true)
    , bPrintFormulaText(true)
    , bPrintFrame(true)
    , bIsSaveOnlyUsedSymbols(true)
    , bIsAutoCloseBrackets(true)
    , bIgnoreSpacesRight(true)
    , bToolboxVisible(true)
    , bAutoRedraw(true)
    , bFormulaCursor(true)
{
}


SmFontFormat::SmFontFormat()
    : aName(FONTNAME_MATH)
    , nCharSet(RTL_TEXTENCODING_UNICODE)
    , nFamily(FAMILY_DONTKNOW)
    , nPitch(PITCH_DONTKNOW)
    , nWeight(WEIGHT_DONTKNOW)
    , nItalic(ITALIC_NONE)
{
}


SmFontFormat::SmFontFormat( const vcl::Font &rFont )
    : aName(rFont.GetFamilyName())
    , nCharSet(static_cast<sal_Int16>(rFont.GetCharSet()))
    , nFamily(static_cast<sal_Int16>(rFont.GetFamilyType()))
    , nPitch(static_cast<sal_Int16>(rFont.GetPitch()))
    , nWeight(static_cast<sal_Int16>(rFont.GetWeight()))
    , nItalic(static_cast<sal_Int16>(rFont.GetItalic()))
{
}


const vcl::Font SmFontFormat::GetFont() const
{
    vcl::Font aRes;
    aRes.SetFamilyName( aName );
    aRes.SetCharSet( static_cast<rtl_TextEncoding>(nCharSet) );
    aRes.SetFamily( static_cast<FontFamily>(nFamily) );
    aRes.SetPitch( static_cast<FontPitch>(nPitch) );
    aRes.SetWeight( static_cast<FontWeight>(nWeight) );
    aRes.SetItalic( static_cast<FontItalic>(nItalic) );
    return aRes;
}


bool SmFontFormat::operator == ( const SmFontFormat &rFntFmt ) const
{
    return  aName    == rFntFmt.aName       &&
            nCharSet == rFntFmt.nCharSet    &&
            nFamily  == rFntFmt.nFamily     &&
            nPitch   == rFntFmt.nPitch      &&
            nWeight  == rFntFmt.nWeight     &&
            nItalic  == rFntFmt.nItalic;
}


SmFntFmtListEntry::SmFntFmtListEntry( const OUString &rId, const SmFontFormat &rFntFmt ) :
    aId     (rId),
    aFntFmt (rFntFmt)
{
}


SmFontFormatList::SmFontFormatList()
    : bModified(false)
{
}


void SmFontFormatList::Clear()
{
    if (!aEntries.empty())
    {
        aEntries.clear();
        SetModified( true );
    }
}


void SmFontFormatList::AddFontFormat( const OUString &rFntFmtId,
        const SmFontFormat &rFntFmt )
{
    const SmFontFormat *pFntFmt = GetFontFormat( rFntFmtId );
    OSL_ENSURE( !pFntFmt, "FontFormatId already exists" );
    if (!pFntFmt)
    {
        SmFntFmtListEntry aEntry( rFntFmtId, rFntFmt );
        aEntries.push_back( aEntry );
        SetModified( true );
    }
}


void SmFontFormatList::RemoveFontFormat( const OUString &rFntFmtId )
{

    // search for entry
    for (size_t i = 0;  i < aEntries.size();  ++i)
    {
        if (aEntries[i].aId == rFntFmtId)
        {
            // remove entry if found
            aEntries.erase( aEntries.begin() + i );
            SetModified( true );
            break;
        }
    }
}


const SmFontFormat * SmFontFormatList::GetFontFormat( const OUString &rFntFmtId ) const
{
    const SmFontFormat *pRes = nullptr;

    for (const auto & rEntry : aEntries)
    {
        if (rEntry.aId == rFntFmtId)
        {
            pRes = &rEntry.aFntFmt;
            break;
        }
    }

    return pRes;
}


const SmFontFormat * SmFontFormatList::GetFontFormat( size_t nPos ) const
{
    const SmFontFormat *pRes = nullptr;
    if (nPos < aEntries.size())
        pRes = &aEntries[nPos].aFntFmt;
    return pRes;
}


const OUString SmFontFormatList::GetFontFormatId( const SmFontFormat &rFntFmt ) const
{
    OUString aRes;

    for (const auto & rEntry : aEntries)
    {
        if (rEntry.aFntFmt == rFntFmt)
        {
            aRes = rEntry.aId;
            break;
        }
    }

    return aRes;
}


const OUString SmFontFormatList::GetFontFormatId( const SmFontFormat &rFntFmt, bool bAdd )
{
    OUString aRes( GetFontFormatId( rFntFmt) );
    if (aRes.isEmpty()  &&  bAdd)
    {
        aRes = GetNewFontFormatId();
        AddFontFormat( aRes, rFntFmt );
    }
    return aRes;
}


const OUString SmFontFormatList::GetFontFormatId( size_t nPos ) const
{
    OUString aRes;
    if (nPos < aEntries.size())
        aRes = aEntries[nPos].aId;
    return aRes;
}


const OUString SmFontFormatList::GetNewFontFormatId() const
{
    // returns first unused FormatId

    sal_Int32 nCnt = GetCount();
    for (sal_Int32 i = 1;  i <= nCnt + 1;  ++i)
    {
        OUString aTmpId = "Id" + OUString::number(i);
        if (!GetFontFormat(aTmpId))
            return aTmpId;
    }
    OSL_ENSURE( false, "failed to create new FontFormatId" );

    return OUString();
}


SmMathConfig::SmMathConfig() :
    ConfigItem("Office.Math")
    , pFormat()
    , pOther()
    , pFontFormatList()
    , pSymbolMgr()
    , bIsOtherModified(false)
    , bIsFormatModified(false)
{
}


SmMathConfig::~SmMathConfig()
{
    Save();
}


void SmMathConfig::SetOtherModified( bool bVal )
{
    bIsOtherModified = bVal;
}


void SmMathConfig::SetFormatModified( bool bVal )
{
    bIsFormatModified = bVal;
}


void SmMathConfig::ReadSymbol( SmSym &rSymbol,
                        const OUString &rSymbolName,
                        const OUString &rBaseNode ) const
{
    Sequence< OUString > aNames = lcl_GetSymbolPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    OUString aDelim( "/" );
    OUString *pName = aNames.getArray();
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        OUString &rName = pName[i];
        OUString aTmp( rName );
        rName = rBaseNode;
        rName += aDelim;
        rName += rSymbolName;
        rName += aDelim;
        rName += aTmp;
    }

    const Sequence< Any > aValues = const_cast<SmMathConfig*>(this)->GetProperties(aNames);

    if (!(nProps  &&  aValues.getLength() == nProps))
        return;

    const Any * pValue = aValues.getConstArray();
    vcl::Font   aFont;
    sal_UCS4    cChar = '\0';
    OUString    aSet;
    bool        bPredefined = false;

    OUString    aTmpStr;
    sal_Int32       nTmp32 = 0;
    bool        bTmp = false;

    bool bOK = true;
    if (pValue->hasValue()  &&  (*pValue >>= nTmp32))
        cChar = static_cast< sal_UCS4 >( nTmp32 );
    else
        bOK = false;
    ++pValue;
    if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
        aSet = aTmpStr;
    else
        bOK = false;
    ++pValue;
    if (pValue->hasValue()  &&  (*pValue >>= bTmp))
        bPredefined = bTmp;
    else
        bOK = false;
    ++pValue;
    if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
    {
        const SmFontFormat *pFntFmt = GetFontFormatList().GetFontFormat( aTmpStr );
        OSL_ENSURE( pFntFmt, "unknown FontFormat" );
        if (pFntFmt)
            aFont = pFntFmt->GetFont();
    }
    else
        bOK = false;
    ++pValue;

    if (bOK)
    {
        OUString aUiName( rSymbolName );
        OUString aUiSetName( aSet );
        if (bPredefined)
        {
            OUString aTmp;
            aTmp = SmLocalizedSymbolData::GetUiSymbolName( rSymbolName );
            OSL_ENSURE( !aTmp.isEmpty(), "localized symbol-name not found" );
            if (!aTmp.isEmpty())
                aUiName = aTmp;
            aTmp = SmLocalizedSymbolData::GetUiSymbolSetName( aSet );
            OSL_ENSURE( !aTmp.isEmpty(), "localized symbolset-name not found" );
            if (!aTmp.isEmpty())
                aUiSetName = aTmp;
        }

        rSymbol = SmSym( aUiName, aFont, cChar, aUiSetName, bPredefined );
        if (aUiName != rSymbolName)
            rSymbol.SetExportName( rSymbolName );
    }
    else
    {
        SAL_WARN("starmath", "symbol read error");
    }
}


SmSymbolManager & SmMathConfig::GetSymbolManager()
{
    if (!pSymbolMgr)
    {
        pSymbolMgr.reset(new SmSymbolManager);
        pSymbolMgr->Load();
    }
    return *pSymbolMgr;
}


void SmMathConfig::ImplCommit()
{
    Save();
}


void SmMathConfig::Save()
{
    SaveOther();
    SaveFormat();
    SaveFontFormatList();
}


void SmMathConfig::GetSymbols( std::vector< SmSym > &rSymbols ) const
{
    Sequence< OUString > aNodes(const_cast<SmMathConfig*>(this)->GetNodeNames(SYMBOL_LIST));
    const OUString *pNode = aNodes.getConstArray();
    sal_Int32 nNodes = aNodes.getLength();

    rSymbols.resize( nNodes );
    for (auto& rSymbol : rSymbols)
    {
        ReadSymbol( rSymbol, *pNode++, SYMBOL_LIST );
    }
}


void SmMathConfig::SetSymbols( const std::vector< SmSym > &rNewSymbols )
{
    auto nCount = sal::static_int_cast<sal_Int32>(rNewSymbols.size());

    Sequence< OUString > aNames = lcl_GetSymbolPropertyNames();
    const OUString *pNames = aNames.getConstArray();
    sal_Int32 nSymbolProps = aNames.getLength();

    Sequence< PropertyValue > aValues( nCount * nSymbolProps );
    PropertyValue *pValues = aValues.getArray();

    PropertyValue *pVal = pValues;
    OUString aDelim( "/" );
    for (const SmSym& rSymbol : rNewSymbols)
    {
        OUString  aNodeNameDelim( SYMBOL_LIST );
        aNodeNameDelim += aDelim;
        aNodeNameDelim += rSymbol.GetExportName();
        aNodeNameDelim += aDelim;

        const OUString *pName = pNames;

        // Char
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= rSymbol.GetCharacter();
        pVal++;
        // Set
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        OUString aTmp( rSymbol.GetSymbolSetName() );
        if (rSymbol.IsPredefined())
            aTmp = SmLocalizedSymbolData::GetExportSymbolSetName( aTmp );
        pVal->Value <<= aTmp;
        pVal++;
        // Predefined
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= rSymbol.IsPredefined();
        pVal++;
        // FontFormatId
        SmFontFormat aFntFmt( rSymbol.GetFace() );
        OUString aFntFmtId( GetFontFormatList().GetFontFormatId( aFntFmt, true ) );
        OSL_ENSURE( !aFntFmtId.isEmpty(), "FontFormatId not found" );
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= aFntFmtId;
        pVal++;
    }
    OSL_ENSURE( pVal - pValues == sal::static_int_cast< ptrdiff_t >(nCount * nSymbolProps), "properties missing" );
    ReplaceSetProperties( SYMBOL_LIST, aValues );

    StripFontFormatList( rNewSymbols );
    SaveFontFormatList();
}


SmFontFormatList & SmMathConfig::GetFontFormatList()
{
    if (!pFontFormatList)
    {
        LoadFontFormatList();
    }
    return *pFontFormatList;
}


void SmMathConfig::LoadFontFormatList()
{
    if (!pFontFormatList)
        pFontFormatList.reset(new SmFontFormatList);
    else
        pFontFormatList->Clear();

    Sequence< OUString > aNodes( GetNodeNames( FONT_FORMAT_LIST ) );
    const OUString *pNode = aNodes.getConstArray();
    sal_Int32 nNodes = aNodes.getLength();

    for (sal_Int32 i = 0;  i < nNodes;  ++i)
    {
        SmFontFormat aFntFmt;
        ReadFontFormat( aFntFmt, pNode[i], FONT_FORMAT_LIST );
        if (!pFontFormatList->GetFontFormat( pNode[i] ))
            pFontFormatList->AddFontFormat( pNode[i], aFntFmt );
    }
    pFontFormatList->SetModified( false );
}


void SmMathConfig::ReadFontFormat( SmFontFormat &rFontFormat,
        const OUString &rSymbolName, const OUString &rBaseNode ) const
{
    Sequence< OUString > aNames = lcl_GetFontPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    OUString aDelim( "/" );
    OUString *pName = aNames.getArray();
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        OUString &rName = pName[i];
        OUString aTmp( rName );
        rName = rBaseNode;
        rName += aDelim;
        rName += rSymbolName;
        rName += aDelim;
        rName += aTmp;
    }

    const Sequence< Any > aValues = const_cast<SmMathConfig*>(this)->GetProperties(aNames);

    if (!(nProps  &&  aValues.getLength() == nProps))
        return;

    const Any * pValue = aValues.getConstArray();

    OUString    aTmpStr;
    sal_Int16       nTmp16 = 0;

    bool bOK = true;
    if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
        rFontFormat.aName = aTmpStr;
    else
        bOK = false;
    ++pValue;
    if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
        rFontFormat.nCharSet = nTmp16; // 6.0 file-format GetSOLoadTextEncoding not needed
    else
        bOK = false;
    ++pValue;
    if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
        rFontFormat.nFamily = nTmp16;
    else
        bOK = false;
    ++pValue;
    if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
        rFontFormat.nPitch = nTmp16;
    else
        bOK = false;
    ++pValue;
    if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
        rFontFormat.nWeight = nTmp16;
    else
        bOK = false;
    ++pValue;
    if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
        rFontFormat.nItalic = nTmp16;
    else
        bOK = false;
    ++pValue;

    OSL_ENSURE( bOK, "read FontFormat failed" );
}


void SmMathConfig::SaveFontFormatList()
{
    SmFontFormatList &rFntFmtList = GetFontFormatList();

    if (!rFntFmtList.IsModified())
        return;

    Sequence< OUString > aNames = lcl_GetFontPropertyNames();
    sal_Int32 nSymbolProps = aNames.getLength();

    size_t nCount = rFntFmtList.GetCount();

    Sequence< PropertyValue > aValues( nCount * nSymbolProps );
    PropertyValue *pValues = aValues.getArray();

    PropertyValue *pVal = pValues;
    OUString aDelim( "/" );
    for (size_t i = 0;  i < nCount;  ++i)
    {
        OUString aFntFmtId(rFntFmtList.GetFontFormatId(i));
        const SmFontFormat *pFntFmt = rFntFmtList.GetFontFormat(i);
        assert(pFntFmt);
        const SmFontFormat aFntFmt(*pFntFmt);

        OUString  aNodeNameDelim( FONT_FORMAT_LIST );
        aNodeNameDelim += aDelim;
        aNodeNameDelim += aFntFmtId;
        aNodeNameDelim += aDelim;

        const OUString *pName = aNames.getConstArray();

        // Name
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= aFntFmt.aName;
        pVal++;
        // CharSet
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= aFntFmt.nCharSet; // 6.0 file-format GetSOStoreTextEncoding not needed
        pVal++;
        // Family
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= aFntFmt.nFamily;
        pVal++;
        // Pitch
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= aFntFmt.nPitch;
        pVal++;
        // Weight
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= aFntFmt.nWeight;
        pVal++;
        // Italic
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= aFntFmt.nItalic;
        pVal++;
    }
    OSL_ENSURE( sal::static_int_cast<size_t>(pVal - pValues) == nCount * nSymbolProps, "properties missing" );
    ReplaceSetProperties( FONT_FORMAT_LIST, aValues );

    rFntFmtList.SetModified( false );
}


void SmMathConfig::StripFontFormatList( const std::vector< SmSym > &rSymbols )
{
    size_t i;

    // build list of used font-formats only
    //!! font-format IDs may be different !!
    SmFontFormatList aUsedList;
    for (i = 0;  i < rSymbols.size();  ++i)
    {
        OSL_ENSURE( rSymbols[i].GetName().getLength() > 0, "non named symbol" );
        aUsedList.GetFontFormatId( SmFontFormat( rSymbols[i].GetFace() ) , true );
    }
    const SmFormat & rStdFmt = GetStandardFormat();
    for (i = FNT_BEGIN;  i <= FNT_END;  ++i)
    {
        aUsedList.GetFontFormatId( SmFontFormat( rStdFmt.GetFont( i ) ) , true );
    }

    // remove unused font-formats from list
    SmFontFormatList &rFntFmtList = GetFontFormatList();
    size_t nCnt = rFntFmtList.GetCount();
    std::unique_ptr<SmFontFormat[]> pTmpFormat(new SmFontFormat[ nCnt ]);
    std::unique_ptr<OUString[]> pId(new OUString[ nCnt ]);
    size_t k;
    for (k = 0;  k < nCnt;  ++k)
    {
        pTmpFormat[k] = *rFntFmtList.GetFontFormat( k );
        pId[k]     = rFntFmtList.GetFontFormatId( k );
    }
    for (k = 0;  k < nCnt;  ++k)
    {
        if (aUsedList.GetFontFormatId( pTmpFormat[k] ).isEmpty())
        {
            rFntFmtList.RemoveFontFormat( pId[k] );
        }
    }
}


void SmMathConfig::LoadOther()
{
    if (!pOther)
        pOther.reset(new SmCfgOther);

    pOther->bPrintTitle = officecfg::Office::Math::Print::Title::get();
    pOther->bPrintFormulaText = officecfg::Office::Math::Print::FormulaText::get();
    pOther->bPrintFrame = officecfg::Office::Math::Print::Frame::get();
    pOther->ePrintSize = static_cast<SmPrintSize>(officecfg::Office::Math::Print::Size::get());
    pOther->nPrintZoomFactor = officecfg::Office::Math::Print::ZoomFactor::get();
    pOther->bIsSaveOnlyUsedSymbols = officecfg::Office::Math::LoadSave::IsSaveOnlyUsedSymbols::get();
    pOther->bIsAutoCloseBrackets = officecfg::Office::Math::Misc::AutoCloseBrackets::get();
    pOther->bIgnoreSpacesRight = officecfg::Office::Math::Misc::IgnoreSpacesRight::get();
    pOther->bToolboxVisible = officecfg::Office::Math::View::ToolboxVisible::get();
    pOther->bAutoRedraw = officecfg::Office::Math::View::AutoRedraw::get();
    pOther->bFormulaCursor = officecfg::Office::Math::View::FormulaCursor::get();
    SetOtherModified( false );
}


void SmMathConfig::SaveOther()
{
    if (!pOther || !IsOtherModified())
        return;

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    officecfg::Office::Math::Print::Title::set(pOther->bPrintTitle, batch);
    officecfg::Office::Math::Print::FormulaText::set(pOther->bPrintFormulaText, batch);
    officecfg::Office::Math::Print::Frame::set(pOther->bPrintFrame, batch);
    officecfg::Office::Math::Print::Size::set(pOther->ePrintSize, batch);
    officecfg::Office::Math::Print::ZoomFactor::set(pOther->nPrintZoomFactor, batch);
    officecfg::Office::Math::LoadSave::IsSaveOnlyUsedSymbols::set(pOther->bIsSaveOnlyUsedSymbols, batch);
    officecfg::Office::Math::Misc::AutoCloseBrackets::set(pOther->bIsAutoCloseBrackets, batch);
    officecfg::Office::Math::Misc::IgnoreSpacesRight::set(pOther->bIgnoreSpacesRight, batch);
    officecfg::Office::Math::View::ToolboxVisible::set(pOther->bToolboxVisible, batch);
    officecfg::Office::Math::View::AutoRedraw::set(pOther->bAutoRedraw, batch);
    officecfg::Office::Math::View::FormulaCursor::set(pOther->bFormulaCursor, batch);

    batch->commit();
    SetOtherModified( false );
}

namespace {

// Latin default-fonts
const DefaultFontType aLatinDefFnts[FNT_END] =
{
    DefaultFontType::SERIF,  // FNT_VARIABLE
    DefaultFontType::SERIF,  // FNT_FUNCTION
    DefaultFontType::SERIF,  // FNT_NUMBER
    DefaultFontType::SERIF,  // FNT_TEXT
    DefaultFontType::SERIF,  // FNT_SERIF
    DefaultFontType::SANS,   // FNT_SANS
    DefaultFontType::FIXED   // FNT_FIXED
    //OpenSymbol,    // FNT_MATH
};

// CJK default-fonts
//! we use non-asian fonts for variables, functions and numbers since they
//! look better and even in asia only latin letters will be used for those.
//! At least that's what I was told...
const DefaultFontType aCJKDefFnts[FNT_END] =
{
    DefaultFontType::SERIF,          // FNT_VARIABLE
    DefaultFontType::SERIF,          // FNT_FUNCTION
    DefaultFontType::SERIF,          // FNT_NUMBER
    DefaultFontType::CJK_TEXT,       // FNT_TEXT
    DefaultFontType::CJK_TEXT,       // FNT_SERIF
    DefaultFontType::CJK_DISPLAY,    // FNT_SANS
    DefaultFontType::CJK_TEXT        // FNT_FIXED
    //OpenSymbol,    // FNT_MATH
};

// CTL default-fonts
const DefaultFontType aCTLDefFnts[FNT_END] =
{
    DefaultFontType::CTL_TEXT,    // FNT_VARIABLE
    DefaultFontType::CTL_TEXT,    // FNT_FUNCTION
    DefaultFontType::CTL_TEXT,    // FNT_NUMBER
    DefaultFontType::CTL_TEXT,    // FNT_TEXT
    DefaultFontType::CTL_TEXT,    // FNT_SERIF
    DefaultFontType::CTL_TEXT,    // FNT_SANS
    DefaultFontType::CTL_TEXT     // FNT_FIXED
    //OpenSymbol,    // FNT_MATH
};


OUString lcl_GetDefaultFontName( LanguageType nLang, sal_uInt16 nIdent )
{
    assert(nIdent < FNT_END);
    const DefaultFontType *pTable;
    switch ( SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ) )
    {
        case SvtScriptType::LATIN :     pTable = aLatinDefFnts; break;
        case SvtScriptType::ASIAN :     pTable = aCJKDefFnts; break;
        case SvtScriptType::COMPLEX :   pTable = aCTLDefFnts; break;
        default :
            pTable = aLatinDefFnts;
            SAL_WARN("starmath", "unknown script-type");
    }

    return OutputDevice::GetDefaultFont(pTable[ nIdent ], nLang,
                                        GetDefaultFontFlags::OnlyOne ).GetFamilyName();
}

}


void SmMathConfig::LoadFormat()
{
    if (!pFormat)
        pFormat.reset(new SmFormat);


    Sequence< OUString > aNames = lcl_GetFormatPropertyNames();

    sal_Int32 nProps = aNames.getLength();

    Sequence< Any > aValues( GetProperties( aNames ) );
    if (!(nProps  &&  aValues.getLength() == nProps))
        return;

    const Any *pValues = aValues.getConstArray();
    const Any *pVal = pValues;

    OUString    aTmpStr;
    sal_Int16       nTmp16 = 0;
    bool        bTmp = false;

    // StandardFormat/Textmode
    if (pVal->hasValue()  &&  (*pVal >>= bTmp))
        pFormat->SetTextmode( bTmp );
    ++pVal;
    // StandardFormat/GreekCharStyle
    if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
        pFormat->SetGreekCharStyle( nTmp16 );
    ++pVal;
    // StandardFormat/ScaleNormalBracket
    if (pVal->hasValue()  &&  (*pVal >>= bTmp))
        pFormat->SetScaleNormalBrackets( bTmp );
    ++pVal;
    // StandardFormat/HorizontalAlignment
    if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
        pFormat->SetHorAlign( static_cast<SmHorAlign>(nTmp16) );
    ++pVal;
    // StandardFormat/BaseSize
    if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
        pFormat->SetBaseSize( Size(0, SmPtsTo100th_mm( nTmp16 )) );
    ++pVal;

    sal_uInt16 i;
    for (i = SIZ_BEGIN;  i <= SIZ_END;  ++i)
    {
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pFormat->SetRelSize( i, nTmp16 );
        ++pVal;
    }

    for (i = DIS_BEGIN;  i <= DIS_END;  ++i)
    {
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pFormat->SetDistance( i, nTmp16 );
        ++pVal;
    }

    LanguageType nLang = Application::GetSettings().GetUILanguageTag().getLanguageType();
    for (i = FNT_BEGIN;  i < FNT_END;  ++i)
    {
        vcl::Font aFnt;
        bool bUseDefaultFont = true;
        if (pVal->hasValue()  &&  (*pVal >>= aTmpStr))
        {
            bUseDefaultFont = aTmpStr.isEmpty();
            if (bUseDefaultFont)
            {
                aFnt = pFormat->GetFont( i );
                aFnt.SetFamilyName( lcl_GetDefaultFontName( nLang, i ) );
            }
            else
            {
                const SmFontFormat *pFntFmt = GetFontFormatList().GetFontFormat( aTmpStr );
                OSL_ENSURE( pFntFmt, "unknown FontFormat" );
                if (pFntFmt)
                    aFnt = pFntFmt->GetFont();
            }
        }
        ++pVal;

        aFnt.SetFontSize( pFormat->GetBaseSize() );
        pFormat->SetFont( i, aFnt, bUseDefaultFont );
    }

    OSL_ENSURE( pVal - pValues == nProps, "property mismatch" );
    SetFormatModified( false );
}


void SmMathConfig::SaveFormat()
{
    if (!pFormat || !IsFormatModified())
        return;

    const Sequence< OUString > aNames = lcl_GetFormatPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    Sequence< Any > aValues( nProps );
    Any *pValues = aValues.getArray();
    Any *pValue  = pValues;

    // StandardFormat/Textmode
    *pValue++ <<= pFormat->IsTextmode();
    // StandardFormat/GreekCharStyle
    *pValue++ <<= pFormat->GetGreekCharStyle();
    // StandardFormat/ScaleNormalBracket
    *pValue++ <<= pFormat->IsScaleNormalBrackets();
    // StandardFormat/HorizontalAlignment
    *pValue++ <<= static_cast<sal_Int16>(pFormat->GetHorAlign());
    // StandardFormat/BaseSize
    *pValue++ <<= static_cast<sal_Int16>(SmRoundFraction( Sm100th_mmToPts(
                                    pFormat->GetBaseSize().Height() ) ));

    sal_uInt16 i;
    for (i = SIZ_BEGIN;  i <= SIZ_END;  ++i)
        *pValue++ <<= static_cast<sal_Int16>(pFormat->GetRelSize( i ));

    for (i = DIS_BEGIN;  i <= DIS_END;  ++i)
        *pValue++ <<= static_cast<sal_Int16>(pFormat->GetDistance( i ));

    for (i = FNT_BEGIN;  i < FNT_END;  ++i)
    {
        OUString aFntFmtId;

        if (!pFormat->IsDefaultFont( i ))
        {
            SmFontFormat aFntFmt( pFormat->GetFont( i ) );
            aFntFmtId = GetFontFormatList().GetFontFormatId( aFntFmt, true );
            OSL_ENSURE( !aFntFmtId.isEmpty(), "FontFormatId not found" );
        }

        *pValue++ <<= aFntFmtId;
    }

    OSL_ENSURE( pValue - pValues == nProps, "property mismatch" );
    PutProperties( aNames , aValues );

    SetFormatModified( false );
}


const SmFormat & SmMathConfig::GetStandardFormat() const
{
    if (!pFormat)
        const_cast<SmMathConfig*>(this)->LoadFormat();
    return *pFormat;
}


void SmMathConfig::SetStandardFormat( const SmFormat &rFormat, bool bSaveFontFormatList )
{
    if (!pFormat)
        LoadFormat();
    if (rFormat == *pFormat)
        return;

    *pFormat = rFormat;
    SetFormatModified( true );
    SaveFormat();

    if (bSaveFontFormatList)
    {
        // needed for SmFontTypeDialog's DefaultButtonClickHdl
        if (pFontFormatList)
            pFontFormatList->SetModified( true );
        SaveFontFormatList();
    }
}


SmPrintSize SmMathConfig::GetPrintSize() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->ePrintSize;
}


void SmMathConfig::SetPrintSize( SmPrintSize eSize )
{
    if (!pOther)
        LoadOther();
    if (eSize != pOther->ePrintSize)
    {
        pOther->ePrintSize = eSize;
        SetOtherModified( true );
    }
}


sal_uInt16 SmMathConfig::GetPrintZoomFactor() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->nPrintZoomFactor;
}


void SmMathConfig::SetPrintZoomFactor( sal_uInt16 nVal )
{
    if (!pOther)
        LoadOther();
    if (nVal != pOther->nPrintZoomFactor)
    {
        pOther->nPrintZoomFactor = nVal;
        SetOtherModified( true );
    }
}


void SmMathConfig::SetOtherIfNotEqual( bool &rbItem, bool bNewVal )
{
    if (bNewVal != rbItem)
    {
        rbItem = bNewVal;
        SetOtherModified( true );
    }
}


bool SmMathConfig::IsPrintTitle() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->bPrintTitle;
}


void SmMathConfig::SetPrintTitle( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintTitle, bVal );
}


bool SmMathConfig::IsPrintFormulaText() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->bPrintFormulaText;
}


void SmMathConfig::SetPrintFormulaText( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintFormulaText, bVal );
}

bool SmMathConfig::IsSaveOnlyUsedSymbols() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->bIsSaveOnlyUsedSymbols;
}

bool SmMathConfig::IsAutoCloseBrackets() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->bIsAutoCloseBrackets;
}

bool SmMathConfig::IsPrintFrame() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->bPrintFrame;
}


void SmMathConfig::SetPrintFrame( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintFrame, bVal );
}


void SmMathConfig::SetSaveOnlyUsedSymbols( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bIsSaveOnlyUsedSymbols, bVal );
}


void SmMathConfig::SetAutoCloseBrackets( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bIsAutoCloseBrackets, bVal );
}


bool SmMathConfig::IsIgnoreSpacesRight() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->bIgnoreSpacesRight;
}


void SmMathConfig::SetIgnoreSpacesRight( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bIgnoreSpacesRight, bVal );
}


bool SmMathConfig::IsAutoRedraw() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->bAutoRedraw;
}


void SmMathConfig::SetAutoRedraw( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bAutoRedraw, bVal );
}


bool SmMathConfig::IsShowFormulaCursor() const
{
    if (!pOther)
        const_cast<SmMathConfig*>(this)->LoadOther();
    return pOther->bFormulaCursor;
}


void SmMathConfig::SetShowFormulaCursor( bool bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bFormulaCursor, bVal );
}

void SmMathConfig::Notify( const css::uno::Sequence< OUString >& )
{}


void SmMathConfig::ItemSetToConfig(const SfxItemSet &rSet)
{
    const SfxPoolItem *pItem     = nullptr;

    sal_uInt16 nU16;
    bool bVal;
    if (rSet.GetItemState(SID_PRINTSIZE, true, &pItem) == SfxItemState::SET)
    {   nU16 = static_cast<const SfxUInt16Item *>(pItem)->GetValue();
        SetPrintSize( static_cast<SmPrintSize>(nU16) );
    }
    if (rSet.GetItemState(SID_PRINTZOOM, true, &pItem) == SfxItemState::SET)
    {   nU16 = static_cast<const SfxUInt16Item *>(pItem)->GetValue();
        SetPrintZoomFactor( nU16 );
    }
    if (rSet.GetItemState(SID_PRINTTITLE, true, &pItem) == SfxItemState::SET)
    {   bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
        SetPrintTitle( bVal );
    }
    if (rSet.GetItemState(SID_PRINTTEXT, true, &pItem) == SfxItemState::SET)
    {   bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
        SetPrintFormulaText( bVal );
    }
    if (rSet.GetItemState(SID_PRINTFRAME, true, &pItem) == SfxItemState::SET)
    {   bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
        SetPrintFrame( bVal );
    }
    if (rSet.GetItemState(SID_AUTOREDRAW, true, &pItem) == SfxItemState::SET)
    {   bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
        SetAutoRedraw( bVal );
    }
    if (rSet.GetItemState(SID_NO_RIGHT_SPACES, true, &pItem) == SfxItemState::SET)
    {   bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
        if (IsIgnoreSpacesRight() != bVal)
        {
            SetIgnoreSpacesRight( bVal );

            // reformat (displayed) formulas accordingly
            Broadcast(SfxHint(SfxHintId::MathFormatChanged));
        }
    }
    if (rSet.GetItemState(SID_SAVE_ONLY_USED_SYMBOLS, true, &pItem) == SfxItemState::SET)
    {   bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
        SetSaveOnlyUsedSymbols( bVal );
    }

    if (rSet.GetItemState(SID_AUTO_CLOSE_BRACKETS, true, &pItem) == SfxItemState::SET)
    {
        bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
        SetAutoCloseBrackets( bVal );
    }

    SaveOther();
}


void SmMathConfig::ConfigToItemSet(SfxItemSet &rSet) const
{
    const SfxItemPool *pPool = rSet.GetPool();

    rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTSIZE),
                           sal::static_int_cast<sal_uInt16>(GetPrintSize())));
    rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTZOOM),
                           GetPrintZoomFactor()));

    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTITLE), IsPrintTitle()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTEXT),  IsPrintFormulaText()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTFRAME), IsPrintFrame()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_AUTOREDRAW), IsAutoRedraw()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_NO_RIGHT_SPACES), IsIgnoreSpacesRight()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_SAVE_ONLY_USED_SYMBOLS), IsSaveOnlyUsedSymbols()));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_AUTO_CLOSE_BRACKETS), IsAutoCloseBrackets()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
