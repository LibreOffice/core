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

/*
 This is part of GSoC 2025 "New Dialog to Edit Table Styles" project.
 Previously the autoformats loaded from 'autotbl.fmt'. Now we load them from
 'tablestyles.xml'
 */

#include <editeng/adjustitem.hxx>
#include <sax/tools/converter.hxx>
#include <svx/svxtableitems.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlprmap.hxx>
#include <svx/TableAutoFmt.hxx>
#include <svx/TableStylesParser.hxx>

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

const std::map<OUString, sal_Int16> CellFieldToIndex
    = { { u"first-row"_ustr, FIRST_ROW },
        { u"last-row"_ustr, LAST_ROW },
        { u"first-column"_ustr, FIRST_COL },
        { u"last-column"_ustr, LAST_COL },
        { u"body"_ustr, BODY },
        { u"even-rows"_ustr, EVEN_ROW },
        { u"odd-rows"_ustr, ODD_ROW },
        { u"even-columns"_ustr, EVEN_COL },
        { u"odd-columns"_ustr, ODD_COL },
        { u"background"_ustr, BACKGROUND },
        { u"first-row-even-column"_ustr, FIRST_ROW_EVEN_COL },
        { u"last-row-even-column"_ustr, LAST_ROW_EVEN_COL },
        { u"first-row-end-column"_ustr, FIRST_ROW_END_COL },
        { u"first-row-start-column"_ustr, FIRST_ROW_START_COL },
        { u"last-row-end-column"_ustr, LAST_ROW_END_COL },
        { u"last-row-start-column"_ustr, LAST_ROW_START_COL } };

const std::map<sal_Int16, OUString> IndexToCellField
    = { { FIRST_ROW, u"first-row"_ustr },
        { LAST_ROW, u"last-row"_ustr },
        { FIRST_COL, u"first-column"_ustr },
        { LAST_COL, u"last-column"_ustr },
        { BODY, u"body"_ustr },
        { EVEN_ROW, u"even-rows"_ustr },
        { ODD_ROW, u"odd-rows"_ustr },
        { EVEN_COL, u"even-columns"_ustr },
        { ODD_COL, u"odd-columns"_ustr },
        { BACKGROUND, u"background"_ustr },
        { FIRST_ROW_EVEN_COL, u"first-row-even-column"_ustr },
        { LAST_ROW_EVEN_COL, u"last-row-even-column"_ustr },
        { FIRST_ROW_END_COL, u"first-row-end-column"_ustr },
        { FIRST_ROW_START_COL, u"first-row-start-column"_ustr },
        { LAST_ROW_END_COL, u"last-row-end-column"_ustr },
        { LAST_ROW_START_COL, u"last-row-start-column"_ustr } };

const std::map<OUString, ::xmloff::token::XMLTokenEnum> CellStyleNameMap
    = { { u"first-row"_ustr, XML_FIRST_ROW },
        { u"last-row"_ustr, XML_LAST_ROW },
        { u"first-column"_ustr, XML_FIRST_COLUMN },
        { u"last-column"_ustr, XML_LAST_COLUMN },
        { u"body"_ustr, XML_BODY },
        { u"even-rows"_ustr, XML_EVEN_ROWS },
        { u"odd-rows"_ustr, XML_ODD_ROWS },
        { u"even-columns"_ustr, XML_EVEN_COLUMNS },
        { u"odd-columns"_ustr, XML_ODD_COLUMNS },
        { u"background"_ustr, XML_BACKGROUND },
        { u"first-row-even-column"_ustr, XML_FIRST_ROW_EVEN_COLUMN },
        { u"last-row-even-column"_ustr, XML_LAST_ROW_EVEN_COLUMN },
        { u"first-row-end-column"_ustr, XML_FIRST_ROW_END_COLUMN },
        { u"first-row-start-column"_ustr, XML_FIRST_ROW_START_COLUMN },
        { u"last-row-end-column"_ustr, XML_LAST_ROW_END_COLUMN },
        { u"last-row-start-column"_ustr, XML_LAST_ROW_START_COLUMN },
        { OUString(), XML_TOKEN_END } };

SvxTableStylesImport::SvxTableStylesImport(const Reference<XComponentContext>& rContext,
                                           SvxAutoFormat& rAutoFormat)
    : SvXMLImport(rContext, "SvxTableStylesImport")
    , mpAutoFormat(rAutoFormat)
{
}

SvxTableStylesImport::~SvxTableStylesImport() = default;

SvXMLImportContext*
SvxTableStylesImport::CreateFastContext(sal_Int32 nElement,
                                        const Reference<XFastAttributeList>& xAttrList)
{
    switch (nElement)
    {
        case XML_ELEMENT(OFFICE, XML_STYLES):
            return new SvxTableStylesContext(*this);
        default:
            return SvXMLImport::CreateFastContext(nElement, xAttrList);
    }
}

void SvxTableStylesImport::addCellStyle(const OUString& rName, const OUString& rParentName,
                                        std::unique_ptr<SvxAutoFormatDataField> pField)
{
    if (pField)
        maCellStyles[rName] = { rParentName, std::move(pField) };
}

const TableStyle* SvxTableStylesImport::getCellStyle(const OUString& rName)
{
    auto it = maCellStyles.find(rName);
    return (it != maCellStyles.end()) ? &it->second : nullptr;
}

void SvxTableStylesImport::addTableTemplate(const OUString& rsStyleName,
                                            const XMLTableTemplate& xTableTemplate,
                                            std::bitset<6>& rUseSet)
{
    auto xPtr = std::make_shared<XMLTableTemplate>(xTableTemplate);
    maTableTemplates.emplace_back(rsStyleName, xPtr);
    maTableStyles[rsStyleName] = rUseSet;
}

SvxTableStylesContext::SvxTableStylesContext(SvxTableStylesImport& rImport)
    : SvXMLStylesContext(rImport)
    , mpImport(&rImport)
{
}

SvXMLStyleContext* SvxTableStylesContext::CreateStyleChildContext(
    sal_Int32 nElement, const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList)
{
    if (nElement == XML_ELEMENT(TABLE, XML_TABLE_TEMPLATE))
        return new SvxTableTemplateContext(GetImport(), *mpImport);

    return SvXMLStylesContext::CreateStyleChildContext(nElement, xAttrList);
}

SvXMLStyleContext*
SvxTableStylesContext::CreateStyleStyleChildContext(XmlStyleFamily nFamily, sal_Int32 nElement,
                                                    const Reference<XFastAttributeList>& xAttrList)
{
    if (nFamily == XmlStyleFamily::TABLE_CELL)
    {
        auto* pContext = new SvxCellStyleContext(GetImport(), *this, *mpImport);
        return pContext;
    }

    return SvXMLStylesContext::CreateStyleStyleChildContext(nFamily, nElement, xAttrList);
}

SvXMLImportPropertyMapper*
SvxTableStylesContext::GetImportPropertyMapper(XmlStyleFamily nFamily) const
{
    if (nFamily == XmlStyleFamily::TABLE_CELL)
    {
        if (!mxTableCellPropMapper)
        {
            mxTableCellPropMapper = XMLTextImportHelper::CreateTableCellExtPropMapper(
                const_cast<SvxTableStylesContext*>(this)->GetImport());
        }
        return mxTableCellPropMapper.get();
    }

    return SvXMLStylesContext::GetImportPropertyMapper(nFamily);
}

void SAL_CALL SvxTableStylesContext::endFastElement(sal_Int32)
{
    FinishStyles(true);
    mpImport->finishStyles();
}

SvxTableTemplateContext::SvxTableTemplateContext(SvXMLImport& rImport,
                                                 SvxTableStylesImport& rSvxImport)
    : SvXMLStyleContext(rImport, XmlStyleFamily::TABLE_TEMPLATE_ID)
    , mpImport(&rSvxImport)
{
}

void SvxTableTemplateContext::SetAttribute(sal_Int32 nElement, const OUString& rValue)
{
    if (nElement == XML_ELEMENT(TABLE, XML_NAME))
        msTemplateName = rValue;

    switch (nElement)
    {
        case XML_ELEMENT(TABLE, XML_USE_FIRST_ROW_STYLES):
            maUseSet[0] = true;
            break;
        case XML_ELEMENT(TABLE, XML_USE_LAST_ROW_STYLES):
            maUseSet[1] = true;
            break;
        case XML_ELEMENT(TABLE, XML_USE_FIRST_COLUMN_STYLES):
            maUseSet[2] = true;
            break;
        case XML_ELEMENT(TABLE, XML_USE_LAST_COLUMN_STYLES):
            maUseSet[3] = true;
            break;
        case XML_ELEMENT(TABLE, XML_USE_BANDING_ROWS_STYLES):
            maUseSet[4] = true;
            break;
        case XML_ELEMENT(TABLE, XML_USE_BANDING_COLUMNS_STYLES):
            maUseSet[5] = true;
            break;
        default:
            break;
    }
}

css::uno::Reference<css::xml::sax::XFastContextHandler>
SvxTableTemplateContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList)
{
    sal_Int32 nLocalName = nElement & TOKEN_MASK;

    auto it = std::find_if(CellStyleNameMap.begin(), CellStyleNameMap.end(),
                           [nLocalName](const auto& pair) { return pair.second == nLocalName; });

    if (it != CellStyleNameMap.end() && it->second != XML_TOKEN_END)
    {
        for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
        {
            if (aIter.getToken() == XML_ELEMENT(TABLE, XML_STYLE_NAME))
            {
                maTableTemplate[it->first] = aIter.toString();
                break;
            }
        }
    }
    return nullptr;
}

void SvxTableTemplateContext::endFastElement(sal_Int32)
{
    mpImport->addTableTemplate(msTemplateName, maTableTemplate, maUseSet);
}

SvxCellStyleContext::SvxCellStyleContext(SvXMLImport& rImport, SvxTableStylesContext& rStyles,
                                         SvxTableStylesImport& rSvxImport)
    : XMLPropStyleContext(rImport, rStyles, XmlStyleFamily::TABLE_CELL)
    , mpField(rSvxImport.GetFormat().GetDefaultField())
    , mpImport(&rSvxImport)
{
}

void SvxCellStyleContext::Finish(bool)
{
    SvXMLImportPropertyMapper* pMapper
        = GetStyles()->GetImportPropertyMapper(XmlStyleFamily::TABLE_CELL);

    ::std::vector<XMLPropertyState> props = GetProperties();
    for (auto& prop : props)
    {
        OUString propName = pMapper->getPropertySetMapper()->GetEntryAPIName(prop.mnIndex);
        setPropertyValue(propName, prop.maValue);
    }
    mpImport->addCellStyle(GetName(), GetParentName(), std::move(mpField));
}

void SvxCellStyleContext::setPropertyValue(OUString& rPropName, const css::uno::Any& aValue)
{
    try
    {
        const SfxItemPropertyMapEntry* pEntry
            = SvxAutoFormat::GetTablePropertySet().getPropertyMap().getByName(rPropName);

        if (!pEntry)
        {
            SAL_WARN("svx", "Unknown property: " << rPropName);
            return;
        }

        switch (pEntry->nWID)
        {
            case SVX_TABLE_BACKGROUND:
            {
                SvxBrushItem rBrush = mpField->GetBackground();
                if (rBrush.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetBackground(rBrush);
                    mpField->SetPropertyFlag(PROP_BACKGROUND);
                }
                return;
            }
            case SVX_TABLE_BOX:
            {
                SvxBoxItem rBox = mpField->GetBox();
                if (rBox.PutValue(aValue, pEntry->nMemberId))
                {
                    if (rPropName == "Border")
                    {
                        rBox.SetLine(rBox.GetLeft(), SvxBoxItemLine::RIGHT);
                        rBox.SetLine(rBox.GetLeft(), SvxBoxItemLine::TOP);
                        rBox.SetLine(rBox.GetLeft(), SvxBoxItemLine::BOTTOM);
                    }

                    if (rPropName == "BorderDistance")
                        mpField->SetPropertyFlag(PROP_PADDING);
                    else
                        mpField->SetPropertyFlag(PROP_BOX);

                    mpField->SetBox(rBox);
                }
                return;
            }
            case SVX_TABLE_VER_JUSTIFY:
            {
                SvxVerJustifyItem rVertOrient = mpField->GetVerJustify();
                if (rVertOrient.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetVerJustify(rVertOrient);
                    mpField->SetPropertyFlag(PROP_VER_JUSTIFY);
                }
                return;
            }
            case SVX_TABLE_PARA_ADJUST:
            {
                SvxAdjustItem rAdjustItem = mpField->GetAdjust();
                if (rAdjustItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetAdjust(rAdjustItem);
                    mpField->SetPropertyFlag(PROP_HOR_JUSTIFY);

                    SvxHorJustifyItem rHoriItem(SvxCellHorJustify::Left, SVX_TABLE_HOR_JUSTIFY);
                    if (rAdjustItem.GetAdjust() == SvxAdjust::Right)
                    {
                        rHoriItem.SetValue(SvxCellHorJustify::Right);
                    }
                    else if (rAdjustItem.GetAdjust() == SvxAdjust::Center)
                    {
                        rHoriItem.SetValue(SvxCellHorJustify::Center);
                    }
                    mpField->SetHorJustify(rHoriItem);
                }
                return;
            }
            case SVX_TABLE_FONT_COLOR:
            {
                SvxColorItem rColorItem = mpField->GetColor();
                if (rColorItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetColor(rColorItem);
                    mpField->SetPropertyFlag(PROP_COLOR);
                }
                return;
            }
            case SVX_TABLE_FONT_UNDERLINE:
            {
                SvxUnderlineItem rUnderlineItem = mpField->GetUnderline();
                if (rUnderlineItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetUnderline(rUnderlineItem);
                    mpField->SetPropertyFlag(PROP_UNDERLINE);
                }
                return;
            }
            case SVX_TABLE_FONT_HEIGHT:
            {
                SvxFontHeightItem rFontHeightItem = mpField->GetHeight();
                if (rFontHeightItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetHeight(rFontHeightItem);
                    mpField->SetPropertyFlag(PROP_HEIGHT);
                }
                return;
            }
            case SVX_TABLE_FONT_WEIGHT:
            {
                SvxWeightItem rWeightItem = mpField->GetWeight();
                if (rWeightItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetWeight(rWeightItem);
                    mpField->SetPropertyFlag(PROP_WEIGHT);
                }
                return;
            }
            case SVX_TABLE_FONT_POSTURE:
            {
                SvxPostureItem rPostureItem = mpField->GetPosture();
                if (rPostureItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetPosture(rPostureItem);
                    mpField->SetPropertyFlag(PROP_POSTURE);
                }
                return;
            }
            case SVX_TABLE_FONT:
            {
                SvxFontItem rFontItem = mpField->GetFont();
                if (rFontItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetFont(rFontItem);
                    mpField->SetPropertyFlag(PROP_FONT);
                }
                return;
            }
            case SVX_TABLE_CJK_FONT_HEIGHT:
            {
                SvxFontHeightItem rFontHeightItem = mpField->GetCJKHeight();
                if (rFontHeightItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetCJKHeight(rFontHeightItem);
                    mpField->SetPropertyFlag(PROP_CJK_HEIGHT);
                }
                return;
            }
            case SVX_TABLE_CJK_FONT_WEIGHT:
            {
                SvxWeightItem rWeightItem = mpField->GetCJKWeight();
                if (rWeightItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetCJKWeight(rWeightItem);
                    mpField->SetPropertyFlag(PROP_CJK_WEIGHT);
                }
                return;
            }
            case SVX_TABLE_CJK_FONT_POSTURE:
            {
                SvxPostureItem rPostureItem = mpField->GetCJKPosture();
                if (rPostureItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetCJKPosture(rPostureItem);
                    mpField->SetPropertyFlag(PROP_CJK_POSTURE);
                }
                return;
            }
            case SVX_TABLE_CJK_FONT:
            {
                SvxFontItem rFontItem = mpField->GetCJKFont();
                if (rFontItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetCJKFont(rFontItem);
                    mpField->SetPropertyFlag(PROP_CJK_FONT);
                }
                return;
            }
            case SVX_TABLE_CTL_FONT_HEIGHT:
            {
                SvxFontHeightItem rFontHeightItem = mpField->GetCTLHeight();
                if (rFontHeightItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetCTLHeight(rFontHeightItem);
                    mpField->SetPropertyFlag(PROP_CTL_HEIGHT);
                }
                return;
            }
            case SVX_TABLE_CTL_FONT_WEIGHT:
            {
                SvxWeightItem rWeightItem = mpField->GetCTLWeight();
                if (rWeightItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetCTLWeight(rWeightItem);
                    mpField->SetPropertyFlag(PROP_CTL_WEIGHT);
                }
                return;
            }
            case SVX_TABLE_CTL_FONT_POSTURE:
            {
                SvxPostureItem rPostureItem = mpField->GetCTLPosture();
                if (rPostureItem.PutValue(aValue, pEntry->nMemberId))
                {
                    mpField->SetCTLPosture(rPostureItem);
                    mpField->SetPropertyFlag(PROP_CTL_POSTURE);
                }
                return;
            }
            case SVX_TABLE_CTL_FONT:
            {
                SvxFontItem rFontItem = mpField->GetCTLFont();
                if (rFontItem.PutValue(aValue, pEntry->nMemberId))

                {
                    mpField->SetCTLFont(rFontItem);
                    mpField->SetPropertyFlag(PROP_CTL_FONT);
                }
                return;
            }
            default:
                return;
        }
    }
    catch (const css::uno::Exception& e)
    {
        SAL_WARN("svx", "Exception in setPropertyValue for " << rPropName << ": " << e.Message);
    }
}

void SvxTableStylesImport::finishStyles()
{
    if (maTableTemplates.empty())
        return;

    for (const auto& rTemplate : maTableTemplates)
    {
        const OUString sTemplateName(rTemplate.first);
        std::shared_ptr<XMLTableTemplate> xTemplate(rTemplate.second);
        auto pData = mpAutoFormat.GetDefaultData();

        const TableStyle* aStyle = nullptr;
        for (const auto& rStyle : *xTemplate)
        {
            const OUString sFieldName(rStyle.first);
            const OUString sStyleName(rStyle.second);

            aStyle = getCellStyle(sStyleName);
            if (aStyle)
            {
                auto fieldIt = CellFieldToIndex.find(sFieldName);
                if (fieldIt != CellFieldToIndex.end())
                    pData->SetField(fieldIt->second, *aStyle->pDataField);
            }
            else
            {
                SAL_WARN("svx", "Style not found: " << sStyleName);
            }
        }

        if (aStyle && aStyle->sParentName.getLength())
        {
            OUString sParent = aStyle->sParentName;
            int idx = sParent.lastIndexOf(".");
            if (idx != -1)
            {
                sParent = sParent.copy(0, idx).replaceAll("-", " ");
                pData->SetParent(sParent);
            }
        }

        pData->SetName(sTemplateName);
        auto it = maTableStyles.find(sTemplateName);
        if (it != maTableStyles.end())
        {
            const auto& UseSet = it->second;
            pData->SetUseFirstRowStyles(UseSet[0]);
            pData->SetUseLastRowStyles(UseSet[1]);
            pData->SetUseFirstColStyles(UseSet[2]);
            pData->SetUseLastColStyles(UseSet[3]);
            pData->SetUseBandedRowStyles(UseSet[4]);
            pData->SetUseBandedColStyles(UseSet[5]);
        }
        mpAutoFormat.InsertAutoFormat(pData);
    }
}

SvxTableStylesExport::SvxTableStylesExport(
    const css::uno::Reference<css::uno::XComponentContext>& xContext, OUString const& rFileName,
    const css::uno::Reference<com::sun::star::xml::sax::XDocumentHandler>& xHandler,
    SvxAutoFormat& rAutoFormat)
    : SvXMLExport(xContext, "SvxTableStylesExport", rFileName,
                  com::sun::star::util::MeasureUnit::INCH, xHandler)
    , mpAutoFormat(rAutoFormat)
{
    GetNamespaceMap_().Add(GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE),
                           XML_NAMESPACE_OFFICE);
    GetNamespaceMap_().Add(GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE),
                           XML_NAMESPACE_STYLE);
    GetNamespaceMap_().Add(GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO), XML_NAMESPACE_FO);
    GetNamespaceMap_().Add(GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE),
                           XML_NAMESPACE_TABLE);
    GetNamespaceMap_().Add(GetXMLToken(XML_NP_LO_EXT), GetXMLToken(XML_N_LO_EXT),
                           XML_NAMESPACE_LO_EXT);
}

void SvxTableStylesExport::ExportStyles()
{
    GetDocHandler()->startDocument();

    {
        // Add namespace attributes to <office:styles>
        AddAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
        AddAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
        AddAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
        AddAttribute("xmlns:table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0");
        AddAttribute("xmlns:loext",
                     "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0");

        // Start office:styles element
        SvXMLElementExport styles(*this, XML_NAMESPACE_OFFICE, XML_STYLES, true, true);

        // styles elements
        for (size_t i = 0; i < mpAutoFormat.size(); i++)
        {
            const SvxAutoFormatData* pData = mpAutoFormat.GetData(i);
            const OUString& sTemplateName = pData->GetName();

            auto it = CellFieldToIndex.begin();
            while (it != CellFieldToIndex.end())
            {
                OUString sStyleName = sTemplateName.replaceAll(" ", "-") + "." + it->first;
                OUString sParentName;
                if (pData->GetParent().getLength())
                    sParentName = pData->GetParent().replaceAll(" ", "-") + "." + it->first;

                exportCellStyle(*pData->GetField(it->second), sStyleName, sParentName);
                it++;
            }
        }

        // table-templates
        for (size_t i = 0; i < mpAutoFormat.size(); i++)
        {
            exportTableTemplate(*mpAutoFormat.GetData(i));
        }
    }

    GetDocHandler()->endDocument();
}

void SvxTableStylesExport::exportTableTemplate(const SvxAutoFormatData& rData)
{
    AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, rData.GetName());
    if (rData.UseFirstRowStyles())
        AddAttribute(XML_NAMESPACE_TABLE, XML_USE_FIRST_ROW_STYLES, "true");
    if (rData.UseLastRowStyles())
        AddAttribute(XML_NAMESPACE_TABLE, XML_USE_LAST_ROW_STYLES, "true");
    if (rData.UseFirstColStyles())
        AddAttribute(XML_NAMESPACE_TABLE, XML_USE_FIRST_COLUMN_STYLES, "true");
    if (rData.UseLastColStyles())
        AddAttribute(XML_NAMESPACE_TABLE, XML_USE_LAST_COLUMN_STYLES, "true");
    if (rData.UseBandedRowStyles())
        AddAttribute(XML_NAMESPACE_TABLE, XML_USE_BANDING_ROWS_STYLES, "true");
    if (rData.UseBandedColStyles())
        AddAttribute(XML_NAMESPACE_TABLE, XML_USE_BANDING_COLUMNS_STYLES, "true");

    SvXMLElementExport aTemplate(*this, XML_NAMESPACE_TABLE, XML_TABLE_TEMPLATE, true, true);

    for (sal_uInt16 i = 0; i < 16; i++)
    {
        OUString sCellStyleName
            = rData.GetName().replaceAll(" ", "-") + "." + IndexToCellField.at(i);
        AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, sCellStyleName);

        if (i < 10)
        {
            SvXMLElementExport aElement(*this, XML_NAMESPACE_TABLE,
                                        CellStyleNameMap.at(IndexToCellField.at(i)), true, false);
        }
        else
        {
            SvXMLElementExport aElement(*this, XML_NAMESPACE_LO_EXT,
                                        CellStyleNameMap.at(IndexToCellField.at(i)), true, false);
        }
    }
}

void SvxTableStylesExport::exportCellStyle(const SvxAutoFormatDataField& rField,
                                           OUString& rStyleName, OUString& rParentName)
{
    AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, rStyleName);
    AddAttribute(XML_NAMESPACE_STYLE, XML_FAMILY, "table-cell");
    if (rParentName.getLength())
        AddAttribute(XML_NAMESPACE_STYLE, XML_PARENT_STYLE_NAME, rParentName);

    SvXMLElementExport style(*this, XML_NAMESPACE_STYLE, XML_STYLE, true, true);

    auto pParentField = mpAutoFormat.GetDefaultField();

    if (rParentName.getLength())
    {
        int idx = rParentName.lastIndexOf(".");
        if (idx != -1)
        {
            OUString sParentName = rParentName.copy(0, idx).replaceAll("-", " "),
                     sFieldName = rParentName.copy(idx + 1);
            if (SvxAutoFormatData* pParent = mpAutoFormat.FindAutoFormat(sParentName))
            {
                pParentField = pParent->GetField(CellFieldToIndex.at(sFieldName));
            }
        }
    }

    exportCellProperties(rField, *pParentField);
    exportParaProperties(rField, *pParentField);
    exportTextProperties(rField, *pParentField);
}

static OUString lcl_getBorderLineStyle(SvxBorderLineStyle rLineStyle)
{
    switch (rLineStyle)
    {
        case SvxBorderLineStyle::SOLID:
            return "solid";
        case SvxBorderLineStyle::DOTTED:
            return "dotted";
        case SvxBorderLineStyle::DASHED:
            return "dashed";
        case SvxBorderLineStyle::FINE_DASHED:
            return "fine-dashed";
        case SvxBorderLineStyle::DASH_DOT:
            return "dash-dot";
        case SvxBorderLineStyle::DASH_DOT_DOT:
            return "dash-dot-dot";
        case SvxBorderLineStyle::DOUBLE_THIN:
            return "double-thin";
        case SvxBorderLineStyle::EMBOSSED:
            return "ridge";
        case SvxBorderLineStyle::ENGRAVED:
            return "groove";
        case SvxBorderLineStyle::INSET:
            return "inset";
        case SvxBorderLineStyle::OUTSET:
            return "outset";
        default:
            return "double";
    }
}

static OUString lcl_getBorderStyle(editeng::SvxBorderLine& rBorder)
{
    OUStringBuffer strBuffer;
    sax::Converter::convertMeasure(strBuffer, rBorder.GetWidth(),
                                   com::sun::star::util::MeasureUnit::TWIP,
                                   com::sun::star::util::MeasureUnit::POINT);
    strBuffer.append(" " + lcl_getBorderLineStyle(rBorder.GetBorderLineStyle()) + " #"
                     + rBorder.GetColor().AsRGBHexString());

    return strBuffer.makeStringAndClear();
}

void SvxTableStylesExport::exportCellProperties(const SvxAutoFormatDataField& rField,
                                                const SvxAutoFormatDataField& rParent)
{
    // exporting <style:table-cell-properties>
    if (rField.IsPropertySet(PROP_BOX) && rField.GetBox() != rParent.GetBox())
    {
        SvxBoxItem aBox = rField.GetBox();

        // <fo:border>
        editeng::SvxBorderLine* aRight = aBox.GetRight();
        editeng::SvxBorderLine* aLeft = aBox.GetLeft();
        editeng::SvxBorderLine* aTop = aBox.GetTop();
        editeng::SvxBorderLine* aBottom = aBox.GetBottom();

        if (aRight && aLeft && aTop && aBottom && *aRight == *aTop && *aRight == *aBottom
            && *aRight == *aLeft)
        {
            AddAttribute(XML_NAMESPACE_FO, XML_BORDER, lcl_getBorderStyle(*aRight));
        }
        else if (!aRight && !aLeft && !aTop && !aBottom)
        {
            AddAttribute(XML_NAMESPACE_FO, XML_BORDER, "0pt solid #000000");
        }
        else
        {
            if (aRight)
                AddAttribute(XML_NAMESPACE_FO, XML_BORDER_RIGHT, lcl_getBorderStyle(*aRight));
            if (aLeft)
                AddAttribute(XML_NAMESPACE_FO, XML_BORDER_LEFT, lcl_getBorderStyle(*aLeft));
            if (aTop)
                AddAttribute(XML_NAMESPACE_FO, XML_BORDER_TOP, lcl_getBorderStyle(*aTop));
            if (aBottom)
                AddAttribute(XML_NAMESPACE_FO, XML_BORDER_BOTTOM, lcl_getBorderStyle(*aBottom));
        }

        // <fo:padding>
        if (rField.IsPropertySet(PROP_PADDING)
            && rField.GetBox().GetDistance(SvxBoxItemLine::RIGHT)
                   != rParent.GetBox().GetDistance(SvxBoxItemLine::RIGHT))
        {
            auto borderPadding = aBox.GetDistance(SvxBoxItemLine::RIGHT);
            OUStringBuffer strBuffer;
            sax::Converter::convertMeasure(strBuffer, borderPadding,
                                           com::sun::star::util::MeasureUnit::TWIP,
                                           com::sun::star::util::MeasureUnit::INCH);
            AddAttribute(XML_NAMESPACE_FO, XML_PADDING, strBuffer.makeStringAndClear());
        }
    }

    // <fo:background-color>
    if (rField.IsPropertySet(PROP_BACKGROUND) && rField.GetBackground() != rParent.GetBackground())
        AddAttribute(XML_NAMESPACE_FO, XML_BACKGROUND_COLOR,
                     "#" + rField.GetBackground().GetColor().AsRGBHexString());

    // <style:vertical-align>
    if (rField.IsPropertySet(PROP_VER_JUSTIFY) && rField.GetVerJustify() != rParent.GetVerJustify())
    {
        SvxCellVerJustify aVertAlign(rField.GetVerJustify().GetValue());

        OUString sVertAlign;
        if (aVertAlign == SvxCellVerJustify::Top)
            sVertAlign = "top";
        else if (aVertAlign == SvxCellVerJustify::Bottom)
            sVertAlign = "bottom";
        else
            sVertAlign = "middle";

        AddAttribute(XML_NAMESPACE_STYLE, XML_VERTICAL_ALIGN, sVertAlign);
    }

    SvXMLElementExport aCellProps(*this, XML_NAMESPACE_STYLE, XML_TABLE_CELL_PROPERTIES, true,
                                  true);
}

void SvxTableStylesExport::exportParaProperties(const SvxAutoFormatDataField& rField,
                                                const SvxAutoFormatDataField& rParent)
{
    // exporting <style:paragraph-properties>
    // <fo:text-align>
    if (rField.IsPropertySet(PROP_HOR_JUSTIFY) && rField.GetHorJustify() != rParent.GetHorJustify())
    {
        SvxCellHorJustify aItem = rField.GetHorJustify().GetValue();

        OUString sTextAlign;
        if (aItem == SvxCellHorJustify::Right)
            sTextAlign = "right";
        else if (aItem == SvxCellHorJustify::Center)
            sTextAlign = "center";
        else
            sTextAlign = "left";

        AddAttribute(XML_NAMESPACE_FO, XML_TEXT_ALIGN, sTextAlign);
    }

    SvXMLElementExport aParaProps(*this, XML_NAMESPACE_STYLE, XML_PARAGRAPH_PROPERTIES, true, true);
}

static OUString getFontSize(const SvxFontHeightItem& rFont)
{
    OUStringBuffer strBuffer;
    sax::Converter::convertMeasure(strBuffer, rFont.GetHeight(),
                                   com::sun::star::util::MeasureUnit::TWIP,
                                   com::sun::star::util::MeasureUnit::POINT);
    return strBuffer.makeStringAndClear();
}

void SvxTableStylesExport::exportTextProperties(const SvxAutoFormatDataField& rField,
                                                const SvxAutoFormatDataField& rParent)
{
    // exporting <style:text-properties>
    // <fo:color>
    if (rField.IsPropertySet(PROP_COLOR) && rField.GetColor() != rParent.GetColor())
        AddAttribute(XML_NAMESPACE_FO, XML_COLOR,
                     "#" + rField.GetColor().GetValue().AsRGBHexString());

    // <style:font-name, style:font-family>
    if (rField.IsPropertySet(PROP_FONT) && rField.GetFont() != rParent.GetFont())
    {
        AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_NAME, rField.GetFont().GetFamilyName());
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_FAMILY, rField.GetFont().GetFamilyName());
    }
    if (rField.IsPropertySet(PROP_CJK_FONT) && rField.GetCJKFont() != rParent.GetCJKFont())
    {
        AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_NAME_ASIAN, rField.GetCJKFont().GetFamilyName());
        AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_FAMILY_ASIAN,
                     rField.GetCJKFont().GetFamilyName());
    }
    if (rField.IsPropertySet(PROP_CTL_FONT) && rField.GetCTLFont() != rParent.GetCTLFont())
    {
        AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_NAME_COMPLEX,
                     rField.GetCTLFont().GetFamilyName());
        AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_FAMILY_COMPLEX,
                     rField.GetCTLFont().GetFamilyName());
    }

    // <fo:font-size>
    if (rField.IsPropertySet(PROP_HEIGHT) && rField.GetHeight() != rParent.GetHeight())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_SIZE, getFontSize(rField.GetHeight()));
    if (rField.IsPropertySet(PROP_CJK_HEIGHT) && rField.GetCJKHeight() != rParent.GetCJKHeight())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_SIZE_ASIAN, getFontSize(rField.GetCJKHeight()));
    if (rField.IsPropertySet(PROP_CTL_HEIGHT) && rField.GetCTLHeight() != rParent.GetCTLHeight())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_SIZE_COMPLEX, getFontSize(rField.GetCTLHeight()));

    // <fo:font-weight>
    if (rField.IsPropertySet(PROP_WEIGHT) && rField.GetWeight() != rParent.GetWeight())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_WEIGHT,
                     rField.GetWeight().GetWeight() == WEIGHT_BOLD ? u"bold"_ustr : u"normal"_ustr);
    if (rField.IsPropertySet(PROP_CJK_WEIGHT) && rField.GetCJKWeight() != rParent.GetCJKWeight())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_WEIGHT_ASIAN,
                     rField.GetCJKWeight().GetWeight() == WEIGHT_BOLD ? u"bold"_ustr
                                                                      : u"normal"_ustr);
    if (rField.IsPropertySet(PROP_CTL_WEIGHT) && rField.GetCTLWeight() != rParent.GetCTLWeight())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_WEIGHT_COMPLEX,
                     rField.GetCTLWeight().GetWeight() == WEIGHT_BOLD ? u"bold"_ustr
                                                                      : u"normal"_ustr);

    // <fo:font-style>
    if (rField.IsPropertySet(PROP_POSTURE) && rField.GetPosture() != rParent.GetPosture())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_STYLE,
                     rField.GetPosture().GetPosture() == ITALIC_NORMAL ? u"italic"_ustr
                                                                       : u"normal"_ustr);
    if (rField.IsPropertySet(PROP_CJK_POSTURE) && rField.GetCJKPosture() != rParent.GetCJKPosture())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_STYLE_ASIAN,
                     rField.GetCJKPosture().GetPosture() == ITALIC_NORMAL ? u"italic"_ustr
                                                                          : u"normal"_ustr);
    if (rField.IsPropertySet(PROP_CTL_POSTURE) && rField.GetCTLPosture() != rParent.GetCTLPosture())
        AddAttribute(XML_NAMESPACE_FO, XML_FONT_STYLE_COMPLEX,
                     rField.GetCTLPosture().GetPosture() == ITALIC_NORMAL ? u"italic"_ustr
                                                                          : u"normal"_ustr);

    // <style:text-underline-style>
    if (rField.IsPropertySet(PROP_UNDERLINE) && rField.GetUnderline() != rParent.GetUnderline())
        AddAttribute(XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_STYLE,
                     rField.GetUnderline().GetLineStyle() == LINESTYLE_NONE ? u"none"_ustr
                                                                            : u"solid"_ustr);

    SvXMLElementExport aTextProps(*this, XML_NAMESPACE_STYLE, XML_TEXT_PROPERTIES, true, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
