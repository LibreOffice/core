/*************************************************************************
 *
 *  $RCSfile: templdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-27 09:21:30 $
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

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _EMBOBJ_HXX //autogen
#include <so3/embobj.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _IFACE_HXX //autogen
#include <so3/iface.hxx>
#endif
#pragma hdrstop

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#include <sfxhelp.hxx>
#include "app.hxx"
#include "dispatch.hxx"
#include "bindings.hxx"
#include "templdlg.hxx"
#include "templdgi.hxx"
#include "tplcitem.hxx"
#include "sfxtypes.hxx"
#include "styfitem.hxx"
#include "objsh.hxx"
#include "viewsh.hxx"
#include "newstyle.hxx"
#include "tplpitem.hxx"
#include "sfxresid.hxx"

#include "templdlg.hrc"
#include "sfx.hrc"
#include "dialog.hrc"
#include "arrdecl.hxx"
#include "fltfnc.hxx"
#include "docfilt.hxx"
#include "docfac.hxx"

#include "docvor.hxx"
#include "doctempl.hxx"
#include "module.hxx"
#include "imgmgr.hxx"
#include "helpid.hrc"
#include "appdata.hxx"
#include <objshimp.hxx>

#include "viewfrm.hxx"

//=========================================================================

// Fenster wird ab jetzt dynamisch erstellt. Daher hier R"ander usw.

#define SFX_TEMPLDLG_HFRAME         3
#define SFX_TEMPLDLG_VTOPFRAME      3

#ifdef MAC
#define SFX_TEMPLDLG_VBOTFRAME      8
#else
#define SFX_TEMPLDLG_VBOTFRAME      3
#endif

#define SFX_TEMPLDLG_MIDHSPACE      3
#define SFX_TEMPLDLG_MIDVSPACE      3
#define SFX_TEMPLDLG_FILTERHEIGHT   100

static USHORT nLastItemId = USHRT_MAX;

//=========================================================================

TYPEINIT0(SfxCommonTemplateDialog_Impl);
TYPEINIT1(SfxTemplateDialog_Impl,SfxCommonTemplateDialog_Impl);
TYPEINIT1(SfxTemplateCatalog_Impl,SfxCommonTemplateDialog_Impl);

SFX_IMPL_DOCKINGWINDOW(SfxTemplateDialogWrapper, SID_STYLE_DESIGNER)

//-------------------------------------------------------------------------

// Redirektionsfunktionen

SfxTemplateDialog::SfxTemplateDialog
(
    SfxBindings *pBindings,
    SfxChildWindow *pCW,
    Window *pParent
)

/*  [Beschreibung]
    Gestalterklasse.
*/
    : SfxDockingWindow( pBindings, pCW, pParent, SfxResId(DLG_STYLE_DESIGNER) ),
      pImpl(0)
{
    pImpl=new SfxTemplateDialog_Impl(pParent, pBindings, this);
}

//-------------------------------------------------------------------------

SfxTemplateDialog::~SfxTemplateDialog()
{
    delete pImpl;
}

ISfxTemplateCommon* SfxTemplateDialog::GetISfxTemplateCommon()
{
    return pImpl->GetISfxTemplateCommon();
}

//-------------------------------------------------------------------------

void SfxTemplateDialog::Update()
{
    pImpl->Update();
}

//-------------------------------------------------------------------------

void SfxTemplateDialog::Resize()
{
    if(pImpl)
        pImpl->Resize();
    SfxDockingWindow::Resize();
}


//-------------------------------------------------------------------------

SfxChildAlignment SfxTemplateDialog::CheckAlignment(SfxChildAlignment eActAlign,SfxChildAlignment eAlign)
{
    switch (eAlign)
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_HIGHESTBOTTOM:
            return eActAlign;
            break;

        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_FIRSTLEFT:
        case SFX_ALIGN_LASTLEFT:
        case SFX_ALIGN_FIRSTRIGHT:
        case SFX_ALIGN_LASTRIGHT:
            return eAlign;
            break;

        default:
            return eAlign;
            break;
    }
}

//-------------------------------------------------------------------------

SfxTemplateCatalog::SfxTemplateCatalog(Window *pParent, SfxBindings *pBindings)
 : SfxModalDialog(pParent,SfxResId(RID_STYLECATALOG))
{
    pImpl = new SfxTemplateCatalog_Impl(pParent, pBindings, this);
}

//-------------------------------------------------------------------------

SfxTemplateCatalog::~SfxTemplateCatalog()
{
    delete pImpl;
}

//-------------------------------------------------------------------------

void DropListBox_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    nModifier = rMEvt.GetModifier();
    SvTreeListBox::MouseButtonDown( rMEvt );
}


BOOL DropListBox_Impl::QueryDrop( DropEvent& rEvt )

/*  [Beschreibung ]

    Droppen eigentlich immer dann erlaubt, wenn von den selektieren
    Objekten ine Stylesheet erstellt werden kann und der Dragvorgang
    von der Selektion aus gestartet wurde.
    Hier vereinfachend: Es liegen Daten im Clipboardformat der
    Applikation vor. Cave mehrere Writer gleichzeitig!
*/

{
    SvDataObjectRef xObj = SvDataObject::PasteDragServer( rEvt );
    SvObjectDescriptor aObj( xObj );
    SfxObjectShell* pDocShell = pDialog->GetObjectShell();
    if ( pDocShell && aObj.GetClassName() == pDocShell->GetFactory() )
    {
        if( rEvt.IsDefaultAction() )
            rEvt.SetAction( DROP_COPY );
        if( rEvt.GetAction() != DROP_COPY )
            return FALSE;
        SvLBoxEntry *pEntry = GetEntry( rEvt.GetPosPixel(), TRUE );
        if( pPreDropEntry && ( pEntry != pPreDropEntry || rEvt.IsLeaveWindow()) )
        {
            ShowTargetEmphasis( pPreDropEntry, FALSE );
            pPreDropEntry = 0;
        }
        if( pEntry && pEntry!=pPreDropEntry && !rEvt.IsLeaveWindow() )
        {
            ShowTargetEmphasis( pEntry, TRUE );
//          MakeVisible( pEntry );
            pPreDropEntry = pEntry;
        }
        if( !pEntry && pDialog->bNewByExampleDisabled ||
            pEntry && pDialog->bUpdateByExampleDisabled )
            return FALSE;
        else
            return TRUE;
    }
    return SvTreeListBox::QueryDrop( rEvt );
}

//-------------------------------------------------------------------------

BOOL DropListBox_Impl::Drop( const DropEvent& rEvt )
{
    BOOL bRet = TRUE;
    SvDataObjectRef xObj = SvDataObject::PasteDragServer( rEvt );
    SvObjectDescriptor aObj( xObj );
    SfxObjectShell* pDocShell = pDialog->GetObjectShell();
    if ( pDocShell && aObj.GetClassName() == pDocShell->GetFactory() )
    {
        SvLBoxEntry *pEntry = GetEntry( rEvt.GetPosPixel(), TRUE );
        if( pEntry && pEntry!=pPreDropEntry )
        {
            ShowTargetEmphasis( pEntry, FALSE );
        }
        if( pEntry )
        {
            pDialog->SelectStyle( GetEntryText( pEntry)  );
            pDialog->ActionSelect( SID_STYLE_UPDATE_BY_EXAMPLE );
        }
        else
        {
            pDialog->ActionSelect( SID_STYLE_NEW_BY_EXAMPLE );
        }
    }
    else
        bRet =  SvTreeListBox::Drop( rEvt );
    return bRet;
}

//-------------------------------------------------------------------------


SfxActionListBox::SfxActionListBox
(
    SfxCommonTemplateDialog_Impl* pParent,
    WinBits nWinBits
)

/*  [Beschreibung]

    ListBox- Klasse, die im Command-Handler ein PopupMenu (Gestalter
    spezifisch) startet.

*/

:       DropListBox_Impl(pParent->GetWindow(), nWinBits, pParent)

{
}

//-------------------------------------------------------------------------

SfxActionListBox::SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent,
                                    const ResId &rResId) :
    DropListBox_Impl(pParent->GetWindow(), rResId, pParent)
{
}

//-------------------------------------------------------------------------

void SfxActionListBox::Command( const CommandEvent& rCEvt )
{
    if ( COMMAND_CONTEXTMENU  == rCEvt.GetCommand() )
        pDialog->ExecuteContextMenu_Impl( rCEvt.GetMousePosPixel(), this );
    else
        DropListBox_Impl::Command( rCEvt );
}

//-------------------------------------------------------------------------

SfxTemplateDialogWrapper::SfxTemplateDialogWrapper(Window *pParent,
        USHORT nId,  SfxBindings *p, SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParent, nId)
{
    SfxTemplateDialog *pWin = new SfxTemplateDialog(p, this, pParent);
    pWindow = pWin;
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    Point aEmptyPoint;

    Rectangle aRect( aEmptyPoint, pParent->GetOutputSizePixel() );
    Size aSize = aRect.GetSize();
    Point aPoint = aRect.TopLeft();
    Size aWinSize = pWin->GetSizePixel();
    aPoint.X() += aSize.Width() - aWinSize.Width() - 20;
    aPoint.Y() += aSize.Height() / 2 - aWinSize.Height() / 2;
    pWin->SetFloatingPos( aPoint );

    pWin->Initialize( pInfo );
    pWin->SetMinOutputSizePixel(pWin->pImpl->GetMinOutputSizePixel());
}

//=========================================================================
SV_DECL_PTRARR_DEL(ExpandedEntries, StringPtr,16,8)
SV_IMPL_PTRARR(ExpandedEntries, StringPtr)


/*  [Beschreibung]

    TreeListBox- Klasse f"ur die Anzeige der hierarchischen View
    der Vorlagen

*/
class StyleTreeListBox_Impl: public DropListBox_Impl
{
    SvLBoxEntry *pCurEntry;
    Link            aDoubleClickLink;
    Link            aDropLink;
    String          aParent;
    String          aStyle;
    SfxCommonTemplateDialog_Impl* pCommon;
protected:

    virtual void    Command( const CommandEvent& rMEvt );
    virtual BOOL    DoubleClickHdl();
    virtual long    ExpandingHdl();
    virtual void    ExpandedHdl();
    virtual BOOL    NotifyMoving(SvLBoxEntry*  pTarget,
                                 SvLBoxEntry*  pEntry,
                                 SvLBoxEntry*& rpNewParent,
                                 ULONG&           rNewChildPos);
public:
    StyleTreeListBox_Impl( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle = 0);
    void SetDoubleClickHdl(const Link &rLink) { aDoubleClickLink = rLink; }
    void SetDropHdl(const Link &rLink) { aDropLink = rLink; }
    const String &GetParent() const { return aParent; }
    const String &GetStyle() const { return aStyle; }
    void MakeExpanded_Impl(ExpandedEntries& rEntries) const;
};

//-------------------------------------------------------------------------


void StyleTreeListBox_Impl::MakeExpanded_Impl(ExpandedEntries& rEntries) const
{
    SvLBoxEntry *pEntry;
    USHORT nCount=0;
    for(pEntry=(SvLBoxEntry*)FirstVisible();pEntry;pEntry=(SvLBoxEntry*)NextVisible(pEntry))
    {
        if(IsExpanded(pEntry))
        {
            StringPtr pString=new String(GetEntryText(pEntry));
            rEntries.Insert(pString,nCount++);
        }
    }
}


BOOL StyleTreeListBox_Impl::DoubleClickHdl()

/*  [Beschreibung]


    DoubleClick-Handler; ruft entsprechenden Link.
    Virtuelle Methode aus SV.

*/
{
    aDoubleClickLink.Call(this);
    return FALSE;
}

//-------------------------------------------------------------------------

void StyleTreeListBox_Impl::Command( const CommandEvent& rCEvt )

/*  [Beschreibung]

    Command Handler; dieser executed ein PopupMenu (Gestalter
    spezifisch).
    Virtuelle Methode aus SV.

*/
{
    if(COMMAND_CONTEXTMENU  == rCEvt.GetCommand())
//      pCommon->ExecuteContextMenu_Impl(OutputToScreenPixel(rCEvt.GetMousePosPixel()));
        pCommon->ExecuteContextMenu_Impl( rCEvt.GetMousePosPixel(), this );
    else
        SvTreeListBox::Command(rCEvt);
}

//-------------------------------------------------------------------------

BOOL StyleTreeListBox_Impl::NotifyMoving(SvLBoxEntry*  pTarget,
                                         SvLBoxEntry*  pEntry,
                                         SvLBoxEntry*& rpNewParent,
                                         ULONG& lPos)
/*  [Beschreibung]

    NotifyMoving Handler; dieser leitet per Link das Event an den Dialog
    weiter.
    Virtuelle Methode aus SV.

*/
{
    const International aInter(Application::GetAppInternational());
    if(!pTarget || !pEntry)
        return FALSE;
    aParent = GetEntryText(pTarget);
    aStyle  = GetEntryText(pEntry);
    const BOOL bRet = (BOOL)aDropLink.Call(this);
    rpNewParent = pTarget;
    lPos=0;
    for(SvLBoxEntry *pTmpEntry=FirstChild(pTarget);
        pTmpEntry && COMPARE_LESS==aInter.Compare(
            GetEntryText(pTmpEntry),GetEntryText(pEntry));
        pTmpEntry=NextSibling(pTmpEntry),lPos++);

    return bRet? (BOOL)2: FALSE;
}

//-------------------------------------------------------------------------

long  StyleTreeListBox_Impl::ExpandingHdl()

/*  [Beschreibung]

    ExpandingHdl Handler; der aktuelle Eintrag wird gemerkt.
    Virtuelle Methode aus SV.

    [Querverweise]
    <StyleTreeListBox_Impl::ExpandedHdl()>

*/
{
    pCurEntry = GetCurEntry();
    return TRUE;
}

//-------------------------------------------------------------------------

void  StyleTreeListBox_Impl::ExpandedHdl()

/*  [Beschreibung]

    ExpandedHdl Handler;
    Virtuelle Methode aus SV.

    [Querverweise]
    <StyleTreeListBox_Impl::ExpandingHdl()>

*/

{
    SvLBoxEntry *pEntry = GetHdlEntry();
    if(!IsExpanded(pEntry) && pCurEntry != GetCurEntry())
        SelectAll( FALSE );
    pCurEntry = 0;
}

//-------------------------------------------------------------------------

StyleTreeListBox_Impl::StyleTreeListBox_Impl(
    SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle) :
    DropListBox_Impl(pParent->GetWindow(), nWinStyle, pParent),
    pCurEntry(0),
    pCommon(pParent)

/*  [Beschreibung]

    Konstruktor StyleTreeListBox_Impl

*/

{
    SetDragOptions(DROP_MOVE);
}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

class StyleTreeArr_Impl;


/*  [Beschreibung]

    Interne Struktur f"ur den Aufbau der hierarchischen View

*/

struct StyleTree_Impl
{
    String aName;
    String aParent;
    StyleTreeArr_Impl *pChilds;
    BOOL bIsExpanded;
    BOOL HasParent() const { return aParent.Len() != 0; }

    StyleTree_Impl(const String &rName, const String &rParent):
        aName(rName), aParent(rParent), pChilds(0), bIsExpanded(0) {}
    ~StyleTree_Impl();
    void Put(StyleTree_Impl* pIns, ULONG lPos=ULONG_MAX);
    ULONG Count();
};

typedef StyleTree_Impl* StyleTree_ImplPtr;
SV_DECL_PTRARR_DEL(StyleTreeArr_Impl, StyleTree_ImplPtr, 16, 8)
SV_IMPL_PTRARR(StyleTreeArr_Impl, StyleTree_ImplPtr)


ULONG StyleTree_Impl::Count()
{
    return pChilds ? pChilds->Count() : 0L;
}

//-------------------------------------------------------------------------

StyleTree_Impl::~StyleTree_Impl()
{
    delete pChilds;
}

//-------------------------------------------------------------------------

void StyleTree_Impl::Put(StyleTree_Impl* pIns, ULONG lPos)
{
    if ( !pChilds )
        pChilds = new StyleTreeArr_Impl;

    if ( ULONG_MAX == lPos )
        lPos = pChilds->Count();
    pChilds->Insert( pIns, (USHORT)lPos );
}

//-------------------------------------------------------------------------

StyleTreeArr_Impl &MakeTree_Impl(StyleTreeArr_Impl &rArr)
{
    const International aInter(Application::GetAppInternational());
    const USHORT nCount = rArr.Count();
    // Alle unter ihren Parents einordnen
    USHORT i;
    for(i = 0; i < nCount; ++i)
    {
        StyleTree_ImplPtr pEntry = rArr[i];
        if(pEntry->HasParent())
        {
            for(USHORT j = 0; j < nCount; ++j)
            {
                StyleTree_ImplPtr pCmp = rArr[j];
                if(pCmp->aName == pEntry->aParent)
                {
                    // initial sortiert einfuegen
                    USHORT ii;
                    for ( ii = 0;
                         ii < pCmp->Count() && COMPARE_LESS ==
                         aInter.Compare( (*pCmp->pChilds)[ii]->aName,
                                        pEntry->aName);++ii);
                    pCmp->Put(pEntry,ii);
                    break;
                }
            }
        }
    }
    // alle, die schon unter ihrem Parent eingeordnet wurden
    // entfernen
    for(i = 0; i < rArr.Count(); )
    {
        if(rArr[i]->HasParent())
            rArr.Remove(i);
        else
            ++i;
    }
    return rArr;
}

//-------------------------------------------------------------------------


inline BOOL IsExpanded_Impl( const ExpandedEntries& rEntries,
                             const String &rStr)
{
    USHORT nCount=rEntries.Count();
    for(USHORT n=0;n<nCount;n++)
        if(*rEntries[n]==rStr)
            return TRUE;
    return FALSE;
}



SvLBoxEntry* FillBox_Impl(SvTreeListBox *pBox,
                                 StyleTree_ImplPtr pEntry,
                                 const ExpandedEntries& rEntries,
                                 SvLBoxEntry* pParent = 0)
{
    SvLBoxEntry* pNewEntry = pBox->InsertEntry(pEntry->aName, pParent);
    const USHORT nCount = pEntry->pChilds? pEntry->pChilds->Count(): 0;
    for(USHORT i = 0; i < nCount; ++i)
        FillBox_Impl(pBox, (*pEntry->pChilds)[i], rEntries, pNewEntry);
    return pNewEntry;
}

//-------------------------------------------------------------------------

// Konstruktor

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl( SfxBindings* pB, SfxDockingWindow* pW ) :

    aISfxTemplateCommon     ( this ),
    pBindings               ( pB ),
    pWindow                 ( pW ),
    pModule                 ( NULL ),
    pTimer                  ( NULL ),
    pStyleSheetPool         ( NULL ),
    pTreeBox                ( NULL ),
    pCurObjShell            ( NULL ),

    aFmtLb                  ( this, WB_BORDER | WB_TABSTOP | WB_SORT ),
    aFilterLb               ( pW, WB_BORDER | WB_DROPDOWN | WB_TABSTOP ),

    nActFamily              ( 0xffff ),
    nActFilter              ( 0 ),
    nAppFilter              ( 0 ),

    bDontUpdate             ( FALSE ),
    bIsWater                ( FALSE ),
    bEnabled                ( TRUE ),
    bUpdate                 ( FALSE ),
    bUpdateFamily           ( FALSE ),
    bCanEdit                ( FALSE ),
    bCanDel                 ( FALSE ),
    bCanNew                 ( TRUE ),
    bWaterDisabled          ( FALSE ),
    bNewByExampleDisabled   ( FALSE ),
    bUpdateByExampleDisabled( FALSE ),
    bTreeDrag               ( TRUE ),
    bHierarchical           ( FALSE ),
    bBindingUpdate          ( TRUE )

{
    aFmtLb.SetHelpId( HID_TEMPLATE_FMT );
    aFilterLb.SetHelpId( HID_TEMPLATE_FILTER );
    aFmtLb.SetWindowBits( WB_SORT );
    Font aFont = aFmtLb.GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
#ifdef MAC
    aFont.SetName( String::CreateFromAscii( "Geneva" ) );
    aFont.SetSize( Size( 0, 10 ) );
#endif
    aFmtLb.SetFont( aFont );
}

//-------------------------------------------------------------------------

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl( SfxBindings* pB, ModalDialog* pW ) :

    aISfxTemplateCommon     ( this ),
    pBindings               ( pB ),
    pWindow                 ( pW ),
    pModule                 ( NULL ),
    pTimer                  ( NULL ),
    pStyleSheetPool         ( NULL ),
    pTreeBox                ( NULL ),
    pCurObjShell            ( NULL ),

    aFmtLb                  ( this, SfxResId( BT_VLIST ) ),
    aFilterLb               ( pW, SfxResId( BT_FLIST ) ),

    nActFamily              ( 0xffff ),
    nActFilter              ( 0 ),
    nAppFilter              ( 0 ),

    bDontUpdate             ( FALSE ),
    bIsWater                ( FALSE ),
    bEnabled                ( TRUE ),
    bUpdate                 ( FALSE ),
    bUpdateFamily           ( FALSE ),
    bCanEdit                ( FALSE ),
    bCanDel                 ( FALSE ),
    bCanNew                 ( TRUE ),
    bWaterDisabled          ( FALSE ),
    bNewByExampleDisabled   ( FALSE ),
    bUpdateByExampleDisabled( FALSE ),
    bTreeDrag               ( TRUE ),
    bHierarchical           ( FALSE ),
    bBindingUpdate          ( TRUE )

{
    aFmtLb.SetWindowBits( WB_SORT );
}

//-------------------------------------------------------------------------

USHORT SfxCommonTemplateDialog_Impl::StyleNrToInfoOffset(USHORT nId)
{
    const SfxStyleFamilyItem *pItem=pStyleFamilies->GetObject(nId);
    return SfxFamilyIdToNId(pItem->GetFamily())-1;
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableEdit(BOOL bEnable)
{
    SfxCommonTemplateDialog_Impl::EnableEdit( bEnable );
    if( !bEnable || !bUpdateByExampleDisabled )
        EnableItem( SID_STYLE_UPDATE_BY_EXAMPLE, bEnable);
}

//-------------------------------------------------------------------------


USHORT SfxCommonTemplateDialog_Impl::InfoOffsetToStyleNr(USHORT nId)
{
    for ( USHORT i=0;i<pStyleFamilies->Count();i++ )
        if ( SfxFamilyIdToNId(pStyleFamilies->GetObject(i)->GetFamily()) == nId+1 )
            return i;
    DBG_ERROR("Style Nummer nicht gefunden");
    return 0;
}


//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ReadResource()
{
    // globale Benutzer-Resource auslesen
    USHORT i;
    for(i = 0; i < MAX_FAMILIES; ++i)
        pFamilyState[i] = 0;

    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    pCurObjShell = pViewFrame->GetObjectShell();
    ResMgr* pMgr = pCurObjShell ? pCurObjShell->GetResMgr() : NULL;
    ResId aFamId( DLG_STYLE_DESIGNER, pMgr );
    aFamId.SetRT(RSC_SFX_STYLE_FAMILIES);
    if( !pMgr || !pMgr->IsAvailable( aFamId ) )
        pStyleFamilies = new SfxStyleFamilies;
    else
        pStyleFamilies = new SfxStyleFamilies( aFamId );

    nActFilter = pCurObjShell ? pCurObjShell->Get_Impl()->nStyleFilter : 0xFFFF;

        // Einfuegen in die Toolbox
        // umgekehrte Reihenfolge, da immer vorne eingefuegt wird.
    USHORT nCount = pStyleFamilies->Count();

    pBindings->ENTERREGISTRATIONS();

    for(i = 0; i < nCount; ++i)
    {
        USHORT nSlot = 0;
        switch((USHORT)pStyleFamilies->GetObject(i)->GetFamily())
        {
            case SFX_STYLE_FAMILY_CHAR: nSlot = SID_STYLE_FAMILY1; break;
            case SFX_STYLE_FAMILY_PARA: nSlot = SID_STYLE_FAMILY2; break;
            case SFX_STYLE_FAMILY_FRAME:nSlot = SID_STYLE_FAMILY3; break;
            case SFX_STYLE_FAMILY_PAGE: nSlot = SID_STYLE_FAMILY4; break;
            case SFX_STYLE_FAMILY_PSEUDO: nSlot = SID_STYLE_FAMILY5; break;
            default: DBG_ERROR("unbekannte StyleFamily"); break;
        }
        pBoundItems[i] =
            new SfxTemplateControllerItem(nSlot, *this, *pBindings);
    }
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_WATERCAN, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_NEW_BY_EXAMPLE, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_UPDATE_BY_EXAMPLE, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_NEW, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_DRAGHIERARCHIE, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_EDIT, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_DELETE, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_FAMILY, *this, *pBindings);
    pBindings->LEAVEREGISTRATIONS();

    for(; i < COUNT_BOUND_FUNC; ++i)
        pBoundItems[i] = 0;

    StartListening(*pBindings);

//In umgekehrter Reihenfolge des Auftretens in den Stylefamilies einfuegen.
//Das ist fuer den Toolbar des Gestalters. Die Listbox des Kataloges achtet
//selbst auf korrekte Reihenfolge.

//Reihenfolgen: Reihenfolge in der Resource = Reihenfolge in Toolbar bzw.
//Listbox.
//Reihenfolge aufsteigender SIDs: Niedrige SIDs werden als erstes angezeigt,
//wenn Vorlagen mehrerer Familien aktiv sind.

    for( ; nCount--; )
    {
        const SfxStyleFamilyItem *pItem = pStyleFamilies->GetObject( nCount );
        USHORT nId = SfxFamilyIdToNId( (USHORT) pItem->GetFamily() );
        InsertFamilyItem( nId, pItem );
    }

    Resize();

    USHORT nStart = SID_STYLE_FAMILY1;
    USHORT nEnd = SID_STYLE_FAMILY4;

    for ( i = nStart; i <= nEnd; i++ )
        pBindings->Update(i);

    pModule = pCurObjShell ? pCurObjShell->GetModule() : NULL;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ClearResource()
{
    ClearFamilyList();
    DELETEX(pStyleFamilies);
    USHORT i;
    for ( i = 0; i < MAX_FAMILIES; ++i )
        DELETEX(pFamilyState[i]);
    for ( i = 0; i < COUNT_BOUND_FUNC; ++i )
        delete pBoundItems[i];
    pCurObjShell = NULL;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::Initialize()
{
    // globale Benutzer-Resource auslesen
    ReadResource();
    pBindings->Invalidate( SID_STYLE_FAMILY );
    pBindings->Update( SID_STYLE_FAMILY );
    Update_Impl();

    aFilterLb.SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, FilterSelectHdl ) );
    aFmtLb.SetDoubleClickHdl( LINK( this, SfxCommonTemplateDialog_Impl, ApplyHdl ) );
    aFmtLb.SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, FmtSelectHdl ) );

    aFilterLb.Show();
    aFmtLb.Show();
}

//-------------------------------------------------------------------------

SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl()
{
    String aEmpty;
    if ( bIsWater )
        Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
    GetWindow()->Hide();
    DELETEX(pStyleFamilies);
    USHORT i;
    for ( i = 0; i < MAX_FAMILIES; ++i )
        DELETEX(pFamilyState[i]);
    for ( i = 0; i < COUNT_BOUND_FUNC; ++i )
        delete pBoundItems[i];
    if ( pStyleSheetPool )
        EndListening(*pStyleSheetPool);
    pStyleSheetPool = NULL;
    delete pTreeBox;
    delete pTimer;
}

//-------------------------------------------------------------------------

USHORT SfxCommonTemplateDialog_Impl::SfxFamilyIdToNId( USHORT nFamily )
{
    switch ( nFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:     return 1; break;
        case SFX_STYLE_FAMILY_PARA:     return 2; break;
        case SFX_STYLE_FAMILY_FRAME:    return 3; break;
        case SFX_STYLE_FAMILY_PAGE:     return 4; break;
        case SFX_STYLE_FAMILY_PSEUDO:   return 5; break;
        default:                        return 0;
    }
}

//-------------------------------------------------------------------------

// Hilfsfunktion: Zugriff auf aktuelles Family-Item
const SfxStyleFamilyItem *SfxCommonTemplateDialog_Impl::GetFamilyItem_Impl() const
{
    const USHORT nCount = pStyleFamilies->Count();
    for(USHORT i = 0; i < nCount; ++i)
    {
        const SfxStyleFamilyItem *pItem = pStyleFamilies->GetObject(i);
//        if(!pItem)continue;
        USHORT nId = SfxFamilyIdToNId((USHORT)pItem->GetFamily());
        if(nId == nActFamily)
            return pItem;
    }
    return 0;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::SelectStyle(const String &rStr)
{
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if ( !pItem )
        return;
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSheetBase* pStyle = pStyleSheetPool->Find( rStr, eFam, SFXSTYLEBIT_ALL );
    if( pStyle )
        EnableEdit( !(pStyle->GetMask() & SFXSTYLEBIT_READONLY) );

    if ( pTreeBox )
    {
        if ( rStr.Len() )
        {
            SvLBoxEntry* pEntry = pTreeBox->First();
            while ( pEntry )
            {
                if ( pTreeBox->GetEntryText( pEntry ) == rStr )
                {
                    pTreeBox->MakeVisible( pEntry );
                    pTreeBox->Select( pEntry );
                    return;
                }
                pEntry = pTreeBox->Next( pEntry );
            }
        }
        else
            pTreeBox->SelectAll( FALSE );
    }
    else
    {
        BOOL bSelect = ( rStr.Len() > 0 );
        if ( bSelect )
        {
            SvLBoxEntry* pEntry = (SvLBoxEntry*)aFmtLb.FirstVisible();
            while ( pEntry && aFmtLb.GetEntryText( pEntry ) != rStr )
                pEntry = (SvLBoxEntry*)aFmtLb.NextVisible( pEntry );
            if ( !pEntry )
                bSelect = FALSE;
            else
            {
                aFmtLb.MakeVisible( pEntry );
                aFmtLb.Select( pEntry );
                FmtSelectHdl( NULL );
            }
        }

        if ( !bSelect )
            aFmtLb.SelectAll( FALSE );
    }
}

//-------------------------------------------------------------------------

String SfxCommonTemplateDialog_Impl::GetSelectedEntry() const
{
    String aRet;
    if ( pTreeBox )
    {
        SvLBoxEntry* pEntry = pTreeBox->FirstSelected();
        if( !pEntry )
            pEntry = aFmtLb.GetPreDropEntry();
        if ( pEntry )
            aRet = pTreeBox->GetEntryText( pEntry );
    }
    else
    {
        SvLBoxEntry* pEntry = aFmtLb.FirstSelected();
        if ( pEntry )
            aRet = aFmtLb.GetEntryText( pEntry );
    }
    return aRet;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::EnableTreeDrag( BOOL bEnable )
{
    if ( pStyleSheetPool )
    {
        SfxStyleSheetBase* pStyle = pStyleSheetPool->First();
        if ( pTreeBox )
        {
            if ( pStyle && pStyle->HasParentSupport() && bEnable )
                pTreeBox->SetDragDropMode(SV_DRAGDROP_CTRL_MOVE);
            else
                pTreeBox->SetDragDropMode(SV_DRAGDROP_NONE);
        }
    }
    bTreeDrag = bEnable;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::FillTreeBox()
{
    DBG_ASSERT(pTreeBox, "FillTreeBox ohne TreeBox");
    if(pStyleSheetPool && nActFamily != 0xffff)
    {
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        pStyleSheetPool->SetSearchMask(pItem->GetFamily(), SFXSTYLEBIT_ALL);
        StyleTreeArr_Impl aArr;
        SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
        if(pStyle && pStyle->HasParentSupport() && bTreeDrag )
            pTreeBox->SetDragDropMode(SV_DRAGDROP_CTRL_MOVE);
        else
            pTreeBox->SetDragDropMode(SV_DRAGDROP_NONE);
        while(pStyle)
        {
            StyleTree_ImplPtr pNew =
                new StyleTree_Impl(pStyle->GetName(), pStyle->GetParent());
            aArr.Insert(pNew, aArr.Count());
            pStyle = pStyleSheetPool->Next();
        }
        MakeTree_Impl(aArr);
        ExpandedEntries aEntries;
        if(pTreeBox)
            ((const StyleTreeListBox_Impl *)pTreeBox)->
                MakeExpanded_Impl( aEntries);
        pTreeBox->SetUpdateMode( FALSE );
        pTreeBox->Clear();
        const USHORT nCount = aArr.Count();
        for(USHORT i = 0; i < nCount; ++i)
            FillBox_Impl(pTreeBox, aArr[i], aEntries);

//      EnableEdit(FALSE);
        EnableItem(SID_STYLE_WATERCAN,FALSE);

        SfxTemplateItem *pState = pFamilyState[nActFamily-1];

        if(nCount)
            pTreeBox->Expand(pTreeBox->First());

        for(SvLBoxEntry *pEntry=pTreeBox->First();
            pEntry;pEntry=pTreeBox->Next(pEntry))
        {
            if(IsExpanded_Impl(aEntries,pTreeBox->GetEntryText(pEntry)))
                pTreeBox->Expand(pEntry);
        }

        if(pState)  //Aktuellen Eintrag selektieren
        {
            const String aStyle(pState->GetStyleName());
            SelectStyle(aStyle);
        }
        EnableDelete();

        pTreeBox->SetUpdateMode( TRUE );
    }
}

//-------------------------------------------------------------------------
inline BOOL SfxCommonTemplateDialog_Impl::HasSelectedStyle() const
{
    return pTreeBox? pTreeBox->FirstSelected() != 0:
            aFmtLb.GetSelectionCount() != 0;
}


//-------------------------------------------------------------------------

// intern: Aktualisierung der Anzeige
void SfxCommonTemplateDialog_Impl::UpdateStyles_Impl(USHORT nFlags)     // Flags, was aktualisiert werden soll (s.o.)
{
    DBG_ASSERT(nFlags, "nichts zu tun");
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    if (!pItem)
    {
        // Ist beim Vorlagenkatalog der Fall
        SfxTemplateItem **ppItem = pFamilyState;
        const USHORT nFamilyCount = pStyleFamilies->Count();
        USHORT n;
        for(n=0;n<nFamilyCount;n++)
            if(ppItem[StyleNrToInfoOffset(n)])break;
        if ( n == nFamilyCount )
            // passiert gelegentlich bei Beichten, Formularen etc.; weiß der Teufel warum
            return;
        ppItem+=StyleNrToInfoOffset(n);
        nAppFilter = (*ppItem)->GetValue();
        FamilySelect(  StyleNrToInfoOffset(n)+1 );
        pItem = GetFamilyItem_Impl();
    }

    const SfxStyleFamily eFam = pItem->GetFamily();

    SfxFilterTupel *pT = pItem->GetFilterList().GetObject(nActFilter);
    USHORT nFilter = pT ? pItem->GetFilterList().GetObject(nActFilter)->nFlags : 0;
    if(!nFilter)    // automatisch
        nFilter = nAppFilter;

    DBG_ASSERT(pStyleSheetPool, "kein StyleSheetPool");
    if(pStyleSheetPool)
    {
        pStyleSheetPool->SetSearchMask(eFam, nFilter);
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        if((nFlags & UPDATE_FAMILY) == UPDATE_FAMILY)
        {
            if(ISA(SfxTemplateDialog_Impl))
                ((DockingWindow* )GetWindow())->SetText(
                    pItem->GetText()); // Titel setzen
            CheckItem(nActFamily, TRUE);    // Button in Toolbox checken
            aFilterLb.SetUpdateMode(FALSE);
            aFilterLb.Clear();
            const SfxStyleFilter& rFilter = pItem->GetFilterList();
            for(USHORT i = 0; i < rFilter.Count(); ++i)
                aFilterLb.InsertEntry(rFilter.GetObject(i)->aName);
            if(nActFilter < aFilterLb.GetEntryCount())
                aFilterLb.SelectEntryPos(nActFilter);
            else
                aFilterLb.SelectEntryPos(nActFilter = 0);
            aFilterLb.InsertEntry(String(SfxResId(STR_STYLE_FILTER_HIERARCHICAL)));

            if(pTreeBox)
                aFilterLb.SelectEntry(String(SfxResId(STR_STYLE_FILTER_HIERARCHICAL)));
            //Falls in Treedarstellung wieder Family Hierarchie selektieren

            aFilterLb.SetUpdateMode(TRUE);
        }
        else
        {
            if( nActFilter < aFilterLb.GetEntryCount() )
                aFilterLb.SelectEntryPos(nActFilter);
            else
                aFilterLb.SelectEntryPos( nActFilter = 0 );
        }

        if(nFlags & UPDATE_FAMILY_LIST)
        {
//          EnableEdit(FALSE);
            EnableItem(SID_STYLE_WATERCAN,FALSE);

            SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
            SvLBoxEntry* pEntry = aFmtLb.First();
            SvStringsDtor aStrings;

            while( pStyle )
            {
                //Bubblesort
                for( USHORT nPos = aStrings.Count() + 1 ; nPos-- ;)
                {
                    if( !nPos || *aStrings[nPos-1] < pStyle->GetName() )
                    {
                        // Die Namen stehen in den Styles, also nicht kopieren
                        // Reingefallen!: Writer hat insgesamt nur 1 Style
                        aStrings.Insert(
                            new String( pStyle->GetName() ), nPos );
                        break;
                    }
                }
                pStyle = pStyleSheetPool->Next();
            }


            USHORT nCount = aStrings.Count();
            USHORT nPos = 0;
            while( nPos < nCount && pEntry &&
                   *aStrings[ nPos ] == aFmtLb.GetEntryText( pEntry ) )
            {
                nPos++;
                pEntry = aFmtLb.Next( pEntry );
            }

            if( nPos < nCount || pEntry )
            {
                // Box mit den Vorlagen fuellen
                aFmtLb.SetUpdateMode(FALSE);
                aFmtLb.Clear();

                nPos = 0;
                while( nPos < nCount )
                    aFmtLb.InsertEntry( *aStrings.GetObject( nPos++ ));
                aFmtLb.SetUpdateMode(TRUE);
            }
                // aktuelle Vorlage anzeigen
            SfxTemplateItem *pState = pFamilyState[nActFamily-1];
            if(pState)
            {
                const String aStyle(pState->GetStyleName());
                SelectStyle(aStyle);
            }
            EnableDelete();
        }
    }
}

//-------------------------------------------------------------------------

// Aktualisierung Anzeige: Gie\skanne an/aus
void SfxCommonTemplateDialog_Impl::SetWaterCanState(const SfxBoolItem *pItem)
{
//  EnableItem(SID_STYLE_WATERCAN, pItem != 0);
    bWaterDisabled =  pItem == 0;

    if(pItem)
    {
        CheckItem(SID_STYLE_WATERCAN, pItem->GetValue());
        EnableItem( SID_STYLE_WATERCAN, TRUE );
    }
    else
        EnableItem(SID_STYLE_WATERCAN, FALSE);

//Waehrend Giesskannenmodus Statusupdates ignorieren.

    USHORT nCount=pStyleFamilies->Count();
    pBindings->EnterRegistrations();
    for(USHORT n=0; n<nCount; n++)
    {
        SfxControllerItem *pCItem=pBoundItems[n];
        BOOL bChecked = pItem && pItem->GetValue();
        if( pCItem->IsBound() == bChecked )
            if( !bChecked )
                pCItem->ReBind();
            else
                pCItem->UnBind();
    }
    pBindings->LeaveRegistrations();
}

//-------------------------------------------------------------------------

// Item mit dem Status einer Family wird kopiert und gemerkt
// (Aktualisierung erfolgt, wenn alle Stati aktualisiert worden sind.
// Siehe auch: <SfxBindings::AddDoneHdl(const Link &)>

void SfxCommonTemplateDialog_Impl::SetFamilyState( USHORT nSlotId, const SfxTemplateItem* pItem )
{
    USHORT nIdx = nSlotId - SID_STYLE_FAMILY_START;
    DELETEZ(pFamilyState[nIdx]);
    if ( pItem )
        pFamilyState[nIdx] = new SfxTemplateItem(*pItem);
    bUpdate = TRUE;

    // Wenn verwendete Vorlagen ( wie zum Teufel findet man das heraus ?? )
    bUpdateFamily = TRUE;
}

//-------------------------------------------------------------------------

// Benachrichtigung durch SfxBindings, da"s die Aktualisierung
// beendet ist. St"o\st die Aktualisierung der Anzeige an.

void SfxCommonTemplateDialog_Impl::Update_Impl()
{
    BOOL bDocChanged=FALSE;
    SfxStyleSheetBasePool *pNewPool = 0;
    SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();
    if( pDocShell )
        pNewPool = pDocShell->GetStyleSheetPool();

    if ( pNewPool != pStyleSheetPool && pDocShell )
    {
        SfxModule* pNewModule = pDocShell->GetModule();
        if( pNewModule && pNewModule != pModule )
        {
            ClearResource();
            ReadResource();
        }
        if ( pStyleSheetPool )
        {
            EndListening(*pStyleSheetPool);
            pStyleSheetPool = 0;
        }

        if ( pNewPool )
        {
            StartListening(*pNewPool);
            pStyleSheetPool = pNewPool;
            bDocChanged=TRUE;
        }
//      InvalidateBindings();
    }

    if (bUpdateFamily)
        UpdateFamily_Impl();

    USHORT i;
    for(i = 0; i < MAX_FAMILIES; ++i)
        if(pFamilyState[i])
            break;
    if(i == MAX_FAMILIES || !pNewPool)
        // nichts erlaubt
        return;

     SfxTemplateItem *pItem = 0;
     // aktueller Bereich nicht innerhalb der erlaubten Bereiche
     // oder Default
     if(nActFamily == 0xffff || 0 == (pItem = pFamilyState[nActFamily-1] ) )
     {
         CheckItem(nActFamily, FALSE);
         SfxTemplateItem **ppItem = pFamilyState;
         const USHORT nFamilyCount = pStyleFamilies->Count();
         USHORT n;
         for(n=0;n<nFamilyCount;n++)
             if(ppItem[StyleNrToInfoOffset(n)])break;
         ppItem+=StyleNrToInfoOffset(n);

         nAppFilter = (*ppItem)->GetValue();
         FamilySelect(  StyleNrToInfoOffset(n)+1 );

         pItem = *ppItem;
     }
     else if( bDocChanged )
     {
         // andere DocShell -> alles neu
         CheckItem( nActFamily, TRUE );
         nActFilter = pDocShell->Get_Impl()->nStyleFilter;
         nAppFilter = pItem->GetValue();
         if(!pTreeBox)
         {
             UpdateStyles_Impl(UPDATE_FAMILY_LIST);
         }
         else
             FillTreeBox();
     }
     else
     {
         // anderer Filter fuer automatisch
         CheckItem( nActFamily, TRUE );
         const SfxStyleFamilyItem *pStyleItem =  GetFamilyItem_Impl();
#ifdef DEBUG
         SfxFilterTupel *pT = pStyleItem->GetFilterList().GetObject(nActFilter);
#endif
         if(0 == pStyleItem->GetFilterList().GetObject(nActFilter)->nFlags
            && nAppFilter != pItem->GetValue())
         {
             nAppFilter = pItem->GetValue();
             if(!pTreeBox)
                 UpdateStyles_Impl(UPDATE_FAMILY_LIST);
             else
                 FillTreeBox();
         }
         else
             nAppFilter = pItem->GetValue();
     }
     const String aStyle(pItem->GetStyleName());
     SelectStyle(aStyle);
     EnableDelete();
     EnableNew( bCanNew );
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxCommonTemplateDialog_Impl, TimeOut, Timer *, pTim )
{
    if(!bDontUpdate)
    {
        bDontUpdate=TRUE;
        if(!pTreeBox)
            UpdateStyles_Impl(UPDATE_FAMILY_LIST);
        else
        {
            FillTreeBox();
            SfxTemplateItem *pState = pFamilyState[nActFamily-1];
            if(pState)
            {
                const String aStyle(pState->GetStyleName());
                SelectStyle(aStyle);
                EnableDelete();
            }
        }
        bDontUpdate=FALSE;
        DELETEZ(pTimer);
    }
    else
        pTimer->Start();
    return 0;
}


//-------------------------------------------------------------------------
void SfxCommonTemplateDialog_Impl::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                                          const SfxHint& rHint, const TypeId& rHintType)
{
    // Aktualisierung anstossen
    if(rHint.Type() == TYPE(SfxSimpleHint))
    {
        switch(((SfxSimpleHint&) rHint ).GetId())
        {
          case SFX_HINT_UPDATEDONE:
            {
                SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
                SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();
                if( bUpdate && (
                    !IsCheckedItem(SID_STYLE_WATERCAN) || pDocShell
                    && pDocShell->GetStyleSheetPool() != pStyleSheetPool) )
                {
                    bUpdate = FALSE;
                    Update_Impl();
                }
                else if ( bUpdateFamily )
                {
                    UpdateFamily_Impl();
                }

                if( pStyleSheetPool )
                {
                    String aStr = GetSelectedEntry();
                    if( aStr.Len() && pStyleSheetPool )
                    {
                        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
                        if( !pItem ) break;
                        const SfxStyleFamily eFam = pItem->GetFamily();
                        SfxStyleSheetBase *pStyle =
                            pStyleSheetPool->Find(
                                aStr, eFam, SFXSTYLEBIT_ALL );
                        if( pStyle )
                            EnableEdit(
                                !(pStyle->GetMask() & SFXSTYLEBIT_READONLY) );
                    }
                }
                break;
            }
        // noetig, wenn zwichen Dokumenten umgeschaltet wird,
        // aber in beiden Dokumenten die gleiche Vorlage gilt.
        // Nicht sofort Update_Impl rufen, fr den Fall da\s eines
        // der Dokumente ein internes InPlaceObjekt ist!
          case SFX_HINT_DOCCHANGED:
            bUpdate = TRUE;
            break;
          case SFX_HINT_DYING:
          {
            EndListening(*pStyleSheetPool);
            pStyleSheetPool=0;
            break;
          }
        }
    }

    // Timer nicht aufsetzen, wenn der StyleSheetPool in die Kiste geht, denn
    // es kann sein, da\s sich ein neuer erst anmeldet, nachdem der Timer
    // abgelaufen ist - macht sich schlecht in UpdateStyles_Impl() !

    ULONG nId = ((SfxSimpleHint&) rHint).GetId();

    if(!bDontUpdate && nId != SFX_HINT_DYING &&
       (rHint.Type() == TYPE(SfxStyleSheetPoolHint)||
       rHint.Type() == TYPE(SfxStyleSheetHint) ||
       rHint.Type() == TYPE( SfxStyleSheetHintExtended )))
    {
        if(!pTimer)
        {
            pTimer=new Timer;
            pTimer->SetTimeout(500);
            pTimer->SetTimeoutHdl(LINK(this,SfxCommonTemplateDialog_Impl,TimeOut));
        }
        pTimer->Start();

    }
}


//-------------------------------------------------------------------------

// Anderer Filter; kann durch den Benutzer umgeschaltet werden
// oder als Folge von Neu oder Bearbeiten, wenn die aktuelle
// Vorlage einem anderen Filter zugewiesen wurde.
void SfxCommonTemplateDialog_Impl::FilterSelect(
                USHORT nEntry,                  // Idx des neuen Filters
                BOOL bForce )                   // Aktualisierung erzwingen, auch wenn der neue Filter gleich dem aktuellen ist
{
    if( nEntry != nActFilter || bForce )
    {
        nActFilter = nEntry;
        SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();
        if (pDocShell)
            pDocShell->Get_Impl()->nStyleFilter = nActFilter;

        SfxStyleSheetBasePool *pOldStyleSheetPool = pStyleSheetPool;
        pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): 0;
        if ( pOldStyleSheetPool != pStyleSheetPool )
        {
            if ( pOldStyleSheetPool )
                EndListening(*pOldStyleSheetPool);
            if ( pStyleSheetPool )
                StartListening(*pOldStyleSheetPool);
        }

        UpdateStyles_Impl(UPDATE_FAMILY_LIST);
    }
}

//-------------------------------------------------------------------------

// Intern: Ausf"uhren von Funktionen "uber den Dispatcher
BOOL SfxCommonTemplateDialog_Impl::Execute_Impl(
    USHORT nId, const String &rStr, const String& rRefStr, USHORT nFamily,
    USHORT nMask, USHORT *pIdx, const USHORT* pModifier)
{
    SfxDispatcher &rDispatcher = *SFX_APP()->GetDispatcher_Impl();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, nFamily);
    SfxUInt16Item aMask( SID_STYLE_MASK, nMask );
    SfxStringItem aUpdName(SID_STYLE_UPD_BY_EX_NAME, rStr);
    SfxStringItem aRefName( SID_STYLE_REFERENCE, rRefStr );
    const SfxPoolItem* pItems[ 6 ];
    USHORT nCount = 0;
    if( rStr.Len() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;
    if( nMask )
        pItems[ nCount++ ] = &aMask;
    if(SID_STYLE_UPDATE_BY_EXAMPLE == nId)
    {
        //Sonderloesung fuer Numerierungsupdate im Writer
        const String aTemplName(GetSelectedEntry());
        aUpdName.SetValue(aTemplName);
        pItems[ nCount++ ] = &aUpdName;
    }
    if ( rRefStr.Len() )
        pItems[ nCount++ ] = &aRefName;

    pItems[ nCount++ ] = 0;

    const SfxPoolItem* pItem;
    USHORT nModi = pModifier ? *pModifier : 0;
    pItem = rDispatcher.Execute(
        nId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD | SFX_CALLMODE_MODAL,
        pItems, nModi );

    if ( !pItem )
        return FALSE;

    if ( nId == SID_STYLE_NEW || SID_STYLE_EDIT == nId )
    {
        SfxUInt16Item *pFilterItem = PTR_CAST(SfxUInt16Item, pItem);
        DBG_ASSERT(pFilterItem, "SfxUINT16Item erwartet");
        USHORT nFilterFlags = pFilterItem->GetValue() & ~SFXSTYLEBIT_USERDEF;
        if(!nFilterFlags)       // Benutzervorlage?
            nFilterFlags = pFilterItem->GetValue();
        const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
        const USHORT nCount = (USHORT) pFamilyItem->GetFilterList().Count();

        for ( USHORT i = 0; i < nCount; ++i )
        {
            const SfxFilterTupel *pTupel =
                pFamilyItem->GetFilterList().GetObject(i);

            if ( ( pTupel->nFlags & nFilterFlags ) == nFilterFlags && pIdx )
                *pIdx = i;
        }
    }
    return TRUE;
}

//-------------------------------------------------------------------------

// Handler der Listbox der Filter
IMPL_LINK( SfxCommonTemplateDialog_Impl, FilterSelectHdl, ListBox *, pBox )
{
    if ( pBox->GetSelectEntry() ==  String(SfxResId(STR_STYLE_FILTER_HIERARCHICAL)) )
    {
        if ( !bHierarchical )
        {
            // TreeView einschalten
            bHierarchical=TRUE;
            const String aSelectEntry( GetSelectedEntry());
            aFmtLb.Hide();
            // aFilterLb.Disable();

            pTreeBox = new StyleTreeListBox_Impl(
                    this, WB_HASBUTTONS | WB_HASLINES |
                    WB_BORDER | WB_TABSTOP | WB_HASLINESATROOT |
                    WB_HASBUTTONSATROOT );
            pTreeBox->SetFont( aFmtLb.GetFont() );

            pTreeBox->SetPosSizePixel(aFmtLb.GetPosPixel(), aFmtLb.GetSizePixel());
            pTreeBox->SetNodeBitmaps(Image(SfxResId(BMP_COLLAPSED)),
                                     Image(SfxResId(BMP_EXPANDED)));
            pTreeBox->SetSelectHdl(
                LINK(this, SfxCommonTemplateDialog_Impl, FmtSelectHdl));
            ((StyleTreeListBox_Impl*)pTreeBox)->
                SetDoubleClickHdl(
                    LINK(this, SfxCommonTemplateDialog_Impl,  ApplyHdl));
            ((StyleTreeListBox_Impl*)pTreeBox)->
                SetDropHdl(LINK(this, SfxCommonTemplateDialog_Impl,  DropHdl));
            pTreeBox->SetIndent(10);

            FillTreeBox();
            SelectStyle(aSelectEntry);
            pTreeBox->Show();
        }
    }

    else
    {
        DELETEZ(pTreeBox);
        aFmtLb.Show();
        //                              aFilterLb.Enable();
        // Falls bHierarchical, kann sich die Familie geaendert haben
        FilterSelect(pBox->GetSelectEntryPos(), bHierarchical );
        bHierarchical=FALSE;
//      UpdateStyles_Impl(UPDATE_FAMILY_LIST);  // Anzeige aktualisieren
    }

    return 0;
}

//-------------------------------------------------------------------------

// Select-Handler der Toolbox
void SfxCommonTemplateDialog_Impl::FamilySelect(USHORT nEntry)
{
    USHORT nFamily = nActFamily;
    if( nEntry != nActFamily )
    {
        CheckItem( nActFamily, FALSE );
        nActFamily = nEntry;
        SfxDispatcher* pDispat = pBindings->GetDispatcher_Impl();
        SfxUInt16Item aItem( SID_STYLE_FAMILY, nEntry );
        pDispat->Execute( SID_STYLE_FAMILY, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
        pBindings->Invalidate( SID_STYLE_FAMILY );
        pBindings->Update( SID_STYLE_FAMILY );
        UpdateFamily_Impl();
    }
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ActionSelect(USHORT nEntry)
{
    String aEmpty;
    switch(nEntry)
    {
        case SID_STYLE_WATERCAN:
        {
            const BOOL bState = IsCheckedItem(nEntry);
            BOOL bCheck;
            SfxBoolItem aBool;
            // wenn eine Vorlage ausgewaehlt ist.
            if(!bState && aFmtLb.GetSelectionCount())
            {
                const String aTemplName(
                    GetSelectedEntry());
                Execute_Impl(
                    SID_STYLE_WATERCAN, aTemplName, aEmpty,
                    (USHORT)GetFamilyItem_Impl()->GetFamily() );
                bCheck = TRUE;
            }
            else
            {
                Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
                bCheck = FALSE;
            }
            CheckItem(nEntry, bCheck);
            aBool.SetValue(bCheck);
            SetWaterCanState(&aBool);
            break;
        }
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if(pStyleSheetPool && nActFamily != 0xffff)
            {
                const SfxStyleFamily eFam=GetFamilyItem_Impl()->GetFamily();
//pStyleSheetPool->GetSearchFamily();
                const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
                USHORT nFilter;
                if(pItem&&nActFilter!=0xffff)
                {
                    nFilter = pItem->GetFilterList().GetObject(
                        nActFilter)->nFlags;
                    if(!nFilter)    // automatisch
                        nFilter = nAppFilter;
                }
                else
                    nFilter=pStyleSheetPool->GetSearchMask();
                pStyleSheetPool->SetSearchMask( eFam, SFXSTYLEBIT_USERDEF );

                SfxNewStyleDlg *pDlg =
                    // FloatingWindow must not be parent of a modal dialog
                    new SfxNewStyleDlg(SFX_APP()->GetTopWindow(), *pStyleSheetPool);
                if(RET_OK == pDlg->Execute())
                {
                    pStyleSheetPool->SetSearchMask(eFam, nFilter);
                    const String aTemplName(pDlg->GetName());
                    Execute_Impl(SID_STYLE_NEW_BY_EXAMPLE,
                                 aTemplName, aEmpty,
                                 (USHORT)GetFamilyItem_Impl()->GetFamily(),
                                 nFilter);
                }
                pStyleSheetPool->SetSearchMask( eFam, nFilter );
                delete pDlg;
            }
            break;
        }
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            Execute_Impl(SID_STYLE_UPDATE_BY_EXAMPLE,
                    aEmpty, aEmpty,
                    (USHORT)GetFamilyItem_Impl()->GetFamily());
            break;
        }
        default: DBG_ERROR("not implemented"); break;
    }
}


//-------------------------------------------------------------------------

IMPL_LINK( SfxCommonTemplateDialog_Impl, DropHdl, StyleTreeListBox_Impl *, pBox )
{
    bDontUpdate=TRUE;
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    long ret= pStyleSheetPool->SetParent(eFam,pBox->GetStyle(), pBox->GetParent())? 1L: 0L;
    bDontUpdate=FALSE;
    return ret;
}

//-------------------------------------------------------------------------

// Handler des Neu-Buttons
void SfxCommonTemplateDialog_Impl::NewHdl(void *)
{
    String aEmpty;
    if ( nActFamily != 0xffff )
    {
        Window* pTmp;
        pTmp = Application::GetDefDialogParent();
        if ( ISA(SfxTemplateDialog_Impl) )
            Application::SetDefDialogParent( pWindow->GetParent() );
        else
            Application::SetDefDialogParent( pWindow );

        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam=pItem->GetFamily();
        USHORT nMask;
        if(pItem&&nActFilter!=0xffff)
        {
            nMask = pItem->GetFilterList().GetObject(
                nActFilter)->nFlags;
            if(!nMask)    // automatisch
                nMask = nAppFilter;
        }
        else
            nMask=pStyleSheetPool->GetSearchMask();

        pStyleSheetPool->SetSearchMask(eFam,nMask);

        Execute_Impl(SID_STYLE_NEW,
                     aEmpty, GetSelectedEntry(),
                     ( USHORT )GetFamilyItem_Impl()->GetFamily(),
                     nMask);

        Application::SetDefDialogParent( pTmp );

/*              {
            DBG_ASSERT(nFilter < aFilterLb.GetEntryCount(),
                       "Filter ueberindiziert");

            if(!pTreeBox)
            {
//                              aFilterLb.SelectEntryPos(nFilter);
                FilterSelect(nActFilter, TRUE);
            }
            else
            {
                FillTreeBox();
                SfxTemplateItem *pState = pFamilyState[nActFamily-1];
                if(pState)
                {
                    const String aStyle(pState->GetStyleName());
                    SelectStyle(aStyle);
                }
                EnableDelete();
            }*/
//              }
    }
}

//-------------------------------------------------------------------------

// Handler des Bearbeiten-Buttons
void SfxCommonTemplateDialog_Impl::EditHdl(void *)
{
    if(IsInitialized() && HasSelectedStyle())
    {
        USHORT nFilter = nActFilter;
        String aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam = pItem->GetFamily();
        SfxStyleSheetBase *pStyle =
            pStyleSheetPool->Find(aTemplName,eFam,SFXSTYLEBIT_ALL);
        Window* pTmp;
        //DefModalDialogParent setzen fuer
        //Modalitaet der nachfolgenden Dialoge
        pTmp = Application::GetDefDialogParent();
        if ( ISA(SfxTemplateDialog_Impl) )
            Application::SetDefDialogParent( pWindow->GetParent() );
        else
            Application::SetDefDialogParent( pWindow );
        if ( Execute_Impl( SID_STYLE_EDIT, aTemplName, String(),
                          (USHORT)GetFamilyItem_Impl()->GetFamily(), 0, &nFilter ) )
        {
//          DBG_ASSERT(nFilter < aFilterLb.GetEntryCount(), "Filter ueberindiziert");
//          aTemplName = pStyle->GetName();
// kann durch Bearbeiten umbenannt worden sein
/*                      if(!pTreeBox)
            {
                //                              aFilterLb.SelectEntryPos(nFilter);
                //                              FilterSelect(nFilter, TRUE);
            }
            else
                FillTreeBox();*/
        }
        Application::SetDefDialogParent( pTmp );
    }
}

//-------------------------------------------------------------------------

// Handler des L"oschen-Buttons
void SfxCommonTemplateDialog_Impl::DeleteHdl(void *)
{
    if ( IsInitialized() && HasSelectedStyle() )
    {
        const String aTemplName( GetSelectedEntry() );
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
        SfxStyleSheetBase* pStyle =
            pStyleSheetPool->Find( aTemplName, pItem->GetFamily(), SFXSTYLEBIT_ALL );
        if ( pStyle )
        {
            String aMsg;
            if ( pStyle->IsUsed() )
                aMsg = String( SfxResId( STR_DELETE_STYLE_USED ) );
            aMsg += String ( SfxResId( STR_DELETE_STYLE ) );
            aMsg.SearchAndReplaceAscii( "$1", aTemplName );
#if defined UNX
            QueryBox aBox( SFX_APP()->GetTopWindow(), WB_YES_NO | WB_DEF_NO, aMsg );
#else
            QueryBox aBox( GetWindow(), WB_YES_NO | WB_DEF_NO , aMsg );
#endif
            if ( RET_YES == aBox.Execute() )
            {
                if ( pTreeBox ) // Damit die Treelistbox beim L"oschen nicht zuklappt
                {
                    bDontUpdate = TRUE;
                }
                Execute_Impl( SID_STYLE_DELETE, aTemplName,
                              String(), (USHORT)GetFamilyItem_Impl()->GetFamily() );

                if ( pTreeBox )
                {
                    pTreeBox->RemoveParentKeepChilds( pTreeBox->FirstSelected() );
                    bDontUpdate = FALSE;
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

void    SfxCommonTemplateDialog_Impl::EnableDelete()
{
    if(IsInitialized() && HasSelectedStyle())
    {
        DBG_ASSERT(pStyleSheetPool, "Kein StyleSheetPool");
        const String aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam = pItem->GetFamily();
        USHORT nFilter = pItem->GetFilterList().GetObject(nActFilter)->nFlags;
        if(!nFilter)    // automatisch
            nFilter = nAppFilter;
        const SfxStyleSheetBase *pStyle =
            pStyleSheetPool->Find(aTemplName,eFam,
                                  pTreeBox? SFXSTYLEBIT_ALL: nFilter);
        DBG_ASSERT(pStyle, "Style nicht gefunden");
        if(pStyle && pStyle->IsUserDefined())
        {
            EnableDel(TRUE);
        }
        else
        {
            EnableDel(FALSE);
        }
    }
    else
    {
        EnableDel(FALSE);
    }
//  rBindings.Invalidate( SID_STYLE_DELETE );
//  rBindings.Update( SID_STYLE_DELETE );
}

//-------------------------------------------------------------------------

// nach Selektion eines Eintrags den Focus gfs. wieder auf das App-Fenster
// setzen
void    SfxCommonTemplateDialog_Impl::ResetFocus()
{
    if(ISA(SfxTemplateDialog_Impl))
    {
        SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        SfxViewShell *pVu = pViewFrame->GetViewShell();
        Window *pAppWin = pVu ? pVu->GetWindow(): 0;
        if(pAppWin)
            pAppWin->GrabFocus();
    }
}

//-------------------------------------------------------------------------

// Doppelclick auf ein StyleSheet in der ListBox, wird angewendet.
IMPL_LINK( SfxCommonTemplateDialog_Impl, ApplyHdl, Control *, pControl )
{
        // nur, wenn dieser Bereich erlaubt ist
    if ( IsInitialized() && 0 != pFamilyState[nActFamily-1] &&
         GetSelectedEntry().Len() )
    {
        USHORT nModifier = aFmtLb.GetModifier();
        Execute_Impl(SID_STYLE_APPLY,
                     GetSelectedEntry(), String(),
                     ( USHORT )GetFamilyItem_Impl()->GetFamily(),
                     0, 0, &nModifier );
        if(ISA(SfxTemplateCatalog_Impl))
            ((SfxTemplateCatalog_Impl*) this)->pReal->EndDialog(RET_OK);
    }
//  ResetFocus();
    return 0;
}

//-------------------------------------------------------------------------

// Selektion einer Vorlage w"ahrend des Watercan-Status
IMPL_LINK( SfxCommonTemplateDialog_Impl, FmtSelectHdl, SvTreeListBox *, pListBox )
{
    // HilfePI antriggern, wenn von Call als Handler und Bereich erlaubt ist
    if( !pListBox || pListBox->IsSelected( pListBox->GetHdlEntry() ) )
    {
#ifdef WIR_KOENNEN_WIEDER_HILFE_FUER_STYLESHEETS
        SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();
        if ( pHelpPI && pListBox && IsInitialized() &&
             GetSelectedEntry().Len() )
        {
            const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
            const SfxStyleFamily eFam = pItem->GetFamily();
            DBG_ASSERT(pStyleSheetPool, "Kein Pool");
            // SfxStyleSheetBase* pStyle = pStyleSheetPool
            //      ? pStyleSheetPool->Find( GetSelectedEntry(), eFam ) : 0;
            SfxStyleSheetBase *pStyle;
            if ( pStyleSheetPool )
                pStyle = pStyleSheetPool->Find ( GetSelectedEntry(), eFam );
            else
                pStyle = 0;

            if ( pStyle )
            {
                String aHelpFile;
                ULONG nHelpId=pStyle->GetHelpId(aHelpFile);
                if ( nHelpId )
                    pHelpPI->LoadTopic( nHelpId );
            }
        }
#endif

        // nur, wenn Giesskanne an ist
        if ( IsInitialized() &&
             IsCheckedItem(SID_STYLE_WATERCAN) &&
             // nur, wenn dieser Bereich erlaubt ist
             0 != pFamilyState[nActFamily-1] )
        {
            String aEmpty;
            Execute_Impl(SID_STYLE_WATERCAN,
                         aEmpty, aEmpty, 0);
            Execute_Impl(SID_STYLE_WATERCAN,
                         GetSelectedEntry(), aEmpty,
                         ( USHORT )GetFamilyItem_Impl()->GetFamily());
        }
//      EnableEdit(TRUE);
        EnableItem(SID_STYLE_WATERCAN, !bWaterDisabled);
        EnableDelete();
    }
    if( pListBox )
        SelectStyle( pListBox->GetEntryText( pListBox->GetHdlEntry() ));

    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxCommonTemplateDialog_Impl, MenuSelectHdl, Menu *, pMenu )
{
    if( pMenu )
    {
        nLastItemId = pMenu->GetCurItemId();
        Application::PostUserEvent(
            LINK( this, SfxCommonTemplateDialog_Impl, MenuSelectHdl ), 0 );
        return TRUE;
    }

    switch(nLastItemId) {
    case ID_NEW: NewHdl(0); break;
    case ID_EDIT: EditHdl(0); break;
    case ID_DELETE: DeleteHdl(0); break;
    default: return FALSE;
    }
    return TRUE;
}

// -----------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ExecuteContextMenu_Impl( const Point& rPos, Window* pWin )
{
    if ( bBindingUpdate )
    {
        pBindings->Invalidate( SID_STYLE_NEW, TRUE, FALSE );
        pBindings->Update( SID_STYLE_NEW );
        bBindingUpdate = FALSE;
    }
    PopupMenu* pMenu = new PopupMenu( SfxResId( MN_CONTEXT_TEMPLDLG ) );
    pMenu->SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, MenuSelectHdl ) );
    pMenu->EnableItem( ID_EDIT, bCanEdit );
    pMenu->EnableItem( ID_DELETE, bCanDel );
    pMenu->EnableItem( ID_NEW, bCanNew );
    pMenu->Execute( pWin, rPos );
    delete pMenu;
}

// -----------------------------------------------------------------------

SfxStyleFamily SfxCommonTemplateDialog_Impl::GetActualFamily() const
{
    const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
    if( !pFamilyItem || nActFamily == 0xffff )
        return SFX_STYLE_FAMILY_PARA;
    else
        return pFamilyItem->GetFamily();
}

// -----------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::EnableExample_Impl(USHORT nId, BOOL bEnable)
{
    if( nId == SID_STYLE_NEW_BY_EXAMPLE )
        bNewByExampleDisabled = !bEnable;
    else if( nId == SID_STYLE_UPDATE_BY_EXAMPLE )
        bUpdateByExampleDisabled = !bEnable;
    EnableItem(nId, bEnable);
}

// ------------------------------------------------------------------------

SfxTemplateDialog_Impl::SfxTemplateDialog_Impl(
    Window* pParent, SfxBindings* pB, SfxTemplateDialog* pWindow ) :

    SfxCommonTemplateDialog_Impl( pB, pWindow ),

    aActionTbL  ( pWindow ),
    pFloat      ( pWindow ),
    aActionTbR  ( pWindow, ResId( TB_ACTION ) ),
    bZoomIn     ( FALSE )

{

    pWindow->FreeResource();
    Initialize();

    aActionTbL.SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxLSelect));
    aActionTbR.SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRSelect));
    aActionTbL.Show();
    aActionTbR.Show();
    Font aFont=aFilterLb.GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFilterLb.SetFont( aFont );
    aActionTbL.SetHelpId( HID_TEMPLDLG_TOOLBOX_LEFT );

    SFX_IMAGEMANAGER()->RegisterToolBox( &aActionTbL, SFX_TOOLBOX_CHANGEOUTSTYLE );
    SFX_IMAGEMANAGER()->RegisterToolBox( &aActionTbR, SFX_TOOLBOX_CHANGEOUTSTYLE );
}

// ------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableFamilyItem( USHORT nId, BOOL bEnable )
{
    aActionTbL.EnableItem( nId, bEnable );
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::InsertFamilyItem(USHORT nId,const SfxStyleFamilyItem *pItem)
{
    USHORT nHelpId = 0;
    switch( (USHORT) pItem->GetFamily() )
    {
        case SFX_STYLE_FAMILY_CHAR: nHelpId = SID_STYLE_FAMILY1; break;
        case SFX_STYLE_FAMILY_PARA: nHelpId = SID_STYLE_FAMILY2; break;
        case SFX_STYLE_FAMILY_FRAME:nHelpId = SID_STYLE_FAMILY3; break;
        case SFX_STYLE_FAMILY_PAGE: nHelpId = SID_STYLE_FAMILY4; break;
        case SFX_STYLE_FAMILY_PSEUDO: nHelpId = SID_STYLE_FAMILY5; break;
        default: DBG_ERROR("unbekannte StyleFamily"); break;
    }
    aActionTbL.InsertItem( nId, pItem->GetImage(), pItem->GetText(), 0, 0);
    aActionTbL.SetHelpId( nId, nHelpId );
}

// ------------------------------------------------------------------------

void SfxTemplateDialog_Impl::ClearFamilyList()
{
    aActionTbL.Clear();
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::InvalidateBindings()
{
    pBindings->Invalidate(SID_STYLE_NEW_BY_EXAMPLE, TRUE, FALSE);
    pBindings->Update( SID_STYLE_NEW_BY_EXAMPLE );
    pBindings->Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE, TRUE, FALSE);
    pBindings->Update( SID_STYLE_UPDATE_BY_EXAMPLE );
    pBindings->Invalidate( SID_STYLE_WATERCAN, TRUE, FALSE);
    pBindings->Update( SID_STYLE_WATERCAN );
    pBindings->Invalidate( SID_STYLE_NEW, TRUE, FALSE );
    pBindings->Update( SID_STYLE_NEW );
    pBindings->Invalidate( SID_STYLE_DRAGHIERARCHIE, TRUE, FALSE );
    pBindings->Update( SID_STYLE_DRAGHIERARCHIE );
}

//-------------------------------------------------------------------------

SfxTemplateDialog_Impl::~SfxTemplateDialog_Impl()
{
    SFX_IMAGEMANAGER()->ReleaseToolBox( &aActionTbL );
    SFX_IMAGEMANAGER()->ReleaseToolBox( &aActionTbR );
}

//-------------------------------------------------------------------------

// "Uberladener Resize-Handler ( StarView )
// Die Groesse der Listboxen wird angepasst
void SfxTemplateDialog_Impl::Resize()
{
    FloatingWindow *pF = pFloat->GetFloatingWindow();
    if ( pF )
    {
//      if(pF->IsZoomedIn() && bZoomIn==FALSE)
//          pF->SetText(String(SfxResId( DLG_STYLE_DESIGNER )));
//      if(!pF->IsZoomedIn() && bZoomIn==TRUE && GetFamilyItem_Impl())
//          UpdateStyles_Impl(UPDATE_FAMILY); //Bereich wieder in Titel schreiben
        bZoomIn = pF->IsRollUp();
        if ( bZoomIn )
            return;
    }

    Size aDlgSize=pFloat->PixelToLogic(pFloat->GetOutputSizePixel());
    Size aSizeATL=pFloat->PixelToLogic(aActionTbL.CalcWindowSizePixel());
    Size aSizeATR=pFloat->PixelToLogic(aActionTbR.CalcWindowSizePixel());
    Size aMinSize = GetMinOutputSizePixel();

    long nListHeight = pFloat->PixelToLogic( aFilterLb.GetSizePixel() ).Height();
    long nWidth = aDlgSize.Width()- 2 * SFX_TEMPLDLG_HFRAME;

    aActionTbL.SetPosSizePixel(pFloat->LogicToPixel(Point(SFX_TEMPLDLG_HFRAME,SFX_TEMPLDLG_VTOPFRAME)),
                                 pFloat->LogicToPixel(aSizeATL));

    // Die Position der rechten Toolbox nur ver"andern, wenn das Fenster
    // breit genug ist
    Point aPosATR(aDlgSize.Width()-SFX_TEMPLDLG_HFRAME-aSizeATR.Width(),SFX_TEMPLDLG_VTOPFRAME);
    if(aDlgSize.Width() >= aMinSize.Width())
        aActionTbR.SetPosPixel(pFloat->LogicToPixel(aPosATR));
    else
        aActionTbR.SetPosPixel( pFloat->LogicToPixel(
            Point( SFX_TEMPLDLG_HFRAME + aSizeATL.Width() + SFX_TEMPLDLG_MIDHSPACE,
                   SFX_TEMPLDLG_VTOPFRAME ) ) );

    aActionTbR.SetSizePixel(pFloat->LogicToPixel(aSizeATR));

    Point aFilterPos(
        pFloat->LogicToPixel(Point(SFX_TEMPLDLG_HFRAME,
            aDlgSize.Height()-SFX_TEMPLDLG_VBOTFRAME-nListHeight)) );

    Size aFilterSize(
        pFloat->LogicToPixel(Size(nWidth,SFX_TEMPLDLG_FILTERHEIGHT)) );

    Point aFmtPos(
        pFloat->LogicToPixel(Point(SFX_TEMPLDLG_HFRAME, SFX_TEMPLDLG_VTOPFRAME +
                            SFX_TEMPLDLG_MIDVSPACE+aSizeATL.Height())) );
    Size aFmtSize(
        pFloat->LogicToPixel(Size(nWidth,
                    aDlgSize.Height() - SFX_TEMPLDLG_VBOTFRAME -
                    SFX_TEMPLDLG_VTOPFRAME - 2*SFX_TEMPLDLG_MIDVSPACE-
                    nListHeight-aSizeATL.Height())) );

    // Die Position der Listboxen nur ver"andern, wenn das Fenster
    // hoch genug ist
    if(aDlgSize.Height() >= aMinSize.Height())
    {
        aFilterLb.SetPosPixel(aFilterPos);
        aFmtLb.SetPosPixel( aFmtPos );
        if(pTreeBox)
            pTreeBox->SetPosPixel(aFmtPos);
    }
    else
        aFmtSize.Height() += aFilterSize.Height();

    aFilterLb.SetSizePixel(aFilterSize);
    aFmtLb.SetSizePixel( aFmtSize );
    if(pTreeBox)
        pTreeBox->SetSizePixel(aFmtSize);
}

// -----------------------------------------------------------------------


Size SfxTemplateDialog_Impl::GetMinOutputSizePixel()
{
    Size aSizeATL=pFloat->PixelToLogic(aActionTbL.CalcWindowSizePixel());
    Size aSizeATR=pFloat->PixelToLogic(aActionTbR.CalcWindowSizePixel());
    Size aMinSize=Size(
        aSizeATL.Width()+aSizeATR.Width()+
        2*SFX_TEMPLDLG_HFRAME + SFX_TEMPLDLG_MIDHSPACE,
        4*aSizeATL.Height()+2*SFX_TEMPLDLG_MIDVSPACE);
    return aMinSize;
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::Command( const CommandEvent& rCEvt )
{
    if(COMMAND_CONTEXTMENU  == rCEvt.GetCommand())
        ExecuteContextMenu_Impl( rCEvt.GetMousePosPixel(), pFloat );
//      ExecuteContextMenu_Impl(pFloat->OutputToScreenPixel(rCEvt.GetMousePosPixel()));
    else
        pFloat->Command(rCEvt);
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableItem(USHORT nMesId, BOOL bCheck)
{
    String aEmpty;
    switch(nMesId)
    {
      case SID_STYLE_WATERCAN :
          if(!bCheck && IsCheckedItem(SID_STYLE_WATERCAN))
            Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
      case SID_STYLE_NEW_BY_EXAMPLE:
      case SID_STYLE_UPDATE_BY_EXAMPLE:
        aActionTbR.EnableItem(nMesId,bCheck);
        break;
    }
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::CheckItem(USHORT nMesId, BOOL bCheck)
{
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            bIsWater=bCheck;
            aActionTbR.CheckItem(SID_STYLE_WATERCAN,bCheck);
            break;
        default:
            aActionTbL.CheckItem(nMesId,bCheck); break;
    }
}

//-------------------------------------------------------------------------

BOOL SfxTemplateDialog_Impl::IsCheckedItem(USHORT nMesId)
{
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            return aActionTbR.GetItemState(SID_STYLE_WATERCAN)==STATE_CHECK;
        default:
            return aActionTbL.GetItemState(nMesId)==STATE_CHECK;
    }
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateDialog_Impl, ToolBoxLSelect, ToolBox *, pBox )
{
    const USHORT nEntry = pBox->GetCurItemId();
    FamilySelect(nEntry);
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateDialog_Impl, ToolBoxLSelect, ToolBox *, pBox )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateDialog_Impl, ToolBoxRSelect, ToolBox *, pBox )
{
    const USHORT nEntry = pBox->GetCurItemId();
    ActionSelect(nEntry);
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateDialog_Impl, ToolBoxRSelect, ToolBox *, pBox )

//-------------------------------------------------------------------------

SfxTemplateCatalog_Impl::SfxTemplateCatalog_Impl( Window* pParent, SfxBindings* pB,
                                                  SfxTemplateCatalog* pWindow ) :

    SfxCommonTemplateDialog_Impl( pB, pWindow ),

    aOkBtn      ( pWindow, SfxResId( BT_OK ) ),
    aCancelBtn  ( pWindow, SfxResId( BT_CANCEL ) ),
    aNewBtn     ( pWindow, SfxResId( BT_NEW ) ),
    aChangeBtn  ( pWindow, SfxResId( BT_EDIT ) ),
    aDelBtn     ( pWindow, SfxResId( BT_DEL ) ),
    aHelpBtn    ( pWindow, SfxResId( BT_HELP ) ),
    aOrgBtn     ( pWindow, SfxResId( BT_ORG ) ),
    aFamList    ( pWindow, SfxResId( BT_TOOL ) ),
    pReal       ( pWindow ),
    aHelper     ( pWindow )

{
    aNewBtn.Disable();
    aDelBtn.Disable();
    aChangeBtn.Disable();

    SFX_APP()->Get_Impl()->pTemplateCommon = GetISfxTemplateCommon();
    pWindow->FreeResource();

    Initialize();

    aFamList.SetSelectHdl(  LINK( this, SfxTemplateCatalog_Impl, FamListSelect ) );
    aOkBtn.SetClickHdl(     LINK( this, SfxTemplateCatalog_Impl, OkHdl ) );
    aCancelBtn.SetClickHdl( LINK( this, SfxTemplateCatalog_Impl, CancelHdl ) );
    aNewBtn.SetClickHdl(    LINK( this, SfxTemplateCatalog_Impl, NewHdl ) );
    aDelBtn.SetClickHdl(    LINK( this, SfxTemplateCatalog_Impl, DelHdl ) );
    aChangeBtn.SetClickHdl( LINK( this, SfxTemplateCatalog_Impl, ChangeHdl ) );
    aOrgBtn.SetClickHdl(    LINK( this, SfxTemplateCatalog_Impl, OrgHdl ) );
}

//-------------------------------------------------------------------------

SfxTemplateCatalog_Impl::~SfxTemplateCatalog_Impl()
{
    SFX_APP()->Get_Impl()->pTemplateCommon = 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, OkHdl, Button *, pButton )
{
    ApplyHdl( NULL );
    pReal->EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, OkHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, CancelHdl, Button *, pButton )
{
    pReal->EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, CancelHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, NewHdl, Button *, pButton )
{
    aCancelBtn.SetText( String( SfxResId( STR_CLOSE ) ) );
    SfxCommonTemplateDialog_Impl::NewHdl( NULL );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, NewHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, ChangeHdl, Button *, pButton )
{
    aCancelBtn.SetText( String( SfxResId( STR_CLOSE ) ) );
    SfxCommonTemplateDialog_Impl::EditHdl( NULL );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, ChangeHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, DelHdl, Button *, pButton )
{
    aDelBtn.Disable();
    aCancelBtn.SetText( String( SfxResId( STR_CLOSE ) ) );
    SfxCommonTemplateDialog_Impl::DeleteHdl( NULL );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, DelHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK( SfxTemplateCatalog_Impl, OrgHdl, Button *, pButton )
{
    aCancelBtn.SetText( String( SfxResId( STR_CLOSE ) ) );
    SfxDocumentTemplates aTemplates;
    aTemplates.Construct();
    SfxTemplateOrganizeDlg* pDlg = new SfxTemplateOrganizeDlg( pReal, &aTemplates );
    const short nRet = pDlg->Execute();
    delete pDlg;
    if ( RET_OK == nRet )
        Update_Impl();
    else if ( RET_EDIT_STYLE == nRet )
        pReal->EndDialog( RET_CANCEL );
    return 0;
}

//-------------------------------------------------------------------------

void SfxTemplateCatalog_Impl::EnableEdit( BOOL bEnable )
{
    SfxCommonTemplateDialog_Impl::EnableEdit( bEnable );
    aChangeBtn.Enable( bEnable );
}

//-------------------------------------------------------------------------

void SfxTemplateCatalog_Impl::EnableDel( BOOL bEnable )
{
    SfxCommonTemplateDialog_Impl::EnableDel( bEnable );
    aDelBtn.Enable( bEnable );
}

void SfxTemplateCatalog_Impl::EnableNew(BOOL bEnable)
{
    SfxCommonTemplateDialog_Impl::EnableNew( bEnable );
    aNewBtn.Enable( bEnable );
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, FamListSelect, ListBox *, pList )
{
    const USHORT nEntry = aFamIds[pList->GetSelectEntryPos()];
    FamilySelect(nEntry);
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, FamListSelect, ListBox *, pList )

//-------------------------------------------------------------------------

void SfxTemplateCatalog_Impl::EnableItem( USHORT nMesId, BOOL bCheck )
{
    if ( nMesId == SID_STYLE_WATERCAN )
        aOkBtn.Enable( bCheck );
    if ( nMesId > SFX_STYLE_FAMILY_PSEUDO || nMesId < SFX_STYLE_FAMILY_CHAR )
        return;

/*      for(USHORT i=0;i<aFamIds.Count&&aFamIds[i]!=nMesId;i++);
    if(i!=aFamIds.Count())
        aFamList.SelectEntry(aFamIds[i]);
    else
        DBG_ERROR("Entry nicht gefunden");*/

}

//-------------------------------------------------------------------------

void SfxTemplateCatalog_Impl::CheckItem(USHORT nMesId, BOOL bCheck)
{
    if ( nMesId > SFX_STYLE_FAMILY_PSEUDO || nMesId < SFX_STYLE_FAMILY_CHAR )
        return;
    USHORT i;
    for ( i = 0; i < aFamIds.Count() && aFamIds[i] != nMesId; i++ );
    aFamList.SelectEntryPos(i);
}

//-------------------------------------------------------------------------

BOOL SfxTemplateCatalog_Impl::IsCheckedItem(USHORT nMesId)
{
    if ( nMesId > SFX_STYLE_FAMILY_PSEUDO || nMesId < SFX_STYLE_FAMILY_CHAR )
        return FALSE;
    USHORT i;
    for ( i = 0; i < aFamIds.Count() && aFamIds[i] != nMesId; i++ )
        ;
    return aFamList.IsEntrySelected( String::CreateFromInt32(i) );
}

//-------------------------------------------------------------------------

// Der Katalog muss nur das Disablen beherrschen, da waehrend seiner
// Lebenszeit keine Selektionsaenderungen vorgenommen werden koennen
void SfxTemplateCatalog_Impl::EnableFamilyItem( USHORT nId, BOOL bEnable )
{
    if ( !bEnable )
        for ( USHORT nPos = aFamIds.Count(); nPos--; )
            if ( aFamIds[ nPos ] == nId )
            {
                aFamIds.Remove( nPos );
                aFamList.RemoveEntry( nPos );
            }
}

void SfxTemplateCatalog_Impl::InsertFamilyItem( USHORT nId, const SfxStyleFamilyItem* pItem )
{
    if ( nId > SFX_STYLE_FAMILY_PSEUDO || nId < SFX_STYLE_FAMILY_CHAR )
        return;
    aFamList.InsertEntry( pItem->GetText(), 0 );
    aFamIds.Insert( nId, 0 );
}

void SfxTemplateCatalog_Impl::ClearFamilyList()
{
    aFamList.Clear();
    aFamIds.Remove( 0, aFamIds.Count() );
}

void SfxCommonTemplateDialog_Impl::SetFamily( USHORT nId )
{
    if ( nId != nActFamily )
    {
        if ( nActFamily != 0xFFFF )
            CheckItem( nActFamily, FALSE );
        nActFamily = nId;
        if ( nId != 0xFFFF )
            bUpdateFamily = TRUE;
    }
}

void SfxCommonTemplateDialog_Impl::UpdateFamily_Impl()
{
    bUpdateFamily = FALSE;

    SfxDispatcher* pDispat = pBindings->GetDispatcher_Impl();
    SfxViewFrame *pViewFrame = pDispat->GetFrame();
    SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();

    SfxStyleSheetBasePool *pOldStyleSheetPool = pStyleSheetPool;
    pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): 0;
    if ( pOldStyleSheetPool != pStyleSheetPool )
    {
        if ( pOldStyleSheetPool )
            EndListening(*pOldStyleSheetPool);
        if ( pStyleSheetPool )
            StartListening(*pOldStyleSheetPool);
    }

    bWaterDisabled = FALSE;
    bCanNew = TRUE;
    bTreeDrag = TRUE;
    bUpdateByExampleDisabled = FALSE;

    if ( pStyleSheetPool )
    {
        if(!pTreeBox)
            UpdateStyles_Impl(UPDATE_FAMILY | UPDATE_FAMILY_LIST);
        else
        {
            UpdateStyles_Impl(UPDATE_FAMILY);
            FillTreeBox();
        }
    }

    InvalidateBindings();

    if ( IsCheckedItem( SID_STYLE_WATERCAN ) &&
         // nur, wenn dieser Bereich erlaubt ist
         0 != pFamilyState[ nActFamily - 1 ] )
        Execute_Impl( SID_STYLE_APPLY, GetSelectedEntry(),
                      String(), (USHORT)GetFamilyItem_Impl()->GetFamily() );
}


