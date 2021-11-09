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

#include <memory>
#include "xmlExport.hxx"
#include "xmlAutoStyle.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/numehelp.hxx>
#include "xmlHelper.hxx"
#include <strings.hxx>
#include "xmlPropertyHandler.hxx"
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/report/GroupOn.hpp>
#include <com/sun/star/report/XFixedText.hpp>
#include <com/sun/star/report/XImageControl.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFunction.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <RptDef.hxx>
#include <vcl/svapp.hxx>
#include <osl/diagnose.h>
#include <tools/color.hxx>

#define DEFAULT_LINE_WIDTH 2

namespace rptxml
{
    using namespace xmloff;
    using namespace comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::xml;


    /** Exports only settings
     * \ingroup reportdesign_source_filter_xml
     *
     */
    extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
    reportdesign_ORptExportHelper_get_implementation(
        css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
    {
        return cppu::acquire(new ORptExport(context,
            "com.sun.star.comp.report.XMLSettingsExporter",
            SvXMLExportFlags::SETTINGS ));
    }

    /** Exports only content
     * \ingroup reportdesign_source_filter_xml
     *
     */
    extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
    reportdesign_ORptContentExportHelper_get_implementation(
        css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
    {
        return cppu::acquire(new ORptExport(context,
            "com.sun.star.comp.report.XMLContentExporter",
            SvXMLExportFlags::CONTENT ));
    }

    /** Exports only styles
     * \ingroup reportdesign_source_filter_xml
     *
     */
    extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
    reportdesign_ORptStylesExportHelper_get_implementation(
        css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
    {
        return cppu::acquire(new ORptExport(context,
            "com.sun.star.comp.report.XMLStylesExporter",
            SvXMLExportFlags::STYLES | SvXMLExportFlags::MASTERSTYLES | SvXMLExportFlags::AUTOSTYLES |
                SvXMLExportFlags::FONTDECLS|SvXMLExportFlags::OASIS ));
    }

    /** Exports only meta data
     * \ingroup reportdesign_source_filter_xml
     *
     */
    extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
    reportdesign_ORptMetaExportHelper_get_implementation(
        css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
    {
        return cppu::acquire(new ORptExport(context,
            "com.sun.star.comp.report.XMLMetaExporter",
            SvXMLExportFlags::META ));
    }

    /** Exports all
     * \ingroup reportdesign_source_filter_xml
     *
     */
    extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
    reportdesign_ODBFullExportHelper_get_implementation(
        css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
    {
        return cppu::acquire(new ORptExport(context,
            "com.sun.star.comp.report.XMLFullExporter",
            SvXMLExportFlags::ALL));
    }

    namespace {

    class OSpecialHandleXMLExportPropertyMapper : public SvXMLExportPropertyMapper
    {
    public:
        explicit OSpecialHandleXMLExportPropertyMapper(const rtl::Reference< XMLPropertySetMapper >& rMapper) : SvXMLExportPropertyMapper(rMapper )
        {
        }
        /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
        virtual void handleSpecialItem(
                SvXMLAttributeList& /*rAttrList*/,
                const XMLPropertyState& /*rProperty*/,
                const SvXMLUnitConverter& /*rUnitConverter*/,
                const SvXMLNamespaceMap& /*rNamespaceMap*/,
                const ::std::vector< XMLPropertyState >* /*pProperties*/ = nullptr,
                sal_uInt32 /*nIdx*/ = 0 ) const override
        {
            // nothing to do here
        }
    };

    }

static void lcl_adjustColumnSpanOverRows(ORptExport::TSectionsGrid& _rGrid)
{
    for (auto& rEntry : _rGrid)
    {
        ORptExport::TGrid::iterator aRowIter = rEntry.second.begin();
        ORptExport::TGrid::const_iterator aRowEnd = rEntry.second.end();
        for (; aRowIter != aRowEnd; ++aRowIter)
        {
            if ( aRowIter->first )
            {
                sal_Int32 nColIndex = 0;
                for (const auto& rCell : aRowIter->second)
                {
                    if ( rCell.nRowSpan > 1 )
                    {
                        sal_Int32 nColSpan = rCell.nColSpan;
                        for (sal_Int32 i = 1; i < rCell.nRowSpan; ++i)
                        {
                            (aRowIter+i)->second[nColIndex].nColSpan = nColSpan;
                        }
                    }
                    ++nColIndex;
                }
            }
        }
    }
}

ORptExport::ORptExport(const Reference< XComponentContext >& _rxContext, OUString const & implementationName, SvXMLExportFlags nExportFlag)
: SvXMLExport( _rxContext, implementationName, util::MeasureUnit::MM_100TH, XML_REPORT, SvXMLExportFlags::OASIS)
,m_bAllreadyFilled(false)
{
    setExportFlags( SvXMLExportFlags::OASIS | nExportFlag);
    GetMM100UnitConverter().SetCoreMeasureUnit(css::util::MeasureUnit::MM_100TH);
    GetMM100UnitConverter().SetXMLMeasureUnit(css::util::MeasureUnit::CM);

    // (getExportFlags() & EXPORT_CONTENT) != 0 ? : XML_N_OOO
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE ), XML_NAMESPACE_OFFICE );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );

    GetNamespaceMap_().Add( GetXMLToken(XML_NP_RPT), GetXMLToken(XML_N_RPT), XML_NAMESPACE_REPORT );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG_COMPAT),  XML_NAMESPACE_SVG );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_FORM), GetXMLToken(XML_N_FORM), XML_NAMESPACE_FORM );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_TEXT), GetXMLToken(XML_N_TEXT), XML_NAMESPACE_TEXT );


    if( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::FONTDECLS) )
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO_COMPAT), XML_NAMESPACE_FO );

    if( getExportFlags() & (SvXMLExportFlags::META|SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::SCRIPTS|SvXMLExportFlags::SETTINGS) )
    {
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    }
    if( getExportFlags() & SvXMLExportFlags::SETTINGS )
    {
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_CONFIG), GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
    }

    if( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::FONTDECLS) )
    {
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
    }
    // RDFa: needed for content and header/footer styles
    if( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_XHTML),GetXMLToken(XML_N_XHTML), XML_NAMESPACE_XHTML );
        // loext, needed for paragraphs inside shapes
        if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
        {
            GetNamespaceMap_().Add(
                GetXMLToken(XML_NP_LO_EXT), GetXMLToken(XML_N_LO_EXT),
                XML_NAMESPACE_LO_EXT);
        }
    }
    // GRDDL: to convert RDFa and meta.xml to RDF
    if( getExportFlags() & (SvXMLExportFlags::META|SvXMLExportFlags::STYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_GRDDL),GetXMLToken(XML_N_GRDDL), XML_NAMESPACE_GRDDL );
    }

    GetNamespaceMap_().Add( GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_NUMBER), GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );

    m_sTableStyle = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_STYLE_NAME) );
    m_sCellStyle = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_REPORT, GetXMLToken(XML_STYLE_NAME) );


    m_xPropHdlFactory = new OXMLRptPropHdlFactory();
    rtl::Reference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
    rtl::Reference < XMLPropertySetMapper > xTableStylesPropertySetMapper1 = new XMLPropertySetMapper(OXMLHelper::GetTableStyleProps(),xFac, true);
    rtl::Reference < XMLPropertySetMapper > xTableStylesPropertySetMapper2 = new XMLTextPropertySetMapper(TextPropMap::TABLE_DEFAULTS, true );
    xTableStylesPropertySetMapper1->AddMapperEntry(xTableStylesPropertySetMapper2);

    m_xTableStylesExportPropertySetMapper = new SvXMLExportPropertyMapper(xTableStylesPropertySetMapper1);

    m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylePropertyMap( false, true);
    m_xCellStylesExportPropertySetMapper = new OSpecialHandleXMLExportPropertyMapper(m_xCellStylesPropertySetMapper);
    m_xCellStylesExportPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(*this));

    rtl::Reference < XMLPropertySetMapper > xColumnStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetColumnStyleProps(), m_xPropHdlFactory, true);
    m_xColumnStylesExportPropertySetMapper = new OSpecialHandleXMLExportPropertyMapper(xColumnStylesPropertySetMapper);

    rtl::Reference < XMLPropertySetMapper > xRowStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetRowStyleProps(), m_xPropHdlFactory, true);
    m_xRowStylesExportPropertySetMapper = new OSpecialHandleXMLExportPropertyMapper(xRowStylesPropertySetMapper);

    rtl::Reference < XMLPropertySetMapper > xPropMapper(new XMLTextPropertySetMapper( TextPropMap::PARA, true ));
    m_xParaPropMapper = new OSpecialHandleXMLExportPropertyMapper( xPropMapper);

    const OUString& sFamily( GetXMLToken(XML_PARAGRAPH) );
    GetAutoStylePool()->AddFamily( XmlStyleFamily::TEXT_PARAGRAPH, sFamily,
                              m_xParaPropMapper, "P" );

    GetAutoStylePool()->AddFamily(XmlStyleFamily::TABLE_CELL, XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME,
        m_xCellStylesExportPropertySetMapper, XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX);
    GetAutoStylePool()->AddFamily(XmlStyleFamily::TABLE_COLUMN, XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME,
        m_xColumnStylesExportPropertySetMapper, XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX);
    GetAutoStylePool()->AddFamily(XmlStyleFamily::TABLE_ROW, XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME,
        m_xRowStylesExportPropertySetMapper, XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX);
    GetAutoStylePool()->AddFamily(XmlStyleFamily::TABLE_TABLE, XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME,
        m_xTableStylesExportPropertySetMapper, XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_ORptExport_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ORptExport(context,
        "com.sun.star.comp.report.ExportFilter",
        SvXMLExportFlags::CONTENT | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::FONTDECLS));
}


void ORptExport::exportFunctions(const Reference<XIndexAccess>& _xFunctions)
{
    const sal_Int32 nCount = _xFunctions->getCount();
    for (sal_Int32 i = 0; i< nCount; ++i)
    {
        uno::Reference< report::XFunction> xFunction(_xFunctions->getByIndex(i),uno::UNO_QUERY_THROW);
        exportFunction(xFunction);
    }
}

void ORptExport::exportFunction(const uno::Reference< XFunction>& _xFunction)
{
    exportFormula(XML_FORMULA,_xFunction->getFormula());
    beans::Optional< OUString> aInitial = _xFunction->getInitialFormula();
    if ( aInitial.IsPresent && !aInitial.Value.isEmpty() )
        exportFormula(XML_INITIAL_FORMULA ,aInitial.Value );
    AddAttribute( XML_NAMESPACE_REPORT, XML_NAME , _xFunction->getName() );
    if ( _xFunction->getPreEvaluated() )
        AddAttribute( XML_NAMESPACE_REPORT, XML_PRE_EVALUATED , XML_TRUE );
    if ( _xFunction->getDeepTraversing() )
        AddAttribute( XML_NAMESPACE_REPORT, XML_DEEP_TRAVERSING , XML_TRUE );

    SvXMLElementExport aFunction(*this,XML_NAMESPACE_REPORT, XML_FUNCTION, true, true);
}

void ORptExport::exportMasterDetailFields(const Reference<XReportComponent>& _xReportComponent)
{
    const uno::Sequence< OUString> aMasterFields = _xReportComponent->getMasterFields();
    if ( !aMasterFields.hasElements() )
        return;

    SvXMLElementExport aElement(*this,XML_NAMESPACE_REPORT, XML_MASTER_DETAIL_FIELDS, true, true);
    const uno::Sequence< OUString> aDetailFields = _xReportComponent->getDetailFields();

    OSL_ENSURE(aDetailFields.getLength() == aMasterFields.getLength(),"not equal length for master and detail fields!");

    const OUString* pDetailFieldsIter = aDetailFields.getConstArray();
    for(const OUString& rMasterField : aMasterFields)
    {
        AddAttribute( XML_NAMESPACE_REPORT, XML_MASTER , rMasterField );
        if ( !pDetailFieldsIter->isEmpty() )
            AddAttribute( XML_NAMESPACE_REPORT, XML_DETAIL , *pDetailFieldsIter );
        SvXMLElementExport aPair(*this,XML_NAMESPACE_REPORT, XML_MASTER_DETAIL_FIELD, true, true);
        ++pDetailFieldsIter;
    }
}

void ORptExport::exportReport(const Reference<XReportDefinition>& _xReportDefinition)
{
    if ( !_xReportDefinition.is() )
        return;

    exportFunctions(_xReportDefinition->getFunctions());
    exportGroupsExpressionAsFunction(_xReportDefinition->getGroups());

    if ( _xReportDefinition->getReportHeaderOn() )
    {
        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_REPORT_HEADER, true, true);
        exportSection(_xReportDefinition->getReportHeader());
    }
    if ( _xReportDefinition->getPageHeaderOn() )
    {
        OUStringBuffer sValue;
        sal_Int16 nRet = _xReportDefinition->getPageHeaderOption();
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
        if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
            AddAttribute(XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION,sValue.makeStringAndClear());

        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_PAGE_HEADER, true, true);
        exportSection(_xReportDefinition->getPageHeader(),true);
    }

    exportGroup(_xReportDefinition,0);

    if ( _xReportDefinition->getPageFooterOn() )
    {
        OUStringBuffer sValue;
        sal_Int16 nRet = _xReportDefinition->getPageFooterOption();
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
        if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
            AddAttribute(XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION,sValue.makeStringAndClear());
        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_PAGE_FOOTER, true, true);
        exportSection(_xReportDefinition->getPageFooter(),true);
    }
    if ( _xReportDefinition->getReportFooterOn() )
    {
        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_REPORT_FOOTER, true, true);
        exportSection(_xReportDefinition->getReportFooter());
    }
}

void ORptExport::exportComponent(const Reference<XReportComponent>& _xReportComponent)
{
    OSL_ENSURE(_xReportComponent.is(),"No component interface!");
    if ( !_xReportComponent.is() )
        return;

    AddAttribute(XML_NAMESPACE_DRAW, XML_NAME,_xReportComponent->getName());

    SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_REPORT_COMPONENT, false, false);
}

void ORptExport::exportFormatConditions(const Reference<XReportControlModel>& _xReportElement)
{
    OSL_ENSURE(_xReportElement.is(),"_xReportElement is NULL -> GPF");
    const sal_Int32 nCount = _xReportElement->getCount();
    try
    {
        for (sal_Int32 i = 0; i < nCount ; ++i)
        {
            uno::Reference< report::XFormatCondition > xCond(_xReportElement->getByIndex(i),uno::UNO_QUERY);
            if ( !xCond->getEnabled() )
                AddAttribute(XML_NAMESPACE_REPORT, XML_ENABLED,XML_FALSE);

            AddAttribute(XML_NAMESPACE_REPORT, XML_FORMULA,xCond->getFormula());

            exportStyleName(xCond.get(),GetAttrList(),m_sCellStyle);
            SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_FORMAT_CONDITION, true, true);
        }
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Can not access format condition!");
    }
}

void ORptExport::exportReportElement(const Reference<XReportControlModel>& _xReportElement)
{
    OSL_ENSURE(_xReportElement.is(),"_xReportElement is NULL -> GPF");
    if ( !_xReportElement->getPrintWhenGroupChange() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_PRINT_WHEN_GROUP_CHANGE, XML_FALSE );

    if ( !_xReportElement->getPrintRepeatedValues() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_PRINT_REPEATED_VALUES,XML_FALSE);

    SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_REPORT_ELEMENT, true, true);
    if ( _xReportElement->getCount() )
    {
        exportFormatConditions(_xReportElement);
    }

    OUString sExpr = _xReportElement->getConditionalPrintExpression();
    if ( !sExpr.isEmpty() )
    {
        exportFormula(XML_FORMULA,sExpr);
        SvXMLElementExport aPrintExpr(*this,XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION, true, true);
    }

    // only export when parent exists
    uno::Reference< report::XSection> xParent(_xReportElement->getParent(),uno::UNO_QUERY);
    if ( xParent.is() )
        exportComponent(_xReportElement);
}

static void lcl_calculate(const ::std::vector<sal_Int32>& _aPosX,const ::std::vector<sal_Int32>& _aPosY,ORptExport::TGrid& _rColumns)
{
    sal_Int32 nCountX = _aPosX.size() - 1;
    sal_Int32 nCountY = _aPosY.size() - 1;
    for (sal_Int32 j = 0; j < nCountY; ++j)
    {
        sal_Int32 nHeight = _aPosY[j+1] - _aPosY[j];
        if ( nHeight )
            for (sal_Int32 i = 0; i < nCountX ; ++i)
            {
                _rColumns[j].second[i] = ORptExport::TCell(1,1);
                _rColumns[j].second[i].bSet = true;
            }
    }
}

void ORptExport::collectStyleNames(XmlStyleFamily _nFamily,const ::std::vector< sal_Int32>& _aSize, std::vector<OUString>& _rStyleNames)
{
    ::std::vector<sal_Int32>::const_iterator aIter = _aSize.begin();
    ::std::vector<sal_Int32>::const_iterator aIter2 = aIter + 1;
    ::std::vector<sal_Int32>::const_iterator aEnd = _aSize.end();
    for (;aIter2 != aEnd ; ++aIter,++aIter2)
    {
        ::std::vector< XMLPropertyState > aPropertyStates(1, 0);
        sal_Int32 nValue = static_cast<sal_Int32>(*aIter2 - *aIter);
        aPropertyStates[0].maValue <<= nValue;
        _rStyleNames.push_back(GetAutoStylePool()->Add(_nFamily, std::move(aPropertyStates) ));
    }
}

void ORptExport::collectStyleNames(XmlStyleFamily _nFamily, const ::std::vector< sal_Int32>& _aSize, const ::std::vector< sal_Int32>& _aSizeAutoGrow, std::vector<OUString>& _rStyleNames)
{
    ::std::vector<sal_Int32>::const_iterator aIter = _aSize.begin();
    ::std::vector<sal_Int32>::const_iterator aIter2 = aIter + 1;
    ::std::vector<sal_Int32>::const_iterator aEnd = _aSize.end();
    for (;aIter2 != aEnd; ++aIter, ++aIter2)
    {
        ::std::vector< XMLPropertyState > aPropertyStates(1, 0);
        sal_Int32 nValue = static_cast<sal_Int32>(*aIter2 - *aIter);
        aPropertyStates[0].maValue <<= nValue;
        // note: there cannot be 0-height rows, because a call to std::unique has removed them
        // it cannot be predicted that the size of _aSizeAutoGrow has any relation to the size of
        // _aSize, because of the same std::unique operation (and _aSizeAutoGrow wasn't even the same
        // size before that), so the matching element in _aSizeAutoGrow has to be found by lookup.
        ::std::vector<sal_Int32>::const_iterator aAutoGrow = ::std::find(_aSizeAutoGrow.begin(), _aSizeAutoGrow.end(), *aIter2);
        bool bAutoGrow = aAutoGrow != _aSizeAutoGrow.end();
        // the mnIndex is into the array returned by OXMLHelper::GetRowStyleProps()
        aPropertyStates[0].mnIndex = bAutoGrow ? 1 : 0;
        _rStyleNames.push_back(GetAutoStylePool()->Add(_nFamily, std::move(aPropertyStates)));
    }
}

void ORptExport::exportSectionAutoStyle(const Reference<XSection>& _xProp)
{
    OSL_ENSURE(_xProp != nullptr,"Section is NULL -> GPF");
    exportAutoStyle(_xProp);

    Reference<XReportDefinition> xReport = _xProp->getReportDefinition();
    const awt::Size aSize   = rptui::getStyleProperty<awt::Size>(xReport,PROPERTY_PAPERSIZE);
    const sal_Int32 nOffset = rptui::getStyleProperty<sal_Int32>(xReport,PROPERTY_LEFTMARGIN);
    const sal_Int32 nCount  = _xProp->getCount();

    ::std::vector<sal_Int32> aColumnPos;
    aColumnPos.reserve(2*(nCount + 1));
    aColumnPos.push_back(nOffset);
    aColumnPos.push_back(aSize.Width - rptui::getStyleProperty<sal_Int32>(xReport,PROPERTY_RIGHTMARGIN));

    ::std::vector<sal_Int32> aRowPos;
    aRowPos.reserve(2*(nCount + 1));
    aRowPos.push_back(0);
    aRowPos.push_back(_xProp->getHeight());


    ::std::vector<sal_Int32> aRowPosAutoGrow;
    aRowPosAutoGrow.reserve(2 * (nCount + 1));


    sal_Int32 i;
    for (i = 0 ; i< nCount ; ++i)
    {
        Reference<XReportComponent> xReportElement(_xProp->getByIndex(i),uno::UNO_QUERY);
        uno::Reference< XShape> xShape(xReportElement,uno::UNO_QUERY);
        if ( xShape.is() )
            continue;
        OSL_ENSURE( xReportElement.is(),"NULL Element in Section!" );
        if ( !xReportElement.is() )
            continue;
        sal_Int32 nX = xReportElement->getPositionX();
        aColumnPos.push_back(nX);
        Reference<XFixedLine> xFixedLine(xReportElement,uno::UNO_QUERY);
        if ( xFixedLine.is() && xFixedLine->getOrientation() == 1 ) // vertical
        {
            sal_Int32 nWidth = static_cast<sal_Int32>(xReportElement->getWidth()*0.5);
            nX += nWidth;
            aColumnPos.push_back(nX);
            nX += xReportElement->getWidth() - nWidth;
        }
        else
            nX += xReportElement->getWidth();
        aColumnPos.push_back(nX); // --nX why?

        sal_Int32 nY = xReportElement->getPositionY();
        aRowPos.push_back(nY);
        nY += xReportElement->getHeight();
        aRowPos.push_back(nY); // --nY why?
        bool bAutoGrow = xReportElement->getAutoGrow();
        if (bAutoGrow)
        {
            // the resulting table row ending at nY should auto-grow
            aRowPosAutoGrow.push_back(nY);
        }
    }

    ::std::sort(aColumnPos.begin(),aColumnPos.end(),::std::less<sal_Int32>());
    aColumnPos.erase(::std::unique(aColumnPos.begin(),aColumnPos.end()),aColumnPos.end());

    // note: the aRowPos contains top and bottom position of every report control; we now compute the
    // top of every row in the resulting table, by sorting and eliminating unnecessary duplicate
    // positions. (the same for the columns in the preceding lines.)
    ::std::sort(aRowPos.begin(),aRowPos.end(),::std::less<sal_Int32>());
    aRowPos.erase(::std::unique(aRowPos.begin(),aRowPos.end()),aRowPos.end());

    TSectionsGrid::iterator aInsert = m_aSectionsGrid.emplace(
                                    _xProp.get(),
                                    TGrid(aRowPos.size() - 1,TGrid::value_type(false,TRow(aColumnPos.size() - 1)))
        ).first;
    lcl_calculate(aColumnPos,aRowPos,aInsert->second);

    TGridStyleMap::iterator aPos = m_aColumnStyleNames.emplace(_xProp.get(),std::vector<OUString>()).first;
    collectStyleNames(XmlStyleFamily::TABLE_COLUMN,aColumnPos,aPos->second);
    aPos = m_aRowStyleNames.emplace(_xProp.get(),std::vector<OUString>()).first;
    collectStyleNames(XmlStyleFamily::TABLE_ROW, aRowPos, aRowPosAutoGrow, aPos->second);

    sal_Int32 x1 = 0;
    sal_Int32 y1 = 0;
    sal_Int32 x2 = 0;
    sal_Int32 y2 = 0;
    sal_Int32 xi = 0;
    sal_Int32 yi = 0;
    bool isOverlap = false;

    for (i = 0 ; i< nCount ; ++i)
    {
        Reference<XReportComponent> xReportElement(_xProp->getByIndex(i),uno::UNO_QUERY);
        uno::Reference< XShape> xShape(xReportElement,uno::UNO_QUERY);
        if ( xShape.is() )
            continue;
        sal_Int32 nPos = xReportElement->getPositionX();
        x1 = (::std::find(aColumnPos.begin(),aColumnPos.end(),nPos) - aColumnPos.begin());
        Reference<XFixedLine> xFixedLine(xReportElement,uno::UNO_QUERY);
        if ( xFixedLine.is() && xFixedLine->getOrientation() == 1 ) // vertical
            nPos += static_cast<sal_Int32>(xReportElement->getWidth()*0.5);
        else
            nPos += xReportElement->getWidth(); // -1 why
        x2 = (::std::find(aColumnPos.begin(),aColumnPos.end(),nPos) - aColumnPos.begin());

        nPos = xReportElement->getPositionY();
        y1 = (::std::find(aRowPos.begin(),aRowPos.end(),nPos) - aRowPos.begin());
        nPos += xReportElement->getHeight(); // -1 why?
        y2 = (::std::find(aRowPos.begin(),aRowPos.end(),nPos) - aRowPos.begin());

        isOverlap = false;
        yi = y1;
        while(yi < y2 && !isOverlap) // find overlapping controls
        {
            xi = x1;
            while(xi < x2 && !isOverlap)
            {
                if ( aInsert->second[yi].second[xi].xElement.is() )
                {
                    isOverlap = true;
                }
                ++xi;
            }
            ++yi;
        }

        if (!isOverlap)
        {
            yi = y1;
            while(yi < y2)
            {
                xi = x1;
                while(xi < x2)
                {
                    aInsert->second[yi].second[xi] = TCell();
                    ++xi;
                }
                aInsert->second[yi].first = true;
                ++yi;
            }

            if (x2 - x1 != 0 && y2 - y1 != 0)
            {
                sal_Int32 nColSpan = x2 - x1;
                sal_Int32 nRowSpan = y2 - y1;
                aInsert->second[y1].second[x1] =
                    TCell(
                        nColSpan,
                        nRowSpan,
                        xReportElement
                        );
            }
        }
    }

    lcl_adjustColumnSpanOverRows(m_aSectionsGrid);
    exportReportComponentAutoStyles(_xProp);
}

void ORptExport::exportReportComponentAutoStyles(const Reference<XSection>& _xProp)
{
    const sal_Int32 nCount = _xProp->getCount();
    for (sal_Int32 i = 0 ; i< nCount ; ++i)
    {
        const Reference<XReportComponent> xReportElement(_xProp->getByIndex(i),uno::UNO_QUERY);
        const Reference< report::XShape > xShape(xReportElement,uno::UNO_QUERY);
        if ( xShape.is() )
        {
            rtl::Reference< XMLShapeExport > xShapeExport = GetShapeExport();
            xShapeExport->seekShapes(_xProp);
            SolarMutexGuard aGuard;
            xShapeExport->collectShapeAutoStyles(xShape);
        }
        else
        {
            exportAutoStyle(xReportElement.get());

            Reference<XFormattedField> xFormattedField(xReportElement,uno::UNO_QUERY);
            if ( xFormattedField.is() )
            {
                try
                {
                    const sal_Int32 nFormatCount = xFormattedField->getCount();
                    for (sal_Int32 j = 0; j < nFormatCount ; ++j)
                    {
                        uno::Reference< report::XFormatCondition > xCond(xFormattedField->getByIndex(j),uno::UNO_QUERY);
                        exportAutoStyle(xCond.get(),xFormattedField);
                    }
                }
                catch(uno::Exception&)
                {
                    OSL_FAIL("Can not access format condition!");
                }
            }
        }
    }
}

void ORptExport::exportSection(const Reference<XSection>& _xSection,bool bHeader)
{
    OSL_ENSURE(_xSection.is(),"Section is NULL -> GPF");
    AddAttribute(XML_NAMESPACE_TABLE, XML_NAME,_xSection->getName());

    if ( !_xSection->getVisible() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_VISIBLE,XML_FALSE);

    if ( !bHeader )
    {
        OUStringBuffer sValue;
        sal_Int16 nRet = _xSection->getForceNewPage();
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetForceNewPageOptions();
        if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
            AddAttribute(XML_NAMESPACE_REPORT, XML_FORCE_NEW_PAGE,sValue.makeStringAndClear());

        nRet = _xSection->getNewRowOrCol();
        if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
            AddAttribute(XML_NAMESPACE_REPORT, XML_FORCE_NEW_COLUMN,sValue.makeStringAndClear());
        if ( _xSection->getKeepTogether() )
            AddAttribute(XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER, XML_TRUE );
    }

    exportStyleName(_xSection.get(),GetAttrList(),m_sTableStyle);

    /// TODO export as table layout
    SvXMLElementExport aComponents(*this,XML_NAMESPACE_TABLE, XML_TABLE, true, true);

    OUString sExpr = _xSection->getConditionalPrintExpression();
    if ( !sExpr.isEmpty() )
    {
        exportFormula(XML_FORMULA,sExpr);
        SvXMLElementExport aPrintExpr(*this,XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION, true, false);
    }

    exportContainer(_xSection);
}

void ORptExport::exportTableColumns(const Reference< XSection>& _xSection)
{
    SvXMLElementExport aColumns(*this,XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS, true, true);
    TGridStyleMap::const_iterator aColFind = m_aColumnStyleNames.find(_xSection);
    OSL_ENSURE(aColFind != m_aColumnStyleNames.end(),"ORptExport::exportTableColumns: Section not found in m_aColumnStyleNames!");
    if ( aColFind == m_aColumnStyleNames.end() )
        return;

    for (auto& aCol : aColFind->second)
    {
        AddAttribute(m_sTableStyle, aCol);
        SvXMLElementExport aColumn(*this,XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, true, true);
    }
}

void ORptExport::exportContainer(const Reference< XSection>& _xSection)
{
    OSL_ENSURE(_xSection.is(),"Section is NULL -> GPF");

    exportTableColumns(_xSection);

    TSectionsGrid::const_iterator aFind = m_aSectionsGrid.find(_xSection);
    OSL_ENSURE(aFind != m_aSectionsGrid.end(),"ORptExport::exportContainer: Section not found in grid!");
    if ( aFind == m_aSectionsGrid.end() )
        return;
    TGrid::const_iterator aRowIter = aFind->second.begin();
    TGrid::const_iterator aRowEnd = aFind->second.end();

    TGridStyleMap::const_iterator aRowFind = m_aRowStyleNames.find(_xSection);
    auto aHeightIter = aRowFind->second.cbegin();
    OSL_ENSURE(aRowFind->second.size() == aFind->second.size(),"Different count for rows");

    bool bShapeHandled = false;
    ::std::map<sal_Int32,sal_Int32> aRowSpan;
    for (sal_Int32 j = 0; aRowIter != aRowEnd; ++aRowIter,++j,++aHeightIter)
    {
        AddAttribute( m_sTableStyle,*aHeightIter );
        SvXMLElementExport aRow(*this,XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true);
        if ( aRowIter->first )
        {
            ::std::vector< TCell >::const_iterator aColIter = aRowIter->second.begin();
            ::std::vector< TCell >::const_iterator aColEnd = aRowIter->second.end();
            sal_Int32 nEmptyCellColSpan = 0;
            for (; aColIter != aColEnd; ++aColIter)
            {
                bool bCoveredCell = false;
                sal_Int32 nColSpan = 0;
                sal_Int32 nColIndex = aColIter - aRowIter->second.begin();
                ::std::map<sal_Int32,sal_Int32>::iterator aRowSpanFind = aRowSpan.find(nColIndex);
                if ( aRowSpanFind != aRowSpan.end() )
                {
                    nColSpan = 1;
                    if ( !--(aRowSpanFind->second) )
                        aRowSpan.erase(aRowSpanFind);

                    if ( aColIter->nColSpan > 1 )
                        nColSpan += aColIter->nColSpan - 1;

                    bCoveredCell = true;
                    aColIter = aColIter + (aColIter->nColSpan - 1);
                }
                else if ( aColIter->bSet )
                {
                    if ( nEmptyCellColSpan > 0 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED, OUString::number(nEmptyCellColSpan) );
                        bCoveredCell = true;
                        nColSpan = nEmptyCellColSpan - 1;
                        nEmptyCellColSpan = 0;
                    }
                    sal_Int32 nSpan = aColIter->nColSpan;
                    if ( nSpan > 1 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED, OUString::number(nSpan) );
                        nColSpan = nSpan - 1;
                        bCoveredCell = true;
                    }
                    nSpan = aColIter->nRowSpan;
                    if ( nSpan > 1 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_ROWS_SPANNED, OUString::number(nSpan) );
                        aRowSpan[nColIndex] = nSpan - 1;
                    }
                    if ( aColIter->xElement.is() )
                        exportStyleName(aColIter->xElement.get(),GetAttrList(),m_sTableStyle);

                    // start <table:table-cell>
                    Reference<XFormattedField> xFormattedField(aColIter->xElement,uno::UNO_QUERY);
                    if ( xFormattedField.is() )
                    {
                        sal_Int32 nFormatKey = xFormattedField->getFormatKey();
                        XMLNumberFormatAttributesExportHelper aHelper(GetNumberFormatsSupplier(),*this);
                        bool bIsStandard = false;
                        sal_Int16 nCellType = aHelper.GetCellType(nFormatKey,bIsStandard);
                        // "Standard" means "no format set, value could be anything",
                        // so don't set a format attribute in this case.
                        // P.S.: "Standard" is called "General" in some languages
                        if (!bIsStandard)
                        {
                            if ( nCellType == util::NumberFormat::TEXT )
                                aHelper.SetNumberFormatAttributes("", u"");
                            else
                                aHelper.SetNumberFormatAttributes(nFormatKey, 0.0, false);
                        }
                    }
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, false);

                    if ( aColIter->xElement.is() )
                    {
                        // start <text:p>
                        SvXMLElementExport aParagraphContent(*this,XML_NAMESPACE_TEXT, XML_P, true, false);
                        Reference<XServiceInfo> xElement(aColIter->xElement,uno::UNO_QUERY);

                        if ( !bShapeHandled )
                        {
                            bShapeHandled = true;
                            exportShapes(_xSection,false);
                        }
                        uno::Reference< XShape > xShape(xElement,uno::UNO_QUERY);
                        uno::Reference< XFixedLine > xFixedLine(xElement,uno::UNO_QUERY);
                        if ( !xShape.is() && !xFixedLine.is() )
                        {
                            Reference<XReportControlModel> xReportElement(xElement,uno::UNO_QUERY);
                            Reference<XReportDefinition> xReportDefinition(xElement,uno::UNO_QUERY);
                            Reference< XImageControl > xImage(xElement,uno::UNO_QUERY);
                            Reference<XSection> xSection(xElement,uno::UNO_QUERY);

                            XMLTokenEnum eToken = XML_SECTION;
                            bool bExportData = false;
                            if ( xElement->supportsService(SERVICE_FIXEDTEXT) )
                            {
                                eToken = XML_FIXED_CONTENT;
                            }
                            else if ( xElement->supportsService(SERVICE_FORMATTEDFIELD) )
                            {
                                eToken = XML_FORMATTED_TEXT;
                                bExportData = true;
                            }
                            else if ( xElement->supportsService(SERVICE_IMAGECONTROL) )
                            {
                                eToken = XML_IMAGE;
                                OUString sTargetLocation = xImage->getImageURL();
                                if ( !sTargetLocation.isEmpty() )
                                {
                                    sTargetLocation = GetRelativeReference(sTargetLocation);
                                    AddAttribute(XML_NAMESPACE_FORM, XML_IMAGE_DATA,sTargetLocation);
                                }
                                bExportData = true;
                                OUStringBuffer sValue;
                                const SvXMLEnumMapEntry<sal_Int16>* aXML_ImageScaleEnumMap = OXMLHelper::GetImageScaleOptions();
                                if ( SvXMLUnitConverter::convertEnum( sValue, xImage->getScaleMode(),aXML_ImageScaleEnumMap ) )
                                    AddAttribute(XML_NAMESPACE_REPORT, XML_SCALE, sValue.makeStringAndClear() );
                            }
                            else if ( xReportDefinition.is() )
                            {
                                eToken = XML_SUB_DOCUMENT;
                            }

                            if ( bExportData )
                            {
                                const bool bPageSet = exportFormula(XML_FORMULA,xReportElement->getDataField());
                                if ( bPageSet )
                                    eToken = XML_FIXED_CONTENT;
                                else if ( eToken == XML_IMAGE )
                                    AddAttribute(XML_NAMESPACE_REPORT, XML_PRESERVE_IRI, xImage->getPreserveIRI() ? XML_TRUE : XML_FALSE );
                            }

                            {
                                // start <report:eToken>
                                SvXMLElementExport aComponents(*this,XML_NAMESPACE_REPORT, eToken, false, false);
                                if ( eToken == XML_FIXED_CONTENT )
                                    exportParagraph(xReportElement);
                                if ( xReportElement.is() )
                                    exportReportElement(xReportElement);

                                if (eToken == XML_SUB_DOCUMENT && xReportDefinition.is())
                                {
                                    SvXMLElementExport aOfficeElement( *this, XML_NAMESPACE_OFFICE, XML_BODY, true, true );
                                    SvXMLElementExport aElem( *this, true,
                                                            XML_NAMESPACE_OFFICE, XML_REPORT,
                                                              true, true );

                                    exportReportAttributes(xReportDefinition);
                                    exportReport(xReportDefinition);
                                }
                                else if ( xSection.is() )
                                    exportSection(xSection);
                            }
                        }
                    }
                    else if ( !bShapeHandled )
                    {
                        bShapeHandled = true;
                        exportShapes(_xSection);
                    }
                    aColIter = aColIter + (aColIter->nColSpan - 1);
                }
                else
                    ++nEmptyCellColSpan;
                if ( bCoveredCell )
                {
                    for (sal_Int32 k = 0; k < nColSpan; ++k)
                    {
                        SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, true, true);
                    }

                }
            }
            if ( nEmptyCellColSpan )
            {
                {
                    AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED, OUString::number(nEmptyCellColSpan) );
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                    if ( !bShapeHandled )
                    {
                        bShapeHandled = true;
                        exportShapes(_xSection);
                    }
                }
                for (sal_Int32 k = 0; k < nEmptyCellColSpan; ++k)
                {
                    SvXMLElementExport aCoveredCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, true, true);
                }
            }
        }
        else
        { // empty rows
            sal_Int32 nEmptyCellColSpan = aRowIter->second.size();
            if ( nEmptyCellColSpan )
            {
                {
                    AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED, OUString::number(nEmptyCellColSpan) );
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                    if ( !bShapeHandled )
                    {
                        bShapeHandled = true;
                        exportShapes(_xSection);
                    }
                }
                for (sal_Int32 k = 1; k < nEmptyCellColSpan; ++k)
                {
                    SvXMLElementExport aCoveredCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, true, true);
                }
            }
        }
    }
}

OUString ORptExport::convertFormula(const OUString& _sFormula)
{
    OUString sFormula = _sFormula;
    if ( _sFormula == "rpt:" )
        sFormula.clear();
    return sFormula;
}

bool ORptExport::exportFormula(enum ::xmloff::token::XMLTokenEnum eName,const OUString& _sFormula)
{
    const OUString sFieldData = convertFormula(_sFormula);
    sal_Int32 nPageNumberIndex = sFieldData.indexOf("PageNumber()");
    sal_Int32 nPageCountIndex = sFieldData.indexOf("PageCount()");
    bool bRet = nPageNumberIndex != -1 || nPageCountIndex != -1;
    if ( !bRet )
        AddAttribute(XML_NAMESPACE_REPORT, eName,sFieldData);

    return bRet;
}

void ORptExport::exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt,const OUString& _sName)
{
    Reference<XPropertySet> xFind(_xProp);
    TPropertyStyleMap::const_iterator aFind = m_aAutoStyleNames.find(xFind);
    if ( aFind != m_aAutoStyleNames.end() )
    {
        _rAtt.AddAttribute( _sName,
                            aFind->second );
        m_aAutoStyleNames.erase(aFind);
    }
}

void ORptExport::exportGroup(const Reference<XReportDefinition>& _xReportDefinition,sal_Int32 _nPos,bool _bExportAutoStyle)
{
    if ( !_xReportDefinition.is() )
        return;

    Reference< XGroups > xGroups = _xReportDefinition->getGroups();
    if ( !xGroups.is() )
        return;

    sal_Int32 nCount = xGroups->getCount();
    if ( _nPos >= 0 && _nPos < nCount )
    {
        Reference<XGroup> xGroup(xGroups->getByIndex(_nPos),uno::UNO_QUERY);
        OSL_ENSURE(xGroup.is(),"No Group prepare for GPF");
        if ( _bExportAutoStyle )
        {
            if ( xGroup->getHeaderOn() )
                exportSectionAutoStyle(xGroup->getHeader());
            exportGroup(_xReportDefinition,_nPos+1,_bExportAutoStyle);
            if ( xGroup->getFooterOn() )
                exportSectionAutoStyle(xGroup->getFooter());
        }
        else
        {
            if ( xGroup->getSortAscending() )
                AddAttribute(XML_NAMESPACE_REPORT, XML_SORT_ASCENDING, XML_TRUE );

            if ( xGroup->getStartNewColumn() )
                AddAttribute(XML_NAMESPACE_REPORT, XML_START_NEW_COLUMN, XML_TRUE);
            if ( xGroup->getResetPageNumber() )
                AddAttribute(XML_NAMESPACE_REPORT, XML_RESET_PAGE_NUMBER, XML_TRUE );

            const OUString sField = xGroup->getExpression();
            OUString sExpression  = sField;
            if ( !sExpression.isEmpty() )
            {
                sal_Int32 nIndex = sExpression.indexOf('"');
                while ( nIndex > -1 )
                {
                    sExpression = sExpression.replaceAt(nIndex, 1, u"\"\"");
                    nIndex = sExpression.indexOf('"',nIndex+2);
                }

                TGroupFunctionMap::const_iterator aGroupFind = m_aGroupFunctionMap.find(xGroup);
                if ( aGroupFind != m_aGroupFunctionMap.end() )
                    sExpression = aGroupFind->second->getName();
                sExpression = "rpt:HASCHANGED(\"" + sExpression + "\")";
            }
            AddAttribute(XML_NAMESPACE_REPORT, XML_SORT_EXPRESSION, sField);
            AddAttribute(XML_NAMESPACE_REPORT, XML_GROUP_EXPRESSION,sExpression);
            sal_Int16 nRet = xGroup->getKeepTogether();
            OUStringBuffer sValue;
            const SvXMLEnumMapEntry<sal_Int16>* aXML_KeepTogetherEnumMap = OXMLHelper::GetKeepTogetherOptions();
            if ( SvXMLUnitConverter::convertEnum( sValue, nRet, aXML_KeepTogetherEnumMap ) )
                AddAttribute(XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER,sValue.makeStringAndClear());

            SvXMLElementExport aGroup(*this,XML_NAMESPACE_REPORT, XML_GROUP, true, true);
            exportFunctions(xGroup->getFunctions());
            if ( xGroup->getHeaderOn() )
            {
                Reference<XSection> xSection = xGroup->getHeader();
                if ( xSection->getRepeatSection() )
                    AddAttribute(XML_NAMESPACE_REPORT, XML_REPEAT_SECTION,XML_TRUE );
                SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_GROUP_HEADER, true, true);
                exportSection(xSection);
            }
            exportGroup(_xReportDefinition,_nPos+1,_bExportAutoStyle);
            if ( xGroup->getFooterOn() )
            {
                Reference<XSection> xSection = xGroup->getFooter();
                if ( xSection->getRepeatSection() )
                    AddAttribute(XML_NAMESPACE_REPORT, XML_REPEAT_SECTION,XML_TRUE );
                SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_GROUP_FOOTER, true, true);
                exportSection(xSection);
            }
        }
    }
    else if ( _bExportAutoStyle )
    {
        exportSectionAutoStyle(_xReportDefinition->getDetail());
    }
    else
    {
        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_DETAIL, true, true);
        exportSection(_xReportDefinition->getDetail());
    }
}

void ORptExport::exportAutoStyle(XPropertySet* _xProp,const Reference<XFormattedField>& _xParentFormattedField)
{
    const uno::Reference<report::XReportControlFormat> xFormat(_xProp,uno::UNO_QUERY);
    if ( xFormat.is() )
    {
        try
        {
            const awt::FontDescriptor aFont = xFormat->getFontDescriptor();
            OSL_ENSURE(!aFont.Name.isEmpty(),"No Font Name !");
            GetFontAutoStylePool()->Add(aFont.Name,aFont.StyleName,static_cast<FontFamily>(aFont.Family),
                static_cast<FontPitch>(aFont.Pitch),aFont.CharSet );
        }
        catch(beans::UnknownPropertyException&)
        {
            // not interested in
        }
    }
    const uno::Reference< report::XShape> xShape(_xProp,uno::UNO_QUERY);
    if ( xShape.is() )
    {
        ::std::vector<XMLPropertyState> aPropertyStates(m_xParaPropMapper->Filter(*this, _xProp));
        if ( !aPropertyStates.empty() )
            m_aAutoStyleNames.emplace( _xProp,GetAutoStylePool()->Add( XmlStyleFamily::TEXT_PARAGRAPH, std::move(aPropertyStates) ));
    }
    ::std::vector<XMLPropertyState> aPropertyStates(m_xCellStylesExportPropertySetMapper->Filter(*this, _xProp));
    Reference<XFixedLine> xFixedLine(_xProp,uno::UNO_QUERY);
    if ( xFixedLine.is() )
    {
        uno::Reference<beans::XPropertySet> xBorderProp = OXMLHelper::createBorderPropertySet();
        table::BorderLine2 aValue;
        aValue.Color = sal_uInt32(COL_BLACK);
        aValue.InnerLineWidth = aValue.LineDistance = 0;
        aValue.OuterLineWidth = DEFAULT_LINE_WIDTH;
        aValue.LineStyle = table::BorderLineStyle::SOLID;
        aValue.LineWidth = DEFAULT_LINE_WIDTH;

        awt::Point aPos = xFixedLine->getPosition();
        awt::Size aSize = xFixedLine->getSize();
        sal_Int32 nSectionHeight = xFixedLine->getSection()->getHeight();

        OUString sBorderProp;
        ::std::vector< OUString> aProps;
        if ( xFixedLine->getOrientation() == 1 ) // vertical
        {
            // check if border should be left
            if ( !aPos.X )
            {
                sBorderProp = PROPERTY_BORDER_LEFT;
                aProps.emplace_back(PROPERTY_BORDER_RIGHT);
            }
            else
            {
                sBorderProp = PROPERTY_BORDER_RIGHT;
                aProps.emplace_back(PROPERTY_BORDER_LEFT);
            }
            aProps.emplace_back(PROPERTY_BORDER_TOP);
            aProps.emplace_back(PROPERTY_BORDER_BOTTOM);
        }
        else // horizontal
        {
            // check if border should be bottom
            if ( (aPos.Y + aSize.Height) == nSectionHeight )
            {
                sBorderProp = PROPERTY_BORDER_BOTTOM;
                aProps.emplace_back(PROPERTY_BORDER_TOP);
            }
            else
            {
                sBorderProp = PROPERTY_BORDER_TOP;
                aProps.emplace_back(PROPERTY_BORDER_BOTTOM);
            }
            aProps.emplace_back(PROPERTY_BORDER_RIGHT);
            aProps.emplace_back(PROPERTY_BORDER_LEFT);
        }

        xBorderProp->setPropertyValue(sBorderProp,uno::makeAny(aValue));

        aValue.Color = aValue.OuterLineWidth = aValue.LineWidth = 0;
        aValue.LineStyle = table::BorderLineStyle::NONE;
        uno::Any aEmpty;
        aEmpty <<= aValue;
        for (auto const& it : aProps)
        {
            xBorderProp->setPropertyValue(it, aEmpty);
        }

        ::std::vector<XMLPropertyState> aBorderStates(m_xCellStylesExportPropertySetMapper->Filter(*this, xBorderProp));
        aPropertyStates.insert( aPropertyStates.end(), aBorderStates.begin(), aBorderStates.end() );
    }
    else
    {
        const Reference<XFormattedField> xFormattedField(_xProp,uno::UNO_QUERY);
        if ( (_xParentFormattedField.is() || xFormattedField.is()) && !aPropertyStates.empty() )
        {
            sal_Int32 nNumberFormat = 0;
            if ( _xParentFormattedField.is() )
                nNumberFormat = _xParentFormattedField->getFormatKey();
            else
                nNumberFormat = xFormattedField->getFormatKey();
            {
                sal_Int32 nStyleMapIndex = m_xCellStylesExportPropertySetMapper->getPropertySetMapper()->FindEntryIndex( CTF_RPT_NUMBERFORMAT );
                addDataStyle(nNumberFormat);
                XMLPropertyState aNumberStyleState( nStyleMapIndex, uno::makeAny( getDataStyleName(nNumberFormat) ) );
                auto const iter(::std::find_if(
                    aPropertyStates.begin(), aPropertyStates.end(),
                    [nStyleMapIndex] (XMLPropertyState const& rItem)
                        { return rItem.mnIndex == nStyleMapIndex; } ));
                if (iter == aPropertyStates.end())
                {
                    aPropertyStates.push_back( aNumberStyleState );
                }
                else
                {   // there is already a property but it has the wrong type
                    // (integer not string); TODO: can we prevent it
                    // getting added earlier?
                    (*iter) = aNumberStyleState;
                }
            }
        }
    }

    if ( !aPropertyStates.empty() )
        m_aAutoStyleNames.emplace( _xProp,GetAutoStylePool()->Add( XmlStyleFamily::TABLE_CELL, std::move(aPropertyStates) ));
}

void ORptExport::exportAutoStyle(const Reference<XSection>& _xProp)
{
    ::std::vector<XMLPropertyState> aPropertyStates(m_xTableStylesExportPropertySetMapper->Filter(*this, _xProp));
    if ( !aPropertyStates.empty() )
        m_aAutoStyleNames.emplace( _xProp.get(),GetAutoStylePool()->Add( XmlStyleFamily::TABLE_TABLE, std::move(aPropertyStates) ));
}

void ORptExport::SetBodyAttributes()
{
    Reference<XReportDefinition> xProp(getReportDefinition());
    exportReportAttributes(xProp);
}

void ORptExport::exportReportAttributes(const Reference<XReportDefinition>& _xReport)
{
    if ( !_xReport.is() )
        return;

    OUStringBuffer sValue;
    const SvXMLEnumMapEntry<sal_Int32>* aXML_CommandTypeEnumMap = OXMLHelper::GetCommandTypeOptions();
    if ( SvXMLUnitConverter::convertEnum( sValue, _xReport->getCommandType(), aXML_CommandTypeEnumMap ) )
        AddAttribute(XML_NAMESPACE_REPORT, XML_COMMAND_TYPE,sValue.makeStringAndClear());

    OUString sCommand = _xReport->getCommand();
    if ( !sCommand.isEmpty() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_COMMAND, sCommand);

    OUString sFilter( _xReport->getFilter() );
    if ( !sFilter.isEmpty() )
        AddAttribute( XML_NAMESPACE_REPORT, XML_FILTER, sFilter );

    AddAttribute(XML_NAMESPACE_OFFICE, XML_MIMETYPE,_xReport->getMimeType());

    bool bEscapeProcessing( _xReport->getEscapeProcessing() );
    if ( !bEscapeProcessing )
        AddAttribute( XML_NAMESPACE_REPORT, XML_ESCAPE_PROCESSING, ::xmloff::token::GetXMLToken( XML_FALSE ) );

    OUString sName = _xReport->getCaption();
    if ( !sName.isEmpty() )
        AddAttribute(XML_NAMESPACE_OFFICE, XML_CAPTION,sName);
    sName = _xReport->getName();
    if ( !sName.isEmpty() )
        AddAttribute(XML_NAMESPACE_DRAW, XML_NAME,sName);
}

void ORptExport::ExportContent_()
{
    exportReport(getReportDefinition());
}

void ORptExport::ExportMasterStyles_()
{
    GetPageExport()->exportMasterStyles( true );
}

void ORptExport::collectComponentStyles()
{
    if ( m_bAllreadyFilled )
        return;

    m_bAllreadyFilled = true;
    Reference<XReportDefinition> xProp(getReportDefinition());
    if ( !xProp.is() )
        return;

    uno::Reference< report::XSection> xParent(xProp->getParent(),uno::UNO_QUERY);
    if ( xParent.is() )
        exportAutoStyle(xProp.get());

    if ( xProp->getReportHeaderOn() )
        exportSectionAutoStyle(xProp->getReportHeader());
    if ( xProp->getPageHeaderOn() )
        exportSectionAutoStyle(xProp->getPageHeader());

    exportGroup(xProp,0,true);

    if ( xProp->getPageFooterOn() )
        exportSectionAutoStyle(xProp->getPageFooter());
    if ( xProp->getReportFooterOn() )
        exportSectionAutoStyle(xProp->getReportFooter());
}

void ORptExport::ExportAutoStyles_()
{
    // there are no styles that require their own autostyles
    if ( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        collectComponentStyles();
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_TABLE);
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_COLUMN);
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_ROW);
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_CELL);
        exportDataStyles();
        GetShapeExport()->exportAutoStyles();
    }
    // exported in _ExportMasterStyles
    if( getExportFlags() & SvXMLExportFlags::MASTERSTYLES )
        GetPageExport()->collectAutoStyles( false );
    if( getExportFlags() & SvXMLExportFlags::MASTERSTYLES )
        GetPageExport()->exportAutoStyles();
}

void ORptExport::ExportStyles_(bool bUsed)
{
    SvXMLExport::ExportStyles_(bUsed);

    // write draw:style-name for object graphic-styles
    GetShapeExport()->ExportGraphicDefaults();
}

SvXMLAutoStylePoolP* ORptExport::CreateAutoStylePool()
{
    return new OXMLAutoStylePoolP(*this);
}

void SAL_CALL ORptExport::setSourceDocument( const Reference< XComponent >& xDoc )
{
    m_xReportDefinition.set(xDoc,UNO_QUERY_THROW);
    SvXMLExport::setSourceDocument(xDoc);
}

void ORptExport::ExportFontDecls_()
{
    GetFontAutoStylePool(); // make sure the pool is created
    collectComponentStyles();
    SvXMLExport::ExportFontDecls_();
}

void ORptExport::exportParagraph(const Reference< XReportControlModel >& _xReportElement)
{
    OSL_PRECOND(_xReportElement.is(),"Element is null!");
    // start <text:p>
    SvXMLElementExport aParagraphContent(*this,XML_NAMESPACE_TEXT, XML_P, false, false);
    if ( Reference<XFormattedField>(_xReportElement,uno::UNO_QUERY).is() )
    {
        OUString sFieldData = _xReportElement->getDataField();
        static const char s_sPageNumber[] = "PageNumber()";
        static const char s_sReportPrefix[] = "rpt:";
        sFieldData = sFieldData.copy(strlen(s_sReportPrefix));
        sal_Int32 nPageNumberIndex = sFieldData.indexOf(s_sPageNumber);
        if ( nPageNumberIndex != -1 )
        {
            sal_Int32 nIndex = 0;
            do
            {
                OUString sToken = sFieldData.getToken( 0, '&', nIndex );
                sToken = sToken.trim();
                if ( !sToken.isEmpty() )
                {
                    if ( sToken == s_sPageNumber )
                    {
                        AddAttribute(XML_NAMESPACE_TEXT, XML_SELECT_PAGE, "current" );
                        SvXMLElementExport aPageNumber(*this,XML_NAMESPACE_TEXT, XML_PAGE_NUMBER, false, false);
                        Characters("1");
                    }
                    else if ( sToken == "PageCount()" )
                    {
                        SvXMLElementExport aPageNumber(*this,XML_NAMESPACE_TEXT, XML_PAGE_COUNT, false, false);
                        Characters("1");
                    }
                    else
                    {

                        if ( sToken.startsWith("\"") && sToken.endsWith("\"") )
                            sToken = sToken.copy(1,sToken.getLength()-2);

                        bool bPrevCharIsSpace = false;
                        GetTextParagraphExport()->exportCharacterData(sToken, bPrevCharIsSpace);
                    }
                }
            }
            while ( nIndex >= 0 );
        }
    }
    Reference< XFixedText > xFT(_xReportElement,UNO_QUERY);
    if ( xFT.is() )
    {
        OUString sExpr = xFT->getLabel();
        bool bPrevCharIsSpace = false; // FIXME this looks quite broken - does the corresponding import filter do whitespace collapsing at all?
        GetTextParagraphExport()->exportCharacterData(sExpr, bPrevCharIsSpace);
    }
}

XMLShapeExport* ORptExport::CreateShapeExport()
{
    XMLShapeExport* pShapeExport = new XMLShapeExport( *this, XMLTextParagraphExport::CreateShapeExtPropMapper( *this ) );
    return pShapeExport;
}

void ORptExport::exportShapes(const Reference< XSection>& _xSection,bool _bAddParagraph)
{
    rtl::Reference< XMLShapeExport > xShapeExport = GetShapeExport();
    xShapeExport->seekShapes(_xSection);
    const sal_Int32 nCount = _xSection->getCount();
    ::std::unique_ptr<SvXMLElementExport> pParagraphContent;
    if ( _bAddParagraph )
        pParagraphContent.reset(new SvXMLElementExport(*this,XML_NAMESPACE_TEXT, XML_P, true, false));

    awt::Point aRefPoint;
    aRefPoint.X = rptui::getStyleProperty<sal_Int32>(_xSection->getReportDefinition(),PROPERTY_LEFTMARGIN);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference< XShape > xShape(_xSection->getByIndex(i),uno::UNO_QUERY);
        if ( xShape.is() )
        {
            ::std::unique_ptr<SvXMLElementExport> pSubDocument;
            uno::Reference< frame::XModel> xModel(xShape->getPropertyValue("Model"),uno::UNO_QUERY);
            if ( xModel.is() ) // special handling for chart object
            {
                pSubDocument.reset(new SvXMLElementExport(*this,XML_NAMESPACE_REPORT, XML_SUB_DOCUMENT, false, false));
                exportMasterDetailFields(xShape);
                exportReportElement(xShape);
            }

            AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, XML_PARAGRAPH );
            xShapeExport->exportShape(xShape, SEF_DEFAULT|XMLShapeExportFlags::NO_WS,&aRefPoint);
        }
    }
}

void ORptExport::exportGroupsExpressionAsFunction(const Reference< XGroups>& _xGroups)
{
    if ( !_xGroups.is() )
        return;

    uno::Reference< XFunctions> xFunctions = _xGroups->getReportDefinition()->getFunctions();
    const sal_Int32 nCount = _xGroups->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference< XGroup> xGroup(_xGroups->getByIndex(i),uno::UNO_QUERY_THROW);
        const ::sal_Int16 nGroupOn = xGroup->getGroupOn();
        if ( nGroupOn != report::GroupOn::DEFAULT )
        {
            uno::Reference< XFunction> xFunction = xFunctions->createFunction();
            OUString sFunction,sPrefix,sPostfix;
            OUString sExpression = xGroup->getExpression();
            OUString sFunctionName;
            OUString sInitialFormula;
            switch(nGroupOn)
            {
                case report::GroupOn::PREFIX_CHARACTERS:
                    sFunction = "LEFT";
                    sPrefix = ";" + OUString::number(xGroup->getGroupInterval());
                    break;
                case report::GroupOn::YEAR:
                    sFunction = "YEAR";
                    break;
                case report::GroupOn::QUARTAL:
                    sFunction   = "INT((MONTH";
                    sPostfix    = "-1)/3)+1";
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
                        uno::Reference< XFunction> xCountFunction = xFunctions->createFunction();
                        xCountFunction->setInitialFormula(beans::Optional< OUString>(true,OUString("rpt:0")));
                        OUString sCountName = sFunction + "_count_" + sExpression;
                        xCountFunction->setName(sCountName);
                        xCountFunction->setFormula( "rpt:[" + sCountName + "] + 1" );
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
                default:
                    ;
            }
            if ( sFunctionName.isEmpty() )
                sFunctionName = sFunction + "_" + sExpression;
            if ( !sFunction.isEmpty() )
            {
                const sal_Unicode pReplaceChars[] = { '(',')',';',',','+','-','[',']','/','*'};
                for(sal_Unicode ch : pReplaceChars)
                    sFunctionName = sFunctionName.replace(ch,'_');

                xFunction->setName(sFunctionName);
                if ( !sInitialFormula.isEmpty() )
                    xFunction->setInitialFormula(beans::Optional< OUString>(true, sInitialFormula));
                sFunction = "rpt:" + sFunction + "([" + sExpression + "]";

                if ( !sPrefix.isEmpty() )
                    sFunction += sPrefix;
                sFunction += ")";
                if ( !sPostfix.isEmpty() )
                    sFunction += sPostfix;
                xFunction->setFormula(sFunction);
                exportFunction(xFunction);
                m_aGroupFunctionMap.emplace(xGroup,xFunction);
            }
        }
    }
}


}// rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
