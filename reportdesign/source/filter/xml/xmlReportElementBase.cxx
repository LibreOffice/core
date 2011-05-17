/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"
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
