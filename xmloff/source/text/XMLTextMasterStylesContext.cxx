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

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

#include <xmloff/XMLTextMasterPageContext.hxx>
#include <xmloff/XMLTextMasterStylesContext.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_MASTER_PAGE;


bool XMLTextMasterStylesContext::InsertStyleFamily( XmlStyleFamily ) const
{
    return true;
}

XMLTextMasterStylesContext::XMLTextMasterStylesContext(
        SvXMLImport& rImport ) :
    SvXMLStylesContext( rImport )
{
}

XMLTextMasterStylesContext::~XMLTextMasterStylesContext()
{
}

SvXMLStyleContext *XMLTextMasterStylesContext::CreateStyleChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList > & xAttrList )
{
    SvXMLStyleContext *pContext = nullptr;

    if( nElement == XML_ELEMENT(STYLE, XML_MASTER_PAGE) &&
         InsertStyleFamily( XmlStyleFamily::MASTER_PAGE ) )
        pContext = new XMLTextMasterPageContext(
                        GetImport(), nElement,
                         xAttrList,
                        !GetImport().GetTextImport()->IsInsertMode() );

    // any other style will be ignored here!

    return pContext;
}

SvXMLStyleContext *XMLTextMasterStylesContext::CreateStyleStyleChildContext(
        XmlStyleFamily /*nFamily*/,
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const Reference< XAttributeList > & /*xAttrList*/ )
{
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
