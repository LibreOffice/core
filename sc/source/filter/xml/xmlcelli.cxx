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

#include "xmlcelli.hxx"
#include "xmlimprt.hxx"
#include "xmltabi.hxx"
#include "xmlstyli.hxx"
#include "xmlannoi.hxx"
#include "global.hxx"
#include "cellvalue.hxx"
#include "document.hxx"
#include "cellsuno.hxx"
#include "docuno.hxx"
#include "unonames.hxx"
#include "postit.hxx"
#include "sheetdata.hxx"
#include "docsh.hxx"
#include "cellform.hxx"
#include "validat.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "docpool.hxx"

#include "XMLTableShapeImportHelper.hxx"
#include "XMLStylesImportHelper.hxx"
#include "celltextparacontext.hxx"

#include "arealink.hxx"
#include <sfx2/linkmgr.hxx>
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "scerrors.hxx"
#include "editutil.hxx"
#include "formulacell.hxx"
#include "editattributemap.hxx"
#include "stringutil.hxx"
#include "tokenarray.hxx"
#include "scmatrix.hxx"
#include "documentimport.hxx"
#include <datastream.hxx>
#include <rangeutl.hxx>

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/families.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/prstylei.hxx>
#include <svl/zforlist.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/unoapi.hxx>
#include <svl/languageoptions.hxx>
#include <svl/sharedstringpool.hxx>
#include <svtools/miscopt.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>

#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/document/XActionLockable.hpp>

#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>

#include <rtl/ustrbuf.hxx>
#include <tools/date.hxx>
#include <i18nlangtag/lang.h>
#include <comphelper/extract.hxx>
#include <o3tl/make_unique.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableRowCellContext::ParaFormat::ParaFormat(ScEditEngineDefaulter& rEditEngine) :
    maItemSet(rEditEngine.GetEmptyItemSet()) {}

ScXMLTableRowCellContext::Field::Field(SvxFieldData* pData) : mpData(pData) {}

ScXMLTableRowCellContext::Field::~Field()
{
    delete mpData;
}

ScXMLTableRowCellContext::ScXMLTableRowCellContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      const bool bTempIsCovered,
                                      const sal_Int32 nTempRepeatedRows ) :
    ScXMLImportContext(rImport, nPrfx, rLName),
    mpEditEngine(GetScImport().GetEditEngine()),
    mnCurParagraph(0),
    pDetectiveObjVec(nullptr),
    pCellRangeSource(nullptr),
    fValue(0.0),
    nMergedRows(1),
    nMatrixRows(0),
    nRepeatedRows(nTempRepeatedRows),
    nMergedCols(1),
    nMatrixCols(0),
    nColsRepeated(1),
    rXMLImport((ScXMLImport&)rImport),
    eGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT),
    nCellType(util::NumberFormat::TEXT),
    bIsMerged(false),
    bIsMatrix(false),
    bIsCovered(bTempIsCovered),
    bIsEmpty(true),
    mbNewValueType(false),
    mbErrorValue(false),
    bSolarMutexLocked(false),
    bFormulaTextResult(false),
    mbPossibleErrorCell(false),
    mbCheckWithCompilerForError(false),
    mbEditEngineHasText(false),
    mbHasFormatRuns(false),
    mbHasStyle(false)
{
    rtl::math::setNan(&fValue); // NaN by default

    rXMLImport.SetRemoveLastChar(false);
    rXMLImport.GetTables().AddColumn(bTempIsCovered);
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    OUString aLocalName;
    OUString* pStyleName = nullptr;
    OUString* pCurrencySymbol = nullptr;
    const SvXMLTokenMap& rTokenMap = rImport.GetTableRowCellAttrTokenMap();
    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        sal_uInt16 nAttrPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
            xAttrList->getNameByIndex(i), &aLocalName);

        const OUString& sValue = xAttrList->getValueByIndex(i);
        sal_uInt16 nToken = rTokenMap.Get(nAttrPrefix, aLocalName);
        switch (nToken)
        {
            case XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME:
                pStyleName = new OUString(sValue);
                mbHasStyle = true;
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_CONTENT_VALIDATION_NAME:
                OSL_ENSURE(!maContentValidationName, "here should be only one Validation Name");
                if (!sValue.isEmpty())
                    maContentValidationName.reset(sValue);
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS:
                bIsMerged = true;
                nMergedRows = static_cast<SCROW>(sValue.toInt32());
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS:
                bIsMerged = true;
                nMergedCols = static_cast<SCCOL>(sValue.toInt32());
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS:
                bIsMatrix = true;
                nMatrixCols = static_cast<SCCOL>(sValue.toInt32());
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS:
                bIsMatrix = true;
                nMatrixRows = static_cast<SCROW>(sValue.toInt32());
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED:
                nColsRepeated = static_cast<SCCOL>(std::min<sal_Int32>( MAXCOLCOUNT,
                            std::max( sValue.toInt32(), static_cast<sal_Int32>(1) ) ));
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE:
                nCellType = GetScImport().GetCellType(sValue);
                bIsEmpty = false;
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_NEW_VALUE_TYPE:
                if(sValue == "error")
                    mbErrorValue = true;
                else
                    nCellType = GetScImport().GetCellType(sValue);
                bIsEmpty = false;
                mbNewValueType = true;
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    ::sax::Converter::convertDouble(fValue, sValue);
                    bIsEmpty = false;

                    //if office:value="0", let's get the text:p in case this is
                    //a special case in HasSpecialCaseFormulaText(). If it
                    //turns out not to be a special case, we'll use the 0 value.
                    if(fValue == 0.0)
                        bFormulaTextResult = true;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE:
            {
                if (!sValue.isEmpty() && rXMLImport.SetNullDateOnUnitConverter())
                {
                    rXMLImport.GetMM100UnitConverter().convertDateTime(fValue, sValue);
                    bIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    ::sax::Converter::convertDuration(fValue, sValue);
                    bIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    OSL_ENSURE(!maStringValue, "here should be only one string value");
                    maStringValue.reset(sValue);
                    bIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    if ( IsXMLToken(sValue, XML_TRUE) )
                        fValue = 1.0;
                    else if ( IsXMLToken(sValue, XML_FALSE) )
                        fValue = 0.0;
                    else
                        ::sax::Converter::convertDouble(fValue, sValue);
                    bIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_FORMULA:
            {
                if (!sValue.isEmpty())
                {
                    OSL_ENSURE(!maFormula, "here should be only one formula");
                    OUString aFormula, aFormulaNmsp;
                    rXMLImport.ExtractFormulaNamespaceGrammar( aFormula, aFormulaNmsp, eGrammar, sValue );
                    maFormula.reset( FormulaWithNamespace(aFormula, aFormulaNmsp) );
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_CURRENCY:
                pCurrencySymbol = new OUString(sValue);
            break;
            default:
                ;
        }
    }
    if (maFormula)
    {
        if (nCellType == util::NumberFormat::TEXT)
            bFormulaTextResult = true;
        if(nCellType == util::NumberFormat::DATETIME)
            nCellType = util::NumberFormat::UNDEFINED;
        //if bIsEmpty is true at this point, then there is no office value.
        //we must get the text:p (even if it is empty) in case this a special
        //case in HasSpecialCaseFormulaText().
        if(bIsEmpty)
            bFormulaTextResult = true;
    }
    rXMLImport.GetStylesImportHelper()->SetAttributes(pStyleName, pCurrencySymbol, nCellType);
}

ScXMLTableRowCellContext::~ScXMLTableRowCellContext()
{
    delete pDetectiveObjVec;
    delete pCellRangeSource;
}

void ScXMLTableRowCellContext::LockSolarMutex()
{
    if (!bSolarMutexLocked)
    {
        GetScImport().LockSolarMutex();
        bSolarMutexLocked = true;
    }
}

void ScXMLTableRowCellContext::UnlockSolarMutex()
{
    if (bSolarMutexLocked)
    {
        GetScImport().UnlockSolarMutex();
        bSolarMutexLocked = false;
    }
}

namespace {

bool cellExists( const ScAddress& rCellPos )
{
    return( rCellPos.Col() >= 0 && rCellPos.Row() >= 0 &&
            rCellPos.Col() <= MAXCOL && rCellPos.Row() <= MAXROW );
}

}

void ScXMLTableRowCellContext::PushParagraphSpan(const OUString& rSpan, const OUString& rStyleName)
{
    sal_Int32 nBegin = maParagraph.getLength();
    sal_Int32 nEnd = nBegin + rSpan.getLength();
    maParagraph.append(rSpan);

    PushFormat(nBegin, nEnd, rStyleName);
}

void ScXMLTableRowCellContext::PushParagraphField(SvxFieldData* pData, const OUString& rStyleName)
{
    mbHasFormatRuns = true;
    maFields.push_back(o3tl::make_unique<Field>(pData));
    Field& rField = *maFields.back().get();

    sal_Int32 nPos = maParagraph.getLength();
    maParagraph.append('\1'); // Placeholder text for inserted field item.
    rField.maSelection.nStartPara = mnCurParagraph;
    rField.maSelection.nEndPara = mnCurParagraph;
    rField.maSelection.nStartPos = nPos;
    rField.maSelection.nEndPos = nPos+1;

    PushFormat(nPos, nPos+1, rStyleName);
}

void ScXMLTableRowCellContext::PushFormat(sal_Int32 nBegin, sal_Int32 nEnd, const OUString& rStyleName)
{
    if (rStyleName.isEmpty())
        return;

    // Get the style information from xmloff.
    rtl::Reference<XMLPropertySetMapper> xMapper = GetImport().GetTextImport()->GetTextImportPropertySetMapper()->getPropertySetMapper();
    if (!xMapper.is())
        // We can't do anything without the mapper.
        return;

    sal_Int32 nEntryCount = xMapper->GetEntryCount();

    SvXMLStylesContext* pAutoStyles = GetImport().GetAutoStyles();

    // Style name for text span corresponds with the name of an automatic style.
    const XMLPropStyleContext* pStyle = dynamic_cast<const XMLPropStyleContext*>(
        pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TEXT_TEXT, rStyleName));

    if (!pStyle)
        // No style by that name found.
        return;

    const std::vector<XMLPropertyState>& rProps = pStyle->GetProperties();
    if (rProps.empty())
        return;

    const ScXMLEditAttributeMap& rEditAttrMap = GetScImport().GetEditAttributeMap();

    mbHasFormatRuns = true;
    maFormats.push_back(o3tl::make_unique<ParaFormat>(*mpEditEngine));
    ParaFormat& rFmt = *maFormats.back().get();
    rFmt.maSelection.nStartPara = rFmt.maSelection.nEndPara = mnCurParagraph;
    rFmt.maSelection.nStartPos = nBegin;
    rFmt.maSelection.nEndPos = nEnd;

    // Store the used text styles for export.
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rXMLImport.GetModel())->GetSheetSaveData();
    ScAddress aCellPos = rXMLImport.GetTables().GetCurrentCellPos();
    pSheetData->AddTextStyle(rStyleName, aCellPos, rFmt.maSelection);

    std::unique_ptr<SfxPoolItem> pPoolItem;
    sal_uInt16 nLastItemID = EE_CHAR_END + 1;

    std::vector<XMLPropertyState>::const_iterator it = rProps.begin(), itEnd = rProps.end();
    for (; it != itEnd; ++it)
    {
        if (it->mnIndex == -1 || it->mnIndex >= nEntryCount)
            continue;

        const OUString& rName = xMapper->GetEntryAPIName(it->mnIndex);
        const ScXMLEditAttributeMap::Entry* pEntry = rEditAttrMap.getEntryByAPIName(rName);
        if (!pEntry)
            continue;

        if (nLastItemID != pEntry->mnItemID && pPoolItem)
        {
            // Flush the last item when the item ID changes.
            rFmt.maItemSet.Put(*pPoolItem);
            pPoolItem.reset();
        }

        switch (pEntry->mnItemID)
        {
            case EE_CHAR_FONTINFO:
            case EE_CHAR_FONTINFO_CJK:
            case EE_CHAR_FONTINFO_CTL:
            {
                // Font properties need to be consolidated into a single item.
                if (!pPoolItem)
                    pPoolItem.reset(new SvxFontItem(pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_WEIGHT:
            case EE_CHAR_WEIGHT_CJK:
            case EE_CHAR_WEIGHT_CTL:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxWeightItem(WEIGHT_NORMAL, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_FONTHEIGHT:
            case EE_CHAR_FONTHEIGHT_CJK:
            case EE_CHAR_FONTHEIGHT_CTL:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxFontHeightItem(240, 100, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_ITALIC:
            case EE_CHAR_ITALIC_CJK:
            case EE_CHAR_ITALIC_CTL:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxPostureItem(ITALIC_NONE, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_UNDERLINE:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxUnderlineItem(LINESTYLE_NONE, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_OVERLINE:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxOverlineItem(LINESTYLE_NONE, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_COLOR:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxColorItem(pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_WLM:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxWordLineModeItem(false, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_STRIKEOUT:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxCrossedOutItem(STRIKEOUT_NONE, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_RELIEF:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxCharReliefItem(FontRelief::NONE, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_OUTLINE:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxContourItem(false, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_SHADOW:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxShadowedItem(false, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_KERNING:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxKerningItem(0, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_PAIRKERNING:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxAutoKernItem(false, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_FONTWIDTH:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxCharScaleWidthItem(100, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_ESCAPEMENT:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxEscapementItem(pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_EMPHASISMARK:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxEmphasisMarkItem(FontEmphasisMark::NONE, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_LANGUAGE:
            case EE_CHAR_LANGUAGE_CJK:
            case EE_CHAR_LANGUAGE_CTL:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxLanguageItem(LANGUAGE_DONTKNOW, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            default:
                ;
        }

        nLastItemID = pEntry->mnItemID;
    }

    if (pPoolItem)
        rFmt.maItemSet.Put(*pPoolItem);
}

OUString ScXMLTableRowCellContext::GetFirstParagraph() const
{
    if (!maFirstParagraph)
        return mpEditEngine->GetText(0);

    return *maFirstParagraph;
}

void ScXMLTableRowCellContext::PushParagraphFieldDate(const OUString& rStyleName)
{
    PushParagraphField(new SvxDateField, rStyleName);
}

void ScXMLTableRowCellContext::PushParagraphFieldSheetName(const OUString& rStyleName)
{
    SCTAB nTab = GetScImport().GetTables().GetCurrentCellPos().Tab();
    PushParagraphField(new SvxTableField(nTab), rStyleName);
}

void ScXMLTableRowCellContext::PushParagraphFieldDocTitle(const OUString& rStyleName)
{
    PushParagraphField(new SvxFileField, rStyleName);
}

void ScXMLTableRowCellContext::PushParagraphFieldURL(
    const OUString& rURL, const OUString& rRep, const OUString& rStyleName)
{
#ifdef UNX
    OUString aAbsURL = GetScImport().GetAbsoluteReference(rURL, true);
#else
    OUString aAbsURL = GetScImport().GetAbsoluteReference(rURL);
#endif
    PushParagraphField(new SvxURLField(aAbsURL, rRep, SVXURLFORMAT_REPR), rStyleName);
}

void ScXMLTableRowCellContext::PushParagraphEnd()
{
    // EditEngine always has at least one paragraph even when its content is empty.

    if (mbEditEngineHasText)
    {
        if (maFirstParagraph)
        {
            // Flush the cached first paragraph first.
            mpEditEngine->Clear();
            mpEditEngine->SetText(*maFirstParagraph);
            maFirstParagraph.reset();
        }
        mpEditEngine->InsertParagraph(mpEditEngine->GetParagraphCount(), maParagraph.makeStringAndClear());
    }
    else if (mbHasFormatRuns)
    {
        mpEditEngine->Clear();
        mpEditEngine->SetText(maParagraph.makeStringAndClear());
        mbEditEngineHasText = true;
    }
    else if (mnCurParagraph == 0)
    {
        maFirstParagraph.reset(maParagraph.makeStringAndClear());
        mbEditEngineHasText = true;
    }

    ++mnCurParagraph;
}

SvXMLImportContext *ScXMLTableRowCellContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = rXMLImport.GetTableRowCellElemTokenMap();
    bool bTextP(false);
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_TABLE_ROW_CELL_P:
        {
            bIsEmpty = false;
            bTextP = true;

            pContext = new ScXMLCellTextParaContext(rXMLImport, nPrefix, rLName, *this);
        }
        break;
        case XML_TOK_TABLE_ROW_CELL_TABLE:
        {
            SAL_WARN("sc", "ScXMLTableRowCellContext::CreateChildContext: subtables are not supported");
        }
        break;
        case XML_TOK_TABLE_ROW_CELL_ANNOTATION:
        {
            bIsEmpty = false;
            OSL_ENSURE( !mxAnnotationData.get(), "ScXMLTableRowCellContext::CreateChildContext - multiple annotations in one cell" );
            mxAnnotationData.reset( new ScXMLAnnotationData );
            pContext = new ScXMLAnnotationContext( rXMLImport, nPrefix, rLName,
                                                    xAttrList, *mxAnnotationData, this);
        }
        break;
        case XML_TOK_TABLE_ROW_CELL_DETECTIVE:
        {
            bIsEmpty = false;
            if (!pDetectiveObjVec)
                pDetectiveObjVec = new ScMyImpDetectiveObjVec();
            pContext = new ScXMLDetectiveContext(
                rXMLImport, nPrefix, rLName, pDetectiveObjVec );
        }
        break;
        case XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE:
        {
            bIsEmpty = false;
            if (!pCellRangeSource)
                pCellRangeSource = new ScMyImpCellRangeSource();
            pContext = new ScXMLCellRangeSourceContext(
                rXMLImport, nPrefix, rLName, xAttrList, pCellRangeSource );
        }
        break;
    }

    if (!pContext && !bTextP)
    {
        ScAddress aCellPos = rXMLImport.GetTables().GetCurrentCellPos();
        uno::Reference<drawing::XShapes> xShapes (rXMLImport.GetTables().GetCurrentXShapes());
        if (xShapes.is())
        {
            if (aCellPos.Col() > MAXCOL)
                aCellPos.SetCol(MAXCOL);
            if (aCellPos.Row() > MAXROW)
                aCellPos.SetRow(MAXROW);
            XMLTableShapeImportHelper* pTableShapeImport =
                    static_cast< XMLTableShapeImportHelper* >( rXMLImport.GetShapeImport().get() );
            pTableShapeImport->SetOnTable(false);
            css::table::CellAddress aCellAddress;
            ScUnoConversion::FillApiAddress( aCellAddress, aCellPos );
            pTableShapeImport->SetCell(aCellAddress);
            pContext = rXMLImport.GetShapeImport()->CreateGroupChildContext(
                rXMLImport, nPrefix, rLName, xAttrList, xShapes);
            if (pContext)
            {
                bIsEmpty = false;
                rXMLImport.ProgressBarIncrement();
            }
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableRowCellContext::DoMerge( const ScAddress& rScAddress, const SCCOL nCols, const SCROW nRows )
{
    SCCOL mergeToCol = rScAddress.Col() + nCols;
    SCROW mergeToRow = rScAddress.Row() + nRows;
    bool bInBounds = rScAddress.Col() <= MAXCOL && rScAddress.Row() <= MAXROW &&
                       mergeToCol <= MAXCOL && mergeToRow <= MAXROW;
    if( bInBounds )
    {
        rXMLImport.GetDocument()->DoMerge( rScAddress.Tab(),
            rScAddress.Col(), rScAddress.Row(), mergeToCol, mergeToRow );
    }
}

namespace {

ScValidationMode validationTypeToMode( const sheet::ValidationType eVType )
{
    ScValidationMode eMode;
    switch( eVType )
    {
        case sheet::ValidationType_WHOLE:               eMode = SC_VALID_WHOLE;     break;
        case sheet::ValidationType_DECIMAL:             eMode = SC_VALID_DECIMAL;   break;
        case sheet::ValidationType_DATE:                eMode = SC_VALID_DATE;      break;
        case sheet::ValidationType_TIME:                eMode = SC_VALID_TIME;      break;
        case sheet::ValidationType_TEXT_LEN:            eMode = SC_VALID_TEXTLEN;   break;
        case sheet::ValidationType_LIST:                eMode = SC_VALID_LIST;      break;
        case sheet::ValidationType_CUSTOM:              eMode = SC_VALID_CUSTOM;    break;
        default:                                        eMode = SC_VALID_ANY;       break;
    }
    return eMode;
}

ScValidErrorStyle validAlertToValidError( const sheet::ValidationAlertStyle eVAlertStyle )
{
    ScValidErrorStyle eVErrStyle;
    switch( eVAlertStyle )
    {
        case sheet::ValidationAlertStyle_STOP:          eVErrStyle = SC_VALERR_STOP;      break;
        case sheet::ValidationAlertStyle_WARNING:       eVErrStyle = SC_VALERR_WARNING;   break;
        case sheet::ValidationAlertStyle_MACRO:         eVErrStyle = SC_VALERR_MACRO;     break;
        default:                                        eVErrStyle = SC_VALERR_INFO;      break;
        //should INFO be the default?  seems to be the most unobtrusive choice.
    }
    return eVErrStyle;
}

}

void ScXMLTableRowCellContext::SetContentValidation( const ScRange& rScRange )
{
    if (maContentValidationName)
    {
        ScDocument* pDoc = rXMLImport.GetDocument();
        ScMyImportValidation aValidation;
        aValidation.eGrammar1 = aValidation.eGrammar2 = pDoc->GetStorageGrammar();
        if( rXMLImport.GetValidation(*maContentValidationName, aValidation) )
        {
            ScValidationData aScValidationData(
                validationTypeToMode(aValidation.aValidationType),
                ScConditionEntry::GetModeFromApi(static_cast<sal_Int32>(aValidation.aOperator)),
                aValidation.sFormula1, aValidation.sFormula2, pDoc, ScAddress(),
                aValidation.sFormulaNmsp1, aValidation.sFormulaNmsp2,
                aValidation.eGrammar1, aValidation.eGrammar2
            );

            aScValidationData.SetIgnoreBlank( aValidation.bIgnoreBlanks );
            aScValidationData.SetListType( aValidation.nShowList );

            // set strings for error / input even if disabled (and disable afterwards)
            aScValidationData.SetInput( aValidation.sImputTitle, aValidation.sImputMessage );
            if( !aValidation.bShowImputMessage )
                aScValidationData.ResetInput();
            aScValidationData.SetError( aValidation.sErrorTitle, aValidation.sErrorMessage, validAlertToValidError(aValidation.aAlertStyle) );
            if( !aValidation.bShowErrorMessage )
                aScValidationData.ResetError();

            if( !aValidation.sBaseCellAddress.isEmpty() )
                aScValidationData.SetSrcString( aValidation.sBaseCellAddress );

            sal_uLong nIndex = pDoc->AddValidationEntry( aScValidationData );

            ScPatternAttr aPattern( pDoc->GetPool() );
            aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, nIndex ) );
            if( rScRange.aStart == rScRange.aEnd )  //for a single cell
            {
                pDoc->ApplyPattern( rScRange.aStart.Col(), rScRange.aStart.Row(),
                                    rScRange.aStart.Tab(), aPattern );
            }
            else  //for repeating cells
            {
                pDoc->ApplyPatternAreaTab( rScRange.aStart.Col(), rScRange.aStart.Row(),
                                       rScRange.aEnd.Col(), rScRange.aEnd.Row(),
                                       rScRange.aStart.Tab(), aPattern );
            }

            // is the below still needed?
            // For now, any sheet with validity is blocked from stream-copying.
            // Later, the validation names could be stored along with the style names.
            ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetImport().GetModel())->GetSheetSaveData();
            pSheetData->BlockSheet( GetScImport().GetTables().GetCurrentSheet() );
        }
    }
}

void ScXMLTableRowCellContext::SetContentValidation( const ScAddress& rCellPos )
{
    SetContentValidation( ScRange(rCellPos, rCellPos) );
}

void ScXMLTableRowCellContext::SetAnnotation(const ScAddress& rPos)
{
    ScDocument* pDoc = rXMLImport.GetDocument();
    if( !pDoc || !mxAnnotationData.get() )
        return;

    LockSolarMutex();

    ScPostIt* pNote = nullptr;

    uno::Reference< drawing::XShapes > xShapes = rXMLImport.GetTables().GetCurrentXShapes();
    uno::Reference< container::XIndexAccess > xShapesIA( xShapes, uno::UNO_QUERY );
    sal_Int32 nOldShapeCount = xShapesIA.is() ? xShapesIA->getCount() : 0;

    OSL_ENSURE( !mxAnnotationData->mxShape.is() || mxAnnotationData->mxShapes.is(),
        "ScXMLTableRowCellContext::SetAnnotation - shape without drawing page" );
    if( mxAnnotationData->mxShape.is() && mxAnnotationData->mxShapes.is() )
    {
        OSL_ENSURE( mxAnnotationData->mxShapes.get() == xShapes.get(), "ScXMLTableRowCellContext::SetAnnotation - different drawing pages" );
        SdrObject* pObject = ::GetSdrObjectFromXShape( mxAnnotationData->mxShape );
        OSL_ENSURE( pObject, "ScXMLTableRowCellContext::SetAnnotation - cannot get SdrObject from shape" );

        /*  Try to reuse the drawing object already created (but only if the
            note is visible, and the object is a caption object). */
        if( mxAnnotationData->mbShown && mxAnnotationData->mbUseShapePos )
        {
            if( SdrCaptionObj* pCaption = dynamic_cast< SdrCaptionObj* >( pObject ) )
            {
                OSL_ENSURE( !pCaption->GetLogicRect().IsEmpty(), "ScXMLTableRowCellContext::SetAnnotation - invalid caption rectangle" );
                // create the cell note with the caption object
                pNote = ScNoteUtil::CreateNoteFromCaption( *pDoc, rPos, *pCaption, true );
                // forget pointer to object (do not create note again below)
                pObject = nullptr;
            }
        }

        // drawing object has not been used to create a note -> use shape data
        if( pObject )
        {
            // rescue settings from drawing object before the shape is removed
            ::std::unique_ptr< SfxItemSet > xItemSet( new SfxItemSet( pObject->GetMergedItemSet() ) );
            ::std::unique_ptr< OutlinerParaObject > xOutlinerObj;
            if( OutlinerParaObject* pOutlinerObj = pObject->GetOutlinerParaObject() )
                xOutlinerObj.reset( new OutlinerParaObject( *pOutlinerObj ) );
            Rectangle aCaptionRect;
            if( mxAnnotationData->mbUseShapePos )
                aCaptionRect = pObject->GetLogicRect();
            // remove the shape from the drawing page, this invalidates pObject
            mxAnnotationData->mxShapes->remove( mxAnnotationData->mxShape );
            pObject = nullptr;
            // update current number of existing objects
            if( xShapesIA.is() )
                nOldShapeCount = xShapesIA->getCount();

            // an outliner object is required (empty note captions not allowed)
            if( xOutlinerObj.get() )
            {
                // create cell note with all data from drawing object
                pNote = ScNoteUtil::CreateNoteFromObjectData( *pDoc, rPos,
                    xItemSet.release(), xOutlinerObj.release(),
                    aCaptionRect, mxAnnotationData->mbShown, false );
            }
        }
    }
    else if( !mxAnnotationData->maSimpleText.isEmpty() )
    {
        // create note from simple text
        pNote = ScNoteUtil::CreateNoteFromString( *pDoc, rPos,
            mxAnnotationData->maSimpleText, mxAnnotationData->mbShown, false );
    }

    // set author and date
    if( pNote )
    {
        double fDate;
        if (rXMLImport.GetMM100UnitConverter().convertDateTime(fDate, mxAnnotationData->maCreateDate))
        {
            SvNumberFormatter* pNumForm = pDoc->GetFormatTable();
            sal_uInt32 nfIndex = pNumForm->GetFormatIndex( NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM );
            OUString aDate;
            Color* pColor = nullptr;
            Color** ppColor = &pColor;
            pNumForm->GetOutputString( fDate, nfIndex, aDate, ppColor );
            pNote->SetDate( aDate );
        }
        pNote->SetAuthor( mxAnnotationData->maAuthor );
    }

    // register a shape that has been newly created in the ScNoteUtil functions
    if( xShapesIA.is() && (nOldShapeCount < xShapesIA->getCount()) )
    {
        uno::Reference< drawing::XShape > xShape;
        rXMLImport.GetShapeImport()->shapeWithZIndexAdded( xShape, xShapesIA->getCount() );
    }

    // store the style names for stream copying
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rXMLImport.GetModel())->GetSheetSaveData();
    pSheetData->HandleNoteStyles( mxAnnotationData->maStyleName, mxAnnotationData->maTextStyle, rPos );

    std::vector<ScXMLAnnotationStyleEntry>::const_iterator aIter = mxAnnotationData->maContentStyles.begin();
    std::vector<ScXMLAnnotationStyleEntry>::const_iterator aEnd = mxAnnotationData->maContentStyles.end();
    while (aIter != aEnd)
    {
        pSheetData->AddNoteContentStyle( aIter->mnFamily, aIter->maName, rPos, aIter->maSelection );
        ++aIter;
    }
}

// core implementation
void ScXMLTableRowCellContext::SetDetectiveObj( const ScAddress& rPosition )
{
    if( cellExists(rPosition) && pDetectiveObjVec && pDetectiveObjVec->size() )
    {
        LockSolarMutex();
        ScDetectiveFunc aDetFunc( rXMLImport.GetDocument(), rPosition.Tab() );
        uno::Reference<container::XIndexAccess> xShapesIndex (rXMLImport.GetTables().GetCurrentXShapes(), uno::UNO_QUERY); // make draw page
        ScMyImpDetectiveObjVec::iterator aItr(pDetectiveObjVec->begin());
        ScMyImpDetectiveObjVec::iterator aEndItr(pDetectiveObjVec->end());
        while(aItr != aEndItr)
        {
            aDetFunc.InsertObject( aItr->eObjType, rPosition, aItr->aSourceRange, aItr->bHasError );
            if (xShapesIndex.is())
            {
                sal_Int32 nShapes = xShapesIndex->getCount();
                uno::Reference < drawing::XShape > xShape;
                rXMLImport.GetShapeImport()->shapeWithZIndexAdded(xShape, nShapes);
            }
            ++aItr;
        }
    }
}

// core implementation
void ScXMLTableRowCellContext::SetCellRangeSource( const ScAddress& rPosition )
{
    if( cellExists(rPosition) && pCellRangeSource  && !pCellRangeSource->sSourceStr.isEmpty() &&
        !pCellRangeSource->sFilterName.isEmpty() && !pCellRangeSource->sURL.isEmpty() )
    {
        ScDocument* pDoc = rXMLImport.GetDocument();
        if (pDoc)
        {
            LockSolarMutex();
            ScRange aDestRange( rPosition.Col(), rPosition.Row(), rPosition.Tab(),
                rPosition.Col() + static_cast<SCCOL>(pCellRangeSource->nColumns - 1),
                rPosition.Row() + static_cast<SCROW>(pCellRangeSource->nRows - 1), rPosition.Tab() );
            OUString sFilterName( pCellRangeSource->sFilterName );
            OUString sSourceStr( pCellRangeSource->sSourceStr );
            ScAreaLink* pLink = new ScAreaLink( pDoc->GetDocumentShell(), pCellRangeSource->sURL,
                sFilterName, pCellRangeSource->sFilterOptions, sSourceStr, aDestRange, pCellRangeSource->nRefresh );
            sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
            pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, pCellRangeSource->sURL, &sFilterName, &sSourceStr );
        }
    }
}

void ScXMLTableRowCellContext::SetFormulaCell(ScFormulaCell* pFCell) const
{
    if(pFCell)
    {
        if(mbErrorValue)
        {
            // don't do anything here
            // we need to recalc anyway
        }
        else if( bFormulaTextResult && maStringValue )
        {
            if( !IsPossibleErrorString() )
            {
                ScDocument* pDoc = rXMLImport.GetDocument();
                pFCell->SetHybridString(pDoc->GetSharedStringPool().intern(*maStringValue));
                pFCell->ResetDirty();
            }
        }
        else if (!rtl::math::isNan(fValue))
        {
            pFCell->SetHybridDouble(fValue);
            pFCell->ResetDirty();
        }
    }
}

void ScXMLTableRowCellContext::PutTextCell( const ScAddress& rCurrentPos,
        const SCCOL nCurrentCol, const ::boost::optional< OUString >& pOUText )
{
    bool bDoIncrement = true;
    //matrix reference cells that contain text formula results;
    //cell was already put in document, just need to set text here.
    if( rXMLImport.GetTables().IsPartOfMatrix(rCurrentPos) )
    {
        ScRefCellValue aCell(*rXMLImport.GetDocument(), rCurrentPos);
        bDoIncrement = aCell.meType == CELLTYPE_FORMULA;
        if ( bDoIncrement )
        {
            ScFormulaCell* pFCell = aCell.mpFormula;
            OUString aCellString;
            if (maStringValue)
                aCellString = *maStringValue;
            else if (mbEditEngineHasText)
                aCellString = GetFirstParagraph();
            else if ( nCurrentCol > 0 && pOUText && !pOUText->isEmpty() )
                aCellString = *pOUText;
            else
                bDoIncrement = false;

            if(mbErrorValue)
                bDoIncrement = false;

            if(!aCellString.isEmpty())
            {
                if (bDoIncrement && !IsPossibleErrorString() && pFCell)
                {
                    ScDocument* pDoc = rXMLImport.GetDocument();
                    pFCell->SetHybridString(pDoc->GetSharedStringPool().intern(aCellString));
                    pFCell->ResetDirty();
                }
                else
                {
                    ScAddress aTopLeftMatrixCell;
                    if (pFCell && pFCell->GetMatrixOrigin(aTopLeftMatrixCell))
                    {
                        ScFormulaCell* pMatrixCell = rXMLImport.GetDocument()->GetFormulaCell(aTopLeftMatrixCell);
                        if (pMatrixCell)
                            pMatrixCell->SetDirty();
                    }
                    else
                        SAL_WARN("sc", "matrix cell without matrix");
                }
            }
        }
    }
    else //regular text cells
    {
        ScDocumentImport& rDoc = rXMLImport.GetDoc();
        if (maStringValue)
        {
            rDoc.setStringCell(rCurrentPos, *maStringValue);
            bDoIncrement = true;
        }
        else if (mbEditEngineHasText)
        {
            if (maFirstParagraph)
            {
                // This is a normal text without format runs.
                rDoc.setStringCell(rCurrentPos, *maFirstParagraph);
            }
            else
            {
                // This text either has format runs, has field(s), or consists of multiple lines.
                {
                    ParaFormatsType::const_iterator it = maFormats.begin(), itEnd = maFormats.end();
                    for (; it != itEnd; ++it)
                        mpEditEngine->QuickSetAttribs((*it)->maItemSet, (*it)->maSelection);
                }

                {
                    FieldsType::const_iterator it = maFields.begin(), itEnd = maFields.end();
                    for (; it != itEnd; ++it)
                        mpEditEngine->QuickInsertField(SvxFieldItem(*(*it)->mpData, EE_FEATURE_FIELD), (*it)->maSelection);
                }

                // This edit engine uses the SfxItemPool instance returned
                // from pDoc->GetEditPool() to create the text object, which
                // is a prerequisite for using this constructor of ScEditCell.
                rDoc.setEditCell(rCurrentPos, mpEditEngine->CreateTextObject());
            }
            bDoIncrement = true;
        }
        else if ( nCurrentCol > 0 && pOUText && !pOUText->isEmpty() )
        {
            rDoc.setStringCell(rCurrentPos, *pOUText);
            bDoIncrement = true;
        }
        else
            bDoIncrement = false;
    }

    // #i56027# This is about setting simple text, not edit cells,
    // so ProgressBarIncrement must be called with bEditCell = FALSE.
    // Formatted text that is put into the cell by the child context
    // is handled in AddCellsToTable() (bIsEmpty is true then).
    if (bDoIncrement)
        rXMLImport.ProgressBarIncrement();
}

void ScXMLTableRowCellContext::PutValueCell( const ScAddress& rCurrentPos )
{
    //matrix reference cells that contain value formula results;
    //cell was already put in document, just need to set value here.
    if( rXMLImport.GetTables().IsPartOfMatrix(rCurrentPos) )
    {
        ScRefCellValue aCell(*rXMLImport.GetDocument(), rCurrentPos);
        if (aCell.meType == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFCell = aCell.mpFormula;;
            SetFormulaCell(pFCell);
            if (pFCell)
                pFCell->SetNeedNumberFormat( true );
        }
    }
    else  //regular value cell
    {
        // fdo#62250 absent values are not NaN, set to 0.0
        // PutValueCell() is called only for a known cell value type,
        // bIsEmpty==false in all these cases, no sense to check it here.
        if (::rtl::math::isNan( fValue))
            fValue = 0.0;

        // #i62435# Initialize the value cell's script type if the default
        // style's number format is latin-only. If the cell uses a different
        // format, the script type will be reset when the style is applied.

        rXMLImport.GetDoc().setNumericCell(rCurrentPos, fValue);
    }
    rXMLImport.ProgressBarIncrement();
}

namespace {

bool isEmptyOrNote( ScDocument* pDoc, const ScAddress& rCurrentPos )
{
    CellType eType = pDoc->GetCellType(rCurrentPos);
    return (eType == CELLTYPE_NONE);
}

}

void ScXMLTableRowCellContext::AddTextAndValueCell( const ScAddress& rCellPos,
        const ::boost::optional< OUString >& pOUText, ScAddress& rCurrentPos )
{
    ScMyTables& rTables = rXMLImport.GetTables();
    bool bWasEmpty = bIsEmpty;
    for (SCCOL i = 0; i < nColsRepeated; ++i)
    {
        rCurrentPos.SetCol( rCellPos.Col() + i );

        // it makes no sense to import data after the last supported column
        // fdo#58539 & gnome#627150
        if(rCurrentPos.Col() > MAXCOL)
            break;

        if (i > 0)
            rTables.AddColumn(false);
        if (!bIsEmpty)
        {
            for (SCROW j = 0; j < nRepeatedRows; ++j)
            {
                rCurrentPos.SetRow( rCellPos.Row() + j );

                // it makes no sense to import data after last supported row
                // fdo#58539 & gnome#627150
                if(rCurrentPos.Row() > MAXROW)
                    break;

                if( (rCurrentPos.Col() == 0) && (j > 0) )
                {
                    rTables.AddRow();
                    rTables.AddColumn(false);
                }
                if( cellExists(rCurrentPos) )
                {
                    if(  ( !(bIsCovered) || isEmptyOrNote(rXMLImport.GetDocument(), rCurrentPos) )  )
                    {
                        switch (nCellType)
                        {
                            case util::NumberFormat::TEXT:
                            {
                                PutTextCell( rCurrentPos, i, pOUText );
                            }
                            break;
                            case util::NumberFormat::NUMBER:
                            case util::NumberFormat::PERCENT:
                            case util::NumberFormat::CURRENCY:
                            case util::NumberFormat::TIME:
                            case util::NumberFormat::DATETIME:
                            case util::NumberFormat::LOGICAL:
                            {
                                PutValueCell( rCurrentPos );
                            }
                            break;
                            default:
                            {
                                OSL_FAIL("no cell type given");
                            }
                            break;
                        }
                    }

                    SetAnnotation( rCurrentPos );
                    SetDetectiveObj( rCurrentPos );
                    SetCellRangeSource( rCurrentPos );
                }
                else
                {
                    if (!bWasEmpty || mxAnnotationData.get())
                    {
                        if (rCurrentPos.Row() > MAXROW)
                            rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_ROW_OVERFLOW);
                        else
                            rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_COLUMN_OVERFLOW);
                    }
                }
            }
        }
        else
        {
            if ((i == 0) && (rCellPos.Col() == 0))
            {
                for (sal_Int32 j = 1; j < nRepeatedRows; ++j)
                {
                    rTables.AddRow();
                    rTables.AddColumn(false);
                }
            }
        }
    }
}

bool ScXMLTableRowCellContext::HasSpecialContent() const
{
    return (mxAnnotationData.get() || pDetectiveObjVec || pCellRangeSource);
}

bool ScXMLTableRowCellContext::CellsAreRepeated() const
{
    return ( (nColsRepeated > 1) || (nRepeatedRows > 1) );
}

namespace {

// from ScCellObj::GetOutputString_Imp().  all of it may not be necessary.
OUString getOutputString( ScDocument* pDoc, const ScAddress& aCellPos )
{
    if (!pDoc)
        return OUString();

    ScRefCellValue aCell(*pDoc, aCellPos);
    switch (aCell.meType)
    {
        case CELLTYPE_NONE:
            return OUString();
        case CELLTYPE_EDIT:
        {
            //  GetString on EditCell replaces linebreaks with spaces;
            //  however here we need line breaks
            const EditTextObject* pData = aCell.mpEditText;
            EditEngine& rEngine = pDoc->GetEditEngine();
            rEngine.SetText(*pData);
            return rEngine.GetText();
            //  also don't format EditCells per NumberFormatter
        }
        break;
        default:
        {
            //  like in GetString for document (column)
            Color* pColor;
            sal_uLong nNumFmt = pDoc->GetNumberFormat(aCellPos);
            OUString aStr;
            ScCellFormat::GetString(aCell, nNumFmt, aStr, &pColor, *pDoc->GetFormatTable(), pDoc);
            return aStr;
        }
    }
}

}

void ScXMLTableRowCellContext::AddNonFormulaCell( const ScAddress& rCellPos )
{
    ::boost::optional< OUString > pOUText;

    if( nCellType == util::NumberFormat::TEXT )
    {
        if( !bIsEmpty && !maStringValue && !mbEditEngineHasText && cellExists(rCellPos) && CellsAreRepeated() )
            pOUText.reset( getOutputString(rXMLImport.GetDocument(), rCellPos) );

        if (!mbEditEngineHasText && !pOUText && !maStringValue)
            bIsEmpty = true;
    }

    ScAddress aCurrentPos( rCellPos );
    if( HasSpecialContent() )
        bIsEmpty = false;

    AddTextAndValueCell( rCellPos, pOUText, aCurrentPos );

    if( CellsAreRepeated() )
    {
        SCCOL nStartCol( rCellPos.Col() < MAXCOL ? rCellPos.Col() : MAXCOL );
        SCROW nStartRow( rCellPos.Row() < MAXROW ? rCellPos.Row() : MAXROW );
        SCCOL nEndCol( rCellPos.Col() + nColsRepeated - 1 < MAXCOL ? rCellPos.Col() + nColsRepeated - 1 : MAXCOL );
        SCROW nEndRow( rCellPos.Row() + nRepeatedRows - 1 < MAXROW ? rCellPos.Row() + nRepeatedRows - 1 : MAXROW );
        ScRange aScRange( nStartCol, nStartRow, rCellPos.Tab(), nEndCol, nEndRow, rCellPos.Tab() );
        SetContentValidation( aScRange );
        rXMLImport.GetStylesImportHelper()->AddRange( aScRange );
    }
    else if( cellExists(rCellPos) )
    {
        rXMLImport.GetStylesImportHelper()->AddCell(rCellPos);
        SetContentValidation( rCellPos );
    }
}

void ScXMLTableRowCellContext::PutFormulaCell( const ScAddress& rCellPos )
{
    ScDocument* pDoc = rXMLImport.GetDocument();
    ScDocumentImport& rDoc = rXMLImport.GetDoc();

    OUString aText = maFormula->first;

    std::unique_ptr<ScExternalRefManager::ApiGuard> pExtRefGuard (
            new ScExternalRefManager::ApiGuard(pDoc));

    if ( !aText.isEmpty() )
    {
        if ( aText[0] == '=' && aText.getLength() > 1 )
        {
            // temporary formula string as string tokens
            ScTokenArray *pCode = new ScTokenArray();

            OUString aFormulaNmsp = maFormula->second;
            if( eGrammar != formula::FormulaGrammar::GRAM_EXTERNAL )
                aFormulaNmsp.clear();
            pCode->AssignXMLString( aText, aFormulaNmsp );

            rDoc.getDoc().IncXMLImportedFormulaCount( aText.getLength() );
            ScFormulaCell* pNewCell = new ScFormulaCell(pDoc, rCellPos, pCode, eGrammar, MM_NONE);
            SetFormulaCell(pNewCell);
            rDoc.setFormulaCell(rCellPos, pNewCell);

            // Re-calculate to get number format only when style is not set.
            pNewCell->SetNeedNumberFormat(!mbHasStyle);
        }
        else if ( aText[0] == '\'' && aText.getLength() > 1 )
        {
            //  for bEnglish, "'" at the beginning is always interpreted as text
            //  marker and stripped
            rDoc.setStringCell(rCellPos, aText.copy(1));
        }
        else
        {
            SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
            sal_uInt32 nEnglish = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
            double fVal;
            if ( pFormatter->IsNumberFormat( aText, nEnglish, fVal ) )
                rDoc.setNumericCell(rCellPos, fVal);
            //the (english) number format will not be set
            //search matching local format and apply it
            else
                rDoc.setStringCell(rCellPos, aText);
        }
    }
}

void ScXMLTableRowCellContext::AddFormulaCell( const ScAddress& rCellPos )
{
    if( cellExists(rCellPos) )
    {
        SetContentValidation( rCellPos );
        SAL_WARN_IF((nColsRepeated != 1) || (nRepeatedRows != 1), "sc", "repeated cells with formula not possible now");
        rXMLImport.GetStylesImportHelper()->AddCell(rCellPos);

        //add matrix
        if(bIsMatrix)
        {
            if (nMatrixCols > 0 && nMatrixRows > 0)
            {
                //matrix cells are put in the document, but we must set the
                //value/text of each matrix cell later
                rXMLImport.GetTables().AddMatrixRange(
                        rCellPos.Col(), rCellPos.Row(),
                        std::min<SCCOL>(rCellPos.Col() + nMatrixCols - 1, MAXCOL),
                        std::min<SCROW>(rCellPos.Row() + nMatrixRows - 1, MAXROW),
                        maFormula->first, maFormula->second, eGrammar);

                // Set the value/text of the top-left matrix position in its
                // cached result.  For import, we only need to set the correct
                // matrix geometry and the value type of the top-left element.
                ScFormulaCell* pFCell = rXMLImport.GetDocument()->GetFormulaCell(rCellPos);
                if (pFCell)
                {
                    ScMatrixRef pMat(new ScFullMatrix(nMatrixCols, nMatrixRows));
                    if (bFormulaTextResult && maStringValue)
                    {
                        if (!IsPossibleErrorString())
                        {
                            pFCell->SetResultMatrix(
                                    nMatrixCols, nMatrixRows, pMat, new formula::FormulaStringToken(
                                        rXMLImport.GetDocument()->GetSharedStringPool().intern( *maStringValue)));
                            pFCell->ResetDirty();
                        }
                    }
                    else if (!rtl::math::isNan(fValue))
                    {
                        pFCell->SetResultMatrix(
                            nMatrixCols, nMatrixRows, pMat, new formula::FormulaDoubleToken(fValue));
                        pFCell->ResetDirty();
                    }
                }
            }
        }
        else
            PutFormulaCell( rCellPos );

        SetAnnotation( rCellPos );
        SetDetectiveObj( rCellPos );
        SetCellRangeSource( rCellPos );
        rXMLImport.ProgressBarIncrement();
    }
    else
    {
        if (rCellPos.Row() > MAXROW)
            rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_ROW_OVERFLOW);
        else
            rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_COLUMN_OVERFLOW);
    }
}

//There are cases where a formula cell is exported with an office:value of 0 or
//no office:value at all, but the formula cell will have a text:p value which
//contains the intended formula result.
//These cases include when a formula result:
// - is blank
// - has a constant error value beginning with "#" (such as "#VALUE!" or "#N/A")
// - has an "Err:[###]" (where "[###]" is an error number)
// Libreoffice 4.1+ with ODF1.2 extended write however calcext:value-type="error" in that case
void ScXMLTableRowCellContext::HasSpecialCaseFormulaText()
{
    if (!mbEditEngineHasText || mbNewValueType)
        return;

    OUString aStr = GetFirstParagraph();

    if (aStr.isEmpty() || aStr.startsWith("Err:"))
        mbPossibleErrorCell = true;
    else if (aStr.startsWith("#"))
        mbCheckWithCompilerForError = true;
}

bool ScXMLTableRowCellContext::IsPossibleErrorString() const
{
    if(mbNewValueType && !mbErrorValue)
        return false;
    else if(mbNewValueType && mbErrorValue)
        return true;
    return mbPossibleErrorCell || ( mbCheckWithCompilerForError && GetScImport().IsFormulaErrorConstant(*maStringValue) );
}

void ScXMLTableRowCellContext::EndElement()
{
    HasSpecialCaseFormulaText();
    if( bFormulaTextResult && (mbPossibleErrorCell || mbCheckWithCompilerForError) )
    {
        maStringValue.reset(GetFirstParagraph());
    }

    ScAddress aCellPos = rXMLImport.GetTables().GetCurrentCellPos();
    if( aCellPos.Col() > 0 && nRepeatedRows > 1 )
        aCellPos.SetRow( aCellPos.Row() - (nRepeatedRows - 1) );
    if( bIsMerged )
        DoMerge( aCellPos, nMergedCols - 1, nMergedRows - 1 );

    if (maFormula)
        AddFormulaCell(aCellPos);
    else
        AddNonFormulaCell(aCellPos);

    UnlockSolarMutex(); //if LockSolarMutex got used, we presumably need to ensure an UnlockSolarMutex

    bIsMerged = false;
    nMergedCols = 1;
    nMergedRows = 1;
    nColsRepeated = 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
