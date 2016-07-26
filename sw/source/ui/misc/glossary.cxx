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
#include <vcl/layout.hxx>
#include <vcl/help.hxx>
#include <vcl/builderfactory.hxx>
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
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <svl/urihelper.hxx>
#include <unotools/charclass.hxx>
#include <svtools/treelistentry.hxx>
#include <swwait.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <glossary.hxx>
#include <gloshdl.hxx>
#include <glosbib.hxx>
#include <initui.hxx>
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

#include <memory>

#include "access.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::ucbhelper;
using namespace ::sfx2;

static OUString lcl_GetValidShortCut( const OUString& rName )
{
    const sal_Int32 nSz = rName.getLength();

    if ( 0 == nSz )
        return rName;

    sal_Int32 nStart = 1;
    while( rName[nStart-1]==' ' && nStart < nSz )
        nStart++;

    OUString aBuf = OUString(rName[nStart-1]);

    for( ; nStart < nSz; ++nStart )
    {
        if( rName[nStart-1]==' ' && rName[nStart]!=' ')
            aBuf += OUString(rName[nStart]);
    }
    return aBuf;
}

struct GroupUserData
{
    OUString    sGroupName;
    sal_uInt16  nPathIdx;
    bool        bReadonly;

    GroupUserData()
        : nPathIdx(0),
          bReadonly(false)  {}
};

// dialog for new block name
class SwNewGlosNameDlg : public ModalDialog
{
    VclPtr<Edit>        m_pNewName;
    TextFilter          m_aNoSpaceFilter;
    VclPtr<Edit>        m_pNewShort;
    VclPtr<OKButton>    m_pOk;
    VclPtr<Edit>        m_pOldName;
    VclPtr<Edit>        m_pOldShort;

protected:
    DECL_LINK_TYPED( Modify, Edit&, void );
    DECL_LINK_TYPED(Rename, Button*, void);

public:
    SwNewGlosNameDlg( vcl::Window* pParent,
                      const OUString& rOldName,
                      const OUString& rOldShort );
    virtual ~SwNewGlosNameDlg();
    virtual void dispose() override;

    OUString GetNewName()  const { return m_pNewName->GetText(); }
    OUString GetNewShort() const { return m_pNewShort->GetText(); }
};

SwNewGlosNameDlg::SwNewGlosNameDlg(vcl::Window* pParent,
                            const OUString& rOldName,
                            const OUString& rOldShort )
    : ModalDialog(pParent, "RenameAutoTextDialog",
        "modules/swriter/ui/renameautotextdialog.ui")
{
    get(m_pNewName, "newname");
    get(m_pNewShort, "newsc");
    m_pNewShort->SetTextFilter(&m_aNoSpaceFilter);
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

SwNewGlosNameDlg::~SwNewGlosNameDlg()
{
    disposeOnce();
}

void SwNewGlosNameDlg::dispose()
{
    m_pNewName.clear();
    m_pNewShort.clear();
    m_pOk.clear();
    m_pOldName.clear();
    m_pOldShort.clear();
    ModalDialog::dispose();
}

// query / set currently set group
OUString SwGlossaryDlg::GetCurrGroup()
{
    if( !::GetCurrGlosGroup().isEmpty() )
        return ::GetCurrGlosGroup();
    return SwGlossaries::GetDefName();
}

void SwGlossaryDlg::SetActGroup(const OUString &rGrp)
{
    ::SetCurrGlosGroup(rGrp);
}

SwGlossaryDlg::SwGlossaryDlg(SfxViewFrame* pViewFrame,
                            SwGlossaryHdl * pGlosHdl, SwWrtShell *pWrtShell)
    : SvxStandardDialog(&pViewFrame->GetWindow(), "AutoTextDialog",
        "modules/swriter/ui/autotext.ui")
    , sReadonlyPath(SW_RESSTR(STR_READONLY_PATH))
    , pExampleFrame(nullptr)
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
    m_pShortNameEdit->SetTextFilter(&m_aNoSpaceFilter);
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
        m_pInsertBtn->Enable(false);
    m_pNameED->GrabFocus();
    m_pCategoryBox->SetStyle(m_pCategoryBox->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL|WB_VSCROLL|WB_CLIPCHILDREN|WB_SORT);
    m_pCategoryBox->GetModel()->SetSortMode(SortAscending);
    m_pCategoryBox->SetHighlightRange();   // select over full width
    m_pCategoryBox->SetNodeDefaultImages( );

    Init();
}

SwGlossaryDlg::~SwGlossaryDlg()
{
    disposeOnce();
}

void SwGlossaryDlg::dispose()
{
    m_pCategoryBox->Clear();
    delete pExampleFrame;
    m_pInsertTipCB.clear();
    m_pNameED.clear();
    m_pShortNameLbl.clear();
    m_pShortNameEdit.clear();
    m_pCategoryBox.clear();
    m_pFileRelCB.clear();
    m_pNetRelCB.clear();
    m_pExampleWIN.clear();
    m_pInsertBtn.clear();
    m_pEditBtn.clear();
    m_pBibBtn.clear();
    m_pPathBtn.clear();
    SvxStandardDialog::dispose();
}

// select new group
IMPL_LINK_TYPED( SwGlossaryDlg, GrpSelect, SvTreeListBox *, pBox, void )
{
    SvTreeListEntry* pEntry = pBox->FirstSelected();
    if(!pEntry)
        return;
    SvTreeListEntry* pParent = pBox->GetParent(pEntry) ? pBox->GetParent(pEntry) : pEntry;
    GroupUserData* pGroupData = static_cast<GroupUserData*>(pParent->GetUserData());
    ::SetCurrGlosGroup(pGroupData->sGroupName
        + OUStringLiteral1<GLOS_DELIM>()
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
        m_pShortNameEdit->SetText(*static_cast<OUString*>(pEntry->GetUserData()));
        m_pInsertBtn->Enable( !bIsDocReadOnly);
        ShowAutoText(::GetCurrGlosGroup(), m_pShortNameEdit->GetText());
    }
    else
    {
        m_pNameED->SetText("");
        m_pShortNameEdit->SetText("");
        m_pShortNameEdit->Enable(false);
        ShowAutoText("", "");
    }
    // update controls
    NameModify(*m_pShortNameEdit);
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
}

void SwGlossaryDlg::Apply()
{
    const OUString aGlosName(m_pShortNameEdit->GetText());
    if (!aGlosName.isEmpty())
    {
        pGlossaryHdl->InsertGlossary(aGlosName);
    }
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

void SwGlossaryDlg::EnableShortName(bool bOn)
{
    m_pShortNameLbl->Enable(bOn);
    m_pShortNameEdit->Enable(bOn);
}

// does the title exist in the selected group?
SvTreeListEntry* SwGlossaryDlg::DoesBlockExist(const OUString& rBlock,
                const OUString& rShort)
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
            if (rBlock == m_pCategoryBox->GetEntryText(pChild) &&
                (rShort.isEmpty() ||
                 rShort==*static_cast<OUString*>(pChild->GetUserData()))
               )
            {
                return pChild;
            }
        }
    }
    return nullptr;
}

IMPL_LINK_TYPED( SwGlossaryDlg, NameModify, Edit&, rEdit, void )
{
    const OUString aName(m_pNameED->GetText());
    bool bNameED = &rEdit == m_pNameED;
    if( aName.isEmpty() )
    {
        if(bNameED)
            m_pShortNameEdit->SetText(aName);
        m_pInsertBtn->Enable(false);
        return;
    }
    const bool bNotFound = !DoesBlockExist(aName, bNameED ? OUString() : rEdit.GetText());
    if(bNameED)
    {
            // did the text get in to the Listbox in the Edit with a click?
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
            bool bEnable = !bNotFound;
            bEnable &= !bIsDocReadOnly;
            m_pInsertBtn->Enable(bEnable);
        }
    }
}

IMPL_LINK_TYPED( SwGlossaryDlg, NameDoubleClick, SvTreeListBox*, pBox, bool )
{
    SvTreeListEntry* pEntry = pBox->FirstSelected();
    if(pBox->GetParent(pEntry) && !bIsDocReadOnly)
        EndDialog( RET_OK );
    return false;
}

IMPL_LINK_TYPED( SwGlossaryDlg, EnableHdl, Menu *, pMn, bool )
{
    SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();

    const OUString aEditText(m_pNameED->GetText());
    const bool bHasEntry = !aEditText.isEmpty() && !m_pShortNameEdit->GetText().isEmpty();
    const bool bExists = nullptr != DoesBlockExist(aEditText, m_pShortNameEdit->GetText());
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
    return true;
}

IMPL_LINK_TYPED( SwGlossaryDlg, MenuHdl, Menu *, pMn, bool )
{
    OString sItemIdent(pMn->GetCurItemIdent());

    if (sItemIdent == "replace")
    {
        pGlossaryHdl->NewGlossary(m_pNameED->GetText(),
                                  m_pShortNameEdit->GetText());
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

        const OUString aStr(m_pNameED->GetText());
        const OUString aShortName(m_pShortNameEdit->GetText());
        if(pGlossaryHdl->HasShortName(aShortName))
        {
            ScopedVclPtrInstance<MessageDialog>(this, SW_RES(STR_DOUBLE_SHORTNAME), VCL_MESSAGE_INFO)->Execute();
            m_pShortNameEdit->SetSelection(Selection(0, SELECTION_MAX));
            m_pShortNameEdit->GrabFocus();
            return true;
        }
        if(pGlossaryHdl->NewGlossary(aStr, aShortName, false, bNoAttr ))
        {
            SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();
            if(m_pCategoryBox->GetParent(pEntry))
                pEntry = m_pCategoryBox->GetParent(pEntry);

            SvTreeListEntry* pChild = m_pCategoryBox->InsertEntry(aStr, pEntry);
            pChild->SetUserData(new OUString(aShortName));
            m_pNameED->SetText(aStr);
            m_pShortNameEdit->SetText(aShortName);
            NameModify(*m_pNameED);       // for toggling the buttons

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
        ScopedVclPtrInstance<SwNewGlosNameDlg> pNewNameDlg(this, m_pNameED->GetText(),
                                                           m_pShortNameEdit->GetText());
        if( RET_OK == pNewNameDlg->Execute() &&
            pGlossaryHdl->Rename( m_pShortNameEdit->GetText(),
                                    pNewNameDlg->GetNewShort(),
                                    pNewNameDlg->GetNewName()))
        {
            SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();
            SvTreeListEntry* pNewEntry = m_pCategoryBox->InsertEntry(
                    pNewNameDlg->GetNewName(), m_pCategoryBox->GetParent(pEntry));
            pNewEntry->SetUserData(new OUString(pNewNameDlg->GetNewShort()));
            delete static_cast<OUString*>(pEntry->GetUserData());
            m_pCategoryBox->GetModel()->Remove(pEntry);
            m_pCategoryBox->Select(pNewEntry);
            m_pCategoryBox->MakeVisible(pNewEntry);
        }
        GrpSelect(m_pCategoryBox);
    }
    else if (sItemIdent == "delete")
    {
        ScopedVclPtrInstance< MessageDialog > aQuery(this, SW_RES(STR_QUERY_DELETE), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
        if (RET_YES == aQuery->Execute())
        {
            const OUString aShortName(m_pShortNameEdit->GetText());
            const OUString aTitle(m_pNameED->GetText());
            if (!aTitle.isEmpty() && pGlossaryHdl->DelGlossary(aShortName))
            {
                SvTreeListEntry* pChild = DoesBlockExist(aTitle, aShortName);
                OSL_ENSURE(pChild, "entry not found!");
                SvTreeListEntry* pParent = m_pCategoryBox->GetParent(pChild);
                m_pCategoryBox->Select(pParent);

                m_pCategoryBox->GetModel()->Remove(pChild);
                m_pNameED->SetText(OUString());
                NameModify(*m_pNameED);
            }
        }
    }
    else if (sItemIdent == "macro")
    {
        SfxItemSet aSet( pSh->GetAttrPool(), RES_FRMMACRO, RES_FRMMACRO, SID_EVENTCONFIG, SID_EVENTCONFIG, 0 );

        SvxMacro aStart(aEmptyOUStr, aEmptyOUStr, STARBASIC);
        SvxMacro aEnd(aEmptyOUStr, aEmptyOUStr, STARBASIC);
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
        std::unique_ptr<SfxAbstractDialog> pMacroDlg(pFact->CreateSfxDialog( this, aSet,
            pSh->GetView().GetViewFrame()->GetFrame().GetFrameInterface(), SID_EVENTCONFIG ));
        if ( pMacroDlg && pMacroDlg->Execute() == RET_OK &&
            SfxItemState::SET == pMacroDlg->GetOutputItemSet()->GetItemState( RES_FRMMACRO, false, &pItem ) )
        {
            const SvxMacroTableDtor& rTable = static_cast<const SvxMacroItem*>(pItem)->GetMacroTable();
            pGlossaryHdl->SetMacros( m_pShortNameEdit->GetText(),
                                        rTable.Get( SW_EVENT_START_INS_GLOSSARY ),
                                        rTable.Get( SW_EVENT_END_INS_GLOSSARY ) );
        }
    }
    else if (sItemIdent == "import")
    {
        // call the FileOpenDialog do find WinWord - Files with templates
        FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE );
        uno::Reference < XFilePicker2 > xFP = aDlgHelper.GetFilePicker();

        SvtPathOptions aPathOpt;
        xFP->setDisplayDirectory(aPathOpt.GetWorkPath() );

        uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
        SfxFilterMatcher aMatcher( OUString::createFromAscii(SwDocShell::Factory().GetShortName()) );
        SfxFilterMatcherIter aIter( aMatcher );
        std::shared_ptr<const SfxFilter> pFilter = aIter.First();
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
            if( pGlossaryHdl->ImportGlossaries( xFP->getSelectedFiles().getConstArray()[0] ))
                Init();
            else
            {
                ScopedVclPtrInstance<MessageDialog>(this, SW_RES( STR_NO_GLOSSARIES ), VCL_MESSAGE_INFO)->Execute();
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

// dialog manage regions
IMPL_LINK_NOARG_TYPED(SwGlossaryDlg, BibHdl, Button*, void)
{
    SwGlossaries* pGloss = ::GetGlossaries();
    if( pGloss->IsGlosPathErr() )
        pGloss->ShowError();
    else
    {
        //check if at least one glossary path is write enabled
        SvtPathOptions aPathOpt;
        const OUString sGlosPath( aPathOpt.GetAutoTextPath() );
        const sal_Int32 nPaths = comphelper::string::getTokenCount(sGlosPath, ';');
        bool bIsWritable = false;
        for(sal_Int32 nPath = 0; nPath < nPaths; nPath++)
        {
            const OUString sPath = URIHelper::SmartRel2Abs(
                INetURLObject(), sGlosPath.getToken(nPath, ';'),
                URIHelper::GetMaybeFileHdl());
            try
            {
                Content aTestContent( sPath,
                            uno::Reference< XCommandEnvironment >(),
                            comphelper::getProcessComponentContext() );
                Any aAny = aTestContent.getPropertyValue( "IsReadOnly" );
                if(aAny.hasValue())
                {
                    bIsWritable = !*static_cast<sal_Bool const *>(aAny.getValue());
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

            ScopedVclPtrInstance< SwGlossaryGroupDlg > pDlg( this, pGloss->GetPathArray(), pGlossaryHdl );
            if ( RET_OK == pDlg->Execute() )
            {
                Init();
                //if new groups were created - select one of them
                const OUString sNewGroup = pDlg->GetCreatedGroupName();
                SvTreeListEntry* pEntry = m_pCategoryBox->First();
                while (!sNewGroup.isEmpty() && pEntry)
                {
                    if(!m_pCategoryBox->GetParent(pEntry))
                    {
                        GroupUserData* pGroupData = static_cast<GroupUserData*>(pEntry->GetUserData());
                        const OUString sGroup = pGroupData->sGroupName
                            + OUStringLiteral1<GLOS_DELIM>()
                            + OUString::number(pGroupData->nPathIdx);
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
        }
        else
        {
            ScopedVclPtrInstance< MessageDialog > aBox(this, sReadonlyPath, VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

            if(RET_YES == aBox->Execute())
                PathHdl(m_pPathBtn);
        }
    }
}

// initialisation; from Ctor and after editing regions
void SwGlossaryDlg::Init()
{
    m_pCategoryBox->SetUpdateMode( false );
    m_pCategoryBox->Clear();
    // display text block regions
    const size_t nCnt = pGlossaryHdl->GetGroupCnt();
    SvTreeListEntry* pSelEntry = nullptr;
    const OUString sSelStr(::GetCurrGlosGroup().getToken(0, GLOS_DELIM));
    const sal_Int32 nSelPath = ::GetCurrGlosGroup().getToken(1, GLOS_DELIM).toInt32();
    // #i66304# - "My AutoText" comes from mytexts.bau, but should be translated
    const OUString sMyAutoTextEnglish("My AutoText");
    const OUString sMyAutoTextTranslated(SW_RES(STR_MY_AUTOTEXT));
    for(size_t nId = 0; nId < nCnt; ++nId )
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
        const sal_Int32 nPath = sGroupName.getToken( 1, GLOS_DELIM ).toInt32();

        GroupUserData* pData = new GroupUserData;
        pData->sGroupName = sGroupName.getToken(0, GLOS_DELIM);
        pData->nPathIdx = static_cast< sal_uInt16 >(nPath);
        pData->bReadonly = pGlossaryHdl->IsReadOnly(&sGroupName);

        pEntry->SetUserData(pData);
        if(sSelStr == pData->sGroupName && nSelPath == nPath)
            pSelEntry = pEntry;

        // fill entries for the groups
        {
            pGlossaryHdl->SetCurGroup(sGroupName, false, true);
            const sal_uInt16 nCount = pGlossaryHdl->GetGlossaryCnt();
            for(sal_uInt16 i = 0; i < nCount; ++i)
            {
                SvTreeListEntry* pChild = m_pCategoryBox->InsertEntry(
                                    pGlossaryHdl->GetGlossaryName(i), pEntry);
                pChild->SetUserData(new OUString(pGlossaryHdl->GetGlossaryShortName(i)));
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
                GroupUserData* pData = static_cast<GroupUserData*>(pSearch->GetUserData());
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
    //JP 16.11.99: the SvxTreeListBox has a Bug. The Box don't recalc the
    //      outputsize, when all entries are insertet. The result is, that
    //      the Focus/Highlight rectangle is to large and paintet over the
    //      HScrollbar. -> Fix: call the resize
    m_pCategoryBox->Resize();

    m_pCategoryBox->GetModel()->Resort();
    m_pCategoryBox->SetUpdateMode( true );
    m_pCategoryBox->Update();

    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    m_pFileRelCB->Check( rCfg.IsSaveRelFile() );
    m_pFileRelCB->SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    m_pNetRelCB->Check( rCfg.IsSaveRelNet() );
    m_pNetRelCB->SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    m_pInsertTipCB->Check( rCfg.IsAutoTextTip() );
    m_pInsertTipCB->SetClickHdl(LINK(this, SwGlossaryDlg, CheckBoxHdl));
}

IMPL_LINK_NOARG_TYPED(SwGlossaryDlg, EditHdl, MenuButton *, void)
{
// EndDialog must not be called in MenuHdl
    if (m_pEditBtn->GetCurItemIdent() == "edit")
    {
        SwTextBlocks *pGroup = ::GetGlossaries()->GetGroupDoc (  GetCurrGrpName () );
        delete pGroup;
        EndDialog(RET_EDIT);
    }
}

// KeyInput for ShortName - Edits without Spaces
IMPL_LINK_TYPED( SwNewGlosNameDlg, Modify, Edit&, rBox, void )
{
    OUString aName(m_pNewName->GetText());
    SwGlossaryDlg* pDlg = static_cast<SwGlossaryDlg*>(GetParent());
    if (&rBox == m_pNewName)
        m_pNewShort->SetText( lcl_GetValidShortCut( aName ) );

    bool bEnable = !aName.isEmpty() && !m_pNewShort->GetText().isEmpty() &&
        (!pDlg->DoesBlockExist(aName, m_pNewShort->GetText())
            || aName == m_pOldName->GetText());
    m_pOk->Enable(bEnable);
}

IMPL_LINK_NOARG_TYPED(SwNewGlosNameDlg, Rename, Button*, void)
{
    SwGlossaryDlg* pDlg = static_cast<SwGlossaryDlg*>(GetParent());
    OUString sNew = GetAppCharClass().uppercase(m_pNewShort->GetText());
    if( pDlg->pGlossaryHdl->HasShortName(m_pNewShort->GetText())
        && sNew != m_pOldShort->GetText() )
    {
        ScopedVclPtrInstance<MessageDialog>(this, SW_RES(STR_DOUBLE_SHORTNAME), VCL_MESSAGE_INFO)->Execute();
        m_pNewShort->GrabFocus();
    }
    else
        EndDialog(RET_OK);
}

IMPL_LINK_TYPED( SwGlossaryDlg, CheckBoxHdl, Button *, pBox, void )
{
    SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    bool bCheck = static_cast<CheckBox*>(pBox)->IsChecked();
    if (pBox == m_pInsertTipCB)
        rCfg.SetAutoTextTip(bCheck);
    else if(pBox == m_pFileRelCB)
        rCfg.SetSaveRelFile(bCheck);
    else
        rCfg.SetSaveRelNet(bCheck);
    rCfg.Commit();
}

// TreeListBox for groups and blocks
SwGlTreeListBox::SwGlTreeListBox(vcl::Window* pParent, WinBits nBits)
    : SvTreeListBox(pParent, nBits)
    , sReadonly(SW_RESSTR(SW_STR_READONLY)),
    pDragEntry(nullptr)
{
    SetDragDropMode( DragDropMode::CTRL_MOVE|DragDropMode::CTRL_COPY );
}

Size SwGlTreeListBox::GetOptimalSize() const
{
    return LogicToPixel(Size(212, 84), MapMode(MAP_APPFONT));
}

VCL_BUILDER_FACTORY_ARGS(SwGlTreeListBox, WB_BORDER | WB_TABSTOP)

void SwGlTreeListBox::Clear()
{
    SvTreeListEntry* pEntry = First();
    while(pEntry)
    {
        if(GetParent(pEntry))
            delete static_cast<OUString*>(pEntry->GetUserData());
        else
            delete static_cast<GroupUserData*>(pEntry->GetUserData());
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
            OUString sMsg;
            if(!GetParent(pEntry))
            {
                GroupUserData* pData = static_cast<GroupUserData*>(pEntry->GetUserData());
                const std::vector<OUString> & rPathArr = ::GetGlossaries()->GetPathArray();
                if( !rPathArr.empty() )
                {
                    INetURLObject aTmp(rPathArr[pData->nPathIdx]
                                       + "/"
                                       + pData->sGroupName
                                       + SwGlossaries::GetExtension());
                    sMsg = aTmp.GetPath();

                    if(pData->bReadonly)
                    {
                        sMsg += " (" + sReadonly + ")";
                    }
                }
            }
            else
                sMsg = *static_cast<OUString*>(pEntry->GetUserData());
            Help::ShowQuickHelp( this, aItemRect, sMsg,
                        QuickHelpFlags::Left|QuickHelpFlags::VCenter );
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
        eRet = DragDropMode::NONE;
    else
    {
        SwGlossaryDlg* pDlg = static_cast<SwGlossaryDlg*>(GetParentDialog());
        SvTreeListEntry* pParent = GetParent(pEntry);

        GroupUserData* pGroupData = static_cast<GroupUserData*>(pParent->GetUserData());
        OUString sEntry = pGroupData->sGroupName
            + OUStringLiteral1<GLOS_DELIM>()
            + OUString::number(pGroupData->nPathIdx);
        sal_Int8 nDragOption = DND_ACTION_COPY;
        eRet = DragDropMode::CTRL_COPY;
        if(!pDlg->pGlossaryHdl->IsReadOnly(&sEntry))
        {
            eRet |= DragDropMode::CTRL_MOVE;
            nDragOption |= DND_ACTION_MOVE;
        }
        SetDragOptions( nDragOption );
    }
    return eRet;
}

bool SwGlTreeListBox::NotifyAcceptDrop( SvTreeListEntry* pEntry)
{
    // TODO: Readonly - check still missing!
    SvTreeListEntry* pSrcParent = GetParent(pEntry) ? GetParent(pEntry) : pEntry;
    SvTreeListEntry* pDestParent =
        GetParent(pDragEntry ) ? GetParent(pDragEntry ) : pDragEntry ;
    return pDestParent != pSrcParent;

}

TriState SwGlTreeListBox::NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& /*rpNewParent*/,
                                    sal_uLong&        /*rNewChildPos*/
                                )
{
    return NotifyCopyingOrMoving(pTarget, pEntry, true);
}

TriState SwGlTreeListBox::NotifyCopying(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& /*rpNewParent*/,
                                    sal_uLong&        /*rNewChildPos*/
                                )
{
    return NotifyCopyingOrMoving(pTarget, pEntry, false);
}

TriState SwGlTreeListBox::NotifyCopyingOrMoving(
    SvTreeListEntry*  pTarget,
    SvTreeListEntry*  pEntry,
    bool              bIsMove)
{
    pDragEntry = nullptr;
    // 1. move in different groups?
    // 2. allowed to write to both groups?
    if(!pTarget) // move to the beginning
    {
        pTarget = GetEntry(0);
    }
    SvTreeListEntry* pSrcParent = GetParent(pEntry);
    SvTreeListEntry* pDestParent =
        GetParent(pTarget) ? GetParent(pTarget) : pTarget;
    if(pDestParent != pSrcParent)
    {
        SwGlossaryDlg* pDlg = static_cast<SwGlossaryDlg*>(GetParentDialog());
        SwWait aWait( *pDlg->pSh->GetView().GetDocShell(), true );

        GroupUserData* pGroupData = static_cast<GroupUserData*>(pSrcParent->GetUserData());
        OUString sSourceGroup = pGroupData->sGroupName
            + OUStringLiteral1<GLOS_DELIM>()
            + OUString::number(pGroupData->nPathIdx);

        pDlg->pGlossaryHdl->SetCurGroup(sSourceGroup);
        OUString sTitle(GetEntryText(pEntry));
        OUString sShortName(*static_cast<OUString*>(pEntry->GetUserData()));

        GroupUserData* pDestData = static_cast<GroupUserData*>(pDestParent->GetUserData());
        OUString sDestName = pDestData->sGroupName
            + OUStringLiteral1<GLOS_DELIM>()
            + OUString::number(pDestData->nPathIdx);

        const bool bRet = pDlg->pGlossaryHdl->CopyOrMove( sSourceGroup,  sShortName,
                        sDestName, sTitle, bIsMove );
        if(bRet)
        {
            SvTreeListEntry* pChild = InsertEntry(sTitle, pDestParent);
            pChild->SetUserData(new OUString(sShortName));
            if (bIsMove)
            {
                GetModel()->Remove(pEntry);
            }
        }
    }
    return TRISTATE_FALSE; // otherwise the entry is being set automatically
}

OUString SwGlossaryDlg::GetCurrGrpName() const
{
    SvTreeListEntry* pEntry = m_pCategoryBox->FirstSelected();
    if(pEntry)
    {
        pEntry =
            m_pCategoryBox->GetParent(pEntry) ? m_pCategoryBox->GetParent(pEntry) : pEntry;
        GroupUserData* pGroupData = static_cast<GroupUserData*>(pEntry->GetUserData());
        return pGroupData->sGroupName + OUStringLiteral1<GLOS_DELIM>() + OUString::number(pGroupData->nPathIdx);
    }
    return OUString();
}

IMPL_LINK_TYPED( SwGlossaryDlg, PathHdl, Button *, pBtn, void )
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        std::unique_ptr<AbstractSvxMultiPathDialog> pDlg(pFact->CreateSvxPathSelectDialog(pBtn));
        OSL_ENSURE(pDlg, "Dialog creation failed!");
        SvtPathOptions aPathOpt;
        const OUString sGlosPath( aPathOpt.GetAutoTextPath() );
        pDlg->SetPath(sGlosPath);
        if(RET_OK == pDlg->Execute())
        {
            const OUString sTmp(pDlg->GetPath());
            if(sTmp != sGlosPath)
            {
                aPathOpt.SetAutoTextPath( sTmp );
                ::GetGlossaries()->UpdateGlosPath( true );
                Init();
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED(SwGlossaryDlg, InsertHdl, Button*, void)
{
    EndDialog(RET_OK);
}

void SwGlossaryDlg::ShowPreview()
{
    //create example
    if (!pExampleFrame)
    {
        Link<SwOneExampleFrame&,void> aLink(LINK(this, SwGlossaryDlg, PreviewLoadedHdl));
        pExampleFrame = new SwOneExampleFrame( *m_pExampleWIN,
                        EX_SHOW_ONLINE_LAYOUT, &aLink );
    }

    ShowAutoText(::GetCurrGlosGroup(), m_pShortNameEdit->GetText());
};

IMPL_LINK_NOARG_TYPED(SwGlossaryDlg, PreviewLoadedHdl, SwOneExampleFrame&, void)
{
    ResumeShowAutoText();
}

void SwGlossaryDlg::ShowAutoText(const OUString& rGroup, const OUString& rShortName)
{
    if(m_pExampleWIN->IsVisible())
    {
        SetResumeData(rGroup, rShortName);
        //try to make an Undo()
        pExampleFrame->ClearDocument();
    }
}

void SwGlossaryDlg::ResumeShowAutoText()
{
    OUString sGroup;
    OUString sShortName;
    if(GetResumeData(sGroup, sShortName) && m_pExampleWIN->IsVisible())
    {
        if(!m_xAutoText.is())
        {
            //now the AutoText ListBoxes have to be filled
            m_xAutoText = text::AutoTextContainer::create( comphelper::getProcessComponentContext() );
        }

        uno::Reference< XTextCursor > & xCursor = pExampleFrame->GetTextCursor();
        if(xCursor.is())
        {
            if (!sShortName.isEmpty())
            {
                uno::Any aGroup = m_xAutoText->getByName(sGroup);
                uno::Reference< XAutoTextGroup >  xGroup;
                if((aGroup >>= xGroup) && xGroup->hasByName(sShortName))
                {
                    uno::Any aEntry(xGroup->getByName(sShortName));
                    uno::Reference< XAutoTextEntry >  xEntry;
                    aEntry >>= xEntry;
                    uno::Reference< XTextRange >  xRange(xCursor, uno::UNO_QUERY);
                    xEntry->applyTo(xRange);
                }
            }
        }
    }
    ResetResumeData();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
