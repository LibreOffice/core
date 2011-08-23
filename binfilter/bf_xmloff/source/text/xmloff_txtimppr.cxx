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

#include <tools/debug.hxx>

#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>

#include <tools/string.hxx>

#include "XMLFontStylesContext.hxx"

#include "txtprmap.hxx"

#include "txtimppr.hxx"
namespace binfilter {

#define XML_LINE_LEFT 0
#define XML_LINE_RIGHT 1
#define XML_LINE_TOP 2
#define XML_LINE_BOTTOM 3
#define MIN_BORDER_DIST 49

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;

sal_Bool XMLTextImportPropertyMapper::handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const ::rtl::OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nIndex = rProperty.mnIndex;
    switch( getPropertySetMapper()->GetEntryContextId( nIndex  ) )
    {
    case CTF_FONTNAME:
    case CTF_FONTNAME_CJK:
    case CTF_FONTNAME_CTL:
        if( xFontDecls.Is() )
        {
            DBG_ASSERT(
                ( CTF_FONTFAMILYNAME ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+1) &&
                  CTF_FONTSTYLENAME ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+2) &&
                  CTF_FONTFAMILY ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+3) &&
                  CTF_FONTPITCH ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+4) &&
                  CTF_FONTCHARSET ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+5) ) ||
                ( CTF_FONTFAMILYNAME_CJK ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+1) &&
                  CTF_FONTSTYLENAME_CJK ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+2) &&
                  CTF_FONTFAMILY_CJK ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+3) &&
                  CTF_FONTPITCH_CJK ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+4) &&
                  CTF_FONTCHARSET_CJK ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+5) ) ||
                ( CTF_FONTFAMILYNAME_CTL ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+1) &&
                  CTF_FONTSTYLENAME_CTL ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+2) &&
                  CTF_FONTFAMILY_CTL ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+3) &&
                  CTF_FONTPITCH_CTL ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+4) &&
                  CTF_FONTCHARSET_CTL ==
                    getPropertySetMapper()->GetEntryContextId(nIndex+5) ),
                "illegal property map" );

            ((XMLFontStylesContext *)&xFontDecls)->FillProperties(
                            rValue, rProperties,
                            rProperty.mnIndex+1, rProperty.mnIndex+2,
                            rProperty.mnIndex+3, rProperty.mnIndex+4,
                            rProperty.mnIndex+5 );
            bRet = sal_False; // the property hasn't been filled
        }
        break;

#ifdef CONV_STAR_FONTS
    // If we want to do StarMath/StarSymbol font conversion, then we'll 
    // want these special items to be treated just like regular ones...
    // For the Writer, we'll catch and convert them in _FillPropertySet;
    // the other apps probably don't care. For the other apps, we just
    // imitate the default non-special-item mechanism.
    case CTF_FONTFAMILYNAME:
    case CTF_FONTFAMILYNAME_CJK:
    case CTF_FONTFAMILYNAME_CTL:
        bRet = getPropertySetMapper()->importXML( rValue, rProperty,
                                                  rUnitConverter );			
        break;
#endif

    default:
        bRet = SvXMLImportPropertyMapper::handleSpecialItem( rProperty,
                    rProperties, rValue, rUnitConverter, rNamespaceMap );
        break;
    }

    return bRet;
}

XMLTextImportPropertyMapper::XMLTextImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport,
            XMLFontStylesContext *pFontDecls ) :
    SvXMLImportPropertyMapper( rMapper, rImport ),
    nSizeTypeIndex( -2 ),
    xFontDecls( pFontDecls )
{
}

XMLTextImportPropertyMapper::~XMLTextImportPropertyMapper()
{
}

void XMLTextImportPropertyMapper::SetFontDecls(
        XMLFontStylesContext *pFontDecls )
{
    xFontDecls = pFontDecls;
}

void XMLTextImportPropertyMapper::FontFinished(
    XMLPropertyState *pFontFamilyNameState,
    XMLPropertyState *pFontStyleNameState,
    XMLPropertyState *pFontFamilyState,
    XMLPropertyState *pFontPitchState,
    XMLPropertyState *pFontCharsetState ) const
{
    if( pFontFamilyNameState && pFontFamilyNameState->mnIndex != -1 )
    {
        OUString sName;
        pFontFamilyNameState->maValue >>= sName;
        if( !sName.getLength() )
            pFontFamilyNameState->mnIndex = -1;
    }
    if( !pFontFamilyNameState || pFontFamilyNameState->mnIndex == -1 )
    {
        if( pFontStyleNameState )
            pFontStyleNameState->mnIndex = -1;
        if( pFontFamilyState )
            pFontFamilyState->mnIndex = -1;
        if( pFontPitchState )
            pFontPitchState->mnIndex = -1;
        if( pFontCharsetState )
            pFontCharsetState->mnIndex = -1;
    }
}

/** since the properties "CharFontFamilyName", "CharFontStyleName", "CharFontFamily",
    "CharFontPitch" and "CharFontSet" and theire CJK and CTL counterparts are only
    usable as a union, we add defaults to all values that are not set as long as we
    have an "CharFontFamilyName"

    #99928# CL */
void XMLTextImportPropertyMapper::FontDefaultsCheck(
                                        XMLPropertyState* pFontFamilyName,
                                        XMLPropertyState* pFontStyleName,
                                        XMLPropertyState* pFontFamily,
                                        XMLPropertyState* pFontPitch,
                                        XMLPropertyState* pFontCharSet,
                                        XMLPropertyState** ppNewFontStyleName,
                                        XMLPropertyState** ppNewFontFamily,
                                        XMLPropertyState** ppNewFontPitch,
                                        XMLPropertyState** ppNewFontCharSet ) const
{
    if( pFontFamilyName )
    {
        OUString sEmpty;
        Any aAny;

        if( !pFontStyleName )
        {
            aAny <<= sEmpty;
    #ifdef DBG_UTIL
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                                pFontFamilyName->mnIndex + 1 );
                DBG_ASSERT( nTmp == CTF_FONTSTYLENAME || nTmp == CTF_FONTSTYLENAME_CJK || nTmp == CTF_FONTSTYLENAME_CTL,
                            "wrong property context id" );
    #endif
                *ppNewFontStyleName = new XMLPropertyState( pFontFamilyName->mnIndex + 1,
                                                       aAny );
        }

        if( !pFontFamily )
        {
            aAny <<= (sal_Int16)com::sun::star::awt::FontFamily::DONTKNOW;

    #ifdef DBG_UTIL
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                                pFontFamilyName->mnIndex + 2 );
                DBG_ASSERT( nTmp == CTF_FONTFAMILY || nTmp == CTF_FONTFAMILY_CJK || nTmp == CTF_FONTFAMILY_CTL,
                            "wrong property context id" );
    #endif
                *ppNewFontFamily = new XMLPropertyState( pFontFamilyName->mnIndex + 2,
                                                       aAny );
        }

        if( !pFontPitch )
        {
            aAny <<= (sal_Int16)com::sun::star::awt::FontPitch::DONTKNOW;
    #ifdef DBG_UTIL
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                                pFontFamilyName->mnIndex + 3 );
                DBG_ASSERT( nTmp == CTF_FONTPITCH || nTmp == CTF_FONTPITCH_CJK || nTmp == CTF_FONTPITCH_CTL,
                            "wrong property context id" );
    #endif
                *ppNewFontPitch = new XMLPropertyState( pFontFamilyName->mnIndex + 3,
                                                       aAny );
        }

        if( !pFontCharSet )
        {
            aAny <<= (sal_Int16)gsl_getSystemTextEncoding();
    #ifdef DBG_UTIL
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                                pFontFamilyName->mnIndex + 4 );
                DBG_ASSERT( nTmp == CTF_FONTCHARSET || nTmp == CTF_FONTCHARSET_CJK || nTmp == CTF_FONTCHARSET_CTL,
                            "wrong property context id" );
    #endif
                *ppNewFontCharSet = new XMLPropertyState( pFontFamilyName->mnIndex + 4,
                                                       aAny );
        }
    }
}

void XMLTextImportPropertyMapper::finished(
            ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const
{
    sal_Bool bHasAnyHeight = sal_False;
    sal_Bool bHasAnyMinHeight = sal_False;

    XMLPropertyState* pFontFamilyName = 0;
    XMLPropertyState* pFontStyleName = 0;
    XMLPropertyState* pFontFamily = 0;
    XMLPropertyState* pFontPitch = 0;
    XMLPropertyState* pFontCharSet = 0;
    XMLPropertyState* pNewFontStyleName = 0;
    XMLPropertyState* pNewFontFamily = 0;
    XMLPropertyState* pNewFontPitch = 0;
    XMLPropertyState* pNewFontCharSet = 0;
    XMLPropertyState* pFontFamilyNameCJK = 0;
    XMLPropertyState* pFontStyleNameCJK = 0;
    XMLPropertyState* pFontFamilyCJK = 0;
    XMLPropertyState* pFontPitchCJK = 0;
    XMLPropertyState* pFontCharSetCJK = 0;
    XMLPropertyState* pNewFontStyleNameCJK = 0;
    XMLPropertyState* pNewFontFamilyCJK = 0;
    XMLPropertyState* pNewFontPitchCJK = 0;
    XMLPropertyState* pNewFontCharSetCJK = 0;
    XMLPropertyState* pFontFamilyNameCTL = 0;
    XMLPropertyState* pFontStyleNameCTL = 0;
    XMLPropertyState* pFontFamilyCTL = 0;
    XMLPropertyState* pFontPitchCTL = 0;
    XMLPropertyState* pFontCharSetCTL = 0;
    XMLPropertyState* pNewFontStyleNameCTL = 0;
    XMLPropertyState* pNewFontFamilyCTL = 0;
    XMLPropertyState* pNewFontPitchCTL = 0;
    XMLPropertyState* pNewFontCharSetCTL = 0;
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
    XMLPropertyState* pBackTransparency = NULL; // transparency in %
    XMLPropertyState* pBackTransparent = NULL;  // transparency as boolean

    for( ::std::vector< XMLPropertyState >::iterator property = rProperties.begin();
         property != rProperties.end();
         property++ )
    {
        switch( getPropertySetMapper()->GetEntryContextId( property->mnIndex ) )
        {
        case CTF_FONTFAMILYNAME:	pFontFamilyName = &(*property);	break;
        case CTF_FONTSTYLENAME:	pFontStyleName = &(*property);	break;
        case CTF_FONTFAMILY:	pFontFamily = &(*property);	break;
        case CTF_FONTPITCH:	pFontPitch = &(*property);	break;
        case CTF_FONTCHARSET:	pFontCharSet = &(*property);	break;
                                
        case CTF_FONTFAMILYNAME_CJK:	pFontFamilyNameCJK = &(*property);	break;
        case CTF_FONTSTYLENAME_CJK:	pFontStyleNameCJK = &(*property);	break;
        case CTF_FONTFAMILY_CJK:	pFontFamilyCJK = &(*property);	break;
        case CTF_FONTPITCH_CJK:	pFontPitchCJK = &(*property);	break;
        case CTF_FONTCHARSET_CJK:	pFontCharSetCJK = &(*property);	break;

        case CTF_FONTFAMILYNAME_CTL:	pFontFamilyNameCTL = &(*property);	break;
        case CTF_FONTSTYLENAME_CTL:	pFontStyleNameCTL = &(*property);	break;
        case CTF_FONTFAMILY_CTL:	pFontFamilyCTL = &(*property);	break;
        case CTF_FONTPITCH_CTL:	pFontPitchCTL = &(*property);	break;
        case CTF_FONTCHARSET_CTL:	pFontCharSetCTL = &(*property);	break;

        case CTF_ALLBORDERDISTANCE:		pAllBorderDistance = &(*property); break;
        case CTF_LEFTBORDERDISTANCE:	pBorderDistances[XML_LINE_LEFT] = &(*property); break;
        case CTF_RIGHTBORDERDISTANCE:	pBorderDistances[XML_LINE_RIGHT] = &(*property); break;
        case CTF_TOPBORDERDISTANCE:		pBorderDistances[XML_LINE_TOP] = &(*property); break;
        case CTF_BOTTOMBORDERDISTANCE:	pBorderDistances[XML_LINE_BOTTOM] = &(*property); break;
        case CTF_ALLBORDER:				pAllBorder = &(*property); break;
        case CTF_LEFTBORDER:			pBorders[XML_LINE_LEFT] = &(*property); break;
        case CTF_RIGHTBORDER:			pBorders[XML_LINE_RIGHT] = &(*property); break;
        case CTF_TOPBORDER:				pBorders[XML_LINE_TOP] = &(*property); break;
        case CTF_BOTTOMBORDER:			pBorders[XML_LINE_BOTTOM] = &(*property); break;

        case CTF_ALLBORDERWIDTH:		pAllBorderWidth = &(*property); break;
        case CTF_LEFTBORDERWIDTH:		pBorderWidths[XML_LINE_LEFT] = &(*property); break;
        case CTF_RIGHTBORDERWIDTH:		pBorderWidths[XML_LINE_RIGHT] = &(*property); break;
        case CTF_TOPBORDERWIDTH:		pBorderWidths[XML_LINE_TOP] = &(*property); break;
        case CTF_BOTTOMBORDERWIDTH:		pBorderWidths[XML_LINE_BOTTOM] = &(*property); break;
        case CTF_ANCHORTYPE:			pAnchorType = &(*property); break;
        case CTF_VERTICALPOS:  			pVertOrient = &(*&(*property)); break;
        case CTF_VERTICALREL_ASCHAR: 	pVertOrientRelAsChar = &(*property); break;

        case CTF_FRAMEHEIGHT_MIN_ABS:
        case CTF_FRAMEHEIGHT_MIN_REL:
//		case CTF_SYNCHEIGHT_MIN:
                                        bHasAnyMinHeight = sal_True;
                                        // no break here!
        case CTF_FRAMEHEIGHT_ABS:
        case CTF_FRAMEHEIGHT_REL:
//		case CTF_SYNCHEIGHT:
                                        bHasAnyHeight = sal_True; break;
        case CTF_BACKGROUND_TRANSPARENCY: pBackTransparency = &(*property); break;
        case CTF_BACKGROUND_TRANSPARENT:  pBackTransparent = &(*property); break;

        }
    }

    if( pFontFamilyName || pFontStyleName || pFontFamily ||
        pFontPitch || pFontCharSet )
        FontFinished( pFontFamilyName, pFontStyleName, pFontFamily,
                      pFontPitch, pFontCharSet );
    if( pFontFamilyNameCJK || pFontStyleNameCJK || pFontFamilyCJK ||
        pFontPitchCJK || pFontCharSetCJK )
        FontFinished( pFontFamilyNameCJK, pFontStyleNameCJK, pFontFamilyCJK,
                      pFontPitchCJK, pFontCharSetCJK );
    if( pFontFamilyNameCTL || pFontStyleNameCTL || pFontFamilyCTL ||
        pFontPitchCTL || pFontCharSetCTL )
        FontFinished( pFontFamilyNameCTL, pFontStyleNameCTL, pFontFamilyCTL,
                      pFontPitchCTL, pFontCharSetCTL );

    sal_uInt16 i;

    for( i=0; i<4; i++ )
    {
        if( pAllBorderDistance && !pBorderDistances[i] )
        {
#ifdef DBG_UTIL
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
#ifdef DBG_UTIL
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
#ifdef DBG_UTIL
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

    if( pVertOrient && pVertOrientRelAsChar )
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
        pVertOrient->maValue <<= nVertOrient;
        pVertOrientRelAsChar->mnIndex = -1;
    }

    FontDefaultsCheck( pFontFamilyName,
                       pFontStyleName, pFontFamily, pFontPitch, pFontCharSet,
                       &pNewFontStyleName, &pNewFontFamily, &pNewFontPitch, &pNewFontCharSet );

    FontDefaultsCheck( pFontFamilyNameCJK,
                       pFontStyleNameCJK, pFontFamilyCJK, pFontPitchCJK, pFontCharSetCJK,
                       &pNewFontStyleNameCJK, &pNewFontFamilyCJK, &pNewFontPitchCJK, &pNewFontCharSetCJK );

    FontDefaultsCheck( pFontFamilyNameCTL,
                       pFontStyleNameCTL, pFontFamilyCTL, pFontPitchCTL, pFontCharSetCTL,
                       &pNewFontStyleNameCTL, &pNewFontFamilyCTL, &pNewFontPitchCTL, &pNewFontCharSetCTL );

    // #i5775# don't overwrite %transparency with binary transparency
    if( ( pBackTransparency != NULL ) && ( pBackTransparent != NULL ) )
    {
        if( ! *(sal_Bool*)(pBackTransparent->maValue.getValue()) )
            pBackTransparent->mnIndex = -1;
    }


    // insert newly created properties. This inavlidates all iterators!
    // Most of the pXXX variables in this method are iterators and will be 
    // invalidated!!!

    if( pNewFontStyleName )
    {
        rProperties.push_back( *pNewFontStyleName );
        delete pNewFontStyleName;
    }

    if( pNewFontFamily )
    {
        rProperties.push_back( *pNewFontFamily );
        delete pNewFontFamily;
    }

    if( pNewFontPitch )
    {
        rProperties.push_back( *pNewFontPitch );
        delete pNewFontPitch;
    }

    if( pNewFontCharSet )
    {
        rProperties.push_back( *pNewFontCharSet );
        delete pNewFontCharSet;
    }

    if( pNewFontStyleNameCJK )
    {
        rProperties.push_back( *pNewFontStyleNameCJK );
        delete pNewFontStyleNameCJK;
    }

    if( pNewFontFamilyCJK )
    {
        rProperties.push_back( *pNewFontFamilyCJK );
        delete pNewFontFamilyCJK;
    }

    if( pNewFontPitchCJK )
    {
        rProperties.push_back( *pNewFontPitchCJK );
        delete pNewFontPitchCJK;
    }

    if( pNewFontCharSetCJK )
    {
        rProperties.push_back( *pNewFontCharSetCJK );
        delete pNewFontCharSetCJK;
    }

    if( pNewFontStyleNameCTL)
    {
        rProperties.push_back( *pNewFontStyleNameCTL );
        delete pNewFontStyleNameCTL;
    }

    if( pNewFontFamilyCTL )
    {
        rProperties.push_back( *pNewFontFamilyCTL );
        delete pNewFontFamilyCTL;
    }

    if( pNewFontPitchCTL )
    {
        rProperties.push_back( *pNewFontPitchCTL );
        delete pNewFontPitchCTL;
    }

    if( pNewFontCharSetCTL )
    {
        rProperties.push_back( *pNewFontCharSetCTL );
        delete pNewFontCharSetCTL;
    }

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

    if( bHasAnyHeight )
    {
        if( nSizeTypeIndex == -2 )
        {
            const_cast < XMLTextImportPropertyMapper * > ( this )
                ->nSizeTypeIndex  = -1;
            sal_Int32 nCount = getPropertySetMapper()->GetEntryCount();
            for( sal_Int32 i=0; i < nCount; i++ )
            {
                if( CTF_SIZETYPE == getPropertySetMapper()
                        ->GetEntryContextId( i ) )
                {
                    const_cast < XMLTextImportPropertyMapper * > ( this )
                        ->nSizeTypeIndex = i;
                    break;
                }
            }
        }
        if( nSizeTypeIndex != -1 )
        {
            XMLPropertyState aSizeTypeState( nSizeTypeIndex );
            aSizeTypeState.maValue <<= (sal_Int16)( bHasAnyMinHeight
                                                        ? SizeType::MIN
                                                        : SizeType::FIX);
            rProperties.push_back( aSizeTypeState );
        }
    }

    // DO NOT USE ITERATORS/POINTERS INTO THE rProperties-VECTOR AFTER
    // THIS LINE.  All iterators into the rProperties-vector, especially all
    // pXXX-type variables set in the first switch statement of this method, 
    // may have been invalidated by the above push_back() calls!
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
