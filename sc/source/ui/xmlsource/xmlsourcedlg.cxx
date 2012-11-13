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
#include "tabvwsh.hxx"

#include "unotools/pathoptions.hxx"
#include "tools/urlobj.hxx"
#include "svtools/svlbitm.hxx"
#include "sfx2/objsh.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

using namespace com::sun::star;

namespace {

bool isAttribute(const SvTreeListEntry& rEntry)
{
    const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rEntry);
    if (!pUserData)
        return false;

    return pUserData->meType == ScOrcusXMLTreeParam::Attribute;
}

OUString getXPath(const SvTreeListBox& rTree, const SvTreeListEntry& rEntry)
{
    OUStringBuffer aBuf;
    for (SvTreeListEntry* p = const_cast<SvTreeListEntry*>(&rEntry); p; p = rTree.GetParent(p))
    {
        const SvLBoxItem* pItem = p->GetFirstItem(SV_ITEM_ID_LBOXSTRING);
        if (!pItem)
            continue;

        const SvLBoxString* pStr = static_cast<const SvLBoxString*>(pItem);
        aBuf.insert(0, pStr->GetText());
        aBuf.insert(0, isAttribute(*p) ? '@' : '/');
    }

    return aBuf.makeStringAndClear();
}

}

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
    maLbTree(this, ScResId(LB_SOURCE_TREE)),
    maRefEdit(this, this, ScResId(ED_MAPPED_CELL)),
    maRefBtn(this, ScResId(BTN_MAPPED_CELL), &maRefEdit, this),
    maBtnOk(this, ScResId(BTN_OK)),
    maBtnCancel(this, ScResId(BTN_CANCEL)),
    maImgFileOpen(ScResId(IMG_FILE_OPEN)),
    mpDoc(pDoc),
    mpActiveEdit(&maRefEdit),
    mbDlgLostFocus(false)
{
    maXMLParam.maImgElementDefault = Image(ScResId(IMG_ELEMENT_DEFAULT));
    maXMLParam.maImgElementRepeat = Image(ScResId(IMG_ELEMENT_REPEAT));
    maXMLParam.maImgAttribute = Image(ScResId(IMG_ELEMENT_ATTRIBUTE));

    maBtnSelectSource.SetModeImage(maImgFileOpen);
    FreeResource();

    Link aBtnHdl = LINK(this, ScXMLSourceDlg, BtnPressedHdl);
    maBtnSelectSource.SetClickHdl(aBtnHdl);
    maBtnOk.SetClickHdl(aBtnHdl);
    maBtnCancel.SetClickHdl(aBtnHdl);

    Link aLink = LINK(this, ScXMLSourceDlg, GetFocusHdl);
    maRefEdit.SetGetFocusHdl(aLink);
    maRefBtn.SetGetFocusHdl(aLink);
    aLink = LINK(this, ScXMLSourceDlg, LoseFocusHdl);
    maRefEdit.SetLoseFocusHdl(aLink);
    maRefBtn.SetLoseFocusHdl(aLink);

    aLink = LINK(this, ScXMLSourceDlg, TreeItemSelectHdl);
    maLbTree.SetSelectHdl(aLink);

    aLink = LINK(this, ScXMLSourceDlg, RefModifiedHdl);
    maRefEdit.SetModifyHdl(aLink);

    maBtnOk.Disable();

    SetNonLinkable();
    maBtnSelectSource.GrabFocus(); // Initial focus is on the select source button.
}

ScXMLSourceDlg::~ScXMLSourceDlg()
{
}

sal_Bool ScXMLSourceDlg::IsRefInputMode() const
{
    return mpActiveEdit != NULL && mpActiveEdit->IsEnabled();
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

    RefEditModified();
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

    pOrcus->loadXMLStructure(rPath, maLbTree, maXMLParam);
}

void ScXMLSourceDlg::HandleGetFocus(Control* pCtrl)
{
    mpActiveEdit = NULL;
    if (pCtrl == &maRefEdit || pCtrl == &maRefBtn)
        mpActiveEdit = &maRefEdit;

    if (mpActiveEdit)
        mpActiveEdit->SetSelection(Selection(0, SELECTION_MAX));
}

void ScXMLSourceDlg::HandleLoseFocus(Control* /*pCtrl*/)
{
}

void ScXMLSourceDlg::TreeItemSelected()
{
    SvTreeListEntry* pEntry = maLbTree.GetCurEntry();
    if (!pEntry)
        return;

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*pEntry);
    OSL_ASSERT(pUserData);

    const ScAddress& rPos = pUserData->maLinkedPos;
    if (rPos.IsValid())
    {
        OUString aStr;
        rPos.Format(aStr, SCA_ABS_3D, mpDoc, mpDoc->GetAddressConvention());
        maRefEdit.SetRefString(aStr);
    }
    else
        maRefEdit.SetRefString(OUString());

    switch (pUserData->meType)
    {
        case ScOrcusXMLTreeParam::Attribute:
            AttributeSelected(*pEntry);
        break;
        case ScOrcusXMLTreeParam::ElementDefault:
            DefaultElementSelected(*pEntry);
        break;
        case ScOrcusXMLTreeParam::ElementRepeat:
            RepeatElementSelected(*pEntry);
        break;
        default:
            ;
    }
}

void ScXMLSourceDlg::DefaultElementSelected(SvTreeListEntry& rEntry)
{
    ScOrcusXMLTreeParam::EntryData* pUserData = NULL;

    if (maLbTree.GetChildCount(&rEntry) > 0)
    {
        // Only an element with no child elements (leaf element) can be linked.
        bool bHasChild = false;
        for (SvTreeListEntry* pChild = maLbTree.FirstChild(&rEntry); pChild; pChild = maLbTree.NextSibling(pChild))
        {
            pUserData = ScOrcusXMLTreeParam::getUserData(*pChild);
            OSL_ASSERT(pUserData);
            if (pUserData->meType != ScOrcusXMLTreeParam::Attribute)
            {
                // This child is not an attribute. Bail out.
                bHasChild = true;
                break;
            }
        }

        if (bHasChild)
        {
            SetNonLinkable();
            return;
        }
    }

    // Check all its parents and make sure non of them are range-linked nor
    // repeat elements.
    if (IsParentDirty(&rEntry))
    {
        SetNonLinkable();
        return;
    }

    SetSingleLinkable();
}

void ScXMLSourceDlg::RepeatElementSelected(SvTreeListEntry& rEntry)
{
    // Check all its parents first.

    if (IsParentDirty(&rEntry))
    {
        SetNonLinkable();
        return;
    }

    // Check all its child elements / attributes and make sure non of them are
    // linked or repeat elements.  In the future we will support range linking
    // of repeat element who has another repeat elements. But first I need to
    // support that scenario in orcus.

    if (IsChildrenDirty(&rEntry))
    {
        SetNonLinkable();
        return;
    }

    SetRangeLinkable();
}

void ScXMLSourceDlg::AttributeSelected(SvTreeListEntry& rEntry)
{
    // Check all its parent elements and make sure non of them are linked nor
    // repeat elements.  In attribute's case, it's okay to have the immediate
    // parent element linked (but not range-linked).

    SvTreeListEntry* pParent = maLbTree.GetParent(&rEntry);
    OSL_ASSERT(pParent); // attribute should have a parent element.

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*pParent);
    OSL_ASSERT(pUserData);
    if (pUserData->maLinkedPos.IsValid() && pUserData->mbRangeParent)
    {
        // Parent element is range-linked.  Bail out.
        SetNonLinkable();
        return;
    }

    if (IsParentDirty(&rEntry))
    {
        SetNonLinkable();
        return;
    }

    SetSingleLinkable();
}

void ScXMLSourceDlg::SetNonLinkable()
{
    maFtMappedCellTitle.Disable();
    maRefEdit.Disable();
    maRefBtn.Disable();
}

void ScXMLSourceDlg::SetSingleLinkable()
{
    maFtMappedCellTitle.Enable();
    maRefEdit.Enable();
    maRefBtn.Enable();
}

void ScXMLSourceDlg::SetRangeLinkable()
{
    maFtMappedCellTitle.Enable();
    maRefEdit.Enable();
    maRefBtn.Enable();
}

bool ScXMLSourceDlg::IsParentDirty(SvTreeListEntry* pEntry) const
{
    ScOrcusXMLTreeParam::EntryData* pUserData = NULL;
    SvTreeListEntry* pParent = maLbTree.GetParent(pEntry);
    while (pParent)
    {
        pUserData = ScOrcusXMLTreeParam::getUserData(*pParent);
        OSL_ASSERT(pUserData);
        if (pUserData->maLinkedPos.IsValid())
        {
            // This parent is already linked.
            return true;
        }
        if (pUserData->meType == ScOrcusXMLTreeParam::ElementRepeat)
        {
            // This is a repeat element.
            return true;
        }
        pParent = maLbTree.GetParent(pParent);
    }
    return false;
}

bool ScXMLSourceDlg::IsChildrenDirty(SvTreeListEntry* pEntry) const
{
    ScOrcusXMLTreeParam::EntryData* pUserData = NULL;
    for (SvTreeListEntry* pChild = maLbTree.FirstChild(pEntry); pChild; pChild = maLbTree.NextSibling(pChild))
    {
        pUserData = ScOrcusXMLTreeParam::getUserData(*pChild);
        OSL_ASSERT(pUserData);
        if (pUserData->maLinkedPos.IsValid())
            // Already linked.
            return true;

        if (pUserData->meType == ScOrcusXMLTreeParam::ElementRepeat)
            // We don't support linking of nested repeat elements (yet).
            return true;

        if (pUserData->meType == ScOrcusXMLTreeParam::ElementDefault)
        {
            // Check recursively.
            if (IsChildrenDirty(pChild))
                return true;
        }
    }

    return false;
}

namespace {

/**
 * Pick only the leaf elements.
 */
void getFieldLinks(ScOrcusImportXMLParam::RangeLink& rRangeLink, const SvTreeListBox& rTree, const SvTreeListEntry& rEntry)
{
    const SvTreeListEntries& rChildren = rEntry.GetChildEntries();
    if (rChildren.empty())
        // No more children.  We're done.
        return;

    SvTreeListEntries::const_iterator it = rChildren.begin(), itEnd = rChildren.end();
    for (; it != itEnd; ++it)
    {
        const SvTreeListEntry& rChild = *it;
        OUString aPath = getXPath(rTree, rChild);
        const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rChild);

        if (pUserData && pUserData->mbLeafNode)
        {
            if (!aPath.isEmpty())
                // XPath should never be empty anyway, but it won't hurt to check...
                rRangeLink.maFieldPaths.push_back(rtl::OUStringToOString(aPath, RTL_TEXTENCODING_UTF8));
        }

        // Walk recursively.
        getFieldLinks(rRangeLink, rTree, rChild);
    }
}

}

void ScXMLSourceDlg::OkPressed()
{
    // Begin import.

    ScOrcusImportXMLParam aParam;

    // Convert single cell links.
    {
        std::set<const SvTreeListEntry*>::const_iterator it = maCellLinks.begin(), itEnd = maCellLinks.end();
        for (; it != itEnd; ++it)
        {
            const SvTreeListEntry& rEntry = **it;
            OUString aPath = getXPath(maLbTree, rEntry);
            const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rEntry);

            aParam.maCellLinks.push_back(
                ScOrcusImportXMLParam::CellLink(
                    pUserData->maLinkedPos, rtl::OUStringToOString(aPath, RTL_TEXTENCODING_UTF8)));
        }
    }

    // Convert range links. For now, an element with range link takes all its
    // child elements as its fields.
    {
        std::set<const SvTreeListEntry*>::const_iterator it = maRangeLinks.begin(), itEnd = maRangeLinks.end();
        for (; it != itEnd; ++it)
        {
            const SvTreeListEntry& rEntry = **it;
            const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rEntry);

            ScOrcusImportXMLParam::RangeLink aRangeLink;
            aRangeLink.maPos = pUserData->maLinkedPos;

            // Go through all its child elements.
            getFieldLinks(aRangeLink, maLbTree, rEntry);

            aParam.maRangeLinks.push_back(aRangeLink);
        }
    }

    // Now do the import.
    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    if (!pOrcus)
        return;

    pOrcus->importXML(*mpDoc, maSrcPath, aParam);

    // Don't forget to broadcast the change.
    SfxObjectShell* pShell = mpDoc->GetDocumentShell();
    pShell->Broadcast(SfxSimpleHint(FID_DATACHANGED));

    // Repaint the grid to force repaint the cell values.
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->PaintGrid();

    Close();
}

void ScXMLSourceDlg::CancelPressed()
{
    Close();
}

void ScXMLSourceDlg::RefEditModified()
{
    OUString aRefStr = maRefEdit.GetText();

    // Check if the address is valid.
    ScAddress aLinkedPos;
    sal_uInt16 nRes = aLinkedPos.Parse(aRefStr, mpDoc, mpDoc->GetAddressConvention());
    bool bValid = (nRes & SCA_VALID) == SCA_VALID;

    // TODO: For some unknown reason, setting the ref invalid will hide the text altogether.
    // Find out how to make this work.
//  maRefEdit.SetRefValid(bValid);

    if (!bValid)
        aLinkedPos.SetInvalid();

    // Set this address to currently selected tree item.
    SvTreeListEntry* pEntry = maLbTree.GetCurEntry();
    if (!pEntry)
        // This should never happen.
        return;

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*pEntry);
    if (!pUserData)
        // This should never happen either.
        return;

    bool bRepeatElem = pUserData->meType == ScOrcusXMLTreeParam::ElementRepeat;
    pUserData->maLinkedPos = aLinkedPos;
    pUserData->mbRangeParent = aLinkedPos.IsValid() && bRepeatElem;

    if (bRepeatElem)
    {
        if (bValid)
            maRangeLinks.insert(pEntry);
        else
            maRangeLinks.erase(pEntry);
    }
    else
    {
        if (bValid)
            maCellLinks.insert(pEntry);
        else
            maCellLinks.erase(pEntry);
    }

    // Enable the import button only when at least one link exists.
    bool bHasLink = !maCellLinks.empty() || !maRangeLinks.empty();
    maBtnOk.Enable(bHasLink);
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
    else if (pBtn == &maBtnOk)
        OkPressed();
    else if (pBtn == &maBtnCancel)
        CancelPressed();
    return 0;
}

IMPL_LINK_NOARG(ScXMLSourceDlg, TreeItemSelectHdl)
{
    TreeItemSelected();
    return 0;
}

IMPL_LINK_NOARG(ScXMLSourceDlg, RefModifiedHdl)
{
    RefEditModified();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
