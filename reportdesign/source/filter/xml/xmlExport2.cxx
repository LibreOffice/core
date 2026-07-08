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

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <reportformula.hxx>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <comphelper/sequence.hxx>
#include <com/sun/star/sheet/AddressConvention.hpp>
#include <tools/date.hxx>
#include <o3tl/safeint.hxx>
#include <math.h>

namespace rptxml
{
using namespace xmloff;
using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;

ORptExecuteExport::ORptExecuteExport(const Reference<XComponentContext>& _rxContext,
                                     OUString const& implementationName,
                                     SvXMLExportFlags nExportFlag)
    : ORptExport(_rxContext, implementationName, nExportFlag)
{
    // The FormulaParser service requires a Calc document context.
    // Keep the hidden document alive for the lifetime of this exporter.
    uno::Reference<lang::XMultiComponentFactory> xFactory = _rxContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory(xFactory, uno::UNO_QUERY);
    m_xCalcComponent.set(
        xServiceFactory->createInstance(u"com.sun.star.comp.Calc.SpreadsheetDocument"_ustr),
        css::uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xSM(m_xCalcComponent, css::uno::UNO_QUERY_THROW);
    m_xFormulaParser.set(xSM->createInstance(u"com.sun.star.sheet.FormulaParser"_ustr), UNO_QUERY);

    Reference<XPropertySet> xParserProps(m_xFormulaParser, uno::UNO_QUERY);
    xParserProps->setPropertyValue(u"FormulaConvention"_ustr,
                                   uno::Any(css::sheet::AddressConvention::XL_OOX));
    xParserProps->setPropertyValue(u"RefConventionChartOOXML"_ustr, uno::Any(true));
}

ORptExecuteExport::~ORptExecuteExport()
{
    // Release the FormulaParser before disposing the Calc document it depends on
    m_xFormulaParser.clear();
    if (m_xCalcComponent.is())
        m_xCalcComponent->dispose();
}

void ORptExecuteExport::exportReport(const Reference<XReportDefinition>& _xReportDefinition)
{
    if (!_xReportDefinition.is())
        return;

    if (_xReportDefinition->getReportHeaderOn())
        exportSection(_xReportDefinition->getReportHeader());

    if (_xReportDefinition->getPageHeaderOn())
    {
        OUStringBuffer sValue;
        sal_Int16 nRet = _xReportDefinition->getPageHeaderOption();
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
        if (SvXMLUnitConverter::convertEnum(sValue, nRet, aXML_EnumMap))
            AddAttribute(XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION, sValue.makeStringAndClear());

        exportSection(_xReportDefinition->getPageHeader(), true);
    }

    // Detail section gets generated
    exportGroup(_xReportDefinition, false);

    if (_xReportDefinition->getPageFooterOn())
    {
        OUStringBuffer sValue;
        sal_Int16 nRet = _xReportDefinition->getPageFooterOption();
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
        if (SvXMLUnitConverter::convertEnum(sValue, nRet, aXML_EnumMap))
            AddAttribute(XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION, sValue.makeStringAndClear());

        exportSection(_xReportDefinition->getPageFooter(), true);
    }
    if (_xReportDefinition->getReportFooterOn())
        exportSection(_xReportDefinition->getReportFooter());
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
    exportTableColumns(_xSection);
    exportContainer(_xSection);
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

            // Field
            if (aFormula.getType() == rptui::ReportFormula::BindType::Field && aFormula.isValid())
                GetTextParagraphExport()->exportCharacterData(
                    m_xRow->getString(getColumnNum(aFormula.getFieldName(), m_xRow)),
                    bPrevCharIsSpace);

            // Expression
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
                    bPageSet = exportFormula(aFormulaCopy.toString());
                }
                else
                    bPageSet = exportFormula(aFormula.getExpression());
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

bool ORptExecuteExport::exportFormula(const OUString& _sFormula)
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
        // Extract the function name from the actual formula string being evaluated,
        // not from the original (possibly unsubstituted) ReportFormula object
        //TODO if the functions parameters are not formatted correctly we get an ugly
        // xml formatting error
        const sal_Int32 nParen = _sFormula.indexOf('(');
        if (nParen != -1)
        {
            const OUString sFuncName = _sFormula.copy(0, nParen);
            Any aAnswer = callFunction(aParams, sFuncName);
            bool bPrevCharIsSpace = false;
            OUString aRet = getStringFromAny(aAnswer);
            GetTextParagraphExport()->exportCharacterData(aRet, bPrevCharIsSpace);
        }
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
                                    bool _bExportAutoStyle)
{
    if (!_xReportDefinition.is())
        return;

    Reference<XGroups> xGroups = _xReportDefinition->getGroups();
    if (!xGroups.is())
        return;

    Reference<XResultSet> xResultSet = getResultSet(m_pReportDefinition);
    Reference<XRow> xRow(xResultSet, UNO_QUERY);
    m_xRow = xRow;

    if (xGroups->getCount() > 0)
    {
        std::vector<std::vector<sal_Int32>> vSubGroups;
        // Rows where an already-processed outer group changed; passed to each
        // subsequent (inner) group so its boundaries are forced to align with
        // its outer group's, even on a coincidental own-value repeat.
        std::vector<bool> vOuterBoundary;
        for (sal_Int32 i = 0; i < xGroups->getCount(); ++i)
        {
            // The database driver sets the ResultSet type, so we can't be sure
            // that we can set it to SCROLL_XXX, i.e. we can't rewind the ResultSet
            // so use a second ResultSet here as a workaround
            Reference<XResultSet> xResultSet2 = getResultSet(_xReportDefinition);
            Reference<XGroup> xGroup(xGroups->getByIndex(i), uno::UNO_QUERY);
            std::vector<sal_Int32> vThisGroup = findSubGroups(xGroup, xResultSet2, vOuterBoundary);

            if (vOuterBoundary.empty() && !vThisGroup.empty())
                vOuterBoundary.assign(vThisGroup.size(), false);
            for (size_t nRow = 1; nRow < vThisGroup.size(); ++nRow)
                if (vThisGroup[nRow] != vThisGroup[nRow - 1])
                    vOuterBoundary[nRow] = true;

            vSubGroups.push_back(std::move(vThisGroup));
        }

        std::vector<sal_Int32> vPrevSubGroup(vSubGroups.size(), 0);
        sal_Int32 counter = 0;

        while (xResultSet->next())
        {
            for (size_t nGroupNum = 0; nGroupNum < vSubGroups.size(); ++nGroupNum)
            {
                Reference<XGroup> xGroup(xGroups->getByIndex(nGroupNum), uno::UNO_QUERY);

                // Empty expression, single implicit group: header on first row only
                if (vSubGroups[nGroupNum].empty())
                {
                    if (xResultSet->isFirst() && xGroup->getHeaderOn())
                    {
                        if (_bExportAutoStyle)
                            exportSectionAutoStyle(xGroup->getHeader());
                        else
                            exportSection(xGroup->getHeader());
                    }
                    continue;
                }

                if (xResultSet->isFirst())
                {
                    if (xGroup->getHeaderOn())
                    {
                        if (_bExportAutoStyle)
                            exportSectionAutoStyle(xGroup->getHeader());
                        else
                            exportSection(xGroup->getHeader());
                    }
                }
                else if (vSubGroups[nGroupNum][counter] != vPrevSubGroup[nGroupNum])
                {
                    vPrevSubGroup[nGroupNum] = vSubGroups[nGroupNum][counter];
                    if (xGroup->getHeaderOn())
                    {
                        if (_bExportAutoStyle)
                            exportSectionAutoStyle(xGroup->getHeader());
                        else
                            exportSection(xGroup->getHeader());
                    }
                }
            }

            exportSection(_xReportDefinition->getDetail());

            for (sal_Int32 nGroupNum = vSubGroups.size() - 1; nGroupNum >= 0; --nGroupNum)
            {
                Reference<XGroup> xGroup(xGroups->getByIndex(nGroupNum), uno::UNO_QUERY);

                // Empty expression, single implicit group: footer on last row only
                if (vSubGroups[nGroupNum].empty())
                {
                    if (xResultSet->isLast() && xGroup->getFooterOn())
                    {
                        if (_bExportAutoStyle)
                            exportSectionAutoStyle(xGroup->getFooter());
                        else
                            exportSection(xGroup->getFooter());
                    }
                    continue;
                }

                if (xResultSet->isLast())
                {
                    if (xGroup->getFooterOn())
                    {
                        if (_bExportAutoStyle)
                            exportSectionAutoStyle(xGroup->getFooter());
                        else
                            exportSection(xGroup->getFooter());
                    }
                }
                else if (vSubGroups[nGroupNum][counter + 1] != vPrevSubGroup[nGroupNum])
                {
                    if (xGroup->getFooterOn())
                    {
                        if (_bExportAutoStyle)
                            exportSectionAutoStyle(xGroup->getFooter());
                        else
                            exportSection(xGroup->getFooter());
                    }
                }
            }
            ++counter;
        }
    }
    else if (_bExportAutoStyle)
    {
        while (xResultSet->next())
        {
            exportSectionAutoStyle(_xReportDefinition->getDetail());
        }
    }
    else
    {
        while (xResultSet->next())
        {
            exportSection(_xReportDefinition->getDetail(), false);
        }
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
    if (!xColumns.is())
        return -1;
    Reference<container::XNameAccess> xColumnAccess(xColumns->getColumns());
    Sequence<OUString> sColumnNames = xColumnAccess->getElementNames();
    for (sal_Int32 i = 0; i < sColumnNames.getLength(); i++)
    {
        if (sColumnNames[i] == sColumnName)
            return i + 1; //database columns start at 1
    }
    return -1;
}

// A helper function to build a list of comma-separated Column Names
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

Reference<XResultSet>
ORptExecuteExport::getResultSet(const Reference<XReportDefinition>& _xReportDefinition)
{
    Reference<XConnection> xConnection = _xReportDefinition->getActiveConnection();
    Reference<XStatement> xStatement = xConnection->createStatement();
    OUString sCommand = _xReportDefinition->getCommand();

    OUString sColumnNames = getColumnNameString(_xReportDefinition);

    Reference<XGroups> xGroups = _xReportDefinition->getGroups();
    sal_Int32 nCount = xGroups.is() ? xGroups->getCount() : 0;

    OUString sOrderByStatement;

    if (nCount > 0)
    {
        bool bFirst = true;
        for (sal_Int32 nPos = 0; nPos < nCount; ++nPos)
        {
            Reference<XGroup> xGroup(xGroups->getByIndex(nPos), uno::UNO_QUERY);
            const OUString sField = xGroup->getExpression();

            if (sField.isEmpty())
                continue;

            if (bFirst)
            {
                sOrderByStatement = u" ORDER BY "_ustr;
                bFirst = false;
            }
            else
                sOrderByStatement += u", "_ustr;

            sOrderByStatement += u"\""_ustr + sField + u"\""_ustr
                                 + (xGroup->getSortAscending() ? u" ASC"_ustr : u" DESC"_ustr);
        }
    }

    // The report's command can name a table, a stored query, or be a free-form
    // SQL statement; each needs different treatment to end up in the FROM
    // clause of a valid SELECT.
    OUString sFromClause;
    switch (_xReportDefinition->getCommandType())
    {
        case sdb::CommandType::TABLE:
        {
            OUString sCatalog, sSchema, sTable;
            dbtools::qualifiedNameComponents(xConnection->getMetaData(), sCommand, sCatalog,
                                             sSchema, sTable,
                                             dbtools::EComposeRule::InDataManipulation);
            sFromClause
                = dbtools::composeTableNameForSelect(xConnection, sCatalog, sSchema, sTable);
            break;
        }
        case sdb::CommandType::QUERY:
        {
            OUString sQueryCommand;
            Reference<sdb::XQueriesSupplier> xQueriesAccess(xConnection, UNO_QUERY);
            Reference<container::XNameAccess> xQueries(
                xQueriesAccess.is() ? xQueriesAccess->getQueries() : nullptr);
            if (xQueries.is() && xQueries->hasByName(sCommand))
            {
                Reference<XPropertySet> xQuery(xQueries->getByName(sCommand), UNO_QUERY);
                if (xQuery.is())
                    xQuery->getPropertyValue(u"Command"_ustr) >>= sQueryCommand;
            }
            sFromClause = u"("_ustr + sQueryCommand + u") AS \"T_ReportSource\""_ustr;
            break;
        }
        default: // sdb::CommandType::COMMAND: sCommand is already a full SQL statement
            sFromClause = u"("_ustr + sCommand + u") AS \"T_ReportSource\""_ustr;
            break;
    }

    Reference<XResultSet> xResultSet = xStatement->executeQuery(
        u"SELECT "_ustr + sColumnNames + u" FROM "_ustr + sFromClause + sOrderByStatement);

    return xResultSet;
}

template <typename T>
sal_Int32 ORptExecuteExport::compare(T& _value, T& _groupOnValue, sal_Int32& _i,
                                     const Reference<XResultSet>& _xResultSet, bool bForceBreak)
{
    if (_value != _groupOnValue || bForceBreak)
    {
        _groupOnValue = _value;
        if (!_xResultSet->isFirst())
            ++_i;
    }
    return _i;
}

sal_Int32 ORptExecuteExport::compareQuartal(sal_uInt16 _aMonth, sal_uInt16& _aGroupOnMonth,
                                            sal_Int32& _i, const Reference<XResultSet>& _xResultSet,
                                            bool bForceBreak)
{
    sal_uInt16 nQuarter = std::ceil(_aMonth / 3.0);
    if (_xResultSet->isFirst())
        _aGroupOnMonth = std::ceil(_aGroupOnMonth / 3.0);
    return compare(nQuarter, _aGroupOnMonth, _i, _xResultSet, bForceBreak);
}

std::vector<sal_Int32> ORptExecuteExport::findSubGroups(const Reference<XGroup>& _xGroup,
                                                        const Reference<XResultSet>& _xResultSet,
                                                        const std::vector<bool>& _rOuterBoundary)
{
    const ::sal_Int16 nGroupOn = _xGroup->getGroupOn();

    OUString sExp = _xGroup->getExpression();
    if (sExp.isEmpty())
        return std::vector<sal_Int32>();

    sal_Int32 nInterval = _xGroup->getGroupInterval();

    Reference<XRow> xRow(_xResultSet, UNO_QUERY);
    OUString sRow;
    OUString sCurrentGroupOn;
    std::vector<sal_Int32> vSubGroups = {};
    sal_Int32 i = 0;
    sal_Int32 nRepeater = 0;
    sal_Int32 nRow = 0;
    css::util::Date aDate;
    css::util::Date aCurrentGroupOnDate;
    css::util::DateTime aDateTime;
    css::util::DateTime aCurrentGroupOnDateTime;
    sal_uInt16 nGroupOnMonthOfYear = 0;

    while (_xResultSet->next())
    {
        // If an outer (already-processed) group changed at this row, this
        // group must also break here even if its own value is the same as the previous row's.
        const bool bForceBreak
            = o3tl::make_unsigned(nRow) < _rOuterBoundary.size() && _rOuterBoundary[nRow];

        // Currently the UI will allow the user to GroupOn date values
        // (Month, Day, etc.) if the table column is a time value. The
        // current behavior in this case is to lump all the values into
        // one group - see each date related case statement for details.
        switch (nGroupOn)
        {
            case report::GroupOn::PREFIX_CHARACTERS:
            {
                sRow = xRow->getString(getColumnNum(sExp, xRow)).copy(0, nInterval);
                vSubGroups.push_back(compare(sRow, sCurrentGroupOn, i, _xResultSet, bForceBreak));
                break;
            }
            case report::GroupOn::DEFAULT:
            {
                sRow = xRow->getString(getColumnNum(sExp, xRow));
                vSubGroups.push_back(compare(sRow, sCurrentGroupOn, i, _xResultSet, bForceBreak));
                break;
            }
            // There is an option to set the interval when
            // grouping on YEAR in the UI, the GroupOn::YEAR
            // seems to override the GroupON::PREFIX_CHARACTERS
            case report::GroupOn::YEAR:
            {
                try
                {
                    aDate = xRow->getDate(getColumnNum(sExp, xRow));
                    vSubGroups.push_back(
                        compare(aDate.Year, aCurrentGroupOnDate.Year, i, _xResultSet, bForceBreak));
                }
                catch (const com::sun::star::uno::Exception&)
                {
                    vSubGroups.push_back(0);
                }
                break;
            }
            case report::GroupOn::QUARTAL:
            {
                try
                {
                    aDate = xRow->getDate(getColumnNum(sExp, xRow));
                    vSubGroups.push_back(compareQuartal(aDate.Month, aCurrentGroupOnDate.Month, i,
                                                        _xResultSet, bForceBreak));
                }
                catch (const com::sun::star::uno::Exception&)
                {
                    vSubGroups.push_back(0);
                }
                break;
            }
            case report::GroupOn::MONTH:
            {
                try
                {
                    aDate = xRow->getDate(getColumnNum(sExp, xRow));
                    vSubGroups.push_back(compare(aDate.Month, aCurrentGroupOnDate.Month, i,
                                                 _xResultSet, bForceBreak));
                }
                catch (const com::sun::star::uno::Exception&)
                {
                    vSubGroups.push_back(0);
                }
                break;
            }
            case report::GroupOn::WEEK:
            {
                try
                {
                    aDate = xRow->getDate(getColumnNum(sExp, xRow));
                    Date aD(aDate.Day, aDate.Month, aDate.Year);
                    sal_uInt16 nMonthOfYear = aD.GetWeekOfYear();
                    vSubGroups.push_back(
                        compare(nMonthOfYear, nGroupOnMonthOfYear, i, _xResultSet, bForceBreak));
                }
                catch (const com::sun::star::uno::Exception&)
                {
                    vSubGroups.push_back(0);
                }
                break;
            }
            case report::GroupOn::DAY:
            {
                try
                {
                    aDate = xRow->getDate(getColumnNum(sExp, xRow));
                    vSubGroups.push_back(
                        compare(aDate.Day, aCurrentGroupOnDate.Day, i, _xResultSet, bForceBreak));
                }
                catch (const com::sun::star::uno::Exception&)
                {
                    vSubGroups.push_back(0);
                }
                break;
            }
            case report::GroupOn::HOUR:
            {
                try
                {
                    aDateTime = xRow->getTimestamp(getColumnNum(sExp, xRow));
                    vSubGroups.push_back(compare(aDateTime.Hours, aCurrentGroupOnDateTime.Hours, i,
                                                 _xResultSet, bForceBreak));
                }
                catch (const com::sun::star::uno::Exception&)
                {
                    vSubGroups.push_back(0);
                }
                break;
            }
            case report::GroupOn::MINUTE:
            {
                try
                {
                    aDateTime = xRow->getTimestamp(getColumnNum(sExp, xRow));
                    vSubGroups.push_back(compare(aDateTime.Minutes, aCurrentGroupOnDateTime.Minutes,
                                                 i, _xResultSet, bForceBreak));
                }
                catch (const com::sun::star::uno::Exception&)
                {
                    vSubGroups.push_back(0);
                }
                break;
            }
            case report::GroupOn::INTERVAL:
            {
                if (nRepeater == nInterval || bForceBreak)
                {
                    if (!_xResultSet->isFirst())
                        ++i;
                    nRepeater = 0;
                }
                vSubGroups.push_back(i);
                ++nRepeater;
                // Current default behavior for interval = 0
                // is for the first row to be one group and
                // all the other rows are their own group
                if (nInterval == sal_Int32(0))
                {
                    i = 1;
                    nRepeater = -1;
                }
                break;
            }
        }
        ++nRow;
    }
    return vSubGroups;
}

} // rptxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
