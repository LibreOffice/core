/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "xmlimp.hxx"
#include "nmspmap.hxx"
#include "xmlnmspe.hxx"
#include "xmluconv.hxx"
#include "XMLTextFrameContext.hxx"
#include "XMLTextFrameHyperlinkContext.hxx"
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;

TYPEINIT1( XMLTextFrameHyperlinkContext, SvXMLImportContext );

XMLTextFrameHyperlinkContext::XMLTextFrameHyperlinkContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        TextContentAnchorType eATyp,
           Reference < XTextContent> *pTxtCntnt,
        TextContentAnchorType *pAnchrType ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    eAnchorType( eATyp ),
    pTextContent( pTxtCntnt ),
    pAnchorType( pAnchrType ),
    bMap( sal_False )
{
    OUString sShow;
    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextHyperlinkAttrTokenMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_HYPERLINK_HREF:
            sHRef = GetImport().GetAbsoluteReference( rValue );
            break;
        case XML_TOK_TEXT_HYPERLINK_NAME:
            sName = rValue;
            break;
        case XML_TOK_TEXT_HYPERLINK_TARGET_FRAME:
            sTargetFrameName = rValue;
            break;
        case XML_TOK_TEXT_HYPERLINK_SHOW:
            sShow = rValue;
            break;
        case XML_TOK_TEXT_HYPERLINK_SERVER_MAP:
            {
                sal_Bool bTmp;
                if( rImport.GetMM100UnitConverter().convertBool( bTmp,
                                                                  rValue ) )
                {
                    bMap = bTmp;	
                }
            }
            break;
        }
    }

    if( sShow.getLength() && !sTargetFrameName.getLength() )
    {
        if( IsXMLToken( sShow, XML_NEW ) )
            sTargetFrameName =
                    OUString( RTL_CONSTASCII_USTRINGPARAM("_blank" ) );
        else if( IsXMLToken( sShow, XML_REPLACE ) )
            sTargetFrameName =
                    OUString( RTL_CONSTASCII_USTRINGPARAM("_self" ) );
    }
}

XMLTextFrameHyperlinkContext::~XMLTextFrameHyperlinkContext()
{
}

void XMLTextFrameHyperlinkContext::EndElement()
{
}

SvXMLImportContext *XMLTextFrameHyperlinkContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    XMLTextFrameContext *pTextFrameContext = 0;

    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        sal_uInt16 nFrameType = USHRT_MAX;
        if( IsXMLToken( rLocalName, XML_TEXT_BOX ) )
            nFrameType = XML_TEXT_FRAME_TEXTBOX;
        else if( IsXMLToken( rLocalName, XML_IMAGE ) )
            nFrameType = XML_TEXT_FRAME_GRAPHIC;
        else if( IsXMLToken( rLocalName, XML_OBJECT ) )
            nFrameType = XML_TEXT_FRAME_OBJECT;
        else if( IsXMLToken( rLocalName, XML_OBJECT_OLE ) )
            nFrameType = XML_TEXT_FRAME_OBJECT_OLE;
        else if( IsXMLToken( rLocalName, XML_APPLET) )
            nFrameType = XML_TEXT_FRAME_APPLET;
        else if( IsXMLToken( rLocalName, XML_PLUGIN ) )
            nFrameType = XML_TEXT_FRAME_PLUGIN;
        else if( IsXMLToken( rLocalName, XML_FLOATING_FRAME ) )
            nFrameType = XML_TEXT_FRAME_FLOATING_FRAME;

        if( USHRT_MAX != nFrameType )
            pTextFrameContext = new XMLTextFrameContext( GetImport(), nPrefix,
                                                rLocalName, xAttrList, 
                                                eAnchorType,
                                                nFrameType );
    }

    if( pTextFrameContext )
    {
        pTextFrameContext->SetHyperlink( sHRef, sName, sTargetFrameName, bMap );
        if( pAnchorType )
            *pAnchorType = pTextFrameContext->GetAnchorType();
        if( pTextContent )
            *pTextContent = pTextFrameContext->GetTextContent();
        pContext = pTextFrameContext;
    }
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}



}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
