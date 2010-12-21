/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include "XMLReplacementImageContext.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::makeAny;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;

TYPEINIT1( XMLReplacementImageContext, SvXMLImportContext );

XMLReplacementImageContext::XMLReplacementImageContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & rAttrList,
        const Reference< XPropertySet > & rPropSet ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    m_xPropSet( rPropSet ),
    m_sGraphicURL(RTL_CONSTASCII_USTRINGPARAM("GraphicURL"))
{
    UniReference < XMLTextImportHelper > xTxtImport =
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
    OSL_ENSURE( m_sHRef.getLength() > 0 || m_xBase64Stream.is(),
                "neither URL nor base64 image data given" );
    UniReference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    OUString sHRef;
    if( m_sHRef.getLength() )
    {
        sal_Bool bForceLoad = xTxtImport->IsInsertMode() ||
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


