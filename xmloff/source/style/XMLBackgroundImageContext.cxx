/*************************************************************************
 *
 *  $RCSfile: XMLBackgroundImageContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-19 14:25:17 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLBACKGROUNDIMAGECONTEXT_HXX
#include "XMLBackgroundImageContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;

enum SvXMLTokenMapAttrs
{
    XML_TOK_BGIMG_HREF,
    XML_TOK_BGIMG_TYPE,
    XML_TOK_BGIMG_ACTUATE,
    XML_TOK_BGIMG_SHOW,
    XML_TOK_BGIMG_POSITION,
    XML_TOK_BGIMG_REPEAT,
    XML_TOK_BGIMG_FILTER,
    XML_TOK_NGIMG_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aBGImgAttributesAttrTokenMap[] =
{
    { XML_NAMESPACE_XLINK, sXML_href,       XML_TOK_BGIMG_HREF      },
    { XML_NAMESPACE_XLINK, sXML_type,       XML_TOK_BGIMG_TYPE      },
    { XML_NAMESPACE_XLINK, sXML_actuate,    XML_TOK_BGIMG_ACTUATE   },
    { XML_NAMESPACE_XLINK, sXML_show,       XML_TOK_BGIMG_SHOW      },
    { XML_NAMESPACE_STYLE, sXML_position,   XML_TOK_BGIMG_POSITION  },
    { XML_NAMESPACE_STYLE, sXML_repeat,     XML_TOK_BGIMG_REPEAT    },
    { XML_NAMESPACE_STYLE, sXML_filter_name,XML_TOK_BGIMG_FILTER    },
    XML_TOKEN_MAP_END
};

SvXMLEnumMapEntry psXML_BrushRepeat[] =
{
    { sXML_background_repeat,       GraphicLocation_TILED   },
    { sXML_background_no_repeat,    GraphicLocation_MIDDLE_MIDDLE       },
    { sXML_background_stretch,      GraphicLocation_AREA    },
    { 0,                            0           }
};

SvXMLEnumMapEntry psXML_BrushHoriPos[] =
{
    { sXML_left,        GraphicLocation_LEFT_MIDDLE },
    { sXML_right,       GraphicLocation_RIGHT_MIDDLE    },
    { 0,                            0           }
};

SvXMLEnumMapEntry psXML_BrushVertPos[] =
{
    { sXML_top,         GraphicLocation_MIDDLE_TOP  },
    { sXML_bottom,      GraphicLocation_MIDDLE_BOTTOM   },
    { 0,                            0           }
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

    OUString sURL, sFilter;
    GraphicLocation ePos = GraphicLocation_NONE;

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
                        long nPrc = 50;
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
                    else if( aToken.equalsAsciiL( sXML_center,
                                                  sizeof(sXML_center)-1 ) )
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
        }
    }

    if( !sURL.getLength() )
        ePos = GraphicLocation_NONE;
    else if( GraphicLocation_NONE == ePos )
        ePos = GraphicLocation_TILED;

    aProp.maValue <<= sURL;
    aPosProp.maValue <<= ePos;
    aFilterProp.maValue <<= sFilter;
}

XMLBackgroundImageContext::XMLBackgroundImageContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nPosIdx,
        sal_Int32 nFilterIdx,
        ::std::vector< XMLPropertyState > &rProps ) :
    XMLElementPropertyContext( rImport, nPrfx, rLName, rProp, rProps ),
    aPosProp( nPosIdx ),
    aFilterProp( nFilterIdx )
{
    ProcessAttrs( xAttrList );
}

XMLBackgroundImageContext::~XMLBackgroundImageContext()
{
}

void XMLBackgroundImageContext::EndElement()
{
    SetInsert( sal_True );
    XMLElementPropertyContext::EndElement();

    if( -1 != aPosProp.mnIndex )
        rProperties.push_back( aPosProp );
    if( -1 != aFilterProp.mnIndex )
        rProperties.push_back( aFilterProp );
}
