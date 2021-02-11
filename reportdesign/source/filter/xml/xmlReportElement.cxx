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
#include "xmlReportElement.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include "xmlComponent.hxx"
#include "xmlCondPrtExpr.hxx"
#include "xmlFormatCondition.hxx"
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace report;
    using namespace uno;
    using namespace xml::sax;

OXMLReportElement::OXMLReportElement( ORptFilter& rImport,
                const Reference< XFastAttributeList > & _xAttrList
                ,const Reference< XReportControlModel > & _xComponent) :
    SvXMLImportContext( rImport )
,m_xComponent(_xComponent)
{

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");

    try
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            switch( aIter.getToken() )
            {
                case XML_ELEMENT(REPORT, XML_PRINT_WHEN_GROUP_CHANGE):
                    m_xComponent->setPrintWhenGroupChange(IsXMLToken(aIter, XML_TRUE));
                    break;
                case XML_ELEMENT(REPORT, XML_PRINT_REPEATED_VALUES):
                    m_xComponent->setPrintRepeatedValues(IsXMLToken(aIter, XML_TRUE));
                    break;
                default:
                    XMLOFF_WARN_UNKNOWN("reportdesign", aIter);
                    break;
            }
        }
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "Exception caught while filling the report definition props");
    }
}


OXMLReportElement::~OXMLReportElement()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLReportElement::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;
    ORptFilter& rImport = GetOwnImport();

    switch( nElement )
    {
        case XML_ELEMENT(REPORT, XML_REPORT_COMPONENT):
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLComponent( rImport,xAttrList,m_xComponent);
            break;
        case XML_ELEMENT(REPORT, XML_CONDITIONAL_PRINT_EXPRESSION):
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLCondPrtExpr( rImport,xAttrList,m_xComponent);
            break;
        case XML_ELEMENT(REPORT, XML_FORMAT_CONDITION):
            {
                uno::Reference< report::XFormatCondition > xNewCond = m_xComponent->createFormatCondition();
                m_xComponent->insertByIndex(m_xComponent->getCount(),uno::makeAny(xNewCond));
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                xContext = new OXMLFormatCondition( rImport,xAttrList,xNewCond);
            }
            break;
        default:
            break;
    }

    return xContext;
}

ORptFilter& OXMLReportElement::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
