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

#include <boost/scoped_ptr.hpp>

#include <hintids.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <svx/pageitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <unotools/textsearch.hxx>
#include <svl/itempool.hxx>
#include <svl/eitem.hxx>
#include <svl/srchitem.hxx>
#include <sal/macros.h>
#include <sfx2/request.hxx>
#include <svx/srchdlg.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include "editeng/unolingu.hxx"
#include <swmodule.hxx>
#include <swwait.hxx>
#include <workctrl.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>                   // for Undo-Ids
#include <uitool.hxx>
#include <cmdid.h>
#include <docsh.hxx>
#include <doc.hxx>
#include <unocrsr.hxx>

#include <view.hrc>
#include <SwRewriter.hxx>
#include <comcore.hrc>

#include "PostItMgr.hxx"

using namespace com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//Search Parameter

struct SwSearchOptions
{
    SwDocPositions eStart, eEnd;
    sal_Bool bDontWrap;

    SwSearchOptions( SwWrtShell* pSh, sal_Bool bBackward );
};

inline Window* GetParentWindow( SvxSearchDialog* m_pSrchDlg )
{
    Window* pWin;
    if( m_pSrchDlg && m_pSrchDlg->IsVisible() )
        pWin = m_pSrchDlg;
    else
        pWin = 0;
    return pWin;
}

inline void ShowNotFoundInfoBox( SvxSearchDialog* m_pSrchDlg )
{
    Window* pParentWindow = GetParentWindow( m_pSrchDlg );
    InfoBox aBox( pParentWindow, SW_RES(MSG_NOT_FOUND));
    if (pParentWindow)
    {
        aBox.SetText(pParentWindow->GetText());
    }
    else
    {
        aBox.SetText(SVX_RESSTR(RID_SVXSTR_FINDBAR_FIND));
    }
    aBox.Execute();
}

void SwView::ExecSearch(SfxRequest& rReq, sal_Bool bNoMessage)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
    SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);
    sal_Bool bQuiet = sal_False;
    if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_SEARCH_QUIET, sal_False, &pItem))
        bQuiet = ((const SfxBoolItem*) pItem)->GetValue();

    sal_Bool bApi = bQuiet | bNoMessage;

    sal_uInt16 nSlot = rReq.GetSlot();
    if (nSlot == FN_REPEAT_SEARCH && !m_pSrchItem)
    {
        if(bApi)
        {
            rReq.SetReturnValue(SfxBoolItem(nSlot, sal_False));
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
        delete m_pSrchItem;
        m_pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();
    }
    break;

    case FID_SEARCH_ON:
        m_bJustOpened = true;
        GetViewFrame()->GetBindings().Invalidate(SID_SEARCH_ITEM);
        break;

    case FID_SEARCH_OFF:
        if(pArgs)
        {
            // Unregister dialog
            delete m_pSrchItem;
            m_pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();

            DELETEZ( m_pSrchList );
            DELETEZ( m_pReplList );

            if ( pWrp )
            {
                m_pSrchDlg = static_cast <SvxSearchDialog*> (pWrp->getDialog ());
                // We will remember the search-/replace items.
                const SearchAttrItemList* pList = m_pSrchDlg->GetSearchItemList();
                if( pList && pList->Count() )
                    m_pSrchList = new SearchAttrItemList( *pList );

                if( 0 != (pList = m_pSrchDlg->GetReplaceItemList() ) &&
                    pList->Count() )
                    m_pReplList = new SearchAttrItemList( *pList );
            }
        }
        break;

        case FN_REPEAT_SEARCH:
        case FID_SEARCH_NOW:
        {
            {
                if(FID_SEARCH_NOW == nSlot && !rReq.IsAPI())
                    SwView::SetMoveType(NID_SRCH_REP);
                if ( pWrp )
                {
                    m_pSrchDlg = static_cast <SvxSearchDialog*> (pWrp->getDialog ());
                }
                else
                    m_pSrchDlg = 0;
            }

            if (m_pSrchDlg)
            {
                DELETEZ( m_pSrchList );
                DELETEZ( m_pReplList );

                const SearchAttrItemList* pList = m_pSrchDlg->GetSearchItemList();
                if( pList && pList->Count() )
                    m_pSrchList = new SearchAttrItemList( *pList );

                if( 0 != (pList = m_pSrchDlg->GetReplaceItemList() ) &&
                    pList->Count() )
                    m_pReplList = new SearchAttrItemList( *pList );
            }

            if (nSlot == FN_REPEAT_SEARCH)
            {
                OSL_ENSURE(m_pSrchItem, "SearchItem missing");
                if( !m_pSrchItem )
                    m_pSrchItem = new SvxSearchItem(SID_SEARCH_ITEM);
            }
            else
            {
                // Get SearchItem from request
                OSL_ENSURE(pArgs, "Args missing");
                if ( pArgs )
                {
                    delete m_pSrchItem;
                    m_pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();
                }
            }
            switch (m_pSrchItem->GetCommand())
            {
            case SVX_SEARCHCMD_FIND:
            {
                sal_Bool bRet = SearchAndWrap(bApi);
                if( bRet )
                    Scroll(m_pWrtShell->GetCharRect().SVRect());
                rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
            }
            break;
            case SVX_SEARCHCMD_FIND_ALL:
            {
                sal_Bool bRet = SearchAll();
                if( !bRet )
                {
                    if( !bApi )
                    {
                        ShowNotFoundInfoBox( m_pSrchDlg );
                    }
                    m_bFound = sal_False;
                }
                rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
            }
            break;
            case SVX_SEARCHCMD_REPLACE:
                {

                    // 1) Replace selection (Not if only attributes should be replaced)
//JP 27.04.95: Why ?
//      what if you only want to assign attributes to the found??

                    sal_uInt16 nCmd = SVX_SEARCHCMD_FIND;
                    if( !m_pSrchItem->GetReplaceString().isEmpty() ||
                        !m_pReplList )
                    {
                        // Prevent, that the replaced string will be found again
                        // if the replacement string is containing the search string.
                        sal_Bool bBack = m_pSrchItem->GetBackward();
                        if (bBack)
                            m_pWrtShell->Push();
                        String aReplace( m_pSrchItem->GetReplaceString() );
                        SearchOptions aTmp( m_pSrchItem->GetSearchOptions() );
                        String *pBackRef = ReplaceBackReferences( aTmp, m_pWrtShell->GetCrsr() );
                        if( pBackRef )
                            m_pSrchItem->SetReplaceString( *pBackRef );
                        Replace();
                        if( pBackRef )
                        {
                            m_pSrchItem->SetReplaceString( aReplace );
                            delete pBackRef;
                        }
                        if (bBack)
                        {
                            m_pWrtShell->Pop();
                            m_pWrtShell->SwapPam();
                        }
                    }
                    else if( m_pReplList )
                        nCmd = SVX_SEARCHCMD_REPLACE;

                    // 2) Search further (without replacing!)

                    sal_uInt16 nOldCmd = m_pSrchItem->GetCommand();
                    m_pSrchItem->SetCommand( nCmd );
                    sal_Bool bRet = SearchAndWrap(bApi);
                    if( bRet )
                        Scroll( m_pWrtShell->GetCharRect().SVRect());
                    m_pSrchItem->SetCommand( nOldCmd );
                    rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
                }
                break;

            case SVX_SEARCHCMD_REPLACE_ALL:
                {
                    SwSearchOptions aOpts( m_pWrtShell, m_pSrchItem->GetBackward() );
                    m_bExtra = sal_False;
                    sal_uLong nFound;

                    {   //Scope for SwWait-Object
                        SwWait aWait( *GetDocShell(), sal_True );
                        m_pWrtShell->StartAllAction();
                        if (!m_pSrchItem->GetSelection())
                        {
                            // if we don't want to search in the selection...
                            m_pWrtShell->KillSelection(0, false);
                            // i#8288 "replace all" should not change cursor
                            // position, so save current cursor
                            m_pWrtShell->Push();
                            if (DOCPOS_START == aOpts.eEnd)
                            {
                                m_pWrtShell->EndDoc();
                            }
                            else
                            {
                                m_pWrtShell->SttDoc();
                            }
                        }
                        nFound = FUNC_Search( aOpts );
                        if (!m_pSrchItem->GetSelection())
                        {
                            // create it just to overwrite it with stack cursor
                            m_pWrtShell->CreateCrsr();
                            // i#8288 restore the original cursor position
                            m_pWrtShell->Pop(false);
                        }
                        m_pWrtShell->EndAllAction();
                    }

                    rReq.SetReturnValue(SfxBoolItem(nSlot, nFound != 0 && ULONG_MAX != nFound));
                    if( !nFound )
                    {
                        if( !bApi )
                        {
                            ShowNotFoundInfoBox( m_pSrchDlg );
                        }
                        m_bFound = sal_False;
                        return;
                    }

                    if( !bApi && ULONG_MAX != nFound)
                    {
                        String aText( SW_RES( STR_NB_REPLACED ) );
                        const xub_StrLen nPos = aText.Search( OUString("XX") );
                        aText.Erase( nPos, 2 );
                        aText.Insert( OUString::number( nFound ), nPos );
                        Window* pParentWindow = GetParentWindow( m_pSrchDlg );
                        InfoBox( pParentWindow, aText ).Execute();
                    }
                }
                break;
            }

            uno::Reference< frame::XDispatchRecorder > xRecorder =
                    GetViewFrame()->GetBindings().GetRecorder();
            //prevent additional dialogs in recorded macros
            if ( xRecorder.is() )
                rReq.AppendItem(SfxBoolItem(SID_SEARCH_QUIET, sal_True));

            rReq.Done();
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
// insert position for CJK/CTL attributes!
/*14 */         RES_PARATR_LINESPACING, RES_PARATR_HYPHENZONE,
/*16 */         RES_PARATR_REGISTER,    RES_PARATR_REGISTER,
/*18 */         RES_PARATR_VERTALIGN,   RES_PARATR_VERTALIGN,
/*20 */         RES_LR_SPACE,           RES_UL_SPACE,
/*22 */         SID_ATTR_PARA_MODEL,    SID_ATTR_PARA_KEEP,
/*24 */         0
            };

            static const sal_uInt16 aCJKAttr[] =
            {
                RES_CHRATR_CJK_FONT,    RES_CHRATR_CJK_WEIGHT,
                RES_CHRATR_EMPHASIS_MARK, RES_CHRATR_TWO_LINES,
                RES_PARATR_SCRIPTSPACE, RES_PARATR_FORBIDDEN_RULES
            };
            static const sal_uInt16 aCTLAttr[] =
            {
                RES_CHRATR_CTL_FONT,    RES_CHRATR_CTL_WEIGHT
            };

            std::vector<sal_uInt16> aArr;
            aArr.insert( aArr.begin(), aNormalAttr,
                    aNormalAttr + SAL_N_ELEMENTS( aNormalAttr ));
            if( SW_MOD()->GetCTLOptions().IsCTLFontEnabled() )
            {
                aArr.insert( aArr.begin() + 14, aCTLAttr,
                        aCTLAttr + SAL_N_ELEMENTS( aCTLAttr ));
            }
            SvtCJKOptions aCJKOpt;
            if( aCJKOpt.IsAnyEnabled() )
            {
                aArr.insert( aArr.begin() + 14, aCJKAttr,
                        aCJKAttr + SAL_N_ELEMENTS( aCJKAttr ));
            }

            SfxItemSet aSet( m_pWrtShell->GetAttrPool(), &aArr[0] );
            sal_uInt16 nWhich = SID_SEARCH_SEARCHSET;

            if ( FID_SEARCH_REPLACESET == nSlot )
            {
                nWhich = SID_SEARCH_REPLACESET;

                if ( m_pReplList )
                {
                    m_pReplList->Get( aSet );
                    DELETEZ( m_pReplList );
                }
            }
            else if ( m_pSrchList )
            {
                m_pSrchList->Get( aSet );
                DELETEZ( m_pSrchList );
            }
            rReq.SetReturnValue( SvxSetItem( nWhich, aSet ) );
        }
        break;
        default:
#if OSL_DEBUG_LEVEL > 1
            if(nSlot)
            {
                OString sStr("nSlot: " + OString::number(nSlot) + " wrong Dispatcher (viewsrch.cxx)");
                OSL_FAIL(sStr.getStr());
            }
#endif
            return;
    }
}


sal_Bool SwView::SearchAndWrap(sal_Bool bApi)
{
    SwSearchOptions aOpts( m_pWrtShell, m_pSrchItem->GetBackward() );

        // Remember starting position of the search for wraparound
        // Start- / EndAction perhaps because existing selections of 'search all'
    m_pWrtShell->StartAllAction();
    m_pWrtShell->Push();

    // fdo#65014 : Ensure that the point of the cursor is at the extremity of the
    // selection closest to the end being searched to as to exclude the selected
    // region from the search. (This doesn't work in the case of multiple
    // selected regions as the cursor doesn't mark the selection in that case.)
    m_pWrtShell->GetCrsr()->Normalize( m_pSrchItem->GetBackward() );

        // If you want to search in selected areas, they must not be unselected.
    if (!m_pSrchItem->GetSelection())
        m_pWrtShell->KillSelection(0, false);

    SwWait *pWait = new SwWait( *GetDocShell(), sal_True );
    if( FUNC_Search( aOpts ) )
    {
        m_bFound = sal_True;
        if(m_pWrtShell->IsSelFrmMode())
        {
            m_pWrtShell->UnSelectFrm();
            m_pWrtShell->LeaveSelFrmMode();
        }
        m_pWrtShell->Pop();
        m_pWrtShell->EndAllAction();
        delete pWait;
        return sal_True;
    }
    delete pWait, pWait = 0;

        // Search in the specialized areas when no search is present in selections.
        // When searching selections will already searched in these special areas.
    bool bHasSrchInOther = m_bExtra;
    if (!m_pSrchItem->GetSelection() && !m_bExtra )
    {
        m_bExtra = true;
        if( FUNC_Search( aOpts ) )
        {
            m_bFound = sal_True;
            m_pWrtShell->Pop();
            m_pWrtShell->EndAllAction();
            return sal_True;
        }
        m_bExtra = false;
    }
    else
        m_bExtra = !m_bExtra;

    const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
    SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);
    m_pSrchDlg = pWrp ? static_cast <SvxSearchDialog*> (pWrp->getDialog ()) : 0;

        // If starting position is at the end or beginning of the document.
    if (aOpts.bDontWrap)
    {
        m_pWrtShell->EndAllAction();
        if( !bApi )
        {
            ShowNotFoundInfoBox( m_pSrchDlg );
        }
        m_bFound = sal_False;
        m_pWrtShell->Pop();
        return sal_False;
    }
    m_pWrtShell->EndAllAction();
        // Try again with WrapAround?

    if( bApi || RET_NO == QueryBox( GetParentWindow( m_pSrchDlg ),
                                        SW_RES( DOCPOS_START == aOpts.eEnd
                                            ? MSG_SEARCH_START
                                            : MSG_SEARCH_END )
                                    ).Execute() )
    {
        m_bFound = sal_False;
        m_pWrtShell->Pop();
        return sal_False;
    }
    m_pWrtShell->StartAllAction();
    m_pWrtShell->Pop(sal_False);
    pWait = new SwWait( *GetDocShell(), sal_True );

    bool bSrchBkwrd = DOCPOS_START == aOpts.eEnd;

    aOpts.eEnd =  bSrchBkwrd ? DOCPOS_START : DOCPOS_END;
    aOpts.eStart = bSrchBkwrd ? DOCPOS_END : DOCPOS_START;

    if (bHasSrchInOther)
    {
        m_pWrtShell->ClearMark();
        if (bSrchBkwrd)
            m_pWrtShell->EndDoc();
        else
            m_pWrtShell->SttDoc();
    }

    m_bFound = 0 != FUNC_Search( aOpts );
    m_pWrtShell->EndAllAction();
    delete pWait;
    if ( m_bFound )
        return m_bFound;
    if(!bApi)
    {
        ShowNotFoundInfoBox( m_pSrchDlg );
    }
    return m_bFound = sal_False;
}


sal_Bool SwView::SearchAll(sal_uInt16* pFound)
{
    SwWait aWait( *GetDocShell(), sal_True );
    m_pWrtShell->StartAllAction();

    SwSearchOptions aOpts( m_pWrtShell, m_pSrchItem->GetBackward() );

    if (!m_pSrchItem->GetSelection())
    {
        // Cancel existing selections, if should not be sought in selected areas.
        m_pWrtShell->KillSelection(0, false);

        if( DOCPOS_START == aOpts.eEnd )
            m_pWrtShell->EndDoc();
        else
            m_pWrtShell->SttDoc();
    }
    m_bExtra = false;
    sal_uInt16 nFound = (sal_uInt16)FUNC_Search( aOpts );
    if(pFound)
        *pFound = nFound;
    m_bFound = 0 != nFound;

    m_pWrtShell->EndAllAction();
    return m_bFound;
}


void SwView::Replace()
{
    SwWait aWait( *GetDocShell(), sal_True );

    m_pWrtShell->StartAllAction();

    if( m_pSrchItem->GetPattern() ) // Templates?
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, m_pSrchItem->GetSearchString());
        aRewriter.AddRule(UndoArg2, SW_RESSTR(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, m_pSrchItem->GetReplaceString());

        m_pWrtShell->StartUndo(UNDO_UI_REPLACE_STYLE, &aRewriter);

        m_pWrtShell->SetTxtFmtColl( m_pWrtShell->GetParaStyle(
                            m_pSrchItem->GetReplaceString(),
                            SwWrtShell::GETSTYLE_CREATESOME ));

        m_pWrtShell->EndUndo();
    }
    else
    {
        if (GetPostItMgr()->HasActiveSidebarWin())
            GetPostItMgr()->Replace(m_pSrchItem);

        sal_Bool bReqReplace = true;

        if(m_pWrtShell->HasSelection())
        {
            /* check that the selection match the search string*/
            //save state
            SwPosition aStartPos = (* m_pWrtShell->GetSwCrsr()->Start());
            SwPosition aEndPos = (* m_pWrtShell->GetSwCrsr()->End());
            sal_Bool   bHasSelection = m_pSrchItem->GetSelection();
            sal_uInt16 nOldCmd = m_pSrchItem->GetCommand();

            //set state for checking if current selection has a match
            m_pSrchItem->SetCommand( SVX_SEARCHCMD_FIND );
            m_pSrchItem->SetSelection(true);

            //check if it matchs
            SwSearchOptions aOpts( m_pWrtShell, m_pSrchItem->GetBackward() );
            if( ! FUNC_Search(aOpts) )
            {

                //no matching therefore should not replace selection
                // => remove selection

                if(! m_pSrchItem->GetBackward() )
                {
                    (* m_pWrtShell->GetSwCrsr()->Start()) = aStartPos;
                    (* m_pWrtShell->GetSwCrsr()->End()) = aEndPos;
                }
                else
                {
                    (* m_pWrtShell->GetSwCrsr()->Start()) = aEndPos;
                    (* m_pWrtShell->GetSwCrsr()->End()) = aStartPos;
                }
                bReqReplace = false;
            }

            //set back old search state
            m_pSrchItem->SetCommand( nOldCmd );
            m_pSrchItem->SetSelection(bHasSelection);
        }
        /*
         * remove current selection
         * otherwise it is always replaced
         * no matter if the search string exists or not in the selection
         * Now the selection is removed and the next matching string is selected
         */

        if( bReqReplace )
        {

            sal_Bool bReplaced = m_pWrtShell->SwEditShell::Replace( m_pSrchItem->GetReplaceString(),
                                                                  m_pSrchItem->GetRegExp());
            if( bReplaced && m_pReplList && m_pReplList->Count() && m_pWrtShell->HasSelection() )
            {
                SfxItemSet aReplSet( m_pWrtShell->GetAttrPool(),
                                     aTxtFmtCollSetRange );
                if( m_pReplList->Get( aReplSet ).Count() )
                {
                    ::SfxToSwPageDescAttr( *m_pWrtShell, aReplSet );
                    m_pWrtShell->SwEditShell::SetAttr( aReplSet );
                }
            }
        }
    }

    m_pWrtShell->EndAllAction();
}

SwSearchOptions::SwSearchOptions( SwWrtShell* pSh, sal_Bool bBackward )
{
    eStart = DOCPOS_CURR;
    if( bBackward )
    {
        eEnd = DOCPOS_START;
        bDontWrap = pSh->IsEndOfDoc();
    }
    else
    {
        eEnd = DOCPOS_END;
        bDontWrap = pSh->IsStartOfDoc();
    }
}

sal_uLong SwView::FUNC_Search( const SwSearchOptions& rOptions )
{
    sal_Bool bDoReplace = m_pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE ||
                      m_pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE_ALL;

    int eRanges = m_pSrchItem->GetSelection() ?
        FND_IN_SEL : m_bExtra ? FND_IN_OTHER : FND_IN_BODY;
    if (m_pSrchItem->GetCommand() == SVX_SEARCHCMD_FIND_ALL    ||
        m_pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE_ALL)
        eRanges |= FND_IN_SELALL;

    m_pWrtShell->SttSelect();

    static const sal_uInt16 aSearchAttrRange[] = {
        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
        RES_CHRATR_BEGIN, RES_CHRATR_END-1,
        RES_PARATR_BEGIN, RES_PARATR_END-1,
        SID_ATTR_PARA_MODEL, SID_ATTR_PARA_KEEP,
        0 };

    SfxItemSet aSrchSet( m_pWrtShell->GetAttrPool(), aSearchAttrRange);
    if( m_pSrchList && m_pSrchList->Count() )
    {
        m_pSrchList->Get( aSrchSet );

        // -- Page break with page template
        ::SfxToSwPageDescAttr( *m_pWrtShell, aSrchSet );
    }

    SfxItemSet* pReplSet = 0;
    if( bDoReplace && m_pReplList && m_pReplList->Count() )
    {
        pReplSet = new SfxItemSet( m_pWrtShell->GetAttrPool(),
                                        aSearchAttrRange );
        m_pReplList->Get( *pReplSet );

        // -- Page break with page template
        ::SfxToSwPageDescAttr( *m_pWrtShell, *pReplSet );

        if( !pReplSet->Count() )        // too bad, we don't know
            DELETEZ( pReplSet );        // the attributes
    }

    //
    // build SearchOptions to be used
    //
    SearchOptions aSearchOpt( m_pSrchItem->GetSearchOptions() );
    aSearchOpt.Locale = GetAppLanguageTag().getLocale();
    if( !bDoReplace )
        aSearchOpt.replaceString = aEmptyStr;

    sal_uLong nFound;
    if( aSrchSet.Count() || ( pReplSet && pReplSet->Count() ))
    {
        nFound = m_pWrtShell->SearchAttr(
            aSrchSet,
            !m_pSrchItem->GetPattern(),
            rOptions.eStart,
            rOptions.eEnd,
            FindRanges(eRanges),
            !m_pSrchItem->GetSearchString().isEmpty() ? &aSearchOpt : 0,
            pReplSet );
    }
    else if( m_pSrchItem->GetPattern() )
    {
        // Searching (and replacing) templates
        const String sRplStr( m_pSrchItem->GetReplaceString() );
        nFound = m_pWrtShell->SearchTempl( m_pSrchItem->GetSearchString(),
            rOptions.eStart,
            rOptions.eEnd,
            FindRanges(eRanges),
            bDoReplace ? &sRplStr : 0 );
    }
    else
    {
        // Normal search
        nFound = m_pWrtShell->SearchPattern(aSearchOpt, m_pSrchItem->GetNotes(),
                                          rOptions.eStart,
                                          rOptions.eEnd,
                                          FindRanges(eRanges),
                                          bDoReplace );
    }
    m_pWrtShell->EndSelect();
    return nFound;
}

Dialog* SwView::GetSearchDialog()
{
    const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
    SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)SfxViewFrame::Current()->GetChildWindow(nId);
    if ( pWrp )
        m_pSrchDlg = pWrp->getDialog ();
    else
        m_pSrchDlg = 0;
    return m_pSrchDlg;
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
                sal_uInt16 nOpt = 0xFFFF;
                if( GetDocShell()->IsReadOnly() )
                    nOpt &= ~( SEARCH_OPTIONS_REPLACE |
                               SEARCH_OPTIONS_REPLACE_ALL );
                rSet.Put( SfxUInt16Item( SID_SEARCH_OPTIONS, nOpt));
            }
            break;
            case SID_SEARCH_ITEM:
            {
                if ( !m_pSrchItem )
                {
                    m_pSrchItem = new SvxSearchItem( SID_SEARCH_ITEM );
                    m_pSrchItem->SetFamily(SFX_STYLE_FAMILY_PARA);
                    m_pSrchItem->SetSearchString( m_pWrtShell->GetSelTxt() );
                }

                if( m_bJustOpened && m_pWrtShell->IsSelection() )
                {
                    String aTxt;
                    if( 1 == m_pWrtShell->GetCrsrCnt() &&
                        ( aTxt = m_pWrtShell->SwCrsrShell::GetSelTxt() ).Len() )
                    {
                        m_pSrchItem->SetSearchString( aTxt );
                        m_pSrchItem->SetSelection( sal_False );
                    }
                    else
                        m_pSrchItem->SetSelection( sal_True );
                }

                m_bJustOpened = false;
                rSet.Put( *m_pSrchItem );
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
