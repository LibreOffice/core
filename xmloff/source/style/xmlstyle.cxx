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

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XAutoStylesSupplier.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include "PageMasterPropMapper.hxx"
#include <sal/log.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

#include <xmloff/families.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/txtstyli.hxx>
#include <xmloff/txtprmap.hxx>
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
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;

static const SvXMLTokenMapEntry aStyleStylesElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  XML_STYLE,          XML_TOK_STYLE_STYLE                },
    { XML_NAMESPACE_STYLE,  XML_PAGE_LAYOUT,    XML_TOK_STYLE_PAGE_MASTER          },
    { XML_NAMESPACE_TEXT,   XML_LIST_STYLE,     XML_TOK_TEXT_LIST_STYLE            },
    { XML_NAMESPACE_TEXT,   XML_OUTLINE_STYLE,  XML_TOK_TEXT_OUTLINE               },
    { XML_NAMESPACE_STYLE,  XML_DEFAULT_STYLE,  XML_TOK_STYLE_DEFAULT_STYLE        },
    { XML_NAMESPACE_DRAW,   XML_GRADIENT,       XML_TOK_STYLES_GRADIENTSTYLES      },
    { XML_NAMESPACE_DRAW,   XML_HATCH,          XML_TOK_STYLES_HATCHSTYLES         },
    { XML_NAMESPACE_DRAW,   XML_FILL_IMAGE,     XML_TOK_STYLES_BITMAPSTYLES        },
    { XML_NAMESPACE_DRAW,   XML_OPACITY,        XML_TOK_STYLES_TRANSGRADIENTSTYLES },
    { XML_NAMESPACE_DRAW,   XML_MARKER,         XML_TOK_STYLES_MARKERSTYLES        },
    { XML_NAMESPACE_DRAW,   XML_STROKE_DASH,    XML_TOK_STYLES_DASHSTYLES        },
    { XML_NAMESPACE_TEXT,   XML_NOTES_CONFIGURATION,    XML_TOK_TEXT_NOTE_CONFIG },
    { XML_NAMESPACE_TEXT,   XML_BIBLIOGRAPHY_CONFIGURATION, XML_TOK_TEXT_BIBLIOGRAPHY_CONFIG },
    { XML_NAMESPACE_TEXT,   XML_LINENUMBERING_CONFIGURATION,XML_TOK_TEXT_LINENUMBERING_CONFIG },
    { XML_NAMESPACE_STYLE,  XML_DEFAULT_PAGE_LAYOUT,    XML_TOK_STYLE_DEFAULT_PAGE_LAYOUT        },
    XML_TOKEN_MAP_END
};

static const OUStringLiteral gsParaStyleServiceName( "com.sun.star.style.ParagraphStyle" );
static const OUStringLiteral gsTextStyleServiceName( "com.sun.star.style.CharacterStyle" );

const SvXMLTokenMap& SvXMLStylesContext::GetStyleStylesElemTokenMap()
{
    if( !mpStyleStylesElemTokenMap )
        mpStyleStylesElemTokenMap.reset(
            new SvXMLTokenMap( aStyleStylesElemTokenMap ) );

    return *mpStyleStylesElemTokenMap;
}

void SvXMLStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                      const OUString& rLocalName,
                                      const OUString& rValue )
{
    // TODO: use a map here
    if( XML_NAMESPACE_STYLE == nPrefixKey )
    {
        if( IsXMLToken( rLocalName, XML_FAMILY ) )
        {
            if( IsXMLToken( rValue, XML_PARAGRAPH ) )
                mnFamily = sal_uInt16(SfxStyleFamily::Para);
            else if( IsXMLToken( rValue, XML_TEXT ) )
                mnFamily = sal_uInt16(SfxStyleFamily::Char);
        }
        else if( IsXMLToken( rLocalName, XML_NAME ) )
        {
            maName = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_DISPLAY_NAME ) )
        {
            maDisplayName = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_PARENT_STYLE_NAME ) )
        {
            maParentName = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_NEXT_STYLE_NAME ) )
        {
            maFollow = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_HIDDEN ) )
        {
            mbHidden = rValue.toBoolean();
        }
    }
}


SvXMLStyleContext::SvXMLStyleContext(
        SvXMLImport& rImp, sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >&,
        sal_uInt16 nFam, bool bDefault ) :
    SvXMLImportContext( rImp, nPrfx, rLName ),
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

SvXMLImportContextRef SvXMLStyleContext::CreateChildContext( sal_uInt16 nPrefix,
                                                           const OUString& rLocalName,
                                                           const uno::Reference< xml::sax::XAttributeList > & )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void SvXMLStyleContext::StartElement( const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        SetAttribute( nPrefix, aLocalName, rValue );
    }
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

class SvXMLStyleIndex_Impl
{
    OUString const              sName;
    sal_uInt16 const            nFamily;
    const rtl::Reference<SvXMLStyleContext> mxStyle;

public:

    SvXMLStyleIndex_Impl( sal_uInt16 nFam, const OUString& rName ) :
        sName( rName ),
        nFamily( nFam )
    {
    }

    SvXMLStyleIndex_Impl( const rtl::Reference<SvXMLStyleContext> &rStl ) :
        sName( rStl->GetName() ),
        nFamily( rStl->GetFamily() ),
        mxStyle ( rStl )
    {
    }

    const OUString& GetName() const { return sName; }
    sal_uInt16 GetFamily() const { return nFamily; }
    const SvXMLStyleContext *GetStyle() const { return mxStyle.get(); }
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

class SvXMLStylesContext_Impl
{
    typedef std::set<SvXMLStyleIndex_Impl, SvXMLStyleIndexCmp_Impl> IndicesType;

    std::vector<rtl::Reference<SvXMLStyleContext>> aStyles;
    mutable std::unique_ptr<IndicesType> pIndices;
    bool const bAutomaticStyle;

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
    void Clear();

    const SvXMLStyleContext *FindStyleChildContext( sal_uInt16 nFamily,
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
    aStyles.emplace_back(pStyle );

    FlushIndex();
}

void SvXMLStylesContext_Impl::Clear()
{
    FlushIndex();
    aStyles.clear();
}

const SvXMLStyleContext *SvXMLStylesContext_Impl::FindStyleChildContext( sal_uInt16 nFamily,
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

SvXMLStyleContext *SvXMLStylesContext::CreateStyleChildContext( sal_uInt16 p_nPrefix,
                                                                const OUString& rLocalName,
                                                                const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = nullptr;

    if(GetImport().GetDataStylesImport())
    {
        pStyle = GetImport().GetDataStylesImport()->CreateChildContext(GetImport(), p_nPrefix,
                                               rLocalName, xAttrList, *this);
    }

    if (!pStyle)
    {
        const SvXMLTokenMap& rTokenMap = GetStyleStylesElemTokenMap();
        sal_uInt16 nToken = rTokenMap.Get( p_nPrefix, rLocalName );
        switch( nToken )
        {
            case XML_TOK_STYLE_STYLE:
            case XML_TOK_STYLE_DEFAULT_STYLE:
            {
                sal_uInt16 nFamily = 0;
                sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
                for( sal_Int16 i=0; i < nAttrCount; i++ )
                {
                    const OUString& rAttrName = xAttrList->getNameByIndex( i );
                    OUString aLocalName;
                    sal_uInt16 nPrefix =
                        GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                    &aLocalName );
                    if( XML_NAMESPACE_STYLE == nPrefix &&
                        IsXMLToken( aLocalName, XML_FAMILY ) )
                    {
                        const OUString& rValue = xAttrList->getValueByIndex( i );
                        nFamily = GetFamily( rValue );
                        break;
                    }
                }
                pStyle = XML_TOK_STYLE_STYLE==nToken
                    ? CreateStyleStyleChildContext( nFamily, p_nPrefix,
                                                    rLocalName, xAttrList )
                    : CreateDefaultStyleStyleChildContext( nFamily, p_nPrefix,
                                                    rLocalName, xAttrList );
            }
            break;
            case XML_TOK_STYLE_PAGE_MASTER:
            case XML_TOK_STYLE_DEFAULT_PAGE_LAYOUT:
            {
                //there is not page family in ODF now, so I specify one for it
                bool bDefaultStyle  = XML_TOK_STYLE_DEFAULT_PAGE_LAYOUT == nToken;
                pStyle = new PageStyleContext( GetImport(), p_nPrefix,
                                                    rLocalName, xAttrList, *this, bDefaultStyle );
            }
            break;
            case XML_TOK_TEXT_LIST_STYLE:
                pStyle = new SvxXMLListStyleContext( GetImport(), p_nPrefix,
                                                    rLocalName, xAttrList );
                break;
            case XML_TOK_TEXT_OUTLINE:
                pStyle = new SvxXMLListStyleContext( GetImport(), p_nPrefix,
                                                    rLocalName, xAttrList, true );
                break;
            case XML_TOK_TEXT_NOTE_CONFIG:
                pStyle = new XMLFootnoteConfigurationImportContext(GetImport(),
                                                                   p_nPrefix,
                                                                   rLocalName,
                                                                   xAttrList);
                break;

            case XML_TOK_TEXT_BIBLIOGRAPHY_CONFIG:
                pStyle = new XMLIndexBibliographyConfigurationContext(
                    GetImport(), p_nPrefix, rLocalName, xAttrList);
                break;

            case XML_TOK_TEXT_LINENUMBERING_CONFIG:
                pStyle = new XMLLineNumberingImportContext(
                    GetImport(), p_nPrefix, rLocalName, xAttrList);
                break;


            // FillStyles

            case XML_TOK_STYLES_GRADIENTSTYLES:
            {
                pStyle = new XMLGradientStyleContext( GetImport(), p_nPrefix, rLocalName, xAttrList );
                break;
            }
            case XML_TOK_STYLES_HATCHSTYLES:
            {
                pStyle = new XMLHatchStyleContext( GetImport(), p_nPrefix, rLocalName, xAttrList );
                break;
            }
            case XML_TOK_STYLES_BITMAPSTYLES:
            {
                pStyle = new XMLBitmapStyleContext( GetImport(), p_nPrefix, rLocalName, xAttrList );
                break;
            }
            case XML_TOK_STYLES_TRANSGRADIENTSTYLES:
            {
                pStyle = new XMLTransGradientStyleContext( GetImport(), p_nPrefix, rLocalName, xAttrList );
                break;
            }
            case XML_TOK_STYLES_MARKERSTYLES:
            {
                pStyle = new XMLMarkerStyleContext( GetImport(), p_nPrefix, rLocalName, xAttrList );
                break;
            }
            case XML_TOK_STYLES_DASHSTYLES:
            {
                pStyle = new XMLDashStyleContext( GetImport(), p_nPrefix, rLocalName, xAttrList );
                break;
            }
        }
    }

    return pStyle;
}

SvXMLStyleContext *SvXMLStylesContext::CreateStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = nullptr;

    switch( nFamily )
    {
        case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        case XML_STYLE_FAMILY_TEXT_TEXT:
        case XML_STYLE_FAMILY_TEXT_SECTION:
            pStyle = new XMLTextStyleContext( GetImport(), nPrefix, rLocalName,
                                              xAttrList, *this, nFamily );
            break;

        case XML_STYLE_FAMILY_TEXT_RUBY:
            pStyle = new XMLPropStyleContext( GetImport(), nPrefix, rLocalName,
                                              xAttrList, *this, nFamily );
            break;
        case XML_STYLE_FAMILY_SCH_CHART_ID:
            pStyle = new XMLChartStyleContext( GetImport(), nPrefix, rLocalName,
                                               xAttrList, *this, nFamily );
            break;

        case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        case XML_STYLE_FAMILY_SD_PRESENTATION_ID:
        case XML_STYLE_FAMILY_SD_POOL_ID:
            pStyle = new XMLShapeStyleContext( GetImport(), nPrefix, rLocalName,
                                               xAttrList, *this, nFamily );
    }

    return pStyle;
}

SvXMLStyleContext *SvXMLStylesContext::CreateDefaultStyleStyleChildContext(
        sal_uInt16 /*nFamily*/, sal_uInt16 /*nPrefix*/, const OUString& /*rLocalName*/,
        const uno::Reference< xml::sax::XAttributeList > & )
{
    return nullptr;
}

bool SvXMLStylesContext::InsertStyleFamily( sal_uInt16 ) const
{
    return true;
}

sal_uInt16 SvXMLStylesContext::GetFamily( const OUString& rValue )
{
    sal_uInt16 nFamily = 0U;
    if( IsXMLToken( rValue, XML_PARAGRAPH ) )
    {
        nFamily = XML_STYLE_FAMILY_TEXT_PARAGRAPH;
    }
    else if( IsXMLToken( rValue, XML_TEXT ) )
    {
        nFamily = XML_STYLE_FAMILY_TEXT_TEXT;
    }
    else if( IsXMLToken( rValue, XML_DATA_STYLE ) )
    {
        nFamily = XML_STYLE_FAMILY_DATA_STYLE;
    }
    else if ( IsXMLToken( rValue, XML_SECTION ) )
    {
        nFamily = XML_STYLE_FAMILY_TEXT_SECTION;
    }
    else if( IsXMLToken( rValue, XML_TABLE ) )
    {
        nFamily = XML_STYLE_FAMILY_TABLE_TABLE;
    }
    else if( IsXMLToken( rValue, XML_TABLE_COLUMN ) )
        nFamily = XML_STYLE_FAMILY_TABLE_COLUMN;
    else if( IsXMLToken( rValue, XML_TABLE_ROW ) )
        nFamily = XML_STYLE_FAMILY_TABLE_ROW;
    else if( IsXMLToken( rValue, XML_TABLE_CELL ) )
        nFamily = XML_STYLE_FAMILY_TABLE_CELL;
    else if ( rValue == XML_STYLE_FAMILY_SD_GRAPHICS_NAME )
    {
        nFamily = XML_STYLE_FAMILY_SD_GRAPHICS_ID;
    }
    else if ( rValue == XML_STYLE_FAMILY_SD_PRESENTATION_NAME )
    {
        nFamily = XML_STYLE_FAMILY_SD_PRESENTATION_ID;
    }
    else if ( rValue == XML_STYLE_FAMILY_SD_POOL_NAME )
    {
        nFamily = XML_STYLE_FAMILY_SD_POOL_ID;
    }
    else if ( rValue == XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME )
    {
        nFamily = XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID;
    }
    else if ( rValue == XML_STYLE_FAMILY_SCH_CHART_NAME )
    {
        nFamily = XML_STYLE_FAMILY_SCH_CHART_ID;
    }
    else if ( IsXMLToken( rValue, XML_RUBY ) )
    {
        nFamily = XML_STYLE_FAMILY_TEXT_RUBY;
    }

    return nFamily;
}

rtl::Reference < SvXMLImportPropertyMapper > SvXMLStylesContext::GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const
{
    rtl::Reference < SvXMLImportPropertyMapper > xMapper;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        if( !mxParaImpPropMapper.is() )
        {
            SvXMLStylesContext * pThis = const_cast<SvXMLStylesContext *>(this);
            pThis->mxParaImpPropMapper =
                pThis->GetImport().GetTextImport()
                     ->GetParaImportPropertySetMapper();
        }
        xMapper = mxParaImpPropMapper;
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        if( !mxTextImpPropMapper.is() )
        {
            SvXMLStylesContext * pThis = const_cast<SvXMLStylesContext *>(this);
            pThis->mxTextImpPropMapper =
                pThis->GetImport().GetTextImport()
                     ->GetTextImportPropertySetMapper();
        }
        xMapper = mxTextImpPropMapper;
        break;

    case XML_STYLE_FAMILY_TEXT_SECTION:
        // don't cache section mapper, as it's rarely used
        // *sigh*, cast to non-const, because this is a const method,
        // but SvXMLImport::GetTextImport() isn't.
        xMapper = const_cast<SvXMLStylesContext*>(this)->GetImport().GetTextImport()->
            GetSectionImportPropertySetMapper();
        break;

    case XML_STYLE_FAMILY_TEXT_RUBY:
        // don't cache section mapper, as it's rarely used
        // *sigh*, cast to non-const, because this is a const method,
        // but SvXMLImport::GetTextImport() isn't.
        xMapper = const_cast<SvXMLStylesContext*>(this)->GetImport().GetTextImport()->
            GetRubyImportPropertySetMapper();
        break;

    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
    case XML_STYLE_FAMILY_SD_PRESENTATION_ID:
    case XML_STYLE_FAMILY_SD_POOL_ID:
        if(!mxShapeImpPropMapper.is())
        {
            rtl::Reference< XMLShapeImportHelper > aImpHelper = const_cast<SvXMLImport&>(GetImport()).GetShapeImport();
            const_cast<SvXMLStylesContext*>(this)->mxShapeImpPropMapper =
                aImpHelper->GetPropertySetMapper();
        }
        xMapper = mxShapeImpPropMapper;
        break;
    case XML_STYLE_FAMILY_SCH_CHART_ID:
        if( ! mxChartImpPropMapper.is() )
        {
            XMLPropertySetMapper *pPropMapper = new XMLChartPropertySetMapper( false );
            mxChartImpPropMapper = new XMLChartImportPropertyMapper( pPropMapper, GetImport() );
        }
        xMapper = mxChartImpPropMapper;
        break;
    case XML_STYLE_FAMILY_PAGE_MASTER:
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
    }

    return xMapper;
}

Reference < XAutoStyleFamily > SvXMLStylesContext::GetAutoStyles( sal_uInt16 nFamily ) const
{
    Reference < XAutoStyleFamily > xAutoStyles;
    if( XML_STYLE_FAMILY_TEXT_TEXT == nFamily || XML_STYLE_FAMILY_TEXT_PARAGRAPH == nFamily)
    {
        bool bPara = XML_STYLE_FAMILY_TEXT_PARAGRAPH == nFamily;
        OUString sName;
        if( !bPara && mxTextAutoStyles.is() )
            xAutoStyles = mxTextAutoStyles;
        else if( bPara && mxParaAutoStyles.is() )
            xAutoStyles = mxParaAutoStyles;
        else
        {
            sName = bPara ? OUString( "ParagraphStyles" ): OUString( "CharacterStyles" );
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
                                                sal_uInt16 nFamily ) const
{
    Reference < XNameContainer > xStyles;
    OUString sName;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        if( mxParaStyles.is() )
            xStyles = mxParaStyles;
        else
            sName = "ParagraphStyles";
        break;

    case XML_STYLE_FAMILY_TEXT_TEXT:
        if( mxTextStyles.is() )
            xStyles = mxTextStyles;
        else
            sName = "CharacterStyles";
        break;
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
                case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
                    const_cast<SvXMLStylesContext *>(this)->mxParaStyles = xStyles;
                    break;

                case XML_STYLE_FAMILY_TEXT_TEXT:
                    const_cast<SvXMLStylesContext *>(this)->mxTextStyles = xStyles;
                    break;
                }
            }
        }
    }

    return xStyles;
}

OUString SvXMLStylesContext::GetServiceName( sal_uInt16 nFamily ) const
{
    OUString sServiceName;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        sServiceName = gsParaStyleServiceName;
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        sServiceName = gsTextStyleServiceName;
        break;
    }

    return sServiceName;
}

SvXMLStylesContext::SvXMLStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                        const OUString& rLName,
                                        const uno::Reference< xml::sax::XAttributeList > &, bool bAuto ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpImpl( new SvXMLStylesContext_Impl( bAuto ) )
{
}

SvXMLStylesContext::~SvXMLStylesContext()
{
}

SvXMLImportContextRef SvXMLStylesContext::CreateChildContext( sal_uInt16 nPrefix,
                                                            const OUString& rLocalName,
                                                            const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    SvXMLStyleContext *pStyle =
        CreateStyleChildContext( nPrefix, rLocalName, xAttrList );
    if( pStyle )
    {
        if( !pStyle->IsTransient() )
            mpImpl->AddStyle( pStyle );
        pContext = pStyle;
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix,
                                           rLocalName );
    }

    return pContext;
}

void SvXMLStylesContext::EndElement()
{
}

void SvXMLStylesContext::AddStyle(SvXMLStyleContext& rNew)
{
    mpImpl->AddStyle( &rNew );
}

void SvXMLStylesContext::Clear()
{
    mpImpl->Clear();
}

void SvXMLStylesContext::CopyAutoStylesToDoc()
{
    sal_uInt32 nCount = GetStyleCount();
    sal_uInt32 i;
    for( i = 0; i < nCount; i++ )
    {
        SvXMLStyleContext *pStyle = GetStyle( i );
        if( !pStyle || ( pStyle->GetFamily() != XML_STYLE_FAMILY_TEXT_TEXT &&
            pStyle->GetFamily() != XML_STYLE_FAMILY_TEXT_PARAGRAPH  &&
            pStyle->GetFamily() != XML_STYLE_FAMILY_TABLE_CELL ) )
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
            pStyle->SetDefaults();
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
                                  sal_uInt16 nFamily,
                                  const OUString& rName,
                                  bool bCreateIndex ) const
{
    return mpImpl->FindStyleChildContext( nFamily, rName, bCreateIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
