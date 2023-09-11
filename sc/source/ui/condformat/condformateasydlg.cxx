#include <docfunc.hxx>
#include <condformateasydlg.hxx>
#include <stlpool.hxx>
#include <viewdata.hxx>
#include <reffact.hxx>
#include <svl/style.hxx>

namespace
{
void FillStyleListBox(const ScDocument* pDocument, weld::ComboBox& rCombo)
{
    std::set<OUString> aStyleNames;
    SfxStyleSheetIterator aStyleIter(pDocument->GetStyleSheetPool(), SfxStyleFamily::Para);
    for (SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next())
    {
        aStyleNames.insert(pStyle->GetName());
    }
    for (const auto& rStyleName : aStyleNames)
    {
        rCombo.append_text(rStyleName);
    }
}

void UpdateStyleList(const ScDocument* pDocument, weld::ComboBox& rCombo)
{
    OUString sSelectedStyle = rCombo.get_active_text();
    for (sal_Int32 i = rCombo.get_count(); i > 1; --i)
        rCombo.remove(i - 1);
    FillStyleListBox(pDocument, rCombo);
    rCombo.set_active_text(sSelectedStyle);
}

ScTabViewShell* GetTabViewShell(const SfxBindings* pBindings)
{
    if (!pBindings)
        return nullptr;
    SfxDispatcher* pDispacher = pBindings->GetDispatcher();
    if (!pDispacher)
        return nullptr;
    SfxViewFrame* pFrame = pDispacher->GetFrame();
    if (!pFrame)
        return nullptr;
    SfxViewShell* pViewShell = pFrame->GetViewShell();
    if (!pViewShell)
        return nullptr;
    return dynamic_cast<ScTabViewShell*>(pViewShell);
}
}

namespace sc
{
ConditionalFormatEasyDialog::ConditionalFormatEasyDialog(SfxBindings* pBindings,
                                                         SfxChildWindow* pChildWindow,
                                                         weld::Window* pParent,
                                                         ScViewData* pViewData)
    : ScAnyRefDlgController(pBindings, pChildWindow, pParent,
                            "modules/scalc/ui/conditionaleasydialog.ui", "CondFormatEasyDlg")
    , mpViewData(pViewData)
    , mpDocument(&mpViewData->GetDocument())
    , mxNumberEntry(m_xBuilder->weld_spin_button("entryNumber"))
    , mxNumberEntry2(m_xBuilder->weld_spin_button("entryNumber2"))
    , mxRangeEntry(new formula::RefEdit(m_xBuilder->weld_entry("entryRange")))
    , mxStyles(m_xBuilder->weld_combo_box("themeCombo"))
    , mxDescription(m_xBuilder->weld_label("description"))
    , mxButtonOk(m_xBuilder->weld_button("ok"))
    , mxButtonCancel(m_xBuilder->weld_button("cancel"))
{
    mpTabViewShell = GetTabViewShell(pBindings);
    if (!mpTabViewShell)
        mpTabViewShell = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    OSL_ENSURE(mpTabViewShell, "Missing view shell!");
    const ScConditionMode* pCurrentMode
        = pViewData->GetDocument().GetEasyConditionalFormatDialogData();
    if (!pCurrentMode)
    {
        SAL_WARN(
            "sc",
            "Condition mode not set for easy conditional format dialog, this should not happen");
        meMode = ScConditionMode::Greater;
    }
    else
    {
        meMode = *pCurrentMode;
    }
    mxNumberEntry2->hide();
    mxNumberEntry->set_range(SAL_MIN_INT64, SAL_MAX_INT64);
    mxNumberEntry2->set_range(SAL_MIN_INT64, SAL_MAX_INT64);
    mxNumberEntry->set_increments(1, 0);
    mxNumberEntry2->set_increments(1, 0);
    switch (meMode)
    {
        case ScConditionMode::Greater:
            mxDescription->set_label(mxDescription->get_label() + " greater than ");
            break;
        case ScConditionMode::Less:
            mxDescription->set_label(mxDescription->get_label() + " less than ");
            break;
        case ScConditionMode::Equal:
            mxDescription->set_label(mxDescription->get_label() + " equal to ");
            break;
        case ScConditionMode::Between:
            mxDescription->set_label(mxDescription->get_label() + " between ");
            mxNumberEntry2->show();
            break;
        default:
            SAL_WARN("sc",
                     "ConditionalFormatEasyDialog::ConditionalFormatEasyDialog: invalid format");
            break;
    }

    mxButtonOk->connect_clicked(LINK(this, ConditionalFormatEasyDialog, ButtonPressed));
    mxButtonCancel->connect_clicked(LINK(this, ConditionalFormatEasyDialog, ButtonPressed));

    ScRangeList aRange;
    mpViewData->GetMarkData().FillRangeListWithMarks(&aRange, false);
    if (aRange.empty())
    {
        ScAddress aPosition(mpViewData->GetCurX(), mpViewData->GetCurY(), mpViewData->GetTabNo());
        aRange.push_back(ScRange(aPosition));
    }
    maPosition = aRange.GetTopLeftCorner();

    OUString sRangeString;
    aRange.Format(sRangeString, ScRefFlags::VALID, *mpDocument, mpDocument->GetAddressConvention());
    mxRangeEntry->SetText(sRangeString);

    StartListening(*mpDocument->GetStyleSheetPool(), DuplicateHandling::Prevent);
    FillStyleListBox(mpDocument, *mxStyles);

    mxStyles->set_active(1);
}

ConditionalFormatEasyDialog::~ConditionalFormatEasyDialog() {}

void ConditionalFormatEasyDialog::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::StyleSheetModified)
        UpdateStyleList(mpDocument, *mxStyles);
}

void ConditionalFormatEasyDialog::SetReference(const ScRange& rRange, ScDocument&)
{
    formula::RefEdit* pEdit = mxRangeEntry.get();
    if (rRange.aStart != rRange.aEnd)
        RefInputStart(pEdit);

    ScRefFlags nFlags = ScRefFlags::RANGE_ABS;
    const ScDocument& rDoc = mpViewData->GetDocument();
    OUString sRange(
        rRange.Format(rDoc, nFlags, ScAddress::Details(mpDocument->GetAddressConvention(), 0, 0)));
    pEdit->SetRefString(sRange);
    maPosition = rRange.aStart;
}

void ConditionalFormatEasyDialog::SetActive()
{
    mxRangeEntry->GrabFocus();
    RefInputDone();
}

void ConditionalFormatEasyDialog::Close()
{
    DoClose(ConditionalFormatEasyDialogWrapper::GetChildWindowId());
}

IMPL_LINK(ConditionalFormatEasyDialog, ButtonPressed, weld::Button&, rButton, void)
{
    if (&rButton == mxButtonOk.get())
    {
        std::unique_ptr<ScConditionalFormat> pFormat(new ScConditionalFormat(0, mpDocument));
        ScFormatEntry* pEntry
            = new ScCondFormatEntry(meMode, mxNumberEntry->get_text(), mxNumberEntry2->get_text(),
                                    *mpDocument, maPosition, mxStyles->get_active_text());
        ScRangeList aRange;
        ScRefFlags nFlags
            = aRange.Parse(mxRangeEntry->GetText(), mpViewData->GetDocument(),
                           mpViewData->GetDocument().GetAddressConvention(), maPosition.Tab());
        if ((nFlags & ScRefFlags::VALID) && !aRange.empty())
        {
            pFormat->AddEntry(pEntry);
            pFormat->SetRange(aRange);
            auto& rRangeList = pFormat->GetRange();
            mpViewData->GetDocShell()->GetDocFunc().ReplaceConditionalFormat(
                0, std::move(pFormat), maPosition.Tab(), rRangeList);
        }
        m_xDialog->response(RET_OK);
    }
    else if (&rButton == mxButtonCancel.get())
        m_xDialog->response(RET_CANCEL);
}

} // namespace sc