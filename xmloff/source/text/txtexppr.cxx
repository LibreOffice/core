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
#include <tools/color.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/maptype.hxx>
#include "XMLSectionFootnoteConfigExport.hxx"

//UUUU
#include <xmlsdtypes.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;

void XMLTextExportPropertySetMapper::handleElementItem(
        SvXMLExport& rExp,
        const XMLPropertyState& rProperty,
        SvXmlExportFlags nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    XMLTextExportPropertySetMapper *pThis =
           const_cast<XMLTextExportPropertySetMapper*>(this);

    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
    case CTF_DROPCAPFORMAT:
        pThis->maDropCapExport.exportXML( rProperty.maValue, bDropWholeWord,
                                          sDropCharStyle );
        pThis->bDropWholeWord = false;
        (pThis->sDropCharStyle).clear();
        break;

    case CTF_TABSTOP:
        pThis->maTabStopExport.Export( rProperty.maValue );
        break;

    case CTF_TEXTCOLUMNS:
        pThis->maTextColumnsExport.exportXML( rProperty.maValue );
        break;

    case CTF_BACKGROUND_URL:
        {
            const Any *pPos = nullptr, *pFilter = nullptr, *pTrans = nullptr;
            sal_uInt32 nPropIndex = rProperty.mnIndex;

            // these are all optional, so have to check them in order
            // note: this index order dependency is a steaming pile of manure
            if (nIdx)
            {
                const XMLPropertyState& rFilter = (*pProperties)[nIdx - 1];
                if (CTF_BACKGROUND_FILTER == getPropertySetMapper()
                        ->GetEntryContextId(rFilter.mnIndex))
                {
                    pFilter = &rFilter.maValue;
                    --nIdx;
                }
            }

            if (nIdx)
            {
                const XMLPropertyState& rPos = (*pProperties)[nIdx - 1];
                if (CTF_BACKGROUND_POS == getPropertySetMapper()
                        ->GetEntryContextId(rPos.mnIndex))
                {
                    pPos = &rPos.maValue;
                    --nIdx;
                }
            }

            if (nIdx)
            {
                const XMLPropertyState& rTrans = (*pProperties)[nIdx - 1];
                // #99657# transparency may be there, but doesn't have to be.
                // If it's there, it must be in the right position.
                if( CTF_BACKGROUND_TRANSPARENCY == getPropertySetMapper()
                      ->GetEntryContextId( rTrans.mnIndex ) )
                    pTrans = &rTrans.maValue;
            }

            pThis->maBackgroundImageExport.exportXML(
                    rProperty.maValue, pPos, pFilter, pTrans,
                    getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                    getPropertySetMapper()->GetEntryXMLName( nPropIndex ) );
        }
        break;

    case CTF_SECTION_FOOTNOTE_END:
        XMLSectionFootnoteConfigExport::exportXML(rExp, false,
                                                  pProperties, nIdx,
                                                  getPropertySetMapper());
        break;

    case CTF_SECTION_ENDNOTE_END:
        XMLSectionFootnoteConfigExport::exportXML(rExp, true,
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
           const_cast<XMLTextExportPropertySetMapper*>(this);

    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
    case CTF_DROPCAPWHOLEWORD:
        DBG_ASSERT( !bDropWholeWord, "drop whole word is set already!" );
        pThis->bDropWholeWord = *static_cast<sal_Bool const *>(rProperty.maValue.getValue());
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
        const rtl::Reference< XMLPropertySetMapper >& rMapper,
        SvXMLExport& rExp ) :
    SvXMLExportPropertyMapper( rMapper ),
    rExport( rExp ),
    bDropWholeWord( false ),
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

    //So assuming that the main purpose is just to convert the properties in
    //the main when we can, and to leave them alone when we can't. And with a
    //secondary purpose to filter out empty font properties, then is would
    //appear to make sense to base attempting the conversion if we have
    //both of the major facts of the font description

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
    XMLPropertyState* pCharDiffHeightState )
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
 * Compress border attributes. If one of groupable attributes (border type, border width, padding)
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
    XMLPropertyState *pFontNameState = nullptr;
    XMLPropertyState *pFontFamilyNameState = nullptr;
    XMLPropertyState *pFontStyleNameState = nullptr;
    XMLPropertyState *pFontFamilyState = nullptr;
    XMLPropertyState *pFontPitchState = nullptr;
    XMLPropertyState *pFontCharsetState = nullptr;
    XMLPropertyState *pFontNameCJKState = nullptr;
    XMLPropertyState *pFontFamilyNameCJKState = nullptr;
    XMLPropertyState *pFontStyleNameCJKState = nullptr;
    XMLPropertyState *pFontFamilyCJKState = nullptr;
    XMLPropertyState *pFontPitchCJKState = nullptr;
    XMLPropertyState *pFontCharsetCJKState = nullptr;
    XMLPropertyState *pFontNameCTLState = nullptr;
    XMLPropertyState *pFontFamilyNameCTLState = nullptr;
    XMLPropertyState *pFontStyleNameCTLState = nullptr;
    XMLPropertyState *pFontFamilyCTLState = nullptr;
    XMLPropertyState *pFontPitchCTLState = nullptr;
    XMLPropertyState *pFontCharsetCTLState = nullptr;

    // filter char height point/percent
    XMLPropertyState* pCharHeightState = nullptr;
    XMLPropertyState* pCharPropHeightState = nullptr;
    XMLPropertyState* pCharDiffHeightState = nullptr;
    XMLPropertyState* pCharHeightCJKState = nullptr;
    XMLPropertyState* pCharPropHeightCJKState = nullptr;
    XMLPropertyState* pCharDiffHeightCJKState = nullptr;
    XMLPropertyState* pCharHeightCTLState = nullptr;
    XMLPropertyState* pCharPropHeightCTLState = nullptr;
    XMLPropertyState* pCharDiffHeightCTLState = nullptr;

    // filter left margin measure/percent
    XMLPropertyState* pParaLeftMarginState = nullptr;
    XMLPropertyState* pParaLeftMarginRelState = nullptr;

    // filter right margin measure/percent
    XMLPropertyState* pParaRightMarginState = nullptr;
    XMLPropertyState* pParaRightMarginRelState = nullptr;

    // filter first line indent measure/percent
    XMLPropertyState* pParaFirstLineState = nullptr;
    XMLPropertyState* pParaFirstLineRelState = nullptr;

    // filter ParaTopMargin/Relative
    XMLPropertyState* pParaTopMarginState = nullptr;
    XMLPropertyState* pParaTopMarginRelState = nullptr;

    // filter ParaTopMargin/Relative
    XMLPropertyState* pParaBottomMarginState = nullptr;
    XMLPropertyState* pParaBottomMarginRelState = nullptr;

    // filter (Left|Right|Top|Bottom|)BorderWidth
    XMLPropertyState* pAllBorderWidthState = nullptr;
    XMLPropertyState* pLeftBorderWidthState = nullptr;
    XMLPropertyState* pRightBorderWidthState = nullptr;
    XMLPropertyState* pTopBorderWidthState = nullptr;
    XMLPropertyState* pBottomBorderWidthState = nullptr;

    // filter (Left|Right|Top|)BorderDistance
    XMLPropertyState* pAllBorderDistanceState = nullptr;
    XMLPropertyState* pLeftBorderDistanceState = nullptr;
    XMLPropertyState* pRightBorderDistanceState = nullptr;
    XMLPropertyState* pTopBorderDistanceState = nullptr;
    XMLPropertyState* pBottomBorderDistanceState = nullptr;

    // filter (Left|Right|Top|Bottom|)Border
    XMLPropertyState* pAllBorderState = nullptr;
    XMLPropertyState* pLeftBorderState = nullptr;
    XMLPropertyState* pRightBorderState = nullptr;
    XMLPropertyState* pTopBorderState = nullptr;
    XMLPropertyState* pBottomBorderState = nullptr;

    // filter Char(Left|Right|Top|Bottom|)BorderWidth
    XMLPropertyState* pCharAllBorderWidthState = nullptr;
    XMLPropertyState* pCharLeftBorderWidthState = nullptr;
    XMLPropertyState* pCharRightBorderWidthState = nullptr;
    XMLPropertyState* pCharTopBorderWidthState = nullptr;
    XMLPropertyState* pCharBottomBorderWidthState = nullptr;

    // filter Char(Left|Right|Top|)BorderDistance
    XMLPropertyState* pCharAllBorderDistanceState = nullptr;
    XMLPropertyState* pCharLeftBorderDistanceState = nullptr;
    XMLPropertyState* pCharRightBorderDistanceState = nullptr;
    XMLPropertyState* pCharTopBorderDistanceState = nullptr;
    XMLPropertyState* pCharBottomBorderDistanceState = nullptr;

    // filter Char(Left|Right|Top|Bottom|)Border
    XMLPropertyState* pCharAllBorderState = nullptr;
    XMLPropertyState* pCharLeftBorderState = nullptr;
    XMLPropertyState* pCharRightBorderState = nullptr;
    XMLPropertyState* pCharTopBorderState = nullptr;
    XMLPropertyState* pCharBottomBorderState = nullptr;

    // filter height properties
    XMLPropertyState* pHeightMinAbsState = nullptr;
    XMLPropertyState* pHeightMinRelState = nullptr;
    XMLPropertyState* pHeightAbsState = nullptr;
    XMLPropertyState* pHeightRelState = nullptr;
    XMLPropertyState* pSizeTypeState = nullptr;

    // filter width properties
    XMLPropertyState* pWidthMinAbsState = nullptr;
    XMLPropertyState* pWidthMinRelState = nullptr;
    XMLPropertyState* pWidthAbsState = nullptr;
    XMLPropertyState* pWidthRelState = nullptr;
    XMLPropertyState* pWidthTypeState = nullptr;

    // wrap
    XMLPropertyState* pWrapState = nullptr;
    XMLPropertyState* pWrapContourState = nullptr;
    XMLPropertyState* pWrapContourModeState = nullptr;
    XMLPropertyState* pWrapParagraphOnlyState = nullptr;

    // anchor
    XMLPropertyState* pAnchorTypeState = nullptr;

    // horizontal position and relation
    XMLPropertyState* pHoriOrientState = nullptr;
    XMLPropertyState* pHoriOrientMirroredState = nullptr;
    XMLPropertyState* pHoriOrientRelState = nullptr;
    XMLPropertyState* pHoriOrientRelFrameState = nullptr;
    XMLPropertyState* pHoriOrientMirrorState = nullptr;
    // Horizontal position and relation for shapes (#i28749#)
    XMLPropertyState* pShapeHoriOrientState = nullptr;
    XMLPropertyState* pShapeHoriOrientMirroredState = nullptr;
    XMLPropertyState* pShapeHoriOrientRelState = nullptr;
    XMLPropertyState* pShapeHoriOrientRelFrameState = nullptr;
    XMLPropertyState* pShapeHoriOrientMirrorState = nullptr;

    // vertical position and relation
    XMLPropertyState* pVertOrientState = nullptr;
    XMLPropertyState* pVertOrientAtCharState = nullptr;
    XMLPropertyState* pVertOrientRelState = nullptr;
    XMLPropertyState* pVertOrientRelPageState = nullptr;
    XMLPropertyState* pVertOrientRelFrameState = nullptr;
    XMLPropertyState* pVertOrientRelAsCharState = nullptr;
    XMLPropertyState* pRelWidthRel = nullptr;
    XMLPropertyState* pRelHeightRel = nullptr;

    // Vertical position and relation for shapes (#i28749#)
    XMLPropertyState* pShapeVertOrientState = nullptr;
    XMLPropertyState* pShapeVertOrientAtCharState = nullptr;
    XMLPropertyState* pShapeVertOrientRelState = nullptr;
    XMLPropertyState* pShapeVertOrientRelPageState = nullptr;
    XMLPropertyState* pShapeVertOrientRelFrameState = nullptr;

    // filter underline color
    XMLPropertyState* pUnderlineState = nullptr;
    XMLPropertyState* pUnderlineColorState = nullptr;
    XMLPropertyState* pUnderlineHasColorState = nullptr;

    // filter list style name
    XMLPropertyState* pListStyleName = nullptr;

    // filter fo:clip
    XMLPropertyState* pClip11State = nullptr;
    XMLPropertyState* pClipState = nullptr;

    // filter fo:margin
    XMLPropertyState* pAllParaMarginRel = nullptr;
    XMLPropertyState* pAllParaMargin = nullptr;
    XMLPropertyState* pAllMargin = nullptr;

    //UUUU
    XMLPropertyState* pRepeatOffsetX = nullptr;
    XMLPropertyState* pRepeatOffsetY = nullptr;

    // character background and highlight
    XMLPropertyState* pCharBackground = nullptr;
    XMLPropertyState* pCharBackgroundTransparency = nullptr;
    XMLPropertyState* pCharHighlight = nullptr;

    bool bNeedsAnchor = false;

    for( ::std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
         aIter != rProperties.end();
         ++aIter )
    {
        XMLPropertyState *propertyState = &(*aIter);
        if( propertyState->mnIndex == -1 )
            continue;

        switch( getPropertySetMapper()->GetEntryContextId( propertyState->mnIndex ) )
        {
        case CTF_CHARHEIGHT:            pCharHeightState = propertyState; break;
        case CTF_CHARHEIGHT_REL:        pCharPropHeightState = propertyState; break;
        case CTF_CHARHEIGHT_DIFF:       pCharDiffHeightState = propertyState; break;
        case CTF_CHARHEIGHT_CJK:        pCharHeightCJKState = propertyState; break;
        case CTF_CHARHEIGHT_REL_CJK:    pCharPropHeightCJKState = propertyState; break;
        case CTF_CHARHEIGHT_DIFF_CJK:   pCharDiffHeightCJKState = propertyState; break;
        case CTF_CHARHEIGHT_CTL:        pCharHeightCTLState = propertyState; break;
        case CTF_CHARHEIGHT_REL_CTL:    pCharPropHeightCTLState = propertyState; break;
        case CTF_CHARHEIGHT_DIFF_CTL:   pCharDiffHeightCTLState = propertyState; break;
        case CTF_PARALEFTMARGIN:        pParaLeftMarginState = propertyState; break;
        case CTF_PARALEFTMARGIN_REL:    pParaLeftMarginRelState = propertyState; break;
        case CTF_PARARIGHTMARGIN:       pParaRightMarginState = propertyState; break;
        case CTF_PARARIGHTMARGIN_REL:   pParaRightMarginRelState = propertyState; break;
        case CTF_PARAFIRSTLINE:         pParaFirstLineState = propertyState; break;
        case CTF_PARAFIRSTLINE_REL:     pParaFirstLineRelState = propertyState; break;
        case CTF_PARATOPMARGIN:         pParaTopMarginState = propertyState; break;
        case CTF_PARATOPMARGIN_REL:     pParaTopMarginRelState = propertyState; break;
        case CTF_PARABOTTOMMARGIN:      pParaBottomMarginState = propertyState; break;
        case CTF_PARABOTTOMMARGIN_REL:  pParaBottomMarginRelState = propertyState; break;

        case CTF_ALLBORDERWIDTH:        pAllBorderWidthState = propertyState; break;
        case CTF_LEFTBORDERWIDTH:       pLeftBorderWidthState = propertyState; break;
        case CTF_RIGHTBORDERWIDTH:      pRightBorderWidthState = propertyState; break;
        case CTF_TOPBORDERWIDTH:        pTopBorderWidthState = propertyState; break;
        case CTF_BOTTOMBORDERWIDTH:     pBottomBorderWidthState = propertyState; break;
        case CTF_ALLBORDERDISTANCE:     pAllBorderDistanceState = propertyState; break;
        case CTF_LEFTBORDERDISTANCE:    pLeftBorderDistanceState = propertyState; break;
        case CTF_RIGHTBORDERDISTANCE:   pRightBorderDistanceState = propertyState; break;
        case CTF_TOPBORDERDISTANCE:     pTopBorderDistanceState = propertyState; break;
        case CTF_BOTTOMBORDERDISTANCE:  pBottomBorderDistanceState = propertyState; break;
        case CTF_ALLBORDER:             pAllBorderState = propertyState; break;
        case CTF_LEFTBORDER:            pLeftBorderState = propertyState; break;
        case CTF_RIGHTBORDER:           pRightBorderState = propertyState; break;
        case CTF_TOPBORDER:             pTopBorderState = propertyState; break;
        case CTF_BOTTOMBORDER:          pBottomBorderState = propertyState; break;

        case CTF_CHARALLBORDERWIDTH:        pCharAllBorderWidthState = propertyState; break;
        case CTF_CHARLEFTBORDERWIDTH:       pCharLeftBorderWidthState = propertyState; break;
        case CTF_CHARRIGHTBORDERWIDTH:      pCharRightBorderWidthState = propertyState; break;
        case CTF_CHARTOPBORDERWIDTH:        pCharTopBorderWidthState = propertyState; break;
        case CTF_CHARBOTTOMBORDERWIDTH:     pCharBottomBorderWidthState = propertyState; break;
        case CTF_CHARALLBORDERDISTANCE:     pCharAllBorderDistanceState = propertyState; break;
        case CTF_CHARLEFTBORDERDISTANCE:    pCharLeftBorderDistanceState = propertyState; break;
        case CTF_CHARRIGHTBORDERDISTANCE:   pCharRightBorderDistanceState = propertyState; break;
        case CTF_CHARTOPBORDERDISTANCE:     pCharTopBorderDistanceState = propertyState; break;
        case CTF_CHARBOTTOMBORDERDISTANCE:  pCharBottomBorderDistanceState = propertyState; break;
        case CTF_CHARALLBORDER:             pCharAllBorderState = propertyState; break;
        case CTF_CHARLEFTBORDER:            pCharLeftBorderState = propertyState; break;
        case CTF_CHARRIGHTBORDER:           pCharRightBorderState = propertyState; break;
        case CTF_CHARTOPBORDER:             pCharTopBorderState = propertyState; break;
        case CTF_CHARBOTTOMBORDER:          pCharBottomBorderState = propertyState; break;

        case CTF_FRAMEHEIGHT_MIN_ABS:   pHeightMinAbsState = propertyState; break;
        case CTF_FRAMEHEIGHT_MIN_REL:   pHeightMinRelState = propertyState; break;
        case CTF_FRAMEHEIGHT_ABS:       pHeightAbsState = propertyState; break;
        case CTF_FRAMEHEIGHT_REL:       pHeightRelState = propertyState; break;
        case CTF_SIZETYPE:              pSizeTypeState = propertyState; break;

        case CTF_FRAMEWIDTH_MIN_ABS:    pWidthMinAbsState = propertyState; break;
        case CTF_FRAMEWIDTH_MIN_REL:    pWidthMinRelState = propertyState; break;
        case CTF_FRAMEWIDTH_ABS:        pWidthAbsState = propertyState; break;
        case CTF_FRAMEWIDTH_REL:        pWidthRelState = propertyState; break;
        case CTF_FRAMEWIDTH_TYPE:       pWidthTypeState = propertyState; break;

        case CTF_WRAP:                  pWrapState = propertyState; break;
        case CTF_WRAP_CONTOUR:          pWrapContourState = propertyState; break;
        case CTF_WRAP_CONTOUR_MODE:     pWrapContourModeState = propertyState; break;
        case CTF_WRAP_PARAGRAPH_ONLY:   pWrapParagraphOnlyState = propertyState; break;
        case CTF_ANCHORTYPE:            pAnchorTypeState = propertyState; break;

        case CTF_HORIZONTALPOS:             pHoriOrientState = propertyState; bNeedsAnchor = true; break;
        case CTF_HORIZONTALPOS_MIRRORED:    pHoriOrientMirroredState = propertyState; bNeedsAnchor = true; break;
        case CTF_HORIZONTALREL:             pHoriOrientRelState = propertyState; bNeedsAnchor = true; break;
        case CTF_HORIZONTALREL_FRAME:       pHoriOrientRelFrameState = propertyState; bNeedsAnchor = true; break;
        case CTF_HORIZONTALMIRROR:          pHoriOrientMirrorState = propertyState; bNeedsAnchor = true; break;
        case CTF_RELWIDTHREL:               pRelWidthRel = propertyState; break;
        case CTF_VERTICALPOS:           pVertOrientState = propertyState; bNeedsAnchor = true; break;
        case CTF_VERTICALPOS_ATCHAR:    pVertOrientAtCharState = propertyState; bNeedsAnchor = true; break;
        case CTF_VERTICALREL:           pVertOrientRelState = propertyState; bNeedsAnchor = true; break;
        case CTF_VERTICALREL_PAGE:      pVertOrientRelPageState = propertyState; bNeedsAnchor = true; break;
        case CTF_VERTICALREL_FRAME:     pVertOrientRelFrameState = propertyState; bNeedsAnchor = true; break;
        case CTF_VERTICALREL_ASCHAR:    pVertOrientRelAsCharState = propertyState; bNeedsAnchor = true; break;
        case CTF_RELHEIGHTREL:          pRelHeightRel = propertyState; break;

        // Handle new CTFs for shape positioning properties (#i28749#)
        case CTF_SHAPE_HORIZONTALPOS:             pShapeHoriOrientState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_HORIZONTALPOS_MIRRORED:    pShapeHoriOrientMirroredState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_HORIZONTALREL:             pShapeHoriOrientRelState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_HORIZONTALREL_FRAME:       pShapeHoriOrientRelFrameState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_HORIZONTALMIRROR:          pShapeHoriOrientMirrorState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_VERTICALPOS:           pShapeVertOrientState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_VERTICALPOS_ATCHAR:    pShapeVertOrientAtCharState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_VERTICALREL:           pShapeVertOrientRelState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_VERTICALREL_PAGE:      pShapeVertOrientRelPageState = propertyState; bNeedsAnchor = true; break;
        case CTF_SHAPE_VERTICALREL_FRAME:     pShapeVertOrientRelFrameState = propertyState; bNeedsAnchor = true; break;
        case CTF_FONTNAME:              pFontNameState = propertyState; break;
        case CTF_FONTFAMILYNAME:        pFontFamilyNameState = propertyState; break;
        case CTF_FONTSTYLENAME:         pFontStyleNameState = propertyState; break;
        case CTF_FONTFAMILY:            pFontFamilyState = propertyState; break;
        case CTF_FONTPITCH:             pFontPitchState = propertyState; break;
        case CTF_FONTCHARSET:           pFontCharsetState = propertyState; break;

        case CTF_FONTNAME_CJK:          pFontNameCJKState = propertyState; break;
        case CTF_FONTFAMILYNAME_CJK:    pFontFamilyNameCJKState = propertyState; break;
        case CTF_FONTSTYLENAME_CJK:     pFontStyleNameCJKState = propertyState; break;
        case CTF_FONTFAMILY_CJK:        pFontFamilyCJKState = propertyState; break;
        case CTF_FONTPITCH_CJK:         pFontPitchCJKState = propertyState; break;
        case CTF_FONTCHARSET_CJK:       pFontCharsetCJKState = propertyState; break;

        case CTF_FONTNAME_CTL:          pFontNameCTLState = propertyState; break;
        case CTF_FONTFAMILYNAME_CTL:    pFontFamilyNameCTLState = propertyState; break;
        case CTF_FONTSTYLENAME_CTL:     pFontStyleNameCTLState = propertyState; break;
        case CTF_FONTFAMILY_CTL:        pFontFamilyCTLState = propertyState; break;
        case CTF_FONTPITCH_CTL:         pFontPitchCTLState = propertyState; break;
        case CTF_FONTCHARSET_CTL:       pFontCharsetCTLState = propertyState; break;
        case CTF_UNDERLINE:             pUnderlineState = propertyState; break;
        case CTF_UNDERLINE_COLOR:       pUnderlineColorState = propertyState; break;
        case CTF_UNDERLINE_HASCOLOR:    pUnderlineHasColorState = propertyState; break;
        case CTF_NUMBERINGSTYLENAME:    pListStyleName = propertyState; break;
        case CTF_TEXT_CLIP11:           pClip11State = propertyState; break;
        case CTF_TEXT_CLIP:             pClipState = propertyState; break;
        case CTF_PARAMARGINALL_REL:     pAllParaMarginRel = propertyState; break;
        case CTF_PARAMARGINALL:         pAllParaMargin = propertyState; break;
        case CTF_MARGINALL:             pAllMargin = propertyState; break;

        //UUUU
        case CTF_REPEAT_OFFSET_X:
            pRepeatOffsetX = propertyState;
            break;

        //UUUU
        case CTF_REPEAT_OFFSET_Y:
            pRepeatOffsetY = propertyState;
            break;

        //UUUU
        case CTF_FILLGRADIENTNAME:
        case CTF_FILLHATCHNAME:
        case CTF_FILLBITMAPNAME:
        case CTF_FILLTRANSNAME:
            {
                OUString aStr;
                if( (propertyState->maValue >>= aStr) && 0 == aStr.getLength() )
                    propertyState->mnIndex = -1;
            }
            break;

        case CTF_CHAR_BACKGROUND: pCharBackground = propertyState; break;
        case CTF_CHAR_BACKGROUND_TRANSPARENCY: pCharBackgroundTransparency = propertyState; break;
        case CTF_CHAR_HIGHLIGHT: pCharHighlight = propertyState; break;
        }
    }

    //UUUU
    if( pRepeatOffsetX && pRepeatOffsetY )
    {
        sal_Int32 nOffset = 0;
        if( ( pRepeatOffsetX->maValue >>= nOffset ) && ( nOffset == 0 ) )
            pRepeatOffsetX->mnIndex = -1;
        else
            pRepeatOffsetY->mnIndex = -1;
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
        bool bClear = !pUnderlineState;
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
            SAL_FALLTHROUGH;
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
             !*static_cast<sal_Bool const *>(pWrapContourState ->maValue.getValue()) ) )
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
                *static_cast<sal_Bool const *>(pHoriOrientMirrorState->maValue.getValue()) )
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
        if (pRelWidthRel)
        {
            sal_Int16 nRelWidth = 0;
            rPropSet->getPropertyValue("RelativeWidth") >>= nRelWidth;
            if (!nRelWidth)
                pRelWidthRel->mnIndex = -1;
        }

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
        if (pRelHeightRel)
        {
            sal_Int16 nRelHeight = 0;
            rPropSet->getPropertyValue("RelativeHeight") >>= nRelHeight;
            if (!nRelHeight)
                pRelHeightRel->mnIndex = -1;
        }
    }

    // States for shape positioning properties (#i28749#)
    if ( eAnchor != TextContentAnchorType_AS_CHARACTER &&
         !( GetExport().getExportFlags() & SvXMLExportFlags::OASIS ) )
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
                *static_cast<sal_Bool const *>(pShapeHoriOrientMirrorState->maValue.getValue()) )
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
    if( pListStyleName != nullptr )
    {
        OUString sListStyleName;
        pListStyleName->maValue >>= sListStyleName;
        if( lcl_IsOutlineStyle( GetExport(), sListStyleName ) )
            pListStyleName->mnIndex = -1;
    }

    if( pClipState != nullptr && pClip11State != nullptr  )
        pClip11State->mnIndex = -1;

    // When both background attributes are available export the visible one
    if( pCharHighlight && pCharBackground )
    {
        assert(pCharBackgroundTransparency); // always together
        sal_uInt32 nColor = COL_TRANSPARENT;
        pCharHighlight->maValue >>= nColor;
        if( nColor == COL_TRANSPARENT )
        {
            // actually this would not be exported as transparent anyway
            // and we'd need another property CharHighlightTransparent for that
            pCharHighlight->mnIndex = -1;
        }
        else
        {
            pCharBackground->mnIndex = -1;
            pCharBackgroundTransparency->mnIndex = -1;
        }
    }

    SvXMLExportPropertyMapper::ContextFilter(bEnableFoFontFamily, rProperties, rPropSet);
}

namespace {

static bool lcl_IsOutlineStyle(const SvXMLExport &rExport, const OUString & rName)
{
    Reference< XChapterNumberingSupplier >
        xCNSupplier(rExport.GetModel(), UNO_QUERY);

    OUString sOutlineName;

    if (xCNSupplier.is())
    {
        Reference<XPropertySet> xNumRule(
            xCNSupplier->getChapterNumberingRules(), UNO_QUERY );
        DBG_ASSERT( xNumRule.is(), "no chapter numbering rules" );
        if (xNumRule.is())
        {
            xNumRule->getPropertyValue("Name") >>= sOutlineName;
        }
    }

    return rName == sOutlineName;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
