/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
#include <svtools/txtcmp.hxx>
#include <svl/itempool.hxx>
#include <svl/eitem.hxx>
#include <svl/srchitem.hxx>
#include <sal/macros.h>
#include <sfx2/request.hxx>
#include <svx/srchdlg.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include "editeng/unolingu.hxx"
#include <swmodule.hxx>
#include <swwait.hxx>
#include <workctrl.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>                   // fuer Undo-Ids
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
using namespace ::com::sun::star::i18n;

/*--------------------------------------------------------------------
    Beschreibung:   Search Parameter
 --------------------------------------------------------------------*/

struct SwSearchOptions
{
    SwDocPositions eStart, eEnd;
    sal_Bool bDontWrap;

    SwSearchOptions( SwWrtShell* pSh, sal_Bool bBackward );
};


inline Window* GetParentWindow( SvxSearchDialog* pSrchDlg )
{
    Window* pWin;
    if( pSrchDlg && pSrchDlg->IsVisible() )
        pWin = pSrchDlg;
    else
        pWin = 0;
    return pWin;
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
    if (nSlot == FN_REPEAT_SEARCH && !pSrchItem)
    {
        if(bApi)
        {
            rReq.SetReturnValue(SfxBoolItem(nSlot, sal_False));
            nSlot = 0;
        }
    }
    if( pWrtShell->IsBlockMode() )
        pWrtShell->LeaveBlockMode();
    switch (nSlot)
    {
    // erstmal Nichts tun
    case SID_SEARCH_ITEM:
    {
        delete pSrchItem;
        pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();
    }
    break;

    case FID_SEARCH_ON:
        bJustOpened = sal_True;
        GetViewFrame()->GetBindings().Invalidate(SID_SEARCH_ITEM);
        break;

    case FID_SEARCH_OFF:
        if(pArgs)
        {
            // Dialog abmelden
            delete pSrchItem;
            pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();

            DELETEZ( pSrchList );
            DELETEZ( pReplList );

            if ( pWrp )
            {
                pSrchDlg = static_cast <SvxSearchDialog*> (pWrp->getDialog ());
                // die Search / Replace -Items merken wir uns
                const SearchAttrItemList* pList = pSrchDlg->GetSearchItemList();
                if( pList && pList->Count() )
                    pSrchList = new SearchAttrItemList( *pList );

                if( 0 != (pList = pSrchDlg->GetReplaceItemList() ) &&
                    pList->Count() )
                    pReplList = new SearchAttrItemList( *pList );
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
                    pSrchDlg = static_cast <SvxSearchDialog*> (pWrp->getDialog ());
                }
                else
                    pSrchDlg = 0;
            }

            if (pSrchDlg)
            {
                DELETEZ( pSrchList );
                DELETEZ( pReplList );

                const SearchAttrItemList* pList = pSrchDlg->GetSearchItemList();
                if( pList && pList->Count() )
                    pSrchList = new SearchAttrItemList( *pList );

                if( 0 != (pList = pSrchDlg->GetReplaceItemList() ) &&
                    pList->Count() )
                    pReplList = new SearchAttrItemList( *pList );
            }

            if (nSlot == FN_REPEAT_SEARCH)
            {
                OSL_ENSURE(pSrchItem, "SearchItem missing");
                if( !pSrchItem )
                    pSrchItem = new SvxSearchItem(SID_SEARCH_ITEM);
            }
            else
            {
                // SearchItem aus Request besorgen
                OSL_ENSURE(pArgs, "Args missing");
                if ( pArgs )
                {
                    delete pSrchItem;
                    pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();
                }
            }
            switch (pSrchItem->GetCommand())
            {
            case SVX_SEARCHCMD_FIND:
            {
                sal_Bool bRet = SearchAndWrap(bApi);
                if( bRet )
                    Scroll(pWrtShell->GetCharRect().SVRect());
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
                        Window* pParentWindow = GetParentWindow( pSrchDlg );
                        InfoBox( pParentWindow, SW_RES(MSG_NOT_FOUND)).Execute();
                    }
                    bFound = sal_False;
                }
                rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
            }
            break;
            case SVX_SEARCHCMD_REPLACE:
                {

                    // 1) Selektion ersetzen (nicht. wenn nur Attribute ersetzt
                    //    werden sollen)
//JP 27.04.95: warum ?
//      was ist, wenn man das gefundene nur attributieren will??

                    sal_uInt16 nCmd = SVX_SEARCHCMD_FIND;
                    if( pSrchItem->GetReplaceString().Len() ||
                        !pReplList )
                    {
                        // Verhindern, dass - falls der Suchstring im
                        // Ersetzungsstring enthalten ist - der ersetzte String
                        // noch einmal gefunden wird.

                        sal_Bool bBack = pSrchItem->GetBackward();
                        if (bBack)
                            pWrtShell->Push();
                        String aReplace( pSrchItem->GetReplaceString() );
                        SearchOptions aTmp( pSrchItem->GetSearchOptions() );
                        String *pBackRef = ReplaceBackReferences( aTmp, pWrtShell->GetCrsr() );
                        if( pBackRef )
                            pSrchItem->SetReplaceString( *pBackRef );
                        Replace();
                        if( pBackRef )
                        {
                            pSrchItem->SetReplaceString( aReplace );
                            delete pBackRef;
                        }
                        if (bBack)
                        {
                            pWrtShell->Pop();
                            pWrtShell->SwapPam();
                        }
                    }
                    else if( pReplList )
                        nCmd = SVX_SEARCHCMD_REPLACE;

                    // 2) Weiter suchen (ohne zu ersetzen!)

                    sal_uInt16 nOldCmd = pSrchItem->GetCommand();
                    pSrchItem->SetCommand( nCmd );
                    sal_Bool bRet = SearchAndWrap(bApi);
                    if( bRet )
                        Scroll( pWrtShell->GetCharRect().SVRect());
                    pSrchItem->SetCommand( nOldCmd );
                    rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
                }
                break;

            case SVX_SEARCHCMD_REPLACE_ALL:
                {
                    SwSearchOptions aOpts( pWrtShell, pSrchItem->GetBackward() );
                    bExtra = sal_False;
                    sal_uLong nFound;

                    {   //Scope for SwWait-Object
                        SwWait aWait( *GetDocShell(), sal_True );
                        pWrtShell->StartAllAction();
                        if (!pSrchItem->GetSelection())
                        {
                            // if we don't want to search in the selection...
                            pWrtShell->KillSelection(0, false);
                            // i#8288 "replace all" should not change cursor
                            // position, so save current cursor
                            pWrtShell->Push();
                            if (DOCPOS_START == aOpts.eEnd)
                            {
                                pWrtShell->EndDoc();
                            }
                            else
                            {
                                pWrtShell->SttDoc();
                            }
                        }
                        nFound = FUNC_Search( aOpts );
                        if (!pSrchItem->GetSelection())
                        {
                            // create it just to overwrite it with stack cursor
                            pWrtShell->CreateCrsr();
                            // i#8288 restore the original cursor position
                            pWrtShell->Pop(false);
                        }
                        pWrtShell->EndAllAction();
                    }

                    rReq.SetReturnValue(SfxBoolItem(nSlot, nFound != 0 && ULONG_MAX != nFound));
                    if( !nFound )
                    {
                        if( !bApi )
                        {
                            Window* pParentWindow = GetParentWindow( pSrchDlg );
                            InfoBox( pParentWindow, SW_RES(MSG_NOT_FOUND)).Execute();
                        }
                        bFound = sal_False;
                        return;
                    }

                    if( !bApi && ULONG_MAX != nFound)
                    {
                        String aText( SW_RES( STR_NB_REPLACED ) );
                        const xub_StrLen nPos = aText.Search( rtl::OUString("XX") );
                        aText.Erase( nPos, 2 );
                        aText.Insert( String::CreateFromInt32( nFound ), nPos );
                        Window* pParentWindow = GetParentWindow( pSrchDlg );
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

            SfxItemSet aSet( pWrtShell->GetAttrPool(), &aArr[0] );
            sal_uInt16 nWhich = SID_SEARCH_SEARCHSET;

            if ( FID_SEARCH_REPLACESET == nSlot )
            {
                nWhich = SID_SEARCH_REPLACESET;

                if ( pReplList )
                {
                    pReplList->Get( aSet );
                    DELETEZ( pReplList );
                }
            }
            else if ( pSrchList )
            {
                pSrchList->Get( aSet );
                DELETEZ( pSrchList );
            }
            rReq.SetReturnValue( SvxSetItem( nWhich, aSet ) );
        }
        break;
        default:
#if OSL_DEBUG_LEVEL > 1
            if(nSlot)
            {
                rtl::OStringBuffer sStr(RTL_CONSTASCII_STRINGPARAM("nSlot: "));
                sStr.append(static_cast<sal_Int32>(nSlot));
                sStr.append(RTL_CONSTASCII_STRINGPARAM(
                    " wrong Dispatcher (viewsrch.cxx)"));
                OSL_FAIL(sStr.getStr());
            }
#endif
            return;
    }
}


sal_Bool SwView::SearchAndWrap(sal_Bool bApi)
{
    SwSearchOptions aOpts( pWrtShell, pSrchItem->GetBackward() );

        // Startposition der Suche fuer WrapAround merken
        // Start- / EndAction wegen vielleicht bestehender Selektionen
        // aus 'Suche alle'
    pWrtShell->StartAllAction();
    pWrtShell->Push();
        // falls in selektierten Bereichen gesucht werden soll, duerfen sie
        // nicht aufgehoben werden
    if (!pSrchItem->GetSelection())
        pWrtShell->KillSelection(0, false);

    SwWait *pWait = new SwWait( *GetDocShell(), sal_True );
    if( FUNC_Search( aOpts ) )
    {
        bFound = sal_True;
        if(pWrtShell->IsSelFrmMode())
        {
            pWrtShell->UnSelectFrm();
            pWrtShell->LeaveSelFrmMode();
        }
        pWrtShell->Pop();
        pWrtShell->EndAllAction();
        delete pWait;
        return sal_True;
    }
    delete pWait, pWait = 0;

        // Suchen in den Sonderbereichen, wenn keine
        // Suche in Selektionen vorliegt. Bei Suche in Selektionen
        // wird ohnehin in diesen Sonderbereichen gesucht
    sal_Bool bHasSrchInOther = bExtra;
    if (!pSrchItem->GetSelection() && !bExtra )
    {
        bExtra = sal_True;
        if( FUNC_Search( aOpts ) )
        {
            bFound = sal_True;
            pWrtShell->Pop();
            pWrtShell->EndAllAction();
            return sal_True;
        }
        bExtra = sal_False;
    }
    else
        bExtra = !bExtra;

    const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
    SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);
    pSrchDlg = pWrp ? static_cast <SvxSearchDialog*> (pWrp->getDialog ()) : 0;

        // falls Startposition am Dokumentende / -anfang
    if (aOpts.bDontWrap)
    {
        pWrtShell->EndAllAction();
        if( !bApi )
        {
            Window* pParentWindow = GetParentWindow( pSrchDlg );
            InfoBox( pParentWindow, SW_RES(MSG_NOT_FOUND)).Execute();
        }
        bFound = sal_False;
        pWrtShell->Pop();
        return sal_False;
    }
    pWrtShell->EndAllAction();
        // noch mal mit WrapAround versuchen?

    if( bApi || RET_NO == QueryBox( GetParentWindow( pSrchDlg ),
                                        SW_RES( DOCPOS_START == aOpts.eEnd
                                            ? MSG_SEARCH_START
                                            : MSG_SEARCH_END )
                                    ).Execute() )
    {
        bFound = sal_False;
        pWrtShell->Pop();
        return sal_False;
    }
    pWrtShell->StartAllAction();
    pWrtShell->Pop(sal_False);
    pWait = new SwWait( *GetDocShell(), sal_True );

    sal_Bool bSrchBkwrd = DOCPOS_START == aOpts.eEnd;

    aOpts.eEnd =  bSrchBkwrd ? DOCPOS_START : DOCPOS_END;
    aOpts.eStart = bSrchBkwrd ? DOCPOS_END : DOCPOS_START;

    if (bHasSrchInOther)
    {
        pWrtShell->ClearMark();
        if (bSrchBkwrd)
            pWrtShell->EndDoc();
        else
            pWrtShell->SttDoc();
    }

    bFound = 0 != FUNC_Search( aOpts );
    pWrtShell->EndAllAction();
    delete pWait;
    if ( bFound )
        return bFound;
    if(!bApi)
    {
        Window* pParentWindow = GetParentWindow( pSrchDlg );
        InfoBox( pParentWindow, SW_RES(MSG_NOT_FOUND)).Execute();
    }
    return bFound = sal_False;
}


sal_Bool SwView::SearchAll(sal_uInt16* pFound)
{
    SwWait aWait( *GetDocShell(), sal_True );
    pWrtShell->StartAllAction();

    SwSearchOptions aOpts( pWrtShell, pSrchItem->GetBackward() );

    if (!pSrchItem->GetSelection())
    {
        // bestehende Selektionen aufheben,
        // wenn nicht in selektierten Bereichen gesucht werden soll
        pWrtShell->KillSelection(0, false);

        if( DOCPOS_START == aOpts.eEnd )
            pWrtShell->EndDoc();
        else
            pWrtShell->SttDoc();
    }
    bExtra = sal_False;
    sal_uInt16 nFound = (sal_uInt16)FUNC_Search( aOpts );
    if(pFound)
        *pFound = nFound;
    bFound = 0 != nFound;

    pWrtShell->EndAllAction();
    return bFound;
}


void SwView::Replace()
{
    SwWait aWait( *GetDocShell(), sal_True );

    pWrtShell->StartAllAction();

    if( pSrchItem->GetPattern() ) // Vorlagen?
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, pSrchItem->GetSearchString());
        aRewriter.AddRule(UndoArg2, SW_RESSTR(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, pSrchItem->GetReplaceString());

        pWrtShell->StartUndo(UNDO_UI_REPLACE_STYLE, &aRewriter);

        pWrtShell->SetTxtFmtColl( pWrtShell->GetParaStyle(
                            pSrchItem->GetReplaceString(),
                            SwWrtShell::GETSTYLE_CREATESOME ));

        pWrtShell->EndUndo();
    }
    else
    {
        if (GetPostItMgr()->HasActiveSidebarWin())
            GetPostItMgr()->Replace(pSrchItem);

        sal_Bool bReqReplace = true;

        if(pWrtShell->HasSelection())
        {
            /* check that the selection match the search string*/
            //save state
            SwPosition aStartPos = (* pWrtShell->GetSwCrsr()->Start());
            SwPosition aEndPos = (* pWrtShell->GetSwCrsr()->End());
            sal_Bool   bHasSelection = pSrchItem->GetSelection();
            sal_uInt16 nOldCmd = pSrchItem->GetCommand();

            //set state for checking if current selection has a match
            pSrchItem->SetCommand( SVX_SEARCHCMD_FIND );
            pSrchItem->SetSelection(true);

            //check if it matchs
            SwSearchOptions aOpts( pWrtShell, pSrchItem->GetBackward() );
            if( ! FUNC_Search(aOpts) )
            {

                //no matching therefore should not replace selection
                // => remove selection

                if(! pSrchItem->GetBackward() )
                {
                    (* pWrtShell->GetSwCrsr()->Start()) = aStartPos;
                    (* pWrtShell->GetSwCrsr()->End()) = aEndPos;
                }
                else
                {
                    (* pWrtShell->GetSwCrsr()->Start()) = aEndPos;
                    (* pWrtShell->GetSwCrsr()->End()) = aStartPos;
                }
                bReqReplace = false;
            }

            //set back old search state
            pSrchItem->SetCommand( nOldCmd );
            pSrchItem->SetSelection(bHasSelection);
        }
        /*
         * remove current selection
         * otherwise it is always replaced
         * no matter if the search string exists or not in the selection
         * Now the selection is removed and the next matching string is selected
         */

        if( bReqReplace )
        {

            sal_Bool bReplaced = pWrtShell->SwEditShell::Replace( pSrchItem->GetReplaceString(),
                                                                  pSrchItem->GetRegExp());
            if( bReplaced && pReplList && pReplList->Count() && pWrtShell->HasSelection() )
            {
                SfxItemSet aReplSet( pWrtShell->GetAttrPool(),
                                     aTxtFmtCollSetRange );
                if( pReplList->Get( aReplSet ).Count() )
                {
                    ::SfxToSwPageDescAttr( *pWrtShell, aReplSet );
                    pWrtShell->SwEditShell::SetAttr( aReplSet );
                }
            }
        }
    }

    pWrtShell->EndAllAction();
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
    sal_Bool bDoReplace = pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE ||
                      pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE_ALL;

    int eRanges = pSrchItem->GetSelection() ?
        FND_IN_SEL : bExtra ? FND_IN_OTHER : FND_IN_BODY;
    if (pSrchItem->GetCommand() == SVX_SEARCHCMD_FIND_ALL    ||
        pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE_ALL)
        eRanges |= FND_IN_SELALL;

    pWrtShell->SttSelect();

    static sal_uInt16 aSearchAttrRange[] = {
        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
        RES_CHRATR_BEGIN, RES_CHRATR_END-1,
        RES_PARATR_BEGIN, RES_PARATR_END-1,
        SID_ATTR_PARA_MODEL, SID_ATTR_PARA_KEEP,
        0 };

    SfxItemSet aSrchSet( pWrtShell->GetAttrPool(), aSearchAttrRange);
    if( pSrchList && pSrchList->Count() )
    {
        pSrchList->Get( aSrchSet );

        /*  -- Seitenumbruch mit Seitenvorlage */
        ::SfxToSwPageDescAttr( *pWrtShell, aSrchSet );
    }

    SfxItemSet* pReplSet = 0;
    if( bDoReplace && pReplList && pReplList->Count() )
    {
        pReplSet = new SfxItemSet( pWrtShell->GetAttrPool(),
                                        aSearchAttrRange );
        pReplList->Get( *pReplSet );

        /*  -- Seitenumbruch mit Seitenvorlage */
        ::SfxToSwPageDescAttr( *pWrtShell, *pReplSet );

        if( !pReplSet->Count() )        // schade, die Attribute
            DELETEZ( pReplSet );        // kennen wir nicht
    }

    //
    // build SearchOptions to be used
    //
    SearchOptions aSearchOpt( pSrchItem->GetSearchOptions() );
    aSearchOpt.Locale = SvxCreateLocale( (sal_uInt16)GetAppLanguage() );
    if( !bDoReplace )
        aSearchOpt.replaceString = aEmptyStr;

    sal_uLong nFound;
    if( aSrchSet.Count() || ( pReplSet && pReplSet->Count() ))
    {
        nFound = pWrtShell->SearchAttr(
            aSrchSet,
            !pSrchItem->GetPattern(),
            rOptions.eStart,
            rOptions.eEnd,
            FindRanges(eRanges),
            pSrchItem->GetSearchString().Len() ? &aSearchOpt : 0,
            pReplSet );
    }
    else if( pSrchItem->GetPattern() )
    {
        // Suchen (und ersetzen) von Vorlagen
        const String sRplStr( pSrchItem->GetReplaceString() );
        nFound = pWrtShell->SearchTempl( pSrchItem->GetSearchString(),
            rOptions.eStart,
            rOptions.eEnd,
            FindRanges(eRanges),
            bDoReplace ? &sRplStr : 0 );
    }
    else
    {
        // Normale Suche
        nFound = pWrtShell->SearchPattern(aSearchOpt, pSrchItem->GetNotes(),
                                          rOptions.eStart,
                                          rOptions.eEnd,
                                          FindRanges(eRanges),
                                          bDoReplace );
    }
    pWrtShell->EndSelect();
    return nFound;
}

Dialog* SwView::GetSearchDialog()
{
    const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
    SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)SfxViewFrame::Current()->GetChildWindow(nId);
    if ( pWrp )
        pSrchDlg = pWrp->getDialog ();
    else
        pSrchDlg = 0;
    return pSrchDlg;
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
                if ( !pSrchItem )
                {
                    pSrchItem = new SvxSearchItem( SID_SEARCH_ITEM );
                    pSrchItem->SetFamily(SFX_STYLE_FAMILY_PARA);
                    pSrchItem->SetSearchString( pWrtShell->GetSelTxt() );
                }

                if( bJustOpened && pWrtShell->IsSelection() )
                {
                    String aTxt;
                    if( 1 == pWrtShell->GetCrsrCnt() &&
                        ( aTxt = pWrtShell->SwCrsrShell::GetSelTxt() ).Len() )
                    {
                        pSrchItem->SetSearchString( aTxt );
                        pSrchItem->SetSelection( sal_False );
                    }
                    else
                        pSrchItem->SetSelection( sal_True );
                }

                bJustOpened = sal_False;
                rSet.Put( *pSrchItem );
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
