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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XAutoStylesSupplier.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <PageMasterPropMapper.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>
#include <svl/style.hxx>
#include <utility>
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
#include <XMLThemeContext.hxx>
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
constexpr OUString gsParagraphStyles(u"ParagraphStyles"_ustr);
constexpr OUString gsCharacterStyles(u"CharacterStyles"_ustr);

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
struct StyleIndexCompareByName
{
    bool operator()(const SvXMLStyleContext* r1, const SvXMLStyleContext* r2) const
    {
        if( r1->GetFamily() < r2->GetFamily() )
            return true;
        if( r1->GetFamily() > r2->GetFamily() )
            return false;
        return r1->GetName() < r2->GetName();
    }
};
struct StyleIndexCompareByDisplayName
{
    bool operator()(const SvXMLStyleContext* r1, const SvXMLStyleContext* r2) const
    {
        if( r1->GetFamily() < r2->GetFamily() )
            return true;
        if( r1->GetFamily() > r2->GetFamily() )
            return false;
        return r1->GetDisplayName() < r2->GetDisplayName();
    }
};
}

class SvXMLStylesContext_Impl
{
    std::vector<rtl::Reference<SvXMLStyleContext>> aStyles;
    // it would be better if we could share one vector for the styles and the index, but some code in calc
    // is sensitive to having styles re-ordered
    mutable SvXMLStylesContext::StyleIndex maStylesIndexByName;
    mutable SvXMLStylesContext::StyleIndex maStylesIndexByDisplayName;
    bool bAutomaticStyle;

#if OSL_DEBUG_LEVEL > 0
    mutable sal_uInt32 m_nIndexCreated;
#endif

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

    std::pair<SvXMLStylesContext::StyleIndex::const_iterator, SvXMLStylesContext::StyleIndex::const_iterator>
    FindStyleChildContextByDisplayNamePrefix( XmlStyleFamily nFamily,
                                    const OUString& rPrefix ) const;

    bool IsAutomaticStyle() const { return bAutomaticStyle; }

private:
    void BuildNameIndex() const;
    void BuildDisplayNameIndex() const;
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

    maStylesIndexByName.clear();
    maStylesIndexByDisplayName.clear();
}

void SvXMLStylesContext_Impl::dispose()
{
    aStyles.clear();
}

const SvXMLStyleContext *SvXMLStylesContext_Impl::FindStyleChildContext( XmlStyleFamily nFamily,
                                                                         const OUString& rName,
                                                                         bool bCreateIndex ) const
{
    const SvXMLStyleContext *pStyle = nullptr;

    if( maStylesIndexByName.empty() && bCreateIndex && !aStyles.empty() )
        BuildNameIndex();

    if( !maStylesIndexByName.empty() )
    {
        auto it = std::lower_bound(maStylesIndexByName.begin(), maStylesIndexByName.end(), true,
            [&nFamily, &rName](const SvXMLStyleContext* lhs, bool /*rhs*/)
            {
                if (lhs->GetFamily() < nFamily)
                    return true;
                if (lhs->GetFamily() > nFamily)
                    return false;
                return lhs->GetName() < rName;
            });
        if (it != maStylesIndexByName.end() && (*it)->GetFamily() == nFamily && (*it)->GetName() == rName)
            pStyle = *it;
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

namespace
{
struct PrefixProbeLowerBound
{
    XmlStyleFamily nFamily;
    const OUString& rPrefix;

    bool operator()(const SvXMLStyleContext* lhs, bool /*rhs*/)
    {
        if (lhs->GetFamily() < nFamily)
            return true;
        if (lhs->GetFamily() > nFamily)
            return false;
        return lhs->GetDisplayName() < rPrefix;
    }
};
struct PrefixProbeUpperBound
{
    XmlStyleFamily nFamily;
    const OUString& rPrefix;

    bool operator()(bool /*lhs*/, const SvXMLStyleContext* rhs)
    {
        if (nFamily < rhs->GetFamily())
            return true;
        if (nFamily > rhs->GetFamily())
            return false;
        std::u16string_view rhsName = rhs->GetDisplayName();
        // For the upper bound we want to view the vector's data as if
        // every element was truncated to the size of the prefix.
        // Then perform a normal match.
        rhsName = rhsName.substr(0, rPrefix.getLength());
        // compare UP TO the length of the prefix and no farther
        if (int cmp = rPrefix.compareTo(rhsName))
            return cmp < 0;
        // The strings are equal to the length of the prefix so
        // behave as if they are equal. That means s1 < s2 == false
        return false;
    }
};
}

std::pair<SvXMLStylesContext::StyleIndex::const_iterator, SvXMLStylesContext::StyleIndex::const_iterator>
SvXMLStylesContext_Impl::FindStyleChildContextByDisplayNamePrefix( XmlStyleFamily nFamily,
                                                        const OUString& rPrefix ) const
{
    if( maStylesIndexByDisplayName.empty() )
        BuildDisplayNameIndex();
    auto itStart = std::lower_bound(maStylesIndexByDisplayName.begin(), maStylesIndexByDisplayName.end(), true, PrefixProbeLowerBound{nFamily,rPrefix});
    auto itEnd = std::upper_bound(itStart, maStylesIndexByDisplayName.end(), true, PrefixProbeUpperBound{nFamily,rPrefix});
    return {itStart, itEnd};
}

void SvXMLStylesContext_Impl::BuildNameIndex() const
{
    maStylesIndexByName.reserve(aStyles.size());

    for (const auto & i : aStyles)
        maStylesIndexByName.push_back(i.get());
    std::sort(maStylesIndexByName.begin(), maStylesIndexByName.end(), StyleIndexCompareByName());

#if OSL_DEBUG_LEVEL > 0
    SAL_WARN_IF(0 != m_nIndexCreated, "xmloff.style",
                "Performance warning: sdbcx::Index created multiple times");
    ++m_nIndexCreated;
#endif
}

void SvXMLStylesContext_Impl::BuildDisplayNameIndex() const
{
    maStylesIndexByDisplayName.reserve(aStyles.size());
    for (const auto & i : aStyles)
        maStylesIndexByDisplayName.push_back(i.get());
    std::sort(maStylesIndexByDisplayName.begin(), maStylesIndexByDisplayName.end(), StyleIndexCompareByDisplayName());
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
#if !ENABLE_WASM_STRIP_CHART
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

SvXMLImportPropertyMapper* SvXMLStylesContext::GetImportPropertyMapper(
                        XmlStyleFamily nFamily ) const
{
    SvXMLImportPropertyMapper* pMapper = nullptr;

    SvXMLStylesContext * pThis = const_cast<SvXMLStylesContext *>(this);
    switch( nFamily )
    {
    case XmlStyleFamily::TEXT_PARAGRAPH:
        pMapper =
            pThis->GetImport().GetTextImport()
                 ->GetParaImportPropertySetMapper();
        break;
    case XmlStyleFamily::TEXT_TEXT:
        pMapper =
            pThis->GetImport().GetTextImport()
                 ->GetTextImportPropertySetMapper();
        break;

    case XmlStyleFamily::TEXT_SECTION:
        // *sigh*, cast to non-const, because this is a const method,
        // but SvXMLImport::GetTextImport() isn't.
        pMapper = pThis->GetImport().GetTextImport()->
                GetSectionImportPropertySetMapper();
        break;

    case XmlStyleFamily::TEXT_RUBY:
        // *sigh*, cast to non-const, because this is a const method,
        // but SvXMLImport::GetTextImport() isn't.
        pMapper = pThis->GetImport().GetTextImport()->
            GetRubyImportPropertySetMapper();
        break;

    case XmlStyleFamily::SD_GRAPHICS_ID:
    case XmlStyleFamily::SD_PRESENTATION_ID:
    case XmlStyleFamily::SD_POOL_ID:
        pMapper = const_cast<SvXMLImport&>(GetImport()).GetShapeImport()->GetPropertySetMapper();
        break;
#if !ENABLE_WASM_STRIP_CHART
    // WASM_CHART change
    case XmlStyleFamily::SCH_CHART_ID:
        if( ! mxChartImpPropMapper )
        {
            XMLPropertySetMapper *const pPropMapper = new XMLChartPropertySetMapper(nullptr);
            mxChartImpPropMapper = std::make_unique<XMLChartImportPropertyMapper>( pPropMapper, GetImport() );
        }
        pMapper = mxChartImpPropMapper.get();
        break;
#endif
    case XmlStyleFamily::PAGE_MASTER:
        if( ! mxPageImpPropMapper )
        {
            XMLPropertySetMapper *pPropMapper =
                new XMLPageMasterPropSetMapper();
            mxPageImpPropMapper =
                std::make_unique<PageMasterImportPropertyMapper>( pPropMapper,
                                    const_cast<SvXMLStylesContext*>(this)->GetImport() );
        }
        pMapper = mxPageImpPropMapper.get();
        break;
    default: break;
    }

    return pMapper;
}

Reference < XAutoStyleFamily > SvXMLStylesContext::GetAutoStyles( XmlStyleFamily nFamily ) const
{
    Reference < XAutoStyleFamily > xAutoStyles;
    if( XmlStyleFamily::TEXT_TEXT == nFamily || XmlStyleFamily::TEXT_PARAGRAPH == nFamily)
    {
        bool bPara = XmlStyleFamily::TEXT_PARAGRAPH == nFamily;
        const Reference<XAutoStyleFamily>& rxAutoStyles = bPara ? mxParaAutoStyles : mxTextAutoStyles;
        if (!rxAutoStyles)
        {
            OUString sName(bPara ? gsParagraphStyles : gsCharacterStyles);
            Reference< XAutoStylesSupplier > xAutoStylesSupp(   GetImport().GetModel(), UNO_QUERY );
            Reference< XAutoStyles > xAutoStyleFamilies = xAutoStylesSupp->getAutoStyles();
            if (xAutoStyleFamilies->hasByName(sName))
            {
                Any aAny = xAutoStyleFamilies->getByName( sName );
                aAny >>= const_cast<Reference<XAutoStyleFamily>&>(rxAutoStyles);
            }
        }
        xAutoStyles = rxAutoStyles;
    }
    return xAutoStyles;
}

Reference < XNameContainer > SvXMLStylesContext::GetStylesContainer(
                                                XmlStyleFamily nFamily ) const
{
    Reference < XNameContainer > xStyles;
    if (XmlStyleFamily::TEXT_TEXT == nFamily || XmlStyleFamily::TEXT_PARAGRAPH == nFamily)
    {
        bool bPara = XmlStyleFamily::TEXT_PARAGRAPH == nFamily;
        const Reference<XNameContainer>& rxStyles = bPara ? mxParaStyles : mxTextStyles;
        if (!rxStyles)
        {
            OUString sName(bPara ? gsParagraphStyles : gsCharacterStyles);
            Reference<XStyleFamiliesSupplier> xFamiliesSupp(GetImport().GetModel(), UNO_QUERY);
            if (xFamiliesSupp.is())
            {
                Reference<XNameAccess> xFamilies = xFamiliesSupp->getStyleFamilies();
                if (xFamilies->hasByName(sName))
                {
                    Any aAny = xFamilies->getByName(sName);
                    aAny >>= const_cast<Reference<XNameContainer>&>(rxStyles);
                }
            }
        }
        xStyles = rxStyles;
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
    if (nElement ==  XML_ELEMENT(LO_EXT, XML_THEME))
    {
        if (auto xImportInfo = GetImport().getImportInfo())
        {
            try
            {
                if (auto xPropertySetInfo = xImportInfo->getPropertySetInfo())
                {
                    if (xPropertySetInfo->hasPropertyByName(u"IsInPaste"_ustr))
                    {
                        css::uno::Any value = xImportInfo->getPropertyValue(u"IsInPaste"_ustr);
                        if (bool b; (value >>= b) && b)
                            return nullptr; // do not import themes in paste mode
                    }
                }
            }
            catch (const css::uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("xmloff");
            }
        }

        uno::Reference<uno::XInterface> xObject(GetImport().GetModel(), uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPageSupplier> const xDrawPageSupplier(GetImport().GetModel(), uno::UNO_QUERY);
        if (xDrawPageSupplier.is())
        {
            uno::Reference<drawing::XDrawPage> xPage = xDrawPageSupplier->getDrawPage();
            if (xPage.is())
                xObject = xPage;
        }

        return new XMLThemeContext(GetImport(), xAttrList, xObject);
    }

    SvXMLStyleContext* pStyle = CreateStyleChildContext( nElement, xAttrList );
    if (pStyle)
    {
        if (!pStyle->IsTransient())
            mpImpl->AddStyle(pStyle);
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

std::pair<SvXMLStylesContext::StyleIndex::const_iterator, SvXMLStylesContext::StyleIndex::const_iterator>
SvXMLStylesContext::FindStyleChildContextByDisplayNamePrefix(
                                  XmlStyleFamily nFamily,
                                  const OUString& rNamePrefix ) const
{
    return mpImpl->FindStyleChildContextByDisplayNamePrefix( nFamily, rNamePrefix );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
