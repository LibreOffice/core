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

#include "hintids.hxx"
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outliner.hxx>
#include <svx/xfillit.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svx/svdpool.hxx>

#include <rtl/strbuf.hxx>

#include <IDocumentDrawModelAccess.hxx>
#include "charatr.hxx"
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtsrnd.hxx>
#include "ndtxt.hxx"
#include "doc.hxx"
#include "dcontact.hxx"
#include "poolfmt.hxx"

#include "wrthtml.hxx"


using namespace css;

const HtmlFrmOpts HTML_FRMOPTS_MARQUEE   =
    HtmlFrmOpts::Align |
    HtmlFrmOpts::Space;

const HtmlFrmOpts HTML_FRMOPTS_MARQUEE_CSS1  =
    HtmlFrmOpts::SAlign |
    HtmlFrmOpts::SSpace;

const SdrObject *SwHTMLWriter::GetMarqueeTextObj( const SwDrawFrameFormat& rFormat )
{
    const SdrObject* pObj = rFormat.FindSdrObject();
    return (pObj && ::IsMarqueeTextObj( *pObj )) ? pObj : nullptr;
}

void SwHTMLWriter::GetEEAttrsFromDrwObj( SfxItemSet& rItemSet,
                                         const SdrObject *pObj,
                                         bool bSetDefaults )
{
    // get the edit script::Engine attributes from object
    SfxItemSet rObjItemSet = pObj->GetMergedItemSet();

    // iterate over Edit script::Engine attributes and convert them
    // into SW-Attrs resp. set default
    SfxWhichIter aIter( rObjItemSet );
    sal_uInt16 nEEWhich = aIter.FirstWhich();
    while( nEEWhich )
    {
        const SfxPoolItem *pEEItem;
        bool bSet = SfxItemState::SET == rObjItemSet.GetItemState( nEEWhich, false,
                                                              &pEEItem );

        if( bSet || bSetDefaults )
        {
            sal_uInt16 nSwWhich = 0;
            switch( nEEWhich )
            {
            case EE_CHAR_COLOR:         nSwWhich = RES_CHRATR_COLOR;        break;
            case EE_CHAR_STRIKEOUT:     nSwWhich = RES_CHRATR_CROSSEDOUT;   break;
            case EE_CHAR_ESCAPEMENT:    nSwWhich = RES_CHRATR_ESCAPEMENT;   break;
            case EE_CHAR_FONTINFO:      nSwWhich = RES_CHRATR_FONT;         break;
            case EE_CHAR_FONTINFO_CJK:  nSwWhich = RES_CHRATR_CJK_FONT;     break;
            case EE_CHAR_FONTINFO_CTL:  nSwWhich = RES_CHRATR_CTL_FONT;     break;
            case EE_CHAR_FONTHEIGHT:    nSwWhich = RES_CHRATR_FONTSIZE;     break;
            case EE_CHAR_FONTHEIGHT_CJK:nSwWhich = RES_CHRATR_CJK_FONTSIZE; break;
            case EE_CHAR_FONTHEIGHT_CTL:nSwWhich = RES_CHRATR_CTL_FONTSIZE; break;
            case EE_CHAR_KERNING:       nSwWhich = RES_CHRATR_KERNING;      break;
            case EE_CHAR_ITALIC:        nSwWhich = RES_CHRATR_POSTURE;      break;
            case EE_CHAR_ITALIC_CJK:    nSwWhich = RES_CHRATR_CJK_POSTURE;  break;
            case EE_CHAR_ITALIC_CTL:    nSwWhich = RES_CHRATR_CTL_POSTURE;  break;
            case EE_CHAR_UNDERLINE:     nSwWhich = RES_CHRATR_UNDERLINE;    break;
            case EE_CHAR_WEIGHT:        nSwWhich = RES_CHRATR_WEIGHT;       break;
            case EE_CHAR_WEIGHT_CJK:    nSwWhich = RES_CHRATR_CJK_WEIGHT;   break;
            case EE_CHAR_WEIGHT_CTL:    nSwWhich = RES_CHRATR_CTL_WEIGHT;   break;
            }

            if( nSwWhich )
            {
                // if the item isn't set we maybe take the default item
                if( !bSet )
                    pEEItem = &rObjItemSet.GetPool()->GetDefaultItem(nEEWhich);

                // now we clone the item with the which id of the writer
                SfxPoolItem *pSwItem = pEEItem->Clone();
                pSwItem->SetWhich( nSwWhich );
                rItemSet.Put( *pSwItem );
                delete pSwItem;
            }
        }

        nEEWhich = aIter.NextWhich();
    }
}

Writer& OutHTML_DrawFrameFormatAsMarquee( Writer& rWrt,
                                     const SwDrawFrameFormat& rFormat,
                                     const SdrObject& rSdrObject )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    OSL_ENSURE( rWrt.pDoc->getIDocumentDrawModelAccess().GetDrawModel(),
            "There is a Draw-Obj with no Draw-Model?" );
    const SdrTextObj *pTextObj = static_cast<const SdrTextObj *>(&rSdrObject);

    // Is there text to output
    const OutlinerParaObject *pOutlinerParaObj =
        pTextObj->GetOutlinerParaObject();
    if( !pOutlinerParaObj )
        return rWrt;

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_marquee);

    // get attributes of the object
    const SfxItemSet& rItemSet = pTextObj->GetMergedItemSet();

    // BEHAVIOUR
    SdrTextAniKind eAniKind = pTextObj->GetTextAniKind();
    OSL_ENSURE( SdrTextAniKind::Scroll==eAniKind ||
            SdrTextAniKind::Alternate==eAniKind ||
            SdrTextAniKind::Slide==eAniKind,
            "Text-Draw-Object not suitable for marquee" );

    const sal_Char *pStr = nullptr;
    switch( eAniKind )
    {
    case SdrTextAniKind::Scroll:     pStr = OOO_STRING_SVTOOLS_HTML_BEHAV_scroll;        break;
    case SdrTextAniKind::Slide:      pStr = OOO_STRING_SVTOOLS_HTML_BEHAV_slide;     break;
    case SdrTextAniKind::Alternate:  pStr = OOO_STRING_SVTOOLS_HTML_BEHAV_alternate; break;
    default:
        ;
    }

    if( pStr )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_behavior).
            append("=\"").append(pStr).append("\"");
    }

    // DIRECTION
    pStr = nullptr;
    SdrTextAniDirection eAniDir = pTextObj->GetTextAniDirection();
    switch( eAniDir )
    {
    case SdrTextAniDirection::Left:       pStr = OOO_STRING_SVTOOLS_HTML_AL_left;     break;
    case SdrTextAniDirection::Right:      pStr = OOO_STRING_SVTOOLS_HTML_AL_right;        break;
    default:
        ;
    }

    if( pStr )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_direction).
            append("=\"").append(pStr).append("\"");
    }

    // LOOP
    sal_Int32 nCount =
        static_cast<const SdrTextAniCountItem&>(rItemSet.Get( SDRATTR_TEXT_ANICOUNT ))
                                             .GetValue();
    if( 0==nCount )
        nCount = SdrTextAniKind::Slide==eAniKind ? 1 : -1;
    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_loop).append("=\"").
        append(nCount).append("\"");

    // SCROLLDELAY
    sal_uInt16 nDelay =
        static_cast<const SdrTextAniDelayItem&>(rItemSet.Get( SDRATTR_TEXT_ANIDELAY ))
                                            .GetValue();
    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_scrolldelay).
        append("=\"").append(static_cast<sal_Int32>(nDelay)).append("\"");

    // SCROLLAMOUNT
    sal_Int16 nAmount =
        static_cast<const SdrTextAniAmountItem&>(rItemSet.Get( SDRATTR_TEXT_ANIAMOUNT ))
                                             .GetValue();
    if( nAmount < 0 )
    {
        nAmount = -nAmount;
    }
    else if( nAmount && Application::GetDefaultDevice() )
    {
        nAmount = Application::GetDefaultDevice()
                            ->LogicToPixel( Size(nAmount,0),
                                            MapMode(MapUnit::MapTwip) ).Width();
    }
    if( nAmount )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_scrollamount).
            append("=\"").append(static_cast<sal_Int32>(nAmount)).append("\"");
    }

    Size aTwipSz( pTextObj->GetLogicRect().GetSize() );
    if( pTextObj->IsAutoGrowWidth() )
        aTwipSz.Width() = 0;
    // The height is at MS a minimum height, therefore we output the minimum
    // height, if they exists. Because a minimum height MINFLY is coming with
    // high probability from import, we aren't outputting it. You can't
    // do anything wrong, because every font is higher.
    if( pTextObj->IsAutoGrowHeight() )
    {
        aTwipSz.Height() = pTextObj->GetMinTextFrameHeight();
        if( MINFLY==aTwipSz.Height() )
            aTwipSz.Height() = 0;
    }

    if( (aTwipSz.Width() || aTwipSz.Height()) &&
        Application::GetDefaultDevice() )
    {
        Size aPixelSz =
            Application::GetDefaultDevice()->LogicToPixel( aTwipSz,
                                                MapMode(MapUnit::MapTwip) );
        if( !aPixelSz.Width() && aTwipSz.Width() )
            aPixelSz.Width() = 1;
        if( !aPixelSz.Height() && aTwipSz.Height() )
            aPixelSz.Height() = 1;

        if( aPixelSz.Width() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).
                append("=\"").append(static_cast<sal_Int32>(aPixelSz.Width())).append("\"");
        }

        if( aPixelSz.Height() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_height).
                append("=\"").append(static_cast<sal_Int32>(aPixelSz.Height())).append("\"");
        }
    }

    // BGCOLOR
    drawing::FillStyle eFillStyle =
        static_cast<const XFillStyleItem&>(rItemSet.Get(XATTR_FILLSTYLE)).GetValue();
    if( drawing::FillStyle_SOLID==eFillStyle )
    {
        const Color& rFillColor =
            static_cast<const XFillColorItem&>(rItemSet.Get(XATTR_FILLCOLOR)).GetColorValue();

        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_bgcolor).append("=");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_Color( rWrt.Strm(), rFillColor );
    }

    if (!sOut.isEmpty())
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // and now ALIGN, HSPACE and VSPACE
    HtmlFrmOpts nFrameFlags = HTML_FRMOPTS_MARQUEE;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_DRAW ) )
        nFrameFlags |= HTML_FRMOPTS_MARQUEE_CSS1;
    OString aEndTags = rHTMLWrt.OutFrameFormatOptions( rFormat, aEmptyOUStr, nFrameFlags );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_DRAW ) )
        rHTMLWrt.OutCSS1_FrameFormatOptions( rFormat, nFrameFlags, &rSdrObject );

    rWrt.Strm().WriteChar( '>' );

    // What follows now is the counterpart of SdrTextObject::SetText()
    Outliner aOutliner(nullptr, OutlinerMode::TextObject);
    aOutliner.SetUpdateMode( false );
    aOutliner.SetText( *pOutlinerParaObj );
    OUString aText( aOutliner.GetText( aOutliner.GetParagraph(0),
                                     aOutliner.GetParagraphCount() ) );
    HTMLOutFuncs::Out_String( rWrt.Strm(), aText,
                                rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );

    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_marquee, false );

    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteCharPtr( aEndTags.getStr() );

    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
