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

#include "ximpstyl.hxx"
#include <xmloff/XMLShapeStyleContext.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "ximpnote.hxx"
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#include <comphelper/namecontainer.hxx>
#include <xmloff/autolayout.hxx>
#include <xmloff/xmlprcon.hxx>
#include <xmloff/families.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <svl/zforlist.hxx>
#include <PropertySetMerger.hxx>
#include "sdpropls.hxx"
#include "layerimp.hxx"
#include <xmloff/XMLGraphicsDefaultStyle.hxx>
#include <XMLNumberStylesImport.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/table/XMLTableImport.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class SdXMLDrawingPagePropertySetContext : public SvXMLPropertySetContext
{
public:


    SdXMLDrawingPagePropertySetContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                 const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const rtl::Reference < SvXMLImportPropertyMapper > &rMap );

    using SvXMLPropertySetContext::CreateChildContext;
    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
                                   ::std::vector< XMLPropertyState > &rProperties,
                                   const XMLPropertyState& rProp) override;
};


SdXMLDrawingPagePropertySetContext::SdXMLDrawingPagePropertySetContext(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const uno::Reference< xml::sax::XAttributeList > & xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const rtl::Reference < SvXMLImportPropertyMapper > &rMap ) :
    SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList,
                             XML_TYPE_PROP_DRAWING_PAGE, rProps, rMap )
{
}

SvXMLImportContextRef SdXMLDrawingPagePropertySetContext::CreateChildContext(
                   sal_uInt16 p_nPrefix,
                   const OUString& rLocalName,
                   const uno::Reference< xml::sax::XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContextRef xContext;

    switch( mxMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
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
                uno::Any aAny( GetImport().GetAbsoluteReference( xAttrList->getValueByIndex(i) ) );
                XMLPropertyState aPropState( rProp.mnIndex, aAny );
                rProperties.push_back( aPropState );
            }
        }
        break;
    }
    }

    if (!xContext)
        xContext = SvXMLPropertySetContext::CreateChildContext( p_nPrefix, rLocalName,
                                                            xAttrList,
                                                            rProperties, rProp );

    return xContext;
}

class SdXMLDrawingPageStyleContext : public XMLPropStyleContext
{
public:

    SdXMLDrawingPageStyleContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
        SvXMLStylesContext& rStyles,
        sal_uInt16 nFamily = XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID);

    SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void Finish( bool bOverwrite ) override;

    // #i35918#
    virtual void FillPropertySet( const css::uno::Reference< css::beans::XPropertySet > & rPropSet ) override;
};


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

SvXMLImportContextRef SdXMLDrawingPageStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_DRAWING_PAGE_PROPERTIES ) )
    {
        rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
            xContext = new SdXMLDrawingPagePropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    GetProperties(),
                                                    xImpPrMap );
    }

    if (!xContext)
        xContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return xContext;
}

void SdXMLDrawingPageStyleContext::Finish( bool bOverwrite )
{
    XMLPropStyleContext::Finish( bOverwrite );

    ::std::vector< XMLPropertyState > &rProperties = GetProperties();

    const rtl::Reference< XMLPropertySetMapper >& rImpPrMap = GetStyles()->GetImportPropertyMapper( GetFamily() )->getPropertySetMapper();

    for(auto& property : rProperties)
    {
        if( property.mnIndex == -1 )
            continue;

        sal_Int16 nContextID = rImpPrMap->GetEntryContextId(property.mnIndex);
        switch( nContextID )
        {
            case CTF_DATE_TIME_FORMAT:
            {
                OUString sStyleName;
                property.maValue >>= sStyleName;

                sal_Int32 nStyle = 0;

                const SdXMLNumberFormatImportContext* pSdNumStyle =
                    dynamic_cast< const SdXMLNumberFormatImportContext*> (
                        GetStyles()->FindStyleChildContext( XML_STYLE_FAMILY_DATA_STYLE, sStyleName, true ) );

                if( pSdNumStyle )
                    nStyle = pSdNumStyle->GetDrawKey();

                property.maValue <<= nStyle;
            }
            break;
        }
    }

}

// #i35918#
void SdXMLDrawingPageStyleContext::FillPropertySet(
    const Reference< beans::XPropertySet > & rPropSet )
{
    static const sal_uInt16 MAX_SPECIAL_DRAW_STYLES = 7;
    struct ContextID_Index_Pair aContextIDs[MAX_SPECIAL_DRAW_STYLES+1] =
    {
        { CTF_DASHNAME , -1 },
        { CTF_LINESTARTNAME , -1 },
        { CTF_LINEENDNAME , -1 },
        { CTF_FILLGRADIENTNAME, -1 },
        { CTF_FILLTRANSNAME , -1 },
        { CTF_FILLHATCHNAME , -1 },
        { CTF_FILLBITMAPNAME , -1 },
        { -1, -1 }
    };
    static const sal_uInt16 aFamilies[MAX_SPECIAL_DRAW_STYLES] =
    {
        XML_STYLE_FAMILY_SD_STROKE_DASH_ID,
        XML_STYLE_FAMILY_SD_MARKER_ID,
        XML_STYLE_FAMILY_SD_MARKER_ID,
        XML_STYLE_FAMILY_SD_GRADIENT_ID,
        XML_STYLE_FAMILY_SD_GRADIENT_ID,
        XML_STYLE_FAMILY_SD_HATCH_ID,
        XML_STYLE_FAMILY_SD_FILL_IMAGE_ID
    };

    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
        GetStyles()->GetImportPropertyMapper( GetFamily() );
    SAL_WARN_IF( !xImpPrMap.is(), "xmloff", "There is the import prop mapper" );
    if( xImpPrMap.is() )
        xImpPrMap->FillPropertySet( GetProperties(), rPropSet, aContextIDs );

    Reference< beans::XPropertySetInfo > xInfo;
    for( sal_uInt16 i=0; i<MAX_SPECIAL_DRAW_STYLES; i++ )
    {
        sal_Int32 nIndex = aContextIDs[i].nIndex;
        if( nIndex != -1 )
        {
            struct XMLPropertyState& rState = GetProperties()[nIndex];
            OUString sStyleName;
            rState.maValue >>= sStyleName;
            sStyleName = GetImport().GetStyleDisplayName( aFamilies[i],
                                                          sStyleName );
            // get property set mapper
            rtl::Reference<XMLPropertySetMapper> rPropMapper =
                                        xImpPrMap->getPropertySetMapper();

            // set property
            const OUString& rPropertyName =
                    rPropMapper->GetEntryAPIName(rState.mnIndex);
            if( !xInfo.is() )
                xInfo = rPropSet->getPropertySetInfo();
            if ( xInfo->hasPropertyByName( rPropertyName ) )
            {
                rPropSet->setPropertyValue( rPropertyName, Any( sStyleName ) );
            }
        }
    }
}


SdXMLPageMasterStyleContext::SdXMLPageMasterStyleContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_SD_PAGEMASTERSTYLECONEXT_ID),
    mnBorderBottom( 0 ),
    mnBorderLeft( 0 ),
    mnBorderRight( 0 ),
    mnBorderTop( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 ),
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
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnBorderTop, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_MARGIN_BOTTOM:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnBorderBottom, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_MARGIN_LEFT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnBorderLeft, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_MARGIN_RIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnBorderRight, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_PAGE_WIDTH:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnWidth, sValue);
                break;
            }
            case XML_TOK_PAGEMASTERSTYLE_PAGE_HEIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnHeight, sValue);
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

SdXMLPageMasterStyleContext::~SdXMLPageMasterStyleContext()
{
}


SdXMLPageMasterContext::SdXMLPageMasterContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_SD_PAGEMASTERCONEXT_ID)
{
    // set family to something special at SvXMLStyleContext
    // for differences in search-methods

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetPageMasterAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_PAGEMASTER_NAME:
            {
                break;
            }
        }
    }
}

SvXMLImportContextRef SdXMLPageMasterContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    if(nPrefix == XML_NAMESPACE_STYLE && IsXMLToken( rLocalName, XML_PAGE_LAYOUT_PROPERTIES) )
    {
        DBG_ASSERT(!mxPageMasterStyle.is(), "PageMasterStyle is set, there seem to be two of them (!)");
        mxPageMasterStyle.set(new SdXMLPageMasterStyleContext(GetSdImport(), nPrefix, rLocalName, xAttrList));
        xContext = mxPageMasterStyle.get();
    }

    // call base class
    if (!xContext)
        xContext = SvXMLStyleContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return xContext;
}

SdXMLPresentationPageLayoutContext::SdXMLPresentationPageLayoutContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_SD_PRESENTATIONPAGELAYOUT_ID),
    mnTypeId( AUTOLAYOUT_NONE )
{
    // set family to something special at SvXMLStyleContext
    // for differences in search-methods
}

SvXMLImportContextRef SdXMLPresentationPageLayoutContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    if(nPrefix == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_PLACEHOLDER ) )
    {
        const rtl::Reference< SdXMLPresentationPlaceholderContext > xLclContext{
            new SdXMLPresentationPlaceholderContext(GetSdImport(), nPrefix, rLocalName, xAttrList)};
        // presentation:placeholder inside style:presentation-page-layout context
        xContext = xLclContext.get();

        // remember SdXMLPresentationPlaceholderContext for later evaluation
        maList.push_back(xLclContext);
    }

    // call base class
    if (!xContext)
        xContext = SvXMLStyleContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return xContext;
}

void SdXMLPresentationPageLayoutContext::EndElement()
{
    // build presentation page layout type here
    // calc mnTpeId due to content of maList
    // at the moment only use number of types used there
    if( !maList.empty() )
    {
        SdXMLPresentationPlaceholderContext* pObj0 = maList[ 0 ].get();
        if( pObj0->GetName() == "handout" )
        {
            switch( maList.size() )
            {
            case 1:
                mnTypeId = AUTOLAYOUT_HANDOUT1;
                break;
            case 2:
                mnTypeId = AUTOLAYOUT_HANDOUT2;
                break;
            case 3:
                mnTypeId = AUTOLAYOUT_HANDOUT3;
                break;
            case 4:
                mnTypeId = AUTOLAYOUT_HANDOUT4;
                break;
            case 9:
                mnTypeId = AUTOLAYOUT_HANDOUT9;
                break;
            default:
                mnTypeId = AUTOLAYOUT_HANDOUT6;
            }
        }
        else
        {
            switch( maList.size() )
            {
                case 1:
                {
                    if( pObj0->GetName() == "title" )
                    {
                        mnTypeId = AUTOLAYOUT_TITLE_ONLY;
                    }
                    else
                    {
                        mnTypeId = AUTOLAYOUT_ONLY_TEXT;
                    }
                    break;
                }
                case 2:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList[ 1 ].get();

                    if( pObj1->GetName() == "subtitle" )
                    {
                        mnTypeId = AUTOLAYOUT_TITLE;
                    }
                    else if( pObj1->GetName() == "outline" )
                    {
                        mnTypeId = AUTOLAYOUT_TITLE_CONTENT;
                    }
                    else if( pObj1->GetName() == "chart" )
                    {
                        mnTypeId = AUTOLAYOUT_CHART;
                    }
                    else if( pObj1->GetName() == "table" )
                    {
                        mnTypeId = AUTOLAYOUT_TAB;
                    }
                    else if( pObj1->GetName() == "object" )
                    {
                        mnTypeId = AUTOLAYOUT_OBJ;
                    }
                    else if( pObj1->GetName() == "vertical_outline" )
                    {
                        if( pObj0->GetName() == "vertical_title" )
                        {
                            mnTypeId = AUTOLAYOUT_VTITLE_VCONTENT;
                        }
                        else
                        {
                            mnTypeId = AUTOLAYOUT_TITLE_VCONTENT;
                        }
                    }
                    else
                    {
                        mnTypeId = AUTOLAYOUT_NOTES;
                    }
                    break;
                }
                case 3:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList[ 1 ].get();
                    SdXMLPresentationPlaceholderContext* pObj2 = maList[ 2 ].get();

                    if( pObj1->GetName() == "outline" )
                    {
                        if( pObj2->GetName() == "outline" )
                        {
                            mnTypeId = AUTOLAYOUT_TITLE_2CONTENT;
                        }
                        else if( pObj2->GetName() == "chart" )
                        {
                            mnTypeId = AUTOLAYOUT_TEXTCHART;
                        }
                        else if( pObj2->GetName() == "graphic" )
                        {
                            mnTypeId = AUTOLAYOUT_TEXTCLIP;
                        }
                        else
                        {
                            if(pObj1->GetX() < pObj2->GetX())
                            {
                                mnTypeId = AUTOLAYOUT_TEXTOBJ; // outline left, object right
                            }
                            else
                            {
                                mnTypeId = AUTOLAYOUT_TEXTOVEROBJ; // outline top, object right
                            }
                        }
                    }
                    else if( pObj1->GetName() == "chart" )
                    {
                        mnTypeId = AUTOLAYOUT_CHARTTEXT;
                    }
                    else if( pObj1->GetName() == "graphic" )
                    {
                        if( pObj2->GetName() == "vertical_outline" )
                        {
                            mnTypeId = AUTOLAYOUT_TITLE_2VTEXT;
                        }
                        else
                        {
                            mnTypeId = AUTOLAYOUT_CLIPTEXT;
                        }
                    }
                    else if( pObj1->GetName() == "vertical_outline" )
                    {
                        mnTypeId = AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT;
                    }
                    else
                    {
                        if(pObj1->GetX() < pObj2->GetX())
                        {
                            mnTypeId = AUTOLAYOUT_OBJTEXT; // left, right
                        }
                        else
                        {
                            mnTypeId = AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT; // top, bottom
                        }
                    }
                    break;
                }
                case 4:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList[ 1 ].get();
                    SdXMLPresentationPlaceholderContext* pObj2 = maList[ 2 ].get();

                    if( pObj1->GetName() == "object" )
                    {
                        if(pObj1->GetX() < pObj2->GetX())
                        {
                            mnTypeId = AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT;
                        }
                        else
                        {
                            mnTypeId = AUTOLAYOUT_TITLE_2CONTENT_CONTENT;
                        }
                    }
                    else
                    {
                        mnTypeId = AUTOLAYOUT_TITLE_CONTENT_2CONTENT;
                    }
                    break;
                }
                case 5:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList[ 1 ].get();

                    if( pObj1->GetName() == "object" )
                    {
                        mnTypeId = AUTOLAYOUT_TITLE_4CONTENT;
                    }
                    else
                    {
                        mnTypeId = AUTOLAYOUT_4CLIPART;
                    }
                    break;

                }
                case 7:
                {
                    mnTypeId = AUTOLAYOUT_4CLIPART; // FIXME: not AUTOLAYOUT_TITLE_6CONTENT?
                    break;
                }
                default:
                {
                    mnTypeId = AUTOLAYOUT_NONE;
                    break;
                }
            }
        }

        // release remembered contexts, they are no longer needed
        maList.clear();
    }
}

SdXMLPresentationPlaceholderContext::SdXMLPresentationPlaceholderContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx, const
    OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList)
:   SvXMLImportContext( rImport, nPrfx, rLName),
    mnX(0)
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
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnX, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_Y:
            {
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH:
            {
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT:
            {
                break;
            }
        }
    }
}

SdXMLPresentationPlaceholderContext::~SdXMLPresentationPlaceholderContext()
{
}


SdXMLMasterPageContext::SdXMLMasterPageContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXMLGenericPageContext( rImport, nPrfx, rLName, xAttrList, rShapes )
{
    const bool bHandoutMaster = IsXMLToken( rLName, XML_HANDOUT_MASTER );
    OUString sStyleName, sPageMasterName;

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
            case XML_TOK_MASTERPAGE_DISPLAY_NAME:
            {
                msDisplayName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_PAGE_MASTER_NAME:
            {
                sPageMasterName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_STYLE_NAME:
            {
                sStyleName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_PAGE_LAYOUT_NAME:
            {
                maPageLayoutName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_HEADER_NAME:
            {
                maUseHeaderDeclName =  sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_FOOTER_NAME:
            {
                maUseFooterDeclName =  sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_USE_DATE_TIME_NAME:
            {
                maUseDateTimeDeclName =  sValue;
                break;
            }
        }
    }

    if( msDisplayName.isEmpty() )
        msDisplayName = msName;
    else if( msDisplayName != msName )
        GetImport().AddStyleDisplayName( XML_STYLE_FAMILY_MASTER_PAGE, msName, msDisplayName );

    GetImport().GetShapeImport()->startPage( GetLocalShapesContext() );

    // set page name?
    if(!bHandoutMaster && !msDisplayName.isEmpty() && GetLocalShapesContext().is())
    {
        uno::Reference < container::XNamed > xNamed(GetLocalShapesContext(), uno::UNO_QUERY);
        if(xNamed.is())
            xNamed->setName(msDisplayName);
    }

    // set page-master?
    if(!sPageMasterName.isEmpty())
    {
        SetPageMaster( sPageMasterName );
    }

    SetStyle( sStyleName );

    SetLayout();

    DeleteAllShapes();
}

SdXMLMasterPageContext::~SdXMLMasterPageContext()
{
}

void SdXMLMasterPageContext::EndElement()
{
    // set styles on master-page
    if(!msName.isEmpty() && GetSdImport().GetShapeImport()->GetStylesContext())
    {
        SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetStylesContext();
        if (SdXMLStylesContext* pSdContext = dynamic_cast<SdXMLStylesContext*>(pContext))
            pSdContext->SetMasterPageStyles(*this);
    }

    SdXMLGenericPageContext::EndElement();
    GetImport().GetShapeImport()->endPage(GetLocalShapesContext());
}

SvXMLImportContextRef SdXMLMasterPageContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;
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
                xContext = pNew;
                GetSdImport().GetShapeImport()->GetStylesContext()->AddStyle(*pNew);
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
                            xContext = new SdXMLNotesContext( GetSdImport(), nPrefix, rLocalName, xAttrList, xNewShapes);
                        }
                    }
                }
            }
        }
    }

    // call base class
    if (!xContext)
        xContext = SdXMLGenericPageContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return xContext;
}

SdXMLStylesContext::SdXMLStylesContext(
    SdXMLImport& rImport,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    bool bIsAutoStyle)
:   SvXMLStylesContext(rImport, XML_NAMESPACE_OFFICE, rLName, xAttrList),
    mbIsAutoStyle(bIsAutoStyle)
{
    Reference< uno::XComponentContext > xContext = rImport.GetComponentContext();
    mpNumFormatter = std::make_unique<SvNumberFormatter>( xContext, LANGUAGE_SYSTEM );
    mpNumFmtHelper = std::make_unique<SvXMLNumFmtHelper>( mpNumFormatter.get(), xContext );
}

SvXMLStyleContext* SdXMLStylesContext::CreateStyleChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    SvXMLStyleContext* pContext = nullptr;
    const SvXMLTokenMap& rStyleTokenMap = GetSdImport().GetStylesElemTokenMap();

    switch(rStyleTokenMap.Get(nPrefix, rLocalName))
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

    if(!pContext && nPrefix == XML_NAMESPACE_PRESENTATION )
    {
        if( IsXMLToken( rLocalName, XML_HEADER_DECL ) ||
            IsXMLToken( rLocalName, XML_FOOTER_DECL ) ||
            IsXMLToken( rLocalName, XML_DATE_TIME_DECL ) )
        {
            pContext = new SdXMLHeaderFooterDeclContext( GetImport(), nPrefix, rLocalName, xAttrList );
        }
    }

    if(!pContext && (nPrefix == XML_NAMESPACE_TABLE) && IsXMLToken( rLocalName, XML_TABLE_TEMPLATE ) )
    {
        pContext = GetImport().GetShapeImport()->GetShapeTableImport()->CreateTableTemplateContext(nPrefix, rLocalName, xAttrList );
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStylesContext::CreateStyleChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

SvXMLStyleContext* SdXMLStylesContext::CreateStyleStyleChildContext(
    sal_uInt16 nFamily,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< css::xml::sax::XAttributeList >& xAttrList)
{
    SvXMLStyleContext* pContext = nullptr;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID:
        pContext = new SdXMLDrawingPageStyleContext(GetSdImport(), nPrefix, rLocalName, xAttrList, *this );
        break;
    case XML_STYLE_FAMILY_TABLE_CELL:
    case XML_STYLE_FAMILY_TABLE_COLUMN:
    case XML_STYLE_FAMILY_TABLE_ROW:
        pContext = new XMLShapeStyleContext( GetSdImport(), nPrefix, rLocalName, xAttrList, *this, nFamily );
        break;
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStylesContext::CreateStyleStyleChildContext(nFamily, nPrefix, rLocalName, xAttrList);

    return pContext;
}

SvXMLStyleContext* SdXMLStylesContext::CreateDefaultStyleStyleChildContext(
    sal_uInt16 nFamily,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< XAttributeList > & xAttrList )
{
    SvXMLStyleContext* pContext = nullptr;

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

rtl::Reference< SvXMLImportPropertyMapper > SdXMLStylesContext::GetImportPropertyMapper(
    sal_uInt16 nFamily) const
{
    rtl::Reference < SvXMLImportPropertyMapper > xMapper;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID:
    {
        if(!xPresImpPropMapper.is())
        {
            rtl::Reference< XMLShapeImportHelper > aImpHelper = const_cast<SvXMLImport&>(GetImport()).GetShapeImport();
            const_cast<SdXMLStylesContext*>(this)->xPresImpPropMapper =
                aImpHelper->GetPresPagePropsMapper();
        }
        xMapper = xPresImpPropMapper;
        break;
    }

    case XML_STYLE_FAMILY_TABLE_COLUMN:
    case XML_STYLE_FAMILY_TABLE_ROW:
    case XML_STYLE_FAMILY_TABLE_CELL:
    {
        const rtl::Reference< XMLTableImport >& xTableImport( const_cast< SvXMLImport& >( GetImport() ).GetShapeImport()->GetShapeTableImport() );

        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TABLE_COLUMN: xMapper = xTableImport->GetColumnImportPropertySetMapper().get(); break;
        case XML_STYLE_FAMILY_TABLE_ROW: xMapper = xTableImport->GetRowImportPropertySetMapper().get(); break;
        case XML_STYLE_FAMILY_TABLE_CELL: xMapper = xTableImport->GetCellImportPropertySetMapper().get(); break;
        }
        break;
    }
    }

    // call base class
    if( !xMapper.is() )
        xMapper = SvXMLStylesContext::GetImportPropertyMapper(nFamily);
    return xMapper;
}

// Process all style and object info

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
        for(sal_uInt32 a(0); a < GetStyleCount(); a++)
        {
            const SvXMLStyleContext* pStyle = GetStyle(a);
            if (const XMLShapeStyleContext* pDocStyle = dynamic_cast<const XMLShapeStyleContext*>(pStyle))
            {
                SvXMLStylesContext* pStylesContext = GetSdImport().GetShapeImport()->GetStylesContext();
                if (pStylesContext)
                {
                    pStyle = pStylesContext->FindStyleChildContext(pStyle->GetFamily(), pStyle->GetParentName());

                    if (const XMLShapeStyleContext* pParentStyle = dynamic_cast<const XMLShapeStyleContext*>(pStyle))
                    {
                        if(pParentStyle->GetStyle().is())
                        {
                            const_cast<XMLShapeStyleContext*>(pDocStyle)->SetStyle(pParentStyle->GetStyle());
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
        ImpSetCellStyles();
        GetImport().GetShapeImport()->GetShapeTableImport()->finishStyles();

        // put style infos in the info set for other components ( content import f.e. )
        uno::Reference< beans::XPropertySet > xInfoSet( GetImport().getImportInfo() );
        if( xInfoSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

            if( xInfoSetInfo->hasPropertyByName("PageLayouts") )
                xInfoSet->setPropertyValue("PageLayouts", uno::makeAny( getPageLayouts() ) );
        }

    }
}

// set master-page styles (all with family="presentation" and a special
// prefix) on given master-page.

void SdXMLStylesContext::SetMasterPageStyles(SdXMLMasterPageContext const & rMaster) const
{
    const uno::Reference<container::XNameAccess>& rStyleFamilies =
        GetSdImport().GetLocalDocStyleFamilies();

    if (!rStyleFamilies.is())
        return;

    if (!rStyleFamilies->hasByName(rMaster.GetDisplayName()))
        return;

    try
    {
        uno::Reference< container::XNameAccess > xMasterPageStyles( rStyleFamilies->getByName(rMaster.GetDisplayName()), UNO_QUERY_THROW );
        OUString sPrefix(rMaster.GetDisplayName());
        sPrefix += "-";
        ImpSetGraphicStyles(xMasterPageStyles, XML_STYLE_FAMILY_SD_PRESENTATION_ID, sPrefix);
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL( "xmloff::SdXMLStylesContext::SetMasterPageStyles(), exception caught!" );
    }
}

// Process styles list:
// set graphic styles (all with family="graphics"). Remember xStyle at list element.

void SdXMLStylesContext::ImpSetGraphicStyles() const
{
    if(GetSdImport().GetLocalDocStyleFamilies().is()) try
    {
        const OUString sGraphicStyleName("graphics");
        uno::Reference< container::XNameAccess > xGraphicPageStyles( GetSdImport().GetLocalDocStyleFamilies()->getByName(sGraphicStyleName), uno::UNO_QUERY_THROW );

        ImpSetGraphicStyles(xGraphicPageStyles, XML_STYLE_FAMILY_SD_GRAPHICS_ID, OUString());
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( "xmloff::SdXMLStylesContext::ImpSetGraphicStyles(), exception caught!" );
    }
}

void SdXMLStylesContext::ImpSetCellStyles() const
{
    if(GetSdImport().GetLocalDocStyleFamilies().is()) try
    {
        const OUString sCellStyleName("cell");
        uno::Reference< container::XNameAccess > xGraphicPageStyles( GetSdImport().GetLocalDocStyleFamilies()->getByName(sCellStyleName), uno::UNO_QUERY_THROW );

        ImpSetGraphicStyles(xGraphicPageStyles, XML_STYLE_FAMILY_TABLE_CELL, OUString());
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( "xmloff::SdXMLStylesContext::ImpSetCellStyles(), exception caught!" );
    }
}

//Resolves: fdo#34987 if the style's auto height before and after is the same
//then don't reset it back to the underlying default of true for the small
//period before its going to be reset to false again. Doing this avoids the
//master page shapes from resizing themselves due to autoheight becoming
//enabled before having autoheight turned off again and getting stuck on that
//autosized height
static bool canSkipReset(const OUString &rName, const XMLPropStyleContext* pPropStyle,
    const uno::Reference< beans::XPropertySet > &rPropSet, const rtl::Reference < XMLPropertySetMapper >& rPrMap)
{
    bool bCanSkipReset = false;
    if (pPropStyle && rName == "TextAutoGrowHeight")
    {
        bool bOldStyleTextAutoGrowHeight(false);
        rPropSet->getPropertyValue("TextAutoGrowHeight") >>= bOldStyleTextAutoGrowHeight;

        sal_Int32 nIndexStyle = rPrMap->GetEntryIndex(XML_NAMESPACE_DRAW, "auto-grow-height", 0);
        if (nIndexStyle != -1)
        {
            const ::std::vector< XMLPropertyState > &rProperties = pPropStyle->GetProperties();
            auto property = std::find_if(rProperties.cbegin(), rProperties.cend(),
                [nIndexStyle](const XMLPropertyState& rProp) { return rProp.mnIndex == nIndexStyle; });
            if (property != rProperties.cend())
            {
                bool bNewStyleTextAutoGrowHeight(false);
                property->maValue >>= bNewStyleTextAutoGrowHeight;
                bCanSkipReset = (bNewStyleTextAutoGrowHeight == bOldStyleTextAutoGrowHeight);
            }
        }
    }
    return bCanSkipReset;
}

// help function used by ImpSetGraphicStyles() and ImpSetMasterPageStyles()

void SdXMLStylesContext::ImpSetGraphicStyles( uno::Reference< container::XNameAccess > const & xPageStyles,  sal_uInt16 nFamily,  const OUString& rPrefix) const
{
    sal_Int32 nPrefLen(rPrefix.getLength());

    sal_uInt32 a;

    // set defaults
    for( a = 0; a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(nFamily == pStyle->GetFamily() && pStyle->IsDefaultStyle())
        {
            const_cast<SvXMLStyleContext*>(pStyle)->SetDefaults();
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
                OUString aStyleName(pStyle->GetDisplayName());

                if( nPrefLen )
                {
                    sal_Int32 nStylePrefLen = aStyleName.lastIndexOf( '-' ) + 1;
                    if( (nPrefLen != nStylePrefLen) || !aStyleName.startsWith(rPrefix) )
                        continue;

                    aStyleName = aStyleName.copy( nPrefLen );
                }

                XMLPropStyleContext* pPropStyle = dynamic_cast< XMLPropStyleContext* >(const_cast< SvXMLStyleContext* >( pStyle ) );

                uno::Reference< style::XStyle > xStyle;
                if(xPageStyles->hasByName(aStyleName))
                {
                    xPageStyles->getByName(aStyleName) >>= xStyle;

                    // set properties of existing styles to default
                    uno::Reference< beans::XPropertySet > xPropSet( xStyle, uno::UNO_QUERY );
                    uno::Reference< beans::XPropertySetInfo > xPropSetInfo;
                    if( xPropSet.is() )
                        xPropSetInfo = xPropSet->getPropertySetInfo();

                    uno::Reference< beans::XPropertyState > xPropState( xStyle, uno::UNO_QUERY );

                    if( xPropState.is() )
                    {
                        rtl::Reference < XMLPropertySetMapper > xPrMap;
                        rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap = GetImportPropertyMapper( nFamily );
                        SAL_WARN_IF( !xImpPrMap.is(), "xmloff", "There is the import prop mapper" );
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
                                    bool bCanSkipReset = canSkipReset(rName, pPropStyle, xPropSet, xPrMap);
                                    if (bCanSkipReset)
                                        continue;
                                    xPropState->setPropertyToDefault( rName );
                                }
                            }
                        }
                    }
                }
                else
                {
                    // graphics style does not exist, create and add it
                    uno::Reference< lang::XSingleServiceFactory > xServiceFact(xPageStyles, uno::UNO_QUERY);
                    if(xServiceFact.is())
                    {
                        uno::Reference< style::XStyle > xNewStyle( xServiceFact->createInstance(), uno::UNO_QUERY);

                        if(xNewStyle.is())
                        {
                            // remember style
                            xStyle = xNewStyle;

                            // add new style to graphics style pool
                            uno::Reference< container::XNameContainer > xInsertContainer(xPageStyles, uno::UNO_QUERY);
                            if(xInsertContainer.is())
                                xInsertContainer->insertByName(aStyleName, uno::Any( xStyle ) );
                        }
                    }
                }

                if(xStyle.is())
                {
                    // set properties at style
                    uno::Reference< beans::XPropertySet > xPropSet(xStyle, uno::UNO_QUERY);
                    if(xPropSet.is() && pPropStyle)
                    {
                        pPropStyle->FillPropertySet(xPropSet);
                        pPropStyle->SetStyle(xStyle);
                    }
                }
            }
        }
        catch(const Exception& e)
        {
            uno::Sequence<OUString> aSeq(0);
            const_cast<SdXMLImport*>(&GetSdImport())->SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, nullptr );
        }
    }

    // now set parents for all styles (when necessary)
    for(a = 0; a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(pStyle && !pStyle->GetDisplayName().isEmpty() && (nFamily == pStyle->GetFamily())) try
        {
            OUString aStyleName(pStyle->GetDisplayName());
            if( nPrefLen )
            {
                sal_Int32 nStylePrefLen = aStyleName.lastIndexOf( '-' ) + 1;
                if( (nPrefLen != nStylePrefLen) || !aStyleName.startsWith( rPrefix ) )
                    continue;

                aStyleName = aStyleName.copy( nPrefLen );
            }

            uno::Reference< style::XStyle > xStyle( xPageStyles->getByName(aStyleName), UNO_QUERY );
            if(xStyle.is())
            {
                // set parent style name
                OUString sParentStyleDisplayName( GetImport().GetStyleDisplayName( pStyle->GetFamily(), pStyle->GetParentName() ) );
                if( nPrefLen )
                {
                    sal_Int32 nStylePrefLen = sParentStyleDisplayName.lastIndexOf( '-' ) + 1;
                    if( (nPrefLen != nStylePrefLen) || !sParentStyleDisplayName.startsWith( rPrefix ) )
                        continue;

                    sParentStyleDisplayName = sParentStyleDisplayName.copy( nPrefLen );
                }
                xStyle->setParentStyle( sParentStyleDisplayName );
            }
        }
        catch( const Exception& e )
        {
            uno::Sequence<OUString> aSeq(0);
            const_cast<SdXMLImport*>(&GetSdImport())->SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, nullptr );
        }
    }
}

// helper function to create the uno component that hold the mappings from
// xml auto layout name to internal autolayout id

uno::Reference< container::XNameAccess > SdXMLStylesContext::getPageLayouts() const
{
    uno::Reference< container::XNameContainer > xLayouts( comphelper::NameContainer_createInstance( ::cppu::UnoType<sal_Int32>::get()) );

    for(sal_uInt32 a(0); a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);
        if (const SdXMLPresentationPageLayoutContext* pContext = dynamic_cast<const SdXMLPresentationPageLayoutContext*>(pStyle))
        {
            xLayouts->insertByName(pStyle->GetName(), uno::makeAny(static_cast<sal_Int32>(pContext->GetTypeId())));
        }
    }

    return uno::Reference< container::XNameAccess >::query( xLayouts );
}


SdXMLMasterStylesContext::SdXMLMasterStylesContext(
    SdXMLImport& rImport,
    const OUString& rLName)
:   SvXMLImportContext( rImport, XML_NAMESPACE_OFFICE, rLName)
{
}

SvXMLImportContextRef SdXMLMasterStylesContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    SvXMLImportContextRef xContext;

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
                xMasterPages->getByIndex(GetSdImport().GetNewMasterPageCount()) >>= xNewMasterPage;
            }

            // increment global import page counter
            GetSdImport().IncrementNewMasterPageCount();

            if(xNewMasterPage.is())
            {
                uno::Reference< drawing::XShapes > xNewShapes(xNewMasterPage, uno::UNO_QUERY);
                if(xNewShapes.is() && GetSdImport().GetShapeImport()->GetStylesContext())
                {
                    const rtl::Reference<SdXMLMasterPageContext> xLclContext{
                        new SdXMLMasterPageContext(GetSdImport(),
                            nPrefix, rLocalName, xAttrList, xNewShapes)};
                    xContext = xLclContext.get();
                    maMasterPageList.push_back(xLclContext);
                }
            }
        }
    }
    else if(nPrefix == XML_NAMESPACE_STYLE
        && IsXMLToken( rLocalName, XML_HANDOUT_MASTER ) )
    {
        uno::Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetSdImport().GetModel(), uno::UNO_QUERY );
        if( xHandoutSupp.is() )
        {
            uno::Reference< drawing::XShapes > xHandoutPage( xHandoutSupp->getHandoutMasterPage(), uno::UNO_QUERY );
            if(xHandoutPage.is() && GetSdImport().GetShapeImport()->GetStylesContext())
            {
                xContext = new SdXMLMasterPageContext(GetSdImport(),
                    nPrefix, rLocalName, xAttrList, xHandoutPage);
            }
        }
    }
    else if( (nPrefix == XML_NAMESPACE_DRAW )&& IsXMLToken( rLocalName, XML_LAYER_SET ) )
    {
        xContext = new SdXMLLayerSetContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }

    // call base class
    if (!xContext)
        xContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return xContext;
}

SdXMLHeaderFooterDeclContext::SdXMLHeaderFooterDeclContext(SvXMLImport& rImport,
    sal_uInt16 nPrfx, const OUString& rLName,
    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList)
    : SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList)
    , mbFixed(false)
{
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString aLocalName;
        const OUString aValue( xAttrList->getValueByIndex(i) );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(xAttrList->getNameByIndex(i), &aLocalName);

        if( nPrefix == XML_NAMESPACE_PRESENTATION )
        {
            if( IsXMLToken( aLocalName, XML_NAME ) )
            {
                maStrName = aValue;
            }
            else if( IsXMLToken( aLocalName, XML_SOURCE ) )
            {
                mbFixed = IsXMLToken( aValue, XML_FIXED );
            }
        }
        else if( nPrefix == XML_NAMESPACE_STYLE )
        {
            if( IsXMLToken( aLocalName, XML_DATA_STYLE_NAME ) )
            {
                maStrDateTimeFormat = aValue;
            }
        }
    }
}

bool SdXMLHeaderFooterDeclContext::IsTransient() const
{
    return true;
}

void SdXMLHeaderFooterDeclContext::EndElement()
{
    SdXMLImport& rImport = dynamic_cast<SdXMLImport&>(GetImport());
    if( IsXMLToken( GetLocalName(), XML_HEADER_DECL ) )
    {
        rImport.AddHeaderDecl( maStrName, maStrText );
    }
    else if( IsXMLToken( GetLocalName(), XML_FOOTER_DECL ) )
    {
        rImport.AddFooterDecl( maStrName, maStrText );
    }
    else if( IsXMLToken( GetLocalName(), XML_DATE_TIME_DECL ) )
    {
        rImport.AddDateTimeDecl( maStrName, maStrText, mbFixed, maStrDateTimeFormat );
    }
}

void SdXMLHeaderFooterDeclContext::Characters( const OUString& rChars )
{
    maStrText += rChars;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
