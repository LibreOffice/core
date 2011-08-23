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

#ifndef _TOOLS_DEBUG_HXX 
#include <tools/debug.hxx>
#endif


#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#include "XMLBase64ImportContext.hxx"
#endif

#ifndef _XMLBACKGROUNDIMAGECONTEXT_HXX
#include "XMLBackgroundImageContext.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::io;
using namespace ::binfilter::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_BGIMG_HREF,
    XML_TOK_BGIMG_TYPE,
    XML_TOK_BGIMG_ACTUATE,
    XML_TOK_BGIMG_SHOW,
    XML_TOK_BGIMG_POSITION,
    XML_TOK_BGIMG_REPEAT,
    XML_TOK_BGIMG_FILTER,
    XML_TOK_BGIMG_TRANSPARENCY,
    XML_TOK_NGIMG_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aBGImgAttributesAttrTokenMap[] =
{
    { XML_NAMESPACE_XLINK, XML_HREF, 		XML_TOK_BGIMG_HREF		},
    { XML_NAMESPACE_XLINK, XML_TYPE, 		XML_TOK_BGIMG_TYPE		},
    { XML_NAMESPACE_XLINK, XML_ACTUATE,	    XML_TOK_BGIMG_ACTUATE	},
    { XML_NAMESPACE_XLINK, XML_SHOW, 		XML_TOK_BGIMG_SHOW 		},
    { XML_NAMESPACE_STYLE, XML_POSITION, 	XML_TOK_BGIMG_POSITION	},
    { XML_NAMESPACE_STYLE, XML_REPEAT, 	    XML_TOK_BGIMG_REPEAT	},
    { XML_NAMESPACE_STYLE, XML_FILTER_NAME, XML_TOK_BGIMG_FILTER	},
    { XML_NAMESPACE_DRAW,  XML_TRANSPARENCY,XML_TOK_BGIMG_TRANSPARENCY },
    XML_TOKEN_MAP_END 
};

SvXMLEnumMapEntry psXML_BrushRepeat[] =
{
    { XML_BACKGROUND_REPEAT,		GraphicLocation_TILED	},
    { XML_BACKGROUND_NO_REPEAT, 	GraphicLocation_MIDDLE_MIDDLE		},
    { XML_BACKGROUND_STRETCH,		GraphicLocation_AREA	},
    { XML_TOKEN_INVALID,			0			}
};

SvXMLEnumMapEntry psXML_BrushHoriPos[] =
{
    { XML_LEFT, 		GraphicLocation_LEFT_MIDDLE	},
    { XML_RIGHT,		GraphicLocation_RIGHT_MIDDLE	},
    { XML_TOKEN_INVALID,					0			}
};

SvXMLEnumMapEntry psXML_BrushVertPos[] =
{
    { XML_TOP,			GraphicLocation_MIDDLE_TOP	},
    { XML_BOTTOM,		GraphicLocation_MIDDLE_BOTTOM	},
    { XML_TOKEN_INVALID,					0			}
};

void lcl_xmlbic_MergeHoriPos( GraphicLocation& ePos,
                                   GraphicLocation eHori )
{
    DBG_ASSERT( GraphicLocation_LEFT_MIDDLE==eHori ||
                GraphicLocation_MIDDLE_MIDDLE==eHori ||
                GraphicLocation_RIGHT_MIDDLE==eHori,
                "lcl_xmlbic_MergeHoriPos: vertical pos must be middle" );

    switch( ePos )
    {
    case GraphicLocation_LEFT_TOP:
    case GraphicLocation_MIDDLE_TOP:
    case GraphicLocation_RIGHT_TOP:
        ePos = GraphicLocation_LEFT_MIDDLE==eHori
                ? GraphicLocation_LEFT_TOP
                : (GraphicLocation_MIDDLE_MIDDLE==eHori
                        ? GraphicLocation_MIDDLE_TOP
                        : GraphicLocation_RIGHT_TOP);
        break;

    case GraphicLocation_LEFT_MIDDLE:
    case GraphicLocation_MIDDLE_MIDDLE:
    case GraphicLocation_RIGHT_MIDDLE:
        ePos = eHori;
        break;

    case GraphicLocation_LEFT_BOTTOM:
    case GraphicLocation_MIDDLE_BOTTOM:
    case GraphicLocation_RIGHT_BOTTOM:
        ePos = GraphicLocation_LEFT_MIDDLE==eHori
                ? GraphicLocation_LEFT_BOTTOM
                : (GraphicLocation_MIDDLE_MIDDLE==eHori
                        ? GraphicLocation_MIDDLE_BOTTOM
                        : GraphicLocation_RIGHT_BOTTOM);
        break;
    }
}

void lcl_xmlbic_MergeVertPos( GraphicLocation& ePos,
                                                   GraphicLocation eVert )
{
    DBG_ASSERT( GraphicLocation_MIDDLE_TOP==eVert ||
                GraphicLocation_MIDDLE_MIDDLE==eVert ||
                GraphicLocation_MIDDLE_BOTTOM==eVert,
                "lcl_xmlbic_MergeVertPos: horizontal pos must be middle" );

    switch( ePos )
    {
    case GraphicLocation_LEFT_TOP:
    case GraphicLocation_LEFT_MIDDLE:
    case GraphicLocation_LEFT_BOTTOM:
        ePos = GraphicLocation_MIDDLE_TOP==eVert
                ? GraphicLocation_LEFT_TOP
                : (GraphicLocation_MIDDLE_MIDDLE==eVert
                        ? GraphicLocation_LEFT_MIDDLE
                        : GraphicLocation_LEFT_BOTTOM);
        ePos = eVert;
        break;

    case GraphicLocation_MIDDLE_TOP:
    case GraphicLocation_MIDDLE_MIDDLE:
    case GraphicLocation_MIDDLE_BOTTOM:
        ePos = eVert;
        break;

    case GraphicLocation_RIGHT_TOP:
    case GraphicLocation_RIGHT_MIDDLE:
    case GraphicLocation_RIGHT_BOTTOM:
        ePos = GraphicLocation_MIDDLE_TOP==eVert
                ? GraphicLocation_RIGHT_TOP
                : (GraphicLocation_MIDDLE_MIDDLE==eVert
                        ? GraphicLocation_RIGHT_MIDDLE
                        : GraphicLocation_RIGHT_BOTTOM);
        break;
    }
}

TYPEINIT1( XMLBackgroundImageContext, XMLElementPropertyContext );

void XMLBackgroundImageContext::ProcessAttrs(
        const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLTokenMap aTokenMap( aBGImgAttributesAttrTokenMap );

    ePos = GraphicLocation_NONE;

    SvXMLUnitConverter& rUnitConverter = GetImport().GetMM100UnitConverter();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_BGIMG_HREF:
            sURL = rValue;
            if( GraphicLocation_NONE == ePos )
                ePos = GraphicLocation_TILED;
            break;
        case XML_TOK_BGIMG_TYPE:
        case XML_TOK_BGIMG_ACTUATE:
        case XML_TOK_BGIMG_SHOW:
            break;
        case XML_TOK_BGIMG_POSITION:
            {
                GraphicLocation eNewPos = GraphicLocation_NONE, eTmp;
                sal_uInt16 nTmp;
                SvXMLTokenEnumerator aTokenEnum( rValue );
                OUString aToken;
                sal_Bool bHori = sal_False, bVert = sal_False;
                sal_Bool bOK = sal_True;
                while( bOK && aTokenEnum.getNextToken( aToken ) )
                {
                    if( bHori && bVert )
                    {
                        bOK = sal_False;
                    }
                    else if( -1 != aToken.indexOf( sal_Unicode('%') ) )
                    {
                        sal_Int32 nPrc = 50;
                        if( rUnitConverter.convertPercent( nPrc, aToken ) )
                        {
                            if( !bHori )
                            {
                                eNewPos = nPrc < 25
                                    ? GraphicLocation_LEFT_TOP
                                    : (nPrc < 75 ? GraphicLocation_MIDDLE_MIDDLE
                                                : GraphicLocation_RIGHT_BOTTOM);
                                bHori = sal_True;
                            }
                            else
                            {
                                eTmp = nPrc < 25
                                    ? GraphicLocation_LEFT_TOP
                                    : (nPrc < 75 ? GraphicLocation_LEFT_MIDDLE
                                                 : GraphicLocation_LEFT_BOTTOM);
                                lcl_xmlbic_MergeVertPos( eNewPos, eTmp );
                                bVert = sal_True;
                            }
                        }
                        else
                        {
                            // wrong percentage
                            bOK = sal_False;
                        }
                    }
                    else if( IsXMLToken( aToken, XML_CENTER ) )
                    {
                        if( bHori )
                            lcl_xmlbic_MergeVertPos( eNewPos,
                                          GraphicLocation_MIDDLE_MIDDLE );
                        else if ( bVert )
                            lcl_xmlbic_MergeHoriPos( eNewPos,
                                          GraphicLocation_MIDDLE_MIDDLE );
                        else
                            eNewPos = GraphicLocation_MIDDLE_MIDDLE;
                    }
                    else if( rUnitConverter.convertEnum( nTmp, aToken,
                                                         psXML_BrushHoriPos ) )
                    {
                        if( bVert )
                            lcl_xmlbic_MergeHoriPos( eNewPos,
                                        (GraphicLocation)nTmp );
                        else if( !bHori )
                            eNewPos = (GraphicLocation)nTmp;
                        else
                            bOK = sal_False;
                        bHori = sal_True;
                    }
                    else if( rUnitConverter.convertEnum( nTmp, aToken,
                                                         psXML_BrushVertPos ) )
                    {
                        if( bHori )
                            lcl_xmlbic_MergeVertPos( eNewPos,
                                            (GraphicLocation)nTmp );
                        else if( !bVert )
                            eNewPos = (GraphicLocation)nTmp;
                        else
                            bOK = sal_False;
                        bVert = sal_True;
                    }
                    else
                    {
                        bOK = sal_False;
                    }
                }

                bOK &= GraphicLocation_NONE != eNewPos;
                if( bOK )
                    ePos = eNewPos;
            }
            break;
        case XML_TOK_BGIMG_REPEAT:
            {
                sal_uInt16 nPos = GraphicLocation_NONE;
                if( rUnitConverter.convertEnum( nPos, rValue,
                                                psXML_BrushRepeat ) )
                {
                    if( GraphicLocation_MIDDLE_MIDDLE != nPos ||
                        GraphicLocation_NONE == ePos ||
                        GraphicLocation_AREA == ePos ||
                        GraphicLocation_TILED == ePos )
                        ePos = (GraphicLocation)nPos;
                }
            }
            break;
        case XML_TOK_BGIMG_FILTER:
            sFilter = rValue;
            break;
        case XML_TOK_BGIMG_TRANSPARENCY:
            {
                sal_Int32 nTmp;
                // convert from percent and clip
                if( SvXMLUnitConverter::convertPercent( nTmp, rValue ) )
                {
                    if( (nTmp >= 0) && (nTmp <= 100) )
                        nTransparency = static_cast<sal_Int8>( nTmp );
                }
            }
            break;
        }
    }
    
}

XMLBackgroundImageContext::XMLBackgroundImageContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nPosIdx,
        sal_Int32 nFilterIdx,
        sal_Int32 nTransparencyIdx,
        ::std::vector< XMLPropertyState > &rProps ) :
    XMLElementPropertyContext( rImport, nPrfx, rLName, rProp, rProps ),
    aPosProp( nPosIdx ),
    aFilterProp( nFilterIdx ),
    aTransparencyProp( nTransparencyIdx ),
    nTransparency( 0 )
{
    ProcessAttrs( xAttrList );
}

XMLBackgroundImageContext::~XMLBackgroundImageContext()
{
}

SvXMLImportContext *XMLBackgroundImageContext::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = NULL;
    if( (XML_NAMESPACE_OFFICE == nPrefix) &&
        xmloff::token::IsXMLToken( rLocalName,
                                        xmloff::token::XML_BINARY_DATA ) )
    {
        if( !sURL.getLength() && !xBase64Stream.is() )
        {
            xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( xBase64Stream.is() )
                pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    xBase64Stream );
        }
    }
    if( !pContext )
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void XMLBackgroundImageContext::EndElement()
{
    if( sURL.getLength() )
    {
        sURL = GetImport().ResolveGraphicObjectURL( sURL, sal_False );
    }
    else if( xBase64Stream.is() )
    {
        sURL = GetImport().ResolveGraphicObjectURLFromBase64( xBase64Stream );
        xBase64Stream = 0;
    }

    if( !sURL.getLength() )
        ePos = GraphicLocation_NONE;
    else if( GraphicLocation_NONE == ePos )
        ePos = GraphicLocation_TILED;

    aProp.maValue <<= sURL;
    aPosProp.maValue <<= ePos;
    aFilterProp.maValue <<= sFilter;
    aTransparencyProp.maValue <<= nTransparency;

    SetInsert( sal_True );
    XMLElementPropertyContext::EndElement();

    if( -1 != aPosProp.mnIndex )
        rProperties.push_back( aPosProp );
    if( -1 != aFilterProp.mnIndex )
        rProperties.push_back( aFilterProp );
    if( -1 != aTransparencyProp.mnIndex )
        rProperties.push_back( aTransparencyProp );
}
}//end of namespace binfilter
