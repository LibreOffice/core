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

#include <SchXMLCalculationSettingsContext.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>



using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

SchXMLCalculationSettingsContext::SchXMLCalculationSettingsContext( SvXMLImport& rImport,
                                    sal_uInt16 p_nPrefix,
                                    const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
: SvXMLImportContext ( rImport, p_nPrefix, rLocalName )
{
    const SvXMLNamespaceMap& rMap = GetImport().GetNamespaceMap();
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName(sAttrName, &aLocalName );
        if ( nPrefix == XML_NAMESPACE_TABLE && IsXMLToken( aLocalName, XML_DATE_VALUE ) )
        {
            util::DateTime aNullDate;
            const OUString sValue = xAttrList->getValueByIndex( i );
            ::sax::Converter::parseDateTime(aNullDate, 0, sValue);
            m_aNullDate <<= aNullDate;
        }
    }
}
SvXMLImportContext* SchXMLCalculationSettingsContext::CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    return new SchXMLCalculationSettingsContext(GetImport(),nPrefix,rLocalName,xAttrList);
}

void SchXMLCalculationSettingsContext::EndElement()
{
    if ( m_aNullDate.hasValue() )
    {
        Reference < XPropertySet > xPropSet ( GetImport().GetModel(), UNO_QUERY );
        OUString sNullDate( "NullDate" );
        xPropSet->setPropertyValue ( sNullDate, m_aNullDate );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
