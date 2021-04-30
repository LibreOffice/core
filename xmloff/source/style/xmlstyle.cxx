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

#include <config_wasm_strip.h>

#include <sal/config.h>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XAutoStylesSupplier.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <PageMasterPropMapper.hxx>
#include <sal/log.hxx>
#include <svl/style.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

#include <xmloff/families.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/txtstyli.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <XMLChartStyleContext.hxx>
#include <XMLChartPropertySetMapper.hxx>
#include <xmloff/XMLShapeStyleContext.hxx>
#include "FillStyleContext.hxx"
#include <XMLFootnoteConfigurationImportContext.hxx>
#include <XMLIndexBibliographyConfigurationContext.hxx>
#include <XMLLineNumberingImportContext.hxx>
#include <PageMasterImportContext.hxx>
#include "PageMasterImportPropMapper.hxx"

#include <memory>
#include <set>
#include <string_view>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;

constexpr OUStringLiteral gsParaStyleServiceName( u"com.sun.star.style.ParagraphStyle" );
constexpr OUStringLiteral gsTextStyleServiceName( u"com.sun.star.style.CharacterStyle" );

void SvXMLStyleContext::SetAttribute( sal_Int32 nElement,
                                      const OUString& rValue )
{
    switch (nElement)
    {
        case XML_ELEMENT(STYLE, XML_FAMILY):
        {
            if( IsXMLToken( rValue, XML_PARAGRAPH ) )
                mnFamily = XmlStyleFamily(SfxStyleFamily::Para);
            else if( IsXMLToken( rValue, XML_TEXT ) )
                mnFamily = XmlStyleFamily(SfxStyleFamily::Char);
            break;
        }
        case XML_ELEMENT(STYLE, XML_NAME):
            maName = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_DISPLAY_NAME):
            maDisplayName = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_PARENT_STYLE_NAME):
            maParentName = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_NEXT_STYLE_NAME):
            maFollow = rValue;
            break;
        case XML_ELEMENT(LO_EXT, XML_LINKED_STYLE_NAME):
            maLinked = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_HIDDEN):
            mbHidden = rValue.toBoolean();
            break;
        case XML_ELEMENT(LO_EXT, XML_HIDDEN):
            mbHidden = rValue.toBoolean();
            break;
    }
}


SvXMLStyleContext::SvXMLStyleContext(
        SvXMLImport& rImp,
        XmlStyleFamily nFam, bool bDefault ) :
    SvXMLImportContext( rImp ),
    mbHidden( false ),
    mnFamily( nFam ),
    mbValid( true ),
    mbNew( true ),
    mbDefaultStyle( bDefault )
{
}

SvXMLStyleContext::~SvXMLStyleContext()
{
}

void SvXMLStyleContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for( auto &it : sax_fastparser::castToFastAttributeList( xAttrList ) )
        SetAttribute( it.getToken(), it.toString() );
}

void SvXMLStyleContext::SetDefaults()
{
}

void SvXMLStyleContext::CreateAndInsert( bool /*bOverwrite*/ )
{
}

void SvXMLStyleContext::CreateAndInsertLate( bool /*bOverwrite*/ )
{
}

void SvXMLStyleContext::Finish( bool /*bOverwrite*/ )
{
}

bool SvXMLStyleContext::IsTransient() const
{
    return false;
}

namespace {

class SvXMLStyleIndex_Impl
{
    OUString               sName;
    XmlStyleFamily         nFamily;
    // we deliberately don't use a reference here, to avoid creating a ref-count-cycle
    SvXMLStyleContext*     mpStyle;

public:

    SvXMLStyleIndex_Impl( XmlStyleFamily nFam, const OUString& rName ) :
        sName( rName ),
        nFamily( nFam ),
        mpStyle(nullptr)
    {
    }

    SvXMLStyleIndex_Impl( const rtl::Reference<SvXMLStyleContext> &rStl ) :
        sName( rStl->GetName() ),
        nFamily( rStl->GetFamily() ),
        mpStyle ( rStl.get() )
    {
    }

    const OUString& GetName() const { return sName; }
    XmlStyleFamily GetFamily() const { return nFamily; }
    const SvXMLStyleContext *GetStyle() const { return mpStyle; }
};

struct SvXMLStyleIndexCmp_Impl
{
    bool operator()(const SvXMLStyleIndex_Impl& r1, const SvXMLStyleIndex_Impl& r2) const
    {
        sal_Int32 nRet;

        if( r1.GetFamily() < r2.GetFamily() )
            nRet = -1;
        else if( r1.GetFamily() > r2.GetFamily() )
            nRet = 1;
        else
            nRet = r1.GetName().compareTo( r2.GetName() );

        return nRet < 0;
    }
};

}

class SvXMLStylesContext_Impl
{
    typedef std::set<SvXMLStyleIndex_Impl, SvXMLStyleIndexCmp_Impl> IndicesType;

    std::vector<rtl::Reference<SvXMLStyleContext>> aStyles;
    mutable std::unique_ptr<IndicesType> pIndices;
    bool bAutomaticStyle;

#if OSL_DEBUG_LEVEL > 0
    mutable sal_uInt32 m_nIndexCreated;
#endif

    void FlushIndex() { pIndices.reset(); }

public:
    explicit SvXMLStylesContext_Impl( bool bAuto );

    size_t GetStyleCount() const { return aStyles.size(); }

    SvXMLStyleContext *GetStyle( size_t i )
    {
        return i < aStyles.size() ? aStyles[ i ].get() : nullptr;
    }

    inline void AddStyle( SvXMLStyleContext *pStyle );
    void dispose();

    const SvXMLStyleContext *FindStyleChildContext( XmlStyleFamily nFamily,
                                                    const OUString& rName,
                                                    bool bCreateIndex ) const;
    bool IsAutomaticStyle() const { return bAutomaticStyle; }
};

SvXMLStylesContext_Impl::SvXMLStylesContext_Impl( bool bAuto ) :
    bAutomaticStyle( bAuto )
#if OSL_DEBUG_LEVEL > 0
    , m_nIndexCreated( 0 )
#endif
{}

inline void SvXMLStylesContext_Impl::AddStyle( SvXMLStyleContext *pStyle )
{
#if OSL_DEBUG_LEVEL > 0
//    for (auto const & xStyle : aStyles)
//        if (xStyle->GetFamily() == pStyle->GetFamily() && xStyle->GetName() == pStyle->GetName())
//            assert(false && "duplicate style");
#endif
    aStyles.emplace_back(pStyle );

    FlushIndex();
}

void SvXMLStylesContext_Impl::dispose()
{
    FlushIndex();
    aStyles.clear();
}

const SvXMLStyleContext *SvXMLStylesContext_Impl::FindStyleChildContext( XmlStyleFamily nFamily,
                                                                         const OUString& rName,
                                                                         bool bCreateIndex ) const
{
    const SvXMLStyleContext *pStyle = nullptr;

    if( !pIndices && bCreateIndex && !aStyles.empty() )
    {
        pIndices = std::make_unique<IndicesType>(aStyles.begin(), aStyles.end());
        SAL_WARN_IF(pIndices->size() != aStyles.size(), "xmloff.style", "Here is a duplicate Style");
#if OSL_DEBUG_LEVEL > 0
        SAL_WARN_IF(0 != m_nIndexCreated, "xmloff.style",
                    "Performance warning: sdbcx::Index created multiple times");
        ++m_nIndexCreated;
#endif
    }

    if( pIndices )
    {
        SvXMLStyleIndex_Impl aIndex( nFamily, rName );
        IndicesType::iterator aFind = pIndices->find(aIndex);
        if( aFind != pIndices->end() )
            pStyle = aFind->GetStyle();
    }
    else
    {
        for( size_t i = 0; !pStyle && i < aStyles.size(); i++ )
        {
            const SvXMLStyleContext *pS = aStyles[ i ].get();
            if( pS->GetFamily() == nFamily &&
                pS->GetName() == rName )
                pStyle = pS;
        }
    }
    return pStyle;
}


sal_uInt32 SvXMLStylesContext::GetStyleCount() const
{
    return mpImpl->GetStyleCount();
}

SvXMLStyleContext *SvXMLStylesContext::GetStyle( sal_uInt32 i )
{
    return mpImpl->GetStyle( i );
}

const SvXMLStyleContext *SvXMLStylesContext::GetStyle( sal_uInt32 i ) const
{
    return mpImpl->GetStyle( i );
}

bool SvXMLStylesContext::IsAutomaticStyle() const
{
    return mpImpl->IsAutomaticStyle();
}

SvXMLStyleContext *SvXMLStylesContext::CreateStyleChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    SvXMLStyleContext *pStyle = nullptr;

    if(GetImport().GetDataStylesImport())
    {
        pStyle = GetImport().GetDataStylesImport()->CreateChildContext(GetImport(), nElement,
                                               xAttrList, *this);
        if (pStyle)
            return pStyle;
    }

    switch (nElement)
    {
        case XML_ELEMENT(STYLE, XML_STYLE):
        case XML_ELEMENT(STYLE, XML_DEFAULT_STYLE):
        {
            XmlStyleFamily nFamily = XmlStyleFamily::DATA_STYLE;
            for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
            {
                if( aIter.getToken() == XML_ELEMENT(STYLE, XML_FAMILY) )
                {
                    nFamily = GetFamily( aIter.toString() );
                    break;
                }
            }
            pStyle = XML_ELEMENT(STYLE, XML_STYLE)==nElement
                ? CreateStyleStyleChildContext( nFamily, nElement, xAttrList )
                : CreateDefaultStyleStyleChildContext( nFamily, nElement, xAttrList );
            break;
        }
        case XML_ELEMENT(TEXT, XML_BIBLIOGRAPHY_CONFIGURATION):
            pStyle = new XMLIndexBibliographyConfigurationContext(GetImport());
            break;
        case XML_ELEMENT(TEXT, XML_NOTES_CONFIGURATION):
            pStyle = new XMLFootnoteConfigurationImportContext(
                GetImport(), nElement, xAttrList);
            break;
        case XML_ELEMENT(TEXT, XML_LINENUMBERING_CONFIGURATION):
            pStyle = new XMLLineNumberingImportContext(GetImport());
            break;
        case XML_ELEMENT(STYLE, XML_PAGE_LAYOUT):
        case XML_ELEMENT(STYLE, XML_DEFAULT_PAGE_LAYOUT):
        {
            //there is not page family in ODF now, so I specify one for it
            bool bDefaultStyle  = XML_ELEMENT(STYLE, XML_DEFAULT_PAGE_LAYOUT) == nElement;
            pStyle = new PageStyleContext( GetImport(), *this, bDefaultStyle );
        }
        break;
        case XML_ELEMENT(TEXT, XML_LIST_STYLE):
            pStyle = new SvxXMLListStyleContext( GetImport() );
            break;
        case XML_ELEMENT(TEXT, XML_OUTLINE_STYLE):
            pStyle = new SvxXMLListStyleContext( GetImport(), true );
            break;

        // FillStyles

        case XML_ELEMENT(DRAW, XML_GRADIENT):
        {
            pStyle = new XMLGradientStyleContext( GetImport(), nElement, xAttrList );
            break;
        }
        case XML_ELEMENT(DRAW, XML_HATCH):
        {
            pStyle = new XMLHatchStyleContext( GetImport(), nElement, xAttrList );
            break;
        }
        case XML_ELEMENT(DRAW, XML_FILL_IMAGE):
        {
            pStyle = new XMLBitmapStyleContext( GetImport(), nElement, xAttrList );
            break;
        }
        case XML_ELEMENT(DRAW, XML_OPACITY):
        {
            pStyle = new XMLTransGradientStyleContext( GetImport(), nElement, xAttrList );
            break;
        }
        case XML_ELEMENT(DRAW, XML_MARKER):
        {
            pStyle = new XMLMarkerStyleContext( GetImport(), nElement, xAttrList );
            break;
        }
        case XML_ELEMENT(DRAW, XML_STROKE_DASH):
        {
            pStyle = new XMLDashStyleContext( GetImport(), nElement, xAttrList );
            break;
        }
    }

    if (!pStyle)
        SAL_WARN("xmloff", "Unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));

    return pStyle;
}

SvXMLStyleContext *SvXMLStylesContext::CreateStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 /*nElement*/,
        const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    SvXMLStyleContext *pStyle = nullptr;

    switch( nFamily )
    {
        case XmlStyleFamily::TEXT_PARAGRAPH:
        case XmlStyleFamily::TEXT_TEXT:
        case XmlStyleFamily::TEXT_SECTION:
            pStyle = new XMLTextStyleContext( GetImport(), *this, nFamily );
            break;

        case XmlStyleFamily::TEXT_RUBY:
            pStyle = new XMLPropStyleContext( GetImport(), *this, nFamily );
            break;
#ifndef ENABLE_WASM_STRIP_CHART
        // WASM_CHART change
        case XmlStyleFamily::SCH_CHART_ID:
            pStyle = new XMLChartStyleContext( GetImport(), *this, nFamily );
            break;
#endif
        case XmlStyleFamily::SD_GRAPHICS_ID:
        case XmlStyleFamily::SD_PRESENTATION_ID:
        case XmlStyleFamily::SD_POOL_ID:
            pStyle = new XMLShapeStyleContext( GetImport(), *this, nFamily );
            break;
        default: break;
    }

    return pStyle;
}

SvXMLStyleContext *SvXMLStylesContext::CreateDefaultStyleStyleChildContext(
        XmlStyleFamily /*nFamily*/, sal_Int32 /*nElement*/,
        const uno::Reference< xml::sax::XFastAttributeList > & )
{
    return nullptr;
}

bool SvXMLStylesContext::InsertStyleFamily( XmlStyleFamily ) const
{
    return true;
}

XmlStyleFamily SvXMLStylesContext::GetFamily( std::u16string_view rValue )
{
    XmlStyleFamily nFamily = XmlStyleFamily::DATA_STYLE;
    if( IsXMLToken( rValue, XML_PARAGRAPH ) )
    {
        nFamily = XmlStyleFamily::TEXT_PARAGRAPH;
    }
    else if( IsXMLToken( rValue, XML_TEXT ) )
    {
        nFamily = XmlStyleFamily::TEXT_TEXT;
    }
    else if( IsXMLToken( rValue, XML_DATA_STYLE ) )
    {
        nFamily = XmlStyleFamily::DATA_STYLE;
    }
    else if ( IsXMLToken( rValue, XML_SECTION ) )
    {
        nFamily = XmlStyleFamily::TEXT_SECTION;
    }
    else if( IsXMLToken( rValue, XML_TABLE ) )
    {
        nFamily = XmlStyleFamily::TABLE_TABLE;
    }
    else if( IsXMLToken( rValue, XML_TABLE_COLUMN ) )
        nFamily = XmlStyleFamily::TABLE_COLUMN;
    else if( IsXMLToken( rValue, XML_TABLE_ROW ) )
        nFamily = XmlStyleFamily::TABLE_ROW;
    else if( IsXMLToken( rValue, XML_TABLE_CELL ) )
        nFamily = XmlStyleFamily::TABLE_CELL;
    else if ( rValue == XML_STYLE_FAMILY_SD_GRAPHICS_NAME )
    {
        nFamily = XmlStyleFamily::SD_GRAPHICS_ID;
    }
    else if ( rValue == XML_STYLE_FAMILY_SD_PRESENTATION_NAME )
    {
        nFamily = XmlStyleFamily::SD_PRESENTATION_ID;
    }
    else if ( rValue == XML_STYLE_FAMILY_SD_POOL_NAME )
    {
        nFamily = XmlStyleFamily::SD_POOL_ID;
    }
    else if ( rValue == XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME )
    {
        nFamily = XmlStyleFamily::SD_DRAWINGPAGE_ID;
    }
    else if ( rValue == XML_STYLE_FAMILY_SCH_CHART_NAME )
    {
        nFamily = XmlStyleFamily::SCH_CHART_ID;
    }
    else if ( IsXMLToken( rValue, XML_RUBY ) )
    {
        nFamily = XmlStyleFamily::TEXT_RUBY;
    }

    return nFamily;
}

rtl::Reference < SvXMLImportPropertyMapper > SvXMLStylesContext::GetImportPropertyMapper(
                        XmlStyleFamily nFamily ) const
{
    rtl::Reference < SvXMLImportPropertyMapper > xMapper;

    switch( nFamily )
    {
    case XmlStyleFamily::TEXT_PARAGRAPH:
        if( !mxParaImpPropMapper.is() )
        {
            SvXMLStylesContext * pThis = const_cast<SvXMLStylesContext *>(this);
            pThis->mxParaImpPropMapper =
                pThis->GetImport().GetTextImport()
                     ->GetParaImportPropertySetMapper();
        }
        xMapper = mxParaImpPropMapper;
        break;
    case XmlStyleFamily::TEXT_TEXT:
        if( !mxTextImpPropMapper.is() )
        {
            SvXMLStylesContext * pThis = const_cast<SvXMLStylesContext *>(this);
            pThis->mxTextImpPropMapper =
                pThis->GetImport().GetTextImport()
                     ->GetTextImportPropertySetMapper();
        }
        xMapper = mxTextImpPropMapper;
        break;

    case XmlStyleFamily::TEXT_SECTION:
        // don't cache section mapper, as it's rarely used
        // *sigh*, cast to non-const, because this is a const method,
        // but SvXMLImport::GetTextImport() isn't.
        xMapper = const_cast<SvXMLStylesContext*>(this)->GetImport().GetTextImport()->
            GetSectionImportPropertySetMapper();
        break;

    case XmlStyleFamily::TEXT_RUBY:
        // don't cache section mapper, as it's rarely used
        // *sigh*, cast to non-const, because this is a const method,
        // but SvXMLImport::GetTextImport() isn't.
        xMapper = const_cast<SvXMLStylesContext*>(this)->GetImport().GetTextImport()->
            GetRubyImportPropertySetMapper();
        break;

    case XmlStyleFamily::SD_GRAPHICS_ID:
    case XmlStyleFamily::SD_PRESENTATION_ID:
    case XmlStyleFamily::SD_POOL_ID:
        if(!mxShapeImpPropMapper.is())
        {
            rtl::Reference< XMLShapeImportHelper > aImpHelper = const_cast<SvXMLImport&>(GetImport()).GetShapeImport();
            const_cast<SvXMLStylesContext*>(this)->mxShapeImpPropMapper =
                aImpHelper->GetPropertySetMapper();
        }
        xMapper = mxShapeImpPropMapper;
        break;
#ifndef ENABLE_WASM_STRIP_CHART
    // WASM_CHART change
    case XmlStyleFamily::SCH_CHART_ID:
        if( ! mxChartImpPropMapper.is() )
        {
            XMLPropertySetMapper *const pPropMapper = new XMLChartPropertySetMapper(nullptr);
            mxChartImpPropMapper = new XMLChartImportPropertyMapper( pPropMapper, GetImport() );
        }
        xMapper = mxChartImpPropMapper;
        break;
#endif
    case XmlStyleFamily::PAGE_MASTER:
        if( ! mxPageImpPropMapper.is() )
        {
            XMLPropertySetMapper *pPropMapper =
                new XMLPageMasterPropSetMapper();
            mxPageImpPropMapper =
                new PageMasterImportPropertyMapper( pPropMapper,
                                    const_cast<SvXMLStylesContext*>(this)->GetImport() );
        }
        xMapper = mxPageImpPropMapper;
        break;
    default: break;
    }

    return xMapper;
}

Reference < XAutoStyleFamily > SvXMLStylesContext::GetAutoStyles( XmlStyleFamily nFamily ) const
{
    Reference < XAutoStyleFamily > xAutoStyles;
    if( XmlStyleFamily::TEXT_TEXT == nFamily || XmlStyleFamily::TEXT_PARAGRAPH == nFamily)
    {
        bool bPara = XmlStyleFamily::TEXT_PARAGRAPH == nFamily;
        if( !bPara && mxTextAutoStyles.is() )
            xAutoStyles = mxTextAutoStyles;
        else if( bPara && mxParaAutoStyles.is() )
            xAutoStyles = mxParaAutoStyles;
        else
        {
            OUString sName(bPara ? std::u16string_view( u"ParagraphStyles" ): std::u16string_view( u"CharacterStyles" ));
            Reference< XAutoStylesSupplier > xAutoStylesSupp(   GetImport().GetModel(), UNO_QUERY );
            Reference< XAutoStyles > xAutoStyleFamilies = xAutoStylesSupp->getAutoStyles();
            if (xAutoStyleFamilies->hasByName(sName))
            {
                Any aAny = xAutoStyleFamilies->getByName( sName );
                aAny >>= xAutoStyles;
                if( bPara )
                    const_cast<SvXMLStylesContext *>(this)->mxParaAutoStyles = xAutoStyles;
                else
                    const_cast<SvXMLStylesContext *>(this)->mxTextAutoStyles = xAutoStyles;
            }
        }
    }
    return xAutoStyles;
}

Reference < XNameContainer > SvXMLStylesContext::GetStylesContainer(
                                                XmlStyleFamily nFamily ) const
{
    Reference < XNameContainer > xStyles;
    OUString sName;
    switch( nFamily )
    {
    case XmlStyleFamily::TEXT_PARAGRAPH:
        if( mxParaStyles.is() )
            xStyles = mxParaStyles;
        else
            sName = "ParagraphStyles";
        break;

    case XmlStyleFamily::TEXT_TEXT:
        if( mxTextStyles.is() )
            xStyles = mxTextStyles;
        else
            sName = "CharacterStyles";
        break;
    default: break;
    }
    if( !xStyles.is() && !sName.isEmpty() )
    {
        Reference< XStyleFamiliesSupplier > xFamiliesSupp(
                                        GetImport().GetModel(), UNO_QUERY );
        if ( xFamiliesSupp.is() )
        {
            Reference< XNameAccess > xFamilies = xFamiliesSupp->getStyleFamilies();
            if (xFamilies->hasByName(sName))
            {
                xStyles.set(xFamilies->getByName( sName ),uno::UNO_QUERY);

                switch( nFamily )
                {
                case XmlStyleFamily::TEXT_PARAGRAPH:
                    const_cast<SvXMLStylesContext *>(this)->mxParaStyles = xStyles;
                    break;

                case XmlStyleFamily::TEXT_TEXT:
                    const_cast<SvXMLStylesContext *>(this)->mxTextStyles = xStyles;
                    break;
                default: break;
                }
            }
        }
    }

    return xStyles;
}

OUString SvXMLStylesContext::GetServiceName( XmlStyleFamily nFamily ) const
{
    OUString sServiceName;
    switch( nFamily )
    {
    case XmlStyleFamily::TEXT_PARAGRAPH:
        sServiceName = gsParaStyleServiceName;
        break;
    case XmlStyleFamily::TEXT_TEXT:
        sServiceName = gsTextStyleServiceName;
        break;
    default: break;
    }

    return sServiceName;
}

SvXMLStylesContext::SvXMLStylesContext( SvXMLImport& rImport, bool bAuto ) :
    SvXMLImportContext( rImport ),
    mpImpl( new SvXMLStylesContext_Impl( bAuto ) )
{
}

SvXMLStylesContext::~SvXMLStylesContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SvXMLStylesContext::createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLStyleContext *pStyle =
        CreateStyleChildContext( nElement, xAttrList );
    if( pStyle )
    {
        if( !pStyle->IsTransient() )
            mpImpl->AddStyle( pStyle );
        return pStyle;
    }

    return nullptr;
}

void SvXMLStylesContext::AddStyle(SvXMLStyleContext& rNew)
{
    mpImpl->AddStyle( &rNew );
}

void SvXMLStylesContext::dispose()
{
    mpImpl->dispose();
}

void SvXMLStylesContext::CopyAutoStylesToDoc()
{
    sal_uInt32 nCount = GetStyleCount();
    sal_uInt32 i;
    for( i = 0; i < nCount; i++ )
    {
        SvXMLStyleContext *pStyle = GetStyle( i );
        if( !pStyle || ( pStyle->GetFamily() != XmlStyleFamily::TEXT_TEXT &&
            pStyle->GetFamily() != XmlStyleFamily::TEXT_PARAGRAPH  &&
            pStyle->GetFamily() != XmlStyleFamily::TABLE_CELL ) )
            continue;
        pStyle->CreateAndInsert( false );
    }
}

void SvXMLStylesContext::CopyStylesToDoc( bool bOverwrite,
                                          bool bFinish )
{
    // pass 1: create text, paragraph and frame styles
    sal_uInt32 nCount = GetStyleCount();
    sal_uInt32 i;

    for( i = 0; i < nCount; i++ )
    {
        SvXMLStyleContext *pStyle = GetStyle( i );
        if( !pStyle )
            continue;

        if (pStyle->IsDefaultStyle())
        {
            if (bOverwrite) pStyle->SetDefaults();
        }
        else if( InsertStyleFamily( pStyle->GetFamily() ) )
            pStyle->CreateAndInsert( bOverwrite );
    }

    // pass 2: create list styles (they require char styles)
    for( i=0; i<nCount; i++ )
    {
        SvXMLStyleContext *pStyle = GetStyle( i );
        if( !pStyle || pStyle->IsDefaultStyle())
            continue;

        if( InsertStyleFamily( pStyle->GetFamily() ) )
            pStyle->CreateAndInsertLate( bOverwrite );
    }

    // pass3: finish creation of styles
    if( bFinish )
        FinishStyles( bOverwrite );
}

void SvXMLStylesContext::FinishStyles( bool bOverwrite )
{
    sal_uInt32 nCount = GetStyleCount();
    for( sal_uInt32 i=0; i<nCount; i++ )
    {
        SvXMLStyleContext *pStyle = GetStyle( i );
        if( !pStyle || !pStyle->IsValid() || pStyle->IsDefaultStyle() )
            continue;

        if( InsertStyleFamily( pStyle->GetFamily() ) )
            pStyle->Finish( bOverwrite );
    }
}

const SvXMLStyleContext *SvXMLStylesContext::FindStyleChildContext(
                                  XmlStyleFamily nFamily,
                                  const OUString& rName,
                                  bool bCreateIndex ) const
{
    return mpImpl->FindStyleChildContext( nFamily, rName, bCreateIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
