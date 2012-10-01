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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmlHelper.hxx"
#include "xmlGroup.hxx"
#include "xmlSection.hxx"
#include "xmlEnums.hxx"
#include "xmlFunction.hxx"
#include <tools/diagnose_ex.h>
#include <com/sun/star/sdb/CommandType.hpp>
#include "xmlMasterFields.hxx"

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace uno;
    using namespace xml::sax;

DBG_NAME( rpt_OXMLReport )

OXMLReport::OXMLReport( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,const uno::Reference< report::XReportDefinition >& _xComponent
                ,OXMLTable* _pContainer) :
    OXMLReportElementBase( rImport, nPrfx, rLName,_xComponent.get(),_pContainer)
    ,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLReport,NULL);
    OSL_ENSURE(m_xComponent.is(),"No Report definition!");

    impl_initRuntimeDefaults();

    const SvXMLNamespaceMap& rMap = m_rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = m_rImport.GetReportElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
         ::rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_COMMAND_TYPE:
                    {
                        sal_uInt16 nRet = static_cast<sal_uInt16>(sdb::CommandType::COMMAND);
                        const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetCommandTypeOptions();
                        SvXMLUnitConverter::convertEnum( nRet, sValue, aXML_EnumMap );
                        m_xComponent->setCommandType(nRet);
                    }
                    break;
                case XML_TOK_COMMAND:
                    m_xComponent->setCommand(sValue);
                    break;
                case XML_TOK_FILTER:
                    m_xComponent->setFilter(sValue);
                    break;
                case XML_TOK_CAPTION:
                    m_xComponent->setCaption(sValue);
                    break;
                case XML_TOK_ESCAPE_PROCESSING:
                    m_xComponent->setEscapeProcessing(sValue == s_sTRUE);
                    break;
                case XML_TOK_REPORT_MIMETYPE:
                    m_xComponent->setMimeType(sValue);
                    break;
                case XML_TOK_REPORT_NAME:
                    m_xComponent->setName(sValue);
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception catched while filling the report definition props");
    }
}
// -----------------------------------------------------------------------------

OXMLReport::~OXMLReport()
{
    DBG_DTOR( rpt_OXMLReport,NULL);
}

// -----------------------------------------------------------------------------

void OXMLReport::impl_initRuntimeDefaults() const
{
    OSL_PRECOND( m_xComponent.is(), "OXMLReport::impl_initRuntimeDefaults: no component!" );
    if ( !m_xComponent.is() )
        return;

    try
    {
        m_xComponent->setCommandType( sdb::CommandType::COMMAND );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLReport::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = _CreateChildContext(nPrefix,rLocalName,xAttrList);
    if ( pContext )
        return pContext;
    const SvXMLTokenMap&    rTokenMap   = m_rImport.GetReportElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_REPORT_FUNCTION:
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLFunction( m_rImport, nPrefix, rLocalName,xAttrList,m_xComponent.get(),true);
            }
            break;
        case XML_TOK_MASTER_DETAIL_FIELDS:
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLMasterFields(m_rImport, nPrefix, rLocalName,xAttrList ,this);
            break;
        case XML_TOK_REPORT_HEADER:
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xComponent->setReportHeaderOn(sal_True);
                pContext = new OXMLSection( m_rImport, nPrefix, rLocalName,xAttrList ,m_xComponent->getReportHeader());
            }
            break;
        case XML_TOK_PAGE_HEADER:
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xComponent->setPageHeaderOn(sal_True);
                pContext = new OXMLSection( m_rImport, nPrefix, rLocalName,xAttrList ,m_xComponent->getPageHeader());
            }
            break;
        case XML_TOK_GROUP:
            m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLGroup( m_rImport, nPrefix, rLocalName,xAttrList);
            break;
        case XML_TOK_DETAIL:
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLSection( m_rImport, nPrefix, rLocalName,xAttrList ,m_xComponent->getDetail());
            }
            break;
        case XML_TOK_PAGE_FOOTER:
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xComponent->setPageFooterOn(sal_True);
                pContext = new OXMLSection( m_rImport, nPrefix, rLocalName,xAttrList ,m_xComponent->getPageFooter(),sal_False);
            }
            break;
        case XML_TOK_REPORT_FOOTER:
            {
                m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xComponent->setReportFooterOn(sal_True);
                pContext = new OXMLSection( m_rImport, nPrefix, rLocalName,xAttrList ,m_xComponent->getReportFooter());
            }
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );


    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLReport::EndElement()
{
    Reference< XFunctions > xFunctions = m_xComponent->getFunctions();
    const ORptFilter::TGroupFunctionMap& aFunctions = m_rImport.getFunctions();
    ORptFilter::TGroupFunctionMap::const_iterator aIter = aFunctions.begin();
    const ORptFilter::TGroupFunctionMap::const_iterator aEnd = aFunctions.end();
    for (; aIter != aEnd; ++aIter)
        xFunctions->insertByIndex(xFunctions->getCount(),uno::makeAny(aIter->second));

    if ( !m_aMasterFields.empty() )
        m_xComponent->setMasterFields(Sequence< ::rtl::OUString>(&*m_aMasterFields.begin(),m_aMasterFields.size()));
    if ( !m_aDetailFields.empty() )
        m_xComponent->setDetailFields(Sequence< ::rtl::OUString>(&*m_aDetailFields.begin(),m_aDetailFields.size()));
}
// -----------------------------------------------------------------------------
void OXMLReport::addMasterDetailPair(const ::std::pair< ::rtl::OUString,::rtl::OUString >& _aPair)
{
    m_aMasterFields.push_back(_aPair.first);
    m_aDetailFields.push_back(_aPair.second);
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
