/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_feature_desktop.h>
#include <config_options.h>

#include <memory>
#include <unordered_map>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <comphelper/lok.hxx>
#include <i18nutil/unicode.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/module.hxx>
#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/builder.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/toolkit/field.hxx>
#include <vcl/fieldvalues.hxx>
#include <vcl/toolkit/fmtfield.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/toolkit/fixedhyper.hxx>
#include <vcl/headbar.hxx>
#include <vcl/IPrioritable.hxx>
#include <vcl/toolkit/ivctrl.hxx>
#include <vcl/layout.hxx>
#include <vcl/toolkit/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/toolkit/prgsbar.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/split.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolkit/svtabbx.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/toolkit/throbber.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/toolkit/vclmedit.hxx>
#include <vcl/settings.hxx>
#include <slider.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <iconview.hxx>
#include <svdata.hxx>
#include <bitmaps.hlst>
#include <messagedialog.hxx>
#include <OptionalBox.hxx>
#include <window.h>
#include <xmlreader/xmlreader.hxx>
#include <desktop/crashreport.hxx>
#include <calendar.hxx>
#include <salinst.hxx>
#include <strings.hrc>
#include <treeglue.hxx>
#include <tools/diagnose_ex.h>
#include <verticaltabctrl.hxx>
#include <wizdlg.hxx>
#include <tools/svlibrary.h>
#include <jsdialog/jsdialogbuilder.hxx>

#if defined(DISABLE_DYNLOADING) || defined(LINUX)
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
            return SV_RESID_BITMAP_INDEX;
        else if (sType == "gtk-refresh")
            return SV_RESID_BITMAP_REFRESH;
        else if (sType == "gtk-apply")
            return IMG_APPLY;
        else if (sType == "gtk-dialog-error")
            return IMG_ERROR;
        else if (sType == "gtk-add")
            return IMG_ADD;
        else if (sType == "gtk-remove")
            return IMG_REMOVE;
        else if (sType == "gtk-copy")
            return IMG_COPY;
        else if (sType == "gtk-paste")
            return IMG_PASTE;
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
        else if (sType == "gtk-new")
            eRet = SymbolType::PLUS;
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

weld::Builder* Application::CreateBuilder(weld::Widget* pParent, const OUString &rUIFile, bool bMobile)
{
    bool bUseJSBuilder = false;

    if (bMobile)
    {
        if (rUIFile == "modules/swriter/ui/wordcount-mobile.ui" ||
            rUIFile == "svx/ui/findreplacedialog-mobile.ui" ||
            rUIFile == "modules/swriter/ui/watermarkdialog.ui" ||
            rUIFile == "modules/scalc/ui/validationdialog.ui" ||
            rUIFile == "modules/scalc/ui/validationcriteriapage.ui" ||
            rUIFile == "modules/scalc/ui/validationhelptabpage-mobile.ui" ||
            rUIFile == "modules/scalc/ui/erroralerttabpage-mobile.ui" ||
            rUIFile == "modules/scalc/ui/validationdialog.ui")
            bUseJSBuilder = true;
    }

    if (bUseJSBuilder)
        return new JSInstanceBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile);
    else
        return ImplGetSVData()->mpDefInst->CreateBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile);
}

weld::Builder* Application::CreateInterimBuilder(vcl::Window* pParent, const OUString &rUIFile, bool bAllowCycleFocusOut, sal_uInt64 nLOKWindowId)
{
    if (comphelper::LibreOfficeKit::isActive()
        && (rUIFile == "svx/ui/stylespreview.ui"
        || rUIFile == "modules/scalc/ui/numberbox.ui"))
    {
        return new JSInstanceBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile, css::uno::Reference<css::frame::XFrame>(), nLOKWindowId);
    }

    return ImplGetSVData()->mpDefInst->CreateInterimBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile, bAllowCycleFocusOut, nLOKWindowId);
}

weld::MessageDialog* Application::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType,
                                                      VclButtonsType eButtonType, const OUString& rPrimaryMessage,
                                                      bool bMobile)
{
    if (bMobile)
        return JSInstanceBuilder::CreateMessageDialog(pParent, eMessageType, eButtonType, rPrimaryMessage);
    else
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
        OUString sNewText(format_number(rSpinButton.get_value()));
        if (sNewText != rSpinButton.get_text())
            rSpinButton.set_text(sNewText);
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
            OUString aSuffix = MetricToString(m_eSrcUnit);
            if (m_eSrcUnit != FieldUnit::NONE && m_eSrcUnit != FieldUnit::DEGREE && m_eSrcUnit != FieldUnit::INCH && m_eSrcUnit != FieldUnit::FOOT)
                aStr += " ";
            if (m_eSrcUnit == FieldUnit::INCH)
            {
                OUString sDoublePrime = u"\u2033";
                if (aSuffix != "\"" && aSuffix != sDoublePrime)
                    aStr += " ";
                else
                    aSuffix = sDoublePrime;
            }
            else if (m_eSrcUnit == FieldUnit::FOOT)
            {
                OUString sPrime = u"\u2032";
                if (aSuffix != "'" && aSuffix != sPrime)
                    aStr += " ";
                else
                    aSuffix = sPrime;
            }

            assert(m_eSrcUnit != FieldUnit::PERCENT);
            aStr += aSuffix;
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
        auto nRet = vcl::ConvertValue(nValue, 0, m_xSpinButton->get_digits(), eInUnit, eOutUnit);
        if (nRet > SAL_MAX_INT32)
            nRet = SAL_MAX_INT32;
        else if (nRet < SAL_MIN_INT32)
            nRet = SAL_MIN_INT32;
        return nRet;
    }

    IMPL_LINK(MetricSpinButton, spin_button_input, int*, result, bool)
    {
        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        double fResult(0.0);
        bool bRet = vcl::TextToValue(get_text(), fResult, 0, m_xSpinButton->get_digits(), rLocaleData, m_eSrcUnit);
        if (bRet)
        {
            if (fResult > SAL_MAX_INT32)
                fResult = SAL_MAX_INT32;
            else if (fResult < SAL_MIN_INT32)
                fResult = SAL_MIN_INT32;
            *result = fResult;
        }
        return bRet;
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

    size_t GetAbsPos(const weld::TreeView& rTreeView, const weld::TreeIter& rIter)
    {
        size_t nAbsPos = 0;

        std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator(&rIter));
        if (!rTreeView.get_iter_first(*xEntry))
            xEntry.reset();

        while (xEntry && rTreeView.iter_compare(*xEntry, rIter) != 0)
        {
            if (!rTreeView.iter_next(*xEntry))
                xEntry.reset();
            nAbsPos++;
        }

        return nAbsPos;
    }

    bool IsEntryVisible(const weld::TreeView& rTreeView, const weld::TreeIter& rIter)
    {
        // short circuit for the common case
        if (rTreeView.get_iter_depth(rIter) == 0)
            return true;

        std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator(&rIter));
        bool bRetVal = false;
        do
        {
            if (rTreeView.get_iter_depth(*xEntry) == 0)
            {
                bRetVal = true;
                break;
            }
        }  while (rTreeView.iter_parent(*xEntry) && rTreeView.get_row_expanded(*xEntry));
        return bRetVal;
    }
}

VclBuilder::VclBuilder(vcl::Window* pParent, const OUString& sUIDir, const OUString& sUIFile,
                       const OString& sID, const css::uno::Reference<css::frame::XFrame>& rFrame,
                       bool bLegacy, const NotebookBarAddonsItem* pNotebookBarAddonsItem)
    : m_pNotebookBarAddonsItem(pNotebookBarAddonsItem
                                   ? new NotebookBarAddonsItem(*pNotebookBarAddonsItem)
                                   : new NotebookBarAddonsItem{})
    , m_sID(sID)
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
    m_sHelpRoot += OString('/');

    OUString sUri = sUIDir + sUIFile;

    try
    {
        xmlreader::XmlReader reader(sUri);

        handleChild(pParent, nullptr, reader);
    }
    catch (const css::uno::Exception &rExcept)
    {
        DBG_UNHANDLED_EXCEPTION("vcl.builder", "Unable to read .ui file");
        CrashReporter::addKeyValue("VclBuilderException", "Unable to read .ui file: " + rExcept.Message, CrashReporter::Write);
        throw;
    }

    //Set Mnemonic widgets when everything has been imported
    for (auto const& mnemonicWidget : m_pParserState->m_aMnemonicWidgetMaps)
    {
        FixedText *pOne = get<FixedText>(mnemonicWidget.m_sID);
        vcl::Window *pOther = get(mnemonicWidget.m_sValue.toUtf8());
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
                vcl::Window *pTarget = get(rParam.toUtf8());
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
                    SAL_WARN("vcl.builder", "unhandled a11y relation :" << rType);
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
        vcl::Window* pTarget = get(elem.m_sID);
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
        NumericFormatter *pTarget = dynamic_cast<NumericFormatter*>(get(elem.m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget, "vcl", "missing NumericFormatter element of spinbutton/adjustment");
        SAL_WARN_IF(!pAdjustment, "vcl", "missing Adjustment element of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (auto const& elem : m_pParserState->m_aFormattedFormatterAdjustmentMaps)
    {
        FormattedField *pTarget = dynamic_cast<FormattedField*>(get(elem.m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue.toUtf8());
        SAL_WARN_IF(!pTarget, "vcl", "missing FormattedField element of spinbutton/adjustment");
        SAL_WARN_IF(!pAdjustment, "vcl", "missing Adjustment element of spinbutton/adjustment");
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
        Slider* pTarget = dynamic_cast<Slider*>(get(elem.m_sID));
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
            vcl::Window* pWindow = get(elem.getStr());
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
            {
                pTargetButton->SetModeImage(pImage->GetImage());
                if (pImage->GetStyle() & WB_SMALLSTYLE)
                {
                    pTargetButton->SetStyle(pTargetButton->GetStyle() | WB_SMALLSTYLE);
                    Size aSz(pTargetButton->GetModeImage().GetSizePixel());
                    aSz.AdjustWidth(6);
                    aSz.AdjustHeight(6);
                    if (pTargetButton->get_width_request() == -1)
                        pTargetButton->set_width_request(aSz.Width());
                    if (pTargetButton->get_height_request() == -1)
                        pTargetButton->set_height_request(aSz.Height());
                }
            }
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
                SAL_WARN_IF(eType != SymbolType::IMAGE, "vcl.builder", "unimplemented symbol type for radiobuttons");
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
                    SAL_WARN("vcl.builder", "unsupported image size " << rImageInfo.m_nSize);
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
        SAL_WARN_IF(!pImage, "vcl", "missing elements of image/stock: " << elem.first);
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

    SAL_WARN_IF(!m_sID.isEmpty() && (!m_bToplevelParentFound && !get_by_name(m_sID)), "vcl.builder",
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
        SAL_WARN_IF(nButtons && !bHasDefButton, "vcl.builder", "No default button defined in " << sUIFile);
    }
#endif

    const bool bHideHelp = comphelper::LibreOfficeKit::isActive() &&
        officecfg::Office::Common::Help::HelpRootURL::get().isEmpty();
    if (bHideHelp)
    {
        if (vcl::Window *pHelpButton = get("help"))
            pHelpButton->Hide();
    }
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
    bool extractHasFrame(VclBuilder::stringmap& rMap)
    {
        bool bHasFrame = true;
        VclBuilder::stringmap::iterator aFind = rMap.find("has-frame");
        if (aFind != rMap.end())
        {
            bHasFrame = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bHasFrame;
    }

    bool extractDrawValue(VclBuilder::stringmap& rMap)
    {
        bool bDrawValue = true;
        VclBuilder::stringmap::iterator aFind = rMap.find("draw-value");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("popup");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("value-pos");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("type-hint");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("resizable");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("modal");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("decorated");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("deletable");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("has-entry");
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
        VclBuilder::stringmap::iterator aFind = rMap.find("orientation");
        if (aFind != rMap.end())
        {
            bVertical = aFind->second.equalsIgnoreAsciiCase("vertical");
            rMap.erase(aFind);
        }
        return bVertical;
    }

    bool extractVerticalTabPos(VclBuilder::stringmap &rMap)
    {
        bool bVertical = false;
        VclBuilder::stringmap::iterator aFind = rMap.find("tab-pos");
        if (aFind != rMap.end())
        {
            bVertical = aFind->second.equalsIgnoreAsciiCase("left") ||
                        aFind->second.equalsIgnoreAsciiCase("right");
            rMap.erase(aFind);
        }
        return bVertical;
    }

    bool extractInconsistent(VclBuilder::stringmap &rMap)
    {
        bool bInconsistent = false;
        VclBuilder::stringmap::iterator aFind = rMap.find("inconsistent");
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

    OUString extractStockId(VclBuilder::stringmap &rMap)
    {
        OUString sIconName;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("stock-id"));
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
        SAL_WARN("vcl.builder", "unknown stock type: " << rType);
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
        bool bRet = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(OString("visible"));
        if (aFind != rMap.end())
        {
            bRet = toBool(aFind->second);
            rMap.erase(aFind);
        }
        return bRet;
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
        aFind = rMap.find("height-request");
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
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(aCommand, aModuleName);
        OUString aLabel(vcl::CommandInfoProvider::GetLabelForCommand(aProperties));
        if (!aLabel.isEmpty())
            pButton->SetText(aLabel);

        OUString aTooltip(vcl::CommandInfoProvider::GetTooltipForCommand(aCommand, aProperties, rFrame));
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

    WinBits extractDeferredBits(VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_3DLOOK|WB_HIDE;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        if (extractCloseable(rMap))
            nBits |= WB_CLOSEABLE;
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
    if (aFind == rMap.end())
        return;

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
        rWinStyle |= nScrollBits;
        if (static_cast<VclScrolledWindow*>(pParent)->HasVisibleBorder())
            rWinStyle |= WB_BORDER;
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

namespace {

// Don't unload the module on destruction
class NoAutoUnloadModule : public osl::Module
{
public:
    ~NoAutoUnloadModule() { release(); }
};

}

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
    for (const auto & lib : aWidgetLibs)
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

namespace
{
// Takes a string like "sfxlo-SidebarToolBox"
VclBuilder::customMakeWidget GetCustomMakeWidget(const OString& name)
{
    VclBuilder::customMakeWidget pFunction = nullptr;
    if (sal_Int32 nDelim = name.indexOf('-'); nDelim != -1)
    {
        const OString aFunction("make" + name.copy(nDelim + 1));
        const OUString sFunction(OStringToOUString(aFunction, RTL_TEXTENCODING_UTF8));

#ifndef DISABLE_DYNLOADING
        const OUString sModule = SAL_DLLPREFIX
                                 + OStringToOUString(name.copy(0, nDelim), RTL_TEXTENCODING_UTF8)
                                 + SAL_DLLEXTENSION;
        ModuleMap::iterator aI = g_aModuleMap.find(sModule);
        if (aI == g_aModuleMap.end())
        {
            std::shared_ptr<NoAutoUnloadModule> pModule;
#if ENABLE_MERGELIBS
            if (!g_pMergedLib->is())
                g_pMergedLib->loadRelative(&thisModule, SVLIBRARY("merged"));
            if ((pFunction = reinterpret_cast<VclBuilder::customMakeWidget>(
                     g_pMergedLib->getFunctionSymbol(sFunction))))
                pModule = g_pMergedLib;
#endif
            if (!pFunction)
            {
                pModule = std::make_shared<NoAutoUnloadModule>();
                bool ok = pModule->loadRelative(&thisModule, sModule);
                if (!ok)
                {
#ifdef LINUX
                    // in the case of preloading, we don't have eg. the
                    // libcuilo.so, but still need to dlsym the symbols -
                    // which are already in-process
                    if (comphelper::LibreOfficeKit::isActive())
                    {
                        pFunction = reinterpret_cast<VclBuilder::customMakeWidget>(dlsym(RTLD_DEFAULT, aFunction.getStr()));
                        ok = !!pFunction;
                        assert(ok && "couldn't even directly dlsym the sFunction (available via preload)");
                    }
#endif
                    assert(ok && "bad module name in .ui");
                }
                else
                {
                    pFunction = reinterpret_cast<VclBuilder::customMakeWidget>(
                            pModule->getFunctionSymbol(sFunction));
                }
            }
            g_aModuleMap.insert(std::make_pair(sModule, pModule));
        }
        else
            pFunction = reinterpret_cast<VclBuilder::customMakeWidget>(
                aI->second->getFunctionSymbol(sFunction));
#elif !HAVE_FEATURE_DESKTOP
        pFunction = lo_get_custom_widget_func(sFunction.toUtf8().getStr());
        SAL_WARN_IF(!pFunction, "vcl.builder", "Could not find " << sFunction);
        assert(pFunction);
#else
        pFunction = reinterpret_cast<VclBuilder::customMakeWidget>(
            osl_getFunctionSymbol((oslModule)RTLD_DEFAULT, sFunction.pData));
#endif
    }
    return pFunction;
}
}

VclPtr<vcl::Window> VclBuilder::makeObject(vcl::Window *pParent, const OString &name, const OString &id,
    stringmap &rMap)
{
    bool bIsPlaceHolder = name.isEmpty();
    bool bVertical = false;

    if (pParent && (pParent->GetType() == WindowType::TABCONTROL ||
                    pParent->GetType() == WindowType::VERTICALTABCONTROL))
    {
        bool bTopLevel(name == "GtkDialog" || name == "GtkMessageDialog" ||
                       name == "GtkWindow" || name == "GtkPopover" || name == "GtkAssistant");
        if (!bTopLevel)
        {
            if (pParent->GetType() == WindowType::TABCONTROL)
            {
                //We have to add a page
                //make default pageid == position
                TabControl *pTabControl = static_cast<TabControl*>(pParent);
                sal_uInt16 nNewPageCount = pTabControl->GetPageCount()+1;
                sal_uInt16 nNewPageId = nNewPageCount;
                pTabControl->InsertPage(nNewPageId, OUString());
                pTabControl->SetCurPageId(nNewPageId);
                SAL_WARN_IF(bIsPlaceHolder, "vcl.builder", "we should have no placeholders for tabpages");
                if (!bIsPlaceHolder)
                {
                    VclPtrInstance<TabPage> pPage(pTabControl);
                    pPage->Show();

                    //Make up a name for it
                    OString sTabPageId = get_by_window(pParent) +
                        "-page" +
                        OString::number(nNewPageCount);
                    m_aChildren.emplace_back(sTabPageId, pPage, false);
                    pPage->SetHelpId(m_sHelpRoot + sTabPageId);

                    pParent = pPage;

                    pTabControl->SetTabPage(nNewPageId, pPage);
                }
            }
            else
            {
                VerticalTabControl *pTabControl = static_cast<VerticalTabControl*>(pParent);
                SAL_WARN_IF(bIsPlaceHolder, "vcl.builder", "we should have no placeholders for tabpages");
                if (!bIsPlaceHolder)
                    pParent = pTabControl->GetPageParent();
            }
        }
    }

    if (bIsPlaceHolder || name == "GtkTreeSelection")
        return nullptr;

    ToolBox *pToolBox = (pParent && pParent->GetType() == WindowType::TOOLBOX) ? static_cast<ToolBox*>(pParent) : nullptr;

    extractButtonImage(id, rMap, name == "GtkRadioButton");

    VclPtr<vcl::Window> xWindow;
    if (name == "GtkDialog" || name == "GtkAssistant")
    {
        // WB_ALLOWMENUBAR because we don't know in advance if we will encounter
        // a menubar, and menubars need a BorderWindow in the toplevel, and
        // such border windows need to be in created during the dialog ctor
        WinBits nBits = WB_MOVEABLE|WB_3DLOOK|WB_ALLOWMENUBAR;
        if (extractResizable(rMap))
            nBits |= WB_SIZEABLE;
        if (extractCloseable(rMap))
            nBits |= WB_CLOSEABLE;
        Dialog::InitFlag eInit = !pParent ? Dialog::InitFlag::NoParent : Dialog::InitFlag::Default;
        if (name == "GtkAssistant")
            xWindow = VclPtr<vcl::RoadmapWizard>::Create(pParent, nBits, eInit);
        else
            xWindow = VclPtr<Dialog>::Create(pParent, nBits, eInit);
#if HAVE_FEATURE_DESKTOP
        if (!extractModal(rMap))
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
#if defined _WIN32
        xWindow->set_border_width(3);
#else
        xWindow->set_border_width(12);
#endif
    }
    else if (name == "GtkBox" || name == "GtkStatusbar")
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
        if (bVertical)
            xWindow = VclPtr<VclVPaned>::Create(pParent);
        else
            xWindow = VclPtr<VclHPaned>::Create(pParent);
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
        bool bIsTriState = extractInconsistent(rMap);
        VclPtr<CheckBox> xCheckBox = VclPtr<CheckBox>::Create(pParent, nBits);
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

        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_3DLOOK|WB_SPIN|WB_REPEAT;
        if (extractHasFrame(rMap))
            nBits |= WB_BORDER;

        connectFormattedFormatterAdjustment(id, sAdjustment);
        VclPtrInstance<FormattedField> xField(pParent, nBits);
        xField->GetFormatter().SetMinValue(0);
        xWindow = xField;
    }
    else if (name == "GtkLinkButton")
        xWindow = VclPtr<FixedHyperlink>::Create(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_NOLABEL);
    else if (name == "GtkComboBox" || name == "GtkComboBoxText")
    {
        extractModel(id, rMap);

        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

        bool bDropdown = BuilderUtils::extractDropdown(rMap);

        if (bDropdown)
            nBits |= WB_DROPDOWN;

        if (extractEntry(rMap))
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
    else if (name == "VclOptionalBox" || name == "sfxlo-OptionalBox")
    {
        // tdf#135495 fallback sfxlo-OptionalBox to VclOptionalBox as a stopgap
        xWindow = VclPtr<OptionalBox>::Create(pParent);
    }
    else if (name == "GtkIconView")
    {
        assert(rMap.find(OString("model")) != rMap.end() && "GtkIconView must have a model");

        //window we want to apply the packing props for this GtkIconView to
        VclPtr<vcl::Window> xWindowForPackingProps;
        extractModel(id, rMap);
        WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;
        //IconView manages its own scrolling,
        vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);

        VclPtr<IconView> xBox = VclPtr<IconView>::Create(pRealParent, nWinStyle);
        xWindowForPackingProps = xBox;

        xWindow = xBox;
        xBox->SetNoAutoCurEntry(true);
        xBox->SetQuickSearch(true);

        if (pRealParent != pParent)
            cleanupWidgetOwnScrolling(pParent, xWindowForPackingProps, rMap);
    }
    else if (name == "GtkTreeView")
    {
        if (!m_bLegacy)
        {
            assert(rMap.find(OString("model")) != rMap.end() && "GtkTreeView must have a model");
        }

        //window we want to apply the packing props for this GtkTreeView to
        VclPtr<vcl::Window> xWindowForPackingProps;
        //To-Do
        //a) make SvHeaderTabListBox/SvTabListBox the default target for GtkTreeView
        //b) remove the non-drop down mode of ListBox and convert
        //   everything over to SvHeaderTabListBox/SvTabListBox
        //c) remove the users of makeSvTabListBox and makeSvTreeListBox
        extractModel(id, rMap);
        WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;
        if (m_bLegacy)
        {
            OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
            if (!sBorder.isEmpty())
                nWinStyle |= WB_BORDER;
        }
        else
        {
            nWinStyle |= WB_HASBUTTONS | WB_HASBUTTONSATROOT;
        }
        //ListBox/SvHeaderTabListBox manages its own scrolling,
        vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
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
                xHeader->set_width_request(0); // let the headerbar width not affect the size request
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
        WinBits nWinStyle = WB_LEFT|WB_VCENTER|WB_3DLOOK;
        if (extractHasFrame(rMap))
            nWinStyle |= WB_BORDER;
        xWindow = VclPtr<Edit>::Create(pParent, nWinStyle);
        BuilderUtils::ensureDefaultWidthChars(rMap);
    }
    else if (name == "GtkNotebook")
    {
        if (!extractVerticalTabPos(rMap))
            xWindow = VclPtr<TabControl>::Create(pParent, WB_STDTABCONTROL|WB_3DLOOK);
        else
            xWindow = VclPtr<VerticalTabControl>::Create(pParent);
    }
    else if (name == "GtkDrawingArea")
    {
        xWindow = VclPtr<VclDrawingArea>::Create(pParent, WB_TABSTOP);
    }
    else if (name == "GtkTextView")
    {
        extractBuffer(id, rMap);

        WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT;
        //VclMultiLineEdit manages its own scrolling,
        vcl::Window *pRealParent = prepareWidgetOwnScrolling(pParent, nWinStyle);
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
    else if(name == "NotebookBarAddonsToolMergePoint")
    {
        customMakeWidget pFunction = GetCustomMakeWidget("sfxlo-NotebookbarToolBox");
        if(pFunction != nullptr)
            NotebookBarAddonsMerger::MergeNotebookBarAddons(pParent, pFunction, m_xFrame, *m_pNotebookBarAddonsItem, rMap);
        return nullptr;
    }
    else if (name == "GtkToolButton" || name == "GtkMenuToolButton" ||
             name == "GtkToggleToolButton" || name == "GtkRadioToolButton" || name == "GtkToolItem")
    {
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
                if (aCommand.isEmpty() && !m_bLegacy)
                    aCommand = OUString::fromUtf8(id);
                pToolBox->SetItemCommand(nItemId, aCommand);
            }

            pToolBox->SetHelpId(nItemId, m_sHelpRoot + id);
            OUString sTooltip(extractTooltipText(rMap));
            if (!sTooltip.isEmpty())
                pToolBox->SetQuickHelpText(nItemId, sTooltip);

            OUString sIconName(extractIconName(rMap));
            if (sIconName.isEmpty())
                sIconName = mapStockToImageResource(extractStockId(rMap));
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
    else if (name == "GtkCalendar")
    {
        WinBits nBits = extractDeferredBits(rMap);
        xWindow = VclPtr<Calendar>::Create(pParent, nBits);
    }
    else
    {
        if (customMakeWidget pFunction = GetCustomMakeWidget(name))
        {
            pFunction(xWindow, pParent, rMap);
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

    SAL_INFO_IF(!xWindow, "vcl.builder", "probably need to implement " << name << " or add a make" << name << " function");
    if (xWindow)
    {
        // child windows of disabled windows are made disabled by vcl by default, we don't want that
        WindowImpl *pWindowImpl = xWindow->ImplGetWindowImpl();
        pWindowImpl->mbDisabled = false;

        xWindow->SetHelpId(m_sHelpRoot + id);
        SAL_INFO("vcl.builder", "for name '" << name << "' and id '" << id <<
            "', created " << xWindow.get() << " child of " <<
            pParent << "(" << xWindow->ImplGetWindowImpl()->mpParent.get() << "/" <<
            xWindow->ImplGetWindowImpl()->mpRealParent.get() << "/" <<
            xWindow->ImplGetWindowImpl()->mpBorderWindow.get() << ") with helpid " <<
            xWindow->GetHelpId());
        m_aChildren.emplace_back(id, xWindow, bVertical);

        // if the parent was a toolbox set it as an itemwindow for the latest itemid
        if (pToolBox)
        {
            Size aSize(xWindow->GetSizePixel());
            aSize.setHeight(xWindow->get_preferred_size().Height());
            xWindow->SetSizePixel(aSize);
            pToolBox->SetItemWindow(m_pParserState->m_nLastToolbarId, xWindow);
            pToolBox->SetItemExpand(m_pParserState->m_nLastToolbarId, true);
        }
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
            SAL_INFO("vcl.builder", "for toplevel dialog " << this << " " <<
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
    {
        bool bToolbarParent = (pParent && pParent->GetType() == WindowType::TOOLBOX);
        pCurrentChild = (m_aChildren.empty() || bToolbarParent) ? pParent : m_aChildren.back().m_pWindow.get();
    }
    return pCurrentChild;
}

void VclBuilder::handleTabChild(vcl::Window *pParent, xmlreader::XmlReader &reader)
{
    std::vector<OString> sIDs;

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
            if (name == "object")
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name == "id")
                    {
                        name = reader.getAttributeValue(false);
                        OString sID(name.begin, name.length);
                        sal_Int32 nDelim = sID.indexOf(':');
                        if (nDelim != -1)
                        {
                            OString sPattern = sID.copy(nDelim+1);
                            aProperties[OString("customproperty")] = OUString::fromUtf8(sPattern);
                            sID = sID.copy(0, nDelim);
                        }
                        sIDs.push_back(sID);
                    }
                }
            }
            else if (name == "style")
            {
                int nPriority = 0;
                context = handleStyle(reader, nPriority);
                --nLevel;
            }
            else if (name == "property")
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

    TabControl *pTabControl = pParent->GetType() == WindowType::TABCONTROL ?
        static_cast<TabControl*>(pParent) : nullptr;
    VerticalTabControl *pVerticalTabControl = pParent->GetType() == WindowType::VERTICALTABCONTROL ?
        static_cast<VerticalTabControl*>(pParent) : nullptr;
    assert(pTabControl || pVerticalTabControl);
    VclBuilder::stringmap::iterator aFind = aProperties.find(OString("label"));
    if (aFind != aProperties.end())
    {
        if (pTabControl)
        {
            sal_uInt16 nPageId = pTabControl->GetCurPageId();
            pTabControl->SetPageText(nPageId, aFind->second);
            pTabControl->SetPageName(nPageId, sIDs.back());
            if (!context.empty())
            {
                TabPage* pPage = pTabControl->GetTabPage(nPageId);
                pPage->SetContext(context);
            }
        }
        else
        {
            OUString sLabel(aFind->second);
            OUString sIconName(extractIconName(aProperties));
            OUString sTooltip(extractTooltipText(aProperties));
            pVerticalTabControl->InsertPage(sIDs.front(), sLabel, FixedImage::loadThemeImage(sIconName), sTooltip,
                                            pVerticalTabControl->GetPageParent()->GetWindow(GetWindowType::LastChild));
        }
    }
    else
    {
        if (pTabControl)
            pTabControl->RemovePage(pTabControl->GetCurPageId());
    }
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

void VclBuilder::handleChild(vcl::Window *pParent, stringmap* pAtkProps, xmlreader::XmlReader &reader)
{
    vcl::Window *pCurrentChild = nullptr;

    xmlreader::Span name;
    int nsId;
    OString sType, sInternalChild;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name == "type")
        {
            name = reader.getAttributeValue(false);
            sType = OString(name.begin, name.length);
        }
        else if (name == "internal-child")
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
            if (name == "object" || name == "placeholder")
            {
                pCurrentChild = handleObject(pParent, pAtkProps, reader).get();

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
            else if (name == "packing")
            {
                handlePacking(pCurrentChild, pParent, reader);
            }
            else if (name == "interface")
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name == "domain")
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
        if (span == "name")
        {
            span = reader.getAttributeValue(false);
            sProperty = OString(span.begin, span.length);
        }
        else if (span == "value")
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
        if (span == "type")
        {
            span = reader.getAttributeValue(false);
            sProperty = OString(span.begin, span.length);
        }
        else if (span == "target")
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
        if (span == "type")
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
            if (name == "col")
            {
                bool bTranslated = false;
                sal_uInt32 nId = 0;
                OString sContext;

                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name == "id")
                    {
                        name = reader.getAttributeValue(false);
                        nId = OString(name.begin, name.length).toInt32();
                    }
                    else if (nId == 0 && name == "translatable" && reader.getAttributeValue(false) == "yes")
                    {
                        bTranslated = true;
                    }
                    else if (name == "context")
                    {
                        name = reader.getAttributeValue(false);
                        sContext = OString(name.begin, name.length);
                    }
                }

                (void)reader.nextItem(
                    xmlreader::XmlReader::Text::Raw, &name, &nsId);

                OString sValue(name.begin, name.length);
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
            if (name == "row")
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

VclBuilder::stringmap VclBuilder::handleAtkObject(xmlreader::XmlReader &reader)
{
    int nLevel = 1;

    stringmap aProperties;

    while (true)
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
            if (name == "property")
                collectProperty(reader, aProperties);
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    return aProperties;
}

void VclBuilder::applyAtkProperties(vcl::Window *pWindow, const stringmap& rProperties)
{
    assert(pWindow);
    for (auto const& prop : rProperties)
    {
        const OString &rKey = prop.first;
        const OUString &rValue = prop.second;

        if (pWindow && rKey.match("AtkObject::"))
            pWindow->set_property(rKey.copy(RTL_CONSTASCII_LENGTH("AtkObject::")), rValue);
        else
            SAL_WARN("vcl.builder", "unhandled atk prop: " << rKey);
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
            if (name == "item")
            {
                bool bTranslated = false;
                OString sContext, sId;

                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name == "translatable" && reader.getAttributeValue(false) == "yes")
                    {
                        bTranslated = true;
                    }
                    else if (name == "context")
                    {
                        name = reader.getAttributeValue(false);
                        sContext = OString(name.begin, name.length);
                    }
                    else if (name == "id")
                    {
                        name = reader.getAttributeValue(false);
                        sId = OString(name.begin, name.length);
                    }
                }

                (void)reader.nextItem(
                    xmlreader::XmlReader::Text::Raw, &name, &nsId);

                OString sValue(name.begin, name.length);
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

VclPtr<Menu> VclBuilder::handleMenu(xmlreader::XmlReader &reader, const OString &rID, bool bMenuBar)
{
    VclPtr<Menu> pCurrentMenu;
    if (bMenuBar)
        pCurrentMenu = VclPtr<MenuBar>::Create();
    else
        pCurrentMenu = VclPtr<PopupMenu>::Create();

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
            if (name == "child")
            {
                handleMenuChild(pCurrentMenu, reader);
            }
            else
            {
                ++nLevel;
                if (name == "property")
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

    return pCurrentMenu;
}

void VclBuilder::handleMenuChild(Menu *pParent, xmlreader::XmlReader &reader)
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
            if (name == "object" || name == "placeholder")
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

void VclBuilder::handleMenuObject(Menu *pParent, xmlreader::XmlReader &reader)
{
    OString sClass;
    OString sID;
    OUString sCustomProperty;
    PopupMenu *pSubMenu = nullptr;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name == "class")
        {
            name = reader.getAttributeValue(false);
            sClass = OString(name.begin, name.length);
        }
        else if (name == "id")
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
    stringmap aAtkProperties;
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
            if (name == "child")
            {
                size_t nChildMenuIdx = m_aMenus.size();
                handleChild(nullptr, &aAtkProperties, reader);
                bool bSubMenuInserted = m_aMenus.size() > nChildMenuIdx;
                if (bSubMenuInserted)
                    pSubMenu = dynamic_cast<PopupMenu*>(m_aMenus[nChildMenuIdx].m_pMenu.get());
            }
            else
            {
                ++nLevel;
                if (name == "property")
                    collectProperty(reader, aProperties);
                else if (name == "accelerator")
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

    insertMenuObject(pParent, pSubMenu, sClass, sID, aProperties, aAtkProperties, aAccelerators);
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
            if (name == "widget")
            {
                while (reader.nextAttribute(&nsId, &name))
                {
                    if (name == "name")
                    {
                        name = reader.getAttributeValue(false);
                        OString sWidget(name.begin, name.length);
                        sal_Int32 nDelim = sWidget.indexOf(':');
                        if (nDelim != -1)
                            sWidget = sWidget.copy(0, nDelim);
                        rSizeGroup.m_aWidgets.push_back(sWidget);
                    }
                }
            }
            else
            {
                if (name == "property")
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
        else if (rKey.first == "Return")
            return vcl::KeyCode(KEY_RETURN, bShift, bMod1, bMod2, bMod3);
        else if (rKey.first == "Up")
            return vcl::KeyCode(KEY_UP, bShift, bMod1, bMod2, bMod3);
        else if (rKey.first == "Down")
            return vcl::KeyCode(KEY_DOWN, bShift, bMod1, bMod2, bMod3);
        else if (rKey.first == "Left")
            return vcl::KeyCode(KEY_LEFT, bShift, bMod1, bMod2, bMod3);
        else if (rKey.first == "Right")
            return vcl::KeyCode(KEY_RIGHT, bShift, bMod1, bMod2, bMod3);
        else if (rKey.first == "asterisk")
            return vcl::KeyCode(KEY_MULTIPLY, bShift, bMod1, bMod2, bMod3);

        assert (rKey.first.getLength() == 1);
        char cChar = rKey.first.toChar();

        if (cChar >= 'a' && cChar <= 'z')
            return vcl::KeyCode(KEY_A + (cChar - 'a'), bShift, bMod1, bMod2, bMod3);
        else if (cChar >= 'A' && cChar <= 'Z')
            return vcl::KeyCode(KEY_A + (cChar - 'A'), bShift, bMod1, bMod2, bMod3);
        else if (cChar >= '0' && cChar <= '9')
            return vcl::KeyCode(KEY_0 + (cChar - 'A'), bShift, bMod1, bMod2, bMod3);

        return vcl::KeyCode(cChar, bShift, bMod1, bMod2, bMod3);
    }
}

void VclBuilder::insertMenuObject(Menu *pParent, PopupMenu *pSubMenu, const OString &rClass, const OString &rID,
    stringmap &rProps, stringmap &rAtkProps, accelmap &rAccels)
{
    sal_uInt16 nOldCount = pParent->GetItemCount();
    sal_uInt16 nNewId = ++m_pParserState->m_nLastMenuItemId;

    if(rClass == "NotebookBarAddonsMenuMergePoint")
    {
        NotebookBarAddonsMerger::MergeNotebookBarMenuAddons(pParent, nNewId, rID, *m_pNotebookBarAddonsItem);
        m_pParserState->m_nLastMenuItemId = pParent->GetItemCount();
    }
    else if (rClass == "GtkMenuItem")
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

    SAL_WARN_IF(nOldCount == pParent->GetItemCount(), "vcl.builder", "probably need to implement " << rClass);

    if (nOldCount != pParent->GetItemCount())
    {
        pParent->SetHelpId(nNewId, m_sHelpRoot + rID);
        if (!extractVisible(rProps))
            pParent->HideItem(nNewId);

        for (auto const& prop : rProps)
        {
            const OString &rKey = prop.first;
            const OUString &rValue = prop.second;

            if (rKey == "tooltip-markup")
                pParent->SetTipHelpText(nNewId, rValue);
            else if (rKey == "tooltip-text")
                pParent->SetTipHelpText(nNewId, rValue);
            else
                SAL_INFO("vcl.builder", "unhandled property: " << rKey);
        }

        for (auto const& prop : rAtkProps)
        {
            const OString &rKey = prop.first;
            const OUString &rValue = prop.second;

            if (rKey == "AtkObject::accessible-name")
                pParent->SetAccessibleName(nNewId, rValue);
            else if (rKey == "AtkObject::accessible-description")
                pParent->SetAccessibleDescription(nNewId, rValue);
            else
                SAL_INFO("vcl.builder", "unhandled atk property: " << rKey);
        }

        for (auto const& accel : rAccels)
        {
            const OString &rSignal = accel.first;
            const auto &rValue = accel.second;

            if (rSignal == "activate")
                pParent->SetAccelKey(nNewId, makeKeyCode(rValue));
            else
                SAL_INFO("vcl.builder", "unhandled accelerator for: " << rSignal);
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

VclPtr<vcl::Window> VclBuilder::handleObject(vcl::Window *pParent, stringmap *pAtkProps, xmlreader::XmlReader &reader)
{
    OString sClass;
    OString sID;
    OUString sCustomProperty;

    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name == "class")
        {
            name = reader.getAttributeValue(false);
            sClass = OString(name.begin, name.length);
        }
        else if (name == "id")
        {
            name = reader.getAttributeValue(false);
            sID = OString(name.begin, name.length);
            if (m_bLegacy)
            {
                sal_Int32 nDelim = sID.indexOf(':');
                if (nDelim != -1)
                {
                    sCustomProperty = OUString::fromUtf8(sID.copy(nDelim+1));
                    sID = sID.copy(0, nDelim);
                }
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
        handleMenu(reader, sID, false);
        return nullptr;
    }
    else if (sClass == "GtkMenuBar")
    {
        VclPtr<Menu> xMenu = handleMenu(reader, sID, true);
        if (SystemWindow* pTopLevel = pParent ? pParent->GetSystemWindow() : nullptr)
            pTopLevel->SetMenuBar(dynamic_cast<MenuBar*>(xMenu.get()));
        return nullptr;
    }
    else if (sClass == "GtkSizeGroup")
    {
        handleSizeGroup(reader);
        return nullptr;
    }
    else if (sClass == "AtkObject")
    {
        assert((pParent || pAtkProps) && "must have one set");
        assert(!(pParent && pAtkProps) && "must not have both");
        auto aAtkProperties = handleAtkObject(reader);
        if (pParent)
            applyAtkProperties(pParent, aAtkProperties);
        if (pAtkProps)
            *pAtkProps = aAtkProperties;
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
            if (name == "child")
            {
                if (!pCurrentChild)
                {
                    pCurrentChild = insertObject(pParent, sClass, sID,
                        aProperties, aPangoAttributes, aAtkAttributes);
                }
                handleChild(pCurrentChild, nullptr, reader);
            }
            else if (name == "items")
                aItems = handleItems(reader);
            else if (name == "style")
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
                if (name == "property")
                    collectProperty(reader, aProperties);
                else if (name == "attribute")
                    collectPangoAttribute(reader, aPangoAttributes);
                else if (name == "relation")
                    collectAtkRelationAttribute(reader, aAtkAttributes);
                else if (name == "role")
                    collectAtkRoleAttribute(reader, aAtkAttributes);
                else if (name == "action-widget")
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
            if (name == "property")
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
        if (name == "name")
        {
            name = reader.getAttributeValue(false);
            OString sKey(name.begin, name.length);
            sKey = sKey.replace('_', '-');
            (void)reader.nextItem(
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
                SAL_WARN("vcl.builder", "unknown packing: " << sKey);
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
            if (name == "class")
            {
                OString classStyle = getStyleClass(reader);

                if (classStyle.startsWith("context-"))
                {
                    OString sContext = classStyle.copy(classStyle.indexOf('-') + 1);
                    OUString sContext2(sContext.getStr(), sContext.getLength(), RTL_TEXTENCODING_UTF8);
                    aContext.push_back(vcl::EnumContext::GetContextEnum(sContext2));
                }
                else if (classStyle.startsWith("priority-"))
                {
                    OString aPriority = classStyle.copy(classStyle.indexOf('-') + 1);
                    OUString aPriority2(aPriority.getStr(), aPriority.getLength(), RTL_TEXTENCODING_UTF8);
                    nPriority = aPriority2.toInt32();
                }
                else
                {
                    SAL_WARN("vcl.builder", "unknown class: " << classStyle);
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
        if (name == "name")
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
        if (name == "name")
        {
            name = reader.getAttributeValue(false);
            sProperty = OString(name.begin, name.length);
        }
        else if (name == "context")
        {
            name = reader.getAttributeValue(false);
            sContext = OString(name.begin, name.length);
        }
        else if (name == "translatable" && reader.getAttributeValue(false) == "yes")
        {
            bTranslated = true;
        }
    }

    (void)reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
    OString sValue(name.begin, name.length);
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
        if (name == "response")
        {
            name = reader.getAttributeValue(false);
            sResponse = OString(name.begin, name.length);
        }
    }

    (void)reader.nextItem(xmlreader::XmlReader::Text::Raw, &name, &nsId);
    OString sID(name.begin, name.length);
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
        if (name == "key")
        {
            name = reader.getAttributeValue(false);
            sValue = OString(name.begin, name.length);
        }
        else if (name == "signal")
        {
            name = reader.getAttributeValue(false);
            sProperty = OString(name.begin, name.length);
        }
        else if (name == "modifiers")
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
            return dynamic_cast<PopupMenu*>(menu.m_pMenu.get());
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
        default:
            assert(nResponse >= 100 && "keep non-canned responses in range 100+ to avoid collision with vcl RET_*");
            break;
    }

    for (const auto & child : m_aChildren)
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
            SAL_INFO("vcl.builder", "unhandled property :" << rKey);
        }
    }
}

void VclBuilder::mungeAdjustment(FormattedField &rTarget, const Adjustment &rAdjustment)
{
    double nMaxValue = 0, nMinValue = 0, nValue = 0, nSpinSize = 0;

    for (auto const& elem : rAdjustment)
    {
        const OString &rKey = elem.first;
        const OUString &rValue = elem.second;

        if (rKey == "upper")
            nMaxValue = rValue.toDouble();
        else if (rKey == "lower")
            nMinValue = rValue.toDouble();
        else if (rKey == "value")
            nValue = rValue.toDouble();
        else if (rKey == "step-increment")
            nSpinSize = rValue.toDouble();
        else
            SAL_INFO("vcl.builder", "unhandled property :" << rKey);
    }

    Formatter& rFormatter = rTarget.GetFormatter();
    rFormatter.SetMinValue(nMinValue);
    rFormatter.SetMaxValue(nMaxValue);
    rFormatter.SetValue(nValue);
    rFormatter.SetSpinSize(nSpinSize);
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
            SAL_INFO("vcl.builder", "unhandled property :" << rKey);
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
            SAL_INFO("vcl.builder", "unhandled property :" << rKey);
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
            SAL_INFO("vcl.builder", "unhandled property :" << rKey);
        }
    }
}

VclBuilder::ParserState::ParserState()
    : m_nLastToolbarId(0)
    , m_nLastMenuItemId(0)
{}

VclBuilder::MenuAndId::MenuAndId(const OString &rId, Menu *pMenu)
    : m_sID(rId)
    , m_pMenu(pMenu)
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
