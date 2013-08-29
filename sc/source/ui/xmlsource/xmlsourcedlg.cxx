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
#include "svtools/treelistentry.hxx"
#include "svtools/viewdataentry.hxx"
#include "sfx2/objsh.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

using namespace com::sun::star;

namespace {

bool isAttribute(const SvTreeListEntry& rEntry)
{
    const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rEntry);
    if (!pUserData)
        return false;

    return pUserData->meType == ScOrcusXMLTreeParam::Attribute;
}

OUString getXPath(
    const SvTreeListBox& rTree, const SvTreeListEntry& rEntry, std::vector<size_t>& rNamespaces)
{
    OUStringBuffer aBuf;
    for (const SvTreeListEntry* p = &rEntry; p; p = rTree.GetParent(p))
    {
        const SvLBoxItem* pItem = p->GetFirstItem(SV_ITEM_ID_LBOXSTRING);
        if (!pItem)
            continue;

        // Collect used namespace.
        const ScOrcusXMLTreeParam::EntryData* pData = ScOrcusXMLTreeParam::getUserData(*p);
        if (pData)
            rNamespaces.push_back(pData->mnNamespaceID);

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
    maRefEdit(this, this, NULL, ScResId(ED_MAPPED_CELL)),
    maRefBtn(this, ScResId(BTN_MAPPED_CELL), &maRefEdit, this),
    maBtnOk(this, ScResId(BTN_OK)),
    maBtnCancel(this, ScResId(BTN_CANCEL)),
    maImgFileOpen(ScResId(IMG_FILE_OPEN)),
    mpCurRefEntry(NULL),
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

    OUString aStr(rRange.aStart.Format(SCA_ABS_3D, pDoc, pDoc->GetAddressConvention()));
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
    uno::Reference<ui::dialogs::XFilePicker3> xFilePicker = ui::dialogs::FilePicker::createWithMode( comphelper::getProcessComponentContext(), ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );

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
    maHighlightedEntries.clear();
    LoadSourceFileStructure(maSrcPath);
}

void ScXMLSourceDlg::LoadSourceFileStructure(const OUString& rPath)
{
    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    if (!pOrcus)
        return;

    mpXMLContext.reset(pOrcus->createXMLContext(*mpDoc, rPath));
    if (!mpXMLContext)
        return;

    mpXMLContext->loadXMLStructure(maLbTree, maXMLParam);
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

namespace {

class UnhighlightEntry : std::unary_function<SvTreeListEntry*, void>
{
    SvTreeListBox& mrTree;
public:
    UnhighlightEntry(SvTreeListBox& rTree) : mrTree(rTree) {}

    void operator() (SvTreeListEntry* p)
    {
        SvViewDataEntry* pView = mrTree.GetViewDataEntry(p);
        if (!pView)
            return;

        pView->SetHighlighted(false);
        mrTree.PaintEntry(p);
    }
};

/**
 * When the current entry is a direct or indirect child of a mappable
 * repeat element entry, that entry becomes the reference entry.
 * Otherwise the reference entry equals the current entry.  A reference
 * entry is the entry that stores mapped cell position.
 */
SvTreeListEntry* getReferenceEntry(SvTreeListBox& rTree, SvTreeListEntry* pCurEntry)
{
    SvTreeListEntry* pParent = rTree.GetParent(pCurEntry);
    SvTreeListEntry* pRefEntry = NULL;
    while (pParent)
    {
        ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*pParent);
        OSL_ASSERT(pUserData);
        if (pUserData->meType == ScOrcusXMLTreeParam::ElementRepeat)
        {
            // This is a repeat element.
            if (pRefEntry)
            {
                // Second repeat element encountered. Not good.
                return pCurEntry;
            }

            pRefEntry = pParent;
        }
        pParent = rTree.GetParent(pParent);
    }

    return pRefEntry ? pRefEntry : pCurEntry;
}

}

void ScXMLSourceDlg::TreeItemSelected()
{
    SvTreeListEntry* pEntry = maLbTree.GetCurEntry();
    if (!pEntry)
        return;

    if (!maHighlightedEntries.empty())
    {
        // Remove highlights from all previously highlighted entries (if any).
        std::for_each(maHighlightedEntries.begin(), maHighlightedEntries.end(), UnhighlightEntry(maLbTree));
        maHighlightedEntries.clear();
    }

    mpCurRefEntry = getReferenceEntry(maLbTree, pEntry);

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mpCurRefEntry);
    OSL_ASSERT(pUserData);

    const ScAddress& rPos = pUserData->maLinkedPos;
    if (rPos.IsValid())
    {
        OUString aStr(rPos.Format(SCA_ABS_3D, mpDoc, mpDoc->GetAddressConvention()));
        maRefEdit.SetRefString(aStr);
    }
    else
        maRefEdit.SetRefString(OUString());

    switch (pUserData->meType)
    {
        case ScOrcusXMLTreeParam::Attribute:
            AttributeSelected(*mpCurRefEntry);
        break;
        case ScOrcusXMLTreeParam::ElementDefault:
            DefaultElementSelected(*mpCurRefEntry);
        break;
        case ScOrcusXMLTreeParam::ElementRepeat:
            RepeatElementSelected(*mpCurRefEntry);
        break;
        default:
            ;
    }
}

void ScXMLSourceDlg::DefaultElementSelected(SvTreeListEntry& rEntry)
{

    if (maLbTree.GetChildCount(&rEntry) > 0)
    {
        // Only an element with no child elements (leaf element) can be linked.
        bool bHasChild = false;
        ScOrcusXMLTreeParam::EntryData* pUserData = NULL;
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

    SvViewDataEntry* p = maLbTree.GetViewDataEntry(&rEntry);
    if (!p->IsHighlighted())
    {
        // Highlight the entry if not highlighted already.  This can happen
        // when the current entry is a child entry of a repeat element entry.
        p->SetHighlighted(true);
        maLbTree.PaintEntry(&rEntry);
        maHighlightedEntries.push_back(&rEntry);
    }

    SelectAllChildEntries(rEntry);
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

void ScXMLSourceDlg::SelectAllChildEntries(SvTreeListEntry& rEntry)
{
    SvTreeListEntries& rChildren = rEntry.GetChildEntries();
    SvTreeListEntries::iterator it = rChildren.begin(), itEnd = rChildren.end();
    for (; it != itEnd; ++it)
    {
        SvTreeListEntry& r = *it;
        SelectAllChildEntries(r); // select recursively.
        SvViewDataEntry* p = maLbTree.GetViewDataEntry(&r);
        p->SetHighlighted(true);
        maLbTree.PaintEntry(&r);
        maHighlightedEntries.push_back(&r);
    }
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
void getFieldLinks(
    ScOrcusImportXMLParam::RangeLink& rRangeLink, std::vector<size_t>& rNamespaces,
    const SvTreeListBox& rTree, const SvTreeListEntry& rEntry)
{
    const SvTreeListEntries& rChildren = rEntry.GetChildEntries();
    if (rChildren.empty())
        // No more children.  We're done.
        return;

    SvTreeListEntries::const_iterator it = rChildren.begin(), itEnd = rChildren.end();
    for (; it != itEnd; ++it)
    {
        const SvTreeListEntry& rChild = *it;
        OUString aPath = getXPath(rTree, rChild, rNamespaces);
        const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rChild);

        if (pUserData && pUserData->mbLeafNode)
        {
            if (!aPath.isEmpty())
                // XPath should never be empty anyway, but it won't hurt to check...
                rRangeLink.maFieldPaths.push_back(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8));
        }

        // Walk recursively.
        getFieldLinks(rRangeLink, rNamespaces, rTree, rChild);
    }
}

void removeDuplicates(std::vector<size_t>& rArray)
{
    std::sort(rArray.begin(), rArray.end());
    std::vector<size_t>::iterator it = std::unique(rArray.begin(), rArray.end());
    rArray.erase(it, rArray.end());
}

}

void ScXMLSourceDlg::OkPressed()
{
    if (!mpXMLContext)
        return;

    // Begin import.

    ScOrcusImportXMLParam aParam;

    // Convert single cell links.
    {
        std::set<const SvTreeListEntry*>::const_iterator it = maCellLinks.begin(), itEnd = maCellLinks.end();
        for (; it != itEnd; ++it)
        {
            const SvTreeListEntry& rEntry = **it;
            OUString aPath = getXPath(maLbTree, rEntry, aParam.maNamespaces);
            const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rEntry);

            aParam.maCellLinks.push_back(
                ScOrcusImportXMLParam::CellLink(
                    pUserData->maLinkedPos, OUStringToOString(aPath, RTL_TEXTENCODING_UTF8)));
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
            getFieldLinks(aRangeLink, aParam.maNamespaces, maLbTree, rEntry);

            aParam.maRangeLinks.push_back(aRangeLink);
        }
    }

    // Remove duplicate namespace IDs.
    removeDuplicates(aParam.maNamespaces);

    // Now do the import.
    mpXMLContext->importXML(aParam);

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

    // Set this address to the current reference entry.
    if (!mpCurRefEntry)
        // This should never happen.
        return;

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mpCurRefEntry);
    if (!pUserData)
        // This should never happen either.
        return;

    bool bRepeatElem = pUserData->meType == ScOrcusXMLTreeParam::ElementRepeat;
    pUserData->maLinkedPos = aLinkedPos;
    pUserData->mbRangeParent = aLinkedPos.IsValid() && bRepeatElem;

    if (bRepeatElem)
    {
        if (bValid)
            maRangeLinks.insert(mpCurRefEntry);
        else
            maRangeLinks.erase(mpCurRefEntry);
    }
    else
    {
        if (bValid)
            maCellLinks.insert(mpCurRefEntry);
        else
            maCellLinks.erase(mpCurRefEntry);
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
