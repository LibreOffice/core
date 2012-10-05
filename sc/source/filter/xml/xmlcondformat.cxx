/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
#include "markdata.hxx"
#include "docpool.hxx"
#include "scitems.hxx"
#include "patattr.hxx"
#include "svl/intitem.hxx"
#include "XMLConverter.hxx"


ScXMLConditionalFormatsContext::ScXMLConditionalFormatsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    GetScImport().SetNewCondFormatData();
    GetScImport().GetDocument()->SetCondFormList(new ScConditionalFormatList(), GetScImport().GetTables().GetCurrentSheet());
}

SvXMLImportContext* ScXMLConditionalFormatsContext::CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
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
}

ScXMLConditionalFormatContext::ScXMLConditionalFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    rtl::OUString sRange;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetCondFormatAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

    mpFormat = new ScConditionalFormat(0, GetScImport().GetDocument());
    mpFormat->AddRange(maRange);
}

SvXMLImportContext* ScXMLConditionalFormatContext::CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetScImport().GetCondFormatTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    SvXMLImportContext* pContext = NULL;
    switch (nToken)
    {
        case XML_TOK_CONDFORMAT_CONDITION:
            pContext = new ScXMLCondContext( GetScImport(), nPrefix, rLocalName, xAttrList, mpFormat );
            break;
        case XML_TOK_CONDFORMAT_COLORSCALE:
            pContext = new ScXMLColorScaleFormatContext( GetScImport(), nPrefix, rLocalName, mpFormat );
            break;
        case XML_TOK_CONDFORMAT_DATABAR:
            pContext = new ScXMLDataBarFormatContext( GetScImport(), nPrefix, rLocalName, xAttrList, mpFormat );
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
    sal_uLong nIndex = pDoc->AddCondFormat(mpFormat, nTab);
    mpFormat->SetKey(nIndex);

    pDoc->AddCondFormatData( mpFormat->GetRange(), nTab, nIndex);
}

ScXMLColorScaleFormatContext::ScXMLColorScaleFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName, ScConditionalFormat* pFormat):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pColorScaleFormat(NULL)
{
    pColorScaleFormat = new ScColorScaleFormat(GetScImport().GetDocument());
    pFormat->AddEntry(pColorScaleFormat);
}

SvXMLImportContext* ScXMLColorScaleFormatContext::CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
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

void ScXMLColorScaleFormatContext::EndElement()
{

}

ScXMLDataBarFormatContext::ScXMLDataBarFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpDataBarFormat(NULL),
    mpFormatData(NULL)
{
    rtl::OUString sPositiveColor;
    rtl::OUString sNegativeColor;
    rtl::OUString sGradient;
    rtl::OUString sAxisPosition;
    rtl::OUString sShowValue;
    rtl::OUString sAxisColor;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataBarAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

    pFormat->AddEntry(mpDataBarFormat);
}

SvXMLImportContext* ScXMLDataBarFormatContext::CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataBarTokenMap();
    sal_uInt16 nToken = rTokenMap.Get(nPrefix, rLocalName);
    SvXMLImportContext* pContext = NULL;
    switch (nToken)
    {
        case XML_TOK_DATABAR_DATABARENTRY:
            pContext = new ScXMLDataBarFormatEntryContext( GetScImport(), nPrefix, rLocalName, xAttrList, mpFormatData );
            break;
        default:
            break;
    }

    return pContext;
}

void ScXMLDataBarFormatContext::EndElement()
{
}

namespace {

void GetConditionData(const rtl::OUString& rValue, ScConditionMode& eMode, rtl::OUString& rExpr1, rtl::OUString& rExpr2)
{
    if(rValue.indexOf("unique") == 0)
    {
        eMode = SC_COND_NOTDUPLICATE;
    }
    else if(rValue.indexOf("duplicate") == 0)
    {
        eMode = SC_COND_DUPLICATE;
    }
    else if(rValue.indexOf("between") == 0)
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 8;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ',');
        rExpr2 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_BETWEEN;
    }
    else if(rValue.indexOf("not-between") == 0)
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 12;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ',');
        rExpr2 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_NOTBETWEEN;
    }
    else if(rValue.indexOf("<=") == 0)
    {
        rExpr1 = rValue.copy(2);
        eMode = SC_COND_EQLESS;
    }
    else if(rValue.indexOf(">=") == 0)
    {
        rExpr1 = rValue.copy(2);
        eMode = SC_COND_EQGREATER;
    }
    else if(rValue.indexOf("!=") == 0)
    {
        rExpr1 = rValue.copy(2);
        eMode = SC_COND_NOTEQUAL;
    }
    else if(rValue.indexOf('<') == 0)
    {
        rExpr1 = rValue.copy(1);
        eMode = SC_COND_LESS;
    }
    else if(rValue.indexOf('=') == 0)
    {
        rExpr1 = rValue.copy(1);
        eMode = SC_COND_EQUAL;
    }
    else if(rValue.indexOf('>') == 0)
    {
        rExpr1 = rValue.copy(1);
        eMode = SC_COND_GREATER;
    }
    else if(rValue.indexOf("formula-is") == 0)
    {
        const sal_Unicode* pStr = rValue.getStr();
        const sal_Unicode* pStart = pStr + 11;
        const sal_Unicode* pEnd = pStr + rValue.getLength();
        rExpr1 = ScXMLConditionHelper::getExpression( pStart, pEnd, ')');
        eMode = SC_COND_DIRECT;
    }
    else
        eMode = SC_COND_NONE;
}

}

ScXMLCondContext::ScXMLCondContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat ):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    rtl::OUString sExpression;
    rtl::OUString sStyle;
    rtl::OUString sAddress;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetConditionAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONDITION_VALUE:
                sExpression = sValue;
                break;
            case XML_TOK_CONDITION_APPLY_STYLE_NAME:
                sStyle = sValue;
            case XML_TOK_CONDITION_BASE_CELL_ADDRESS:
                sAddress = sValue;
            default:
                break;
        }
    }

    rtl::OUString aExpr1;
    rtl::OUString aExpr2;
    ScConditionMode eMode;
    GetConditionData(sExpression, eMode, aExpr1, aExpr2);
    ScAddress aPos;
    sal_Int32 nIndex = 0;
    ScRangeStringConverter::GetAddressFromString(aPos, sAddress, GetScImport().GetDocument(), formula::FormulaGrammar::CONV_ODF, nIndex);

    ScCondFormatEntry* pFormatEntry = new ScCondFormatEntry(eMode, aExpr1, aExpr2, GetScImport().GetDocument(), aPos, sStyle,
                                                        rtl::OUString(), rtl::OUString(), formula::FormulaGrammar::GRAM_ODFF, formula::FormulaGrammar::GRAM_ODFF);

    pFormat->AddEntry(pFormatEntry);
}

void ScXMLCondContext::EndElement()
{

}

namespace {

void setColorEntryType(const rtl::OUString& rType, ScColorScaleEntry* pEntry, const rtl::OUString rFormula,
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
        pEntry->SetType(COLORSCALE_AUTOMIN);
    else if(rType == "auto-maximum")
        pEntry->SetType(COLORSCALE_AUTOMAX);

    //TODO: add formulas
}

}

ScXMLColorScaleFormatEntryContext::ScXMLColorScaleFormatEntryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScColorScaleFormat* pFormat):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpFormatEntry( NULL )
{
    double nVal = 0;
    Color aColor;

    rtl::OUString sType;
    rtl::OUString sVal;
    rtl::OUString sColor;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetColorScaleEntryAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

void ScXMLColorScaleFormatEntryContext::EndElement()
{
}

ScXMLDataBarFormatEntryContext::ScXMLDataBarFormatEntryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScDataBarFormatData* pData):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    rtl::OUString sVal;
    rtl::OUString sType;

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataBarEntryAttrMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                    sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

    ScColorScaleEntry* pEntry = new ScColorScaleEntry(nVal, Color());
    setColorEntryType(sType, pEntry, sVal, GetScImport());
    if(pData->mpLowerLimit)
    {
        pData->mpUpperLimit.reset(pEntry);
    }
    else
    {
        pData->mpLowerLimit.reset(pEntry);
    }
}

void ScXMLDataBarFormatEntryContext::EndElement()
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
