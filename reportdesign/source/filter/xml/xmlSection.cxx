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
#include "xmlSection.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmlHelper.hxx"
#include "xmlEnums.hxx"
#include "xmlColumn.hxx"
#include "xmlCondPrtExpr.hxx"
#include "xmlStyleImport.hxx"
#include <connectivity/dbtools.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/report/ReportPrintOption.hpp>
#include "xmlstrings.hrc"
#include "xmlTable.hxx"


namespace rptxml
{
    using namespace ::xmloff;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

      sal_uInt16 lcl_getReportPrintOption(const ::rtl::OUString& _sValue)
    {
        sal_uInt16 nRet = report::ReportPrintOption::ALL_PAGES;
        const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
        SvXMLUnitConverter::convertEnum( nRet, _sValue, aXML_EnumMap );
        return nRet;
    }


DBG_NAME( rpt_OXMLSection )

OXMLSection::OXMLSection( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& _sLocalName,
                const uno::Reference< xml::sax::XAttributeList > & _xAttrList
                ,const uno::Reference< report::XSection >& _xSection
                ,sal_Bool _bPageHeader)
:SvXMLImportContext( rImport, nPrfx, _sLocalName )
,m_xSection(_xSection)
,m_bPageHeader(_bPageHeader)
{
    DBG_CTOR( rpt_OXMLSection,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetSectionElemTokenMap();

    const sal_Int16 nLength = (m_xSection.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {

                case XML_TOK_PAGE_PRINT_OPTION:
                    if ( _bPageHeader )
                        m_xSection->getReportDefinition()->setPageHeaderOption(lcl_getReportPrintOption(sValue));
                    else
                        m_xSection->getReportDefinition()->setPageFooterOption(lcl_getReportPrintOption(sValue));
                    break;
                case XML_TOK_REPEAT_SECTION:
                    m_xSection->setRepeatSection(sValue == s_sTRUE );
                    break;

                default:
                    OSL_FAIL("OXMLSection: Unknown attribute!");
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception catched while filling the section props");
    }
}
// -----------------------------------------------------------------------------
OXMLSection::~OXMLSection()
{
    DBG_DTOR( rpt_OXMLSection,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLSection::CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetSectionElemTokenMap();
    uno::Reference<lang::XMultiServiceFactory> xFactor = rImport.getServiceFactory();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_TABLE:
            pContext = new OXMLTable( rImport, _nPrefix, _rLocalName,xAttrList,m_xSection);
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( rImport, _nPrefix, _rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLSection::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
