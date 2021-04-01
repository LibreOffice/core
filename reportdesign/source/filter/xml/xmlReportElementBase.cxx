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
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmlReportElementBase.hxx"
#include "xmlfilter.hxx"
#include "xmlControlProperty.hxx"
#include "xmlReportElement.hxx"
#include "xmlEnums.hxx"
#include "xmlTable.hxx"
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLReportElementBase::OXMLReportElementBase( ORptFilter& rImport
                ,const Reference< XReportComponent > & _xComponent
                ,OXMLTable* _pContainer) :
    SvXMLImportContext( rImport )
,m_rImport(rImport)
,m_pContainer(_pContainer)
,m_xReportComponent(_xComponent)
{
}


OXMLReportElementBase::~OXMLReportElementBase()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLReportElementBase::createFastChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList > & xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;

    switch( nElement )
    {
        case XML_ELEMENT(REPORT, XML_REPORT_ELEMENT):
            {
                uno::Reference<report::XReportControlModel> xReportModel(m_xReportComponent,uno::UNO_QUERY);
                if ( xReportModel.is() )
                {
                    m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                    xContext = new OXMLReportElement( m_rImport,xAttrList,xReportModel);
                }
            }
            break;
        case XML_ELEMENT(FORM, XML_PROPERTIES):
            m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLControlProperty( m_rImport,xAttrList,m_xReportComponent);
            break;
        default:
            break;
    }

    return xContext;
}

void OXMLReportElementBase::endFastElement(sal_Int32 )
{
    try
    {
        if ( m_pContainer && m_pContainer->getSection().is() && m_xReportComponent.is() )
            m_pContainer->getSection()->add(m_xReportComponent);
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "Exception caught while inserting a new control!");
    }
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
