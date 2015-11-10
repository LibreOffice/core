/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlsourcedlg.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include "document.hxx"
#include "orcusfilters.hxx"
#include "filter.hxx"
#include "reffact.hxx"
#include "tabvwsh.hxx"

#include <unotools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <svtools/viewdataentry.hxx>
#include <sfx2/objsh.hxx>

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

ScXMLSourceDlg::ScXMLSourceDlg(
    SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent, ScDocument* pDoc)
    : ScAnyRefDlg(pB, pCW, pParent, "XMLSourceDialog",
        "modules/scalc/ui/xmlsourcedialog.ui")
    , mpCurRefEntry(nullptr)
    , mpDoc(pDoc)
    , mbDlgLostFocus(false)
{
    get(mpBtnSelectSource, "selectsource");
    get(mpFtSourceFile, "sourcefile");
    get(mpMapGrid, "mapgrid");
    get(mpLbTree, "tree");
    Size aTreeSize(mpLbTree->LogicToPixel(Size(130, 120), MAP_APPFONT));
    mpLbTree->set_width_request(aTreeSize.Width());
    mpLbTree->set_height_request(aTreeSize.Height());
    get(mpRefEdit, "edit");
    mpRefEdit->SetReferences(this, nullptr);
    get(mpRefBtn, "ref");
    mpRefBtn->SetReferences(this, mpRefEdit);
    get(mpBtnCancel, "cancel");
    get(mpBtnOk, "ok");

    mpActiveEdit = mpRefEdit;

    maXMLParam.maImgElementDefault = Image(ScResId(IMG_ELEMENT_DEFAULT));
    maXMLParam.maImgElementRepeat = Image(ScResId(IMG_ELEMENT_REPEAT));
    maXMLParam.maImgAttribute = Image(ScResId(IMG_ELEMENT_ATTRIBUTE));

    Link<Button*,void> aBtnHdl = LINK(this, ScXMLSourceDlg, BtnPressedHdl);
    mpBtnSelectSource->SetClickHdl(aBtnHdl);
    mpBtnOk->SetClickHdl(aBtnHdl);
    mpBtnCancel->SetClickHdl(aBtnHdl);

    Link<Control&,void> aLink2 = LINK(this, ScXMLSourceDlg, GetFocusHdl);
    mpRefEdit->SetGetFocusHdl(aLink2);
    mpRefBtn->SetGetFocusHdl(aLink2);

    mpLbTree->SetSelectHdl(LINK(this, ScXMLSourceDlg, TreeItemSelectHdl));

    Link<Edit&,void> aLink = LINK(this, ScXMLSourceDlg, RefModifiedHdl);
    mpRefEdit->SetModifyHdl(aLink);

    mpBtnOk->Disable();

    SetNonLinkable();
    mpBtnSelectSource->GrabFocus(); // Initial focus is on the select source button.
}

ScXMLSourceDlg::~ScXMLSourceDlg()
{
    disposeOnce();
}

void ScXMLSourceDlg::dispose()
{
    mpBtnSelectSource.clear();
    mpFtSourceFile.clear();
    mpMapGrid.clear();
    mpLbTree.clear();
    mpRefEdit.clear();
    mpRefBtn.clear();
    mpBtnOk.clear();
    mpBtnCancel.clear();
    mpActiveEdit.clear();
    ScAnyRefDlg::dispose();
}

bool ScXMLSourceDlg::IsRefInputMode() const
{
    return mpActiveEdit != nullptr && mpActiveEdit->IsEnabled();
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

bool ScXMLSourceDlg::Close()
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

    uno::Sequence<OUString> aFiles = xFilePicker->getSelectedFiles();
    if (!aFiles.getLength())
        return;

    // There should only be one file returned from the file picker.
    maSrcPath = aFiles[0];
    mpFtSourceFile->SetText(maSrcPath);
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

    mpXMLContext->loadXMLStructure(*mpLbTree, maXMLParam);
}

void ScXMLSourceDlg::HandleGetFocus(Control* pCtrl)
{
    mpActiveEdit = nullptr;
    if (pCtrl == mpRefEdit || pCtrl == mpRefBtn)
        mpActiveEdit = mpRefEdit;

    if (mpActiveEdit)
        mpActiveEdit->SetSelection(Selection(0, SELECTION_MAX));
}

namespace {

class UnhighlightEntry : std::unary_function<SvTreeListEntry*, void>
{
    SvTreeListBox& mrTree;
public:
    explicit UnhighlightEntry(SvTreeListBox& rTree) : mrTree(rTree) {}

    void operator() (SvTreeListEntry* p)
    {
        SvViewDataEntry* pView = mrTree.GetViewDataEntry(p);
        if (!pView)
            return;

        pView->SetHighlighted(false);
        mrTree.Invalidate();
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
    SvTreeListEntry* pRefEntry = nullptr;
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
    SvTreeListEntry* pEntry = mpLbTree->GetCurEntry();
    if (!pEntry)
        return;

    if (!maHighlightedEntries.empty())
    {
        // Remove highlights from all previously highlighted entries (if any).
        std::for_each(maHighlightedEntries.begin(), maHighlightedEntries.end(), UnhighlightEntry(*mpLbTree));
        maHighlightedEntries.clear();
    }

    mpCurRefEntry = getReferenceEntry(*mpLbTree, pEntry);

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mpCurRefEntry);
    OSL_ASSERT(pUserData);

    const ScAddress& rPos = pUserData->maLinkedPos;
    if (rPos.IsValid())
    {
        OUString aStr(rPos.Format(SCA_ABS_3D, mpDoc, mpDoc->GetAddressConvention()));
        mpRefEdit->SetRefString(aStr);
    }
    else
        mpRefEdit->SetRefString(OUString());

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

    if (mpLbTree->GetChildCount(&rEntry) > 0)
    {
        // Only an element with no child elements (leaf element) can be linked.
        bool bHasChild = false;
        for (SvTreeListEntry* pChild = mpLbTree->FirstChild(&rEntry); pChild; pChild = SvTreeListBox::NextSibling(pChild))
        {
            ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*pChild);
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

    SvViewDataEntry* p = mpLbTree->GetViewDataEntry(&rEntry);
    if (!p->IsHighlighted())
    {
        // Highlight the entry if not highlighted already.  This can happen
        // when the current entry is a child entry of a repeat element entry.
        p->SetHighlighted(true);
        mpLbTree->Invalidate();
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

    SvTreeListEntry* pParent = mpLbTree->GetParent(&rEntry);
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
    mpMapGrid->Disable();
}

void ScXMLSourceDlg::SetSingleLinkable()
{
    mpMapGrid->Enable();
}

void ScXMLSourceDlg::SetRangeLinkable()
{
    mpMapGrid->Enable();
}

void ScXMLSourceDlg::SelectAllChildEntries(SvTreeListEntry& rEntry)
{
    SvTreeListEntries& rChildren = rEntry.GetChildEntries();
    for (auto const& it : rChildren)
    {
        SvTreeListEntry& r = *it;
        SelectAllChildEntries(r); // select recursively.
        SvViewDataEntry* p = mpLbTree->GetViewDataEntry(&r);
        p->SetHighlighted(true);
        mpLbTree->Invalidate();
        maHighlightedEntries.push_back(&r);
    }
}

bool ScXMLSourceDlg::IsParentDirty(SvTreeListEntry* pEntry) const
{
    SvTreeListEntry* pParent = mpLbTree->GetParent(pEntry);
    while (pParent)
    {
        ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*pParent);
        assert(pUserData);
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
        pParent = mpLbTree->GetParent(pParent);
    }
    return false;
}

bool ScXMLSourceDlg::IsChildrenDirty(SvTreeListEntry* pEntry) const
{
    for (SvTreeListEntry* pChild = mpLbTree->FirstChild(pEntry); pChild; pChild = SvTreeListBox::NextSibling(pChild))
    {
        ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*pChild);
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

    for (auto const& it : rChildren)
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
            OUString aPath = getXPath(*mpLbTree, rEntry, aParam.maNamespaces);
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
            getFieldLinks(aRangeLink, aParam.maNamespaces, *mpLbTree, rEntry);

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
    OUString aRefStr = mpRefEdit->GetText();

    // Check if the address is valid.
    ScAddress aLinkedPos;
    sal_uInt16 nRes = aLinkedPos.Parse(aRefStr, mpDoc, mpDoc->GetAddressConvention());
    bool bValid = (nRes & SCA_VALID) == SCA_VALID;

    // TODO: For some unknown reason, setting the ref invalid will hide the text altogether.
    // Find out how to make this work.
//  mpRefEdit->SetRefValid(bValid);

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
    mpBtnOk->Enable(bHasLink);
}

IMPL_LINK_TYPED(ScXMLSourceDlg, GetFocusHdl, Control&, rCtrl, void)
{
    HandleGetFocus(&rCtrl);
}

IMPL_LINK_TYPED(ScXMLSourceDlg, BtnPressedHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnSelectSource)
        SelectSourceFile();
    else if (pBtn == mpBtnOk)
        OkPressed();
    else if (pBtn == mpBtnCancel)
        CancelPressed();
}

IMPL_LINK_NOARG_TYPED(ScXMLSourceDlg, TreeItemSelectHdl, SvTreeListBox*, void)
{
    TreeItemSelected();
}

IMPL_LINK_NOARG_TYPED(ScXMLSourceDlg, RefModifiedHdl, Edit&, void)
{
    RefEditModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
