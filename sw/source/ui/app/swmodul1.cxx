/*************************************************************************
 *
 *  $RCSfile: swmodul1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _SBASLTID_HRC //autogen
#include <offmgr/sbasltid.hrc>
#endif
#ifndef _SBA_SBAOBJ_HXX //autogen
#include <offmgr/sbaobj.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SDB_SDBSTAT_HXX
#include <sdb/sdbstat.hxx>
#endif

#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _UINUMS_HXX
#include <uinums.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>       // fuer PrintOptions
#endif
#ifndef _NAVICFG_HXX
#include <navicfg.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif
using namespace ::rtl;
using namespace ::com::sun::star;
/* -----------------------------05.01.00 15:14--------------------------------

 ---------------------------------------------------------------------------*/

class SwXDispatchStatusListener : public cppu::WeakImplHelper1
<
    frame::XStatusListener
>
{
    uno::Reference< frame::XStatusListener >            xSelf;
    util::URL                       aDispURL;
    uno::Reference< frame::XDispatch >              xDispatch;

    void            Invalidate();
public:
    SwXDispatchStatusListener(uno::Reference< frame::XDispatch > &  xDisp, const util::URL&     aURL);
    ~SwXDispatchStatusListener();

    //XStatusListener
    virtual void SAL_CALL statusChanged(const frame::FeatureStateEvent& Event) throw( uno::RuntimeException );

    //XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& Source) throw( uno::RuntimeException );
};
/* -----------------------------05.01.00 15:18--------------------------------

 ---------------------------------------------------------------------------*/
SwXDispatchStatusListener::SwXDispatchStatusListener(
                                uno::Reference< frame::XDispatch > & xDisp, const util::URL&    aURL) :
    xDispatch(xDisp),
    aDispURL(aURL)
{
    DBG_ASSERT(xDisp.is(),  "XDispatch not set")
    if(xDisp.is())
        xSelf = this;
}
/* -----------------------------05.01.00 15:19--------------------------------

 ---------------------------------------------------------------------------*/
SwXDispatchStatusListener::~SwXDispatchStatusListener()
{
}
/* -----------------------------05.01.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
void SwXDispatchStatusListener::disposing(const lang::EventObject& Source)
    throw( uno::RuntimeException )
{
    Invalidate();
}
/* -----------------------------05.01.00 15:22--------------------------------

 ---------------------------------------------------------------------------*/
void SwXDispatchStatusListener::statusChanged(const frame::FeatureStateEvent& rEvent)
    throw( uno::RuntimeException )
{
    if(rEvent.FeatureURL.Complete == aDispURL.Complete && rEvent.IsEnabled)
    {
        uno::Sequence <beans::PropertyValue > aArgs(0);
        xDispatch->dispatch(aDispURL, aArgs);
        Invalidate();
    }
}
/* -----------------------------05.01.00 15:45--------------------------------

 ---------------------------------------------------------------------------*/
void    SwXDispatchStatusListener::Invalidate()
{
    if(xDispatch.is())
    {
        try
        {
            xDispatch->removeStatusListener(xSelf, aDispURL);
        }
        catch(...)
        {
        }
    }
    xSelf = 0;
}
/*-----------------08/28/97 08:41pm-----------------

--------------------------------------------------*/
void lcl_SetUIPrefs(const SwViewOption* pPref, SwView* pView, ViewShell* pSh )
{
    // in FrameSets kann die tatsaechliche Sichtbarkeit von der Einstellung der ViewOptions abweichen
    sal_Bool bVScrollChanged = pPref->IsViewVScrollBar() != pSh->GetViewOptions()->IsViewVScrollBar();
    sal_Bool bHScrollChanged = pPref->IsViewHScrollBar() != pSh->GetViewOptions()->IsViewHScrollBar();;

    pSh->SetUIOptions(*pPref);

    const SwViewOption* pNewPref = pSh->GetViewOptions();

    // Scrollbars an / aus
    if(bVScrollChanged)
    {
        if(pNewPref->IsViewVScrollBar())
            pView->CreateVScrollbar();
        else
            pView->KillVScrollbar();
    }
    if(bHScrollChanged)
    {
        if ( pNewPref->IsViewHScrollBar() || pSh->IsBrowseMode() )
            pView->CreateHScrollbar();
        else
            pView->KillHScrollbar();
    }

    // Lineale an / aus
    if(pNewPref->IsViewVLin())
        pView->CreateVLineal();
    else
        pView->KillVLineal();

    // TabWindow an/aus
    if(pNewPref->IsViewTabwin())
        pView->CreateTab();
    else
        pView->KillTab();
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
    DBG_ASSERT(PTR_CAST(SwView, pView),"keine SwView uebergeben")
    const TypeId aTypeId = TYPE(SwView);
    // auf Sichtbarkeit pruefen, bis der Sfx das GetFirst/Next
    //mit bOnlyVisible implementiert hat
    SwView* pNView = (SwView*)SfxViewShell::GetNext(*pView, &aTypeId);
    while(pNView && !pNView->GetViewFrame()->IsVisible())
    {
        pNView = (SwView*)SfxViewShell::GetNext(*pNView, &aTypeId);
    }
    return pNView;
}

/*------------------------------------------------------------------------
 Beschreibung:  Liefert UsrPref mit den Einstellungen des aktuellen
                Fensters
------------------------------------------------------------------------*/


void SwModule::MakeUsrPref( SwViewOption &rToFill, sal_Bool bWeb ) const
{
        // Einstellungen Ansicht der Core setzen
    SwView* pView = ::GetActiveView();
    SwPagePreView* pPPView;

    if(pView)
    {
        rToFill = *(pView->GetWrtShell().GetViewOptions());
        SfxFrame* pTopFrame = pView->GetViewFrame()->GetTopFrame();
        if( pTopFrame->GetChildFrameCount() != 0 )
        {
            rToFill.SetViewTabwin(pUsrPref->IsViewTabwin());
            rToFill.SetViewVLin(pUsrPref->IsViewVLin());
        }
    }
    else if( 0 != (pPPView = PTR_CAST( SwPagePreView, SfxViewShell::Current())) )
        rToFill = *(pPPView->GetViewShell().GetViewOptions());
    else
        rToFill = *((SwModule*)this)->GetUsrPref( bWeb );
}


/*------------------------------------------------------------------------
 Beschreibung:  Neuer Master fuer die Einstellungen wird gesetzt;
                dieser wirkt sich auf die aktuelle Sicht und alle
                folgenden aus.
------------------------------------------------------------------------*/

void SwModule::ApplyUsrPref(const SwViewOption &rUsrPref, SwView* pActView,
                            sal_uInt16 nDest )
{
    SwView* pView = pActView;
    ViewShell* pSh = 0;
    sal_Bool bFrameDoc = sal_False;

    if ( pView )
    {
        //Fuer FrameViews wird das Umschalten der Grafiken in allen Frames
        //mit Writer-Dokumenten eingestellt.
        //Auserdem werden die UI-Prefs in allen Frames eingestellt.

        pSh = &pView->GetWrtShell();
        SfxFrameIterator aIter( *pView->GetViewFrame()->GetTopFrame() );
        SfxFrame *pChildFrame;
        if ( 0 != (pChildFrame = aIter.FirstFrame()) )
        {
            bFrameDoc = sal_True;
            sal_Bool bGraphic = rUsrPref.IsGraphic();
            do
            {
                SfxViewShell* pVShell = pChildFrame->GetCurrentViewFrame()
                                        ? pChildFrame->GetCurrentViewFrame()->GetViewShell()
                                        : 0;
                SwView* pFrameView = PTR_CAST(SwView, pVShell);
                if(!pFrameView || pFrameView == pView)
                    continue;

                SwWrtShell &rSh = pFrameView->GetWrtShell();
                const SwViewOption* pVOpt = rSh.GetViewOptions();
                SwViewOption aVOpt(*pVOpt);
                if(aVOpt.IsGraphic() != bGraphic)
                {
                    aVOpt.SetReadonly(pVOpt->IsReadonly());
                    aVOpt.SetGraphic( bGraphic );
                    rSh.ApplyViewOptions(aVOpt);
                }
                lcl_SetUIPrefs(&rUsrPref, pFrameView, &rSh );

            } while ( 0 != (pChildFrame = aIter.NextFrame( *pChildFrame )));
        }

    }

    SwMasterUsrPref* pPref = (SwMasterUsrPref*)GetUsrPref(
                                         VIEWOPT_DEST_WEB == nDest ? sal_True  :
                                         VIEWOPT_DEST_TEXT== nDest ? sal_False :
                                         pView && pView->ISA(SwWebView) );

    //per Uno soll nur die sdbcx::View, aber nicht das Module veraendert werden
    sal_Bool bViewOnly = VIEWOPT_DEST_VIEW_ONLY == nDest;
    //PreView abfruehstuecken
    SwPagePreView* pPPView;
    if( !pView && 0 != (pPPView = PTR_CAST( SwPagePreView, SfxViewShell::Current())) )
    {
        if(!bViewOnly)
            pPref->SetUIOptions( rUsrPref );
        if(pPref->IsViewVScrollBar())
            pPPView->CreateVScrollbar();
        else
            pPPView->KillVScrollbar();
        if(pPref->IsViewHScrollBar())
            pPPView->CreateHScrollbar();
        else
            pPPView->KillHScrollbar();
        if(!bViewOnly)
        {
            pPref->SetPagePrevRow(rUsrPref.GetPagePrevRow());
            pPref->SetPagePrevCol(rUsrPref.GetPagePrevCol());
        }
        return;
    }

    sal_Bool bViewHLin, bViewVLin;
    if(bFrameDoc)
    {
        //In Frame-Dokumenten soll die Linealumschaltung nicht in den
        //default uebernommen werden.
        const sal_uInt32 nUIOptions = pPref->GetUIOptions();
        bViewHLin = 0 != (nUIOptions & VIEWOPT_2_TABWIN);
        bViewVLin = 0 != (nUIOptions & VIEWOPT_2_VLIN);
    }

    if(!bViewOnly)
    {
        pPref->SetDefault( sal_False );
        pPref->SetUsrPref( rUsrPref );
    }

    if( !pView )
        return;

    // Weitergabe an die CORE
    const sal_Bool bReadonly = pView->GetDocShell()->IsReadOnly();
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

    lcl_SetUIPrefs(pViewOpt, pView, pSh);
    if(bFrameDoc && !bViewOnly)
    {
        pPref->SetViewTabwin(bViewHLin);
        pPref->SetViewVLin(bViewVLin);
    }
    // zum Schluss wird das Idle-Flag wieder gesetzt
    // #42510#
    pPref->SetIdle(sal_True);

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
        pWebPrtOpt = new SwPrintOptions(TRUE);
    }
    else if(!bWeb && !pPrtOpt)
    {
        pPrtOpt = new SwPrintOptions(FALSE);
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

void SwModule::ExecDB(SfxRequest &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwNewDBMgr* pNewDBMgr = NULL;

    if (GetView())
        pNewDBMgr = GetView()->GetWrtShell().GetNewDBMgr();

    sal_uInt16 nSlot(rReq.GetSlot());

    switch (nSlot)
    {
        case FN_QRY_MERGE:
        case FN_QRY:
        {
            String sDBName;

            if (GetView())
            {
                SwWrtShell &rSh = GetView()->GetWrtShell();
                sDBName = rSh.GetDBName();
#ifdef REPLACE_OFADBMGR
#ifdef DEBUG
                sDBName = String::CreateFromAscii("Address Book File");
                sDBName += DB_DELIM;
                sDBName += String::CreateFromAscii("address");
#endif //DEBUG
#endif //REPLACE_OFADBMGR

                rSh.EnterStdMode(); // Wechsel in Textshell erzwingen; ist fuer
                                    // das Mischen von DB-Feldern notwendig.
                GetView()->AttrChangedNotify( &rSh );
                pNewDBMgr->SetMergeType( DBMGR_MERGE );
                ShowDBObj(rSh, sDBName, sal_True);
            }

            if (pNewDBMgr && nSlot == FN_QRY_MERGE)
            {
                SfxViewFrame* pVFrame = GetView()->GetViewFrame();
                uno::Reference< frame::XFrame >  xFrame = pVFrame->GetFrame()->GetFrameInterface();

                uno::Reference< frame::XFrame >  xBeamerFrame = xFrame->findFrame(
                    OUString::createFromAscii("_beamer"), frame::FrameSearchFlag::ALL);

                if(xBeamerFrame.is())
                {
                    uno::Reference< frame::XController >  xCtrl = xBeamerFrame->getController();
                    uno::Reference< frame::XDispatchProvider >  xDispProv(xCtrl, uno::UNO_QUERY);
                    if(xDispProv.is())
                    {
                        OUString uEmpty;
                         util::URL aURL;
                        aURL.Complete = OUString::createFromAscii(".uno:WriterDB/MailMerge");
                        uno::Reference< frame::XDispatch >  xDisp = xDispProv->queryDispatch(aURL, uEmpty, frame::FrameSearchFlag::AUTO);
                        if(xDisp.is())
                        {
                            uno::Reference< frame::XStatusListener >  xDispatchListener =
                                        new SwXDispatchStatusListener(xDisp, aURL);
                            xDisp->addStatusListener(xDispatchListener, aURL);
                        }
                    }
                }
            }
        }
        break;

        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Schaut nach ob's min eine sdbcx::View gibt
 --------------------------------------------------------------------*/

void SwModule::StateIsView(SfxItemSet& rSet)
{
    SwView *pView = ::GetActiveView();

    // Ist ein Writer-Dok ganz oben?
    if( !pView || pView->GetDocShell()->GetProtocol().IsInPlaceActive() )
        rSet.DisableItem(FN_QRY);

    TypeId aType( TYPE(SwView) );
    if( !SfxViewShell::GetFirst(&aType) )   // Ist irgendein Writer-Dok vorhanden?
        rSet.DisableItem(FN_QRY_MERGE);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModule::ShowDBObj(SwWrtShell& rSh, const String& rDBName, sal_Bool bShowError)
{
    String sDBName(rDBName.GetToken(0, DB_DELIM));

#ifdef REPLACE_OFADBMGR
        String sTable(rDBName.GetToken(1, DB_DELIM));
        rSh.GetNewDBMgr()->ShowInBeamer( sDBName, sTable, SW_DB_SELECT_UNKNOWN, aEmptyStr );
#else
    if (sDBName.Len())
    {
        OfaDBMgr *pDBMgr = OFF_APP()->GetOfaDBMgr();
        SbaDatabaseRef pConnection = pDBMgr->GetSbaObject()->
                                GetDatabase(sDBName, bShowError);
        if ( pConnection )
        {
            String sTable(rDBName.GetToken(1, DB_DELIM));
            DBObject eType;
            if( !pConnection->HasObject( eType = dbTable, sTable ) &&
                !pConnection->HasObject( eType = dbQuery , sTable ))
                eType = dbTable;

            //wenn keine Connection gefunden wurde, dann braucht man auch
            //den Beamer nicht
            pDBMgr->ShowInBeamer( sDBName, sTable, eType, aEmptyStr );
        }
    }
#endif
}

/*--------------------------------------------------------------------
    Beschreibung: Redlining
 --------------------------------------------------------------------*/

sal_uInt16 SwModule::GetRedlineAuthor()
{
    if (!bAuthorInitialised)
    {
        sActAuthor = SFX_APP()->GetIniManager()->GetUserFullName();
        bAuthorInitialised = sal_True;
    }

    return InsertRedlineAuthor(sActAuthor);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

const String& SwModule::GetRedlineAuthor(sal_uInt16 nPos)
{
    return *pAuthorNames->GetObject(nPos);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_uInt16 SwModule::GetRedlineAuthorCount()
{
    return pAuthorNames->Count();
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
    Color aCol((ColorData)rAttr.nColor);

    if (rAttr.nColor == COL_TRANSPARENT)
    {
// dynamische Vergabe der Attribute
        static ColorData aColArr[ 10 ] = {
                COL_LIGHTRED,       COL_LIGHTBLUE,      COL_LIGHTMAGENTA,
                COL_GREEN,          COL_RED,            COL_BLUE,
                COL_BROWN,          COL_MAGENTA,        COL_CYAN };

        aCol.SetColor(aColArr[ nAuthor % 10 ]);
    }

    sal_Bool bBackGr = sal_False;

    if (rAttr.nColor == COL_NONE)
        bBackGr = sal_True;

    switch (rAttr.nItemId)
    {
        case SID_ATTR_CHAR_WEIGHT:
            rSet.Put(SvxWeightItem( (FontWeight)rAttr.nAttr, RES_CHRATR_WEIGHT));
            break;

        case SID_ATTR_CHAR_POSTURE:
            rSet.Put(SvxPostureItem((FontItalic)rAttr.nAttr, RES_CHRATR_POSTURE));
            break;

        case SID_ATTR_CHAR_UNDERLINE:
            rSet.Put(SvxUnderlineItem( (FontUnderline)rAttr.nAttr, RES_CHRATR_UNDERLINE));
            break;

        case SID_ATTR_CHAR_STRIKEOUT:
            rSet.Put(SvxCrossedOutItem( (FontStrikeout)rAttr.nAttr, RES_CHRATR_CROSSEDOUT));
            break;

        case SID_ATTR_CHAR_CASEMAP:
            rSet.Put(SvxCaseMapItem((SvxCaseMap)rAttr.nAttr, RES_CHRATR_CASEMAP));
            break;

        case SID_ATTR_BRUSH:
            rSet.Put(SvxBrushItem(aCol, RES_CHRATR_BACKGROUND));
            bBackGr = sal_True;
            break;
    }

    if (!bBackGr)
        rSet.Put(SvxColorItem(aCol));
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
    Beschreibung:
 --------------------------------------------------------------------*/

const AuthorCharAttr& SwModule::GetInsertAuthorAttr() const
{
    return pModuleConfig->GetInsertAuthorAttr();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

const AuthorCharAttr& SwModule::GetDeletedAuthorAttr() const
{
    return pModuleConfig->GetDeletedAuthorAttr();
}

/*--------------------------------------------------------------------
    Beschreibung: Fuer zukuenftige Erweiterung:
 --------------------------------------------------------------------*/

void SwModule::GetFormatAuthorAttr( sal_uInt16 nAuthor, SfxItemSet &rSet )
{
    lcl_FillAuthorAttr( nAuthor, rSet, pModuleConfig->GetFormatAuthorAttr() );
}

const AuthorCharAttr& SwModule::GetFormatAuthorAttr() const
{
    return pModuleConfig->GetFormatAuthorAttr();
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


// Durchreichen der Metric von der ModuleConfig (fuer HTML-Export)
sal_uInt16 SwModule::GetMetric( sal_Bool bWeb ) const
{
    return pModuleConfig->GetMetric( bWeb );
}

// Update-Stati durchreichen
sal_uInt16 SwModule::GetLinkUpdMode( sal_Bool ) const
{
    return pModuleConfig->GetLinkMode();
}

sal_uInt16 SwModule::GetFldUpdateFlags( sal_Bool ) const
{
    return pModuleConfig->GetFldUpdateFlags();
}

void SwModule::CheckSpellChanges( sal_Bool bOnlineSpelling,
        sal_Bool bIsSpellWrongAgain, sal_Bool bIsSpellAllAgain )
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
            if ( pTmp->GetRootFrm() )
                pTmp->SpellItAgainSam( bInvalid, bOnlyWrong );
        }
//      pSpell->SetSpellWrongAgain( sal_False );
//      pSpell->SetSpellAllAgain( sal_False );
    }
}

/*-------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.50  2000/09/18 16:05:12  willem.vandorp
    OpenOffice header added.

    Revision 1.49  2000/09/08 15:11:56  os
    use configuration service

    Revision 1.48  2000/09/07 15:59:20  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.47  2000/09/07 08:25:24  os
    SwPrintOptions uses configuration service

    Revision 1.46  2000/07/18 12:50:07  os
    replace ofadbmgr

    Revision 1.45  2000/05/26 07:21:28  os
    old SW Basic API Slots removed

    Revision 1.44  2000/05/16 09:15:11  os
    project usr removed

    Revision 1.43  2000/05/11 12:05:29  tl
    if[n]def ONE_LINGU entfernt

    Revision 1.42  2000/03/30 13:28:04  os
    UNO III

    Revision 1.41  2000/03/29 12:46:55  jp
    Bug #74570#: ShowDBObj - if no table and no query exist, set default to table

    Revision 1.40  2000/03/21 15:47:50  os
    UNOIII

    Revision 1.39  2000/02/11 14:43:07  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.38  2000/02/09 07:58:08  os
    #72824# check if beamer is available

    Revision 1.37  2000/01/13 11:31:37  tl
    #70735# fixed CheckSpellChanges bug

    Revision 1.36  2000/01/11 10:33:54  tl
    #70735# CheckSpellChanges moved from SwDoc to here

    Revision 1.35  2000/01/06 12:54:01  hr
    #65293#: END_CATCH

    Revision 1.34  2000/01/06 07:32:38  os
    #71436# mail merge dialog: execute via status method disposed

    Revision 1.33  1999/06/22 15:37:58  JP
    Bug #43028#: disableflag for edit in mailbody


      Rev 1.32   22 Jun 1999 17:37:58   JP
   Bug #43028#: disableflag for edit in mailbody

      Rev 1.31   20 Apr 1999 18:59:24   JP
   Task #65061#: neu: ZahlenFormaterkennung abschaltbar

      Rev 1.30   12 Mar 1999 09:55:32   JP
   Task #61405#: Optionen setzen

      Rev 1.29   12 Mar 1999 09:45:04   OS
   #63044# Datenbank-Fehlermeldung anzeigen

      Rev 1.28   01 Mar 1999 16:20:52   MA
   #62490# Altlast entfernt (Drucken und Briefumschlaege/Etiketten und Datenbank)

      Rev 1.27   23 Feb 1999 16:19:38   OS
   #62281# UsrPrefs per UNO nur auf aktuelle sdbcx::View anwenden

      Rev 1.26   17 Feb 1999 08:37:24   OS
   #58158# Einfuegen TabPage auch in HTML-Docs

      Rev 1.25   27 Jan 1999 10:05:22   OS
   #58677# Cursor in Readonly-Bereichen

      Rev 1.24   26 Jan 1999 11:50:10   MIB
   #60875#: Beim HTML-Export Einheit aus Extras/Optionen/Text-/HTML-Dokument nehmen

      Rev 1.23   23 Nov 1998 17:36:52   JP
   Bug #59754#: TerminateHdl wird nicht mehr fuers Clipboard benoetigt

      Rev 1.22   20 Nov 1998 14:01:32   OM
   #59720# Neue Tabellenoptionen beim Einfuegen

      Rev 1.21   05 Oct 1998 17:16:54   OM
   #57458# Auch Queries ueber F4 anzeigen

      Rev 1.20   24 Sep 1998 13:33:02   OS
   #52654# #56685# XTerminateListener fuer die Anmeldung an der Application

      Rev 1.19   24 Aug 1998 12:20:54   OM
   #54552# Serienbriefdialog: Statusupdate immer erzwingen

      Rev 1.18   14 Jul 1998 12:09:06   OS
   Scrollbars nur umschalten, wenn sich die ViewOptions wirklich veraendert haben #52708#

      Rev 1.17   25 May 1998 12:41:28   JP
   nMergeType wurde private, ueber SetMergeType setzen

      Rev 1.16   15 May 1998 12:50:02   OM
   Worttrenner

      Rev 1.15   14 May 1998 16:46:00   OM
   Worttrenner konfigurierbar

      Rev 1.14   24 Apr 1998 19:36:54   JP
   neu: DocStat WordDelimiter aus der Configuration holen

      Rev 1.13   24 Mar 1998 17:45:28   OM
   Formataenderung anzeigen

      Rev 1.12   24 Mar 1998 13:43:56   JP
   neu: Redline fuer harte Attributierung

      Rev 1.11   16 Mar 1998 19:09:48   OM
   Zugriff auf Ini optimiert

      Rev 1.10   15 Mar 1998 16:57:22   MA
   #48342# richtig initialisieren

      Rev 1.9   10 Mar 1998 12:23:42   OM
   Get...AuthorAttr

      Rev 1.8   05 Mar 1998 14:44:08   OM
   Redline-Attribute in Module-Cfg speichern

      Rev 1.7   03 Mar 1998 17:00:46   OS
   GetViewOption

      Rev 1.6   28 Feb 1998 15:12:42   OM
   Accept / reject changes

      Rev 1.5   24 Feb 1998 15:32:04   OM
   Redline-Darstellungsoptionen setzen

      Rev 1.4   08 Dec 1997 11:46:16   OS
   benannte Numerierungen entfernt

      Rev 1.3   24 Nov 1997 14:22:36   MA
   includes

      Rev 1.2   18 Nov 1997 14:33:42   OM
   Sba-Umstellung 372

      Rev 1.1   02 Sep 1997 10:21:08   OS
   includes

      Rev 1.0   01 Sep 1997 13:09:16   OS
   Initial revision.


-------------------------------------------------------------------------*/

