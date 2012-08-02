/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <tools/debug.hxx>
#include <svl/itemset.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>

#include <xmloff/families.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/txtstyli.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlnumfi.hxx>
#include "XMLChartStyleContext.hxx"
#include "XMLChartPropertySetMapper.hxx"
#include <xmloff/XMLShapeStyleContext.hxx>
#include "FillStyleContext.hxx"
#include "XMLFootnoteConfigurationImportContext.hxx"
#include "XMLIndexBibliographyConfigurationContext.hxx"
#include "XMLLineNumberingImportContext.hxx"
#include "PageMasterImportContext.hxx"
#include "PageMasterImportPropMapper.hxx"

#include <set>
#include <vector>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::vector;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;

// ---------------------------------------------------------------------

static SvXMLTokenMapEntry aStyleStylesElemTokenMap[] =
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

const SvXMLTokenMap& SvXMLStylesContext::GetStyleStylesElemTokenMap()
{
    if( !mpStyleStylesElemTokenMap )
        mpStyleStylesElemTokenMap =
            new SvXMLTokenMap( aStyleStylesElemTokenMap );

    return *mpStyleStylesElemTokenMap;
}

// ---------------------------------------------------------------------

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
                mnFamily = (sal_uInt16)SFX_STYLE_FAMILY_PARA;
            else if( IsXMLToken( rValue, XML_TEXT ) )
                mnFamily = (sal_uInt16)SFX_STYLE_FAMILY_CHAR;
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
        else if( IsXMLToken( rLocalName, XML_HELP_FILE_NAME ) )
        {
            maHelpFile = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_HELP_ID ) )
        {
            sal_Int32 nTmp = rValue.toInt32();
            mnHelpId =
                (nTmp < 0L) ? 0U : ( (nTmp > USHRT_MAX) ? USHRT_MAX
                                                        : (sal_uInt16)nTmp );
        }
    }
}

TYPEINIT1( SvXMLStyleContext, SvXMLImportContext );

SvXMLStyleContext::SvXMLStyleContext(
        SvXMLImport& rImp, sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >&,
        sal_uInt16 nFam, sal_Bool bDefault ) :
    SvXMLImportContext( rImp, nPrfx, rLName ),
    mnHelpId( UCHAR_MAX ),
    mnFamily( nFam ),
    mbValid( sal_True ),
    mbNew( sal_True ),
    mbDefaultStyle( bDefault )
{
}

SvXMLStyleContext::~SvXMLStyleContext()
{
}

SvXMLImportContext *SvXMLStyleContext::CreateChildContext( sal_uInt16 nPrefix,
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

void SvXMLStyleContext::CreateAndInsert( sal_Bool /*bOverwrite*/ )
{
}

void SvXMLStyleContext::CreateAndInsertLate( sal_Bool /*bOverwrite*/ )
{
}

void SvXMLStyleContext::Finish( sal_Bool /*bOverwrite*/ )
{
}

sal_Bool SvXMLStyleContext::IsTransient() const
{
    return sal_False;
}

// ---------------------------------------------------------------------

class SvXMLStyleIndex_Impl
{
    OUString              sName;
    sal_uInt16            nFamily;
    const SvXMLStyleContext *pStyle;

public:

    SvXMLStyleIndex_Impl( sal_uInt16 nFam, const OUString& rName ) :
        sName( rName ),
        nFamily( nFam ),
        pStyle ( 0 )
    {
    }

    SvXMLStyleIndex_Impl( const SvXMLStyleContext *pStl ) :
        sName( pStl->GetName() ),
        nFamily( pStl->GetFamily() ),
        pStyle ( pStl )
    {
    }

    const OUString& GetName() const { return sName; }
    sal_uInt16 GetFamily() const { return nFamily; }
    const SvXMLStyleContext *GetStyle() const { return pStyle; }
};

struct SvXMLStyleIndexCmp_Impl
{
    bool operator()(const SvXMLStyleIndex_Impl& r1, const SvXMLStyleIndex_Impl& r2) const
    {
        sal_Int32 nRet;

        if( (sal_uInt16)r1.GetFamily() < (sal_uInt16)r2.GetFamily() )
            nRet = -1;
        else if( (sal_uInt16)r1.GetFamily() > (sal_uInt16)r2.GetFamily() )
            nRet = 1;
        else
            nRet = r1.GetName().compareTo( r2.GetName() );

        return nRet < 0;
    }
};

// ---------------------------------------------------------------------

typedef SvXMLStyleContext *SvXMLStyleContextPtr;
typedef vector< SvXMLStyleContextPtr > SvXMLStyleContexts_Impl;

typedef std::set<SvXMLStyleIndex_Impl, SvXMLStyleIndexCmp_Impl> SvXMLStyleIndices_Impl;

class SvXMLStylesContext_Impl
{
    SvXMLStyleContexts_Impl aStyles;
    SvXMLStyleIndices_Impl  *pIndices;
    sal_Bool bAutomaticStyle;

#ifdef DBG_UTIL
    sal_uInt32 nIndexCreated;
#endif

    void FlushIndex() { delete pIndices; pIndices = 0; }

public:
    SvXMLStylesContext_Impl( sal_Bool bAuto );
    ~SvXMLStylesContext_Impl();

    size_t GetStyleCount() const { return aStyles.size(); }

    const SvXMLStyleContext *GetStyle( size_t i ) const
    {
        return i < aStyles.size() ? aStyles[ i ] : 0;
    }

    SvXMLStyleContext *GetStyle( size_t i )
    {
        return i < aStyles.size() ? aStyles[ i ] : 0;
    }

    inline void AddStyle( SvXMLStyleContext *pStyle );
    void Clear();

    const SvXMLStyleContext *FindStyleChildContext( sal_uInt16 nFamily,
                            const OUString& rName, sal_Bool bCreateIndex ) const;
    sal_Bool IsAutomaticStyle() const { return bAutomaticStyle; }
};

SvXMLStylesContext_Impl::SvXMLStylesContext_Impl( sal_Bool bAuto ) :
    pIndices( 0 ),
    bAutomaticStyle( bAuto )
#ifdef DBG_UTIL
,   nIndexCreated( 0 )
#endif
{}

SvXMLStylesContext_Impl::~SvXMLStylesContext_Impl()
{
    delete pIndices;

    for ( size_t i = 0, n = aStyles.size(); i < n; ++i )
    {
        SvXMLStyleContext *pStyle = aStyles[ i ];
        pStyle->ReleaseRef();
    }
    aStyles.clear();
}

inline void SvXMLStylesContext_Impl::AddStyle( SvXMLStyleContext *pStyle )
{
    aStyles.push_back( pStyle );
    pStyle->AddRef();

    FlushIndex();
}

void SvXMLStylesContext_Impl::Clear()
{
    FlushIndex();

    for ( size_t i = 0, n = aStyles.size(); i < n; ++i )
    {
        SvXMLStyleContext *pStyle = aStyles[ i ];
        pStyle->ReleaseRef();
    }
    aStyles.clear();
}

const SvXMLStyleContext *SvXMLStylesContext_Impl::FindStyleChildContext(
                                  sal_uInt16 nFamily,
                                  const OUString& rName,
                                  sal_Bool bCreateIndex ) const
{
    const SvXMLStyleContext *pStyle = 0;

    if( !pIndices && bCreateIndex && !aStyles.empty() )
    {
#ifdef DBG_UTIL
        DBG_ASSERT( 0==nIndexCreated,
                    "Performance warning: sdbcx::Index created multiple times" );
#endif
        ((SvXMLStylesContext_Impl *)this)->pIndices =
            new SvXMLStyleIndices_Impl( aStyles.begin(), aStyles.end() );
        SAL_WARN_IF(pIndices->size() != aStyles.size(), "xmloff", "Here is a duplicate Style");
#ifdef DBG_UTIL
        ((SvXMLStylesContext_Impl *)this)->nIndexCreated++;
#endif
    }

    if( pIndices )
    {
        SvXMLStyleIndex_Impl aIndex( nFamily, rName );
        SvXMLStyleIndices_Impl::iterator aFind = pIndices->find(aIndex);
        if( aFind != pIndices->end() )
            pStyle = aFind->GetStyle();
    }
    else
    {
        for( size_t i = 0; !pStyle && i < aStyles.size(); i++ )
        {
            const SvXMLStyleContext *pS = aStyles[ i ];
            if( pS->GetFamily() == nFamily &&
                pS->GetName() == rName )
                pStyle = pS;
        }
    }
    return pStyle;
}

// ---------------------------------------------------------------------

TYPEINIT1( SvXMLStylesContext, SvXMLImportContext );

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

sal_Bool SvXMLStylesContext::IsAutomaticStyle() const
{
    return mpImpl->IsAutomaticStyle();
}

SvXMLStyleContext *SvXMLStylesContext::CreateStyleChildContext(
        sal_uInt16 p_nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = NULL;

    if(GetImport().GetDataStylesImport())
    {
        pStyle = GetImport().GetDataStylesImport()->CreateChildContext(GetImport(), p_nPrefix,
                                               rLocalName, xAttrList, *this);
    }

    if (!pStyle)
    {
        const SvXMLTokenMap& rTokenMap = GetStyleStylesElemTokenMap();
        sal_uInt16 nToken = rTokenMap.Get( p_nPrefix, rLocalName );
        switch( nToken  )
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
                //there is not page family in odf now, so I specify one for it
                sal_Bool bDefaultStyle  = XML_TOK_STYLE_DEFAULT_PAGE_LAYOUT == nToken
                    ? sal_True: sal_False;
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
                                                    rLocalName, xAttrList, sal_True );
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

            //
            // FillStyles
            //
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
    SvXMLStyleContext *pStyle = 0;

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
    return 0;
}


sal_Bool SvXMLStylesContext::InsertStyleFamily( sal_uInt16 ) const
{
    return sal_True;
}

sal_uInt16 SvXMLStylesContext::GetFamily(
        const ::rtl::OUString& rValue ) const
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

UniReference < SvXMLImportPropertyMapper > SvXMLStylesContext::GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        if( !mxParaImpPropMapper.is() )
        {
            SvXMLStylesContext * pThis = (SvXMLStylesContext *)this;
            pThis->mxParaImpPropMapper =
                pThis->GetImport().GetTextImport()
                     ->GetParaImportPropertySetMapper();
        }
        xMapper = mxParaImpPropMapper;
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        if( !mxTextImpPropMapper.is() )
        {
            SvXMLStylesContext * pThis = (SvXMLStylesContext *)this;
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
        xMapper = ((SvXMLStylesContext*)this)->GetImport().GetTextImport()->
            GetSectionImportPropertySetMapper();
        break;

    case XML_STYLE_FAMILY_TEXT_RUBY:
        // don't cache section mapper, as it's rarely used
        // *sigh*, cast to non-const, because this is a const method,
        // but SvXMLImport::GetTextImport() isn't.
        xMapper = ((SvXMLStylesContext*)this)->GetImport().GetTextImport()->
            GetRubyImportPropertySetMapper();
        break;

    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
    case XML_STYLE_FAMILY_SD_PRESENTATION_ID:
    case XML_STYLE_FAMILY_SD_POOL_ID:
        if(!mxShapeImpPropMapper.is())
        {
            UniReference< XMLShapeImportHelper > aImpHelper = ((SvXMLImport&)GetImport()).GetShapeImport();
            ((SvXMLStylesContext*)this)->mxShapeImpPropMapper =
                aImpHelper->GetPropertySetMapper();
        }
        xMapper = mxShapeImpPropMapper;
        break;
    case XML_STYLE_FAMILY_SCH_CHART_ID:
        if( ! mxChartImpPropMapper.is() )
        {
            XMLPropertySetMapper *pPropMapper = new XMLChartPropertySetMapper();
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
                                    ((SvXMLStylesContext*)this)->GetImport() );
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
            sName = bPara ?
                OUString( RTL_CONSTASCII_USTRINGPARAM( "ParagraphStyles" ) ):
                OUString( RTL_CONSTASCII_USTRINGPARAM( "CharacterStyles" ) );
            Reference< XAutoStylesSupplier > xAutoStylesSupp(   GetImport().GetModel(), UNO_QUERY );
            Reference< XAutoStyles > xAutoStyleFamilies = xAutoStylesSupp->getAutoStyles();
            if (xAutoStyleFamilies->hasByName(sName))
            {
                Any aAny = xAutoStyleFamilies->getByName( sName );
                xAutoStyles = *(Reference<XAutoStyleFamily>*)aAny.getValue();
                if( bPara )
                    ((SvXMLStylesContext *)this)->mxParaAutoStyles = xAutoStyles;
                else
                    ((SvXMLStylesContext *)this)->mxTextAutoStyles = xAutoStyles;
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
            sName =
                OUString( RTL_CONSTASCII_USTRINGPARAM( "ParagraphStyles" ) );
        break;

    case XML_STYLE_FAMILY_TEXT_TEXT:
        if( mxTextStyles.is() )
            xStyles = mxTextStyles;
        else
            sName =
                OUString( RTL_CONSTASCII_USTRINGPARAM( "CharacterStyles" ) );
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
                    ((SvXMLStylesContext *)this)->mxParaStyles = xStyles;
                    break;

                case XML_STYLE_FAMILY_TEXT_TEXT:
                    ((SvXMLStylesContext *)this)->mxTextStyles = xStyles;
                    break;
                }
            }
        }
    }

    return xStyles;
}

OUString SvXMLStylesContext::GetServiceName( sal_uInt16 nFamily ) const
{
    String sServiceName;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        sServiceName = msParaStyleServiceName;
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        sServiceName = msTextStyleServiceName;
        break;
    }

    return sServiceName;
}


SvXMLStylesContext::SvXMLStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                        const OUString& rLName,
                                        const uno::Reference< xml::sax::XAttributeList > &, sal_Bool bAuto ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    msParaStyleServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.ParagraphStyle" ) ),
    msTextStyleServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.CharacterStyle" ) ),
    mpImpl( new SvXMLStylesContext_Impl( bAuto ) ),
    mpStyleStylesElemTokenMap( 0 )
{
}


SvXMLStylesContext::~SvXMLStylesContext()
{
    delete mpStyleStylesElemTokenMap;
    delete mpImpl;
}

SvXMLImportContext *SvXMLStylesContext::CreateChildContext( sal_uInt16 nPrefix,
                                         const OUString& rLocalName,
                                         const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

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
        pStyle->CreateAndInsert( sal_False );
    }
}

void SvXMLStylesContext::CopyStylesToDoc( sal_Bool bOverwrite,
                                          sal_Bool bFinish )
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

void SvXMLStylesContext::FinishStyles( sal_Bool bOverwrite )
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
                                  sal_Bool bCreateIndex ) const
{
    return mpImpl->FindStyleChildContext( nFamily, rName, bCreateIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
