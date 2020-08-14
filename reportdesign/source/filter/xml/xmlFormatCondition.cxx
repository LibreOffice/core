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
#include "xmlFormatCondition.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include "xmlHelper.hxx"
#include <sal/log.hxx>
#include <osl/diagnose.h>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
    using namespace ::com::sun::star::beans;

OXMLFormatCondition::OXMLFormatCondition( ORptFilter& rImport,
                const Reference< XFastAttributeList > & _xAttrList
                ,const Reference< XFormatCondition > & _xComponent ) :
    SvXMLImportContext( rImport )
,m_rImport(rImport)
,m_xComponent(_xComponent)
{

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    try
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            OUString sValue = aIter.toString();

            switch( aIter.getToken() )
            {
                case XML_ELEMENT(REPORT, XML_ENABLED):
                    m_xComponent->setEnabled(sValue == s_sTRUE);
                    break;
                case XML_ELEMENT(REPORT, XML_FORMULA):
                    m_xComponent->setFormula(ORptFilter::convertFormula(sValue));
                    break;
                case XML_ELEMENT(REPORT, XML_STYLE_NAME):
                    m_sStyleName = sValue;
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


OXMLFormatCondition::~OXMLFormatCondition()
{
}

void OXMLFormatCondition::endFastElement(sal_Int32 )
{
    OXMLHelper::copyStyleElements(m_rImport.isOldFormat(),m_sStyleName,GetImport().GetAutoStyles(),m_xComponent.get());
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
