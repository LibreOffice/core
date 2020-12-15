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


#include "XMLCalculationSettingsContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <sal/log.hxx>
#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

XMLCalculationSettingsContext::XMLCalculationSettingsContext( SvXMLImport& rImport,
                                    sal_Int32 /*nElement*/,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
: SvXMLImportContext ( rImport )
, nYear( 1930 )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if (aIter.getToken() == XML_ELEMENT(TABLE, XML_NULL_YEAR) )
        {
            nYear = static_cast <sal_Int16> (aIter.toInt32());
        }
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }
}

XMLCalculationSettingsContext::~XMLCalculationSettingsContext()
{
}
void XMLCalculationSettingsContext::endFastElement(sal_Int32 )
{
    if (nYear != 1930 )
    {
        Reference < XTextDocument > xTextDoc ( GetImport().GetModel(), UNO_QUERY);
        if (xTextDoc.is())
        {
            Reference < XPropertySet > xPropSet ( xTextDoc, UNO_QUERY );
            xPropSet->setPropertyValue ( "TwoDigitYear", Any(nYear) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
