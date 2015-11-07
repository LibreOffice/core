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
#include <com/sun/star/io/XOutputStream.hpp>
#include <osl/diagnose.h>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include "XMLReplacementImageContext.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::makeAny;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;


XMLReplacementImageContext::XMLReplacementImageContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & rAttrList,
        const Reference< XPropertySet > & rPropSet ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    m_xPropSet( rPropSet ),
    m_sGraphicURL("GraphicURL")
{
    rtl::Reference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    const SvXMLTokenMap& rTokenMap =
        xTxtImport->GetTextFrameAttrTokenMap();

    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = rAttrList->getNameByIndex( i );
        const OUString& rValue = rAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_FRAME_HREF:
            m_sHRef = rValue;
            break;
        }
    }
}

XMLReplacementImageContext::~XMLReplacementImageContext()
{
}

void XMLReplacementImageContext::EndElement()
{
    OSL_ENSURE( !m_sHRef.isEmpty() || m_xBase64Stream.is(),
                "neither URL nor base64 image data given" );
    rtl::Reference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    OUString sHRef;
    if( !m_sHRef.isEmpty() )
    {
        bool bForceLoad = xTxtImport->IsInsertMode() ||
                              xTxtImport->IsBlockMode() ||
                              xTxtImport->IsStylesOnlyMode() ||
                              xTxtImport->IsOrganizerMode();
        sHRef = GetImport().ResolveGraphicObjectURL( m_sHRef, !bForceLoad );
    }
    else if( m_xBase64Stream.is() )
    {
        sHRef = GetImport().ResolveGraphicObjectURLFromBase64( m_xBase64Stream );
        m_xBase64Stream = 0;
    }

    Reference < XPropertySetInfo > xPropSetInfo =
        m_xPropSet->getPropertySetInfo();
    if( xPropSetInfo->hasPropertyByName( m_sGraphicURL ) )
        m_xPropSet->setPropertyValue( m_sGraphicURL, makeAny( sHRef ) );
}

SvXMLImportContext *XMLReplacementImageContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_OFFICE == nPrefix &&
        IsXMLToken( rLocalName, ::xmloff::token::XML_BINARY_DATA ) &&
        !m_xBase64Stream.is() )
    {
        m_xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
        if( m_xBase64Stream.is() )
            pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    m_xBase64Stream );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
