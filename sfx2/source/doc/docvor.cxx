/*************************************************************************
 *
 *  $RCSfile: docvor.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:32 $
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

#ifndef _SV_PRNSETUP_HXX //autogen
#include <svtools/prnsetup.hxx>
#endif
#ifndef _VCL_CMDEVT_HXX //autogen
#include <vcl/cmdevt.hxx>
#endif
#ifndef _SV_MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _SV_PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#include <tools/urlobj.hxx>
#pragma hdrstop

#include "helpid.hrc"
#include "docvor.hxx"
#include "docfac.hxx"
#include "orgmgr.hxx"
#include "doctempl.hxx"
#include "templdlg.hxx"
#include "sfxtypes.hxx"
#include "app.hxx"
#include "dispatch.hxx"
#include "inimgr.hxx"
#include "sfxresid.hxx"
#include "iodlg.hxx"
#include "doc.hrc"
#include "sfx.hrc"
#include "docvor.hrc"
#include "docfilt.hxx"

static const char cDelim = ':';
BOOL SfxOrganizeListBox_Impl::bDropMoveOk=TRUE;

//=========================================================================


class SuspendAccel
{
public:
    Accelerator*    pAccel;

    SuspendAccel( Accelerator* pA )
    {
        pAccel=pA;
        GetpApp()->RemoveAccel( pAccel );
    }
    ~SuspendAccel()
    {
        GetpApp()->InsertAccel( pAccel );
    }
};

//=========================================================================


class SfxOrganizeDlg_Impl
{
friend class SfxTemplateOrganizeDlg;
friend class SfxOrganizeListBox_Impl;

    SuspendAccel            *pSuspend;
    SfxTemplateOrganizeDlg* pDialog;

    SfxOrganizeListBox_Impl aLeftLb;
    ListBox                 aLeftTypLb;

    SfxOrganizeListBox_Impl aRightLb;
    ListBox                 aRightTypLb;

    OKButton                aOkBtn;
    MenuButton              aEditBtn;
    HelpButton              aHelpBtn;
    PushButton              aFilesBtn;

//    FixedText               aDefaultTemplateLabel;
//    FixedInfo               aDefaultTemplate;

    Accelerator             aEditAcc;

    String                  aLastDir;
    SfxOrganizeMgr          aMgr;
    SfxOrganizeListBox_Impl*pFocusBox;
    Printer*                pPrt;

    long                    Dispatch_Impl( USHORT nId );
    String                  GetPath_Impl( BOOL bOpen, const String& rFileName );

    DECL_LINK( GetFocus_Impl, SfxOrganizeListBox_Impl * );
    DECL_LINK( LeftListBoxSelect_Impl, ListBox * );
    DECL_LINK( RightListBoxSelect_Impl, ListBox * );
    DECL_LINK( AccelSelect_Impl, Accelerator * );
    DECL_LINK( MenuSelect_Impl, Menu * );
    DECL_LINK( MenuActivate_Impl, Menu * );
    DECL_LINK( AddFiles_Impl, Button * );
    BOOL                    DontDelete_Impl( SvLBoxEntry *pEntry);
    void                    OkHdl(Button *);

public:
                            SfxOrganizeDlg_Impl( SfxTemplateOrganizeDlg* pParent,
                                                 SfxDocumentTemplates* pTempl );
};

//-------------------------------------------------------------------------

SfxOrganizeDlg_Impl::SfxOrganizeDlg_Impl( SfxTemplateOrganizeDlg* pParent,
                                          SfxDocumentTemplates* pTempl )
:   aHelpBtn( pParent, ResId( BTN_HELP ) ),
    pDialog( pParent ),
    aOkBtn( pParent, ResId( BTN_OK ) ),
    aLeftTypLb(  pParent, ResId( LB_LEFT_TYP ) ),
    aRightTypLb( pParent, ResId( LB_RIGHT_TYP ) ),
    aLeftLb( this, pParent, WB_BORDER | WB_TABSTOP | WB_HSCROLL, SfxOrganizeListBox_Impl::VIEW_TEMPLATES ),
    aRightLb( this, pParent, WB_BORDER | WB_TABSTOP | WB_HSCROLL, SfxOrganizeListBox_Impl::VIEW_FILES ),
    aFilesBtn( pParent, ResId( BTN_FILES ) ),
    aEditAcc( ResId( ACC_EDIT ) ),
    aEditBtn( pParent, ResId( BTN_EDIT ) ),
    aMgr(&aLeftLb, &aRightLb, pTempl),
    pFocusBox(0),
    pPrt(0)
{
    aLeftLb.SetHelpId( HID_CTL_ORGANIZER_LEFT );
    aRightLb.SetHelpId( HID_CTL_ORGANIZER_RIGHT );

    SfxIniManager* pIniMgr = SFX_INIMANAGER();
    String aWorkPath = pIniMgr->Get( SFX_KEY_WORK_PATH );
    if ( aWorkPath.Len() )
    {
        INetURLObject aObj( aWorkPath, INET_PROT_FILE );
        aObj.setFinalSlash();
        aLastDir = aObj.GetMainURL();
    }
    else
    {
        // fallback
        INetURLObject aObj( Application::GetAppFileName(), INET_PROT_FILE );
        aObj.removeSegment();
        aObj.setFinalSlash();
        aLastDir = aObj.GetMainURL();
    }

    // die Ordner-Bitmaps haben als Maskenfarbe rot
    Color aMaskColor( 0xFF, 0x00, 0x00 );
    Bitmap aBitmapOpenedFolder(SfxResId( BMP_OPENED_FOLDER ));
    Image aOpenedFolderBmp( aBitmapOpenedFolder, aMaskColor );
    Bitmap aBitmapClosedFolder(SfxResId(BMP_CLOSED_FOLDER));
    Image aClosedFolderBmp( aBitmapClosedFolder, aMaskColor );
    // die Dokumenten-Bitmaps haben keine Maskenfarbe
    Bitmap aBitmapOpenedDoc(SfxResId(BMP_OPENED_DOC) );
    Image aOpenedDocBmp( aBitmapOpenedDoc );
    Bitmap aBitmapClosedDoc(SfxResId( BMP_CLOSED_DOC) );
    Image aClosedDocBmp( aBitmapClosedDoc );

    aLeftLb.SetBitmaps(aOpenedFolderBmp, aClosedFolderBmp,
                       aOpenedDocBmp, aClosedDocBmp);
    aRightLb.SetBitmaps(aOpenedFolderBmp, aClosedFolderBmp,
                        aOpenedDocBmp, aClosedDocBmp);

    aEditBtn.GetPopupMenu()->SetSelectHdl( LINK( this, SfxOrganizeDlg_Impl, MenuSelect_Impl ) );
    aEditBtn.GetPopupMenu()->SetActivateHdl( LINK( this, SfxOrganizeDlg_Impl, MenuActivate_Impl ) );
    aEditAcc.SetSelectHdl( LINK( this, SfxOrganizeDlg_Impl, AccelSelect_Impl ) );
    GetpApp()->InsertAccel( &aEditAcc );

    aFilesBtn.SetClickHdl(
        LINK(this,SfxOrganizeDlg_Impl, AddFiles_Impl));
    aLeftTypLb.SetSelectHdl(
        LINK(this, SfxOrganizeDlg_Impl, LeftListBoxSelect_Impl));
    aRightTypLb.SetSelectHdl(
        LINK(this, SfxOrganizeDlg_Impl, RightListBoxSelect_Impl));
    aLeftLb.SetGetFocusHdl(
        LINK(this, SfxOrganizeDlg_Impl, GetFocus_Impl));
    aRightLb.SetGetFocusHdl(
        LINK(this, SfxOrganizeDlg_Impl, GetFocus_Impl));
    aLeftLb.SetPosSizePixel(pParent->LogicToPixel(Point(3, 6), MAP_APPFONT),
                            pParent->LogicToPixel(Size(94, 132), MAP_APPFONT));
    aRightLb.SetPosSizePixel(pParent->LogicToPixel(Point(103, 6), MAP_APPFONT),
                             pParent->LogicToPixel(Size(94, 132), MAP_APPFONT));

    Font aFont(aLeftLb.GetFont());
    aFont.SetWeight(WEIGHT_NORMAL);
    aLeftLb.SetFont(aFont);
    aRightLb.SetFont(aFont);
    const long nIndent = aLeftLb.GetIndent() / 2;
    aLeftLb.SetIndent( (short)nIndent );
    aRightLb.SetIndent( (short)nIndent );

    aLeftLb.SetMgr(&aMgr);
    aRightLb.SetMgr(&aMgr);
    aLeftLb.Reset();
    aRightLb.Reset();//SetModel(aLeftLb.GetModel());
    aLeftLb.Show();
    aRightLb.Show();
    aLeftLb.EnableInplaceEditing(TRUE);
    aRightLb.EnableInplaceEditing(TRUE);


    aRightLb.SelectAll( FALSE );
    aLeftLb.GrabFocus();
}

//=========================================================================

BOOL QueryDelete_Impl(Window *pParent,      // Parent der QueryBox
                             USHORT nId,            // Resource Id
                             const String &rTemplateName)   // Name der zu l"oschenden Vorlage
/*  [Beschreibung]

    "oschabfrage

*/
{
    SfxResId aResId( nId );
    String aEntryText( aResId );
    aEntryText.SearchAndReplaceAscii( "$1", rTemplateName );
    QueryBox aBox( pParent, WB_YES_NO | WB_DEF_NO, aEntryText );
    return RET_NO != aBox.Execute();
}

//-------------------------------------------------------------------------

void ErrorDelete_Impl(Window *pParent, const String &rName)

/*  [Beschreibung]

    Benutzerinformation, da"s die Vorlage rName nicht gel"oscht werden konnte

*/
{
    String aText( SfxResId( STR_ERROR_DELETE_TEMPLATE ) );
    aText.SearchAndReplaceAscii( "$1", rName );
    ErrorBox( pParent, WB_OK, aText ).Execute();
}


//=========================================================================

/*  [Beschreibung]

    Implementierungsklasse; Referenzklasse f"ur USHORT-Array

*/

struct ImpPath_Impl
{
    SvUShorts   aUS;
    USHORT      nRef;

    ImpPath_Impl();
    ImpPath_Impl( const ImpPath_Impl& rCopy );
};

//-------------------------------------------------------------------------

ImpPath_Impl::ImpPath_Impl() : aUS(5), nRef(1)
{
}

//-------------------------------------------------------------------------

ImpPath_Impl::ImpPath_Impl( const ImpPath_Impl& rCopy ) :

    aUS ( (BYTE)rCopy.aUS.Count() ),
    nRef( 1 )

{
    const USHORT nCount = rCopy.aUS.Count();

    for ( USHORT i = 0; i < nCount; ++i )
        aUS.Insert( rCopy.aUS[i], i );
}

//==========================================================================

/*  [Beschreibung]

    Implementierungsklasse; Darstellung einer Position in der Outline-
    Listbox als USHORT-Array; dieses beschreibt die Position jeweil
    als relative Postion zum "ubergeordneten Eintrag

*/
class Path
{
    ImpPath_Impl *pData;
    void NewImp();
public:
    Path(SvLBox *pBox, SvLBoxEntry *pEntry);
    Path(const Path &rPath):
        pData(rPath.pData)
    {
        ++pData->nRef;
    }
    const Path &operator=(const Path &rPath)
    {
        if(&rPath != this)
        {
            if(!--pData->nRef)
                delete pData;
            pData = rPath.pData;
            pData->nRef++;
        }
        return *this;
    }
    ~Path()
    {
        if(!--pData->nRef)
            delete pData;
    }
    USHORT Count() const { return pData->aUS.Count(); }
    USHORT operator[]( USHORT i ) const
    {
        return i < Count()? pData->aUS[i]: INDEX_IGNORE;
    }
};

//-------------------------------------------------------------------------

Path::Path(SvLBox *pBox, SvLBoxEntry *pEntry) :
    pData(new ImpPath_Impl)
{
    DBG_ASSERT(pEntry != 0, "EntryPtr ist NULL");
    if(!pEntry)
        return;
    SvLBoxEntry *pParent = pBox->GetParent(pEntry);
    do {
        pData->aUS.Insert(pBox->GetModel()->GetRelPos(pEntry), 0);
        if(0 == pParent)
            break;
        pEntry = pParent;
        pParent = pBox->GetParent(pEntry);
    } while(1);
}

//-------------------------------------------------------------------------

void Path::NewImp()
{
    if(pData->nRef != 1)
    {
        pData->nRef--;
        pData = new ImpPath_Impl(*pData);
    }
}

//-------------------------------------------------------------------------

void SfxOrganizeListBox_Impl::Command( const CommandEvent& rCEvt )
{
    if ( COMMAND_CONTEXTMENU  == rCEvt.GetCommand())
    {
        PopupMenu* pMenu = pDlg->aEditBtn.GetPopupMenu();
        pMenu->Execute( this, rCEvt.GetMousePosPixel() );
    }
    else
        SvTreeListBox::Command(rCEvt);
}

BOOL SfxOrganizeListBox_Impl::NotifyQueryDrop(SvLBoxEntry *pEntry)

/* [Beschreibung]

    QueryDrop-Handler (SV); Funktionsweise kann der Wahrheits,
    tabelle unten entnommen werden.


                    B       D       C1      C2      C3

        Quelle      0       1       2       3       4
    Ziel
     0              -       +***    -       -       -
     1              -       +***    [-]*    +       -
     2              -       -       [-]     p=p +** -
     3              -       -       -       p=p +** p=p && pp=pp +
     4              -       -       -       -       p=p && pp=pp +
    ----
    *   Problem Move; logische Bereiche koennen nicht geloescht werden
    **  p = Parent -> p=p: identische Parents
        pp = ParentParent
    *** geht bei Vorlagen
*/
{
    if(!pEntry)
        return FALSE;
    SvLBox *pSource = GetSourceView();
    SvLBoxEntry *pSourceEntry = pSource->FirstSelected();
    if(pEntry == pSourceEntry)
        return FALSE;
    USHORT nSourceLevel = pSource->GetModel()->GetDepth(pSourceEntry);
    if(VIEW_FILES == ((SfxOrganizeListBox_Impl *)pSource)->GetViewType())
        ++nSourceLevel;
    USHORT nTargetLevel = GetModel()->GetDepth(pEntry);
    if(VIEW_FILES == GetViewType())
        ++nTargetLevel;
    Path aSource(pSource, pSourceEntry);
    Path aTarget(this, pEntry);
    const USHORT SL = ((SfxOrganizeListBox_Impl *)pSource)->GetDocLevel();
    const USHORT TL = GetDocLevel();

    return( (nSourceLevel == 1 && nTargetLevel == 0 &&
            VIEW_TEMPLATES ==
            ((SfxOrganizeListBox_Impl *)pSource)->GetViewType()) ||
           (nSourceLevel == 1 && nTargetLevel == 1 &&
            VIEW_TEMPLATES ==
            ((SfxOrganizeListBox_Impl *)pSource)->GetViewType() &&
            VIEW_TEMPLATES == GetViewType()) ||
           (nSourceLevel == 3 && nTargetLevel == 1) ||
           (nSourceLevel == 3 && nTargetLevel == 2 &&
            aSource[1+SL] == aTarget[1+TL]) ||
           (nSourceLevel == 3 && nTargetLevel == 3 &&
            aSource[1+SL] == aTarget[1+TL]) ||
           (nSourceLevel == 4 && nTargetLevel == 3 &&
            aSource[1+SL] == aTarget[1+TL] &&
            aSource[2+SL] == aTarget[2+TL]) ||
           (nSourceLevel == 4 && nTargetLevel == 4 &&
            aSource[1+SL] == aTarget[1+TL] &&
            aSource[2+SL] == aTarget[2+TL]));
}

//-------------------------------------------------------------------------

SvLBoxEntry *GetIndices_Impl(SvLBox *pBox,
                               SvLBoxEntry *pEntry,
                               USHORT &rRegion,
                               USHORT &rOffset)
/*  [Beschreibung]

    Bereich und Position innerhalb eines Bereiches f"ur eine
    Dokumentvorlage wird ermittelt.

    [Parameter]

    SvLBox *pBox            Listbox, an der das Ereignis auftrat
    SvLBoxEntry *pEntry     Eintrag, dessen Position ermittelt werden soll
    USHORT &rRegion         der Bereich innerhalb der Bereiche der
                            Dokumentvorlagen (Out-Parameter)
    USHORT &rOffset         die Position innerhalb des Bereiches
                            Dokumentvorlagen (Out-Parameter)

    [Querverweise]

    <class Path>    (unter Umst"anden kann auf diese Funktion zugunsten
                     von Path verzichtet werden.)

*/

{
    if(!pEntry)
    {
        rRegion = rOffset = 0;
        return pEntry;
    }
    if(0 == pBox->GetModel()->GetDepth(pEntry))
    {
        rRegion = (USHORT)pBox->GetModel()->GetRelPos(pEntry);
        rOffset = USHRT_MAX;
        return pEntry;
    }
    SvLBoxEntry *pParent = pBox->GetParent(pEntry);
    rRegion = (USHORT)pBox->GetModel()->GetRelPos(pParent);
    rOffset = (USHORT)pBox->GetModel()->GetRelPos(pEntry);
    return pEntry;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::Select( SvLBoxEntry* pEntry, BOOL bSelect )
{
    if(!bSelect)
        return SvTreeListBox::Select(pEntry,bSelect);
    USHORT nLevel = GetDocLevel();
    if(GetModel()->GetDepth(pEntry)+nLevel<3)
        return SvTreeListBox::Select(pEntry,bSelect);

    Path aPath(this, pEntry);
    GetObjectShell(aPath)->TriggerHelpPI(
        aPath[nLevel+1], aPath[nLevel+2], aPath[nLevel+3]);
    return SvTreeListBox::Select(pEntry,bSelect);
}

DragDropMode SfxOrganizeListBox_Impl::NotifyBeginDrag(SvLBoxEntry *pSourceEntry)
/*  [Beschreibung]

    Was fuer DragActions sind an dieser Stelle erlaubt.
    Fuer Dokumentinhalte darf nur kopiert, nicht verschoben werden.
*/
{
    USHORT nSourceLevel = GetModel()->GetDepth(pSourceEntry);
    if(VIEW_FILES == GetViewType())
        ++nSourceLevel;
    if(nSourceLevel>=2)
    {
        bDropMoveOk=FALSE;
        return GetDragDropMode();
    }
    else
    {
        bDropMoveOk=TRUE;
        return GetDragDropMode();
    }

}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::IsStandard_Impl( SvLBoxEntry *pEntry) const
{
    String aStd(SfxResId(STR_STANDARD));
    aStd.ToUpperAscii();
    String aEntry = GetEntryText(pEntry);
    aEntry.ToUpperAscii();
    return !GetModel()->GetDepth(pEntry) &&
        aEntry.Match(aStd)>=aStd.Len();
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx,
                                            BOOL bCopy)
/*  [Beschreibung]

    Verschieben oder Kopieren von Dokumentvorlagen

    [Parameter]

    SvLBox *pSourceBox          Quell-Listbox, an der das Ereignis auftrat
    SvLBoxEntry *pSource        Quell-Eintrag, der kopiert / verschoben werden soll
    SvLBoxEntry* pTarget        Ziel-Eintrag, auf den verschoben werden soll
    SvLBoxEntry *&pNewParent    der Parent der an der Zielposition erzeugten
                                Eintrags (Out-Parameter)
    ULONG &rIdx                 Index des Zieleintrags
    BOOL bCopy                  Flag f"ur Kopieren / Verschieben


    [Returnwert]                BOOL: Erfolg oder Mi"serfolg

    [Querverweise]

    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx,
                                            BOOL bCopy)>
    <BOOL SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx)>
    <BOOL SfxOrganizeListBox_Impl::NotifyCopying(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx)>
*/

{
    BOOL bOk = FALSE;

    if(pSource)
    {
        USHORT nTargetRegion = 0, nTargetIndex = 0;
        GetIndices_Impl(this, pTarget, nTargetRegion, nTargetIndex);

        USHORT nSourceRegion = 0, nSourceIndex = 0;
        GetIndices_Impl(pSourceBox, pSource, nSourceRegion, nSourceIndex);

        bOk =  bCopy ?
            pMgr->Copy(nTargetRegion, nTargetIndex+1,
                       nSourceRegion, nSourceIndex):
            pMgr->Move(nTargetRegion, nTargetIndex+1,
                       nSourceRegion, nSourceIndex);

        if(bOk)
        {
            if(pSourceBox->GetModel()->GetDepth(pSource) == GetModel()->GetDepth(pTarget))
            {
                pNewParent = GetParent(pTarget);
                rIdx = GetModel()->GetRelPos(pTarget)+1;
            }
            else
            {
                if(nTargetIndex == USHRT_MAX)
                {
                    pNewParent = pTarget;
                    rIdx = 0;
                }
                else
                    SvLBox::NotifyCopying(
                        pTarget, pSource, pNewParent, rIdx);
            }
        }
        else
        {
            String aText( SfxResId( bCopy ? STR_ERROR_COPY_TEMPLATE : STR_ERROR_MOVE_TEMPLATE ) );
            aText.SearchAndReplaceAscii( "$1",
                                         ( (SvTreeListBox *)pSourceBox )->GetEntryText( pSource ) );
            ErrorBox( this, WB_OK, aText ).Execute();
        }
    }
    return bOk;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::MoveOrCopyContents(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx,
                                            BOOL bCopy)
/*  [Beschreibung]

    Verschieben oder Kopieren von Dokumentinhalten

    [Parameter]

    SvLBox *pSourceBox          Quell-Listbox, an der das Ereignis auftrat
    SvLBoxEntry *pSource        Quell-Eintrag, der kopiert / verschoben werden soll
    SvLBoxEntry* pTarget        Ziel-Eintrag, auf den verschoben werden soll
    SvLBoxEntry *&pNewParent    der Parent der an der Zielposition erzeugten
                                Eintrags (Out-Parameter)
    ULONG &rIdx                 Index des Zieleintrags
    BOOL bCopy                  Flag f"ur Kopieren / Verschieben


    [Returnwert]                BOOL: Erfolg oder Mi"serfolg

    [Querverweise]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx,
                                            BOOL bCopy)>
    <BOOL SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx)>
    <BOOL SfxOrganizeListBox_Impl::NotifyCopying(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx)>
*/

{
    SfxErrorContext aEc( ERRCTX_SFX_MOVEORCOPYCONTENTS, this);
    BOOL bOk = FALSE, bKeepExpansion = FALSE;
    Path aSource(pSourceBox, pSource);
    Path aTarget(this, pTarget);
    SfxObjectShellRef aSourceDoc =
        ((SfxOrganizeListBox_Impl *)pSourceBox)->GetObjectShell(aSource);

    SfxObjectShellRef aTargetDoc = GetObjectShell(aTarget);
    const USHORT nSLevel =
        ((SfxOrganizeListBox_Impl *)pSourceBox)->GetDocLevel();
    const USHORT nTLevel = GetDocLevel();

    if(aSourceDoc.Is() && aTargetDoc.Is())
    {
        if (aSourceDoc->GetStyleSheetPool())
            aSourceDoc->GetStyleSheetPool()->SetSearchMask(
                SFX_STYLE_FAMILY_ALL, SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);

        if (aTargetDoc->GetStyleSheetPool())
            aTargetDoc->GetStyleSheetPool()->SetSearchMask(
                SFX_STYLE_FAMILY_ALL, SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);
        USHORT p[3];
        USHORT nIdxDeleted = INDEX_IGNORE;
        p[0]=aTarget[nTLevel+1];
        p[1]=aTarget[nTLevel+2];
        if(p[1]!=INDEX_IGNORE)p[1]++;
        p[2]=aTarget[nTLevel+3];

        bOk = aTargetDoc->Insert(
            *aSourceDoc, aSource[nSLevel+1],
            aSource[nSLevel+2], aSource[nSLevel+3],
            p[0], p[1], p[2],  nIdxDeleted);
        // Positionskorrektur auswerten
        // a = Dokumentinhalt
        // b = Position Sub-Inhalt 1
        // c = Position Sub-Inhalt 2
        // doppelte Eintraege loeschen
        if(bOk)
        {
            SvLBoxEntry *pParentIter = pTarget;
            // bis auf die DokumentEbene nach oben als
            // allgemeiner Bezugspunkt
            while(GetModel()->GetDepth(pParentIter) != nTLevel)
                pParentIter = GetParent(pParentIter);
            if(pParentIter->HasChildsOnDemand() &&
                !GetModel()->HasChilds(pParentIter))
                RequestingChilds(pParentIter);
            SvLBoxEntry *pChildIter = 0;

            USHORT i = 0;
            while(i < 2 && p[i+1] != INDEX_IGNORE)
            {
                pChildIter = FirstChild(pParentIter);
                // bis zum Index der aktuellen Ebene
                for(USHORT j = 0; j < p[i]; ++j)
                    pChildIter = NextSibling(pChildIter);
                // gfs Fuellen bei Items onDemand
                ++i;
                if(p[i+1] != INDEX_IGNORE &&
                   pChildIter->HasChildsOnDemand() &&
                   !GetModel()->HasChilds(pChildIter))
                    RequestingChilds(pChildIter);
                pParentIter = pChildIter;
            }
            rIdx = p[i];
            pNewParent = pParentIter;
            if(!IsExpanded(pNewParent) &&
               pNewParent->HasChildsOnDemand() &&
               !GetModel()->HasChilds(pNewParent))
            {
                bOk = FALSE;
                if(!bCopy)
                    pSourceBox->GetModel()->Remove(pSource);
            }
            // Geloeschte Eintraege entfernen
            // (kann durch Ueberschreiben geschehen)
            if(nIdxDeleted != INDEX_IGNORE)
            {
                pChildIter = FirstChild(pParentIter);
                for(USHORT i = 0; i < nIdxDeleted; ++i)
                    pChildIter = NextSibling(pChildIter);
                if( pChildIter && pChildIter != pSource )
                {
                    bKeepExpansion = IsExpanded(pParentIter);
                    GetModel()->Remove(pChildIter);
                }
                else
                    bOk = FALSE;
            }
            if(!bCopy && &aSourceDoc != &aTargetDoc)
                    aSourceDoc->Remove(aSource[nSLevel+1],
                                     aSource[nSLevel+2],
                                     aSource[nSLevel+3]);
        }
    }
//  rIdx++;
    return (rIdx != INDEX_IGNORE && bOk )
        ? bKeepExpansion? (BOOL)2: TRUE: FALSE;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                        SvLBoxEntry* pSource,
                                        SvLBoxEntry *&pNewParent,
                                        ULONG &rIdx)

/*  [Beschreibung]

    Benachrichtigung, da"s ein Eintrag verschoben werden soll
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pTarget        Ziel-Eintrag, auf den verschoben werden soll
    SvLBoxEntry *pSource        Quell-Eintrag, der verschoben werden soll
    SvLBoxEntry *&pNewParent    der Parent der an der Zielposition erzeugten
                                Eintrags (Out-Parameter)
    ULONG &rIdx                 Index des Zieleintrags


    [Returnwert]                BOOL: Erfolg oder Mi"serfolg

    [Querverweise]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx,
                                            BOOL bCopy)>
    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx,
                                            BOOL bCopy)>
    <BOOL SfxOrganizeListBox_Impl::NotifyCopying(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx)>
*/

{
    BOOL bOk =  FALSE;
    SvLBox *pSourceBox = GetSourceView();
    if(pSourceBox->GetModel()->GetDepth(pSource) <= GetDocLevel() &&
        GetModel()->GetDepth(pTarget) <= GetDocLevel())
        bOk = MoveOrCopyTemplates(pSourceBox, pSource, pTarget,
                                  pNewParent, rIdx, FALSE);
    else
        bOk = MoveOrCopyContents(pSourceBox, pSource, pTarget,
                                  pNewParent, rIdx, FALSE);
    return bOk;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::NotifyCopying(SvLBoxEntry *pTarget,
                                        SvLBoxEntry* pSource,
                                        SvLBoxEntry *&pNewParent,
                                        ULONG &rIdx)
/*  [Beschreibung]

    Benachrichtigung, da"s ein Eintrag kopiert werden soll
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pTarget        Ziel-Eintrag, auf den kopiert werden soll
    SvLBoxEntry *pSource        Quell-Eintrag, der kopiert werden soll
    SvLBoxEntry *&pNewParent    der Parent der an der Zielposition erzeugten
                                Eintrags (Out-Parameter)
    ULONG &rIdx                 Index des Zieleintrags


    [Returnwert]                BOOL: Erfolg oder Mi"serfolg

    [Querverweise]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx,
                                            BOOL bCopy)>
    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx,
                                            BOOL bCopy)>
    <BOOL SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            ULONG &rIdx)>
*/
{
    BOOL bOk =  FALSE;
    SvLBox *pSourceBox = GetSourceView();
    if(pSourceBox->GetModel()->GetDepth(pSource) <= GetDocLevel() &&
        GetModel()->GetDepth(pTarget) <= GetDocLevel())
        bOk = MoveOrCopyTemplates(pSourceBox, pSource, pTarget,
                                  pNewParent, rIdx, TRUE);
    else
        bOk = MoveOrCopyContents(pSourceBox, pSource, pTarget,
                                  pNewParent, rIdx, TRUE );
    return bOk;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::EditingEntry( SvLBoxEntry* pEntry, Selection&  )

/*  [Beschreibung]

    Nachfrage, ob ein Eintrag editierbar ist
    (SV-Handler)

    [Querverweise]
    <SfxOrganizeListBox_Impl::EditedEntry(SvLBoxEntry* pEntry, const String& rText)>
*/

{
    if( VIEW_TEMPLATES == eViewType &&
        GetModel()->GetDepth(pEntry) < 2 &&  !IsStandard_Impl(pEntry))
    {
        pDlg->pSuspend=new SuspendAccel(&pDlg->aEditAcc);
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::EditedEntry(SvLBoxEntry* pEntry, const String& rText)

/*  [Beschreibung]

    Der Name eines Eintrags wurde bearbeitet; ist der eingegebene Name
    ein g"ultiger Name ("ange > 0), wird das Model aktualisiert.
    (SV-Handler)

    [Returnwert]

    BOOL                TRUE: der Name soll in der Anzeige ge"andert werden
                        FALSE:der Name soll nicht ge"andert werden

    [Querverweise]
    <SfxOrganizeListBox_Impl::EditingEntry(SvLBoxEntry* pEntry, const String& rText)>
*/

{
    DBG_ASSERT(pEntry, "kein Entry selektiert");
    delete pDlg->pSuspend;
    pDlg->pSuspend=0;
    SvLBoxEntry* pParent = GetParent(pEntry);
    if( !rText.Len() )
    {
        ErrorBox aBox( this, SfxResId( MSG_ERROR_EMPTY_NAME ) );
        aBox.GrabFocus();
        aBox.Execute();
        return FALSE;
    }
    if ( !IsUniqName_Impl( rText, pParent, pEntry ) )
    {
        ErrorBox aBox( this, SfxResId( MSG_ERROR_UNIQ_NAME ) );
        aBox.GrabFocus();
        aBox.Execute();
        return FALSE;
    }
    USHORT nRegion = 0, nIndex = 0;
    GetIndices_Impl( this, pEntry, nRegion, nIndex );
    String aOldName;
    if ( USHRT_MAX != nIndex )
        aOldName = pMgr->GetTemplates()->GetName( nRegion, nIndex );
    else
        aOldName = pMgr->GetTemplates()->GetRegionName( nRegion );

    if ( !pMgr->SetName( rText, nRegion, nIndex ) )
    {
        SfxResId aResId( USHRT_MAX != nIndex ? MSG_ERROR_RENAME_TEMPLATE
                                             : MSG_ERROR_RENAME_TEMPLATE_REGION );
        ErrorBox( this, aResId ).Execute();
        return FALSE;
    }
    else
    {
        SfxTemplateOrganizeDlg* pDlg = (SfxTemplateOrganizeDlg*)Window::GetParent();
    }
    return TRUE;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::Drop( DropEvent& rEvt )

/*  [Beschreibung]

    Drop Handler; wird verwendet, um in der Dokumentenansicht weitere
    Dokumente per Drag&Drop hinzuf"ugen zu k"onnen
    (SV-Handler)

    [Returnwert]

    BOOL                Erfolg oder Mi"serfolg

    [Querverweise]
    <SfxOrganizeListBox_Impl::QueryDrop(DropEvent& rEvt)>
*/

{
    const USHORT nCount = DragServer::GetItemCount();
    BOOL bSuccess = FALSE;
    for ( USHORT i = 0; i < nCount; ++i )
    {
        const String aFileName( DragServer::PasteFile(i) );
        if ( !aFileName.Len() )
            continue;
        INetURLObject aObj( aFileName, INET_PROT_FILE );
        bSuccess |= pMgr->InsertFile( this, aObj.GetMainURL() );
    }
    bDropMoveOk = TRUE;
    return bSuccess ? bSuccess : SvTreeListBox::Drop( rEvt );
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::QueryDrop( DropEvent& rEvt )

/*  [Beschreibung]

    QueryDrop Handler; wird verwendet, um in der Dokumentenansicht weitere
    Dokumente per Drag&Drop hinzuf"ugen zu k"onnen
    (SV-Handler)

    [Returnwert]

    BOOL                Erfolg oder Mi"serfolg

    [Querverweise]
    <SfxOrganizeListBox_Impl::Drop(DropEvent& rEvt)>
*/

{

/*  if(rEvt.GetAction()==DROP_MOVE && !bDropMoveOk)
        return FALSE;*/
    if( rEvt.IsDefaultAction() )
        rEvt.SetAction( DROP_COPY );
    return eViewType == VIEW_FILES && DragServer::HasFormat(0, FORMAT_FILE)
        ? TRUE : SvTreeListBox::QueryDrop(rEvt);
}

//-------------------------------------------------------------------------

inline USHORT SfxOrganizeListBox_Impl::GetDocLevel() const

/*  [Beschreibung]

    Ermittelt, auf welche Ebene sich Dokumente befinden (unterschiedlich
    in der Dokumentvorlagensicht und der Dokumentensicht)

    [Returnwert]

    USHORT              Die Ebene der Dokumente

*/

{
    return eViewType == VIEW_FILES? 0: 1;
}

//-------------------------------------------------------------------------

SfxObjectShellRef SfxOrganizeListBox_Impl::GetObjectShell(const Path &rPath)

/*  [Beschreibung]

    Zugriff auf die ObjectShell, die dem aktuellen Eintrag zugeordnet
    ist.

    [Parameter]

    const Path &rPath       Beschreibung des aktuellen Eintrags

    [Returnwert]

    SfxObjectShellRef     Referenz auf die ObjectShell

    [Querverweise]

    <class Path>

*/

{
    SfxObjectShellRef aDoc;
    if(eViewType == VIEW_FILES)
        aDoc = pMgr->CreateObjectShell(rPath[0]);
    else
        aDoc = pMgr->CreateObjectShell(rPath[0], rPath[1]);
    return aDoc;
}

//-------------------------------------------------------------------------

void SfxOrganizeListBox_Impl::RequestingChilds( SvLBoxEntry* pEntry )

/*  [Beschreibung]

    Aufforderung, der Childs eines Eintrags einzuf"ugen
    ist.
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pEntry     der Eintrag, dessen Childs erfragt werden


*/

{
    // wenn keine Childs vorhanden sind, gfs. Childs
    // einfuegen
    if(!GetModel()->HasChilds(pEntry))
    {
        // hier sind alle initial eingefuegt
        SfxErrorContext aEc(ERRCTX_SFX_CREATEOBJSH, pDlg->pDialog);
        if(VIEW_TEMPLATES == GetViewType() && 0 == GetModel()->GetDepth(pEntry))
        {
            USHORT i = (USHORT)GetModel()->GetRelPos(pEntry);
            const USHORT nEntryCount = pMgr->GetTemplates()->GetCount(i);
            for(USHORT j = 0; j < nEntryCount; ++j)
                InsertEntry( pMgr->GetTemplates()->GetName( i, j ),
                             aOpenedDocBmp, aClosedDocBmp, pEntry, TRUE );
        }
        else
        {
            const USHORT nDocLevel = GetDocLevel();
            Path aPath(this, pEntry);
            SfxObjectShellRef aRef = GetObjectShell(aPath);
            if(aRef.Is())
            {
                const USHORT nCount = aRef->GetContentCount(
                    aPath[nDocLevel+1], aPath[nDocLevel+2]);
                String aText;
                Bitmap aClosedBmp, aOpenedBmp;
                const BOOL bCanHaveChilds =
                    aRef->CanHaveChilds(aPath[nDocLevel+1],
                                        aPath[nDocLevel+2]);
                for(USHORT i = 0; i < nCount; ++i)
                {
                    BOOL bDeletable;
                    aRef->GetContent(
                        aText, aClosedBmp, aOpenedBmp, bDeletable,
                        i, aPath[nDocLevel+1], aPath[nDocLevel+2]);
                    SvLBoxEntry *pNew=InsertEntry(
                        aText, aOpenedBmp, aClosedBmp,
                        pEntry, bCanHaveChilds);
                    pNew->SetUserData(bDeletable ? &bDeletable : 0);
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

long SfxOrganizeListBox_Impl::ExpandingHdl()

/*  [Beschreibung]

    SV-Handler, der nach dem und vor dem Aufklappen eines Eintrags
    gerufen wird.
    Wird verwendet, um gfs. die ObjectShell wieder zu schlie"sen;
    die Eintr"age mit den Inhalten dieser Shell werden ebenfalls
    entfernt.

*/

{
    if ( !(nImpFlags & SVLBOX_IS_EXPANDING) )
    {
        SvLBoxEntry* pEntry  = GetHdlEntry();
        const USHORT nLevel = GetModel()->GetDepth(pEntry);
        if((eViewType == VIEW_FILES && nLevel == 0) ||
           (eViewType == VIEW_TEMPLATES && nLevel == 1))
        {
            Path aPath(this, pEntry);
            // Beim Schliessen des Files die ObjectShell freigeben
            if(eViewType == VIEW_FILES && nLevel == 0)
                pMgr->DeleteObjectShell(aPath[0]);
            else
                pMgr->DeleteObjectShell(aPath[0], aPath[1]);
            // alle SubEntries loeschen
            SvLBoxEntry *pToDel = SvLBox::GetEntry(pEntry, 0);
            while(pToDel)
            {
                GetModel()->Remove(pToDel);
                pToDel = SvLBox::GetEntry(pEntry, 0);
            }
        }
    }
    return TRUE;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeListBox_Impl::IsUniqName_Impl(const String &rText,
                                         SvLBoxEntry* pParent, SvLBoxEntry *pEntry) const

/*  [Beschreibung]

    Pr"uft, ob eine Name auf seiner Ebene eindeutig ist.

    [Parameter]

    const String &         Name des zu suchenden Eintrags
    SvLBoxEntry* pSibling  Geschwister (bezeichnet die Ebene)

    [Returnwert]

    BOOL                     TRUE, wenn der Name eindeutig ist, sonst FALSE
*/

{
    SvLBoxEntry* pChild = FirstChild(pParent);
    while(pChild)  {
        const String aEntryText(GetEntryText(pChild));
        if(COMPARE_EQUAL == aEntryText.CompareIgnoreCaseToAscii(rText)&&(!pEntry || pEntry!=pChild))
            return FALSE;
        pChild = NextSibling(pChild);
    }
    return TRUE;
}

//-------------------------------------------------------------------------

USHORT SfxOrganizeListBox_Impl::GetLevelCount_Impl(SvLBoxEntry* pParent) const
{
    SvLBoxEntry* pChild = FirstChild(pParent);
    USHORT nCount = 0;
    while(pChild)  {
        pChild = NextSibling(pChild);
        ++nCount;
    }
    return nCount;
}

//-------------------------------------------------------------------------

SfxOrganizeListBox_Impl::SfxOrganizeListBox_Impl(
    SfxOrganizeDlg_Impl *pArgDlg, Window *pParent,
    WinBits nBits, DataEnum eType)
:   SvTreeListBox(pParent, nBits), pMgr(0), eViewType(eType),
    pDlg(pArgDlg)

/*  [Beschreibung]

    Konstruktor SfxOrganizeListBox

*/

{
    SetDragDropMode( SV_DRAGDROP_CTRL_MOVE |
                     SV_DRAGDROP_CTRL_COPY |
                     SV_DRAGDROP_APP_MOVE  |
                     SV_DRAGDROP_APP_COPY  |
                     SV_DRAGDROP_APP_DROP);
//  SetDragOptions(DROP_COPY);
    SetEntryHeight( 16 );
    SetSelectionMode(SINGLE_SELECTION);
    EnableDrop();
    GetModel()->SetSortMode(SortNone);      // Bug in SvTools 303
}

//-------------------------------------------------------------------------

void SfxOrganizeListBox_Impl::Reset()

/*  [Beschreibung]

    Einf"ugen der Elemente in die ListBox

*/

{
    DBG_ASSERT( pMgr != 0, "kein Manager" );
    // Inhalte l"oschen
    SetUpdateMode(FALSE);
    Clear();
    if ( VIEW_TEMPLATES == eViewType )
    {
        const USHORT nCount = pMgr->GetTemplates()->GetRegionCount();
        for ( USHORT i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = InsertEntry( pMgr->GetTemplates()->GetFullRegionName(i),
                                               aOpenedFolderBmp, aClosedFolderBmp, 0, TRUE );
        }
    }
    else
    {
        const SfxObjectList& rList = pMgr->GetObjectList();
        const USHORT nCount = rList.Count();
        for ( USHORT i = 0; i < nCount; ++i )
            InsertEntry( rList.GetBaseName(i), aOpenedDocBmp, aClosedDocBmp, 0, TRUE );

    }
    SetUpdateMode(TRUE);
    Invalidate();
    Update();
}

//-------------------------------------------------------------------------

const Image &SfxOrganizeListBox_Impl::GetClosedBmp(USHORT nLevel) const

/*  [Beschreibung]

    Zugriff auf die Bitmap f"ur einen geschlossenen Eintrag
    der jeweiligen Ebene

    [Parameter]

    USHORT nLevel       Angabe der Ebene, 2 Ebenen sind erlaubt

    [Returnwert]

    const Image &       das Image auf der Ebenen nLevel

*/

{
    switch(nLevel) {
    case 0: return aClosedFolderBmp;
    case 1: return aClosedDocBmp;
    }
    DBG_ERROR("Bitmaps ueberindiziert");
    return aClosedFolderBmp;
}

//-------------------------------------------------------------------------

const Image &SfxOrganizeListBox_Impl::GetOpenedBmp(USHORT nLevel) const

/*  [Beschreibung]

    Zugriff auf die Bitmap f"ur einen ge"offneten Eintrag
    der jeweiligen Ebene

    [Parameter]

    USHORT nLevel       Angabe der Ebene, 2 Ebenen sind erlaubt

    [Returnwert]

    const Image &       das Image auf der Ebenen nLevel

*/

{
    switch(nLevel)
    {
      case 0:
        return aOpenedFolderBmp;
      case 1:
        return aOpenedDocBmp;
    }
    DBG_ERROR("Bitmaps ueberindiziert");
    return aClosedFolderBmp;
}

//-------------------------------------------------------------------------

String SfxOrganizeDlg_Impl::GetPath_Impl( BOOL bOpen, const String& rFileName )

/*  [Beschreibung]

    Pfad per FileDialog erfragen, f"ur Import / Export von
    Dokumentvorlagen

    [Parameter]

    BOOL bOpen                      Flag: "Offnen / Speichern
    const String& rFileName         aktueller Dateiname als Vorschlag

    [R"uckgabewert]                 Dateiname mit Pfad oder Leerstring, wenn
                                    der Benutzer 'Abbrechen' gedr"uckt hat
*/

{
    String aPath;
    ULONG nBits = bOpen ? WB_OPEN | WB_3DLOOK : WB_SAVEAS | WB_3DLOOK;
    SfxSimpleFileDialog* pFileDlg = new SfxSimpleFileDialog( pDialog, nBits );
    pFileDlg->SetDefaultExt( DEFINE_CONST_UNICODE( "vor" ) );
    pFileDlg->AddFilter( String( SfxResId( STR_FILTERNAME_ALL ) ), DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );
    const String aFilter( SfxResId( STR_TEMPLATE_FILTER ) );
    pFileDlg->AddFilter( aFilter, DEFINE_CONST_UNICODE( "*.vor" ) );
    pFileDlg->SetCurFilter( aFilter );
    if ( aLastDir.Len() || rFileName.Len() )
    {
        INetURLObject aObj;
        aObj.SetSmartProtocol( INET_PROT_FILE );
        if ( aLastDir.Len() )
        {
            aObj.SetSmartURL( aLastDir );
            if ( rFileName.Len() )
                aObj.insertName( rFileName );
        }
        else
            aObj.SetSmartURL( rFileName );
        pFileDlg->SetPath( aObj.GetMainURL() );
    }
    if ( RET_OK == pFileDlg->Execute() )
    {
        aPath = pFileDlg->GetPath();
        INetURLObject aObj( aPath );
        aObj.removeSegment();
        aLastDir = aObj.GetMainURL();
    }
    delete pFileDlg;
    return aPath;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeDlg_Impl::DontDelete_Impl( SvLBoxEntry *pEntry)
{
    USHORT nDepth = pFocusBox->GetModel()->GetDepth(pEntry);
    if(SfxOrganizeListBox_Impl::VIEW_FILES ==
       pFocusBox->GetViewType())
        nDepth++;
    if( nDepth > 2 && !pEntry->GetUserData() ||
       //Delete ueber GetContent verboten
       pFocusBox->IsStandard_Impl(pEntry) ||
       //StandardVorlage nicht loeschen
       nDepth==2 || //Vorlage / Konfigurtionsrubrik nicht loeshcen
       (nDepth==1 && SfxOrganizeListBox_Impl::VIEW_FILES ==
        pFocusBox->GetViewType()) || //Files nicht loeschen
       (0 == nDepth && pFocusBox->GetLevelCount_Impl(0) < 2))
        //Mindestens eine Vorlage behalten
        return TRUE;
    else
        return FALSE;
}

long SfxOrganizeDlg_Impl::Dispatch_Impl(USHORT nId)

/*  [Beschreibung]

    Verarbeiten der Events aus MenuButton oder Accelerator

    [Parameter]

    USHORT nId                      ID des Events

    [R"uckgabewert]                 1: Event wurde verarbeitet,
                                    0: Event wurde nicht verarbeitet (SV-Menu)

*/

{
    SuspendAccel aTmp(&aEditAcc);
    SvLBoxEntry *pEntry = pFocusBox? pFocusBox->FirstSelected(): 0;
    switch(nId)
    {
        case ID_NEW:
        {
            if(!pEntry)
                return 1;
            if(pFocusBox->GetViewType() == SfxOrganizeListBox_Impl::VIEW_TEMPLATES)
            {
                if(0 == pFocusBox->GetModel()->GetDepth(pEntry))
                {
                    const String aNoName( SfxResId(STR_NONAME) );
                    SvLBoxEntry* pParent = pFocusBox->GetParent(pEntry);
                    String aName(aNoName);
                    USHORT n = 1;
                    while(!pFocusBox->IsUniqName_Impl(aName, pParent))
                    {
                        aName = aNoName;
                        aName += String::CreateFromInt32( n++ );
                    }
                    aMgr.InsertDir( pFocusBox, aName,
                            (USHORT)pFocusBox->GetModel()->GetRelPos(pEntry)+1);
                }
            }
            break;
        }

        case ID_DELETE:
        {
            if(!pEntry || DontDelete_Impl(pEntry))
                return 1;
            const USHORT nDepth = pFocusBox->GetModel()->GetDepth(pEntry);
            if(nDepth < 2)
            {
                if(0 == nDepth && pFocusBox->GetLevelCount_Impl(0) < 2) return 1;
                if(SfxOrganizeListBox_Impl::VIEW_TEMPLATES ==
                        pFocusBox->GetViewType())
                {
                    USHORT nResId = nDepth? STR_DELETE_TEMPLATE :
                                            STR_DELETE_REGION;
                    String aStd(SfxResId(STR_STANDARD));
                    aStd.ToUpperAscii();
                    String aStdText(pFocusBox->GetEntryText(pEntry));
                    aStdText.ToUpperAscii();
                    if(!nDepth && aStdText.Match(aStd)>=aStd.Len())
                        return 1;
                    if( !QueryDelete_Impl(
                        pDialog, nResId, pFocusBox->GetEntryText(pEntry)))
                        return 1;
                    if ( STR_DELETE_REGION == nResId &&
                         pFocusBox->GetChildCount(pEntry))
                    {
                        QueryBox aQBox(pDialog, SfxResId(MSG_REGION_NOTEMPTY));
                        if(RET_NO == aQBox.Execute())
                            return 1;
                    }
                    USHORT nRegion = 0, nIndex = 0;
                    GetIndices_Impl(pFocusBox, pEntry, nRegion, nIndex);
                    BOOL bResetDef=FALSE;

                    if ( !aMgr.Delete(
                        pFocusBox, nRegion,
                        STR_DELETE_REGION == nResId? USHRT_MAX: nIndex) )
                        ErrorDelete_Impl(
                            pDialog, pFocusBox->GetEntryText(pEntry));

                }
            }
            // Inhaltsformen
            else if(nDepth + pFocusBox->GetDocLevel() >= 2)
            {
                String aStd(SfxResId(STR_STANDARD));
                aStd.ToUpperAscii();
                String aStdText(pFocusBox->GetEntryText(pEntry));
                aStdText.ToUpperAscii();
                if(!nDepth && aStdText.Match(aStd)>=aStd.Len())
                    return 1;
                if(!QueryDelete_Impl(pDialog, STR_DELETE_TEMPLATE, pFocusBox->GetEntryText(pEntry)))
                    return 1;
                Path aPath(pFocusBox, pEntry);
                SfxObjectShellRef aRef = pFocusBox->GetObjectShell(aPath);
                if(aRef.Is() &&
                    aRef->Remove(aPath[1+pFocusBox->GetDocLevel()],
                                 aPath[2+pFocusBox->GetDocLevel()],
                                 aPath[3+pFocusBox->GetDocLevel()]))
                    pFocusBox->GetModel()->Remove(pEntry);
                else
                    ErrorDelete_Impl(pDialog, pFocusBox->GetEntryText(pEntry));
            }
            break;
        }

        case ID_EDIT:
        {
            if(!pEntry)
                return 1;
            USHORT nRegion = 0, nIndex = 0;
            GetIndices_Impl( pFocusBox, pEntry, nRegion, nIndex );
            const SfxStringItem aName( SID_FILE_NAME, aMgr.GetTemplates()->GetPath( nRegion, nIndex ) );
            const SfxStringItem aLongName( SID_FILE_LONGNAME, pFocusBox->GetEntryText( pEntry ) );
            const SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE( "private:user" ) );

            SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_OPENTEMPLATE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                      &aName, &aLongName, &aReferer, 0L );
            pDialog->EndDialog( RET_EDIT_STYLE );
            break;
        }

        case ID_COPY_FROM:
        {
            if ( !pEntry )
                return 1;
            USHORT nRegion = 0, nIndex = 0;
            GetIndices_Impl( pFocusBox, pEntry, nRegion, nIndex );
            String aPath = GetPath_Impl( TRUE, String() );

            if ( aPath.Len() && !aMgr.CopyFrom( pFocusBox, nRegion, nIndex, aPath ) )
            {
                String aText( SfxResId( STR_ERROR_COPY_TEMPLATE ) );
                aText.SearchAndReplaceAscii( "$1", aPath );
                ErrorBox( pDialog, WB_OK, aText ).Execute();
            }
            break;
        }

        case ID_COPY_TO:
        {
            if ( !pEntry )
                return 1;
            USHORT nRegion = 0, nIndex = 0;
            GetIndices_Impl( pFocusBox, pEntry, nRegion, nIndex );
            String aPath = GetPath_Impl( FALSE, aMgr.GetTemplates()->GetFileName( nRegion, nIndex ) );

            if ( aPath.Len() && !aMgr.CopyTo( nRegion, nIndex, aPath ) )
            {
                String aText( SfxResId( STR_ERROR_COPY_TEMPLATE ) );
                aText.SearchAndReplaceAscii( "$1", aPath );
                ErrorBox( pDialog, WB_OK, aText ).Execute();
            }
            break;
        }

        case ID_RESCAN:
            if ( !aMgr.Rescan() )
                ErrorBox( pDialog, SfxResId( MSG_ERROR_RESCAN ) ).Execute();
            if ( SfxOrganizeListBox_Impl::VIEW_TEMPLATES == aLeftLb.GetViewType() )
                aLeftLb.Reset();
            if ( SfxOrganizeListBox_Impl::VIEW_TEMPLATES == aRightLb.GetViewType() )
                aRightLb.Reset();
            break;

        case ID_PRINT:
        {
            if ( !pEntry )
                return 1;
            Path aPath( pFocusBox, pEntry );
            SfxObjectShellRef aRef = pFocusBox->GetObjectShell( aPath );
            if ( aRef.Is() )
            {
                const USHORT nDocLevel = pFocusBox->GetDocLevel();
                if ( !pPrt )
                    pPrt = new Printer;
                SvLBoxEntry *pDocEntry = pEntry;
                while ( pFocusBox->GetModel()->GetDepth( pDocEntry ) > nDocLevel )
                    pDocEntry = pFocusBox->GetParent( pDocEntry );
                const String aName(pFocusBox->GetEntryText(pDocEntry));
                if ( !aRef->Print( *pPrt, aPath[1+nDocLevel],
                                   aPath[2+nDocLevel], aPath[3+nDocLevel], &aName ) )
                    ErrorBox( pDialog, SfxResId( MSG_PRINT_ERROR ) ).Execute();
            }
            break;
        }

        case ID_PRINTER_SETUP:
        {
            PrinterSetupDialog* pDlg = new PrinterSetupDialog( pDialog );
            if ( !pPrt )
                pPrt = new Printer;
            pDlg->SetPrinter( pPrt );
            pDlg->Execute();
            delete pDlg;
#ifdef MAC
            // bei bestimmten Druckertreibern gibt es Probleme mit
            // dem Repaint. Deshalb hier explizit Invalidate() rufen
            pDialog->Invalidate();
#endif
            break;
        }

        default: return 0;
    }
    return 1;

}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxOrganizeDlg_Impl, MenuSelect_Impl, Menu *, pMenu )

/*  [Beschreibung]

    SelectHandler des Men"us des Men"ubuttons (SV)

    [Parameter]

    MenuButton *pBtn                der das Event ausl"osende Button

    [R"uckgabewert]                 1: Event wurde verarbeitet,
                                    0: Event wurde nicht verarbeitet (SV-Menu)

*/
{
    return Dispatch_Impl(pMenu->GetCurItemId());
}
IMPL_LINK_INLINE_END( SfxOrganizeDlg_Impl, MenuSelect_Impl, Menu *, pMenu )

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, AccelSelect_Impl, Accelerator *, pAccel )

/*  [Beschreibung]

    SelectHandler des Accelerators (SV)

    [Parameter]

    Accelerator *pAccel             der das Event ausl"osende Accelerator

    [R"uckgabewert]                 1: Event wurde verarbeitet,
                                    0: Event wurde nicht verarbeitet (SV)

*/
{
    SvLBoxEntry *pEntry=pFocusBox && pFocusBox->GetSelectionCount()
        ? pFocusBox->FirstSelected() : 0 ;
    return pEntry && (pAccel->GetCurItemId() == ID_NEW  ||
                      !DontDelete_Impl(pEntry)) ?
        Dispatch_Impl(pAccel->GetCurItemId()): 0;
    return 0;
}

//-------------------------------------------------------------------------

void SfxOrganizeDlg_Impl::OkHdl(Button *pButton)
{
    if(pFocusBox && pFocusBox->IsEditingActive())
        pFocusBox->EndEditing(FALSE);
    pButton->Click();
}



IMPL_LINK( SfxOrganizeDlg_Impl, MenuActivate_Impl, Menu *, pMenu )

/*  [Beschreibung]

    ActivateHandler des Men"us des Men"ubuttons (SV)

    [Parameter]

    Menu *pMenu                     das das Event ausl"osende Men"u

    [R"uckgabewert]                 1: Event wurde verarbeitet,
                                    0: Event wurde nicht verarbeitet (SV-Menu)

*/
{
    if ( pFocusBox && pFocusBox->IsEditingActive() )
        pFocusBox->EndEditing( FALSE );
    BOOL bEnable = ( pFocusBox && pFocusBox->GetSelectionCount() );
    SvLBoxEntry* pEntry = bEnable ? pFocusBox->FirstSelected() : NULL;
    const USHORT nDepth =
        ( bEnable && pFocusBox->GetSelectionCount() ) ? pFocusBox->GetModel()->GetDepth( pEntry ) : 0;
    const USHORT nDocLevel = bEnable ? pFocusBox->GetDocLevel() : 0;
    int eVT = pFocusBox ? pFocusBox->GetViewType() : 0;
        // nur Vorlagen anlegen
    pMenu->EnableItem( ID_NEW, bEnable && 0 == nDepth && SfxOrganizeListBox_Impl::VIEW_TEMPLATES == eVT );
    // Vorlagen: Loeschen Ebene 0,1,3ff
    //           ein Bereich mu"s mindestens erhalten bleiben
    // Dateien : Loeschen Ebene > 2

    pMenu->EnableItem( ID_DELETE, bEnable && !DontDelete_Impl( pEntry ) );
    pMenu->EnableItem( ID_EDIT,
                       bEnable && eVT == SfxOrganizeListBox_Impl::VIEW_TEMPLATES && nDepth == nDocLevel );
    pMenu->EnableItem( ID_COPY_FROM,
                       bEnable && eVT == SfxOrganizeListBox_Impl::VIEW_TEMPLATES &&
                       ( nDepth == nDocLevel || nDepth == nDocLevel - 1 ) );
    pMenu->EnableItem( ID_COPY_TO,
                       bEnable && eVT == SfxOrganizeListBox_Impl::VIEW_TEMPLATES &&
                       nDepth == nDocLevel );
    pMenu->EnableItem( ID_RESCAN,
                       SfxOrganizeListBox_Impl::VIEW_TEMPLATES == aRightLb.GetViewType() ||
                       SfxOrganizeListBox_Impl::VIEW_TEMPLATES == aLeftLb.GetViewType() );
    BOOL bPrint = bEnable && nDepth > pFocusBox->GetDocLevel();
    if ( bPrint && pPrt )
        bPrint = !pPrt->IsPrinting() && !pPrt->IsJobActive();
    if ( bPrint && bEnable )
    {
        // only styles printable
        Path aPath( pFocusBox, pFocusBox->FirstSelected() );
        USHORT nIndex = aPath[ nDocLevel + 1 ];
        bPrint = ( nIndex == CONTENT_STYLE );
    }
    pMenu->EnableItem( ID_PRINT, bPrint );
    pMenu->EnableItem( ID_RESET_DEFAULT_TEMPLATE, bEnable );
    pMenu->EnableItem( ID_DEFAULT_TEMPLATE,
                       bEnable && eVT == SfxOrganizeListBox_Impl::VIEW_TEMPLATES && nDepth == nDocLevel );
    return 1;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, GetFocus_Impl, SfxOrganizeListBox_Impl *, pBox )

/*  [Beschreibung]

    GetFocus-Handler, wird aus den Select-Handler der Listboxen
    gerufen.
    Wird verwendet, im die Listbox, die den Focus besitzt sowie
    deren Zustand zu ermitteln.

    [Parameter]

    SfxOrganizeListBox *pBox        die rufende Box

*/

{
    if(pFocusBox && pFocusBox != pBox)
        pFocusBox->SelectAll(FALSE);
    pFocusBox = pBox;
    aFilesBtn.Enable( SfxOrganizeListBox_Impl::VIEW_FILES ==
                      pFocusBox->GetViewType() );
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, LeftListBoxSelect_Impl, ListBox *, pBox )

/*  [Beschreibung]

    Select-Handler, wird aus den Select-Handler der Listboxen
    gerufen.
    Wenn sich der Modus der Boxen (Dokumentsicht, Dokumentvorlagensicht)
    unterscheiden, werden die Models getrennt; andernfalls zusammengefa"st.

    [Parameter]

    ListBox *pBox               die rufende Box

*/
{
    const SfxOrganizeListBox_Impl::DataEnum
        eViewType = pBox->GetSelectEntryPos() == 0 ?
        SfxOrganizeListBox_Impl::VIEW_TEMPLATES : SfxOrganizeListBox_Impl::VIEW_FILES;
    if(eViewType!= aLeftLb.GetViewType()) {
        aLeftLb.SetViewType(eViewType);
        if(aRightLb.GetViewType() == eViewType)
            aLeftLb.SetModel(aRightLb.GetModel());
        else {
            // Models trennen
            aLeftLb.DisconnectFromModel();
            aLeftLb.Reset();
        }
    }
    GetFocus_Impl(&aLeftLb);
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, RightListBoxSelect_Impl, ListBox *, pBox )

/*  [Beschreibung]

    Select-Handler, wird aus den Select-Handler der Listboxen
    gerufen.
    Wenn sich der Modus der Boxen (Dokumentsicht, Dokumentvorlagensicht)
    unterscheiden, werden die Models getrennt; andernfalls zusammengefa"st.

    [Parameter]

    ListBox *pBox               die rufende Box

*/
{
    const SfxOrganizeListBox_Impl::DataEnum eViewType =
        pBox->GetSelectEntryPos() == 0 ?
        SfxOrganizeListBox_Impl::VIEW_TEMPLATES : SfxOrganizeListBox_Impl::VIEW_FILES;
    if(eViewType!= aRightLb.GetViewType())
    {
        aRightLb.SetViewType(eViewType);
        if(aLeftLb.GetViewType() == eViewType)
            aRightLb.SetModel(aLeftLb.GetModel());
        else
        {
            // Models trennen
            aRightLb.DisconnectFromModel();
            aRightLb.Reset();
        }
    }
    aRightLb.GrabFocus();
    GetFocus_Impl(&aRightLb);
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, AddFiles_Impl, Button *, pButton )

/*  [Beschreibung]

    Handler des Buttons f"ur das Hinzuf"ugen von Dateien per Dialog.

    [Parameter]

    Button *                der Button, der dieses Events ausgel"ost hat.

*/
{
    SfxSimpleFileDialog *pFileDlg = new SfxSimpleFileDialog( pDialog, WB_OPEN );
    const SfxObjectFactory& rFact = SfxObjectFactory::GetDefaultFactory();
    USHORT nMax = rFact.GetFilterCount();
    for ( USHORT i = 0; i < nMax; ++i )
    {
        const SfxFilter* pFilter = rFact.GetFilter(i);
        if ( pFilter->IsInternal() )
            continue;
        BOOL bIsImpFilter = pFilter->CanImport();
        if (bIsImpFilter && pFilter->IsAllowedAsTemplate())
        {
            pFileDlg->AddFilter(
                pFilter->GetUIName(), pFilter->GetWildcard()(),
                pFilter->GetTypeName() );
        }
    }
    pFileDlg->AddFilter( String(SfxResId(RID_STR_FILTCONFIG)), DEFINE_CONST_UNICODE( "*.cfg" ) );
    pFileDlg->AddFilter( String(SfxResId(RID_STR_FILTBASIC)), DEFINE_CONST_UNICODE( "*.sbl" ) );

    if ( aLastDir.Len() )
        pFileDlg->SetPath( aLastDir );
    if ( RET_OK == pFileDlg->Execute() )
    {
        String aPath = pFileDlg->GetPath();
        aMgr.InsertFile( pFocusBox, aPath );
        INetURLObject aObj( aPath );
        aObj.removeSegment();
        aObj.setFinalSlash();
        aLastDir = aObj.GetMainURL();
    }
    delete pFileDlg;
    return 0;
}

//-------------------------------------------------------------------------

short SfxTemplateOrganizeDlg::Execute()

/*  [Beschreibung]

    "Uberladene Execute- Methode; speichert gfs. "Anderungen an den
    Dokumentvorlagen
    (SV-Methode)

*/

{
    const short nRet = ModalDialog::Execute();
    if(RET_CANCEL != nRet)
    {
        pImp->aMgr.SaveAll(this);
        SfxTemplateDialog* pTemplDlg = SFX_APP()->GetTemplateDialog();
        if(pTemplDlg)
            pTemplDlg->Update();
    }
    return nRet;
}


//-------------------------------------------------------------------------

SfxTemplateOrganizeDlg::SfxTemplateOrganizeDlg(Window * pParent,
                                                SfxDocumentTemplates *pTempl)
:   ModalDialog( pParent, SfxResId(DLG_ORGANIZE)),
    pImp( new SfxOrganizeDlg_Impl(this, pTempl) )

/*  [Beschreibung]

    Konstruktor

*/
{
    FreeResource();
}

//-------------------------------------------------------------------------

SfxTemplateOrganizeDlg::~SfxTemplateOrganizeDlg()
{
    GetpApp()->RemoveAccel(&pImp->aEditAcc);
    delete pImp->pPrt;
    delete pImp;
}


