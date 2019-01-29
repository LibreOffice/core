/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include "xmlcondformat.hxx"
#include "xmlimprt.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlictxt.hxx>
#include <sal/log.hxx>

#include <compiler.hxx>
#include <colorscale.hxx>
#include <conditio.hxx>
#include <document.hxx>
#include <sax/tools/converter.hxx>
#include <rangelst.hxx>
#include <rangeutl.hxx>
#include "XMLConverter.hxx"
#include <stylehelper.hxx>
#include <tokenarray.hxx>

using namespace xmloff::token;

ScXMLConditionalFormatsContext::ScXMLConditionalFormatsContext( ScXMLImport& rImport ):
    ScXMLImportContext( rImport )
{
    GetScImport().SetNewCondFormatData();
    GetScImport().GetDocument()->SetCondFormList(new ScConditionalFormatList(), GetScImport().GetTables().GetCurrentSheet());
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL ScXMLConditionalFormatsContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( CALC_EXT, XML_CONDITIONAL_FORMAT ):
            pContext = new ScXMLConditionalFormatContext( GetScImport(), pAttribList, *this );
            break;
    }

    return pContext;
}

IMPL_LINK(ScXMLConditionalFormatsContext, FormatDeletedHdl, ScConditionalFormat*, pFormat, void)
{
    mvCondFormatData.erase(std::remove_if(mvCondFormatData.begin(), mvCondFormatData.end(),
                                          [pFormat](CondFormatData& r){ return r.mpFormat == pFormat; }),
                           mvCondFormatData.end());
}

void SAL_CALL ScXMLConditionalFormatsContext::endFastElement( sal_Int32 /*nElement*/ )
{
    ScDocument* pDoc = GetScImport().GetDocument();

    SCTAB nTab = GetScImport().GetTables().GetCurrentSheet();
    ScConditionalFormatList* pCondFormatList = pDoc->GetCondFormList(nTab);
    bool bDeleted = !pCondFormatList->CheckAllEntries(LINK(this, ScXMLConditionalFormatsContext, FormatDeletedHdl));

    SAL_WARN_IF(bDeleted, "sc", "conditional formats have been deleted because they contained empty range info");

    for (const auto& i : mvCondFormatData)
    {
        pDoc->AddCondFormatData( i.mpFormat->GetRange(), i.mnTab, i.mpFormat->GetKey() );
    }
}

ScXMLConditionalFormatContext::ScXMLConditionalFormatContext( ScXMLImport& rImport,
                                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                                              ScXMLConditionalFormatsContext& rParent ):
    ScXMLImportContext( rImport ),
    mrParent( rParent )
{
    OUString sRange;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( CALC_EXT, XML_TARGET_RANGE_ADDRESS ):
                    sRange = aIter.toString();
                break;
                default:
                    break;
            }
        }
    }

    ScRangeList aRangeList;
    ScRangeStringConverter::GetRangeListFromString(aRangeList, sRange, GetScImport().GetDocument(),
            formula::FormulaGrammar::CONV_ODF);

    mxFormat.reset(new ScConditionalFormat(0, GetScImport().GetDocument()));
    mxFormat->SetRange(aRangeList);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL ScXMLConditionalFormatContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( CALC_EXT, XML_CONDITION ):
            pContext = new ScXMLCondContext( GetScImport(), pAttribList, mxFormat.get() );
            break;
        case XML_ELEMENT( CALC_EXT, XML_COLOR_SCALE ):
            pContext = new ScXMLColorScaleFormatContext( GetScImport(), mxFormat.get() );
            break;
        case XML_ELEMENT( CALC_EXT, XML_DATA_BAR ):
            pContext = new ScXMLDataBarFormatContext( GetScImport(), pAttribList, mxFormat.get() );
            break;
        case XML_ELEMENT( CALC_EXT, XML_ICON_SET ):
            pContext = new ScXMLIconSetFormatContext( GetScImport(), pAttribList, mxFormat.get() );
            break;
        case XML_ELEMENT( CALC_EXT, XML_DATE_IS ):
            pContext = new ScXMLDateContext( GetScImport(), pAttribList, mxFormat.get() );
            break;
        default:
            break;
    }

    return pContext;
}

static bool HasRelRefIgnoringSheet0Relative( ScDocument* pDoc, const ScTokenArray* pTokens, sal_uInt16 nRecursion = 0 )
{
    if (pTokens)
    {
        formula::FormulaTokenArrayPlainIterator aIter( *pTokens );
        formula::FormulaToken* t;
        for( t = aIter.Next(); t; t = aIter.Next() )
        {
            switch( t->GetType() )
            {
                case formula::svDoubleRef:
                {
                    ScSingleRefData& rRef2 = t->GetDoubleRef()->Ref2;
                    if ( rRef2.IsColRel() || rRef2.IsRowRel() || (rRef2.IsFlag3D() && rRef2.IsTabRel()) )
                        return true;
                    [[fallthrough]];
                }

                case formula::svSingleRef:
                {
                    ScSingleRefData& rRef1 = *t->GetSingleRef();
                    if ( rRef1.IsColRel() || rRef1.IsRowRel() || (rRef1.IsFlag3D() && rRef1.IsTabRel()) )
                        return true;
                }
                break;

                case formula::svIndex:
                {
                    if( t->GetOpCode() == ocName )      // DB areas always absolute
                        if( ScRangeData* pRangeData = pDoc->FindRangeNameBySheetAndIndex( t->GetSheet(), t->GetIndex()) )
                            if( (nRecursion < 42) && HasRelRefIgnoringSheet0Relative( pDoc, pRangeData->GetCode(), nRecursion + 1 ) )
                                return true;
                }
                break;

                // #i34474# function result dependent on cell position
                case formula::svByte:
                {
                    switch( t->GetOpCode() )
                    {
                        case ocRow:     // ROW() returns own row index
                        case ocColumn:  // COLUMN() returns own column index
                        case ocSheet:   // SHEET() returns own sheet index
                        case ocCell:    // CELL() may return own cell address
                            return true;
                        default:
                            break;
                    }
                }
                break;

                default:
                    break;
            }
        }
    }
    return false;
}

static bool HasOneSingleFullyRelativeReference( const ScTokenArray* pTokens, ScSingleRefData& rOffset )
{
    int nCount = 0;
    if (pTokens)
    {
        formula::FormulaTokenArrayPlainIterator aIter( *pTokens );
        formula::FormulaToken* t;
        for( t = aIter.Next(); t; t = aIter.Next() )
        {
            switch( t->GetType() )
            {
                case formula::svSingleRef:
                {
                    ScSingleRefData& rRef1 = *t->GetSingleRef();
                    if ( rRef1.IsColRel() && rRef1.IsRowRel() && !rRef1.IsFlag3D() && rRef1.IsTabRel() )
                    {
                        nCount++;
                        if (nCount == 1)
                        {
                            rOffset = rRef1;
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }
    }
    return nCount == 1;
}

void SAL_CALL ScXMLConditionalFormatContext::endFastElement( sal_Int32 /*nElement*/ )
{
    ScDocument* pDoc = GetScImport().GetDocument();

    SCTAB nTab = GetScImport().GetTables().GetCurrentSheet();
    std::unique_ptr<ScConditionalFormat> pFormat(std::move(mxFormat));

    bool bEligibleForCache = true;
    bool bSingleRelativeReference = false;
    ScSingleRefData aOffsetForSingleRelRef;
    std::unique_ptr<ScTokenArray> pTokens;
    for (size_t nFormatEntryIx = 0; nFormatEntryIx < pFormat->size(); ++nFormatEntryIx)
    {
        auto pFormatEntry = pFormat->GetEntry(nFormatEntryIx);
        auto pCondFormatEntry = dynamic_cast<const ScCondFormatEntry*>(pFormatEntry);

        if (pCondFormatEntry == nullptr ||
            (pCondFormatEntry->GetOperation() != ScConditionMode::Equal &&
             pCondFormatEntry->GetOperation() != ScConditionMode::Direct))
        {
            bEligibleForCache = false;
            break;
        }

        ScAddress aSrcPos;
        OUString aSrcString = pCondFormatEntry->GetSrcString();
        if ( !aSrcString.isEmpty() )
            aSrcPos.Parse( aSrcString, pDoc );
        ScCompiler aComp( pDoc, aSrcPos );
        aComp.SetGrammar( formula::FormulaGrammar::GRAM_ODFF );
        pTokens = aComp.CompileString( pCondFormatEntry->GetExpression(aSrcPos, 0), "" );
        if (HasRelRefIgnoringSheet0Relative( pDoc, pTokens.get() ))
        {
            // In general not eligible, but some might be. We handle one very special case: When the
            // conditional format has one entry, the reference position is the first cell of the
            // range, and with a single fully relative reference in its expression. (Possibly these
            // conditions could be loosened, but I am too tired to think on that right now.)
            if (pFormat->size() == 1 &&
                pFormat->GetRange().size() == 1 &&
                pFormat->GetRange()[0].aStart == aSrcPos &&
                HasOneSingleFullyRelativeReference( pTokens.get(), aOffsetForSingleRelRef ))
            {
                bSingleRelativeReference = true;
            }
            else
            {
                bEligibleForCache = false;
                break;
            }
        }
    }

    if (bEligibleForCache)
    {
        for (auto& aCacheEntry : mrParent.maCache)
            if (aCacheEntry.mnAge < SAL_MAX_INT64)
                aCacheEntry.mnAge++;

        for (auto& aCacheEntry : mrParent.maCache)
        {
            if (!aCacheEntry.mpFormat)
                continue;

            if (aCacheEntry.mpFormat->size() != pFormat->size())
                continue;

            // Check if the conditional format is identical to an existing one (but with different range) and can be shared
            for (size_t nFormatEntryIx = 0; nFormatEntryIx < pFormat->size(); ++nFormatEntryIx)
            {
                auto pCacheFormatEntry = aCacheEntry.mpFormat->GetEntry(nFormatEntryIx);
                auto pFormatEntry = pFormat->GetEntry(nFormatEntryIx);
                if (pCacheFormatEntry->GetType() != pFormatEntry->GetType() ||
                    pFormatEntry->GetType() != ScFormatEntry::Type::Condition)
                    break;

                auto pCacheCondFormatEntry = static_cast<const ScCondFormatEntry*>(pCacheFormatEntry);
                auto pCondFormatEntry = static_cast<const ScCondFormatEntry*>(pFormatEntry);

                if (pCacheCondFormatEntry->GetStyle() != pCondFormatEntry->GetStyle())
                    break;

                // Note That comparing the formulas of the ScConditionEntry at this stage is
                // comparing just the *strings* of the formulas. For the bSingleRelativeReference
                // case we compare the tokenized ("compiled") formulas.
                if (bSingleRelativeReference)
                {
                    if (aCacheEntry.mbSingleRelativeReference &&
                        pTokens->EqualTokens(aCacheEntry.mpTokens.get()))
                        ;
                    else
                        break;
                }
                else if (!pCacheCondFormatEntry->IsEqual(*pCondFormatEntry, /*bIgnoreSrcPos*/true))
                {
                    break;
                }
                // If we get here on the last round through the for loop, we have a cache hit
                if (nFormatEntryIx == pFormat->size() - 1)
                {
                    // Mark cache entry as fresh, do necessary mangling of it and just return
                    aCacheEntry.mnAge = 0;
                    for (size_t k = 0; k < pFormat->GetRange().size(); ++k)
                        aCacheEntry.mpFormat->GetRangeList().Join(pFormat->GetRange()[k]);
                    return;
                }
            }
        }
    }

    sal_uLong nIndex = pDoc->AddCondFormat(std::move(pFormat), nTab);
    ScConditionalFormat* pInsertedFormat = pDoc->GetCondFormList(nTab)->GetFormat(nIndex);
    assert(pInsertedFormat && pInsertedFormat->GetKey() == nIndex);

    mrParent.mvCondFormatData.push_back( { pInsertedFormat, nTab } );

    if (bEligibleForCache)
    {
        // Not found in cache, replace oldest cache entry
        sal_Int64 nOldestAge = -1;
        size_t nIndexOfOldest = 0;
        for (auto& aCacheEntry : mrParent.maCache)
        {
            if (aCacheEntry.mnAge > nOldestAge)
            {
                nOldestAge = aCacheEntry.mnAge;
                nIndexOfOldest = (&aCacheEntry - &mrParent.maCache.front());
            }
        }
        mrParent.maCache[nIndexOfOldest].mpFormat = pInsertedFormat;
        mrParent.maCache[nIndexOfOldest].mbSingleRelativeReference = bSingleRelativeReference;
        mrParent.maCache[nIndexOfOldest].mpTokens = std::move(pTokens);
        mrParent.maCache[nIndexOfOldest].mnAge = 0;
    }
}

ScXMLConditionalFormatContext::~ScXMLConditionalFormatContext()
{
}

ScXMLColorScaleFormatContext::ScXMLColorScaleFormatContext( ScXMLImport& rImport,
                        ScConditionalFormat* pFormat):
    ScXMLImportContext( rImport ),
    pColorScaleFormat(nullptr)
{
    pColorScaleFormat = new ScColorScaleFormat(GetScImport().GetDocument());
    pFormat->AddEntry(pColorScaleFormat);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL ScXMLColorScaleFormatContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( CALC_EXT, XML_COLOR_SCALE_ENTRY ):
            pContext = new ScXMLColorScaleFormatEntryContext( GetScImport(), pAttribList, pColorScaleFormat );
            break;
        default:
            break;
    }

    return pContext;
}

ScXMLDataBarFormatContext::ScXMLDataBarFormatContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScConditionalFormat* pFormat):
    ScXMLImportContext( rImport ),
    mpFormatData(nullptr),
    mpParent(pFormat),
    mnIndex(0)
{
    OUString sPositiveColor;
    OUString sNegativeColor;
    OUString sGradient;
    OUString sAxisPosition;
    OUString sShowValue;
    OUString sAxisColor;
    OUString sMinLength;
    OUString sMaxLength;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( CALC_EXT, XML_POSITIVE_COLOR ):
                    sPositiveColor = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_GRADIENT ):
                    sGradient = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_NEGATIVE_COLOR ):
                    sNegativeColor = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_AXIS_POSITION ):
                    sAxisPosition = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_SHOW_VALUE ):
                    sShowValue = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_AXIS_COLOR ):
                    sAxisColor = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_MIN_LENGTH ):
                    sMinLength = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_MAX_LENGTH ):
                    sMaxLength = aIter.toString();
                break;
                default:
                    break;
            }
        }
    }

    ScDataBarFormat* pDataBarFormat = new ScDataBarFormat(rImport.GetDocument());
    mpFormatData = new ScDataBarFormatData();
    pDataBarFormat->SetDataBarData(mpFormatData);
    if(!sGradient.isEmpty())
    {
        bool bGradient = true;
        (void)sax::Converter::convertBool( bGradient, sGradient);
        mpFormatData->mbGradient = bGradient;
    }

    if(!sPositiveColor.isEmpty())
    {
        sax::Converter::convertColor( mpFormatData->maPositiveColor, sPositiveColor );
    }

    if(!sNegativeColor.isEmpty())
    {
        // we might check here for 0xff0000 and don't write it
        Color nColor;
        sax::Converter::convertColor( nColor, sNegativeColor );
        mpFormatData->mpNegativeColor.reset(new Color(nColor));
    }
    else
        mpFormatData->mbNeg = false;

    if(!sAxisPosition.isEmpty())
    {
        if(sAxisPosition == "middle")
            mpFormatData->meAxisPosition = databar::MIDDLE;
        else if (sAxisPosition == "none")
            mpFormatData->meAxisPosition = databar::NONE;
    }

    if(!sAxisColor.isEmpty())
    {
        sax::Converter::convertColor( mpFormatData->maAxisColor, sAxisColor );
    }

    if(!sShowValue.isEmpty())
    {
        bool bShowValue = true;
        (void)sax::Converter::convertBool( bShowValue, sShowValue );
        mpFormatData->mbOnlyBar = !bShowValue;
    }

    if (!sMinLength.isEmpty())
    {
        double nVal = sMinLength.toDouble();
        mpFormatData->mnMinLength = nVal;
    }

    if (!sMaxLength.isEmpty())
    {
        double nVal = sMaxLength.toDouble();
        if (nVal == 0.0)
            nVal = 100.0;
        mpFormatData->mnMaxLength = nVal;
    }

    pFormat->AddEntry(pDataBarFormat);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL ScXMLDataBarFormatContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( CALC_EXT, XML_FORMATTING_ENTRY ):
        case XML_ELEMENT( CALC_EXT, XML_DATA_BAR_ENTRY ):
        {
            ScColorScaleEntry* pEntry(nullptr);
            pContext = new ScXMLFormattingEntryContext( GetScImport(), pAttribList, pEntry );
            pEntry->SetRepaintCallback(mpParent);
            if(mnIndex == 0)
            {
                mpFormatData->mpLowerLimit.reset(pEntry);
            }
            else if (mnIndex == 1)
            {
                mpFormatData->mpUpperLimit.reset(pEntry);
            }
            else
            {
                // data bars only support 2 entries
                assert(false);
            }
            ++mnIndex;
        }
        break;
        default:
            break;
    }

    return pContext;
}

ScXMLIconSetFormatContext::ScXMLIconSetFormatContext(ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScConditionalFormat* pFormat):
    ScXMLImportContext( rImport ),
    mpParent(pFormat)
{
    OUString aIconSetType, sShowValue;
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( CALC_EXT, XML_ICON_SET_TYPE ):
                    aIconSetType = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_SHOW_VALUE ):
                    sShowValue = aIter.toString();
                break;
                default:
                    break;
            }
        }
    }

    const ScIconSetMap* pMap = ScIconSetFormat::g_IconSetMap;
    ScIconSetType eType = IconSet_3Arrows;
    for(; pMap->pName; ++pMap)
    {
        OUString aName = OUString::createFromAscii(pMap->pName);
        if(aName ==aIconSetType)
        {
            eType = pMap->eType;
            break;
        }
    }

    ScIconSetFormat* pIconSetFormat = new ScIconSetFormat(GetScImport().GetDocument());
    ScIconSetFormatData* pIconSetFormatData = new ScIconSetFormatData;

    if(!sShowValue.isEmpty())
    {
        bool bShowValue = true;
        (void)sax::Converter::convertBool( bShowValue, sShowValue );
        pIconSetFormatData->mbShowValue = !bShowValue;
    }

    pIconSetFormatData->eIconSetType = eType;
    pIconSetFormat->SetIconSetData(pIconSetFormatData);
    pFormat->AddEntry(pIconSetFormat);

    mpFormatData = pIconSetFormatData;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL ScXMLIconSetFormatContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( CALC_EXT, XML_FORMATTING_ENTRY ):
            {
                ScColorScaleEntry* pEntry(nullptr);
                pContext = new ScXMLFormattingEntryContext( GetScImport(), pAttribList, pEntry );
                mpFormatData->m_Entries.push_back(std::unique_ptr<ScColorScaleEntry>(pEntry));
                pEntry->SetRepaintCallback(mpParent);
            }
            break;
        default:
            break;
    }

    return pContext;
}

namespace {

void GetConditionData(const OUString& rValue, ScConditionMode& eMode, OUString& rExpr1, OUString& rExpr2)
{
    if(rValue.startsWith("unique"))
    {
        eMode = ScConditionMode::NotDuplicate;
    }
    else if(rValue.startsWith("duplicate"))
    {
        eMode = ScConditionMode::Duplicate;
    }
    else if(rValue.startsWith("between"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 8;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ',');
        rExpr2 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = ScConditionMode::Between;
    }
    else if(rValue.startsWith("not-between"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 12;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ',');
        rExpr2 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = ScConditionMode::NotBetween;
    }
    else if(rValue.startsWith("<="))
    {
        rExpr1 = rValue.copy(2);
        eMode = ScConditionMode::EqLess;
    }
    else if(rValue.startsWith(">="))
    {
        rExpr1 = rValue.copy(2);
        eMode = ScConditionMode::EqGreater;
    }
    else if(rValue.startsWith("!="))
    {
        rExpr1 = rValue.copy(2);
        eMode = ScConditionMode::NotEqual;
    }
    else if(rValue.startsWith("<"))
    {
        rExpr1 = rValue.copy(1);
        eMode = ScConditionMode::Less;
    }
    else if(rValue.startsWith("="))
    {
        rExpr1 = rValue.copy(1);
        eMode = ScConditionMode::Equal;
    }
    else if(rValue.startsWith(">"))
    {
        rExpr1 = rValue.copy(1);
        eMode = ScConditionMode::Greater;
    }
    else if(rValue.startsWith("formula-is"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 11;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = ScConditionMode::Direct;
    }
    else if(rValue.startsWith("top-elements"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 13;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = ScConditionMode::Top10;
    }
    else if(rValue.startsWith("bottom-elements"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 16;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = ScConditionMode::Bottom10;
    }
    else if(rValue.startsWith("top-percent"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 12;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = ScConditionMode::TopPercent;
    }
    else if(rValue.startsWith("bottom-percent"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 15;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = ScConditionMode::BottomPercent;
    }
    else if(rValue.startsWith("above-average"))
    {
        eMode = ScConditionMode::AboveAverage;
    }
    else if(rValue.startsWith("below-average"))
    {
        eMode = ScConditionMode::BelowAverage;
    }
    else if(rValue.startsWith("above-equal-average"))
    {
        eMode = ScConditionMode::AboveEqualAverage;
    }
    else if(rValue.startsWith("below-equal-average"))
    {
        eMode = ScConditionMode::BelowEqualAverage;
    }
    else if(rValue.startsWith("is-error"))
    {
        eMode = ScConditionMode::Error;
    }
    else if(rValue.startsWith("is-no-error"))
    {
        eMode = ScConditionMode::NoError;
    }
    else if(rValue.startsWith("begins-with"))
    {
        eMode = ScConditionMode::BeginsWith;
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 12;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
    }
    else if(rValue.startsWith("ends-with"))
    {
        eMode = ScConditionMode::EndsWith;
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 10;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
    }
    else if(rValue.startsWith("contains-text"))
    {
        eMode = ScConditionMode::ContainsText;
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 14;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
    }
    else if(rValue.startsWith("not-contains-text"))
    {
        eMode = ScConditionMode::NotContainsText;
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 18;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
    }
    else
        eMode = ScConditionMode::NONE;
}

}

ScXMLCondContext::ScXMLCondContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScConditionalFormat* pFormat ):
    ScXMLImportContext( rImport )
{
    OUString sExpression;
    OUString sStyle;
    OUString sAddress;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( CALC_EXT, XML_VALUE ):
                    sExpression = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_APPLY_STYLE_NAME ):
                    sStyle = ScStyleNameConversion::ProgrammaticToDisplayName(aIter.toString(), SfxStyleFamily::Para );
                break;
                case XML_ELEMENT( CALC_EXT, XML_BASE_CELL_ADDRESS ):
                    sAddress = aIter.toString();
                break;
                default:
                    break;
            }
        }
    }

    OUString aExpr1;
    OUString aExpr2;
    ScConditionMode eMode;
    GetConditionData(sExpression, eMode, aExpr1, aExpr2);

    ScCondFormatEntry* pFormatEntry = new ScCondFormatEntry(eMode, aExpr1, aExpr2, GetScImport().GetDocument(), ScAddress(), sStyle,
                                                        OUString(), OUString(), formula::FormulaGrammar::GRAM_ODFF, formula::FormulaGrammar::GRAM_ODFF);
    pFormatEntry->SetSrcString(sAddress);

    pFormat->AddEntry(pFormatEntry);
}

namespace {

void setColorEntryType(const OUString& rType, ScColorScaleEntry* pEntry, const OUString& rFormula,
        ScXMLImport& rImport)
{
    if(rType == "minimum")
        pEntry->SetType(COLORSCALE_MIN);
    else if(rType == "maximum")
        pEntry->SetType(COLORSCALE_MAX);
    else if(rType == "percentile")
        pEntry->SetType(COLORSCALE_PERCENTILE);
    else if(rType == "percent")
        pEntry->SetType(COLORSCALE_PERCENT);
    else if(rType == "formula")
    {
        pEntry->SetType(COLORSCALE_FORMULA);
        //position does not matter, only table is important
        pEntry->SetFormula(rFormula, rImport.GetDocument(), ScAddress(0,0,rImport.GetTables().GetCurrentSheet()), formula::FormulaGrammar::GRAM_ODFF);
    }
    else if(rType == "auto-minimum")
        pEntry->SetType(COLORSCALE_AUTO);
    else if(rType == "auto-maximum")
        pEntry->SetType(COLORSCALE_AUTO);
}

}

ScXMLColorScaleFormatEntryContext::ScXMLColorScaleFormatEntryContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScColorScaleFormat* pFormat):
    ScXMLImportContext( rImport )
{
    double nVal = 0;
    Color aColor;

    OUString sType;
    OUString sVal;
    OUString sColor;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( CALC_EXT, XML_TYPE ):
                    sType = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_VALUE ):
                    sVal = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_COLOR ):
                    sColor = aIter.toString();
                break;
                default:
                    break;
            }
        }
    }

    sax::Converter::convertColor(aColor, sColor);

    if(!sVal.isEmpty())
        sax::Converter::convertDouble(nVal, sVal);

    auto pFormatEntry = new ScColorScaleEntry(nVal, aColor);
    setColorEntryType(sType, pFormatEntry, sVal, GetScImport());
    pFormat->AddEntry(pFormatEntry);
}

ScXMLFormattingEntryContext::ScXMLFormattingEntryContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScColorScaleEntry*& pColorScaleEntry):
    ScXMLImportContext( rImport )
{
    OUString sVal;
    OUString sType;

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( CALC_EXT, XML_TYPE ):
                    sType = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_VALUE ):
                    sVal = aIter.toString();
                break;
                default:
                    break;
            }
        }
    }

    double nVal = 0;
    if(!sVal.isEmpty())
        sax::Converter::convertDouble(nVal, sVal);

    pColorScaleEntry = new ScColorScaleEntry(nVal, Color());
    setColorEntryType(sType, pColorScaleEntry, sVal, GetScImport());
}

namespace {

condformat::ScCondFormatDateType getDateFromString(const OUString& rString)
{
    if(rString == "today")
        return condformat::TODAY;
    else if(rString == "yesterday")
        return condformat::YESTERDAY;
    else if(rString == "tomorrow")
        return condformat::TOMORROW;
    else if(rString == "last-7-days")
        return condformat::LAST7DAYS;
    else if(rString == "this-week")
        return condformat::THISWEEK;
    else if(rString == "last-week")
        return condformat::LASTWEEK;
    else if(rString == "next-week")
        return condformat::NEXTWEEK;
    else if(rString == "this-month")
        return condformat::THISMONTH;
    else if(rString == "last-month")
        return condformat::LASTMONTH;
    else if(rString == "next-month")
        return condformat::NEXTMONTH;
    else if(rString == "this-year")
        return condformat::THISYEAR;
    else if(rString == "last-year")
        return condformat::LASTYEAR;
    else if(rString == "next-year")
        return condformat::NEXTYEAR;

    SAL_WARN("sc", "unknown date type: " << rString);
    return condformat::TODAY;
}

}

ScXMLDateContext::ScXMLDateContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScConditionalFormat* pFormat ):
    ScXMLImportContext( rImport )
{
    OUString sDateType, sStyle;
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( CALC_EXT, XML_DATE ):
                    sDateType = aIter.toString();
                break;
                case XML_ELEMENT( CALC_EXT, XML_STYLE ):
                    sStyle = ScStyleNameConversion::ProgrammaticToDisplayName(aIter.toString(), SfxStyleFamily::Para );
                break;
                default:
                    break;
            }
        }
    }

    ScCondDateFormatEntry* pFormatEntry = new ScCondDateFormatEntry(GetScImport().GetDocument());
    pFormatEntry->SetStyleName(sStyle);
    pFormatEntry->SetDateType(getDateFromString(sDateType));
    pFormat->AddEntry(pFormatEntry);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
