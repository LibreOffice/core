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
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "ximpnote.hxx"
#include <tools/debug.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#include <comphelper/namecontainer.hxx>
#include <xmloff/xmlprcon.hxx>
#include <xmloff/families.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <svl/zforlist.hxx>
#include "PropertySetMerger.hxx"
#include "sdpropls.hxx"
#include "layerimp.hxx"
#include <xmloff/XMLGraphicsDefaultStyle.hxx>
#include "XMLNumberStylesImport.hxx"
#include "xmloff/xmlerror.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class SdXMLDrawingPagePropertySetContext : public SvXMLPropertySetContext
{
public:

    TYPEINFO();

    SdXMLDrawingPagePropertySetContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                 const ::com::sun::star::uno::Reference<
                         ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap );

    virtual ~SdXMLDrawingPagePropertySetContext();

    using SvXMLPropertySetContext::CreateChildContext;
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
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
                   sal_uInt16 p_nPrefix,
                   const OUString& rLocalName,
                   const uno::Reference< xml::sax::XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContext *pContext = 0;

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

    if( !pContext )
        pContext = SvXMLPropertySetContext::CreateChildContext( p_nPrefix, rLocalName,
                                                            xAttrList,
                                                            rProperties, rProp );

    return pContext;
}

class SdXMLDrawingPageStyleContext : public XMLPropStyleContext
{
public:
    TYPEINFO();

    SdXMLDrawingPageStyleContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        SvXMLStylesContext& rStyles,
        sal_uInt16 nFamily = XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID);
    virtual ~SdXMLDrawingPageStyleContext();

    SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void Finish( sal_Bool bOverwrite );

    // #i35918#
    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );
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
    for(; property != rProperties.end(); ++property)
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

// #i35918#
void SdXMLDrawingPageStyleContext::FillPropertySet(
    const Reference< beans::XPropertySet > & rPropSet )
{
    static const sal_uInt16 MAX_SPECIAL_DRAW_STYLES = 7;
    struct _ContextID_Index_Pair aContextIDs[MAX_SPECIAL_DRAW_STYLES+1] =
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

    UniReference < SvXMLImportPropertyMapper > xImpPrMap =
        GetStyles()->GetImportPropertyMapper( GetFamily() );
    DBG_ASSERT( xImpPrMap.is(), "There is the import prop mapper" );
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
            UniReference<XMLPropertySetMapper> rPropMapper =
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

SdXMLPageMasterContext::~SdXMLPageMasterContext()
{
    // release remembered contexts, they are no longer needed
    if(mpPageMasterStyle)
    {
        mpPageMasterStyle->ReleaseRef();
        mpPageMasterStyle = 0L;
    }
}

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
            maList.push_back( (SdXMLPresentationPlaceholderContext*)pContext );
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
    if( !maList.empty() )
    {
        SdXMLPresentationPlaceholderContext* pObj0 = maList[ 0 ];
        if( pObj0->GetName() == "handout" )
        {
            switch( maList.size() )
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
            case 9:
                mnTypeId = 31; // AUTOLAYOUT_HANDOUT9
                break;
            default:
                mnTypeId = 26; // AUTOLAYOUT_HANDOUT6
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
                        mnTypeId = 19; // AUTOLAYOUT_ONLY_TITLE
                    }
                    else
                    {
                        mnTypeId = 32; // AUTOLAYOUT_ONLY_TEXT
                    }
                    break;
                }
                case 2:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList[ 1 ];

                    if( pObj1->GetName() == "subtitle" )
                    {
                        mnTypeId = 0; // AUTOLAYOUT_TITLE
                    }
                    else if( pObj1->GetName() == "outline" )
                    {
                        mnTypeId = 1; // AUTOLAYOUT_ENUM
                    }
                    else if( pObj1->GetName() == "chart" )
                    {
                        mnTypeId = 2; // AUTOLAYOUT_CHART
                    }
                    else if( pObj1->GetName() == "table" )
                    {
                        mnTypeId = 8; // AUTOLAYOUT_TAB
                    }
                    else if( pObj1->GetName() == "object" )
                    {
                        mnTypeId = 11; // AUTOLAYOUT_OBJ
                    }
                    else if( pObj1->GetName() == "vertical_outline" )
                    {
                        if( pObj0->GetName() == "vertical_title" )
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
                    SdXMLPresentationPlaceholderContext* pObj1 = maList[ 1 ];
                    SdXMLPresentationPlaceholderContext* pObj2 = maList[ 2 ];

                    if( pObj1->GetName() == "outline" )
                    {
                        if( pObj2->GetName() == "outline" )
                        {
                            mnTypeId = 3; // AUTOLAYOUT_2TEXT
                        }
                        else if( pObj2->GetName() == "chart" )
                        {
                            mnTypeId = 4; // AUTOLAYOUT_TEXTCHART
                        }
                        else if( pObj2->GetName() == "graphic" )
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
                    else if( pObj1->GetName() == "chart" )
                    {
                        mnTypeId = 7; // AUTOLAYOUT_CHARTTEXT
                    }
                    else if( pObj1->GetName() == "graphic" )
                    {
                        if( pObj2->GetName() == "vertical_outline" )
                        {
                            // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART
                            mnTypeId = 30;
                        }
                        else
                        {
                            mnTypeId = 9; // AUTOLAYOUT_CLIPTEXT
                        }
                    }
                    else if( pObj1->GetName() == "vertical_outline" )
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
                    SdXMLPresentationPlaceholderContext* pObj1 = maList[ 1 ];
                    SdXMLPresentationPlaceholderContext* pObj2 = maList[ 2 ];

                    if( pObj1->GetName() == "object" )
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
                case 5:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList[ 1 ];

                    if( pObj1->GetName() == "object" )
                    {
                        mnTypeId = 18; // AUTOLAYOUT_4OBJ
                    }
                    else
                    {
                        mnTypeId = 33; // AUTOLAYOUT_4CLIPART
                    }
                     break;

                }
                case 7:
                {
                    mnTypeId = 33; // AUTOLAYOUT_6CLIPART
                    break;
                }
                default:
                {
                    mnTypeId = 20; // AUTOLAYOUT_NONE
                    break;
                }
            }
        }

        // release remembered contexts, they are no longer needed
        for ( size_t i = maList.size(); i > 0; )
            maList[ --i ]->ReleaseRef();
        maList.clear();
    }
}

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
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnX, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_Y:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnY, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnWidth, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnHeight, sValue);
                break;
            }
        }
    }
}

SdXMLPresentationPlaceholderContext::~SdXMLPresentationPlaceholderContext()
{
}

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
            case XML_TOK_MASTERPAGE_DISPLAY_NAME:
            {
                msDisplayName = sValue;
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
    if(!msPageMasterName.isEmpty())
    {
        SetPageMaster( msPageMasterName );
    }

    SetStyle( msStyleName );

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
        if( pContext && pContext->ISA( SvXMLStyleContext ) )
            ((SdXMLStylesContext*)pContext)->SetMasterPageStyles(*this);
    }

    SdXMLGenericPageContext::EndElement();
    GetImport().GetShapeImport()->endPage(GetLocalShapesContext());
}

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
    Reference< uno::XComponentContext > xContext = rImport.GetComponentContext();
    mpNumFormatter = new SvNumberFormatter( xContext, LANGUAGE_SYSTEM );
    mpNumFmtHelper = new SvXMLNumFmtHelper( mpNumFormatter, xContext );
}

SdXMLStylesContext::~SdXMLStylesContext()
{
    delete mpNumFmtHelper;
    delete mpNumFormatter;
}

SvXMLStyleContext* SdXMLStylesContext::CreateStyleChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    SvXMLStyleContext* pContext = 0;
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
    const uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList)
{
    SvXMLStyleContext* pContext = 0;

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

sal_uInt16 SdXMLStylesContext::GetFamily( const OUString& rFamily ) const
{
    // call base class
    return SvXMLStylesContext::GetFamily(rFamily);
}

UniReference< SvXMLImportPropertyMapper > SdXMLStylesContext::GetImportPropertyMapper(
    sal_uInt16 nFamily) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID:
    {
        if(!xPresImpPropMapper.is())
        {
            UniReference< XMLShapeImportHelper > aImpHelper = ((SvXMLImport&)GetImport()).GetShapeImport();
            ((SdXMLStylesContext*)this)->xPresImpPropMapper =
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

                SvXMLStylesContext* pStylesContext = GetSdImport().GetShapeImport()->GetStylesContext();
                if( pStylesContext )
                {
                    pStyle = pStylesContext->FindStyleChildContext(pStyle->GetFamily(), pStyle->GetParentName());

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
//
void SdXMLStylesContext::SetMasterPageStyles(SdXMLMasterPageContext& rMaster) const
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
        sPrefix += OUString(static_cast<sal_Unicode>('-'));
        ImpSetGraphicStyles(xMasterPageStyles, XML_STYLE_FAMILY_SD_PRESENTATION_ID, sPrefix);
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL( "xmloff::SdXMLStylesContext::SetMasterPageStyles(), exception caught!" );
    }
}

// Process styles list:
// set graphic styles (all with family="graphics"). Remember xStyle at list element.
//
void SdXMLStylesContext::ImpSetGraphicStyles() const
{
    if(GetSdImport().GetLocalDocStyleFamilies().is()) try
    {
        const OUString sGraphicStyleName(OUString("graphics"));
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
        const OUString sCellStyleName(OUString("cell"));
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
    const uno::Reference< beans::XPropertySet > &rPropSet, const UniReference < XMLPropertySetMapper >& rPrMap)
{
    bool bCanSkipReset = false;
    if (pPropStyle && rName == "TextAutoGrowHeight")
    {
        sal_Bool bOldStyleTextAutoGrowHeight(sal_False);
        rPropSet->getPropertyValue("TextAutoGrowHeight") >>= bOldStyleTextAutoGrowHeight;

        sal_Int32 nIndexStyle = rPrMap->GetEntryIndex(XML_NAMESPACE_DRAW, "auto-grow-height", 0);
        if (nIndexStyle != -1)
        {
            const ::std::vector< XMLPropertyState > &rProperties = pPropStyle->GetProperties();
            ::std::vector< XMLPropertyState >::const_iterator property = rProperties.begin();
            for(; property != rProperties.end(); ++property)
            {
                sal_Int32 nIdx = property->mnIndex;
                if (nIdx == nIndexStyle)
                {
                    sal_Bool bNewStyleTextAutoGrowHeight(sal_False);
                    property->maValue >>= bNewStyleTextAutoGrowHeight;
                    if (bNewStyleTextAutoGrowHeight == bOldStyleTextAutoGrowHeight)
                        bCanSkipReset = true;;
                    break;
                }
            }
        }
    }
    return bCanSkipReset;
}

// help function used by ImpSetGraphicStyles() and ImpSetMasterPageStyles()
//
void SdXMLStylesContext::ImpSetGraphicStyles( uno::Reference< container::XNameAccess >& xPageStyles,  sal_uInt16 nFamily,  const OUString& rPrefix) const
{
    sal_Int32 nPrefLen(rPrefix.getLength());

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
                OUString aStyleName(pStyle->GetDisplayName());

                if( nPrefLen )
                {
                    sal_Int32 nStylePrefLen = aStyleName.lastIndexOf( sal_Unicode('-') ) + 1;
                    if( (nPrefLen != nStylePrefLen) ||  (aStyleName.compareTo( rPrefix, nPrefLen ) != 0) )
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
            const_cast<SdXMLImport*>(&GetSdImport())->SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, NULL );
        }
    }

    // now set parents for all styles (when necessary)
    for(a = 0L; a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(pStyle && !pStyle->GetDisplayName().isEmpty() && (nFamily == pStyle->GetFamily())) try
        {
            OUString aStyleName(pStyle->GetDisplayName());
            if( nPrefLen )
            {
                sal_Int32 nStylePrefLen = aStyleName.lastIndexOf( sal_Unicode('-') ) + 1;
                if( (nPrefLen != nStylePrefLen) ||  (aStyleName.compareTo( rPrefix, nPrefLen ) != 0) )
                    continue;

                aStyleName = aStyleName.copy( nPrefLen );
            }

            uno::Reference< style::XStyle > xStyle( xPageStyles->getByName(aStyleName), UNO_QUERY );
            if(xStyle.is())
            {
                // set parent style name
                OUString sParentStyleDisplayName( const_cast< SvXMLImport& >( GetImport() ).GetStyleDisplayName( pStyle->GetFamily(), pStyle->GetParentName() ) );
                if( nPrefLen )
                {
                    sal_Int32 nStylePrefLen = sParentStyleDisplayName.lastIndexOf( sal_Unicode('-') ) + 1;
                    if( (nPrefLen != nStylePrefLen) || (sParentStyleDisplayName.compareTo( rPrefix, nPrefLen ) != 0) )
                        continue;

                    sParentStyleDisplayName = sParentStyleDisplayName.copy( nPrefLen );
                }
                xStyle->setParentStyle( sParentStyleDisplayName );
            }
        }
        catch( const Exception& e )
        {
            uno::Sequence<OUString> aSeq(0);
            const_cast<SdXMLImport*>(&GetSdImport())->SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, NULL );
        }
    }
}

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

//
TYPEINIT1( SdXMLMasterStylesContext, SvXMLImportContext );

SdXMLMasterStylesContext::SdXMLMasterStylesContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName)
:   SvXMLImportContext( rImport, nPrfx, rLName)
{
}

SdXMLMasterStylesContext::~SdXMLMasterStylesContext()
{
    for ( size_t i = maMasterPageList.size(); i > 0; )
        maMasterPageList[ --i ]->ReleaseRef();
    maMasterPageList.clear();
}

SvXMLImportContext* SdXMLMasterStylesContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
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
                xMasterPages->getByIndex(GetSdImport().GetNewMasterPageCount()) >>= xNewMasterPage;
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
                        maMasterPageList.push_back( (SdXMLMasterPageContext*)pContext );
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
    else if( (nPrefix == XML_NAMESPACE_DRAW )&& IsXMLToken( rLocalName, XML_LAYER_SET ) )
    {
        pContext = new SdXMLLayerSetContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }

    // call base class
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

SdXMLHeaderFooterDeclContext::SdXMLHeaderFooterDeclContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
: SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList )
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

sal_Bool SdXMLHeaderFooterDeclContext::IsTransient() const
{
    return sal_True;
}

void SdXMLHeaderFooterDeclContext::EndElement()
{
    SdXMLImport& rImport = *dynamic_cast< SdXMLImport* >( &GetImport() );
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
