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
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLFormatCondition::OXMLFormatCondition( ORptFilter& rImport,
                const Reference< XFastAttributeList > & _xAttrList
                ,const Reference< XFormatCondition > & _xComponent ) :
    SvXMLImportContext( rImport )
,m_rImport(rImport)
,m_xComponent(_xComponent)
{

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    try
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            switch( aIter.getToken() )
            {
                case XML_ELEMENT(REPORT, XML_ENABLED):
                    m_xComponent->setEnabled(IsXMLToken(aIter, XML_TRUE));
                    break;
                case XML_ELEMENT(REPORT, XML_FORMULA):
                    m_xComponent->setFormula(ORptFilter::convertFormula(aIter.toString()));
                    break;
                case XML_ELEMENT(REPORT, XML_STYLE_NAME):
                    m_sStyleName = aIter.toString();
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


OXMLFormatCondition::~OXMLFormatCondition()
{
}

void OXMLFormatCondition::endFastElement(sal_Int32 )
{
    OXMLHelper::copyStyleElements(m_rImport.isOldFormat(),m_sStyleName,GetImport().GetAutoStyles(),m_xComponent);
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
