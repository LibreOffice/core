#include <docfunc.hxx>
#include <condformateasydlg.hxx>
#include <stlpool.hxx>
#include <viewdata.hxx>
#include <reffact.hxx>
#include <scresid.hxx>
#include <svl/style.hxx>
#include <strings.hrc>
#include <condformathelper.hxx>

namespace
{
condformat::ScCondFormatDateType GetScCondFormatDateType(ScConditionMode mode)
{
    switch (mode)
    {
        case ScConditionMode::Today:
            return condformat::ScCondFormatDateType::TODAY;
        case ScConditionMode::Yesterday:
            return condformat::ScCondFormatDateType::YESTERDAY;
        case ScConditionMode::Tomorrow:
            return condformat::ScCondFormatDateType::TOMORROW;
        case ScConditionMode::Last7days:
            return condformat::ScCondFormatDateType::LAST7DAYS;
        case ScConditionMode::ThisWeek:
            return condformat::ScCondFormatDateType::THISWEEK;
        case ScConditionMode::LastWeek:
            return condformat::ScCondFormatDateType::LASTWEEK;
        case ScConditionMode::NextWeek:
            return condformat::ScCondFormatDateType::NEXTWEEK;
        case ScConditionMode::ThisMonth:
            return condformat::ScCondFormatDateType::THISMONTH;
        case ScConditionMode::LastMonth:
            return condformat::ScCondFormatDateType::LASTMONTH;
        case ScConditionMode::NextMonth:
            return condformat::ScCondFormatDateType::NEXTMONTH;
        case ScConditionMode::ThisYear:
            return condformat::ScCondFormatDateType::THISYEAR;
        case ScConditionMode::LastYear:
            return condformat::ScCondFormatDateType::LASTYEAR;
        case ScConditionMode::NextYear:
            return condformat::ScCondFormatDateType::NEXTYEAR;
        default:
            return condformat::ScCondFormatDateType::TODAY;
    }
}
}

namespace sc
{
void ConditionalFormatEasyDialog::SetDescription(std::u16string_view rCondition)
{
    mxDescription->set_label(mxDescription->get_label().replaceAll("%1", rCondition));
}

ConditionalFormatEasyDialog::ConditionalFormatEasyDialog(SfxBindings* pBindings,
                                                         SfxChildWindow* pChildWindow,
                                                         weld::Window* pParent,
                                                         ScViewData* pViewData)
    : ScAnyRefDlgController(pBindings, pChildWindow, pParent,
                            "modules/scalc/ui/conditionaleasydialog.ui", "CondFormatEasyDlg")
    , mpParent(pParent)
    , mpViewData(pViewData)
    , mpDocument(&mpViewData->GetDocument())
    , mxNumberEntry(m_xBuilder->weld_entry("entryNumber"))
    , mxNumberEntry2(m_xBuilder->weld_entry("entryNumber2"))
    , mxAllInputs(m_xBuilder->weld_container("allInputs"))
    , mxRangeEntry(new formula::RefEdit(m_xBuilder->weld_entry("entryRange")))
    , mxButtonRangeEdit(new formula::RefButton(m_xBuilder->weld_button("rbassign")))
    , mxStyles(m_xBuilder->weld_combo_box("themeCombo"))
    , mxWdPreviewWin(m_xBuilder->weld_widget("previewwin"))
    , mxWdPreview(new weld::CustomWeld(*m_xBuilder, "preview", maWdPreview))
    , mxDescription(m_xBuilder->weld_label("description"))
    , mxButtonOk(m_xBuilder->weld_button("ok"))
    , mxButtonCancel(m_xBuilder->weld_button("cancel"))
{
    mxButtonRangeEdit->SetReferences(this, mxRangeEntry.get());
    const ScConditionEasyDialogData CurrentData
        = pViewData->GetDocument().GetEasyConditionalFormatDialogData();
    if (!CurrentData.Mode)
    {
        SAL_WARN(
            "sc",
            "Condition mode not set for easy conditional format dialog, this should not happen");
        meMode = ScConditionMode::Greater;
    }
    else
    {
        meMode = *CurrentData.Mode;
        mbIsManaged = CurrentData.IsManaged;
        msFormula = CurrentData.Formula;
        mnFormatKey = CurrentData.FormatKey;
        mnEntryIndex = CurrentData.EntryIndex;
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
        case ScConditionMode::NotBetween:
            SetDescription(ScResId(STR_CONDITION_NOT_BETWEEN));
            mxNumberEntry2->show();
            break;
        case ScConditionMode::Duplicate:
            SetDescription(ScResId(STR_CONDITION_DUPLICATE));
            mxAllInputs->hide();
            break;
        case ScConditionMode::NotDuplicate:
            SetDescription(ScResId(STR_CONDITION_NOT_DUPLICATE));
            mxAllInputs->hide();
            break;
        // TODO: Direct
        case ScConditionMode::Top10:
            SetDescription(ScResId(STR_CONDITION_TOP_N_ELEMENTS));
            break;
        case ScConditionMode::Bottom10:
            SetDescription(ScResId(STR_CONDITION_BOTTOM_N_ELEMENTS));
            break;

        case ScConditionMode::TopPercent:
            SetDescription(ScResId(STR_CONDITION_TOP_N_PERCENT));
            break;
        case ScConditionMode::BottomPercent:
            SetDescription(ScResId(STR_CONDITION_BOTTOM_N_PERCENT));
            break;

        case ScConditionMode::AboveAverage:
            SetDescription(ScResId(STR_CONDITION_ABOVE_AVERAGE));
            mxAllInputs->hide();
            break;
        case ScConditionMode::BelowAverage:
            SetDescription(ScResId(STR_CONDITION_BELOW_AVERAGE));
            mxAllInputs->hide();
            break;

        case ScConditionMode::AboveEqualAverage:
            SetDescription(ScResId(STR_CONDITION_ABOVE_OR_EQUAL_AVERAGE));
            mxAllInputs->hide();
            break;
        case ScConditionMode::BelowEqualAverage:
            SetDescription(ScResId(STR_CONDITION_BELOW_OR_EQUAL_AVERAGE));
            mxAllInputs->hide();
            break;
        case ScConditionMode::Error:
            SetDescription(ScResId(STR_CONDITION_ERROR));
            break;
        case ScConditionMode::NoError:
            SetDescription(ScResId(STR_CONDITION_NOERROR));
            break;
        case ScConditionMode::BeginsWith:
            SetDescription(ScResId(STR_CONDITION_BEGINS_WITH));
            break;
        case ScConditionMode::EndsWith:
            SetDescription(ScResId(STR_CONDITION_ENDS_WITH));
            break;
        case ScConditionMode::ContainsText:
            SetDescription(ScResId(STR_CONDITION_CONTAINS_TEXT));
            break;
        case ScConditionMode::NotContainsText:
            SetDescription(ScResId(STR_CONDITION_NOT_CONTAINS_TEXT));
            break;
        case ScConditionMode::Formula:
            SetDescription(ScResId(STR_CONDITION_FORMULA));
            mxAllInputs->hide();
            break;
        case ScConditionMode::Today:
            SetDescription(ScResId(STR_CONDITION_TODAY));
            mxAllInputs->hide();
            break;
        case ScConditionMode::Yesterday:
            SetDescription(ScResId(STR_CONDITION_YESTERDAY));
            mxAllInputs->hide();
            break;
        case ScConditionMode::Tomorrow:
            SetDescription(ScResId(STR_CONDITION_TOMORROW));
            mxAllInputs->hide();
            break;
        case ScConditionMode::Last7days:
            SetDescription(ScResId(STR_CONDITION_LAST7DAYS));
            mxAllInputs->hide();
            break;
        case ScConditionMode::ThisWeek:
            SetDescription(ScResId(STR_CONDITION_THISWEEK));
            mxAllInputs->hide();
            break;
        case ScConditionMode::LastWeek:
            SetDescription(ScResId(STR_CONDITION_LASTWEEK));
            mxAllInputs->hide();
            break;
        case ScConditionMode::NextWeek:
            SetDescription(ScResId(STR_CONDITION_NEXTWEEK));
            mxAllInputs->hide();
            break;
        case ScConditionMode::ThisMonth:
            SetDescription(ScResId(STR_CONDITION_THISMONTH));
            mxAllInputs->hide();
            break;
        case ScConditionMode::LastMonth:
            SetDescription(ScResId(STR_CONDITION_LASTMONTH));
            mxAllInputs->hide();
            break;
        case ScConditionMode::NextMonth:
            SetDescription(ScResId(STR_CONDITION_NEXTMONTH));
            mxAllInputs->hide();
            break;
        case ScConditionMode::ThisYear:
            SetDescription(ScResId(STR_CONDITION_THISYEAR));
            mxAllInputs->hide();
            break;
        case ScConditionMode::LastYear:
            SetDescription(ScResId(STR_CONDITION_LASTYEAR));
            mxAllInputs->hide();
            break;
        case ScConditionMode::NextYear:
            SetDescription(ScResId(STR_CONDITION_NEXTYEAR));
            mxAllInputs->hide();
            break;
        default:
            SAL_WARN("sc",
                     "ConditionalFormatEasyDialog::ConditionalFormatEasyDialog: invalid format");
            break;
    }

    mxButtonOk->connect_clicked(LINK(this, ConditionalFormatEasyDialog, ButtonPressed));
    mxButtonCancel->connect_clicked(LINK(this, ConditionalFormatEasyDialog, ButtonPressed));
    mxStyles->connect_changed(LINK(this, ConditionalFormatEasyDialog, StyleSelectHdl));

    ScRangeList aRange;
    mpViewData->GetMarkData().FillRangeListWithMarks(&aRange, false);
    if (aRange.empty() && mnFormatKey != -1 && mnEntryIndex != -1)
    {
        aRange = mpDocument->GetCondFormList(mpViewData->GetTabNo())
                     ->GetFormat(mnFormatKey)
                     ->GetRangeList();
    }
    else if (aRange.empty())
    {
        ScAddress aPosition(mpViewData->GetCurX(), mpViewData->GetCurY(), mpViewData->GetTabNo());
        aRange.push_back(ScRange(aPosition));
    }
    maPosition = aRange.GetTopLeftCorner();
    // FIX me: Tab is always 0 in some cases
    // Refer to test tdf100793
    maPosition.SetTab(mpViewData->GetTabNo());

    OUString sRangeString;
    aRange.Format(sRangeString, ScRefFlags::VALID, *mpDocument, mpDocument->GetAddressConvention());
    mxRangeEntry->SetText(sRangeString);

    StartListening(*mpDocument->GetStyleSheetPool(), DuplicateHandling::Prevent);
    ScCondFormatHelper::FillStyleListBox(mpDocument, *mxStyles);

    mxStyles->set_active(1);
    mxWdPreviewWin->show();
}

ConditionalFormatEasyDialog::~ConditionalFormatEasyDialog()
{
    if (mbIsManaged)
    {
        GetBindings().GetDispatcher()->Execute(SID_OPENDLG_CONDFRMT_MANAGER,
                                               SfxCallMode::ASYNCHRON);
    }
}

void ConditionalFormatEasyDialog::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::StyleSheetModified)
        ScCondFormatHelper::UpdateStyleList(*mxStyles, mpDocument);
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
        if (mnEntryIndex != -1 && mnFormatKey != -1) // isEdit
            mpDocument->GetCondFormList(maPosition.Tab())
                ->GetFormat(mnFormatKey)
                ->RemoveEntry(mnEntryIndex);

        std::unique_ptr<ScConditionalFormat> pFormat(new ScConditionalFormat(0, mpDocument));

        OUString sExpression1
            = (mxNumberEntry->get_visible() == true && mxAllInputs->get_visible() == true
                   ? mxNumberEntry->get_text()
                   : "");
        OUString sExpression2
            = (mxNumberEntry2->get_visible() == true && mxAllInputs->get_visible() == true
                   ? mxNumberEntry2->get_text()
                   : "");

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

        ScFormatEntry* pEntry;
        if (meMode < ScConditionMode::Formula)
        {
            pEntry = new ScCondFormatEntry(meMode, sExpression1, sExpression2, *mpDocument,
                                           maPosition, mxStyles->get_active_text());
        }
        else if (meMode >= ScConditionMode::Today && meMode < ScConditionMode::NONE)
        {
            ScCondDateFormatEntry entry(mpDocument);
            entry.SetDateType(GetScCondFormatDateType(meMode));
            entry.SetStyleName(mxStyles->get_active_text());
            pEntry = new ScCondDateFormatEntry(mpDocument, entry);
        }
        else if (meMode == ScConditionMode::Formula)
        {
            pEntry = new ScCondFormatEntry(ScConditionMode::Direct, msFormula, OUString(),
                                           *mpDocument, maPosition, mxStyles->get_active_text());
        }

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

IMPL_LINK_NOARG(ConditionalFormatEasyDialog, StyleSelectHdl, weld::ComboBox&, void)
{
    ScCondFormatHelper::StyleSelect(mpParent, *mxStyles, &(mpViewData->GetDocument()), maWdPreview);
}

} // namespace sc
