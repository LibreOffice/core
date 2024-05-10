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
#include "xmlFormattedField.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace xml::sax;

OXMLFormattedField::OXMLFormattedField( ORptFilter& rImport
                ,const uno::Reference< xml::sax::XFastAttributeList > & _xAttrList
                ,const uno::Reference< XFormattedField > & _xComponent
                ,OXMLTable* _pContainer
                ,bool _bPageCount) :
    OXMLReportElementBase( rImport,_xComponent,_pContainer)
{
    OSL_ENSURE(m_xReportComponent.is(),"Component is NULL!");

    try
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            switch( aIter.getToken() )
            {
                case XML_ELEMENT(REPORT, XML_FORMULA):
                    _xComponent->setDataField(ORptFilter::convertFormula(aIter.toString()));
                    break;
                case XML_ELEMENT(REPORT, XML_SELECT_PAGE):
                    _xComponent->setDataField(u"rpt:PageNumber()"_ustr);
                    break;
                default:
                    XMLOFF_WARN_UNKNOWN("reportdesign", aIter);
                    break;
            }
        }
        if ( _bPageCount )
        {
            _xComponent->setDataField(u"rpt:PageCount()"_ustr);
        }
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "Exception caught while filling the report definition props");
    }
}

OXMLFormattedField::~OXMLFormattedField()
{
}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
