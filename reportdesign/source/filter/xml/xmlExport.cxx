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

#include "xmlExport.hxx"
#include "xmlAutoStyle.hxx"
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "xmlEnums.hxx"
#include <xmloff/txtprmap.hxx>
#include <xmloff/numehelp.hxx>
#include "xmlHelper.hxx"
#include "xmlstrings.hrc"
#include "xmlPropertyHandler.hxx"
#include <sax/tools/converter.hxx>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/report/GroupOn.hpp>
#include <com/sun/star/report/XFixedText.hpp>
#include <com/sun/star/report/XImageControl.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/report/XFunction.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/frame/XController.hpp>
#include "RptDef.hxx"
// for locking SolarMutex: svapp + mutex
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <sal/macros.h>

#include <boost/bind.hpp>

//--------------------------------------------------------------------------
namespace rptxml
{
    using namespace xmloff;
    using namespace comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::xml;

    //---------------------------------------------------------------------
    Reference< XInterface > ORptExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(xContext,EXPORT_SETTINGS ));
    }
    //---------------------------------------------------------------------
    OUString ORptExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return OUString("com.sun.star.comp.report.XMLSettingsExporter");
    }
    //---------------------------------------------------------------------
    Sequence< OUString > ORptExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = "com.sun.star.document.ExportFilter";
        return aSupported;
    }
    //---------------------------------------------------------------------
    Reference< XInterface > ORptContentExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(xContext,EXPORT_CONTENT ));
    }
    //---------------------------------------------------------------------
    OUString ORptContentExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return OUString("com.sun.star.comp.report.XMLContentExporter");
    }
    //---------------------------------------------------------------------
    Sequence< OUString > ORptContentExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = "com.sun.star.document.ExportFilter";
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference< XInterface > ORptStylesExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(xContext,EXPORT_STYLES | EXPORT_MASTERSTYLES | EXPORT_AUTOSTYLES |
            EXPORT_FONTDECLS|EXPORT_OASIS ));
    }
    //---------------------------------------------------------------------
    OUString ORptStylesExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return OUString("com.sun.star.comp.report.XMLStylesExporter");
    }
    //---------------------------------------------------------------------
    Sequence< OUString > ORptStylesExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = "com.sun.star.document.ExportFilter";
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference< XInterface > ORptMetaExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(xContext, EXPORT_META ));
    }
    //---------------------------------------------------------------------
    OUString ORptMetaExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return OUString("com.sun.star.comp.report.XMLMetaExporter");
    }
    //---------------------------------------------------------------------
    Sequence< OUString > ORptMetaExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = "com.sun.star.document.ExportFilter";
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference< XInterface > ODBFullExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(xContext,EXPORT_ALL));
    }
    //---------------------------------------------------------------------
    OUString ODBFullExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return OUString("com.sun.star.comp.report.XMLFullExporter");
    }
    //---------------------------------------------------------------------
    Sequence< OUString > ODBFullExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = "com.sun.star.document.ExportFilter";
        return aSupported;
    }

    //---------------------------------------------------------------------

    class OSpecialHanldeXMLExportPropertyMapper : public SvXMLExportPropertyMapper
    {
    public:
        OSpecialHanldeXMLExportPropertyMapper(const UniReference< XMLPropertySetMapper >& rMapper) : SvXMLExportPropertyMapper(rMapper )
        {
        }
        /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
        virtual void handleSpecialItem(
                SvXMLAttributeList& /*rAttrList*/,
                const XMLPropertyState& /*rProperty*/,
                const SvXMLUnitConverter& /*rUnitConverter*/,
                const SvXMLNamespaceMap& /*rNamespaceMap*/,
                const ::std::vector< XMLPropertyState >* /*pProperties*/ = 0,
                sal_uInt32 /*nIdx*/ = 0 ) const
        {
            // nothing to do here
        }
    };
// -----------------------------------------------------------------------------
void lcl_adjustColumnSpanOverRows(ORptExport::TSectionsGrid& _rGrid)
{
    ORptExport::TSectionsGrid::iterator aSectionIter = _rGrid.begin();
    ORptExport::TSectionsGrid::iterator aSectionEnd = _rGrid.end();
    for (;aSectionIter != aSectionEnd ; ++aSectionIter)
    {
        ORptExport::TGrid::iterator aRowIter = aSectionIter->second.begin();
        ORptExport::TGrid::iterator aRowEnd = aSectionIter->second.end();
        for (; aRowIter != aRowEnd; ++aRowIter)
        {
            if ( aRowIter->first )
            {
                ::std::vector< ORptExport::TCell >::iterator aColIter = aRowIter->second.begin();
                ::std::vector< ORptExport::TCell >::iterator aColEnd = aRowIter->second.end();
                for (; aColIter != aColEnd; ++aColIter)
                {
                    if ( aColIter->nRowSpan > 1 )
                    {
                        sal_Int32 nColSpan = aColIter->nColSpan;
                        sal_Int32 nColIndex = aColIter - aRowIter->second.begin();
                        for (sal_Int32 i = 1; i < aColIter->nRowSpan; ++i)
                        {
                            (aRowIter+i)->second[nColIndex].nColSpan = nColSpan;
                        }
                    }
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
ORptExport::ORptExport(const Reference< XComponentContext >& _rxContext,sal_uInt16 nExportFlag)
: SvXMLExport( util::MeasureUnit::MM_100TH, _rxContext, XML_REPORT, EXPORT_OASIS)
,m_bAllreadyFilled(sal_False)
{
    setExportFlags( EXPORT_OASIS | nExportFlag);
    GetMM100UnitConverter().SetCoreMeasureUnit(MAP_100TH_MM);
    GetMM100UnitConverter().SetXMLMeasureUnit(MAP_CM);

    // (getExportFlags() & EXPORT_CONTENT) != 0 ? : XML_N_OOO
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE ), XML_NAMESPACE_OFFICE );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_RPT), GetXMLToken(XML_N_RPT), XML_NAMESPACE_REPORT );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG_COMPAT),  XML_NAMESPACE_SVG );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_FORM), GetXMLToken(XML_N_FORM), XML_NAMESPACE_FORM );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_TEXT), GetXMLToken(XML_N_TEXT), XML_NAMESPACE_TEXT );


    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS) ) != 0 )
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO_COMPAT), XML_NAMESPACE_FO );

    if( (getExportFlags() & (EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS) ) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    }
    if( (getExportFlags() & EXPORT_SETTINGS) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_CONFIG), GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
    }

    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS) ) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
    }
    // RDFa: needed for content and header/footer styles
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_XHTML),GetXMLToken(XML_N_XHTML), XML_NAMESPACE_XHTML );
    }
    // GRDDL: to convert RDFa and meta.xml to RDF
    if( (getExportFlags() & (EXPORT_META|EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_GRDDL),GetXMLToken(XML_N_GRDDL), XML_NAMESPACE_GRDDL );
    }

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_NUMBER), GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );

    m_sTableStyle = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_STYLE_NAME) );
    m_sColumnStyle = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_COLUMN) );
    m_sCellStyle = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_REPORT, GetXMLToken(XML_STYLE_NAME) );


    m_xPropHdlFactory = new OXMLRptPropHdlFactory();
    UniReference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
    UniReference < XMLPropertySetMapper > xTableStylesPropertySetMapper1 = new XMLPropertySetMapper(OXMLHelper::GetTableStyleProps(),xFac, true);
    UniReference < XMLPropertySetMapper > xTableStylesPropertySetMapper2 = new XMLTextPropertySetMapper(TEXT_PROP_MAP_TABLE_DEFAULTS, true );
    xTableStylesPropertySetMapper1->AddMapperEntry(xTableStylesPropertySetMapper2);

    m_xTableStylesExportPropertySetMapper = new SvXMLExportPropertyMapper(xTableStylesPropertySetMapper1);

    m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylePropertyMap( false, true);
    m_xCellStylesExportPropertySetMapper = new OSpecialHanldeXMLExportPropertyMapper(m_xCellStylesPropertySetMapper);
    m_xCellStylesExportPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(*this));

    UniReference < XMLPropertySetMapper > xColumnStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetColumnStyleProps(), m_xPropHdlFactory, true);
    m_xColumnStylesExportPropertySetMapper = new OSpecialHanldeXMLExportPropertyMapper(xColumnStylesPropertySetMapper);

    UniReference < XMLPropertySetMapper > xRowStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetRowStyleProps(), m_xPropHdlFactory, true);
    m_xRowStylesExportPropertySetMapper = new OSpecialHanldeXMLExportPropertyMapper(xRowStylesPropertySetMapper);

    UniReference < XMLPropertySetMapper > xPropMapper(new XMLTextPropertySetMapper( TEXT_PROP_MAP_PARA, true ));
    m_xParaPropMapper = new OSpecialHanldeXMLExportPropertyMapper( xPropMapper);

    OUString sFamily( GetXMLToken(XML_PARAGRAPH) );
    OUString aPrefix( 'P');
    GetAutoStylePool()->AddFamily( XML_STYLE_FAMILY_TEXT_PARAGRAPH, sFamily,
                              m_xParaPropMapper, aPrefix );

    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_CELL, OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME),
        m_xCellStylesExportPropertySetMapper, OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_COLUMN, OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME),
        m_xColumnStylesExportPropertySetMapper, OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_ROW, OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME),
        m_xRowStylesExportPropertySetMapper, OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_TABLE, OUString(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME),
        m_xTableStylesExportPropertySetMapper, OUString(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX));
}
// -----------------------------------------------------------------------------
Reference< XInterface > ORptExport::create(Reference< XComponentContext > const & xContext)
{
    return *(new ORptExport(xContext));
}

// -----------------------------------------------------------------------------
OUString ORptExport::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.report.ExportFilter");
}

//--------------------------------------------------------------------------
OUString SAL_CALL ORptExport::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< OUString > ORptExport::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aServices(1);
    aServices.getArray()[0] = "com.sun.star.document.ExportFilter";

    return aServices;
}
//--------------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL ORptExport::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL ORptExport::supportsService(const OUString& ServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}
// -----------------------------------------------------------------------------
void ORptExport::exportFunctions(const Reference<XIndexAccess>& _xFunctions)
{
    const sal_Int32 nCount = _xFunctions->getCount();
    for (sal_Int32 i = 0; i< nCount; ++i)
    {
        uno::Reference< report::XFunction> xFunction(_xFunctions->getByIndex(i),uno::UNO_QUERY_THROW);
        OSL_ENSURE(xFunction.is(),"Function object is NULL!");
        exportFunction(xFunction);
    }
}
// -----------------------------------------------------------------------------
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

    SvXMLElementExport aFunction(*this,XML_NAMESPACE_REPORT, XML_FUNCTION, sal_True, sal_True);
}
// -----------------------------------------------------------------------------
void ORptExport::exportMasterDetailFields(const Reference<XReportComponent>& _xReportComponet)
{
    const uno::Sequence< OUString> aMasterFields = _xReportComponet->getMasterFields();
    if ( aMasterFields.getLength() )
    {
        SvXMLElementExport aElement(*this,XML_NAMESPACE_REPORT, XML_MASTER_DETAIL_FIELDS, sal_True, sal_True);
        const uno::Sequence< OUString> aDetailFields = _xReportComponet->getDetailFields();

        OSL_ENSURE(aDetailFields.getLength() == aMasterFields.getLength(),"not equal length for master and detail fields!");

        const OUString* pDetailFieldsIter = aDetailFields.getConstArray();
        const OUString* pIter = aMasterFields.getConstArray();
        const OUString* pEnd   = pIter + aMasterFields.getLength();
        for(;pIter != pEnd;++pIter,++pDetailFieldsIter)
        {
            AddAttribute( XML_NAMESPACE_REPORT, XML_MASTER , *pIter );
            if ( !pDetailFieldsIter->isEmpty() )
                AddAttribute( XML_NAMESPACE_REPORT, XML_DETAIL , *pDetailFieldsIter );
            SvXMLElementExport aPair(*this,XML_NAMESPACE_REPORT, XML_MASTER_DETAIL_FIELD, sal_True, sal_True);
        }
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportReport(const Reference<XReportDefinition>& _xReportDefinition)
{
    if ( _xReportDefinition.is() )
    {
        exportFunctions(_xReportDefinition->getFunctions().get());
        exportGroupsExpressionAsFunction(_xReportDefinition->getGroups());

        if ( _xReportDefinition->getReportHeaderOn() )
        {
            SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_REPORT_HEADER, sal_True, sal_True);
            exportSection(_xReportDefinition->getReportHeader());
        }
        if ( _xReportDefinition->getPageHeaderOn() )
        {
            OUStringBuffer sValue;
            sal_uInt16 nRet = _xReportDefinition->getPageHeaderOption();
            const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
            if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
                AddAttribute(XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION,sValue.makeStringAndClear());

            SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_PAGE_HEADER, sal_True, sal_True);
            exportSection(_xReportDefinition->getPageHeader(),true);
        }

        exportGroup(_xReportDefinition,0);

        if ( _xReportDefinition->getPageFooterOn() )
        {
            OUStringBuffer sValue;
            sal_uInt16 nRet = _xReportDefinition->getPageFooterOption();
            const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
            if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
                AddAttribute(XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION,sValue.makeStringAndClear());
            SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_PAGE_FOOTER, sal_True, sal_True);
            exportSection(_xReportDefinition->getPageFooter(),true);
        }
        if ( _xReportDefinition->getReportFooterOn() )
        {
            SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_REPORT_FOOTER, sal_True, sal_True);
            exportSection(_xReportDefinition->getReportFooter());
        }
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportComponent(const Reference<XReportComponent>& _xReportComponent)
{
    OSL_ENSURE(_xReportComponent.is(),"No component interface!");
    if ( !_xReportComponent.is() )
        return;

    AddAttribute(XML_NAMESPACE_DRAW, XML_NAME,_xReportComponent->getName());

    SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_REPORT_COMPONENT, sal_False, sal_False);
}
// -----------------------------------------------------------------------------
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
            SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_FORMAT_CONDITION, sal_True, sal_True);
        }
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Can not access format condition!");
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportReportElement(const Reference<XReportControlModel>& _xReportElement)
{
    OSL_ENSURE(_xReportElement.is(),"_xReportElement is NULL -> GPF");
    if ( !_xReportElement->getPrintWhenGroupChange() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_PRINT_WHEN_GROUP_CHANGE, XML_FALSE );

    if ( !_xReportElement->getPrintRepeatedValues() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_PRINT_REPEATED_VALUES,XML_FALSE);

    SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_REPORT_ELEMENT, sal_True, sal_True);
    if ( _xReportElement->getCount() )
    {
        exportFormatConditions(_xReportElement);
    }

    OUString sExpr = _xReportElement->getConditionalPrintExpression();
    if ( !sExpr.isEmpty() )
    {
        exportFormula(XML_FORMULA,sExpr);
        SvXMLElementExport aPrintExpr(*this,XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION, sal_True, sal_True);
    }

    // only export when parent exists
    uno::Reference< report::XSection> xParent(_xReportElement->getParent(),uno::UNO_QUERY);
    if ( xParent.is() )
        exportComponent(_xReportElement.get());
}
// -----------------------------------------------------------------------------
void lcl_calculate(const ::std::vector<sal_Int32>& _aPosX,const ::std::vector<sal_Int32>& _aPosY,ORptExport::TGrid& _rColumns)
{
    sal_Int32 nCountX = _aPosX.size() - 1;
    sal_Int32 nCountY = _aPosY.size() - 1;
    for (sal_Int32 j = 0; j < nCountY; ++j)
    {
        sal_Int32 nHeight = _aPosY[j+1] - _aPosY[j];
        if ( nHeight )
            for (sal_Int32 i = 0; i < nCountX ; ++i)
            {
                _rColumns[j].second[i] = ORptExport::TCell(_aPosX[i+1] - _aPosX[i],nHeight,1,1);
                _rColumns[j].second[i].bSet = sal_True;
            }
    }
}
// -----------------------------------------------------------------------------
void ORptExport::collectStyleNames(sal_Int32 _nFamily,const ::std::vector< sal_Int32>& _aSize, ORptExport::TStringVec& _rStyleNames)
{
    ::std::vector< XMLPropertyState > aPropertyStates;
    aPropertyStates.push_back(XMLPropertyState(0));
    ::std::vector<sal_Int32>::const_iterator aIter = _aSize.begin();
    ::std::vector<sal_Int32>::const_iterator aIter2 = aIter + 1;
    ::std::vector<sal_Int32>::const_iterator aEnd = _aSize.end();
    for (;aIter2 != aEnd ; ++aIter,++aIter2)
    {
        sal_Int32 nValue = static_cast<sal_Int32>(*aIter2 - *aIter);
        aPropertyStates[0].maValue <<= nValue;
        _rStyleNames.push_back(GetAutoStylePool()->Add(_nFamily, aPropertyStates ));
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportSectionAutoStyle(const Reference<XSection>& _xProp)
{
    OSL_ENSURE(_xProp != NULL,"Section is NULL -> GPF");
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
    }

    ::std::sort(aColumnPos.begin(),aColumnPos.end(),::std::less<sal_Int32>());
    aColumnPos.erase(::std::unique(aColumnPos.begin(),aColumnPos.end()),aColumnPos.end());

    ::std::sort(aRowPos.begin(),aRowPos.end(),::std::less<sal_Int32>());
    aRowPos.erase(::std::unique(aRowPos.begin(),aRowPos.end()),aRowPos.end());

    TSectionsGrid::iterator aInsert = m_aSectionsGrid.insert(
        TSectionsGrid::value_type(
                                    _xProp.get(),
                                    TGrid(aRowPos.size() - 1,TGrid::value_type(sal_False,TRow(aColumnPos.size() - 1)))
                                  )
        ).first;
    lcl_calculate(aColumnPos,aRowPos,aInsert->second);

    TGridStyleMap::iterator aPos = m_aColumnStyleNames.insert(TGridStyleMap::value_type(_xProp.get(),TStringVec())).first;
    collectStyleNames(XML_STYLE_FAMILY_TABLE_COLUMN,aColumnPos,aPos->second);
    aPos = m_aRowStyleNames.insert(TGridStyleMap::value_type(_xProp.get(),TStringVec())).first;
    collectStyleNames(XML_STYLE_FAMILY_TABLE_ROW,aRowPos,aPos->second);

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
                aInsert->second[yi].first = sal_True;
                ++yi;
            }

            if (x2 - x1 != 0 && y2 - y1 != 0)
            {
                awt::Size aElementSize = xReportElement->getSize();
                if ( xFixedLine.is() && xFixedLine->getOrientation() == 1 ) // vertical
                    aElementSize.Width = static_cast<sal_Int32>(xFixedLine->getWidth()*0.5);

                sal_Int32 nColSpan = x2 - x1;
                sal_Int32 nRowSpan = y2 - y1;
                aInsert->second[y1].second[x1] =
                    TCell(
                        aElementSize.Width , // -1 why?
                        aElementSize.Height, // -1 why?
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
// -----------------------------------------------------------------------------
void ORptExport::exportReportComponentAutoStyles(const Reference<XSection>& _xProp)
{
    const sal_Int32 nCount = _xProp->getCount();
    for (sal_Int32 i = 0 ; i< nCount ; ++i)
    {
        const Reference<XReportComponent> xReportElement(_xProp->getByIndex(i),uno::UNO_QUERY);
        const Reference< report::XShape > xShape(xReportElement,uno::UNO_QUERY);
        if ( xShape.is() )
        {
            UniReference< XMLShapeExport > xShapeExport = GetShapeExport();
            xShapeExport->seekShapes(_xProp.get());
            SolarMutexGuard aGuard;
            xShapeExport->collectShapeAutoStyles(xShape.get());
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
// -----------------------------------------------------------------------------
void ORptExport::exportSection(const Reference<XSection>& _xSection,bool bHeader)
{
    OSL_ENSURE(_xSection.is(),"Section is NULL -> GPF");
    OUStringBuffer sValue;
    AddAttribute(XML_NAMESPACE_TABLE, XML_NAME,_xSection->getName());

    if ( !_xSection->getVisible() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_VISIBLE,XML_FALSE);

    if ( !bHeader )
    {
        sal_uInt16 nRet = _xSection->getForceNewPage();
        const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetForceNewPageOptions();
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
    SvXMLElementExport aComponents(*this,XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True);

    OUString sExpr = _xSection->getConditionalPrintExpression();
    if ( !sExpr.isEmpty() )
    {
        exportFormula(XML_FORMULA,sExpr);
        SvXMLElementExport aPrintExpr(*this,XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION, sal_True, sal_False);
    }

    exportContainer(_xSection);
}
// -----------------------------------------------------------------------------
void ORptExport::exportTableColumns(const Reference< XSection>& _xSection)
{
    SvXMLElementExport aColumns(*this,XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS, sal_True, sal_True);
    TGridStyleMap::iterator aColFind = m_aColumnStyleNames.find(_xSection.get());
    OSL_ENSURE(aColFind != m_aColumnStyleNames.end(),"ORptExport::exportTableColumns: Section not found in m_aColumnStyleNames!");
    if ( aColFind == m_aColumnStyleNames.end() )
        return;

    TStringVec::iterator aColIter = aColFind->second.begin();
    TStringVec::iterator aColEnd = aColFind->second.end();
    for (; aColIter != aColEnd; ++aColIter)
    {
        AddAttribute( m_sTableStyle,*aColIter );
        SvXMLElementExport aColumn(*this,XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True);
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportContainer(const Reference< XSection>& _xSection)
{
    OSL_ENSURE(_xSection.is(),"Section is NULL -> GPF");

    exportTableColumns(_xSection);

    TSectionsGrid::iterator aFind = m_aSectionsGrid.find(_xSection.get());
    OSL_ENSURE(aFind != m_aSectionsGrid.end(),"ORptExport::exportContainer: Section not found in grid!");
    if ( aFind == m_aSectionsGrid.end() )
        return;
    TGrid::iterator aRowIter = aFind->second.begin();
    TGrid::iterator aRowEnd = aFind->second.end();

    sal_Int32 nEmptyCellColSpan = 0;
    TGridStyleMap::iterator aRowFind = m_aRowStyleNames.find(_xSection.get());
    TStringVec::iterator aHeightIter = aRowFind->second.begin();
    OSL_ENSURE(aRowFind->second.size() == aFind->second.size(),"Different count for rows");

    bool bShapeHandled = false;
    ::std::map<sal_Int32,sal_Int32> aRowSpan;
    for (sal_Int32 j = 0; aRowIter != aRowEnd; ++aRowIter,++j,++aHeightIter)
    {
        AddAttribute( m_sTableStyle,*aHeightIter );
        SvXMLElementExport aRow(*this,XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True);
        if ( aRowIter->first )
        {
            ::std::vector< TCell >::iterator aColIter = aRowIter->second.begin();
            ::std::vector< TCell >::iterator aColEnd = aRowIter->second.end();
            nEmptyCellColSpan = 0;
            for (; aColIter != aColEnd; ++aColIter)
            {
                sal_Bool bCoveredCell = sal_False;
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

                    bCoveredCell = sal_True;
                    aColIter = aColIter + (aColIter->nColSpan - 1);
                }
                else if ( aColIter->bSet )
                {
                    if ( nEmptyCellColSpan > 0 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED,implConvertNumber(nEmptyCellColSpan) );
                        bCoveredCell = sal_True;
                        nColSpan = nEmptyCellColSpan - 1;
                        nEmptyCellColSpan = 0;
                    }
                    sal_Int32 nSpan = aColIter->nColSpan;
                    if ( nSpan > 1 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED,implConvertNumber(nSpan) );
                        nColSpan = nSpan - 1;
                        bCoveredCell = sal_True;
                    }
                    nSpan = aColIter->nRowSpan;
                    if ( nSpan > 1 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_ROWS_SPANNED,implConvertNumber(nSpan) );
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
                        OUString sEmpty;
                        sal_Int16 nCellType = aHelper.GetCellType(nFormatKey,bIsStandard);
                        // "Standard" means "no format set, value could be anything",
                        // so don't set a format attribute in this case.
                        // P.S.: "Standard" is called "General" in some languages
                        if (!bIsStandard)
                        {
                            if ( nCellType == util::NumberFormat::TEXT )
                                aHelper.SetNumberFormatAttributes(sEmpty, sEmpty);
                            else
                                aHelper.SetNumberFormatAttributes(nFormatKey, 0.0, sal_False);
                        }
                    }
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_False);

                    if ( aColIter->xElement.is() )
                    {
                        // start <text:p>
                        SvXMLElementExport aParagraphContent(*this,XML_NAMESPACE_TEXT, XML_P, sal_True,sal_False);
                        Reference<XServiceInfo> xElement(aColIter->xElement,uno::UNO_QUERY);
                        Reference<XReportComponent> xReportComponent = aColIter->xElement;

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
                            sal_Bool bExportData = sal_False;
                            if ( xElement->supportsService(SERVICE_FIXEDTEXT) )
                            {
                                eToken = XML_FIXED_CONTENT;
                            }
                            else if ( xElement->supportsService(SERVICE_FORMATTEDFIELD) )
                            {
                                eToken = XML_FORMATTED_TEXT;
                                bExportData = sal_True;
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
                                bExportData = sal_True;
                                OUStringBuffer sValue;
                                const SvXMLEnumMapEntry* aXML_ImageScaleEnumMap = OXMLHelper::GetImageScaleOptions();
                                if ( SvXMLUnitConverter::convertEnum( sValue, xImage->getScaleMode(),aXML_ImageScaleEnumMap ) )
                                    AddAttribute(XML_NAMESPACE_REPORT, XML_SCALE, sValue.makeStringAndClear() );
                            }
                            else if ( xReportDefinition.is() )
                            {
                                eToken = XML_SUB_DOCUMENT;
                            }
                            else if ( xSection.is() )
                            {
                            }

                            bool bPageSet = false;
                            if ( bExportData )
                            {
                                bPageSet = exportFormula(XML_FORMULA,xReportElement->getDataField());
                                if ( bPageSet )
                                    eToken = XML_FIXED_CONTENT;
                                else if ( eToken == XML_IMAGE )
                                    AddAttribute(XML_NAMESPACE_REPORT, XML_PRESERVE_IRI, xImage->getPreserveIRI() ? XML_TRUE : XML_FALSE );
                            }

                            //if ( !bPageSet )
                            {
                                // start <report:eToken>
                                SvXMLElementExport aComponents(*this,XML_NAMESPACE_REPORT, eToken, sal_False, sal_False);
                                if ( eToken == XML_FIXED_CONTENT )
                                    exportParagraph(xReportElement);
                                if ( xReportElement.is() )
                                    exportReportElement(xReportElement);

                                if ( eToken == XML_GROUP && xSection.is() )
                                    exportContainer(xSection);
                                else if ( eToken == XML_SUB_DOCUMENT && xReportDefinition.is() )
                                {
                                    SvXMLElementExport aOfficeElement( *this, XML_NAMESPACE_OFFICE, XML_BODY,sal_True, sal_True );
                                    SvXMLElementExport aElem( *this, sal_True,
                                                            XML_NAMESPACE_OFFICE, XML_REPORT,
                                                              sal_True, sal_True );

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
                        SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, sal_True, sal_True);
                    }

                }
            }
            if ( nEmptyCellColSpan )
            {
                {
                    AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED,implConvertNumber(nEmptyCellColSpan) );
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
                    if ( !bShapeHandled )
                    {
                        bShapeHandled = true;
                        exportShapes(_xSection);
                    }
                }
                for (sal_Int32 k = 0; k < nEmptyCellColSpan; ++k)
                {
                    SvXMLElementExport aCoveredCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, sal_True, sal_True);
                }
                nEmptyCellColSpan = 0;
            }
        }
        else
        { // empty rows
            nEmptyCellColSpan = aRowIter->second.size();
            if ( nEmptyCellColSpan )
            {
                {
                    AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED,implConvertNumber(nEmptyCellColSpan) );
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
                    if ( !bShapeHandled )
                    {
                        bShapeHandled = true;
                        exportShapes(_xSection);
                    }
                }
                for (sal_Int32 k = 1; k < nEmptyCellColSpan; ++k)
                {
                    SvXMLElementExport aCoveredCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, sal_True, sal_True);
                }
                nEmptyCellColSpan = 0;
            }
        }
    }
}
// -----------------------------------------------------------------------------
OUString ORptExport::convertFormula(const OUString& _sFormula)
{
    OUString sFormula = _sFormula;
    if ( _sFormula == "rpt:" )
        sFormula = "";
    return sFormula;
}
// -----------------------------------------------------------------------------
bool ORptExport::exportFormula(enum ::xmloff::token::XMLTokenEnum eName,const OUString& _sFormula)
{
    const OUString sFieldData = convertFormula(_sFormula);
    static const OUString s_sPageNumber("PageNumber()");
    static const OUString s_sPageCount("PageCount()");
    sal_Int32 nPageNumberIndex = sFieldData.indexOf(s_sPageNumber);
    sal_Int32 nPageCountIndex = sFieldData.indexOf(s_sPageCount);
    bool bRet = nPageNumberIndex != -1 || nPageCountIndex != -1;
    if ( !bRet )
        AddAttribute(XML_NAMESPACE_REPORT, eName,sFieldData);

    return bRet;
}
// -----------------------------------------------------------------------------
void ORptExport::exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt,const OUString& _sName)
{
    Reference<XPropertySet> xFind(_xProp);
    TPropertyStyleMap::iterator aFind = m_aAutoStyleNames.find(xFind);
    if ( aFind != m_aAutoStyleNames.end() )
    {
        _rAtt.AddAttribute( _sName,
                            aFind->second );
        m_aAutoStyleNames.erase(aFind);
    }
}
// -----------------------------------------------------------------------------
sal_Bool ORptExport::exportGroup(const Reference<XReportDefinition>& _xReportDefinition,sal_Int32 _nPos,sal_Bool _bExportAutoStyle)
{
    sal_Bool bGroupExported = sal_False;
    if ( _xReportDefinition.is() )
    {
        Reference< XGroups > xGroups = _xReportDefinition->getGroups();
        if ( xGroups.is() )
        {
            sal_Int32 nCount = xGroups->getCount();
            if ( _nPos >= 0 && _nPos < nCount )
            {
                bGroupExported = sal_True;
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
                        static OUString s_sQuote("\"\"");
                        sal_Int32 nIndex = sExpression.indexOf('"');
                        while ( nIndex > -1 )
                        {
                            sExpression = sExpression.replaceAt(nIndex,1,s_sQuote);
                            nIndex = sExpression.indexOf('"',nIndex+2);
                        }
                        OUString sFormula("rpt:HASCHANGED(\"");

                        TGroupFunctionMap::iterator aGroupFind = m_aGroupFunctionMap.find(xGroup);
                        if ( aGroupFind != m_aGroupFunctionMap.end() )
                            sExpression = aGroupFind->second->getName();
                        sFormula += sExpression;
                        sFormula += "\")";
                        sExpression = sFormula;
                    }
                    AddAttribute(XML_NAMESPACE_REPORT, XML_SORT_EXPRESSION, sField);
                    AddAttribute(XML_NAMESPACE_REPORT, XML_GROUP_EXPRESSION,sExpression);
                    sal_Int16 nRet = xGroup->getKeepTogether();
                    OUStringBuffer sValue;
                    const SvXMLEnumMapEntry* aXML_KeepTogetherEnumMap = OXMLHelper::GetKeepTogetherOptions();
                    if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_KeepTogetherEnumMap ) )
                        AddAttribute(XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER,sValue.makeStringAndClear());

                    SvXMLElementExport aGroup(*this,XML_NAMESPACE_REPORT, XML_GROUP, sal_True, sal_True);
                    exportFunctions(xGroup->getFunctions().get());
                    if ( xGroup->getHeaderOn() )
                    {
                        Reference<XSection> xSection = xGroup->getHeader();
                        if ( xSection->getRepeatSection() )
                            AddAttribute(XML_NAMESPACE_REPORT, XML_REPEAT_SECTION,XML_TRUE );
                        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_GROUP_HEADER, sal_True, sal_True);
                        exportSection(xSection);
                    }
                    exportGroup(_xReportDefinition,_nPos+1,_bExportAutoStyle);
                    if ( xGroup->getFooterOn() )
                    {
                        Reference<XSection> xSection = xGroup->getFooter();
                        if ( xSection->getRepeatSection() )
                            AddAttribute(XML_NAMESPACE_REPORT, XML_REPEAT_SECTION,XML_TRUE );
                        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_GROUP_FOOTER, sal_True, sal_True);
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
                SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_DETAIL, sal_True, sal_True);
                exportSection(_xReportDefinition->getDetail());
            }
        }
    }
    return bGroupExported;
}
// -----------------------------------------------------------------------------
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
        ::std::vector< XMLPropertyState > aPropertyStates( m_xParaPropMapper->Filter(_xProp) );
        if ( !aPropertyStates.empty() )
            m_aAutoStyleNames.insert( TPropertyStyleMap::value_type(_xProp,GetAutoStylePool()->Add( XML_STYLE_FAMILY_TEXT_PARAGRAPH, aPropertyStates )));
    }
    ::std::vector< XMLPropertyState > aPropertyStates( m_xCellStylesExportPropertySetMapper->Filter(_xProp) );
    Reference<XFixedLine> xFixedLine(_xProp,uno::UNO_QUERY);
    if ( xFixedLine.is() )
    {
        uno::Reference<beans::XPropertySet> xBorderProp = OXMLHelper::createBorderPropertySet();
        table::BorderLine2 aValue;
        aValue.Color = COL_BLACK;
        aValue.InnerLineWidth = aValue.LineDistance = 0;
        aValue.OuterLineWidth = 2;
        aValue.LineStyle = table::BorderLineStyle::SOLID;
        aValue.LineWidth = 2;

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
                sBorderProp = PROPERTY_BORDERLEFT;
                aProps.push_back(static_cast<const OUString&>(PROPERTY_BORDERRIGHT));
            }
            else
            {
                sBorderProp = PROPERTY_BORDERRIGHT;
                aProps.push_back(static_cast<const OUString&>(PROPERTY_BORDERLEFT));
            }
            aProps.push_back(static_cast<const OUString&>(PROPERTY_BORDERTOP));
            aProps.push_back(static_cast<const OUString&>(PROPERTY_BORDERBOTTOM));
        }
        else // horizontal
        {
            // check if border should be bottom
            if ( (aPos.Y + aSize.Height) == nSectionHeight )
            {
                sBorderProp = PROPERTY_BORDERBOTTOM;
                aProps.push_back(static_cast<const OUString&>(PROPERTY_BORDERTOP));
            }
            else
            {
                sBorderProp = PROPERTY_BORDERTOP;
                aProps.push_back(static_cast<const OUString&>(PROPERTY_BORDERBOTTOM));
            }
            aProps.push_back(static_cast<const OUString&>(PROPERTY_BORDERRIGHT));
            aProps.push_back(static_cast<const OUString&>(PROPERTY_BORDERLEFT));
        }

        xBorderProp->setPropertyValue(sBorderProp,uno::makeAny(aValue));

        aValue.Color = aValue.OuterLineWidth = aValue.LineWidth = 0;
        aValue.LineStyle = table::BorderLineStyle::NONE;
        uno::Any aEmpty;
        aEmpty <<= aValue;
        ::std::for_each(aProps.begin(),aProps.end(),
            ::boost::bind(&beans::XPropertySet::setPropertyValue,xBorderProp,_1,aEmpty));

        ::std::vector< XMLPropertyState > aBorderStates(m_xCellStylesExportPropertySetMapper->Filter(xBorderProp));
        ::std::copy(aBorderStates.begin(),aBorderStates.end(),::std::back_inserter(aPropertyStates));
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
                aPropertyStates.push_back( aNumberStyleState );
            }
        }
    }

    if ( !aPropertyStates.empty() )
        m_aAutoStyleNames.insert( TPropertyStyleMap::value_type(_xProp,GetAutoStylePool()->Add( XML_STYLE_FAMILY_TABLE_CELL, aPropertyStates )));
}
// -----------------------------------------------------------------------------
void ORptExport::exportAutoStyle(const Reference<XSection>& _xProp)
{
    ::std::vector< XMLPropertyState > aPropertyStates( m_xTableStylesExportPropertySetMapper->Filter(_xProp.get()) );
    if ( !aPropertyStates.empty() )
        m_aAutoStyleNames.insert( TPropertyStyleMap::value_type(_xProp.get(),GetAutoStylePool()->Add( XML_STYLE_FAMILY_TABLE_TABLE, aPropertyStates )));
}
// -----------------------------------------------------------------------------
void ORptExport::SetBodyAttributes()
{
    Reference<XReportDefinition> xProp(getReportDefinition());
    exportReportAttributes(xProp);
}
// -----------------------------------------------------------------------------
void ORptExport::exportReportAttributes(const Reference<XReportDefinition>& _xReport)
{
    if ( _xReport.is() )
    {
        OUStringBuffer sValue;
        const SvXMLEnumMapEntry* aXML_CommnadTypeEnumMap = OXMLHelper::GetCommandTypeOptions();
        if ( SvXMLUnitConverter::convertEnum( sValue, static_cast<sal_uInt16>(_xReport->getCommandType()),aXML_CommnadTypeEnumMap ) )
            AddAttribute(XML_NAMESPACE_REPORT, XML_COMMAND_TYPE,sValue.makeStringAndClear());

        OUString sComamnd = _xReport->getCommand();
        if ( !sComamnd.isEmpty() )
            AddAttribute(XML_NAMESPACE_REPORT, XML_COMMAND, sComamnd);

        OUString sFilter( _xReport->getFilter() );
        if ( !sFilter.isEmpty() )
            AddAttribute( XML_NAMESPACE_REPORT, XML_FILTER, sFilter );

        AddAttribute(XML_NAMESPACE_OFFICE, XML_MIMETYPE,_xReport->getMimeType());

        sal_Bool bEscapeProcessing( _xReport->getEscapeProcessing() );
        if ( !bEscapeProcessing )
            AddAttribute( XML_NAMESPACE_REPORT, XML_ESCAPE_PROCESSING, ::xmloff::token::GetXMLToken( XML_FALSE ) );

        OUString sName = _xReport->getCaption();
        if ( !sName.isEmpty() )
            AddAttribute(XML_NAMESPACE_OFFICE, XML_CAPTION,sName);
        sName = _xReport->getName();
        if ( !sName.isEmpty() )
            AddAttribute(XML_NAMESPACE_DRAW, XML_NAME,sName);
    }
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportContent()
{
    exportReport(getReportDefinition());
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportMasterStyles()
{
    GetPageExport()->exportMasterStyles( sal_True );
}
// -----------------------------------------------------------------------------
void ORptExport::collectComponentStyles()
{
    if ( m_bAllreadyFilled )
        return;

    m_bAllreadyFilled = sal_True;
    Reference<XReportDefinition> xProp(getReportDefinition());
    if ( xProp.is() )
    {
        uno::Reference< report::XSection> xParent(xProp->getParent(),uno::UNO_QUERY);
        if ( xParent.is() )
            exportAutoStyle(xProp.get());

        if ( xProp->getReportHeaderOn() )
            exportSectionAutoStyle(xProp->getReportHeader());
        if ( xProp->getPageHeaderOn() )
            exportSectionAutoStyle(xProp->getPageHeader());

        exportGroup(xProp,0,sal_True);

        if ( xProp->getPageFooterOn() )
            exportSectionAutoStyle(xProp->getPageFooter());
        if ( xProp->getReportFooterOn() )
            exportSectionAutoStyle(xProp->getReportFooter());
    }
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportAutoStyles()
{
    // there are no styles that require their own autostyles
    if ( getExportFlags() & EXPORT_CONTENT )
    {
        collectComponentStyles();
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_TABLE
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_COLUMN
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_ROW
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );

        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_CELL
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );

        exportDataStyles();
        GetShapeExport()->exportAutoStyles();
    }
    // exported in _ExportMasterStyles
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->collectAutoStyles( sal_False );
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->exportAutoStyles();
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportStyles(sal_Bool bUsed)
{
    SvXMLExport::_ExportStyles(bUsed);

    // write draw:style-name for object graphic-styles
    GetShapeExport()->ExportGraphicDefaults();
}
// -----------------------------------------------------------------------------
sal_uInt32 ORptExport::exportDoc(enum ::xmloff::token::XMLTokenEnum eClass)
{
    return SvXMLExport::exportDoc( eClass );
}
// -----------------------------------------------------------------------------
OUString ORptExport::implConvertNumber(sal_Int32 _nValue)
{
    OUStringBuffer aBuffer;
    ::sax::Converter::convertNumber(aBuffer, _nValue);
    return aBuffer.makeStringAndClear();
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > ORptExport::GetCellStylePropertyMapper() const
{
    return m_xCellStylesPropertySetMapper;
}
// -----------------------------------------------------------------------------
SvXMLAutoStylePoolP* ORptExport::CreateAutoStylePool()
{
    return new OXMLAutoStylePoolP(*this);
}
// -----------------------------------------------------------------------------
void SAL_CALL ORptExport::setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException)
{
    m_xReportDefinition.set(xDoc,UNO_QUERY_THROW);
    OSL_ENSURE(m_xReportDefinition.is(),"DataSource is NULL!");

    SvXMLExport::setSourceDocument(xDoc);
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportFontDecls()
{
    GetFontAutoStylePool(); // make sure the pool is created
    collectComponentStyles();
    SvXMLExport::_ExportFontDecls();
}
// -----------------------------------------------------------------------------
void ORptExport::exportParagraph(const Reference< XReportControlModel >& _xReportElement)
{
    OSL_PRECOND(_xReportElement.is(),"Element is null!");
    // start <text:p>
    SvXMLElementExport aParagraphContent(*this,XML_NAMESPACE_TEXT, XML_P, sal_False, sal_False);
    if ( Reference<XFormattedField>(_xReportElement,uno::UNO_QUERY).is() )
    {
        OUString sFieldData = _xReportElement->getDataField();
        static const OUString s_sPageNumber("PageNumber()");
        static const OUString s_sPageCount("PageCount()");
        static const OUString s_sReportPrefix("rpt:");
        sFieldData = sFieldData.copy(s_sReportPrefix.getLength(),sFieldData.getLength() - s_sReportPrefix.getLength());
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
                        static const OUString s_sCurrent("current");
                        AddAttribute(XML_NAMESPACE_TEXT, XML_SELECT_PAGE, s_sCurrent );
                        SvXMLElementExport aPageNumber(*this,XML_NAMESPACE_TEXT, XML_PAGE_NUMBER, sal_False, sal_False);
                        Characters(OUString("1"));
                    }
                    else if ( sToken == s_sPageCount )
                    {
                        SvXMLElementExport aPageNumber(*this,XML_NAMESPACE_TEXT, XML_PAGE_COUNT, sal_False, sal_False);
                        Characters(OUString("1"));
                    }
                    else
                    {

                        if ( sToken.startsWith("\"") && sToken.endsWith("\"") )
                            sToken = sToken.copy(1,sToken.getLength()-2);

                        bool bPrevCharIsSpace = false;
                        GetTextParagraphExport()->exportText(sToken,bPrevCharIsSpace);
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
        bool bPrevCharIsSpace = false;
        GetTextParagraphExport()->exportText(sExpr,bPrevCharIsSpace);
    }
}
// -----------------------------------------------------------------------------
XMLShapeExport* ORptExport::CreateShapeExport()
{
    XMLShapeExport* pShapeExport = new XMLShapeExport( *this, XMLTextParagraphExport::CreateShapeExtPropMapper( *this ) );
    return pShapeExport;
}
// -----------------------------------------------------------------------------
void ORptExport::exportShapes(const Reference< XSection>& _xSection,bool _bAddParagraph)
{
    UniReference< XMLShapeExport > xShapeExport = GetShapeExport();
    xShapeExport->seekShapes(_xSection.get());
    const sal_Int32 nCount = _xSection->getCount();
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SvXMLElementExport> pParagraphContent;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if ( _bAddParagraph )
        pParagraphContent.reset(new SvXMLElementExport(*this,XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False));

    awt::Point aRefPoint;
    aRefPoint.X = rptui::getStyleProperty<sal_Int32>(_xSection->getReportDefinition(),PROPERTY_LEFTMARGIN);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference< XShape > xShape(_xSection->getByIndex(i),uno::UNO_QUERY);
        if ( xShape.is() )
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr<SvXMLElementExport> pSubDocument;
            SAL_WNODEPRECATED_DECLARATIONS_POP
            uno::Reference< frame::XModel> xModel(xShape->getPropertyValue("Model"),uno::UNO_QUERY);
            if ( xModel.is() ) // special handling for chart object
            {
                pSubDocument.reset(new SvXMLElementExport(*this,XML_NAMESPACE_REPORT, XML_SUB_DOCUMENT, sal_False, sal_False));
                exportMasterDetailFields(xShape.get());
                exportReportElement(xShape.get());
            }

            AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, XML_PARAGRAPH );
            xShapeExport->exportShape(xShape.get(),SEF_DEFAULT|SEF_EXPORT_NO_WS,&aRefPoint);
        }
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportGroupsExpressionAsFunction(const Reference< XGroups>& _xGroups)
{
    if ( _xGroups.is() )
    {
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
                            xCountFunction->setInitialFormula(beans::Optional< OUString>(sal_True,OUString("rpt:0")));
                            OUString sCountName = sFunction + "_count_" + sExpression;
                            xCountFunction->setName(sCountName);
                            xCountFunction->setFormula(OUString("rpt:[") + sCountName + OUString("] + 1"));
                            exportFunction(xCountFunction);
                            sExpression = sCountName;
                            // The reference to sCountName in the formula of sFunctionName refers to the *old* value
                            // so we need to expand the the formula of sCountName
                            sPrefix = " + 1) / " + OUString::number(xGroup->getGroupInterval());
                            sFunctionName = sFunction + "_" + sExpression;
                            sFunction = sFunction + "(";
                            sInitialFormula = "rpt:0";
                        }
                        break;
                    default:
                        ;
                }
                if ( sFunctionName.isEmpty() )
                    sFunctionName = sFunction + OUString("_") + sExpression;
                if ( !sFunction.isEmpty() )
                {
                    sal_Unicode pReplaceChars[] = { '(',')',';',',','+','-','[',']','/','*'};
                    for(sal_uInt32 j= 0; j < sizeof(pReplaceChars)/sizeof(pReplaceChars[0]);++j)
                        sFunctionName = sFunctionName.replace(pReplaceChars[j],'_');

                    xFunction->setName(sFunctionName);
                    if ( !sInitialFormula.isEmpty() )
                        xFunction->setInitialFormula(beans::Optional< OUString>(sal_True, sInitialFormula));
                    sFunction = "rpt:" + sFunction;
                    sFunction += "([";
                    sFunction += sExpression;
                    sFunction += "]";

                    if ( !sPrefix.isEmpty() )
                        sFunction += sPrefix;
                    sFunction += ")";
                    if ( !sPostfix.isEmpty() )
                        sFunction += sPostfix;
                    xFunction->setFormula(sFunction);
                    exportFunction(xFunction);
                    m_aGroupFunctionMap.insert(TGroupFunctionMap::value_type(xGroup,xFunction));
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
}// rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
