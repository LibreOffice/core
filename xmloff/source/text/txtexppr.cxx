/*************************************************************************
 *
 *  $RCSfile: txtexppr.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-23 11:56:13 $
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

#ifndef _XMLOFF_TEXTPRMAP_HXX_
#include "txtprmap.hxx"
#endif

#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTFAMILY_HPP
#include <com/sun/star/awt/FontFamily.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTPITCH_HPP
#include <com/sun/star/awt/FontPitch.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _XMLOFF_TXTEXPPR_HXX
#include "txtexppr.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::awt;

void XMLTextExportPropertySetMapper::handleElementItem(
        const Reference< xml::sax::XDocumentHandler > & rHandler,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
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
                                          sDropCharStyle, rNamespaceMap );
        pThis->bDropWholeWord = sal_False;
        pThis->sDropCharStyle = OUString();
        break;

    case CTF_TABSTOP:
        pThis->maTabStopExport.Export( rProperty.maValue, rNamespaceMap );
        break;

    case CTF_TEXTCOLUMNS:
        pThis->maTextColumnsExport.exportXML( rProperty.maValue );
        break;

    case CTF_BACKGROUND_URL:
        {
            DBG_ASSERT( pProperties && nIdx >= 2,
                        "property vector missing" );
            const Any *pPos = 0, *pFilter = 0;
            if( pProperties && nIdx >= 2 )
            {
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
                    rProperty.maValue, pPos, pFilter,
                    getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                    getPropertySetMapper()->GetEntryXMLName( nPropIndex ) );
        }
        break;

    default:
        DBG_ASSERT( !this, "unknown element property" );
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
        DBG_ASSERT( !sDropCharStyle.getLength(),
                    "drop char style is set already!" );
        rProperty.maValue >>= pThis->sDropCharStyle;
        break;
    case CTF_NUMBERINGSTYLENAME:
    case CTF_PAGEDESCNAME:
    case CTF_OLDTEXTBACKGROUND:
    case CTF_BACKGROUND_POS:
    case CTF_BACKGROUND_FILTER:
        // There's nothing to do here!
        break;
    default:
        DBG_ASSERT( !this, "unknown special property" );
        break;
    }
}

XMLTextExportPropertySetMapper::XMLTextExportPropertySetMapper(
        const UniReference< XMLPropertySetMapper >& rMapper,
        SvXMLExport& rExp ) :
    SvXMLExportPropertyMapper( rMapper ),
    rExport( rExp ),
    bDropWholeWord( sal_False ),
    maTabStopExport( rExp.GetDocHandler(), rExp.GetMM100UnitConverter() ),
    maDropCapExport( rExp.GetDocHandler(), rExp.GetMM100UnitConverter() ),
    maTextColumnsExport( rExp ),
    maBackgroundImageExport( rExp )
{
}

XMLTextExportPropertySetMapper::~XMLTextExportPropertySetMapper()
{
}

void XMLTextExportPropertySetMapper::ContextFontFilter(
    XMLPropertyState *pFontNameState,
    XMLPropertyState *pFontFamilyNameState,
    XMLPropertyState *pFontStyleNameState,
    XMLPropertyState *pFontFamilyState,
    XMLPropertyState *pFontPitchState,
    XMLPropertyState *pFontCharsetState ) const
{
    OUString sFamilyName;
    OUString sStyleName;
    sal_Int16 nFamily = FontFamily::DONTKNOW;
    sal_Int16 nPitch = FontPitch::DONTKNOW;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    OUString sTmp;
    if( pFontFamilyNameState && (pFontFamilyNameState->maValue >>= sTmp ) )
        sFamilyName = sTmp;
    if( pFontStyleNameState && (pFontStyleNameState->maValue >>= sTmp ) )
        sStyleName = sTmp;

    sal_Int16 nTmp;
    if( pFontFamilyState && (pFontFamilyState->maValue >>= nTmp ) )
        nFamily = nTmp;
    if( pFontPitchState && (pFontPitchState->maValue >>= nTmp ) )
        nPitch = nTmp;
    if( pFontCharsetState && (pFontCharsetState->maValue >>= nTmp ) )
        eEnc = (rtl_TextEncoding)nTmp;

    OUString sName( ((SvXMLExport&)GetExport()).GetFontAutoStylePool()->Find(
                        sFamilyName, sStyleName, nFamily, nPitch, eEnc ) );
    if( sName.getLength() )
    {
        pFontNameState->maValue <<= sName;
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
    else
    {
        pFontNameState->mnIndex = -1;
    }
}

void XMLTextExportPropertySetMapper::ContextFontHeightFilter(
    XMLPropertyState* pCharHeightState,
    XMLPropertyState* pCharPropHeightState,
    XMLPropertyState* pCharDiffHeightState ) const
{
    if( pCharPropHeightState )
    {
        sal_Int32 nTemp;
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
        float nTemp;
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

void XMLTextExportPropertySetMapper::ContextFilter(
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

    // filter width/height properties
    XMLPropertyState* pWidthAbsState = NULL;
    XMLPropertyState* pWidthMinAbsState = NULL;
    XMLPropertyState* pWidthRelState = NULL;
    XMLPropertyState* pWidthMinRelState = NULL;
    XMLPropertyState* pHeightAbsState = NULL;
    XMLPropertyState* pHeightMinAbsState = NULL;
    XMLPropertyState* pHeightRelState = NULL;
    XMLPropertyState* pHeightMinRelState = NULL;
    XMLPropertyState* pSizeTypeState = NULL;
    XMLPropertyState* pSyncHeightState = NULL;

    // wrap
    XMLPropertyState* pWrapState = NULL;
    XMLPropertyState* pWrapContourState = NULL;
    XMLPropertyState* pWrapContourModeState = NULL;
    XMLPropertyState* pWrapParagraphOnlyState = NULL;

    // anchor
    XMLPropertyState* pAnchorTypeState = NULL;
    XMLPropertyState* pAnchorPageNumberState = NULL;

    // horizontal position and relation
    XMLPropertyState* pHoriOrientState = NULL;
    XMLPropertyState* pHoriOrientMirroredState = NULL;
    XMLPropertyState* pHoriOrientRelState = NULL;
    XMLPropertyState* pHoriOrientRelFrameState = NULL;
    XMLPropertyState* pHoriOrientMirrorState = NULL;

    // vertical position and relation
    XMLPropertyState* pVertOrientState = NULL;
    XMLPropertyState* pVertOrientRelState = NULL;
    XMLPropertyState* pVertOrientRelPageState = NULL;
    XMLPropertyState* pVertOrientRelFrameState = NULL;
    XMLPropertyState* pVertOrientRelAsCharState = NULL;

    // filter underline color
    XMLPropertyState* pUnderlineState = NULL;
    XMLPropertyState* pUnderlineColorState = NULL;
    XMLPropertyState* pUnderlineHasColorState = NULL;

    sal_Bool bNeedsAnchor = sal_False;

    for( ::std::vector< XMLPropertyState >::iterator propertie = rProperties.begin();
         propertie != rProperties.end();
         propertie++ )
    {
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

        case CTF_FRAMEWIDTH_ABS:        pWidthAbsState = propertie; break;
        case CTF_FRAMEWIDTH_REL:        pWidthRelState = propertie; break;
        case CTF_FRAMEHEIGHT_ABS:       pHeightAbsState = propertie; break;
        case CTF_FRAMEHEIGHT_MIN_ABS:   pHeightMinAbsState = propertie; break;
        case CTF_FRAMEHEIGHT_REL:       pHeightRelState = propertie; break;
        case CTF_FRAMEHEIGHT_MIN_REL:   pHeightMinRelState = propertie; break;
        case CTF_SIZETYPE:              pSizeTypeState = propertie; break;
        case CTF_SYNCHEIGHT:            pSyncHeightState = propertie; break;

        case CTF_WRAP:                  pWrapState = propertie; break;
        case CTF_WRAP_CONTOUR:          pWrapContourState = propertie; break;
        case CTF_WRAP_CONTOUR_MODE:     pWrapContourModeState = propertie; break;
        case CTF_WRAP_PARAGRAPH_ONLY:   pWrapParagraphOnlyState = propertie; break;
        case CTF_ANCHORTYPE:            pAnchorTypeState = propertie; break;
        case CTF_ANCHORPAGENUMBER:      pAnchorPageNumberState = propertie; bNeedsAnchor = sal_True; break;

        case CTF_HORIZONTALPOS:             pHoriOrientState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_HORIZONTALPOS_MIRRORED:    pHoriOrientMirroredState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_HORIZONTALREL:             pHoriOrientRelState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_HORIZONTALREL_FRAME:       pHoriOrientRelFrameState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_HORIZONTALMIRROR:          pHoriOrientMirrorState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALPOS:           pVertOrientState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALREL:           pVertOrientRelState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALREL_PAGE:      pVertOrientRelPageState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALREL_FRAME:     pVertOrientRelFrameState = propertie; bNeedsAnchor = sal_True; break;
        case CTF_VERTICALREL_ASCHAR:    pVertOrientRelAsCharState = propertie; bNeedsAnchor = sal_True; break;

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
        }
    }

    if( pFontNameState )
        ContextFontFilter( pFontNameState, pFontFamilyNameState,
                           pFontStyleNameState, pFontFamilyState,
                           pFontPitchState, pFontCharsetState );
    if( pFontNameCJKState )
        ContextFontFilter( pFontNameCJKState, pFontFamilyNameCJKState,
                           pFontStyleNameCJKState, pFontFamilyCJKState,
                           pFontPitchCJKState, pFontCharsetCJKState );
    if( pFontNameCTLState )
        ContextFontFilter( pFontNameCTLState, pFontFamilyNameCTLState,
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
            sal_Int16 nUnderline;
            pUnderlineState->maValue >>= nUnderline;
            bClear = FontUnderline::NONE == nUnderline;
        }
        if( bClear )
        {
            if( pUnderlineColorState )
                pUnderlineColorState->mnIndex = -1;
            if( pUnderlineHasColorState )
                pUnderlineHasColorState->mnIndex = -1;
        }
    }

    if( pParaLeftMarginState && pParaLeftMarginRelState )
    {
        sal_Int32 nTemp;
        pParaLeftMarginRelState->maValue >>= nTemp;
        if( nTemp == 100 )
        {
            pParaLeftMarginRelState->mnIndex = -1;
            pParaLeftMarginRelState->maValue.clear();
        }
        else
        {
            pParaLeftMarginState->mnIndex = -1;
            pParaLeftMarginState->maValue.clear();
        }

    }

    if( pParaRightMarginState && pParaRightMarginRelState )
    {
        sal_Int32 nTemp;
        pParaRightMarginRelState->maValue >>= nTemp;
        if( nTemp == 100 )
        {
            pParaRightMarginRelState->mnIndex = -1;
            pParaRightMarginRelState->maValue.clear();
        }
        else
        {
            pParaRightMarginState->mnIndex = -1;
            pParaRightMarginState->maValue.clear();
        }
    }

    if( pParaFirstLineState && pParaFirstLineRelState )
    {
        sal_Int32 nTemp;
        pParaFirstLineRelState->maValue >>= nTemp;
        if( nTemp == 100 )
        {
            pParaFirstLineRelState->mnIndex = -1;
            pParaFirstLineRelState->maValue.clear();
        }
        else
        {
            pParaFirstLineState->mnIndex = -1;
            pParaFirstLineState->maValue.clear();
        }
    }

    if( pParaTopMarginState && pParaTopMarginRelState )
    {
        sal_Int32 nTemp;
        pParaTopMarginRelState->maValue >>= nTemp;
        if( nTemp == 100 )
        {
            pParaTopMarginRelState->mnIndex = -1;
            pParaTopMarginRelState->maValue.clear();
        }
        else
        {
            pParaTopMarginState->mnIndex = -1;
            pParaTopMarginState->maValue.clear();
        }

    }

    if( pParaBottomMarginState && pParaBottomMarginRelState )
    {
        sal_Int32 nTemp;
        pParaBottomMarginRelState->maValue >>= nTemp;
        if( nTemp == 100 )
        {
            pParaBottomMarginRelState->mnIndex = -1;
            pParaBottomMarginRelState->maValue.clear();
        }
        else
        {
            pParaBottomMarginState->mnIndex = -1;
            pParaBottomMarginState->maValue.clear();
        }

    }

    if( pAllBorderWidthState )
    {
        if( pLeftBorderWidthState && pRightBorderWidthState && pTopBorderWidthState && pBottomBorderWidthState )
        {
            table::BorderLine aLeft, aRight, aTop, aBottom;

            pLeftBorderWidthState->maValue >>= aLeft;
            pRightBorderWidthState->maValue >>= aRight;
            pTopBorderWidthState->maValue >>= aTop;
            pBottomBorderWidthState->maValue >>= aBottom;
            if( aLeft.Color == aRight.Color && aLeft.InnerLineWidth == aRight.InnerLineWidth &&
                aLeft.OuterLineWidth == aRight.OuterLineWidth && aLeft.LineDistance == aRight.LineDistance &&
                aLeft.Color == aTop.Color && aLeft.InnerLineWidth == aTop.InnerLineWidth &&
                aLeft.OuterLineWidth == aTop.OuterLineWidth && aLeft.LineDistance == aTop.LineDistance &&
                aLeft.Color == aBottom.Color && aLeft.InnerLineWidth == aBottom.InnerLineWidth &&
                aLeft.OuterLineWidth == aBottom.OuterLineWidth && aLeft.LineDistance == aBottom.LineDistance )
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
            sal_Int32 aLeft, aRight, aTop, aBottom;

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
            table::BorderLine aLeft, aRight, aTop, aBottom;

            pLeftBorderState->maValue >>= aLeft;
            pRightBorderState->maValue >>= aRight;
            pTopBorderState->maValue >>= aTop;
            pBottomBorderState->maValue >>= aBottom;
            if( aLeft.Color == aRight.Color && aLeft.InnerLineWidth == aRight.InnerLineWidth &&
                aLeft.OuterLineWidth == aRight.OuterLineWidth && aLeft.LineDistance == aRight.LineDistance &&
                aLeft.Color == aTop.Color && aLeft.InnerLineWidth == aTop.InnerLineWidth &&
                aLeft.OuterLineWidth == aTop.OuterLineWidth && aLeft.LineDistance == aTop.LineDistance &&
                aLeft.Color == aBottom.Color && aLeft.InnerLineWidth == aBottom.InnerLineWidth &&
                aLeft.OuterLineWidth == aBottom.OuterLineWidth && aLeft.LineDistance == aBottom.LineDistance )
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

    if( pWidthAbsState && pWidthRelState )
    {
        sal_Int16 nRelWidth =  0;
        pWidthRelState->maValue >>= nRelWidth;
        if( nRelWidth > 0 )
            pWidthAbsState->mnIndex = -1;
        // TODO: instead of checking this value for 255 a new property
        // must be introduced like for heights.
        if( nRelWidth == 255 )
            pWidthRelState->mnIndex = -1;
    }

    if( pHeightAbsState && pHeightRelState )
    {
        DBG_ASSERT( pHeightMinAbsState, "no min abs state" );
        DBG_ASSERT( pHeightMinRelState, "no min rel state" );
        sal_Int32 nSizeType = SizeType::FIX;
        if( pSizeTypeState )
            pSizeTypeState->maValue >>= nSizeType;

        if( SizeType::VARIABLE == nSizeType ||
            ( pSyncHeightState &&
              *(sal_Bool *)pSyncHeightState->maValue.getValue() ) )
        {
            pHeightAbsState->mnIndex = -1;
            pHeightMinAbsState->mnIndex = -1;
            pHeightRelState->mnIndex = -1;
            pHeightMinRelState->mnIndex = -1;
        }
        else
        {
            sal_Int16 nRelHeight =  0;
            pHeightRelState->maValue >>= nRelHeight;
            sal_Bool bRel = (nRelHeight > 0);
            sal_Bool bMin = (SizeType::MIN == nSizeType);
            if( bRel || bMin )
                pHeightAbsState->mnIndex = -1;
            if( bRel || !bMin )
                pHeightMinAbsState->mnIndex = -1;
            if( !bRel || bMin )
                pHeightRelState->mnIndex = -1;
            if( !bRel || !bMin )
                pHeightMinRelState->mnIndex = -1;
        }
    }
    if( pSizeTypeState )
        pSizeTypeState->mnIndex = -1;
    if( pSyncHeightState )
        pSyncHeightState->mnIndex = -1;

    if( pWrapState )
    {
        WrapTextMode eVal;
        pWrapState->maValue >>= eVal;
        switch( eVal )
        {
        case WrapTextMode_NONE:
        case WrapTextMode_THROUGHT:
            if( pWrapContourState )
                pWrapContourState->mnIndex = -1;
            if( pWrapParagraphOnlyState )
                pWrapParagraphOnlyState->mnIndex = -1;
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
        Any aAny = rPropSet->getPropertyValue(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AnchorType") ) );
        aAny >>= eAnchor;
    }

    if( pAnchorPageNumberState && TextContentAnchorType_AT_PAGE != eAnchor )
        pAnchorPageNumberState->mnIndex = -1;

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
        pHoriOrientRelFrameState->mnIndex = -1;;

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
