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

#include "SchXMLCalculationSettingsContext.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

SchXMLCalculationSettingsContext::SchXMLCalculationSettingsContext( SvXMLImport& rImport,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
: SvXMLImportContext ( rImport )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if ( aIter.getToken() == XML_ELEMENT(TABLE, XML_DATE_VALUE) )
        {
            util::DateTime aNullDate;
            if (::sax::Converter::parseDateTime(aNullDate, aIter.toView()))
                m_aNullDate <<= aNullDate;
            else
                SAL_WARN("xmloff", "SchXMLCalculationSettingsContext: broken DateTime '" << aIter.toView() << "'");
        }
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLCalculationSettingsContext::createFastChildContext(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return new SchXMLCalculationSettingsContext(GetImport(),xAttrList);
}

void SchXMLCalculationSettingsContext::endFastElement(sal_Int32 )
{
    if ( m_aNullDate.hasValue() )
    {
        Reference < XPropertySet > xPropSet ( GetImport().GetModel(), UNO_QUERY );
        xPropSet->setPropertyValue ( "NullDate", m_aNullDate );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
