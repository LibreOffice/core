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

#include <osl/thread.h>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimppr.hxx>
#include <xmloff/maptype.hxx>

#define XML_LINE_LEFT 0
#define XML_LINE_RIGHT 1
#define XML_LINE_TOP 2
#define XML_LINE_BOTTOM 3


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;

bool XMLTextImportPropertyMapper::handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const
{
    bool bRet = false;
    sal_Int32 nIndex = rProperty.mnIndex;
    switch( getPropertySetMapper()->GetEntryContextId( nIndex  ) )
    {
    case CTF_FONTNAME:
    case CTF_FONTNAME_CJK:
    case CTF_FONTNAME_CTL:
        if( GetImport().GetFontDecls() != nullptr )
        {
            assert((
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
                    getPropertySetMapper()->GetEntryContextId(nIndex+5) )
              ) && "illegal property map" );

            GetImport().GetFontDecls()->FillProperties(
                            rValue, rProperties,
                            rProperty.mnIndex+1, rProperty.mnIndex+2,
                            rProperty.mnIndex+3, rProperty.mnIndex+4,
                            rProperty.mnIndex+5 );
            bRet = false; // the property hasn't been filled
        }
        break;

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

    case CTF_TEXT_DISPLAY:
        bRet = getPropertySetMapper()->importXML( rValue, rProperty,
                                                  rUnitConverter );
        if( SvXMLImport::OOo_2x == GetImport().getGeneratorVersion() )
        {
            bool bHidden = false;
            rProperty.maValue >>= bHidden;
            bHidden = !bHidden;
            rProperty.maValue <<= bHidden;
        }
    break;
    default:
        bRet = SvXMLImportPropertyMapper::handleSpecialItem( rProperty,
                    rProperties, rValue, rUnitConverter, rNamespaceMap );
        break;
    }

    return bRet;
}

XMLTextImportPropertyMapper::XMLTextImportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImp ) :
    SvXMLImportPropertyMapper( rMapper, rImp ),
    nSizeTypeIndex( -2 ),
    nWidthTypeIndex( -2 )
{
}

XMLTextImportPropertyMapper::~XMLTextImportPropertyMapper()
{
}

void XMLTextImportPropertyMapper::FontFinished(
    XMLPropertyState *pFontFamilyNameState,
    XMLPropertyState *pFontStyleNameState,
    XMLPropertyState *pFontFamilyState,
    XMLPropertyState *pFontPitchState,
    XMLPropertyState *pFontCharsetState )
{
    if( pFontFamilyNameState && pFontFamilyNameState->mnIndex != -1 )
    {
        OUString sName;
        pFontFamilyNameState->maValue >>= sName;
        if( sName.isEmpty() )
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
        Any aAny;

        if( !pFontStyleName )
        {
            aAny <<= OUString();
    #if OSL_DEBUG_LEVEL > 0
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                                pFontFamilyName->mnIndex + 1 );
                assert(nTmp == CTF_FONTSTYLENAME || nTmp == CTF_FONTSTYLENAME_CJK || nTmp == CTF_FONTSTYLENAME_CTL);
    #endif
                *ppNewFontStyleName = new XMLPropertyState( pFontFamilyName->mnIndex + 1,
                                                       aAny );
        }

        if( !pFontFamily )
        {
            aAny <<= (sal_Int16)com::sun::star::awt::FontFamily::DONTKNOW;

    #if OSL_DEBUG_LEVEL > 0
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                                pFontFamilyName->mnIndex + 2 );
                assert(nTmp == CTF_FONTFAMILY || nTmp == CTF_FONTFAMILY_CJK || nTmp == CTF_FONTFAMILY_CTL);
    #endif
                *ppNewFontFamily = new XMLPropertyState( pFontFamilyName->mnIndex + 2,
                                                       aAny );
        }

        if( !pFontPitch )
        {
            aAny <<= (sal_Int16)com::sun::star::awt::FontPitch::DONTKNOW;
    #if OSL_DEBUG_LEVEL > 0
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                                pFontFamilyName->mnIndex + 3 );
                assert(nTmp == CTF_FONTPITCH || nTmp == CTF_FONTPITCH_CJK || nTmp == CTF_FONTPITCH_CTL);
    #endif
                *ppNewFontPitch = new XMLPropertyState( pFontFamilyName->mnIndex + 3,
                                                       aAny );
        }

        if( !pFontCharSet )
        {
            aAny <<= (sal_Int16)osl_getThreadTextEncoding();
    #if OSL_DEBUG_LEVEL > 0
                sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                                pFontFamilyName->mnIndex + 4 );
                assert(nTmp == CTF_FONTCHARSET || nTmp == CTF_FONTCHARSET_CJK || nTmp == CTF_FONTCHARSET_CTL);
    #endif
                *ppNewFontCharSet = new XMLPropertyState( pFontFamilyName->mnIndex + 4,
                                                       aAny );
        }
    }

    (void) this; // avoid loplugin:staticmethods
}

namespace {
//fdo#58730 The [UL|LR]Space class has a deficiency where "100%" also serves as
//a flag that the value is an absolute value so we can't truly handle an
//up/lower space property which wants to specify its 200% upper but 100% lower
//of its parent (try typing 100% vs 200% into the edit style dialog and revisit
//your style). So on xml load that ends up meaning 200%, 0 lower. This is a
//crock.

//On import clear 100% all-margins relative sizes.
static bool
isNotDefaultRelSize(const XMLPropertyState* pRelState, const rtl::Reference<XMLPropertySetMapper>& rPrMap)
{
    if (rPrMap->GetEntryContextId(pRelState->mnIndex) == CTF_PARAMARGINALL_REL)
    {
        sal_Int32 nTemp = 0;
        pRelState->maValue >>= nTemp;
        return nTemp != 100;
    }
    return true;
}

/**
 * Separate compressed border attributes.
 * During export, border attributes are compressed if there are equal to all four side.
 * Since Writer hasn't the same compressed attributes, but has distinct ones for all
 * four side, we have to duplicate the compressed attribute during import.
**/
static void lcl_SeparateBorder(
    sal_uInt16 nIndex, XMLPropertyState* pAllBorderDistance,
    XMLPropertyState* pBorderDistances[4], XMLPropertyState* pNewBorderDistances[4],
    XMLPropertyState* pAllBorder, XMLPropertyState* pBorders[4],
    XMLPropertyState* pNewBorders[4], XMLPropertyState* pAllBorderWidth,
    XMLPropertyState* pBorderWidths[4]
#if OSL_DEBUG_LEVEL > 0
    , const rtl::Reference< XMLPropertySetMapper >& rMapper
#endif
)
{
    if( pAllBorderDistance && !pBorderDistances[nIndex] )
    {
#if OSL_DEBUG_LEVEL > 0
        sal_Int16 nTmp = rMapper->GetEntryContextId(
                                    pAllBorderDistance->mnIndex + nIndex + 1 );
        if (CTF_CHARALLBORDERDISTANCE ==
                rMapper->GetEntryContextId(pAllBorderDistance->mnIndex))
        {
            assert(nTmp >= CTF_CHARLEFTBORDERDISTANCE &&
                   nTmp <= CTF_CHARBOTTOMBORDERDISTANCE);
        }
        else
        {
            assert(nTmp >= CTF_LEFTBORDERDISTANCE &&
                   nTmp <= CTF_BOTTOMBORDERDISTANCE);
        }
#endif
        pNewBorderDistances[nIndex] =
            new XMLPropertyState( pAllBorderDistance->mnIndex + nIndex + 1,
                                    pAllBorderDistance->maValue );
        pBorderDistances[nIndex] = pNewBorderDistances[nIndex];
    }
    if( pAllBorder && !pBorders[nIndex] )
    {
#if OSL_DEBUG_LEVEL > 0
        sal_Int16 nTmp = rMapper->GetEntryContextId(
                                        pAllBorder->mnIndex + nIndex + 1 );
        if (CTF_CHARALLBORDER ==
                rMapper->GetEntryContextId(pAllBorder->mnIndex))
        {
            assert(nTmp >= CTF_CHARLEFTBORDER && nTmp <= CTF_CHARBOTTOMBORDER);
        }
        else
        {
            assert(nTmp >= CTF_LEFTBORDER && nTmp <= CTF_BOTTOMBORDER);
        }
#endif
        pNewBorders[nIndex] = new XMLPropertyState( pAllBorder->mnIndex + nIndex + 1,
                                                   pAllBorder->maValue );
        pBorders[nIndex] = pNewBorders[nIndex];
    }
    if( !pBorderWidths[nIndex] )
        pBorderWidths[nIndex] = pAllBorderWidth;
    else
        pBorderWidths[nIndex]->mnIndex = -1;

    if( pBorders[nIndex] && pBorderWidths[nIndex] )
    {
        table::BorderLine2 aBorderLine;
        pBorders[nIndex]->maValue >>= aBorderLine;

        table::BorderLine2 aBorderLineWidth;
        pBorderWidths[nIndex]->maValue >>= aBorderLineWidth;

        aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
        aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
        aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
        aBorderLine.LineWidth = aBorderLineWidth.LineWidth;

        pBorders[nIndex]->maValue <<= aBorderLine;
    }
}

}

void XMLTextImportPropertyMapper::finished(
            ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 /*nStartIndex*/, sal_Int32 /*nEndIndex*/ ) const
{
    bool bHasAnyHeight = false;
    bool bHasAnyMinHeight = false;
    bool bHasAnyWidth = false;
    bool bHasAnyMinWidth = false;

    XMLPropertyState* pFontFamilyName = nullptr;
    XMLPropertyState* pFontStyleName = nullptr;
    XMLPropertyState* pFontFamily = nullptr;
    XMLPropertyState* pFontPitch = nullptr;
    XMLPropertyState* pFontCharSet = nullptr;
    XMLPropertyState* pNewFontStyleName = nullptr;
    XMLPropertyState* pNewFontFamily = nullptr;
    XMLPropertyState* pNewFontPitch = nullptr;
    XMLPropertyState* pNewFontCharSet = nullptr;
    XMLPropertyState* pFontFamilyNameCJK = nullptr;
    XMLPropertyState* pFontStyleNameCJK = nullptr;
    XMLPropertyState* pFontFamilyCJK = nullptr;
    XMLPropertyState* pFontPitchCJK = nullptr;
    XMLPropertyState* pFontCharSetCJK = nullptr;
    XMLPropertyState* pNewFontStyleNameCJK = nullptr;
    XMLPropertyState* pNewFontFamilyCJK = nullptr;
    XMLPropertyState* pNewFontPitchCJK = nullptr;
    XMLPropertyState* pNewFontCharSetCJK = nullptr;
    XMLPropertyState* pFontFamilyNameCTL = nullptr;
    XMLPropertyState* pFontStyleNameCTL = nullptr;
    XMLPropertyState* pFontFamilyCTL = nullptr;
    XMLPropertyState* pFontPitchCTL = nullptr;
    XMLPropertyState* pFontCharSetCTL = nullptr;
    XMLPropertyState* pNewFontStyleNameCTL = nullptr;
    XMLPropertyState* pNewFontFamilyCTL = nullptr;
    XMLPropertyState* pNewFontPitchCTL = nullptr;
    XMLPropertyState* pNewFontCharSetCTL = nullptr;
    XMLPropertyState* pAllBorderDistance = nullptr;
    XMLPropertyState* pBorderDistances[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pNewBorderDistances[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllBorder = nullptr;
    XMLPropertyState* pBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pNewBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllBorderWidth = nullptr;
    XMLPropertyState* pBorderWidths[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pCharAllBorderDistance = nullptr;
    XMLPropertyState* pCharBorderDistances[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pCharNewBorderDistances[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pCharAllBorder = nullptr;
    XMLPropertyState* pCharBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pCharNewBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pCharAllBorderWidth = nullptr;
    XMLPropertyState* pCharBorderWidths[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pVertOrient = nullptr;
    XMLPropertyState* pVertOrientRelAsChar = nullptr;
    XMLPropertyState* pBackTransparency = nullptr; // transparency in %
    XMLPropertyState* pBackTransparent = nullptr;  // transparency as boolean
    XMLPropertyState* pAllParaMargin = nullptr;
    XMLPropertyState* pParaMargins[4] = { nullptr, nullptr, nullptr, nullptr };
    ::std::unique_ptr<XMLPropertyState> pNewParaMargins[4];
    XMLPropertyState* pAllMargin = nullptr;
    XMLPropertyState* pMargins[4] = { nullptr, nullptr, nullptr, nullptr };
    ::std::unique_ptr<XMLPropertyState> pNewMargins[4];
    XMLPropertyState* pFillStyle(nullptr);
    XMLPropertyState* pFillColor(nullptr);

    for( ::std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
         aIter != rProperties.end();
         ++aIter )
    {
        XMLPropertyState* property = &(*aIter);
        if( -1 == property->mnIndex )
            continue;

        switch( getPropertySetMapper()->GetEntryContextId( property->mnIndex ) )
        {
        case CTF_FONTFAMILYNAME:    pFontFamilyName = property; break;
        case CTF_FONTSTYLENAME: pFontStyleName = property;  break;
        case CTF_FONTFAMILY:    pFontFamily = property; break;
        case CTF_FONTPITCH: pFontPitch = property;  break;
        case CTF_FONTCHARSET:   pFontCharSet = property;    break;

        case CTF_FONTFAMILYNAME_CJK:    pFontFamilyNameCJK = property;  break;
        case CTF_FONTSTYLENAME_CJK: pFontStyleNameCJK = property;   break;
        case CTF_FONTFAMILY_CJK:    pFontFamilyCJK = property;  break;
        case CTF_FONTPITCH_CJK: pFontPitchCJK = property;   break;
        case CTF_FONTCHARSET_CJK:   pFontCharSetCJK = property; break;

        case CTF_FONTFAMILYNAME_CTL:    pFontFamilyNameCTL = property;  break;
        case CTF_FONTSTYLENAME_CTL: pFontStyleNameCTL = property;   break;
        case CTF_FONTFAMILY_CTL:    pFontFamilyCTL = property;  break;
        case CTF_FONTPITCH_CTL: pFontPitchCTL = property;   break;
        case CTF_FONTCHARSET_CTL:   pFontCharSetCTL = property; break;

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

        case CTF_CHARALLBORDERDISTANCE:     pCharAllBorderDistance = property; break;
        case CTF_CHARLEFTBORDERDISTANCE:    pCharBorderDistances[XML_LINE_LEFT] = property; break;
        case CTF_CHARRIGHTBORDERDISTANCE:   pCharBorderDistances[XML_LINE_RIGHT] = property; break;
        case CTF_CHARTOPBORDERDISTANCE:     pCharBorderDistances[XML_LINE_TOP] = property; break;
        case CTF_CHARBOTTOMBORDERDISTANCE:  pCharBorderDistances[XML_LINE_BOTTOM] = property; break;
        case CTF_CHARALLBORDER:             pCharAllBorder = property; break;
        case CTF_CHARLEFTBORDER:            pCharBorders[XML_LINE_LEFT] = property; break;
        case CTF_CHARRIGHTBORDER:           pCharBorders[XML_LINE_RIGHT] = property; break;
        case CTF_CHARTOPBORDER:             pCharBorders[XML_LINE_TOP] = property; break;
        case CTF_CHARBOTTOMBORDER:          pCharBorders[XML_LINE_BOTTOM] = property; break;
        case CTF_CHARALLBORDERWIDTH:        pCharAllBorderWidth = property; break;
        case CTF_CHARLEFTBORDERWIDTH:       pCharBorderWidths[XML_LINE_LEFT] = property; break;
        case CTF_CHARRIGHTBORDERWIDTH:      pCharBorderWidths[XML_LINE_RIGHT] = property; break;
        case CTF_CHARTOPBORDERWIDTH:        pCharBorderWidths[XML_LINE_TOP] = property; break;
        case CTF_CHARBOTTOMBORDERWIDTH:     pCharBorderWidths[XML_LINE_BOTTOM] = property; break;

        case CTF_ANCHORTYPE:            break;
        case CTF_VERTICALPOS:           pVertOrient = property; break;
        case CTF_VERTICALREL_ASCHAR:    pVertOrientRelAsChar = property; break;

        case CTF_FRAMEHEIGHT_MIN_ABS:
        case CTF_FRAMEHEIGHT_MIN_REL:
//      case CTF_SYNCHEIGHT_MIN:
                                        bHasAnyMinHeight = true;
                                        SAL_FALLTHROUGH;
        case CTF_FRAMEHEIGHT_ABS:
        case CTF_FRAMEHEIGHT_REL:
//      case CTF_SYNCHEIGHT:
                                        bHasAnyHeight = true; break;
        case CTF_FRAMEWIDTH_MIN_ABS:
        case CTF_FRAMEWIDTH_MIN_REL:
                                        bHasAnyMinWidth = true;
                                        SAL_FALLTHROUGH;
        case CTF_FRAMEWIDTH_ABS:
        case CTF_FRAMEWIDTH_REL:
                                        bHasAnyWidth = true; break;
        case CTF_BACKGROUND_TRANSPARENCY: pBackTransparency = property; break;
        case CTF_BACKGROUND_TRANSPARENT:  pBackTransparent = property; break;
        case CTF_FILLSTYLE:             pFillStyle = property; break;
        case CTF_FILLCOLOR:             pFillColor = property; break;
        case CTF_PARAMARGINALL:
        case CTF_PARAMARGINALL_REL:
                pAllParaMargin = property; break;
        case CTF_PARALEFTMARGIN:
        case CTF_PARALEFTMARGIN_REL:
                pParaMargins[XML_LINE_LEFT] = property; break;
        case CTF_PARARIGHTMARGIN:
        case CTF_PARARIGHTMARGIN_REL:
                pParaMargins[XML_LINE_RIGHT] = property; break;
        case CTF_PARATOPMARGIN:
        case CTF_PARATOPMARGIN_REL:
                pParaMargins[XML_LINE_TOP] = property; break;
        case CTF_PARABOTTOMMARGIN:
        case CTF_PARABOTTOMMARGIN_REL:
                pParaMargins[XML_LINE_BOTTOM] = property; break;
        case CTF_MARGINALL:
                pAllMargin = property; break;
        case CTF_MARGINLEFT:
                pMargins[XML_LINE_LEFT] = property; break;
        case CTF_MARGINRIGHT:
                pMargins[XML_LINE_RIGHT] = property; break;
        case CTF_MARGINTOP:
                pMargins[XML_LINE_TOP] = property; break;
        case CTF_MARGINBOTTOM:
                pMargins[XML_LINE_BOTTOM] = property; break;
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

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pAllParaMargin && !pParaMargins[i]
            && isNotDefaultRelSize(pAllParaMargin, getPropertySetMapper()))
        {
#if OSL_DEBUG_LEVEL > 0
            sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                        pAllParaMargin->mnIndex + (2*i) + 2 );
            assert(nTmp >= CTF_PARALEFTMARGIN &&
                   nTmp <= CTF_PARABOTTOMMARGIN_REL);
#endif
            pNewParaMargins[i].reset(new XMLPropertyState(
                pAllParaMargin->mnIndex + (2*i) + 2, pAllParaMargin->maValue));
        }
        if (pAllMargin && !pMargins[i])
        {
#if OSL_DEBUG_LEVEL > 0
            sal_Int16 nTmp = getPropertySetMapper()->GetEntryContextId(
                                        pAllMargin->mnIndex + i + 1 );
            assert(nTmp >= CTF_MARGINLEFT && nTmp <= CTF_MARGINBOTTOM);
#endif
            pNewMargins[i].reset(new XMLPropertyState(
                pAllMargin->mnIndex + i + 1, pAllMargin->maValue));
        }

        lcl_SeparateBorder(
            i, pAllBorderDistance, pBorderDistances, pNewBorderDistances,
            pAllBorder, pBorders, pNewBorders,
            pAllBorderWidth, pBorderWidths
#if OSL_DEBUG_LEVEL > 0
            , getPropertySetMapper()
#endif
            );

        lcl_SeparateBorder(
            i, pCharAllBorderDistance, pCharBorderDistances,
            pCharNewBorderDistances, pCharAllBorder, pCharBorders,
            pCharNewBorders, pCharAllBorderWidth, pCharBorderWidths
#if OSL_DEBUG_LEVEL > 0
            , getPropertySetMapper()
#endif
            );
    }

    if (pAllParaMargin)
    {
        pAllParaMargin->mnIndex = -1;
    }
    if (pAllMargin)
    {
        pAllMargin->mnIndex = -1;
    }

    if( pAllBorderDistance )
        pAllBorderDistance->mnIndex = -1;

    if( pAllBorder )
        pAllBorder->mnIndex = -1;

    if( pAllBorderWidth )
        pAllBorderWidth->mnIndex = -1;

    if( pCharAllBorderDistance )
        pCharAllBorderDistance->mnIndex = -1;

    if( pCharAllBorder )
        pCharAllBorder->mnIndex = -1;

    if( pCharAllBorderWidth )
        pCharAllBorderWidth->mnIndex = -1;

    if( pVertOrient && pVertOrientRelAsChar )
    {
        sal_Int16 nVertOrient;
        pVertOrient->maValue >>= nVertOrient;
        sal_Int16 nVertOrientRel = 0;
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

    if (pFillStyle && !pFillColor && pBackTransparent
        && drawing::FillStyle_SOLID == pFillStyle->maValue.get<drawing::FillStyle>()
        && pBackTransparent->maValue.get<bool>())
    {
        // fo:background="transparent", draw:fill="solid" without draw:fill-color
        // prevent getSvxBrushItemFromSourceSet from adding bogus default color
        pFillStyle->mnIndex = -1;
    }

    // #i5775# don't overwrite %transparency with binary transparency
    if( ( pBackTransparency != nullptr ) && ( pBackTransparent != nullptr ) )
    {
        if( ! *static_cast<sal_Bool const *>(pBackTransparent->maValue.getValue()) )
            pBackTransparent->mnIndex = -1;
    }


    // insert newly created properties. This invalidates all iterators!
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

    for (sal_uInt16 i=0; i<4; i++)
    {
        if (pNewParaMargins[i].get())
        {
            rProperties.push_back(*pNewParaMargins[i]);
        }
        if (pNewMargins[i].get())
        {
            rProperties.push_back(*pNewMargins[i]);
        }
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
        if( pCharNewBorderDistances[i] )
        {
            rProperties.push_back( *pCharNewBorderDistances[i] );
            delete pCharNewBorderDistances[i];
        }
        if( pCharNewBorders[i] )
        {
            rProperties.push_back( *pCharNewBorders[i] );
            delete pCharNewBorders[i];
        }
    }

    if( bHasAnyHeight )
    {
        if( nSizeTypeIndex == -2 )
        {
            const_cast < XMLTextImportPropertyMapper * > ( this )
                ->nSizeTypeIndex  = -1;
            sal_Int32 nPropCount = getPropertySetMapper()->GetEntryCount();
            for( sal_Int32 j=0; j < nPropCount; j++ )
            {
                if( CTF_SIZETYPE == getPropertySetMapper()
                        ->GetEntryContextId( j ) )
                {
                    const_cast < XMLTextImportPropertyMapper * > ( this )
                        ->nSizeTypeIndex = j;
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

    if( bHasAnyWidth )
    {
        if( nWidthTypeIndex == -2 )
        {
            const_cast < XMLTextImportPropertyMapper * > ( this )
                ->nWidthTypeIndex  = -1;
            sal_Int32 nCount = getPropertySetMapper()->GetEntryCount();
            for( sal_Int32 j=0; j < nCount; j++ )
            {
                if( CTF_FRAMEWIDTH_TYPE  == getPropertySetMapper()
                        ->GetEntryContextId( j ) )
                {
                    const_cast < XMLTextImportPropertyMapper * > ( this )
                        ->nWidthTypeIndex = j;
                    break;
                }
            }
        }
        if( nWidthTypeIndex != -1 )
        {
            XMLPropertyState aSizeTypeState( nWidthTypeIndex );
            aSizeTypeState.maValue <<= (sal_Int16)( bHasAnyMinWidth
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
