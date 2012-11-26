/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "XMLTextFrameContext.hxx"
#include "XMLTextFrameHyperlinkContext.hxx"

// OD 2004-04-21 #i26791#
#include <txtparaimphint.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

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
        return dynamic_cast< XMLTextFrameContext* >( pContext ) ->GetAnchorType();
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
        xTxt = dynamic_cast< XMLTextFrameContext* >( pContext )->GetTextContent();
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
        xShape = dynamic_cast< XMLTextFrameContext* >( pContext )->GetShape();
    }

    return xShape;
}
// <--


