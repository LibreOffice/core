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

#include <hintids.hxx>

#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
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
#include <comphelper/string.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/text/AutoTextContainer.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <svl/urihelper.hxx>
#include <unotools/charclass.hxx>
#include "svtools/treelistentry.hxx"
#include <swwait.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <glossary.hxx>
#include <gloshdl.hxx>
#include <glosbib.hxx>
#include <initui.hxx>                   // for ::GetGlossaries()
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
#include <swmodule.hxx>
#include <sfx2/filedlghelper.hxx>

#include "access.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::ucbhelper;
using namespace ::sfx2;

static String lcl_GetValidShortCut( const String& rName )
{
    const sal_uInt16 nSz = rName.Len();

    if ( 0 == nSz )
        return rName;

    sal_uInt16 nStart = 1;
    while( rName.GetChar( nStart-1 ) == ' ' && nStart < nSz )
        nStart++;

    String aBuf = OUString(rName.GetChar(nStart-1));

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
    bool        bReadonly;

    GroupUserData()
        : nPathIdx(0),
          bReadonly(false)  {}
};

/*------------------------------------------------------------------------
 Description:   dialog for new block name
------------------------------------------------------------------------*/
class SwNewGlosNameDlg : public ModalDialog
{
    Edit*        m_pNewName;
    NoSpaceEdit* m_pNewShort;
    OKButton*    m_pOk;
    Edit*        m_pOldName;
    Edit*        m_pOldShort;

protected:
    DECL_LINK( Modify, Edit * );
    DECL_LINK(Rename, void *);

public:
    SwNewGlosNameDlg( Window* pParent,
                      const String& rOldName,
                      const String& rOldShort );

    String GetNewName()  const { return m_pNewName->GetText(); }
    String GetNewShort() const { return m_pNewShort->GetText(); }
};

SwNewGlosNameDlg::SwNewGlosNameDlg(Window* pParent,
                            const String& rOldName,
                            const String& rOldShort )
    : ModalDialog(pParent, "RenameAutoTextDialog",
        "modules/swriter/ui/renameautotextdialog.ui")
{
    get(m_pNewName, "newname");
    get(m_pNewShort, "newsc");
    get(m_pOk, "ok");
    get(m_pOldName, "oldname");
    get(m_pOldShort, "oldsc");

    m_pOldName->SetText( rOldName );
    m_pOldShort->SetText( rOldShort );
    m_pNewName->SetModifyHdl(LINK(this, SwNewGlosNameDlg, Modify ));
    m_pNewShort->SetModifyHdl(LINK(this, SwNewGlosNameDlg, Modify ));
    m_pOk->SetClickHdl(LINK(this, SwNewGlosNameDlg, Rename ));
    m_pNewName->GrabFocus();
}

/*------------------------------------------------------------------------
 Description:   query / set currently set group
------------------------------------------------------------------------*/
String SwGlossaryDlg::GetCurrGroup()
{
    if( !::GetCurrGlosGroup().isEmpty() )
        return ::GetCurrGlosGroup();
    return SwGlossaries::GetDefName();
}

void SwGlossaryDlg::SetActGroup(const String &rGrp)
{
    ::SetCurrGlosGroup(rGrp);
}

SwGlossaryDlg::SwGlossaryDlg(SfxViewFrame* pViewFrame,
                            SwGlossaryHdl * pGlosHdl, SwWrtShell *pWrtShell)
    : SvxStandardDialog(&pViewFrame->GetWindow(), "AutoTextDialog",
        "modules/swriter/ui/autotext.ui")
    , sReadonlyPath(SW_RESSTR(STR_READONLY_PATH))
    , pExampleFrame(0)
    , pGlossaryHdl(pGlosHdl)
    , bResume(false)
    , bSelection(pWrtShell->IsSelection())
    , bReadOnly(false)
    , bIsOld(false)
    , bIsDocReadOnly(false)
    , pSh(pWrtShell)
{
    get(m_pInsertTipCB, "inserttip");
    get(m_pNameED, "name");
    get(m_pShortNameLbl, "shortnameft");
    get(m_pShortNameEdit, "shortname");
    get(m_pCategoryBox, "category");
    get(m_pFileRelCB, "relfile");
    get(m_pNetRelCB, "relnet");
    get(m_pInsertBtn, "insert");
    get(m_pBibBtn, "categories");
    get(m_pPathBtn, "path");
    get(m_pExampleWIN, "example");
    get(m_pEditBtn, "autotext");

    SvtLinguConfig aLocalLinguConfig;

    PopupMenu *pMenu = m_pEditBtn->GetPopupMenu();
    assert(pMenu);
    pMenu->SetActivateHdl(LINK(this,SwGlossaryDlg,EnableHdl));
    pMenu->SetSelectHdl(LINK(this,SwGlossaryDlg,MenuHdl));
    m_pEditBtn->SetSelectHdl(LINK(this,SwGlossaryDlg,EditHdl));
    m_pPathBtn->SetClickHdl(LINK(this, SwGlossaryDlg, PathHdl));

    m_pNameED->SetModifyHdl(LINK(this,SwGlossaryDlg,NameModify));
    m_pShortNameEdit->SetModifyHdl(LINK(this,SwGlossaryDlg,NameModify));

    m_pCategoryBox->SetDoubleClickHdl(LINK(this,SwGlossaryDlg, NameDoubleClick));
    m_pCategoryBox->SetSelectHdl(LINK(this,SwGlossaryDlg,GrpSelect));
    m_pBibBtn->SetClickHdl(LINK(this,SwGlossaryDlg,BibHdl));

    m_pInsertBtn->SetClickHdl(LINK(this,SwGlossaryDlg,InsertHdl));

    ShowPreview();

    bIsDocReadOnly = pSh->GetView().GetDocShell()->IsReadOnly() ||
                      pSh->HasReadonlySel();
    if( bIsDocReadOnly )
        m_pInsertBtn->Enable(sal_False);
    m_pNameED->GrabFocus();
    m_pCategoryBox->SetStyle(m_pCategoryBox->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL|WB_VSCROLL|WB_CLIPCHILDREN|WB_SORT);
    m_pCategoryBox->GetModel()->SetSortMode(SortAscending);
    m_pCategoryBox->SetHighlightRange();   // select over full width
    m_pCategoryBox->SetNodeDefaultImages( );

    Init();
}

SwGlossaryDlg::~SwGlossaryDlg()
{
    m_pCategoryBox->Clear();
    delete pExampleFrame;
}

/*------------------------------------------------------------------------
 Description:   select new group
------------------------------------------------------------------------*/
IMPL_LINK( SwGlossaryDlg, GrpSelect, SvTreeListBox *, pBox )
{
    SvTreeListEntry* pEntry = pBox->FirstSelected();
    if(!pEntry)
        return 0;
    SvTreeListEntry* pParent = pBox->GetParent(pEntry) ? pBox->GetParent(pEntry) : pEntry;
    GroupUserData* pGroupData = (GroupUserData*)pParent->GetUserData();
    ::SetCurrGlosGroup(pGroupData->sGroupName
        + OUString(GLOS_DELIM)
        + OUString::number(pGroupData->nPathIdx));
    pGlossaryHdl->SetCurGroup(::GetCurrGlosGroup());
    // set current text block
    bReadOnly = pGlossaryHdl->IsReadOnly();
    EnableShortName( !bReadOnly );
    m_pEditBtn->Enable(!bReadOnly);
    bIsOld = pGlossaryHdl->IsOld();
    if( pParent != pEntry)
    {
        OUString aName(pBox->GetEntryText(pEntry));
        m_pNameED->SetText(aName);
        m_pShortNameEdit->SetText(*(String*)pEntry->GetUserData());
        pEntry = pBox->GetParent(pEntry);
        m_pInsertBtn->Enable( !bIsDocReadOnly);
        ShowAutoText(::GetCurrGlosGroup(), m_pShortNameEdit->GetText());
    }
    else
        ShowAutoText(aEmptyStr, aEmptyStr);
    // update controls
    NameModify(m_pShortNameEdit);
    if( SfxRequest::HasMacroRecorder( pSh->GetView().GetViewFrame() ) )
    {
        SfxRequest aReq( pSh->GetView().GetViewFrame(), FN_SET_ACT_GLOSSARY );
        OUString sTemp(::GetCurrGlosGroup());
        // the zeroth path is not being recorded!
        if (sTemp.getToken(1, GLOS_DELIM).startsWith("0"))
            sTemp = sTemp.getToken(0, GLOS_DELIM);
        aReq.AppendItem(SfxStringItem(FN_SET_ACT_GLOSSARY, sTemp));
        aReq.Done();
    }
    return 0;
}

void SwGlossaryDlg::Apply()
{
    const String aGlosName(m_pShortNameEdit->GetText());
    if(aGlosName.Len()) pGlossaryHdl->InsertGlossary(aGlosName);
    if( SfxRequest::HasMacroRecorder( pSh->GetView().GetViewFrame() ) )
    {
        SfxRequest aReq( pSh->GetView().GetViewFrame(), FN_INSERT_GLOSSARY );
        OUString sTemp(::GetCurrGlosGroup());
        // the zeroth path is not being recorded!
        if (sTemp.getToken(1, GLOS_DELIM).startsWith("0"))
            sTemp = sTemp.getToken(0, GLOS_DELIM);
        aReq.AppendItem(SfxStringItem(FN_INSERT_GLOSSARY, sTemp));
        aReq.AppendItem(SfxStringItem(FN_PARAM_1, aGlosName));
        aReq.Done();
    }
}

void SwGlossaryDlg::EnableShortName(sal_Bool bOn)
{
    m_pShortNameLbl->Enable(bOn);
    m_pShortNameEdit->Enable(bOn);
}

/* --------------------------------------------------
 * does the title exist in the selected group?
 * --------------------------------------------------*/
SvTreeListEntry* SwGlossaryDlg::DoesBlockExist(const String& rBlock,
                const String& rShort)
{
    // look for possible entry in TreeListBox
    SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();
    if(pEntry)
    {
        if(m_pCategoryBox->GetParent(pEntry))
            pEntry = m_pCategoryBox->GetParent(pEntry);
        sal_uInt32 nChildCount = m_pCategoryBox->GetChildCount( pEntry );
        for(sal_uInt32 i = 0; i < nChildCount; i++)
        {
            SvTreeListEntry* pChild = m_pCategoryBox->GetEntry( pEntry, i );
            if(rBlock == m_pCategoryBox->GetEntryText(pChild) &&
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
    String aName(m_pNameED->GetText());
    bool bNameED = pEdit == m_pNameED;
    if( !aName.Len() )
    {
        if(bNameED)
            m_pShortNameEdit->SetText(aName);
        m_pInsertBtn->Enable(sal_False);
        return 0;
    }
    String sShortSearch;
    if(!bNameED)
        sShortSearch = pEdit->GetText();
    bool bNotFound = !DoesBlockExist(aName, sShortSearch);
    if(bNameED)
    {
            // did the text get in to the Listbbox in the Edit with a click?
        if(bNotFound)
        {
            m_pShortNameEdit->SetText( lcl_GetValidShortCut( aName ) );
            EnableShortName();
        }
        else
        {
            m_pShortNameEdit->SetText(pGlossaryHdl->GetGlossaryShortName(aName));
            EnableShortName(!bReadOnly);
        }
        m_pInsertBtn->Enable(!bNotFound && !bIsDocReadOnly);
    }
    else
    {
        //ShortNameEdit
        if(!bNotFound)
        {
            sal_Bool bEnable = !bNotFound;
            bEnable &= !bIsDocReadOnly;
            m_pInsertBtn->Enable(bEnable);
        }
    }
    return 0;
}

IMPL_LINK_INLINE_START( SwGlossaryDlg, NameDoubleClick, SvTreeListBox*, pBox )
{
    SvTreeListEntry* pEntry = pBox->FirstSelected();
    if(pBox->GetParent(pEntry) && !bIsDocReadOnly)
        EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SwGlossaryDlg, NameDoubleClick, SvTreeListBox*, pBox )

IMPL_LINK( SwGlossaryDlg, EnableHdl, Menu *, pMn )
{
    SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();

    const String aEditText(m_pNameED->GetText());
    const bool bHasEntry = aEditText.Len() && !m_pShortNameEdit->GetText().isEmpty();
    const bool bExists = 0 != DoesBlockExist(aEditText, m_pShortNameEdit->GetText());
    const bool bIsGroup = pEntry && !m_pCategoryBox->GetParent(pEntry);
    pMn->EnableItem("new", bSelection && bHasEntry && !bExists);
    pMn->EnableItem("newtext", bSelection && bHasEntry && !bExists);
    pMn->EnableItem("copy", bExists && !bIsGroup);
    pMn->EnableItem("replace", bSelection && bExists && !bIsGroup && !bIsOld );
    pMn->EnableItem("replacetext", bSelection && bExists && !bIsGroup && !bIsOld );
    pMn->EnableItem("edit", bExists && !bIsGroup );
    pMn->EnableItem("rename", bExists && !bIsGroup );
    pMn->EnableItem("delete", bExists && !bIsGroup );
    pMn->EnableItem("macro", bExists && !bIsGroup && !bIsOld &&
                                    !pGlossaryHdl->IsReadOnly() );
    pMn->EnableItem("import", bIsGroup && !bIsOld && !pGlossaryHdl->IsReadOnly() );
    return 1;
}

IMPL_LINK( SwGlossaryDlg, MenuHdl, Menu *, pMn )
{
    OString sItemIdent(pMn->GetCurItemIdent());

    if (sItemIdent == "replace")
    {
        pGlossaryHdl->NewGlossary(m_pNameED->GetText(),
                                  m_pShortNameEdit->GetText(),
                                  false, false);
    }
    else if (sItemIdent == "replacetext")
    {
        pGlossaryHdl->NewGlossary(m_pNameED->GetText(),
                                  m_pShortNameEdit->GetText(),
                                  false, true);
    }
    else if (sItemIdent == "new" || sItemIdent == "newtext")
    {
        bool bNoAttr = sItemIdent == "newtext";

        const String aStr(m_pNameED->GetText());
        const String aShortName(m_pShortNameEdit->GetText());
        if(pGlossaryHdl->HasShortName(aShortName))
        {
            InfoBox(this, SW_RES(MSG_DOUBLE_SHORTNAME)).Execute();
            m_pShortNameEdit->SetSelection(Selection(0, SELECTION_MAX));
            m_pShortNameEdit->GrabFocus();
            return 1;
        }
        if(pGlossaryHdl->NewGlossary(aStr, aShortName, sal_False, bNoAttr ))
        {
            SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();
            if(m_pCategoryBox->GetParent(pEntry))
                pEntry = m_pCategoryBox->GetParent(pEntry);

            SvTreeListEntry* pChild = m_pCategoryBox->InsertEntry(aStr, pEntry);
            pChild->SetUserData(new String(aShortName));
            m_pNameED->SetText(aStr);
            m_pShortNameEdit->SetText(aShortName);
            NameModify(m_pNameED);       // for toggling the buttons

            if( SfxRequest::HasMacroRecorder( pSh->GetView().GetViewFrame() ) )
            {
                SfxRequest aReq(pSh->GetView().GetViewFrame(), FN_NEW_GLOSSARY);
                OUString sTemp(::GetCurrGlosGroup());
                // the zeroth path is not being recorded!
                if (sTemp.getToken(1, GLOS_DELIM).startsWith("0"))
                    sTemp = sTemp.getToken(0, GLOS_DELIM);
                aReq.AppendItem(SfxStringItem(FN_NEW_GLOSSARY, sTemp));
                aReq.AppendItem(SfxStringItem(FN_PARAM_1, aShortName));
                aReq.AppendItem(SfxStringItem(FN_PARAM_2, aStr));
                aReq.Done();
            }
        }
    }
    else if (sItemIdent == "copy")
    {
        pGlossaryHdl->CopyToClipboard(*pSh, m_pShortNameEdit->GetText());
    }
    else if (sItemIdent == "rename")
    {
        m_pShortNameEdit->SetText(pGlossaryHdl->GetGlossaryShortName(m_pNameED->GetText()));
        SwNewGlosNameDlg* pNewNameDlg = new SwNewGlosNameDlg(this, m_pNameED->GetText(),
                                        m_pShortNameEdit->GetText() );
        if( RET_OK == pNewNameDlg->Execute() &&
            pGlossaryHdl->Rename( m_pShortNameEdit->GetText(),
                                    pNewNameDlg->GetNewShort(),
                                    pNewNameDlg->GetNewName()))
        {
            SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();
            SvTreeListEntry* pNewEntry = m_pCategoryBox->InsertEntry(
                    pNewNameDlg->GetNewName(), m_pCategoryBox->GetParent(pEntry));
            pNewEntry->SetUserData(new String(pNewNameDlg->GetNewShort()));
            delete (String*)pEntry->GetUserData();
            m_pCategoryBox->GetModel()->Remove(pEntry);
            m_pCategoryBox->Select(pNewEntry);
            m_pCategoryBox->MakeVisible(pNewEntry);
        }
        GrpSelect(m_pCategoryBox);
        delete pNewNameDlg;
    }
    else if (sItemIdent == "delete")
    {
        QueryBox aQuery(this, SW_RES(MSG_QUERY_DELETE));
        if(RET_YES == aQuery.Execute())
        {
            const String aShortName(m_pShortNameEdit->GetText());
            const String aTitle(m_pNameED->GetText());
            if(aTitle.Len() && pGlossaryHdl->DelGlossary(aShortName))
            {
                SvTreeListEntry* pChild = DoesBlockExist(aTitle, aShortName);
                OSL_ENSURE(pChild, "entry not found!");
                SvTreeListEntry* pParent = m_pCategoryBox->GetParent(pChild);
                m_pCategoryBox->Select(pParent);

                m_pCategoryBox->GetModel()->Remove(pChild);
                m_pNameED->SetText(OUString());
                NameModify(m_pNameED);
            }
        }
    }
    else if (sItemIdent == "macro")
    {
        SfxItemSet aSet( pSh->GetAttrPool(), RES_FRMMACRO, RES_FRMMACRO, SID_EVENTCONFIG, SID_EVENTCONFIG, 0 );

        SvxMacro aStart(aEmptyStr, aEmptyStr, STARBASIC);
        SvxMacro aEnd(aEmptyStr, aEmptyStr, STARBASIC);
        pGlossaryHdl->GetMacros(m_pShortNameEdit->GetText(), aStart, aEnd );

        SvxMacroItem aItem(RES_FRMMACRO);
        if( aStart.HasMacro() )
            aItem.SetMacro( SW_EVENT_START_INS_GLOSSARY, aStart );
        if( aEnd.HasMacro() )
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
            pGlossaryHdl->SetMacros( m_pShortNameEdit->GetText(),
                                        rTbl.Get( SW_EVENT_START_INS_GLOSSARY ),
                                        rTbl.Get( SW_EVENT_END_INS_GLOSSARY ) );
        }

        delete pMacroDlg;
    }
    else if (sItemIdent == "import")
    {
        // call the FileOpenDialog do find WinWord - Files with templates
        FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
        uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

        SvtPathOptions aPathOpt;
        xFP->setDisplayDirectory(aPathOpt.GetWorkPath() );

        uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
        SfxFilterMatcher aMatcher( OUString::createFromAscii(SwDocShell::Factory().GetShortName()) );
        SfxFilterMatcherIter aIter( aMatcher );
        const SfxFilter* pFilter = aIter.First();
        while ( pFilter )
        {
            if( pFilter->GetUserData() == FILTER_WW8 )
            {
                xFltMgr->appendFilter( pFilter->GetUIName(),
                            pFilter->GetWildcard().getGlob() );
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
    else
    {
        return 0;
    }
    return 1;
}

/*--------------------------------------------------------------------
     Description:   dialog manage regions
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwGlossaryDlg, BibHdl)
{
    SwGlossaries* pGloss = ::GetGlossaries();
    if( pGloss->IsGlosPathErr() )
        pGloss->ShowError();
    else
    {
        //check if at least one glossary path is write enabled
        SvtPathOptions aPathOpt;
        String sGlosPath( aPathOpt.GetAutoTextPath() );
        sal_uInt16 nPaths = comphelper::string::getTokenCount(sGlosPath, ';');
        bool bIsWritable = false;
        for(sal_uInt16 nPath = 0; nPath < nPaths; nPath++)
        {
            String sPath = URIHelper::SmartRel2Abs(
                INetURLObject(), sGlosPath.GetToken(nPath, ';'),
                URIHelper::GetMaybeFileHdl());
            try
            {
                Content aTestContent( sPath,
                            uno::Reference< XCommandEnvironment >(),
                            comphelper::getProcessComponentContext() );
                Any aAny = aTestContent.getPropertyValue( "IsReadOnly" );
                if(aAny.hasValue())
                {
                    bIsWritable = !*(sal_Bool*)aAny.getValue();
                }
            }
            catch (const Exception&)
            {
            }
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
                SvTreeListEntry* pEntry = m_pCategoryBox->First();
                while(sNewGroup.Len() && pEntry)
                {
                    if(!m_pCategoryBox->GetParent(pEntry))
                    {
                        GroupUserData* pGroupData = (GroupUserData*)pEntry->GetUserData();
                        String sGroup = pGroupData->sGroupName;
                        sGroup += GLOS_DELIM;
                        sGroup += OUString::number(pGroupData->nPathIdx);
                        if(sGroup == sNewGroup)
                        {
                            m_pCategoryBox->Select(pEntry);
                            m_pCategoryBox->MakeVisible(pEntry);
                            GrpSelect(m_pCategoryBox);
                            break;
                        }
                    }
                    pEntry = m_pCategoryBox->Next(pEntry);
                }

            }
            delete pDlg;
        }
        else
        {
            QueryBox aBox(this, WB_YES_NO, sReadonlyPath);
            if(RET_YES == aBox.Execute())
                PathHdl(m_pPathBtn);
        }
    }
    return 0;
}

/*------------------------------------------------------------------------
 Description:   initialisation; from Ctor and after editing regions
------------------------------------------------------------------------*/
void SwGlossaryDlg::Init()
{
    m_pCategoryBox->SetUpdateMode( sal_False );
    m_pCategoryBox->Clear();
    // display text block regions
    const sal_uInt16 nCnt = pGlossaryHdl->GetGroupCnt();
    SvTreeListEntry* pSelEntry = 0;
    const OUString sSelStr(::GetCurrGlosGroup().getToken(0, GLOS_DELIM));
    const sal_uInt16 nSelPath = static_cast< sal_uInt16 >(::GetCurrGlosGroup().getToken(1, GLOS_DELIM).toInt32());
    // #i66304# - "My AutoText" comes from mytexts.bau, but should be translated
    const OUString sMyAutoTextEnglish("My AutoText");
    const OUString sMyAutoTextTranslated(SW_RES(STR_MY_AUTOTEXT));
    for(sal_uInt16 nId = 0; nId < nCnt; ++nId )
    {
        OUString sTitle;
        OUString sGroupName(pGlossaryHdl->GetGroupName(nId, &sTitle));
        if(sGroupName.isEmpty())
            continue;
        if(sTitle.isEmpty())
            sTitle = sGroupName.getToken( 0, GLOS_DELIM );
        if(sTitle == sMyAutoTextEnglish)
            sTitle = sMyAutoTextTranslated;
        SvTreeListEntry* pEntry = m_pCategoryBox->InsertEntry( sTitle );
        sal_uInt16 nPath = static_cast< sal_uInt16 >(sGroupName.getToken( 1, GLOS_DELIM ).toInt32());

        GroupUserData* pData = new GroupUserData;
        pData->sGroupName = sGroupName.getToken(0, GLOS_DELIM);
        pData->nPathIdx = nPath;
        pData->bReadonly = pGlossaryHdl->IsReadOnly(&sGroupName);

        pEntry->SetUserData(pData);
        if(sSelStr == pData->sGroupName && nSelPath == nPath)
            pSelEntry = pEntry;

        // fill entries for the groups
        {
            pGlossaryHdl->SetCurGroup(sGroupName, sal_False, sal_True);
            const sal_uInt16 nCount = pGlossaryHdl->GetGlossaryCnt();
            for(sal_uInt16 i = 0; i < nCount; ++i)
            {
                String sGroupTitle(pGlossaryHdl->GetGlossaryName(i));
                SvTreeListEntry* pChild = m_pCategoryBox->InsertEntry(
                                    sGroupTitle, pEntry);
                pChild->SetUserData(new String(pGlossaryHdl->GetGlossaryShortName(i)));
            }
        }
    }
        // set current group and display text blocks
    if(!pSelEntry)
    {
        //find a non-readonly group
        SvTreeListEntry* pSearch = m_pCategoryBox->First();
        while(pSearch)
        {
            if(!m_pCategoryBox->GetParent(pSearch))
            {
                GroupUserData* pData = (GroupUserData*)pSearch->GetUserData();
                if(!pData->bReadonly)
                {
                    pSelEntry = pSearch;
                    break;
                }
            }
            pSearch = m_pCategoryBox->Next(pSearch);
        }
        if(!pSelEntry)
            pSelEntry = m_pCategoryBox->GetEntry(0);
    }
    if(pSelEntry)
    {
        m_pCategoryBox->Expand(pSelEntry);
        m_pCategoryBox->Select(pSelEntry);
        m_pCategoryBox->MakeVisible(pSelEntry);
        GrpSelect(m_pCategoryBox);
    }
    //JP 16.11.99: the SvxTreeListBox has a Bug. The Box dont recalc the
    //      outputsize, when all entries are insertet. The result is, that
    //      the Focus/Highlight rectangle is to large and paintet over the
    //      HScrollbar. -> Fix: call the resize
    m_pCategoryBox->Resize();

    m_pCategoryBox->GetModel()->Resort();
    m_pCategoryBox->SetUpdateMode( sal_True );
    m_pCategoryBox->Update();

    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    m_pFileRelCB->Check( rCfg.IsSaveRelFile() );
    m_pFileRelCB->SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    m_pNetRelCB->Check( rCfg.IsSaveRelNet() );
    m_pNetRelCB->SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    m_pInsertTipCB->Check( rCfg.IsAutoTextTip() );
    m_pInsertTipCB->SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
}

IMPL_LINK_NOARG_INLINE_START(SwGlossaryDlg, EditHdl)
{
// EndDialog must not be called in MenuHdl
    if (m_pEditBtn->GetCurItemIdent() == "edit")
    {
        SwTextBlocks *pGroup = ::GetGlossaries()->GetGroupDoc (  GetCurrGrpName () );
        sal_Bool bRet = pGlossaryHdl->ConvertToNew ( *pGroup );
        delete pGroup;
        if ( bRet )
            EndDialog(RET_EDIT);
    }
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwGlossaryDlg, EditHdl)

/*------------------------------------------------------------------------
 Description:   KeyInput for ShortName - Edits without Spaces
------------------------------------------------------------------------*/
IMPL_LINK( SwNewGlosNameDlg, Modify, Edit *, pBox )
{
    OUString aName(m_pNewName->GetText());
    SwGlossaryDlg* pDlg = (SwGlossaryDlg*)GetParent();

    if (pBox == m_pNewName)
        m_pNewShort->SetText( lcl_GetValidShortCut( aName ) );

    sal_Bool bEnable = !aName.isEmpty() && !m_pNewShort->GetText().isEmpty() &&
        (!pDlg->DoesBlockExist(aName, m_pNewShort->GetText())
            || aName == m_pOldName->GetText());
    m_pOk->Enable(bEnable);
    return 0;
}

IMPL_LINK_NOARG(SwNewGlosNameDlg, Rename)
{
    SwGlossaryDlg* pDlg = (SwGlossaryDlg*)GetParent();
    OUString sNew = GetAppCharClass().uppercase(m_pNewShort->GetText());
    if( pDlg->pGlossaryHdl->HasShortName(m_pNewShort->GetText())
        && sNew != m_pOldShort->GetText() )
    {
        InfoBox(this, SW_RES(MSG_DOUBLE_SHORTNAME)).Execute();
        m_pNewShort->GrabFocus();
    }
    else
        EndDialog(sal_True);
    return 0;
}

IMPL_LINK( SwGlossaryDlg, CheckBoxHdl, CheckBox *, pBox )
{
    SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    bool bCheck = pBox->IsChecked();
    if (pBox == m_pInsertTipCB)
        rCfg.SetAutoTextTip(bCheck);
    else if(pBox == m_pFileRelCB)
        rCfg.SetSaveRelFile(bCheck);
    else
        rCfg.SetSaveRelNet(bCheck);
    return 0;
}

/* --------------------------------------------------
 * TreeListBox for groups and blocks
 * --------------------------------------------------*/
SwGlTreeListBox::SwGlTreeListBox(Window* pParent, WinBits nBits)
    : SvTreeListBox(pParent, nBits)
    , sReadonly(SW_RESSTR(SW_STR_READONLY)),
    pDragEntry(0)
{
    SetDragDropMode( SV_DRAGDROP_CTRL_MOVE|SV_DRAGDROP_CTRL_COPY );
}

Size SwGlTreeListBox::GetOptimalSize() const
{
    return LogicToPixel(Size(212, 84), MapMode(MAP_APPFONT));
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwGlTreeListBox(Window *pParent, VclBuilder::stringmap &)
{
    return new SwGlTreeListBox(pParent, WB_BORDER | WB_TABSTOP);
}

void SwGlTreeListBox::Clear()
{
    SvTreeListEntry* pEntry = First();
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
    SvTreeListEntry* pEntry = GetEntry( aPos );
    // there's only help for groups' names
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
                const std::vector<OUString> & rPathArr = ::GetGlossaries()->GetPathArray();
                if( !rPathArr.empty() )
                {
                    sMsg = rPathArr[pData->nPathIdx];
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
                    SvTreeListEntry* pEntry )
{
    DragDropMode  eRet;
    pDragEntry = pEntry;
    if(!GetParent(pEntry))
        eRet = SV_DRAGDROP_NONE;
    else
    {
        SwGlossaryDlg* pDlg = (SwGlossaryDlg*)GetParentDialog();
        SvTreeListEntry* pParent = GetParent(pEntry);

        GroupUserData* pGroupData = (GroupUserData*)pParent->GetUserData();
        OUString sEntry = pGroupData->sGroupName
            + OUString(GLOS_DELIM)
            + OUString::number(pGroupData->nPathIdx);
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

sal_Bool    SwGlTreeListBox::NotifyAcceptDrop( SvTreeListEntry* pEntry)
{
    // TODO: Readonly - check still missing!
    SvTreeListEntry* pSrcParent = GetParent(pEntry) ? GetParent(pEntry) : pEntry;
    SvTreeListEntry* pDestParent =
        GetParent(pDragEntry ) ? GetParent(pDragEntry ) : pDragEntry ;
    return pDestParent != pSrcParent;

}

sal_Bool  SwGlTreeListBox::NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& /*rpNewParent*/,
                                    sal_uLong&        /*rNewChildPos*/
                                )
{
    return NotifyCopyingOrMoving(pTarget, pEntry, true);
}

sal_Bool  SwGlTreeListBox::NotifyCopying(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& /*rpNewParent*/,
                                    sal_uLong&        /*rNewChildPos*/
                                )
{
    return NotifyCopyingOrMoving(pTarget, pEntry, false);
}

sal_Bool SwGlTreeListBox::NotifyCopyingOrMoving(
    SvTreeListEntry*  pTarget,
    SvTreeListEntry*  pEntry,
    bool              bIsMove)
{
    pDragEntry = 0;
    // 1. move in different groups?
    // 2. allowed to write to both groups?
    if(!pTarget) // move to the beginning
    {
        pTarget = GetEntry(0);
    }
    SvTreeListEntry* pSrcParent = GetParent(pEntry);
    SvTreeListEntry* pDestParent =
        GetParent(pTarget) ? GetParent(pTarget) : pTarget;
    sal_Bool bRet = sal_False;
    if(pDestParent != pSrcParent)
    {
        SwGlossaryDlg* pDlg = (SwGlossaryDlg*)GetParentDialog();
        SwWait aWait( *pDlg->pSh->GetView().GetDocShell(), sal_True );

        GroupUserData* pGroupData = (GroupUserData*)pSrcParent->GetUserData();
        OUString sSourceGroup = pGroupData->sGroupName
            + OUString(GLOS_DELIM)
            + OUString::number(pGroupData->nPathIdx);

        pDlg->pGlossaryHdl->SetCurGroup(sSourceGroup);
        OUString sTitle(GetEntryText(pEntry));
        OUString sShortName(*(String*)pEntry->GetUserData());

        GroupUserData* pDestData = (GroupUserData*)pDestParent->GetUserData();
        OUString sDestName = pDestData->sGroupName
            + OUString(GLOS_DELIM)
            + OUString::number(pDestData->nPathIdx);

        bRet = pDlg->pGlossaryHdl->CopyOrMove( sSourceGroup,  sShortName,
                        sDestName, sTitle, bIsMove );
        if(bRet)
        {
            SvTreeListEntry* pChild = InsertEntry(sTitle, pDestParent);
            pChild->SetUserData(new String(sShortName));
            if (bIsMove)
            {
                GetModel()->Remove(pEntry);
            }
        }
    }
    return sal_False; // otherwise the entry is being set automatically
}

String SwGlossaryDlg::GetCurrGrpName() const
{
    SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();
    String sRet;
    if(pEntry)
    {
        pEntry =
            m_pCategoryBox->GetParent(pEntry) ? m_pCategoryBox->GetParent(pEntry) : pEntry;
        GroupUserData* pGroupData = (GroupUserData*)pEntry->GetUserData();
        sRet = pGroupData->sGroupName;
        sRet += GLOS_DELIM;
        sRet += OUString::number(pGroupData->nPathIdx);
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

IMPL_LINK_NOARG(SwGlossaryDlg, InsertHdl)
{
    EndDialog(true);
    return 0;
}

void SwGlossaryDlg::ShowPreview()
{
    //create example
    if (!pExampleFrame)
    {
        Link aLink(LINK(this, SwGlossaryDlg, PreviewLoadedHdl));
        pExampleFrame = new SwOneExampleFrame( *m_pExampleWIN,
                        EX_SHOW_ONLINE_LAYOUT, &aLink );
    }

    ShowAutoText(::GetCurrGlosGroup(), m_pShortNameEdit->GetText());
};

IMPL_LINK_NOARG(SwGlossaryDlg, PreviewLoadedHdl)
{
    ResumeShowAutoText();
    return 0;
}

void SwGlossaryDlg::ShowAutoText(const String& rGroup, const String& rShortName)
{
    if(m_pExampleWIN->IsVisible())
    {
        SetResumeData(rGroup, rShortName);
        //try to make an Undo()
        pExampleFrame->ClearDocument( sal_True );
    }
}

void SwGlossaryDlg::ResumeShowAutoText()
{
    String sGroup, sShortName;
    if(GetResumeData(sGroup, sShortName) && m_pExampleWIN->IsVisible())
    {
        if(!m_xAutoText.is())
        {
            //now the AutoText ListBoxes have to be filled
            m_xAutoText = text::AutoTextContainer::create( comphelper::getProcessComponentContext() );
        }

        uno::Reference< XTextCursor > & xCrsr = pExampleFrame->GetTextCursor();
        if(xCrsr.is())
        {
            if(sShortName.Len())
            {
                uno::Any aGroup = m_xAutoText->getByName(sGroup);
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
