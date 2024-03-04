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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <osl/diagnose.h>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <XMLReplacementImageContext.hxx>
#include <sal/log.hxx>

using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;
using namespace css;
using namespace ::xmloff::token;

XMLReplacementImageContext::XMLReplacementImageContext(
        SvXMLImport& rImport,
        sal_Int32 /*nElement*/,
        const Reference< XFastAttributeList > & rAttrList,
        const Reference< XPropertySet > & rPropSet ) :
    SvXMLImportContext( rImport ),
    m_xPropSet( rPropSet )
{
    m_sHRef = rAttrList->getOptionalValue(XML_ELEMENT(XLINK, XML_HREF));
}

XMLReplacementImageContext::~XMLReplacementImageContext()
{
}

void XMLReplacementImageContext::endFastElement(sal_Int32 )
{
    OSL_ENSURE( !m_sHRef.isEmpty() || m_xBase64Stream.is(),
                "neither URL nor base64 image data given" );
    uno::Reference<graphic::XGraphic> xGraphic;

    try
    {
        if( !m_sHRef.isEmpty() )
        {
            xGraphic = GetImport().loadGraphicByURL(m_sHRef);
        }
        else if( m_xBase64Stream.is() )
        {
            xGraphic = GetImport().loadGraphicFromBase64(m_xBase64Stream);
            m_xBase64Stream = nullptr;
        }
    }
    catch (uno::Exception const &)
    {}

    Reference < XPropertySetInfo > xPropSetInfo = m_xPropSet->getPropertySetInfo();

    if (xGraphic.is() && xPropSetInfo->hasPropertyByName("Graphic"))
    {
        m_xPropSet->setPropertyValue("Graphic", uno::Any(xGraphic));
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLReplacementImageContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if( nElement == XML_ELEMENT(OFFICE, XML_BINARY_DATA) &&
        !m_xBase64Stream.is() )
    {
        m_xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
        if( m_xBase64Stream.is() )
            return new XMLBase64ImportContext( GetImport(),
                                                    m_xBase64Stream );
    }

    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
