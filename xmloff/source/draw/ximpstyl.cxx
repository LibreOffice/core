/*************************************************************************
 *
 *  $RCSfile: ximpstyl.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:27:42 $
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

#pragma hdrstop

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#include "XMLShapeStyleContext.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XIMPNOTES_HXX
#include "ximpnote.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONPAGE_HPP_
#include <com/sun/star/presentation/XPresentationPage.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGES_HPP_
#include <com/sun/star/drawing/XDrawPages.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_XHANDOUTMASTERSUPPLIER_HPP_
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#endif

// #110680#
//#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
//#include <comphelper/processfactory.hxx>
//#endif

#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include "xmlprcon.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _XMLOFF_LAYERIMP_HXX
#include "layerimp.hxx"
#endif

#ifndef _XMLOFF_XMLGRAPHICSDEFAULTSTYLE_HXX
#include "XMLGraphicsDefaultStyle.hxx"
#endif

#ifndef _XMLOFF_NUMBERSTYLESIMPORT_HXX
#include "XMLNumberStylesImport.hxx"
#endif

#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

//////////////////////////////////////////////////////////////////////////////

class SdXMLDrawingPagePropertySetContext : public SvXMLPropertySetContext
{
public:

    TYPEINFO();

    SdXMLDrawingPagePropertySetContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName,
                 const ::com::sun::star::uno::Reference<
                         ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap );

    virtual ~SdXMLDrawingPagePropertySetContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                                   ::std::vector< XMLPropertyState > &rProperties,
                                   const XMLPropertyState& rProp);
};

TYPEINIT1( SdXMLDrawingPagePropertySetContext, SvXMLPropertySetContext );

SdXMLDrawingPagePropertySetContext::SdXMLDrawingPagePropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const uno::Reference< xml::sax::XAttributeList > & xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap ) :
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList,
                             XML_TYPE_PROP_DRAWING_PAGE, rProps, rMap )
{
}

SdXMLDrawingPagePropertySetContext::~SdXMLDrawingPagePropertySetContext()
{
}

SvXMLImportContext *SdXMLDrawingPagePropertySetContext::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const uno::Reference< xml::sax::XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContext *pContext = 0;

    switch( xMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_PAGE_SOUND_URL:
    {
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(xAttrList->getNameByIndex(i), &aLocalName);

            if( (nPrefix == XML_NAMESPACE_XLINK) && IsXMLToken( aLocalName, XML_HREF ) )
            {
                uno::Any aAny;
                aAny <<= GetImport().GetAbsoluteReference( xAttrList->getValueByIndex(i) );
                XMLPropertyState aPropState( rProp.mnIndex, aAny );
                rProperties.push_back( aPropState );
            }
        }
        break;
    }
    }

    if( !pContext )
        pContext = SvXMLPropertySetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList,
                                                            rProperties, rProp );

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

class SdXMLDrawingPageStyleContext : public XMLPropStyleContext
{
public:
    TYPEINFO();

    SdXMLDrawingPageStyleContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        SvXMLStylesContext& rStyles,
        sal_uInt16 nFamily = XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID);
    virtual ~SdXMLDrawingPageStyleContext();

    SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void Finish( sal_Bool bOverwrite );
};

TYPEINIT1( SdXMLDrawingPageStyleContext, XMLPropStyleContext );

SdXMLDrawingPageStyleContext::SdXMLDrawingPageStyleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    SvXMLStylesContext& rStyles,
    sal_uInt16 nFamily)
:   XMLPropStyleContext(rImport, nPrfx, rLName, xAttrList, rStyles, nFamily )
{
}

SdXMLDrawingPageStyleContext::~SdXMLDrawingPageStyleContext()
{
}

SvXMLImportContext *SdXMLDrawingPageStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_DRAWING_PAGE_PROPERTIES ) )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
            pContext = new SdXMLDrawingPagePropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    GetProperties(),
                                                    xImpPrMap );
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void SdXMLDrawingPageStyleContext::Finish( sal_Bool bOverwrite )
{
    XMLPropStyleContext::Finish( bOverwrite );

    ::std::vector< XMLPropertyState > &rProperties = GetProperties();

    const UniReference< XMLPropertySetMapper >& rImpPrMap = GetStyles()->GetImportPropertyMapper( GetFamily() )->getPropertySetMapper();

    ::std::vector< XMLPropertyState >::iterator property = rProperties.begin();
    for (property; property != rProperties.end(); property++)
    {
        if( property->mnIndex == -1 )
            continue;

        sal_Int16 nContextID = rImpPrMap->GetEntryContextId(property->mnIndex);
        switch( nContextID )
        {
            case CTF_DATE_TIME_FORMAT:
            {
                OUString sStyleName;
                (*property).maValue >>= sStyleName;

                sal_Int32 nStyle = 0;

                SdXMLNumberFormatImportContext* pSdNumStyle =
                    PTR_CAST( SdXMLNumberFormatImportContext,
                        GetStyles()->FindStyleChildContext( XML_STYLE_FAMILY_DATA_STYLE, sStyleName, sal_True ) );

                if( pSdNumStyle )
                    nStyle = pSdNumStyle->GetDrawKey();

                (*property).maValue <<= nStyle;
            }
            break;
        }
    }

}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPageMasterStyleContext, SvXMLStyleContext );

SdXMLPageMasterStyleContext::SdXMLPageMasterStyleContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_SD_PAGEMASTERSTYLECONEXT_ID),
    mnBorderBottom( 0L ),
    mnBorderLeft( 0L ),
    mnBorderRight( 0L ),
    mnBorderTop( 0L ),
    mnWidth( 0L ),
    mnHeight( 0L ),
    meOrientation(GetSdImport().IsDraw() ? view::PaperOrientation_PORTRAIT : view::PaperOrientation_LANDSCAPE)
{
    // set family to something special at SvXMLStyleContext
    // for differences in search-methods

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetPageMasterStyleAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_PAGEMASTERSTYLE_MARGIN_TOP:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnBorderTop, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_MARGIN_BOTTOM:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnBorderBottom, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_MARGIN_LEFT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnBorderLeft, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_MARGIN_RIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnBorderRight, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_PAGE_WIDTH:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_PAGE_HEIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_PAGE_ORIENTATION:
            {
                if( IsXMLToken( sValue, XML_PORTRAIT ) )
                    meOrientation = view::PaperOrientation_PORTRAIT;
                else
                    meOrientation = view::PaperOrientation_LANDSCAPE;
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPageMasterStyleContext::~SdXMLPageMasterStyleContext()
{
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPageMasterContext, SvXMLStyleContext );

SdXMLPageMasterContext::SdXMLPageMasterContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_SD_PAGEMASTERCONEXT_ID),
    mpPageMasterStyle( 0L )
{
    // set family to something special at SvXMLStyleContext
    // for differences in search-methods

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetPageMasterAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_PAGEMASTER_NAME:
            {
                msName = sValue;
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPageMasterContext::~SdXMLPageMasterContext()
{
    // release remembered contexts, they are no longer needed
    if(mpPageMasterStyle)
    {
        mpPageMasterStyle->ReleaseRef();
        mpPageMasterStyle = 0L;
    }
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLPageMasterContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    if(nPrefix == XML_NAMESPACE_STYLE && IsXMLToken( rLocalName, XML_PAGE_LAYOUT_PROPERTIES) )
    {
        pContext = new SdXMLPageMasterStyleContext(GetSdImport(), nPrefix, rLocalName, xAttrList);

        // remember SdXMLPresentationPlaceholderContext for later evaluation
        if(pContext)
        {
            pContext->AddRef();
            DBG_ASSERT(!mpPageMasterStyle, "PageMasterStyle is set, there seem to be two of them (!)");
            mpPageMasterStyle = (SdXMLPageMasterStyleContext*)pContext;
        }
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStyleContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPresentationPageLayoutContext, SvXMLStyleContext );

SdXMLPresentationPageLayoutContext::SdXMLPresentationPageLayoutContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_SD_PRESENTATIONPAGELAYOUT_ID),
    mnTypeId( 20 ) // AUTOLAYOUT_NONE
{
    // set family to somethiong special at SvXMLStyleContext
    // for differences in search-methods

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );

        if(nPrefix == XML_NAMESPACE_STYLE && IsXMLToken( aLocalName, XML_NAME ) )
        {
            msName = xAttrList->getValueByIndex( i );
        }
    }
}

SdXMLPresentationPageLayoutContext::~SdXMLPresentationPageLayoutContext()
{
}

SvXMLImportContext *SdXMLPresentationPageLayoutContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    if(nPrefix == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_PLACEHOLDER ) )
    {
        // presentation:placeholder inside style:presentation-page-layout context
        pContext = new SdXMLPresentationPlaceholderContext(
            GetSdImport(), nPrefix, rLocalName, xAttrList);

        // remember SdXMLPresentationPlaceholderContext for later evaluation
        if(pContext)
        {
            pContext->AddRef();
            maList.Insert((SdXMLPresentationPlaceholderContext*)pContext, LIST_APPEND);
        }
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStyleContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

void SdXMLPresentationPageLayoutContext::EndElement()
{
    // build presentation page layout type here
    // calc mnTpeId due to content of maList
    // at the moment only use number of types used there
    if(maList.Count())
    {
        SdXMLPresentationPlaceholderContext* pObj0 = maList.GetObject(0);
        if(pObj0->GetName().equals(OUString(RTL_CONSTASCII_USTRINGPARAM("handout"))))
        {
            switch( maList.Count() )
            {
            case 1:
                mnTypeId = 22; // AUTOLAYOUT_HANDOUT1
                break;
            case 2:
                mnTypeId = 23; // AUTOLAYOUT_HANDOUT2
                break;
            case 3:
                mnTypeId = 24; // AUTOLAYOUT_HANDOUT3
                break;
            case 4:
                mnTypeId = 25; // AUTOLAYOUT_HANDOUT4
                break;
            default:
                mnTypeId = 26; // AUTOLAYOUT_HANDOUT6
            }
        }
        else
        {
            switch(maList.Count())
            {
                case 1:
                {
                    mnTypeId = 19; // AUTOLAYOUT_ONLY_TITLE
                    break;
                }
                case 2:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList.GetObject(1);

                    if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("subtitle"))))
                    {
                        mnTypeId = 0; // AUTOLAYOUT_TITLE
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("outline"))))
                    {
                        mnTypeId = 1; // AUTOLAYOUT_ENUM
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("chart"))))
                    {
                        mnTypeId = 2; // AUTOLAYOUT_CHART
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("table"))))
                    {
                        mnTypeId = 8; // AUTOLAYOUT_TAB
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("object"))))
                    {
                        mnTypeId = 11; // AUTOLAYOUT_OBJ
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("vertical_outline"))))
                    {
                        if(pObj0->GetName().equals(OUString(RTL_CONSTASCII_USTRINGPARAM("vertical_title"))))
                        {
                            // AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE
                            mnTypeId = 28;
                        }
                        else
                        {
                            // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE
                            mnTypeId = 29;
                        }
                    }
                    else
                    {
                        mnTypeId = 21; // AUTOLAYOUT_NOTES
                    }
                    break;
                }
                case 3:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList.GetObject(1);
                    SdXMLPresentationPlaceholderContext* pObj2 = maList.GetObject(2);

                    if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("outline"))))
                    {
                        if(pObj2->GetName().equals(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("outline"))))
                        {
                            mnTypeId = 3; // AUTOLAYOUT_2TEXT
                        }
                        else if(pObj2->GetName().equals(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("chart"))))
                        {
                            mnTypeId = 4; // AUTOLAYOUT_TEXTCHART
                        }
                        else if(pObj2->GetName().equals(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("graphic"))))
                        {
                            mnTypeId = 6; // AUTOLAYOUT_TEXTCLIP
                        }
                        else
                        {
                            if(pObj1->GetX() < pObj2->GetX())
                            {
                                mnTypeId = 10; // AUTOLAYOUT_TEXTOBJ -> outline left, object right
                            }
                            else
                            {
                                mnTypeId = 17; // AUTOLAYOUT_TEXTOVEROBJ -> outline top, object right
                            }
                        }
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("chart"))))
                    {
                        mnTypeId = 7; // AUTOLAYOUT_CHARTTEXT
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("graphic"))))
                    {
                        if(pObj2->GetName().equals(OUString(RTL_CONSTASCII_USTRINGPARAM("vertical_outline"))))
                        {
                            // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART
                            mnTypeId = 30;
                        }
                        else
                        {
                            mnTypeId = 9; // AUTOLAYOUT_CLIPTEXT
                        }
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("vertical_outline"))))
                    {
                        // AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART
                        mnTypeId = 27;
                    }
                    else
                    {
                        if(pObj1->GetX() < pObj2->GetX())
                        {
                            mnTypeId = 13; // AUTOLAYOUT_OBJTEXT -> left, right
                        }
                        else
                        {
                            mnTypeId = 14; // AUTOLAYOUT_OBJOVERTEXT -> top, bottom
                        }
                    }
                    break;
                }
                case 4:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList.GetObject(1);
                    SdXMLPresentationPlaceholderContext* pObj2 = maList.GetObject(2);

                    if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("object"))))
                    {
                        if(pObj1->GetX() < pObj2->GetX())
                        {
                            mnTypeId = 16; // AUTOLAYOUT_2OBJOVERTEXT
                        }
                        else
                        {
                            mnTypeId = 15; // AUTOLAYOUT_2OBJTEXT
                        }
                    }
                    else
                    {
                        mnTypeId = 12; // AUTOLAYOUT_TEXT2OBJ
                    }
                    break;
                }
                default: // 5 items
                {
                    mnTypeId = 18; // AUTOLAYOUT_4OBJ
                    break;
                }
            }
        }

        // release remembered contexts, they are no longer needed
        while(maList.Count())
            maList.Remove(maList.Count() - 1)->ReleaseRef();
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLPresentationPlaceholderContext::SdXMLPresentationPlaceholderContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx, const
    OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList)
:   SvXMLImportContext( rImport, nPrfx, rLName),
    mnX(0L),
    mnY(0L),
    mnWidth(1L),
    mnHeight(1L)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetPresentationPlaceholderAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_PRESENTATIONPLACEHOLDER_OBJECTNAME:
            {
                msName = sValue;
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_X:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_Y:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPresentationPlaceholderContext::~SdXMLPresentationPlaceholderContext()
{
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLMasterPageContext, SdXMLGenericPageContext );

SdXMLMasterPageContext::SdXMLMasterPageContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLGenericPageContext( rImport, nPrfx, rLName, xAttrList, rShapes )
{
    const sal_Bool bHandoutMaster = IsXMLToken( rLName, XML_HANDOUT_MASTER );

    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetMasterPageAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_MASTERPAGE_NAME:
            {
                msName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_PAGE_MASTER_NAME:
            {
                msPageMasterName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_STYLE_NAME:
            {
                msStyleName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_PAGE_LAYOUT_NAME:
            {
                maPageLayoutName = sValue;
                break;
            }
        }
    }

    GetImport().GetShapeImport()->startPage( GetLocalShapesContext() );

    // set page name?
    if(!bHandoutMaster && msName.getLength() && GetLocalShapesContext().is())
    {
        uno::Reference < container::XNamed > xNamed(GetLocalShapesContext(), uno::UNO_QUERY);
        if(xNamed.is())
            xNamed->setName(msName);
    }

    // set page-master?
    if(msPageMasterName.getLength())
    {
        SetPageMaster( msPageMasterName );
    }

    SetStyle( msStyleName );

    SetLayout();

    DeleteAllShapes();
}

//////////////////////////////////////////////////////////////////////////////

SdXMLMasterPageContext::~SdXMLMasterPageContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLMasterPageContext::EndElement()
{
    // set styles on master-page
    if(msName.getLength() && GetSdImport().GetShapeImport()->GetStylesContext())
    {
        SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetStylesContext();
        if( pContext && pContext->ISA( SvXMLStyleContext ) )
            ((SdXMLStylesContext*)pContext)->SetMasterPageStyles(*this);
    }

    SdXMLGenericPageContext::EndElement();
    GetImport().GetShapeImport()->endPage(GetLocalShapesContext());
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLMasterPageContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = GetSdImport().GetMasterPageElemTokenMap();

    // some special objects inside style:masterpage context
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_MASTERPAGE_STYLE:
        {
            if(GetSdImport().GetShapeImport()->GetStylesContext())
            {
                // style:style inside master-page context -> presentation style
                XMLShapeStyleContext* pNew = new XMLShapeStyleContext(
                    GetSdImport(), nPrefix, rLocalName, xAttrList,
                    *GetSdImport().GetShapeImport()->GetStylesContext(),
                    XML_STYLE_FAMILY_SD_PRESENTATION_ID);

                // add this style to the outer StylesContext class for later processing
                if(pNew)
                {
                    pContext = pNew;
                    GetSdImport().GetShapeImport()->GetStylesContext()->AddStyle(*pNew);
                }
            }
            break;
        }
        case XML_TOK_MASTERPAGE_NOTES:
        {
            if( GetSdImport().IsImpress() )
            {
                // get notes page
                uno::Reference< presentation::XPresentationPage > xPresPage(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xPresPage.is())
                {
                    uno::Reference< drawing::XDrawPage > xNotesDrawPage(xPresPage->getNotesPage(), uno::UNO_QUERY);
                    if(xNotesDrawPage.is())
                    {
                        uno::Reference< drawing::XShapes > xNewShapes(xNotesDrawPage, uno::UNO_QUERY);
                        if(xNewShapes.is())
                        {
                            // presentation:notes inside master-page context
                            pContext = new SdXMLNotesContext( GetSdImport(), nPrefix, rLocalName, xAttrList, xNewShapes);
                        }
                    }
                }
            }
        }
    }

    // call base class
    if(!pContext)
        pContext = SdXMLGenericPageContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLStylesContext, SvXMLStyleContext );

SdXMLStylesContext::SdXMLStylesContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    sal_Bool bIsAutoStyle)
:   SvXMLStylesContext(rImport, nPrfx, rLName, xAttrList),
    mbIsAutoStyle(bIsAutoStyle)
{
    // #110680#
    // Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference< lang::XMultiServiceFactory > xMSF = rImport.getServiceFactory();

    mpNumFormatter = new SvNumberFormatter( xMSF, LANGUAGE_SYSTEM );

    // #110680#
    // mpNumFmtHelper = new SvXMLNumFmtHelper( mpNumFormatter );
    mpNumFmtHelper = new SvXMLNumFmtHelper( mpNumFormatter, xMSF );
}

//////////////////////////////////////////////////////////////////////////////

SdXMLStylesContext::~SdXMLStylesContext()
{
    delete mpNumFmtHelper;
    delete mpNumFormatter;
}

//////////////////////////////////////////////////////////////////////////////

SvXMLStyleContext* SdXMLStylesContext::CreateStyleChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    SvXMLStyleContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = GetSdImport().GetStylesElemTokenMap();

    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_STYLES_PAGE_MASTER:
        {
            // style:page-master inside office:styles context
            pContext = new SdXMLPageMasterContext(GetSdImport(), nPrefix, rLocalName, xAttrList);
            break;
        }
        case XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT:
        {
            // style:presentation-page-layout inside office:styles context
            pContext = new SdXMLPresentationPageLayoutContext(GetSdImport(), nPrefix, rLocalName, xAttrList);
            break;
        }
    }

    if(!pContext)
    {
        const SvXMLTokenMap& rTokenMap = mpNumFmtHelper->GetStylesElemTokenMap();
        sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
        switch (nToken)
        {
            case XML_TOK_STYLES_DATE_STYLE:
            case XML_TOK_STYLES_TIME_STYLE:
                // number:date-style or number:time-style
                pContext = new SdXMLNumberFormatImportContext( GetSdImport(), nPrefix, rLocalName, mpNumFmtHelper->getData(), nToken, xAttrList, *this );
                break;

            case XML_TOK_STYLES_NUMBER_STYLE:
            case XML_TOK_STYLES_CURRENCY_STYLE:
            case XML_TOK_STYLES_PERCENTAGE_STYLE:
            case XML_TOK_STYLES_BOOLEAN_STYLE:
            case XML_TOK_STYLES_TEXT_STYLE:
                pContext = new SvXMLNumFormatContext( GetSdImport(), nPrefix, rLocalName,
                                                        mpNumFmtHelper->getData(), nToken, xAttrList, *this );
                break;
        }
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStylesContext::CreateStyleChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

SvXMLStyleContext* SdXMLStylesContext::CreateStyleStyleChildContext(
    sal_uInt16 nFamily,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList)
{
    SvXMLStyleContext* pContext = 0;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID:
        pContext = new SdXMLDrawingPageStyleContext(GetSdImport(), nPrefix, rLocalName, xAttrList, *this );
        break;
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStylesContext::CreateStyleStyleChildContext(nFamily, nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

SvXMLStyleContext* SdXMLStylesContext::CreateDefaultStyleStyleChildContext(
    sal_uInt16 nFamily,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< XAttributeList > & xAttrList )
{
    SvXMLStyleContext* pContext = 0;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        pContext = new XMLGraphicsDefaultStyle(GetSdImport(), nPrefix, rLocalName, xAttrList, *this );
        break;
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStylesContext::CreateDefaultStyleStyleChildContext(nFamily, nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

sal_uInt16 SdXMLStylesContext::GetFamily( const OUString& rFamily ) const
{
//  if(rFamily.getLength())
//  {
//      if(rFamily.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME))))
//          return XML_STYLE_FAMILY_SD_GRAPHICS_ID;
//
//      if(rFamily.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_NAME))))
//          return XML_STYLE_FAMILY_SD_PRESENTATION_ID;
//
//      if(rFamily.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_POOL_NAME))))
//          return XML_STYLE_FAMILY_SD_POOL_ID;
//
//      if(rFamily.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME))))
//          return XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID;
//  }

    // call base class
    return SvXMLStylesContext::GetFamily(rFamily);
}

//////////////////////////////////////////////////////////////////////////////

UniReference< SvXMLImportPropertyMapper > SdXMLStylesContext::GetImportPropertyMapper(
    sal_uInt16 nFamily) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper;

//  if(XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily
//      || XML_STYLE_FAMILY_SD_PRESENTATION_ID == nFamily
//      || XML_STYLE_FAMILY_SD_POOL_ID == nFamily)
//  {
//      if(!xPropImpPropMapper.is())
//      {
//          UniReference< XMLShapeImportHelper > aImpHelper = ((SvXMLImport&)GetImport()).GetShapeImport();
//          ((SdXMLStylesContext*)this)->xPropImpPropMapper =
//              new SvXMLImportPropertyMapper(aImpHelper->GetPropertySetMapper());
//      }
//      xMapper = xPropImpPropMapper;
//      return xMapper;
//  }

    if(XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID == nFamily)
    {
        if(!xPresImpPropMapper.is())
        {
            UniReference< XMLShapeImportHelper > aImpHelper = ((SvXMLImport&)GetImport()).GetShapeImport();
            ((SdXMLStylesContext*)this)->xPresImpPropMapper =
                aImpHelper->GetPresPagePropsMapper();
        }
        xMapper = xPresImpPropMapper;
        return xMapper;
    }

    // call base class
    return SvXMLStylesContext::GetImportPropertyMapper(nFamily);
}

//////////////////////////////////////////////////////////////////////////////
// Process all style and object info
//
void SdXMLStylesContext::EndElement()
{
    if(mbIsAutoStyle)
    {
        // AutoStyles for text import
        GetImport().GetTextImport()->SetAutoStyles( this );

        // AutoStyles for chart
        GetImport().GetChartImport()->SetAutoStylesContext( this );

        // AutoStyles for forms
        GetImport().GetFormImport()->setAutoStyleContext( this );

        // associate AutoStyles with styles in preparation to setting Styles on shapes
        for(sal_uInt32 a(0L); a < GetStyleCount(); a++)
        {
            const SvXMLStyleContext* pStyle = GetStyle(a);
            if(pStyle && pStyle->ISA(XMLShapeStyleContext))
            {
                XMLShapeStyleContext* pDocStyle = (XMLShapeStyleContext*)pStyle;
//              pDocStyle->Filter();

                SvXMLStylesContext* pStylesContext = GetSdImport().GetShapeImport()->GetStylesContext();
                if( pStylesContext )
                {
                    pStyle = pStylesContext->FindStyleChildContext(pStyle->GetFamily(), pStyle->GetParent());

                    if(pStyle && pStyle->ISA(XMLShapeStyleContext))
                    {
                        XMLShapeStyleContext* pParentStyle = (XMLShapeStyleContext*)pStyle;
                        if(pParentStyle->GetStyle().is())
                        {
                            pDocStyle->SetStyle(pParentStyle->GetStyle());
                        }
                    }
                }
            }
        }

        FinishStyles( false );
    }
    else
    {
        // Process styles list
        ImpSetGraphicStyles();

        // put style infos in the info set for other components ( content import f.e. )
        uno::Reference< beans::XPropertySet > xInfoSet( GetImport().getImportInfo() );
        if( xInfoSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

            if( xInfoSetInfo->hasPropertyByName( OUString( RTL_CONSTASCII_USTRINGPARAM( "PageLayouts" ) ) ) )
                xInfoSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PageLayouts" ) ), uno::makeAny( getPageLayouts() ) );
        }

    }
}

//////////////////////////////////////////////////////////////////////////////
// set master-page styles (all with family="presentation" and a special
// prefix) on given master-page.
//
void SdXMLStylesContext::SetMasterPageStyles(SdXMLMasterPageContext& rMaster) const
{
    UniString sPrefix(rMaster.GetName(), (sal_uInt16)rMaster.GetName().getLength());
    sPrefix += sal_Unicode('-');

    if(GetSdImport().GetLocalDocStyleFamilies().is() && GetSdImport().GetLocalDocStyleFamilies()->hasByName(rMaster.GetName()))
    {
        uno::Any aAny(GetSdImport().GetLocalDocStyleFamilies()->getByName(rMaster.GetName()));
        uno::Reference< container::XNameAccess > xMasterPageStyles;
        aAny >>= xMasterPageStyles;

        if(xMasterPageStyles.is())
        {
            ImpSetGraphicStyles(xMasterPageStyles, XML_STYLE_FAMILY_SD_PRESENTATION_ID, sPrefix);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// Process styles list:
// set graphic styles (all with family="graphics"). Remember xStyle at list element.
//
void SdXMLStylesContext::ImpSetGraphicStyles() const
{
    if(GetSdImport().GetLocalDocStyleFamilies().is())
    {
        uno::Reference< container::XNameAccess > xGraphicPageStyles;
        try
        {
            const OUString sGraphicStyleName(OUString(RTL_CONSTASCII_USTRINGPARAM("graphics")));
            GetSdImport().GetLocalDocStyleFamilies()->getByName(sGraphicStyleName) >>= xGraphicPageStyles;

            if(xGraphicPageStyles.is())
            {
                UniString aPrefix;
                ImpSetGraphicStyles(xGraphicPageStyles, XML_STYLE_FAMILY_SD_GRAPHICS_ID, aPrefix);
            }
        }
        catch( uno::Exception& e )
        {
            (void)e;
        }

        DBG_ASSERT( xGraphicPageStyles.is(), "xmloff::SdXMLStylesContext::ImpSetGraphicStyles(), no graphic style" );
    }
}

//////////////////////////////////////////////////////////////////////////////
// help function used by ImpSetGraphicStyles() and ImpSetMasterPageStyles()
//
void SdXMLStylesContext::ImpSetGraphicStyles(
    uno::Reference< container::XNameAccess >& xPageStyles,
    sal_uInt16 nFamily,
    const UniString& rPrefix) const
{
    xub_StrLen nPrefLen(rPrefix.Len());
    uno::Any aAny;

    sal_uInt32 a;

    // set defaults
    for( a = 0; a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(nFamily == pStyle->GetFamily() && pStyle->IsDefaultStyle())
        {
            ((SvXMLStyleContext*)pStyle)->SetDefaults();
        }
    }

    // create all styles and set properties
    for( a = 0; a < GetStyleCount(); a++)
    {
        try
        {
            const SvXMLStyleContext* pStyle = GetStyle(a);
            if(nFamily == pStyle->GetFamily() && !pStyle->IsDefaultStyle())
            {
                const UniString aStyleName(pStyle->GetDisplayName(), (sal_uInt16)pStyle->GetDisplayName().getLength());
                sal_uInt16 nStylePrefLen = aStyleName.SearchBackward( sal_Unicode('-') ) + 1;

                uno::Reference< style::XStyle > xStyle;
                const OUString aPureStyleName = nPrefLen ?
                    pStyle->GetDisplayName().copy((sal_Int32)nPrefLen) : pStyle->GetDisplayName();

                if(!nPrefLen || ((nPrefLen == nStylePrefLen) && aStyleName.Equals(rPrefix, 0, nPrefLen)))
                {
                    uno::Reference< style::XStyle > xStyle;
                    const OUString aPureStyleName = nPrefLen ?
                        pStyle->GetName().copy((sal_Int32)nPrefLen) : pStyle->GetName();

                    if(xPageStyles->hasByName(aPureStyleName))
                    {
                        aAny = xPageStyles->getByName(aPureStyleName);
                        aAny >>= xStyle;

                        // set properties of existing styles to default
                        uno::Reference< beans::XPropertySet > xPropSet( xStyle, uno::UNO_QUERY );
                        uno::Reference< beans::XPropertySetInfo > xPropSetInfo;
                        if( xPropSet.is() )
                            xPropSetInfo = xPropSet->getPropertySetInfo();

                        uno::Reference< beans::XPropertyState > xPropState( xStyle, uno::UNO_QUERY );

                        if( xPropState.is() )
                        {
                            UniReference < XMLPropertySetMapper > xPrMap;
                            UniReference < SvXMLImportPropertyMapper > xImpPrMap = GetImportPropertyMapper( nFamily );
                            DBG_ASSERT( xImpPrMap.is(), "There is the import prop mapper" );
                            if( xImpPrMap.is() )
                                xPrMap = xImpPrMap->getPropertySetMapper();
                            if( xPrMap.is() )
                            {
                                const sal_Int32 nCount = xPrMap->GetEntryCount();
                                for( sal_Int32 i = 0; i < nCount; i++ )
                                {
                                    const OUString& rName = xPrMap->GetEntryAPIName( i );
                                    if( xPropSetInfo->hasPropertyByName( rName ) && beans::PropertyState_DIRECT_VALUE == xPropState->getPropertyState( rName ) )
                                    {
                                        xPropState->setPropertyToDefault( rName );
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // graphics style does not exist, create and add it
                        uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetSdImport().GetModel(), uno::UNO_QUERY);
                        if(xServiceFact.is())
                        {
                            uno::Reference< style::XStyle > xNewStyle(
                                xServiceFact->createInstance(
                                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.Style"))),
                                uno::UNO_QUERY);

                            if(xNewStyle.is())
                            {
                                // remember style
                                xStyle = xNewStyle;

                                // add new style to graphics style pool
                                uno::Reference< container::XNameContainer > xInsertContainer(xPageStyles, uno::UNO_QUERY);
                                if(xInsertContainer.is())
                                {
                                    aAny <<= xStyle;
                                    xInsertContainer->insertByName(aPureStyleName, aAny);
                                    const_cast< SvXMLImport&>( GetImport() ).AddStyleDisplayName( XML_STYLE_FAMILY_SD_GRAPHICS_ID, pStyle->GetName(), aPureStyleName );
                                }
                            }
                        }
                    }

                    if(xStyle.is())
                    {
                        // set properties at style
                        XMLShapeStyleContext* pPropStyle =
                            (pStyle->ISA(XMLShapeStyleContext)) ? (XMLShapeStyleContext*)pStyle : 0L;
                        uno::Reference< beans::XPropertySet > xPropSet(xStyle, uno::UNO_QUERY);

                        if(xPropSet.is() && pPropStyle)
                        {
                            pPropStyle->FillPropertySet(xPropSet);
                            pPropStyle->SetStyle(xStyle);
                        }
                    }
                }
            }
        }
        catch( Exception&e )
        {
            (void)e;
            uno::Sequence<OUString> aSeq(0);
            const_cast<SdXMLImport*>(&GetSdImport())->SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, NULL );
        }
    }

    // now set parents for all styles (when necessary)
    for(a = 0L; a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(pStyle && nFamily == pStyle->GetFamily())
        {
            const UniString aStyleName(pStyle->GetName(), (sal_uInt16)pStyle->GetName().getLength());
            sal_uInt16 nStylePrefLen = aStyleName.SearchBackward( sal_Unicode('-') ) + 1;

            if(pStyle->GetName().getLength() && (!nPrefLen || ((nPrefLen == nStylePrefLen) && aStyleName.Equals(rPrefix, 0, nPrefLen))))
            {
                try
                {

                    uno::Reference< style::XStyle > xStyle;
                    const OUString aPureStyleName = nPrefLen ? pStyle->GetName().copy((sal_Int32)nPrefLen) : pStyle->GetName();
                    xPageStyles->getByName(aPureStyleName) >>= xStyle;

                    if(xStyle.is())
                    {
                            // set parent style name
                            xStyle->setParentStyle(pStyle->GetParent());
                    }
                }
                catch( Exception& e )
                {
                    (void)e;
                    uno::Sequence<OUString> aSeq(0);
                    const_cast<SdXMLImport*>(&GetSdImport())->SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, NULL );
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// helper function to create the uno component that hold the mappings from
// xml auto layout name to internal autolayout id

uno::Reference< container::XNameAccess > SdXMLStylesContext::getPageLayouts() const
{
    uno::Reference< container::XNameContainer > xLayouts( comphelper::NameContainer_createInstance( ::getCppuType((const sal_Int32*)0)) );

    for(sal_uInt32 a(0L); a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);
        if(pStyle && pStyle->ISA(SdXMLPresentationPageLayoutContext))
        {
            xLayouts->insertByName( pStyle->GetName(), uno::makeAny(
            (sal_Int32)((SdXMLPresentationPageLayoutContext*)pStyle)->GetTypeId() ) );
        }
    }

    return uno::Reference< container::XNameAccess >::query( xLayouts );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
TYPEINIT1( SdXMLMasterStylesContext, SvXMLImportContext );

SdXMLMasterStylesContext::SdXMLMasterStylesContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const rtl::OUString& rLName)
:   SvXMLImportContext( rImport, nPrfx, rLName)
{
}

SdXMLMasterStylesContext::~SdXMLMasterStylesContext()
{
    while(maMasterPageList.Count())
        maMasterPageList.Remove(maMasterPageList.Count() - 1)->ReleaseRef();
}

SvXMLImportContext* SdXMLMasterStylesContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    SvXMLImportContext* pContext = 0;

    if(nPrefix == XML_NAMESPACE_STYLE
       && IsXMLToken( rLocalName, XML_MASTER_PAGE ) )
    {
        // style:masterpage inside office:styles context
        uno::Reference< drawing::XDrawPage > xNewMasterPage;
        uno::Reference< drawing::XDrawPages > xMasterPages(GetSdImport().GetLocalMasterPages(), uno::UNO_QUERY);

        if( xMasterPages.is() )
        {
            if(GetSdImport().GetNewMasterPageCount() + 1 > xMasterPages->getCount())
            {
                // new page, create and insert
                xNewMasterPage = xMasterPages->insertNewByIndex(xMasterPages->getCount());
            }
            else
            {
                // existing page, use it
                uno::Any aAny(xMasterPages->getByIndex(GetSdImport().GetNewMasterPageCount()));
                aAny >>= xNewMasterPage;
            }

            // increment global import page counter
            GetSdImport().IncrementNewMasterPageCount();

            if(xNewMasterPage.is())
            {
                uno::Reference< drawing::XShapes > xNewShapes(xNewMasterPage, uno::UNO_QUERY);
                if(xNewShapes.is() && GetSdImport().GetShapeImport()->GetStylesContext())
                {
                    pContext = new SdXMLMasterPageContext(GetSdImport(),
                        nPrefix, rLocalName, xAttrList, xNewShapes);

                    if(pContext)
                    {
                        pContext->AddRef();
                        maMasterPageList.Insert((SdXMLMasterPageContext*)pContext, LIST_APPEND);
                    }
                }
            }
        }
    }
    else    if(nPrefix == XML_NAMESPACE_STYLE
        && IsXMLToken( rLocalName, XML_HANDOUT_MASTER ) )
    {
        uno::Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetSdImport().GetModel(), uno::UNO_QUERY );
        if( xHandoutSupp.is() )
        {
            uno::Reference< drawing::XShapes > xHandoutPage( xHandoutSupp->getHandoutMasterPage(), uno::UNO_QUERY );
            if(xHandoutPage.is() && GetSdImport().GetShapeImport()->GetStylesContext())
            {
                pContext = new SdXMLMasterPageContext(GetSdImport(),
                    nPrefix, rLocalName, xAttrList, xHandoutPage);
            }
        }
    }
    else if( nPrefix == XML_NAMESPACE_DRAW
        && IsXMLToken( rLocalName, XML_LAYER_SET ) )
    {
        pContext = new SdXMLLayerSetContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }

    // call base class
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}


