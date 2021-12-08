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
#include <xmloff/maptype.hxx>
#include <xmloff/XMLDrawingPageStyleContext.hxx>
#include <xmloff/XMLShapeStyleContext.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "ximpnote.hxx"
#include <xmlsdtypes.hxx>
#include <tools/debug.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#include <com/sun/star/util/Color.hpp>
#include <comphelper/namecontainer.hxx>
#include <xmloff/autolayout.hxx>
#include <xmloff/xmlprcon.hxx>
#include <xmloff/families.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include "layerimp.hxx"
#include <xmloff/XMLGraphicsDefaultStyle.hxx>
#include <XMLNumberStylesImport.hxx>
#include <unotools/configmgr.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/table/XMLTableImport.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <sax/tools/converter.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

namespace {

class SdXMLDrawingPagePropertySetContext : public SvXMLPropertySetContext
{
public:

    SdXMLDrawingPagePropertySetContext( SvXMLImport& rImport, sal_Int32 nElement,
                 const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const rtl::Reference < SvXMLImportPropertyMapper > &rMap );

    using SvXMLPropertySetContext::createFastChildContext;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
        ::std::vector< XMLPropertyState > &rProperties,
        const XMLPropertyState& rProp ) override;
};

/// Imports <loext:theme>.
class XMLThemeContext : public SvXMLImportContext
{
    uno::Reference<beans::XPropertySet> m_xMasterPage;
    comphelper::SequenceAsHashMap m_aTheme;

public:
    XMLThemeContext(SvXMLImport& rImport,
                    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                    const uno::Reference<beans::XPropertySet>& xMasterPage);
    ~XMLThemeContext();

    uno::Reference<xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs) override;
};

/// Imports <loext:color-table> inside <loext:theme>.
class XMLColorTableContext : public SvXMLImportContext
{
    comphelper::SequenceAsHashMap& m_rTheme;
    std::vector<util::Color> m_aColorScheme;

public:
    XMLColorTableContext(SvXMLImport& rImport,
                         const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                         comphelper::SequenceAsHashMap& rTheme);
    ~XMLColorTableContext();

    uno::Reference<xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs) override;
};

/// Imports <loext:color> inside <loext:color-table>.
class XMLColorContext : public SvXMLImportContext
{
public:
    XMLColorContext(SvXMLImport& rImport,
                    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                    std::vector<util::Color>& rColorScheme);
};
}

SdXMLDrawingPagePropertySetContext::SdXMLDrawingPagePropertySetContext(
                 SvXMLImport& rImport, sal_Int32 nElement,
                 const uno::Reference< xml::sax::XFastAttributeList > & xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const rtl::Reference < SvXMLImportPropertyMapper > &rMap ) :
    SvXMLPropertySetContext( rImport, nElement, xAttrList,
                             XML_TYPE_PROP_DRAWING_PAGE, rProps, rMap )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLDrawingPagePropertySetContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
    ::std::vector< XMLPropertyState > &rProperties,
    const XMLPropertyState& rProp )
{
    switch( mxMapper->getPropertySetMapper()->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_PAGE_SOUND_URL:
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
        {
            if( aIter.getToken() == XML_ELEMENT(XLINK, XML_HREF) )
            {
                uno::Any aAny( GetImport().GetAbsoluteReference( aIter.toString() ) );
                XMLPropertyState aPropState( rProp.mnIndex, aAny );
                rProperties.push_back( aPropState );
            }
            else
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
        break;
    }
    }

    return SvXMLPropertySetContext::createFastChildContext( nElement,
                                                            xAttrList,
                                                            rProperties, rProp );
}

namespace {


class SdXMLDrawingPageStyleContext : public XMLDrawingPageStyleContext
{
public:

    SdXMLDrawingPageStyleContext(
        SvXMLImport& rImport,
        SvXMLStylesContext& rStyles);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void Finish( bool bOverwrite ) override;
};

const sal_uInt16 MAX_SPECIAL_DRAW_STYLES = 7;
ContextID_Index_Pair const g_ContextIDs[MAX_SPECIAL_DRAW_STYLES+1] =
{
    { CTF_DASHNAME,         -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
    { CTF_LINESTARTNAME,    -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
    { CTF_LINEENDNAME,      -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
    { CTF_FILLGRADIENTNAME, -1, drawing::FillStyle::FillStyle_GRADIENT},
    { CTF_FILLTRANSNAME,    -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
    { CTF_FILLHATCHNAME,    -1, drawing::FillStyle::FillStyle_HATCH },
    { CTF_FILLBITMAPNAME,   -1, drawing::FillStyle::FillStyle_BITMAP },
    { -1, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE }
};
XmlStyleFamily const g_Families[MAX_SPECIAL_DRAW_STYLES] =
{
    XmlStyleFamily::SD_STROKE_DASH_ID,
    XmlStyleFamily::SD_MARKER_ID,
    XmlStyleFamily::SD_MARKER_ID,
    XmlStyleFamily::SD_GRADIENT_ID,
    XmlStyleFamily::SD_GRADIENT_ID,
    XmlStyleFamily::SD_HATCH_ID,
    XmlStyleFamily::SD_FILL_IMAGE_ID
};

}

XMLDrawingPageStyleContext::XMLDrawingPageStyleContext(
    SvXMLImport& rImport,
    SvXMLStylesContext& rStyles,
    ContextID_Index_Pair const pContextIDs[],
    XmlStyleFamily const pFamilies[])
    : XMLPropStyleContext(rImport, rStyles, XmlStyleFamily::SD_DRAWINGPAGE_ID)
    , m_pFamilies(pFamilies)
{
    size_t size(1); // for the -1 entry
    for (ContextID_Index_Pair const* pTemp(pContextIDs); pTemp->nContextID != -1; ++size, ++pTemp);
    m_pContextIDs.reset(new ContextID_Index_Pair[size]);
    std::memcpy(m_pContextIDs.get(), pContextIDs, size * sizeof(ContextID_Index_Pair));
}

SdXMLDrawingPageStyleContext::SdXMLDrawingPageStyleContext(
    SvXMLImport& rImport,
    SvXMLStylesContext& rStyles)
    : XMLDrawingPageStyleContext(rImport, rStyles, g_ContextIDs, g_Families)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLDrawingPageStyleContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(STYLE, XML_DRAWING_PAGE_PROPERTIES) )
    {
        rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
            return new SdXMLDrawingPagePropertySetContext( GetImport(), nElement,
                                                    xAttrList,
                                                    GetProperties(),
                                                    xImpPrMap );
    }

    return XMLPropStyleContext::createFastChildContext( nElement, xAttrList );
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
                        GetStyles()->FindStyleChildContext( XmlStyleFamily::DATA_STYLE, sStyleName, true ) );

                if( pSdNumStyle )
                    nStyle = pSdNumStyle->GetDrawKey();

                property.maValue <<= nStyle;
            }
            break;
        }
    }

}


// #i35918#
void XMLDrawingPageStyleContext::FillPropertySet(
    const Reference< beans::XPropertySet > & rPropSet )
{
    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
        GetStyles()->GetImportPropertyMapper( GetFamily() );
    SAL_WARN_IF( !xImpPrMap.is(), "xmloff", "There is the import prop mapper" );
    if( xImpPrMap.is() )
        xImpPrMap->FillPropertySet(GetProperties(), rPropSet, m_pContextIDs.get());

    Reference< beans::XPropertySetInfo > xInfo;
    for (size_t i=0; m_pContextIDs[i].nContextID != -1; ++i)
    {
        sal_Int32 nIndex = m_pContextIDs[i].nIndex;
        if( nIndex != -1 )
        {
            struct XMLPropertyState& rState = GetProperties()[nIndex];
            OUString sStyleName;
            rState.maValue >>= sStyleName;

            if (::xmloff::IsIgnoreFillStyleNamedItem(rPropSet, m_pContextIDs[i].nExpectedFillStyle))
            {
                SAL_INFO("xmloff.style", "XMLDrawingPageStyleContext: dropping fill named item: " << sStyleName);
                break; // ignore it, it's not used
            }

            sStyleName = GetImport().GetStyleDisplayName( m_pFamilies[i],
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
    sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList)
:   SvXMLStyleContext(rImport, XmlStyleFamily::SD_PAGEMASTERSTYLECONTEXT_ID),
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

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(FO, XML_MARGIN_TOP):
            case XML_ELEMENT(FO_COMPAT, XML_MARGIN_TOP):
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnBorderTop, aIter.toView());
                break;
            }
            case XML_ELEMENT(FO, XML_MARGIN_BOTTOM):
            case XML_ELEMENT(FO_COMPAT, XML_MARGIN_BOTTOM):
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnBorderBottom, aIter.toView());
                break;
            }
            case XML_ELEMENT(FO, XML_MARGIN_LEFT):
            case XML_ELEMENT(FO_COMPAT, XML_MARGIN_LEFT):
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnBorderLeft, aIter.toView());
                break;
            }
            case XML_ELEMENT(FO, XML_MARGIN_RIGHT):
            case XML_ELEMENT(FO_COMPAT, XML_MARGIN_RIGHT):
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnBorderRight, aIter.toView());
                break;
            }
            case XML_ELEMENT(FO, XML_PAGE_WIDTH):
            case XML_ELEMENT(FO_COMPAT, XML_PAGE_WIDTH):
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnWidth, aIter.toView());
                break;
            }
            case XML_ELEMENT(FO, XML_PAGE_HEIGHT):
            case XML_ELEMENT(FO_COMPAT, XML_PAGE_HEIGHT):
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnHeight, aIter.toView());
                break;
            }
            case XML_ELEMENT(STYLE, XML_PRINT_ORIENTATION):
            {
                if( IsXMLToken( aIter, XML_PORTRAIT ) )
                    meOrientation = view::PaperOrientation_PORTRAIT;
                else
                    meOrientation = view::PaperOrientation_LANDSCAPE;
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

SdXMLPageMasterStyleContext::~SdXMLPageMasterStyleContext()
{
}


SdXMLPageMasterContext::SdXMLPageMasterContext(
    SdXMLImport& rImport,
    sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList>& /*xAttrList*/)
:   SvXMLStyleContext(rImport, XmlStyleFamily::SD_PAGEMASTERCONTEXT_ID)
{
    // set family to something special at SvXMLStyleContext
    // for differences in search-methods

}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLPageMasterContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if(nElement == XML_ELEMENT(STYLE, XML_PAGE_LAYOUT_PROPERTIES))
    {
        DBG_ASSERT(!mxPageMasterStyle.is(), "PageMasterStyle is set, there seem to be two of them (!)");
        mxPageMasterStyle.set(new SdXMLPageMasterStyleContext(GetSdImport(), nElement, xAttrList));
        return mxPageMasterStyle;
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

SdXMLPresentationPageLayoutContext::SdXMLPresentationPageLayoutContext(
    SdXMLImport& rImport,
    sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/)
:   SvXMLStyleContext(rImport, XmlStyleFamily::SD_PRESENTATIONPAGELAYOUT_ID),
    mnTypeId( AUTOLAYOUT_NONE )
{
    // set family to something special at SvXMLStyleContext
    // for differences in search-methods
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLPresentationPageLayoutContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    if(nElement == XML_ELEMENT(PRESENTATION, XML_PLACEHOLDER))
    {
        const rtl::Reference< SdXMLPresentationPlaceholderContext > xLclContext{
            new SdXMLPresentationPlaceholderContext(GetSdImport(), nElement, xAttrList)};
        // presentation:placeholder inside style:presentation-page-layout context
        xContext = xLclContext.get();

        // remember SdXMLPresentationPlaceholderContext for later evaluation
        maList.push_back(xLclContext);
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return xContext;
}

void SdXMLPresentationPageLayoutContext::endFastElement(sal_Int32 )
{
    // build presentation page layout type here
    // calc mnTpeId due to content of maList
    // at the moment only use number of types used there
    if( maList.empty() )
        return;

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
                mnTypeId = AUTOLAYOUT_TITLE_6CONTENT; // tdf#141978: Apply 6content layout
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

SdXMLPresentationPlaceholderContext::SdXMLPresentationPlaceholderContext(
    SdXMLImport& rImport,
    sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList)
:   SvXMLImportContext( rImport ),
    mnX(0)
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(PRESENTATION, XML_OBJECT):
            {
                msName = aIter.toString();
                break;
            }
            case XML_ELEMENT(SVG, XML_X):
            case XML_ELEMENT(SVG_COMPAT, XML_X):
            {
                GetSdImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnX, aIter.toView());
                break;
            }
            case XML_ELEMENT(SVG, XML_Y):
            case XML_ELEMENT(SVG_COMPAT, XML_Y):
            {
                break;
            }
            case XML_ELEMENT(SVG, XML_WIDTH):
            case XML_ELEMENT(SVG_COMPAT, XML_WIDTH):
            {
                break;
            }
            case XML_ELEMENT(SVG, XML_HEIGHT):
            case XML_ELEMENT(SVG_COMPAT, XML_HEIGHT):
            {
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

SdXMLPresentationPlaceholderContext::~SdXMLPresentationPlaceholderContext()
{
}


SdXMLMasterPageContext::SdXMLMasterPageContext(
    SdXMLImport& rImport,
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXMLGenericPageContext( rImport, xAttrList, rShapes )
{
    const bool bHandoutMaster = (nElement & TOKEN_MASK) == XML_HANDOUT_MASTER;
    OUString sStyleName, sPageMasterName;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        const OUString sValue = aIter.toString();
        switch(aIter.getToken())
        {
            case XML_ELEMENT(STYLE, XML_NAME):
            {
                msName = sValue;
                break;
            }
            case XML_ELEMENT(STYLE, XML_DISPLAY_NAME):
            {
                msDisplayName = sValue;
                break;
            }
            case XML_ELEMENT(STYLE, XML_PAGE_LAYOUT_NAME):
            {
                sPageMasterName = sValue;
                break;
            }
            case XML_ELEMENT(DRAW, XML_STYLE_NAME):
            {
                sStyleName = sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_PRESENTATION_PAGE_LAYOUT_NAME):
            {
                maPageLayoutName = sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_HEADER_NAME):
            {
                maUseHeaderDeclName =  sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_FOOTER_NAME):
            {
                maUseFooterDeclName =  sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_DATE_TIME_NAME):
            {
                maUseDateTimeDeclName =  sValue;
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( msDisplayName.isEmpty() )
        msDisplayName = msName;
    else if( msDisplayName != msName )
        GetImport().AddStyleDisplayName( XmlStyleFamily::MASTER_PAGE, msName, msDisplayName );

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

void SdXMLMasterPageContext::endFastElement(sal_Int32 nElement)
{
    // set styles on master-page
    if(!msName.isEmpty() && GetSdImport().GetShapeImport()->GetStylesContext())
    {
        SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetStylesContext();
        if (SdXMLStylesContext* pSdContext = dynamic_cast<SdXMLStylesContext*>(pContext))
            pSdContext->SetMasterPageStyles(*this);
    }

    SdXMLGenericPageContext::endFastElement(nElement);
    GetImport().GetShapeImport()->endPage(GetLocalShapesContext());
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLMasterPageContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    switch (nElement)
    {
        // some special objects inside style:masterpage context
        case XML_ELEMENT(STYLE, XML_STYLE):
        {
            if(GetSdImport().GetShapeImport()->GetStylesContext())
            {
                // style:style inside master-page context -> presentation style
                XMLShapeStyleContext* pNew = new XMLShapeStyleContext(
                    GetSdImport(),
                    *GetSdImport().GetShapeImport()->GetStylesContext(),
                    XmlStyleFamily::SD_PRESENTATION_ID);

                // add this style to the outer StylesContext class for later processing
                GetSdImport().GetShapeImport()->GetStylesContext()->AddStyle(*pNew);
                return pNew;
            }
            break;
        }
        case XML_ELEMENT(PRESENTATION, XML_NOTES):
        {
            if( GetSdImport().IsImpress() )
            {
                // get notes page
                uno::Reference< presentation::XPresentationPage > xPresPage(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xPresPage.is())
                {
                    uno::Reference< drawing::XDrawPage > xNotesDrawPage = xPresPage->getNotesPage();
                    if(xNotesDrawPage.is())
                    {
                        // presentation:notes inside master-page context
                        return new SdXMLNotesContext( GetSdImport(),  xAttrList, xNotesDrawPage);
                    }
                }
            }
            break;
        }
        case XML_ELEMENT(LO_EXT, XML_THEME):
        {
            if (GetSdImport().IsImpress())
            {
                uno::Reference<beans::XPropertySet> xMasterPage(GetLocalShapesContext(),
                                                                uno::UNO_QUERY);
                return new XMLThemeContext(GetSdImport(), xAttrList, xMasterPage);
            }
            break;
        }
    }
    return SdXMLGenericPageContext::createFastChildContext(nElement, xAttrList);
}

SdXMLStylesContext::SdXMLStylesContext(
    SdXMLImport& rImport,
    bool bIsAutoStyle)
:   SvXMLStylesContext(rImport),
    mbIsAutoStyle(bIsAutoStyle)
{
    Reference< uno::XComponentContext > xContext = rImport.GetComponentContext();
    mpNumFormatter = std::make_unique<SvNumberFormatter>( xContext, LANGUAGE_SYSTEM );
    mpNumFmtHelper = std::make_unique<SvXMLNumFmtHelper>( mpNumFormatter.get(), xContext );
}

SvXMLStyleContext* SdXMLStylesContext::CreateStyleChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList)
{
    switch (nElement)
    {
        case XML_ELEMENT(TABLE, XML_TABLE_TEMPLATE):
        {
            auto pContext = GetImport().GetShapeImport()->GetShapeTableImport()->CreateTableTemplateContext(nElement, xAttrList );
            if (pContext)
                return pContext;
            break;
        }
        case XML_ELEMENT(STYLE, XML_PAGE_LAYOUT):
            // style:page-master inside office:styles context
            return new SdXMLPageMasterContext(GetSdImport(), nElement, xAttrList);
        case XML_ELEMENT(STYLE, XML_PRESENTATION_PAGE_LAYOUT):
            // style:presentation-page-layout inside office:styles context
            return new SdXMLPresentationPageLayoutContext(GetSdImport(), nElement, xAttrList);
        case XML_ELEMENT(NUMBER, XML_DATE_STYLE):
            // number:date-style or number:time-style
            return new SdXMLNumberFormatImportContext( GetSdImport(), nElement, mpNumFmtHelper->getData(), SvXMLStylesTokens::DATE_STYLE, xAttrList, *this );
        case XML_ELEMENT(NUMBER, XML_TIME_STYLE):
            // number:date-style or number:time-style
            return new SdXMLNumberFormatImportContext( GetSdImport(), nElement, mpNumFmtHelper->getData(), SvXMLStylesTokens::TIME_STYLE, xAttrList, *this );
        case XML_ELEMENT(NUMBER, XML_NUMBER_STYLE):
            return new SvXMLNumFormatContext( GetSdImport(), nElement,
                                            mpNumFmtHelper->getData(), SvXMLStylesTokens::NUMBER_STYLE, xAttrList, *this );
        case XML_ELEMENT(NUMBER, XML_CURRENCY_STYLE):
            return new SvXMLNumFormatContext( GetSdImport(), nElement,
                                            mpNumFmtHelper->getData(), SvXMLStylesTokens::CURRENCY_STYLE, xAttrList, *this );
        case XML_ELEMENT(NUMBER, XML_PERCENTAGE_STYLE):
            return new SvXMLNumFormatContext( GetSdImport(), nElement,
                                            mpNumFmtHelper->getData(), SvXMLStylesTokens::PERCENTAGE_STYLE, xAttrList, *this );
        case XML_ELEMENT(NUMBER, XML_BOOLEAN_STYLE):
            return new SvXMLNumFormatContext( GetSdImport(), nElement,
                                            mpNumFmtHelper->getData(), SvXMLStylesTokens::BOOLEAN_STYLE, xAttrList, *this );
        case XML_ELEMENT(NUMBER, XML_TEXT_STYLE):
            return new SvXMLNumFormatContext( GetSdImport(), nElement,
                                            mpNumFmtHelper->getData(), SvXMLStylesTokens::TEXT_STYLE, xAttrList, *this );
        case XML_ELEMENT(PRESENTATION, XML_HEADER_DECL):
        case XML_ELEMENT(PRESENTATION, XML_FOOTER_DECL):
        case XML_ELEMENT(PRESENTATION, XML_DATE_TIME_DECL):
            return new SdXMLHeaderFooterDeclContext( GetImport(), xAttrList );
        case XML_ELEMENT(STYLE, XML_STYLE):
            break; // ignore
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    // call base class
    return SvXMLStylesContext::CreateStyleChildContext(nElement, xAttrList);
}

SvXMLStyleContext* SdXMLStylesContext::CreateStyleStyleChildContext(
    XmlStyleFamily nFamily,
    sal_Int32 nElement,
    const uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    switch( nFamily )
    {
    case XmlStyleFamily::SD_DRAWINGPAGE_ID:
        return new SdXMLDrawingPageStyleContext(GetSdImport(), *this );
    case XmlStyleFamily::TABLE_CELL:
    case XmlStyleFamily::TABLE_COLUMN:
    case XmlStyleFamily::TABLE_ROW:
        return new XMLShapeStyleContext( GetSdImport(), *this, nFamily );
    default: break;
    }

    // call base class
    return SvXMLStylesContext::CreateStyleStyleChildContext(nFamily, nElement, xAttrList);
}

SvXMLStyleContext* SdXMLStylesContext::CreateDefaultStyleStyleChildContext(
    XmlStyleFamily nFamily,
    sal_Int32  nElement,
    const Reference< XFastAttributeList > & xAttrList )
{
    switch( nFamily )
    {
    case XmlStyleFamily::SD_GRAPHICS_ID:
        return new XMLGraphicsDefaultStyle(GetSdImport(), *this );
    default: break;
    }

    // call base class
    return SvXMLStylesContext::CreateDefaultStyleStyleChildContext(nFamily, nElement, xAttrList);
}

rtl::Reference< SvXMLImportPropertyMapper > SdXMLStylesContext::GetImportPropertyMapper(
    XmlStyleFamily nFamily) const
{
    rtl::Reference < SvXMLImportPropertyMapper > xMapper;

    switch( nFamily )
    {
    case XmlStyleFamily::SD_DRAWINGPAGE_ID:
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

    case XmlStyleFamily::TABLE_COLUMN:
    case XmlStyleFamily::TABLE_ROW:
    case XmlStyleFamily::TABLE_CELL:
    {
        const rtl::Reference< XMLTableImport >& xTableImport( const_cast< SvXMLImport& >( GetImport() ).GetShapeImport()->GetShapeTableImport() );

        switch( nFamily )
        {
        case XmlStyleFamily::TABLE_COLUMN: xMapper = xTableImport->GetColumnImportPropertySetMapper().get(); break;
        case XmlStyleFamily::TABLE_ROW: xMapper = xTableImport->GetRowImportPropertySetMapper().get(); break;
        case XmlStyleFamily::TABLE_CELL: xMapper = xTableImport->GetCellImportPropertySetMapper().get(); break;
        default: break;
        }
        break;
    }
    default: break;
    }

    // call base class
    if( !xMapper.is() )
        xMapper = SvXMLStylesContext::GetImportPropertyMapper(nFamily);
    return xMapper;
}

// Process all style and object info

void SdXMLStylesContext::endFastElement(sal_Int32 )
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
        OUString sPrefix(rMaster.GetDisplayName() + "-");
        ImpSetGraphicStyles(xMasterPageStyles, XmlStyleFamily::SD_PRESENTATION_ID, sPrefix);
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }
}

// Process styles list:
// set graphic styles (all with family="graphics"). Remember xStyle at list element.

void SdXMLStylesContext::ImpSetGraphicStyles() const
{
    if(GetSdImport().GetLocalDocStyleFamilies().is()) try
    {
        uno::Reference< container::XNameAccess > xGraphicPageStyles( GetSdImport().GetLocalDocStyleFamilies()->getByName("graphics"), uno::UNO_QUERY_THROW );

        ImpSetGraphicStyles(xGraphicPageStyles, XmlStyleFamily::SD_GRAPHICS_ID, OUString());
    }
    catch( uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }
}

void SdXMLStylesContext::ImpSetCellStyles() const
{
    if(GetSdImport().GetLocalDocStyleFamilies().is()) try
    {
        uno::Reference< container::XNameAccess > xGraphicPageStyles( GetSdImport().GetLocalDocStyleFamilies()->getByName("cell"), uno::UNO_QUERY_THROW );

        ImpSetGraphicStyles(xGraphicPageStyles, XmlStyleFamily::TABLE_CELL, OUString());
    }
    catch( uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }
}

//Resolves: fdo#34987 if the style's auto height before and after is the same
//then don't reset it back to the underlying default of true for the small
//period before it's going to be reset to false again. Doing this avoids the
//master page shapes from resizing themselves due to autoheight becoming
//enabled before having autoheight turned off again and getting stuck on that
//autosized height
static bool canSkipReset(std::u16string_view rName, const XMLPropStyleContext* pPropStyle,
    const uno::Reference< beans::XPropertySet > &rPropSet, const rtl::Reference < XMLPropertySetMapper >& rPrMap)
{
    bool bCanSkipReset = false;
    if (pPropStyle && rName == u"TextAutoGrowHeight")
    {
        bool bOldStyleTextAutoGrowHeight(false);
        rPropSet->getPropertyValue("TextAutoGrowHeight") >>= bOldStyleTextAutoGrowHeight;

        sal_Int32 nIndexStyle = rPrMap->GetEntryIndex(XML_NAMESPACE_DRAW, u"auto-grow-height", 0);
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

void SdXMLStylesContext::ImpSetGraphicStyles( uno::Reference< container::XNameAccess > const & xPageStyles,  XmlStyleFamily nFamily,  const OUString& rPrefix) const
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
            const_cast<SdXMLImport*>(&GetSdImport())->SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, {}, e.Message, nullptr );
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
            const_cast<SdXMLImport*>(&GetSdImport())->SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, {}, e.Message, nullptr );
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

    return xLayouts;
}


SdXMLMasterStylesContext::SdXMLMasterStylesContext(
    SdXMLImport& rImport)
:   SvXMLImportContext( rImport )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLMasterStylesContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(DRAW, XML_LAYER_SET) )
    {
        return new SdXMLLayerSetContext( GetImport() );
    }
    else if( nElement == XML_ELEMENT(STYLE, XML_MASTER_PAGE) )
    {
        // style:masterpage inside office:styles context
        uno::Reference< drawing::XDrawPage > xNewMasterPage;
        uno::Reference< drawing::XDrawPages > xMasterPages(GetSdImport().GetLocalMasterPages(), uno::UNO_QUERY);

        if( xMasterPages.is() )
        {
            sal_Int32 nNewMasterPageCount = GetSdImport().GetNewMasterPageCount();
            sal_Int32 nMasterPageCount = xMasterPages->getCount();
            if (nNewMasterPageCount + 1 > nMasterPageCount)
            {
                // arbitrary limit to master pages when fuzzing to avoid deadend timeouts
                if (nMasterPageCount >= 64 && utl::ConfigManager::IsFuzzing())
                    return nullptr;

                // new page, create and insert
                xNewMasterPage = xMasterPages->insertNewByIndex(nMasterPageCount);
            }
            else
            {
                // existing page, use it
                xMasterPages->getByIndex(nNewMasterPageCount) >>= xNewMasterPage;
            }

            // increment global import page counter
            GetSdImport().IncrementNewMasterPageCount();

            if(xNewMasterPage.is())
            {
                if(GetSdImport().GetShapeImport()->GetStylesContext())
                {
                    const rtl::Reference<SdXMLMasterPageContext> xLclContext{
                        new SdXMLMasterPageContext(GetSdImport(),
                            nElement, xAttrList, xNewMasterPage)};
                    maMasterPageList.push_back(xLclContext);
                    return xLclContext;
                }
            }
        }
    }
    else if( nElement == XML_ELEMENT(STYLE, XML_HANDOUT_MASTER) )
    {
        uno::Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetSdImport().GetModel(), uno::UNO_QUERY );
        if( xHandoutSupp.is() )
        {
            uno::Reference< drawing::XShapes > xHandoutPage = xHandoutSupp->getHandoutMasterPage();
            if(xHandoutPage.is() && GetSdImport().GetShapeImport()->GetStylesContext())
            {
                return new SdXMLMasterPageContext(GetSdImport(),
                    nElement, xAttrList, xHandoutPage);
            }
        }
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

SdXMLHeaderFooterDeclContext::SdXMLHeaderFooterDeclContext(SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
    : SvXMLStyleContext( rImport )
    , mbFixed(false)
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if( aIter.getToken() == XML_ELEMENT(PRESENTATION, XML_NAME) )
        {
            maStrName = aIter.toString();
        }
        else if( aIter.getToken() == XML_ELEMENT(PRESENTATION, XML_SOURCE) )
        {
            mbFixed = IsXMLToken( aIter, XML_FIXED );
        }
        else if( aIter.getToken() == XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME) )
        {
            maStrDateTimeFormat = aIter.toString();
        }
        else
        {
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

bool SdXMLHeaderFooterDeclContext::IsTransient() const
{
    return true;
}

void SdXMLHeaderFooterDeclContext::endFastElement(sal_Int32 nToken)
{
    SdXMLImport& rImport = dynamic_cast<SdXMLImport&>(GetImport());
    auto nElement = nToken & TOKEN_MASK;
    if( nElement == XML_HEADER_DECL )
    {
        rImport.AddHeaderDecl( maStrName, maStrText );
    }
    else if( nElement == XML_FOOTER_DECL )
    {
        rImport.AddFooterDecl( maStrName, maStrText );
    }
    else if( nElement == XML_DATE_TIME_DECL )
    {
        rImport.AddDateTimeDecl( maStrName, maStrText, mbFixed, maStrDateTimeFormat );
    }
    else
    {
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nToken);
    }
}

void SdXMLHeaderFooterDeclContext::characters( const OUString& rChars )
{
    maStrText += rChars;
}

XMLThemeContext::XMLThemeContext(SvXMLImport& rImport,
                                 const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                 const uno::Reference<beans::XPropertySet>& xMasterPage)
    : SvXMLImportContext(rImport)
    , m_xMasterPage(xMasterPage)
{
    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_NAME):
            {
                m_aTheme["Name"] <<= rAttribute.toString();
                break;
            }
        }
    }
}

XMLThemeContext::~XMLThemeContext()
{
    uno::Any aTheme = uno::makeAny(m_aTheme.getAsConstPropertyValueList());
    m_xMasterPage->setPropertyValue("Theme", aTheme);
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL XMLThemeContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_COLOR_TABLE))
    {
        return new XMLColorTableContext(GetImport(), xAttribs, m_aTheme);
    }

    return nullptr;
}

XMLColorTableContext::XMLColorTableContext(
    SvXMLImport& rImport, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
    comphelper::SequenceAsHashMap& rTheme)
    : SvXMLImportContext(rImport)
    , m_rTheme(rTheme)
{
    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_NAME):
            {
                m_rTheme["ColorSchemeName"] <<= rAttribute.toString();
                break;
            }
        }
    }
}

XMLColorTableContext::~XMLColorTableContext()
{
    m_rTheme["ColorScheme"] <<= comphelper::containerToSequence(m_aColorScheme);
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL XMLColorTableContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_COLOR))
    {
        return new XMLColorContext(GetImport(), xAttribs, m_aColorScheme);
    }

    return nullptr;
}

XMLColorContext::XMLColorContext(SvXMLImport& rImport,
                                 const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                 std::vector<util::Color>& rColorScheme)
    : SvXMLImportContext(rImport)
{
    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_COLOR):
            {
                util::Color nColor;
                sax::Converter::convertColor(nColor, rAttribute.toView());
                rColorScheme.push_back(nColor);
                break;
            }
        }
    }
}

namespace xmloff {

bool IsIgnoreFillStyleNamedItem(
        css::uno::Reference<css::beans::XPropertySet> const& xProps,
        drawing::FillStyle const nExpectedFillStyle)
{
    assert(xProps.is());
    if (nExpectedFillStyle == drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE)
    {
        return false;
    }

    // note: the caller must have called FillPropertySet() previously
    drawing::FillStyle fillStyle{drawing::FillStyle_NONE};
    xProps->getPropertyValue("FillStyle") >>= fillStyle;
    return fillStyle != nExpectedFillStyle;
}

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
