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
#include "xmlCondPrtExpr.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <strings.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace report;
    using namespace uno;
    using namespace xml::sax;

OXMLCondPrtExpr::OXMLCondPrtExpr( ORptFilter& _rImport
                ,const uno::Reference< xml::sax::XFastAttributeList > & _xAttrList
                ,const Reference< XPropertySet > & _xComponent ) :
    SvXMLImportContext( _rImport )
,m_xComponent(_xComponent)
{
    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    try
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            OUString sValue = aIter.toString();

            switch( aIter.getToken() )
            {
                case XML_ELEMENT(REPORT, XML_FORMULA):
                    m_xComponent->setPropertyValue(PROPERTY_CONDITIONALPRINTEXPRESSION,uno::makeAny(ORptFilter::convertFormula(sValue)));
                    break;
                default:
                    SAL_WARN("reportdesign", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << " = " << sValue);
                    break;
            }

        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("Exception caught while putting Function props!");
    }
}


OXMLCondPrtExpr::~OXMLCondPrtExpr()
{
}


void OXMLCondPrtExpr::characters( const OUString& rChars )
{
    m_aCharBuffer.append(rChars);
}

void OXMLCondPrtExpr::endFastElement( sal_Int32 )
{
    if (m_aCharBuffer.getLength())
        m_xComponent->setPropertyValue(PROPERTY_CONDITIONALPRINTEXPRESSION,makeAny(m_aCharBuffer.makeStringAndClear()));
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
