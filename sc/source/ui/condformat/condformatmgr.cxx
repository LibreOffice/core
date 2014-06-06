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
    InsertHeaderEntry(sHeader.makeStringAndClear(), HEADERBAR_APPEND, HIB_LEFT | HIB_VCENTER);
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

    for(ScConditionalFormatList::iterator itr = mpFormatList->begin(); itr != mpFormatList->end(); ++itr)
    {
        SvTreeListEntry* pEntry = InsertEntryToColumn( createEntryString(*itr), TREELIST_APPEND, 0xffff );
        maMapLBoxEntryToCondIndex.insert(std::pair<SvTreeListEntry*,sal_Int32>(pEntry,itr->GetKey()));
    }

    SetUpdateMode(true);

    if (mpFormatList->size())
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
        for(SvTreeListEntry* pEntry = FirstSelected(); pEntry != NULL; pEntry = NextSelected(pEntry))
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
        return NULL;

    sal_Int32 nIndex = maMapLBoxEntryToCondIndex.find(pEntry)->second;
    return mpFormatList->GetFormat(nIndex);
}

void ScCondFormatManagerWindow::Update()
{
    Clear();
    maMapLBoxEntryToCondIndex.clear();
    Init();
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

ScCondFormatManagerDlg::ScCondFormatManagerDlg(Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList, const ScAddress& rPos):
    ModalDialog(pParent, "CondFormatManager", "modules/scalc/ui/condformatmanager.ui"),
    mpFormatList( pFormatList ? new ScConditionalFormatList(*pFormatList) : NULL),
    mpDoc(pDoc),
    maPos(rPos),
    mbModified(false)
{
    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("CONTAINER");
    Size aSize(LogicToPixel(Size(290, 220), MAP_APPFONT));
    pContainer->set_width_request(aSize.Width());
    pContainer->set_height_request(aSize.Height());
    m_pCtrlManager = new ScCondFormatManagerWindow(*pContainer, mpDoc, mpFormatList);
    get(m_pBtnAdd, "add");
    get(m_pBtnRemove, "remove");
    get(m_pBtnEdit, "edit");

    m_pBtnRemove->SetClickHdl(LINK(this, ScCondFormatManagerDlg, RemoveBtnHdl));
    m_pBtnEdit->SetClickHdl(LINK(this, ScCondFormatManagerDlg, EditBtnHdl));
    m_pBtnAdd->SetClickHdl(LINK(this, ScCondFormatManagerDlg, AddBtnHdl));
    m_pCtrlManager->SetDoubleClickHdl(LINK(this, ScCondFormatManagerDlg, EditBtnHdl));
}

ScCondFormatManagerDlg::~ScCondFormatManagerDlg()
{
    delete m_pCtrlManager;
    delete mpFormatList;
}

bool ScCondFormatManagerDlg::IsInRefMode() const
{
    return true;
}

ScConditionalFormatList* ScCondFormatManagerDlg::GetConditionalFormatList()
{
    ScConditionalFormatList* pList = mpFormatList;
    mpFormatList = NULL;
    return pList;
}

bool ScCondFormatManagerDlg::CondFormatsChanged()
{
    return mbModified;
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, RemoveBtnHdl)
{
    m_pCtrlManager->DeleteSelection();
    mbModified = true;
    return 0;
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, EditBtnHdl)
{
    ScConditionalFormat* pFormat = m_pCtrlManager->GetSelection();

    if(!pFormat)
        return 0;

    sal_uInt16 nId = 1;
    ScModule* pScMod = SC_MOD();
    pScMod->SetRefDialog( nId, true );
    boost::scoped_ptr<ScCondFormatDlg> pDlg(new ScCondFormatDlg(this, mpDoc, pFormat, pFormat->GetRange(),
                                               pFormat->GetRange().GetTopLeftCorner(), condformat::dialog::NONE));
    Show(false, 0);
    if(pDlg->Execute() == RET_OK)
    {
        sal_Int32 nKey = pFormat->GetKey();
        mpFormatList->erase(nKey);
        ScConditionalFormat* pNewFormat = pDlg->GetConditionalFormat();
        if(pNewFormat)
        {
            pNewFormat->SetKey(nKey);
            mpFormatList->InsertNew(pNewFormat);
        }

        m_pCtrlManager->Update();
        mbModified = true;
    }
    Show(true, 0);

    pScMod->SetRefDialog( nId, false );

    return 0;
}

namespace {

sal_uInt32 FindKey(ScConditionalFormatList* pFormatList)
{
    sal_uInt32 nKey = 0;
    for(ScConditionalFormatList::const_iterator itr = pFormatList->begin(), itrEnd = pFormatList->end();
            itr != itrEnd; ++itr)
    {
        if(itr->GetKey() > nKey)
            nKey = itr->GetKey();
    }

    return nKey + 1;
}

}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, AddBtnHdl)
{
    sal_uInt16 nId = 1;
    ScModule* pScMod = SC_MOD();
    pScMod->SetRefDialog( nId, true );
    boost::scoped_ptr<ScCondFormatDlg> pDlg(new ScCondFormatDlg(this, mpDoc, NULL, ScRangeList(),
                                               maPos, condformat::dialog::CONDITION));
    Show(false, 0);
    if(pDlg->Execute() == RET_OK)
    {
        ScConditionalFormat* pNewFormat = pDlg->GetConditionalFormat();
        if(pNewFormat)
        {
            mpFormatList->InsertNew(pNewFormat);
            pNewFormat->SetKey(FindKey(mpFormatList));
            m_pCtrlManager->Update();

            mbModified = true;
        }
    }
    Show(true, 0);
    pScMod->SetRefDialog( nId, false );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
