/*************************************************************************
 *
 *  $RCSfile: txtimppr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:07 $
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

#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif

#ifndef _XMLOFF_TEXTPRMAP_HXX_
#include "txtprmap.hxx"
#endif

#ifndef _XMLOFF_TXTIMPPR_HXX
#include "txtimppr.hxx"
#endif

#define XML_LINE_LEFT 0
#define XML_LINE_RIGHT 0
#define XML_LINE_TOP 0
#define XML_LINE_BOTTOM 0
#define MIN_BORDER_DIST 49

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;

XMLTextImportPropertyMapper::XMLTextImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper ) :
    SvXMLImportPropertyMapper( rMapper )
{
}

XMLTextImportPropertyMapper::~XMLTextImportPropertyMapper()
{
}

void XMLTextImportPropertyMapper::finished(
            ::std::vector< XMLPropertyState >& rProperties ) const
{
    XMLPropertyState* pFontName = 0;
    XMLPropertyState* pFontStyleName = 0;
    XMLPropertyState* pFontFamily = 0;
    XMLPropertyState* pFontPitch = 0;
    XMLPropertyState* pFontCharSet = 0;
    XMLPropertyState* pAllBorderDistance = 0;
    XMLPropertyState* pBorderDistances[4] = { 0, 0, 0, 0 };
    XMLPropertyState* pNewBorderDistances[4] = { 0, 0, 0, 0 };
    XMLPropertyState* pAllBorder = 0;
    XMLPropertyState* pBorders[4] = { 0, 0, 0, 0 };
    XMLPropertyState* pNewBorders[4] = { 0, 0, 0, 0 };
    XMLPropertyState* pAllBorderWidth = 0;
    XMLPropertyState* pBorderWidths[4] = { 0, 0, 0, 0 };
    XMLPropertyState* pAnchorType = 0;
    XMLPropertyState* pVertOrient = 0;
    XMLPropertyState* pVertOrientRelAsChar = 0;

    for( ::std::vector< XMLPropertyState >::iterator property = rProperties.begin();
         property != rProperties.end();
         property++ )
    {
        switch( getPropertySetMapper()->GetEntryContextId( property->mnIndex ) )
        {
        case CTF_FONTNAME:  pFontName = property;   break;
        case CTF_FONTSTYLENAME: pFontStyleName = property;  break;
        case CTF_FONTFAMILY:    pFontFamily = property; break;
        case CTF_FONTPITCH: pFontPitch = property;  break;
        case CTF_FONTCHARSET:   pFontCharSet = property;    break;

        case CTF_ALLBORDERDISTANCE:     pAllBorderDistance = property; break;
        case CTF_LEFTBORDERDISTANCE:    pBorderDistances[XML_LINE_LEFT] = property; break;
        case CTF_RIGHTBORDERDISTANCE:   pBorderDistances[XML_LINE_RIGHT] = property; break;
        case CTF_TOPBORDERDISTANCE:     pBorderDistances[XML_LINE_TOP] = property; break;
        case CTF_BOTTOMBORDERDISTANCE:  pBorderDistances[XML_LINE_BOTTOM] = property; break;
        case CTF_ALLBORDER:             pAllBorder = property; break;
        case CTF_LEFTBORDER:            pBorders[XML_LINE_LEFT] = property; break;
        case CTF_RIGHTBORDER:           pBorders[XML_LINE_RIGHT] = property; break;
        case CTF_TOPBORDER:             pBorders[XML_LINE_TOP] = property; break;
        case CTF_BOTTOMBORDER:          pBorders[XML_LINE_BOTTOM] = property; break;

        case CTF_ALLBORDERWIDTH:        pAllBorderWidth = property; break;
        case CTF_LEFTBORDERWIDTH:       pBorderWidths[XML_LINE_LEFT] = property; break;
        case CTF_RIGHTBORDERWIDTH:      pBorderWidths[XML_LINE_RIGHT] = property; break;
        case CTF_TOPBORDERWIDTH:        pBorderWidths[XML_LINE_TOP] = property; break;
        case CTF_BOTTOMBORDERWIDTH:     pBorderWidths[XML_LINE_BOTTOM] = property; break;
        case CTF_ANCHORTYPE:            pAnchorType = property; break;
        case CTF_VERTICALPOS:           pVertOrient = property; break;
        case CTF_VERTICALREL_ASCHAR:    pVertOrientRelAsChar = property; break;
        }
    }

    if( pFontName )
    {
        OUString sName;
        pFontName->maValue >>= sName;
        if( !sName.getLength() )
        {
            pFontName->mnIndex = -1;
            pFontName = 0;
        }
    }
    if( !pFontName )
    {
        if( pFontStyleName )
            pFontStyleName->mnIndex = -1;
        if( pFontFamily )
            pFontFamily->mnIndex = -1;
        if( pFontPitch )
            pFontPitch->mnIndex = -1;
        if( pFontCharSet )
            pFontCharSet->mnIndex = -1;
    }

    for( sal_uInt16 i=0; i<4; i++ )
    {
        if( pAllBorderDistance && !pBorderDistances[i] )
        {
#ifndef PRODUCT
            sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                        pAllBorderDistance->mnIndex + i + 1 );
            DBG_ASSERT( nTmp >= CTF_LEFTBORDERDISTANCE &&
                        nTmp <= CTF_BOTTOMBORDERDISTANCE,
                        "wrong property context id" );
#endif
            pNewBorderDistances[i] =
                new XMLPropertyState( pAllBorderDistance->mnIndex + i + 1,
                                      pAllBorderDistance->maValue );
            pBorderDistances[i] = pNewBorderDistances[i];
        }
        if( pAllBorder && !pBorders[i] )
        {
#ifndef PRODUCT
            sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                            pAllBorder->mnIndex + i + 1 );
            DBG_ASSERT( nTmp >= CTF_LEFTBORDER && nTmp <= CTF_BOTTOMBORDER,
                        "wrong property context id" );
#endif
            pNewBorders[i] = new XMLPropertyState( pAllBorder->mnIndex + i + 1,
                                                   pAllBorder->maValue );
            pBorders[i] = pNewBorders[i];
        }
        if( !pBorderWidths[i] )
            pBorderWidths[i] = pAllBorderWidth;
        else
            pBorderWidths[i]->mnIndex = -1;

#ifdef XML_CHECK_UI_CONSTRAINS
        sal_Bool bHasBorder = sal_False;
        if( pBorders[i] )
        {
            table::BorderLine aBorderLine;
            pBorders[i]->maValue >>= aBorderLine;

             if( pBorderWidths[i] )
            {
                table::BorderLine aBorderLineWidth;
                pBorderWidths[i]->maValue >>= aBorderLineWidth;
                aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
                aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
                aBorderLine.LineDistance = aBorderLineWidth.LineDistance;

                pBorders[i]->maValue <<= aBorderLine;
            }
            bHasBorder = (aBorderLine.OuterLineWidth +
                          aBorderLine.InnerLineWidth) > 0;
        }
        if( bHasBorder )
        {
            if( !pBorderDistances[i] )
            {
#ifndef PRODUCT
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                        pBorders[i]->mnIndex + 5 );
                DBG_ASSERT( nTmp >= CTF_LEFTBORDERDISTANCE &&
                            nTmp <= CTF_BOTTOMBORDERDISTANCE,
                            "wrong property context id" );
#endif

                pNewBorderDistances[i] =
                    new XMLPropertyState( pBorders[i]->mnIndex + 5 );
                pNewBorderDistances[i]->maValue <<= (sal_Int32)MIN_BORDER_DIST;
                pBorderDistances[i] = pNewBorderDistances[i];
            }
            else
            {
                sal_Int32 nDist;
                pBorderDistances[i]->maValue >>= nDist;
                if( nDist < MIN_BORDER_DIST )
                    pBorderDistances[i]->maValue <<= (sal_Int32)MIN_BORDER_DIST;
            }
        }
        else
        {
            if( pBorderDistances[i] )
            {
                sal_Int32 nDist;
                pBorderDistances[i]->maValue >>= nDist;
                if( nDist > 0 )
                    pBorderDistances[i]->maValue <<= (sal_Int32)0;
            }
        }
#else
        if( pBorders[i] && pBorderWidths[i] )
        {
            table::BorderLine aBorderLine;
            pBorders[i]->maValue >>= aBorderLine;

            table::BorderLine aBorderLineWidth;
            pBorderWidths[i]->maValue >>= aBorderLineWidth;

            aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
            aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
            aBorderLine.LineDistance = aBorderLineWidth.LineDistance;

            pBorders[i]->maValue <<= aBorderLine;
        }
#endif
    }
    if( pAllBorderDistance )
        pAllBorderDistance->mnIndex = -1;

    if( pAllBorder )
        pAllBorder->mnIndex = -1;

    if( pAllBorderWidth )
        pAllBorderWidth->mnIndex = -1;

    // insert newly created properties. This inavlidates all iterators!
    for( i=0; i<4; i++ )
    {
        if( pNewBorderDistances[i] )
        {
            rProperties.push_back( *pNewBorderDistances[i] );
            delete pNewBorderDistances[i];
        }
        if( pNewBorders[i] )
        {
            rProperties.push_back( *pNewBorders[i] );
            delete pNewBorders[i];
        }
    }

    if( pVertOrient && pVertOrientRelAsChar )
    {
        TextContentAnchorType eAnchorType;
        if( pAnchorType && (pAnchorType->maValue >>= eAnchorType) &&
            TextContentAnchorType_AS_CHARACTER == eAnchorType )
        {
            sal_Int16 nVertOrient;
            pVertOrient->maValue >>= nVertOrient;
            sal_Int16 nVertOrientRel;
            pVertOrientRelAsChar->maValue >>= nVertOrientRel;
            switch( nVertOrient )
            {
            case VertOrientation::TOP:
                nVertOrient = nVertOrientRel;
                break;
            case VertOrientation::CENTER:
                switch( nVertOrientRel )
                {
                case VertOrientation::CHAR_TOP:
                    nVertOrient = VertOrientation::CHAR_CENTER;
                    break;
                case VertOrientation::LINE_TOP:
                    nVertOrient = VertOrientation::LINE_CENTER;
                    break;
                }
                break;
            case VertOrientation::BOTTOM:
                switch( nVertOrientRel )
                {
                case VertOrientation::CHAR_TOP:
                    nVertOrient = VertOrientation::CHAR_BOTTOM;
                    break;
                case VertOrientation::LINE_TOP:
                    nVertOrient = VertOrientation::LINE_BOTTOM;
                    break;
                }
                break;
            }
            pVertOrient->maValue <<= nVertOrientRel;
        }
        pVertOrientRelAsChar->mnIndex = -1;
    }
}


