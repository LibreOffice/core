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
#include <editeng/crsditem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brshitem.hxx>
#include <vcl/msgbox.hxx>
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
#include <prtopt.hxx>       // fuer PrintOptions
#include <navicfg.hxx>
#include <doc.hxx>
#include <cmdid.h>
#include <app.hrc>
#include "helpid.h"

#include <unomid.h>
#include <tools/color.hxx>
#include "PostItMgr.hxx"

using ::rtl::OUString;
using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::lang;


/*-----------------08/28/97 08:41pm-----------------

--------------------------------------------------*/
void lcl_SetUIPrefs(const SwViewOption* pPref, SwView* pView, ViewShell* pSh )
{
    // in FrameSets kann die tatsaechliche Sichtbarkeit von der Einstellung der ViewOptions abweichen
    sal_Bool bVScrollChanged = pPref->IsViewVScrollBar() != pSh->GetViewOptions()->IsViewVScrollBar();
    sal_Bool bHScrollChanged = pPref->IsViewHScrollBar() != pSh->GetViewOptions()->IsViewHScrollBar();
    sal_Bool bVAlignChanged = pPref->IsVRulerRight() != pSh->GetViewOptions()->IsVRulerRight();

    pSh->SetUIOptions(*pPref);
    const SwViewOption* pNewPref = pSh->GetViewOptions();

    // Scrollbars an / aus
    if(bVScrollChanged)
    {
        pView->ShowVScrollbar(pNewPref->IsViewVScrollBar());
    }
    if(bHScrollChanged)
    {
        pView->ShowHScrollbar( pNewPref->IsViewHScrollBar() || pNewPref->getBrowseMode() );
    }
    //if only the position of the vertical ruler has been changed initiate an update
    if(bVAlignChanged && !bHScrollChanged && !bVScrollChanged)
        pView->InvalidateBorder();

    // Lineale an / aus
    if(pNewPref->IsViewVRuler())
        pView->CreateVLineal();
    else
        pView->KillVLineal();

    // TabWindow an/aus
    if(pNewPref->IsViewHRuler())
        pView->CreateTab();
    else
        pView->KillTab();

    pView->GetPostItMgr()->PrepareView(true);
}

/*--------------------------------------------------------------------
    Beschreibung:   Aktuelle SwWrtShell
 --------------------------------------------------------------------*/


SwWrtShell* GetActiveWrtShell()
{
    SwView *pActive = ::GetActiveView();
    if( pActive )
        return &pActive->GetWrtShell();
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Pointer auf die aktuelle Sicht
 --------------------------------------------------------------------*/


SwView* GetActiveView()
{
    SfxViewShell* pView = SfxViewShell::Current();
    return PTR_CAST( SwView, pView );
}
/*--------------------------------------------------------------------
    Beschreibung:   Ueber Views iterieren - static
 --------------------------------------------------------------------*/

SwView* SwModule::GetFirstView()
{
    // liefert nur sichtbare SwViews
    const TypeId aTypeId = TYPE(SwView);
    SwView* pView = (SwView*)SfxViewShell::GetFirst(&aTypeId);
    return pView;
}


SwView* SwModule::GetNextView(SwView* pView)
{
    DBG_ASSERT(PTR_CAST(SwView, pView),"keine SwView uebergeben");
    const TypeId aTypeId = TYPE(SwView);
    SwView* pNView = (SwView*)SfxViewShell::GetNext(*pView, &aTypeId, sal_True);
    return pNView;
}

/*------------------------------------------------------------------------
 Beschreibung:  Neuer Master fuer die Einstellungen wird gesetzt;
                dieser wirkt sich auf die aktuelle Sicht und alle
                folgenden aus.
------------------------------------------------------------------------*/

void SwModule::ApplyUsrPref(const SwViewOption &rUsrPref, SwView* pActView,
                            sal_uInt16 nDest )
{
    SwView* pCurrView = pActView;
    ViewShell* pSh = pCurrView ? &pCurrView->GetWrtShell() : 0;

    SwMasterUsrPref* pPref = (SwMasterUsrPref*)GetUsrPref( static_cast< sal_Bool >(
                                         VIEWOPT_DEST_WEB == nDest ? sal_True  :
                                         VIEWOPT_DEST_TEXT== nDest ? sal_False :
                                         pCurrView && pCurrView->ISA(SwWebView) ));

    //per Uno soll nur die sdbcx::View, aber nicht das Module veraendert werden
    sal_Bool bViewOnly = VIEWOPT_DEST_VIEW_ONLY == nDest;
    //PreView abfruehstuecken
    SwPagePreView* pPPView;
    if( !pCurrView && 0 != (pPPView = PTR_CAST( SwPagePreView, SfxViewShell::Current())) )
    {
        if(!bViewOnly)
            pPref->SetUIOptions( rUsrPref );
        pPPView->ShowVScrollbar(pPref->IsViewVScrollBar());
        pPPView->ShowHScrollbar(pPref->IsViewHScrollBar());
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

    // Weitergabe an die CORE
    sal_Bool bReadonly;
    const SwDocShell* pDocSh = pCurrView->GetDocShell();
    if (pDocSh)
        bReadonly = pDocSh->IsReadOnly();
    else //Use existing option if DocShell missing
        bReadonly = pSh->GetViewOptions()->IsReadonly();
    SwViewOption* pViewOpt;
    if(!bViewOnly)
        pViewOpt = new SwViewOption( *pPref );
    else
        pViewOpt = new SwViewOption( rUsrPref );
    pViewOpt->SetReadonly( bReadonly );
    if( !(*pSh->GetViewOptions() == *pViewOpt) )
    {
        //Ist evtl. nur eine ViewShell
        pSh->StartAction();
        pSh->ApplyViewOptions( *pViewOpt );
        ((SwWrtShell*)pSh)->SetReadOnlyAvailable(pViewOpt->IsCursorInProtectedArea());
        pSh->EndAction();
    }
    if ( pSh->GetViewOptions()->IsReadonly() != bReadonly )
        pSh->SetReadonlyOption(bReadonly);

    lcl_SetUIPrefs(pViewOpt, pCurrView, pSh);

    // zum Schluss wird das Idle-Flag wieder gesetzt
    // #42510#
    pPref->SetIdle(sal_True);
}
/* -----------------------------28.09.00 12:36--------------------------------

 ---------------------------------------------------------------------------*/
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
        // fuer alle MDI-Fenster das Lineal umschalten
        while(pTmpView)
        {
            if(bWeb == (0 != PTR_CAST(SwWebView, pTmpView)))
            {
                pTmpView->ChangeVLinealMetric(eVScrollMetric);
                pTmpView->ChangeTabMetric(eHScrollMetric);
            }

            pTmpView = SwModule::GetNextView(pTmpView);
        }
}
/*-- 12.11.2008 14:47:58---------------------------------------------------

  -----------------------------------------------------------------------*/
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
                pTmpView->ChangeVLinealMetric(eMetric);
        }
        pTmpView = SwModule::GetNextView(pTmpView);
    }
}
/*-----------------13.11.96 11.57-------------------

--------------------------------------------------*/

SwNavigationConfig*  SwModule::GetNavigationConfig()
{
    if(!pNavigationConfig)
    {
        pNavigationConfig = new SwNavigationConfig;
    }
    return pNavigationConfig;
}

/*-----------------05.02.97 08.03-------------------

--------------------------------------------------*/

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

/*-----------------26.06.97 07.52-------------------

--------------------------------------------------*/
SwChapterNumRules*  SwModule::GetChapterNumRules()
{
    if(!pChapterNumRules)
        pChapterNumRules = new SwChapterNumRules;
    return pChapterNumRules;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModule::ShowDBObj(SwView& rView, const SwDBData& rData, sal_Bool /*bOnlyIfAvailable*/)
{
    Reference<XFrame> xFrame = rView.GetViewFrame()->GetFrame().GetFrameInterface();
    Reference<XDispatchProvider> xDP(xFrame, uno::UNO_QUERY);

    uno::Reference<frame::XFrame> xBeamerFrame = xFrame->findFrame(
                                        rtl::OUString::createFromAscii("_beamer"),
                                        FrameSearchFlag::CHILDREN);
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
            DBG_ERROR("no selection supplier in the beamer!");
        }
    }
}
/*--------------------------------------------------------------------
    Beschreibung: Redlining
 --------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

const String& SwModule::GetRedlineAuthor(sal_uInt16 nPos)
{
    DBG_ASSERT(nPos<pAuthorNames->Count(), "author not found!"); //#i45342# RTF doc with no author table caused reader to crash
    while (!(nPos<pAuthorNames->Count()))
    {
        InsertRedlineAuthor(String(RTL_CONSTASCII_USTRINGPARAM("nn")));
    };
    return *pAuthorNames->GetObject(nPos);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_uInt16 SwModule::InsertRedlineAuthor(const String& rAuthor)
{
    sal_uInt16 nPos = 0;

    while (nPos < pAuthorNames->Count() && *pAuthorNames->GetObject(nPos) != rAuthor)
        nPos++;

    if (nPos == pAuthorNames->Count())
        pAuthorNames->Insert(new String(rAuthor), nPos);

    return nPos;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void lcl_FillAuthorAttr( sal_uInt16 nAuthor, SfxItemSet &rSet,
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

    sal_Bool bBackGr = COL_NONE == rAttr.nColor;

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
        bBackGr = sal_True;
        break;
    }

    if( !bBackGr )
        rSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModule::GetInsertAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet)
{
    lcl_FillAuthorAttr(nAuthor, rSet, pModuleConfig->GetInsertAuthorAttr());
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModule::GetDeletedAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet)
{
    lcl_FillAuthorAttr(nAuthor, rSet, pModuleConfig->GetDeletedAuthorAttr());
}

/*--------------------------------------------------------------------
    Beschreibung: Fuer zukuenftige Erweiterung:
 --------------------------------------------------------------------*/

void SwModule::GetFormatAuthorAttr( sal_uInt16 nAuthor, SfxItemSet &rSet )
{
    lcl_FillAuthorAttr( nAuthor, rSet, pModuleConfig->GetFormatAuthorAttr() );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_uInt16 SwModule::GetRedlineMarkPos()
{
    return pModuleConfig->GetMarkAlignMode();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool SwModule::IsInsTblFormatNum(sal_Bool bHTML) const
{
    return pModuleConfig->IsInsTblFormatNum(bHTML);
}

sal_Bool SwModule::IsInsTblChangeNumFormat(sal_Bool bHTML) const
{
    return pModuleConfig->IsInsTblChangeNumFormat(bHTML);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool SwModule::IsInsTblAlignNum(sal_Bool bHTML) const
{
    return pModuleConfig->IsInsTblAlignNum(bHTML);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

const Color &SwModule::GetRedlineMarkColor()
{
    return pModuleConfig->GetMarkAlignColor();
}

/*-----------------03.03.98 16:47-------------------

--------------------------------------------------*/
const SwViewOption* SwModule::GetViewOption(sal_Bool bWeb)
{
    return GetUsrPref( bWeb );
}

// returne den definierten DocStat - WordDelimiter
const String& SwModule::GetDocStatWordDelim() const
{
    return pModuleConfig->GetWordDelimiter();
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
// Durchreichen der Metric von der ModuleConfig (fuer HTML-Export)
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
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
// Update-Stati durchreichen
sal_uInt16 SwModule::GetLinkUpdMode( sal_Bool ) const
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    return (sal_uInt16)pUsrPref->GetUpdateLinkMode();
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwFldUpdateFlags SwModule::GetFldUpdateFlags( sal_Bool ) const
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    return pUsrPref->GetFldUpdateFlags();
}
/* -----------------------------28.09.00 14:18--------------------------------

 ---------------------------------------------------------------------------*/
void SwModule::ApplyFldUpdateFlags(SwFldUpdateFlags eFldFlags)
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    pUsrPref->SetFldUpdateFlags(eFldFlags);
}
/* -----------------------------28.09.00 14:18--------------------------------

 ---------------------------------------------------------------------------*/
void SwModule::ApplyLinkMode(sal_Int32 nNewLinkMode)
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    pUsrPref->SetUpdateLinkMode(nNewLinkMode);
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwModule::CheckSpellChanges( sal_Bool bOnlineSpelling,
        sal_Bool bIsSpellWrongAgain, sal_Bool bIsSpellAllAgain, sal_Bool bSmartTags )
{
    sal_Bool bOnlyWrong = bIsSpellWrongAgain && !bIsSpellAllAgain;
    sal_Bool bInvalid = bOnlyWrong || bIsSpellAllAgain;
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
//      pSpell->SetSpellWrongAgain( sal_False );
//      pSpell->SetSpellAllAgain( sal_False );
    }
}

void SwModule::ApplyDefaultPageMode(sal_Bool bIsSquaredPageMode)
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    pUsrPref->SetDefaultPageMode(bIsSquaredPageMode);
}
