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
#include "xmlReportElementBase.hxx"
#include "xmlfilter.hxx"
#include "xmlControlProperty.hxx"
#include "xmlReportElement.hxx"
#include "xmlEnums.hxx"
#include "xmlTable.hxx"

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLReportElementBase::OXMLReportElementBase( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                , const ::rtl::OUString& rLName
                ,const Reference< XReportComponent > & _xComponent
                ,OXMLTable* _pContainer) :
    SvXMLImportContext( rImport, nPrfx, rLName )
,m_rImport(rImport)
,m_pContainer(_pContainer)
,m_xComponent(_xComponent)
{
}
// -----------------------------------------------------------------------------

OXMLReportElementBase::~OXMLReportElementBase()
{
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLReportElementBase::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = _CreateChildContext(nPrefix,rLocalName,xAttrList);
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    return pContext;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLReportElementBase::_CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = m_rImport.GetControlElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_REPORT_ELEMENT:
            {
                uno::Reference<report::XReportControlModel> xReportModel(m_xComponent,uno::UNO_QUERY);
                if ( xReportModel.is() )
                {
                    m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                    pContext = new OXMLReportElement( m_rImport, nPrefix, rLocalName,xAttrList,xReportModel);
                }
            }
            break;
        case XML_TOK_PROPERTIES:
            m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLControlProperty( m_rImport, nPrefix, rLocalName,xAttrList,m_xComponent.get());
            break;
        default:
            break;
    }

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLReportElementBase::EndElement()
{
    try
    {
        if ( m_pContainer && m_pContainer->getSection().is() && m_xComponent.is() )
            m_pContainer->getSection()->add(m_xComponent.get());
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception caught while inserting a new control!");
    }
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
