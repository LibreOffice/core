/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "condformatmgr.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "condformatdlg.hxx"
#include <vcl/msgbox.hxx>
#include "document.hxx"

ScCondFormatManagerWindow::ScCondFormatManagerWindow(SvSimpleTableContainer& rParent,
    ScDocument* pDoc, ScConditionalFormatList* pFormatList)
    : SvSimpleTable(rParent, WB_HSCROLL | WB_SORT | WB_TABSTOP)
    , mpDoc(pDoc)
    , mpFormatList(pFormatList)
{
    OUString aConditionStr(ScGlobal::GetRscString(STR_HEADER_COND));
    OUString aRangeStr(ScGlobal::GetRscString(STR_HEADER_RANGE));

    OUStringBuffer sHeader;
    sHeader.append(aRangeStr).append("\t").append(aConditionStr);
    InsertHeaderEntry(sHeader.makeStringAndClear(), HEADERBAR_APPEND, HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER);
    setColSizes();

    Init();
    Show();
    SetSelectionMode(MULTIPLE_SELECTION);
}

OUString ScCondFormatManagerWindow::createEntryString(const ScConditionalFormat& rFormat)
{
    ScRangeList aRange = rFormat.GetRange();
    OUString aStr;
    aRange.Format(aStr, SCA_VALID, mpDoc, mpDoc->GetAddressConvention());
    aStr += "\t";
    aStr += ScCondFormatHelper::GetExpression(rFormat, aRange.GetTopLeftCorner());
    return aStr;
}

void ScCondFormatManagerWindow::Init()
{
    SetUpdateMode(false);

    if (mpFormatList)
    {
        for(ScConditionalFormatList::iterator itr = mpFormatList->begin(); itr != mpFormatList->end(); ++itr)
        {
            SvTreeListEntry* pEntry = InsertEntryToColumn( createEntryString(**itr) );
            maMapLBoxEntryToCondIndex.insert(std::pair<SvTreeListEntry*,sal_Int32>(pEntry, (*itr)->GetKey()));
        }
    }

    SetUpdateMode(true);

    if (mpFormatList && mpFormatList->size())
        SelectRow(0);
}

void ScCondFormatManagerWindow::Resize()
{
    SvSimpleTable::Resize();
    if (GetParentDialog()->isCalculatingInitialLayoutSize())
        setColSizes();
}

void ScCondFormatManagerWindow::DeleteSelection()
{
    if(GetSelectionCount())
    {
        for(SvTreeListEntry* pEntry = FirstSelected(); pEntry != nullptr; pEntry = NextSelected(pEntry))
        {
            sal_Int32 nIndex = maMapLBoxEntryToCondIndex.find(pEntry)->second;
            mpFormatList->erase(nIndex);
        }
        RemoveSelection();
    }
}

ScConditionalFormat* ScCondFormatManagerWindow::GetSelection()
{
    SvTreeListEntry* pEntry = FirstSelected();
    if(!pEntry)
        return nullptr;

    sal_Int32 nIndex = maMapLBoxEntryToCondIndex.find(pEntry)->second;
    return mpFormatList->GetFormat(nIndex);
}

void ScCondFormatManagerWindow::setColSizes()
{
    HeaderBar &rBar = GetTheHeaderBar();
    if (rBar.GetItemCount() < 2)
        return;
    long aStaticTabs[]= { 2, 0, 0 };
    aStaticTabs[2] = rBar.GetSizePixel().Width() / 2;
    SvSimpleTable::SetTabs(aStaticTabs, MAP_PIXEL);
}

ScCondFormatManagerDlg::ScCondFormatManagerDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList, const ScAddress& rPos):
    ModalDialog(pParent, "CondFormatManager", "modules/scalc/ui/condformatmanager.ui"),
    mpFormatList( pFormatList ? new ScConditionalFormatList(*pFormatList) : nullptr),
    mpDoc(pDoc),
    maPos(rPos),
    mbModified(false)
{
    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("CONTAINER");
    Size aSize(LogicToPixel(Size(290, 220), MAP_APPFONT));
    pContainer->set_width_request(aSize.Width());
    pContainer->set_height_request(aSize.Height());
    m_pCtrlManager = VclPtr<ScCondFormatManagerWindow>::Create(*pContainer, mpDoc, mpFormatList);
    get(m_pBtnAdd, "add");
    get(m_pBtnRemove, "remove");
    get(m_pBtnEdit, "edit");

    m_pBtnRemove->SetClickHdl(LINK(this, ScCondFormatManagerDlg, RemoveBtnHdl));
    m_pBtnEdit->SetClickHdl(LINK(this, ScCondFormatManagerDlg, EditBtnClickHdl));
    m_pBtnAdd->SetClickHdl(LINK(this, ScCondFormatManagerDlg, AddBtnHdl));
    m_pCtrlManager->SetDoubleClickHdl(LINK(this, ScCondFormatManagerDlg, EditBtnHdl));
}

ScCondFormatManagerDlg::~ScCondFormatManagerDlg()
{
    disposeOnce();
}

void ScCondFormatManagerDlg::dispose()
{
    delete mpFormatList;
    m_pBtnAdd.clear();
    m_pBtnRemove.clear();
    m_pBtnEdit.clear();
    m_pCtrlManager.disposeAndClear();
    ModalDialog::dispose();
}


ScConditionalFormatList* ScCondFormatManagerDlg::GetConditionalFormatList()
{
    ScConditionalFormatList* pList = mpFormatList;
    mpFormatList = nullptr;
    return pList;
}

// ---------------------------------------------------------------
// Get the current conditional format selected.
//
ScConditionalFormat* ScCondFormatManagerDlg::GetCondFormatSelected()
{
    return m_pCtrlManager->GetSelection();
}

IMPL_LINK_NOARG_TYPED(ScCondFormatManagerDlg, RemoveBtnHdl, Button*, void)
{
    m_pCtrlManager->DeleteSelection();
    mbModified = true;
}

IMPL_LINK_NOARG_TYPED(ScCondFormatManagerDlg, EditBtnClickHdl, Button*, void)
{
    EditBtnHdl(nullptr);
}
IMPL_LINK_NOARG_TYPED(ScCondFormatManagerDlg, EditBtnHdl, SvTreeListBox*, bool)
{
    ScConditionalFormat* pFormat = m_pCtrlManager->GetSelection();

    if(!pFormat)
        return false;

    EndDialog( DLG_RET_EDIT );

    return false;
}

IMPL_LINK_NOARG_TYPED(ScCondFormatManagerDlg, AddBtnHdl, Button*, void)
{
    EndDialog( DLG_RET_ADD );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
