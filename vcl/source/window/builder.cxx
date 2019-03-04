/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <memory>
#include <unordered_map>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <i18nutil/unicode.hxx>
#include <osl/module.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/builder.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/button.hxx>
#include <vcl/calendar.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fmtfield.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/headbar.hxx>
#include <vcl/IPrioritable.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/throbber.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/settings.hxx>
#include <vcl/slider.hxx>
#include <vcl/listctrl.hxx>
#include <vcl/weld.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <svdata.hxx>
#include <bitmaps.hlst>
#include <messagedialog.hxx>
#include <window.h>
#include <xmlreader/xmlreader.hxx>
#include <desktop/crashreport.hxx>
#include <salinst.hxx>
#include <strings.hrc>
#include <treeglue.hxx>
#include <tools/svlibrary.h>
#include <tools/diagnose_ex.h>

#ifdef DISABLE_DYNLOADING
#include <dlfcn.h>
#endif

static bool toBool(const OString &rValue)
{
    return (!rValue.isEmpty() && (rValue[0] == 't' || rValue[0] == 'T' || rValue[0] == '1'));
}

namespace
{
    OUString mapStockToImageResource(const OUString& sType)
    {
        if (sType == "gtk-index")
            return OUString(SV_RESID_BITMAP_INDEX);
        else if (sType == "gtk-refresh")
            return OUString(SV_RESID_BITMAP_REFRESH);
        else if (sType == "gtk-apply")
            return OUString(IMG_APPLY);
        else if (sType == "gtk-dialog-error")
            return OUString(IMG_ERROR);
        return OUString();
    }

    SymbolType mapStockToSymbol(const OUString& sType)
    {
        SymbolType eRet = SymbolType::DONTKNOW;
        if (sType == "gtk-media-next")
            eRet = SymbolType::NEXT;
        else if (sType == "gtk-media-previous")
            eRet = SymbolType::PREV;
        else if (sType == "gtk-media-play")
            eRet = SymbolType::PLAY;
        else if (sType == "gtk-media-stop")
            eRet = SymbolType::STOP;
        else if (sType == "gtk-goto-first")
            eRet = SymbolType::FIRST;
        else if (sType == "gtk-goto-last")
            eRet = SymbolType::LAST;
        else if (sType == "gtk-go-back")
            eRet = SymbolType::ARROW_LEFT;
        else if (sType == "gtk-go-forward")
            eRet = SymbolType::ARROW_RIGHT;
        else if (sType == "gtk-go-up")
            eRet = SymbolType::ARROW_UP;
        else if (sType == "gtk-go-down")
            eRet = SymbolType::ARROW_DOWN;
        else if (sType == "gtk-missing-image")
            eRet = SymbolType::IMAGE;
        else if (sType == "gtk-help")
            eRet = SymbolType::HELP;
        else if (sType == "gtk-close")
            eRet = SymbolType::CLOSE;
        else if (!mapStockToImageResource(sType).isEmpty())
            eRet = SymbolType::IMAGE;
        return eRet;
    }

    void setupFromActionName(Button *pButton, VclBuilder::stringmap &rMap, const css::uno::Reference<css::frame::XFrame>& rFrame);
}

#if defined SAL_LOG_WARN
namespace
{
    bool isButtonType(WindowType nType)
    {
        return nType == WindowType::PUSHBUTTON ||
               nType == WindowType::OKBUTTON ||
               nType == WindowType::CANCELBUTTON ||
               nType == WindowType::HELPBUTTON ||
               nType == WindowType::IMAGEBUTTON ||
               nType == WindowType::MENUBUTTON ||
               nType == WindowType::MOREBUTTON ||
               nType == WindowType::SPINBUTTON;
    }
}
#endif

weld::Builder* Application::CreateBuilder(weld::Widget* pParent, const OUString &rUIFile)
{
    return ImplGetSVData()->mpDefInst->CreateBuilder(pParent, VclBuilderContainer::getUIRootDir(), rUIFile);
}

weld::Builder* Application::CreateInterimBuilder(vcl::Window* pParent, const OUString &rUIFile)
{
    return SalInstance::CreateInterimBuilder(pParent, VclBuilderContainer::getUIRootDir(), rUIFile);
}

weld::MessageDialog* Application::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType,
                                                      VclButtonsType eButtonType, const OUString& rPrimaryMessage)
{
    return ImplGetSVData()->mpDefInst->CreateMessageDialog(pParent, eMessageType, eButtonType, rPrimaryMessage);
}

weld::Window* Application::GetFrameWeld(const css::uno::Reference<css::awt::XWindow>& rWindow)
{
    return ImplGetSVData()->mpDefInst->GetFrameWeld(rWindow);
}

namespace weld
{
    OUString MetricSpinButton::MetricToString(FieldUnit rUnit)
    {
        const FieldUnitStringList& rList = ImplGetFieldUnits();
        // return unit's default string (ie, the first one )
        auto it = std::find_if(
            rList.begin(), rList.end(),
            [&rUnit](const std::pair<OUString, FieldUnit>& rItem) { return rItem.second == rUnit; });
        if (it != rList.end())
            return it->first;

        return OUString();
    }

    IMPL_LINK_NOARG(MetricSpinButton, spin_button_value_changed, SpinButton&, void)
    {
        signal_value_changed();
    }

    IMPL_LINK(MetricSpinButton, spin_button_output, SpinButton&, rSpinButton, void)
    {
        rSpinButton.set_text(format_number(rSpinButton.get_value()));
    }

    void MetricSpinButton::update_width_chars()
    {
        int min, max;
        m_xSpinButton->get_range(min, max);
        auto width = std::max(m_xSpinButton->get_pixel_size(format_number(min)).Width(),
                              m_xSpinButton->get_pixel_size(format_number(max)).Width());
        int chars = ceil(width / m_xSpinButton->get_approximate_digit_width());
        m_xSpinButton->set_width_chars(chars);
    }

    unsigned int SpinButton::Power10(unsigned int n)
    {
        unsigned int nValue = 1;
        for (unsigned int i = 0; i < n; ++i)
            nValue *= 10;
        return nValue;
    }

    int SpinButton::denormalize(int nValue) const
    {
        const int nFactor = Power10(get_digits());

        if ((nValue < (SAL_MIN_INT32 + nFactor)) || (nValue > (SAL_MAX_INT32 - nFactor)))
        {
            return nValue / nFactor;
        }

        const int nHalf = nFactor / 2;

        if (nValue < 0)
            return (nValue - nHalf) / nFactor;
        return (nValue + nHalf) / nFactor;
    }

    OUString MetricSpinButton::format_number(int nValue) const
    {
        OUString aStr;

        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();

        unsigned int nDecimalDigits = m_xSpinButton->get_digits();
        //pawn percent off to icu to decide whether percent is separated from its number for this locale
        if (m_eSrcUnit == FieldUnit::PERCENT)
        {
            double fValue = nValue;
            fValue /= SpinButton::Power10(nDecimalDigits);
            aStr = unicode::formatPercent(fValue, rLocaleData.getLanguageTag());
        }
        else
        {
            aStr = rLocaleData.getNum(nValue, nDecimalDigits, true, true);
            if (m_eSrcUnit != FieldUnit::NONE && m_eSrcUnit != FieldUnit::DEGREE)
                aStr += " ";
            assert(m_eSrcUnit != FieldUnit::PERCENT);
            aStr += MetricToString(m_eSrcUnit);
        }

        return aStr;
    }

    void MetricSpinButton::set_digits(unsigned int digits)
    {
        int step, page;
        get_increments(step, page, m_eSrcUnit);
        int value = get_value(m_eSrcUnit);
        m_xSpinButton->set_digits(digits);
        set_increments(step, page, m_eSrcUnit);
        set_value(value, m_eSrcUnit);
        update_width_chars();
    }

    void MetricSpinButton::set_unit(FieldUnit eUnit)
    {
        if (eUnit != m_eSrcUnit)
        {
            int step, page;
            get_increments(step, page, m_eSrcUnit);
            int value = get_value(m_eSrcUnit);
            m_eSrcUnit = eUnit;
            set_increments(step, page, m_eSrcUnit);
            set_value(value, m_eSrcUnit);
            spin_button_output(*m_xSpinButton);
            update_width_chars();
        }
    }

    int MetricSpinButton::ConvertValue(int nValue, FieldUnit eInUnit, FieldUnit eOutUnit) const
    {
        return MetricField::ConvertValue(nValue, 0, m_xSpinButton->get_digits(), eInUnit, eOutUnit);
    }

    IMPL_LINK(MetricSpinButton, spin_button_input, int*, result, bool)
    {
        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        double fResult(0.0);
        bool bRet = MetricFormatter::TextToValue(get_text(), fResult, 0, m_xSpinButton->get_digits(), rLocaleData, m_eSrcUnit);
        if (bRet)
            *result = fResult;
        return bRet;
    }

    IMPL_LINK_NOARG(TimeSpinButton, spin_button_cursor_position, Entry&, void)
    {
        int nStartPos, nEndPos;
        m_xSpinButton->get_selection_bounds(nStartPos, nEndPos);

        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        const int nTimeArea = TimeFormatter::GetTimeArea(m_eFormat, m_xSpinButton->get_text(), nEndPos,
                                                         rLocaleData);

        int nIncrements = 1;

        if (nTimeArea == 1)
            nIncrements = 1000 * 60 * 60;
        else if (nTimeArea == 2)
            nIncrements = 1000 * 60;
        else if (nTimeArea == 3)
            nIncrements = 1000;

        m_xSpinButton->set_increments(nIncrements, nIncrements * 10);
    }

    IMPL_LINK_NOARG(TimeSpinButton, spin_button_value_changed, SpinButton&, void)
    {
        signal_value_changed();
    }

    IMPL_LINK(TimeSpinButton, spin_button_output, SpinButton&, rSpinButton, void)
    {
        int nStartPos, nEndPos;
        rSpinButton.get_selection_bounds(nStartPos, nEndPos);
        rSpinButton.set_text(format_number(rSpinButton.get_value()));
        rSpinButton.set_position(nEndPos);
    }

    IMPL_LINK(TimeSpinButton, spin_button_input, int*, result, bool)
    {
        int nStartPos, nEndPos;
        m_xSpinButton->get_selection_bounds(nStartPos, nEndPos);

        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        tools::Time aResult(0);
        bool bRet = TimeFormatter::TextToTime(m_xSpinButton->get_text(), aResult, m_eFormat, true, rLocaleData);
        if (bRet)
            *result = ConvertValue(aResult);
        return bRet;
    }

    void TimeSpinButton::update_width_chars()
    {
        int min, max;
        m_xSpinButton->get_range(min, max);
        auto width = std::max(m_xSpinButton->get_pixel_size(format_number(min)).Width(),
                              m_xSpinButton->get_pixel_size(format_number(max)).Width());
        int chars = ceil(width / m_xSpinButton->get_approximate_digit_width());
        m_xSpinButton->set_width_chars(chars);
    }

    tools::Time TimeSpinButton::ConvertValue(int nValue)
    {
        tools::Time aTime(0);
        aTime.MakeTimeFromMS(nValue);
        return aTime;
    }

    int TimeSpinButton::ConvertValue(const tools::Time& rTime)
    {
        return rTime.GetMSFromTime();
    }

    OUString TimeSpinButton::format_number(int nValue) const
    {
        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        return TimeFormatter::FormatTime(ConvertValue(nValue), m_eFormat, TimeFormat::Hour24, true, rLocaleData);
    }

    EntryTreeView::EntryTreeView(std::unique_ptr<Entry> xEntry, std::unique_ptr<TreeView> xTreeView)
        : m_xEntry(std::move(xEntry))
        , m_xTreeView(std::move(xTreeView))
    {
        m_xTreeView->connect_changed(LINK(this, EntryTreeView, ClickHdl));
        m_xEntry->connect_changed(LINK(this, EntryTreeView, ModifyHdl));
    }

    IMPL_LINK(EntryTreeView, ClickHdl, weld::TreeView&, rView, void)
    {
        m_xEntry->set_text(rView.get_selected_text());
        m_aChangeHdl.Call(*this);
    }

    IMPL_LINK_NOARG(EntryTreeView, ModifyHdl, weld::Entry&, void)
    {
        m_aChangeHdl.Call(*this);
    }

    void EntryTreeView::set_height_request_by_rows(int nRows)
    {
        int nHeight = nRows == -1 ? -1 : m_xTreeView->get_height_rows(nRows);
        m_xTreeView->set_size_request(m_xTreeView->get_size_request().Width(), nHeight);
    }
}

VclBuilder::VclBuilder(vcl::Window *pParent, const OUString& sUIDir, const OUString& sUIFile, const OString& sID,
                       const css::uno::Reference<css::frame::XFrame>& rFrame, bool bLegacy)
    : m_sID(sID)
    , m_sHelpRoot(OUStringToOString(sUIFile, RTL_TEXTENCODING_UTF8))
    , m_pStringReplace(Translate::GetReadStringHook())
    , m_pParent(pParent)
    , m_bToplevelParentFound(false)
    , m_bLegacy(bLegacy)
    , m_pParserState(new ParserState)
    , m_xFrame(rFrame)
{
    m_bToplevelHasDeferredInit = pParent &&
        ((pParent->IsSystemWindow() && static_cast<SystemWindow*>(pParent)->isDeferredInit()) ||
         (pParent->IsDockingWindow() && static_cast<DockingWindow*>(pParent)->isDeferredInit()));
    m_bToplevelHasDeferredProperties = m_bToplevelHasDeferredInit;

    sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
    if (nIdx != -1)
        m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
    m_sHelpRoot = m_sHelpRoot + OString('/');

    OUString sUri = sUIDir + sUIFile;

    try
    {
        xmlreader::XmlReader reader(sUri);

        handleChild(pParent, reader);
    }
    catch (const css::uno::Exception &rExcept)
    {
        DBG_UNHANDLED_EXCEPTION("vcl.layout", "Unable to read .ui file");
        CrashReporter::AddKeyValue("VclBuilderException", "Unable to read .ui file: " + rExcept.Message);
        throw;
    }

    //Set Mnemonic widgets when everything has been imported
    for (auto const& mnemonicWidget : m_pParserState->m_aMnemonicWidgetMaps)
    {
        FixedText *pOne = get<FixedText>(mnemonicWidget.m_sID);
        vcl::Window *pOther = get<vcl::Window>(mnemonicWidget.m_sValue.toUtf8());
        SAL_WARN_IF(!pOne || !pOther, "vcl", "missing either source " << mnemonicWidget.m_sID
            << " or target " << mnemonicWidget.m_sValue << " member of Mnemonic Widget Mapping");
        if (pOne && pOther)
            pOne->set_mnemonic_widget(pOther);
    }

    //Set a11y relations and role when everything has been imported
    for (auto const& elemAtk : m_pParserState->m_aAtkInfo)
    {
        vcl::Window *pSource = elemAtk.first;
        const stringmap &rMap = elemAtk.second;

        for (auto const& elemMap : rMap)
        {
            const OString &rType = elemMap.first;
            const OUString &rParam = elemMap.second;
            if (rType == "role")
            {
                sal_Int16 role = BuilderUtils::getRoleFromName(rParam.toUtf8());
                if (role != com::sun::star::accessibility::AccessibleRole::UNKNOWN)
                    pSource->SetAccessibleRole(role);
            }
            else
            {
                vcl::Window *pTarget = get<vcl::Window>(rParam.toUtf8());
                SAL_WARN_IF(!pTarget, "vcl", "missing parameter of a11y relation: " << rParam);
                if (!pTarget)
                    continue;
                if (rType == "labelled-by")
                    pSource->SetAccessibleRelationLabeledBy(pTarget);
                else if (rType == "label-for")
                    pSource->SetAccessibleRelationLabelFor(pTarget);
                else if (rType == "member-of")
                    pSource->SetAccessibleRelationMemberOf(pTarget);
                else
                {
                    SAL_WARN("vcl.layout", "unhandled a11y relation :" << rType);
                }
            }
        }
    }

    //Set radiobutton groups when everything has been imported
    for (auto const& elem : m_pParserState->m_aGroupMaps)
    {
        RadioButton *pOne = get<RadioButton>(elem.m_sID);
        RadioButton *pOther = get<RadioButton>(elem.m_sValue);
        SAL_WARN_IF(!pOne || !pOther, "vcl", "missing member of radiobutton group");
        if (pOne && pOther)
        {
            if (m_bLegacy)
                pOne->group(*pOther);
            else
            {
                pOther->group(*pOne);
                std::stable_sort(pOther->m_xGroup->begin(), pOther->m_xGroup->end(), sortIntoBestTabTraversalOrder(this));
            }
        }
    }

    //Set ComboBox models when everything has been imported
    for (auto const& elem : m_pParserState->m_aModelMaps)
    {
        vcl::Window* pTarget = get<vcl::Window>(elem.m_sID);
        ListBox *pListBoxTarget = dynamic_cast<ListBox*>(pTarget);
        ComboBox *pComboBoxTarget = dynamic_cast<ComboBox*>(pTarget);
        SvTabListBox *pTreeBoxTarget = dynamic_cast<SvTabListBox*>(pTarget);
        // pStore may be empty
        const ListStore *pStore = get_model_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pListBoxTarget && !pComboBoxTarget && !pTreeBoxTarget, "vcl", "missing elements of combobox");
        if (pListBoxTarget && pStore)
            mungeModel(*pListBoxTarget, *pStore, elem.m_nActiveId);
        else if (pComboBoxTarget && pStore)
            mungeModel(*pComboBoxTarget, *pStore, elem.m_nActiveId);
        else if (pTreeBoxTarget && pStore)
            mungeModel(*pTreeBoxTarget, *pStore, elem.m_nActiveId);
    }

    //Set TextView buffers when everything has been imported
    for (auto const& elem : m_pParserState->m_aTextBufferMaps)
    {
        VclMultiLineEdit *pTarget = get<VclMultiLineEdit>(elem.m_sID);
        const TextBuffer *pBuffer = get_buffer_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget || !pBuffer, "vcl", "missing elements of textview/textbuffer");
        if (pTarget && pBuffer)
            mungeTextBuffer(*pTarget, *pBuffer);
    }

    //Set SpinButton adjustments when everything has been imported
    for (auto const& elem : m_pParserState->m_aNumericFormatterAdjustmentMaps)
    {
        NumericFormatter *pTarget = dynamic_cast<NumericFormatter*>(get<vcl::Window>(elem.m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget, "vcl", "missing NumericFormatter element of spinbutton/adjustment");
        SAL_WARN_IF(!pAdjustment, "vcl", "missing Adjustment element of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (auto const& elem : m_pParserState->m_aFormattedFormatterAdjustmentMaps)
    {
        FormattedField *pTarget = dynamic_cast<FormattedField*>(get<vcl::Window>(elem.m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget, "vcl", "missing FormattedField element of spinbutton/adjustment");
        SAL_WARN_IF(!pAdjustment, "vcl", "missing Adjustment element of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (auto const& elem : m_pParserState->m_aTimeFormatterAdjustmentMaps)
    {
        TimeField *pTarget = dynamic_cast<TimeField*>(get<vcl::Window>(elem.m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (auto const& elem : m_pParserState->m_aDateFormatterAdjustmentMaps)
    {
        DateField *pTarget = dynamic_cast<DateField*>(get<vcl::Window>(elem.m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    //Set ScrollBar adjustments when everything has been imported
    for (auto const& elem : m_pParserState->m_aScrollAdjustmentMaps)
    {
        ScrollBar *pTarget = get<ScrollBar>(elem.m_sID);
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of scrollbar/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    //Set Scale(Slider) adjustments
    for (auto const& elem : m_pParserState->m_aSliderAdjustmentMaps)
    {
        Slider* pTarget = dynamic_cast<Slider*>(get<vcl::Window>(elem.m_sID));
        const Adjustment* pAdjustment = get_adjustment_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of scale(slider)/adjustment");
        if (pTarget && pAdjustment)
        {
            mungeAdjustment(*pTarget, *pAdjustment);
        }
    }

    //Set size-groups when all widgets have been imported
    for (auto const& sizeGroup : m_pParserState->m_aSizeGroups)
    {
        std::shared_ptr<VclSizeGroup> xGroup(std::make_shared<VclSizeGroup>());

        for (auto const& elem : sizeGroup.m_aProperties)
        {
            const OString &rKey = elem.first;
            const OUString &rValue = elem.second;
            xGroup->set_property(rKey, rValue);
        }

        for (auto const& elem : sizeGroup.m_aWidgets)
        {
            vcl::Window* pWindow = get<vcl::Window>(elem.getStr());
            pWindow->add_to_size_group(xGroup);
        }
    }

    //Set button images when everything has been imported
    std::set<OUString> aImagesToBeRemoved;
    for (auto const& elem : m_pParserState->m_aButtonImageWidgetMaps)
    {
        PushButton *pTargetButton = nullptr;
        RadioButton *pTargetRadio = nullptr;
        Button *pTarget = nullptr;

        if (!elem.m_bRadio)
        {
            pTargetButton = get<PushButton>(elem.m_sID);
            pTarget = pTargetButton;
        }
        else
        {
            pTargetRadio = get<RadioButton>(elem.m_sID);
            pTarget = pTargetRadio;
        }

        FixedImage *pImage = get<FixedImage>(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget || !pImage,
            "vcl", "missing elements of button/image/stock");
        if (!pTarget || !pImage)
            continue;
        aImagesToBeRemoved.insert(elem.m_sValue);

        VclBuilder::StockMap::iterator aFind = m_pParserState->m_aStockMap.find(elem.m_sValue.toUtf8());
        if (aFind == m_pParserState->m_aStockMap.end())
        {
            if (!elem.m_bRadio)
                pTargetButton->SetModeImage(pImage->GetImage());
            else
                pTargetRadio->SetModeRadioImage(pImage->GetImage());
        }
        else
        {
            const stockinfo &rImageInfo = aFind->second;
            SymbolType eType = mapStockToSymbol(rImageInfo.m_sStock);
            SAL_WARN_IF(eType == SymbolType::DONTKNOW, "vcl", "missing stock image element for button");
            if (eType == SymbolType::DONTKNOW)
                continue;
            if (!elem.m_bRadio)
            {
                pTargetButton->SetSymbol(eType);
                //fdo#76457 keep symbol images small e.g. tools->customize->menu
                //but images the right size. Really the PushButton::CalcMinimumSize
                //and PushButton::ImplDrawPushButton are the better place to handle
                //this, but its such a train-wreck
                if (eType != SymbolType::IMAGE)
                    pTargetButton->SetStyle(pTargetButton->GetStyle() | WB_SMALLSTYLE);
            }
            else
                SAL_WARN_IF(eType != SymbolType::IMAGE, "vcl.layout", "unimplemented symbol type for radiobuttons");
            if (eType == SymbolType::IMAGE)
            {
                Image const aImage(StockImage::Yes,
                                   mapStockToImageResource(rImageInfo.m_sStock));
                if (!elem.m_bRadio)
                    pTargetButton->SetModeImage(aImage);
                else
                    pTargetRadio->SetModeRadioImage(aImage);
            }
            switch (rImageInfo.m_nSize)
            {
                case 1:
                    pTarget->SetSmallSymbol();
                    break;
                case 3:
                    // large toolbar, make bigger than normal (4)
                    pTarget->set_width_request(pTarget->GetOptimalSize().Width() * 1.5);
                    pTarget->set_height_request(pTarget->GetOptimalSize().Height() * 1.5);
                    break;
                case 4:
                    break;
                default:
                    SAL_WARN("vcl.layout", "unsupported image size " << rImageInfo.m_nSize);
                    break;
            }
        }
    }

    //There may be duplicate use of an Image, so we used a set to collect and
    //now we can remove them from the tree after their final munge
    for (auto const& elem : aImagesToBeRemoved)
    {
        delete_by_name(elem.toUtf8());
    }

    //fill in any stock icons in surviving images
    for (auto const& elem : m_pParserState->m_aStockMap)
    {
        FixedImage *pImage = get<FixedImage>(elem.first);
        SAL_WARN_IF(!pImage, "vcl", "missing elements of image/stock");
        if (!pImage)
            continue;

        const stockinfo &rImageInfo = elem.second;
        if (rImageInfo.m_sStock == "gtk-missing-image")
            continue;

        SymbolType eType = mapStockToSymbol(rImageInfo.m_sStock);
        SAL_WARN_IF(eType != SymbolType::IMAGE, "vcl", "unimplemented symbol type for images");
        if (eType != SymbolType::IMAGE)
            continue;

        Image const aImage(StockImage::Yes,
                           mapStockToImageResource(rImageInfo.m_sStock));
        pImage->SetImage(aImage);
    }

    //Set button menus when everything has been imported
    for (auto const& elem : m_pParserState->m_aButtonMenuMaps)
    {
        MenuButton *pTarget = get<MenuButton>(elem.m_sID);
        PopupMenu *pMenu = get_menu(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget || !pMenu,
            "vcl", "missing elements of button/menu");
        if (!pTarget || !pMenu)
            continue;
        pTarget->SetPopupMenu(pMenu);
    }

    //Remove ScrollWindow parent widgets whose children in vcl implement scrolling
    //internally.
    for (auto const& elem : m_pParserState->m_aRedundantParentWidgets)
    {
        delete_by_window(elem.first);
    }

    //fdo#67378 merge the label into the disclosure button
    for (auto const& elem : m_pParserState->m_aExpanderWidgets)
    {
        vcl::Window *pChild = elem->get_child();
        vcl::Window* pLabel = elem->GetWindow(GetWindowType::LastChild);
        if (pLabel && pLabel != pChild && pLabel->GetType() == WindowType::FIXEDTEXT)
        {
            FixedText *pLabelWidget = static_cast<FixedText*>(pLabel);
            elem->set_label(pLabelWidget->GetText());
            delete_by_window(pLabel);
        }
    }

    // create message dialog message area now
    for (auto const& elem : m_pParserState->m_aMessageDialogs)
        elem->create_message_area();

    //drop maps, etc. that we don't need again
    m_pParserState.reset();

    SAL_WARN_IF(!m_sID.isEmpty() && (!m_bToplevelParentFound && !get_by_name(m_sID)), "vcl.layout",
        "Requested top level widget \"" << m_sID << "\" not found in " << sUIFile);

#if defined SAL_LOG_WARN
    if (m_bToplevelParentFound && m_pParent->IsDialog())
    {
        int nButtons = 0;
        bool bHasDefButton = false;
        for (auto const& child : m_aChildren)
        {
            if (isButtonType(child.m_pWindow->GetType()))
            {
                ++nButtons;
                if (child.m_pWindow->GetStyle() & WB_DEFBUTTON)
                {
                    bHasDefButton = true;
                    break;
                }
            }
        }
        SAL_WARN_IF(nButtons && !bHasDefButton, "vcl.layout", "No default button defined in " << sUIFile);
    }
#endif
}

VclBuilder::~VclBuilder()
{
    disposeBuilder();
}

void VclBuilder::disposeBuilder()
{
    for (std::vector<WinAndId>::reverse_iterator aI = m_aChildren.rbegin(),
         aEnd = m_aChildren.rend(); aI != aEnd; ++aI)
    {
        aI->m_pWindow.disposeAndClear();
    }
    m_aChildren.clear();

    for (std::vector<MenuAndId>::reverse_iterator aI = m_aMenus.rbegin(),
         aEnd = m_aMenus.rend(); aI != aEnd; ++aI)
    {
        aI->m_pMenu.disposeAndClear();
    }
    m_aMenus.clear();
    m_pParent.clear();
}

namespace
{
    bool extractDrawValue(VclBuilder::stringmap& rMap)
    {
        bool bDrawValue = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("draw_value"));
        if (aFind != rMap.end())
        {
            bDrawValue = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bDrawValue;
    }

    OUString extractPopupMenu(VclBuilder::stringmap& rMap)
    {
        OUString sRet;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("popup"));
        if (aFind != rMap.end())
        {
            sRet = aFind->second;
            rMap.erase(aFind);
        }
        return sRet;
    }

    OUString extractValuePos(VclBuilder::stringmap& rMap)
    {
        OUString sRet("top");
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("value_pos"));
        if (aFind != rMap.end())
        {
            sRet = aFind->second;
            rMap.erase(aFind);
        }
        return sRet;
    }

    OUString extractTypeHint(VclBuilder::stringmap &rMap)
    {
        OUString sRet("normal");
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("type-hint"));
        if (aFind != rMap.end())
        {
            sRet = aFind->second;
            rMap.erase(aFind);
        }
        return sRet;
    }

    bool extractResizable(VclBuilder::stringmap &rMap)
    {
        bool bResizable = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("resizable"));
        if (aFind != rMap.end())
        {
            bResizable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bResizable;
    }

#if HAVE_FEATURE_DESKTOP
    bool extractModal(VclBuilder::stringmap &rMap)
    {
        bool bModal = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("modal"));
        if (aFind != rMap.end())
        {
            bModal = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bModal;
    }
#endif

    bool extractDecorated(VclBuilder::stringmap &rMap)
    {
        bool bDecorated = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("decorated"));
        if (aFind != rMap.end())
        {
            bDecorated = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bDecorated;
    }

    bool extractCloseable(VclBuilder::stringmap &rMap)
    {
        bool bCloseable = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("deletable"));
        if (aFind != rMap.end())
        {
            bCloseable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bCloseable;
    }

    bool extractEntry(VclBuilder::stringmap &rMap)
    {
        bool bHasEntry = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("has-entry"));
        if (aFind != rMap.end())
        {
            bHasEntry = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bHasEntry;
    }

    bool extractOrientation(VclBuilder::stringmap &rMap)
    {
        bool bVertical = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("orientation"));
        if (aFind != rMap.end())
        {
            bVertical = aFind->second.equalsIgnoreAsciiCase("vertical");
            rMap.erase(aFind);
        }
        return bVertical;
    }

    bool extractInconsistent(VclBuilder::stringmap &rMap)
    {
        bool bInconsistent = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("inconsistent"));
        if (aFind != rMap.end())
        {
            bInconsistent = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bInconsistent;
    }

    OUString extractIconName(VclBuilder::stringmap &rMap)
    {
        OUString sIconName;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("icon-name"));
        if (aFind != rMap.end())
        {
            sIconName = aFind->second;
            rMap.erase(aFind);
        }
        return sIconName;
    }

    OUString getStockText(const OUString &rType)
    {
        if (rType == "gtk-ok")
            return VclResId(SV_BUTTONTEXT_OK);
        else if (rType == "gtk-cancel")
            return VclResId(SV_BUTTONTEXT_CANCEL);
        else if (rType == "gtk-help")
            return VclResId(SV_BUTTONTEXT_HELP);
        else if (rType == "gtk-close")
            return VclResId(SV_BUTTONTEXT_CLOSE);
        else if (rType == "gtk-revert-to-saved")
            return VclResId(SV_BUTTONTEXT_RESET);
        else if (rType == "gtk-add")
            return VclResId(SV_BUTTONTEXT_ADD);
        else if (rType == "gtk-delete")
            return VclResId(SV_BUTTONTEXT_DELETE);
        else if (rType == "gtk-remove")
            return VclResId(SV_BUTTONTEXT_REMOVE);
        else if (rType == "gtk-new")
            return VclResId(SV_BUTTONTEXT_NEW);
        else if (rType == "gtk-edit")
            return VclResId(SV_BUTTONTEXT_EDIT);
        else if (rType == "gtk-apply")
            return VclResId(SV_BUTTONTEXT_APPLY);
        else if (rType == "gtk-save")
            return VclResId(SV_BUTTONTEXT_SAVE);
        else if (rType == "gtk-open")
            return VclResId(SV_BUTTONTEXT_OPEN);
        else if (rType == "gtk-undo")
            return VclResId(SV_BUTTONTEXT_UNDO);
        else if (rType == "gtk-paste")
            return VclResId(SV_BUTTONTEXT_PASTE);
        else if (rType == "gtk-media-next")
            return VclResId(SV_BUTTONTEXT_NEXT);
        else if (rType == "gtk-media-previous")
            return VclResId(SV_BUTTONTEXT_PREV);
        else if (rType == "gtk-go-up")
            return VclResId(SV_BUTTONTEXT_GO_UP);
        else if (rType == "gtk-go-down")
            return VclResId(SV_BUTTONTEXT_GO_DOWN);
        else if (rType == "gtk-clear")
            return VclResId(SV_BUTTONTEXT_CLEAR);
        else if (rType == "gtk-media-play")
            return VclResId(SV_BUTTONTEXT_PLAY);
        else if (rType == "gtk-find")
            return VclResId(SV_BUTTONTEXT_FIND);
        else if (rType == "gtk-stop")
            return VclResId(SV_BUTTONTEXT_STOP);
        else if (rType == "gtk-connect")
            return VclResId(SV_BUTTONTEXT_CONNECT);
        else if (rType == "gtk-yes")
            return VclResId(SV_BUTTONTEXT_YES);
        else if (rType == "gtk-no")
            return VclResId(SV_BUTTONTEXT_NO);
        SAL_WARN("vcl.layout", "unknown stock type: " << rType);
        return OUString();
    }

    bool extractStock(VclBuilder::stringmap &rMap)
    {
        bool bIsStock = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("use-stock"));
        if (aFind != rMap.end())
        {
            bIsStock = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bIsStock;
    }

    WinBits extractRelief(VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_3DLOOK;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("relief"));
        if (aFind != rMap.end())
        {
            if (aFind->second == "half")
                nBits = WB_FLATBUTTON | WB_BEVELBUTTON;
            else if (aFind->second == "none")
                nBits = WB_FLATBUTTON;
            rMap.erase(aFind);
        }
        return nBits;
    }

    OUString extractLabel(VclBuilder::stringmap &rMap)
    {
        OUString sType;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("label"));
        if (aFind != rMap.end())
        {
            sType = aFind->second;
            rMap.erase(aFind);
        }
        return sType;
    }

    OUString extractActionName(VclBuilder::stringmap &rMap)
    {
        OUString sActionName;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("action-name"));
        if (aFind != rMap.end())
        {
            sActionName = aFind->second;
            rMap.erase(aFind);
        }
        return sActionName;
    }

    bool extractVisible(VclBuilder::stringmap &rMap)
    {
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("visible"));
        if (aFind != rMap.end())
        {
            return toBool(aFind->second);
        }
        return false;
    }

    Size extractSizeRequest(VclBuilder::stringmap &rMap)
    {
        OUString sWidthRequest("0");
        OUString sHeightRequest("0");
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("width-request"));
        if (aFind != rMap.end())
        {
            sWidthRequest = aFind->second;
            rMap.erase(aFind);
        }
        aFind = rMap.find(OString("height-request"));
        if (aFind != rMap.end())
        {
            sHeightRequest = aFind->second;
            rMap.erase(aFind);
        }
        return Size(sWidthRequest.toInt32(), sHeightRequest.toInt32());
    }

    OUString extractTooltipText(VclBuilder::stringmap &rMap)
    {
        OUString sTooltipText;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("tooltip-text"));
        if (aFind == rMap.end())
            aFind = rMap.find(OString("tooltip-markup"));
        if (aFind != rMap.end())
        {
            sTooltipText = aFind->second;
            rMap.erase(aFind);
        }
        return sTooltipText;
    }

    float extractAlignment(VclBuilder::stringmap &rMap)
    {
        float f = 0.0;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("alignment"));
        if (aFind != rMap.end())
        {
            f = aFind->second.toFloat();
            rMap.erase(aFind);
        }
        return f;
    }

    OUString extractTitle(VclBuilder::stringmap &rMap)
    {
        OUString sTitle;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("title"));
        if (aFind != rMap.end())
        {
            sTitle = aFind->second;
            rMap.erase(aFind);
        }
        return sTitle;
    }

    bool extractHeadersVisible(VclBuilder::stringmap &rMap)
    {
        bool bHeadersVisible = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("headers-visible"));
        if (aFind != rMap.end())
        {
            bHeadersVisible = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bHeadersVisible;
    }

    bool extractSortIndicator(VclBuilder::stringmap &rMap)
    {
        bool bSortIndicator = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("sort-indicator"));
        if (aFind != rMap.end())
        {
            bSortIndicator = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bSortIndicator;
    }

    bool extractClickable(VclBuilder::stringmap &rMap)
    {
        bool bClickable = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("clickable"));
        if (aFind != rMap.end())
        {
            bClickable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bClickable;
    }

    void setupFromActionName(Button *pButton, VclBuilder::stringmap &rMap, const css::uno::Reference<css::frame::XFrame>& rFrame)
    {
        if (!rFrame.is())
            return;

        OUString aCommand(extractActionName(rMap));
        if (aCommand.isEmpty())
            return;

        OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(rFrame));
        OUString aLabel(vcl::CommandInfoProvider::GetLabelForCommand(aCommand, aModuleName));
        if (!aLabel.isEmpty())
            pButton->SetText(aLabel);

        OUString aTooltip(vcl::CommandInfoProvider::GetTooltipForCommand(aCommand, rFrame));
        if (!aTooltip.isEmpty())
            pButton->SetQuickHelpText(aTooltip);

        Image aImage(vcl::CommandInfoProvider::GetImageForCommand(aCommand, rFrame));
        pButton->SetModeImage(aImage);

        pButton->SetCommandHandler(aCommand);
    }

    VclPtr<Button> extractStockAndBuildPushButton(vcl::Window *pParent, VclBuilder::stringmap &rMap, bool bToggle, bool bLegacy)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER;
        if (bToggle)
            nBits |= WB_TOGGLE;

        nBits |= extractRelief(rMap);

        VclPtr<Button> xWindow;

        if (extractStock(rMap))
        {
            OUString sType = extractLabel(rMap);
            if (bLegacy)
            {
                if (sType == "gtk-ok")
                    xWindow = VclPtr<OKButton>::Create(pParent, nBits);
                else if (sType == "gtk-cancel")
                    xWindow = VclPtr<CancelButton>::Create(pParent, nBits);
                else if (sType == "gtk-close")
                    xWindow = VclPtr<CloseButton>::Create(pParent, nBits);
                else if (sType == "gtk-help")
                    xWindow = VclPtr<HelpButton>::Create(pParent, nBits);
            }
            if (!xWindow)
            {
                xWindow = VclPtr<PushButton>::Create(pParent, nBits);
                xWindow->SetText(getStockText(sType));
            }
        }

        if (!xWindow)
            xWindow = VclPtr<PushButton>::Create(pParent, nBits);
        return xWindow;
    }

    VclPtr<MenuButton> extractStockAndBuildMenuButton(vcl::Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;

        nBits |= extractRelief(rMap);

        VclPtr<MenuButton> xWindow = VclPtr<MenuButton>::Create(pParent, nBits);

        if (extractStock(rMap))
        {
            xWindow->SetText(getStockText(extractLabel(rMap)));
        }

        return xWindow;
    }

    VclPtr<Button> extractStockAndBuildMenuToggleButton(vcl::Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;

        nBits |= extractRelief(rMap);

        VclPtr<Button> xWindow = VclPtr<MenuToggleButton>::Create(pParent, nBits);

        if (extractStock(rMap))
        {
            xWindow->SetText(getStockText(extractLabel(rMap)));
        }

        return xWindow;
    }

    OUString extractUnit(const OUString& sPattern)
    {
        OUString sUnit(sPattern);
        for (sal_Int32 i = 0; i < sPattern.getLength(); ++i)
        {
            if (sPattern[i] != '.' && sPattern[i] != ',' && sPattern[i] != '0')
            {
                sUnit = sPattern.copy(i);
                break;
            }
        }
        return sUnit;
    }

    int extractDecimalDigits(const OUString& sPattern)
    {
        int nDigits = 0;
        bool bAfterPoint = false;
        for (sal_Int32 i = 0; i < sPattern.getLength(); ++i)
        {
            if (sPattern[i] == '.' || sPattern[i] == ',')
                bAfterPoint = true;
            else if (sPattern[i] == '0')
            {
                if (bAfterPoint)
                    ++nDigits;
            }
            else
                break;
        }
        return nDigits;
    }

    FieldUnit detectMetricUnit(const OUString& sUnit)
    {
        FieldUnit eUnit = FieldUnit::NONE;

        if (sUnit == "mm")
            eUnit = FieldUnit::MM;
        else if (sUnit == "cm")
            eUnit = FieldUnit::CM;
        else if (sUnit == "m")
            eUnit = FieldUnit::M;
        else if (sUnit == "km")
            eUnit = FieldUnit::KM;
        else if ((sUnit == "twips") || (sUnit == "twip"))
            eUnit = FieldUnit::TWIP;
        else if (sUnit == "pt")
            eUnit = FieldUnit::POINT;
        else if (sUnit == "pc")
            eUnit = FieldUnit::PICA;
        else if (sUnit == "\"" || (sUnit == "in") || (sUnit == "inch"))
            eUnit = FieldUnit::INCH;
        else if ((sUnit == "'") || (sUnit == "ft") || (sUnit == "foot") || (sUnit == "feet"))
            eUnit = FieldUnit::FOOT;
        else if (sUnit == "mile" || (sUnit == "miles"))
            eUnit = FieldUnit::MILE;
        else if (sUnit == "ch")
            eUnit = FieldUnit::CHAR;
        else if (sUnit == "line")
            eUnit = FieldUnit::LINE;
        else if (sUnit == "%")
            eUnit = FieldUnit::PERCENT;
        else if ((sUnit == "pixels") || (sUnit == "pixel") || (sUnit == "px"))
            eUnit = FieldUnit::PIXEL;
        else if ((sUnit == "degrees") || (sUnit == "degree"))
            eUnit = FieldUnit::DEGREE;
        else if ((sUnit == "sec") || (sUnit == "seconds") || (sUnit == "second"))
            eUnit = FieldUnit::SECOND;
        else if ((sUnit == "ms") || (sUnit == "milliseconds") || (sUnit == "millisecond"))
            eUnit = FieldUnit::MILLISECOND;
        else if (sUnit != "0")
            eUnit = FieldUnit::CUSTOM;

        return eUnit;
    }

    WinBits extractDeferredBits(VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_3DLOOK|WB_HIDE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        if (extractCloseable(rMap))
            nBits |= WB_CLOSEABLE;
        OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
        if (!sBorder.isEmpty())
            nBits |= WB_BORDER;
        if (!extractDecorated(rMap))
            nBits |= WB_OWNERDRAWDECORATION;
        OUString sType(extractTypeHint(rMap));
        if (sType == "utility")
            nBits |= WB_SYSTEMWINDOW | WB_DIALOGCONTROL | WB_MOVEABLE;
        else if (sType == "popup-menu")
            nBits |= WB_SYSTEMWINDOW | WB_DIALOGCONTROL | WB_POPUP;
        else if (sType == "dock")
            nBits |= WB_DOCKABLE | WB_MOVEABLE;
        else
            nBits |= WB_MOVEABLE;
        return nBits;
    }
}

void VclBuilder::extractGroup(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("group"));
    if (aFind != rMap.end())
    {
        OUString sID = aFind->second;
        sal_Int32 nDelim = sID.indexOf(':');
        if (nDelim != -1)
            sID = sID.copy(0, nDelim);
        m_pParserState->m_aGroupMaps.emplace_back(id, sID.toUtf8());
        rMap.erase(aFind);
    }
}

void VclBuilder::connectNumericFormatterAdjustment(const OString &id, const OUString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pParserState->m_aNumericFormatterAdjustmentMaps.emplace_back(id, rAdjustment);
}

void VclBuilder::connectFormattedFormatterAdjustment(const OString &id, const OUString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pParserState->m_aFormattedFormatterAdjustmentMaps.emplace_back(id, rAdjustment);
}

void VclBuilder::connectTimeFormatterAdjustment(const OString &id, const OUString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pParserState->m_aTimeFormatterAdjustmentMaps.emplace_back(id, rAdjustment);
}

void VclBuilder::connectDateFormatterAdjustment(const OString &id, const OUString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pParserState->m_aDateFormatterAdjustmentMaps.emplace_back(id, rAdjustment);
}

bool VclBuilder::extractAdjustmentToMap(const OString& id, VclBuilder::stringmap& rMap, std::vector<WidgetAdjustmentMap>& rAdjustmentMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("adjustment"));
    if (aFind != rMap.end())
    {
        rAdjustmentMap.emplace_back(id, aFind->second);
        rMap.erase(aFind);
        return true;
    }
    return false;
}

namespace
{
    sal_Int32 extractActive(VclBuilder::stringmap &rMap)
    {
        sal_Int32 nActiveId = 0;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("active"));
        if (aFind != rMap.end())
        {
            nActiveId = aFind->second.toInt32();
            rMap.erase(aFind);
        }
        return nActiveId;
    }

    bool extractSelectable(VclBuilder::stringmap &rMap)
    {
        bool bSelectable = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("selectable"));
        if (aFind != rMap.end())
        {
            bSelectable = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bSelectable;
    }

    OUString extractAdjustment(VclBuilder::stringmap &rMap)
    {
        OUString sAdjustment;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("adjustment"));
        if (aFind != rMap.end())
        {
            sAdjustment= aFind->second;
            rMap.erase(aFind);
            return sAdjustment;
        }
        return sAdjustment;
    }

    bool extractDrawIndicator(VclBuilder::stringmap &rMap)
    {
        bool bDrawIndicator = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("draw-indicator"));
        if (aFind != rMap.end())
        {
            bDrawIndicator = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bDrawIndicator;
    }
}

void VclBuilder::extractModel(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("model"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aModelMaps.emplace_back(id, aFind->second,
            extractActive(rMap));
        rMap.erase(aFind);
    }
}

void VclBuilder::extractBuffer(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("buffer"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aTextBufferMaps.emplace_back(id, aFind->second);
        rMap.erase(aFind);
    }
}

void VclBuilder::extractStock(const OString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("stock"));
    if (aFind != rMap.end())
    {
        stockinfo aInfo;
        aInfo.m_sStock = aFind->second;
        rMap.erase(aFind);
        aFind = rMap.find(OString("icon-size"));
        if (aFind != rMap.end())
        {
            aInfo.m_nSize = aFind->second.toInt32();
            rMap.erase(aFind);
        }
        m_pParserState->m_aStockMap[id] = aInfo;
    }
}

void VclBuilder::extractButtonImage(const OString &id, stringmap &rMap, bool bRadio)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("image"));
    if (aFind != rMap.end())
    {
        m_pParserState->m_aButtonImageWidgetMaps.emplace_back(id, aFind->second, bRadio);
        rMap.erase(aFind);
    }
}

void VclBuilder::extractMnemonicWidget(const OString &rLabelID, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(OString("mnemonic-widget"));
    if (aFind != rMap.end())
    {
        OUString sID = aFind->second;
        sal_Int32 nDelim = sID.indexOf(':');
        if (nDelim != -1)
            sID = sID.copy(0, nDelim);
        m_pParserState->m_aMnemonicWidgetMaps.emplace_back(rLabelID, sID);
        rMap.erase(aFind);
    }
}

vcl::Window* VclBuilder::prepareWidgetOwnScrolling(vcl::Window *pParent, WinBits &rWinStyle)
{
    //For Widgets that manage their own scrolling, if one appears as a child of
    //a scrolling window shoehorn that scrolling settings to this widget and
    //return the real parent to use
    if (pParent && pParent->GetType() == WindowType::SCROLLWINDOW)
    {
        WinBits nScrollBits = pParent->GetStyle();
        nScrollBits &= (WB_AUTOHSCROLL|WB_HSCROLL|WB_AUTOVSCROLL|WB_VSCROLL);
        rWinStyle |= nScrollBits | WB_BORDER;
        pParent = pParent->GetParent();
    }

    return pParent;
}

void VclBuilder::cleanupWidgetOwnScrolling(vcl::Window *pScrollParent, vcl::Window *pWindow, stringmap &rMap)
{
    //remove the redundant scrolling parent
    sal_Int32 nWidthReq = pScrollParent->get_width_request();
    rMap[OString("width-request")] = OUString::number(nWidthReq);
    sal_Int32 nHeightReq = pScrollParent->get_height_request();
    rMap[OString("height-request")] = OUString::number(nHeightReq);

    m_pParserState->m_aRedundantParentWidgets[pScrollParent] = pWindow;
}

#ifndef DISABLE_DYNLOADING

extern "C" { static void thisModule() {} }

// Don't unload the module on destruction
class NoAutoUnloadModule : public osl::Module
{
public:
    ~NoAutoUnloadModule() { release(); }
};

typedef std::map<OUString, std::shared_ptr<NoAutoUnloadModule>> ModuleMap;
static ModuleMap g_aModuleMap;

#if ENABLE_MERGELIBS
static std::shared_ptr<NoAutoUnloadModule> g_pMergedLib = std::make_shared<NoAutoUnloadModule>();
#endif

#ifndef SAL_DLLPREFIX
#  define SAL_DLLPREFIX ""
#endif

#endif

void VclBuilder::preload()
{
#ifndef DISABLE_DYNLOADING

#if ENABLE_MERGELIBS
    g_pMergedLib->loadRelative(&thisModule, SVLIBRARY("merged"));
#else
// find -name '*ui*' | xargs grep 'class=".*lo-' |
//     sed 's/.*class="//' | sed 's/-.*$//' | sort | uniq
    static const char *aWidgetLibs[] = {
        "sfxlo",  "svtlo",     "svxcorelo", "foruilo",
        "vcllo",  "svxlo",     "cuilo",     "swlo",
        "swuilo", "sclo",      "sdlo",      "chartcontrollerlo",
        "smlo",   "scuilo",    "basctllo",  "sduilo",
        "scnlo",  "xsltdlglo", "pcrlo" // "dbulo"
    };
    for (auto & lib : aWidgetLibs)
    {
        std::unique_ptr<NoAutoUnloadModule> pModule(new NoAutoUnloadModule);
        OUString sModule = SAL_DLLPREFIX + OUString::createFromAscii(lib) + SAL_DLLEXTENSION;
        if (pModule->loadRelative(&thisModule, sModule))
            g_aModuleMap.insert(std::make_pair(sModule, std::move(pModule)));
    }
#endif // ENABLE_MERGELIBS
#endif // DISABLE_DYNLOADING
}

#if defined DISABLE_DYNLOADING && !HAVE_FEATURE_DESKTOP
extern "C" VclBuilder::customMakeWidget lo_get_custom_widget_func(const char* name);
#endif

VclPtr<vcl::Window> VclBuilder::makeObject(vcl::Window *pParent, const OString &name, const OString &id,
    stringmap &rMap)
{
    bool bIsPlaceHolder = name.isEmpty();
    bool bVertical = false;

    if (pParent && pParent->GetType() == WindowType::TABCONTROL)
    {
        bool bTopLevel(name == "GtkDialog" || name == "GtkMessageDialog" ||
                       name == "GtkWindow" || name == "GtkPopover");
        if (!bTopLevel)
        {
            //We have to add a page
            //make default pageid == position
            TabControl *pTabControl = static_cast<TabControl*>(pParent);
            sal_uInt16 nNewPageCount = pTabControl->GetPageCount()+1;
            sal_uInt16 nNewPageId = nNewPageCount;
            pTabControl->InsertPage(nNewPageId, OUString());
            pTabControl->SetCurPageId(nNewPageId);
            SAL_WARN_IF(bIsPlaceHolder, "vcl.layout", "we should have no placeholders for tabpages");
            if (!bIsPlaceHolder)
            {
                VclPtrInstance<TabPage> pPage(pTabControl);
                pPage->Show();

                //Make up a name for it
                OString sTabPageId = get_by_window(pParent) +
                    OString("-page") +
                    OString::number(nNewPageCount);
                m_aChildren.emplace_back(sTabPageId, pPage, false);
                pPage->SetHelpId(m_sHelpRoot + sTabPageId);

                pParent = pPage;

                pTabControl->SetTabPage(nNewPageId, pPage);
            }
        }
    }

    if (bIsPlaceHolder || name == "GtkTreeSelection")
        return nullptr;

    extractButtonImage(id, rMap, name == "GtkRadioButton");

    VclPtr<vcl::Window> xWindow;
    if (name == "GtkDialog")
    {
        WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        xWindow = VclPtr<Dialog>::Create(pParent, nBits, !pParent ? Dialog::InitFlag::NoParent : Dialog::InitFlag::Default);
#if HAVE_FEATURE_DESKTOP
        if (!m_bLegacy && !extractModal(rMap))
            xWindow->SetType(WindowType::MODELESSDIALOG);
#endif
    }
    else if (name == "GtkMessageDialog")
    {
        WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_CLOSEABLE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        VclPtr<MessageDialog> xDialog(VclPtr<MessageDialog>::Create(pParent, nBits));
        m_pParserState->m_aMessageDialogs.push_back(xDialog);
        xWindow = xDialog;
#if defined WNT
        xWindow->set_border_width(3);
#else
        xWindow->set_border_width(12);
#endif
    }
    else if (name == "GtkBox")
    {
        bVertical = extractOrientation(rMap);
        if (bVertical)
            xWindow = VclPtr<VclVBox>::Create(pParent);
        else
            xWindow = VclPtr<VclHBox>::Create(pParent);
    }
    else if (name == "GtkPaned")
    {
        bVertical = extractOrientation(rMap);
        assert(bVertical && "hori not implemented, shouldn't be hard though");
        xWindow = VclPtr<VclVPaned>::Create(pParent);
    }
    else if (name == "GtkHBox")
        xWindow = VclPtr<VclHBox>::Create(pParent);
    else if (name == "GtkVBox")
        xWindow = VclPtr<VclVBox>::Create(pParent);
    else if (name == "GtkButtonBox")
    {
        bVertical = extractOrientation(rMap);
        if (bVertical)
            xWindow = VclPtr<VclVButtonBox>::Create(pParent);
        else
            xWindow = VclPtr<VclHButtonBox>::Create(pParent);
    }
    else if (name == "GtkHButtonBox")
        xWindow = VclPtr<VclHButtonBox>::Create(pParent);
    else if (name == "GtkVButtonBox")
        xWindow = VclPtr<VclVButtonBox>::Create(pParent);
    else if (name == "GtkGrid")
        xWindow = VclPtr<VclGrid>::Create(pParent);
    else if (name == "GtkFrame")
        xWindow = VclPtr<VclFrame>::Create(pParent);
    else if (name == "GtkExpander")
    {
        VclPtrInstance<VclExpander> pExpander(pParent);
        m_pParserState->m_aExpanderWidgets.push_back(pExpander);
        xWindow = pExpander;
    }
    else if (name == "GtkAlignment")
        xWindow = VclPtr<VclAlignment>::Create(pParent);
    else if (name == "GtkButton" || (!m_bLegacy && name == "GtkToggleButton"))
    {
        VclPtr<Button> xButton;
        OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
        if (sMenu.isEmpty())
            xButton = extractStockAndBuildPushButton(pParent, rMap, name == "GtkToggleButton", m_bLegacy);
        else
        {
            assert(m_bLegacy && "use GtkMenuButton");
            xButton = extractStockAndBuildMenuButton(pParent, rMap);
            m_pParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
        }
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        setupFromActionName(xButton, rMap, m_xFrame);
        xWindow = xButton;
    }
    else if (name == "GtkMenuButton")
    {
        VclPtr<MenuButton> xButton = extractStockAndBuildMenuButton(pParent, rMap);
        OUString sMenu = extractPopupMenu(rMap);
        if (!sMenu.isEmpty())
            m_pParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        xButton->SetAccessibleRole(css::accessibility::AccessibleRole::BUTTON_MENU);

        if (!extractDrawIndicator(rMap))
            xButton->SetDropDown(PushButtonDropdownStyle::NONE);

        setupFromActionName(xButton, rMap, m_xFrame);
        xWindow = xButton;
    }
    else if (name == "GtkToggleButton" && m_bLegacy)
    {
        VclPtr<Button> xButton;
        OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
        assert(sMenu.getLength() && "not implemented yet");
        xButton = extractStockAndBuildMenuToggleButton(pParent, rMap);
        m_pParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        setupFromActionName(xButton, rMap, m_xFrame);
        xWindow = xButton;
    }
    else if (name == "GtkRadioButton")
    {
        extractGroup(id, rMap);
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;
        OUString sWrap = BuilderUtils::extractCustomProperty(rMap);
        if (!sWrap.isEmpty())
            nBits |= WB_WORDBREAK;
        VclPtr<RadioButton> xButton = VclPtr<RadioButton>::Create(pParent, nBits);
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        xWindow = xButton;

        if (::extractStock(rMap))
        {
            xWindow->SetText(getStockText(extractLabel(rMap)));
        }
    }
    else if (name == "GtkCheckButton")
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;
        OUString sWrap = BuilderUtils::extractCustomProperty(rMap);
        if (!sWrap.isEmpty())
            nBits |= WB_WORDBREAK;
        //maybe always import as TriStateBox and enable/disable tristate
        bool bIsTriState = extractInconsistent(rMap);
        VclPtr<CheckBox> xCheckBox;
        if (bIsTriState && m_bLegacy)
            xCheckBox = VclPtr<TriStateBox>::Create(pParent, nBits);
        else
            xCheckBox = VclPtr<CheckBox>::Create(pParent, nBits);
        if (bIsTriState)
        {
            xCheckBox->EnableTriState(true);
            xCheckBox->SetState(TRISTATE_INDET);
        }
        xCheckBox->SetImageAlign(ImageAlign::Left); //default to left

        xWindow = xCheckBox;

        if (::extractStock(rMap))
        {
            xWindow->SetText(getStockText(extractLabel(rMap)));
        }
    }
    else if (name == "GtkSpinButton")
    {
        OUString sAdjustment = extractAdjustment(rMap);
        OUString sPattern = BuilderUtils::extractCustomProperty(rMap);
        OUString sUnit = extractUnit(sPattern);

        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_BORDER|WB_3DLOOK;
        if (!id.endsWith("-nospin"))
            nBits |= WB_SPIN | WB_REPEAT;

        if (sPattern.isEmpty())
        {
            SAL_INFO("vcl.layout", "making numeric field for " << name << " " << sUnit);
            if (m_bLegacy)
            {
                connectNumericFormatterAdjustment(id, sAdjustment);
                xWindow = VclPtr<NumericField>::Create(pParent, nBits);
            }
            else
            {
                connectFormattedFormatterAdjustment(id, sAdjustment);
                VclPtrInstance<FormattedField> xField(pParent, nBits);
                xField->SetMinValue(0);
                xWindow = xField;
            }
        }
        else
        {
            if (sPattern == "hh:mm")
            {
                connectTimeFormatterAdjustment(id, sAdjustment);
                SAL_INFO("vcl.layout", "making time field for " << name << " " << sUnit);
                xWindow = VclPtr<TimeField>::Create(pParent, nBits);
            }
            else if (sPattern == "yy:mm:dd")
            {
                connectDateFormatterAdjustment(id, sAdjustment);
                SAL_INFO("vcl.layout", "making date field for " << name << " " << sUnit);
                xWindow = VclPtr<DateField>::Create(pParent, nBits);
            }
            else
            {
                connectNumericFormatterAdjustment(id, sAdjustment);
                FieldUnit eUnit = detectMetricUnit(sUnit);
                SAL_INFO("vcl.layout", "making metric field for " << name << " " << sUnit);
                VclPtrInstance<MetricField> xField(pParent, nBits);
                xField->SetUnit(eUnit);
                if (eUnit == FieldUnit::CUSTOM)
                    xField->SetCustomUnitText(sUnit);
                xWindow = xField;
            }
        }
    }
    else if (name == "GtkLinkButton")
        xWindow = VclPtr<FixedHyperlink>::Create(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_NOLABEL);
    else if ((name == "GtkComboBox") || (name == "GtkComboBoxText") || (name == "VclComboBoxText"))
    {
        OUString sPattern = BuilderUtils::extractCustomProperty(rMap);
        extractModel(id, rMap);

        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

        bool bDropdown = BuilderUtils::extractDropdown(rMap);

        if (bDropdown)
            nBits |= WB_DROPDOWN;

        if (!sPattern.isEmpty())
        {
            OUString sAdjustment = extractAdjustment(rMap);
            connectNumericFormatterAdjustment(id, sAdjustment);
            OUString sUnit = extractUnit(sPattern);
            FieldUnit eUnit = detectMetricUnit(sUnit);
            SAL_WARN("vcl.layout", "making metric box for type: " << name
                << " unit: " << sUnit
                << " name: " << id
                << " use a VclComboBoxNumeric instead");
            VclPtrInstance<MetricBox> xBox(pParent, nBits);
            xBox->EnableAutoSize(true);
            xBox->SetUnit(eUnit);
            xBox->SetDecimalDigits(extractDecimalDigits(sPattern));
            if (eUnit == FieldUnit::CUSTOM)
                xBox->SetCustomUnitText(sUnit);
            xWindow = xBox;
        }
        else if (extractEntry(rMap))
        {
            VclPtrInstance<ComboBox> xComboBox(pParent, nBits);
            xComboBox->EnableAutoSize(true);
            xWindow = xComboBox;
        }
        else
        {
            VclPtrInstance<ListBox> xListBox(pParent, nBits|WB_SIMPLEMODE);
            xListBox->EnableAutoSize(true);
            xWindow = xListBox;
        }
    }
    else if (name == "VclComboBoxNumeric")
    {
        OUString sPattern = BuilderUtils::extractCustomProperty(rMap);
        OUString sAdjustment = extractAdjustment(rMap);
        extractModel(id, rMap);

        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

        bool bDropdown = BuilderUtils::extractDropdown(rMap);

        if (bDropdown)
            nBits |= WB_DROPDOWN;

        if (!sPattern.isEmpty())
        {
            connectNumericFormatterAdjustment(id, sAdjustment);
            OUString sUnit = extractUnit(sPattern);
            FieldUnit eUnit = detectMetricUnit(sUnit);
            SAL_INFO("vcl.layout", "making metric box for " << name << " " << sUnit);
            VclPtrInstance<MetricBox> xBox(pParent, nBits);
            xBox->EnableAutoSize(true);
            xBox->SetUnit(eUnit);
            xBox->SetDecimalDigits(extractDecimalDigits(sPattern));
            if (eUnit == FieldUnit::CUSTOM)
                xBox->SetCustomUnitText(sUnit);
            xWindow = xBox;
        }
        else
        {
            SAL_INFO("vcl.layout", "making numeric box for " << name);
            connectNumericFormatterAdjustment(id, sAdjustment);
            VclPtrInstance<NumericBox> xBox(pParent, nBits);
            if (bDropdown)
                xBox->EnableAutoSize(true);
            xWindow = xBox;
        }
    }
    else if (name == "GtkTreeView")
    {
        //window we want to apply the packing props for this GtkTreeView to
        VclPtr<vcl::Window> xWindowForPackingProps;
        //To-Do
        //a) make SvHeaderTabListBox/SvTabListBox the default target for GtkTreeView
        //b) remove the non-drop down mode of ListBox and convert
        //   everything over to SvHeaderTabListBox/SvTabListBox
        //c) remove the users of makeSvTabListBox and makeSvTreeListBox
        extractModel(id, rMap);
        WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_HIDESELECTION;
        if (m_bLegacy)
        {
            OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
            if (!sBorder.isEmpty())
                nWinStyle |= WB_BORDER;
        }
        //ListBox/SvHeaderTabListBox manages its own scrolling,
        vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
        if (pRealParent != pParent)
            nWinStyle |= WB_BORDER;
        if (m_bLegacy)
        {
            xWindow = VclPtr<ListBox>::Create(pRealParent, nWinStyle | WB_SIMPLEMODE);
            xWindowForPackingProps = xWindow;
        }
        else
        {
            VclPtr<SvTabListBox> xBox;
            bool bHeadersVisible = extractHeadersVisible(rMap);
            if (bHeadersVisible)
            {
                VclPtr<VclVBox> xContainer = VclPtr<VclVBox>::Create(pRealParent);
                OString containerid(id + "-container");
                xContainer->SetHelpId(m_sHelpRoot + containerid);
                m_aChildren.emplace_back(containerid, xContainer, true);

                VclPtrInstance<HeaderBar> xHeader(xContainer, WB_BUTTONSTYLE | WB_BORDER | WB_TABSTOP | WB_3DLOOK);
                xHeader->set_width_request(0); // let the headerbar width not affect the size requistion
                OString headerid(id + "-header");
                xHeader->SetHelpId(m_sHelpRoot + headerid);
                m_aChildren.emplace_back(headerid, xHeader, true);

                VclPtr<LclHeaderTabListBox> xHeaderBox = VclPtr<LclHeaderTabListBox>::Create(xContainer, nWinStyle);
                xHeaderBox->InitHeaderBar(xHeader);
                xContainer->set_expand(true);
                xHeader->Show();
                xContainer->Show();
                xBox = xHeaderBox;
                xWindowForPackingProps = xContainer;
            }
            else
            {
                xBox = VclPtr<LclTabListBox>::Create(pRealParent, nWinStyle);
                xWindowForPackingProps = xBox;
            }
            xWindow = xBox;
            xBox->SetNoAutoCurEntry(true);
            xBox->SetQuickSearch(true);
            xBox->SetSpaceBetweenEntries(3);
            xBox->SetEntryHeight(16);
            xBox->SetHighlightRange(); // select over the whole width
        }
        if (pRealParent != pParent)
            cleanupWidgetOwnScrolling(pParent, xWindowForPackingProps, rMap);
    }
    else if (name == "GtkTreeViewColumn")
    {
        if (!m_bLegacy)
        {
            SvHeaderTabListBox* pTreeView = dynamic_cast<SvHeaderTabListBox*>(pParent);
            if (HeaderBar* pHeaderBar = pTreeView ? pTreeView->GetHeaderBar() : nullptr)
            {
                HeaderBarItemBits nBits = HeaderBarItemBits::LEFTIMAGE;
                if (extractClickable(rMap))
                    nBits |= HeaderBarItemBits::CLICKABLE;
                if (extractSortIndicator(rMap))
                    nBits |= HeaderBarItemBits::DOWNARROW;
                float fAlign = extractAlignment(rMap);
                if (fAlign == 0.0)
                    nBits |= HeaderBarItemBits::LEFT;
                else if (fAlign == 1.0)
                    nBits |= HeaderBarItemBits::RIGHT;
                else if (fAlign == 0.5)
                    nBits |= HeaderBarItemBits::CENTER;
                auto nItemId = pHeaderBar->GetItemCount() + 1;
                OUString sTitle(extractTitle(rMap));
                pHeaderBar->InsertItem(nItemId, sTitle, 100, nBits);
            }
        }
    }
    else if (name == "GtkLabel")
    {
        WinBits nWinStyle = WB_CENTER|WB_VCENTER|WB_3DLOOK;
        OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
        if (!sBorder.isEmpty())
            nWinStyle |= WB_BORDER;
        extractMnemonicWidget(id, rMap);
        if (extractSelectable(rMap))
            xWindow = VclPtr<SelectableFixedText>::Create(pParent, nWinStyle);
        else
            xWindow = VclPtr<FixedText>::Create(pParent, nWinStyle);
    }
    else if (name == "GtkImage")
    {
        extractStock(id, rMap);
        xWindow = VclPtr<FixedImage>::Create(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_SCALE);
        //such parentless GtkImages are temps used to set icons on buttons
        //default them to hidden to stop e.g. insert->index entry flicking temp
        //full screen windows
        if (!pParent)
        {
            rMap["visible"] = "false";
        }
    }
    else if (name == "GtkSeparator")
    {
        bVertical = extractOrientation(rMap);
        xWindow = VclPtr<FixedLine>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
    }
    else if (name == "GtkScrollbar")
    {
        extractAdjustmentToMap(id, rMap, m_pParserState->m_aScrollAdjustmentMaps);
        bVertical = extractOrientation(rMap);
        xWindow = VclPtr<ScrollBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
    }
    else if (name == "GtkProgressBar")
    {
        extractAdjustmentToMap(id, rMap, m_pParserState->m_aScrollAdjustmentMaps);
        bVertical = extractOrientation(rMap);
        xWindow = VclPtr<ProgressBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
    }
    else if (name == "GtkScrolledWindow")
    {
        xWindow = VclPtr<VclScrolledWindow>::Create(pParent);
    }
    else if (name == "GtkViewport")
    {
        xWindow = VclPtr<VclViewport>::Create(pParent);
    }
    else if (name == "GtkEventBox")
    {
        xWindow = VclPtr<VclEventBox>::Create(pParent);
    }
    else if (name == "GtkEntry")
    {
        xWindow = VclPtr<Edit>::Create(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK|WB_NOHIDESELECTION);
        BuilderUtils::ensureDefaultWidthChars(rMap);
    }
    else if (name == "GtkNotebook")
    {
        xWindow = VclPtr<TabControl>::Create(pParent, WB_STDTABCONTROL|WB_3DLOOK);
    }
    else if (name == "GtkDrawingArea")
    {
        OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
        xWindow = VclPtr<VclDrawingArea>::Create(pParent, sBorder.isEmpty() ? WB_TABSTOP : WB_BORDER | WB_TABSTOP);
    }
    else if (name == "GtkTextView")
    {
        extractBuffer(id, rMap);

        WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT|WB_NOHIDESELECTION;
        if (m_bLegacy)
        {
            OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
            if (!sBorder.isEmpty())
                nWinStyle |= WB_BORDER;
        }
        //VclMultiLineEdit manages its own scrolling,
        vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
        if (pRealParent != pParent)
            nWinStyle |= WB_BORDER;
        xWindow = VclPtr<VclMultiLineEdit>::Create(pRealParent, nWinStyle);
        if (pRealParent != pParent)
            cleanupWidgetOwnScrolling(pParent, xWindow, rMap);
    }
    else if (name == "GtkSpinner")
    {
        xWindow = VclPtr<Throbber>::Create(pParent, WB_3DLOOK);
    }
    else if (name == "GtkScale")
    {
        extractAdjustmentToMap(id, rMap, m_pParserState->m_aSliderAdjustmentMaps);
        bool bDrawValue = extractDrawValue(rMap);
        if (bDrawValue)
        {
            OUString sValuePos = extractValuePos(rMap);
            (void)sValuePos;
        }
        bVertical = extractOrientation(rMap);

        WinBits nWinStyle = bVertical ? WB_VERT : WB_HORZ;

        xWindow = VclPtr<Slider>::Create(pParent, nWinStyle);
    }
    else if (name == "GtkToolbar")
    {
        xWindow = VclPtr<ToolBox>::Create(pParent, WB_3DLOOK | WB_TABSTOP);
    }
    else if (name == "GtkToolButton" || name == "GtkMenuToolButton" ||
             name == "GtkToggleToolButton" || name == "GtkRadioToolButton")
    {
        ToolBox *pToolBox = dynamic_cast<ToolBox*>(pParent);
        if (pToolBox)
        {
            OUString aCommand(extractActionName(rMap));

            sal_uInt16 nItemId = 0;
            ToolBoxItemBits nBits = ToolBoxItemBits::NONE;
            if (name == "GtkMenuToolButton")
                nBits |= ToolBoxItemBits::DROPDOWN;
            else if (name == "GtkToggleToolButton")
                nBits |= ToolBoxItemBits::AUTOCHECK | ToolBoxItemBits::CHECKABLE;
            else if (name == "GtkRadioToolButton")
                nBits |= ToolBoxItemBits::AUTOCHECK | ToolBoxItemBits::RADIOCHECK;

            if (!aCommand.isEmpty() && m_xFrame.is())
            {
                pToolBox->InsertItem(aCommand, m_xFrame, nBits, extractSizeRequest(rMap));
                nItemId = pToolBox->GetItemId(aCommand);
            }
            else
            {
                nItemId = pToolBox->GetItemCount() + 1;
                    //TODO: ImplToolItems::size_type -> sal_uInt16!
                pToolBox->InsertItem(nItemId, extractLabel(rMap), nBits);
                pToolBox->SetItemCommand(nItemId, aCommand);
            }

            pToolBox->SetHelpId(nItemId, m_sHelpRoot + id);
            OUString sTooltip(extractTooltipText(rMap));
            if (!sTooltip.isEmpty())
                pToolBox->SetQuickHelpText(nItemId, sTooltip);

            OUString sIconName(extractIconName(rMap));
            if (!sIconName.isEmpty())
                pToolBox->SetItemImage(nItemId, FixedImage::loadThemeImage(sIconName));

            if (!extractVisible(rMap))
                pToolBox->HideItem(nItemId);

            m_pParserState->m_nLastToolbarId = nItemId;

            return nullptr; // no widget to be created
        }
    }
    else if (name == "GtkSeparatorToolItem")
    {
        ToolBox *pToolBox = dynamic_cast<ToolBox*>(pParent);
        if (pToolBox)
        {
            pToolBox->InsertSeparator();
            return nullptr; // no widget to be created
        }
    }
    else if (name == "GtkWindow")
    {
        WinBits nBits = extractDeferredBits(rMap);
        if (nBits & WB_DOCKABLE)
            xWindow = VclPtr<DockingWindow>::Create(pParent, nBits|WB_MOVEABLE);
        else
            xWindow = VclPtr<FloatingWindow>::Create(pParent, nBits|WB_MOVEABLE);
    }
    else if (name == "GtkPopover")
    {
        WinBits nBits = extractDeferredBits(rMap);
        xWindow = VclPtr<DockingWindow>::Create(pParent, nBits|WB_DOCKABLE|WB_MOVEABLE);
    }
    else if (name == "GtkListBox")
    {
        WinBits nBits = extractDeferredBits(rMap);
        xWindow = VclPtr<ListControl>::Create(pParent, nBits);
    }
    else if (name == "GtkCalendar")
    {
        WinBits nBits = extractDeferredBits(rMap);
        xWindow = VclPtr<Calendar>::Create(pParent, nBits);
    }
    else
    {
        sal_Int32 nDelim = name.indexOf('-');
        if (nDelim != -1)
        {
            OUString sFunction(OStringToOUString(OString("make") + name.copy(nDelim+1), RTL_TEXTENCODING_UTF8));

            customMakeWidget pFunction = nullptr;
#ifndef DISABLE_DYNLOADING
            OUStringBuffer sModuleBuf;
            sModuleBuf.append(SAL_DLLPREFIX);
            sModuleBuf.append(OStringToOUString(name.copy(0, nDelim), RTL_TEXTENCODING_UTF8));
            sModuleBuf.append(SAL_DLLEXTENSION);

            OUString sModule = sModuleBuf.makeStringAndClear();
            ModuleMap::iterator aI = g_aModuleMap.find(sModule);
            if (aI == g_aModuleMap.end())
            {
                std::shared_ptr<NoAutoUnloadModule> pModule;
#if ENABLE_MERGELIBS
                if (!g_pMergedLib->is())
                    g_pMergedLib->loadRelative(&thisModule, SVLIBRARY("merged"));
                if ((pFunction = reinterpret_cast<customMakeWidget>(g_pMergedLib->getFunctionSymbol(sFunction))))
                    pModule = g_pMergedLib;
#endif
                if (!pFunction)
                {
                    pModule.reset(new NoAutoUnloadModule);
                    bool ok = pModule->loadRelative(&thisModule, sModule);
                    assert(ok && "bad module name in .ui");
                    (void) ok;
                    pFunction = reinterpret_cast<customMakeWidget>(pModule->getFunctionSymbol(sFunction));
                }
                g_aModuleMap.insert(std::make_pair(sModule, pModule));
            }
            else
                pFunction = reinterpret_cast<customMakeWidget>(aI->second->getFunctionSymbol(sFunction));
#elif !HAVE_FEATURE_DESKTOP
            pFunction = lo_get_custom_widget_func(sFunction.toUtf8().getStr());
            SAL_WARN_IF(!pFunction, "vcl.layout", "Could not find " << sFunction);
            assert(pFunction);
#else
            pFunction = reinterpret_cast<customMakeWidget>(osl_getFunctionSymbol((oslModule) RTLD_DEFAULT, sFunction.pData));
#endif
            if (pFunction)
            {
                VclPtr<vcl::Window> xParent(pParent);
                pFunction(xWindow, xParent, rMap);
                if (xWindow->GetType() == WindowType::PUSHBUTTON)
                    setupFromActionName(static_cast<Button*>(xWindow.get()), rMap, m_xFrame);
                else if (xWindow->GetType() == WindowType::MENUBUTTON)
                {
                    OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
                    if (!sMenu.isEmpty())
                        m_pParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
                    setupFromActionName(static_cast<Button*>(xWindow.get()), rMap, m_xFrame);
                }
            }
        }
    }
    SAL_INFO_IF(!xWindow, "vcl.layout", "probably need to implement " << name << " or add a make" << name << " function");
    if (xWindow)
    {
        xWindow->SetHelpId(m_sHelpRoot + id);
        SAL_INFO("vcl.layout", "for " << name <<
            ", created " << xWindow.get() << " child of " <<
            pParent << "(" << xWindow->ImplGetWindowImpl()->mpParent.get() << "/" <<
            xWindow->ImplGetWindowImpl()->mpRealParent.get() << "/" <<
            xWindow->ImplGetWindowImpl()->mpBorderWindow.get() << ") with helpid " <<
            xWindow->GetHelpId());
        m_aChildren.emplace_back(id, xWindow, bVertical);
    }
    return xWindow;
}

namespace
{
    //return true for window types which exist in vcl but are not themselves
    //represented in the .ui format, i.e. only their children exist.
    bool isConsideredGtkPseudo(vcl::Window const *pWindow)
    {
        return pWindow->GetType() == WindowType::TABPAGE;
    }
}

//Any properties from .ui load we couldn't set because of potential virtual methods
//during ctor are applied here
void VclBuilder::setDeferredProperties()
{
    if (!m_bToplevelHasDeferredProperties)
        return;
    stringmap aDeferredProperties;
    aDeferredProperties.swap(m_aDeferredProperties);
    m_bToplevelHasDeferredProperties = false;
    BuilderUtils::set_properties(m_pParent, aDeferredProperties);
}

namespace BuilderUtils
{
    void set_properties(vcl::Window *pWindow, const VclBuilder::stringmap &rProps)
    {
        for (auto const& prop : rProps)
        {
            const OString &rKey = prop.first;
            const OUString &rValue = prop.second;
            pWindow->set_property(rKey, rValue);
        }
    }

    OUString convertMnemonicMarkup(const OUString &rIn)
    {
        OUStringBuffer aRet(rIn);
        for (sal_Int32 nI = 0; nI < aRet.getLength(); ++nI)
        {
            if (aRet[nI] == '_' && nI+1 < aRet.getLength())
            {
                if (aRet[nI+1] != '_')
                    aRet[nI] = MNEMONIC_CHAR;
                else
                    aRet.remove(nI, 1);
                ++nI;
            }
        }
        return aRet.makeStringAndClear();
    }

    OUString extractCustomProperty(VclBuilder::stringmap &rMap)
    {
        OUString sCustomProperty;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("customproperty"));
        if (aFind != rMap.end())
        {
            sCustomProperty = aFind->second;
            rMap.erase(aFind);
        }
        return sCustomProperty;
    }

    FieldUnit detectUnit(OUString const& rString)
    {
        OUString const unit(extractUnit(rString));
        return detectMetricUnit(unit);
    }

    void ensureDefaultWidthChars(VclBuilder::stringmap &rMap)
    {
        OString sWidthChars("width-chars");
        VclBuilder::stringmap::iterator aFind = rMap.find(sWidthChars);
        if (aFind == rMap.end())
            rMap[sWidthChars] = "25";
    }

    bool extractDropdown(VclBuilder::stringmap &rMap)
    {
        bool bDropdown = true;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("dropdown"));
        if (aFind != rMap.end())
        {
            bDropdown = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bDropdown;
    }

    void reorderWithinParent(vcl::Window &rWindow, sal_uInt16 nNewPosition)
    {
        WindowImpl *pWindowImpl = rWindow.ImplGetWindowImpl();
        if (pWindowImpl->mpParent != pWindowImpl->mpRealParent)
        {
            assert(pWindowImpl->mpBorderWindow == pWindowImpl->mpParent);
            assert(pWindowImpl->mpBorderWindow->ImplGetWindowImpl()->mpParent == pWindowImpl->mpRealParent);
            reorderWithinParent(*pWindowImpl->mpBorderWindow, nNewPosition);
            return;
        }
        rWindow.reorderWithinParent(nNewPosition);
    }

    void reorderWithinParent(std::vector<vcl::Window*>& rChilds, bool bIsButtonBox)
    {
        for (size_t i = 0; i < rChilds.size(); ++i)
        {
            reorderWithinParent(*rChilds[i], i);

            if (!bIsButtonBox)
                continue;

            //The first member of the group for legacy code needs WB_GROUP set and the
            //others not
            WinBits nBits = rChilds[i]->GetStyle();
            nBits &= ~WB_GROUP;
            if (i == 0)
                nBits |= WB_GROUP;
            rChilds[i]->SetStyle(nBits);
        }
    }

    sal_Int16 getRoleFromName(const OString& roleName)
    {
        using namespace com::sun::star::accessibility;

        static const std::unordered_map<OString, sal_Int16> aAtkRoleToAccessibleRole = {
            /* This is in atkobject.h's AtkRole order */
            { "invalid",               AccessibleRole::UNKNOWN },
            { "accelerator label",     AccessibleRole::UNKNOWN },
            { "alert",                 AccessibleRole::ALERT },
            { "animation",             AccessibleRole::UNKNOWN },
            { "arrow",                 AccessibleRole::UNKNOWN },
            { "calendar",              AccessibleRole::UNKNOWN },
            { "canvas",                AccessibleRole::CANVAS },
            { "check box",             AccessibleRole::CHECK_BOX },
            { "check menu item",       AccessibleRole::CHECK_MENU_ITEM },
            { "color chooser",         AccessibleRole::COLOR_CHOOSER },
            { "column header",         AccessibleRole::COLUMN_HEADER },
            { "combo box",             AccessibleRole::COMBO_BOX },
            { "date editor",           AccessibleRole::DATE_EDITOR },
            { "desktop icon",          AccessibleRole::DESKTOP_ICON },
            { "desktop frame",         AccessibleRole::DESKTOP_PANE }, // ?
            { "dial",                  AccessibleRole::UNKNOWN },
            { "dialog",                AccessibleRole::DIALOG },
            { "directory pane",        AccessibleRole::DIRECTORY_PANE },
            { "drawing area",          AccessibleRole::UNKNOWN },
            { "file chooser",          AccessibleRole::FILE_CHOOSER },
            { "filler",                AccessibleRole::FILLER },
            { "font chooser",          AccessibleRole::FONT_CHOOSER },
            { "frame",                 AccessibleRole::FRAME },
            { "glass pane",            AccessibleRole::GLASS_PANE },
            { "html container",        AccessibleRole::UNKNOWN },
            { "icon",                  AccessibleRole::ICON },
            { "image",                 AccessibleRole::GRAPHIC },
            { "internal frame",        AccessibleRole::INTERNAL_FRAME },
            { "label",                 AccessibleRole::LABEL },
            { "layered pane",          AccessibleRole::LAYERED_PANE },
            { "list",                  AccessibleRole::LIST },
            { "list item",             AccessibleRole::LIST_ITEM },
            { "menu",                  AccessibleRole::MENU },
            { "menu bar",              AccessibleRole::MENU_BAR },
            { "menu item",             AccessibleRole::MENU_ITEM },
            { "option pane",           AccessibleRole::OPTION_PANE },
            { "page tab",              AccessibleRole::PAGE_TAB },
            { "page tab list",         AccessibleRole::PAGE_TAB_LIST },
            { "panel",                 AccessibleRole::PANEL }, // or SHAPE or TEXT_FRAME ?
            { "password text",         AccessibleRole::PASSWORD_TEXT },
            { "popup menu",            AccessibleRole::POPUP_MENU },
            { "progress bar",          AccessibleRole::PROGRESS_BAR },
            { "push button",           AccessibleRole::PUSH_BUTTON }, // or BUTTON_DROPDOWN or BUTTON_MENU
            { "radio button",          AccessibleRole::RADIO_BUTTON },
            { "radio menu item",       AccessibleRole::RADIO_MENU_ITEM },
            { "root pane",             AccessibleRole::ROOT_PANE },
            { "row header",            AccessibleRole::ROW_HEADER },
            { "scroll bar",            AccessibleRole::SCROLL_BAR },
            { "scroll pane",           AccessibleRole::SCROLL_PANE },
            { "separator",             AccessibleRole::SEPARATOR },
            { "slider",                AccessibleRole::SLIDER },
            { "split pane",            AccessibleRole::SPLIT_PANE },
            { "spin button",           AccessibleRole::SPIN_BOX }, // ?
            { "statusbar",             AccessibleRole::STATUS_BAR },
            { "table",                 AccessibleRole::TABLE },
            { "table cell",            AccessibleRole::TABLE_CELL },
            { "table column header",   AccessibleRole::COLUMN_HEADER }, // approximate
            { "table row header",      AccessibleRole::ROW_HEADER }, // approximate
            { "tear off menu item",    AccessibleRole::UNKNOWN },
            { "terminal",              AccessibleRole::UNKNOWN },
            { "text",                  AccessibleRole::TEXT },
            { "toggle button",         AccessibleRole::TOGGLE_BUTTON },
            { "tool bar",              AccessibleRole::TOOL_BAR },
            { "tool tip",              AccessibleRole::TOOL_TIP },
            { "tree",                  AccessibleRole::TREE },
            { "tree table",            AccessibleRole::TREE_TABLE },
            { "unknown",               AccessibleRole::UNKNOWN },
            { "viewport",              AccessibleRole::VIEW_PORT },
            { "window",                AccessibleRole::WINDOW },
            { "header",                AccessibleRole::HEADER },
            { "footer",                AccessibleRole::FOOTER },
            { "paragraph",             AccessibleRole::PARAGRAPH },
            { "ruler",                 AccessibleRole::RULER },
            { "application",           AccessibleRole::UNKNOWN },
            { "autocomplete",          AccessibleRole::UNKNOWN },
            { "edit bar",              AccessibleRole::EDIT_BAR },
            { "embedded",              AccessibleRole::EMBEDDED_OBJECT },
            { "entry",                 AccessibleRole::UNKNOWN },
            { "chart",                 AccessibleRole::CHART },
            { "caption",               AccessibleRole::CAPTION },
            { "document frame",        AccessibleRole::DOCUMENT },
            { "heading",               AccessibleRole::HEADING },
            { "page",                  AccessibleRole::PAGE },
            { "section",               AccessibleRole::SECTION },
            { "redundant object",      AccessibleRole::UNKNOWN },
            { "form",                  AccessibleRole::FORM },
            { "link",                  AccessibleRole::HYPER_LINK },
            { "input method window",   AccessibleRole::UNKNOWN },
            { "table row",             AccessibleRole::UNKNOWN },
            { "tree item",             AccessibleRole::TREE_ITEM },
            { "document spreadsheet",  AccessibleRole::DOCUMENT_SPREADSHEET },
            { "document presentation", AccessibleRole::DOCUMENT_PRESENTATION },
            { "document text",         AccessibleRole::DOCUMENT_TEXT },
            { "document web",          AccessibleRole::DOCUMENT }, // approximate
            { "document email",        AccessibleRole::DOCUMENT }, // approximate
            { "comment",               AccessibleRole::COMMENT }, // or NOTE or END_NOTE or FOOTNOTE or SCROLL_PANE
            { "list box",              AccessibleRole::UNKNOWN },
            { "grouping",              AccessibleRole::GROUP_BOX },
            { "image map",             AccessibleRole::IMAGE_MAP },
            { "notification",          AccessibleRole::UNKNOWN },
            { "info bar",              AccessibleRole::UNKNOWN },
            { "level bar",             AccessibleRole::UNKNOWN },
            { "title bar",             AccessibleRole::UNKNOWN },
            { "block quote",           AccessibleRole::UNKNOWN },
            { "audio",                 AccessibleRole::UNKNOWN },
            { "video",                 AccessibleRole::UNKNOWN },
            { "definition",            AccessibleRole::UNKNOWN },
            { "article",               AccessibleRole::UNKNOWN },
            { "landmark",              AccessibleRole::UNKNOWN },
            { "log",                   AccessibleRole::UNKNOWN },
            { "marquee",               AccessibleRole::UNKNOWN },
            { "math",                  AccessibleRole::UNKNOWN },
            { "rating",                AccessibleRole::UNKNOWN },
            { "timer",                 AccessibleRole::UNKNOWN },
            { "description list",      AccessibleRole::UNKNOWN },
            { "description term",      AccessibleRole::UNKNOWN },
            { "description value",     AccessibleRole::UNKNOWN },
            { "static",                AccessibleRole::STATIC },
            { "math fraction",         AccessibleRole::UNKNOWN },
            { "math root",             AccessibleRole::UNKNOWN },
            { "subscript",             AccessibleRole::UNKNOWN },
            { "superscript",           AccessibleRole::UNKNOWN },
            { "footnote",              AccessibleRole::FOOTNOTE },
        };

        auto it = aAtkRoleToAccessibleRole.find(roleName);
        if (it == aAtkRoleToAccessibleRole.end())
            return AccessibleRole::UNKNOWN;
        return it->second;
    }
}

VclPtr<vcl::Window> VclBuilder::insertObject(vcl::Window *pParent, const OString &rClass,
    const OString &rID, stringmap &rProps, stringmap &rPango, stringmap &rAtk)
{
    VclPtr<vcl::Window> pCurrentChild;

    if (m_pParent && !isConsideredGtkPseudo(m_pParent) && !m_sID.isEmpty() && rID == m_sID)
    {
        pCurrentChild = m_pParent;

        //toplevels default to resizable and apparently you can't change them
        //afterwards, so we need to wait until now before we can truly
        //initialize the dialog.
        if (pParent && pParent->IsSystemWindow())
        {
            SystemWindow *pSysWin = static_cast<SystemWindow*>(pCurrentChild.get());
            pSysWin->doDeferredInit(extractDeferredBits(rProps));
            m_bToplevelHasDeferredInit = false;
        }
        else if (pParent && pParent->IsDockingWindow())
        {
            DockingWindow *pDockWin = static_cast<DockingWindow*>(pCurrentChild.get());
            pDockWin->doDeferredInit(extractDeferredBits(rProps));
            m_bToplevelHasDeferredInit = false;
        }

        if (pCurrentChild->GetHelpId().isEmpty())
        {
            pCurrentChild->SetHelpId(m_sHelpRoot + m_sID);
            SAL_INFO("vcl.layout", "for toplevel dialog " << this << " " <<
                rID << ", set helpid " << pCurrentChild->GetHelpId());
        }
        m_bToplevelParentFound = true;
    }
    else
    {
        //if we're being inserting under a toplevel dialog whose init is
        //deferred due to waiting to encounter it in this .ui, and it hasn't
        //been seen yet, then make unattached widgets parent-less toplevels
        if (pParent == m_pParent.get() && m_bToplevelHasDeferredInit)
            pParent = nullptr;
        pCurrentChild = makeObject(pParent, rClass, rID, rProps);
    }

    if (pCurrentChild)
    {
        pCurrentChild->set_id(OStringToOUString(rID, RTL_TEXTENCODING_UTF8));
        if (pCurrentChild == m_pParent.get() && m_bToplevelHasDeferredProperties)
            m_aDeferredProperties = rProps;
        else
            BuilderUtils::set_properties(pCurrentChild, rProps);

        for (auto const& elem : rPango)
        {
            const OString &rKey = elem.first;
            const OUString &rValue = elem.second;
            pCurrentChild->set_font_attribute(rKey, rValue);
        }

        m_pParserState->m_aAtkInfo[pCurrentChild] = rAtk;
    }

    rProps.clear();
    rPango.clear();
    rAtk.clear();

    if (!pCurrentChild)
        pCurrentChild = m_aChildren.empty() ? pParent : m_aChildren.back().m_pWindow.get();
    return pCurrentChild;
}

void VclBuilder::handleTabChild(vcl::Window *pParent, xmlreader::XmlReader &reader)
{
    OString sID;

    int nLevel = 1;
    stringmap aProperties;
    std::vector<vcl::EnumContext::Context> context;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("object"))
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("id"))
                    {
                        name = reader.getAttributeValue(false);
                        sID = OString(name.begin, name.length);
                        sal_Int32 nDelim = sID.indexOf(':');
                        if (nDelim != -1)
                        {
                            OString sPattern = sID.copy(nDelim+1);
                            aProperties[OString("customproperty")] = OUString::fromUtf8(sPattern);
                            sID = sID.copy(0, nDelim);
                        }
                    }
                }
            }
            else if (name.equals("style"))
            {
                int nPriority = 0;
                context = handleStyle(reader, nPriority);
                --nLevel;
            }
            else if (name.equals("property"))
                collectProperty(reader, aProperties);
        }

        if (res == xmlreader::XmlReader::Result::End)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::Result::Done)
            break;
    }

    if (!pParent)
        return;

    TabControl *pTabControl = static_cast<TabControl*>(pParent);
    VclBuilder::stringmap::iterator aFind = aProperties.find(OString("label"));
    if (aFind != aProperties.end())
    {
        sal_uInt16 nPageId = pTabControl->GetCurPageId();
        pTabControl->SetPageText(nPageId, aFind->second);
        pTabControl->SetPageName(nPageId, sID);
        if (!context.empty())
        {
            TabPage* pPage = pTabControl->GetTabPage(nPageId);
            pPage->SetContext(context);
        }
    }
    else
        pTabControl->RemovePage(pTabControl->GetCurPageId());
}

//so that tabbing between controls goes in a visually sensible sequence
//we sort these into a best-tab-order sequence
bool VclBuilder::sortIntoBestTabTraversalOrder::operator()(const vcl::Window *pA, const vcl::Window *pB) const
{
    //sort child order within parent list by grid position
    sal_Int32 nTopA = pA->get_grid_top_attach();
    sal_Int32 nTopB = pB->get_grid_top_attach();
    if (nTopA < nTopB)
        return true;
    if (nTopA > nTopB)
        return false;
    sal_Int32 nLeftA = pA->get_grid_left_attach();
    sal_Int32 nLeftB = pB->get_grid_left_attach();
    if (nLeftA < nLeftB)
        return true;
    if (nLeftA > nLeftB)
        return false;
    //sort into two groups of pack start and pack end
    VclPackType ePackA = pA->get_pack_type();
    VclPackType ePackB = pB->get_pack_type();
    if (ePackA < ePackB)
        return true;
    if (ePackA > ePackB)
        return false;
    bool bVerticalContainer = m_pBuilder->get_window_packing_data(pA->GetParent()).m_bVerticalOrient;
    bool bPackA = pA->get_secondary();
    bool bPackB = pB->get_secondary();
    if (!bVerticalContainer)
    {
        //for horizontal boxes group secondaries before primaries
        if (bPackA > bPackB)
            return true;
        if (bPackA < bPackB)
            return false;
    }
    else
    {
        //for vertical boxes group secondaries after primaries
        if (bPackA < bPackB)
            return true;
        if (bPackA > bPackB)
            return false;
    }
    //honour relative box positions with pack group, (numerical order is reversed
    //for VclPackType::End, they are packed from the end back, but here we need
    //them in visual layout order so that tabbing works as expected)
    sal_Int32 nPackA = m_pBuilder->get_window_packing_data(pA).m_nPosition;
    sal_Int32 nPackB = m_pBuilder->get_window_packing_data(pB).m_nPosition;
    if (nPackA < nPackB)
        return ePackA == VclPackType::Start;
    if (nPackA > nPackB)
        return ePackA != VclPackType::Start;
    //sort labels of Frames before body
    if (pA->GetParent() == pB->GetParent())
    {
        const VclFrame *pFrameParent = dynamic_cast<const VclFrame*>(pA->GetParent());
        if (pFrameParent)
        {
            const vcl::Window *pLabel = pFrameParent->get_label_widget();
            int nFramePosA = (pA == pLabel) ? 0 : 1;
            int nFramePosB = (pB == pLabel) ? 0 : 1;
            return nFramePosA < nFramePosB;
        }
    }
    return false;
}

void VclBuilder::handleChild(vcl::Window *pParent, xmlreader::XmlReader &reader)
{
    vcl::Window *pCurrentChild = nullptr;

    xmlreader::Span name;
    int nsId;
    OString sType, sInternalChild;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("type"))
        {
            name = reader.getAttributeValue(false);
            sType = OString(name.begin, name.length);
        }
        else if (name.equals("internal-child"))
        {
            name = reader.getAttributeValue(false);
            sInternalChild = OString(name.begin, name.length);
        }
    }

    if (sType == "tab")
    {
        handleTabChild(pParent, reader);
        return;
    }

    int nLevel = 1;
    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("object") || name.equals("placeholder"))
            {
                pCurrentChild = handleObject(pParent, reader).get();

                bool bObjectInserted = pCurrentChild && pParent != pCurrentChild;

                if (bObjectInserted)
                {
                    //Internal-children default in glade to not having their visible bits set
                    //even though they are visible (generally anyway)
                    if (!sInternalChild.isEmpty())
                        pCurrentChild->Show();

                    //Select the first page if it's a notebook
                    if (pCurrentChild->GetType() == WindowType::TABCONTROL)
                    {
                        TabControl *pTabControl = static_cast<TabControl*>(pCurrentChild);
                        pTabControl->SetCurPageId(pTabControl->GetPageId(0));

                        //To-Do add reorder capability to the TabControl
                    }
                    else
                    {
                        // We want to sort labels before contents of frames
                        // for keyboard traversal, especially if there
                        // are multiple widgets using the same mnemonic
                        if (sType == "label")
                        {
                            if (VclFrame *pFrameParent = dynamic_cast<VclFrame*>(pParent))
                                pFrameParent->designate_label(pCurrentChild);
                        }
                        if (sInternalChild.startsWith("vbox") || sInternalChild.startsWith("messagedialog-vbox"))
                        {
                            if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pParent))
                                pBoxParent->set_content_area(static_cast<VclBox*>(pCurrentChild)); // FIXME-VCLPTR
                        }
                        else if (sInternalChild.startsWith("action_area") || sInternalChild.startsWith("messagedialog-action_area"))
                        {
                            vcl::Window *pContentArea = pCurrentChild->GetParent();
                            if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pContentArea ? pContentArea->GetParent() : nullptr))
                            {
                                pBoxParent->set_action_area(static_cast<VclButtonBox*>(pCurrentChild)); // FIXME-VCLPTR
                            }
                        }

                        bool bIsButtonBox = dynamic_cast<VclButtonBox*>(pCurrentChild) != nullptr;

                        //To-Do make reorder a virtual in Window, move this foo
                        //there and see above
                        std::vector<vcl::Window*> aChilds;
                        for (vcl::Window* pChild = pCurrentChild->GetWindow(GetWindowType::FirstChild); pChild;
                            pChild = pChild->GetWindow(GetWindowType::Next))
                        {
                            if (bIsButtonBox)
                            {
                                if (PushButton* pPushButton = dynamic_cast<PushButton*>(pChild))
                                    pPushButton->setAction(true);
                            }

                            aChilds.push_back(pChild);
                        }

                        //sort child order within parent so that tabbing
                        //between controls goes in a visually sensible sequence
                        std::stable_sort(aChilds.begin(), aChilds.end(), sortIntoBestTabTraversalOrder(this));
                        BuilderUtils::reorderWithinParent(aChilds, bIsButtonBox);
                    }
                }
            }
            else if (name.equals("packing"))
            {
                handlePacking(pCurrentChild, pParent, reader);
            }
            else if (name.equals("interface"))
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("domain"))
                    {
                        name = reader.getAttributeValue(false);
                        sType = OString(name.begin, name.length);
                        m_pParserState->m_aResLocale = Translate::Create(sType.getStr());
                    }
                }
                ++nLevel;
            }
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::Result::End)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::Result::Done)
            break;
    }
}

void VclBuilder::collectPangoAttribute(xmlreader::XmlReader &reader, stringmap &rMap)
{
    xmlreader::Span span;
    int nsId;

    OString sProperty;
    OString sValue;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span.equals("name"))
        {
            span = reader.getAttributeValue(false);
            sProperty = OString(span.begin, span.length);
        }
        else if (span.equals("value"))
        {
            span = reader.getAttributeValue(false);
            sValue = OString(span.begin, span.length);
        }
    }

    if (!sProperty.isEmpty())
        rMap[sProperty] = OUString::fromUtf8(sValue);
}

void VclBuilder::collectAtkRelationAttribute(xmlreader::XmlReader &reader, stringmap &rMap)
{
    xmlreader::Span span;
    int nsId;

    OString sProperty;
    OString sValue;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span.equals("type"))
        {
            span = reader.getAttributeValue(false);
            sProperty = OString(span.begin, span.length);
        }
        else if (span.equals("target"))
        {
            span = reader.getAttributeValue(false);
            sValue = OString(span.begin, span.length);
            sal_Int32 nDelim = sValue.indexOf(':');
            if (nDelim != -1)
                sValue = sValue.copy(0, nDelim);
        }
    }

    if (!sProperty.isEmpty())
        rMap[sProperty] = OUString::fromUtf8(sValue);
}

void VclBuilder::collectAtkRoleAttribute(xmlreader::XmlReader &reader, stringmap &rMap)
{
    xmlreader::Span span;
    int nsId;

    OString sProperty;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span.equals("type"))
        {
            span = reader.getAttributeValue(false);
            sProperty = OString(span.begin, span.length);
        }
    }

    if (!sProperty.isEmpty())
        rMap["role"] = OUString::fromUtf8(sProperty);
}

void VclBuilder::handleRow(xmlreader::XmlReader &reader, const OString &rID)
{
    int nLevel = 1;

    ListStore::row aRow;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("col"))
            {
                bool bTranslated = false;
                sal_uInt32 nId = 0;
                OString sContext;

                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("id"))
                    {
                        name = reader.getAttributeValue(false);
                        nId = OString(name.begin, name.length).toInt32();
                    }
                    else if (nId == 0 && name.equals("translatable") && reader.getAttributeValue(false).equals("yes"))
                    {
                        bTranslated = true;
                    }
                    else if (name.equals("context"))
                    {
                        name = reader.getAttributeValue(false);
                        sContext = OString(name.begin, name.length);
                    }
                }

                reader.nextItem(
                    xmlreader::XmlReader::Text::Raw, &name, &nsId);

                OString sValue = OString(name.begin, name.length);
                OUString sFinalValue;
                if (bTranslated)
                {
                    if (!sContext.isEmpty())
                        sValue = sContext + "\004" + sValue;
                    sFinalValue = Translate::get(sValue.getStr(), m_pParserState->m_aResLocale);
                }
                else
                    sFinalValue = OUString::fromUtf8(sValue);


                if (aRow.size() < nId+1)
                    aRow.resize(nId+1);
                aRow[nId] = sFinalValue;
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    m_pParserState->m_aModels[rID].m_aEntries.push_back(aRow);
}

void VclBuilder::handleListStore(xmlreader::XmlReader &reader, const OString &rID, const OString &rClass)
{
    int nLevel = 1;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("row"))
            {
                bool bNotTreeStore = rClass != "GtkTreeStore";
                if (bNotTreeStore)
                    handleRow(reader, rID);
                assert(bNotTreeStore && "gtk, as the time of writing, doesn't support data in GtkTreeStore serialization");
            }
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

void VclBuilder::handleAtkObject(xmlreader::XmlReader &reader, vcl::Window *pWindow)
{
    assert(pWindow);

    int nLevel = 1;

    stringmap aProperties;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("property"))
                collectProperty(reader, aProperties);
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    for (auto const& prop : aProperties)
    {
        const OString &rKey = prop.first;
        const OUString &rValue = prop.second;

        if (pWindow && rKey.match("AtkObject::"))
            pWindow->set_property(rKey.copy(RTL_CONSTASCII_LENGTH("AtkObject::")), rValue);
        else
            SAL_WARN("vcl.layout", "unhandled atk prop: " << rKey);
    }
}

std::vector<ComboBoxTextItem> VclBuilder::handleItems(xmlreader::XmlReader &reader) const
{
    int nLevel = 1;

    std::vector<ComboBoxTextItem> aItems;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("item"))
            {
                bool bTranslated = false;
                OString sContext, sId;

                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("translatable") && reader.getAttributeValue(false).equals("yes"))
                    {
                        bTranslated = true;
                    }
                    else if (name.equals("context"))
                    {
                        name = reader.getAttributeValue(false);
                        sContext = OString(name.begin, name.length);
                    }
                    else if (name.equals("id"))
                    {
                        name = reader.getAttributeValue(false);
                        sId = OString(name.begin, name.length);
                    }
                }

                reader.nextItem(
                    xmlreader::XmlReader::Text::Raw, &name, &nsId);

                OString sValue = OString(name.begin, name.length);
                OUString sFinalValue;
                if (bTranslated)
                {
                    if (!sContext.isEmpty())
                        sValue = sContext + "\004" + sValue;
                    sFinalValue = Translate::get(sValue.getStr(), m_pParserState->m_aResLocale);
                }
                else
                    sFinalValue = OUString::fromUtf8(sValue);

                if (m_pStringReplace)
                    sFinalValue = (*m_pStringReplace)(sFinalValue);

                aItems.emplace_back(sFinalValue, sId);
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    return aItems;
}

void VclBuilder::handleMenu(xmlreader::XmlReader &reader, const OString &rID)
{
    VclPtr<PopupMenu> pCurrentMenu = VclPtr<PopupMenu>::Create();

    int nLevel = 1;

    stringmap aProperties;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("child"))
            {
                handleMenuChild(pCurrentMenu, reader);
            }
            else
            {
                ++nLevel;
                if (name.equals("property"))
                    collectProperty(reader, aProperties);
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    m_aMenus.emplace_back(rID, pCurrentMenu);
}

void VclBuilder::handleMenuChild(PopupMenu *pParent, xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    int nLevel = 1;
    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("object") || name.equals("placeholder"))
            {
                handleMenuObject(pParent, reader);
            }
            else
                ++nLevel;
        }

        if (res == xmlreader::XmlReader::Result::End)
            --nLevel;

        if (!nLevel)
            break;

        if (res == xmlreader::XmlReader::Result::Done)
            break;
    }
}

void VclBuilder::handleMenuObject(PopupMenu *pParent, xmlreader::XmlReader &reader)
{
    OString sClass;
    OString sID;
    OUString sCustomProperty;
    PopupMenu *pSubMenu = nullptr;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("class"))
        {
            name = reader.getAttributeValue(false);
            sClass = OString(name.begin, name.length);
        }
        else if (name.equals("id"))
        {
            name = reader.getAttributeValue(false);
            sID = OString(name.begin, name.length);
            sal_Int32 nDelim = sID.indexOf(':');
            if (nDelim != -1)
            {
                sCustomProperty = OUString::fromUtf8(sID.copy(nDelim+1));
                sID = sID.copy(0, nDelim);
            }
        }
    }

    int nLevel = 1;

    stringmap aProperties;
    accelmap aAccelerators;

    if (!sCustomProperty.isEmpty())
        aProperties[OString("customproperty")] = sCustomProperty;

    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("child"))
            {
                size_t nChildMenuIdx = m_aMenus.size();
                handleChild(nullptr, reader);
                assert(m_aMenus.size() > nChildMenuIdx && "menu not inserted");
                pSubMenu = m_aMenus[nChildMenuIdx].m_pMenu;
            }
            else
            {
                ++nLevel;
                if (name.equals("property"))
                    collectProperty(reader, aProperties);
                else if (name.equals("accelerator"))
                    collectAccelerator(reader, aAccelerators);
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    insertMenuObject(pParent, pSubMenu, sClass, sID, aProperties, aAccelerators);
}

void VclBuilder::handleSizeGroup(xmlreader::XmlReader &reader)
{
    m_pParserState->m_aSizeGroups.emplace_back();
    SizeGroup &rSizeGroup = m_pParserState->m_aSizeGroups.back();

    int nLevel = 1;

    while(true)
    {
        xmlreader::Span name;
        int nsId;

        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("widget"))
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name.equals("name"))
                    {
                        name = reader.getAttributeValue(false);
                        OString sWidget = OString(name.begin, name.length);
                        sal_Int32 nDelim = sWidget.indexOf(':');
                        if (nDelim != -1)
                            sWidget = sWidget.copy(0, nDelim);
                        rSizeGroup.m_aWidgets.push_back(sWidget);
                    }
                }
            }
            else
            {
                if (name.equals("property"))
                    collectProperty(reader, rSizeGroup.m_aProperties);
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

namespace
{
    vcl::KeyCode makeKeyCode(const std::pair<OString,OString> &rKey)
    {
        bool bShift = rKey.second.indexOf("GDK_SHIFT_MASK") != -1;
        bool bMod1 = rKey.second.indexOf("GDK_CONTROL_MASK") != -1;
        bool bMod2 = rKey.second.indexOf("GDK_MOD1_MASK") != -1;
        bool bMod3 = rKey.second.indexOf("GDK_MOD2_MASK") != -1;

        if (rKey.first == "Insert")
            return vcl::KeyCode(KEY_INSERT, bShift, bMod1, bMod2, bMod3);
        else if (rKey.first == "Delete")
            return vcl::KeyCode(KEY_DELETE, bShift, bMod1, bMod2, bMod3);

        assert (rKey.first.getLength() == 1);
        sal_Char cChar = rKey.first.toChar();

        if (cChar >= 'a' && cChar <= 'z')
            return vcl::KeyCode(KEY_A + (cChar - 'a'), bShift, bMod1, bMod2, bMod3);
        else if (cChar >= 'A' && cChar <= 'Z')
            return vcl::KeyCode(KEY_A + (cChar - 'A'), bShift, bMod1, bMod2, bMod3);
        else if (cChar >= '0' && cChar <= '9')
            return vcl::KeyCode(KEY_0 + (cChar - 'A'), bShift, bMod1, bMod2, bMod3);

        return vcl::KeyCode(cChar, bShift, bMod1, bMod2, bMod3);
    }
}

void VclBuilder::insertMenuObject(PopupMenu *pParent, PopupMenu *pSubMenu, const OString &rClass, const OString &rID,
    stringmap &rProps, accelmap &rAccels)
{
    sal_uInt16 nOldCount = pParent->GetItemCount();
    sal_uInt16 nNewId = ++m_pParserState->m_nLastMenuItemId;

    if (rClass == "GtkMenuItem")
    {
        OUString sLabel(BuilderUtils::convertMnemonicMarkup(extractLabel(rProps)));
        OUString aCommand(extractActionName(rProps));
        pParent->InsertItem(nNewId, sLabel, MenuItemBits::NONE , rID);
        pParent->SetItemCommand(nNewId, aCommand);
        if (pSubMenu)
            pParent->SetPopupMenu(nNewId, pSubMenu);
    }
    else if (rClass == "GtkCheckMenuItem")
    {
        OUString sLabel(BuilderUtils::convertMnemonicMarkup(extractLabel(rProps)));
        OUString aCommand(extractActionName(rProps));
        pParent->InsertItem(nNewId, sLabel, MenuItemBits::CHECKABLE, rID);
        pParent->SetItemCommand(nNewId, aCommand);
    }
    else if (rClass == "GtkRadioMenuItem")
    {
        OUString sLabel(BuilderUtils::convertMnemonicMarkup(extractLabel(rProps)));
        OUString aCommand(extractActionName(rProps));
        pParent->InsertItem(nNewId, sLabel, MenuItemBits::AUTOCHECK | MenuItemBits::RADIOCHECK, rID);
        pParent->SetItemCommand(nNewId, aCommand);
    }
    else if (rClass == "GtkSeparatorMenuItem")
    {
        pParent->InsertSeparator(rID);
    }

    SAL_WARN_IF(nOldCount == pParent->GetItemCount(), "vcl.layout", "probably need to implement " << rClass);

    if (nOldCount != pParent->GetItemCount())
    {
        pParent->SetHelpId(nNewId, m_sHelpRoot + rID);

        for (auto const& prop : rProps)
        {
            const OString &rKey = prop.first;
            const OUString &rValue = prop.second;

            if (rKey == "tooltip-markup")
                pParent->SetTipHelpText(nNewId, rValue);
            else if (rKey == "tooltip-text")
                pParent->SetTipHelpText(nNewId, rValue);
            else if (rKey == "visible")
                pParent->ShowItem(nNewId, toBool(rValue));
            else if (rKey == "has-default" && toBool(rValue))
                pParent->SetSelectedEntry(nNewId);
            else
                SAL_INFO("vcl.layout", "unhandled property: " << rKey);
        }

        for (auto const& accel : rAccels)
        {
            const OString &rSignal = accel.first;
            const auto &rValue = accel.second;

            if (rSignal == "activate")
                pParent->SetAccelKey(nNewId, makeKeyCode(rValue));
            else
                SAL_INFO("vcl.layout", "unhandled accelerator for: " << rSignal);
        }
    }

    rProps.clear();
}

/// Insert items to a ComboBox or a ListBox.
/// They have no common ancestor that would have 'InsertEntry()', so use a template.
template<typename T> static bool insertItems(vcl::Window *pWindow, VclBuilder::stringmap &rMap,
                                             std::vector<std::unique_ptr<OUString>>& rUserData,
                                             const std::vector<ComboBoxTextItem> &rItems)
{
    T *pContainer = dynamic_cast<T*>(pWindow);
    if (!pContainer)
        return false;

    sal_uInt16 nActiveId = extractActive(rMap);
    for (auto const& item : rItems)
    {
        sal_Int32 nPos = pContainer->InsertEntry(item.m_sItem);
        if (!item.m_sId.isEmpty())
        {
            rUserData.emplace_back(std::make_unique<OUString>(OUString::fromUtf8(item.m_sId)));
            pContainer->SetEntryData(nPos, rUserData.back().get());
        }
    }
    if (nActiveId < rItems.size())
        pContainer->SelectEntryPos(nActiveId);

    return true;
}

VclPtr<vcl::Window> VclBuilder::handleObject(vcl::Window *pParent, xmlreader::XmlReader &reader)
{
    OString sClass;
    OString sID;
    OUString sCustomProperty;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("class"))
        {
            name = reader.getAttributeValue(false);
            sClass = OString(name.begin, name.length);
        }
        else if (name.equals("id"))
        {
            name = reader.getAttributeValue(false);
            sID = OString(name.begin, name.length);
            sal_Int32 nDelim = sID.indexOf(':');
            if (nDelim != -1)
            {
                sCustomProperty = OUString::fromUtf8(sID.copy(nDelim+1));
                sID = sID.copy(0, nDelim);
            }
        }
    }

    if (sClass == "GtkListStore" || sClass == "GtkTreeStore")
    {
        handleListStore(reader, sID, sClass);
        return nullptr;
    }
    else if (sClass == "GtkMenu")
    {
        handleMenu(reader, sID);
        return nullptr;
    }
    else if (sClass == "GtkSizeGroup")
    {
        handleSizeGroup(reader);
        return nullptr;
    }
    else if (sClass == "AtkObject")
    {
        handleAtkObject(reader, pParent);
        return nullptr;
    }

    int nLevel = 1;

    stringmap aProperties, aPangoAttributes;
    stringmap aAtkAttributes;
    std::vector<ComboBoxTextItem> aItems;

    if (!sCustomProperty.isEmpty())
        aProperties[OString("customproperty")] = sCustomProperty;

    VclPtr<vcl::Window> pCurrentChild;
    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            if (name.equals("child"))
            {
                if (!pCurrentChild)
                {
                    pCurrentChild = insertObject(pParent, sClass, sID,
                        aProperties, aPangoAttributes, aAtkAttributes);
                }
                handleChild(pCurrentChild, reader);
            }
            else if (name.equals("items"))
                aItems = handleItems(reader);
            else if (name.equals("style"))
            {
                int nPriority = 0;
                std::vector<vcl::EnumContext::Context> aContext = handleStyle(reader, nPriority);
                if (nPriority != 0)
                {
                    vcl::IPrioritable* pPrioritable = dynamic_cast<vcl::IPrioritable*>(pCurrentChild.get());
                    SAL_WARN_IF(!pPrioritable, "vcl", "priority set for not supported item");
                    if (pPrioritable)
                        pPrioritable->SetPriority(nPriority);
                }
                if (!aContext.empty())
                {
                    vcl::IContext* pContextControl = dynamic_cast<vcl::IContext*>(pCurrentChild.get());
                    SAL_WARN_IF(!pContextControl, "vcl", "context set for not supported item");
                    if (pContextControl)
                        pContextControl->SetContext(aContext);
                }
            }
            else
            {
                ++nLevel;
                if (name.equals("property"))
                    collectProperty(reader, aProperties);
                else if (name.equals("attribute"))
                    collectPangoAttribute(reader, aPangoAttributes);
                else if (name.equals("relation"))
                    collectAtkRelationAttribute(reader, aAtkAttributes);
                else if (name.equals("role"))
                    collectAtkRoleAttribute(reader, aAtkAttributes);
                else if (name.equals("action-widget"))
                    handleActionWidget(reader);
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    if (sClass == "GtkAdjustment")
    {
        m_pParserState->m_aAdjustments[sID] = aProperties;
        return nullptr;
    }
    else if (sClass == "GtkTextBuffer")
    {
        m_pParserState->m_aTextBuffers[sID] = aProperties;
        return nullptr;
    }

    if (!pCurrentChild)
    {
        pCurrentChild = insertObject(pParent, sClass, sID, aProperties,
            aPangoAttributes, aAtkAttributes);
    }

    if (!aItems.empty())
    {
        // try to fill-in the items
        if (!insertItems<ComboBox>(pCurrentChild, aProperties, m_aUserData, aItems))
            insertItems<ListBox>(pCurrentChild, aProperties, m_aUserData, aItems);
    }

    return pCurrentChild;
}

void VclBuilder::handlePacking(vcl::Window *pCurrent, vcl::Window *pParent, xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    int nLevel = 1;

    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("property"))
                applyPackingProperty(pCurrent, pParent, reader);
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }
}

void VclBuilder::applyPackingProperty(vcl::Window *pCurrent,
    vcl::Window *pParent,
    xmlreader::XmlReader &reader)
{
    if (!pCurrent)
        return;

    //ToolBoxItems are not true widgets just elements
    //of the ToolBox itself
    ToolBox *pToolBoxParent = nullptr;
    if (pCurrent == pParent)
        pToolBoxParent = dynamic_cast<ToolBox*>(pParent);

    xmlreader::Span name;
    int nsId;

    if (pCurrent->GetType() == WindowType::SCROLLWINDOW)
    {
        auto aFind = m_pParserState->m_aRedundantParentWidgets.find(VclPtr<vcl::Window>(pCurrent));
        if (aFind != m_pParserState->m_aRedundantParentWidgets.end())
        {
            pCurrent = aFind->second;
            assert(pCurrent);
        }
    }

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("name"))
        {
            name = reader.getAttributeValue(false);
            OString sKey(name.begin, name.length);
            sKey = sKey.replace('_', '-');
            reader.nextItem(
                xmlreader::XmlReader::Text::Raw, &name, &nsId);
            OString sValue(name.begin, name.length);

            if (sKey == "expand" || sKey == "resize")
            {
                bool bTrue = (!sValue.isEmpty() && (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1'));
                if (pToolBoxParent)
                    pToolBoxParent->SetItemExpand(m_pParserState->m_nLastToolbarId, bTrue);
                else
                    pCurrent->set_expand(bTrue);
                continue;
            }

            if (pToolBoxParent)
                continue;

            if (sKey == "fill")
            {
                bool bTrue = (!sValue.isEmpty() && (sValue[0] == 't' || sValue[0] == 'T' || sValue[0] == '1'));
                pCurrent->set_fill(bTrue);
            }
            else if (sKey == "pack-type")
            {
                VclPackType ePackType = (!sValue.isEmpty() && (sValue[0] == 'e' || sValue[0] == 'E')) ? VclPackType::End : VclPackType::Start;
                pCurrent->set_pack_type(ePackType);
            }
            else if (sKey == "left-attach")
            {
                pCurrent->set_grid_left_attach(sValue.toInt32());
            }
            else if (sKey == "top-attach")
            {
                pCurrent->set_grid_top_attach(sValue.toInt32());
            }
            else if (sKey == "width")
            {
                pCurrent->set_grid_width(sValue.toInt32());
            }
            else if (sKey == "height")
            {
                pCurrent->set_grid_height(sValue.toInt32());
            }
            else if (sKey == "padding")
            {
                pCurrent->set_padding(sValue.toInt32());
            }
            else if (sKey == "position")
            {
                set_window_packing_position(pCurrent, sValue.toInt32());
            }
            else if (sKey == "secondary")
            {
                pCurrent->set_secondary(toBool(sValue));
            }
            else if (sKey == "non-homogeneous")
            {
                pCurrent->set_non_homogeneous(toBool(sValue));
            }
            else if (sKey == "homogeneous")
            {
                pCurrent->set_non_homogeneous(!toBool(sValue));
            }
            else
            {
                SAL_WARN("vcl.layout", "unknown packing: " << sKey);
            }
        }
    }
}

std::vector<vcl::EnumContext::Context> VclBuilder::handleStyle(xmlreader::XmlReader &reader, int &nPriority)
{
    std::vector<vcl::EnumContext::Context> aContext;

    xmlreader::Span name;
    int nsId;

    int nLevel = 1;

    while(true)
    {
        xmlreader::XmlReader::Result res = reader.nextItem(
            xmlreader::XmlReader::Text::NONE, &name, &nsId);

        if (res == xmlreader::XmlReader::Result::Done)
            break;

        if (res == xmlreader::XmlReader::Result::Begin)
        {
            ++nLevel;
            if (name.equals("class"))
            {
                OString classStyle = getStyleClass(reader);

                if (classStyle.startsWith("context-"))
                {
                    OString sContext = classStyle.copy(classStyle.indexOf('-') + 1);
                    OUString sContext2 = OUString(sContext.getStr(), sContext.getLength(), RTL_TEXTENCODING_UTF8);
                    aContext.push_back(vcl::EnumContext::GetContextEnum(sContext2));
                }
                else if (classStyle.startsWith("priority-"))
                {
                    OString aPriority = classStyle.copy(classStyle.indexOf('-') + 1);
                    OUString aPriority2 = OUString(aPriority.getStr(), aPriority.getLength(), RTL_TEXTENCODING_UTF8);
                    nPriority = aPriority2.toInt32();
                }
                else
                {
                    SAL_WARN("vcl.layout", "unknown class: " << classStyle);
                }
            }
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    return aContext;
}

OString VclBuilder::getStyleClass(xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;
    OString aRet;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("name"))
        {
            name = reader.getAttributeValue(false);
            aRet = OString (name.begin, name.length);
        }
    }

    return aRet;
}

void VclBuilder::collectProperty(xmlreader::XmlReader &reader, stringmap &rMap) const
{
    xmlreader::Span name;
    int nsId;

    OString sProperty, sContext;

    bool bTranslated = false;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("name"))
        {
            name = reader.getAttributeValue(false);
            sProperty = OString(name.begin, name.length);
        }
        else if (name.equals("context"))
        {
            name = reader.getAttributeValue(false);
            sContext = OString(name.begin, name.length);
        }
        else if (name.equals("translatable") && reader.getAttributeValue(false).equals("yes"))
        {
            bTranslated = true;
        }
    }

    reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
    OString sValue = OString(name.begin, name.length);
    OUString sFinalValue;
    if (bTranslated)
    {
        if (!sContext.isEmpty())
            sValue = sContext + "\004" + sValue;
        sFinalValue = Translate::get(sValue.getStr(), m_pParserState->m_aResLocale);
    }
    else
        sFinalValue = OUString::fromUtf8(sValue);

    if (!sProperty.isEmpty())
    {
        sProperty = sProperty.replace('_', '-');
        if (m_pStringReplace)
            sFinalValue = (*m_pStringReplace)(sFinalValue);
        rMap[sProperty] = sFinalValue;
    }
}

void VclBuilder::handleActionWidget(xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;

    OString sResponse;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("response"))
        {
            name = reader.getAttributeValue(false);
            sResponse = OString(name.begin, name.length);
        }
    }

    reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
    OString sID = OString(name.begin, name.length);
    sal_Int32 nDelim = sID.indexOf(':');
    if (nDelim != -1)
        sID = sID.copy(0, nDelim);
    set_response(sID, sResponse.toInt32());
}

void VclBuilder::collectAccelerator(xmlreader::XmlReader &reader, accelmap &rMap)
{
    xmlreader::Span name;
    int nsId;

    OString sProperty;
    OString sValue;
    OString sModifiers;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name.equals("key"))
        {
            name = reader.getAttributeValue(false);
            sValue = OString(name.begin, name.length);
        }
        else if (name.equals("signal"))
        {
            name = reader.getAttributeValue(false);
            sProperty = OString(name.begin, name.length);
        }
        else if (name.equals("modifiers"))
        {
            name = reader.getAttributeValue(false);
            sModifiers = OString(name.begin, name.length);
        }
    }

    if (!sProperty.isEmpty() && !sValue.isEmpty())
    {
        rMap[sProperty] = std::make_pair(sValue, sModifiers);
    }
}

vcl::Window *VclBuilder::get_widget_root()
{
    return m_aChildren.empty() ? nullptr : m_aChildren[0].m_pWindow.get();
}

vcl::Window *VclBuilder::get_by_name(const OString& sID)
{
    for (auto const& child : m_aChildren)
    {
        if (child.m_sID == sID)
            return child.m_pWindow;
    }

    return nullptr;
}

PopupMenu *VclBuilder::get_menu(const OString& sID)
{
    for (auto const& menu : m_aMenus)
    {
        if (menu.m_sID == sID)
            return menu.m_pMenu;
    }

    return nullptr;
}

void VclBuilder::set_response(const OString& sID, short nResponse)
{
    switch (nResponse)
    {
        case -5:
            nResponse = RET_OK;
            break;
        case -6:
            nResponse = RET_CANCEL;
            break;
        case -7:
            nResponse = RET_CLOSE;
            break;
        case -8:
            nResponse = RET_YES;
            break;
        case -9:
            nResponse = RET_NO;
            break;
        case -11:
            nResponse = RET_HELP;
            break;
    };

    assert(nResponse >= 0);

    for (auto & child : m_aChildren)
    {
        if (child.m_sID == sID)
        {
            PushButton* pPushButton = dynamic_cast<PushButton*>(child.m_pWindow.get());
            assert(pPushButton);
            Dialog* pDialog = pPushButton->GetParentDialog();
            assert(pDialog);
            pDialog->add_button(pPushButton, nResponse, false);
            return;
        }
    }

    assert(false);
}

void VclBuilder::delete_by_name(const OString& sID)
{
    auto aI = std::find_if(m_aChildren.begin(), m_aChildren.end(),
        [&sID](WinAndId& rItem) { return rItem.m_sID == sID; });
    if (aI != m_aChildren.end())
    {
        aI->m_pWindow.disposeAndClear();
        m_aChildren.erase(aI);
    }
}

void VclBuilder::delete_by_window(vcl::Window *pWindow)
{
    drop_ownership(pWindow);
    pWindow->disposeOnce();
}

void VclBuilder::drop_ownership(const vcl::Window *pWindow)
{
    auto aI = std::find_if(m_aChildren.begin(), m_aChildren.end(),
        [&pWindow](WinAndId& rItem) { return rItem.m_pWindow == pWindow; });
    if (aI != m_aChildren.end())
        m_aChildren.erase(aI);
}

OString VclBuilder::get_by_window(const vcl::Window *pWindow) const
{
    for (auto const& child : m_aChildren)
    {
        if (child.m_pWindow == pWindow)
            return child.m_sID;
    }

    return OString();
}

VclBuilder::PackingData VclBuilder::get_window_packing_data(const vcl::Window *pWindow) const
{
    //We've stored the return of new Control, some of these get
    //border windows placed around them which are what you get
    //from GetChild, so scoot up a level if necessary to get the
    //window whose position value we have
    const vcl::Window *pPropHolder = pWindow->ImplGetWindow();

    for (auto const& child : m_aChildren)
    {
        if (child.m_pWindow == pPropHolder)
            return child.m_aPackingData;
    }

    return PackingData();
}

void VclBuilder::set_window_packing_position(const vcl::Window *pWindow, sal_Int32 nPosition)
{
    for (auto & child : m_aChildren)
    {
        if (child.m_pWindow == pWindow)
            child.m_aPackingData.m_nPosition = nPosition;
    }
}

const VclBuilder::ListStore *VclBuilder::get_model_by_name(const OString& sID) const
{
    std::map<OString, ListStore>::const_iterator aI = m_pParserState->m_aModels.find(sID);
    if (aI != m_pParserState->m_aModels.end())
        return &(aI->second);
    return nullptr;
}

const VclBuilder::TextBuffer *VclBuilder::get_buffer_by_name(const OString& sID) const
{
    std::map<OString, TextBuffer>::const_iterator aI = m_pParserState->m_aTextBuffers.find(sID);
    if (aI != m_pParserState->m_aTextBuffers.end())
        return &(aI->second);
    return nullptr;
}

const VclBuilder::Adjustment *VclBuilder::get_adjustment_by_name(const OString& sID) const
{
    std::map<OString, Adjustment>::const_iterator aI = m_pParserState->m_aAdjustments.find(sID);
    if (aI != m_pParserState->m_aAdjustments.end())
        return &(aI->second);
    return nullptr;
}

void VclBuilder::mungeModel(ComboBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId)
{
    for (auto const& entry : rStore.m_aEntries)
    {
        const ListStore::row &rRow = entry;
        sal_uInt16 nEntry = rTarget.InsertEntry(rRow[0]);
        if (rRow.size() > 1)
        {
            if (m_bLegacy)
            {
                sal_IntPtr nValue = rRow[1].toInt32();
                rTarget.SetEntryData(nEntry, reinterpret_cast<void*>(nValue));
            }
            else
            {
                if (!rRow[1].isEmpty())
                {
                    m_aUserData.emplace_back(std::make_unique<OUString>(rRow[1]));
                    rTarget.SetEntryData(nEntry, m_aUserData.back().get());
                }
            }
        }
    }
    if (nActiveId < rStore.m_aEntries.size())
        rTarget.SelectEntryPos(nActiveId);
}

void VclBuilder::mungeModel(ListBox &rTarget, const ListStore &rStore, sal_uInt16 nActiveId)
{
    for (auto const& entry : rStore.m_aEntries)
    {
        const ListStore::row &rRow = entry;
        sal_uInt16 nEntry = rTarget.InsertEntry(rRow[0]);
        if (rRow.size() > 1)
        {
            if (m_bLegacy)
            {
                sal_IntPtr nValue = rRow[1].toInt32();
                rTarget.SetEntryData(nEntry, reinterpret_cast<void*>(nValue));
            }
            else
            {
                if (!rRow[1].isEmpty())
                {
                    m_aUserData.emplace_back(std::make_unique<OUString>(rRow[1]));
                    rTarget.SetEntryData(nEntry, m_aUserData.back().get());
                }
            }
        }
    }
    if (nActiveId < rStore.m_aEntries.size())
        rTarget.SelectEntryPos(nActiveId);
}

void VclBuilder::mungeModel(SvTabListBox& rTarget, const ListStore &rStore, sal_uInt16 nActiveId)
{
    for (auto const& entry : rStore.m_aEntries)
    {
        const ListStore::row &rRow = entry;
        auto pEntry = rTarget.InsertEntry(rRow[0]);
        if (rRow.size() > 1)
        {
            if (m_bLegacy)
            {
                sal_IntPtr nValue = rRow[1].toInt32();
                pEntry->SetUserData(reinterpret_cast<void*>(nValue));
            }
            else
            {
                if (!rRow[1].isEmpty())
                {
                    m_aUserData.emplace_back(std::make_unique<OUString>(rRow[1]));
                    pEntry->SetUserData(m_aUserData.back().get());
                }
            }
        }
    }
    if (nActiveId < rStore.m_aEntries.size())
    {
        SvTreeListEntry* pEntry = rTarget.GetEntry(nullptr, nActiveId);
        rTarget.Select(pEntry);
    }
}

void VclBuilder::mungeAdjustment(NumericFormatter &rTarget, const Adjustment &rAdjustment)
{
    int nMul = rtl_math_pow10Exp(1, rTarget.GetDecimalDigits());

    for (auto const& elem : rAdjustment)
    {
        const OString &rKey = elem.first;
        const OUString &rValue = elem.second;

        if (rKey == "upper")
        {
            sal_Int64 nUpper = rValue.toDouble() * nMul;
            rTarget.SetMax(nUpper);
            rTarget.SetLast(nUpper);
        }
        else if (rKey == "lower")
        {
            sal_Int64 nLower = rValue.toDouble() * nMul;
            rTarget.SetMin(nLower);
            rTarget.SetFirst(nLower);
        }
        else if (rKey == "value")
        {
            sal_Int64 nValue = rValue.toDouble() * nMul;
            rTarget.SetValue(nValue);
        }
        else if (rKey == "step-increment")
        {
            sal_Int64 nSpinSize = rValue.toDouble() * nMul;
            rTarget.SetSpinSize(nSpinSize);
        }
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey);
        }
    }
}

void VclBuilder::mungeAdjustment(FormattedField &rTarget, const Adjustment &rAdjustment)
{
    for (auto const& elem : rAdjustment)
    {
        const OString &rKey = elem.first;
        const OUString &rValue = elem.second;

        if (rKey == "upper")
        {
            rTarget.SetMaxValue(rValue.toDouble());
        }
        else if (rKey == "lower")
        {
            rTarget.SetMinValue(rValue.toDouble());
        }
        else if (rKey == "value")
        {
            rTarget.SetValue(rValue.toDouble());
        }
        else if (rKey == "step-increment")
        {
            rTarget.SetSpinSize(rValue.toDouble());
        }
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey);
        }
    }
}

void VclBuilder::mungeAdjustment(TimeField &rTarget, const Adjustment &rAdjustment)
{
    for (auto const& elem : rAdjustment)
    {
        const OString &rKey = elem.first;
        const OUString &rValue = elem.second;

        if (rKey == "upper")
        {
            tools::Time aUpper(rValue.toInt32());
            rTarget.SetMax(aUpper);
            rTarget.SetLast(aUpper);
        }
        else if (rKey == "lower")
        {
            tools::Time aLower(rValue.toInt32());
            rTarget.SetMin(aLower);
            rTarget.SetFirst(aLower);
        }
        else if (rKey == "value")
        {
            tools::Time aValue(rValue.toInt32());
            rTarget.SetTime(aValue);
        }
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey);
        }
    }
}

void VclBuilder::mungeAdjustment(DateField &rTarget, const Adjustment &rAdjustment)
{
    for (auto const& elem : rAdjustment)
    {
        const OString &rKey = elem.first;
        const OUString &rValue = elem.second;

        if (rKey == "upper")
        {
            Date aUpper(rValue.toInt32());
            rTarget.SetMax(aUpper);
            rTarget.SetLast(aUpper);
        }
        else if (rKey == "lower")
        {
            Date aLower(rValue.toInt32());
            rTarget.SetMin(aLower);
            rTarget.SetFirst(aLower);
        }
        else if (rKey == "value")
        {
            Date aValue(rValue.toInt32());
            rTarget.SetDate(aValue);
        }
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey);
        }
    }
}

void VclBuilder::mungeAdjustment(ScrollBar &rTarget, const Adjustment &rAdjustment)
{
    for (auto const& elem : rAdjustment)
    {
        const OString &rKey = elem.first;
        const OUString &rValue = elem.second;

        if (rKey == "upper")
            rTarget.SetRangeMax(rValue.toInt32());
        else if (rKey == "lower")
            rTarget.SetRangeMin(rValue.toInt32());
        else if (rKey == "value")
            rTarget.SetThumbPos(rValue.toInt32());
        else if (rKey == "step-increment")
            rTarget.SetLineSize(rValue.toInt32());
        else if (rKey == "page-increment")
            rTarget.SetPageSize(rValue.toInt32());
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey);
        }
    }
}

void VclBuilder::mungeAdjustment(Slider& rTarget, const Adjustment& rAdjustment)
{
    for (auto const& elem : rAdjustment)
    {
        const OString &rKey = elem.first;
        const OUString &rValue = elem.second;

        if (rKey == "upper")
            rTarget.SetRangeMax(rValue.toInt32());
        else if (rKey == "lower")
            rTarget.SetRangeMin(rValue.toInt32());
        else if (rKey == "value")
            rTarget.SetThumbPos(rValue.toInt32());
        else if (rKey == "step-increment")
            rTarget.SetLineSize(rValue.toInt32());
        else if (rKey == "page-increment")
            rTarget.SetPageSize(rValue.toInt32());
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey);
        }
    }
}

void VclBuilder::mungeTextBuffer(VclMultiLineEdit &rTarget, const TextBuffer &rTextBuffer)
{
    for (auto const& elem : rTextBuffer)
    {
        const OString &rKey = elem.first;
        const OUString &rValue = elem.second;

        if (rKey == "text")
            rTarget.SetText(rValue);
        else
        {
            SAL_INFO("vcl.layout", "unhandled property :" << rKey);
        }
    }
}

VclBuilder::ParserState::ParserState()
    : m_nLastToolbarId(0)
    , m_nLastMenuItemId(0)
{}

VclBuilder::MenuAndId::MenuAndId(const OString &rId, PopupMenu *pMenu)
            : m_sID(rId)
            , m_pMenu(pMenu)
{};

VclBuilder::MenuAndId::~MenuAndId() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
