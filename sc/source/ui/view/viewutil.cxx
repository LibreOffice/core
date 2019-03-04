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

#include <scitems.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>

#include <viewutil.hxx>
#include <global.hxx>
#include <chgtrack.hxx>
#include <chgviset.hxx>
#include <markdata.hxx>
#include <document.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <memory>

void ScViewUtil::PutItemScript( SfxItemSet& rShellSet, const SfxItemSet& rCoreSet,
                                sal_uInt16 nWhichId, SvtScriptType nScript )
{
    //  take the effective item from rCoreSet according to nScript
    //  and put in rShellSet under the (base) nWhichId

    SfxItemPool& rPool = *rShellSet.GetPool();
    SvxScriptSetItem aSetItem( rPool.GetSlotId(nWhichId), rPool );
    //  use PutExtended with eDefaultAs = SfxItemState::SET, so defaults from rCoreSet
    //  (document pool) are read and put into rShellSet (MessagePool)
    aSetItem.GetItemSet().PutExtended( rCoreSet, SfxItemState::DONTCARE, SfxItemState::SET );
    const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
    if (pI)
    {
        std::unique_ptr<SfxPoolItem> pNewItem(pI->CloneSetWhich(nWhichId));
        rShellSet.Put( *pNewItem );
    }
    else
        rShellSet.InvalidateItem( nWhichId );
}

LanguageType ScViewUtil::GetEffLanguage( ScDocument* pDoc, const ScAddress& rPos )
{
    //  used for thesaurus

    SvtScriptType nScript = pDoc->GetScriptType(rPos.Col(), rPos.Row(), rPos.Tab());
    sal_uInt16 nWhich = ( nScript == SvtScriptType::ASIAN ) ? ATTR_CJK_FONT_LANGUAGE :
                    ( ( nScript == SvtScriptType::COMPLEX ) ? ATTR_CTL_FONT_LANGUAGE : ATTR_FONT_LANGUAGE );
    const SfxPoolItem* pItem = pDoc->GetAttr( rPos.Col(), rPos.Row(), rPos.Tab(), nWhich);
    const SvxLanguageItem* pLangIt = dynamic_cast<const SvxLanguageItem*>( pItem  );
    LanguageType eLnge;
    if (pLangIt)
    {
        eLnge = pLangIt->GetValue();
        if (eLnge == LANGUAGE_DONTKNOW)                 //! can this happen?
        {
            LanguageType eLatin, eCjk, eCtl;
            pDoc->GetLanguage( eLatin, eCjk, eCtl );
            eLnge = ( nScript == SvtScriptType::ASIAN ) ? eCjk :
                    ( ( nScript == SvtScriptType::COMPLEX ) ? eCtl : eLatin );
        }
    }
    else
        eLnge = LANGUAGE_ENGLISH_US;
    if ( eLnge == LANGUAGE_SYSTEM )
        eLnge = Application::GetSettings().GetLanguageTag().getLanguageType();   // never use SYSTEM for spelling

    return eLnge;
}

TransliterationFlags ScViewUtil::GetTransliterationType( sal_uInt16 nSlotID )
{
    TransliterationFlags nType = TransliterationFlags::NONE;
    switch ( nSlotID )
    {
        case SID_TRANSLITERATE_SENTENCE_CASE:
            nType = TransliterationFlags::SENTENCE_CASE;
            break;
        case SID_TRANSLITERATE_TITLE_CASE:
            nType = TransliterationFlags::TITLE_CASE;
            break;
        case SID_TRANSLITERATE_TOGGLE_CASE:
            nType = TransliterationFlags::TOGGLE_CASE;
            break;
        case SID_TRANSLITERATE_UPPER:
            nType = TransliterationFlags::LOWERCASE_UPPERCASE;
            break;
        case SID_TRANSLITERATE_LOWER:
            nType = TransliterationFlags::UPPERCASE_LOWERCASE;
            break;
        case SID_TRANSLITERATE_HALFWIDTH:
            nType = TransliterationFlags::FULLWIDTH_HALFWIDTH;
            break;
        case SID_TRANSLITERATE_FULLWIDTH:
            nType = TransliterationFlags::HALFWIDTH_FULLWIDTH;
            break;
        case SID_TRANSLITERATE_HIRAGANA:
            nType = TransliterationFlags::KATAKANA_HIRAGANA;
            break;
        case SID_TRANSLITERATE_KATAKANA:
            nType = TransliterationFlags::HIRAGANA_KATAKANA;
            break;
    }
    return nType;
}

bool ScViewUtil::IsActionShown( const ScChangeAction& rAction,
                                const ScChangeViewSettings& rSettings,
                                ScDocument& rDocument )
{
    // discarded are displayed as inverted accepted action, because of this
    // order of ShowRejected/ShowAccepted is important

    if ( !rSettings.IsShowRejected() && rAction.IsRejecting() )
        return false;

    if ( !rSettings.IsShowAccepted() && rAction.IsAccepted() && !rAction.IsRejecting() )
        return false;

    if ( rSettings.HasAuthor() && rAction.GetUser() != rSettings.GetTheAuthorToShow() )
        return false;

    if ( rSettings.HasComment() )
    {
        OUStringBuffer aBuf(rAction.GetComment());
        aBuf.append(" (");
        OUString aTmp;
        rAction.GetDescription(aTmp, &rDocument);
        aBuf.append(aTmp);
        aBuf.append(')');
        OUString aComStr = aBuf.makeStringAndClear();

        if(!rSettings.IsValidComment(&aComStr))
            return false;
    }

    if ( rSettings.HasRange() )
        if ( !rSettings.GetTheRangeList().Intersects( rAction.GetBigRange().MakeRange() ) )
            return false;

    if (rSettings.HasDate() && rSettings.GetTheDateMode() != SvxRedlinDateMode::NONE)
    {
        DateTime aDateTime = rAction.GetDateTime();
        const DateTime& rFirst = rSettings.GetTheFirstDateTime();
        const DateTime& rLast  = rSettings.GetTheLastDateTime();
        switch ( rSettings.GetTheDateMode() )
        {   // corresponds with ScHighlightChgDlg::OKBtnHdl
            case SvxRedlinDateMode::BEFORE:
                if ( aDateTime > rFirst )
                    return false;
                break;

            case SvxRedlinDateMode::SINCE:
                if ( aDateTime < rFirst )
                    return false;
                break;

            case SvxRedlinDateMode::EQUAL:
            case SvxRedlinDateMode::BETWEEN:
                if ( aDateTime < rFirst || aDateTime > rLast )
                    return false;
                break;

            case SvxRedlinDateMode::NOTEQUAL:
                if ( aDateTime >= rFirst && aDateTime <= rLast )
                    return false;
                break;

            case SvxRedlinDateMode::SAVE:
                {
                ScChangeTrack* pTrack = rDocument.GetChangeTrack();
                if ( !pTrack || pTrack->GetLastSavedActionNumber() >=
                        rAction.GetActionNumber() )
                    return false;
                }
                break;

            default:
            {
                // added to avoid warnings
            }
        }
    }

    if ( rSettings.HasActionRange() )
    {
        sal_uLong nAction = rAction.GetActionNumber();
        sal_uLong nFirstAction;
        sal_uLong nLastAction;
        rSettings.GetTheActionRange( nFirstAction, nLastAction );
        if ( nAction < nFirstAction || nAction > nLastAction )
        {
            return false;
        }
    }

    return true;
}

void ScViewUtil::UnmarkFiltered( ScMarkData& rMark, const ScDocument* pDoc )
{
    rMark.MarkToMulti();

    ScRange aMultiArea;
    rMark.GetMultiMarkArea( aMultiArea );
    SCCOL nStartCol = aMultiArea.aStart.Col();
    SCROW nStartRow = aMultiArea.aStart.Row();
    SCCOL nEndCol = aMultiArea.aEnd.Col();
    SCROW nEndRow = aMultiArea.aEnd.Row();

    bool bChanged = false;
    for (const SCTAB& nTab : rMark)
    {
        for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
        {
            SCROW nLastRow = nRow;
            if (pDoc->RowFiltered(nRow, nTab, nullptr, &nLastRow))
            {
                // use nStartCol/nEndCol, so the multi mark area isn't extended to all columns
                // (visible in repaint for indentation)
                rMark.SetMultiMarkArea(
                    ScRange(nStartCol, nRow, nTab, nEndCol, nLastRow, nTab), false);
                bChanged = true;
                nRow = nLastRow;
            }
        }
    }

    if ( bChanged && !rMark.HasAnyMultiMarks() )
        rMark.ResetMark();

    rMark.MarkToSimple();
}

bool ScViewUtil::FitToUnfilteredRows( ScRange & rRange, const ScDocument * pDoc, size_t nRows )
{
    SCTAB nTab = rRange.aStart.Tab();
    bool bOneTabOnly = (nTab == rRange.aEnd.Tab());
    // Always fit the range on its first sheet.
    OSL_ENSURE( bOneTabOnly, "ScViewUtil::ExtendToUnfilteredRows: works only on one sheet");
    SCROW nStartRow = rRange.aStart.Row();
    SCROW nLastRow = pDoc->LastNonFilteredRow(nStartRow, MAXROW, nTab);
    if (ValidRow(nLastRow))
        rRange.aEnd.SetRow(nLastRow);
    SCROW nCount = pDoc->CountNonFilteredRows(nStartRow, MAXROW, nTab);
    return static_cast<size_t>(nCount) == nRows && bOneTabOnly;
}

bool ScViewUtil::HasFiltered( const ScRange& rRange, const ScDocument* pDoc )
{
    SCROW nStartRow = rRange.aStart.Row();
    SCROW nEndRow = rRange.aEnd.Row();
    for (SCTAB nTab=rRange.aStart.Tab(); nTab<=rRange.aEnd.Tab(); nTab++)
    {
        if (pDoc->HasFilteredRows(nStartRow, nEndRow, nTab))
            return true;
    }

    return false;
}

void ScViewUtil::HideDisabledSlot( SfxItemSet& rSet, SfxBindings& rBindings, sal_uInt16 nSlotId )
{
    SvtCJKOptions aCJKOptions;
    SvtCTLOptions aCTLOptions;
    bool bEnabled = true;

    switch( nSlotId )
    {
        case SID_CHINESE_CONVERSION:
        case SID_HANGUL_HANJA_CONVERSION:
            bEnabled = aCJKOptions.IsAnyEnabled();
        break;

        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAKANA:
            bEnabled = aCJKOptions.IsChangeCaseMapEnabled();
        break;

        case SID_INSERT_RLM:
        case SID_INSERT_LRM:
            bEnabled = aCTLOptions.IsCTLFontEnabled();
        break;

        default:
            OSL_FAIL( "ScViewUtil::HideDisabledSlot - unknown slot ID" );
            return;
    }

    rBindings.SetVisibleState( nSlotId, bEnabled );
    if( !bEnabled )
        rSet.DisableItem( nSlotId );
}

void ScViewUtil::ExecuteCharMap( const SvxFontItem& rOldFont,
                                 SfxViewFrame& rFrame )
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAllItemSet aSet( rFrame.GetObjectShell()->GetPool() );
    aSet.Put( SfxBoolItem( FN_PARAM_1, false ) );
    aSet.Put( SvxFontItem( rOldFont.GetFamily(), rOldFont.GetFamilyName(), rOldFont.GetStyleName(), rOldFont.GetPitch(), rOldFont.GetCharSet(), aSet.GetPool()->GetWhich( SID_ATTR_CHAR_FONT ) ) );
    ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateCharMapDialog(rFrame.GetWindow().GetFrameWeld(), aSet, true));
    pDlg->Execute();
}

bool ScViewUtil::IsFullScreen( const SfxViewShell& rViewShell )
{
    SfxBindings&    rBindings       = rViewShell.GetViewFrame()->GetBindings();
    std::unique_ptr<SfxPoolItem> pItem;
    bool            bIsFullScreen   = false;

    if (rBindings.QueryState( SID_WIN_FULLSCREEN, pItem ) >= SfxItemState::DEFAULT)
        bIsFullScreen = static_cast< SfxBoolItem* >( pItem.get() )->GetValue();

    return bIsFullScreen;
}

void ScViewUtil::SetFullScreen( const SfxViewShell& rViewShell, bool bSet )
{
    if( IsFullScreen( rViewShell ) != bSet )
    {
        SfxBoolItem aItem( SID_WIN_FULLSCREEN, bSet );
        rViewShell.GetDispatcher()->ExecuteList(SID_WIN_FULLSCREEN,
                SfxCallMode::RECORD, { &aItem });
    }
}

ScUpdateRect::ScUpdateRect( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 )
    : nNewStartX(0)
    , nNewStartY(0)
    , nNewEndX(0)
    , nNewEndY(0)
{
    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    nOldStartX = nX1;
    nOldStartY = nY1;
    nOldEndX = nX2;
    nOldEndY = nY2;
}

void ScUpdateRect::SetNew( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 )
{
    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    nNewStartX = nX1;
    nNewStartY = nY1;
    nNewEndX = nX2;
    nNewEndY = nY2;
}

bool ScUpdateRect::GetDiff( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 )
{
    if ( nNewStartX == nOldStartX && nNewEndX == nOldEndX &&
         nNewStartY == nOldStartY && nNewEndY == nOldEndY )
    {
        rX1 = nNewStartX;
        rY1 = nNewStartY;
        rX2 = nNewStartX;
        rY2 = nNewStartY;
        return false;
    }

    rX1 = std::min(nNewStartX,nOldStartX);
    rY1 = std::min(nNewStartY,nOldStartY);
    rX2 = std::max(nNewEndX,nOldEndX);
    rY2 = std::max(nNewEndY,nOldEndY);

    if ( nNewStartX == nOldStartX && nNewEndX == nOldEndX )
    {
        if ( nNewStartY == nOldStartY )
        {
            rY1 = std::min( nNewEndY, nOldEndY );
            rY2 = std::max( nNewEndY, nOldEndY );
        }
        else if ( nNewEndY == nOldEndY )
        {
            rY1 = std::min( nNewStartY, nOldStartY );
            rY2 = std::max( nNewStartY, nOldStartY );
        }
    }
    else if ( nNewStartY == nOldStartY && nNewEndY == nOldEndY )
    {
        if ( nNewStartX == nOldStartX )
        {
            rX1 = std::min( nNewEndX, nOldEndX );
            rX2 = std::max( nNewEndX, nOldEndX );
        }
        else if ( nNewEndX == nOldEndX )
        {
            rX1 = std::min( nNewStartX, nOldStartX );
            rX2 = std::max( nNewStartX, nOldStartX );
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
