/*************************************************************************
 *
 *  $RCSfile: ximpstyl.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-04 23:07:53 $
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
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

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGES_HPP_
#include <com/sun/star/drawing/XDrawPages.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include "xmlprcon.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
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

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

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
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, rProps, rMap )
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

            if( (nPrefix == XML_NAMESPACE_XLINK) && aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_href ) ) )
            {
                uno::Any aAny;
                aAny <<= xAttrList->getValueByIndex(i);
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
        rLocalName.compareToAscii( sXML_properties ) == 0 )
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
                if(sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_portrait))))
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

    if(nPrefix == XML_NAMESPACE_STYLE && rLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_properties))))
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

        if(nPrefix == XML_NAMESPACE_STYLE && aLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_name))))
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

    if(nPrefix == XML_NAMESPACE_PRESENTATION && rLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_placeholder))))
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
            if(maList.Count() == 1)
                mnTypeId = 22; // AUTOLAYOUT_HANDOUT1
            if(maList.Count() == 2)
                mnTypeId = 23; // AUTOLAYOUT_HANDOUT2
            if(maList.Count() == 3)
                mnTypeId = 24; // AUTOLAYOUT_HANDOUT3
            if(maList.Count() == 4)
                mnTypeId = 25; // AUTOLAYOUT_HANDOUT4
            else
                mnTypeId = 26; // AUTOLAYOUT_HANDOUT6
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
                        mnTypeId = 9; // AUTOLAYOUT_CLIPTEXT
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
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
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
        }
    }

    // set page name?
    if(msName.getLength() && GetLocalShapesContext().is())
    {
        uno::Reference < container::XNamed > xNamed(GetLocalShapesContext(), uno::UNO_QUERY);
        if(xNamed.is())
            xNamed->setName(msName);
    }

    // set page-master?
    if(msPageMasterName.getLength() && GetSdImport().GetShapeImport()->GetStylesContext())
    {
        // look for PageMaster with this name

        // #80012# GetStylesContext() replaced with GetAutoStylesContext()
        const SvXMLStyleContext* pStyle =
            GetSdImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(
            XML_STYLE_FAMILY_SD_PAGEMASTERCONEXT_ID, msPageMasterName);

        if(pStyle && pStyle->ISA(SdXMLPageMasterContext))
        {
            const SdXMLPageMasterContext* pPageMaster = (SdXMLPageMasterContext*)pStyle;
            const SdXMLPageMasterStyleContext* pPageMasterContext = pPageMaster->GetPageMasterStyle();

            if(pPageMasterContext)
            {
                uno::Reference< drawing::XDrawPage > xMasterPage(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xMasterPage.is())
                {
                    // set sizes for this masterpage
                    uno::Reference <beans::XPropertySet> xPropSet(xMasterPage, uno::UNO_QUERY);
                    if(xPropSet.is())
                    {
                        uno::Any aAny;

                        aAny <<= pPageMasterContext->GetBorderBottom();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("BorderBottom")), aAny);

                        aAny <<= pPageMasterContext->GetBorderLeft();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("BorderLeft")), aAny);

                        aAny <<= pPageMasterContext->GetBorderRight();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("BorderRight")), aAny);

                        aAny <<= pPageMasterContext->GetBorderTop();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("BorderTop")), aAny);

                        aAny <<= pPageMasterContext->GetWidth();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Width")), aAny);

                        aAny <<= pPageMasterContext->GetHeight();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Height")), aAny);

                        aAny <<= pPageMasterContext->GetOrientation();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Orientation")), aAny);
                    }
                }
            }
        }
    }

    // set PageProperties?
    if(msStyleName.getLength())
    {
        const SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetAutoStylesContext();

        if( pContext && pContext->ISA( SvXMLStyleContext ) )
        {
            const SdXMLStylesContext* pStyles = (SdXMLStylesContext*)pContext;
            if(pStyles)
            {
                const SvXMLStyleContext* pStyle = pStyles->FindStyleChildContext(
                    XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, msStyleName);

                if(pStyle && pStyle->ISA(XMLPropStyleContext))
                {
                    XMLPropStyleContext* pPropStyle = (XMLPropStyleContext*)pStyle;

                    uno::Reference <beans::XPropertySet> xPropSet1(rShapes, uno::UNO_QUERY);
                    if(xPropSet1.is())
                    {
                        const OUString aBackground(RTL_CONSTASCII_USTRINGPARAM("Background"));
                        uno::Reference< beans::XPropertySet > xPropSet2;
                        uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet1->getPropertySetInfo() );
                        if( xInfo.is() && xInfo->hasPropertyByName( aBackground ) )
                        {
                            uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetSdImport().GetModel(), uno::UNO_QUERY);
                            if(xServiceFact.is())
                            {
                                uno::Reference< beans::XPropertySet > xTempSet(
                                    xServiceFact->createInstance(
                                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Background"))),
                                    uno::UNO_QUERY);

                                xPropSet2 = xTempSet;
                            }
                        }

                        uno::Reference< beans::XPropertySet > xPropSet;
                        if( xPropSet2.is() )
                            xPropSet = PropertySetMerger_CreateInstance( xPropSet1, xPropSet2 );
                        else
                            xPropSet = xPropSet1;

                        if(xPropSet.is())
                        {
                            try
                            {
                                pPropStyle->FillPropertySet(xPropSet);

                                uno::Any aAny;
                                aAny <<= xPropSet2;
                                xPropSet1->setPropertyValue( aBackground, aAny );
                            }
                            catch( uno::Exception )
                            {
                                DBG_ERROR( "uno::Exception catched!" );
                            }
                        }
                    }
                }
            }
        }
    }

    // now delete all up-to-now contained shapes.
    while(rShapes->getCount())
    {
        uno::Reference< drawing::XShape > xShape;
        uno::Any aAny(rShapes->getByIndex(0L));

        aAny >>= xShape;

        if(xShape.is())
        {
            rShapes->remove(xShape);
        }
    }
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
    GetImport().GetShapeImport()->restoreConnections();
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
            // presentation:notes inside master-page context
            pContext = new SdXMLNotesContext( GetSdImport(), nPrefix, rLocalName, xAttrList,
                GetLocalShapesContext());
            break;
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
}

//////////////////////////////////////////////////////////////////////////////

SdXMLStylesContext::~SdXMLStylesContext()
{
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

    if(GetSdImport().GetLocalDocStyleFamilies()->hasByName(rMaster.GetName()))
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
        const OUString sGraphicStyleName(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)));
        uno::Any aAny(GetSdImport().GetLocalDocStyleFamilies()->getByName(sGraphicStyleName));
        uno::Reference< container::XNameAccess > xGraphicPageStyles;
        aAny >>= xGraphicPageStyles;

        if(xGraphicPageStyles.is())
        {
            UniString aPrefix;
            ImpSetGraphicStyles(xGraphicPageStyles, XML_STYLE_FAMILY_SD_GRAPHICS_ID, aPrefix);
        }
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
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(nFamily == pStyle->GetFamily() && !pStyle->IsDefaultStyle())
        {
            const UniString aStyleName(pStyle->GetName(), (sal_uInt16)pStyle->GetName().getLength());
            if(!nPrefLen || aStyleName.Equals(rPrefix, 0, nPrefLen))
            {
                uno::Reference< style::XStyle > xStyle;
                const OUString aPureStyleName = nPrefLen ?
                    pStyle->GetName().copy((sal_Int32)nPrefLen) : pStyle->GetName();

                if(xPageStyles->hasByName(aPureStyleName))
                {
                    aAny = xPageStyles->getByName(aPureStyleName);
                    aAny >>= xStyle;
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

    // now set parents for all styles (when necessary)
    for(a = 0L; a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(pStyle && nFamily == pStyle->GetFamily())
        {
            const UniString aStyleName(pStyle->GetName(), (sal_uInt16)pStyle->GetName().getLength());
            if(!nPrefLen || aStyleName.Equals(rPrefix, 0, nPrefLen))
            {
                if(pStyle->GetParent().getLength())
                {
                    const OUString aPureStyleName = nPrefLen ? pStyle->GetName().copy((sal_Int32)nPrefLen) : pStyle->GetName();
                    uno::Reference< style::XStyle > xStyle;
                    aAny = xPageStyles->getByName(aPureStyleName);
                    aAny >>= xStyle;

                    if(xStyle.is())
                    {
                        // set parent style name
                        xStyle->setParentStyle(pStyle->GetParent());
                    }
                }
            }
        }
    }
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
        && rLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_master_page))))
    {
        // style:masterpage inside office:styles context
        uno::Reference< drawing::XDrawPage > xNewMasterPage;
        uno::Reference< drawing::XDrawPages > xMasterPages(GetSdImport().GetLocalMasterPages(), uno::UNO_QUERY);

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
    else if( nPrefix == XML_NAMESPACE_DRAW
        && rLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_layer_set))))
    {
        pContext = new SdXMLLayerSetContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }

    // call base class
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}


