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


#include <memory>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>

#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/frmdiritem.hxx>

#include "impedit.hxx"
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editdoc.hxx>

#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>

#if defined( DBG_UTIL ) || ( OSL_DEBUG_LEVEL > 1 )

namespace
{
struct DebOutBuffer
{
    OStringBuffer str;
    void append(std::string_view descr, const SfxEnumItemInterface& rItem)
    {
        str.append(descr + OString::number(rItem.GetEnumValue()));
    }
    void append(std::string_view descr, const SvxLRSpaceItem& rItem)
    {
        str.append(OString::Concat(descr) + "FI=" + OString::number(rItem.GetTextFirstLineOffset())
                   + ", LI=" + OString::number(rItem.GetTextLeft())
                   + ", RI=" + OString::number(rItem.GetRight()));
    }
    void append(std::string_view descr, const SvxNumBulletItem& rItem)
    {
        str.append(descr);
        for (sal_uInt16 nLevel = 0; nLevel < 3; nLevel++)
        {
            str.append("Level" + OString::number(nLevel) + "=");
            const SvxNumberFormat* pFmt = rItem.GetNumRule().Get(nLevel);
            if (pFmt)
            {
                str.append("(" + OString::number(pFmt->GetFirstLineOffset()) + ","
                           + OString::number(pFmt->GetAbsLSpace()) + ",");
                if (pFmt->GetNumberingType() == SVX_NUM_BITMAP)
                    str.append("Bitmap");
                else if (pFmt->GetNumberingType() != SVX_NUM_CHAR_SPECIAL)
                    str.append("Number");
                else
                {
                    str.append("Char=[" + OString::number(pFmt->GetBulletChar()) + "]");
                }
                str.append(") ");
            }
        }
    }
    void append(std::string_view descr, const SfxBoolItem& rItem)
    {
        str.append(descr + OString::number(static_cast<int>(rItem.GetValue())));
    }
    void append(std::string_view descr, const SfxInt16Item& rItem)
    {
        str.append(descr + OString::number(rItem.GetValue()));
    }
    void append(std::string_view descr, const SfxUInt16Item& rItem)
    {
        str.append(descr + OString::number(rItem.GetValue()));
    }
    void append(const SvxULSpaceItem& rItem)
    {
        str.append("SB=" + OString::number(rItem.GetUpper())
                   + ", SA=" + OString::number(rItem.GetLower()));
    }
    void append(std::string_view descr, const SvxLineSpacingItem& rItem)
    {
        str.append(descr);
        if (rItem.GetLineSpaceRule() == SvxLineSpaceRule::Min)
        {
            str.append("Min: " + OString::number(rItem.GetInterLineSpace()));
        }
        else if (rItem.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Prop)
        {
            str.append("Prop: " + OString::number(rItem.GetPropLineSpace()));
        }
        else
            str.append("Unsupported Type!");
    }
    void append(const SvxTabStopItem& rTabs)
    {
        str.append("Tabs: " + OString::number(rTabs.Count()));
        if (rTabs.Count())
        {
            str.append("( ");
            for (sal_uInt16 i = 0; i < rTabs.Count(); ++i)
            {
                const SvxTabStop& rTab = rTabs[i];
                str.append(OString::number(rTab.GetTabPos()) + " ");
            }
            str.append(')');
        }
    }
    void append(std::string_view descr, const SvxColorItem& rItem)
    {
        Color aColor(rItem.GetValue());
        str.append(descr + OString::number(aColor.GetRed()) + ", "
                   + OString::number(aColor.GetGreen()) + ", " + OString::number(aColor.GetBlue()));
    }
    void append(std::string_view descr, const SvxFontItem& rItem)
    {
        str.append(descr + OUStringToOString(rItem.GetFamilyName(), RTL_TEXTENCODING_ASCII_US)
                   + " (CharSet: " + OString::number(rItem.GetCharSet()) + ")");
    }
    void append(std::string_view descr, const SvxEscapementItem& rItem)
    {
        str.append(descr + OString::number(rItem.GetEsc()) + ", "
                   + OString::number(rItem.GetProportionalHeight()));
    }
    void appendHeightAndPts(std::string_view descr, tools::Long h, MapUnit eUnit)
    {
        MapMode aItemMapMode(eUnit);
        MapMode aPntMap(MapUnit::MapPoint);
        Size aSz = OutputDevice::LogicToLogic(Size(0, h), aItemMapMode, aPntMap);
        str.append(descr + OString::number(h) + " Points=" + OString::number(aSz.Height()));
    }
    void append(std::string_view descr, const SvxFontHeightItem& rItem, const SfxItemPool& rPool)
    {
        appendHeightAndPts(descr, rItem.GetHeight(), rPool.GetMetric(rItem.Which()));
    }
    void append(std::string_view descr, const SvxKerningItem& rItem, const SfxItemPool& rPool)
    {
        appendHeightAndPts(descr, rItem.GetValue(), rPool.GetMetric(rItem.Which()));
    }
};
}

static OString DbgOutItem(const SfxItemPool& rPool, const SfxPoolItem& rItem)
{
    DebOutBuffer buffer;
    switch ( rItem.Which() )
    {
        case EE_PARA_WRITINGDIR:
            buffer.append("WritingDir=", rItem.StaticWhichCast(EE_PARA_WRITINGDIR));
        break;
        case EE_PARA_OUTLLRSPACE:
            buffer.append("Outline ", rItem.StaticWhichCast(EE_PARA_OUTLLRSPACE));
        break;
        case EE_PARA_LRSPACE:
            buffer.append("", rItem.StaticWhichCast(EE_PARA_LRSPACE));
        break;
        case EE_PARA_NUMBULLET:
            buffer.append("NumItem ", rItem.StaticWhichCast(EE_PARA_NUMBULLET));
        break;
        case EE_PARA_BULLETSTATE:
            buffer.append("ShowBullet=", rItem.StaticWhichCast(EE_PARA_BULLETSTATE));
        break;
        case EE_PARA_HYPHENATE:
            buffer.append("Hyphenate=", rItem.StaticWhichCast(EE_PARA_HYPHENATE));
        break;
        case EE_PARA_OUTLLEVEL:
            buffer.append("Level=", rItem.StaticWhichCast(EE_PARA_OUTLLEVEL));
        break;
        case EE_PARA_ULSPACE:
            buffer.append(rItem.StaticWhichCast(EE_PARA_ULSPACE));
        break;
        case EE_PARA_SBL:
            buffer.append("SBL=", rItem.StaticWhichCast(EE_PARA_SBL));
        break;
        case EE_PARA_JUST:
            buffer.append("SvxAdust=", rItem.StaticWhichCast(EE_PARA_JUST));
        break;
        case EE_PARA_TABS:
            buffer.append(rItem.StaticWhichCast(EE_PARA_TABS));
        break;
        case EE_CHAR_LANGUAGE:
            buffer.append("Language=", rItem.StaticWhichCast(EE_CHAR_LANGUAGE));
        break;
        case EE_CHAR_LANGUAGE_CJK:
            buffer.append("LanguageCJK=", rItem.StaticWhichCast(EE_CHAR_LANGUAGE_CJK));
        break;
        case EE_CHAR_LANGUAGE_CTL:
            buffer.append("LanguageCTL=", rItem.StaticWhichCast(EE_CHAR_LANGUAGE_CTL));
        break;
        case EE_CHAR_COLOR:
            buffer.append("Color= ", rItem.StaticWhichCast(EE_CHAR_COLOR));
        break;
        case EE_CHAR_BKGCOLOR:
            buffer.append("FillColor= ", rItem.StaticWhichCast(EE_CHAR_BKGCOLOR));
        break;
        case EE_CHAR_FONTINFO:
            buffer.append("Font=", rItem.StaticWhichCast(EE_CHAR_FONTINFO));
        break;
        case EE_CHAR_FONTINFO_CJK:
            buffer.append("FontCJK=", rItem.StaticWhichCast(EE_CHAR_FONTINFO_CJK));
        break;
        case EE_CHAR_FONTINFO_CTL:
            buffer.append("FontCTL=", rItem.StaticWhichCast(EE_CHAR_FONTINFO_CTL));
        break;
        case EE_CHAR_FONTHEIGHT:
            buffer.append("Size=", rItem.StaticWhichCast(EE_CHAR_FONTHEIGHT), rPool);
        break;
        case EE_CHAR_FONTHEIGHT_CJK:
            buffer.append("SizeCJK=", rItem.StaticWhichCast(EE_CHAR_FONTHEIGHT_CJK), rPool);
        break;
        case EE_CHAR_FONTHEIGHT_CTL:
            buffer.append("SizeCTL=", rItem.StaticWhichCast(EE_CHAR_FONTHEIGHT_CTL), rPool);
        break;
        case EE_CHAR_FONTWIDTH:
            buffer.append("Width=", rItem.StaticWhichCast(EE_CHAR_FONTWIDTH));
        break;
        case EE_CHAR_WEIGHT:
            buffer.append("FontWeight=", rItem.StaticWhichCast(EE_CHAR_WEIGHT));
        break;
        case EE_CHAR_WEIGHT_CJK:
            buffer.append("FontWeightCJK=", rItem.StaticWhichCast(EE_CHAR_WEIGHT_CJK));
        break;
        case EE_CHAR_WEIGHT_CTL:
            buffer.append("FontWeightCTL=", rItem.StaticWhichCast(EE_CHAR_WEIGHT_CTL));
        break;
        case EE_CHAR_UNDERLINE:
            buffer.append("FontUnderline=", rItem.StaticWhichCast(EE_CHAR_UNDERLINE));
        break;
        case EE_CHAR_OVERLINE:
            buffer.append("FontOverline=", rItem.StaticWhichCast(EE_CHAR_OVERLINE));
        break;
        case EE_CHAR_EMPHASISMARK:
            buffer.append("FontEmphasisMark=", rItem.StaticWhichCast(EE_CHAR_EMPHASISMARK));
        break;
        case EE_CHAR_RELIEF:
            buffer.append("FontRelief=", rItem.StaticWhichCast(EE_CHAR_RELIEF));
        break;
        case EE_CHAR_STRIKEOUT:
            buffer.append("FontStrikeout=", rItem.StaticWhichCast(EE_CHAR_STRIKEOUT));
        break;
        case EE_CHAR_ITALIC:
            buffer.append("FontPosture=", rItem.StaticWhichCast(EE_CHAR_ITALIC));
        break;
        case EE_CHAR_ITALIC_CJK:
            buffer.append("FontPostureCJK=", rItem.StaticWhichCast(EE_CHAR_ITALIC_CJK));
        break;
        case EE_CHAR_ITALIC_CTL:
            buffer.append("FontPostureCTL=", rItem.StaticWhichCast(EE_CHAR_ITALIC_CTL));
        break;
        case EE_CHAR_OUTLINE:
            buffer.append("FontOutline=", rItem.StaticWhichCast(EE_CHAR_OUTLINE));
        break;
        case EE_CHAR_SHADOW:
            buffer.append("FontShadowed=", rItem.StaticWhichCast(EE_CHAR_SHADOW));
        break;
        case EE_CHAR_ESCAPEMENT:
            buffer.append("Escape=", rItem.StaticWhichCast(EE_CHAR_ESCAPEMENT));
        break;
        case EE_CHAR_PAIRKERNING:
            buffer.append("PairKerning=", rItem.StaticWhichCast(EE_CHAR_PAIRKERNING));
        break;
        case EE_CHAR_KERNING:
            buffer.append("Kerning=", rItem.StaticWhichCast(EE_CHAR_KERNING), rPool);
        break;
        case EE_CHAR_WLM:
            buffer.append("WordLineMode=", rItem.StaticWhichCast(EE_CHAR_WLM));
        break;
        case EE_CHAR_XMLATTRIBS:
            buffer.str.append("XMLAttribs=...");
        break;
    }
    return buffer.str.makeStringAndClear();
}

static void DbgOutItemSet(FILE* fp, const SfxItemSet& rSet, bool bSearchInParent, bool bShowALL)
{
    for ( sal_uInt16 nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        fprintf( fp, "\nWhich: %i\t", nWhich );
        if ( rSet.GetItemState( nWhich, bSearchInParent ) == SfxItemState::DEFAULT )
            fprintf( fp, "ITEM_OFF   " );
        else if ( rSet.GetItemState( nWhich, bSearchInParent ) == SfxItemState::INVALID )
            fprintf( fp, "ITEM_DC    " );
        else if ( rSet.GetItemState( nWhich, bSearchInParent ) == SfxItemState::SET )
            fprintf( fp, "ITEM_ON   *" );

        if ( !bShowALL && ( rSet.GetItemState( nWhich, bSearchInParent ) != SfxItemState::SET ) )
            continue;

        const SfxPoolItem& rItem = rSet.Get( nWhich, bSearchInParent );
        OString aDebStr = DbgOutItem( *rSet.GetPool(), rItem );
        fprintf( fp, "%s", aDebStr.getStr() );
    }
}

void EditEngine::DumpData(const EditEngine* pEE, bool bInfoBox)
{
    if (!pEE)
        return;

    FILE* fp = fopen( "editenginedump.log", "w" );
    if ( fp == nullptr )
    {
        OSL_FAIL( "Log file could not be created!" );
        return;
    }

    const SfxItemPool& rPool = *pEE->GetEmptyItemSet().GetPool();

    fprintf( fp, "================================================================================" );
    fprintf( fp, "\n==================   Document   ================================================" );
    fprintf( fp, "\n================================================================================" );
    for ( sal_Int32 nPortion = 0; nPortion < pEE->getImpl().GetParaPortions().Count(); nPortion++)
    {
        ParaPortion const& rPPortion = pEE->getImpl().GetParaPortions().getRef(nPortion);
        fprintf( fp, "\nParagraph %" SAL_PRIdINT32 ": Length = %" SAL_PRIdINT32 ", Invalid = %i\nText = '%s'",
                 nPortion, rPPortion.GetNode()->Len(), rPPortion.IsInvalid(),
                 OUStringToOString(rPPortion.GetNode()->GetString(), RTL_TEXTENCODING_UTF8).getStr() );
        fprintf( fp, "\nVorlage:" );
        SfxStyleSheet* pStyle = rPPortion.GetNode()->GetStyleSheet();
        if ( pStyle )
            fprintf( fp, " %s", OUStringToOString( pStyle->GetName(), RTL_TEXTENCODING_UTF8).getStr() );
        fprintf( fp, "\nParagraph attribute:" );
        DbgOutItemSet( fp, rPPortion.GetNode()->GetContentAttribs().GetItems(), false, false );

        fprintf( fp, "\nCharacter attribute:" );
        bool bZeroAttr = false;
        for ( sal_Int32 z = 0; z < rPPortion.GetNode()->GetCharAttribs().Count(); ++z )
        {
            const std::unique_ptr<EditCharAttrib>& rAttr = rPPortion.GetNode()->GetCharAttribs().GetAttribs()[z];
            OString aCharAttribs =
                "\nA"
                + OString::number(nPortion)
                + ":  "
                + OString::number(rAttr->GetItem()->Which())
                + "\t"
                + OString::number(rAttr->GetStart())
                + "\t"
                + OString::number(rAttr->GetEnd());
            if ( rAttr->IsEmpty() )
                bZeroAttr = true;
            fprintf(fp, "%s => ", aCharAttribs.getStr());

            OString aDebStr = DbgOutItem( rPool, *rAttr->GetItem() );
            fprintf( fp, "%s", aDebStr.getStr() );
        }
        if ( bZeroAttr )
            fprintf( fp, "\nNULL-Attribute!" );

        const sal_Int32 nTextPortions = rPPortion.GetTextPortions().Count();
        OStringBuffer aPortionStr("\nText portions: #"
            + OString::number(nTextPortions)
            + " \nA"
            + OString::number(nPortion)
            + ": Paragraph Length = "
            + OString::number(rPPortion.GetNode()->Len())
            + "\nA"
            + OString::number(nPortion)
            + ": ");
        sal_Int32 n = 0;
        for ( sal_Int32 z = 0; z < nTextPortions; ++z )
        {
            TextPortion const& rPortion = rPPortion.GetTextPortions()[z];
            aPortionStr.append(" "
                + OString::number(rPortion.GetLen())
                + "("
                + OString::number(rPortion.GetSize().Width())
                + ")"
                "["
                + OString::number(static_cast<sal_Int32>(rPortion.GetKind()))
                + "];");
            n += rPortion.GetLen();
        }
        aPortionStr.append("\nA"
            + OString::number(nPortion)
            + ": Total length: "
            + OString::number(n));
        if ( rPPortion.GetNode()->Len() != n )
            aPortionStr.append(" => Error !!!");
        fprintf(fp, "%s", aPortionStr.getStr());

        fprintf( fp, "\n\nLines:" );
        // First the content ...
        for ( sal_Int32 nLine = 0; nLine < rPPortion.GetLines().Count(); nLine++ )
        {
            EditLine const& rLine = rPPortion.GetLines()[nLine];

            OString aLine(OUStringToOString(rPPortion.GetNode()->Copy(rLine.GetStart(), rLine.GetEnd() - rLine.GetStart()), RTL_TEXTENCODING_ASCII_US));
            fprintf( fp, "\nLine %" SAL_PRIdINT32 "\t>%s<", nLine, aLine.getStr() );
        }
        // then the internal data ...
        for ( sal_Int32 nLine = 0; nLine < rPPortion.GetLines().Count(); nLine++ )
        {
            EditLine const& rLine = rPPortion.GetLines()[nLine];
            fprintf( fp, "\nLine %" SAL_PRIdINT32 ":\tStart: %" SAL_PRIdINT32 ",\tEnd: %" SAL_PRIdINT32, nLine, rLine.GetStart(), rLine.GetEnd() );
            fprintf( fp, "\t\tPortions: %" SAL_PRIdINT32 " - %" SAL_PRIdINT32 ".\tHight: %i, Ascent=%i", rLine.GetStartPortion(), rLine.GetEndPortion(), rLine.GetHeight(), rLine.GetMaxAscent() );
        }

        fprintf( fp, "\n-----------------------------------------------------------------------------" );
    }

    if (pEE->getImpl().GetStyleSheetPool())
    {
        SfxStyleSheetIterator aIter(pEE->getImpl().GetStyleSheetPool(), SfxStyleFamily::All);
        sal_uInt16 nStyles = aIter.Count();
        fprintf( fp, "\n\n================================================================================" );
        fprintf( fp, "\n==================   Stylesheets   =============================================" );
        fprintf( fp, "\n================================================================================" );
        fprintf( fp, "\n#Template:   %" SAL_PRIuUINT32 "\n", sal_uInt32(nStyles) );
        SfxStyleSheetBase* pStyle = aIter.First();
        while ( pStyle )
        {
            fprintf( fp, "\nTemplate:   %s", OUStringToOString( pStyle->GetName(), RTL_TEXTENCODING_ASCII_US ).getStr() );
            fprintf( fp, "\nParent:    %s", OUStringToOString( pStyle->GetParent(), RTL_TEXTENCODING_ASCII_US ).getStr() );
            fprintf( fp, "\nFollow:    %s", OUStringToOString( pStyle->GetFollow(), RTL_TEXTENCODING_ASCII_US ).getStr() );
            DbgOutItemSet( fp, pStyle->GetItemSet(), false, false );
            fprintf( fp, "\n----------------------------------" );

            pStyle = aIter.Next();
        }
    }

    fprintf( fp, "\n\n================================================================================" );
    fprintf( fp, "\n==================   Defaults   ================================================" );
    fprintf( fp, "\n================================================================================" );
    DbgOutItemSet(fp, pEE->getImpl().GetEmptyItemSet(), true, true);

    fprintf( fp, "\n\n================================================================================" );
    fprintf( fp, "\n==================   EditEngine & Views   ======================================" );
    fprintf( fp, "\n================================================================================" );
    fprintf( fp, "\nControl: %x", unsigned( pEE->GetControlWord() ) );
    fprintf( fp, "\nRefMapMode: %i", int( pEE->getImpl().mpRefDev->GetMapMode().GetMapUnit()));
    fprintf( fp, "\nPaperSize: %" SAL_PRIdINT64 " x %" SAL_PRIdINT64, sal_Int64(pEE->GetPaperSize().Width()), sal_Int64(pEE->GetPaperSize().Height()) );
    fprintf( fp, "\nMaxAutoPaperSize: %" SAL_PRIdINT64 " x %" SAL_PRIdINT64, sal_Int64(pEE->GetMaxAutoPaperSize().Width()), sal_Int64(pEE->GetMaxAutoPaperSize().Height()) );
    fprintf( fp, "\nMinAutoPaperSize: %" SAL_PRIdINT64 " x %" SAL_PRIdINT64 , sal_Int64(pEE->GetMinAutoPaperSize().Width()), sal_Int64(pEE->GetMinAutoPaperSize().Height()) );
    fprintf( fp, "\nCalculateLayout: %i", pEE->IsUpdateLayout() );
    fprintf( fp, "\nNumber of Views: %" SAL_PRI_SIZET "i", pEE->GetViewCount() );
    for ( size_t nView = 0; nView < pEE->GetViewCount(); nView++ )
    {
        EditView* pV = pEE->GetView( nView );
        assert(pV && "View not found!");
        fprintf( fp, "\nView %zu: Focus=%i", nView, pV->GetWindow()->HasFocus() );
        tools::Rectangle aR( pV->GetOutputArea() );
        fprintf( fp, "\n  OutputArea: nX=%" SAL_PRIdINT64 ", nY=%" SAL_PRIdINT64 ", dX=%" SAL_PRIdINT64 ", dY=%" SAL_PRIdINT64 ", MapMode = %i",
            sal_Int64(aR.Left()), sal_Int64(aR.Top()), sal_Int64(aR.GetSize().Width()), sal_Int64(aR.GetSize().Height()) , int( pV->GetWindow()->GetMapMode().GetMapUnit() ) );
        aR = pV->GetVisArea();
        fprintf( fp, "\n  VisArea: nX=%" SAL_PRIdINT64 ", nY=%" SAL_PRIdINT64 ", dX=%" SAL_PRIdINT64 ", dY=%" SAL_PRIdINT64,
            sal_Int64(aR.Left()), sal_Int64(aR.Top()), sal_Int64(aR.GetSize().Width()), sal_Int64(aR.GetSize().Height()) );
        ESelection aSel = pV->GetSelection();
        fprintf( fp, "\n  Selection: Start=%" SAL_PRIdINT32 ",%" SAL_PRIdINT32 ", End=%" SAL_PRIdINT32 ",%" SAL_PRIdINT32, aSel.nStartPara, aSel.nStartPos, aSel.nEndPara, aSel.nEndPos );
    }
    if ( pEE->GetActiveView() )
    {
        fprintf( fp, "\n\n================================================================================" );
        fprintf( fp, "\n==================   Current View   ===========================================" );
        fprintf( fp, "\n================================================================================" );
        DbgOutItemSet( fp, pEE->GetActiveView()->GetAttribs(), true, false );
    }
    fclose( fp );
    if ( bInfoBox )
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(nullptr,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      u"Dumped editenginedump.log!"_ustr ));
        xInfoBox->run();
    }
}
#endif

#if OSL_DEBUG_LEVEL > 0
bool ParaPortion::DbgCheckTextPortions(ParaPortion const& rPara)
{
    // check, if Portion length ok:
    sal_uInt16 nXLen = 0;

    for (sal_Int32 nPortion = 0; nPortion < rPara.maTextPortionList.Count(); nPortion++)
    {
        nXLen = nXLen + rPara.maTextPortionList[nPortion].GetLen();
    }
    return nXLen == rPara.mpNode->Len();
}
#endif

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
void CheckOrderedList(const CharAttribList::AttribsType& rAttribs)
{
    sal_Int32 nPrev = 0;
    for (const std::unique_ptr<EditCharAttrib>& rAttr : rAttribs)
    {
        sal_Int32 const nCur = rAttr->GetStart();
        assert(nCur >= nPrev);
        nPrev = nCur;
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
