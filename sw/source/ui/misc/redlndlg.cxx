/*************************************************************************
 *
 *  $RCSfile: redlndlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-25 19:14:21 $
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

#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif
#ifndef _TOOLS_INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVSTDARR_HXX
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_CTREDLIN_HXX //autogen
#include <svx/ctredlin.hxx>
#endif
#ifndef _SVX_POSTATTR_HXX //autogen
#include <svx/postattr.hxx>
#endif
#ifndef _SVX_POSTDLG_HXX //autogen
#include <svx/postdlg.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _REDLNDLG_HXX
#define _REDLNACCEPTDLG
#include <redlndlg.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _REDLNDLG_HRC
#include <redlndlg.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

#define C2S(cChar) UniString::CreateFromAscii(cChar)
/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SFX_IMPL_MODELESSDIALOG( SwRedlineAcceptChild, FN_REDLINE_ACCEPT )

struct SwRedlineDataChild
{
    const SwRedlineData*        pChild;     // Verweis auf originale gestackte Daten
    const SwRedlineDataChild*   pNext;      // Verweis auf gestackte Daten
    SvLBoxEntry*                pTLBChild;  // zugehoeriger TreeListBox-Eintrag
};

struct SwRedlineDataParent
{
    const SwRedlineData*        pData;      // RedlineDataPtr
    const SwRedlineDataChild*   pNext;      // Verweis auf gestackte Daten
    SvLBoxEntry*                pTLBParent; // zugehoeriger TreeListBox-Eintrag
    String                      sComment;   // Redline-Kommentar

    inline BOOL operator==( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() == rObj.pData->GetSeqNo()); }
    inline BOOL operator< ( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() <  rObj.pData->GetSeqNo()); }
};

typedef SwRedlineDataParent* SwRedlineDataParentPtr;
SV_DECL_PTRARR_DEL(SwRedlineDataParentArr, SwRedlineDataParentPtr, 10, 20)
SV_IMPL_PTRARR(SwRedlineDataParentArr, SwRedlineDataParentPtr)

SV_DECL_PTRARR_SORT(SwRedlineDataParentSortArr, SwRedlineDataParentPtr, 10, 20)
SV_IMPL_OP_PTRARR_SORT(SwRedlineDataParentSortArr, SwRedlineDataParentPtr)

typedef SwRedlineDataChild* SwRedlineDataChildPtr;
SV_DECL_PTRARR_DEL(SwRedlineDataChildArr, SwRedlineDataChildPtr, 4, 4)
SV_IMPL_PTRARR(SwRedlineDataChildArr, SwRedlineDataChildPtr)

typedef SvLBoxEntry* SvLBoxEntryPtr;
SV_DECL_PTRARR(SvLBoxEntryArr, SvLBoxEntryPtr, 100, 100)
SV_IMPL_PTRARR(SvLBoxEntryArr, SvLBoxEntryPtr)

static USHORT nSortMode = 0xffff;
static BOOL   bSortDir = TRUE;

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

class SwRedlineAcceptDlg
{
    Dialog*                 pParentDlg;
    SwRedlineDataParentArr  aRedlineParents;
    SwRedlineDataChildArr   aRedlineChilds;
    SwRedlineDataParentSortArr aUsedSeqNo;
    SvxAcceptChgCtr         aTabPagesCTRL;
    const International&    rIntl;
    PopupMenu               aPopup;
    Timer                   aDeselectTimer;
    Timer                   aSelectTimer;
    Bitmap                  aRootOpened;
    Bitmap                  aRootClosed;
    String                  sInserted;
    String                  sDeleted;
    String                  sFormated;
    String                  sTableChgd;
    String                  sFmtCollSet;
    String                  sFilterAction;
    String                  sAutoFormat;
    Size                    aBorderSz;
    SvxTPView*              pTPView;
    SvxRedlinTable*         pTable;
    Link                    aOldSelectHdl;
    Link                    aOldDeselectHdl;
    BOOL                    bOnlyFormatedRedlines;
    BOOL                    bHasReadonlySel;
    BOOL                    bRedlnAutoFmt;

    DECL_LINK( AcceptHdl,       void* );
    DECL_LINK( AcceptAllHdl,    void* );
    DECL_LINK( RejectHdl,       void* );
    DECL_LINK( RejectAllHdl,    void* );
    DECL_LINK( UndoHdl,         void* );
    DECL_LINK( DeselectHdl,     void* );
    DECL_LINK( SelectHdl,       void* );
    DECL_LINK( GotoHdl,         void* );
    DECL_LINK( CommandHdl,      void* );

    USHORT          CalcDiff(USHORT nStart, BOOL bChild);
    void            InsertChilds(SwRedlineDataParent *pParent, const SwRedline& rRedln, const USHORT nAutoFmt);
    void            InsertParents(USHORT nStart, USHORT nEnd = USHRT_MAX);
    void            RemoveParents(USHORT nStart, USHORT nEnd);
    void            InitAuthors();

    String          GetRedlineText(const SwRedline& rRedln, DateTime &rDateTime, USHORT nStack = 0);
    const String&   GetActionText(const SwRedline& rRedln, USHORT nStack = 0);
    USHORT          GetRedlinePos( const SvLBoxEntry& rEntry) const;

public:
    SwRedlineAcceptDlg(Dialog *pParent, BOOL bAutoFmt = FALSE);
    ~SwRedlineAcceptDlg();

    DECL_LINK( FilterChangedHdl, void *pDummy = 0 );

    inline SvxAcceptChgCtr* GetChgCtrl()        { return &aTabPagesCTRL; }
    inline BOOL     HasRedlineAutoFmt() const   { return bRedlnAutoFmt; }

    void            Init(USHORT nStart = 0);
    void            CallAcceptReject( BOOL bSelect, BOOL bAccept );

    void            Initialize(const String &rExtraData);
    void            FillInfo(String &rExtraData) const;

    virtual void    Activate();
    virtual void    Resize();
};

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwRedlineAcceptChild::SwRedlineAcceptChild( Window* pParent,
                                            USHORT nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
    SwChildWinWrapper( pParent, nId )
{
    pWindow = new SwModelessRedlineAcceptDlg( pBindings, this, pParent);

    ((SwModelessRedlineAcceptDlg *)pWindow)->Initialize(pInfo);
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
 --------------------------------------------------------------------*/

BOOL SwRedlineAcceptChild::ReInitDlg(SwDocShell *pDocSh)
{
    BOOL bRet;

    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == TRUE)  // Sofort aktualisieren, Dok-Wechsel
        ((SwModelessRedlineAcceptDlg*)GetWindow())->Activate();

    return bRet;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwModelessRedlineAcceptDlg::SwModelessRedlineAcceptDlg( SfxBindings* pBindings,
                                                        SwChildWinWrapper* pChild,
                                                        Window *pParent) :
    SfxModelessDialog(pBindings, pChild, pParent, SW_RES(DLG_REDLINE_ACCEPT)),
    pChildWin       (pChild)
{
    pImplDlg = new SwRedlineAcceptDlg(this);

    FreeResource();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModelessRedlineAcceptDlg::Activate()
{
    SwView *pView = ::GetActiveView();

    if (!pView) // Kann passieren, wenn man auf eine andere App umschaltet, wenn
        return; // vorher eine Listbox im Dialog den Focus hatte (eigentlich THs Bug)

    SwDocShell *pDocSh = pView->GetDocShell();

    if (pChildWin->GetOldDocShell() != pDocSh)
    {   // Dok-Wechsel
        SwWait aWait( *pDocSh, FALSE );
        SwWrtShell* pSh = pView->GetWrtShellPtr();

        pChildWin->SetOldDocShell(pDocSh);  // Rekursion vermeiden (durch Modified-Hdl)

        BOOL bMod = pSh->IsModified();
        SfxBoolItem aShow(FN_REDLINE_SHOW, TRUE);
        pSh->GetView().GetViewFrame()->GetDispatcher()->Execute(
            FN_REDLINE_SHOW, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD, &aShow, 0L);
        if (!bMod)
            pSh->ResetModified();
        pImplDlg->Init();

        return;
    }

    pImplDlg->Activate();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModelessRedlineAcceptDlg::Initialize(SfxChildWinInfo *pInfo)
{
    String aStr;
    if (pInfo != NULL)
        pImplDlg->Initialize(pInfo->aExtraString);

    SfxModelessDialog::Initialize(pInfo);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModelessRedlineAcceptDlg::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxModelessDialog::FillInfo(rInfo);
    pImplDlg->FillInfo(rInfo.aExtraString);
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwModelessRedlineAcceptDlg::Resize()
{
    pImplDlg->Resize();
    SfxModelessDialog::Resize();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwModelessRedlineAcceptDlg::~SwModelessRedlineAcceptDlg()
{
    delete pImplDlg;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwModalRedlineAcceptDlg::SwModalRedlineAcceptDlg(Window *pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MOD_REDLINE_ACCEPT))
{
    pImplDlg = new SwRedlineAcceptDlg(this, TRUE);

    pImplDlg->Initialize(GetExtraData());
    pImplDlg->Activate();   // Zur Initialisierung der Daten

    FreeResource();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwModalRedlineAcceptDlg::~SwModalRedlineAcceptDlg()
{
    AcceptAll(FALSE);   // Alles uebriggebliebene ablehnen
    pImplDlg->FillInfo(GetExtraData());

    delete pImplDlg;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::Activate()
{
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::Resize()
{
    pImplDlg->Resize();
    SfxModalDialog::Resize();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::AcceptAll( BOOL bAccept )
{
    SvxTPFilter* pFilterTP = pImplDlg->GetChgCtrl()->GetFilterPage();

    if (pFilterTP->IsDate() || pFilterTP->IsAuthor() ||
        pFilterTP->IsRange() || pFilterTP->IsAction())
    {
        pFilterTP->CheckDate(FALSE);    // Alle Filter abschalten
        pFilterTP->CheckAuthor(FALSE);
        pFilterTP->CheckRange(FALSE);
        pFilterTP->CheckAction(FALSE);
        pImplDlg->FilterChangedHdl();
    }

    pImplDlg->CallAcceptReject( FALSE, bAccept );
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwRedlineAcceptDlg::SwRedlineAcceptDlg(Dialog *pParent, BOOL bAutoFmt) :
    pParentDlg      (pParent),
    rIntl           (Application::GetAppInternational()),
    aTabPagesCTRL   (pParent, SW_RES(CTRL_TABPAGES)),
    aPopup          (SW_RES(MN_REDLINE_POPUP)),
    aRootClosed     (SW_RES(BMP_ROOT_CLOSED)),
    aRootOpened     (SW_RES(BMP_ROOT_OPENED)),
    sInserted       (SW_RES(STR_REDLINE_INSERTED)),
    sDeleted        (SW_RES(STR_REDLINE_DELETED)),
    sFormated       (SW_RES(STR_REDLINE_FORMATED)),
    sTableChgd      (SW_RES(STR_REDLINE_TABLECHG)),
    sFmtCollSet     (SW_RES(STR_REDLINE_FMTCOLLSET)),
    sAutoFormat     (SW_RES(STR_REDLINE_AUTOFMT)),
    bOnlyFormatedRedlines( FALSE ),
    bHasReadonlySel ( FALSE ),
    bRedlnAutoFmt   (bAutoFmt)
{
    aTabPagesCTRL.SetHelpId(HID_REDLINE_CTRL);
    pTPView = aTabPagesCTRL.GetViewPage();
    pTable = pTPView->GetTableControl();

    pTPView->InsertWriterHeader();
    pTPView->SetAcceptClickHdl(LINK(this, SwRedlineAcceptDlg, AcceptHdl));
    pTPView->SetAcceptAllClickHdl(LINK(this, SwRedlineAcceptDlg, AcceptAllHdl));
    pTPView->SetRejectClickHdl(LINK(this, SwRedlineAcceptDlg, RejectHdl));
    pTPView->SetRejectAllClickHdl(LINK(this, SwRedlineAcceptDlg, RejectAllHdl));
    pTPView->SetUndoClickHdl(LINK(this, SwRedlineAcceptDlg, UndoHdl));

    aTabPagesCTRL.GetFilterPage()->SetReadyHdl(LINK(this, SwRedlineAcceptDlg, FilterChangedHdl));

    ListBox *pActLB = aTabPagesCTRL.GetFilterPage()->GetLbAction();
    pActLB->InsertEntry(sInserted);
    pActLB->InsertEntry(sDeleted);
    pActLB->InsertEntry(sFormated);
    pActLB->InsertEntry(sTableChgd);

    if (HasRedlineAutoFmt())
    {
        pActLB->InsertEntry(sFmtCollSet);
        pActLB->InsertEntry(sAutoFormat);
        pTPView->ShowUndo(TRUE);
        pTPView->DisableUndo();     // Noch gibts keine UNDO-Events
    }

    pActLB->SelectEntryPos(0);

    pTable->SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    pTable->SetNodeBitmaps( aRootClosed, aRootOpened );
    pTable->SetSelectionMode(MULTIPLE_SELECTION);
    pTable->SetHighlightRange(1);

    static long aStaticTabs[]=
    {
        4,10,70,120,170
    };

    pTable->SetTabs(aStaticTabs);

    // Minimalgroesse setzen
    Size aMinSz(aTabPagesCTRL.GetMinSizePixel());
    Point aPos(aTabPagesCTRL.GetPosPixel());

    aMinSz.Width() += (aPos.X() * 2 - 1);
    aMinSz.Height() += (aPos.Y() * 2 - 1);
    pParentDlg->SetMinOutputSizePixel(aMinSz);

    if (pParentDlg->GetOutputSizePixel().Width() < aMinSz.Width())
        pParentDlg->SetOutputSizePixel(Size(aMinSz.Width(), pParentDlg->GetOutputSizePixel().Height()));
    if (pParentDlg->GetOutputSizePixel().Height() < aMinSz.Height())
        pParentDlg->SetOutputSizePixel(Size(pParentDlg->GetOutputSizePixel().Width(), aMinSz.Height()));

    pTable->SortByCol(nSortMode, bSortDir);

    aOldSelectHdl = pTable->GetSelectHdl();
    aOldDeselectHdl = pTable->GetDeselectHdl();
    pTable->SetSelectHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    pTable->SetDeselectHdl(LINK(this, SwRedlineAcceptDlg, DeselectHdl));
    pTable->SetCommandHdl(LINK(this, SwRedlineAcceptDlg, CommandHdl));

    // Flackern der Buttons vermeiden:
    aDeselectTimer.SetTimeout(100);
    aDeselectTimer.SetTimeoutHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));

    // Mehrfachselektion der selben Texte vermeiden:
    aSelectTimer.SetTimeout(100);
    aSelectTimer.SetTimeoutHdl(LINK(this, SwRedlineAcceptDlg, GotoHdl));
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwRedlineAcceptDlg::~SwRedlineAcceptDlg()
{
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwRedlineAcceptDlg::Init(USHORT nStart)
{
    SwWait aWait( *::GetActiveView()->GetDocShell(), FALSE );
    pTable->SetUpdateMode(FALSE);
    aUsedSeqNo.Remove((USHORT)0, aUsedSeqNo.Count());

    if (nStart)
        RemoveParents(nStart, aRedlineParents.Count() - 1);
    else
    {
        pTable->Clear();
        aRedlineChilds.DeleteAndDestroy(0, aRedlineChilds.Count());
        aRedlineParents.DeleteAndDestroy(nStart, aRedlineParents.Count() - nStart);
    }

    // Parents einfuegen
    InsertParents(nStart);
    InitAuthors();

    pTable->SetUpdateMode(TRUE);
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwRedlineAcceptDlg::InitAuthors()
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();

    SvxTPFilter *pFilterPage = aTabPagesCTRL.GetFilterPage();

    String sAuthor;
    SvStringsSortDtor aStrings;
    String sOldAuthor(pFilterPage->GetSelectedAuthor());
    pFilterPage->ClearAuthors();

    String sParent;
    USHORT nCount = pSh->GetRedlineCount();

    bOnlyFormatedRedlines = TRUE;
    bHasReadonlySel = FALSE;
    BOOL bReadonlySel = FALSE;
    BOOL bIsNotFormated = FALSE;

    // Autoren ermitteln
    for (USHORT i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);

        if( bOnlyFormatedRedlines && REDLINE_FORMAT != rRedln.GetType() )
            bOnlyFormatedRedlines = FALSE;

        if( !bHasReadonlySel && rRedln.HasReadonlySel() )
            bHasReadonlySel = TRUE;

        String *pAuthor = new String(rRedln.GetAuthorString());
        if (!aStrings.Insert(pAuthor))
            delete pAuthor;

        for (USHORT nStack = 1; nStack < rRedln.GetStackCount(); nStack++)
        {
            pAuthor = new String(rRedln.GetAuthorString(nStack));
            if (!aStrings.Insert(pAuthor))
                delete pAuthor;
        }
    }

    for (i = 0; i < aStrings.Count(); i++)
        pFilterPage->InsertAuthor(*aStrings[i]);

    if (pFilterPage->SelectAuthor(sOldAuthor) == LISTBOX_ENTRY_NOTFOUND && aStrings.Count())
        pFilterPage->SelectAuthor(*aStrings[0]);

    BOOL bEnable = pTable->GetEntryCount() != 0;
    BOOL bSel = pTable->FirstSelected() != 0;

    SvLBoxEntry* pSelEntry = pTable->FirstSelected();
    while (pSelEntry)
    {
        USHORT nPos = GetRedlinePos(*pSelEntry);
        const SwRedline& rRedln = pSh->GetRedline( nPos );

        bIsNotFormated |= REDLINE_FORMAT != rRedln.GetType();
        pSelEntry = pTable->NextSelected(pSelEntry);
    }

    pTPView->EnableAccept( bEnable && bSel );
    pTPView->EnableReject( bEnable && bIsNotFormated && bSel );
    pTPView->EnableAcceptAll( bEnable && !bHasReadonlySel );
    pTPView->EnableRejectAll( bEnable && !bHasReadonlySel &&
                                !bOnlyFormatedRedlines );
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

String SwRedlineAcceptDlg::GetRedlineText(const SwRedline& rRedln, DateTime &rDateTime, USHORT nStack)
{
    String sEntry(GetActionText(rRedln, nStack));
    sEntry += '\t';
    sEntry += rRedln.GetAuthorString(nStack);
    sEntry += '\t';

    const DateTime &rDT = rRedln.GetTimeStamp(nStack);
    rDateTime = rDT;

    sEntry += rIntl.GetDate( rDT );
    sEntry += ' ';
    sEntry += rIntl.GetTime( rDT, FALSE, FALSE );
    sEntry += '\t';

    sEntry += rRedln.GetComment(nStack);

    return sEntry;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

const String &SwRedlineAcceptDlg::GetActionText(const SwRedline& rRedln, USHORT nStack)
{
    switch( rRedln.GetType(nStack) )
    {
        case REDLINE_INSERT:    return sInserted;
        case REDLINE_DELETE:    return sDeleted;
        case REDLINE_FORMAT:    return sFormated;
        case REDLINE_TABLE:     return sTableChgd;
        case REDLINE_FMTCOLL:   return sFmtCollSet;
    }

    return aEmptyStr;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwRedlineAcceptDlg::Resize()
{
    Size aSz(pParentDlg->GetOutputSizePixel());

    Point aPos(aTabPagesCTRL.GetPosPixel());

    aSz.Width() -= (aPos.X() * 2 - 1);
    aSz.Height() -= (aPos.Y() * 2 - 1);

    aTabPagesCTRL.SetOutputSizePixel(aSz);
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Aktivierung neu initialisieren
 --------------------------------------------------------------------*/

void SwRedlineAcceptDlg::Activate()
{
    SwView *pView = ::GetActiveView();
    SwWait aWait( *pView->GetDocShell(), FALSE );

    aUsedSeqNo.Remove((USHORT)0, aUsedSeqNo.Count());

    if (!pView) // Kann passieren, wenn man auf eine andere App umschaltet, wenn
        return; // vorher eine Listbox im Dialog den Focus hatte (eigentlich THs Bug)

/*  if (HasRedlineAutoFmt())
    {
        Init();
        return;
    }*/

    // Hat sich was geaendert?
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    USHORT nCount = pSh->GetRedlineCount();

    // Anzahl und Pointer ueberpruefen
    SwRedlineDataParent *pParent = 0;

    for (USHORT i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);

        if (i >= aRedlineParents.Count())
        {
            // Neue Eintraege wurden angehaengt
            Init(i);
            return;
        }

        pParent = aRedlineParents[i];
        if (&rRedln.GetRedlineData() != pParent->pData)
        {
            // Redline-Parents wurden eingefuegt, geaendert oder geloescht
            if ((i = CalcDiff(i, FALSE)) == USHRT_MAX)
                return;
            continue;
        }

        const SwRedlineData *pRedlineData = rRedln.GetRedlineData().Next();
        const SwRedlineDataChild *pBackupData = pParent->pNext;

        if (!pRedlineData && pBackupData)
        {
            // Redline-Childs wurden geloescht
            if ((i = CalcDiff(i, TRUE)) == USHRT_MAX)
                return;
            continue;
        }
        else
        {
            while (pRedlineData)
            {
                if (pRedlineData != pBackupData->pChild)
                {
                    // Redline-Childs wurden eingefuegt, geaendert oder geloescht
                    if ((i = CalcDiff(i, TRUE)) == USHRT_MAX)
                        return;
                    continue;
                }
                if (pBackupData)
                    pBackupData = pBackupData->pNext;
                pRedlineData = pRedlineData->Next();
            }
        }
    }

    if (nCount != aRedlineParents.Count())
    {
        // Redlines wurden am Ende geloescht
        Init(nCount);
        return;
    }

    // Kommentar ueberpruefen
    for (i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);
        pParent = aRedlineParents[i];

        if(!rRedln.GetComment().Equals(pParent->sComment))
        {
            if (pParent->pTLBParent)
            {
                // Nur Kommentar aktualisieren
                pTable->SetEntryText(rRedln.GetComment(), pParent->pTLBParent, 3);
            }
            pParent->sComment = rRedln.GetComment();
        }
    }

    InitAuthors();
}

/* -----------------05.06.98 13:06-------------------
 *
 * --------------------------------------------------*/

USHORT SwRedlineAcceptDlg::CalcDiff(USHORT nStart, BOOL bChild)
{
    if (!nStart)
    {
        Init();
        return USHRT_MAX;
    }

    pTable->SetUpdateMode(FALSE);
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    USHORT nAutoFmt = HasRedlineAutoFmt() ? REDLINE_FORM_AUTOFMT : 0;
    SvLBoxTreeList* pModel = pTable->GetModel();
    SwRedlineDataParent *pParent = aRedlineParents[nStart];
    const SwRedline& rRedln = pSh->GetRedline(nStart);

    if (bChild)     // Sollte eigentlich nie vorkommen, aber sicher ist sicher...
    {
        // Alle Childs des Eintrags wegwerfen und neu initialisieren
        SwRedlineDataChildPtr pBackupData = (SwRedlineDataChildPtr)pParent->pNext;
        SwRedlineDataChildPtr pNext;

        while (pBackupData)
        {
            pNext = (SwRedlineDataChildPtr)pBackupData->pNext;
            if (pBackupData->pTLBChild)
                pModel->Remove(pBackupData->pTLBChild);

            aRedlineChilds.DeleteAndDestroy(aRedlineChilds.GetPos(pBackupData), 1);
            pBackupData = pNext;
        }
        pParent->pNext = 0;

        // Neue Childs einfuegen
        InsertChilds(pParent, rRedln, nAutoFmt);

        pTable->SetUpdateMode(TRUE);
        return nStart;
    }

    // Wurden Eintraege geloescht?
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    for (USHORT i = nStart + 1; i < aRedlineParents.Count(); i++)
    {
        if (aRedlineParents[i]->pData == pRedlineData)
        {
            // Eintraege von nStart bis i-1 entfernen
            RemoveParents(nStart, i - 1);
            pTable->SetUpdateMode(TRUE);
            return nStart - 1;
        }
    }

    // Wurden Eintraege eingefuegt?
    USHORT nCount = pSh->GetRedlineCount();
    pRedlineData = aRedlineParents[nStart]->pData;

    for (i = nStart + 1; i < nCount; i++)
    {
        if (&pSh->GetRedline(i).GetRedlineData() == pRedlineData)
        {
            // Eintraege von nStart bis i-1 einfuegen
            InsertParents(nStart, i - 1);
            pTable->SetUpdateMode(TRUE);
            return nStart - 1;
        }
    }

    pTable->SetUpdateMode(TRUE);
    Init(nStart);   // Alle Eintraege bis zum Ende abgleichen
    return USHRT_MAX;
}

/* -----------------05.06.98 13:57-------------------
 *
 * --------------------------------------------------*/

void SwRedlineAcceptDlg::InsertChilds(SwRedlineDataParent *pParent, const SwRedline& rRedln, const USHORT nAutoFmt)
{
    String sChild;
    SwRedlineDataChild *pLastRedlineChild = 0;
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    BOOL bAutoFmt = (rRedln.GetRealType() & nAutoFmt) != 0;

    const String *pAction = &GetActionText(rRedln);
    BOOL bValidParent = !sFilterAction.Len() || sFilterAction == *pAction;
    bValidParent = bValidParent && pTable->IsValidEntry(&rRedln.GetAuthorString(), &rRedln.GetTimeStamp(), &rRedln.GetComment());
    if (nAutoFmt)
    {
        USHORT nPos;

        if (pParent->pData->GetSeqNo() && !aUsedSeqNo.Insert(pParent, nPos))    // Gibts schon
        {
            if (pParent->pTLBParent)
            {
                pTable->SetEntryText(sAutoFormat, aUsedSeqNo[nPos]->pTLBParent, 0);
                pTable->GetModel()->Remove(pParent->pTLBParent);
                pParent->pTLBParent = 0;
            }
            return;
        }
        bValidParent = bValidParent && bAutoFmt;
    }
    BOOL bValidTree = bValidParent;

    for (USHORT nStack = 1; nStack < rRedln.GetStackCount(); nStack++)
    {
        pRedlineData = pRedlineData->Next();

        SwRedlineDataChildPtr pRedlineChild = new SwRedlineDataChild;
        pRedlineChild->pChild = pRedlineData;
        aRedlineChilds.Insert(pRedlineChild, aRedlineChilds.Count());

        if ( pLastRedlineChild )
            pLastRedlineChild->pNext = pRedlineChild;
        else
            pParent->pNext = pRedlineChild;

        pAction = &GetActionText(rRedln, nStack);
        BOOL bValidChild = !sFilterAction.Len() || sFilterAction == *pAction;
        bValidChild = bValidChild && pTable->IsValidEntry(&rRedln.GetAuthorString(nStack), &rRedln.GetTimeStamp(nStack), &rRedln.GetComment());
        if (nAutoFmt)
            bValidChild = bValidChild && bAutoFmt;
        bValidTree |= bValidChild;

        if (bValidChild)
        {
            RedlinData *pData = new RedlinData;
            pData->pData = pRedlineChild;
            pData->bDisabled = TRUE;
            sChild = GetRedlineText(rRedln, pData->aDateTime, nStack);

            SvLBoxEntry* pChild = pTable->InsertEntry(sChild, pData, pParent->pTLBParent);

            pRedlineChild->pTLBChild = pChild;
            if (!bValidParent)
                pTable->Expand(pParent->pTLBParent);
        }
        else
            pRedlineChild->pTLBChild = 0;

        pLastRedlineChild = pRedlineChild;
    }

    if (pLastRedlineChild)
        pLastRedlineChild->pNext = 0;

    if (!bValidTree && pParent->pTLBParent)
    {
        pTable->GetModel()->Remove(pParent->pTLBParent);
        pParent->pTLBParent = 0;
        if (nAutoFmt)
            aUsedSeqNo.Remove(pParent);
    }
}

/* -----------------05.06.98 15:20-------------------
 *
 * --------------------------------------------------*/

void SwRedlineAcceptDlg::RemoveParents(USHORT nStart, USHORT nEnd)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    USHORT nCount = pSh->GetRedlineCount();

    SvLBoxEntryArr aLBoxArr;

    // Wegen Bug der TLB, die bei Remove den SelectHandler IMMER ruft:
    pTable->SetSelectHdl(aOldSelectHdl);
    pTable->SetDeselectHdl(aOldDeselectHdl);
    BOOL bChildsRemoved = FALSE;
    pTable->SelectAll(FALSE);

    // Hinter dem letzten Eintrag Cursor setzen, da sonst Performance-Problem in TLB.
    // TLB wuerde sonst bei jedem Remove den Cursor erneut umsetzen (teuer)
    USHORT nPos = Min((USHORT)nCount, (USHORT)aRedlineParents.Count()) - 1;
    SvLBoxEntry *pCurEntry = aRedlineParents[nPos]->pTLBParent;
    if (!pCurEntry)
        while (--nPos && (pCurEntry = aRedlineParents[nPos]->pTLBParent) == 0);

    if (pCurEntry)
        pTable->SetCurEntry(pCurEntry);

    SvLBoxTreeList* pModel = pTable->GetModel();

    for (USHORT i = nStart; i <= nEnd; i++)
    {
        if (!bChildsRemoved && aRedlineParents[i]->pNext)
        {
            SwRedlineDataChildPtr pChildPtr = (SwRedlineDataChildPtr)aRedlineParents[i]->pNext;
            USHORT nPos = aRedlineChilds.GetPos(pChildPtr);

            if (nPos != USHRT_MAX)
            {
                USHORT nChilds = 0;

                while (pChildPtr)
                {
                    pChildPtr = (SwRedlineDataChildPtr)pChildPtr->pNext;
                    nChilds++;
                }

                aRedlineChilds.DeleteAndDestroy(nPos, nChilds);
                bChildsRemoved = TRUE;
            }
        }
        SvLBoxEntry *pEntry = aRedlineParents[i]->pTLBParent;
        if (pEntry)
        {
            long nIdx = (long)aLBoxArr.Count() - 1L;
            ULONG nAbsPos = pModel->GetAbsPos(pEntry);
            while (nIdx >= 0 &&
                    pModel->GetAbsPos(aLBoxArr[nIdx]) > nAbsPos)
                nIdx--;
            aLBoxArr.Insert(pEntry, ++nIdx);
        }
    }

    // TLB von hinten abraeumen
    long nIdx = (long)aLBoxArr.Count() - 1L;
    while (nIdx >= 0)
        pModel->Remove(aLBoxArr[nIdx--]);

    pTable->SetSelectHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    pTable->SetDeselectHdl(LINK(this, SwRedlineAcceptDlg, DeselectHdl));
    // Durch Remove wurde leider wieder dauernd von der TLB selektiert...
    pTable->SelectAll(FALSE);

    aRedlineParents.DeleteAndDestroy( nStart, nEnd - nStart + 1);
}

/* -----------------05.06.98 15:20-------------------
 *
 * --------------------------------------------------*/

void SwRedlineAcceptDlg::InsertParents(USHORT nStart, USHORT nEnd)
{
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    USHORT nAutoFmt = HasRedlineAutoFmt() ? REDLINE_FORM_AUTOFMT : 0;

    String sParent;
    USHORT nCount = pSh->GetRedlineCount();
    nEnd = Min((USHORT)nEnd, (USHORT)(nCount - 1)); // Handelt auch nEnd=USHRT_MAX (bis zum Ende) ab

    if (nEnd == USHRT_MAX)
        return;     // Keine Redlines im Dokument

    RedlinData *pData;
    SvLBoxEntry *pParent, *pCurrEntry = 0;
    SwRedlineDataParentPtr pRedlineParent;
    const SwRedline* pCurrRedline;
    if( !nStart && !pTable->FirstSelected() )
    {
        pCurrRedline = pSh->GetCurrRedline();
        if( !pCurrRedline )
        {
            pSh->SwCrsrShell::Push();
            if( 0 == (pCurrRedline = pSh->SelNextRedline()))
                pCurrRedline = pSh->SelPrevRedline();
            pSh->SwCrsrShell::Pop( FALSE );
        }
    }
    else
        pCurrRedline = 0;

    for (USHORT i = nStart; i <= nEnd; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);
        const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();

        pRedlineParent = new SwRedlineDataParent;
        pRedlineParent->pData    = pRedlineData;
        pRedlineParent->pNext    = 0;
        pRedlineParent->sComment = rRedln.GetComment();
        aRedlineParents.Insert(pRedlineParent, i);

        pData = new RedlinData;
        pData->pData = pRedlineParent;
        pData->bDisabled = FALSE;

        sParent = GetRedlineText(rRedln, pData->aDateTime);
        pParent = pTable->InsertEntry(sParent, pData, 0, i);
        if( pCurrRedline == &rRedln )
        {
            pTable->SetCurEntry( pParent );
            pTable->Select( pParent );
            pTable->MakeVisible( pParent );
        }

        pRedlineParent->pTLBParent = pParent;

        InsertChilds(pRedlineParent, rRedln, nAutoFmt);
    }
}

/* -----------------05.06.98 13:06-------------------
 *
 * --------------------------------------------------*/

void SwRedlineAcceptDlg::CallAcceptReject( BOOL bSelect, BOOL bAccept )
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    SvLBoxEntry* pEntry = bSelect ? pTable->FirstSelected() : pTable->First();
    ULONG nPos = LONG_MAX;
    SvUShortsSort aIdx;

    while( pEntry )
    {
        if( !pTable->GetParent( pEntry ) )
        {
            if( bSelect && LONG_MAX == nPos )
                nPos = pTable->GetModel()->GetAbsPos( pEntry );

            RedlinData *pData = (RedlinData *)pEntry->GetUserData();

            if( !pData->bDisabled )
                aIdx.Insert(GetRedlinePos( *pEntry ));
        }

        pEntry = bSelect ? pTable->NextSelected(pEntry) : pTable->Next(pEntry);
    }

    BOOL (SwEditShell:: *FnAccRej)( USHORT ) = &SwEditShell::AcceptRedline;
    if( !bAccept )
        FnAccRej = &SwEditShell::RejectRedline;

    SwWait aWait( *pSh->GetView().GetDocShell(), TRUE );
    pSh->StartAction();
    pSh->StartUndo();

    for( USHORT i = aIdx.Count(); i; )
        (pSh->*FnAccRej)( aIdx[ --i ] );

    pSh->EndUndo();
    pSh->EndAction();

    Activate();

    if( ULONG_MAX != nPos && pTable->GetEntryCount() )
    {
        if( nPos >= pTable->GetEntryCount() )
            nPos = pTable->GetEntryCount() - 1;
        pEntry = pTable->GetEntry( nPos );
        if( !pEntry && nPos-- )
            pEntry = pTable->GetEntry( nPos );
        if( pEntry )
        {
            pTable->Select( pEntry );
            pTable->MakeVisible( pEntry );
            pTable->SetCurEntry(pEntry);
        }
    }
    pTPView->EnableUndo();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwRedlineAcceptDlg::GetRedlinePos( const SvLBoxEntry& rEntry ) const
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    return pSh->FindRedlineOfData( *((SwRedlineDataParent*)((RedlinData *)
                                    rEntry.GetUserData())->pData)->pData );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, AcceptHdl, void*, EMPTYARG)
{
    CallAcceptReject( TRUE, TRUE );
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, AcceptAllHdl, void*, EMPTYARG )
{
    CallAcceptReject( FALSE, TRUE );
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, RejectHdl, void*, EMPTYARG )
{
    CallAcceptReject( TRUE, FALSE );
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, RejectAllHdl, void*, EMPTYARG )
{
    CallAcceptReject( FALSE, FALSE );
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, UndoHdl, void*, EMPTYARG )
{
    SwView * pView = ::GetActiveView();
    pView->GetViewFrame()->GetDispatcher()->
                Execute(SID_UNDO, SFX_CALLMODE_SYNCHRON);
    pTPView->EnableUndo(pView->GetSlotState(SID_UNDO) != 0);

    Activate();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, FilterChangedHdl, void*, EMPTYARG )
{
    SvxTPFilter *pFilterTP = aTabPagesCTRL.GetFilterPage();

    if (pFilterTP->IsAction())
        sFilterAction = pFilterTP->GetLbAction()->GetSelectEntry();
    else
        sFilterAction = aEmptyStr;

    Init();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, DeselectHdl, void*, EMPTYARG )
{
    // Flackern der Buttons vermeiden:
    aDeselectTimer.Start();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, SelectHdl, void*, EMPTYARG )
{
    aDeselectTimer.Stop();
    aSelectTimer.Start();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, GotoHdl, void*, EMPTYARG )
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    aSelectTimer.Stop();

    BOOL bIsNotFormated = FALSE;
    BOOL bSel = FALSE;
    BOOL bReadonlySel = FALSE;

    SvLBoxEntry* pSelEntry = pTable->FirstSelected();
    if( pSelEntry )
    {
        SvLBoxEntry* pActEntry = pSelEntry;
        pSh->StartAction();
        pSh->EnterStdMode();
        pSh->SetCareWin(pParentDlg);

        while (pSelEntry)
        {
            if (pTable->GetParent(pSelEntry))
            {
                pActEntry = pTable->GetParent(pSelEntry);

                if (pTable->IsSelected(pActEntry))
                {
                    pSelEntry = pActEntry = pTable->NextSelected(pSelEntry);
                    continue;   // Nicht zweimal selektieren
                }
            }
            else
                bSel = TRUE;

            USHORT nPos = GetRedlinePos(*pActEntry);

            const SwRedline& rRedln = pSh->GetRedline( nPos );
            bIsNotFormated |= REDLINE_FORMAT != rRedln.GetType();

            if( !bReadonlySel && rRedln.HasReadonlySel() )
                bReadonlySel = TRUE;

            if (pSh->GotoRedline(nPos, TRUE))
            {
                pSh->SetInSelect();
                pSh->EnterAddMode();
            }

            pSelEntry = pActEntry = pTable->NextSelected(pSelEntry);
        }

        pSh->LeaveAddMode();
        pSh->EndAction();
        pSh->SetCareWin(NULL);
    }
    pTPView->EnableAccept( bSel && !bReadonlySel );
    pTPView->EnableReject( bSel && bIsNotFormated && !bReadonlySel );
    pTPView->EnableRejectAll( !bOnlyFormatedRedlines && !bHasReadonlySel );

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwRedlineAcceptDlg, CommandHdl, void*, EMPTYARG )
{
    const CommandEvent aCEvt(pTable->GetCommandEvent());

    switch ( aCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
            SvLBoxEntry* pEntry = pTable->FirstSelected();
            const SwRedline *pRed = 0;

            if (pEntry)
            {
                SvLBoxEntry* pTopEntry = pEntry;

                if (pTable->GetParent(pEntry))
                    pTopEntry = pTable->GetParent(pEntry);

                USHORT nPos = GetRedlinePos(*pTopEntry);

                // Bei geschuetzten Bereichen kommentieren disablen
                if ((pRed = pSh->GotoRedline(nPos, TRUE)) != 0)
                {
                    if( pSh->IsCrsrPtAtEnd() )
                        pSh->SwapPam();
                    pSh->SetInSelect();
                }
            }

            aPopup.EnableItem( MN_EDIT_COMMENT, pEntry && pRed &&
                                            !pTable->GetParent(pEntry) &&
                                            !pTable->NextSelected(pEntry) &&
                                            !pRed->HasReadonlySel() );

            aPopup.EnableItem( MN_SUB_SORT, pTable->First() != 0 );
            USHORT nColumn = pTable->GetSortedCol();
            if (nColumn == 0xffff)
                nColumn = 4;

            PopupMenu *pSubMenu = aPopup.GetPopupMenu(MN_SUB_SORT);
            if (pSubMenu)
            {
                for (USHORT i = MN_SORT_ACTION; i < MN_SORT_ACTION + 5; i++)
                    pSubMenu->CheckItem(i, FALSE);

                pSubMenu->CheckItem(nColumn + MN_SORT_ACTION);
            }

            USHORT nRet = aPopup.Execute(pTable, aCEvt.GetMousePosPixel());

            switch( nRet )
            {
            case MN_EDIT_COMMENT:
                {
                    String sComment;

                    if (pEntry)
                    {
                        if (pTable->GetParent(pEntry))
                            pEntry = pTable->GetParent(pEntry);

                        USHORT nPos = GetRedlinePos(*pEntry);

                        const SwRedline &rRedline = pSh->GetRedline(nPos);
                        sComment = rRedline.GetComment();
                        SfxItemSet aSet(pSh->GetAttrPool(), SvxPostItDialog::GetRanges());
                        const International& rIntl = Application::GetAppInternational();

                        aSet.Put(SvxPostItTextItem(sComment.ConvertLineEnd(), SID_ATTR_POSTIT_TEXT));
                        aSet.Put(SvxPostItAuthorItem(rRedline.GetAuthorString(), SID_ATTR_POSTIT_AUTHOR));

                        const DateTime &rDT = rRedline.GetRedlineData().GetTimeStamp();

                        String sDate(rIntl.GetDate( rDT ));
                        (sDate += ' ' ) += rIntl.GetTime( rDT, FALSE, FALSE );

                        aSet.Put(SvxPostItDateItem(sDate, SID_ATTR_POSTIT_DATE));

                        SvxPostItDialog *pDlg = new SvxPostItDialog(pParentDlg, aSet, FALSE);
                        pDlg->HideAuthor();

                        USHORT nResId = 0;
                        switch( rRedline.GetType() )
                        {
                        case REDLINE_INSERT:
                            nResId = STR_REDLINE_INSERTED;
                            break;
                        case REDLINE_DELETE:
                            nResId = STR_REDLINE_DELETED;
                            break;
                        case REDLINE_FORMAT:
                            nResId = STR_REDLINE_FORMATED;
                            break;
                        case REDLINE_TABLE:
                            nResId = STR_REDLINE_TABLECHG;
                            break;
                        }
                        String sTitle(SW_RES(STR_REDLINE_COMMENT));
                        if( nResId )
                            sTitle += SW_RESSTR( nResId );
                        pDlg->SetText(sTitle);

                        pSh->SetCareWin(pDlg);

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                            String sMsg(((const SvxPostItTextItem&)pOutSet->Get(SID_ATTR_POSTIT_TEXT)).GetValue());

                            // Kommentar einfuegen bzw aendern
                            pSh->SetRedlineComment(sMsg);
                            pTable->SetEntryText(sMsg, pEntry, 3);
                        }

                        delete pDlg;
                        pSh->SetCareWin(NULL);
                    }
                }
                break;

            case MN_SORT_ACTION:
            case MN_SORT_AUTHOR:
            case MN_SORT_DATE:
            case MN_SORT_COMMENT:
            case MN_SORT_POSITION:
                {
                    bSortDir = TRUE;
                    if (nRet - MN_SORT_ACTION == 4 && pTable->GetSortedCol() == 0xffff)
                        break;  // Haben wir schon

                    nSortMode = nRet - MN_SORT_ACTION;
                    if (nSortMode == 4)
                        nSortMode = 0xffff; // unsortiert bzw sortiert nach Position

                    if (pTable->GetSortedCol() == nSortMode)
                        bSortDir = !pTable->GetSortDirection();

                    SwWait aWait( *::GetActiveView()->GetDocShell(), FALSE );
                    pTable->SortByCol(nSortMode, bSortDir);
                    if (nSortMode == 0xffff)
                        Init();             // Alles neu fuellen
                }
                break;
            }
        }
        break;
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwRedlineAcceptDlg::Initialize(const String& rExtraData)
{
    if (rExtraData.Len())
    {
        USHORT nPos = rExtraData.Search(C2S("AcceptChgDat:"));

        // Versuche, den Alignment-String "ALIGN:(...)" einzulesen; wenn
        // er nicht vorhanden ist, liegt eine "altere Version vor
        if (nPos != STRING_NOTFOUND)
        {
            USHORT n1 = rExtraData.Search('(', nPos);
            if (n1 != STRING_NOTFOUND)
            {
                USHORT n2 = rExtraData.Search(')', n1);
                if (n2 != STRING_NOTFOUND)
                {
                    // Alignment-String herausschneiden
                    String aStr = rExtraData.Copy(nPos, n2 - nPos + 1);
                    aStr.Erase(0, n1 - nPos + 1);

                    if (aStr.Len())
                    {
                        USHORT nCount = aStr.ToInt32();

                        for (USHORT i = 0; i < nCount; i++)
                        {
                            USHORT n1 = aStr.Search(';');
                            aStr.Erase(0, n1 + 1);
                            pTable->SetTab(i, aStr.ToInt32(), MAP_PIXEL);
                        }
                    }
                }
            }
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwRedlineAcceptDlg::FillInfo(String &rExtraData) const
{
    rExtraData.AppendAscii("AcceptChgDat:(");

    USHORT  nCount = pTable->TabCount();

    rExtraData += String(nCount);
    rExtraData += ';';
    for(USHORT i = 0; i < nCount; i++)
    {
        rExtraData += String(pTable->GetTab(i));
        rExtraData += ';';
    }
    rExtraData += ')';
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:45  hr
    initial import

    Revision 1.54  2000/09/18 16:05:59  willem.vandorp
    OpenOffice header added.

    Revision 1.53  2000/09/07 15:59:26  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.52  2000/06/14 09:06:30  os
    #75451# enabling of buttons corrected

    Revision 1.51  2000/04/18 15:08:17  os
    UNICODE

    Revision 1.50  2000/02/25 08:56:58  hr
    #73447#: removed temporary

    Revision 1.49  2000/02/11 14:56:07  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.48  1999/09/10 13:19:27  os
    Chg: resource types removed

    Revision 1.47  1999/07/08 14:31:40  MA
    Use internal object to toggle wait cursor


      Rev 1.46   08 Jul 1999 16:31:40   MA
   Use internal object to toggle wait cursor

      Rev 1.45   22 Jan 1999 11:51:20   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.44   12 Jan 1999 14:37:10   OS
   #60805# ChildWindow darf kein Show rufen

      Rev 1.43   18 Nov 1998 15:44:28   OM
   #59103# Redlines nach Kommentaren filtern

      Rev 1.42   13 Nov 1998 16:41:08   OM
   #59103# Autofmt-Redlines: zusammengehoerige filtern

      Rev 1.41   13 Nov 1998 15:35:18   OM
   #59103# Autofmt-Redlines: zusammengehoerige filtern

      Rev 1.40   21 Oct 1998 10:56:34   OM
   #57586# Redlining nach Autoformat

      Rev 1.39   20 Oct 1998 17:52:22   OM
   #57586# Redlining nach Autoformat

      Rev 1.38   20 Oct 1998 11:39:04   OM
   #57586# Redlining nach Autoformat

      Rev 1.37   13 Oct 1998 16:09:38   OM
   #57859# Redline-Dlg: Spaltenbreite merken

      Rev 1.36   09 Oct 1998 17:05:22   JP
   Bug #57741#: neue ResourceIds

      Rev 1.35   18 Aug 1998 11:46:48   OM
   #54866# Nach Aktionen filtern

      Rev 1.34   28 Jul 1998 13:04:14   JP
   Bug #53951#: Selektionen nicht aufheben, wenn keine Selektion in der Listbox ist

      Rev 1.33   14 Jul 1998 14:19:54   OM
   #52859# Autor-Button nicht anzeigen

      Rev 1.32   13 Jul 1998 11:32:58   OM
   #51840# HelpId fuer Redline-Control

      Rev 1.31   10 Jun 1998 17:14:18   OM
   WaitCursor

      Rev 1.30   08 Jun 1998 17:58:16   OM
   Ohne Redlines kein GPF

      Rev 1.29   08 Jun 1998 15:04:22   OM
   Optimiert

      Rev 1.28   05 Jun 1998 17:13:16   OM
   Optimiert

      Rev 1.27   05 Jun 1998 10:46:00   OM
   Optimierungen

      Rev 1.26   03 Jun 1998 17:27:54   OM
   Performance Optimierungen

      Rev 1.25   03 Jun 1998 12:17:08   OM
   Performance Optimierungen

      Rev 1.24   07 Apr 1998 14:21:34   OM
   RedlinData nicht mehr selber loeschen

      Rev 1.23   07 Apr 1998 08:47:10   OM
   Linien anzeigen

      Rev 1.22   03 Apr 1998 13:05:30   OM
   Kein Absturz bei fehlender SwView

      Rev 1.21   01 Apr 1998 13:09:20   OM
   Rekursion vermieden

      Rev 1.20   31 Mar 1998 13:01:32   OM
   Redline-Kommentare in schreibgeschuetzten bereichen nicht editierbar

      Rev 1.19   27 Mar 1998 14:14:42   OM
   ChildWindows im Modified-Hdl updaten

      Rev 1.18   26 Mar 1998 21:48:36   JP
   CallAcceptReject: wenn Pos am Ende steht, dann sollte der letzte Eintrag wieder selektiert werden

      Rev 1.17   26 Mar 1998 15:30:16   JP
   optimiert und Accept/Reject - behalten ihre Position in der Liste

      Rev 1.16   24 Mar 1998 13:45:02   JP
   neu: Redline fuer harte Attributierung

      Rev 1.15   18 Mar 1998 18:30:46   OM
   #48735# GPF im Redlining behoben

      Rev 1.14   18 Mar 1998 11:34:10   OM
   Resize der Basisklasse rufen

      Rev 1.13   17 Mar 1998 09:36:22   OM
   Groesse korrekt initialisieren

      Rev 1.12   16 Mar 1998 19:10:42   OM
   Zugriff auf Ini optimiert

      Rev 1.11   16 Mar 1998 11:00:38   OM
   Autoren vom Dokument zusammensuchen

      Rev 1.10   16 Mar 1998 09:35:42   OM
   4 Spalten

      Rev 1.9   15 Mar 1998 16:20:36   OM
   Groesse merken

      Rev 1.8   10 Mar 1998 12:52:26   OM
   Keine Sortierung per Default

      Rev 1.7   09 Mar 1998 12:59:24   OM
   Sortieren und filtern

      Rev 1.6   09 Mar 1998 11:52:06   OM
   Sortieren und filtern

      Rev 1.5   07 Mar 1998 14:33:36   OM
   Filtern und sortieren

      Rev 1.4   03 Mar 1998 16:03:00   OM
   Aufgeraeumt

      Rev 1.3   03 Mar 1998 10:42:50   OM
   Redline-Browser

      Rev 1.2   28 Feb 1998 15:21:02   OM
   Author vorselektieren

      Rev 1.1   28 Feb 1998 15:12:06   OM
   Accept / reject changes

      Rev 1.0   27 Feb 1998 18:22:16   OM
   Initial revision.

------------------------------------------------------------------------*/
