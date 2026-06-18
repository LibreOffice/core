/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <xmlExport2.hxx>

#include <xmloff/numehelp.hxx>
#include "xmlHelper.hxx"
#include <strings.hxx>

#include <com/sun/star/report/GroupOn.hpp>
#include <com/sun/star/report/XFixedText.hpp>
#include <com/sun/star/report/XImageControl.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFunction.hpp>
#include <com/sun/star/report/XFixedLine.hpp>

// Putting these here for now in case some are not needed
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/anytostring.hxx>
#include <reportformula.hxx>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <comphelper/sequence.hxx>
#include <com/sun/star/sheet/AddressConvention.hpp>

namespace rptxml
{
using namespace xmloff;
using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;

ORptExecuteExport::ORptExecuteExport(const Reference<XComponentContext>& _rxContext,
                                     OUString const& implementationName,
                                     SvXMLExportFlags nExportFlag)
    : ORptExport(_rxContext, implementationName, nExportFlag)
{
    // Get XFormulaParser
    uno::Reference<lang::XMultiComponentFactory> xFactory = _rxContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory(xFactory, uno::UNO_QUERY);
    css::uno::Reference<::css::lang::XComponent> xCalcComponent(
        xServiceFactory->createInstance(u"com.sun.star.comp.Calc.SpreadsheetDocument"_ustr),
        css::uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xSM(xCalcComponent, css::uno::UNO_QUERY_THROW);
    Reference<css::sheet::XFormulaParser> xFormulaParser(
        xSM->createInstance(u"com.sun.star.sheet.FormulaParser"_ustr), UNO_QUERY);
    m_xFormulaParser = xFormulaParser;

    Reference<XPropertySet> xParserProps(m_xFormulaParser, uno::UNO_QUERY);
    xParserProps->setPropertyValue(u"FormulaConvention"_ustr,
                                   uno::Any(css::sheet::AddressConvention::XL_OOX));
    xParserProps->setPropertyValue(u"RefConventionChartOOXML"_ustr, uno::Any(true));
}

void ORptExecuteExport::exportReport(const Reference<XReportDefinition>& _xReportDefinition)
{
    if (!_xReportDefinition.is())
        return;

    Reference<XConnection> xConnection = _xReportDefinition->getActiveConnection();
    Reference<XStatement> xStatement = xConnection->createStatement();
    OUString sCommand = _xReportDefinition->getCommand();

    // Get a comma delimited list of Column names to use in the SELECT statement
    // Using SELECT * wasn't allowing the column names to be queried for some reason
    // This also ensures we have the correct ordering of the columns
    OUString sColumnNames = getColumnNameString(_xReportDefinition);

    Reference<XResultSet> xResultSet = xStatement->executeQuery(
        u"SELECT "_ustr + sColumnNames + u" FROM \""_ustr + sCommand + u"\""_ustr);
    m_xResultSet = xResultSet;

    exportFunctions(_xReportDefinition->getFunctions());
    exportGroupsExpressionAsFunction(_xReportDefinition->getGroups());

    // Detail section gets generated
    exportGroup(_xReportDefinition, 0, false);
}

void ORptExecuteExport::exportReportElement(const Reference<XReportControlModel>& _xReportElement)
{
    assert(_xReportElement.is() && "_xReportElement is NULL -> GPF");
    if (!_xReportElement->getPrintWhenGroupChange())
        AddAttribute(XML_NAMESPACE_REPORT, XML_PRINT_WHEN_GROUP_CHANGE, XML_FALSE);

    if (!_xReportElement->getPrintRepeatedValues())
        AddAttribute(XML_NAMESPACE_REPORT, XML_PRINT_REPEATED_VALUES, XML_FALSE);

    if (_xReportElement->getCount())
    {
        exportFormatConditions(_xReportElement);
    }

    OUString sExpr = _xReportElement->getConditionalPrintExpression();
    if (!sExpr.isEmpty())
    {
        ORptExport::exportFormula(XML_FORMULA, sExpr);
        SvXMLElementExport aPrintExpr(*this, XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION,
                                      true, true);
    }
}

void ORptExecuteExport::exportSection(const Reference<XSection>& _xSection, bool bHeader)
{
    assert(_xSection.is() && "Section is NULL -> GPF");
    AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, _xSection->getName());

    if (!bHeader)
    {
        OUStringBuffer sValue;
        sal_Int16 nRet = _xSection->getForceNewPage();
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetForceNewPageOptions();
        if (SvXMLUnitConverter::convertEnum(sValue, nRet, aXML_EnumMap))
            AddAttribute(XML_NAMESPACE_REPORT, XML_FORCE_NEW_PAGE, sValue.makeStringAndClear());

        nRet = _xSection->getNewRowOrCol();
        if (SvXMLUnitConverter::convertEnum(sValue, nRet, aXML_EnumMap))
            AddAttribute(XML_NAMESPACE_REPORT, XML_FORCE_NEW_COLUMN, sValue.makeStringAndClear());
        if (_xSection->getKeepTogether())
            AddAttribute(XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER, XML_TRUE);
    }

    exportStyleName(_xSection.get(), GetAttrList(), m_sTableStyle);

    /// TODO export as table layout
    SvXMLElementExport aComponents(*this, XML_NAMESPACE_TABLE, XML_TABLE, true, true);

    OUString sExpr = _xSection->getConditionalPrintExpression();
    if (!sExpr.isEmpty())
    {
        ORptExport::exportFormula(XML_FORMULA, sExpr);
        SvXMLElementExport aPrintExpr(*this, XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION,
                                      true, false);
    }

    if (_xSection->getName() == u"Detail"_ustr)
    {
        Reference<XRow> xRow(m_xResultSet, UNO_QUERY);
        m_xRow = xRow;

        exportTableColumns(_xSection);
        while (m_xResultSet->next())
        {
            exportContainer(_xSection);
        }
    }
    else
    {
        exportTableColumns(_xSection);
        exportContainer(_xSection);
    }
}

void ORptExecuteExport::handleTextElement(const Reference<XServiceInfo>& xElement,
                                          bool bShapeHandled, const Reference<XSection>& _xSection)
{
    if (!bShapeHandled)
        exportShapes(_xSection, false);

    uno::Reference<XShape> xShape(xElement, uno::UNO_QUERY);
    uno::Reference<XFixedLine> xFixedLine(xElement, uno::UNO_QUERY);
    if (!xShape.is() && !xFixedLine.is())
    {
        Reference<XReportControlModel> xReportElement(xElement, uno::UNO_QUERY);
        Reference<XReportDefinition> xReportDefinition(xElement, uno::UNO_QUERY);
        Reference<XImageControl> xImage(xElement, uno::UNO_QUERY);
        Reference<XSection> xSection(xElement, uno::UNO_QUERY);

        XMLTokenEnum eToken = XML_SECTION;
        bool bExportData = false;
        bool bPageSet = false;
        if (xElement->supportsService(SERVICE_FIXEDTEXT))
        {
            eToken = XML_FIXED_CONTENT;
        }
        else if (xElement->supportsService(SERVICE_FORMATTEDFIELD))
        {
            eToken = XML_FORMATTED_TEXT;
            bExportData = true;
        }
        else if (xElement->supportsService(SERVICE_IMAGECONTROL))
        {
            eToken = XML_IMAGE;
            OUString sTargetLocation = xImage->getImageURL();
            if (!sTargetLocation.isEmpty())
            {
                sTargetLocation = GetRelativeReference(sTargetLocation);
                AddAttribute(XML_NAMESPACE_FORM, XML_IMAGE_DATA, sTargetLocation);
            }
            bExportData = true;
            OUStringBuffer sValue;
            const SvXMLEnumMapEntry<sal_Int16>* aXML_ImageScaleEnumMap
                = OXMLHelper::GetImageScaleOptions();
            if (SvXMLUnitConverter::convertEnum(sValue, xImage->getScaleMode(),
                                                aXML_ImageScaleEnumMap))
                AddAttribute(XML_NAMESPACE_REPORT, XML_SCALE, sValue.makeStringAndClear());
        }
        else if (xReportDefinition.is())
        {
            eToken = XML_SUB_DOCUMENT;
        }
        if (bExportData)
        {
            SvXMLElementExport aParagraphContent(*this, XML_NAMESPACE_TEXT, XML_P, false, false);
            bool bPrevCharIsSpace = false;

            rptui::ReportFormula aFormula(xReportElement->getDataField());

            if (aFormula.getType() == rptui::ReportFormula::BindType::Field && aFormula.isValid())
                GetTextParagraphExport()->exportCharacterData(
                    m_xRow->getString(getColumnNum(aFormula.getFieldName(), m_xRow)),
                    bPrevCharIsSpace);

            if (aFormula.getType() == rptui::ReportFormula::BindType::Expression
                && aFormula.isValid())
            {
                OUStringBuffer aFormulaCopy(aFormula.getExpression());
                //TODO sort out bPageSet
                bPageSet = false; //? Making this false for now, need to investigate

                // A BracketedField is a database column
                // Get the column name(s) from the Formula and substitute the value
                if (aFormula.containsBracketedField())
                {
                    while (aFormulaCopy.indexOf(u"[") != -1 && aFormulaCopy.indexOf(u"]") != -1)
                    {
                        sal_Int32 nBeginBracketIndex = aFormulaCopy.indexOf(u"[");
                        sal_Int32 nEndBracketIndex = aFormulaCopy.indexOf(u"]") + 1;
                        OUString sFieldName = aFormulaCopy
                                                  .copy(nBeginBracketIndex + 1,
                                                        (nEndBracketIndex - 2) - nBeginBracketIndex)
                                                  .toString();
                        // TODO: Need checking here to make sure the FieldName is an actual Database Column name
                        aFormulaCopy.remove(nBeginBracketIndex,
                                            nEndBracketIndex - nBeginBracketIndex);
                        aFormulaCopy.insert(nBeginBracketIndex,
                                            m_xRow->getString(getColumnNum(sFieldName, m_xRow)));
                    }
                    bPageSet = exportFormula(aFormulaCopy.toString(), aFormula);
                }
                else
                    bPageSet = exportFormula(aFormula.getExpression(), aFormula);
            }
            if (bPageSet)
                eToken = XML_FIXED_CONTENT;
            else if (eToken == XML_IMAGE)
                AddAttribute(XML_NAMESPACE_REPORT, XML_PRESERVE_IRI,
                             xImage->getPreserveIRI() ? XML_TRUE : XML_FALSE);
        }

        handleEToken(xReportElement, xReportDefinition, xSection, eToken);
    }
}

bool ORptExecuteExport::exportFormula(const OUString& _sFormula, rptui::ReportFormula& aFormula)
{
    const OUString sFieldData = ORptExport::convertFormula(_sFormula);

    sal_Int32 nPageNumberIndex = sFieldData.indexOf("PageNumber()");
    sal_Int32 nPageCountIndex = sFieldData.indexOf("PageCount()");
    bool bRet = nPageNumberIndex != -1 || nPageCountIndex != -1;

    Sequence<sheet::FormulaToken> aTokens
        = m_xFormulaParser->parseFormula(_sFormula, table::CellAddress(0, 0, 0));

    ::std::vector<Any> vParams;

    // Collect parameters
    if (aTokens.getLength() != -1)
    {
        for (sal_Int32 i = 0; i < aTokens.getLength(); i++)
        {
            if (aTokens[i].OpCode == 0)
            {
                vParams.push_back(aTokens[i].Data);
            }
        }
    }

    const Sequence<Any> aParams = comphelper::containerToSequence(vParams);

    if (!bRet)
    {
        Any aAnswer = callFunction(aParams, aFormula.getFormulaName());
        bool bPrevCharIsSpace = false;
        OUString aRet = getStringFromAny(aAnswer);
        GetTextParagraphExport()->exportCharacterData(aRet, bPrevCharIsSpace);
    }

    return bRet;
}

void ORptExecuteExport::exportComponent(const Reference<XReportComponent>& _xReportComponent)
{
    OSL_ENSURE(_xReportComponent.is(), "No component interface!");
    if (!_xReportComponent.is())
        return;

    AddAttribute(XML_NAMESPACE_DRAW, XML_NAME, _xReportComponent->getName());
}

void ORptExecuteExport::exportStyleName(XPropertySet* _xProp, comphelper::AttributeList& _rAtt,
                                        const OUString& _sName)
{
    Reference<XPropertySet> xFind(_xProp);
    TPropertyStyleMap::const_iterator aFind = m_aAutoStyleNames.find(xFind);
    if (aFind != m_aAutoStyleNames.end())
    {
        _rAtt.AddAttribute(_sName, aFind->second);
    }
}

void ORptExecuteExport::exportGroup(const Reference<XReportDefinition>& _xReportDefinition,
                                    sal_Int32 _nPos, bool _bExportAutoStyle)
{
    if (!_xReportDefinition.is())
        return;

    Reference<XGroups> xGroups = _xReportDefinition->getGroups();
    if (!xGroups.is())
        return;

    sal_Int32 nCount = xGroups->getCount();
    if (_nPos >= 0 && _nPos < nCount)
    {
        Reference<XGroup> xGroup(xGroups->getByIndex(_nPos), uno::UNO_QUERY);
        assert(xGroup.is() && "No Group prepare for GPF");
        if (_bExportAutoStyle)
        {
            if (xGroup->getHeaderOn())
                exportSectionAutoStyle(xGroup->getHeader());
            exportGroup(_xReportDefinition, _nPos + 1, _bExportAutoStyle);
            if (xGroup->getFooterOn())
                exportSectionAutoStyle(xGroup->getFooter());
        }
        else
        {
            if (xGroup->getSortAscending())
                AddAttribute(XML_NAMESPACE_REPORT, XML_SORT_ASCENDING, XML_TRUE);

            if (xGroup->getStartNewColumn())
                AddAttribute(XML_NAMESPACE_REPORT, XML_START_NEW_COLUMN, XML_TRUE);
            if (xGroup->getResetPageNumber())
                AddAttribute(XML_NAMESPACE_REPORT, XML_RESET_PAGE_NUMBER, XML_TRUE);

            const OUString sField = xGroup->getExpression();
            OUString sExpression = sField;
            if (!sExpression.isEmpty())
            {
                sExpression = sExpression.replaceAll(u"\"", u"\"\"");

                TGroupFunctionMap::const_iterator aGroupFind = m_aGroupFunctionMap.find(xGroup);
                if (aGroupFind != m_aGroupFunctionMap.end())
                    sExpression = aGroupFind->second->getName();
                sExpression = "rpt:HASCHANGED(\"" + sExpression + "\")";
            }
            AddAttribute(XML_NAMESPACE_REPORT, XML_SORT_EXPRESSION, sField);
            AddAttribute(XML_NAMESPACE_REPORT, XML_GROUP_EXPRESSION, sExpression);
            sal_Int16 nRet = xGroup->getKeepTogether();
            OUStringBuffer sValue;
            const SvXMLEnumMapEntry<sal_Int16>* aXML_KeepTogetherEnumMap
                = OXMLHelper::GetKeepTogetherOptions();
            if (SvXMLUnitConverter::convertEnum(sValue, nRet, aXML_KeepTogetherEnumMap))
                AddAttribute(XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER, sValue.makeStringAndClear());

            SvXMLElementExport aGroup(*this, XML_NAMESPACE_REPORT, XML_GROUP, true, true);
            exportFunctions(xGroup->getFunctions());
            if (xGroup->getHeaderOn())
            {
                Reference<XSection> xSection = xGroup->getHeader();
                if (xSection->getRepeatSection())
                    AddAttribute(XML_NAMESPACE_REPORT, XML_REPEAT_SECTION, XML_TRUE);
                SvXMLElementExport aGroupSection(*this, XML_NAMESPACE_REPORT, XML_GROUP_HEADER,
                                                 true, true);
                exportSection(xSection);
            }
            exportGroup(_xReportDefinition, _nPos + 1, _bExportAutoStyle);
            if (xGroup->getFooterOn())
            {
                Reference<XSection> xSection = xGroup->getFooter();
                if (xSection->getRepeatSection())
                    AddAttribute(XML_NAMESPACE_REPORT, XML_REPEAT_SECTION, XML_TRUE);
                SvXMLElementExport aGroupSection(*this, XML_NAMESPACE_REPORT, XML_GROUP_FOOTER,
                                                 true, true);
                exportSection(xSection);
            }
        }
    }
    else if (_bExportAutoStyle)
    {
        exportSectionAutoStyle(_xReportDefinition->getDetail());
    }
    else
    {
        // Detail section gets exported here
        // Commenting out for now
        //SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_DETAIL, true, true);
        exportSection(_xReportDefinition->getDetail(), false);
    }
}

void ORptExecuteExport::ExportContent_() { exportReport(m_pReportDefinition); }

void ORptExecuteExport::exportGroupsExpressionAsFunction(const Reference<XGroups>& _xGroups)
{
    if (!_xGroups.is())
        return;

    uno::Reference<XFunctions> xFunctions = _xGroups->getReportDefinition()->getFunctions();
    const sal_Int32 nCount = _xGroups->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<XGroup> xGroup(_xGroups->getByIndex(i), uno::UNO_QUERY_THROW);
        const ::sal_Int16 nGroupOn = xGroup->getGroupOn();
        if (nGroupOn != report::GroupOn::DEFAULT)
        {
            uno::Reference<XFunction> xFunction = xFunctions->createFunction();
            OUString sFunction, sPrefix, sPostfix;
            OUString sExpression = xGroup->getExpression();
            OUString sFunctionName;
            OUString sInitialFormula;
            switch (nGroupOn)
            {
                case report::GroupOn::PREFIX_CHARACTERS:
                    sFunction = "LEFT";
                    sPrefix = ";" + OUString::number(xGroup->getGroupInterval());
                    break;
                case report::GroupOn::YEAR:
                    sFunction = "YEAR";
                    break;
                case report::GroupOn::QUARTAL:
                    sFunction = "INT((MONTH";
                    sPostfix = "-1)/3)+1";
                    sFunctionName = "QUARTAL_" + sExpression;
                    break;
                case report::GroupOn::MONTH:
                    sFunction = "MONTH";
                    break;
                case report::GroupOn::WEEK:
                    sFunction = "WEEK";
                    break;
                case report::GroupOn::DAY:
                    sFunction = "DAY";
                    break;
                case report::GroupOn::HOUR:
                    sFunction = "HOUR";
                    break;
                case report::GroupOn::MINUTE:
                    sFunction = "MINUTE";
                    break;
                case report::GroupOn::INTERVAL:
                {
                    sFunction = "INT";
                    uno::Reference<XFunction> xCountFunction = xFunctions->createFunction();
                    xCountFunction->setInitialFormula(
                        beans::Optional<OUString>(true, u"rpt:0"_ustr));
                    OUString sCountName = sFunction + "_count_" + sExpression;
                    xCountFunction->setName(sCountName);
                    xCountFunction->setFormula("rpt:[" + sCountName + "] + 1");
                    exportFunction(xCountFunction);
                    sExpression = sCountName;
                    // The reference to sCountName in the formula of sFunctionName refers to the *old* value
                    // so we need to expand the formula of sCountName
                    sPrefix = " + 1) / " + OUString::number(xGroup->getGroupInterval());
                    sFunctionName = sFunction + "_" + sExpression;
                    sFunction += "(";
                    sInitialFormula = "rpt:0";
                }
                break;
                default:;
            }
            if (sFunctionName.isEmpty())
                sFunctionName = sFunction + "_" + sExpression;
            if (!sFunction.isEmpty())
            {
                const sal_Unicode pReplaceChars[]
                    = { '(', ')', ';', ',', '+', '-', '[', ']', '/', '*' };
                for (sal_Unicode ch : pReplaceChars)
                    sFunctionName = sFunctionName.replace(ch, '_');

                xFunction->setName(sFunctionName);
                if (!sInitialFormula.isEmpty())
                    xFunction->setInitialFormula(beans::Optional<OUString>(true, sInitialFormula));
                sFunction = "rpt:" + sFunction + "([" + sExpression + "]";

                if (!sPrefix.isEmpty())
                    sFunction += sPrefix;
                sFunction += ")";
                if (!sPostfix.isEmpty())
                    sFunction += sPostfix;
                xFunction->setFormula(sFunction);
                exportFunction(xFunction);
                m_aGroupFunctionMap.emplace(xGroup, xFunction);
            }
        }
    }
}

rtl::Reference<ORptExecuteExport>
ORptExecuteExport::createSettingsExporter(const Reference<XComponentContext>& rxContext)
{
    return new ORptExecuteExport(rxContext, u"com.sun.star.comp.report.XMLSettingsExporter"_ustr,
                                 SvXMLExportFlags::SETTINGS);
}

rtl::Reference<ORptExecuteExport>
ORptExecuteExport::createStylesExporter(const Reference<XComponentContext>& rxContext)
{
    return new ORptExecuteExport(rxContext, u"com.sun.star.comp.report.XMLStylesExporter"_ustr,
                                 SvXMLExportFlags::STYLES | SvXMLExportFlags::MASTERSTYLES
                                     | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::FONTDECLS
                                     | SvXMLExportFlags::OASIS);
}

rtl::Reference<ORptExecuteExport>
ORptExecuteExport::createMetaExporter(const Reference<XComponentContext>& rxContext)
{
    return new ORptExecuteExport(rxContext, u"com.sun.star.comp.report.XMLMetaExporter"_ustr,
                                 SvXMLExportFlags::META);
}

rtl::Reference<ORptExecuteExport>
ORptExecuteExport::createExportFilter(const Reference<XComponentContext>& rxContext)
{
    return new ORptExecuteExport(rxContext, u"com.sun.star.comp.report.ExportFilter"_ustr,
                                 SvXMLExportFlags::CONTENT | SvXMLExportFlags::AUTOSTYLES
                                     | SvXMLExportFlags::FONTDECLS);
}

// A helper function that is given a Column Name and returns the Column Number
sal_Int32 ORptExecuteExport::getColumnNum(std::u16string_view sColumnName, Reference<XRow>& xRow)
{
    Reference<sdbcx::XColumnsSupplier> xColumns(xRow, UNO_QUERY);
    Reference<container::XNameAccess> xColumnAccess(xColumns->getColumns());
    Sequence<OUString> sColumnNames = xColumnAccess->getElementNames();
    sal_Int16 columnNum = -1;
    for (sal_Int32 i = 0; i < sColumnNames.getLength(); i++)
    {
        if (sColumnNames[i] == sColumnName)
            return i + 1; //database columns start at 1
    }
    return columnNum;
}

// A helper function to build a list of comma seperated Column Names
// To be used in a SQL statement
OUString
ORptExecuteExport::getColumnNameString(const Reference<XReportDefinition>& _xReportDefinition)
{
    Reference<lang::XComponent> xHoldAlive;
    uno::Reference<container::XNameAccess> xColumns;
    xColumns = dbtools::getFieldsByCommandDescriptor(_xReportDefinition->getActiveConnection(),
                                                     _xReportDefinition->getCommandType(),
                                                     _xReportDefinition->getCommand(), xHoldAlive);
    OUString sColumnNameList;
    Sequence<OUString> sColumnNames = xColumns->getElementNames();

    for (sal_Int32 i = 0; i < sColumnNames.getLength() - 1; i++)
    {
        sColumnNameList = sColumnNameList + u"\""_ustr + sColumnNames[i] + u"\", "_ustr;
    }
    sColumnNameList
        = sColumnNameList + u"\""_ustr + sColumnNames[sColumnNames.getLength() - 1] + u"\""_ustr;
    return sColumnNameList;
}

Any ORptExecuteExport::callFunction(const Sequence<Any>& aArgs, const OUString& sFuncName)
{
    Reference<sheet::XFunctionAccess> xFunc;
    Reference<XMultiServiceFactory> xFactory(getProcessServiceFactory());
    if (xFactory.is())
    {
        xFunc.set(xFactory->createInstance(u"com.sun.star.sheet.FunctionAccess"_ustr),
                  UNO_QUERY_THROW);
    }
    Any aRet = xFunc->callFunction(sFuncName, aArgs);
    return aRet;
}

OUString ORptExecuteExport::getStringFromAny(Any& aAnswer)
{
    OUString aRet;
    switch (aAnswer.getValueTypeClass())
    {
        case uno::TypeClass_STRING:
        {
            aAnswer >>= aRet;
            break;
        }
        case uno::TypeClass_DOUBLE:
        {
            double nAnswer;
            aAnswer >>= nAnswer;
            aRet = OUString::number(nAnswer);
            break;
        }
        default:
        {
            OSL_FAIL("Unrecognized return Type in ReportDesign");
        }
    }
    return aRet;
}
} // rptxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
