/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataproviderdlg.hxx>

#include <document.hxx>
#include <dataprovider.hxx>
#include <datatransformation.hxx>

#include <comphelper/string.hxx>

#include <orcusfilters.hxx>
#include <orcus/xml_structure_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <filter.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <svtools/treelistbox.hxx>

#include <anyrefdg.hxx>
#include <orcusxml.hxx>

#include <set>
#include <memory>
#include <vector>

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

constexpr int MENU_START = 0;
constexpr int MENU_COLUMN = 1;

class ScOrcusXMLContext;

class ScDataProviderBaseControl : public VclContainer,
                                    public VclBuilderContainer
{
    VclPtr<VclContainer> maGrid;
    VclPtr<ListBox> maProviderList;
    VclPtr<Edit> maEditURL;
    VclPtr<Edit> maEditID;
    VclPtr<PushButton> mpApplyBtn;

    VclPtr<VclContainer> mpMapGrid;

    VclPtr<SvTreeListBox> mpLbTree;
    VclPtr<PushButton> mpXmlImportBtn;

    bool mbDirty;
    OUString maOldProvider;
    OUString maURL;
    OUString maID;

    Link<Window*, void> maImportCallback;

    DECL_LINK(ProviderSelectHdl, ListBox&, void);
    DECL_LINK(IDEditHdl, Edit&, void);
    DECL_LINK(URLEditHdl, Edit&, void);
    DECL_LINK(ApplyBtnHdl, Button*, void);
    DECL_LINK(mpXmlImportBtnHdl, Button*, void);

    void updateApplyBtn(bool bValidConfig);
    void LoadSourceFileStructure(const OUString& rPath);
    void TreeItemSelected();
    void DefaultElementSelected(SvTreeListEntry& rEntry);
    void RepeatElementSelected(SvTreeListEntry& rEntry);
    void AttributeSelected(SvTreeListEntry& rEntry);

    void SetNonLinkable();
    void SetSingleLinkable();
    void SetRangeLinkable();
    void SelectAllChildEntries(SvTreeListEntry& rEntry);

    bool IsParentDirty(SvTreeListEntry* pEntry) const;

    bool IsChildrenDirty(SvTreeListEntry* pEntry) const;

    ScOrcusXMLTreeParam maXMLParam;
    std::set<const SvTreeListEntry*> maCellLinks;
    std::set<const SvTreeListEntry*> maRangeLinks;
    std::vector<SvTreeListEntry*> maHighlightedEntries;
    SvTreeListEntry* mpCurRefEntry;
    std::unique_ptr<ScOrcusXMLContext> mpXMLContext;
    ScDocument* mpDoc;

    DECL_LINK(TreeItemSelectHdl, SvTreeListBox*, void);


public:
    ScDataProviderBaseControl(vcl::Window* pParent, const Link<Window*, void>& rImportCallback);
    ~ScDataProviderBaseControl() override;

    virtual void dispose() override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual Size calculateRequisition() const override;

    void isValid();


    sc::ExternalDataSource getDataSource(ScDocument* pDoc);
};

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
        const SvLBoxItem* pItem = p->GetFirstItem(SvLBoxItemType::String);
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

void ScDataProviderBaseControl::SetNonLinkable()
{
    mpMapGrid->Disable();
}

void ScDataProviderBaseControl::SetSingleLinkable()
{
    mpMapGrid->Enable();
}

void ScDataProviderBaseControl::SetRangeLinkable()
{
    mpMapGrid->Enable();
}

void ScDataProviderBaseControl::SelectAllChildEntries(SvTreeListEntry& rEntry)
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

bool ScDataProviderBaseControl::IsChildrenDirty(SvTreeListEntry* pEntry) const
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

bool ScDataProviderBaseControl::IsParentDirty(SvTreeListEntry* pEntry) const
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

ScDataProviderBaseControl::ScDataProviderBaseControl(vcl::Window* pParent,
        const Link<Window*, void>& rImportCallback):
    VclContainer(pParent, WB_CLIPCHILDREN | WB_BORDER),
    mbDirty(false),
    maImportCallback(rImportCallback)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "modules/scalc/ui/dataproviderentry.ui"));

    get(maGrid, "grid");
    get(maProviderList, "provider_lst");
    get(maEditURL, "ed_url");
    get(maEditID, "ed_id");
     get(mpMapGrid, "mapgrid");
    get(mpLbTree, "tree");
    Size aTreeSize(mpLbTree->LogicToPixel(Size(130, 120), MapMode(MapUnit::MapAppFont)));
    mpLbTree->set_width_request(aTreeSize.Width());
    mpLbTree->set_height_request(aTreeSize.Height());
    get(mpXmlImportBtn, "xml_import");

    auto aDataProvider = sc::DataProviderFactory::getDataProviders();
    for (auto& rDataProvider : aDataProvider)
    {
        maProviderList->InsertEntry(rDataProvider);
    }

    maProviderList->SetSelectHdl(LINK(this, ScDataProviderBaseControl, ProviderSelectHdl));
    maEditID->SetModifyHdl(LINK(this, ScDataProviderBaseControl, IDEditHdl));
    maEditURL->SetModifyHdl(LINK(this, ScDataProviderBaseControl, URLEditHdl));

    mpApplyBtn = VclPtr<PushButton>::Create(maGrid, WB_FLATBUTTON);
    mpApplyBtn->set_grid_top_attach(1);
    mpApplyBtn->set_grid_left_attach(5);
    mpApplyBtn->SetQuickHelpText("Apply Changes");
    mpApplyBtn->SetControlForeground(COL_GREEN);
    mpApplyBtn->SetControlBackground(COL_GREEN);
    mpApplyBtn->SetBackground(Wallpaper(COL_LIGHTGREEN));
    mpApplyBtn->SetModeImage(Image(BitmapEx("sc/res/xml_element.png")));
    mpApplyBtn->Show();
    mpApplyBtn->SetClickHdl(LINK(this, ScDataProviderBaseControl, ApplyBtnHdl));

    mpLbTree->SetSelectHdl(LINK(this, ScDataProviderBaseControl, TreeItemSelectHdl));

    SetSizePixel(GetOptimalSize());
    isValid();
}

ScDataProviderBaseControl::~ScDataProviderBaseControl()
{
    disposeOnce();
}

void ScDataProviderBaseControl::LoadSourceFileStructure(const OUString& rPath)
{
    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    if (!pOrcus)
        return;

    mpXMLContext.reset(pOrcus->createXMLContext(*mpDoc, rPath));
    if (!mpXMLContext)
        return;

    mpXMLContext->loadXMLStructure(*mpLbTree, maXMLParam);
}


namespace {

class UnhighlightEntry
{
    SvTreeListBox& mrTree;
public:
    explicit UnhighlightEntry(SvTreeListBox& rTree) : mrTree(rTree) {}

    void operator() (const SvTreeListEntry* p)
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
SvTreeListEntry* getReferenceEntry(const SvTreeListBox& rTree, SvTreeListEntry* pCurEntry)
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


void ScDataProviderBaseControl::TreeItemSelected()
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

    //maRangeLinks.insert(mpCurRefEntry);
    //maCellLinks.insert(mpCurRefEntry);

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



void ScDataProviderBaseControl::AttributeSelected(SvTreeListEntry& rEntry)
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

void ScDataProviderBaseControl::RepeatElementSelected(SvTreeListEntry& rEntry)
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

void ScDataProviderBaseControl::DefaultElementSelected(SvTreeListEntry& rEntry)
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


void ScDataProviderBaseControl::dispose()
{
    maEditID.clear();
    maEditURL.clear();
    maProviderList.clear();
    mpApplyBtn.disposeAndClear();
    maGrid.clear();
    mpMapGrid.clear();
    mpLbTree.clear();
    mpXmlImportBtn.disposeAndClear();
    disposeBuilder();
    VclContainer::dispose();
}

Size ScDataProviderBaseControl::calculateRequisition() const
{
    return getLayoutRequisition(*maGrid);
}

void ScDataProviderBaseControl::setAllocation(const Size &rAllocation)
{
    setLayoutPosSize(*maGrid, Point(0, 0), rAllocation);
}

void ScDataProviderBaseControl::isValid()
{
    bool bValid = !maProviderList->GetSelectedEntry().isEmpty();
    bValid &= !maEditURL->GetText().isEmpty();

    if (bValid)
    {
        Color aColor = GetSettings().GetStyleSettings().GetDialogColor();
        SetBackground(aColor);
        maGrid->SetBackground(aColor);
        Invalidate();
        updateApplyBtn(true);
    }
    else
    {
        SetBackground(Wallpaper(COL_RED));
        maGrid->SetBackground(Wallpaper(COL_RED));
        Invalidate();
        updateApplyBtn(false);
    }
}

sc::ExternalDataSource ScDataProviderBaseControl::getDataSource(ScDocument* pDoc)
{
    OUString aURL = maEditURL->GetText();
    OUString aProvider = maProviderList->GetSelectedEntry();
    sc::ExternalDataSource aSource(aURL, aProvider, pDoc);

    OUString aID = maEditID->GetText();
    aSource.setID(aID);

    if(aProvider == "org.libreoffice.calc.xml"){

        OUString aRefStr = "A1";

        // Check if the address is valid
        ScAddress aLinkedPos;
        ScRefFlags nRes = aLinkedPos.Parse(aRefStr, pDoc, pDoc->GetAddressConvention());
        bool bValid = ( (nRes & ScRefFlags::VALID) == ScRefFlags::VALID );


        if (!bValid)
            aLinkedPos.SetInvalid();

        OSL_ASSERT(mpCurRefEntry);

        ScOrcusXMLTreeParam::EntryData* pUserData1 = ScOrcusXMLTreeParam::getUserData(*mpCurRefEntry);


        OSL_ASSERT(pUserData1);

        bool bRepeatElem = pUserData1->meType == ScOrcusXMLTreeParam::ElementRepeat;
        pUserData1->maLinkedPos = aLinkedPos;
        pUserData1->mbRangeParent = aLinkedPos.IsValid() && bRepeatElem;

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

       ScOrcusImportXMLParam aParam;

        // Convert single cell links.
        {
            std::set<const SvTreeListEntry*>::const_iterator it = maCellLinks.begin(), itEnd = maCellLinks.end();
            for (; it != itEnd; ++it)
            {
                const SvTreeListEntry& rEntry = **it;
                OUString aPath = getXPath(*mpLbTree, rEntry, aParam.maNamespaces);
                const ScOrcusXMLTreeParam::EntryData* pUserData = ScOrcusXMLTreeParam::getUserData(rEntry);

                aParam.maCellLinks.emplace_back(
                        pUserData->maLinkedPos, OUStringToOString(aPath, RTL_TEXTENCODING_UTF8));
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

        aSource.setXMLImportParam(aParam);
    }
    return aSource;
}

void ScDataProviderBaseControl::updateApplyBtn(bool bValidConfig)
{
    if (!bValidConfig)
    {
        mpApplyBtn->Disable();
        mpApplyBtn->SetBackground(Wallpaper(COL_RED));
        mpApplyBtn->SetQuickHelpText("");
        return;
    }

    if (mbDirty)
    {
        mpApplyBtn->Enable();
        mpApplyBtn->SetBackground(Wallpaper(COL_YELLOW));
        mpApplyBtn->SetQuickHelpText("Apply Changes");
    }
    else
    {
        mpApplyBtn->Disable();
        mpApplyBtn->SetBackground(Wallpaper(COL_GREEN));
        mpApplyBtn->SetQuickHelpText("Current Config Applied");
    }
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, ProviderSelectHdl, ListBox&, void)
{
    isValid();
    mbDirty |= maOldProvider != maProviderList->GetSelectedEntry();
    maOldProvider = maProviderList->GetSelectedEntry();
}


IMPL_LINK_NOARG(ScDataProviderBaseControl, mpXmlImportBtnHdl, Button*, void)
{
    maImportCallback.Call(this);
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, TreeItemSelectHdl, SvTreeListBox*, void)
{
    TreeItemSelected();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, IDEditHdl, Edit&, void)
{
    isValid();
    mbDirty |= maEditID->GetText() != maID;
    maID = maEditID->GetText();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, URLEditHdl, Edit&, void)
{
    isValid();
    mbDirty |= maEditURL->GetText() != maURL;
    maURL = maEditURL->GetText();
}

IMPL_LINK_NOARG(ScDataProviderBaseControl, ApplyBtnHdl, Button*, void)
{
     OUString aProvider = maProviderList->GetSelectedEntry();
    if(aProvider != "org.libreoffice.calc.xml"){
        mbDirty = false;
        updateApplyBtn(true);
        maImportCallback.Call(this);
    }
    else {
        OUString aURL = maEditURL->GetText();
        LoadSourceFileStructure(aURL);
        mpXmlImportBtn->SetQuickHelpText("Import XML");
        mpXmlImportBtn->SetControlForeground(COL_GREEN);
        mpXmlImportBtn->SetControlBackground(COL_GREEN);
        mpXmlImportBtn->SetBackground(Wallpaper(COL_LIGHTGREEN));
        mpXmlImportBtn->SetModeImage(Image(BitmapEx("sc/res/xml_element.png")));
        mpXmlImportBtn->Show();
        mpXmlImportBtn->SetClickHdl(LINK(this, ScDataProviderBaseControl, mpXmlImportBtnHdl));

    }
}

namespace {

struct MenuData
{
    int nMenuID;
    const char* aMenuName;
    std::function<void(ScDataProviderDlg*)> maCallback;
};

MenuData aStartData[] = {
    { 0, "Apply & Quit", &ScDataProviderDlg::applyAndQuit },
    { 1, "Cancel & Quit", &ScDataProviderDlg::cancelAndQuit }
};

MenuData aColumnData[] = {
    { 0, "Delete Column", &ScDataProviderDlg::deleteColumn },
    { 1, "Split Column", &ScDataProviderDlg::splitColumn },
    { 2, "Merge Columns", &ScDataProviderDlg::mergeColumns },
};

class ScDataTransformationBaseControl : public VclContainer,
                                    public VclBuilderContainer
{
    VclPtr<VclContainer> maGrid;

public:
    ScDataTransformationBaseControl(vcl::Window* pParent, const OUString& rUIFile);
    ~ScDataTransformationBaseControl() override;

    virtual void dispose() override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual Size calculateRequisition() const override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() = 0;
};

ScDataTransformationBaseControl::ScDataTransformationBaseControl(vcl::Window* pParent, const OUString& rUIFile):
    VclContainer(pParent, WB_BORDER | WB_CLIPCHILDREN)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), rUIFile));

    get(maGrid, "grid");
    SetSizePixel(GetOptimalSize());
}

ScDataTransformationBaseControl::~ScDataTransformationBaseControl()
{
    disposeOnce();
}

void ScDataTransformationBaseControl::dispose()
{
    maGrid.clear();

    VclContainer::dispose();
}

Size ScDataTransformationBaseControl::calculateRequisition() const
{
    return getLayoutRequisition(*maGrid);
}

void ScDataTransformationBaseControl::setAllocation(const Size &rAllocation)
{
    setLayoutPosSize(*maGrid, Point(0, 0), rAllocation);
}

class ScSplitColumnTransformationControl : public ScDataTransformationBaseControl
{
private:
    VclPtr<Edit> maSeparator;
    VclPtr<NumericField> maNumColumns;
    SCCOL mnCol;

public:
    ScSplitColumnTransformationControl(vcl::Window* pParent, SCCOL nCol);
    ~ScSplitColumnTransformationControl() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
};

ScSplitColumnTransformationControl::ScSplitColumnTransformationControl(vcl::Window* pParent, SCCOL nCol):
    ScDataTransformationBaseControl(pParent, "modules/scalc/ui/splitcolumnentry.ui"),
    mnCol(nCol)
{
    get(maSeparator, "ed_separator");
    get(maNumColumns, "num_cols");
}

ScSplitColumnTransformationControl::~ScSplitColumnTransformationControl()
{
    disposeOnce();
}

void ScSplitColumnTransformationControl::dispose()
{
    maSeparator.clear();
    maNumColumns.clear();

    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScSplitColumnTransformationControl::getTransformation()
{
    OUString aSeparator = maSeparator->GetText();
    sal_Unicode cSeparator = aSeparator.isEmpty() ? ',' : aSeparator[0];
    return std::make_shared<sc::SplitColumnTransformation>(mnCol, cSeparator);
}

class ScMergeColumnTransformationControl : public ScDataTransformationBaseControl
{
private:

    VclPtr<Edit> mpSeparator;
    VclPtr<Edit> mpEdColumns;

public:
    ScMergeColumnTransformationControl(vcl::Window* pParent, SCCOL nStartCol, SCCOL nEndCol);
    ~ScMergeColumnTransformationControl() override;

    virtual void dispose() override;

    virtual std::shared_ptr<sc::DataTransformation> getTransformation() override;
};

ScMergeColumnTransformationControl::ScMergeColumnTransformationControl(vcl::Window* pParent, SCCOL nStartCol, SCCOL nEndCol):
    ScDataTransformationBaseControl(pParent, "modules/scalc/ui/mergecolumnentry.ui")
{
    get(mpSeparator, "ed_separator");
    get(mpEdColumns, "ed_columns");

    OUStringBuffer aBuffer;

    // map from zero based to one based column numbers
    aBuffer.append(OUString::number(nStartCol + 1));
    for ( SCCOL nCol = nStartCol + 1; nCol <= nEndCol; ++nCol)
    {
        aBuffer.append(";").append(OUString::number(nCol + 1));
    }

    mpEdColumns->SetText(aBuffer.makeStringAndClear());
}

ScMergeColumnTransformationControl::~ScMergeColumnTransformationControl()
{
    disposeOnce();
}

void ScMergeColumnTransformationControl::dispose()
{
    mpSeparator.clear();
    mpEdColumns.clear();

    ScDataTransformationBaseControl::dispose();
}

std::shared_ptr<sc::DataTransformation> ScMergeColumnTransformationControl::getTransformation()
{
    OUString aColumnString = mpEdColumns->GetText();
    std::vector<OUString> aSplitColumns = comphelper::string::split(aColumnString, ';');
    std::set<SCCOL> aMergedColumns;
    for (auto& rColStr : aSplitColumns)
    {
        sal_Int32 nCol = rColStr.toInt32();
        if (nCol <= 0)
            continue;

        if (nCol > MAXCOL)
            continue;

        // translate from 1-based column notations to internal Calc one
        aMergedColumns.insert(nCol - 1);
    }
    return std::make_shared<sc::MergeColumnTransformation>(aMergedColumns, mpSeparator->GetText());
}

}

ScDataProviderDlg::ScDataProviderDlg(vcl::Window* pParent, std::shared_ptr<ScDocument> pDoc):
    ModalDialog(pParent, "dataproviderdlg", "modules/scalc/ui/dataproviderdlg.ui", true),
    mpDoc(pDoc),
    mpBar(VclPtr<MenuBar>::Create())
{
    get(mpTable, "data_table");
    get(mpList, "operation_ctrl");
    mpTable->Init(mpDoc);

    mpDataProviderCtrl = VclPtr<ScDataProviderBaseControl>::Create(mpList, LINK(this, ScDataProviderDlg, ImportHdl));
    mpList->addEntry(mpDataProviderCtrl);

    pDBData = new ScDBData("data", 0, 0, 0, MAXCOL, MAXROW);
    bool bSuccess = mpDoc->GetDBCollection()->getNamedDBs().insert(pDBData);
    SAL_WARN_IF(!bSuccess, "sc", "temporary warning");

    InitMenu();
}

ScDataProviderDlg::~ScDataProviderDlg()
{
    disposeOnce();
}

void ScDataProviderDlg::dispose()
{
    mpDataProviderCtrl.clear();
    mpTable.clear();
    mpList.clear();
    mpBar.disposeAndClear();

    ModalDialog::dispose();
}

void ScDataProviderDlg::InitMenu()
{
    mpBar->InsertItem(MENU_START, "Start");
    VclPtrInstance<PopupMenu> pPopup;
    for (auto& itrStartData : aStartData)
    {
        pPopup->InsertItem(itrStartData.nMenuID, OUString::createFromAscii(itrStartData.aMenuName));
    }

    mpBar->SetPopupMenu(MENU_START, pPopup);
    pPopup->SetSelectHdl(LINK(this, ScDataProviderDlg, StartMenuHdl));

    mpBar->InsertItem(MENU_COLUMN, "Column");
    VclPtrInstance<PopupMenu> pColumnMenu;
    for (auto& itrColumnData : aColumnData)
 {
        pColumnMenu->InsertItem(itrColumnData.nMenuID, OUString::createFromAscii(itrColumnData.aMenuName));
    }
    pColumnMenu->SetSelectHdl(LINK(this, ScDataProviderDlg, ColumnMenuHdl));

    mpBar->SetPopupMenu(MENU_COLUMN, pColumnMenu);

    SetMenuBar(mpBar.get());
}

void ScDataProviderDlg::MouseButtonUp(const MouseEvent& rMEvt)
{
    VclPtr<FixedText> mpText = VclPtr<FixedText>::Create(mpList);
    mpText->SetText("Some Text " + OUString::number(rMEvt.GetPosPixel().X()) + "x" + OUString::number(rMEvt.GetPosPixel().getY()));
    mpText->SetSizePixel(Size(400, 20));
    mpList->addEntry(mpText);
}

IMPL_LINK(ScDataProviderDlg, StartMenuHdl, Menu*, pMenu, bool)
{
    for (auto& i: aStartData)
    {
        if (i.nMenuID == pMenu->GetCurItemId())
        {
            i.maCallback(this);
            return true;
        }
    }
    return true;
}

IMPL_LINK(ScDataProviderDlg, ColumnMenuHdl, Menu*, pMenu, bool)
{
    for (auto& i: aColumnData)
    {
        if (i.nMenuID == pMenu->GetCurItemId())
        {
            i.maCallback(this);
            return true;
        }
    }
    return true;
}

IMPL_LINK(ScDataProviderDlg, ImportHdl, Window*, pCtrl, void)
{
    if (pCtrl == mpDataProviderCtrl.get())
    {
        import();
    }
}

void ScDataProviderDlg::applyAndQuit()
{
    EndDialog(RET_OK);
}

void ScDataProviderDlg::cancelAndQuit()
{
    EndDialog(RET_CANCEL);
}

void ScDataProviderDlg::deleteColumn()
{
    VclPtr<FixedText> mpText = VclPtr<FixedText>::Create(mpList);
    mpText->SetText("Delete Column");
    mpText->SetSizePixel(Size(400, 20));
    mpList->addEntry(mpText);
}

void ScDataProviderDlg::splitColumn()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mpTable->getColRange(nStartCol, nEndCol);
    VclPtr<ScSplitColumnTransformationControl> pSplitColumnEntry = VclPtr<ScSplitColumnTransformationControl>::Create(mpList, nStartCol);
    mpList->addEntry(pSplitColumnEntry);
}

void ScDataProviderDlg::mergeColumns()
{
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    mpTable->getColRange(nStartCol, nEndCol);
    VclPtr<ScMergeColumnTransformationControl> pMergeColumnEntry = VclPtr<ScMergeColumnTransformationControl>::Create(mpList, nStartCol, nEndCol);
    mpList->addEntry(pMergeColumnEntry);
}

void ScDataProviderDlg::import()
{
    sc::ExternalDataSource aSource = mpDataProviderCtrl->getDataSource(mpDoc.get());
    std::vector<VclPtr<vcl::Window>> aListEntries = mpList->getEntries();
    for (size_t i = 1; i < aListEntries.size(); ++i)
    {
        ScDataTransformationBaseControl* pTransformationCtrl = dynamic_cast<ScDataTransformationBaseControl*>(aListEntries[i].get());
        if (!pTransformationCtrl)
        {
            SAL_WARN("sc", "all children except the provider should inherit from the base control");
            continue;
        }
        aSource.AddDataTransformation(pTransformationCtrl->getTransformation());
    }
    aSource.setDBData(pDBData);
    aSource.refresh(mpDoc.get(), true);
    mpTable->Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
