/*************************************************************************
 *
 *  $RCSfile: glossary.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-12 15:50:41 $
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

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGS

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _SVSTDARR_HXX
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SVX_MULTIPAT_HXX //autogen
#include <svx/multipat.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _OFAACCFG_HXX //autogen
#include <offmgr/ofaaccfg.hxx>
#endif

#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _UNOCRSR_HXX //autogen wg. SwUnoCrsr
#include <unocrsr.hxx>
#endif
#ifndef _UNOTOOLS_HXX
#include <unotools.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTGROUP_HPP_
#include <com/sun/star/text/XAutoTextGroup.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTENTRY_HPP_
#include <com/sun/star/text/XAutoTextEntry.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTCONTAINER_HPP_
#include <com/sun/star/text/XAutoTextContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _SWWAIT_HXX
#include <swwait.hxx>
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
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _GLOSSARY_HXX
#include <glossary.hxx>
#endif
#ifndef _GLOSHDL_HXX
#include <gloshdl.hxx>
#endif
#ifndef _GLOSBIB_HXX
#include <glosbib.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>                   // fuer ::GetGlossaries()
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _MACASSGN_HXX
#include <macassgn.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _GLOSSARY_HRC
#include <glossary.hrc>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#define LONG_LENGTH 60
#define SHORT_LENGTH 30

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::ucb;
using namespace ::utl;
using namespace ::ucb;
using namespace ::rtl;

/* -----------------------------08.02.00 10:28--------------------------------

 ---------------------------------------------------------------------------*/
struct GroupUserData
{
    String      sGroupName;
    sal_uInt16  nPathIdx;
    BOOL        bReadonly;

    GroupUserData()
        : nPathIdx(0),
          bReadonly(FALSE)  {}
};

/*------------------------------------------------------------------------
 Beschreibung:  Dialog fuer neuen Bausteinnamen
------------------------------------------------------------------------*/
class SwNewGlosNameDlg : public ModalDialog
{
    FixedText       aNNFT;
    Edit            aNewName;
    FixedText       aNSFT;
    NoSpaceEdit     aNewShort;
    OKButton        aOk;
    CancelButton    aCancel;
     GroupBox       aGrp;
    FixedText       aONFT;
    Edit            aOldName;
    FixedText       aOSFT;
    Edit            aOldShort;

protected:
    DECL_LINK( Modify, Edit * );
    DECL_LINK( Rename, Button * );

public:
    SwNewGlosNameDlg( Window* pParent,
                      const String& rOldName,
                      const String& rOldShort );

    String GetNewName()  const { return aNewName.GetText(); }
    String GetNewShort() const { return aNewShort.GetText(); }
};

SwNewGlosNameDlg::SwNewGlosNameDlg(Window* pParent,
                            const String& rOldName,
                            const String& rOldShort ) :
    ModalDialog( pParent, SW_RES( DLG_RENAME_GLOS ) ),
    aOk     (this, SW_RES( BT_OKNEW)),
    aCancel (this, SW_RES( BT_CANCEL)),
    aONFT   (this, SW_RES( FT_ON    )),
    aOSFT   (this, SW_RES( FT_OS    )),
    aNNFT   (this, SW_RES( FT_NN    )),
    aNSFT   (this, SW_RES( FT_NS    )),
    aOldName(this, SW_RES( ED_ON    )),
    aOldShort(this,SW_RES( ED_OS    )),
    aNewName(this, SW_RES( ED_NN    )),
    aNewShort(this,SW_RES( ED_NS    )),
    aGrp    (this, SW_RES( GB_NN    ))
{
    FreeResource();
    aOldName.SetText( rOldName );
    aOldShort.SetText( rOldShort );
    aNewShort.SetMaxTextLen(SHORT_LENGTH);
    aNewName.SetMaxTextLen(LONG_LENGTH);
    aNewName.SetModifyHdl(LINK(this, SwNewGlosNameDlg, Modify ));
    aNewShort.SetModifyHdl(LINK(this, SwNewGlosNameDlg, Modify ));
    aOk.SetClickHdl(LINK(this, SwNewGlosNameDlg, Rename ));
    aNewName.GrabFocus();
}

/*------------------------------------------------------------------------
 Beschreibung:  aktuell eingestellte Gruppe erfragen / setzen
------------------------------------------------------------------------*/

String SwGlossaryDlg::GetCurrGroup()
{
    if( pCurrGlosGroup && pCurrGlosGroup->Len() )
        return *pCurrGlosGroup;
    return SwGlossaries::GetDefName();
}



void SwGlossaryDlg::SetActGroup(const String &rGrp)
{
    if( !pCurrGlosGroup )
        pCurrGlosGroup = new String;
    *pCurrGlosGroup = rGrp;
}



SwGlossaryDlg::SwGlossaryDlg(SfxViewFrame* pViewFrame,
                            SwGlossaryHdl * pGlosHdl, SwWrtShell *pWrtShell) :

    SvxStandardDialog(&pViewFrame->GetWindow(), SW_RES(DLG_GLOSSARY)),
    aExampleGB    (this, SW_RES(GB_EXAMPLE  )),
    aExampleWIN   (this, SW_RES(WIN_EXAMPLE )),
    aShowExampleCB(this, SW_RES(CB_SHOW_EXAMPLE )),
    aInsertTipCB  (this, SW_RES(CB_INSERT_TIP)),
    aNameLbl      (this, SW_RES(FT_NAME)),
    aNameED       (this, SW_RES(ED_NAME)),
    aShortNameLbl (this, SW_RES(FT_SHORTNAME)),
    aShortNameEdit(this, SW_RES(ED_SHORTNAME)),
    aCategoryBox  (this, SW_RES(LB_BIB)),
    aGlossaryFrm  (this, SW_RES(GB_GLOSSARY)),
    aFileRelCB    (this, SW_RES(CB_FILE_REL)),
    aNetRelCB     (this, SW_RES(CB_NET_REL)),
    aRelativeGB    (this, SW_RES(GB_RELATIVE)),
    aInsertBtn    (this, SW_RES(PB_INSERT)),
    aEditBtn      (this, SW_RES(PB_EDIT)),
    aBibBtn       (this, SW_RES(PB_BIB)),
    aPathBtn      (this, SW_RES(PB_PATH)),
    aCloseBtn     (this, SW_RES(PB_CLOSE)),
    aHelpBtn      (this, SW_RES(PB_HELP)),
    sReadonlyPath (SW_RES(ST_READONLY_PATH)),
    pMenu         (new PopupMenu(SW_RES(MNU_EDIT))),
    pGlossaryHdl  (pGlosHdl),
    pSh           (pWrtShell),
    pExampleFrame(0),
    bSelection( pWrtShell->IsSelection() ),
    bReadOnly( sal_False ),
    bIsOld( sal_False ),
    bIsDocReadOnly(sal_False)
{
    // Static-Pointer initialisieren
    if( !pCurrGlosGroup )
        pCurrGlosGroup = new String;//(SwGlossaries::GetDefName());

    pMenu->SetActivateHdl(LINK(this,SwGlossaryDlg,EnableHdl));
    pMenu->SetSelectHdl(LINK(this,SwGlossaryDlg,MenuHdl));
    aEditBtn.SetPopupMenu(pMenu);
    aEditBtn.SetSelectHdl(LINK(this,SwGlossaryDlg,EditHdl));
    aPathBtn.SetClickHdl(LINK(this, SwGlossaryDlg, PathHdl));

    aNameED.SetModifyHdl(LINK(this,SwGlossaryDlg,NameModify));
    aShortNameEdit.SetModifyHdl(LINK(this,SwGlossaryDlg,NameModify));

    aCategoryBox.SetDoubleClickHdl(LINK(this,SwGlossaryDlg, NameDoubleClick));
    aCategoryBox.SetSelectHdl(LINK(this,SwGlossaryDlg,GrpSelect));
    aBibBtn.SetClickHdl(LINK(this,SwGlossaryDlg,BibHdl));
    aShowExampleCB.SetClickHdl(LINK(this, SwGlossaryDlg, ShowPreviewHdl));

    aShortNameEdit.SetMaxTextLen(SHORT_LENGTH);
    aNameED.SetMaxTextLen(LONG_LENGTH);
    FreeResource();

    const OfaAutoCorrCfg* pCfg = OFF_APP()->GetAutoCorrConfig();

    aShowExampleCB.Check( pCfg->IsAutoTextPreview());
    ShowPreviewHdl(&aShowExampleCB);

    bIsDocReadOnly = pSh->GetView().GetDocShell()->IsReadOnly() ||
                      pSh->HasReadonlySel();
    if( bIsDocReadOnly )
        aInsertBtn.Enable(sal_False);
    aNameED.GrabFocus();
    aCategoryBox.SetHelpId(HID_MD_GLOS_CATEGORY);
    aCategoryBox.SetWindowBits(WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL|WB_VSCROLL|WB_CLIPCHILDREN|WB_SORT);
    aCategoryBox.GetModel()->SetSortMode(SortAscending);
    aCategoryBox.SetHighlightRange();   // ueber volle Breite selektieren
    Bitmap aRootClosed      (SW_RES(BMP_ROOT_CLOSED));
    Bitmap aRootOpened      (SW_RES(BMP_ROOT_OPENED));
    aCategoryBox.SetNodeBitmaps( aRootClosed, aRootOpened );

    Init();
}
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


SwGlossaryDlg::~SwGlossaryDlg()
{
    OfaAutoCorrCfg* pCfg = OFF_APP()->GetAutoCorrConfig();
    pCfg->SetAutoTextPreview(aShowExampleCB.IsChecked()) ;

    aCategoryBox.Clear();
    aEditBtn.SetPopupMenu(0);
    delete pMenu;
    delete pExampleFrame;
}
/*------------------------------------------------------------------------
 Beschreibung:  Auswahl neue Gruppe
------------------------------------------------------------------------*/


IMPL_LINK( SwGlossaryDlg, GrpSelect, SvTreeListBox *, pBox )
{
    SvLBoxEntry* pEntry = pBox->FirstSelected();
    if(!pEntry)
        return 0;
    SvLBoxEntry* pParent = pBox->GetParent(pEntry) ? pBox->GetParent(pEntry) : pEntry;
    GroupUserData* pGroupData = (GroupUserData*)pParent->GetUserData();
    (*pCurrGlosGroup) = pGroupData->sGroupName;
    (*pCurrGlosGroup) += GLOS_DELIM;
    (*pCurrGlosGroup) += String::CreateFromInt32(pGroupData->nPathIdx);
    pGlossaryHdl->SetCurGroup(*pCurrGlosGroup);
    const sal_uInt16 nCount = pGlossaryHdl->GetGlossaryCnt();
    // Aktuellen Textbaustein setzen
    bReadOnly = pGlossaryHdl->IsReadOnly();
    EnableShortName( !bReadOnly );
    aEditBtn.Enable(!bReadOnly);
    bIsOld = pGlossaryHdl->IsOld();
    if( pParent != pEntry)
    {
        String aName(pBox->GetEntryText(pEntry));
        aNameED.SetText(aName);
        aShortNameEdit.SetText(*(String*)pEntry->GetUserData());
        pEntry = pBox->GetParent(pEntry);
        aInsertBtn.Enable( !bIsDocReadOnly);
        ShowAutoText(*pCurrGlosGroup, aShortNameEdit.GetText());
    }
    else
        ShowAutoText(aEmptyStr, aEmptyStr);
    //Controls aktualisieren
    NameModify(&aShortNameEdit);
    return 0;
}
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


void SwGlossaryDlg::Apply()
{
    const String aGlosName(aShortNameEdit.GetText());
    if(aGlosName.Len()) pGlossaryHdl->InsertGlossary(aGlosName);
}
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/
/* inline */ void SwGlossaryDlg::EnableShortName(sal_Bool bOn)
{
    aShortNameLbl.Enable(bOn);
    aShortNameEdit.Enable(bOn);
}

/* -----------------26.11.98 16:15-------------------
 * existiert der Titel in der ausgewaehlten Gruppe?
 * --------------------------------------------------*/
SvLBoxEntry* SwGlossaryDlg::DoesBlockExist(const String& rBlock,
                const String& rShort)
{
    //evtl. vorhandenen Eintrag in der TreeListBox suchen
    SvLBoxEntry* pEntry = aCategoryBox.FirstSelected();
    DBG_ASSERT(pEntry, "keine Selektion?")
    if(aCategoryBox.GetParent(pEntry))
        pEntry = aCategoryBox.GetParent(pEntry);
    sal_uInt32 nChildCount = aCategoryBox.GetChildCount( pEntry );
    for(sal_uInt32 i = 0; i < nChildCount; i++)
    {
        SvLBoxEntry* pChild = aCategoryBox.GetEntry( pEntry, i );
        if(rBlock == aCategoryBox.GetEntryText(pChild) &&
            (!rShort.Len() || rShort == *(String*)pChild->GetUserData()))
        {
            return pChild;
        }
    }
    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


IMPL_LINK( SwGlossaryDlg, NameModify, Edit *, pEdit )
{
    String aName(aNameED.GetText());
    sal_Bool bNameED = pEdit == &aNameED;
    if( !aName.Len() )
    {
        if(bNameED)
            aShortNameEdit.SetText(aName);
        aInsertBtn.Enable(sal_False);
        return 0;
    }
    String sShortSearch;
    if(!bNameED)
        sShortSearch = pEdit->GetText();
    sal_Bool bNotFound = !DoesBlockExist(aName, sShortSearch);
    if(bNameED)
    {
            // ist der Text durch einen Klick in die Listbox in das
            // Edit gekommem?
        if(bNotFound)
        {
            sal_uInt16 nSz = aName.Len();
            sal_uInt16 nStart = 1;
            while( aName.GetChar( nStart-1 ) == ' ' && nStart < nSz )
                nStart++;
            String aBuf( aName.GetChar( nStart-1 ));
            for( nStart ; nStart < nSz; ++nStart )
            {
                if( aName.GetChar( nStart-1 ) == ' ' && aName.GetChar( nStart ) != ' ')
                    aBuf += aName.GetChar( nStart );
            }
            aShortNameEdit.SetText(aBuf);
            EnableShortName();
        }
        else
        {
            aShortNameEdit.SetText(pGlossaryHdl->GetGlossaryShortName(aName));
            EnableShortName(!bReadOnly);
        }
        aInsertBtn.Enable(!bNotFound && !bIsDocReadOnly);
    }
    else
    {
        //ShortNameEdit
        if(!bNotFound)
        {
            sal_Bool bEnable = !bNotFound;
            bEnable &= !bIsDocReadOnly;
            aInsertBtn.Enable(bEnable);
        }
    }
    return 0;
}
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


IMPL_LINK_INLINE_START( SwGlossaryDlg, NameDoubleClick, SvTreeListBox*, pBox )
{
    SvLBoxEntry* pEntry = pBox->FirstSelected();
    if(pBox->GetParent(pEntry) && !bIsDocReadOnly)
        EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SwGlossaryDlg, NameDoubleClick, SvTreeListBox*, EMPTYARG )
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


IMPL_LINK( SwGlossaryDlg, EnableHdl, Menu *, pMn )
{
    const String aEditText(aNameED.GetText());
    const sal_Bool bHasEntry = aEditText.Len() && aShortNameEdit.GetText().Len();
    const sal_Bool bExists = 0 != DoesBlockExist(aEditText, aShortNameEdit.GetText());
    pMn->EnableItem(FN_GL_DEFINE, bSelection && bHasEntry && !bExists);
    pMn->EnableItem(FN_GL_DEFINE_TEXT, bSelection && bHasEntry && !bExists);
    pMn->EnableItem(FN_GL_COPY_TO_CLIPBOARD, bExists);
    pMn->EnableItem(FN_GL_REPLACE, bSelection && bExists && !bIsOld );
    pMn->EnableItem(FN_GL_EDIT, bExists );
    pMn->EnableItem(FN_GL_RENAME, bExists && !bIsOld );
    pMn->EnableItem(FN_GL_DELETE, bExists && !bIsOld );
    pMn->EnableItem(FN_GL_MACRO, bExists && !bIsOld  &&
                                    !pGlossaryHdl->IsReadOnly() );

    SvLBoxEntry* pEntry = aCategoryBox.FirstSelected();
    DBG_ASSERT(pEntry, "keine Selektion?")
    sal_Bool bEnable = !aCategoryBox.GetParent( pEntry ) && !bIsOld &&
                        !pGlossaryHdl->IsReadOnly();
    pMn->EnableItem( FN_GL_IMPORT, bEnable );
    return 1;
}
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


IMPL_LINK( SwGlossaryDlg, MenuHdl, Menu *, pMn )
{
    sal_Bool bNoAttr = sal_False;

    switch(pMn->GetCurItemId())
    {
        case FN_GL_REPLACE:
            pGlossaryHdl->NewGlossary(  aNameED.GetText(),
                                        aShortNameEdit.GetText() );
        break;
        case FN_GL_DEFINE_TEXT:
            bNoAttr = sal_True;
            // Kein break!!!
        case FN_GL_DEFINE:
        {
            const String aStr(aNameED.GetText());
            const String aShortName(aShortNameEdit.GetText());
            if(pGlossaryHdl->HasShortName(aShortName))
            {
                InfoBox(this, SW_RES(MSG_DOUBLE_SHORTNAME)).Execute();
                aShortNameEdit.SetSelection(Selection(0, SELECTION_MAX));
                aShortNameEdit.GrabFocus();
                break;
            }
            if(pGlossaryHdl->NewGlossary(aStr, aShortName, sal_False, bNoAttr ))
            {
                SvLBoxEntry* pEntry = aCategoryBox.FirstSelected();
                if(aCategoryBox.GetParent(pEntry))
                    pEntry = aCategoryBox.GetParent(pEntry);

                SvLBoxEntry* pChild = aCategoryBox.InsertEntry(aStr, pEntry);
                pChild->SetUserData(new String(aShortName));
                aNameED.SetText(aStr);
                aShortNameEdit.SetText(aShortName);
                NameModify(&aNameED);       // fuer Schalten der Buttons

            }
        }
        break;
        case FN_GL_COPY_TO_CLIPBOARD :
        {
            pGlossaryHdl->CopyToClipboard(*pSh, aShortNameEdit.GetText());
        }
        break;
        case FN_GL_EDIT:
        break;
        case FN_GL_RENAME:
        {
            aShortNameEdit.SetText(pGlossaryHdl->GetGlossaryShortName(aNameED.GetText()));
            SwNewGlosNameDlg* pNewNameDlg = new SwNewGlosNameDlg(this, aNameED.GetText(),
                                            aShortNameEdit.GetText() );
            if(pNewNameDlg->Execute() == RET_OK)
            {
                pGlossaryHdl->Rename( aShortNameEdit.GetText(),
                                        pNewNameDlg->GetNewShort(),
                                        pNewNameDlg->GetNewName());
                SvLBoxEntry* pEntry = aCategoryBox.FirstSelected();
                SvLBoxEntry* pNewEntry = aCategoryBox.InsertEntry(
                        pNewNameDlg->GetNewName(), aCategoryBox.GetParent(pEntry));
                pNewEntry->SetUserData(new String(pNewNameDlg->GetNewShort()));
                delete (String*)pEntry->GetUserData();
                aCategoryBox.GetModel()->Remove(pEntry);
                aCategoryBox.Select(pNewEntry);
                aCategoryBox.MakeVisible(pNewEntry);
            }
            GrpSelect( &aCategoryBox );
            delete pNewNameDlg;
        }
        break;
        case FN_GL_DELETE:
        {
            QueryBox aQuery(this, SW_RES(MSG_QUERY_DELETE));
            if(RET_YES == aQuery.Execute())
            {
                const String aShortName(aShortNameEdit.GetText());
                const String aTitle(aNameED.GetText());
                if(aTitle.Len() && pGlossaryHdl->DelGlossary(aShortName))
                {
                    SvLBoxEntry* pChild = DoesBlockExist(aTitle, aShortName);
                    DBG_ASSERT(pChild, "Eintrag nicht gefunden!")
                    SvLBoxEntry* pParent = aCategoryBox.GetParent(pChild);
                    aCategoryBox.Select(pParent);

                    aCategoryBox.GetModel()->Remove(pChild);
                    aNameED.SetText( aEmptyStr );
                    NameModify(&aNameED);
                }
            }
        }
        break;
        case FN_GL_MACRO:
        {
            SfxItemSet aSet( pSh->GetAttrPool(), RES_FRMMACRO, RES_FRMMACRO );

            SvxMacro aStart(aEmptyStr, aEmptyStr, STARBASIC);
            SvxMacro aEnd(aEmptyStr, aEmptyStr, STARBASIC);
            pGlossaryHdl->GetMacros(aShortNameEdit.GetText(), aStart, aEnd );

            SvxMacroItem aItem;
            if( aStart.GetMacName().Len() )
                aItem.SetMacro( SW_EVENT_START_INS_GLOSSARY, aStart );
            if( aEnd.GetMacName().Len() )
                aItem.SetMacro( SW_EVENT_END_INS_GLOSSARY, aEnd );

            aSet.Put( aItem );

            const SfxPoolItem* pItem;
            SwMacroAssignDlg aMacDlg( this, aSet, MACASSGN_TEXTBAUST );
            if( RET_OK == aMacDlg.Execute() &&
                SFX_ITEM_SET == aMacDlg.GetOutputItemSet()->GetItemState(
                    RES_FRMMACRO, sal_False, &pItem ))
            {
                const SvxMacroTableDtor& rTbl = ((SvxMacroItem*)pItem)->GetMacroTable();

                pGlossaryHdl->SetMacros( aShortNameEdit.GetText(),
                                            rTbl.Get( SW_EVENT_START_INS_GLOSSARY ),
                                            rTbl.Get( SW_EVENT_END_INS_GLOSSARY ) );
            }
        }
        break;

        case FN_GL_IMPORT:
        {
            // call the FileOpenDialog do find WinWord - Files with templates
            SfxFileDialog* pDlg = new SfxFileDialog( this, WB_OPEN | WB_3DLOOK );
            SvtPathOptions aPathOpt;
            pDlg->SetPath( aPathOpt.GetWorkPath() );
            String sWW8( C2S(FILTER_WW8) );

            sal_uInt16 i = 0;

            const SfxFactoryFilterContainer* pFltCnt =
                    SwDocShell::Factory().GetFilterContainer();
            if( pFltCnt )
            {
                const SfxFilter* pFilter;
                sal_uInt16 nCount = pFltCnt->GetFilterCount();
                for( i = 0; i < nCount; ++i )
                    if( ( pFilter = pFltCnt->GetFilter( i ))
                        /*->IsAllowedAsTemplate()
                        && pFilter*/->GetUserData() == sWW8 )
                    {
                        pDlg->AddFilter( pFilter->GetUIName(),
                                ((WildCard&)pFilter->GetWildcard())(),
                                pFilter->GetTypeName() );
                        pDlg->SetCurFilter( pFilter->GetUIName() );
                }
            }

            if( i && RET_OK == pDlg->Execute() )
            {
                if( pGlossaryHdl->ImportGlossaries( pDlg->GetPath() ))
                    Init();
                else
                {
                    InfoBox(this, SW_RES( MSG_NO_GLOSSARIES )).Execute();
                }
            }
            delete pDlg;
        }
           break;

        default:
            return 0;
    }
    return 1;
}
/*--------------------------------------------------------------------
     Beschreibung:  Dialog Verwaltung Bereiche
 --------------------------------------------------------------------*/


IMPL_LINK( SwGlossaryDlg, BibHdl, Button *, EMPTYARG )
{
    SwGlossaries* pGloss = ::GetGlossaries();
    if( pGloss->IsGlosPathErr() )
        pGloss->ShowError();
    else
    {
        //check if at least one glossary path is write enabled
        SvtPathOptions aPathOpt;
        String sGlosPath( aPathOpt.GetGlossaryPath() );
        USHORT nPaths = sGlosPath.GetTokenCount(';');
        BOOL bIsWritable = FALSE;
        for(USHORT nPath = 0; nPath < nPaths; nPath++)
        {
            String sPath = URIHelper::SmartRelToAbs(sGlosPath.GetToken(nPath, ';'));
            try
            {
                Content aTestContent( sPath,
                            uno::Reference< XCommandEnvironment >());
                Any aAny = aTestContent.getPropertyValue( C2U("IsReadOnly") );
                if(aAny.hasValue())
                {
                    bIsWritable = !*(sal_Bool*)aAny.getValue();
                }
            }
            catch(...)
            {}
            if(bIsWritable)
                break;
        }
        if(bIsWritable)
        {

            SwGlossaryGroupDlg *pDlg = new SwGlossaryGroupDlg( this, pGloss->GetPathArray(), pGlossaryHdl );
            if ( RET_OK == pDlg->Execute() )
            {
                Init();
                //if new groups were created - select one of them
                String sNewGroup = pDlg->GetCreatedGroupName();
                SvLBoxEntry* pEntry = aCategoryBox.First();
                while(sNewGroup.Len() && pEntry)
                {
                    if(!aCategoryBox.GetParent(pEntry))
                    {
                        GroupUserData* pGroupData = (GroupUserData*)pEntry->GetUserData();
                        String sGroup = pGroupData->sGroupName;
                        sGroup += GLOS_DELIM;
                        sGroup += String::CreateFromInt32(pGroupData->nPathIdx);
                        if(sGroup == sNewGroup)
                        {
                            aCategoryBox.Select(pEntry);
                            aCategoryBox.MakeVisible(pEntry);
                            GrpSelect(&aCategoryBox);
                            break;
                        }
                    }
                    pEntry = aCategoryBox.Next(pEntry);
                }

            }
            delete pDlg;
        }
        else
        {
            QueryBox aBox(this, WB_YES_NO, sReadonlyPath);
            if(RET_YES == aBox.Execute())
                PathHdl(&aPathBtn);
        }
    }
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:  Initialisierung; aus Ctor und nach Bearbeiten Bereiche
------------------------------------------------------------------------*/


void SwGlossaryDlg::Init()
{
    aCategoryBox.SetUpdateMode( sal_False );
    aCategoryBox.Clear();
    // Textbausteinbereiche anzeigen
    const sal_uInt16 nCnt = pGlossaryHdl->GetGroupCnt();
    SvLBoxEntry* pSelEntry = 0;
    const String sSelStr(pCurrGlosGroup->GetToken(0, GLOS_DELIM));
    const sal_uInt16 nSelPath = pCurrGlosGroup->GetToken(1, GLOS_DELIM).ToInt32();
    for(sal_uInt16 nId = 0; nId < nCnt; ++nId )
    {
        String sTitle;
        String sGroupName(pGlossaryHdl->GetGroupName(nId, &sTitle));
        if(!sTitle.Len())
            sTitle = sGroupName.GetToken( 0, GLOS_DELIM );
        SvLBoxEntry* pEntry = aCategoryBox.InsertEntry( sTitle );
        sal_uInt16 nPath = sGroupName.GetToken( 1, GLOS_DELIM ).ToInt32();

        GroupUserData* pData = new GroupUserData;
        pData->sGroupName = sGroupName.GetToken(0, GLOS_DELIM);
        pData->nPathIdx = nPath;
        pData->bReadonly = pGlossaryHdl->IsReadOnly(&sGroupName);

        pEntry->SetUserData(pData);
        if(sSelStr == pData->sGroupName && nSelPath == nPath)
            pSelEntry = pEntry;

        //Eintraege fuer die Gruppen auffuellen
        {
            pGlossaryHdl->SetCurGroup(sGroupName, sal_False, sal_True);
            const sal_uInt16 nCount = pGlossaryHdl->GetGlossaryCnt();
            for(sal_uInt16 i = 0; i < nCount; ++i)
            {
                String sTitle(pGlossaryHdl->GetGlossaryName(i));
                SvLBoxEntry* pChild = aCategoryBox.InsertEntry(
                                    sTitle, pEntry);
                pChild->SetUserData(new String(pGlossaryHdl->GetGlossaryShortName(i)));
            }
        }
    }
        // Aktuelle Gruppe setzen und Textbausteine anzeigen
    if(!pSelEntry)
    {
        //find a non-readonly group
        SvLBoxEntry* pSearch = aCategoryBox.First();
        while(pSearch)
        {
            if(!aCategoryBox.GetParent(pSearch))
            {
                GroupUserData* pData = (GroupUserData*)pSearch->GetUserData();
                if(!pData->bReadonly)
                {
                    pSelEntry = pSearch;
                    break;
                }
            }
            pSearch = aCategoryBox.Next(pSearch);
        }
        if(!pSelEntry)
            pSelEntry = aCategoryBox.GetEntry(0);
    }
    if(pSelEntry)
    {
        aCategoryBox.Expand(pSelEntry);
        aCategoryBox.Select(pSelEntry);
        aCategoryBox.MakeVisible(pSelEntry);
        GrpSelect(&aCategoryBox);
    }
    //JP 16.11.99: the SvxTreeListBox has a Bug. The Box dont recalc the
    //      outputsize, when all entries are insertet. The result is, that
    //      the Focus/Highlight rectangle is to large and paintet over the
    //      HScrollbar. -> Fix: call the resize
    aCategoryBox.Resize();

    aCategoryBox.GetModel()->Resort();
    aCategoryBox.SetUpdateMode( sal_True );
    aCategoryBox.Update();

    const OfaAutoCorrCfg* pCfg = OFF_APP()->GetAutoCorrConfig();
    aFileRelCB.Check( pCfg->IsSaveRelFile() );
    aFileRelCB.SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    aNetRelCB.Check( pCfg->IsSaveRelNet() );
    aNetRelCB.SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    aInsertTipCB.Check( pCfg->IsAutoTextTip() );
    aInsertTipCB.SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
}
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK_INLINE_START( SwGlossaryDlg, EditHdl, Button *, EMPTYARG )
{
//EndDialog darf nicht im MenuHdl aufgerufen werden
    if(aEditBtn.GetCurItemId() == FN_GL_EDIT )
        EndDialog(RET_EDIT);
    return 0;
}
IMPL_LINK_INLINE_END( SwGlossaryDlg, EditHdl, Button *, EMPTYARG )

/*------------------------------------------------------------------------
 Beschreibung:  KeyInput fuer ShortName - Edits ohne Spaces
------------------------------------------------------------------------*/

IMPL_LINK( SwNewGlosNameDlg, Modify, Edit *, pBox )
{
    String aName(aNewName.GetText());
    SwGlossaryDlg* pDlg = (SwGlossaryDlg*)GetParent();

    if( pBox == &aNewName )
    {
        if( aName.Len() )
            aNewShort.SetText( pDlg->pGlossaryHdl->GetValidShortCut( aName ));
        else
            aNewShort.SetText(aName);
    }

    sal_Bool bEnable = aName.Len() && aNewShort.GetText().Len() &&
        (!pDlg->DoesBlockExist(aName, aNewShort.GetText())
            || aName == aOldName.GetText());
    aOk.Enable(bEnable);
    return 0;
}
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwNewGlosNameDlg, Rename, Button *, EMPTYARG )
{
    SwGlossaryDlg* pDlg = (SwGlossaryDlg*)GetParent();
    String sNew = aNewShort.GetText();
    GetAppCharClass().toUpper(sNew);
    if( pDlg->pGlossaryHdl->HasShortName(aNewShort.GetText())
        && sNew != aOldShort.GetText() )
    {
        InfoBox(this, SW_RES(MSG_DOUBLE_SHORTNAME)).Execute();
        aNewShort.GrabFocus();
    }
    else
        EndDialog(sal_True);
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwGlossaryDlg, CheckBoxHdl, CheckBox *, pBox )
{
    OfaAutoCorrCfg* pCfg = OFF_APP()->GetAutoCorrConfig();
    sal_Bool bCheck = pBox->IsChecked();
    if( pBox == &aInsertTipCB )
        pCfg->SetAutoTextTip(bCheck);
    else if(pBox == &aFileRelCB)
        pCfg->SetSaveRelFile(bCheck);
    else
        pCfg->SetSaveRelNet(bCheck);
    return 0;
}

/* -----------------26.11.98 15:18-------------------
 * TreeListBox fuer Gruppen und Bausteine
 * --------------------------------------------------*/
SwGlTreeListBox::SwGlTreeListBox(Window* pParent, const ResId& rResId) :
    SvTreeListBox(pParent, rResId),
    sReadonly     (SW_RES(ST_READONLY)),
    pDragEntry(0)
{
    FreeResource();
    SetDragDropMode( SV_DRAGDROP_CTRL_MOVE|SV_DRAGDROP_CTRL_COPY );
}
/* -----------------30.11.98 10:49-------------------
 *
 * --------------------------------------------------*/
void SwGlTreeListBox::Clear()
{
    SvLBoxEntry* pEntry = First();
    while(pEntry)
    {
        if(GetParent(pEntry))
            delete (String*)pEntry->GetUserData();
        else
            delete (GroupUserData*)pEntry->GetUserData();
        pEntry = Next(pEntry);
    }
    SvTreeListBox::Clear();
}

/*-----------------10.06.97 14.52-------------------

--------------------------------------------------*/
void SwGlTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
    SvLBoxEntry* pEntry = GetEntry( aPos );
    // Hilfe gibt es nur fuer die Gruppennamen
    if(pEntry)
    {
        SvLBoxTab* pTab;
        SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
        if(pItem)
        {
            aPos = SvTreeListBox::GetEntryPos( pEntry );
             Size aSize(pItem->GetSize( this, pEntry ));
            aPos.X() = GetTabPos( pEntry, pTab );

            if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                aSize.Width() = GetSizePixel().Width() - aPos.X();
            aPos = OutputToScreenPixel(aPos);
             Rectangle aItemRect( aPos, aSize );
            String sMsg;
            if(!GetParent(pEntry))
            {
                GroupUserData* pData = (GroupUserData*)pEntry->GetUserData();
                const SvStrings* pPathArr = ::GetGlossaries()->GetPathArray();
                if(pPathArr->Count())
                {
                    sMsg = (*(*pPathArr)[pData->nPathIdx]);
                    sMsg += INET_PATH_TOKEN;
                    sMsg += pData->sGroupName;
                    sMsg += SwGlossaries::GetExtension();
                    INetURLObject aTmp(sMsg);
                    sMsg = aTmp.GetPath();

                    if(pData->bReadonly)
                    {
                        sMsg += ' ';
                        sMsg += '(';
                        sMsg += sReadonly;
                        sMsg += ')';
                    }


                }
            }
            else
                sMsg = *(String*)pEntry->GetUserData();
            Help::ShowQuickHelp( this, aItemRect, sMsg,
                        QUICKHELP_LEFT|QUICKHELP_VCENTER );
        }
    }
}
/* -----------------26.11.98 14:42-------------------
 *
 * --------------------------------------------------*/
DragDropMode  SwGlTreeListBox::NotifyBeginDrag( SvLBoxEntry* pEntry)
{
    DragDropMode  eRet;
    pDragEntry = pEntry;
    String sEntry;
    if(!GetParent(pEntry))
        eRet = SV_DRAGDROP_NONE;
    else
    {
        SwGlossaryDlg* pDlg = (SwGlossaryDlg*)Window::GetParent();
        SvLBoxEntry* pParent = GetParent(pEntry);

        GroupUserData* pGroupData = (GroupUserData*)pParent->GetUserData();
        String sEntry(pGroupData->sGroupName);
        sEntry += GLOS_DELIM;
        sEntry += String::CreateFromInt32(pGroupData->nPathIdx);
        sal_uInt16 nDragOptions = DRAG_COPYABLE;
        eRet = SV_DRAGDROP_CTRL_COPY;
        if(!pDlg->pGlossaryHdl->IsReadOnly(&sEntry))
        {
            eRet |= SV_DRAGDROP_CTRL_MOVE;
            nDragOptions |= DRAG_MOVEABLE;
        }
        SetDragOptions( nDragOptions );
    }
    return eRet;
}
/* -----------------27.11.98 09:35-------------------
 *
 * --------------------------------------------------*/
sal_Bool    SwGlTreeListBox::NotifyQueryDrop( SvLBoxEntry* pEntry)
{
    // TODO: Readonly - Ueberpruefung fehlt noch!
    SvLBoxEntry* pSrcParent = GetParent(pEntry) ? GetParent(pEntry) : pEntry;
    SvLBoxEntry* pDestParent =
        GetParent(pDragEntry ) ? GetParent(pDragEntry ) : pDragEntry ;
    return pDestParent != pSrcParent;

}
/* -----------------26.11.98 14:42-------------------
 *
 * --------------------------------------------------*/
sal_Bool  SwGlTreeListBox::NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    sal_uInt32&        rNewChildPos
                                )
{
    pDragEntry = 0;
    if(!pTarget) //An den Anfang verschieben
    {
        pTarget = GetEntry(0);
    }
    // 1. wird in verschiedene Gruppen verschoben?
    // 2. darf in beiden Gruppen geschrieben werden?
    SvLBoxEntry* pSrcParent = GetParent(pEntry);
    SvLBoxEntry* pDestParent =
        GetParent(pTarget) ? GetParent(pTarget) : pTarget;
    sal_Bool bRet = sal_False;
    if(pDestParent != pSrcParent)
    {
        SwGlossaryDlg* pDlg = (SwGlossaryDlg*)Window::GetParent();
        SwWait aWait( *pDlg->pSh->GetView().GetDocShell(), sal_True );

        GroupUserData* pGroupData = (GroupUserData*)pSrcParent->GetUserData();
        String sSourceGroup(pGroupData->sGroupName);
        sSourceGroup += GLOS_DELIM;
        sSourceGroup += String::CreateFromInt32(pGroupData->nPathIdx);
        pDlg->pGlossaryHdl->SetCurGroup(sSourceGroup);
        String sTitle(GetEntryText(pEntry));
        String sShortName(*(String*)pEntry->GetUserData());

        GroupUserData* pDestData = (GroupUserData*)pDestParent->GetUserData();
        String sDestName = pDestData->sGroupName;
        sDestName += GLOS_DELIM;
        sDestName += String::CreateFromInt32(pDestData->nPathIdx);
        bRet = pDlg->pGlossaryHdl->CopyOrMove( sSourceGroup,  sShortName,
                        sDestName, sTitle, sal_True );
        if(bRet)
        {
            SvLBoxEntry* pChild = InsertEntry(sTitle, pDestParent);
            pChild->SetUserData(new String(sShortName));
            GetModel()->Remove(pEntry);
        }
    }
    return sal_False; //sonst wird der Eintrag automatisch vorgenommen
}
/* -----------------26.11.98 14:42-------------------
 *
 * --------------------------------------------------*/
sal_Bool  SwGlTreeListBox::NotifyCopying(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    sal_uInt32&        rNewChildPos
                                )
{
    pDragEntry = 0;
    // 1. wird in verschiedene Gruppen verschoben?
    // 2. darf in beiden Gruppen geschrieben werden?
    if(!pTarget) //An den Anfang verschieben
    {
        pTarget = GetEntry(0);
    }
    SvLBoxEntry* pSrcParent = GetParent(pEntry);
    SvLBoxEntry* pDestParent =
        GetParent(pTarget) ? GetParent(pTarget) : pTarget;
    sal_Bool bRet = sal_False;
    if(pDestParent != pSrcParent)
    {
        SwGlossaryDlg* pDlg = (SwGlossaryDlg*)Window::GetParent();
        SwWait aWait( *pDlg->pSh->GetView().GetDocShell(), sal_True );

        GroupUserData* pGroupData = (GroupUserData*)pSrcParent->GetUserData();
        String sSourceGroup(pGroupData->sGroupName);
        sSourceGroup += GLOS_DELIM;
        sSourceGroup += String::CreateFromInt32(pGroupData->nPathIdx);

        pDlg->pGlossaryHdl->SetCurGroup(sSourceGroup);
        String sTitle(GetEntryText(pEntry));
        String sShortName(*(String*)pEntry->GetUserData());

        GroupUserData* pDestData = (GroupUserData*)pDestParent->GetUserData();
        String sDestName = pDestData->sGroupName;
        sDestName += GLOS_DELIM;
        sDestName += String::CreateFromInt32(pDestData->nPathIdx);

        bRet = pDlg->pGlossaryHdl->CopyOrMove( sSourceGroup,  sShortName,
                        sDestName, sTitle, sal_False );
        if(bRet)
        {
            SvLBoxEntry* pChild = InsertEntry(sTitle, pDestParent);
            pChild->SetUserData(new String(sShortName));
        }
    }
    return sal_False; //sonst wird der Eintrag automatisch vorgenommen
}


/*-----------------10.06.97 15.18-------------------

--------------------------------------------------*/
String SwGlossaryDlg::GetCurrGrpName() const
{
    SvLBoxEntry* pEntry = aCategoryBox.FirstSelected();
    String sRet;
    if(pEntry)
    {
        pEntry =
            aCategoryBox.GetParent(pEntry) ? aCategoryBox.GetParent(pEntry) : pEntry;
        GroupUserData* pGroupData = (GroupUserData*)pEntry->GetUserData();
        sRet = pGroupData->sGroupName;
        sRet += GLOS_DELIM;
        sRet += String::CreateFromInt32(pGroupData->nPathIdx);
    }
    return sRet;
}

/*-----------------11.06.97 08.17-------------------

--------------------------------------------------*/
IMPL_LINK( SwGlossaryDlg, PathHdl, Button *, pBtn )
{
    SvxMultiPathDialog* pDlg = new SvxMultiPathDialog(pBtn);
    SvtPathOptions aPathOpt;
    String sGlosPath( aPathOpt.GetGlossaryPath() );
    pDlg->SetPath(sGlosPath);
    if(RET_OK == pDlg->Execute())
    {
        String sTmp(pDlg->GetPath());
        if(sTmp != sGlosPath)
        {
            aPathOpt.SetGlossaryPath( sTmp );
            ::GetGlossaries()->UpdateGlosPath( sal_True );
            Init();
        }
    }
    delete pDlg;
    return 0;
}
/* -----------------28.07.99 13:48-------------------

 --------------------------------------------------*/
IMPL_LINK( SwGlossaryDlg, ShowPreviewHdl, CheckBox *, pBox )
{
    sal_Bool bCreated = sal_False;
    if(pBox->IsChecked())
    {
        //create example
        if(!pExampleFrame)
        {
            Link aLink(LINK(this, SwGlossaryDlg, PreviewLoadedHdl));
            pExampleFrame = new SwOneExampleFrame(aExampleWIN, EX_SHOW_ONLINE_LAYOUT, &aLink);
            bCreated = sal_True;
        }
    }

    aExampleWIN.Show(pBox->IsChecked() && !bCreated);
    if(pCurrGlosGroup)
        ShowAutoText(*pCurrGlosGroup, aShortNameEdit.GetText());

    return 0;
};
/* -----------------18.11.99 17:09-------------------

 --------------------------------------------------*/
IMPL_LINK( SwGlossaryDlg, PreviewLoadedHdl, void *, EMPTYARG )
{
    if(pCurrGlosGroup)
        ShowAutoText(*pCurrGlosGroup, aShortNameEdit.GetText());
    aExampleWIN.Show(aShowExampleCB.IsChecked());
    return 0;
};

/* -----------------28.07.99 16:28-------------------

 --------------------------------------------------*/
void SwGlossaryDlg::ShowAutoText(const String& rGroup, const String& rShortName)
{
    if(aExampleWIN.IsVisible())
    {
        if(!_xAutoText.is())
        {
            uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = getProcessServiceFactory();
            //now the AutoText ListBoxes have to be filled

            uno::Reference< uno::XInterface >  xAText = xMgr->createInstance( C2U("com.sun.star.text.AutoTextContainer") );
            _xAutoText = uno::Reference< container::XNameAccess >(xAText, uno::UNO_QUERY);
        }

        //try to make an Undo()
        pExampleFrame->ExecUndo();
        uno::Reference< XTextCursor > & xCrsr = pExampleFrame->GetTextCursor();
        if(xCrsr.is())
        {
            if(rShortName.Len())
            {
                uno::Any aGroup = _xAutoText->getByName(rGroup);
                uno::Reference< XAutoTextGroup >  xGroup = *(uno::Reference< XAutoTextGroup > *)aGroup.getValue();
                OUString uShortName(rShortName);
                if(xGroup->hasByName(uShortName))
                {
                    uno::Any aEntry(xGroup->getByName(uShortName));
                    uno::Reference< XAutoTextEntry >  xEntry = *(uno::Reference< XAutoTextEntry > *)aEntry.getValue();
                    uno::Reference< XTextRange >  xRange(xCrsr, uno::UNO_QUERY);
                    xEntry->applyTo(xRange);
                }
            }
        }
    }
}


