/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xmlsourcedlg.hxx>
#include <bitmaps.hlst>
#include <document.hxx>
#include <orcusfilters.hxx>
#include <filter.hxx>
#include <reffact.hxx>
#include <tabvwsh.hxx>

#include <unotools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

using namespace com::sun::star;

namespace {

bool isAttribute(const weld::TreeView& rControl, const weld::TreeIter& rEntry)
{
    const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rControl, rEntry);
    if (!pUserData)
        return false;

    return pUserData->meType == ScOrcusXMLTreeParam::Attribute;
}

OUString getXPath(
    const weld::TreeView& rTree, const weld::TreeIter& rEntry, std::vector<size_t>& rNamespaces)
{
    OUStringBuffer aBuf;
    std::unique_ptr<weld::TreeIter> xEntry(rTree.make_iterator(&rEntry));
    do
    {
        // Collect used namespace.
        const ScOrcusXMLTreeParam::EntryData* pData = ScOrcusXMLTreeParam::getUserData(rTree, *xEntry);
        if (pData)
            rNamespaces.push_back(pData->mnNamespaceID);

        // element separator is '/' whereas attribute separator is '/@' in xpath.
        aBuf.insert(0, rTree.get_text(*xEntry, 0));
        if (isAttribute(rTree, *xEntry))
            aBuf.insert(0, "/@");
        else
            aBuf.insert(0, '/');
    }
    while (rTree.iter_parent(*xEntry));

    return aBuf.makeStringAndClear();
}

}

ScXMLSourceDlg::ScXMLSourceDlg(
    SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent, ScDocument* pDoc)
    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/xmlsourcedialog.ui", "XMLSourceDialog")
    , mpDoc(pDoc)
    , mbDlgLostFocus(false)
    , mxBtnSelectSource(m_xBuilder->weld_button("selectsource"))
    , mxFtSourceFile(m_xBuilder->weld_label("sourcefile"))
    , mxMapGrid(m_xBuilder->weld_container("mapgrid"))
    , mxLbTree(m_xBuilder->weld_tree_view("tree"))
    , mxRefEdit(new formula::RefEdit(m_xBuilder->weld_entry("edit")))
    , mxRefBtn(new formula::RefButton(m_xBuilder->weld_button("ref")))
    , mxBtnOk(m_xBuilder->weld_button("ok"))
    , mxBtnCancel(m_xBuilder->weld_button("cancel"))
    , maCustomCompare(*mxLbTree)
    , maCellLinks(maCustomCompare)
    , maRangeLinks(maCustomCompare)
{
    mxLbTree->set_size_request(mxLbTree->get_approximate_digit_width() * 40,
                               mxLbTree->get_height_rows(15));
    mxLbTree->set_selection_mode(SelectionMode::Multiple);
    mxRefEdit->SetReferences(this, nullptr);
    mxRefBtn->SetReferences(this, mxRefEdit.get());

    mpActiveEdit = mxRefEdit.get();

    maXMLParam.maImgElementDefault = RID_BMP_ELEMENT_DEFAULT;
    maXMLParam.maImgElementRepeat = RID_BMP_ELEMENT_REPEAT;
    maXMLParam.maImgAttribute = RID_BMP_ELEMENT_ATTRIBUTE;

    Link<weld::Button&,void> aBtnHdl = LINK(this, ScXMLSourceDlg, BtnPressedHdl);
    mxBtnSelectSource->connect_clicked(aBtnHdl);
    mxBtnOk->connect_clicked(aBtnHdl);
    mxBtnCancel->connect_clicked(aBtnHdl);

    mxLbTree->connect_changed(LINK(this, ScXMLSourceDlg, TreeItemSelectHdl));

    Link<formula::RefEdit&,void> aLink = LINK(this, ScXMLSourceDlg, RefModifiedHdl);
    mxRefEdit->SetModifyHdl(aLink);

    mxBtnOk->set_sensitive(false);

    SetNonLinkable();
    mxBtnSelectSource->grab_focus(); // Initial focus is on the select source button.
}

ScXMLSourceDlg::~ScXMLSourceDlg()
{
}

bool ScXMLSourceDlg::IsRefInputMode() const
{
    return mpActiveEdit != nullptr && mpActiveEdit->GetWidget()->get_sensitive();
}

void ScXMLSourceDlg::SetReference(const ScRange& rRange, ScDocument& rDoc)
{
    if (!mpActiveEdit)
        return;

    if (rRange.aStart != rRange.aEnd)
        RefInputStart(mpActiveEdit);

    OUString aStr(rRange.aStart.Format(ScRefFlags::ADDR_ABS_3D, &rDoc, rDoc.GetAddressConvention()));
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
        m_xDialog->grab_focus();
    }

    RefInputDone();
}

void ScXMLSourceDlg::Close()
{
    DoClose(ScXMLSourceDlgWrapper::GetChildWindowId());
}

void ScXMLSourceDlg::SelectSourceFile()
{
    sfx2::FileDialogHelper aDlgHelper(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                      FileDialogFlags::NONE, m_xDialog.get());

    uno::Reference<ui::dialogs::XFilePicker3> xFilePicker = aDlgHelper.GetFilePicker();

    if (maSrcPath.isEmpty())
        // Use default path.
        xFilePicker->setDisplayDirectory(SvtPathOptions().GetWorkPath());
    else
    {
        // Use the directory of current source file.
        INetURLObject aURL(maSrcPath);
        aURL.removeSegment();
        aURL.removeFinalSlash();
        OUString aPath = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        xFilePicker->setDisplayDirectory(aPath);
    }

    if (xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK)
        // File picker dialog cancelled.
        return;

    uno::Sequence<OUString> aFiles = xFilePicker->getSelectedFiles();
    if (!aFiles.hasElements())
        return;

    // There should only be one file returned from the file picker.
    maSrcPath = aFiles[0];
    mxFtSourceFile->set_label(maSrcPath);
    LoadSourceFileStructure(maSrcPath);
}

void ScXMLSourceDlg::LoadSourceFileStructure(const OUString& rPath)
{
    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    if (!pOrcus)
        return;

    mpXMLContext = pOrcus->createXMLContext(*mpDoc, rPath);
    if (!mpXMLContext)
        return;

    mpXMLContext->loadXMLStructure(*mxLbTree, maXMLParam);
}

namespace {

/**
 * The current entry is the reference entry for a cell link.  For a range
 * link, the reference entry is the shallowest repeat element entry up from
 * the current entry position.  The mapped cell position for a range link is
 * stored with the reference entry.
 */
std::unique_ptr<weld::TreeIter> getReferenceEntry(const weld::TreeView& rTree, weld::TreeIter& rCurEntry)
{
    std::unique_ptr<weld::TreeIter> xParent(rTree.make_iterator(&rCurEntry));
    bool bParent = rTree.iter_parent(*xParent);
    std::unique_ptr<weld::TreeIter> xRefEntry;
    while (bParent)
    {
        ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rTree, *xParent);
        OSL_ASSERT(pUserData);
        if (pUserData->meType == ScOrcusXMLTreeParam::ElementRepeat)
        {
            // This is a repeat element - a potential reference entry.
            xRefEntry = rTree.make_iterator(xParent.get());
        }
        bParent = rTree.iter_parent(*xParent);
    }

    if (xRefEntry)
        return xRefEntry;

    std::unique_ptr<weld::TreeIter> xCurEntry(rTree.make_iterator(&rCurEntry));
    return xCurEntry;
}

}

void ScXMLSourceDlg::TreeItemSelected()
{
    std::unique_ptr<weld::TreeIter> xEntry(mxLbTree->make_iterator());
    if (!mxLbTree->get_cursor(xEntry.get()))
        return;

    mxLbTree->unselect_all();
    mxLbTree->select(*xEntry);

    mxCurRefEntry = getReferenceEntry(*mxLbTree, *xEntry);

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mxLbTree, *mxCurRefEntry);
    OSL_ASSERT(pUserData);

    const ScAddress& rPos = pUserData->maLinkedPos;
    if (rPos.IsValid())
    {
        OUString aStr(rPos.Format(ScRefFlags::ADDR_ABS_3D, mpDoc, mpDoc->GetAddressConvention()));
        mxRefEdit->SetRefString(aStr);
    }
    else
        mxRefEdit->SetRefString(OUString());

    switch (pUserData->meType)
    {
        case ScOrcusXMLTreeParam::Attribute:
            AttributeSelected(*mxCurRefEntry);
        break;
        case ScOrcusXMLTreeParam::ElementDefault:
            DefaultElementSelected(*mxCurRefEntry);
        break;
        case ScOrcusXMLTreeParam::ElementRepeat:
            RepeatElementSelected(*mxCurRefEntry);
        break;
        default:
            ;
    }
}

void ScXMLSourceDlg::DefaultElementSelected(weld::TreeIter& rEntry)
{
    if (mxLbTree->iter_has_child(rEntry))
    {
        // Only an element with no child elements (leaf element) can be linked.
        bool bHasChild = false;
        std::unique_ptr<weld::TreeIter> xChild(mxLbTree->make_iterator(&rEntry));
        (void)mxLbTree->iter_children(*xChild);
        do
        {
            ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mxLbTree, *xChild);
            OSL_ASSERT(pUserData);
            if (pUserData->meType != ScOrcusXMLTreeParam::Attribute)
            {
                // This child is not an attribute. Bail out.
                bHasChild = true;
                break;
            }
        }
        while (mxLbTree->iter_next_sibling(*xChild));

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

void ScXMLSourceDlg::RepeatElementSelected(weld::TreeIter& rEntry)
{
    // Check all its parents first.

    if (IsParentDirty(&rEntry))
    {
        SetNonLinkable();
        return;
    }

    // Check all its child elements / attributes and make sure non of them are
    // linked.

    if (IsChildrenDirty(&rEntry))
    {
        SetNonLinkable();
        return;
    }

    if (!mxLbTree->is_selected(rEntry))
    {
        // Highlight the entry if not highlighted already.  This can happen
        // when the current entry is a child entry of a repeat element entry.
        mxLbTree->select(rEntry);
    }

    SelectAllChildEntries(rEntry);
    SetRangeLinkable();
}

void ScXMLSourceDlg::AttributeSelected(weld::TreeIter& rEntry)
{
    // Check all its parent elements and make sure non of them are linked nor
    // repeat elements.  In attribute's case, it's okay to have the immediate
    // parent element linked (but not range-linked).
    std::unique_ptr<weld::TreeIter> xParent(mxLbTree->make_iterator(&rEntry));
    mxLbTree->iter_parent(*xParent);

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mxLbTree, *xParent);
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
    mxMapGrid->set_sensitive(false);
}

void ScXMLSourceDlg::SetSingleLinkable()
{
    mxMapGrid->set_sensitive(true);
}

void ScXMLSourceDlg::SetRangeLinkable()
{
    mxMapGrid->set_sensitive(true);
}

void ScXMLSourceDlg::SelectAllChildEntries(weld::TreeIter& rEntry)
{
    std::unique_ptr<weld::TreeIter> xChild(mxLbTree->make_iterator(&rEntry));
    if (!mxLbTree->iter_children(*xChild))
        return;
    do
    {
        SelectAllChildEntries(*xChild); // select recursively.
        mxLbTree->select(*xChild);
    } while (mxLbTree->iter_next_sibling(*xChild));
}

bool ScXMLSourceDlg::IsParentDirty(const weld::TreeIter* pEntry) const
{
    std::unique_ptr<weld::TreeIter> xParent(mxLbTree->make_iterator(pEntry));
    if (!mxLbTree->iter_parent(*xParent))
        return false;
    do
    {
        ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mxLbTree, *xParent);
        assert(pUserData);
        if (pUserData->maLinkedPos.IsValid())
        {
            // This parent is already linked.
            return true;
        }
    }
    while (mxLbTree->iter_parent(*xParent));
    return false;
}

bool ScXMLSourceDlg::IsChildrenDirty(const weld::TreeIter* pEntry) const
{
    std::unique_ptr<weld::TreeIter> xChild(mxLbTree->make_iterator(pEntry));
    if (!mxLbTree->iter_children(*xChild))
        return false;

    do
    {
        ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mxLbTree, *xChild);
        OSL_ASSERT(pUserData);
        if (pUserData->maLinkedPos.IsValid())
            // Already linked.
            return true;

        if (pUserData->meType == ScOrcusXMLTreeParam::ElementDefault)
        {
            // Check recursively.
            if (IsChildrenDirty(xChild.get()))
                return true;
        }
    } while (mxLbTree->iter_next_sibling(*xChild));

    return false;
}

namespace {

/**
 * Pick only the leaf elements.
 */
void getFieldLinks(
    ScOrcusImportXMLParam::RangeLink& rRangeLink, std::vector<size_t>& rNamespaces,
    const weld::TreeView& rTree, const weld::TreeIter& rEntry)
{
    std::unique_ptr<weld::TreeIter> xChild(rTree.make_iterator(&rEntry));
    if (!rTree.iter_children(*xChild))
        // No more children.  We're done.
        return;

    do
    {
        OUString aPath = getXPath(rTree, *xChild, rNamespaces);
        const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rTree, *xChild);

        if (pUserData)
        {
            if (pUserData->meType == ScOrcusXMLTreeParam::ElementRepeat)
                // nested repeat element automatically becomes a row-group node.
                rRangeLink.maRowGroups.push_back(
                    OUStringToOString(aPath, RTL_TEXTENCODING_UTF8));

            if (pUserData->mbLeafNode && !aPath.isEmpty())
                // XPath should never be empty anyway, but it won't hurt to check...
                rRangeLink.maFieldPaths.push_back(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8));
        }

        // Walk recursively.
        getFieldLinks(rRangeLink, rNamespaces, rTree, *xChild);
    } while (rTree.iter_next_sibling(*xChild));
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
    for (const auto& rEntry : maCellLinks)
    {
        OUString aPath = getXPath(*mxLbTree, *rEntry, aParam.maNamespaces);
        const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mxLbTree, *rEntry);

        aParam.maCellLinks.emplace_back(
                pUserData->maLinkedPos, OUStringToOString(aPath, RTL_TEXTENCODING_UTF8));
    }

    // Convert range links. For now, an element with range link takes all its
    // child elements as its fields.
    for (const auto& rEntry: maRangeLinks)
    {
        const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mxLbTree, *rEntry);

        ScOrcusImportXMLParam::RangeLink aRangeLink;
        aRangeLink.maPos = pUserData->maLinkedPos;

        // Go through all its child elements.
        getFieldLinks(aRangeLink, aParam.maNamespaces, *mxLbTree, *rEntry);

        // Add the reference entry as a row-group node, which will be used
        // as a row position increment point.
        OUString aThisEntry = getXPath(*mxLbTree, *rEntry, aParam.maNamespaces);
        aRangeLink.maRowGroups.push_back(
            OUStringToOString(aThisEntry, RTL_TEXTENCODING_UTF8));

        aParam.maRangeLinks.push_back(aRangeLink);
    }

    // Remove duplicate namespace IDs.
    removeDuplicates(aParam.maNamespaces);

    // Now do the import.
    mpXMLContext->importXML(aParam);

    // Don't forget to broadcast the change.
    SfxObjectShell* pShell = mpDoc->GetDocumentShell();
    pShell->Broadcast(SfxHint(SfxHintId::ScDataChanged));

    // Repaint the grid to force repaint the cell values.
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->PaintGrid();

    m_xDialog->response(RET_OK);
}

void ScXMLSourceDlg::CancelPressed()
{
    m_xDialog->response(RET_CANCEL);
}

void ScXMLSourceDlg::RefEditModified()
{
    OUString aRefStr = mxRefEdit->GetText();

    // Check if the address is valid.
    // Preset current sheet in case only address was entered.
    ScAddress aLinkedPos;
    aLinkedPos.SetTab( ScDocShell::GetCurTab());
    ScRefFlags nRes = aLinkedPos.Parse(aRefStr, *mpDoc, mpDoc->GetAddressConvention());
    bool bValid = ( (nRes & ScRefFlags::VALID) == ScRefFlags::VALID );

    // TODO: For some unknown reason, setting the ref invalid will hide the text altogether.
    // Find out how to make this work.
//  mxRefEdit->SetRefValid(bValid);

    if (!bValid)
        aLinkedPos.SetInvalid();

    // Set this address to the current reference entry.
    if (!mxCurRefEntry)
        // This should never happen.
        return;

    ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(*mxLbTree, *mxCurRefEntry);
    if (!pUserData)
        // This should never happen either.
        return;

    bool bRepeatElem = pUserData->meType == ScOrcusXMLTreeParam::ElementRepeat;
    pUserData->maLinkedPos = aLinkedPos;
    pUserData->mbRangeParent = aLinkedPos.IsValid() && bRepeatElem;

    if (bRepeatElem)
    {
        if (bValid)
            maRangeLinks.insert(mxLbTree->make_iterator(mxCurRefEntry.get()));
        else
            maRangeLinks.erase(mxCurRefEntry);
    }
    else
    {
        if (bValid)
            maCellLinks.insert(mxLbTree->make_iterator(mxCurRefEntry.get()));
        else
            maCellLinks.erase(mxCurRefEntry);
    }

    // Enable the import button only when at least one link exists.
    bool bHasLink = !maCellLinks.empty() || !maRangeLinks.empty();
    mxBtnOk->set_sensitive(bHasLink);
}

IMPL_LINK(ScXMLSourceDlg, BtnPressedHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxBtnSelectSource.get())
        SelectSourceFile();
    else if (&rBtn == mxBtnOk.get())
        OkPressed();
    else if (&rBtn == mxBtnCancel.get())
        CancelPressed();
}

IMPL_LINK_NOARG(ScXMLSourceDlg, TreeItemSelectHdl, weld::TreeView&, void)
{
    TreeItemSelected();
}

IMPL_LINK_NOARG(ScXMLSourceDlg, RefModifiedHdl, formula::RefEdit&, void)
{
    RefEditModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
