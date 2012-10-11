/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlsourcedlg.hxx"
#include "xmlsourcedlg.hrc"

#include "scresid.hxx"
#include "document.hxx"
#include "orcusfilters.hxx"
#include "filter.hxx"
#include "reffact.hxx"

#include "unotools/pathoptions.hxx"
#include "tools/urlobj.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

using namespace com::sun::star;


ScXMLSourceTree::ScXMLSourceTree(Window* pParent, const ResId& rResId) :
    SvTreeListBox(pParent, rResId) {}

ScXMLSourceDlg::ScXMLSourceDlg(
    SfxBindings* pB, SfxChildWindow* pCW, Window* pParent, ScDocument* pDoc) :
    ScAnyRefDlg(pB, pCW, pParent, RID_SCDLG_XML_SOURCE),
    maFlSourceFile(this, ScResId(FL_SOURCE_FILE)),
    maBtnSelectSource(this, ScResId(BTN_SELECT_SOURCE_FILE)),
    maFtSourceFile(this, ScResId(FT_SOURCE_FILE)),
    maFtMapXmlDoc(this, ScResId(FL_MAP_XML_TO_DOCUMENT)),
    maFtMappedCellTitle(this, ScResId(FT_MAPPED_CELL_TITLE)),
    maEdit(this, this, ScResId(ED_MAPPED_CELL)),
    maBtnRb(this, ScResId(BTN_MAPPED_CELL), &maEdit, this),
    maLbTree(this, ScResId(LB_SOURCE_TREE)),
    maBtnCancel(this, ScResId(BTN_CANCEL)),
    maImgFileOpen(ScResId(IMG_FILE_OPEN)),
    maImgElemDefault(ScResId(IMG_ELEMENT_DEFAULT)),
    maImgElemRepeat(ScResId(IMG_ELEMENT_REPEAT)),
    maImgElemAttribute(ScResId(IMG_ELEMENT_ATTRIBUTE)),
    maStrCellLink(ScResId(STR_CELL_LINK).toString()),
    maStrRangeLink(ScResId(STR_RANGE_LINK).toString()),
    mpDoc(pDoc),
    mpActiveEdit(&maEdit),
    mbDlgLostFocus(false)
{
    maBtnSelectSource.SetModeImage(maImgFileOpen);
    FreeResource();

    maBtnSelectSource.SetClickHdl(LINK(this, ScXMLSourceDlg, BtnPressedHdl));

    Link aLink = LINK(this, ScXMLSourceDlg, GetFocusHdl);
    maEdit.SetGetFocusHdl(aLink);
    maBtnRb.SetGetFocusHdl(aLink);
    aLink = LINK(this, ScXMLSourceDlg, LoseFocusHdl);
    maEdit.SetLoseFocusHdl(aLink);
    maBtnRb.SetLoseFocusHdl(aLink);

    aLink = LINK(this, ScXMLSourceDlg, TreeItemSelectHdl);
    maLbTree.SetSelectHdl(aLink);
    maFtMappedCellTitle.SetText(maStrCellLink);
}

ScXMLSourceDlg::~ScXMLSourceDlg()
{
}

sal_Bool ScXMLSourceDlg::IsRefInputMode() const
{
    return mpActiveEdit != NULL;
}

void ScXMLSourceDlg::SetReference(const ScRange& rRange, ScDocument* pDoc)
{
    if (!mpActiveEdit)
        return;

    if (rRange.aStart != rRange.aEnd)
        RefInputStart(mpActiveEdit);

    OUString aStr;
    rRange.aStart.Format(aStr, SCA_ABS_3D, pDoc, pDoc->GetAddressConvention());
    mpActiveEdit->SetRefString(aStr);
}

void ScXMLSourceDlg::Deactivate()
{
    mbDlgLostFocus = true;
}

void ScXMLSourceDlg::SetActive()
{
    if (mbDlgLostFocus)
    {
        mbDlgLostFocus = false;
        if (mpActiveEdit)
        {
            mpActiveEdit->GrabFocus();
        }
    }
    else
    {
        GrabFocus();
    }

    RefInputDone();
}

sal_Bool ScXMLSourceDlg::Close()
{
    return DoClose(ScXMLSourceDlgWrapper::GetChildWindowId());
}

void ScXMLSourceDlg::SelectSourceFile()
{
    uno::Reference<lang::XMultiServiceFactory> xServiceMgr = mpDoc->GetServiceManager();
    if (!xServiceMgr.is())
        return;

    uno::Reference<ui::dialogs::XFilePicker> xFilePicker(
       xServiceMgr->createInstance("com.sun.star.ui.dialogs.FilePicker"), uno::UNO_QUERY);

    if (!xFilePicker.is())
        return;

    if (maSrcPath.isEmpty())
        // Use default path.
        xFilePicker->setDisplayDirectory(SvtPathOptions().GetWorkPath());
    else
    {
        // Use the directory of current source file.
        INetURLObject aURL(maSrcPath);
        aURL.removeSegment();
        aURL.removeFinalSlash();
        OUString aPath = aURL.GetMainURL(INetURLObject::NO_DECODE);
        xFilePicker->setDisplayDirectory(aPath);
    }

    if (xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK)
        // File picker dialog cancelled.
        return;

    uno::Sequence<OUString> aFiles = xFilePicker->getFiles();
    if (!aFiles.getLength())
        return;

    // There should only be one file returned from the file picker.
    maSrcPath = aFiles[0];
    maFtSourceFile.SetText(maSrcPath);

    LoadSourceFileStructure(maSrcPath);
}

void ScXMLSourceDlg::LoadSourceFileStructure(const OUString& rPath)
{
    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    if (!pOrcus)
        return;

    pOrcus->loadXMLStructure(
        maLbTree, rPath, maImgElemDefault, maImgElemRepeat, maImgElemAttribute);
}

void ScXMLSourceDlg::HandleGetFocus(Control* pCtrl)
{
    mpActiveEdit = NULL;
    if (pCtrl == &maEdit || pCtrl == &maBtnRb)
        mpActiveEdit = &maEdit;

    if (mpActiveEdit)
        mpActiveEdit->SetSelection(Selection(0, SELECTION_MAX));
}

void ScXMLSourceDlg::HandleLoseFocus(Control* /*pCtrl*/)
{
}

void ScXMLSourceDlg::TreeItemSelected()
{
    SvLBoxEntry* pEntry = maLbTree.GetCurEntry();
    OUString aName = maLbTree.GetEntryText(pEntry);
    fprintf(stdout, "ScXMLSourceDlg::TreeItemSelected:   name = '%s'\n",
            rtl::OUStringToOString(aName, RTL_TEXTENCODING_UTF8).getStr());
}

IMPL_LINK(ScXMLSourceDlg, GetFocusHdl, Control*, pCtrl)
{
    HandleGetFocus(pCtrl);
    return 0;
}

IMPL_LINK(ScXMLSourceDlg, LoseFocusHdl, Control*, pCtrl)
{
    HandleLoseFocus(pCtrl);
    return 0;
}

IMPL_LINK(ScXMLSourceDlg, BtnPressedHdl, Button*, pBtn)
{
    if (pBtn == &maBtnSelectSource)
        SelectSourceFile();
    return 0;
}

IMPL_LINK_NOARG(ScXMLSourceDlg, TreeItemSelectHdl)
{
    TreeItemSelected();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
