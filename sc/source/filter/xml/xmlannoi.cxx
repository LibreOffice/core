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

#include "xmlannoi.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "XMLTableShapeImportHelper.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

#include "annotationtextparacontext.hxx"

#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include "editeng/wghtitem.hxx"
#include "editeng/colritem.hxx"
#include "editeng/fhgtitem.hxx"
#include "editeng/postitem.hxx"
#include "editeng/fontitem.hxx"
#include "editeng/udlnitem.hxx"
#include "editeng/wrlmitem.hxx"
#include "editeng/crossedoutitem.hxx"
#include "editeng/charreliefitem.hxx"
#include "editeng/charscaleitem.hxx"
#include "editeng/contouritem.hxx"
#include "editeng/shdditem.hxx"
#include "editeng/kernitem.hxx"
#include "editeng/autokernitem.hxx"
#include "editeng/escapementitem.hxx"
#include "editeng/emphasismarkitem.hxx"
#include "editeng/langitem.hxx"

#include "editattributemap.hxx"
#include "editutil.hxx"
#include "xmloff/prstylei.hxx"
#include "document.hxx"

#include "sheetdata.hxx"
#include "docuno.hxx"

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLAnnotationData::ScXMLAnnotationData() :
    mbUseShapePos( false ),
    mbShown( false )
{
}

ScXMLAnnotationData::~ScXMLAnnotationData()
{
}

//------------------------------------------------------------------

ScXMLAnnotationContext::ScXMLAnnotationContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                      ScXMLAnnotationData& rAnnotationData,
                                      ScXMLTableRowCellContext* pTempCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrAnnotationData( rAnnotationData ),
    pCellContext(pTempCellContext),
    pShapeContext(NULL),
    mpEditEngine(NULL),
    mnCurParagraph(0),
    mbHasFormatRuns(false),
    mbEditEngineHasText(false)
{

    ScDocument* pDoc = rImport.GetDocument();
    mpEditEngine = new ScEditEngineDefaulter(pDoc->GetEnginePool());
    mpEditEngine->SetEditTextObjectPool(pDoc->GetEditPool());
    mpEditEngine->SetUpdateMode(false);
    mpEditEngine->EnableUndo(false);
    mpEditEngine->Clear();

    uno::Reference<drawing::XShapes> xLocalShapes (GetScImport().GetTables().GetCurrentXShapes());
    if (xLocalShapes.is())
    {
        XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)GetScImport().GetShapeImport().get();
        pTableShapeImport->SetAnnotation(this);
        pShapeContext = GetScImport().GetShapeImport()->CreateGroupChildContext(
            GetScImport(), nPrfx, rLName, xAttrList, xLocalShapes, true);
    }

    pCellContext = pTempCellContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAnnotationAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR:
            {
                maAuthorBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE:
            {
                maCreateDateBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING:
            {
                maCreateDateStringBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY:
            {
                mrAnnotationData.mbShown = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_X:
            {
                mrAnnotationData.mbUseShapePos = true;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_Y:
            {
                mrAnnotationData.mbUseShapePos = true;
            }
            break;
        }
    }
}

ScXMLAnnotationContext::~ScXMLAnnotationContext()
{
}

ScXMLAnnotationContext::ParaFormat::ParaFormat(ScEditEngineDefaulter& rEditEngine) :
    maItemSet(rEditEngine.GetEmptyItemSet()) {}

ScXMLAnnotationContext::Field::Field(SvxFieldData* pData) : mpData(pData) {}

ScXMLAnnotationContext::Field::~Field()
{
    delete mpData;
}

void ScXMLAnnotationContext::StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList)
{
    if (pShapeContext)
        pShapeContext->StartElement(xAttrList);
}

SvXMLImportContext *ScXMLAnnotationContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLName, XML_CREATOR ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, maAuthorBuffer);
        else if( IsXMLToken( rLName, XML_DATE ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, maCreateDateBuffer);
    }
    else if( XML_NAMESPACE_META == nPrefix )
    {
        if( IsXMLToken( rLName, XML_DATE_STRING ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, maCreateDateStringBuffer);
    }
    else if( XML_NAMESPACE_TEXT == nPrefix )
    {
        if( IsXMLToken( rLName, XML_P ) )
            pContext = new ScXMLAnnotationTextParaContext(GetScImport(), nPrefix, rLName, *this);
    }

    if( !pContext && pShapeContext )
        pContext = pShapeContext->CreateChildContext(nPrefix, rLName, xAttrList);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLAnnotationContext::Characters( const OUString& rChars )
{
    maTextBuffer.append(rChars);
}

void ScXMLAnnotationContext::EndElement()
{
    if (pShapeContext)
    {
        pShapeContext->EndElement();
        delete pShapeContext;
        pShapeContext = NULL;
    }

    // Flush the cached first paragraph if not done before (single line note).
    if (maFirstParagraph)
    {
        mpEditEngine->Clear();
        mpEditEngine->SetText(*maFirstParagraph);
        maFirstParagraph.reset();
    }

    // This text either has format runs, has field(s), or consists of multiple lines.
    {
        ParaFormatsType::const_iterator it = maFormats.begin(), itEnd = maFormats.end();
        for (; it != itEnd; ++it)
        {
            mpEditEngine->QuickSetAttribs(it->maItemSet, it->maSelection);
        }
    }

    {
        FieldsType::const_iterator it = maFields.begin(), itEnd = maFields.end();
        for (; it != itEnd; ++it)
            mpEditEngine->QuickInsertField(SvxFieldItem(*it->mpData, EE_FEATURE_FIELD), it->maSelection);
    }

    mrAnnotationData.maAuthor = maAuthorBuffer.makeStringAndClear();
    mrAnnotationData.maCreateDate = maCreateDateBuffer.makeStringAndClear();
    if (mrAnnotationData.maCreateDate.isEmpty())
        mrAnnotationData.maCreateDate = maCreateDateStringBuffer.makeStringAndClear();
    mrAnnotationData.maSimpleText = maTextBuffer.makeStringAndClear();

    mrAnnotationData.maEditEngine = mpEditEngine;

    XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)GetScImport().GetShapeImport().get();
    pTableShapeImport->SetAnnotation(NULL);

}

void ScXMLAnnotationContext::SetShape( const uno::Reference< drawing::XShape >& rxShape, const uno::Reference< drawing::XShapes >& rxShapes,
                                       const OUString& rStyleName, const OUString& rTextStyle )
{
    mrAnnotationData.mxShape = rxShape;
    mrAnnotationData.mxShapes = rxShapes;
    mrAnnotationData.maStyleName = rStyleName;
    mrAnnotationData.maTextStyle = rTextStyle;
}

void ScXMLAnnotationContext::AddContentStyle( sal_uInt16 nFamily, const OUString& rName, const ESelection& rSelection )
{
    mrAnnotationData.maContentStyles.push_back( ScXMLAnnotationStyleEntry( nFamily, rName, rSelection ) );
}

void ScXMLAnnotationContext::PushParagraphSpan(const OUString& rSpan, const OUString& rStyleName)
{
    sal_Int32 nBegin = maParagraph.getLength();
    sal_Int32 nEnd = nBegin + rSpan.getLength();
    maParagraph.append(rSpan);

    PushFormat(nBegin, nEnd, rStyleName);
}

void ScXMLAnnotationContext::PushParagraphField(SvxFieldData* pData, const OUString& rStyleName)
{
    mbHasFormatRuns = true;
    maFields.push_back(new Field(pData));
    Field& rField = maFields.back();

    sal_Int32 nPos = maParagraph.getLength();
    maParagraph.append(sal_Unicode('\1')); // Placeholder text for inserted field item.
    rField.maSelection.nStartPara = mnCurParagraph;
    rField.maSelection.nEndPara = mnCurParagraph;
    rField.maSelection.nStartPos = nPos;
    rField.maSelection.nEndPos = nPos+1;

    PushFormat(nPos, nPos+1, rStyleName);
}

void ScXMLAnnotationContext::PushParagraphFieldDate(const OUString& rStyleName)
{
    PushParagraphField(new SvxDateField, rStyleName);
}

void ScXMLAnnotationContext::PushParagraphFieldSheetName(const OUString& rStyleName)
{
    SCTAB nTab = GetScImport().GetTables().GetCurrentCellPos().Tab();
    PushParagraphField(new SvxTableField(nTab), rStyleName);
}

void ScXMLAnnotationContext::PushParagraphFieldDocTitle(const OUString& rStyleName)
{
    PushParagraphField(new SvxFileField, rStyleName);
}

void ScXMLAnnotationContext::PushParagraphFieldURL(
    const OUString& rURL, const OUString& rRep, const OUString& rStyleName)
{
    OUString aAbsURL = GetScImport().GetAbsoluteReference(rURL);
    PushParagraphField(new SvxURLField(aAbsURL, rRep, SVXURLFORMAT_REPR), rStyleName);
}

void ScXMLAnnotationContext::PushParagraphEnd()
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

void ScXMLAnnotationContext::PushFormat(sal_Int32 nBegin, sal_Int32 nEnd, const OUString& rStyleName)
{
    if (rStyleName.isEmpty())
        return;

    // Get the style information from xmloff.
    UniReference<XMLPropertySetMapper> xMapper = GetImport().GetTextImport()->GetTextImportPropertySetMapper()->getPropertySetMapper();
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
    maFormats.push_back(new ParaFormat(*mpEditEngine));
    ParaFormat& rFmt = maFormats.back();
    rFmt.maSelection.nStartPara = rFmt.maSelection.nEndPara = mnCurParagraph;
    rFmt.maSelection.nStartPos = nBegin;
    rFmt.maSelection.nEndPos = nEnd;

    // Store the used text styles for export.
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetScImport().GetModel())->GetSheetSaveData();
    ScAddress aCellPos = GetScImport().GetTables().GetCurrentCellPos();
    pSheetData->AddTextStyle(rStyleName, aCellPos, rFmt.maSelection);

    boost::scoped_ptr<SfxPoolItem> pPoolItem;
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
                    pPoolItem.reset(new SvxUnderlineItem(UNDERLINE_NONE, pEntry->mnItemID));

                pPoolItem->PutValue(it->maValue, pEntry->mnFlag);
            }
            break;
            case EE_CHAR_OVERLINE:
            {
                if (!pPoolItem)
                    pPoolItem.reset(new SvxOverlineItem(UNDERLINE_NONE, pEntry->mnItemID));

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
                    pPoolItem.reset(new SvxCharReliefItem(RELIEF_NONE, pEntry->mnItemID));

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
                    pPoolItem.reset(new SvxEmphasisMarkItem(EMPHASISMARK_NONE, pEntry->mnItemID));

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
    {
        rFmt.maItemSet.Put(*pPoolItem);
    }

    AddContentStyle( XML_STYLE_FAMILY_TEXT_TEXT, rStyleName, rFmt.maSelection );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
