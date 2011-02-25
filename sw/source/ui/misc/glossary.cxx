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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGS
#include <hintids.hxx>

#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <svl/svstdarr.hxx>
#include <svl/stritem.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/fcontnr.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <editeng/acorrcfg.hxx>
#include <sfx2/viewfrm.hxx>
#include <unocrsr.hxx>
#include <unotools.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <svl/urihelper.hxx>
#include <unotools/charclass.hxx>
#include <swwait.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <glossary.hxx>
#include <gloshdl.hxx>
#include <glosbib.hxx>
#include <initui.hxx>                   // fuer ::GetGlossaries()
#include <glosdoc.hxx>
#include <macassgn.hxx>
#include <swevent.hxx>
#include <docsh.hxx>
#include <shellio.hxx>

#include <cmdid.h>
#include <helpid.h>
#include <swerror.h>
#include <globals.hrc>
#include <misc.hrc>
#include <glossary.hrc>
#include <swmodule.hxx>
#include <sfx2/filedlghelper.hxx>

#define LONG_LENGTH 60
#define SHORT_LENGTH 30


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::comphelper;
using namespace ::ucbhelper;
using ::rtl::OUString;
using namespace ::sfx2;

String lcl_GetValidShortCut( const String& rName )
{
    const sal_uInt16 nSz = rName.Len();

    if ( 0 == nSz )
        return rName;

    sal_uInt16 nStart = 1;
    while( rName.GetChar( nStart-1 ) == ' ' && nStart < nSz )
        nStart++;

    String aBuf( rName.GetChar( nStart-1 ));

    for( ; nStart < nSz; ++nStart )
    {
        if( rName.GetChar( nStart-1 ) == ' ' && rName.GetChar( nStart ) != ' ')
            aBuf += rName.GetChar( nStart );
    }
    return aBuf;
}

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
    FixedLine       aFL;
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
    aNNFT   (this, SW_RES( FT_NN    )),
    aNewName(this, SW_RES( ED_NN    )),
    aNSFT   (this, SW_RES( FT_NS    )),
    aNewShort(this,SW_RES( ED_NS    )),
    aOk     (this, SW_RES( BT_OKNEW)),
    aCancel (this, SW_RES( BT_CANCEL)),
    aFL    (this, SW_RES( FL_NN    )),
    aONFT   (this, SW_RES( FT_ON    )),
    aOldName(this, SW_RES( ED_ON    )),
    aOSFT   (this, SW_RES( FT_OS    )),
    aOldShort(this,SW_RES( ED_OS    ))
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
    if( ::GetCurrGlosGroup() && ::GetCurrGlosGroup()->Len() )
        return *(::GetCurrGlosGroup());
    return SwGlossaries::GetDefName();
}

void SwGlossaryDlg::SetActGroup(const String &rGrp)
{
    if( !::GetCurrGlosGroup() )
        ::SetCurrGlosGroup( new String );
    *(::GetCurrGlosGroup()) = rGrp;
}

SwGlossaryDlg::SwGlossaryDlg(SfxViewFrame* pViewFrame,
                            SwGlossaryHdl * pGlosHdl, SwWrtShell *pWrtShell) :

    SvxStandardDialog(&pViewFrame->GetWindow(), SW_RES(DLG_GLOSSARY)),

    aInsertTipCB  (this, SW_RES(CB_INSERT_TIP)),
    aNameLbl      (this, SW_RES(FT_NAME)),
    aNameED       (this, SW_RES(ED_NAME)),
    aShortNameLbl (this, SW_RES(FT_SHORTNAME)),
    aShortNameEdit(this, SW_RES(ED_SHORTNAME)),
    aCategoryBox  (this, SW_RES(LB_BIB)),
    aRelativeFL   (this, SW_RES(FL_RELATIVE)),
    aFileRelCB    (this, SW_RES(CB_FILE_REL)),
    aNetRelCB     (this, SW_RES(CB_NET_REL)),
    aExampleWIN   (this, SW_RES(WIN_EXAMPLE )),
    aExampleDummyWIN(this, SW_RES(WIN_EXAMPLE_DUMMY )),
    aShowExampleCB(this, SW_RES(CB_SHOW_EXAMPLE )),
    aInsertBtn    (this, SW_RES(PB_INSERT)),
    aCloseBtn     (this, SW_RES(PB_CLOSE)),
    aHelpBtn      (this, SW_RES(PB_HELP)),
    aEditBtn      (this, SW_RES(PB_EDIT)),
    aBibBtn       (this, SW_RES(PB_BIB)),
    aPathBtn      (this, SW_RES(PB_PATH)),

    sReadonlyPath (SW_RES(ST_READONLY_PATH)),
    pExampleFrame(0),

    pMenu         (new PopupMenu(SW_RES(MNU_EDIT))),
    pGlossaryHdl  (pGlosHdl),

    bResume(sal_False),

    bSelection( pWrtShell->IsSelection() ),
    bReadOnly( sal_False ),
    bIsOld( sal_False ),
    bIsDocReadOnly(sal_False),

    pSh           (pWrtShell)
{
    SvtLinguConfig aLocalLinguConfig;

    // Static-Pointer initialisieren
    if( !::GetCurrGlosGroup() )
        ::SetCurrGlosGroup(new String);//(SwGlossaries::GetDefName());

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

    const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();

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
    aCategoryBox.SetNodeDefaultImages( );

    Init();
}

SwGlossaryDlg::~SwGlossaryDlg()
{
    SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
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
    String *pGlosGroup = ::GetCurrGlosGroup();
    (*pGlosGroup) = pGroupData->sGroupName;
    (*pGlosGroup) += GLOS_DELIM;
    (*pGlosGroup) += String::CreateFromInt32(pGroupData->nPathIdx);
    pGlossaryHdl->SetCurGroup(*pGlosGroup);
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
        ShowAutoText(*::GetCurrGlosGroup(), aShortNameEdit.GetText());
    }
    else
        ShowAutoText(aEmptyStr, aEmptyStr);
    //Controls aktualisieren
    NameModify(&aShortNameEdit);
    if( SfxRequest::HasMacroRecorder( pSh->GetView().GetViewFrame() ) )
    {
        SfxRequest aReq( pSh->GetView().GetViewFrame(), FN_SET_ACT_GLOSSARY );
        String sTemp(*::GetCurrGlosGroup());
        // der nullte Pfad wird nicht aufgezeichnet!
        if('0' == sTemp.GetToken(1, GLOS_DELIM).GetChar(0))
            sTemp = sTemp.GetToken(0, GLOS_DELIM);
        aReq.AppendItem(SfxStringItem(FN_SET_ACT_GLOSSARY, sTemp));
        aReq.Done();
    }
    return 0;
}

void SwGlossaryDlg::Apply()
{
    const String aGlosName(aShortNameEdit.GetText());
    if(aGlosName.Len()) pGlossaryHdl->InsertGlossary(aGlosName);
    if( SfxRequest::HasMacroRecorder( pSh->GetView().GetViewFrame() ) )
    {
        SfxRequest aReq( pSh->GetView().GetViewFrame(), FN_INSERT_GLOSSARY );
        String sTemp(*::GetCurrGlosGroup());
        // der nullte Pfad wird nicht aufgezeichnet!
        if('0' == sTemp.GetToken(1, GLOS_DELIM).GetChar(0))
            sTemp = sTemp.GetToken(0, GLOS_DELIM);
        aReq.AppendItem(SfxStringItem(FN_INSERT_GLOSSARY, sTemp));
        aReq.AppendItem(SfxStringItem(FN_PARAM_1, aGlosName));
        aReq.Done();
    }
}

/* inline */ void SwGlossaryDlg::EnableShortName(sal_Bool bOn)
{
    aShortNameLbl.Enable(bOn);
    aShortNameEdit.Enable(bOn);
}

/* --------------------------------------------------
 * existiert der Titel in der ausgewaehlten Gruppe?
 * --------------------------------------------------*/
SvLBoxEntry* SwGlossaryDlg::DoesBlockExist(const String& rBlock,
                const String& rShort)
{
    //evtl. vorhandenen Eintrag in der TreeListBox suchen
    SvLBoxEntry* pEntry = aCategoryBox.FirstSelected();
    if(pEntry)
    {
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
    }
    return 0;
}

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
            aShortNameEdit.SetText( lcl_GetValidShortCut( aName ) );
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

IMPL_LINK_INLINE_START( SwGlossaryDlg, NameDoubleClick, SvTreeListBox*, pBox )
{
    SvLBoxEntry* pEntry = pBox->FirstSelected();
    if(pBox->GetParent(pEntry) && !bIsDocReadOnly)
        EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SwGlossaryDlg, NameDoubleClick, SvTreeListBox*, EMPTYARG )

IMPL_LINK( SwGlossaryDlg, EnableHdl, Menu *, pMn )
{
    const String aEditText(aNameED.GetText());
    const sal_Bool bHasEntry = aEditText.Len() && aShortNameEdit.GetText().Len();
    const sal_Bool bExists = 0 != DoesBlockExist(aEditText, aShortNameEdit.GetText());
    pMn->EnableItem(FN_GL_DEFINE, bSelection && bHasEntry && !bExists);
    pMn->EnableItem(FN_GL_DEFINE_TEXT, bSelection && bHasEntry && !bExists);
    pMn->EnableItem(FN_GL_COPY_TO_CLIPBOARD, bExists);
    pMn->EnableItem(FN_GL_REPLACE, bSelection && bExists && !bIsOld );
    pMn->EnableItem(FN_GL_REPLACE_TEXT, bSelection && bExists && !bIsOld );
    pMn->EnableItem(FN_GL_EDIT, bExists );
    pMn->EnableItem(FN_GL_RENAME, bExists  );
    pMn->EnableItem(FN_GL_DELETE, bExists  );
    pMn->EnableItem(FN_GL_MACRO, bExists && !bIsOld  &&
                                    !pGlossaryHdl->IsReadOnly() );

    SvLBoxEntry* pEntry = aCategoryBox.FirstSelected();
    sal_Bool bEnable = sal_False;
    if ( pEntry )
        bEnable = !aCategoryBox.GetParent( pEntry ) && !bIsOld && !pGlossaryHdl->IsReadOnly();
    pMn->EnableItem( FN_GL_IMPORT, bEnable );
    return 1;
}

IMPL_LINK( SwGlossaryDlg, MenuHdl, Menu *, pMn )
{
    sal_Bool bNoAttr = sal_False;

    switch(pMn->GetCurItemId())
    {
        case FN_GL_REPLACE:
        case FN_GL_REPLACE_TEXT:
            pGlossaryHdl->NewGlossary(  aNameED.GetText(),
                                        aShortNameEdit.GetText(),
                                        sal_False,
                                        pMn->GetCurItemId() == FN_GL_REPLACE_TEXT);
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

                if( SfxRequest::HasMacroRecorder( pSh->GetView().GetViewFrame() ) )
                {
                    SfxRequest aReq(pSh->GetView().GetViewFrame(), FN_NEW_GLOSSARY);
                    String sTemp(*::GetCurrGlosGroup());
                    // der nullte Pfad wird nicht aufgezeichnet!
                    if('0' == sTemp.GetToken(1, GLOS_DELIM).GetChar(0))
                        sTemp = sTemp.GetToken(0, GLOS_DELIM);
                    aReq.AppendItem(SfxStringItem(FN_NEW_GLOSSARY, sTemp));
                    aReq.AppendItem(SfxStringItem(FN_PARAM_1, aShortName));
                    aReq.AppendItem(SfxStringItem(FN_PARAM_2, aStr));
                    aReq.Done();
                }
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
            if( RET_OK == pNewNameDlg->Execute() &&
                pGlossaryHdl->Rename( aShortNameEdit.GetText(),
                                        pNewNameDlg->GetNewShort(),
                                        pNewNameDlg->GetNewName()))
            {
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
                    OSL_ENSURE(pChild, "entry not found!");
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
            SfxItemSet aSet( pSh->GetAttrPool(), RES_FRMMACRO, RES_FRMMACRO, SID_EVENTCONFIG, SID_EVENTCONFIG, 0 );

            SvxMacro aStart(aEmptyStr, aEmptyStr, STARBASIC);
            SvxMacro aEnd(aEmptyStr, aEmptyStr, STARBASIC);
            pGlossaryHdl->GetMacros(aShortNameEdit.GetText(), aStart, aEnd );

            SvxMacroItem aItem(RES_FRMMACRO);
            if( aStart.GetMacName().Len() )
                aItem.SetMacro( SW_EVENT_START_INS_GLOSSARY, aStart );
            if( aEnd.GetMacName().Len() )
                aItem.SetMacro( SW_EVENT_END_INS_GLOSSARY, aEnd );

            aSet.Put( aItem );
            aSet.Put( SwMacroAssignDlg::AddEvents( MACASSGN_AUTOTEXT ) );

            const SfxPoolItem* pItem;
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            SfxAbstractDialog* pMacroDlg = pFact->CreateSfxDialog( this, aSet,
                pSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface(), SID_EVENTCONFIG );
            if ( pMacroDlg && pMacroDlg->Execute() == RET_OK &&
                SFX_ITEM_SET == pMacroDlg->GetOutputItemSet()->GetItemState( RES_FRMMACRO, sal_False, &pItem ) )
            {
                const SvxMacroTableDtor& rTbl = ((SvxMacroItem*)pItem)->GetMacroTable();
                pGlossaryHdl->SetMacros( aShortNameEdit.GetText(),
                                            rTbl.Get( SW_EVENT_START_INS_GLOSSARY ),
                                            rTbl.Get( SW_EVENT_END_INS_GLOSSARY ) );
            }

            delete pMacroDlg;
        }
        break;

        case FN_GL_IMPORT:
        {
            // call the FileOpenDialog do find WinWord - Files with templates
            FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
            uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

            SvtPathOptions aPathOpt;
            xFP->setDisplayDirectory(aPathOpt.GetWorkPath() );
            String sWW8( C2S( FILTER_WW8 ) );

            uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
            SfxFilterMatcher aMatcher( String::CreateFromAscii(SwDocShell::Factory().GetShortName()) );
            SfxFilterMatcherIter aIter( &aMatcher );
            const SfxFilter* pFilter = aIter.First();
            while ( pFilter )
            {
                if( pFilter->GetUserData() == sWW8 )
                {
                    xFltMgr->appendFilter( pFilter->GetUIName(),
                                ((WildCard&)pFilter->GetWildcard()).GetWildCard() );
                    xFltMgr->setCurrentFilter( pFilter->GetUIName() ) ;
                }

                pFilter = aIter.Next();
            }

            if( aDlgHelper.Execute() == ERRCODE_NONE )
            {
                if( pGlossaryHdl->ImportGlossaries( xFP->getFiles().getConstArray()[0] ))
                    Init();
                else
                {
                    InfoBox(this, SW_RES( MSG_NO_GLOSSARIES )).Execute();
                }
            }
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
        String sGlosPath( aPathOpt.GetAutoTextPath() );
        USHORT nPaths = sGlosPath.GetTokenCount(';');
        BOOL bIsWritable = FALSE;
        for(USHORT nPath = 0; nPath < nPaths; nPath++)
        {
            String sPath = URIHelper::SmartRel2Abs(
                INetURLObject(), sGlosPath.GetToken(nPath, ';'),
                URIHelper::GetMaybeFileHdl());
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
            catch(Exception&)
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
    const String sSelStr(::GetCurrGlosGroup()->GetToken(0, GLOS_DELIM));
    const sal_uInt16 nSelPath = static_cast< sal_uInt16 >(::GetCurrGlosGroup()->GetToken(1, GLOS_DELIM).ToInt32());
    // #i66304# - "My AutoText" comes from mytexts.bau, but should be translated
    const String sMyAutoTextEnglish(RTL_CONSTASCII_USTRINGPARAM("My AutoText"));
    const String sMyAutoTextTranslated(SW_RESSTR(STR_MY_AUTOTEXT));
    for(sal_uInt16 nId = 0; nId < nCnt; ++nId )
    {
        String sTitle;
        String sGroupName(pGlossaryHdl->GetGroupName(nId, &sTitle));
        if(!sGroupName.Len())
            continue;
        if(!sTitle.Len())
            sTitle = sGroupName.GetToken( 0, GLOS_DELIM );
        if(sTitle == sMyAutoTextEnglish)
            sTitle = sMyAutoTextTranslated;
        SvLBoxEntry* pEntry = aCategoryBox.InsertEntry( sTitle );
        sal_uInt16 nPath = static_cast< sal_uInt16 >(sGroupName.GetToken( 1, GLOS_DELIM ).ToInt32());

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
                String sGroupTitle(pGlossaryHdl->GetGlossaryName(i));
                SvLBoxEntry* pChild = aCategoryBox.InsertEntry(
                                    sGroupTitle, pEntry);
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

    const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
    aFileRelCB.Check( pCfg->IsSaveRelFile() );
    aFileRelCB.SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    aNetRelCB.Check( pCfg->IsSaveRelNet() );
    aNetRelCB.SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    aInsertTipCB.Check( pCfg->IsAutoTextTip() );
    aInsertTipCB.SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
}

IMPL_LINK_INLINE_START( SwGlossaryDlg, EditHdl, Button *, EMPTYARG )
{
//EndDialog darf nicht im MenuHdl aufgerufen werden
    if(aEditBtn.GetCurItemId() == FN_GL_EDIT )
    {
        SwTextBlocks *pGroup = ::GetGlossaries()->GetGroupDoc (  GetCurrGrpName () );
        BOOL bRet = pGlossaryHdl->ConvertToNew ( *pGroup );
        delete pGroup;
        if ( bRet )
            EndDialog(RET_EDIT);
    }
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
        aNewShort.SetText( lcl_GetValidShortCut( aName ) );

    sal_Bool bEnable = aName.Len() && aNewShort.GetText().Len() &&
        (!pDlg->DoesBlockExist(aName, aNewShort.GetText())
            || aName == aOldName.GetText());
    aOk.Enable(bEnable);
    return 0;
}

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

IMPL_LINK( SwGlossaryDlg, CheckBoxHdl, CheckBox *, pBox )
{
    SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
    sal_Bool bCheck = pBox->IsChecked();
    if( pBox == &aInsertTipCB )
        pCfg->SetAutoTextTip(bCheck);
    else if(pBox == &aFileRelCB)
        pCfg->SetSaveRelFile(bCheck);
    else
        pCfg->SetSaveRelNet(bCheck);
    return 0;
}

/* --------------------------------------------------
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
            aPos = GetEntryPosition( pEntry );
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

DragDropMode SwGlTreeListBox::NotifyStartDrag(
                    TransferDataContainer& /*rContainer*/,
                    SvLBoxEntry* pEntry )
{
    DragDropMode  eRet;
    pDragEntry = pEntry;
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
        sal_Int8 nDragOption = DND_ACTION_COPY;
        eRet = SV_DRAGDROP_CTRL_COPY;
        if(!pDlg->pGlossaryHdl->IsReadOnly(&sEntry))
        {
            eRet |= SV_DRAGDROP_CTRL_MOVE;
            nDragOption |= DND_ACTION_MOVE;
        }
        SetDragOptions( nDragOption );
    }
    return eRet;
}

sal_Bool    SwGlTreeListBox::NotifyAcceptDrop( SvLBoxEntry* pEntry)
{
    // TODO: Readonly - Ueberpruefung fehlt noch!
    SvLBoxEntry* pSrcParent = GetParent(pEntry) ? GetParent(pEntry) : pEntry;
    SvLBoxEntry* pDestParent =
        GetParent(pDragEntry ) ? GetParent(pDragEntry ) : pDragEntry ;
    return pDestParent != pSrcParent;

}

sal_Bool  SwGlTreeListBox::NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& /*rpNewParent*/,
                                    ULONG&        /*rNewChildPos*/
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

sal_Bool  SwGlTreeListBox::NotifyCopying(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& /*rpNewParent*/,
                                    ULONG&        /*rNewChildPos*/
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

IMPL_LINK( SwGlossaryDlg, PathHdl, Button *, pBtn )
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        AbstractSvxMultiPathDialog* pDlg = pFact->CreateSvxMultiPathDialog( pBtn );
        OSL_ENSURE(pDlg, "Dialogdiet fail!");
        SvtPathOptions aPathOpt;
        String sGlosPath( aPathOpt.GetAutoTextPath() );
        pDlg->SetPath(sGlosPath);
        if(RET_OK == pDlg->Execute())
        {
            String sTmp(pDlg->GetPath());
            if(sTmp != sGlosPath)
            {
                aPathOpt.SetAutoTextPath( sTmp );
                ::GetGlossaries()->UpdateGlosPath( sal_True );
                Init();
            }
        }
        delete pDlg;
    }
    return 0;
}

IMPL_LINK( SwGlossaryDlg, ShowPreviewHdl, CheckBox *, pBox )
{
    sal_Bool bCreated = sal_False;
    if(pBox->IsChecked())
    {
        //create example
        if(!pExampleFrame)
        {
            Link aLink(LINK(this, SwGlossaryDlg, PreviewLoadedHdl));
            pExampleFrame = new SwOneExampleFrame( aExampleWIN,
                            EX_SHOW_ONLINE_LAYOUT, &aLink );
            bCreated = sal_True;
        }
    }

    BOOL bShow = pBox->IsChecked() && !bCreated;
    aExampleWIN.Show( bShow );
    aExampleDummyWIN.Show(!bShow);
    if( ::GetCurrGlosGroup() )
        ShowAutoText(*::GetCurrGlosGroup(), aShortNameEdit.GetText());

    return 0;
};

IMPL_LINK( SwGlossaryDlg, PreviewLoadedHdl,  void *, EMPTYARG )
{
    BOOL bShow = aShowExampleCB.IsChecked();
    aExampleWIN.Show( bShow );
    aExampleDummyWIN.Show(!bShow);
    ResumeShowAutoText();
    return 0;
}

void SwGlossaryDlg::ShowAutoText(const String& rGroup, const String& rShortName)
{
    if(aExampleWIN.IsVisible())
    {
        SetResumeData(rGroup, rShortName);
        //try to make an Undo()
        pExampleFrame->ClearDocument( TRUE );
    }
}

void    SwGlossaryDlg::ResumeShowAutoText()
{
    String sGroup, sShortName;
    if(GetResumeData(sGroup, sShortName) && aExampleWIN.IsVisible())
    {
        if(!_xAutoText.is())
        {
            uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = getProcessServiceFactory();
            //now the AutoText ListBoxes have to be filled

            uno::Reference< uno::XInterface >  xAText = xMgr->createInstance( C2U("com.sun.star.text.AutoTextContainer") );
            _xAutoText = uno::Reference< container::XNameAccess >(xAText, uno::UNO_QUERY);
        }

        uno::Reference< XTextCursor > & xCrsr = pExampleFrame->GetTextCursor();
        if(xCrsr.is())
        {
            if(sShortName.Len())
            {
                uno::Any aGroup = _xAutoText->getByName(sGroup);
                uno::Reference< XAutoTextGroup >  xGroup;
                OUString uShortName(sShortName);
                if((aGroup >>= xGroup) && xGroup->hasByName(uShortName))
                {
                    uno::Any aEntry(xGroup->getByName(uShortName));
                    uno::Reference< XAutoTextEntry >  xEntry;
                    aEntry >>= xEntry;
                    uno::Reference< XTextRange >  xRange(xCrsr, uno::UNO_QUERY);
                    xEntry->applyTo(xRange);
                }
            }
        }
    }
    ResetResumeData();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
