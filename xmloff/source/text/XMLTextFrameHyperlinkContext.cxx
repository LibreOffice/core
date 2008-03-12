/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextFrameHyperlinkContext.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:02:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLTEXTFRAMECONTEXT_HXX
#include "XMLTextFrameContext.hxx"
#endif
#ifndef _XMLTEXTFRAMEHYPERLINKCONTEXT_HXX
#include "XMLTextFrameHyperlinkContext.hxx"
#endif

// OD 2004-04-21 #i26791#
#ifndef _XMLOFF_TXTPARAIMPHINT_HXX
#include <txtparaimphint.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

TYPEINIT1( XMLTextFrameHyperlinkContext, SvXMLImportContext );

XMLTextFrameHyperlinkContext::XMLTextFrameHyperlinkContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        TextContentAnchorType eATyp ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    eDefaultAnchorType( eATyp ),
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
        if( IsXMLToken( rLocalName, XML_FRAME ) )
            pTextFrameContext = new XMLTextFrameContext( GetImport(), nPrefix,
                                                rLocalName, xAttrList,
                                                eDefaultAnchorType );
    }

    if( pTextFrameContext )
    {
        pTextFrameContext->SetHyperlink( sHRef, sName, sTargetFrameName, bMap );
        pContext = pTextFrameContext;
        xFrameContext = pContext;
    }
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}


TextContentAnchorType XMLTextFrameHyperlinkContext::GetAnchorType() const
{
    if( xFrameContext.Is() )
    {
        SvXMLImportContext *pContext = &xFrameContext;
        return PTR_CAST( XMLTextFrameContext, pContext ) ->GetAnchorType();
    }
    else
        return eDefaultAnchorType;

}

Reference < XTextContent > XMLTextFrameHyperlinkContext::GetTextContent() const
{
    Reference <XTextContent > xTxt;
    if( xFrameContext.Is() )
    {
        SvXMLImportContext *pContext = &xFrameContext;
        xTxt = PTR_CAST( XMLTextFrameContext, pContext )->GetTextContent();
    }

    return xTxt;
}

// --> OD 2004-08-24 #33242#
Reference < drawing::XShape > XMLTextFrameHyperlinkContext::GetShape() const
{
    Reference < drawing::XShape > xShape;
    if( xFrameContext.Is() )
    {
        SvXMLImportContext *pContext = &xFrameContext;
        xShape = PTR_CAST( XMLTextFrameContext, pContext )->GetShape();
    }

    return xShape;
}
// <--


