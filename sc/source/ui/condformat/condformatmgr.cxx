/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "condformatmgr.hxx"
#include "condformatmgr.hrc"
#include "scresid.hxx"
#include "globstr.hrc"
#include "condformatdlg.hxx"
#include "vcl/msgbox.hxx"
#include "document.hxx"

#define ITEMID_RANGE 1
#define ITEMID_CONDITION 2


ScCondFormatManagerWindow::ScCondFormatManagerWindow(Window* pParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList):
    SvTabListBox(pParent, WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP),
    maHeaderBar( pParent, WB_BUTTONSTYLE | WB_BOTTOMBORDER ),
    mpDoc(pDoc),
    mpFormatList(pFormatList)
{
    Size aBoxSize( pParent->GetOutputSizePixel() );

    maHeaderBar.SetPosSizePixel( Point(0, 0), Size( aBoxSize.Width(), 16 ) );

    OUString aConditionStr(ScGlobal::GetRscString(STR_HEADER_COND));
    OUString aRangeStr(ScGlobal::GetRscString(STR_HEADER_RANGE));

    long nTabSize = aBoxSize.Width()/2;
    maHeaderBar.InsertItem( ITEMID_RANGE, aRangeStr, nTabSize, HIB_LEFT| HIB_VCENTER );
    maHeaderBar.InsertItem( ITEMID_CONDITION, aConditionStr, nTabSize, HIB_LEFT| HIB_VCENTER );

    static long nTabs[] = {2, 0, nTabSize };
    Size aHeadSize( maHeaderBar.GetSizePixel() );

    //pParent->SetFocusControl( this );
    SetPosSizePixel( Point( 0, aHeadSize.Height() ), Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    SetTabs( &nTabs[0], MAP_PIXEL );

    maHeaderBar.SetEndDragHdl( LINK(this, ScCondFormatManagerWindow, HeaderEndDragHdl ) );
    HeaderEndDragHdl(NULL);

    Init();
    Show();
    maHeaderBar.Show();
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

IMPL_LINK_NOARG(ScCondFormatManagerWindow, HeaderEndDragHdl)
{
    long aTableSize = maHeaderBar.GetSizePixel().Width();
    long aItemRangeSize = maHeaderBar.GetItemSize(ITEMID_RANGE);

    //calculate column size based on user input and minimum size
    long aItemCondSize = aTableSize - aItemRangeSize;

    Size aSz;
    aSz.Width() = aItemRangeSize;
    SetTab( ITEMID_RANGE, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width(), MAP_APPFONT );
    maHeaderBar.SetItemSize(ITEMID_RANGE, aItemRangeSize);
    aSz.Width() += aItemCondSize;
    SetTab( ITEMID_CONDITION, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width(), MAP_APPFONT );
    maHeaderBar.SetItemSize(ITEMID_CONDITION, aItemCondSize);

    return 0;
}

ScCondFormatManagerCtrl::ScCondFormatManagerCtrl(Window* pParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList):
    Control(pParent, ScResId(CTRL_TABLE)),
    maWdManager(this, pDoc, pFormatList)
{
}

ScConditionalFormat* ScCondFormatManagerCtrl::GetSelection()
{
    return maWdManager.GetSelection();
}

void ScCondFormatManagerCtrl::DeleteSelection()
{
    maWdManager.DeleteSelection();
}

void ScCondFormatManagerCtrl::Update()
{
    maWdManager.Update();
}

ScCondFormatManagerDlg::ScCondFormatManagerDlg(Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList, const ScAddress& rPos):
    ModalDialog(pParent, ScResId(RID_SCDLG_COND_FORMAT_MANAGER)),
    maBtnAdd(this, ScResId(BTN_ADD)),
    maBtnRemove(this, ScResId(BTN_REMOVE)),
    maBtnEdit(this, ScResId(BTN_EDIT)),
    maBtnOk(this, ScResId(BTN_OK)),
    maBtnCancel(this, ScResId(BTN_CANCEL)),
    maFlLine(this, ScResId(FL_LINE)),
    mpFormatList( pFormatList ? new ScConditionalFormatList(*pFormatList) : NULL),
    maCtrlManager(this, pDoc, mpFormatList),
    mpDoc(pDoc),
    maPos(rPos),
    mbModified(false)
{
    FreeResource();

    maBtnRemove.SetClickHdl(LINK(this, ScCondFormatManagerDlg, RemoveBtnHdl));
    maBtnEdit.SetClickHdl(LINK(this, ScCondFormatManagerDlg, EditBtnHdl));
    maBtnAdd.SetClickHdl(LINK(this, ScCondFormatManagerDlg, AddBtnHdl));
    maCtrlManager.GetListControl().SetDoubleClickHdl(LINK(this, ScCondFormatManagerDlg, EditBtnHdl));
}

ScCondFormatManagerDlg::~ScCondFormatManagerDlg()
{
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
    maCtrlManager.DeleteSelection();
    mbModified = true;
    return 0;
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, EditBtnHdl)
{
    ScConditionalFormat* pFormat = maCtrlManager.GetSelection();

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

        maCtrlManager.Update();
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
            maCtrlManager.Update();

            mbModified = true;
        }
    }
    Show(true, 0);
    pScMod->SetRefDialog( nId, false );

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
