/*************************************************************************
 *
 *  $RCSfile: XMLTextFrameContext.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLANCHORTYPEPROPHDL_HXX
#include "XMLAnchorTypePropHdl.hxx"
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include "i18nmap.hxx"
#endif

#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextFrameContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

class XMLTextFrameDescContext_Impl : public SvXMLImportContext
{
    OUString&   rDesc;

public:

    TYPEINFO();

    XMLTextFrameDescContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                  const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            OUString& rD );
    virtual ~XMLTextFrameDescContext_Impl();

    virtual void Characters( const OUString& rText );
};

TYPEINIT1( XMLTextFrameDescContext_Impl, SvXMLImportContext );

XMLTextFrameDescContext_Impl::XMLTextFrameDescContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        OUString& rD  ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rDesc( rD )
{
}

XMLTextFrameDescContext_Impl::~XMLTextFrameDescContext_Impl()
{
}

void XMLTextFrameDescContext_Impl::Characters( const OUString& rText )
{
    rDesc += rText;
}

// ------------------------------------------------------------------------

TYPEINIT1( XMLTextFrameContext, SvXMLImportContext );

XMLTextFrameContext::XMLTextFrameContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        TextContentAnchorType eATyp,
        sal_uInt16 nType ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sWidth(RTL_CONSTASCII_USTRINGPARAM("Width")),
    sRelativeWidth(RTL_CONSTASCII_USTRINGPARAM("RelativeWidth")),
    sHeight(RTL_CONSTASCII_USTRINGPARAM("Height")),
    sRelativeHeight(RTL_CONSTASCII_USTRINGPARAM("RelativeHeight")),
    sSizeType(RTL_CONSTASCII_USTRINGPARAM("SizeType")),
    sSizeRelative(RTL_CONSTASCII_USTRINGPARAM("SizeRelative")),
    sHoriOrientPosition(RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition")),
    sVertOrientPosition(RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition")),
    sChainNextName(RTL_CONSTASCII_USTRINGPARAM("ChainNextName")),
    sAnchorType(RTL_CONSTASCII_USTRINGPARAM("AnchorType")),
    sAnchorPageNo(RTL_CONSTASCII_USTRINGPARAM("AnchorPageNo")),
    sGraphicURL(RTL_CONSTASCII_USTRINGPARAM("GraphicURL")),
    sGraphicFilter(RTL_CONSTASCII_USTRINGPARAM("GraphicFilter")),
    sAlternativeText(RTL_CONSTASCII_USTRINGPARAM("AlternativeText")),
    sFrameStyleName(RTL_CONSTASCII_USTRINGPARAM("FrameStyle")),
    sTextBoxServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFrame")),
    sGraphicServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Graphic"))
{
    OUString    sName;
    OUString    sStyleName;
    OUString    sChainNextName;
    OUString    sHRef;
    OUString    sFilterName;

    sal_Int32   nX = 0;
    sal_Int32   nY = 0;
    sal_Int32   nWidth = 0;
    sal_Int32   nMinWidth = 0;
    sal_Int32   nHeight = 0;
    sal_Int32   nMinHeight;
    sal_Int16   nPage = 0;

    TextContentAnchorType   eAnchorType = eATyp;

    sal_Bool    bRelWidth = sal_False;
    sal_Bool    bRelMinWidth = sal_False;
    sal_Bool    bRelHeight = sal_False;
    sal_Bool    bRelMinHeight = sal_False;
    UniReference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    const SvXMLTokenMap& rTokenMap =
        xTxtImport->GetTextFrameAttrTokenMap();

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
        case XML_TOK_TEXT_FRAME_STYLE_NAME:
            sStyleName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_NAME:
            sName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_ANCHOR_TYPE:
            if( TextContentAnchorType_AT_PARAGRAPH == eAnchorType ||
                TextContentAnchorType_AT_CHARACTER == eAnchorType ||
                TextContentAnchorType_AS_CHARACTER == eAnchorType )
            {
                TextContentAnchorType eNew;
                if( XMLAnchorTypePropHdl::convert( rValue,
                            GetImport().GetMM100UnitConverter(), eNew ) &&
                    ( TextContentAnchorType_AT_PARAGRAPH == eNew ||
                      TextContentAnchorType_AT_CHARACTER == eNew ||
                      TextContentAnchorType_AS_CHARACTER == eNew ) )
                    eAnchorType = eNew;
            }
            break;
        case XML_TOK_TEXT_FRAME_ANCHOR_PAGE_NUMBER:
            {
                sal_Int32 nTmp;
                   if( GetImport().GetMM100UnitConverter().
                                convertNumber( nTmp, rValue, 1, SHRT_MAX ) )
                    nPage = (sal_Int16)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_X:
            GetImport().GetMM100UnitConverter().convertMeasure( nX, rValue );
            break;
        case XML_TOK_TEXT_FRAME_Y:
            GetImport().GetMM100UnitConverter().convertMeasure( nY, rValue );
            break;
        case XML_TOK_TEXT_FRAME_WIDTH:
            bRelWidth = rValue.indexOf( '%' ) != -1;
            if( bRelWidth )
                GetImport().GetMM100UnitConverter().convertPercent( nWidth, rValue );
            else
                GetImport().GetMM100UnitConverter().convertMeasure( nWidth, rValue, 0 );
            break;
        case XML_TOK_TEXT_FRAME_MIN_WIDTH:
            bRelMinWidth = rValue.indexOf( '%' ) != -1;
            if( bRelMinWidth )
                GetImport().GetMM100UnitConverter().convertPercent( nMinWidth, rValue );
            else
                GetImport().GetMM100UnitConverter().convertMeasure( nMinWidth, rValue, 0 );
            break;
        case XML_TOK_TEXT_FRAME_HEIGHT:
            bRelHeight = rValue.indexOf( '%' ) != -1;
            if( bRelHeight )
                GetImport().GetMM100UnitConverter().convertPercent( nHeight, rValue );
            else
                GetImport().GetMM100UnitConverter().convertMeasure( nHeight, rValue, 0 );
            break;
        case XML_TOK_TEXT_FRAME_MIN_HEIGHT:
            bRelMinHeight = rValue.indexOf( '%' ) != -1;
            if( bRelMinHeight )
                GetImport().GetMM100UnitConverter().convertPercent( nMinHeight, rValue );
            else
                GetImport().GetMM100UnitConverter().convertMeasure( nMinHeight, rValue, 0 );
            break;
        case XML_TOK_TEXT_FRAME_NEXT_CHAIN_NAME:
            sChainNextName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_HREF:
            sHRef = rValue;
            break;
        case XML_TOK_TEXT_FRAME_FILTER_NAME:
            sFilterName = rValue;
            break;
        }
    }

    Reference<XMultiServiceFactory> xFactory( GetImport().GetModel(),
                                              UNO_QUERY );
    if( xFactory.is() )
    {
        OUString sServiceName;
        switch( nType )
        {
        case XML_TEXT_FRAME_TEXTBOX: sServiceName = sTextBoxServiceName; break;
        case XML_TEXT_FRAME_GRAPHIC: sServiceName = sGraphicServiceName; break;
        }
        Reference<XInterface> xIfc = xFactory->createInstance( sServiceName );
        DBG_ASSERT( xIfc.is(), "couldn't create frame" );
        if( xIfc.is() )
            xPropSet = Reference < XPropertySet >( xIfc, UNO_QUERY );
    }

    if( !xPropSet.is() )
        return;

    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    // set name
    if( sName.getLength() )
    {
        Reference < XNamed > xNamed( xPropSet, UNO_QUERY );
        if( xNamed.is() )
        {
            OUString sOldName( sName );
            sal_Int32 i = 0;
            while( xTxtImport->HasFrameByName( sName ) )
            {
                sName = sOldName;
                sName += OUString::valueOf( ++i );
            }
            xNamed->setName( sName );
            if( sName != sOldName )
                xTxtImport->GetRenameMap().Add( XML_TEXT_RENAME_TYPE_FRAME,
                                             sOldName, sName );
        }
    }

    // frame style
    XMLPropStyleContext *pStyle = 0;
    if( sStyleName.getLength() )
    {
        pStyle = xTxtImport->FindAutoFrameStyle( sStyleName );
        if( pStyle )
            sStyleName = pStyle->GetParent();
    }

    Any aAny;
    if( sStyleName.getLength() )
    {
        const Reference < XNameContainer > & rStyles =
            xTxtImport->GetFrameStyles();
        if( rStyles.is() &&
            rStyles->hasByName( sStyleName ) )
        {
            aAny <<= sStyleName;
            xPropSet->setPropertyValue( sFrameStyleName, aAny );
        }
    }

    // hard properties
    if( pStyle )
        pStyle->FillPropertySet( xPropSet );

    // anchor type
    aAny <<= eAnchorType;
    xPropSet->setPropertyValue( sAnchorType, aAny );


    // x and y
    aAny <<= nX;
    xPropSet->setPropertyValue( sHoriOrientPosition, aAny );
    aAny <<= nY;
    xPropSet->setPropertyValue( sVertOrientPosition, aAny );

    // width
    if( !nWidth )
    {
        nWidth = nMinWidth;
        bRelWidth = bRelMinWidth;
    }
    sal_Bool bMinHeight = sal_False;
    if( !nHeight )
    {
        nHeight = nMinHeight;
        bRelHeight = bRelMinHeight;
        bMinHeight = sal_True;
    }
    if( nWidth )
    {
        if( bRelWidth > 0 )
        {
            aAny <<= (sal_Int16)nWidth;
            xPropSet->setPropertyValue( sRelativeWidth, aAny );
        }
        else
        {
            aAny <<= nWidth;
            xPropSet->setPropertyValue( sWidth, aAny );
        }
    }
    else if( XML_TEXT_FRAME_GRAPHIC == nType && nHeight > 0 )
    {
        // TODO: synchronize width to height
    }

    if( nHeight > 0 )
    {
        if( xPropSetInfo->hasPropertyByName( sSizeType ) )
        {
            sal_Int16 nSizeType =
                (bMinHeight && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN
                                                                : SizeType::FIX;
            aAny <<= nSizeType;
            xPropSet->setPropertyValue( sSizeType, aAny );
        }
        if( bRelHeight > 0 )
        {
            aAny <<= (sal_Int16)nHeight;
            xPropSet->setPropertyValue( sRelativeHeight, aAny );
        }
        else
        {
            aAny <<= nHeight;
            xPropSet->setPropertyValue( sHeight, aAny );
        }
    }
    else if( XML_TEXT_FRAME_GRAPHIC == nType && nWidth > 0 )
    {
        // synchroize height to width
        sal_Bool bTmp = sal_True;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sSizeRelative, aAny );
    }

    if( XML_TEXT_FRAME_GRAPHIC == nType )
    {
        // URL
        aAny <<=sHRef;
        xPropSet->setPropertyValue( sGraphicURL, aAny );

        // filter name
        aAny <<=sFilterName;
        xPropSet->setPropertyValue( sGraphicFilter, aAny );
    }

    Reference < XTextContent > xTxtCntnt( xPropSet, UNO_QUERY );
    xTxtImport->InsertTextContent( xTxtCntnt );

    // page number (must be set after the frame is inserted, because it
    // will be overwritten then inserting the frame.
    if( TextContentAnchorType_AT_PAGE == eAnchorType )
    {
        aAny <<= nPage;
        xPropSet->setPropertyValue( sAnchorPageNo, aAny );
    }

    if( XML_TEXT_FRAME_TEXTBOX == nType )
    {
        xTxtImport->ConnectFrameChains( sName, sChainNextName, xPropSet );
        Reference < XTextFrame > xTxtFrame( xPropSet, UNO_QUERY );
        Reference < XText > xTxt = xTxtFrame->getText();
        xOldTextCursor = xTxtImport->GetCursor();
        xTxtImport->SetCursor( xTxt->createTextCursor() );
    }

}

XMLTextFrameContext::~XMLTextFrameContext()
{
}

void XMLTextFrameContext::EndElement()
{
    // alternative text
    if( sDesc.getLength() && xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( sAlternativeText ) )
        {
            Any aAny;
            aAny <<= sDesc;
            xPropSet->setPropertyValue( sAlternativeText, aAny );
        }
    }

    if( xOldTextCursor.is() )
    {
        if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, sal_True ) )
        {
            OUString sEmpty;
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), sEmpty,
                sal_True );
        }
        GetImport().GetTextImport()->SetCursor( xOldTextCursor );
    }
}

SvXMLImportContext *XMLTextFrameContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_OFFICE == nPrefix &&
        rLocalName.equalsAsciiL( sXML_desc, sizeof(sXML_desc)-1 ) )
        pContext = new XMLTextFrameDescContext_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, sDesc );
    else if( xOldTextCursor.is() )  // text-box
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
                            GetImport(), nPrefix, rLocalName, xAttrList,
                            XML_TEXT_TYPE_TEXTBOX );

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}



