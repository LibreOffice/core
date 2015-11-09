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

#include <xmloff/xmlnmspe.hxx>
#include "xmlimp.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

class SwXMLBodyContentContext_Impl : public SvXMLImportContext
{
    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:

    SwXMLBodyContentContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                             const OUString& rLName );
    virtual ~SwXMLBodyContentContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList ) override;

    // The body element's text:global attribute can be ignored, because
    // we must have the correct object shell already.
    virtual void EndElement() override;
};

SwXMLBodyContentContext_Impl::SwXMLBodyContentContext_Impl( SwXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SwXMLBodyContentContext_Impl::~SwXMLBodyContentContext_Impl()
{
}

SvXMLImportContext *SwXMLBodyContentContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    pContext = GetSwImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList,
               XML_TEXT_TYPE_BODY );
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void SwXMLBodyContentContext_Impl::EndElement()
{
    /* Code moved to SwXMLOmport::endDocument */
    GetImport().GetTextImport()->SetOutlineStyles( false );
}

SvXMLImportContext *SwXMLImport::CreateBodyContentContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    if( !IsStylesOnlyMode() )
         pContext = new SwXMLBodyContentContext_Impl( *this, XML_NAMESPACE_OFFICE,
                                              rLocalName );
    else
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
                                           rLocalName );

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
