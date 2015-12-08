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

#include "scitems.hxx"
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
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>

#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

#include "viewutil.hxx"
#include "global.hxx"
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include "markdata.hxx"
#include "document.hxx"

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
        rShellSet.Put( *pI, nWhichId );
    else
        rShellSet.InvalidateItem( nWhichId );
}

sal_uInt16 ScViewUtil::GetEffLanguage( ScDocument* pDoc, const ScAddress& rPos )
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
        eLnge = (LanguageType) pLangIt->GetValue();
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

sal_Int32 ScViewUtil::GetTransliterationType( sal_uInt16 nSlotID )
{
    sal_Int32 nType = 0;
    switch ( nSlotID )
    {
        case SID_TRANSLITERATE_SENTENCE_CASE:
            nType = css::i18n::TransliterationModulesExtra::SENTENCE_CASE;
            break;
        case SID_TRANSLITERATE_TITLE_CASE:
            nType = css::i18n::TransliterationModulesExtra::TITLE_CASE;
            break;
        case SID_TRANSLITERATE_TOGGLE_CASE:
            nType = css::i18n::TransliterationModulesExtra::TOGGLE_CASE;
            break;
        case SID_TRANSLITERATE_UPPER:
            nType = css::i18n::TransliterationModules_LOWERCASE_UPPERCASE;
            break;
        case SID_TRANSLITERATE_LOWER:
            nType = css::i18n::TransliterationModules_UPPERCASE_LOWERCASE;
            break;
        case SID_TRANSLITERATE_HALFWIDTH:
            nType = css::i18n::TransliterationModules_FULLWIDTH_HALFWIDTH;
            break;
        case SID_TRANSLITERATE_FULLWIDTH:
            nType = css::i18n::TransliterationModules_HALFWIDTH_FULLWIDTH;
            break;
        case SID_TRANSLITERATE_HIRAGANA:
            nType = css::i18n::TransliterationModules_KATAKANA_HIRAGANA;
            break;
        case SID_TRANSLITERATE_KATAGANA:
            nType = css::i18n::TransliterationModules_HIRAGANA_KATAKANA;
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

    if ( rSettings.HasAuthor() )
    {
        if ( rSettings.IsEveryoneButMe() )
        {
            // GetUser() at ChangeTrack is the current user
            ScChangeTrack* pTrack = rDocument.GetChangeTrack();
            if ( !pTrack || rAction.GetUser().equals(pTrack->GetUser()) )
                return false;
        }
        else if ( !rAction.GetUser().equals(rSettings.GetTheAuthorToShow()) )
            return false;
    }

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

void ScViewUtil::UnmarkFiltered( ScMarkData& rMark, ScDocument* pDoc )
{
    rMark.MarkToMulti();

    ScRange aMultiArea;
    rMark.GetMultiMarkArea( aMultiArea );
    SCCOL nStartCol = aMultiArea.aStart.Col();
    SCROW nStartRow = aMultiArea.aStart.Row();
    SCCOL nEndCol = aMultiArea.aEnd.Col();
    SCROW nEndRow = aMultiArea.aEnd.Row();

    bool bChanged = false;
    ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd; ++itr)
    {
        SCTAB nTab = *itr;
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

bool ScViewUtil::FitToUnfilteredRows( ScRange & rRange, ScDocument * pDoc, size_t nRows )
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

bool ScViewUtil::HasFiltered( const ScRange& rRange, ScDocument* pDoc )
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
        case SID_TRANSLITERATE_KATAGANA:
            bEnabled = aCJKOptions.IsChangeCaseMapEnabled();
        break;

        case SID_INSERT_RLM:
        case SID_INSERT_LRM:
        case SID_INSERT_ZWNBSP:
        case SID_INSERT_ZWSP:
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

bool ScViewUtil::ExecuteCharMap( const SvxFontItem& rOldFont,
                                 SfxViewFrame& rFrame,
                                 SvxFontItem&       rNewFont,
                                 OUString&          rString )
{
    bool bRet = false;
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        SfxAllItemSet aSet( rFrame.GetObjectShell()->GetPool() );
        aSet.Put( SfxBoolItem( FN_PARAM_1, false ) );
        aSet.Put( SvxFontItem( rOldFont.GetFamily(), rOldFont.GetFamilyName(), rOldFont.GetStyleName(), rOldFont.GetPitch(), rOldFont.GetCharSet(), aSet.GetPool()->GetWhich( SID_ATTR_CHAR_FONT ) ) );
        std::unique_ptr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog( &rFrame.GetWindow(), aSet, rFrame.GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP ));
        if ( pDlg->Execute() == RET_OK )
        {
            const SfxStringItem* pItem = SfxItemSet::GetItem<SfxStringItem>(pDlg->GetOutputItemSet(), SID_CHARMAP, false);
            const SvxFontItem* pFontItem = SfxItemSet::GetItem<SvxFontItem>(pDlg->GetOutputItemSet(), SID_ATTR_CHAR_FONT, false);
            if ( pItem )
                rString  = pItem->GetValue();
            if ( pFontItem )
                rNewFont = SvxFontItem( pFontItem->GetFamily(), pFontItem->GetFamilyName(), pFontItem->GetStyleName(), pFontItem->GetPitch(), pFontItem->GetCharSet(), rNewFont.Which() );
            bRet = true;
        }
    }
    return bRet;
}

bool ScViewUtil::IsFullScreen( SfxViewShell& rViewShell )
{
    SfxBindings&    rBindings       = rViewShell.GetViewFrame()->GetBindings();
    std::unique_ptr<SfxPoolItem> pItem;
    bool            bIsFullScreen   = false;

    if (rBindings.QueryState( SID_WIN_FULLSCREEN, pItem ) >= SfxItemState::DEFAULT)
        bIsFullScreen = static_cast< SfxBoolItem* >( pItem.get() )->GetValue();

    return bIsFullScreen;
}

void ScViewUtil::SetFullScreen( SfxViewShell& rViewShell, bool bSet )
{
    if( IsFullScreen( rViewShell ) != bSet )
    {
        SfxBoolItem aItem( SID_WIN_FULLSCREEN, bSet );
        rViewShell.GetDispatcher()->Execute( SID_WIN_FULLSCREEN, SfxCallMode::RECORD, &aItem, 0L );
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
