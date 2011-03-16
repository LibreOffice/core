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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "hintids.hxx"
#include <vcl/graph.hxx>
#include <svx/galbrws.hxx>
#include <svl/srchitem.hxx>
#include <SwSpellDialogChildWindow.hxx>
#include <svl/eitem.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <viewopt.hxx>
#include <globals.h>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <svx/srchdlg.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <uivwimp.hxx>
#include <avmedia/mediaplayer.hxx>
//#include <swlinguconfig.hxx>
#include <swmodule.hxx>

#include <sfx2/objface.hxx>
#include <navipi.hxx>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include "view.hxx"
#include "basesh.hxx"
#include "docsh.hxx"
#include "doc.hxx"
#include "globals.hrc"
#include "cmdid.h"          // FN_       ...
#include "globdoc.hxx"
#include "wview.hxx"
#include "shells.hrc"

#define OLEObjects
#define SwView
#define Frames
#define Graphics
#define Tables
#define Controls
#define GlobalContents
#define Text
#define Frame
#define Graphic
#define Object
#define Draw
#define TextDrawText
#define TextInTable
#define ListInText
#define ListInTable
#define WebTextInTable
#define WebListInText
#define WebListInTable
#define TextPage
#include <sfx2/msg.hxx>
#include "swslots.hxx"
#include <PostItMgr.hxx>


using namespace ::com::sun::star;
using ::rtl::OUString;

#include <unotools/moduleoptions.hxx>

#include <IDocumentSettingAccess.hxx>

#include <unomid.h>

SFX_IMPL_NAMED_VIEWFACTORY(SwView, "Default")
{
    if ( SvtModuleOptions().IsWriter() )
    {
        SFX_VIEW_REGISTRATION(SwDocShell);
        SFX_VIEW_REGISTRATION(SwGlobalDocShell);
    }
}

SFX_IMPL_INTERFACE( SwView, SfxViewShell, SW_RES(RID_TOOLS_TOOLBOX) )
{
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION(SID_NAVIGATOR);
    SFX_CHILDWINDOW_REGISTRATION(SID_TASKPANE);
    SFX_CHILDWINDOW_REGISTRATION(SfxTemplateDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SwSpellDialogChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(FN_REDLINE_ACCEPT);
    SFX_CHILDWINDOW_REGISTRATION(SID_HYPERLINK_DIALOG);
    SFX_CHILDWINDOW_REGISTRATION(GalleryChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(::avmedia::MediaPlayer::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(FN_INSERT_FIELD_DATA_ONLY);
        SFX_FEATURED_CHILDWINDOW_REGISTRATION(FN_SYNC_LABELS,           CHILDWIN_LABEL    );
        SFX_FEATURED_CHILDWINDOW_REGISTRATION(FN_MAILMERGE_CHILDWINDOW, CHILDWIN_MAILMERGE);
//    SFX_CHILDWINDOW_REGISTRATION(FN_MAILMERGE_SENDMAIL_CHILDWINDOW);
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS|
                                SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                SW_RES(RID_TOOLS_TOOLBOX) );
}

TYPEINIT1(SwView,SfxViewShell)

/*-----------------13.12.97 11:06-------------------

--------------------------------------------------*/
ShellModes  SwView::GetShellMode()
{
    return pViewImpl->GetShellMode();
}

/*-----------------13.12.97 11:28-------------------

--------------------------------------------------*/
view::XSelectionSupplier* SwView::GetUNOObject()
{
    return pViewImpl->GetUNOObject();
}
/* -----------------------------06.05.2002 13:18------------------------------

 ---------------------------------------------------------------------------*/
void SwView::ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions)
{
    pWrtShell->ApplyAccessiblityOptions(rAccessibilityOptions);
    //to enable the right state of the selection cursor in readonly documents
    if(GetDocShell()->IsReadOnly())
        pWrtShell->ShowCrsr();

}
/*-- 26.05.2004 09:14:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void   SwView::SetMailMergeConfigItem(SwMailMergeConfigItem*  pConfigItem,
                sal_uInt16 nRestart, sal_Bool bIsSource)
{
    pViewImpl->SetMailMergeConfigItem(pConfigItem, nRestart, bIsSource);
    UIFeatureChanged();
}
/*-- 26.05.2004 09:14:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeConfigItem*  SwView::GetMailMergeConfigItem()
{
    return pViewImpl->GetMailMergeConfigItem();
}
/*-- 26.05.2004 09:14:25---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_uInt16 SwView::GetMailMergeRestartPage() const
{
    return pViewImpl->GetMailMergeRestartPage();
}
/*-- 03.09.2004 11:56:33---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwView::IsMailMergeSourceView() const
{
    return pViewImpl->IsMailMergeSourceView();
}
/*-- 12.04.2006 11:51:40---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool lcl_IsViewMarks( const SwViewOption& rVOpt )
{
    return  rVOpt.IsHardBlank() &&
            rVOpt.IsSoftHyph() &&
            SwViewOption::IsFieldShadings();
}
void lcl_SetViewMarks(SwViewOption& rVOpt, sal_Bool bOn )
{
    rVOpt.SetHardBlank(bOn);
    rVOpt.SetSoftHyph(bOn);
    SwViewOption::SetAppearanceFlag(
            VIEWOPT_FIELD_SHADINGS, bOn, sal_True);
}

void lcl_SetViewMetaChars( SwViewOption& rVOpt, sal_Bool bOn)
{
    rVOpt.SetViewMetaChars( bOn );
    if(bOn && !(rVOpt.IsParagraph()     ||
            rVOpt.IsTab()       ||
            rVOpt.IsLineBreak() ||
            rVOpt.IsShowHiddenChar() ||
            rVOpt.IsBlank()))
    {
        rVOpt.SetParagraph(bOn);
        rVOpt.SetTab(bOn);
        rVOpt.SetLineBreak(bOn);
        rVOpt.SetBlank(bOn);
        rVOpt.SetShowHiddenChar(bOn);
    }
}

void SwView::RecheckBrowseMode()
{
    // OS: numerische Reihenfolge beachten!
    static sal_uInt16 __READONLY_DATA aInva[] =
        {
            //SID_NEWWINDOW,/*5620*/
            SID_BROWSER_MODE, /*6313*/
            SID_RULER_BORDERS, SID_RULER_PAGE_POS,
            //SID_ATTR_LONG_LRSPACE,
            SID_HTML_MODE,
            SID_RULER_PROTECT,
            //SID_AUTOSPELL_CHECK,
            //SID_AUTOSPELL_MARKOFF,
            FN_RULER,       /*20211*/
            FN_VIEW_GRAPHIC,    /*20213*/
            FN_VIEW_BOUNDS,     /**/
            FN_VIEW_FIELDS,     /*20215*/
            FN_VLINEAL,             /*20216*/
            FN_VSCROLLBAR,      /*20217*/
            FN_HSCROLLBAR,      /*20218*/
            FN_VIEW_META_CHARS, /**/
            FN_VIEW_MARKS,      /**/
            //FN_VIEW_FIELDNAME,    /**/
            FN_VIEW_TABLEGRID,  /*20227*/
            FN_PRINT_LAYOUT, /*20237*/
            FN_QRY_MERGE,   /*20364*/
            FN_SHADOWCURSOR, /**/
            0
        };
    // the view must not exist!
    GetViewFrame()->GetBindings().Invalidate(aInva);
    CheckVisArea();

    SvxZoomType eType;
    if( GetWrtShell().GetViewOptions()->getBrowseMode() && SVX_ZOOM_PERCENT != (eType = (SvxZoomType)
        GetWrtShell().GetViewOptions()->GetZoomType()) )
        SetZoom( eType );
    InvalidateBorder();
}

/*--------------------------------------------------------------------
    State of view options
 --------------------------------------------------------------------*/
void SwView::StateViewOptions(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    SfxBoolItem aBool;
    const SwViewOption* pOpt = GetWrtShell().GetViewOptions();
    const IDocumentSettingAccess* pIDSA = GetDocShell()->getIDocumentSettingAccess();

    while(nWhich)
    {
        sal_Bool bReadonly = GetDocShell()->IsReadOnly();
        if ( bReadonly && nWhich != FN_VIEW_GRAPHIC )
        {
            rSet.DisableItem(nWhich);
            nWhich = 0;
        }
        switch(nWhich)
        {
            case FN_RULER:
            {
                if(!pOpt->IsViewHRuler(sal_True) && !pOpt->IsViewVRuler(sal_True))
                {
                    rSet.DisableItem(nWhich);
                    nWhich = 0;
                }
                else
                    aBool.SetValue( pOpt->IsViewAnyRuler());
            }
            break;
            case SID_BROWSER_MODE:
            case FN_PRINT_LAYOUT:
            {
                sal_Bool bState = pOpt->getBrowseMode();
                if(FN_PRINT_LAYOUT == nWhich)
                    bState = !bState;
                aBool.SetValue( bState );
            }
            break;
            case FN_VIEW_BOUNDS:
                aBool.SetValue( SwViewOption::IsDocBoundaries()); break;
            case FN_VIEW_GRAPHIC:
                aBool.SetValue( !pOpt->IsGraphic() ); break;
            case FN_VIEW_FIELDS:
                aBool.SetValue( SwViewOption::IsFieldShadings() ); break;
            case FN_VIEW_FIELDNAME:
                aBool.SetValue( pOpt->IsFldName() ); break;
            case FN_VIEW_MARKS:
                aBool.SetValue( lcl_IsViewMarks(*pOpt) ); break;
            case FN_VIEW_META_CHARS:
                aBool.SetValue( pOpt->IsViewMetaChars() ); break;
            case FN_VIEW_TABLEGRID:
                aBool.SetValue( SwViewOption::IsTableBoundaries() ); break;
            case FN_VIEW_NOTES:
            {
                aBool.SetValue( pOpt->IsPostIts());
                if (!GetPostItMgr()->HasNotes())
                {
                    aBool.SetWhich( nWhich );
                    rSet.Put( aBool );
                    rSet.DisableItem(nWhich);
                    nWhich = 0;
                }
                break;
            }
            case FN_VIEW_HIDDEN_PARA:
                aBool.SetValue( pOpt->IsShowHiddenPara()); break;
            case SID_GRID_VISIBLE:
                aBool.SetValue( pOpt->IsGridVisible() ); break;
            case SID_GRID_USE:
                aBool.SetValue( pOpt->IsSnap() ); break;
            case SID_HELPLINES_MOVE:
                aBool.SetValue( pOpt->IsCrossHair() ); break;
            case FN_VIEW_SMOOTH_SCROLL:
                aBool.SetValue( pOpt->IsSmoothScroll()); break;
            case FN_VLINEAL:
                aBool.SetValue( 0 != StatVLineal() ); break;
            case FN_HSCROLLBAR:
                if( pOpt->getBrowseMode() )
                {
                    rSet.DisableItem(nWhich);
                    nWhich = 0;
                }
                else
                    aBool.SetValue( IsHScrollbarVisible() ); break;
            case FN_VSCROLLBAR:
                aBool.SetValue( IsVScrollbarVisible() ); break;
            case SID_AUTOSPELL_CHECK:
                aBool.SetValue( pOpt->IsOnlineSpell() );
            break;
            case FN_SHADOWCURSOR:
                if (pIDSA == 0 || pOpt->getBrowseMode() )
                {
                    rSet.DisableItem( nWhich );
                    nWhich = 0;
                }
                else
                    aBool.SetValue( pOpt->IsShadowCursor() );
            break;

        }

        if( nWhich )
        {
            aBool.SetWhich( nWhich );
            rSet.Put( aBool );
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    execute view options
 --------------------------------------------------------------------*/
void SwView::ExecViewOptions(SfxRequest &rReq)
{
    SwViewOption* pOpt = new SwViewOption( *GetWrtShell().GetViewOptions() );
    sal_Bool bModified = GetWrtShell().IsModified();

    int eState = STATE_TOGGLE;
    sal_Bool bSet = sal_False;
    bool bBrowseModeChanged = false;

    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxPoolItem* pAttr=NULL;

    if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nSlot , sal_False, &pAttr ))
    {
        bSet = ((SfxBoolItem*)pAttr)->GetValue();
        eState = bSet ? STATE_ON : STATE_OFF;
    }

    sal_Bool bFlag = STATE_ON == eState;
    uno::Reference< beans::XPropertySet >  xLngProp( ::GetLinguPropertySet() );

    switch ( nSlot )
    {
        case FN_VIEW_GRAPHIC:

                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsGraphic();
                pOpt->SetGraphic( bFlag );
                break;

        case FN_VIEW_FIELDS:
                if( STATE_TOGGLE == eState )
                    bFlag = !SwViewOption::IsFieldShadings() ;
                SwViewOption::SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bFlag, sal_True );
                break;

        case FN_VIEW_BOUNDS:
                if( STATE_TOGGLE == eState )
                    bFlag = !SwViewOption::IsDocBoundaries();
                SwViewOption::SetAppearanceFlag(VIEWOPT_DOC_BOUNDARIES, bFlag, sal_True );
                break;

        case SID_GRID_VISIBLE:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsGridVisible();

                pOpt->SetGridVisible( bFlag );
                break;

        case SID_GRID_USE:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsSnap();

                pOpt->SetSnap( bFlag );
                break;

        case SID_HELPLINES_MOVE:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsCrossHair();

                pOpt->SetCrossHair( bFlag );
                break;

        case SID_BROWSER_MODE:
        case FN_PRINT_LAYOUT:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->getBrowseMode();
                else if( nSlot == FN_PRINT_LAYOUT )
                    bFlag = !bFlag;
                bBrowseModeChanged = bFlag != pOpt->getBrowseMode();
                // Disable "multiple layout"
                GetDocShell()->ToggleBrowserMode( bFlag, this );

                pOpt->setBrowseMode( bFlag );
                break;

    case FN_VIEW_NOTES:
                if ( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsPostIts();

                GetPostItMgr()->SetLayout();
                pOpt->SetPostIts( bFlag );
                   if (pOpt->IsPostIts())
            GetPostItMgr()->CheckMetaText();
         break;

        case FN_VIEW_HIDDEN_PARA:
                if ( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsShowHiddenPara();

                pOpt->SetShowHiddenPara( bFlag );
                break;

        case FN_VIEW_SMOOTH_SCROLL:

                if ( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsSmoothScroll();

                pOpt->SetSmoothScroll( bFlag );
                break;


        case FN_VLINEAL:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewVRuler();

                pOpt->SetViewVRuler( bFlag );
                break;

        case FN_VSCROLLBAR:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewVScrollBar();

                pOpt->SetViewVScrollBar( bFlag );
                break;

        case FN_HSCROLLBAR:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewHScrollBar();

                pOpt->SetViewHScrollBar( bFlag );
                break;
        case FN_RULER:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewAnyRuler();

                pOpt->SetViewAnyRuler( bFlag );
                break;

        case FN_VIEW_TABLEGRID:
                if( STATE_TOGGLE == eState )
                    bFlag = !SwViewOption::IsTableBoundaries();
                SwViewOption::SetAppearanceFlag(VIEWOPT_TABLE_BOUNDARIES, bFlag, sal_True );
                break;

        case FN_VIEW_FIELDNAME:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsFldName() ;

                pOpt->SetFldName( bFlag );
                break;
        case FN_VIEW_MARKS:
                if( STATE_TOGGLE == eState )
                    bFlag = !lcl_IsViewMarks(*pOpt) ;

                lcl_SetViewMarks( *pOpt, bFlag );
                break;
        case FN_VIEW_META_CHARS:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewMetaChars();

                lcl_SetViewMetaChars( *pOpt, bFlag );
        break;
        case SID_AUTOSPELL_CHECK:
            if( STATE_TOGGLE == eState )
                bFlag = bSet = !pOpt->IsOnlineSpell();

            pOpt->SetOnlineSpell(bSet);
            {
                uno::Any aVal( &bSet, ::getCppuBooleanType() );
                String aPropName( C2S(UPN_IS_SPELL_AUTO) );

                SvtLinguConfig  aCfg;
                aCfg.SetProperty( aPropName, aVal );

                if (xLngProp.is())
                    xLngProp->setPropertyValue( aPropName, aVal );

                // for the time being we do not have a specific option for grammarchecking.
                // thus we'll use the one for spell checking...
                if (bSet)
                {
                    SwDocShell *pDocSh = GetDocShell();
                    SwDoc *pDoc = pDocSh? pDocSh->GetDoc() : NULL;

                    // right now we don't have view options for automatic grammar checking. Thus...
                    sal_Bool bIsAutoGrammar = sal_False;
                    aCfg.GetProperty( C2U( UPN_IS_GRAMMAR_AUTO ) ) >>= bIsAutoGrammar;

                    if (pDoc && bIsAutoGrammar)
                        StartGrammarChecking( *pDoc );
                }
            }
        break;
        case FN_SHADOWCURSOR:
            if( STATE_TOGGLE == eState )
                bFlag = bSet = !pOpt->IsShadowCursor();

            pOpt->SetShadowCursor(bSet);
        break;

        default:
            ASSERT(sal_False, Falsche Request-Methode);
            return;
    }

    // UserPrefs setzen Request als bearbeitet kennzeichnen
    sal_Bool bWebView =  0 != dynamic_cast<const SwWebView*>(this);
    SwWrtShell &rSh = GetWrtShell();
    rSh.StartAction();
    SwModule* pModule = SW_MOD();
    if( !(*rSh.GetViewOptions() == *pOpt ))
    {
        rSh.ApplyViewOptions( *pOpt );
        if( bBrowseModeChanged )
        {
            RecheckBrowseMode();
            CheckVisArea();
        }

        //Die UsrPref muessen als Modified gekennzeichnet werden.
        //call for initialization
        pModule->GetUsrPref(bWebView);
        pModule->CheckSpellChanges( pOpt->IsOnlineSpell(), sal_False, sal_False, sal_False );
    }
    //OS:   Modified wieder zuruecksetzen, weil Ansicht/Felder
    //      das Doc modified setzt.
    if( !bModified )
        rSh.ResetModified();

    pModule->ApplyUsrPref( *pOpt, this, bWebView ? VIEWOPT_DEST_WEB : VIEWOPT_DEST_TEXT );

    //mod #i6193# let postits know about new spellcheck setting
    if ( nSlot == SID_AUTOSPELL_CHECK )
        GetPostItMgr()->SetSpellChecking();

    const sal_Bool bLockedView = rSh.IsViewLocked();
    rSh.LockView( sal_True );    //lock visible section
    GetWrtShell().EndAction();
    if( bBrowseModeChanged && !bFlag )
        CalcVisArea( GetEditWin().GetOutputSizePixel() );
    rSh.LockView( bLockedView );

    delete pOpt;
    Invalidate(rReq.GetSlot());
    if(!pArgs)
        rReq.AppendItem(SfxBoolItem(nSlot, (sal_Bool)bFlag));
    rReq.Done();
}

IMPL_LINK( SwView, HtmlOptionsHdl, void*, EMPTYARG )
{
    // Invalidierung, falls blinkender Text erlaubt/verboten wurde
    GetViewFrame()->GetBindings().Invalidate(SID_DRAW_TEXT_MARQUEE);
    return 0;
}
