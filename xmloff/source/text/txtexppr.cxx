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

#include <com/sun/star/table/BorderLine2.hpp>

#include "txtexppr.hxx"

#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>

#include <tools/debug.hxx>

#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlexp.hxx>
#include "XMLSectionFootnoteConfigExport.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;

void XMLTextExportPropertySetMapper::handleElementItem(
        SvXMLExport& rExp,
        const XMLPropertyState& rProperty,
        sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    XMLTextExportPropertySetMapper *pThis =
           ((XMLTextExportPropertySetMapper *)this);

    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
    case CTF_DROPCAPFORMAT:
        pThis->maDropCapExport.exportXML( rProperty.maValue, bDropWholeWord,
                                          sDropCharStyle );
        pThis->bDropWholeWord = sal_False;
        pThis->sDropCharStyle = OUString();
        break;

    case CTF_TABSTOP:
        pThis->maTabStopExport.Export( rProperty.maValue );
        break;

    case CTF_TEXTCOLUMNS:
        pThis->maTextColumnsExport.exportXML( rProperty.maValue );
        break;

    case CTF_BACKGROUND_URL:
        {
            DBG_ASSERT( pProperties && nIdx >= 3,
                        "property vector missing" );
            const Any *pPos = 0, *pFilter = 0, *pTrans = 0;
            if( pProperties && nIdx >= 3 )
            {
                const XMLPropertyState& rTrans = (*pProperties)[nIdx-3];
                // #99657# transparency may be there, but doesn't have to be.
                // If it's there, it must be in the right position.
                if( CTF_BACKGROUND_TRANSPARENCY == getPropertySetMapper()
                      ->GetEntryContextId( rTrans.mnIndex ) )
                    pTrans = &rTrans.maValue;

                const XMLPropertyState& rPos = (*pProperties)[nIdx-2];
                DBG_ASSERT( CTF_BACKGROUND_POS == getPropertySetMapper()
                        ->GetEntryContextId( rPos.mnIndex ),
                         "invalid property map: pos expected" );
                if( CTF_BACKGROUND_POS == getPropertySetMapper()
                        ->GetEntryContextId( rPos.mnIndex ) )
                    pPos = &rPos.maValue;

                const XMLPropertyState& rFilter = (*pProperties)[nIdx-1];
                DBG_ASSERT( CTF_BACKGROUND_FILTER == getPropertySetMapper()
                        ->GetEntryContextId( rFilter.mnIndex ),
                         "invalid property map: filter expected" );
                if( CTF_BACKGROUND_FILTER == getPropertySetMapper()
                        ->GetEntryContextId( rFilter.mnIndex ) )
                    pFilter = &rFilter.maValue;
            }
            sal_uInt32 nPropIndex = rProperty.mnIndex;
            pThis->maBackgroundImageExport.exportXML(
                    rProperty.maValue, pPos, pFilter, pTrans,
                    getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                    getPropertySetMapper()->GetEntryXMLName( nPropIndex ) );
        }
        break;

    case CTF_SECTION_FOOTNOTE_END:
        XMLSectionFootnoteConfigExport::exportXML(rExp, sal_False,
                                                  pProperties, nIdx,
                                                  getPropertySetMapper());
        break;

    case CTF_SECTION_ENDNOTE_END:
        XMLSectionFootnoteConfigExport::exportXML(rExp, sal_True,
                                                  pProperties, nIdx,
                                                  getPropertySetMapper());
        break;

    default:
        SvXMLExportPropertyMapper::handleElementItem( rExp, rProperty, nFlags, pProperties, nIdx );
        break;
    }
}

void XMLTextExportPropertySetMapper::handleSpecialItem(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    XMLTextExportPropertySetMapper *pThis =
           ((XMLTextExportPropertySetMapper *)this);

    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
    case CTF_DROPCAPWHOLEWORD:
        DBG_ASSERT( !bDropWholeWord, "drop whole word is set already!" );
        pThis->bDropWholeWord = *(sal_Bool *)rProperty.maValue.getValue();
        break;
    case CTF_DROPCAPCHARSTYLE:
        DBG_ASSERT( sDropCharStyle.isEmpty(), "drop char style is set already!" );
        rProperty.maValue >>= pThis->sDropCharStyle;
        break;
    case CTF_NUMBERINGSTYLENAME:
    case CTF_PAGEDESCNAME:
    case CTF_OLDTEXTBACKGROUND:
    case CTF_BACKGROUND_POS:
    case CTF_BACKGROUND_FILTER:
    case CTF_BACKGROUND_TRANSPARENCY:
    case CTF_SECTION_FOOTNOTE_NUM_OWN:
    case CTF_SECTION_FOOTNOTE_NUM_RESTART:
    case CTF_SECTION_FOOTNOTE_NUM_RESTART_AT:
    case CTF_SECTION_FOOTNOTE_NUM_TYPE:
    case CTF_SECTION_FOOTNOTE_NUM_PREFIX:
    case CTF_SECTION_FOOTNOTE_NUM_SUFFIX:
    case CTF_SECTION_ENDNOTE_NUM_OWN:
    case CTF_SECTION_ENDNOTE_NUM_RESTART:
    case CTF_SECTION_ENDNOTE_NUM_RESTART_AT:
    case CTF_SECTION_ENDNOTE_NUM_TYPE:
    case CTF_SECTION_ENDNOTE_NUM_PREFIX:
    case CTF_SECTION_ENDNOTE_NUM_SUFFIX:
    case CTF_DEFAULT_OUTLINE_LEVEL:
    case CTF_OLD_FLOW_WITH_TEXT:
        // There's nothing to do here!
        break;
    default:
        SvXMLExportPropertyMapper::handleSpecialItem(rAttrList, rProperty, rUnitConverter, rNamespaceMap, pProperties, nIdx );
        break;
    }
}

XMLTextExportPropertySetMapper::XMLTextExportPropertySetMapper(
        const UniReference< XMLPropertySetMapper >& rMapper,
        SvXMLExport& rExp ) :
    SvXMLExportPropertyMapper( rMapper ),
    rExport( rExp ),
    bDropWholeWord( sal_False ),
    maDropCapExport( rExp ),
    maTabStopExport( rExp ),
    maTextColumnsExport( rExp ),
    maBackgroundImageExport( rExp )
{
}

XMLTextExportPropertySetMapper::~XMLTextExportPropertySetMapper()
{
}

void XMLTextExportPropertySetMapper::ContextFontFilter(
    bool bEnableFoFontFamily,
    XMLPropertyState *pFontNameState,
    XMLPropertyState *pFontFamilyNameState,
    XMLPropertyState *pFontStyleNameState,
    XMLPropertyState *pFontFamilyState,
    XMLPropertyState *pFontPitchState,
    XMLPropertyState *pFontCharsetState ) const
{
    OUString sFamilyName;
    OUString sStyleName;
    FontFamily nFamily = FAMILY_DONTKNOW;
    FontPitch nPitch = PITCH_DONTKNOW;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    OUString sTmp;
    if( pFontFamilyNameState && (pFontFamilyNameState->maValue >>= sTmp ) )
        sFamilyName = sTmp;
    if( pFontStyleNameState && (pFontStyleNameState->maValue >>= sTmp ) )
        sStyleName = sTmp;

    sal_Int16 nTmp = sal_Int16();
    if( pFontFamilyState && (pFontFamilyState->maValue >>= nTmp ) )
        nFamily = static_cast< FontFamily >( nTmp );
    if( pFontPitchState && (pFontPitchState->maValue >>= nTmp ) )
        nPitch = static_cast< FontPitch >( nTmp );
    if( pFontCharsetState && (pFontCharsetState->maValue >>= nTmp ) )
        eEnc = (rtl_TextEncoding)nTmp;

    //Resolves: fdo#67665 The purpose here appears to be to replace
    //FontFamilyName and FontStyleName etc with a single FontName property. The
    //problem is that repeated calls to here will first set
    //pFontFamilyNameState->mnIndex to -1 to indicate it is disabled, so the
    //next time pFontFamilyNameState is not passed here at all, which gives an
    //empty sFamilyName resulting in disabling pFontNameState->mnIndex to -1.
    //That doesn't seem right to me.
    //
    //So assuming that the main purpose is just to convert the properties in
    //the main when we can, and to leave them alone when we can't. And with a
    //secondary purpose to filter out empty font properties, then is would
    //appear to make sense to base attempting the conversion if we have
    //both of the major facts of the font description
    //
    //An alternative solution is to *not* fill the FontAutoStylePool with
    //every font in the document, but to partition the fonts into the
    //hard-attribute fonts which go into that pool and the style-attribute
    //fonts which go into some additional pool which get merged just for
    //the purposes of writing the embedded fonts but are not queried by
    //"Find" which restores the original logic.
    if (pFontFamilyNameState || pFontStyleNameState)
    {
        OUString sName( ((SvXMLExport&)GetExport()).GetFontAutoStylePool()->Find(
                            sFamilyName, sStyleName, nFamily, nPitch, eEnc ) );
        if (!sName.isEmpty())
        {
            pFontNameState->maValue <<= sName;
            //Resolves: fdo#68431 style:font-name unrecognized by LibreOffice
            //<= 4.1 in styles (but recognized in autostyles) so add
            //fo:font-family, etc
            if (!bEnableFoFontFamily)
            {
                if( pFontFamilyNameState )
                    pFontFamilyNameState->mnIndex = -1;
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
        else
        {
            pFontNameState->mnIndex = -1;
        }
    }

    if( pFontFamilyNameState && sFamilyName.isEmpty() )
    {
        pFontFamilyNameState->mnIndex = -1;
    }

    if( pFontStyleNameState && sStyleName.isEmpty() )
    {
        pFontStyleNameState->mnIndex = -1;
    }
}

void XMLTextExportPropertySetMapper::ContextFontHeightFilter(
    XMLPropertyState* pCharHeightState,
    XMLPropertyState* pCharPropHeightState,
    XMLPropertyState* pCharDiffHeightState ) const
{
    if( pCharPropHeightState )
    {
        sal_Int32 nTemp = 0;
        pCharPropHeightState->maValue >>= nTemp;
        if( nTemp == 100 )
        {
            pCharPropHeightState->mnIndex = -1;
            pCharPropHeightState->maValue.clear();
        }
        else
        {
            pCharHeightState->mnIndex = -1;
            pCharHeightState->maValue.clear();
        }
    }
    if( pCharDiffHeightState )
    {
        float nTemp = 0;
        pCharDiffHeightState->maValue >>= nTemp;
        if( nTemp == 0. )
        {
            pCharDiffHeightState->mnIndex = -1;
            pCharDiffHeightState->maValue.clear();
        }
        else
        {
            pCharHeightState->mnIndex = -1;
            pCharHeightState->maValue.clear();
        }
    }

}

namespace {

// helper method; implementation below
static bool lcl_IsOutlineStyle(const SvXMLExport&, const OUString&);

static void
lcl_checkMultiProperty(XMLPropertyState *const pState,
                       XMLPropertyState *const pRelState)
{
    if (pState && pRelState)
    {
        sal_Int32 nTemp = 0;
        pRelState->maValue >>= nTemp;
        if (100 == nTemp)
        {
            pRelState->mnIndex = -1;
            pRelState->maValue.clear();
        }
        else
        {
            pState->mnIndex = -1;
            pState->maValue.clear();
        }
    }
}

/**
 * Filter context of paragraph and character borders.
 * Compress border attriubtes. If one of groupable attributes (border type, border width, padding)
 * is equal for all four side then just one general attribute will be exported.
**/
static void lcl_FilterBorders(
    XMLPropertyState* pAllBorderWidthState, XMLPropertyState* pLeftBorderWidthState,
    XMLPropertyState* pRightBorderWidthState, XMLPropertyState* pTopBorderWidthState,
    XMLPropertyState* pBottomBorderWidthState, XMLPropertyState* pAllBorderDistanceState,
    XMLPropertyState* pLeftBorderDistanceState, XMLPropertyState* pRightBorderDistanceState,
    XMLPropertyState* pTopBorderDistanceState, XMLPropertyState* pBottomBorderDistanceState,
    XMLPropertyState* pAllBorderState, XMLPropertyState* pLeftBorderState,
    XMLPropertyState* pRightBorderState,XMLPropertyState* pTopBorderState,
    XMLPropertyState* pBottomBorderState )
{
    if( pAllBorderWidthState )
    {
        if( pLeftBorderWidthState && pRightBorderWidthState && pTopBorderWidthState && pBottomBorderWidthState )
        {
            table::BorderLine2 aLeft, aRight, aTop, aBottom;

            pLeftBorderWidthState->maValue >>= aLeft;
            pRightBorderWidthState->maValue >>= aRight;
            pTopBorderWidthState->maValue >>= aTop;
            pBottomBorderWidthState->maValue >>= aBottom;
            if( aLeft.Color == aRight.Color && aLeft.InnerLineWidth == aRight.InnerLineWidth &&
                aLeft.OuterLineWidth == aRight.OuterLineWidth && aLeft.LineDistance == aRight.LineDistance &&
                aLeft.LineStyle == aRight.LineStyle &&
                aLeft.LineWidth == aRight.LineWidth &&
                aLeft.Color == aTop.Color && aLeft.InnerLineWidth == aTop.InnerLineWidth &&
                aLeft.OuterLineWidth == aTop.OuterLineWidth && aLeft.LineDistance == aTop.LineDistance &&
                aLeft.LineStyle == aTop.LineStyle &&
                aLeft.LineWidth == aTop.LineWidth &&
                aLeft.Color == aBottom.Color && aLeft.InnerLineWidth == aBottom.InnerLineWidth &&
                aLeft.OuterLineWidth == aBottom.OuterLineWidth && aLeft.LineDistance == aBottom.LineDistance &&
                aLeft.LineStyle == aBottom.LineStyle &&
                aLeft.LineWidth == aBottom.LineWidth )
            {
                pLeftBorderWidthState->mnIndex = -1;
                pLeftBorderWidthState->maValue.clear();
                pRightBorderWidthState->mnIndex = -1;
                pRightBorderWidthState->maValue.clear();
                pTopBorderWidthState->mnIndex = -1;
                pTopBorderWidthState->maValue.clear();
                pBottomBorderWidthState->mnIndex = -1;
                pBottomBorderWidthState->maValue.clear();
            }
            else
            {
                pAllBorderWidthState->mnIndex = -1;
                pAllBorderWidthState->maValue.clear();
            }
        }
        else
        {
            pAllBorderWidthState->mnIndex = -1;
            pAllBorderWidthState->maValue.clear();
        }
    }

    if( pAllBorderDistanceState )
    {
        if( pLeftBorderDistanceState && pRightBorderDistanceState && pTopBorderDistanceState && pBottomBorderDistanceState )
        {
            sal_Int32 aLeft = 0, aRight = 0, aTop = 0, aBottom = 0;

            pLeftBorderDistanceState->maValue >>= aLeft;
            pRightBorderDistanceState->maValue >>= aRight;
            pTopBorderDistanceState->maValue >>= aTop;
            pBottomBorderDistanceState->maValue >>= aBottom;
            if( aLeft == aRight && aLeft == aTop && aLeft == aBottom )
            {
                pLeftBorderDistanceState->mnIndex = -1;
                pLeftBorderDistanceState->maValue.clear();
                pRightBorderDistanceState->mnIndex = -1;
                pRightBorderDistanceState->maValue.clear();
                pTopBorderDistanceState->mnIndex = -1;
                pTopBorderDistanceState->maValue.clear();
                pBottomBorderDistanceState->mnIndex = -1;
                pBottomBorderDistanceState->maValue.clear();
            }
            else
            {
                pAllBorderDistanceState->mnIndex = -1;
                pAllBorderDistanceState->maValue.clear();
            }
        }
        else
        {
            pAllBorderDistanceState->mnIndex = -1;
            pAllBorderDistanceState->maValue.clear();
        }
    }

    if( pAllBorderState )
    {
        if( pLeftBorderState && pRightBorderState && pTopBorderState && pBottomBorderState )
        {
            table::BorderLine2 aLeft, aRight, aTop, aBottom;

            pLeftBorderState->maValue >>= aLeft;
            pRightBorderState->maValue >>= aRight;
            pTopBorderState->maValue >>= aTop;
            pBottomBorderState->maValue >>= aBottom;
            if( aLeft.Color == aRight.Color && aLeft.InnerLineWidth == aRight.InnerLineWidth &&
                aLeft.OuterLineWidth == aRight.OuterLineWidth && aLeft.LineDistance == aRight.LineDistance &&
                aLeft.LineStyle == aRight.LineStyle &&
                aLeft.LineWidth == aRight.LineWidth &&
                aLeft.Color == aTop.Color && aLeft.InnerLineWidth == aTop.InnerLineWidth &&
                aLeft.OuterLineWidth == aTop.OuterLineWidth && aLeft.LineDistance == aTop.LineDistance &&
                aLeft.LineStyle == aTop.LineStyle  &&
                aLeft.LineWidth == aTop.LineWidth  &&
                aLeft.Color == aBottom.Color && aLeft.InnerLineWidth == aBottom.InnerLineWidth &&
                aLeft.OuterLineWidth == aBottom.OuterLineWidth && aLeft.LineDistance == aBottom.LineDistance &&
                aLeft.LineWidth == aBottom.LineWidth &&
                aLeft.LineStyle == aBottom.LineStyle )
            {
                pLeftBorderState->mnIndex = -1;
                pLeftBorderState->maValue.clear();
                pRightBorderState->mnIndex = -1;
                pRightBorderState->maValue.clear();
                pTopBorderState->mnIndex = -1;
                pTopBorderState->maValue.clear();
                pBottomBorderState->mnIndex = -1;
                pBottomBorderState->maValue.clear();
            }
            else
            {
                pAllBorderState->mnIndex = -1;
                pAllBorderState->maValue.clear();
            }
        }
        else
        {
            pAllBorderState->mnIndex = -1;
            pAllBorderState->maValue.clear();
        }
    }
}

}

void XMLTextExportPropertySetMapper::ContextFilter(
    bool bEnableFoFontFamily,
    ::std::vector< XMLPropertyState >& rProperties,
    Reference< XPropertySet > rPropSet ) const
{
    // filter font
    XMLPropertyState *pFontNameState = 0;
    XMLPropertyState *pFontFamilyNameState = 0;
    XMLPropertyState *pFontStyleNameState = 0;
    XMLPropertyState *pFontFamilyState = 0;
    XMLPropertyState *pFontPitchState = 0;
    XMLPropertyState *pFontCharsetState = 0;
    XMLPropertyState *pFontNameCJKState = 0;
    XMLPropertyState *pFontFamilyNameCJKState = 0;
    XMLPropertyState *pFontStyleNameCJKState = 0;
    XMLPropertyState *pFontFamilyCJKState = 0;
    XMLPropertyState *pFontPitchCJKState = 0;
    XMLPropertyState *pFontCharsetCJKState = 0;
    XMLPropertyState *pFontNameCTLState = 0;
    XMLPropertyState *pFontFamilyNameCTLState = 0;
    XMLPropertyState *pFontStyleNameCTLState = 0;
    XMLPropertyState *pFontFamilyCTLState = 0;
    XMLPropertyState *pFontPitchCTLState = 0;
    XMLPropertyState *pFontCharsetCTLState = 0;

    // filter char height point/percent
    XMLPropertyState* pCharHeightState = NULL;
    XMLPropertyState* pCharPropHeightState = NULL;
    XMLPropertyState* pCharDiffHeightState = NULL;
    XMLPropertyState* pCharHeightCJKState = NULL;
    XMLPropertyState* pCharPropHeightCJKState = NULL;
    XMLPropertyState* pCharDiffHeightCJKState = NULL;
    XMLPropertyState* pCharHeightCTLState = NULL;
    XMLPropertyState* pCharPropHeightCTLState = NULL;
    XMLPropertyState* pCharDiffHeightCTLState = NULL;

    // filter left margin measure/percent
    XMLPropertyState* pParaLeftMarginState = NULL;
    XMLPropertyState* pParaLeftMarginRelState = NULL;

    // filter right margin measure/percent
    XMLPropertyState* pParaRightMarginState = NULL;
    XMLPropertyState* pParaRightMarginRelState = NULL;

    // filter first line indent measure/percent
    XMLPropertyState* pParaFirstLineState = NULL;
    XMLPropertyState* pParaFirstLineRelState = NULL;

    // filter ParaTopMargin/Relative
    XMLPropertyState* pParaTopMarginState = NULL;
    XMLPropertyState* pParaTopMarginRelState = NULL;

    // filter ParaTopMargin/Relative
    XMLPropertyState* pParaBottomMarginState = NULL;
    XMLPropertyState* pParaBottomMarginRelState = NULL;

    // filter (Left|Right|Top|Bottom|)BorderWidth
    XMLPropertyState* pAllBorderWidthState = NULL;
    XMLPropertyState* pLeftBorderWidthState = NULL;
    XMLPropertyState* pRightBorderWidthState = NULL;
    XMLPropertyState* pTopBorderWidthState = NULL;
    XMLPropertyState* pBottomBorderWidthState = NULL;

    // filter (Left|Right|Top|)BorderDistance
    XMLPropertyState* pAllBorderDistanceState = NULL;
    XMLPropertyState* pLeftBorderDistanceState = NULL;
    XMLPropertyState* pRightBorderDistanceState = NULL;
    XMLPropertyState* pTopBorderDistanceState = NULL;
    XMLPropertyState* pBottomBorderDistanceState = NULL;

    // filter (Left|Right|Top|Bottom|)Border
    XMLPropertyState* pAllBorderState = NULL;
    XMLPropertyState* pLeftBorderState = NULL;
    XMLPropertyState* pRightBorderState = NULL;
    XMLPropertyState* pTopBorderState = NULL;
    XMLPropertyState* pBottomBorderState = NULL;

    // filter Char(Left|Right|Top|Bottom|)BorderWidth
    XMLPropertyState* pCharAllBorderWidthState = NULL;
    XMLPropertyState* pCharLeftBorderWidthState = NULL;
    XMLPropertyState* pCharRightBorderWidthState = NULL;
    XMLPropertyState* pCharTopBorderWidthState = NULL;
    XMLPropertyState* pCharBottomBorderWidthState = NULL;

    // filter Char(Left|Right|Top|)BorderDistance
    XMLPropertyState* pCharAllBorderDistanceState = NULL;
    XMLPropertyState* pCharLeftBorderDistanceState = NULL;
    XMLPropertyState* pCharRightBorderDistanceState = NULL;
    XMLPropertyState* pCharTopBorderDistanceState = NULL;
    XMLPropertyState* pCharBottomBorderDistanceState = NULL;

    // filter Char(Left|Right|Top|Bottom|)Border
    XMLPropertyState* pCharAllBorderState = NULL;
    XMLPropertyState* pCharLeftBorderState = NULL;
    XMLPropertyState* pCharRightBorderState = NULL;
    XMLPropertyState* pCharTopBorderState = NULL;
    XMLPropertyState* pCharBottomBorderState = NULL;

    // filter height properties
    XMLPropertyState* pHeightMinAbsState = NULL;
    XMLPropertyState* pHeightMinRelState = NULL;
    XMLPropertyState* pHeightAbsState = NULL;
    XMLPropertyState* pHeightRelState = NULL;
    XMLPropertyState* pSizeTypeState = NULL;

    // filter width properties
    XMLPropertyState* pWidthMinAbsState = NULL;
    XMLPropertyState* pWidthMinRelState = NULL;
    XMLPropertyState* pWidthAbsState = NULL;
    XMLPropertyState* pWidthRelState = NULL;
    XMLPropertyState* pWidthTypeState = NULL;

    // wrap
    XMLPropertyState* pWrapState = NULL;
    XMLPropertyState* pWrapContourState = NULL;
    XMLPropertyState* pWrapContourModeState = NULL;
    XMLPropertyState* pWrapParagraphOnlyState = NULL;

    // anchor
    XMLPropertyState* pAnchorTypeState = NULL;

    // horizontal position and relation
    XMLPropertyState* pHoriOrientState = NULL;
    XMLPropertyState* pHoriOrientMirroredState = NULL;
    XMLPropertyState* pHoriOrientRelState = NULL;
    XMLPropertyState* pHoriOrientRelFrameState = NULL;
    XMLPropertyState* pHoriOrientMirrorState = NULL;
    // Horizontal position and relation for shapes (#i28749#)
    XMLPropertyState* pShapeHoriOrientState = NULL;
    XMLPropertyState* pShapeHoriOrientMirroredState = NULL;
    XMLPropertyState* pShapeHoriOrientRelState = NULL;
    XMLPropertyState* pShapeHoriOrientRelFrameState = NULL;
    XMLPropertyState* pShapeHoriOrientMirrorState = NULL;

    // vertical position and relation
    XMLPropertyState* pVertOrientState = NULL;
    XMLPropertyState* pVertOrientAtCharState = NULL;
    XMLPropertyState* pVertOrientRelState = NULL;
    XMLPropertyState* pVertOrientRelPageState = NULL;
    XMLPropertyState* pVertOrientRelFrameState = NULL;
    XMLPropertyState* pVertOrientRelAsCharState = NULL;

    // Vertical position and relation for shapes (#i28749#)
    XMLPropertyState* pShapeVertOrientState = NULL;
    XMLPropertyState* pShapeVertOrientAtCharState = NULL;
    XMLPropertyState* pShapeVertOrientRelState = NULL;
    XMLPropertyState* pShapeVertOrientRelPageState = NULL;
    XMLPropertyState* pShapeVertOrientRelFrameState = NULL;

    // filter underline color
    XMLPropertyState* pUnderlineState = NULL;
    XMLPropertyState* pUnderlineColorState = NULL;
    XMLPropertyState* pUnderlineHasColorState = NULL;

    // filter list style name
    XMLPropertyState* pListStyleName = NULL;

    // filter fo:clip
    XMLPropertyState* pClip11State = NULL;
    XMLPropertyState* pClipState = NULL;

    // filter fo:margin
    XMLPropertyState* pAllParaMarginRel = NULL;
    XMLPropertyState* pAllParaMargin = NULL;
    XMLPropertyState* pAllMargin = NULL;

    sal_Bool bNeedsAnchor = sal_False;

    for( ::std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
         aIter != rProperties.end();
         ++aIter )
    {
        XMLPropertyState *propertie = &(*aIter);
        if( propertie->mnIndex == -1 )
            continue;

        switch( getPropertySetMapper()->GetEntryContextId( propertie->mnIndex ) )
        {
        case CTF_CHARHEIGHT:            pCharHeightState = propertie; break;
        case CTF_CHARHEIGHT_REL:        pCharPropHeightState = propertie; break;
        case CTF_CHARHEIGHT_DIFF:       pCharDiffHeightState = propertie; break;
        case CTF_CHARHEIGHT_CJK:        pCharHeightCJKState = propertie; break;
        case CTF_CHARHEIGHT_REL_CJK:    pCharPropHeightCJKState = propertie; break;
        case CTF_CHARHEIGHT_DIFF_CJK:   pCharDiffHeightCJKState = propertie; break;
        case CTF_CHARHEIGHT_CTL:        pCharHeightCTLState = propertie; break;
        case CTF_CHARHEIGHT_REL_CTL:    pCharPropHeightCTLState = propertie; break;
        case CTF_CHARHEIGHT_DIFF_CTL:   pCharDiffHeightCTLState = propertie; break;
        case CTF_PARALEFTMARGIN:        pParaLeftMarginState = propertie; break;
        case CTF_PARALEFTMARGIN_REL:    pParaLeftMarginRelState = propertie; break;
        case CTF_PARARIGHTMARGIN:       pParaRightMarginState = propertie; break;
        case CTF_PARARIGHTMARGIN_REL:   pParaRightMarginRelState = propertie; break;
        case CTF_PARAFIRSTLINE:         pParaFirstLineState = propertie; break;
        case CTF_PARAFIRSTLINE_REL:     pParaFirstLineRelState = propertie; break;
        case CTF_PARATOPMARGIN:         pParaTopMarginState = propertie; break;
        case CTF_PARATOPMARGIN_REL:     pParaTopMarginRelState = propertie; break;
        case CTF_PARABOTTOMMARGIN:      pParaBottomMarginState = propertie; break;
        case CTF_PARABOTTOMMARGIN_REL:  pParaBottomMarginRelState = propertie; break;

        case CTF_ALLBORDERWIDTH:        pAllBorderWidthState = propertie; break;
        case CTF_LEFTBORDERWIDTH:       pLeftBorderWidthState = propertie; break;
        case CTF_RIGHTBORDERWIDTH:      pRightBorderWidthState = propertie; break;
        case CTF_TOPBORDERWIDTH:        pTopBorderWidthState = propertie; break;
        case CTF_BOTTOMBORDERWIDTH:     pBottomBorderWidthState = propertie; break;
        case CTF_ALLBORDERDISTANCE:     pAllBorderDistanceState = propertie; break;
        case CTF_LEFTBORDERDISTANCE:    pLeftBorderDistanceState = propertie; break;
        case CTF_RIGHTBORDERDISTANCE:   pRightBorderDistanceState = propertie; break;
        case CTF_TOPBORDERDISTANCE:     pTopBorderDistanceState = propertie; break;
        case CTF_BOTTOMBORDERDISTANCE:  pBottomBorderDistanceState = propertie; break;
        case CTF_ALLBORDER:             pAllBorderState = propertie; break;
        case CTF_LEFTBORDER:            pLeftBorderState = propertie; break;
        case CTF_RIGHTBORDER:           pRightBorderState = propertie; break;
        case CTF_TOPBORDER:             pTopBorderState = propertie; break;
        case CTF_BOTTOMBORDER:          pBottomBorderState = propertie; break;

        case CTF_CHARALLBORDERWIDTH:        pCharAllBorderWidthState = propertie; break;
        case CTF_CHARLEFTBORDERWIDTH:       pCharLeftBorderWidthState = propertie; break;
        case CTF_CHARRIGHTBORDERWIDTH:      pCharRightBorderWidthState = propertie; break;
        case CTF_CHARTOPBORDERWIDTH:        pCharTopBorderWidthState = propertie; break;
        case CTF_CHARBOTTOMBORDERWIDTH:     pCharBottomBorderWidthState = propertie; break;
        case CTF_CHARALLBORDERDISTANCE:     pCharAllBorderDistanceState = propertie; break;
        case CTF_CHARLEFTBORDERDISTANCE:    pCharLeftBorderDistanceState = propertie; break;
        case CTF_CHARRIGHTBORDERDISTANCE:   pCharRightBorderDistanceState = propertie; break;
        case CTF_CHARTOPBORDERDISTANCE:     pCharTopBorderDistanceState = propertie; break;
        case CTF_CHARBOTTOMBORDERDISTANCE:  pCharBottomBorderDistanceState = propertie; break;
        case CTF_CHARALLBORDER:             pCharAllBorderState = propertie; break;
        case CTF_CHARLEFTBORDER:            pCharLeftBorderState = propertie; break;
        case CTF_CHARRIGHTBORDER:           pCharRightBorderState = propertie; break;
        case CTF_CHARTOPBORDER:             pCharTopBorderState = propertie; break;
        case CTF_CHARBOTTOMBORDER:          pCharBottomBorderState = propertie; break;

        case CTF_FRAMEHEIGHT_MIN_ABS:   pHeightMinAbsState = propertie; break;
        case CTF_FRAMEHEIGHT_MIN_REL:   pHeightMinRelState = propertie; break;
        case CTF_FRAMEHEIGHT_ABS:       pHeightAbsState = propertie; break;
        case CTF_FRAMEHEIGHT_REL:       pHeightRelState = propertie; break;
        case CTF_SIZETYPE:              pSizeTypeState = propertie; break;

        case CTF_FRAMEWIDTH_MIN_ABS:    pWidthMinAbsState = propertie; break;
        case CTF_FRAMEWIDTH_MIN_REL:    pWidthMinRelState = propertie; break;
        case CTF_FRAMEWIDTH_ABS:        pWidthAbsState = propertie; break;
        case CTF_FRAMEWIDTH_REL:        pWidthRelState = propertie; break;
        case CTF_FRAMEWIDTH_TYPE:       pWidthTypeState = propertie; break;

        case CTF_WRAP:                  pWrapState = propertie; break;
        case CTF_WRAP_CONTOUR:          pWrapContourState = propertie; break;
        case CTF_WRAP_CONTOUR_MODE:     pWrapContourModeState = propertie; break;
        case CTF_WRAP_PARAGRAPH_ONLY:   pWrapParagraphOnlyState = propertie; break;
        case CTF_ANCHORTYPE:            pAnchorTypeState = propertie; break;

        case CTF_HORIZONTALPOS:             pHoriOrientState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_HORIZONTALPOS_MIRRORED:    pHoriOrientMirroredState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_HORIZONTALREL:             pHoriOrientRelState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_HORIZONTALREL_FRAME:       pHoriOrientRelFrameState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_HORIZONTALMIRROR:          pHoriOrientMirrorState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALPOS:           pVertOrientState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALPOS_ATCHAR:    pVertOrientAtCharState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALREL:           pVertOrientRelState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALREL_PAGE:      pVertOrientRelPageState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALREL_FRAME:     pVertOrientRelFrameState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALREL_ASCHAR:    pVertOrientRelAsCharState = propertie; bNeedsAnchor = sal_True; break;

        // Handle new CTFs for shape positioning properties (#i28749#)
        case CTF_SHAPE_HORIZONTALPOS:             pShapeHoriOrientState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_HORIZONTALPOS_MIRRORED:    pShapeHoriOrientMirroredState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_HORIZONTALREL:             pShapeHoriOrientRelState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_HORIZONTALREL_FRAME:       pShapeHoriOrientRelFrameState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_HORIZONTALMIRROR:          pShapeHoriOrientMirrorState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_VERTICALPOS:           pShapeVertOrientState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_VERTICALPOS_ATCHAR:    pShapeVertOrientAtCharState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_VERTICALREL:           pShapeVertOrientRelState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_VERTICALREL_PAGE:      pShapeVertOrientRelPageState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_SHAPE_VERTICALREL_FRAME:     pShapeVertOrientRelFrameState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_FONTNAME:              pFontNameState = propertie; break;
        case CTF_FONTFAMILYNAME:        pFontFamilyNameState = propertie; break;
        case CTF_FONTSTYLENAME:         pFontStyleNameState = propertie; break;
        case CTF_FONTFAMILY:            pFontFamilyState = propertie; break;
        case CTF_FONTPITCH:             pFontPitchState = propertie; break;
        case CTF_FONTCHARSET:           pFontCharsetState = propertie; break;

        case CTF_FONTNAME_CJK:          pFontNameCJKState = propertie; break;
        case CTF_FONTFAMILYNAME_CJK:    pFontFamilyNameCJKState = propertie; break;
        case CTF_FONTSTYLENAME_CJK:     pFontStyleNameCJKState = propertie; break;
        case CTF_FONTFAMILY_CJK:        pFontFamilyCJKState = propertie; break;
        case CTF_FONTPITCH_CJK:         pFontPitchCJKState = propertie; break;
        case CTF_FONTCHARSET_CJK:       pFontCharsetCJKState = propertie; break;

        case CTF_FONTNAME_CTL:          pFontNameCTLState = propertie; break;
        case CTF_FONTFAMILYNAME_CTL:    pFontFamilyNameCTLState = propertie; break;
        case CTF_FONTSTYLENAME_CTL:     pFontStyleNameCTLState = propertie; break;
        case CTF_FONTFAMILY_CTL:        pFontFamilyCTLState = propertie; break;
        case CTF_FONTPITCH_CTL:         pFontPitchCTLState = propertie; break;
        case CTF_FONTCHARSET_CTL:       pFontCharsetCTLState = propertie; break;
        case CTF_UNDERLINE:             pUnderlineState = propertie; break;
        case CTF_UNDERLINE_COLOR:       pUnderlineColorState = propertie; break;
        case CTF_UNDERLINE_HASCOLOR:    pUnderlineHasColorState = propertie; break;
        case CTF_NUMBERINGSTYLENAME:    pListStyleName = propertie; break;
        case CTF_TEXT_CLIP11:           pClip11State = propertie; break;
        case CTF_TEXT_CLIP:             pClipState = propertie; break;
        case CTF_PARAMARGINALL_REL:     pAllParaMarginRel = propertie; break;
        case CTF_PARAMARGINALL:         pAllParaMargin = propertie; break;
        case CTF_MARGINALL:             pAllMargin = propertie; break;
        }
    }

    if( pFontNameState )
        ContextFontFilter( bEnableFoFontFamily, pFontNameState, pFontFamilyNameState,
                           pFontStyleNameState, pFontFamilyState,
                           pFontPitchState, pFontCharsetState );
    if( pFontNameCJKState )
        ContextFontFilter( bEnableFoFontFamily, pFontNameCJKState, pFontFamilyNameCJKState,
                           pFontStyleNameCJKState, pFontFamilyCJKState,
                           pFontPitchCJKState, pFontCharsetCJKState );
    if( pFontNameCTLState )
        ContextFontFilter( bEnableFoFontFamily, pFontNameCTLState, pFontFamilyNameCTLState,
                           pFontStyleNameCTLState, pFontFamilyCTLState,
                           pFontPitchCTLState, pFontCharsetCTLState );

    if( pCharHeightState && (pCharPropHeightState || pCharDiffHeightState ) )
        ContextFontHeightFilter( pCharHeightState, pCharPropHeightState,
                                 pCharDiffHeightState  );
    if( pCharHeightCJKState &&
        (pCharPropHeightCJKState || pCharDiffHeightCJKState ) )
        ContextFontHeightFilter( pCharHeightCJKState, pCharPropHeightCJKState,
                                 pCharDiffHeightCJKState  );
    if( pCharHeightCTLState &&
        (pCharPropHeightCTLState || pCharDiffHeightCTLState ) )
        ContextFontHeightFilter( pCharHeightCTLState, pCharPropHeightCTLState,
                                 pCharDiffHeightCTLState  );
    if( pUnderlineColorState || pUnderlineHasColorState )
    {
        sal_Bool bClear = !pUnderlineState;
        if( !bClear )
        {
            sal_Int16 nUnderline = 0;
            pUnderlineState->maValue >>= nUnderline;
            bClear = awt::FontUnderline::NONE == nUnderline;
        }
        if( bClear )
        {
            if( pUnderlineColorState )
                pUnderlineColorState->mnIndex = -1;
            if( pUnderlineHasColorState )
                pUnderlineHasColorState->mnIndex = -1;
        }
    }

    lcl_checkMultiProperty(pParaLeftMarginState, pParaLeftMarginRelState);
    lcl_checkMultiProperty(pParaRightMarginState, pParaRightMarginRelState);
    lcl_checkMultiProperty(pParaTopMarginState, pParaTopMarginRelState);
    lcl_checkMultiProperty(pParaBottomMarginState, pParaBottomMarginRelState);
    lcl_checkMultiProperty(pParaFirstLineState, pParaFirstLineRelState);

    if (pAllParaMarginRel)
    {   // because older OOo/LO versions can't read fo:margin:
        pAllParaMarginRel->mnIndex = -1; // just export individual attributes...
        pAllParaMarginRel->maValue.clear();
    }
    if (pAllParaMargin)
    {
        pAllParaMargin->mnIndex = -1; // just export individual attributes...
        pAllParaMargin->maValue.clear();
    }
    if (pAllMargin)
    {
        pAllMargin->mnIndex = -1; // just export individual attributes...
        pAllMargin->maValue.clear();
    }

    lcl_FilterBorders(
        pAllBorderWidthState, pLeftBorderWidthState, pRightBorderWidthState,
        pTopBorderWidthState, pBottomBorderWidthState, pAllBorderDistanceState,
        pLeftBorderDistanceState, pRightBorderDistanceState, pTopBorderDistanceState,
        pBottomBorderDistanceState, pAllBorderState, pLeftBorderState,
        pRightBorderState, pTopBorderState, pBottomBorderState);

    lcl_FilterBorders(
        pCharAllBorderWidthState, pCharLeftBorderWidthState, pCharRightBorderWidthState,
        pCharTopBorderWidthState, pCharBottomBorderWidthState, pCharAllBorderDistanceState,
        pCharLeftBorderDistanceState, pCharRightBorderDistanceState, pCharTopBorderDistanceState,
        pCharBottomBorderDistanceState, pCharAllBorderState, pCharLeftBorderState,
        pCharRightBorderState, pCharTopBorderState, pCharBottomBorderState);

    sal_Int16 nSizeType = SizeType::FIX;
    if( pSizeTypeState )
    {
        pSizeTypeState->maValue >>= nSizeType;
        pSizeTypeState->mnIndex = -1;
    }

    if( pHeightMinAbsState )
    {
        sal_Int16 nRel = sal_Int16();
        if( (SizeType::FIX == nSizeType) ||
            ( pHeightMinRelState &&
              ( !(pHeightMinRelState->maValue >>= nRel) || nRel > 0 ) ) )
        {
            pHeightMinAbsState->mnIndex = -1;
        }

        // export SizeType::VARIABLE als min-width="0"
        if( SizeType::VARIABLE == nSizeType )
            pHeightMinAbsState->maValue <<= static_cast<sal_Int32>( 0 );
    }
    if( pHeightMinRelState  && SizeType::MIN != nSizeType)
        pHeightMinRelState->mnIndex = -1;
    if( pHeightAbsState && pHeightMinAbsState &&
        -1 != pHeightMinAbsState->mnIndex )
        pHeightAbsState->mnIndex = -1;
    if( pHeightRelState && SizeType::FIX != nSizeType)
        pHeightRelState->mnIndex = -1;

    // frame width
    nSizeType = SizeType::FIX;
    if( pWidthTypeState )
    {
        pWidthTypeState->maValue >>= nSizeType;
        pWidthTypeState->mnIndex = -1;
    }
    if( pWidthMinAbsState )
    {
        sal_Int16 nRel = sal_Int16();
        if( (SizeType::FIX == nSizeType) ||
            ( pWidthMinRelState &&
              ( !(pWidthMinRelState->maValue >>= nRel) || nRel > 0 ) ) )
        {
            pWidthMinAbsState->mnIndex = -1;
        }

        // export SizeType::VARIABLE als min-width="0"
        if( SizeType::VARIABLE == nSizeType )
            pWidthMinAbsState->maValue <<= static_cast<sal_Int32>( 0 );
    }
    if( pWidthMinRelState  && SizeType::MIN != nSizeType)
        pWidthMinRelState->mnIndex = -1;
    if( pWidthAbsState && pWidthMinAbsState &&
        -1 != pWidthMinAbsState->mnIndex )
        pWidthAbsState->mnIndex = -1;
    if( pWidthRelState && SizeType::FIX != nSizeType)
        pWidthRelState->mnIndex = -1;

    if( pWrapState )
    {
        WrapTextMode eVal;
        pWrapState->maValue >>= eVal;
        switch( eVal )
        {
        case WrapTextMode_NONE:
            // no wrapping: disable para-only and contour
            if( pWrapParagraphOnlyState )
                pWrapParagraphOnlyState->mnIndex = -1;
            // no break
        case WrapTextMode_THROUGHT:
            // wrap through: disable only contour
            if( pWrapContourState )
                pWrapContourState->mnIndex = -1;
            break;
        default:
            break;
        }
        if( pWrapContourModeState  &&
            (!pWrapContourState ||
             !*(sal_Bool *)pWrapContourState ->maValue.getValue() ) )
            pWrapContourModeState->mnIndex = -1;
    }

    TextContentAnchorType eAnchor = TextContentAnchorType_AT_PARAGRAPH;
    if( pAnchorTypeState )
        pAnchorTypeState->maValue >>= eAnchor;
    else if( bNeedsAnchor )
    {
        Any aAny = rPropSet->getPropertyValue("AnchorType");
        aAny >>= eAnchor;
    }

    // states for frame positioning attributes
    {
        if( pHoriOrientState && pHoriOrientMirroredState )
        {
            if( pHoriOrientMirrorState &&
                *(sal_Bool *)pHoriOrientMirrorState->maValue.getValue() )
                pHoriOrientState->mnIndex = -1;
            else
                pHoriOrientMirroredState->mnIndex = -1;
        }
        if( pHoriOrientMirrorState )
            pHoriOrientMirrorState->mnIndex = -1;

        if( pHoriOrientRelState && TextContentAnchorType_AT_FRAME == eAnchor )
            pHoriOrientRelState->mnIndex = -1;
        if( pHoriOrientRelFrameState && TextContentAnchorType_AT_FRAME != eAnchor )
            pHoriOrientRelFrameState->mnIndex = -1;

        if( pVertOrientState && TextContentAnchorType_AT_CHARACTER == eAnchor )
            pVertOrientState->mnIndex = -1;
        if( pVertOrientAtCharState && TextContentAnchorType_AT_CHARACTER != eAnchor )
            pVertOrientAtCharState->mnIndex = -1;
        if( pVertOrientRelState && TextContentAnchorType_AT_PARAGRAPH != eAnchor &&
            TextContentAnchorType_AT_CHARACTER != eAnchor )
            pVertOrientRelState->mnIndex = -1;
        if( pVertOrientRelPageState && TextContentAnchorType_AT_PAGE != eAnchor )
            pVertOrientRelPageState->mnIndex = -1;
        if( pVertOrientRelFrameState && TextContentAnchorType_AT_FRAME != eAnchor )
            pVertOrientRelFrameState->mnIndex = -1;
        if( pVertOrientRelAsCharState && TextContentAnchorType_AS_CHARACTER != eAnchor )
            pVertOrientRelAsCharState->mnIndex = -1;
    }

    // States for shape positioning properties (#i28749#)
    if ( eAnchor != TextContentAnchorType_AS_CHARACTER &&
         ( GetExport().getExportFlags() & EXPORT_OASIS ) == 0 )
    {
        // no export of shape positioning properties,
        // if shape isn't anchored as-character and
        // destination file format is OpenOffice.org file format
        if ( pShapeHoriOrientState )
            pShapeHoriOrientState->mnIndex = -1;
        if ( pShapeHoriOrientMirroredState )
            pShapeHoriOrientMirroredState->mnIndex = -1;
        if ( pShapeHoriOrientRelState )
            pShapeHoriOrientRelState->mnIndex = -1;
        if ( pShapeHoriOrientRelFrameState )
            pShapeHoriOrientRelFrameState->mnIndex = -1;
        if ( pShapeHoriOrientMirrorState )
            pShapeHoriOrientMirrorState->mnIndex = -1;
        if ( pShapeVertOrientState )
            pShapeVertOrientState->mnIndex = -1;
        if ( pShapeVertOrientAtCharState )
            pShapeVertOrientAtCharState->mnIndex = -1;
        if ( pShapeVertOrientRelState )
            pShapeVertOrientRelState->mnIndex = -1;
        if ( pShapeVertOrientRelPageState )
            pShapeVertOrientRelPageState->mnIndex = -1;
        if ( pShapeVertOrientRelFrameState )
            pShapeVertOrientRelFrameState->mnIndex = -1;
    }
    else
    {
        // handling of shape positioning property states as for frames - see above
        if( pShapeHoriOrientState && pShapeHoriOrientMirroredState )
        {
            if( pShapeHoriOrientMirrorState &&
                *(sal_Bool *)pShapeHoriOrientMirrorState->maValue.getValue() )
                pShapeHoriOrientState->mnIndex = -1;
            else
                pShapeHoriOrientMirroredState->mnIndex = -1;
        }
        if( pShapeHoriOrientMirrorState )
            pShapeHoriOrientMirrorState->mnIndex = -1;

        if( pShapeHoriOrientRelState && TextContentAnchorType_AT_FRAME == eAnchor )
            pShapeHoriOrientRelState->mnIndex = -1;
        if( pShapeHoriOrientRelFrameState && TextContentAnchorType_AT_FRAME != eAnchor )
            pShapeHoriOrientRelFrameState->mnIndex = -1;

        if( pShapeVertOrientState && TextContentAnchorType_AT_CHARACTER == eAnchor )
            pShapeVertOrientState->mnIndex = -1;
        if( pShapeVertOrientAtCharState && TextContentAnchorType_AT_CHARACTER != eAnchor )
            pShapeVertOrientAtCharState->mnIndex = -1;
        if( pShapeVertOrientRelState && TextContentAnchorType_AT_PARAGRAPH != eAnchor &&
            TextContentAnchorType_AT_CHARACTER != eAnchor )
            pShapeVertOrientRelState->mnIndex = -1;
        if( pShapeVertOrientRelPageState && TextContentAnchorType_AT_PAGE != eAnchor )
            pShapeVertOrientRelPageState->mnIndex = -1;
        if( pShapeVertOrientRelFrameState && TextContentAnchorType_AT_FRAME != eAnchor )
            pShapeVertOrientRelFrameState->mnIndex = -1;
    }

    // list style name: remove list style if it is the default outline style
    if( pListStyleName != NULL )
    {
        OUString sListStyleName;
        pListStyleName->maValue >>= sListStyleName;
        if( lcl_IsOutlineStyle( GetExport(), sListStyleName ) )
            pListStyleName->mnIndex = -1;
    }

    if( pClipState != NULL && pClip11State != NULL  )
        pClip11State->mnIndex = -1;

    SvXMLExportPropertyMapper::ContextFilter(bEnableFoFontFamily, rProperties, rPropSet);
}

namespace {

static bool lcl_IsOutlineStyle(const SvXMLExport &rExport, const OUString & rName)
{
    Reference< XChapterNumberingSupplier >
        xCNSupplier(rExport.GetModel(), UNO_QUERY);

    OUString sOutlineName;
    OUString sName("Name");

    if (xCNSupplier.is())
    {
        Reference<XPropertySet> xNumRule(
            xCNSupplier->getChapterNumberingRules(), UNO_QUERY );
        DBG_ASSERT( xNumRule.is(), "no chapter numbering rules" );
        if (xNumRule.is())
        {
            xNumRule->getPropertyValue(sName) >>= sOutlineName;
        }
    }

    return rName == sOutlineName;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
