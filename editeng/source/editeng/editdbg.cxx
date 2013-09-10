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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

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

#include <impedit.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editdoc.hxx>
#include <editdbg.hxx>

#include <rtl/strbuf.hxx>

#if defined( DBG_UTIL ) || ( OSL_DEBUG_LEVEL > 1 )

OString DbgOutItem(const SfxItemPool& rPool, const SfxPoolItem& rItem)
{
    OStringBuffer aDebStr;
    switch ( rItem.Which() )
    {
        case EE_PARA_WRITINGDIR:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("WritingDir="));
            aDebStr.append(static_cast<sal_Int32>(((SvxFrameDirectionItem&)rItem).GetValue()));
        break;
        case EE_PARA_OUTLLRSPACE:
        case EE_PARA_LRSPACE:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FI="));
            aDebStr.append(static_cast<sal_Int32>(((SvxLRSpaceItem&)rItem).GetTxtFirstLineOfst()));
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(", LI="));
            aDebStr.append(static_cast<sal_Int32>(((SvxLRSpaceItem&)rItem).GetTxtLeft()));
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(", RI="));
            aDebStr.append(static_cast<sal_Int32>(((SvxLRSpaceItem&)rItem).GetRight()));
        break;
        case EE_PARA_NUMBULLET:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("NumItem "));
            for ( sal_uInt16 nLevel = 0; nLevel < 3; nLevel++ )
            {
                aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Level"));
                aDebStr.append(static_cast<sal_Int32>(nLevel));
                aDebStr.append('=');
                const SvxNumberFormat* pFmt = ((const SvxNumBulletItem&)rItem).GetNumRule()->Get( nLevel );
                if ( pFmt )
                {
                    aDebStr.append('(');
                    aDebStr.append(static_cast<sal_Int32>(pFmt->GetFirstLineOffset()));
                    aDebStr.append(',');
                    aDebStr.append(static_cast<sal_Int32>(pFmt->GetAbsLSpace()));
                    aDebStr.append(',');
                    if ( pFmt->GetNumberingType() == SVX_NUM_BITMAP )
                        aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Bitmap"));
                    else if( pFmt->GetNumberingType() != SVX_NUM_CHAR_SPECIAL )
                        aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Number"));
                    else
                    {
                        aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Char=["));
                        aDebStr.append(static_cast<sal_Int32>(pFmt->GetBulletChar()));
                        aDebStr.append(']');
                    }
                    aDebStr.append(RTL_CONSTASCII_STRINGPARAM(") "));
                }
            }
        break;
        case EE_PARA_BULLETSTATE:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("ShowBullet="));
            aDebStr.append(static_cast<sal_Int32>(((SfxBoolItem&)rItem).GetValue()));
        break;
        case EE_PARA_HYPHENATE:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Hyphenate="));
            aDebStr.append(static_cast<sal_Int32>(((SfxBoolItem&)rItem).GetValue()));
        break;
        case EE_PARA_OUTLLEVEL:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Level="));
            aDebStr.append(static_cast<sal_Int32>(((SfxInt16Item&)rItem).GetValue()));
        break;
        case EE_PARA_ULSPACE:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("SB="));
            aDebStr.append(static_cast<sal_Int32>(((SvxULSpaceItem&)rItem).GetUpper()));
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(", SA="));
            aDebStr.append(static_cast<sal_Int32>(((SvxULSpaceItem&)rItem).GetLower()));
        break;
        case EE_PARA_SBL:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("SBL="));
            if ( ((SvxLineSpacingItem&)rItem).GetLineSpaceRule() == SVX_LINE_SPACE_MIN )
            {
                aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Min: "));
                aDebStr.append(static_cast<sal_Int32>(((SvxLineSpacingItem&)rItem).GetInterLineSpace()));
            }
            else if ( ((SvxLineSpacingItem&)rItem).GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Prop: "));
                aDebStr.append(static_cast<sal_Int32>(((SvxLineSpacingItem&)rItem).GetPropLineSpace()));
            }
            else
                aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Unsupported Type!"));
        break;
        case EE_PARA_JUST:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("SvxAdust="));
            aDebStr.append(static_cast<sal_Int32>(((SvxAdjustItem&)rItem).GetAdjust()));
        break;
        case EE_PARA_TABS:
        {
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Tabs: "));
            const SvxTabStopItem& rTabs = (const SvxTabStopItem&) rItem;
            aDebStr.append(static_cast<sal_Int32>(rTabs.Count()));
            if ( rTabs.Count() )
            {
                aDebStr.append(RTL_CONSTASCII_STRINGPARAM("( "));
                for (sal_uInt16 i = 0; i < rTabs.Count(); ++i)
                {
                    const SvxTabStop& rTab = rTabs[i];
                    aDebStr.append(static_cast<sal_Int32>(rTab.GetTabPos()));
                    aDebStr.append(' ');
                }
                aDebStr.append(')');
            }
        }
        break;
        case EE_CHAR_LANGUAGE:
        case EE_CHAR_LANGUAGE_CJK:
        case EE_CHAR_LANGUAGE_CTL:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Language="));
            aDebStr.append(static_cast<sal_Int32>(((SvxLanguageItem&)rItem).GetLanguage()));
        break;
        case EE_CHAR_COLOR:
        {
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Color= "));
            Color aColor( ((SvxColorItem&)rItem).GetValue() );
            aDebStr.append(static_cast<sal_Int32>(aColor.GetRed()));
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(", "));
            aDebStr.append(static_cast<sal_Int32>(aColor.GetGreen()));
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(", "));
            aDebStr.append(static_cast<sal_Int32>(aColor.GetBlue()));
        }
        break;
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Font="));
            aDebStr.append(OUStringToOString(((SvxFontItem&)rItem).GetFamilyName(), RTL_TEXTENCODING_ASCII_US));
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(" (CharSet: "));
            aDebStr.append(static_cast<sal_Int32>(((SvxFontItem&)rItem).GetCharSet()));
            aDebStr.append(')');
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Groesse="));
            aDebStr.append(static_cast<sal_Int32>(((SvxFontHeightItem&)rItem).GetHeight()));
            Size aSz( 0, ((SvxFontHeightItem&)rItem).GetHeight() );
            SfxMapUnit eUnit = rPool.GetMetric( rItem.Which() );
            MapMode aItemMapMode( (MapUnit) eUnit );
            MapMode aPntMap( MAP_POINT );
            aSz = OutputDevice::LogicToLogic( aSz, aItemMapMode, aPntMap );
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(" Points="));
            aDebStr.append(static_cast<sal_Int32>(aSz.Height()));
        }
        break;
        case EE_CHAR_FONTWIDTH:
        {
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Breite="));
            aDebStr.append(static_cast<sal_Int32>(((SvxCharScaleWidthItem&)rItem).GetValue()));
            aDebStr.append('%');
        }
        break;
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontWeight="));
            aDebStr.append(static_cast<sal_Int32>(((SvxWeightItem&)rItem).GetWeight()));
        break;
        case EE_CHAR_UNDERLINE:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontUnderline="));
            aDebStr.append(static_cast<sal_Int32>(((SvxUnderlineItem&)rItem).GetLineStyle()));
        break;
        case EE_CHAR_OVERLINE:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontOverline="));
            aDebStr.append(static_cast<sal_Int32>(((SvxOverlineItem&)rItem).GetLineStyle()));
        break;
        case EE_CHAR_EMPHASISMARK:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontUnderline="));
            aDebStr.append(static_cast<sal_Int32>(((SvxEmphasisMarkItem&)rItem).GetEmphasisMark()));
        break;
        case EE_CHAR_RELIEF:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontRelief="));
            aDebStr.append(static_cast<sal_Int32>(((SvxCharReliefItem&)rItem).GetValue()));
        break;
        case EE_CHAR_STRIKEOUT:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontStrikeout="));
            aDebStr.append(static_cast<sal_Int32>(((SvxCrossedOutItem&)rItem).GetStrikeout()));
        break;
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontPosture="));
            aDebStr.append(static_cast<sal_Int32>(((SvxPostureItem&)rItem).GetPosture()));
        break;
        case EE_CHAR_OUTLINE:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontOutline="));
            aDebStr.append(static_cast<sal_Int32>(((SvxContourItem&)rItem).GetValue()));
        break;
        case EE_CHAR_SHADOW:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("FontShadowed="));
            aDebStr.append(static_cast<sal_Int32>(((SvxShadowedItem&)rItem).GetValue()));
        break;
        case EE_CHAR_ESCAPEMENT:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Escape="));
            aDebStr.append(static_cast<sal_Int32>(((SvxEscapementItem&)rItem).GetEsc()));
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(", "));
            aDebStr.append(static_cast<sal_Int32>(((SvxEscapementItem&)rItem).GetProp()));
        break;
        case EE_CHAR_PAIRKERNING:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("PairKerning="));
            aDebStr.append(static_cast<sal_Int32>(((SvxAutoKernItem&)rItem).GetValue()));
        break;
        case EE_CHAR_KERNING:
        {
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("Kerning="));
            aDebStr.append(static_cast<sal_Int32>(((SvxKerningItem&)rItem).GetValue()));
            Size aSz( 0, (short)((SvxKerningItem&)rItem).GetValue() );
            SfxMapUnit eUnit = rPool.GetMetric( rItem.Which() );
            MapMode aItemMapMode( (MapUnit) eUnit );
            MapMode aPntMap( MAP_POINT );
            aSz = OutputDevice::LogicToLogic( aSz, aItemMapMode, aPntMap );
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM(" Points="));
            aDebStr.append(static_cast<sal_Int32>(aSz.Height()));
        }
        break;
        case EE_CHAR_WLM:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("WordLineMode="));
            aDebStr.append(static_cast<sal_Int32>(((SvxWordLineModeItem&)rItem).GetValue()));
        break;
        case EE_CHAR_XMLATTRIBS:
            aDebStr.append(RTL_CONSTASCII_STRINGPARAM("XMLAttribs=..."));
        break;
    }
    return aDebStr.makeStringAndClear();
}

void DbgOutItemSet( FILE* fp, const SfxItemSet& rSet, sal_Bool bSearchInParent, sal_Bool bShowALL )
{
    for ( sal_uInt16 nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        fprintf( fp, "\nWhich: %i\t", nWhich );
        if ( rSet.GetItemState( nWhich, bSearchInParent ) == SFX_ITEM_OFF )
            fprintf( fp, "ITEM_OFF   " );
        else if ( rSet.GetItemState( nWhich, bSearchInParent ) == SFX_ITEM_DONTCARE )
            fprintf( fp, "ITEM_DC    " );
        else if ( rSet.GetItemState( nWhich, bSearchInParent ) == SFX_ITEM_ON )
            fprintf( fp, "ITEM_ON   *" );

        if ( !bShowALL && ( rSet.GetItemState( nWhich, bSearchInParent ) != SFX_ITEM_ON ) )
            continue;

        const SfxPoolItem& rItem = rSet.Get( nWhich, bSearchInParent );
        OString aDebStr = DbgOutItem( *rSet.GetPool(), rItem );
        fprintf( fp, "%s", aDebStr.getStr() );
    }
}

void EditDbg::ShowEditEngineData( EditEngine* pEE, sal_Bool bInfoBox )
{
#if defined UNX
    FILE* fp = fopen( "/tmp/debug.log", "w" );
#else
    FILE* fp = fopen( "d:\\debug.log", "w" );
#endif
    if ( fp == 0 )
    {
        OSL_FAIL( "Log file could not be created!" );
        return;
    }

    const SfxItemPool& rPool = *pEE->GetEmptyItemSet().GetPool();

    fprintf( fp, "================================================================================" );
    fprintf( fp, "\n==================   Document   ================================================" );
    fprintf( fp, "\n================================================================================" );
    for ( sal_Int32 nPortion = 0; nPortion < pEE->pImpEditEngine->GetParaPortions().Count(); nPortion++)
    {
        ParaPortion* pPPortion = pEE->pImpEditEngine->GetParaPortions()[nPortion];
        fprintf( fp, "\nParagraph %" SAL_PRIdINT32 ": Length = %i, Invalid = %i\nText = '%s'",
                 nPortion, pPPortion->GetNode()->Len(), pPPortion->IsInvalid(),
                 OUStringToOString(pPPortion->GetNode()->GetString(), RTL_TEXTENCODING_UTF8).getStr() );
        fprintf( fp, "\nVorlage:" );
        SfxStyleSheet* pStyle = pPPortion->GetNode()->GetStyleSheet();
        if ( pStyle )
            fprintf( fp, " %s", OUStringToOString( pStyle->GetName(), RTL_TEXTENCODING_UTF8).getStr() );
        fprintf( fp, "\nParagraph attribute:" );
        DbgOutItemSet( fp, pPPortion->GetNode()->GetContentAttribs().GetItems(), sal_False, sal_False );

        fprintf( fp, "\nCharacter attribute:" );
        sal_Bool bZeroAttr = sal_False;
        sal_uInt16 z;
        for ( z = 0; z < pPPortion->GetNode()->GetCharAttribs().Count(); z++ )
        {
            const EditCharAttrib& rAttr = pPPortion->GetNode()->GetCharAttribs().GetAttribs()[z];
            OStringBuffer aCharAttribs;
            aCharAttribs.append(RTL_CONSTASCII_STRINGPARAM("\nA"));
            aCharAttribs.append(static_cast<sal_Int32>(nPortion));
            aCharAttribs.append(RTL_CONSTASCII_STRINGPARAM(":  "));
            aCharAttribs.append(static_cast<sal_Int32>(rAttr.GetItem()->Which()));
            aCharAttribs.append('\t');
            aCharAttribs.append(static_cast<sal_Int32>(rAttr.GetStart()));
            aCharAttribs.append('\t');
            aCharAttribs.append(static_cast<sal_Int32>(rAttr.GetEnd()));
            if ( rAttr.IsEmpty() )
                bZeroAttr = sal_True;
            fprintf(fp, "%s => ", aCharAttribs.getStr());

            OString aDebStr = DbgOutItem( rPool, *rAttr.GetItem() );
            fprintf( fp, "%s", aDebStr.getStr() );
        }
        if ( bZeroAttr )
            fprintf( fp, "\nNULL-Attribute!" );

        sal_uInt16 nTextPortions = pPPortion->GetTextPortions().Count();
        OStringBuffer aPortionStr(
            RTL_CONSTASCII_STRINGPARAM("\nText portions: #"));
        aPortionStr.append(static_cast<sal_Int32>(nTextPortions));
        aPortionStr.append(RTL_CONSTASCII_STRINGPARAM(" \nA"));
        aPortionStr.append(static_cast<sal_Int32>(nPortion));
        aPortionStr.append(
            RTL_CONSTASCII_STRINGPARAM(": Paragraph Length = "));
        aPortionStr.append(static_cast<sal_Int32>(pPPortion->GetNode()->Len()));
        aPortionStr.append(RTL_CONSTASCII_STRINGPARAM("\nA"));
        aPortionStr.append(static_cast<sal_Int32>(nPortion));
        aPortionStr.append(RTL_CONSTASCII_STRINGPARAM(": "));
        sal_uLong n = 0;
        for ( z = 0; z < nTextPortions; z++ )
        {
            TextPortion* pPortion = pPPortion->GetTextPortions()[z];
            aPortionStr.append(' ');
            aPortionStr.append(static_cast<sal_Int32>(pPortion->GetLen()));
            aPortionStr.append('(');
            aPortionStr.append(static_cast<sal_Int32>(pPortion->GetSize().Width()));
            aPortionStr.append(')');
            aPortionStr.append('[');
            aPortionStr.append(static_cast<sal_Int32>(pPortion->GetKind()));
            aPortionStr.append(']');
            aPortionStr.append(';');
            n += pPortion->GetLen();
        }
        aPortionStr.append(RTL_CONSTASCII_STRINGPARAM("\nA"));
        aPortionStr.append(static_cast<sal_Int32>(nPortion));
        aPortionStr.append(RTL_CONSTASCII_STRINGPARAM(": Total length: "));
        aPortionStr.append(static_cast<sal_Int32>(n));
        if ( pPPortion->GetNode()->Len() != n )
            aPortionStr.append(RTL_CONSTASCII_STRINGPARAM(" => Error !!!"));
        fprintf(fp, "%s", aPortionStr.getStr());

        fprintf( fp, "\n\nLines:" );
        // First the content ...
        sal_uInt16 nLine;
        for ( nLine = 0; nLine < pPPortion->GetLines().Count(); nLine++ )
        {
            EditLine* pLine = pPPortion->GetLines()[nLine];

            OString aLine(OUStringToOString(pPPortion->GetNode()->Copy(pLine->GetStart(), pLine->GetEnd() - pLine->GetStart()), RTL_TEXTENCODING_ASCII_US));
            fprintf( fp, "\nLine %i\t>%s<", nLine, aLine.getStr() );
        }
        // then the internal data ...
        for ( nLine = 0; nLine < pPPortion->GetLines().Count(); nLine++ )
        {
            EditLine* pLine = pPPortion->GetLines()[nLine];
            fprintf( fp, "\nZeile %i:\tStart: %i,\tEnd: %i", nLine, pLine->GetStart(), pLine->GetEnd() );
            fprintf( fp, "\t\tPortions: %i - %i.\tHight: %i, Ascent=%i", pLine->GetStartPortion(), pLine->GetEndPortion(), pLine->GetHeight(), pLine->GetMaxAscent() );
        }

        fprintf( fp, "\n-----------------------------------------------------------------------------" );
    }

    if ( pEE->pImpEditEngine->GetStyleSheetPool() )
    {
        sal_uLong nStyles = pEE->pImpEditEngine->GetStyleSheetPool() ? pEE->pImpEditEngine->GetStyleSheetPool()->Count() : 0;
        fprintf( fp, "\n\n================================================================================" );
        fprintf( fp, "\n==================   Stylesheets   =============================================" );
        fprintf( fp, "\n================================================================================" );
        fprintf( fp, "\n#Template:   %lu\n", nStyles );
        SfxStyleSheetIterator aIter( pEE->pImpEditEngine->GetStyleSheetPool(), SFX_STYLE_FAMILY_ALL );
        SfxStyleSheetBase* pStyle = aIter.First();
        while ( pStyle )
        {
            fprintf( fp, "\nTemplate:   %s", OUStringToOString( pStyle->GetName(), RTL_TEXTENCODING_ASCII_US ).getStr() );
            fprintf( fp, "\nParent:    %s", OUStringToOString( pStyle->GetParent(), RTL_TEXTENCODING_ASCII_US ).getStr() );
            fprintf( fp, "\nFollow:    %s", OUStringToOString( pStyle->GetFollow(), RTL_TEXTENCODING_ASCII_US ).getStr() );
            DbgOutItemSet( fp, pStyle->GetItemSet(), sal_False, sal_False );
            fprintf( fp, "\n----------------------------------" );

            pStyle = aIter.Next();
        }
    }

    fprintf( fp, "\n\n================================================================================" );
    fprintf( fp, "\n==================   Defaults   ================================================" );
    fprintf( fp, "\n================================================================================" );
    DbgOutItemSet( fp, pEE->pImpEditEngine->GetEmptyItemSet(), sal_True, sal_True );

    fprintf( fp, "\n\n================================================================================" );
    fprintf( fp, "\n==================   EditEngine & Views   ======================================" );
    fprintf( fp, "\n================================================================================" );
    fprintf( fp, "\nControl: %" SAL_PRIxUINT32, pEE->GetControlWord() );
    fprintf( fp, "\nRefMapMode: %i", pEE->pImpEditEngine->pRefDev->GetMapMode().GetMapUnit() );
    fprintf( fp, "\nPaperSize: %li x %li", pEE->GetPaperSize().Width(), pEE->GetPaperSize().Height() );
    fprintf( fp, "\nMaxAutoPaperSize: %li x %li", pEE->GetMaxAutoPaperSize().Width(), pEE->GetMaxAutoPaperSize().Height() );
    fprintf( fp, "\nMinAutoPaperSize: %li x %li", pEE->GetMinAutoPaperSize().Width(), pEE->GetMinAutoPaperSize().Height() );
    fprintf( fp, "\nUpdate: %i", pEE->GetUpdateMode() );
    fprintf( fp, "\nNumber of Views: %" SAL_PRI_SIZET "i", pEE->GetViewCount() );
    for ( sal_uInt16 nView = 0; nView < pEE->GetViewCount(); nView++ )
    {
        EditView* pV = pEE->GetView( nView );
        DBG_ASSERT( pV, "View not found!" );
        fprintf( fp, "\nView %i: Focus=%i", nView, pV->GetWindow()->HasFocus() );
        Rectangle aR( pV->GetOutputArea() );
        fprintf( fp, "\n  OutputArea: nX=%li, nY=%li, dX=%li, dY=%li, MapMode = %i", aR.TopLeft().X(), aR.TopLeft().Y(), aR.GetSize().Width(), aR.GetSize().Height() , pV->GetWindow()->GetMapMode().GetMapUnit() );
        aR = pV->GetVisArea();
        fprintf( fp, "\n  VisArea: nX=%li, nY=%li, dX=%li, dY=%li", aR.TopLeft().X(), aR.TopLeft().Y(), aR.GetSize().Width(), aR.GetSize().Height() );
        ESelection aSel = pV->GetSelection();
        fprintf( fp, "\n  Selection: Start=%" SAL_PRIdINT32 ",%u, End=%" SAL_PRIdINT32 ",%u", aSel.nStartPara, aSel.nStartPos, aSel.nEndPara, aSel.nEndPos );
    }
    if ( pEE->GetActiveView() )
    {
        fprintf( fp, "\n\n================================================================================" );
        fprintf( fp, "\n==================   Current View   ===========================================" );
        fprintf( fp, "\n================================================================================" );
        DbgOutItemSet( fp, pEE->GetActiveView()->GetAttribs(), sal_True, sal_False );
    }
    fclose( fp );
    if ( bInfoBox )
        InfoBox(0, OUString( "D:\\DEBUG.LOG !" ) ).Execute();
}

#if OSL_DEBUG_LEVEL > 2
sal_Bool ParaPortion::DbgCheckTextPortions()
{
    // check, if Portion length ok:
    sal_uInt16 nXLen = 0;
    for ( sal_uInt16 nPortion = 0; nPortion < aTextPortionList.Count(); nPortion++  )
        nXLen = nXLen + aTextPortionList[nPortion]->GetLen();
    return nXLen == pNode->Len() ? sal_True : sal_False;
}
#endif

bool CheckOrderedList(const CharAttribList::AttribsType& rAttribs, bool bStart)
{
    sal_uInt16 nPrev = 0;
    for (size_t nAttr = 0; nAttr < rAttribs.size(); ++nAttr)
    {
        const EditCharAttrib& rAttr = rAttribs[nAttr];
        sal_uInt16 nCur = bStart ? rAttr.GetStart() : rAttr.GetEnd();
        if ( nCur < nPrev )
            return false;

        nPrev = nCur;
    }
    return true;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
