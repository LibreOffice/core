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

#include <o3tl/any.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svl/stritem.hxx>
#include <svl/macitem.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/request.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfilt.hxx>
#include <osl/diagnose.h>

#include <svx/svxdlg.hxx>
#include <editeng/acorrcfg.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/text/AutoTextContainer.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <svl/urihelper.hxx>
#include <unotools/charclass.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <glossary.hxx>
#include <gloshdl.hxx>
#include <glosbib.hxx>
#include <initui.hxx>
#include <glosdoc.hxx>
#include <macassgn.hxx>
#include <docsh.hxx>
#include <shellio.hxx>
#include <unotextcursor.hxx>

#include <cmdid.h>
#include <sfx2/filedlghelper.hxx>

#include <memory>

#include <strings.hrc>
#include <iodetect.hxx>

#include <officecfg/Office/Writer.hxx>

using namespace ::com::sun::star;
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

    OUStringBuffer aBuf(OUString::number(rName[nStart-1]));

    for( ; nStart < nSz; ++nStart )
    {
        if( rName[nStart-1]==' ' && rName[nStart]!=' ')
            aBuf.append(rName[nStart]);
    }
    return aBuf.makeStringAndClear();
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
class SwNewGlosNameDlg : public weld::GenericDialogController
{
    TextFilter          m_aNoSpaceFilter;
    SwGlossaryDlg*      m_pParent;

    std::unique_ptr<weld::Entry> m_xNewName;
    std::unique_ptr<weld::Entry> m_xNewShort;
    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Entry> m_xOldName;
    std::unique_ptr<weld::Entry> m_xOldShort;

protected:
    DECL_LINK(Modify, weld::Entry&, void);
    DECL_LINK(Rename, weld::Button&, void);
    DECL_LINK(TextFilterHdl, OUString&, bool);

public:
    SwNewGlosNameDlg(SwGlossaryDlg* pParent,
                     const OUString& rOldName,
                     const OUString& rOldShort);

    OUString GetNewName()  const { return m_xNewName->get_text(); }
    OUString GetNewShort() const { return m_xNewShort->get_text(); }
};

IMPL_LINK(SwNewGlosNameDlg, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aNoSpaceFilter.filter(rTest);
    return true;
}

SwNewGlosNameDlg::SwNewGlosNameDlg(SwGlossaryDlg* pParent, const OUString& rOldName, const OUString& rOldShort)
    : GenericDialogController(pParent->getDialog(), u"modules/swriter/ui/renameautotextdialog.ui"_ustr, u"RenameAutoTextDialog"_ustr)
    , m_pParent(pParent)
    , m_xNewName(m_xBuilder->weld_entry(u"newname"_ustr))
    , m_xNewShort(m_xBuilder->weld_entry(u"newsc"_ustr))
    , m_xOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xOldName(m_xBuilder->weld_entry(u"oldname"_ustr))
    , m_xOldShort(m_xBuilder->weld_entry(u"oldsc"_ustr))
{
    m_xNewShort->connect_insert_text(LINK(this, SwNewGlosNameDlg, TextFilterHdl));

    m_xOldName->set_text(rOldName);
    m_xOldShort->set_text(rOldShort);
    m_xNewName->connect_changed(LINK(this, SwNewGlosNameDlg, Modify ));
    m_xNewShort->connect_changed(LINK(this, SwNewGlosNameDlg, Modify ));
    m_xOk->connect_clicked(LINK(this, SwNewGlosNameDlg, Rename ));
    m_xNewName->grab_focus();
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

IMPL_LINK(SwGlossaryDlg, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aNoSpaceFilter.filter(rTest);
    return true;
}

class SwGlossaryDropTarget : public DropTargetHelper
{
private:
    weld::TreeView& m_rTreeView;
    SwGlossaryHdl* m_pGlosHdl;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override
    {
        weld::TreeView* pSource = m_rTreeView.get_drag_source();
        if (!pSource || pSource != &m_rTreeView)
            return DND_ACTION_NONE;

        std::unique_ptr<weld::TreeIter> xSelected(pSource->make_iterator());
        bool bSelected = pSource->get_selected(xSelected.get());
        if (!bSelected)
            return DND_ACTION_NONE;

        while (pSource->get_iter_depth(*xSelected))
            (void)pSource->iter_parent(*xSelected);

        GroupUserData* pSrcRootData = weld::fromId<GroupUserData*>(pSource->get_id(*xSelected));
        GroupUserData* pDestRootData = nullptr;

        std::unique_ptr<weld::TreeIter> xDestEntry(m_rTreeView.make_iterator());
        bool bEntry = m_rTreeView.get_dest_row_at_pos(rEvt.maPosPixel, xDestEntry.get(), true);
        if (bEntry)
        {
            while (m_rTreeView.get_iter_depth(*xDestEntry))
                (void)m_rTreeView.iter_parent(*xDestEntry);
            pDestRootData = weld::fromId<GroupUserData*>(m_rTreeView.get_id(*xDestEntry));
        }
        if (pDestRootData == pSrcRootData)
            return DND_ACTION_NONE;
        sal_uInt8 nRet = DND_ACTION_COPY;
        const bool bCheckForMove = rEvt.mnAction & DND_ACTION_MOVE;
        if (bCheckForMove && !pSrcRootData->bReadonly)
            nRet |= DND_ACTION_MOVE;
        return nRet;
    }

    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override
    {
        weld::TreeView* pSource = m_rTreeView.get_drag_source();
        if (!pSource)
            return DND_ACTION_NONE;

        std::unique_ptr<weld::TreeIter> xDestEntry(m_rTreeView.make_iterator());
        bool bEntry = m_rTreeView.get_dest_row_at_pos(rEvt.maPosPixel, xDestEntry.get(), true);
        if (!bEntry)
            return DND_ACTION_NONE;

        std::unique_ptr<weld::TreeIter> xSelected(pSource->make_iterator());
        bool bSelected = pSource->get_selected(xSelected.get());
        if (!bSelected)
            return DND_ACTION_NONE;

        std::unique_ptr<weld::TreeIter> xSrcParent(pSource->make_iterator(xSelected.get()));
        while (pSource->get_iter_depth(*xSrcParent))
            (void)pSource->iter_parent(*xSrcParent);

        std::unique_ptr<weld::TreeIter> xDestParent(pSource->make_iterator(xDestEntry.get()));
        while (pSource->get_iter_depth(*xDestParent))
            (void)pSource->iter_parent(*xDestParent);

        GroupUserData* pSrcParent = weld::fromId<GroupUserData*>(pSource->get_id(*xSrcParent));
        GroupUserData* pDestParent = weld::fromId<GroupUserData*>(m_rTreeView.get_id(*xDestParent));

        if (pDestParent != pSrcParent)
        {
            weld::WaitObject aBusy(&m_rTreeView);

            OUString sSourceGroup = pSrcParent->sGroupName
                + OUStringChar(GLOS_DELIM)
                + OUString::number(pSrcParent->nPathIdx);

            m_pGlosHdl->SetCurGroup(sSourceGroup);
            OUString sTitle(pSource->get_text(*xSelected));
            OUString sShortName(pSource->get_id(*xSelected));

            OUString sDestName = pDestParent->sGroupName
                + OUStringChar(GLOS_DELIM)
                + OUString::number(pDestParent->nPathIdx);

            bool bIsMove = rEvt.mnAction & DND_ACTION_MOVE;

            const bool bRet = m_pGlosHdl->CopyOrMove(sSourceGroup, sShortName,
                            sDestName, sTitle, bIsMove);

            if(bRet)
            {
                m_rTreeView.insert(xDestParent.get(), -1, &sTitle, &sShortName,
                                       nullptr, nullptr, false, nullptr);
                if (bIsMove)
                {
                    pSource->remove(*xSelected);
                }
            }
        }

        return DND_ACTION_NONE;
    }

public:
    SwGlossaryDropTarget(weld::TreeView& rTreeView, SwGlossaryHdl* pGlosHdl)
        : DropTargetHelper(rTreeView.get_drop_target())
        , m_rTreeView(rTreeView)
        , m_pGlosHdl(pGlosHdl)
    {
    }
};

SwGlossaryDlg::SwGlossaryDlg(const SfxViewFrame& rViewFrame,
                             SwGlossaryHdl * pGlosHdl, SwWrtShell *pWrtShell)
    : SfxDialogController(rViewFrame.GetFrameWeld(), u"modules/swriter/ui/autotext.ui"_ustr, u"AutoTextDialog"_ustr)
    , m_sReadonlyPath(SwResId(STR_READONLY_PATH))
    , m_pGlossaryHdl(pGlosHdl)
    , m_bResume(false)
    , m_bSelection(pWrtShell->IsSelection())
    , m_bReadOnly(false)
    , m_bIsOld(false)
    , m_bIsDocReadOnly(false)
    , m_pShell(pWrtShell)
    , m_xInsertTipCB(m_xBuilder->weld_check_button(u"inserttip"_ustr))
    , m_xNameED(m_xBuilder->weld_entry(u"name"_ustr))
    , m_xShortNameLbl(m_xBuilder->weld_label(u"shortnameft"_ustr))
    , m_xShortNameEdit(m_xBuilder->weld_entry(u"shortname"_ustr))
    , m_xCategoryBox(m_xBuilder->weld_tree_view(u"category"_ustr))
    , m_xFileRelCB(m_xBuilder->weld_check_button(u"relfile"_ustr))
    , m_xNetRelCB(m_xBuilder->weld_check_button(u"relnet"_ustr))
    , m_xInsertBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xEditBtn(m_xBuilder->weld_menu_button(u"autotext"_ustr))
    , m_xBibBtn(m_xBuilder->weld_button(u"categories"_ustr))
    , m_xPathBtn(m_xBuilder->weld_button(u"path"_ustr))
{
    m_xCategoryBox->set_size_request(m_xCategoryBox->get_approximate_digit_width() * 52,
                                     m_xCategoryBox->get_height_rows(12));

    Link<SwOneExampleFrame&,void> aLink(LINK(this, SwGlossaryDlg, PreviewLoadedHdl));
    m_xExampleFrame.reset(new SwOneExampleFrame(EX_SHOW_ONLINE_LAYOUT, &aLink));
    m_xExampleFrameWin.reset(new weld::CustomWeld(*m_xBuilder, u"example"_ustr, *m_xExampleFrame));
    Size aSize = m_xExampleFrame->GetDrawingArea()->get_ref_device().LogicToPixel(
            Size(82, 124), MapMode(MapUnit::MapAppFont));
    m_xExampleFrame->set_size_request(aSize.Width(), aSize.Height());

    m_xShortNameEdit->connect_insert_text(LINK(this, SwGlossaryDlg, TextFilterHdl));

    m_xEditBtn->connect_toggled(LINK(this, SwGlossaryDlg, EnableHdl));
    m_xEditBtn->connect_selected(LINK(this, SwGlossaryDlg, MenuHdl));
    m_xPathBtn->connect_clicked(LINK(this, SwGlossaryDlg, PathHdl));

    m_xNameED->connect_changed(LINK(this,SwGlossaryDlg,NameModify));
    m_xShortNameEdit->connect_changed(LINK(this,SwGlossaryDlg,NameModify));

    m_xCategoryBox->connect_row_activated(LINK(this, SwGlossaryDlg, NameDoubleClick));
    m_xCategoryBox->connect_changed(LINK(this, SwGlossaryDlg, GrpSelect));
    m_xCategoryBox->connect_key_press(LINK(this, SwGlossaryDlg, KeyInputHdl));

    m_xDropTarget.reset(new SwGlossaryDropTarget(*m_xCategoryBox, pGlosHdl));
    rtl::Reference<TransferDataContainer> xHelper(new TransferDataContainer);
    m_xCategoryBox->enable_drag_source(xHelper, DND_ACTION_COPYMOVE);

    m_xBibBtn->connect_clicked(LINK(this,SwGlossaryDlg,BibHdl));

    m_xInsertBtn->connect_clicked(LINK(this,SwGlossaryDlg,InsertHdl));

    ShowPreview();

    m_bIsDocReadOnly = m_pShell->GetView().GetDocShell()->IsReadOnly() ||
                      m_pShell->HasReadonlySel();
    if( m_bIsDocReadOnly )
        m_xInsertBtn->set_sensitive(false);
    m_xNameED->grab_focus();
    m_xCategoryBox->make_sorted();
    m_xCategoryBox->set_sort_order(true);

    Init();
}

SwGlossaryDlg::~SwGlossaryDlg()
{
}

namespace
{

OUString getCurrentGlossary()
{
    const OUString sTemp{ ::GetCurrGlosGroup() };

    // the zeroth path is not being recorded!
    if (o3tl::starts_with(o3tl::getToken(sTemp, 1, GLOS_DELIM), u"0"))
        return sTemp.getToken(0, GLOS_DELIM);

    return sTemp;
}

}

// select new group
IMPL_LINK(SwGlossaryDlg, GrpSelect, weld::TreeView&, rBox, void)
{
    std::unique_ptr<weld::TreeIter> xEntry = rBox.make_iterator();
    if (!rBox.get_selected(xEntry.get()))
        return;

    std::unique_ptr<weld::TreeIter> xParent = rBox.make_iterator(xEntry.get());
    weld::TreeIter* pParent;
    if (rBox.get_iter_depth(*xParent))
    {
        rBox.iter_parent(*xParent);
        pParent = xParent.get();
    }
    else
        pParent = xEntry.get();
    GroupUserData* pGroupData = weld::fromId<GroupUserData*>(rBox.get_id(*pParent));
    ::SetCurrGlosGroup(pGroupData->sGroupName
        + OUStringChar(GLOS_DELIM)
        + OUString::number(pGroupData->nPathIdx));
    m_pGlossaryHdl->SetCurGroup(::GetCurrGlosGroup());
    // set current text block
    m_bReadOnly = m_pGlossaryHdl->IsReadOnly();
    EnableShortName( !m_bReadOnly );
    m_xEditBtn->set_sensitive(!m_bReadOnly);
    m_bIsOld = m_pGlossaryHdl->IsOld();
    if( pParent != xEntry.get())
    {
        OUString aName(rBox.get_text(*xEntry));
        m_xNameED->set_text(aName);
        m_xShortNameEdit->set_text(rBox.get_id(*xEntry));
        m_xInsertBtn->set_sensitive( !m_bIsDocReadOnly);
        ShowAutoText(::GetCurrGlosGroup(), m_xShortNameEdit->get_text());
    }
    else
        ShowAutoText(u""_ustr, u""_ustr);
    // update controls
    NameModify(*m_xShortNameEdit);
    if (SfxRequest::HasMacroRecorder(m_pShell->GetView().GetViewFrame()))
    {
        SfxRequest aReq(m_pShell->GetView().GetViewFrame(), FN_SET_ACT_GLOSSARY);
        aReq.AppendItem(SfxStringItem(FN_SET_ACT_GLOSSARY, getCurrentGlossary()));
        aReq.Done();
    }
}

void SwGlossaryDlg::Apply()
{
    const OUString aGlosName(m_xShortNameEdit->get_text());
    if (!aGlosName.isEmpty())
    {
        m_pGlossaryHdl->InsertGlossary(aGlosName);
    }
    if (SfxRequest::HasMacroRecorder(m_pShell->GetView().GetViewFrame()))
    {
        SfxRequest aReq(m_pShell->GetView().GetViewFrame(), FN_INSERT_GLOSSARY);
        aReq.AppendItem(SfxStringItem(FN_INSERT_GLOSSARY, getCurrentGlossary()));
        aReq.AppendItem(SfxStringItem(FN_PARAM_1, aGlosName));
        aReq.Done();
    }
}

void SwGlossaryDlg::EnableShortName(bool bOn)
{
    m_xShortNameLbl->set_sensitive(bOn);
    m_xShortNameEdit->set_sensitive(bOn);
}

// does the title exist in the selected group?
std::unique_ptr<weld::TreeIter> SwGlossaryDlg::DoesBlockExist(std::u16string_view rBlock,
                                                              std::u16string_view rShort)
{
    // look for possible entry in TreeListBox
    std::unique_ptr<weld::TreeIter> xEntry = m_xCategoryBox->make_iterator();
    if (m_xCategoryBox->get_selected(xEntry.get()))
    {
        if (m_xCategoryBox->get_iter_depth(*xEntry))
            m_xCategoryBox->iter_parent(*xEntry);
        if (!m_xCategoryBox->iter_children(*xEntry))
            return nullptr;
        do
        {
            if (rBlock == m_xCategoryBox->get_text(*xEntry) &&
                (rShort.empty() ||
                 rShort == m_xCategoryBox->get_id(*xEntry))
               )
            {
                return xEntry;
            }
        }
        while (m_xCategoryBox->iter_next_sibling(*xEntry));
    }
    return nullptr;
}

IMPL_LINK(SwGlossaryDlg, NameModify, weld::Entry&, rEdit, void)
{
    const OUString aName(m_xNameED->get_text());
    bool bNameED = &rEdit == m_xNameED.get();
    if( aName.isEmpty() )
    {
        if(bNameED)
            m_xShortNameEdit->set_text(aName);
        m_xInsertBtn->set_sensitive(false);
        return;
    }
    const bool bNotFound = !DoesBlockExist(aName, bNameED ? OUString() : rEdit.get_text());
    if(bNameED)
    {
            // did the text get in to the Listbox in the Edit with a click?
        if(bNotFound)
        {
            m_xShortNameEdit->set_text( lcl_GetValidShortCut( aName ) );
            EnableShortName();
        }
        else
        {
            m_xShortNameEdit->set_text(m_pGlossaryHdl->GetGlossaryShortName(aName));
            EnableShortName(!m_bReadOnly);
        }
        m_xInsertBtn->set_sensitive(!bNotFound && !m_bIsDocReadOnly);
    }
    else
    {
        //ShortNameEdit
        if(!bNotFound)
        {
            m_xInsertBtn->set_sensitive(!m_bIsDocReadOnly);
        }
    }
}

IMPL_LINK( SwGlossaryDlg, NameDoubleClick, weld::TreeView&, rBox, bool )
{
    std::unique_ptr<weld::TreeIter> xEntry = rBox.make_iterator();
    if (rBox.get_selected(xEntry.get()) && rBox.get_iter_depth(*xEntry) && !m_bIsDocReadOnly)
        m_xDialog->response(RET_OK);
    return true;
}

IMPL_LINK_NOARG( SwGlossaryDlg, EnableHdl, weld::Toggleable&, void )
{
    std::unique_ptr<weld::TreeIter> xEntry = m_xCategoryBox->make_iterator();
    bool bEntry = m_xCategoryBox->get_selected(xEntry.get());

    const OUString aEditText(m_xNameED->get_text());
    const bool bHasEntry = !aEditText.isEmpty() && !m_xShortNameEdit->get_text().isEmpty();
    const bool bExists = nullptr != DoesBlockExist(aEditText, m_xShortNameEdit->get_text());
    const bool bIsGroup = bEntry && !m_xCategoryBox->get_iter_depth(*xEntry);
    m_xEditBtn->set_item_visible(u"new"_ustr, m_bSelection && bHasEntry && !bExists);
    m_xEditBtn->set_item_visible(u"newtext"_ustr, m_bSelection && bHasEntry && !bExists);
    m_xEditBtn->set_item_visible(u"copy"_ustr, bExists && !bIsGroup);
    m_xEditBtn->set_item_visible(u"replace"_ustr, m_bSelection && bExists && !bIsGroup && !m_bIsOld );
    m_xEditBtn->set_item_visible(u"replacetext"_ustr, m_bSelection && bExists && !bIsGroup && !m_bIsOld );
    m_xEditBtn->set_item_visible(u"edit"_ustr, bExists && !bIsGroup );
    m_xEditBtn->set_item_visible(u"rename"_ustr, bExists && !bIsGroup );
    m_xEditBtn->set_item_visible(u"delete"_ustr, bExists && !bIsGroup );
    m_xEditBtn->set_item_visible(u"macro"_ustr, bExists && !bIsGroup && !m_bIsOld &&
                                    !m_pGlossaryHdl->IsReadOnly() );
    m_xEditBtn->set_item_visible(u"import"_ustr, bIsGroup && !m_bIsOld && !m_pGlossaryHdl->IsReadOnly() );
}

IMPL_LINK(SwGlossaryDlg, MenuHdl, const OUString&, rItemIdent, void)
{
    if (rItemIdent == "edit")
    {
        std::unique_ptr<SwTextBlocks> pGroup = ::GetGlossaries()->GetGroupDoc (  GetCurrGrpName () );
        pGroup.reset();
        m_xDialog->response(RET_EDIT);
    }
    else if (rItemIdent == "replace")
    {
        m_pGlossaryHdl->NewGlossary(m_xNameED->get_text(),
                                    m_xShortNameEdit->get_text());
    }
    else if (rItemIdent == "replacetext")
    {
        m_pGlossaryHdl->NewGlossary(m_xNameED->get_text(),
                                    m_xShortNameEdit->get_text(),
                                    false, true);
    }
    else if (rItemIdent == "new" || rItemIdent == "newtext")
    {
        bool bNoAttr = rItemIdent == "newtext";

        const OUString aStr(m_xNameED->get_text());
        const OUString aShortName(m_xShortNameEdit->get_text());
        if(m_pGlossaryHdl->HasShortName(aShortName))
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          SwResId(STR_DOUBLE_SHORTNAME)));
            xInfoBox->run();
            m_xShortNameEdit->select_region(0, -1);
            m_xShortNameEdit->grab_focus();
        }
        if(m_pGlossaryHdl->NewGlossary(aStr, aShortName, false, bNoAttr ))
        {
            std::unique_ptr<weld::TreeIter> xEntry = m_xCategoryBox->make_iterator();
            if (!m_xCategoryBox->get_selected(xEntry.get()))
                xEntry.reset();
            else if (m_xCategoryBox->get_iter_depth(*xEntry))
                m_xCategoryBox->iter_parent(*xEntry);
            m_xCategoryBox->insert(xEntry.get(), -1, &aStr, &aShortName,
                                   nullptr, nullptr, false, nullptr);

            m_xNameED->set_text(aStr);
            m_xShortNameEdit->set_text(aShortName);
            NameModify(*m_xNameED);       // for toggling the buttons

            if (SfxRequest::HasMacroRecorder(m_pShell->GetView().GetViewFrame()))
            {
                SfxRequest aReq(m_pShell->GetView().GetViewFrame(), FN_NEW_GLOSSARY);
                aReq.AppendItem(SfxStringItem(FN_NEW_GLOSSARY, getCurrentGlossary()));
                aReq.AppendItem(SfxStringItem(FN_PARAM_1, aShortName));
                aReq.AppendItem(SfxStringItem(FN_PARAM_2, aStr));
                aReq.Done();
            }
        }
    }
    else if (rItemIdent == "copy")
    {
        m_pGlossaryHdl->CopyToClipboard(*m_pShell, m_xShortNameEdit->get_text());
    }
    else if (rItemIdent == "rename")
    {
        m_xShortNameEdit->set_text(m_pGlossaryHdl->GetGlossaryShortName(m_xNameED->get_text()));
        SwNewGlosNameDlg aNewNameDlg(this, m_xNameED->get_text(), m_xShortNameEdit->get_text());
        if (aNewNameDlg.run() == RET_OK && m_pGlossaryHdl->Rename(m_xShortNameEdit->get_text(),
                                                                  aNewNameDlg.GetNewShort(),
                                                                  aNewNameDlg.GetNewName()))
        {
            std::unique_ptr<weld::TreeIter> xEntry = m_xCategoryBox->make_iterator();
            if (m_xCategoryBox->get_selected(xEntry.get()))
            {
                std::unique_ptr<weld::TreeIter> xOldEntry = m_xCategoryBox->make_iterator(xEntry.get());
                if (m_xCategoryBox->get_iter_depth(*xEntry))
                    m_xCategoryBox->iter_parent(*xEntry);

                std::unique_ptr<weld::TreeIter> xNewEntry = m_xCategoryBox->make_iterator();
                OUString sId(aNewNameDlg.GetNewShort());
                OUString sName(aNewNameDlg.GetNewName());

                m_xCategoryBox->insert(xEntry.get(), -1, &sName, &sId,
                                       nullptr, nullptr, false, xNewEntry.get());

                m_xCategoryBox->remove(*xOldEntry);
                m_xCategoryBox->select(*xNewEntry);
                m_xCategoryBox->scroll_to_row(*xNewEntry);
            }
        }
        GrpSelect(*m_xCategoryBox);
    }
    else if (rItemIdent == "delete")
    {
        DeleteEntry();
    }
    else if (rItemIdent == "macro")
    {
        SfxItemSetFixed<RES_FRMMACRO, RES_FRMMACRO, SID_EVENTCONFIG, SID_EVENTCONFIG> aSet( m_pShell->GetAttrPool() );

        SvxMacro aStart(OUString(), OUString(), STARBASIC);
        SvxMacro aEnd(OUString(), OUString(), STARBASIC);
        m_pGlossaryHdl->GetMacros(m_xShortNameEdit->get_text(), aStart, aEnd );

        SvxMacroItem aItem(RES_FRMMACRO);
        if( aStart.HasMacro() )
            aItem.SetMacro( SvMacroItemId::SwStartInsGlossary, aStart );
        if( aEnd.HasMacro() )
            aItem.SetMacro( SvMacroItemId::SwEndInsGlossary, aEnd );

        aSet.Put( aItem );
        aSet.Put( SwMacroAssignDlg::AddEvents( MACASSGN_AUTOTEXT ) );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        VclPtr<SfxAbstractDialog> pMacroDlg(pFact->CreateEventConfigDialog(m_xDialog.get(), aSet,
            m_pShell->GetView().GetViewFrame().GetFrame().GetFrameInterface() ));
        if (pMacroDlg)
            pMacroDlg->StartExecuteAsync(
                [this, pMacroDlg] (sal_Int32 nResult)->void
                {
                    if (nResult == RET_OK)
                    {
                        if (const SvxMacroItem* pMacroItem = pMacroDlg->GetOutputItemSet()->GetItemIfSet( RES_FRMMACRO, false ))
                        {
                            const SvxMacroTableDtor& rTable = pMacroItem->GetMacroTable();
                            m_pGlossaryHdl->SetMacros( m_xShortNameEdit->get_text(),
                                                        rTable.Get( SvMacroItemId::SwStartInsGlossary ),
                                                        rTable.Get( SvMacroItemId::SwEndInsGlossary ) );
                        }
                    }
                    pMacroDlg->disposeOnce();
                }
            );
    }
    else if (rItemIdent == "import")
    {
        // call the FileOpenDialog do find WinWord - Files with templates
        FileDialogHelper aDlgHelper(TemplateDescription::FILEOPEN_SIMPLE,
                                    FileDialogFlags::NONE, m_xDialog.get());
        aDlgHelper.SetContext(FileDialogHelper::WriterImportAutotext);
        uno::Reference < XFilePicker3 > xFP = aDlgHelper.GetFilePicker();

        SfxFilterMatcher aMatcher( SwDocShell::Factory().GetFactoryName() );
        SfxFilterMatcherIter aIter( aMatcher );
        std::shared_ptr<const SfxFilter> pFilter = aIter.First();
        while ( pFilter )
        {
            if( pFilter->GetUserData() == FILTER_WW8 )
            {
                xFP->appendFilter( pFilter->GetUIName(),
                            pFilter->GetWildcard().getGlob() );
                xFP->setCurrentFilter( pFilter->GetUIName() ) ;
            }
            else if( pFilter->GetUserData() == FILTER_DOCX )
            {
                xFP->appendFilter( pFilter->GetUIName(),
                            pFilter->GetWildcard().getGlob() );
                xFP->setCurrentFilter( pFilter->GetUIName() ) ;
            }

            pFilter = aIter.Next();
        }

        if( aDlgHelper.Execute() == ERRCODE_NONE )
        {
            if( m_pGlossaryHdl->ImportGlossaries( xFP->getSelectedFiles().getConstArray()[0] ))
                Init();
            else
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SwResId(STR_NO_GLOSSARIES)));
                xInfoBox->run();
            }
        }
    }
}

// dialog manage regions
IMPL_LINK_NOARG(SwGlossaryDlg, BibHdl, weld::Button&, void)
{
    SwGlossaries* pGloss = ::GetGlossaries();
    if( pGloss->IsGlosPathErr() )
        pGloss->ShowError();
    else
    {
        //check if at least one glossary path is write enabled
        SvtPathOptions aPathOpt;
        const OUString& sGlosPath( aPathOpt.GetAutoTextPath() );
        bool bIsWritable = false;
        sal_Int32 nIdx {sGlosPath.isEmpty() ? -1 : 0};
        while (nIdx>=0)
        {
            const OUString sPath = URIHelper::SmartRel2Abs(
                INetURLObject(), sGlosPath.getToken(0, ';', nIdx),
                URIHelper::GetMaybeFileHdl());
            try
            {
                Content aTestContent( sPath,
                            uno::Reference< XCommandEnvironment >(),
                            comphelper::getProcessComponentContext() );
                Any aAny = aTestContent.getPropertyValue( u"IsReadOnly"_ustr );
                if(aAny.hasValue())
                {
                    bIsWritable = !*o3tl::doAccess<bool>(aAny);
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

            SwGlossaryGroupDlg aDlg(m_xDialog.get(), pGloss->GetPathArray(), m_pGlossaryHdl);
            if (aDlg.run() == RET_OK)
            {
                Init();
                //if new groups were created - select one of them
                const OUString sNewGroup = aDlg.GetCreatedGroupName();

                std::unique_ptr<weld::TreeIter> xEntry = m_xCategoryBox->make_iterator();
                bool bEntry = m_xCategoryBox->get_iter_first(*xEntry);

                while (!sNewGroup.isEmpty() && bEntry)
                {
                    if (!m_xCategoryBox->get_iter_depth(*xEntry))
                    {
                        GroupUserData* pGroupData = weld::fromId<GroupUserData*>(m_xCategoryBox->get_id(*xEntry));
                        const OUString sGroup = pGroupData->sGroupName
                            + OUStringChar(GLOS_DELIM)
                            + OUString::number(pGroupData->nPathIdx);
                        if(sGroup == sNewGroup)
                        {
                            m_xCategoryBox->select(*xEntry);
                            m_xCategoryBox->scroll_to_row(*xEntry);
                            GrpSelect(*m_xCategoryBox);
                            break;
                        }
                    }
                    bEntry = m_xCategoryBox->iter_next(*xEntry);
                }

            }
        }
        else
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Question, VclButtonsType::YesNo,
                                                      m_sReadonlyPath));
            if (RET_YES == xBox->run())
                PathHdl(*m_xPathBtn);
        }
    }
}

// initialisation; from Ctor and after editing regions
void SwGlossaryDlg::Init()
{
    m_xCategoryBox->freeze();
    m_xCategoryBox->clear();
    m_xGroupData.clear();
    m_xCategoryBox->make_unsorted();

    // display text block regions
    const size_t nCnt = m_pGlossaryHdl->GetGroupCnt();
    std::unique_ptr<weld::TreeIter> xSelEntry;
    const OUString sSelStr(::GetCurrGlosGroup().getToken(0, GLOS_DELIM));
    const sal_Int32 nSelPath = o3tl::toInt32(o3tl::getToken(::GetCurrGlosGroup(), 1, GLOS_DELIM));
    // #i66304# - "My AutoText" comes from mytexts.bau, but should be translated
    static constexpr OUStringLiteral sMyAutoTextEnglish(u"My AutoText");
    const OUString sMyAutoTextTranslated(SwResId(STR_MY_AUTOTEXT));
    for(size_t nId = 0; nId < nCnt; ++nId )
    {
        OUString sTitle;
        OUString sGroupName(m_pGlossaryHdl->GetGroupName(nId, &sTitle));
        if(sGroupName.isEmpty())
            continue;
        sal_Int32 nIdx{ 0 };
        const OUString sName{ sGroupName.getToken( 0, GLOS_DELIM, nIdx ) };
        if(sTitle.isEmpty())
            sTitle = sName;
        if(sTitle == sMyAutoTextEnglish)
            sTitle = sMyAutoTextTranslated;

        std::unique_ptr<weld::TreeIter> xEntry = m_xCategoryBox->make_iterator();
        m_xCategoryBox->append(xEntry.get());
        m_xCategoryBox->set_text(*xEntry, sTitle, 0);
        const sal_Int32 nPath = o3tl::toInt32(o3tl::getToken(sGroupName, 0, GLOS_DELIM, nIdx ));

        GroupUserData* pData = new GroupUserData;
        pData->sGroupName = sName;
        pData->nPathIdx = static_cast< sal_uInt16 >(nPath);
        pData->bReadonly = m_pGlossaryHdl->IsReadOnly(&sGroupName);
        m_xGroupData.emplace_back(pData);

        m_xCategoryBox->set_id(*xEntry, weld::toId(pData));
        if (sSelStr == pData->sGroupName && nSelPath == nPath)
            xSelEntry = m_xCategoryBox->make_iterator(xEntry.get());

        // fill entries for the groups
        {
            m_pGlossaryHdl->SetCurGroup(sGroupName, false, true);
            const sal_uInt16 nCount = m_pGlossaryHdl->GetGlossaryCnt();
            for(sal_uInt16 i = 0; i < nCount; ++i)
            {
                OUString sEntryName = m_pGlossaryHdl->GetGlossaryName(i);
                OUString sId = m_pGlossaryHdl->GetGlossaryShortName(i);
                m_xCategoryBox->insert(xEntry.get(), -1, &sEntryName, &sId,
                                       nullptr, nullptr, false, nullptr);
            }
        }
    }
        // set current group and display text blocks
    if (!xSelEntry)
    {
        //find a non-readonly group
        std::unique_ptr<weld::TreeIter> xSearch = m_xCategoryBox->make_iterator();
        if (m_xCategoryBox->get_iter_first(*xSearch))
        {
            do
            {
                if (!m_xCategoryBox->get_iter_depth(*xSearch))
                {
                    GroupUserData* pData = weld::fromId<GroupUserData*>(m_xCategoryBox->get_id(*xSearch));
                    if (!pData->bReadonly)
                    {
                        xSelEntry = std::move(xSearch);
                        break;
                    }
                }
            }
            while (m_xCategoryBox->iter_next(*xSearch));
        }
        if (!xSelEntry)
        {
            xSelEntry = std::move(xSearch);
            if (!xSelEntry || !m_xCategoryBox->get_iter_first(*xSelEntry))
                xSelEntry.reset();
        }
    }

    m_xCategoryBox->thaw();
    m_xCategoryBox->make_sorted();

    if (xSelEntry)
    {
        m_xCategoryBox->expand_row(*xSelEntry);
        m_xCategoryBox->select(*xSelEntry);
        m_xCategoryBox->scroll_to_row(*xSelEntry);
        GrpSelect(*m_xCategoryBox);
    }

    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    m_xFileRelCB->set_active( rCfg.IsSaveRelFile() );
    m_xFileRelCB->connect_toggled(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    m_xNetRelCB->set_active( rCfg.IsSaveRelNet() );
    m_xNetRelCB->connect_toggled(LINK(this, SwGlossaryDlg, CheckBoxHdl));
    m_xInsertTipCB->set_active( rCfg.IsAutoTextTip() );
    m_xInsertTipCB->set_sensitive(!officecfg::Office::Writer::AutoFunction::Text::ShowToolTip::isReadOnly());
    m_xInsertTipCB->connect_toggled(LINK(this, SwGlossaryDlg, CheckBoxHdl));

    // tdf#124088 - propose autotext and shortcut name based on selected text
    if (m_pShell->HasSelection())
    {
        OUString aSelText;
        m_pShell->GetSelectedText(aSelText, ParaBreakType::ToBlank);

        aSelText = aSelText.trim();
        if (aSelText.getLength() > 25)
        {
            aSelText = aSelText.copy(0, 25);
            if (const sal_Int32 nBlankIndex = aSelText.lastIndexOf(' '); nBlankIndex != -1)
                aSelText = aSelText.copy(0, nBlankIndex);
        }

        m_xNameED->set_text(aSelText);
        m_xNameED->select_region(0, -1);
        m_xShortNameEdit->set_text(lcl_GetValidShortCut(aSelText));
    }
}

// KeyInput for ShortName - Edits without Spaces
IMPL_LINK( SwNewGlosNameDlg, Modify, weld::Entry&, rBox, void )
{
    OUString aName(m_xNewName->get_text());
    SwGlossaryDlg* pDlg = m_pParent;
    if (&rBox == m_xNewName.get())
        m_xNewShort->set_text(lcl_GetValidShortCut(aName));

    bool bEnable = !aName.isEmpty() && !m_xNewShort->get_text().isEmpty() &&
        (!pDlg->DoesBlockExist(aName, m_xNewShort->get_text())
            || aName == m_xOldName->get_text());
    m_xOk->set_sensitive(bEnable);
}

IMPL_LINK_NOARG(SwNewGlosNameDlg, Rename, weld::Button&, void)
{
    SwGlossaryDlg* pDlg = m_pParent;
    OUString sNew = GetAppCharClass().uppercase(m_xNewShort->get_text());
    if (pDlg->m_pGlossaryHdl->HasShortName(m_xNewShort->get_text())
        && sNew != m_xOldShort->get_text())
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                  SwResId(STR_DOUBLE_SHORTNAME)));
        xBox->run();
        m_xNewShort->grab_focus();
    }
    else
        m_xDialog->response(RET_OK);
}

IMPL_LINK(SwGlossaryDlg, CheckBoxHdl, weld::Toggleable&, rBox, void)
{
    SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    bool bCheck = rBox.get_active();
    if (&rBox == m_xInsertTipCB.get())
        rCfg.SetAutoTextTip(bCheck);
    else if (&rBox == m_xFileRelCB.get())
        rCfg.SetSaveRelFile(bCheck);
    else
        rCfg.SetSaveRelNet(bCheck);
    rCfg.Commit();
}

IMPL_LINK(SwGlossaryDlg, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_DELETE)
    {
        DeleteEntry();
        return true;
    }
    return false;
}

OUString SwGlossaryDlg::GetCurrGrpName() const
{
    std::unique_ptr<weld::TreeIter> xEntry = m_xCategoryBox->make_iterator();
    if (m_xCategoryBox->get_selected(xEntry.get()))
    {
        if (m_xCategoryBox->get_iter_depth(*xEntry))
            m_xCategoryBox->iter_parent(*xEntry);
        GroupUserData* pGroupData = weld::fromId<GroupUserData*>(m_xCategoryBox->get_id(*xEntry));
        return pGroupData->sGroupName + OUStringChar(GLOS_DELIM) + OUString::number(pGroupData->nPathIdx);
    }
    return OUString();
}

IMPL_LINK_NOARG( SwGlossaryDlg, PathHdl, weld::Button&, void )
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxMultiPathDialog> pDlg(pFact->CreateSvxPathSelectDialog(m_xDialog.get()));
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

IMPL_LINK_NOARG(SwGlossaryDlg, InsertHdl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

void SwGlossaryDlg::ShowPreview()
{
    ShowAutoText(::GetCurrGlosGroup(), m_xShortNameEdit->get_text());
};

IMPL_LINK_NOARG(SwGlossaryDlg, PreviewLoadedHdl, SwOneExampleFrame&, void)
{
    ResumeShowAutoText();
}

void SwGlossaryDlg::ShowAutoText(const OUString& rGroup, const OUString& rShortName)
{
    if (m_xExampleFrameWin->get_visible())
    {
        SetResumeData(rGroup, rShortName);
        //try to make an Undo()
        m_xExampleFrame->ClearDocument();
    }
}

void SwGlossaryDlg::ResumeShowAutoText()
{
    OUString sGroup;
    OUString sShortName;
    if(GetResumeData(sGroup, sShortName) && m_xExampleFrameWin->get_visible())
    {
        if(!m_xAutoText.is())
        {
            //now the AutoText ListBoxes have to be filled
            m_xAutoText = text::AutoTextContainer::create( comphelper::getProcessComponentContext() );
        }

        rtl::Reference< SwXTextCursor > & xCursor = m_xExampleFrame->GetTextCursor();
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
                    xEntry->applyTo(static_cast<XSentenceCursor*>(xCursor.get()));
                }
            }
        }
    }
    m_bResume = false;
}

void SwGlossaryDlg::DeleteEntry()
{
    bool bEntry = m_xCategoryBox->get_selected(nullptr);

    const OUString aTitle(m_xNameED->get_text());
    const OUString aShortName(m_xShortNameEdit->get_text());

    std::unique_ptr<weld::TreeIter> xParent;
    std::unique_ptr<weld::TreeIter> xChild = DoesBlockExist(aTitle, aShortName);
    if (xChild && m_xCategoryBox->get_iter_depth(*xChild))
    {
        xParent = m_xCategoryBox->make_iterator(xChild.get());
        m_xCategoryBox->iter_parent(*xParent);
    }

    const bool bExists = nullptr != xChild;
    const bool bIsGroup = bEntry && !xParent;

    std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(m_xDialog.get(),
                                                VclMessageType::Question, VclButtonsType::YesNo,
                                                SwResId(STR_QUERY_DELETE)));
    if (bExists && !bIsGroup && RET_YES == xQuery->run())
    {
        if (!aTitle.isEmpty() && m_pGlossaryHdl->DelGlossary(aShortName))
        {
            OSL_ENSURE(xChild, "entry not found!");
            m_xCategoryBox->select(*xParent);
            m_xCategoryBox->remove(*xChild);
            m_xNameED->set_text(OUString());
            NameModify(*m_xNameED);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
