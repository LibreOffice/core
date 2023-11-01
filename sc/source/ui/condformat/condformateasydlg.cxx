#include <docfunc.hxx>
#include <condformateasydlg.hxx>
#include <stlpool.hxx>
#include <viewdata.hxx>
#include <reffact.hxx>
#include <scresid.hxx>
#include <svl/style.hxx>
#include <strings.hrc>

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
void ConditionalFormatEasyDialog::SetDescription(std::u16string_view rCondition)
{
    mxDescription->set_label(mxDescription->get_label() + ": " + rCondition);
}

ConditionalFormatEasyDialog::ConditionalFormatEasyDialog(SfxBindings* pBindings,
                                                         SfxChildWindow* pChildWindow,
                                                         weld::Window* pParent,
                                                         ScViewData* pViewData)
    : ScAnyRefDlgController(pBindings, pChildWindow, pParent,
                            "modules/scalc/ui/conditionaleasydialog.ui", "CondFormatEasyDlg")
    , mpViewData(pViewData)
    , mpDocument(&mpViewData->GetDocument())
    , mxNumberEntry(m_xBuilder->weld_entry("entryNumber"))
    , mxNumberEntry2(m_xBuilder->weld_entry("entryNumber2"))
    , mxRangeEntry(new formula::RefEdit(m_xBuilder->weld_entry("entryRange")))
    , mxButtonRangeEdit(new formula::RefButton(m_xBuilder->weld_button("rbassign")))
    , mxStyles(m_xBuilder->weld_combo_box("themeCombo"))
    , mxDescription(m_xBuilder->weld_label("description"))
    , mxButtonOk(m_xBuilder->weld_button("ok"))
    , mxButtonCancel(m_xBuilder->weld_button("cancel"))
{
    mxButtonRangeEdit->SetReferences(this, mxRangeEntry.get());
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
    switch (meMode)
    {
        case ScConditionMode::Equal:
            SetDescription(ScResId(STR_CONDITION_EQUAL));
            break;
        case ScConditionMode::Less:
            SetDescription(ScResId(STR_CONDITION_LESS));
            break;
        case ScConditionMode::Greater:
            SetDescription(ScResId(STR_CONDITION_GREATER));
            break;
        case ScConditionMode::EqLess:
            SetDescription(ScResId(STR_CONDITION_EQLESS));
            break;
        case ScConditionMode::EqGreater:
            SetDescription(ScResId(STR_CONDITION_EQGREATER));
            break;
        case ScConditionMode::NotEqual:
            SetDescription(ScResId(STR_CONDITION_NOT_EQUAL));
            break;
        case ScConditionMode::Between:
            SetDescription(ScResId(STR_CONDITION_BETWEEN));
            mxNumberEntry2->show();
            break;
        // NotBetween
        // Duplicate
        // NotDuplicate
        // Direct
        // Top10
        // Bottom10
        // TopPercent
        // BottomPercent
        // AboveAverage
        // BelowAverage
        // AboveEqualAverage
        // BelowEqualAverage
        case ScConditionMode::Error:
            SetDescription(ScResId(STR_CONDITION_ERROR));
            break;
        case ScConditionMode::NoError:
            SetDescription(ScResId(STR_CONDITION_NOERROR));
            break;
        // BeginsWith
        // EndsWith
        case ScConditionMode::ContainsText:
            SetDescription(ScResId(STR_CONDITION_CONTAINS_TEXT));
            break;
        case ScConditionMode::NotContainsText:
            SetDescription(ScResId(STR_CONDITION_NOT_CONTAINS_TEXT));
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

        OUString sExpression1 = mxNumberEntry->get_text();
        OUString sExpression2 = mxNumberEntry2->get_text();

        switch (meMode)
        {
            case ScConditionMode::ContainsText:
            case ScConditionMode::NotContainsText:
            case ScConditionMode::BeginsWith:
            case ScConditionMode::EndsWith:
                sExpression1 = "\"" + sExpression1 + "\"";
                sExpression2 = "\"" + sExpression2 + "\"";
                break;
            default:
                break;
        }

        ScFormatEntry* pEntry
            = new ScCondFormatEntry(meMode, sExpression1, sExpression2, *mpDocument, maPosition,
                                    mxStyles->get_active_text());

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
