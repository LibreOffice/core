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

#include <string>

#include <memory>
#include <boost/property_tree/json_parser.hpp>

#include <hintids.hxx>

#include <sal/log.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <svx/pageitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <svx/srchdlg.hxx>
#include <edtwin.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <workctrl.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>
#include <uitool.hxx>
#include <cmdid.h>
#include <docsh.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>

#include <strings.hrc>
#include <SwRewriter.hxx>

#include <PostItMgr.hxx>

using namespace com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//Search Parameter

struct SwSearchOptions
{
    SwDocPositions eStart, eEnd;
    bool bDontWrap;

    SwSearchOptions( SwWrtShell const * pSh, bool bBackward );
};

/// Adds rMatches using rKey as a key to the rTree tree.
static void lcl_addContainerToJson(boost::property_tree::ptree& rTree, const OString& rKey, const std::vector<OString>& rMatches)
{
    boost::property_tree::ptree aChildren;

    for (const OString& rMatch : rMatches)
    {
        boost::property_tree::ptree aChild;
        aChild.put("part", "0");
        aChild.put("rectangles", rMatch.getStr());
        aChildren.push_back(std::make_pair("", aChild));
    }

    rTree.add_child(rKey.getStr(), aChildren);
}

/// Emits LOK callbacks (count, selection) for search results.
static void lcl_emitSearchResultCallbacks(SvxSearchItem const * pSearchItem, SwWrtShell const * pWrtShell, bool bHighlightAll)
{
    // Emit a callback also about the selection rectangles, grouped by matches.
    SwPaM* pPaM = pWrtShell->GetCursor();
    if (!pPaM)
        return;

    std::vector<OString> aMatches;
    for (SwPaM& rPaM : pPaM->GetRingContainer())
    {
        if (SwShellCursor* pShellCursor = dynamic_cast<SwShellCursor*>(&rPaM))
        {
            std::vector<OString> aSelectionRectangles;
            pShellCursor->SwSelPaintRects::Show(&aSelectionRectangles);
            std::vector<OString> aRect;
            for (const OString & rSelectionRectangle : aSelectionRectangles)
            {
                if (rSelectionRectangle.isEmpty())
                    continue;
                aRect.push_back(rSelectionRectangle);
            }
            OString sRect = comphelper::string::join("; ", aRect);
            aMatches.push_back(sRect);
        }
    }
    boost::property_tree::ptree aTree;
    aTree.put("searchString", pSearchItem->GetSearchString().toUtf8().getStr());
    aTree.put("highlightAll", bHighlightAll);
    lcl_addContainerToJson(aTree, "searchResultSelection", aMatches);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    OString aPayload = aStream.str().c_str();

    pWrtShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_RESULT_SELECTION, aPayload.getStr());
}

void SwView::ExecSearch(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    bool bQuiet = false;
    if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_SEARCH_QUIET, false, &pItem))
        bQuiet = static_cast<const SfxBoolItem*>( pItem)->GetValue();

    sal_uInt16 nSlot = rReq.GetSlot();
    if (nSlot == FN_REPEAT_SEARCH && !s_pSrchItem)
    {
        if(bQuiet)
        {
            rReq.SetReturnValue(SfxBoolItem(nSlot, false));
            nSlot = 0;
        }
    }
    if( m_pWrtShell->IsBlockMode() )
        m_pWrtShell->LeaveBlockMode();
    switch (nSlot)
    {
    // for now do nothing
    case SID_SEARCH_ITEM:
    {
        delete s_pSrchItem;
        s_pSrchItem = pArgs->Get(SID_SEARCH_ITEM).Clone();
    }
    break;

    case FID_SEARCH_ON:
        s_bJustOpened = true;
        GetViewFrame()->GetBindings().Invalidate(SID_SEARCH_ITEM);
        break;

    case FID_SEARCH_OFF:
        if(pArgs)
        {
            // Unregister dialog
            delete s_pSrchItem;
            s_pSrchItem = pArgs->Get(SID_SEARCH_ITEM).Clone();

            s_xSearchList.reset();
            s_xReplaceList.reset();

            SvxSearchDialog *const pSrchDlg(GetSearchDialog());
            if (pSrchDlg)
            {
                // We will remember the search-/replace items.
                const SearchAttrItemList* pList = pSrchDlg->GetSearchItemList();
                if( nullptr != pList && pList->Count() )
                    s_xSearchList.reset(new SearchAttrItemList( *pList ));

                pList = pSrchDlg->GetReplaceItemList();
                if (nullptr != pList && pList->Count())
                    s_xReplaceList.reset(new SearchAttrItemList( *pList ));
            }
        }
        break;

        case FN_REPEAT_SEARCH:
        case FID_SEARCH_NOW:
        {
            sal_uInt16 nMoveType = SwView::GetMoveType();
            {
                if(FID_SEARCH_NOW == nSlot && !rReq.IsAPI())
                    SwView::SetMoveType(NID_SRCH_REP);
            }

            SvxSearchDialog * pSrchDlg(GetSearchDialog());
            if (pSrchDlg)
            {
                s_xSearchList.reset();
                s_xReplaceList.reset();

                const SearchAttrItemList* pList = pSrchDlg->GetSearchItemList();
                if( nullptr != pList && pList->Count() )
                    s_xSearchList.reset(new SearchAttrItemList( *pList ));

                pList = pSrchDlg->GetReplaceItemList();
                if (nullptr != pList && pList->Count())
                    s_xReplaceList.reset(new SearchAttrItemList( *pList ));
            }

            if (nSlot == FN_REPEAT_SEARCH)
            {
                OSL_ENSURE(s_pSrchItem, "SearchItem missing");
                if( !s_pSrchItem )
                    s_pSrchItem = new SvxSearchItem(SID_SEARCH_ITEM);
            }
            else
            {
                // Get SearchItem from request
                OSL_ENSURE(pArgs, "Args missing");
                if ( pArgs )
                {
                    delete s_pSrchItem;
                    s_pSrchItem = pArgs->Get(SID_SEARCH_ITEM).Clone();
                }
            }
            SvxSearchCmd eCommand = s_pSrchItem->GetCommand();
            switch (eCommand)
            {
            case SvxSearchCmd::FIND:
            {
                bool bRet = SearchAndWrap(bQuiet);
                if( bRet )
                {
                    Scroll(m_pWrtShell->GetCharRect().SVRect());
                    if (comphelper::LibreOfficeKit::isActive())
                        lcl_emitSearchResultCallbacks(s_pSrchItem, m_pWrtShell.get(), /* bHighlightAll = */ false);
                }
                rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));

                GetDocShell()->Broadcast(SfxHint(SfxHintId::SwNavigatorUpdateTracking));
            }
            break;
            case SvxSearchCmd::FIND_ALL:
            {
                // Disable LOK selection notifications during search.
                m_pWrtShell->GetSfxViewShell()->setTiledSearching(true);
                bool bRet = SearchAll();
                m_pWrtShell->GetSfxViewShell()->setTiledSearching(false);

                GetDocShell()->Broadcast(
                            SfxHint(SfxHintId::SwNavigatorSelectOutlinesWithSelections));

                if( !bRet )
                {
#if HAVE_FEATURE_DESKTOP
                    if( !bQuiet )
                    {
                        m_pWrtShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_NOT_FOUND, s_pSrchItem->GetSearchString().toUtf8().getStr());
                        SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::NotFound);
                    }
#endif
                    s_bFound = false;
                }
                else if (comphelper::LibreOfficeKit::isActive())
                    lcl_emitSearchResultCallbacks(s_pSrchItem, m_pWrtShell.get(), /* bHighlightAll = */ true);
                rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
            }
            break;
            case SvxSearchCmd::REPLACE:
                {

                    // 1) Replace selection (Not if only attributes should be replaced)
//JP 27.04.95: Why?
//      what if you only want to assign attributes to the found??

                    SvxSearchCmd nCmd = SvxSearchCmd::FIND;
                    if( !s_pSrchItem->GetReplaceString().isEmpty() ||
                        !s_xReplaceList )
                    {
                        // Prevent, that the replaced string will be found again
                        // if the replacement string is containing the search string.
                        bool bBack = s_pSrchItem->GetBackward();
                        if (bBack)
                            m_pWrtShell->Push();
                        OUString aReplace( s_pSrchItem->GetReplaceString() );
                        i18nutil::SearchOptions2 aTmp( s_pSrchItem->GetSearchOptions() );
                        std::optional<OUString> xBackRef = sw::ReplaceBackReferences(aTmp,
                            m_pWrtShell->GetCursor(), m_pWrtShell->GetLayout());
                        if( xBackRef )
                            s_pSrchItem->SetReplaceString( *xBackRef );
                        Replace();
                        if( xBackRef )
                        {
                            s_pSrchItem->SetReplaceString( aReplace );
                        }
                        if (bBack)
                        {
                            m_pWrtShell->Pop();
                            m_pWrtShell->SwapPam();
                        }
                    }
                    else if( s_xReplaceList )
                        nCmd = SvxSearchCmd::REPLACE;

                    // 2) Search further (without replacing!)

                    SvxSearchCmd nOldCmd = s_pSrchItem->GetCommand();
                    s_pSrchItem->SetCommand( nCmd );
                    bool bRet = SearchAndWrap(bQuiet);
                    if( bRet )
                        Scroll( m_pWrtShell->GetCharRect().SVRect());
                    s_pSrchItem->SetCommand( nOldCmd );
                    rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
                }
                break;

            case SvxSearchCmd::REPLACE_ALL:
                {
                    SwSearchOptions aOpts( m_pWrtShell.get(), s_pSrchItem->GetBackward() );
                    s_bExtra = false;
                    sal_uLong nFound;

                    {   //Scope for SwWait-Object
                        SwWait aWait( *GetDocShell(), true );
                        m_pWrtShell->StartAllAction();

                        // i#8288 "replace all" should not change cursor
                        // position, so save current cursor
                        m_pWrtShell->Push();

                        if (!s_pSrchItem->GetSelection())
                        {
                            // if we don't want to search in the selection...
                            m_pWrtShell->KillSelection(nullptr, false);
                            if (SwDocPositions::Start == aOpts.eEnd)
                            {
                                m_pWrtShell->EndOfSection();
                            }
                            else
                            {
                                m_pWrtShell->StartOfSection();
                            }
                        }
                        nFound = FUNC_Search( aOpts );
                        // create it just to overwrite it with stack cursor
                        m_pWrtShell->CreateCursor();
                        // i#8288 restore the original cursor position
                        m_pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
                        m_pWrtShell->EndAllAction();
                    }

                    rReq.SetReturnValue(SfxBoolItem(nSlot, nFound != 0 && ULONG_MAX != nFound));
                    if( !nFound )
                    {
#if HAVE_FEATURE_DESKTOP
                        if( !bQuiet )
                        {
                            m_pWrtShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_NOT_FOUND, s_pSrchItem->GetSearchString().toUtf8().getStr());
                            SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::NotFound);
                        }
#endif
                        s_bFound = false;
                        SwView::SetMoveType(nMoveType);
                        return;
                    }

                    if( !bQuiet && ULONG_MAX != nFound)
                    {
                        OUString sText( SwResId( STR_NB_REPLACED ) );
                        sText = sText.replaceFirst("XX", OUString::number( nFound ));
                        SvxSearchDialogWrapper::SetSearchLabel(sText);
                    }
                }
                break;
            }

            uno::Reference< frame::XDispatchRecorder > xRecorder =
                    GetViewFrame()->GetBindings().GetRecorder();
            //prevent additional dialogs in recorded macros
            if ( xRecorder.is() )
                rReq.AppendItem(SfxBoolItem(SID_SEARCH_QUIET, true));

            rReq.Done();
            m_eLastSearchCommand = s_pSrchItem->GetCommand();
            SwView::SetMoveType(nMoveType);
        }
        break;
        case FID_SEARCH_SEARCHSET:
        case FID_SEARCH_REPLACESET:
        {
            static const sal_uInt16 aNormalAttr[] =
            {
/* 0 */         RES_CHRATR_CASEMAP,     RES_CHRATR_CASEMAP,
/* 2 */         RES_CHRATR_COLOR,       RES_CHRATR_POSTURE,
/* 4 */         RES_CHRATR_SHADOWED,    RES_CHRATR_WORDLINEMODE,
/* 6 */         RES_CHRATR_BLINK,       RES_CHRATR_BLINK,
/* 8 */         RES_CHRATR_BACKGROUND,  RES_CHRATR_BACKGROUND,
/*10 */         RES_CHRATR_ROTATE,      RES_CHRATR_ROTATE,
/*12 */         RES_CHRATR_SCALEW,      RES_CHRATR_RELIEF,
/*14 */         RES_CHRATR_OVERLINE,    RES_CHRATR_OVERLINE,
/*16 */         RES_PARATR_LINESPACING, RES_PARATR_HYPHENZONE,
/*18 */         RES_PARATR_REGISTER,    RES_PARATR_REGISTER,
/*20 */         RES_PARATR_VERTALIGN,   RES_PARATR_VERTALIGN,
/*22 */         RES_LR_SPACE,           RES_UL_SPACE,
/*24 */         SID_ATTR_PARA_MODEL,    SID_ATTR_PARA_KEEP,
/*26 */         0
            };

            SfxItemSet aSet(m_pWrtShell->GetAttrPool(), aNormalAttr);

            if( SW_MOD()->GetCTLOptions().IsCTLFontEnabled() )
            {
                aSet.MergeRange(RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_WEIGHT);
            }
            SvtCJKOptions aCJKOpt;
            if( aCJKOpt.IsAnyEnabled() )
            {
                aSet.MergeRange(RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_WEIGHT);
                aSet.MergeRange(RES_CHRATR_EMPHASIS_MARK, RES_CHRATR_TWO_LINES);
                aSet.MergeRange(RES_PARATR_SCRIPTSPACE, RES_PARATR_FORBIDDEN_RULES);
            }

            sal_uInt16 nWhich = SID_SEARCH_SEARCHSET;

            if ( FID_SEARCH_REPLACESET == nSlot )
            {
                nWhich = SID_SEARCH_REPLACESET;

                if ( s_xReplaceList )
                {
                    s_xReplaceList->Get( aSet );
                    s_xReplaceList.reset();
                }
            }
            else if ( s_xSearchList )
            {
                s_xSearchList->Get( aSet );
                s_xSearchList.reset();
            }
            rReq.SetReturnValue( SvxSetItem( nWhich, aSet ) );
        }
        break;
        default:
            SAL_WARN_IF( nSlot, "sw", "nSlot: " << nSlot << " wrong Dispatcher (viewsrch.cxx)" );
            return;
    }
}

bool SwView::SearchAndWrap(bool bApi)
{
    SwSearchOptions aOpts( m_pWrtShell.get(), s_pSrchItem->GetBackward() );

        // Remember starting position of the search for wraparound
        // Start- / EndAction perhaps because existing selections of 'search all'
    m_pWrtShell->StartAllAction();
    m_pWrtShell->Push();

    // After a search all action we place the cursor at the beginning of
    // the document so that the single search selects the first matching
    // occurrence in the document instead of the second.
    if( m_eLastSearchCommand == SvxSearchCmd::FIND_ALL )
    {
        if( SwDocPositions::Start == aOpts.eEnd )
            m_pWrtShell->EndOfSection();
        else
            m_pWrtShell->StartOfSection();
    }

    // fdo#65014 : Ensure that the point of the cursor is at the extremity of the
    // selection closest to the end being searched to as to exclude the selected
    // region from the search. (This doesn't work in the case of multiple
    // selected regions as the cursor doesn't mark the selection in that case.)
    m_pWrtShell->GetCursor()->Normalize( s_pSrchItem->GetBackward() );

    if (!m_pWrtShell->HasSelection() && (s_pSrchItem->HasStartPoint()))
    {
        // No selection -> but we have a start point (top left corner of the
        // current view), start searching from there, not from the current
        // cursor position.
        SwEditShell& rShell = GetWrtShell();
        Point aPosition(s_pSrchItem->GetStartPointX(), s_pSrchItem->GetStartPointY());
        rShell.SetCursor(aPosition);
    }

        // If you want to search in selected areas, they must not be unselected.
    if (!s_pSrchItem->GetSelection())
        m_pWrtShell->KillSelection(nullptr, false);

    std::unique_ptr<SwWait> pWait(new SwWait( *GetDocShell(), true ));
    if( FUNC_Search( aOpts ) )
    {
        s_bFound = true;
        if(m_pWrtShell->IsSelFrameMode())
        {
            m_pWrtShell->UnSelectFrame();
            m_pWrtShell->LeaveSelFrameMode();
        }
        m_pWrtShell->Pop();
        m_pWrtShell->EndAllAction();
        return true;
    }
    pWait.reset();

        // Search in the specialized areas when no search is present in selections.
        // When searching selections will already searched in these special areas.
    bool bHasSrchInOther = s_bExtra;
    if (!s_pSrchItem->GetSelection() && !s_bExtra )
    {
        s_bExtra = true;
        if( FUNC_Search( aOpts ) )
        {
            s_bFound = true;
            m_pWrtShell->Pop();
            m_pWrtShell->EndAllAction();
            return true;
        }
        s_bExtra = false;
    }
    else
        s_bExtra = !s_bExtra;

        // If starting position is at the end or beginning of the document.
    if (aOpts.bDontWrap)
    {
        m_pWrtShell->EndAllAction();
        if( !bApi )
        {
#if HAVE_FEATURE_DESKTOP
            m_pWrtShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_NOT_FOUND, s_pSrchItem->GetSearchString().toUtf8().getStr());
            SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::NotFound);
#endif
        }
        s_bFound = false;
        m_pWrtShell->Pop();
        return false;
    }
    m_pWrtShell->EndAllAction();
        // Try again with WrapAround?

    m_pWrtShell->StartAllAction();
    m_pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    pWait.reset(new SwWait( *GetDocShell(), true ));

    bool bSrchBkwrd = SwDocPositions::Start == aOpts.eEnd;

    aOpts.eEnd =  bSrchBkwrd ? SwDocPositions::Start : SwDocPositions::End;
    aOpts.eStart = bSrchBkwrd ? SwDocPositions::End : SwDocPositions::Start;

    if (bHasSrchInOther)
    {
        m_pWrtShell->ClearMark();
        // Select the start or the end of the entire document
        if (bSrchBkwrd)
            m_pWrtShell->SttEndDoc(false);
        else
            m_pWrtShell->SttEndDoc(true);
    }

    s_bFound = bool(FUNC_Search( aOpts ));

    // If WrapAround found no matches in the body text, search in the special
    // sections, too.
    if (!s_bFound && !s_pSrchItem->GetSelection() && !s_bExtra)
    {
        s_bExtra = true;
        if (FUNC_Search(aOpts))
            s_bFound = true;
        else
            s_bExtra = false;
    }

    m_pWrtShell->EndAllAction();
    pWait.reset();
#if HAVE_FEATURE_DESKTOP
    if (s_bFound)
    {
        if (!bSrchBkwrd)
            SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::End);
        else
            SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::Start);
    }
    else if(!bApi)
    {
        m_pWrtShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_NOT_FOUND, s_pSrchItem->GetSearchString().toUtf8().getStr());
        SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::NotFound);
    }
#endif
    return s_bFound;
}

bool SwView::SearchAll()
{
    SwWait aWait( *GetDocShell(), true );
    m_pWrtShell->StartAllAction();

    SwSearchOptions aOpts( m_pWrtShell.get(), s_pSrchItem->GetBackward() );

    if (!s_pSrchItem->GetSelection())
    {
        // Cancel existing selections, if should not be sought in selected areas.
        m_pWrtShell->KillSelection(nullptr, false);

        if( SwDocPositions::Start == aOpts.eEnd )
            m_pWrtShell->EndOfSection();
        else
            m_pWrtShell->StartOfSection();
    }
    s_bExtra = false;
    sal_uInt16 nFound = static_cast<sal_uInt16>(FUNC_Search( aOpts ));
    s_bFound = 0 != nFound;

    m_pWrtShell->EndAllAction();
    return s_bFound;
}

void SwView::Replace()
{
    SwWait aWait( *GetDocShell(), true );

    m_pWrtShell->StartAllAction();

    if( s_pSrchItem->GetPattern() ) // Templates?
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, s_pSrchItem->GetSearchString());
        aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, s_pSrchItem->GetReplaceString());

        m_pWrtShell->StartUndo(SwUndoId::UI_REPLACE_STYLE, &aRewriter);

        m_pWrtShell->SetTextFormatColl( m_pWrtShell->GetParaStyle(
                            s_pSrchItem->GetReplaceString(),
                            SwWrtShell::GETSTYLE_CREATESOME ));

        m_pWrtShell->EndUndo();
    }
    else
    {
        if (GetPostItMgr()->HasActiveSidebarWin())
            GetPostItMgr()->Replace(s_pSrchItem);

        bool bReqReplace = true;

        if(m_pWrtShell->HasSelection())
        {
            /* check that the selection match the search string*/
            //save state
            SwPosition aStartPos = * m_pWrtShell->GetSwCursor()->Start();
            SwPosition aEndPos = * m_pWrtShell->GetSwCursor()->End();
            bool   bHasSelection = s_pSrchItem->GetSelection();
            SvxSearchCmd nOldCmd = s_pSrchItem->GetCommand();

            //set state for checking if current selection has a match
            s_pSrchItem->SetCommand( SvxSearchCmd::FIND );
            s_pSrchItem->SetSelection(true);

            //check if it matches
            SwSearchOptions aOpts( m_pWrtShell.get(), s_pSrchItem->GetBackward() );
            if( ! FUNC_Search(aOpts) )
            {

                //no matching therefore should not replace selection
                // => remove selection

                if(! s_pSrchItem->GetBackward() )
                {
                    (* m_pWrtShell->GetSwCursor()->Start()) = aStartPos;
                    (* m_pWrtShell->GetSwCursor()->End()) = aEndPos;
                }
                else
                {
                    (* m_pWrtShell->GetSwCursor()->Start()) = aEndPos;
                    (* m_pWrtShell->GetSwCursor()->End()) = aStartPos;
                }
                bReqReplace = false;
            }

            //set back old search state
            s_pSrchItem->SetCommand( nOldCmd );
            s_pSrchItem->SetSelection(bHasSelection);
        }
        /*
         * remove current selection
         * otherwise it is always replaced
         * no matter if the search string exists or not in the selection
         * Now the selection is removed and the next matching string is selected
         */

        if( bReqReplace )
        {

            bool bReplaced = m_pWrtShell->SwEditShell::Replace( s_pSrchItem->GetReplaceString(),
                                                                  s_pSrchItem->GetRegExp());
            if( bReplaced && s_xReplaceList && s_xReplaceList->Count() && m_pWrtShell->HasSelection() )
            {
                SfxItemSet aReplSet( m_pWrtShell->GetAttrPool(),
                                     aTextFormatCollSetRange );
                if( s_xReplaceList->Get( aReplSet ).Count() )
                {
                    ::SfxToSwPageDescAttr( *m_pWrtShell, aReplSet );
                    m_pWrtShell->SwEditShell::SetAttrSet( aReplSet );
                }
            }
        }
    }

    m_pWrtShell->EndAllAction();
}

SwSearchOptions::SwSearchOptions( SwWrtShell const * pSh, bool bBackward )
   : eStart(SwDocPositions::Curr)
{
    if( bBackward )
    {
        eEnd = SwDocPositions::Start;
        bDontWrap = pSh->IsEndOfDoc();
    }
    else
    {
        eEnd = SwDocPositions::End;
        bDontWrap = pSh->IsStartOfDoc();
    }
}

sal_uLong SwView::FUNC_Search( const SwSearchOptions& rOptions )
{
#if HAVE_FEATURE_DESKTOP
    SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::Empty);
#endif
    bool bDoReplace = s_pSrchItem->GetCommand() == SvxSearchCmd::REPLACE ||
                      s_pSrchItem->GetCommand() == SvxSearchCmd::REPLACE_ALL;

    FindRanges eRanges = s_pSrchItem->GetSelection()
                        ? FindRanges::InSel
                        : s_bExtra
                          ? FindRanges::InOther : FindRanges::InBody;
    if (s_pSrchItem->GetCommand() == SvxSearchCmd::FIND_ALL    ||
        s_pSrchItem->GetCommand() == SvxSearchCmd::REPLACE_ALL)
        eRanges |= FindRanges::InSelAll;

    m_pWrtShell->SttSelect();

    static const sal_uInt16 aSearchAttrRange[] = {
        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
        RES_CHRATR_BEGIN, RES_CHRATR_END-1,
        RES_PARATR_BEGIN, RES_PARATR_END-1,
        SID_ATTR_PARA_MODEL, SID_ATTR_PARA_KEEP,
        0 };

    SfxItemSet aSrchSet( m_pWrtShell->GetAttrPool(), aSearchAttrRange);
    if( s_xSearchList && s_xSearchList->Count() )
    {
        s_xSearchList->Get( aSrchSet );

        // -- Page break with page template
        ::SfxToSwPageDescAttr( *m_pWrtShell, aSrchSet );
    }

    std::unique_ptr<SfxItemSet> pReplSet;
    if( bDoReplace && s_xReplaceList && s_xReplaceList->Count() )
    {
        pReplSet.reset( new SfxItemSet( m_pWrtShell->GetAttrPool(),
                                        aSearchAttrRange ) );
        s_xReplaceList->Get( *pReplSet );

        // -- Page break with page template
        ::SfxToSwPageDescAttr( *m_pWrtShell, *pReplSet );

        if( !pReplSet->Count() )        // too bad, we don't know
            pReplSet.reset();        // the attributes
    }

    // build SearchOptions to be used

    i18nutil::SearchOptions2 aSearchOpt( s_pSrchItem->GetSearchOptions() );
    aSearchOpt.Locale = GetAppLanguageTag().getLocale();
    if( !bDoReplace )
        aSearchOpt.replaceString.clear();

    sal_uLong nFound;
    if( aSrchSet.Count() || ( pReplSet && pReplSet->Count() ))
    {
        nFound = m_pWrtShell->SearchAttr(
            aSrchSet,
            !s_pSrchItem->GetPattern(),
            rOptions.eStart,
            rOptions.eEnd,
            eRanges,
            !s_pSrchItem->GetSearchString().isEmpty() ? &aSearchOpt : nullptr,
            pReplSet.get() );
    }
    else if( s_pSrchItem->GetPattern() )
    {
        // Searching (and replacing) templates
        const OUString& sRplStr( s_pSrchItem->GetReplaceString() );
        nFound = m_pWrtShell->SearchTempl( s_pSrchItem->GetSearchString(),
            rOptions.eStart,
            rOptions.eEnd,
            eRanges,
            bDoReplace ? &sRplStr : nullptr );
    }
    else
    {
        // Normal search
        nFound = m_pWrtShell->SearchPattern(aSearchOpt, s_pSrchItem->GetNotes(),
                                          rOptions.eStart,
                                          rOptions.eEnd,
                                          eRanges,
                                          bDoReplace );
    }
    m_pWrtShell->EndSelect();
    return nFound;
}

SvxSearchDialog* SwView::GetSearchDialog()
{
#if HAVE_FEATURE_DESKTOP
    const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
    SvxSearchDialogWrapper *pWrp = static_cast<SvxSearchDialogWrapper*>( SfxViewFrame::Current()->GetChildWindow(nId) );
    auto pSrchDlg = pWrp ? pWrp->getDialog() : nullptr;
    return pSrchDlg;
#else
    return nullptr;
#endif
}

void SwView::StateSearch(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_SEARCH_OPTIONS:
            {
                SearchOptionFlags nOpt = SearchOptionFlags::ALL;
                if( GetDocShell()->IsReadOnly() )
                    nOpt &= ~SearchOptionFlags( SearchOptionFlags::REPLACE |
                               SearchOptionFlags::REPLACE_ALL );
                rSet.Put( SfxUInt16Item( SID_SEARCH_OPTIONS, static_cast<sal_uInt16>(nOpt) ));
            }
            break;
            case SID_SEARCH_ITEM:
            {
                if ( !s_pSrchItem )
                {
                    s_pSrchItem = new SvxSearchItem( SID_SEARCH_ITEM );
                    s_pSrchItem->SetFamily(SfxStyleFamily::Para);
                    s_pSrchItem->SetSearchString( m_pWrtShell->GetSelText() );
                }

                if( s_bJustOpened && m_pWrtShell->IsSelection() )
                {
                    OUString aText;
                    if( 1 == m_pWrtShell->GetCursorCnt() &&
                        !( aText = m_pWrtShell->SwCursorShell::GetSelText() ).isEmpty() )
                    {
                        s_pSrchItem->SetSearchString( aText );
                        s_pSrchItem->SetSelection( false );
                    }
                    else
                        s_pSrchItem->SetSelection( true );
                }

                s_bJustOpened = false;
                rSet.Put( *s_pSrchItem );
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
