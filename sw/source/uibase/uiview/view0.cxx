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

#include <config_features.h>

#include <hintids.hxx>
#include <vcl/graph.hxx>
#include <svl/srchitem.hxx>
#include <SwSpellDialogChildWindow.hxx>
#include <svl/eitem.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <viewopt.hxx>
#include <globals.h>
#include <sfx2/app.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <svx/srchdlg.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <uivwimp.hxx>
#include <avmedia/mediaplayer.hxx>
#include <swmodule.hxx>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>

#include <sfx2/objface.hxx>
#include <navipi.hxx>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <globals.hrc>
#include <cmdid.h>
#include <globdoc.hxx>
#include <wview.hxx>

#define ShellClass_SwView
#define ShellClass_Text
#define ShellClass_TextDrawText

#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <PostItMgr.hxx>

using namespace ::com::sun::star;

#include <unotools/moduleoptions.hxx>
#include <sfx2/viewfac.hxx>

#include <IDocumentSettingAccess.hxx>

#include <unomid.h>
#include <memory>
#include <swabstdlg.hxx>

SFX_IMPL_NAMED_VIEWFACTORY(SwView, "Default")
{
    if (utl::ConfigManager::IsFuzzing() || SvtModuleOptions().IsWriter())
    {
        SFX_VIEW_REGISTRATION(SwDocShell);
        SFX_VIEW_REGISTRATION(SwGlobalDocShell);
    }
}

SFX_IMPL_INTERFACE(SwView, SfxViewShell)

void SwView::InitInterface_Impl()
{
#if HAVE_FEATURE_DESKTOP
    GetStaticInterface()->RegisterChildWindow(SID_NAVIGATOR, true);

    GetStaticInterface()->RegisterChildWindow(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());

    GetStaticInterface()->RegisterChildWindow(SfxInfoBarContainerChild::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxSearchDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SwSpellDialogChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(FN_REDLINE_ACCEPT);
    GetStaticInterface()->RegisterChildWindow(SID_HYPERLINK_DIALOG);
    GetStaticInterface()->RegisterChildWindow(FN_WORDCOUNT_DIALOG);
#if HAVE_FEATURE_AVMEDIA
    GetStaticInterface()->RegisterChildWindow(::avmedia::MediaPlayer::GetChildWindowId());
#endif
    GetStaticInterface()->RegisterChildWindow(FN_INSERT_FIELD_DATA_ONLY);

    GetStaticInterface()->RegisterChildWindow(FN_SYNC_LABELS, false, SfxShellFeature::SwChildWindowLabel);

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_TOOLS, SfxVisibilityFlags::Standard|SfxVisibilityFlags::Server,
                                            ToolbarId::Tools_Toolbox);
#endif
}


ShellMode SwView::GetShellMode()
{
    return m_pViewImpl->GetShellMode();
}

view::XSelectionSupplier* SwView::GetUNOObject()
{
    return m_pViewImpl->GetUNOObject();
}

void SwView::ApplyAccessiblityOptions(SvtAccessibilityOptions const & rAccessibilityOptions)
{
    m_pWrtShell->ApplyAccessiblityOptions(rAccessibilityOptions);
    //to enable the right state of the selection cursor in readonly documents
    if(GetDocShell()->IsReadOnly())
        m_pWrtShell->ShowCursor();

}

void SwView::SetMailMergeConfigItem(std::shared_ptr<SwMailMergeConfigItem> const & rConfigItem)
{
    m_pViewImpl->SetMailMergeConfigItem(rConfigItem);
    UIFeatureChanged();
}

std::shared_ptr<SwMailMergeConfigItem> const & SwView::GetMailMergeConfigItem() const
{
    return m_pViewImpl->GetMailMergeConfigItem();
}

static bool lcl_IsViewMarks( const SwViewOption& rVOpt )
{
    return  rVOpt.IsHardBlank() &&
            rVOpt.IsSoftHyph() &&
            SwViewOption::IsFieldShadings();
}
static void lcl_SetViewMarks(SwViewOption& rVOpt, bool bOn )
{
    rVOpt.SetHardBlank(bOn);
    rVOpt.SetSoftHyph(bOn);
    SwViewOption::SetAppearanceFlag(
            ViewOptFlags::FieldShadings, bOn, true);
}

static void lcl_SetViewMetaChars( SwViewOption& rVOpt, bool bOn)
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
    // OS: pay attention to numerical order!
    static sal_uInt16 const aInva[] =
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
    if( GetWrtShell().GetViewOptions()->getBrowseMode() && SvxZoomType::PERCENT != (eType =
        GetWrtShell().GetViewOptions()->GetZoomType()) )
        SetZoom( eType );
    InvalidateBorder();
}

// State of view options

void SwView::StateViewOptions(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    SfxBoolItem aBool;
    const SwViewOption* pOpt = GetWrtShell().GetViewOptions();

    while(nWhich)
    {
        bool bReadonly = GetDocShell()->IsReadOnly();
        if ( bReadonly && nWhich != FN_VIEW_GRAPHIC )
        {
            rSet.DisableItem(nWhich);
            nWhich = 0;
        }
        switch(nWhich)
        {
            case FN_RULER:
            {
                if(!pOpt->IsViewHRuler(true) && !pOpt->IsViewVRuler(true))
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
                bool bState = pOpt->getBrowseMode();
                if(FN_PRINT_LAYOUT == nWhich)
                    bState = !bState;
                aBool.SetValue( bState );
            }
            break;
            case FN_VIEW_BOUNDS:
                aBool.SetValue( SwViewOption::IsDocBoundaries()); break;
            case FN_VIEW_GRAPHIC:
                aBool.SetValue( pOpt->IsGraphic() ); break;
            case FN_VIEW_FIELDS:
                aBool.SetValue( SwViewOption::IsFieldShadings() ); break;
            case FN_VIEW_FIELDNAME:
                aBool.SetValue( pOpt->IsFieldName() ); break;
            case FN_VIEW_MARKS:
                aBool.SetValue( lcl_IsViewMarks(*pOpt) ); break;
            case FN_VIEW_META_CHARS:
                aBool.SetValue( pOpt->IsViewMetaChars() ); break;
            case FN_VIEW_TABLEGRID:
                aBool.SetValue( SwViewOption::IsTableBoundaries() ); break;
            case SID_TOGGLE_NOTES:
            {
                if (!GetPostItMgr()->HasNotes())
                {
                    rSet.DisableItem(nWhich);
                    nWhich = 0;
                }
                else
                    aBool.SetValue( pOpt->IsPostIts());
                break;
            }
            case FN_VIEW_HIDDEN_PARA:
                aBool.SetValue( pOpt->IsShowHiddenPara()); break;
            case FN_VIEW_HIDE_WHITESPACE:
            {
                if (pOpt->getBrowseMode() || !pOpt->CanHideWhitespace())
                {
                    rSet.DisableItem(nWhich);
                    nWhich = 0;
                }
                else
                    aBool.SetValue(pOpt->IsHideWhitespaceMode());
                break;
            }
            case SID_GRID_VISIBLE:
                aBool.SetValue( pOpt->IsGridVisible() ); break;
            case SID_GRID_USE:
                aBool.SetValue( pOpt->IsSnap() ); break;
            case SID_HELPLINES_MOVE:
                aBool.SetValue( pOpt->IsCrossHair() ); break;
            case FN_VIEW_SMOOTH_SCROLL:
                aBool.SetValue( pOpt->IsSmoothScroll()); break;
            case FN_VLINEAL:
                aBool.SetValue( StatVRuler() ); break;
            case FN_HSCROLLBAR:
                if( pOpt->getBrowseMode() )
                {
                    rSet.DisableItem(nWhich);
                    nWhich = 0;
                }
                else
                    aBool.SetValue( IsHScrollbarVisible() );
                break;
            case FN_VSCROLLBAR:
                aBool.SetValue( IsVScrollbarVisible() ); break;
            case SID_AUTOSPELL_CHECK:
                aBool.SetValue( pOpt->IsOnlineSpell() );
            break;
            case FN_SHADOWCURSOR:
                if ( pOpt->getBrowseMode() )
                {
                    rSet.DisableItem( nWhich );
                    nWhich = 0;
                }
                else
                    aBool.SetValue( pOpt->IsShadowCursor() );
            break;
            case FN_SHOW_INLINETOOLTIPS:
              aBool.SetValue( pOpt->IsShowInlineTooltips() );
            break;
            case FN_USE_HEADERFOOTERMENU:
              aBool.SetValue( pOpt->IsUseHeaderFooterMenu() );
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

// execute view options

void SwView::ExecViewOptions(SfxRequest &rReq)
{
    std::unique_ptr<SwViewOption> pOpt(new SwViewOption( *GetWrtShell().GetViewOptions() ));
    bool bModified = GetWrtShell().IsModified();

    int eState = STATE_TOGGLE;
    bool bSet = false;
    bool bBrowseModeChanged = false;

    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxPoolItem* pAttr=nullptr;

    if( pArgs && SfxItemState::SET == pArgs->GetItemState( nSlot , false, &pAttr ))
    {
        bSet = static_cast<const SfxBoolItem*>(pAttr)->GetValue();
        eState = bSet ? STATE_ON : STATE_OFF;
    }

    bool bFlag = STATE_ON == eState;
    uno::Reference< linguistic2::XLinguProperties >  xLngProp( ::GetLinguPropertySet() );

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
        SwViewOption::SetAppearanceFlag(ViewOptFlags::FieldShadings, bFlag, true );
        break;

    case FN_VIEW_BOUNDS:
        if( STATE_TOGGLE == eState )
            bFlag = !SwViewOption::IsDocBoundaries();
        SwViewOption::SetAppearanceFlag(ViewOptFlags::DocBoundaries, bFlag, true );
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
        bBrowseModeChanged = !pOpt->getBrowseMode();
        pOpt->setBrowseMode(true );
        break;

    case FN_PRINT_LAYOUT:
        bBrowseModeChanged = pOpt->getBrowseMode();
        pOpt->setBrowseMode( false );
        break;

    case SID_TOGGLE_NOTES:
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

    case FN_VIEW_HIDE_WHITESPACE:
        if ( STATE_TOGGLE == eState )
            bFlag = !pOpt->IsHideWhitespaceMode();

        pOpt->SetHideWhitespaceMode(bFlag);
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
        SwViewOption::SetAppearanceFlag(ViewOptFlags::TableBoundaries, bFlag, true );
        break;

    case FN_VIEW_FIELDNAME:
        if( STATE_TOGGLE == eState )
            bFlag = !pOpt->IsFieldName() ;

        pOpt->SetFieldName( bFlag );
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
        {
            bFlag = !pOpt->IsOnlineSpell();
            bSet = bFlag;
        }

        pOpt->SetOnlineSpell(bSet);
        {
            SvtLinguConfig  aCfg;
            aCfg.SetProperty( UPN_IS_SPELL_AUTO, uno::makeAny( bSet ) );

            if (xLngProp.is())
                xLngProp->setIsSpellAuto( bSet );

            // for the time being we do not have a specific option for grammarchecking.
            // thus we'll use the one for spell checking...
            if (bSet)
            {
                SwDocShell *pDocSh = GetDocShell();
                SwDoc *pDoc = pDocSh? pDocSh->GetDoc() : nullptr;

                // right now we don't have view options for automatic grammar checking. Thus...
                bool bIsAutoGrammar = false;
                aCfg.GetProperty( UPN_IS_GRAMMAR_AUTO ) >>= bIsAutoGrammar;

                if (pDoc && bIsAutoGrammar)
                    pDoc->StartGrammarChecking();
            }
        }
        break;

    case FN_SHADOWCURSOR:
        if( STATE_TOGGLE == eState )
        {
            bFlag = !pOpt->IsShadowCursor();
            bSet = bFlag;
        }

        pOpt->SetShadowCursor(bSet);
        break;

    case FN_SHOW_INLINETOOLTIPS:
        if( STATE_TOGGLE == eState )
            bFlag = !pOpt->IsShowInlineTooltips();

        pOpt->SetShowInlineTooltips( bFlag );
        break;

    case FN_USE_HEADERFOOTERMENU:
        if( STATE_TOGGLE == eState )
            bFlag = !pOpt->IsUseHeaderFooterMenu();

        pOpt->SetUseHeaderFooterMenu( bFlag );
        break;

    default:
        OSL_FAIL("wrong request method");
        return;
    }

    // Set UserPrefs, mark request as modified
    bool bWebView =  dynamic_cast<const SwWebView*>(this) !=  nullptr;
    SwWrtShell &rSh = GetWrtShell();
    rSh.StartAction();
    SwModule* pModule = SW_MOD();
    if( *rSh.GetViewOptions() != *pOpt )
    {
        rSh.ApplyViewOptions( *pOpt );
        if( bBrowseModeChanged )
        {
            GetDocShell()->ToggleLayoutMode(this);
        }

        // The UsrPref must be marked as modified.
        // call for initialization
        pModule->GetUsrPref(bWebView);
        SwModule::CheckSpellChanges( pOpt->IsOnlineSpell(), false, false, false );
    }
    //OS: Set back modified again, because view/fields sets the Doc modified.
    if( !bModified )
        rSh.ResetModified();

    pModule->ApplyUsrPref( *pOpt, this, bWebView ? SvViewOpt::DestWeb : SvViewOpt::DestText );

    // #i6193# let postits know about new spellcheck setting
    if ( nSlot == SID_AUTOSPELL_CHECK )
        GetPostItMgr()->SetSpellChecking();

    const bool bLockedView = rSh.IsViewLocked();
    rSh.LockView( true );    //lock visible section
    GetWrtShell().EndAction();
    if( bBrowseModeChanged && !bFlag )
        CalcVisArea( GetEditWin().GetOutputSizePixel() );
    rSh.LockView( bLockedView );

    pOpt.reset();
    Invalidate(rReq.GetSlot());
    if(!pArgs)
        rReq.AppendItem(SfxBoolItem(nSlot, bFlag));
    rReq.Done();
}

void SwView::ExecFormatFootnote()
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSwFootNoteOptionDlg(GetFrameWeld(), GetWrtShell()));
    pDlg->Execute();
}

void SwView::ExecNumberingOutline(SfxItemPool & rPool)
{
    SfxItemSet aTmp(rPool, svl::Items<FN_PARAM_1, FN_PARAM_1>{});
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateOutlineTabDialog(GetFrameWeld(), &aTmp, GetWrtShell()));
    pDlg->Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
