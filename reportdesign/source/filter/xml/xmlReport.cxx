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
#include "xmlReport.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlHelper.hxx"
#include "xmlGroup.hxx"
#include "xmlSection.hxx"
#include "xmlEnums.hxx"
#include "xmlFunction.hxx"
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include "xmlMasterFields.hxx"

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace uno;
    using namespace xml::sax;


OXMLReport::OXMLReport( ORptFilter& rImport,
                const Reference< css::xml::sax::XFastAttributeList > & _xAttrList
                ,const uno::Reference< report::XReportDefinition >& _xComponent) :
    OXMLReportElementBase( rImport, _xComponent.get(),nullptr)
    ,m_xReportDefinition(_xComponent)
{
    OSL_ENSURE(m_xReportDefinition.is(),"No Report definition!");

    impl_initRuntimeDefaults();

    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    try
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            OUString sValue = aIter.toString();

            switch( aIter.getToken() )
            {
                case XML_ELEMENT(REPORT, XML_COMMAND_TYPE):
                    {
                        sal_Int32 nRet = sdb::CommandType::COMMAND;
                        const SvXMLEnumMapEntry<sal_Int32>* aXML_EnumMap = OXMLHelper::GetCommandTypeOptions();
                        bool bConvertOk = SvXMLUnitConverter::convertEnum( nRet, sValue, aXML_EnumMap );
                        SAL_WARN_IF(!bConvertOk, "reportdesign", "convertEnum failed");
                        m_xReportDefinition->setCommandType(nRet);
                    }
                    break;
                case XML_ELEMENT(REPORT, XML_COMMAND):
                    m_xReportDefinition->setCommand(sValue);
                    break;
                case XML_ELEMENT(REPORT, XML_FILTER):
                    m_xReportDefinition->setFilter(sValue);
                    break;
                case XML_ELEMENT(REPORT, XML_CAPTION):
                case XML_ELEMENT(OFFICE, XML_CAPTION):
                    m_xReportDefinition->setCaption(sValue);
                    break;
                case XML_ELEMENT(REPORT, XML_ESCAPE_PROCESSING):
                    m_xReportDefinition->setEscapeProcessing(sValue == s_sTRUE);
                    break;
                case XML_ELEMENT(OFFICE, XML_MIMETYPE):
                    m_xReportDefinition->setMimeType(sValue);
                    break;
                case XML_ELEMENT(DRAW, XML_NAME):
                    m_xReportDefinition->setName(sValue);
                    break;
                default:
                    SAL_WARN("reportdesign", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << " = " << sValue);
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception caught while filling the report definition props");
    }
}


OXMLReport::~OXMLReport()
{
}


void OXMLReport::impl_initRuntimeDefaults() const
{
    OSL_PRECOND( m_xReportDefinition.is(), "OXMLReport::impl_initRuntimeDefaults: no component!" );
    if ( !m_xReportDefinition.is() )
        return;

    try
    {
        m_xReportDefinition->setCommandType( sdb::CommandType::COMMAND );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}


css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLReport::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext = OXMLReportElementBase::createFastChildContext(nElement,xAttrList);
    if (xContext)
        return xContext;

    switch( nElement )
    {
        case XML_ELEMENT(REPORT, XML_FUNCTION):
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                xContext = new OXMLFunction( m_rImport,xAttrList,m_xReportDefinition.get(),true);
            }
            break;
        case XML_ELEMENT(REPORT, XML_MASTER_DETAIL_FIELDS):
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                xContext = new OXMLMasterFields(m_rImport,xAttrList ,this);
            break;
        case XML_ELEMENT(REPORT, XML_REPORT_HEADER):
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xReportDefinition->setReportHeaderOn(true);
                xContext = new OXMLSection( m_rImport,xAttrList, m_xReportDefinition->getReportHeader());
            }
            break;
        case XML_ELEMENT(REPORT, XML_PAGE_HEADER):
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xReportDefinition->setPageHeaderOn(true);
                xContext = new OXMLSection( m_rImport,xAttrList, m_xReportDefinition->getPageHeader());
            }
            break;
        case XML_ELEMENT(REPORT, XML_GROUP):
            m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLGroup( m_rImport,xAttrList);
            break;
        case XML_ELEMENT(REPORT, XML_DETAIL):
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                xContext = new OXMLSection( m_rImport,xAttrList, m_xReportDefinition->getDetail());
            }
            break;
        case XML_ELEMENT(REPORT, XML_PAGE_FOOTER):
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xReportDefinition->setPageFooterOn(true);
                xContext = new OXMLSection( m_rImport,xAttrList, m_xReportDefinition->getPageFooter(),false);
            }
            break;
        case XML_ELEMENT(REPORT, XML_REPORT_FOOTER):
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xReportDefinition->setReportFooterOn(true);
                xContext = new OXMLSection( m_rImport, xAttrList, m_xReportDefinition->getReportFooter());
            }
            break;
        default:
            break;
    }

    return xContext;
}

void OXMLReport::endFastElement(sal_Int32)
{
    Reference< XFunctions > xFunctions = m_xReportDefinition->getFunctions();
    const ORptFilter::TGroupFunctionMap& aFunctions = m_rImport.getFunctions();
    for (const auto& rEntry : aFunctions)
        xFunctions->insertByIndex(xFunctions->getCount(),uno::makeAny(rEntry.second));

    if ( !m_aMasterFields.empty() )
        m_xReportDefinition->setMasterFields(Sequence< OUString>(&*m_aMasterFields.begin(),m_aMasterFields.size()));
    if ( !m_aDetailFields.empty() )
        m_xReportDefinition->setDetailFields(Sequence< OUString>(&*m_aDetailFields.begin(),m_aDetailFields.size()));
}

void OXMLReport::addMasterDetailPair(const ::std::pair< OUString,OUString >& _aPair)
{
    m_aMasterFields.push_back(_aPair.first);
    m_aDetailFields.push_back(_aPair.second);
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
