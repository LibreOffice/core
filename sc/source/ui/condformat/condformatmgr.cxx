/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <condformatmgr.hxx>
#include <condformathelper.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <condformatdlg.hxx>
#include <document.hxx>
#include <conditio.hxx>

ScCondFormatManagerWindow::ScCondFormatManagerWindow(SvSimpleTableContainer& rParent,
    ScDocument* pDoc, ScConditionalFormatList* pFormatList)
    : SvSimpleTable(rParent, WB_HSCROLL | WB_SORT | WB_TABSTOP)
    , mpDoc(pDoc)
    , mpFormatList(pFormatList)
{
    OUString aConditionStr(ScResId(STR_HEADER_COND));
    OUString aRangeStr(ScResId(STR_HEADER_RANGE));

    OUStringBuffer sHeader;
    sHeader.append(aRangeStr).append("\t").append(aConditionStr);
    InsertHeaderEntry(sHeader.makeStringAndClear(), HEADERBAR_APPEND, HeaderBarItemBits::LEFT);
    setColSizes();

    Init();
    Show();
    SetSelectionMode(SelectionMode::Multiple);
}

OUString ScCondFormatManagerWindow::createEntryString(const ScConditionalFormat& rFormat)
{
    const ScRangeList& aRange = rFormat.GetRange();
    OUString aStr;
    aRange.Format(aStr, ScRefFlags::VALID, mpDoc, mpDoc->GetAddressConvention());
    aStr += "\t";
    aStr += ScCondFormatHelper::GetExpression(rFormat, aRange.GetTopLeftCorner());
    return aStr;
}

void ScCondFormatManagerWindow::Init()
{
    SetUpdateMode(false);

    if (mpFormatList)
    {
        for(const auto& rItem : *mpFormatList)
        {
            SvTreeListEntry* pEntry = InsertEntryToColumn( createEntryString(*rItem) );
            maMapLBoxEntryToCondIndex.insert(std::pair<SvTreeListEntry*,sal_Int32>(pEntry, rItem->GetKey()));
        }
    }

    SetUpdateMode(true);

    if (mpFormatList && !mpFormatList->empty())
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
    long aStaticTabs[]= { 0, 0 };
    aStaticTabs[1] = rBar.GetSizePixel().Width() / 2;
    SvSimpleTable::SetTabs(SAL_N_ELEMENTS(aStaticTabs), aStaticTabs, MapUnit::MapPixel);
}

ScCondFormatManagerDlg::ScCondFormatManagerDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList):
    ModalDialog(pParent, "CondFormatManager", "modules/scalc/ui/condformatmanager.ui"),
    mpFormatList( pFormatList ? new ScConditionalFormatList(*pFormatList) : nullptr),
    mbModified(false)
{
    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("CONTAINER");
    Size aSize(LogicToPixel(Size(290, 220), MapMode(MapUnit::MapAppFont)));
    pContainer->set_width_request(aSize.Width());
    pContainer->set_height_request(aSize.Height());
    m_pCtrlManager = VclPtr<ScCondFormatManagerWindow>::Create(*pContainer, pDoc, mpFormatList.get());
    get(m_pBtnAdd, "add");
    get(m_pBtnRemove, "remove");
    get(m_pBtnEdit, "edit");

    m_pBtnRemove->SetClickHdl(LINK(this, ScCondFormatManagerDlg, RemoveBtnHdl));
    m_pBtnEdit->SetClickHdl(LINK(this, ScCondFormatManagerDlg, EditBtnClickHdl));
    m_pBtnAdd->SetClickHdl(LINK(this, ScCondFormatManagerDlg, AddBtnHdl));
    m_pCtrlManager->SetDoubleClickHdl(LINK(this, ScCondFormatManagerDlg, EditBtnHdl));

    UpdateButtonSensitivity();
}

ScCondFormatManagerDlg::~ScCondFormatManagerDlg()
{
    disposeOnce();
}

void ScCondFormatManagerDlg::dispose()
{
    mpFormatList.reset();
    m_pBtnAdd.clear();
    m_pBtnRemove.clear();
    m_pBtnEdit.clear();
    m_pCtrlManager.disposeAndClear();
    ModalDialog::dispose();
}


std::unique_ptr<ScConditionalFormatList> ScCondFormatManagerDlg::GetConditionalFormatList()
{
    return std::move(mpFormatList);
}

void ScCondFormatManagerDlg::UpdateButtonSensitivity()
{
    OUString aNewSensitivity = mpFormatList->empty() ? OUString("false") : OUString("true");
    m_pBtnRemove->set_property("sensitive", aNewSensitivity);
    m_pBtnEdit->set_property("sensitive", aNewSensitivity);
}

// Get the current conditional format selected.
//
ScConditionalFormat* ScCondFormatManagerDlg::GetCondFormatSelected()
{
    return m_pCtrlManager->GetSelection();
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, RemoveBtnHdl, Button*, void)
{
    m_pCtrlManager->DeleteSelection();
    mbModified = true;
    UpdateButtonSensitivity();
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, EditBtnClickHdl, Button*, void)
{
    mbModified = true;
    EditBtnHdl(nullptr);
    // EditBtnHdl() might call EndDialog which will blow us away
    if (!IsDisposed())
        UpdateButtonSensitivity();
}
IMPL_LINK_NOARG(ScCondFormatManagerDlg, EditBtnHdl, SvTreeListBox*, bool)
{
    ScConditionalFormat* pFormat = m_pCtrlManager->GetSelection();

    if(!pFormat)
        return false;

    mbModified = true;
    EndDialog( DLG_RET_EDIT );

    return false;
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, AddBtnHdl, Button*, void)
{
    mbModified = true;
    EndDialog( DLG_RET_ADD );
}

void ScCondFormatManagerDlg::SetModified()
{
    mbModified = true;
    UpdateButtonSensitivity();
}

bool ScCondFormatManagerDlg::CondFormatsChanged() const
{
    return mbModified;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
