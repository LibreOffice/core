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


#include <boost/scoped_ptr.hpp>

#include <hintids.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/childwin.hxx>
#include <unotools/useroptions.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <svx/dataaccessdescriptor.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/msgbox.hxx>
#include <svl/cjkoptions.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <usrpref.hxx>
#include <modcfg.hxx>
#include <view.hxx>
#include <pview.hxx>
#include <wview.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <dbmgr.hxx>
#include <uinums.hxx>
#include <prtopt.hxx>   // for PrintOptions
#include <navicfg.hxx>
#include <doc.hxx>
#include <cmdid.h>
#include <app.hrc>
#include "helpid.h"

#include <unomid.h>
#include <tools/color.hxx>
#include "PostItMgr.hxx"

using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::lang;


static void lcl_SetUIPrefs(const SwViewOption &rPref, SwView* pView, ViewShell* pSh )
{
    // in FrameSets the actual visibility can differ from the ViewOption's setting
    bool bVScrollChanged = rPref.IsViewVScrollBar() != pSh->GetViewOptions()->IsViewVScrollBar();
    bool bHScrollChanged = rPref.IsViewHScrollBar() != pSh->GetViewOptions()->IsViewHScrollBar();
    bool bVAlignChanged = rPref.IsVRulerRight() != pSh->GetViewOptions()->IsVRulerRight();

    pSh->SetUIOptions(rPref);
    const SwViewOption* pNewPref = pSh->GetViewOptions();

    // Scrollbars on / off
    if(bVScrollChanged)
    {
        pView->EnableVScrollbar(pNewPref->IsViewVScrollBar());
    }
    if(bHScrollChanged)
    {
        pView->EnableHScrollbar( pNewPref->IsViewHScrollBar() || pNewPref->getBrowseMode() );
    }
    //if only the position of the vertical ruler has been changed initiate an update
    if(bVAlignChanged && !bHScrollChanged && !bVScrollChanged)
        pView->InvalidateBorder();

    // Rulers on / off
    if(pNewPref->IsViewVRuler())
        pView->CreateVRuler();
    else
        pView->KillVRuler();

    // TabWindow on / off
    if(pNewPref->IsViewHRuler())
        pView->CreateTab();
    else
        pView->KillTab();

    pView->GetPostItMgr()->PrepareView(true);
}

SwWrtShell* GetActiveWrtShell()
{
    SwView *pActive = ::GetActiveView();
    if( pActive )
        return &pActive->GetWrtShell();
    return 0;
}

SwView* GetActiveView()
{
    SfxViewShell* pView = SfxViewShell::Current();
    return PTR_CAST( SwView, pView );
}

SwView* SwModule::GetFirstView()
{
    // returns only sivible SwView
    const TypeId aTypeId = TYPE(SwView);
    SwView* pView = (SwView*)SfxViewShell::GetFirst(&aTypeId);
    return pView;
}

SwView* SwModule::GetNextView(SwView* pView)
{
    OSL_ENSURE(PTR_CAST(SwView, pView),"return no SwView");
    const TypeId aTypeId = TYPE(SwView);
    SwView* pNView = (SwView*)SfxViewShell::GetNext(*pView, &aTypeId, sal_True);
    return pNView;
}

// New Master for the settings is set; this affects the current view and all following.
void SwModule::ApplyUsrPref(const SwViewOption &rUsrPref, SwView* pActView,
                            sal_uInt16 nDest )
{
    SwView* pCurrView = pActView;
    ViewShell* pSh = pCurrView ? &pCurrView->GetWrtShell() : 0;

    SwMasterUsrPref* pPref = (SwMasterUsrPref*)GetUsrPref( static_cast< sal_Bool >(
                                         VIEWOPT_DEST_WEB == nDest ? sal_True  :
                                         VIEWOPT_DEST_TEXT== nDest ? sal_False :
                                         pCurrView && pCurrView->ISA(SwWebView) ));

    // with Uno, only sdbcx::View, but not the Module should be changed
    bool bViewOnly = VIEWOPT_DEST_VIEW_ONLY == nDest;
    // fob PreView off
    SwPagePreView* pPPView;
    if( !pCurrView && 0 != (pPPView = PTR_CAST( SwPagePreView, SfxViewShell::Current())) )
    {
        if(!bViewOnly)
            pPref->SetUIOptions( rUsrPref );
        pPPView->EnableVScrollbar(pPref->IsViewVScrollBar());
        pPPView->EnableHScrollbar(pPref->IsViewHScrollBar());
        if(!bViewOnly)
        {
            pPref->SetPagePrevRow(rUsrPref.GetPagePrevRow());
            pPref->SetPagePrevCol(rUsrPref.GetPagePrevCol());
        }
        return;
    }

    if(!bViewOnly)
    {
        pPref->SetUsrPref( rUsrPref );
        pPref->SetModified();
    }

    if( !pCurrView )
        return;

    // Passing on to CORE
    sal_Bool bReadonly;
    const SwDocShell* pDocSh = pCurrView->GetDocShell();
    if (pDocSh)
        bReadonly = pDocSh->IsReadOnly();
    else //Use existing option if DocShell missing
        bReadonly = pSh->GetViewOptions()->IsReadonly();
    boost::scoped_ptr<SwViewOption> xViewOpt;
    if (!bViewOnly)
        xViewOpt.reset(new SwViewOption(*pPref));
    else
        xViewOpt.reset(new SwViewOption(rUsrPref));
    xViewOpt->SetReadonly( bReadonly );
    if( !(*pSh->GetViewOptions() == *xViewOpt) )
    {
        //is maybe only a ViewShell
        pSh->StartAction();
        pSh->ApplyViewOptions( *xViewOpt );
        ((SwWrtShell*)pSh)->SetReadOnlyAvailable(xViewOpt->IsCursorInProtectedArea());
        pSh->EndAction();
    }
    if ( pSh->GetViewOptions()->IsReadonly() != bReadonly )
        pSh->SetReadonlyOption(bReadonly);

    lcl_SetUIPrefs(*xViewOpt, pCurrView, pSh);

    // in the end the Idle-Flag is set again
    pPref->SetIdle(sal_True);
}

void SwModule::ApplyUserMetric( FieldUnit eMetric, sal_Bool bWeb )
{
        SwMasterUsrPref* pPref;
        if(bWeb)
        {
            if(!pWebUsrPref)
                GetUsrPref(sal_True);
            pPref = pWebUsrPref;
        }
        else
        {
            if(!pUsrPref)
                GetUsrPref(sal_False);
            pPref = pUsrPref;
        }
        FieldUnit eOldMetric = pPref->GetMetric();
        if(eOldMetric != eMetric)
            pPref->SetMetric(eMetric);

        FieldUnit eHScrollMetric = pPref->IsHScrollMetric() ? pPref->GetHScrollMetric() : eMetric;
        FieldUnit eVScrollMetric = pPref->IsVScrollMetric() ? pPref->GetVScrollMetric() : eMetric;

        SwView* pTmpView = SwModule::GetFirstView();
        // switch the ruler for all MDI-Windows
        while(pTmpView)
        {
            if(bWeb == (0 != PTR_CAST(SwWebView, pTmpView)))
            {
                pTmpView->ChangeVRulerMetric(eVScrollMetric);
                pTmpView->ChangeTabMetric(eHScrollMetric);
            }

            pTmpView = SwModule::GetNextView(pTmpView);
        }
}

void SwModule::ApplyRulerMetric( FieldUnit eMetric, sal_Bool bHorizontal, sal_Bool bWeb )
{
    SwMasterUsrPref* pPref;
    if(bWeb)
    {
        if(!pWebUsrPref)
            GetUsrPref(sal_True);
        pPref = pWebUsrPref;
    }
    else
    {
        if(!pUsrPref)
            GetUsrPref(sal_False);
        pPref = pUsrPref;
    }
    if( bHorizontal )
        pPref->SetHScrollMetric(eMetric);
    else
        pPref->SetVScrollMetric(eMetric);

    SwView* pTmpView = SwModule::GetFirstView();
    // switch metric at the appropriate rulers
    while(pTmpView)
    {
        if(bWeb == (0 != dynamic_cast<SwWebView *>( pTmpView )))
        {
            if( bHorizontal )
                pTmpView->ChangeTabMetric(eMetric);
            else
                pTmpView->ChangeVRulerMetric(eMetric);
        }
        pTmpView = SwModule::GetNextView(pTmpView);
    }
}

//set the usrpref 's char unit attribute and set rulers unit as char if the "apply char unit" is checked
void SwModule::ApplyUserCharUnit(sal_Bool bApplyChar, sal_Bool bWeb)
{
    SwMasterUsrPref* pPref;
    if(bWeb)
    {
        if(!pWebUsrPref)
            GetUsrPref(sal_True);
        pPref = pWebUsrPref;
    }
    else
    {
        if(!pUsrPref)
            GetUsrPref(sal_False);
        pPref = pUsrPref;
    }
    sal_Bool bOldApplyCharUnit = pPref->IsApplyCharUnit();
    bool bHasChanged = false;
    if(bOldApplyCharUnit != bApplyChar)
    {
        pPref->SetApplyCharUnit(bApplyChar);
        bHasChanged = true;
    }

    if( !bHasChanged )
        return;

    FieldUnit eHScrollMetric = pPref->IsHScrollMetric() ? pPref->GetHScrollMetric() : pPref->GetMetric();
    FieldUnit eVScrollMetric = pPref->IsVScrollMetric() ? pPref->GetVScrollMetric() : pPref->GetMetric();
    if(bApplyChar)
    {
        eHScrollMetric = FUNIT_CHAR;
        eVScrollMetric = FUNIT_LINE;
    }
    else
    {
        SvtCJKOptions aCJKOptions;
        if ( !aCJKOptions.IsAsianTypographyEnabled() && ( eHScrollMetric == FUNIT_CHAR ))
            eHScrollMetric = FUNIT_INCH;
        else if ( eHScrollMetric == FUNIT_CHAR )
            eHScrollMetric = FUNIT_CM;
        if ( !aCJKOptions.IsAsianTypographyEnabled() && ( eVScrollMetric == FUNIT_LINE ))
            eVScrollMetric = FUNIT_INCH;
        else if ( eVScrollMetric == FUNIT_LINE )
            eVScrollMetric = FUNIT_CM;
    }
    SwView* pTmpView = SwModule::GetFirstView();
    // switch rulers for all MDI-Windows
    while(pTmpView)
    {
        if(bWeb == (0 != PTR_CAST(SwWebView, pTmpView)))
        {
            pTmpView->ChangeVRulerMetric(eVScrollMetric);
            pTmpView->ChangeTabMetric(eHScrollMetric);
        }

        pTmpView = SwModule::GetNextView(pTmpView);
    }
}

SwNavigationConfig*  SwModule::GetNavigationConfig()
{
    if(!pNavigationConfig)
    {
        pNavigationConfig = new SwNavigationConfig;
    }
    return pNavigationConfig;
}

SwPrintOptions*     SwModule::GetPrtOptions(sal_Bool bWeb)
{
    if(bWeb && !pWebPrtOpt)
    {
        pWebPrtOpt = new SwPrintOptions(sal_True);
    }
    else if(!bWeb && !pPrtOpt)
    {
        pPrtOpt = new SwPrintOptions(sal_False);
    }

    return bWeb ? pWebPrtOpt : pPrtOpt;
}

SwChapterNumRules*  SwModule::GetChapterNumRules()
{
    if(!pChapterNumRules)
        pChapterNumRules = new SwChapterNumRules;
    return pChapterNumRules;
}

void SwModule::ShowDBObj(SwView& rView, const SwDBData& rData, sal_Bool /*bOnlyIfAvailable*/)
{
    Reference<XFrame> xFrame = rView.GetViewFrame()->GetFrame().GetFrameInterface();
    Reference<XDispatchProvider> xDP(xFrame, uno::UNO_QUERY);

    uno::Reference<XFrame> xBeamerFrame = xFrame->findFrame("_beamer", FrameSearchFlag::CHILDREN);
    if (xBeamerFrame.is())
    {   // the beamer has been opened by the SfxViewFrame
        Reference<XController> xController = xBeamerFrame->getController();
        Reference<XSelectionSupplier> xControllerSelection(xController, UNO_QUERY);
        if (xControllerSelection.is())
        {

            ODataAccessDescriptor aSelection;
            aSelection.setDataSource(rData.sDataSource);
            aSelection[daCommand]       <<= rData.sCommand;
            aSelection[daCommandType]   <<= rData.nCommandType;
            xControllerSelection->select(makeAny(aSelection.createPropertyValueSequence()));
        }
        else {
            OSL_FAIL("no selection supplier in the beamer!");
        }
    }
}

sal_uInt16 SwModule::GetRedlineAuthor()
{
    if (!bAuthorInitialised)
    {
        const SvtUserOptions& rOpt = GetUserOptions();
        if( !(sActAuthor = rOpt.GetFullName()).Len() )
            if( !(sActAuthor = rOpt.GetID()).Len() )
                sActAuthor = String( SW_RES( STR_REDLINE_UNKNOWN_AUTHOR ));
        bAuthorInitialised = sal_True;
    }
    return InsertRedlineAuthor( sActAuthor );
}

void SwModule::SetRedlineAuthor(const String &rAuthor)
{
    bAuthorInitialised = sal_True;
    sActAuthor = rAuthor;
    InsertRedlineAuthor( sActAuthor );
}

const String& SwModule::GetRedlineAuthor(sal_uInt16 nPos)
{
    OSL_ENSURE(nPos < pAuthorNames->size(), "author not found!"); //#i45342# RTF doc with no author table caused reader to crash
    while(!(nPos < pAuthorNames->size()))
    {
        InsertRedlineAuthor(String("nn"));
    };
    return (*pAuthorNames)[nPos];
}

sal_uInt16 SwModule::InsertRedlineAuthor(const String& rAuthor)
{
    sal_uInt16 nPos = 0;

    while(nPos < pAuthorNames->size() && (*pAuthorNames)[nPos] != rAuthor)
        ++nPos;

    if (nPos == pAuthorNames->size())
        pAuthorNames->push_back(rAuthor);

    return nPos;
}

static void lcl_FillAuthorAttr( sal_uInt16 nAuthor, SfxItemSet &rSet,
                        const AuthorCharAttr &rAttr )
{
    Color aCol( rAttr.nColor );

    if( COL_TRANSPARENT == rAttr.nColor )
    {
        static const ColorData aColArr[] = {
         COL_AUTHOR1_DARK,      COL_AUTHOR2_DARK,   COL_AUTHOR3_DARK,
         COL_AUTHOR4_DARK,      COL_AUTHOR5_DARK,   COL_AUTHOR6_DARK,
         COL_AUTHOR7_DARK,      COL_AUTHOR8_DARK,   COL_AUTHOR9_DARK };

        aCol.SetColor( aColArr[ nAuthor % (sizeof( aColArr ) /
                                           sizeof( aColArr[0] )) ] );
    }

    bool bBackGr = COL_NONE == rAttr.nColor;

    switch (rAttr.nItemId)
    {
    case SID_ATTR_CHAR_WEIGHT:
        {
            SvxWeightItem aW( (FontWeight)rAttr.nAttr, RES_CHRATR_WEIGHT );
            rSet.Put( aW );
            aW.SetWhich( RES_CHRATR_CJK_WEIGHT );
            rSet.Put( aW );
            aW.SetWhich( RES_CHRATR_CTL_WEIGHT );
            rSet.Put( aW );
        }
        break;

    case SID_ATTR_CHAR_POSTURE:
        {
            SvxPostureItem aP( (FontItalic)rAttr.nAttr, RES_CHRATR_POSTURE );
            rSet.Put( aP );
            aP.SetWhich( RES_CHRATR_CJK_POSTURE );
            rSet.Put( aP );
            aP.SetWhich( RES_CHRATR_CTL_POSTURE );
            rSet.Put( aP );
        }
        break;

    case SID_ATTR_CHAR_UNDERLINE:
        rSet.Put( SvxUnderlineItem( (FontUnderline)rAttr.nAttr,
                                    RES_CHRATR_UNDERLINE));
        break;

    case SID_ATTR_CHAR_STRIKEOUT:
        rSet.Put(SvxCrossedOutItem( (FontStrikeout)rAttr.nAttr,
                                    RES_CHRATR_CROSSEDOUT));
        break;

    case SID_ATTR_CHAR_CASEMAP:
        rSet.Put( SvxCaseMapItem( (SvxCaseMap)rAttr.nAttr,
                                    RES_CHRATR_CASEMAP));
        break;

    case SID_ATTR_BRUSH:
        rSet.Put( SvxBrushItem( aCol, RES_CHRATR_BACKGROUND ));
        bBackGr = true;
        break;
    }

    if( !bBackGr )
        rSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
}

void SwModule::GetInsertAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet)
{
    lcl_FillAuthorAttr(nAuthor, rSet, pModuleConfig->GetInsertAuthorAttr());
}

void SwModule::GetDeletedAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet)
{
    lcl_FillAuthorAttr(nAuthor, rSet, pModuleConfig->GetDeletedAuthorAttr());
}

// For future extension:
void SwModule::GetFormatAuthorAttr( sal_uInt16 nAuthor, SfxItemSet &rSet )
{
    lcl_FillAuthorAttr( nAuthor, rSet, pModuleConfig->GetFormatAuthorAttr() );
}

sal_uInt16 SwModule::GetRedlineMarkPos()
{
    return pModuleConfig->GetMarkAlignMode();
}

sal_Bool SwModule::IsInsTblFormatNum(sal_Bool bHTML) const
{
    return pModuleConfig->IsInsTblFormatNum(bHTML);
}

sal_Bool SwModule::IsInsTblChangeNumFormat(sal_Bool bHTML) const
{
    return pModuleConfig->IsInsTblChangeNumFormat(bHTML);
}

sal_Bool SwModule::IsInsTblAlignNum(sal_Bool bHTML) const
{
    return pModuleConfig->IsInsTblAlignNum(bHTML);
}

const Color &SwModule::GetRedlineMarkColor()
{
    return pModuleConfig->GetMarkAlignColor();
}

const SwViewOption* SwModule::GetViewOption(sal_Bool bWeb)
{
    return GetUsrPref( bWeb );
}

OUString SwModule::GetDocStatWordDelim() const
{
    return pModuleConfig->GetWordDelimiter();
}

// Passing-through of the ModuleConfig's Metric (for HTML-Export)
sal_uInt16 SwModule::GetMetric( sal_Bool bWeb ) const
{
    SwMasterUsrPref* pPref;
    if(bWeb)
    {
        if(!pWebUsrPref)
            GetUsrPref(sal_True);
        pPref = pWebUsrPref;
    }
    else
    {
        if(!pUsrPref)
            GetUsrPref(sal_False);
        pPref = pUsrPref;
    }
    return static_cast< sal_uInt16 >(pPref->GetMetric());
}

// Pass-through Update-Stati
sal_uInt16 SwModule::GetLinkUpdMode( sal_Bool ) const
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    return (sal_uInt16)pUsrPref->GetUpdateLinkMode();
}

SwFldUpdateFlags SwModule::GetFldUpdateFlags( sal_Bool ) const
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    return pUsrPref->GetFldUpdateFlags();
}

void SwModule::ApplyFldUpdateFlags(SwFldUpdateFlags eFldFlags)
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    pUsrPref->SetFldUpdateFlags(eFldFlags);
}

void SwModule::ApplyLinkMode(sal_Int32 nNewLinkMode)
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    pUsrPref->SetUpdateLinkMode(nNewLinkMode);
}

void SwModule::CheckSpellChanges( bool bOnlineSpelling,
        bool bIsSpellWrongAgain, bool bIsSpellAllAgain, bool bSmartTags )
{
    bool bOnlyWrong = bIsSpellWrongAgain && !bIsSpellAllAgain;
    bool bInvalid = bOnlyWrong || bIsSpellAllAgain;
    if( bOnlineSpelling || bInvalid )
    {
        TypeId aType = TYPE(SwDocShell);
        for( SwDocShell *pDocSh = (SwDocShell*)SfxObjectShell::GetFirst(&aType);
             pDocSh;
             pDocSh = (SwDocShell*)SfxObjectShell::GetNext( *pDocSh, &aType ) )
        {
            SwDoc* pTmp = pDocSh->GetDoc();
            if ( pTmp->GetCurrentViewShell() )  //swmod 071108//swmod 071225
            {
                pTmp->SpellItAgainSam( bInvalid, bOnlyWrong, bSmartTags );
                ViewShell* pViewShell = 0;
                pTmp->GetEditShell( &pViewShell );
                if ( bSmartTags && pViewShell && pViewShell->GetWin() )
                    pViewShell->GetWin()->Invalidate();
            }
        }
    }
}

void SwModule::ApplyDefaultPageMode(sal_Bool bIsSquaredPageMode)
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    pUsrPref->SetDefaultPageMode(bIsSquaredPageMode);
}

SvxCompareMode SwModule::GetCompareMode() const
{
    return pModuleConfig->GetCompareMode();
}

sal_Bool SwModule::IsUseRsid() const
{
    return pModuleConfig->IsUseRsid();
}

sal_Bool SwModule::IsIgnorePieces() const
{
    return pModuleConfig->IsIgnorePieces();
}

sal_uInt16 SwModule::GetPieceLen() const
{
    return pModuleConfig->GetPieceLen();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
