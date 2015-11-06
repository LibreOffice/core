/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlcondformat.hxx"
#include <xmloff/nmspmap.hxx>

#include "colorscale.hxx"
#include "conditio.hxx"
#include "document.hxx"
#include <sax/tools/converter.hxx>
#include "rangelst.hxx"
#include "rangeutl.hxx"
#include "docfunc.hxx"
#include "XMLConverter.hxx"
#include "stylehelper.hxx"

ScXMLConditionalFormatsContext::ScXMLConditionalFormatsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    GetScImport().SetNewCondFormatData();
    GetScImport().GetDocument()->SetCondFormList(new ScConditionalFormatList(), GetScImport().GetTables().GetCurrentSheet());
}

SvXMLImportContext* ScXMLConditionalFormatsContext::CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetScImport().GetCondFormatsTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    SvXMLImportContext* pContext = NULL;
    switch (nToken)
    {
        case XML_TOK_CONDFORMATS_CONDFORMAT:
            pContext = new ScXMLConditionalFormatContext( GetScImport(), nPrefix, rLocalName, xAttrList );
            break;
    }

    return pContext;
}

void ScXMLConditionalFormatsContext::EndElement()
{
    ScDocument* pDoc = GetScImport().GetDocument();

    SCTAB nTab = GetScImport().GetTables().GetCurrentSheet();
    ScConditionalFormatList* pCondFormatList = pDoc->GetCondFormList(nTab);
    bool bDeleted = !pCondFormatList->CheckAllEntries();

    SAL_WARN_IF(bDeleted, "sc", "conditional formats have been deleted because they contained empty range info");
}

ScXMLConditionalFormatContext::ScXMLConditionalFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    OUString sRange;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetCondFormatAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONDFORMAT_TARGET_RANGE:
                sRange = sValue;
            break;
            default:
                break;
        }
    }

    ScRangeStringConverter::GetRangeListFromString(maRange, sRange, GetScImport().GetDocument(),
            formula::FormulaGrammar::CONV_ODF);

    mxFormat.reset(new ScConditionalFormat(0, GetScImport().GetDocument()));
    mxFormat->SetRange(maRange);
}

SvXMLImportContext* ScXMLConditionalFormatContext::CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetScImport().GetCondFormatTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    SvXMLImportContext* pContext = NULL;
    switch (nToken)
    {
        case XML_TOK_CONDFORMAT_CONDITION:
            pContext = new ScXMLCondContext( GetScImport(), nPrefix, rLocalName, xAttrList, mxFormat.get() );
            break;
        case XML_TOK_CONDFORMAT_COLORSCALE:
            pContext = new ScXMLColorScaleFormatContext( GetScImport(), nPrefix, rLocalName, mxFormat.get() );
            break;
        case XML_TOK_CONDFORMAT_DATABAR:
            pContext = new ScXMLDataBarFormatContext( GetScImport(), nPrefix, rLocalName, xAttrList, mxFormat.get() );
            break;
        case XML_TOK_CONDFORMAT_ICONSET:
            pContext = new ScXMLIconSetFormatContext( GetScImport(), nPrefix, rLocalName, xAttrList, mxFormat.get() );
            break;
        case XML_TOK_CONDFORMAT_DATE:
            pContext = new ScXMLDateContext( GetScImport(), nPrefix, rLocalName, xAttrList, mxFormat.get() );
            break;
        default:
            break;
    }

    return pContext;
}

void ScXMLConditionalFormatContext::EndElement()
{
    ScDocument* pDoc = GetScImport().GetDocument();

    SCTAB nTab = GetScImport().GetTables().GetCurrentSheet();
    ScConditionalFormat* pFormat = mxFormat.release();
    sal_uLong nIndex = pDoc->AddCondFormat(pFormat, nTab);
    pFormat->SetKey(nIndex);

    pDoc->AddCondFormatData( pFormat->GetRange(), nTab, nIndex);
}

ScXMLConditionalFormatContext::~ScXMLConditionalFormatContext()
{
}

ScXMLColorScaleFormatContext::ScXMLColorScaleFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName, ScConditionalFormat* pFormat):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pColorScaleFormat(NULL)
{
    pColorScaleFormat = new ScColorScaleFormat(GetScImport().GetDocument());
    pFormat->AddEntry(pColorScaleFormat);
}

SvXMLImportContext* ScXMLColorScaleFormatContext::CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetScImport().GetColorScaleTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    SvXMLImportContext* pContext = NULL;
    switch (nToken)
    {
        case XML_TOK_COLORSCALE_COLORSCALEENTRY:
            pContext = new ScXMLColorScaleFormatEntryContext( GetScImport(), nPrefix, rLocalName, xAttrList, pColorScaleFormat );
            break;
        default:
            break;
    }

    return pContext;
}

ScXMLDataBarFormatContext::ScXMLDataBarFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpDataBarFormat(NULL),
    mpFormatData(NULL)
{
    OUString sPositiveColor;
    OUString sNegativeColor;
    OUString sGradient;
    OUString sAxisPosition;
    OUString sShowValue;
    OUString sAxisColor;
    OUString sMinLength;
    OUString sMaxLength;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataBarAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATABAR_POSITIVE_COLOR:
                sPositiveColor = sValue;
                break;
            case XML_TOK_DATABAR_GRADIENT:
                sGradient = sValue;
                break;
            case XML_TOK_DATABAR_NEGATIVE_COLOR:
                sNegativeColor = sValue;
                break;
            case XML_TOK_DATABAR_AXISPOSITION:
                sAxisPosition = sValue;
                break;
            case XML_TOK_DATABAR_SHOWVALUE:
                sShowValue = sValue;
                break;
            case XML_TOK_DATABAR_AXISCOLOR:
                sAxisColor = sValue;
                break;
            case XML_TOK_DATABAR_MINLENGTH:
                sMinLength = sValue;
                break;
            case XML_TOK_DATABAR_MAXLENGTH:
                sMaxLength = sValue;
                break;
            default:
                break;
        }
    }

    mpDataBarFormat = new ScDataBarFormat(rImport.GetDocument());
    mpFormatData = new ScDataBarFormatData();
    mpDataBarFormat->SetDataBarData(mpFormatData);
    if(!sGradient.isEmpty())
    {
        bool bGradient = true;
        sax::Converter::convertBool( bGradient, sGradient);
        mpFormatData->mbGradient = bGradient;
    }

    if(!sPositiveColor.isEmpty())
    {
        sal_Int32 nColor = 0;
        sax::Converter::convertColor( nColor, sPositiveColor );
        mpFormatData->maPositiveColor = Color(nColor);
    }

    if(!sNegativeColor.isEmpty())
    {
        // we might check here for 0xff0000 and don't write it
        sal_Int32 nColor = 0;
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
        sal_Int32 nColor = 0;
        sax::Converter::convertColor( nColor, sAxisColor );
        mpFormatData->maAxisColor = Color(nColor);
    }

    if(!sShowValue.isEmpty())
    {
        bool bShowValue = true;
        sax::Converter::convertBool( bShowValue, sShowValue );
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

    pFormat->AddEntry(mpDataBarFormat);
}

SvXMLImportContext* ScXMLDataBarFormatContext::CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetScImport().GetFormattingTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    SvXMLImportContext* pContext = NULL;
    switch (nToken)
    {
        case XML_TOK_FORMATTING_ENTRY:
        case XML_TOK_DATABAR_DATABARENTRY:
        {
            ScColorScaleEntry* pEntry(0);
            pContext = new ScXMLFormattingEntryContext( GetScImport(), nPrefix, rLocalName, xAttrList, pEntry );
            if(mpFormatData->mpLowerLimit)
            {
                mpFormatData->mpUpperLimit.reset(pEntry);
            }
            else
            {
                mpFormatData->mpLowerLimit.reset(pEntry);
            }
        }
        break;
        default:
            break;
    }

    return pContext;
}

ScXMLIconSetFormatContext::ScXMLIconSetFormatContext(ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    OUString aIconSetType, sShowValue;
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetIconSetAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_ICONSET_TYPE:
                aIconSetType = sValue;
                break;
            case XML_TOK_ICONSET_SHOWVALUE:
                sShowValue = sValue;
                break;
            default:
                break;
        }
    }

    ScIconSetMap* pMap = ScIconSetFormat::getIconSetMap();
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
        sax::Converter::convertBool( bShowValue, sShowValue );
        pIconSetFormatData->mbShowValue = !bShowValue;
    }

    pIconSetFormatData->eIconSetType = eType;
    pIconSetFormat->SetIconSetData(pIconSetFormatData);
    pFormat->AddEntry(pIconSetFormat);

    mpFormatData = pIconSetFormatData;
}

SvXMLImportContext* ScXMLIconSetFormatContext::CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetScImport().GetFormattingTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    SvXMLImportContext* pContext = NULL;
    switch (nToken)
    {
        case XML_TOK_FORMATTING_ENTRY:
            {
                ScColorScaleEntry* pEntry(0);
                pContext = new ScXMLFormattingEntryContext( GetScImport(), nPrefix, rLocalName, xAttrList, pEntry );
                mpFormatData->m_Entries.push_back(std::unique_ptr<ScColorScaleEntry>(pEntry));
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
        eMode = SC_COND_NOTDUPLICATE;
    }
    else if(rValue.startsWith("duplicate"))
    {
        eMode = SC_COND_DUPLICATE;
    }
    else if(rValue.startsWith("between"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 8;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ',');
        rExpr2 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_BETWEEN;
    }
    else if(rValue.startsWith("not-between"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 12;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ',');
        rExpr2 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_NOTBETWEEN;
    }
    else if(rValue.startsWith("<="))
    {
        rExpr1 = rValue.copy(2);
        eMode = SC_COND_EQLESS;
    }
    else if(rValue.startsWith(">="))
    {
        rExpr1 = rValue.copy(2);
        eMode = SC_COND_EQGREATER;
    }
    else if(rValue.startsWith("!="))
    {
        rExpr1 = rValue.copy(2);
        eMode = SC_COND_NOTEQUAL;
    }
    else if(rValue.startsWith("<"))
    {
        rExpr1 = rValue.copy(1);
        eMode = SC_COND_LESS;
    }
    else if(rValue.startsWith("="))
    {
        rExpr1 = rValue.copy(1);
        eMode = SC_COND_EQUAL;
    }
    else if(rValue.startsWith(">"))
    {
        rExpr1 = rValue.copy(1);
        eMode = SC_COND_GREATER;
    }
    else if(rValue.startsWith("formula-is"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 11;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_DIRECT;
    }
    else if(rValue.startsWith("top-elements"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 13;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_TOP10;
    }
    else if(rValue.startsWith("bottom-elements"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 16;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_BOTTOM10;
    }
    else if(rValue.startsWith("top-percent"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 12;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_TOP_PERCENT;
    }
    else if(rValue.startsWith("bottom-percent"))
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 15;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_BOTTOM_PERCENT;
    }
    else if(rValue.startsWith("above-average"))
    {
        eMode = SC_COND_ABOVE_AVERAGE;
    }
    else if(rValue.startsWith("below-average"))
    {
        eMode = SC_COND_BELOW_AVERAGE;
    }
    else if(rValue.startsWith("above-equal-average"))
    {
        eMode = SC_COND_ABOVE_EQUAL_AVERAGE;
    }
    else if(rValue.startsWith("below-equal-average"))
    {
        eMode = SC_COND_BELOW_EQUAL_AVERAGE;
    }
    else if(rValue.startsWith("is-error"))
    {
        eMode = SC_COND_ERROR;
    }
    else if(rValue.startsWith("is-no-error"))
    {
        eMode = SC_COND_NOERROR;
    }
    else if(rValue.startsWith("begins-with"))
    {
        eMode = SC_COND_BEGINS_WITH;
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 12;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
    }
    else if(rValue.startsWith("ends-with"))
    {
        eMode = SC_COND_ENDS_WITH;
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 10;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
    }
    else if(rValue.startsWith("contains-text"))
    {
        eMode = SC_COND_CONTAINS_TEXT;
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 14;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
    }
    else if(rValue.startsWith("not-contains-text"))
    {
        eMode = SC_COND_NOT_CONTAINS_TEXT;
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 18;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
    }
    else
        eMode = SC_COND_NONE;
}

}

ScXMLCondContext::ScXMLCondContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat ):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    OUString sExpression;
    OUString sStyle;
    OUString sAddress;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetConditionAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONDITION_VALUE:
                sExpression = sValue;
                break;
            case XML_TOK_CONDITION_APPLY_STYLE_NAME:
                sStyle = ScStyleNameConversion::ProgrammaticToDisplayName(sValue, SFX_STYLE_FAMILY_PARA );
                break;
            case XML_TOK_CONDITION_BASE_CELL_ADDRESS:
                sAddress = sValue;
                break;
            default:
                break;
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

ScXMLColorScaleFormatEntryContext::ScXMLColorScaleFormatEntryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                        ScColorScaleFormat* pFormat):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpFormatEntry( NULL )
{
    double nVal = 0;
    Color aColor;

    OUString sType;
    OUString sVal;
    OUString sColor;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetColorScaleEntryAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_COLORSCALEENTRY_TYPE:
                sType = sValue;
                break;
            case XML_TOK_COLORSCALEENTRY_VALUE:
                sVal = sValue;
                break;
            case XML_TOK_COLORSCALEENTRY_COLOR:
                sColor = sValue;
                break;
            default:
                break;
        }
    }

    sal_Int32 nColor;
    sax::Converter::convertColor(nColor, sColor);
    aColor = Color(nColor);

    if(!sVal.isEmpty())
        sax::Converter::convertDouble(nVal, sVal);

    mpFormatEntry = new ScColorScaleEntry(nVal, aColor);
    setColorEntryType(sType, mpFormatEntry, sVal, GetScImport());
    pFormat->AddEntry(mpFormatEntry);
}

ScXMLFormattingEntryContext::ScXMLFormattingEntryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                        ScColorScaleEntry*& pColorScaleEntry):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    OUString sVal;
    OUString sType;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataBarEntryAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATABARENTRY_TYPE:
                sType = sValue;
                break;
            case XML_TOK_DATABARENTRY_VALUE:
                sVal = sValue;
                break;
            default:
                break;
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

ScXMLDateContext::ScXMLDateContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat ):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    OUString sDateType, sStyle;
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetCondDateAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_COND_DATE_VALUE:
                sDateType = sValue;
                break;
            case XML_TOK_COND_DATE_STYLE:
                sStyle = ScStyleNameConversion::ProgrammaticToDisplayName(sValue, SFX_STYLE_FAMILY_PARA );
                break;
            default:
                break;
        }
    }

    ScCondDateFormatEntry* pFormatEntry = new ScCondDateFormatEntry(GetScImport().GetDocument());
    pFormatEntry->SetStyleName(sStyle);
    pFormatEntry->SetDateType(getDateFromString(sDateType));
    pFormat->AddEntry(pFormatEntry);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
