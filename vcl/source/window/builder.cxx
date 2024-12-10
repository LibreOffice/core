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
#include <config_vclplug.h>

#include <memory>
#include <string_view>
#include <unordered_map>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <comphelper/lok.hxx>
#include <i18nutil/unicode.hxx>
#include <jsdialog/enabled.hxx>
#include <o3tl/string_view.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/module.hxx>
#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/resmgr.hxx>
#include <utility>
#include <vcl/builder.hxx>
#include <vcl/dialoghelper.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/toolkit/edit.hxx>
#include <vcl/toolkit/field.hxx>
#include <vcl/fieldvalues.hxx>
#include <vcl/toolkit/fmtfield.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/toolkit/fixedhyper.hxx>
#include <vcl/headbar.hxx>
#include <vcl/notebookbar/NotebookBarAddonsMerger.hxx>
#include <vcl/toolkit/ivctrl.hxx>
#include <vcl/layout.hxx>
#include <vcl/toolkit/lstbox.hxx>
#include <vcl/toolkit/menubtn.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/toolkit/prgsbar.hxx>
#include <vcl/toolkit/scrbar.hxx>
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
#include <managedmenubutton.hxx>
#include <messagedialog.hxx>
#include <ContextVBox.hxx>
#include <DropdownBox.hxx>
#include <OptionalBox.hxx>
#include <PriorityMergedHBox.hxx>
#include <PriorityHBox.hxx>
#include <window.h>
#include <xmlreader/xmlreader.hxx>
#include <desktop/crashreport.hxx>
#include <calendar.hxx>
#include <menutogglebutton.hxx>
#include <salinst.hxx>
#include <strings.hrc>
#include <treeglue.hxx>
#include <verticaltabctrl.hxx>
#include <wizdlg.hxx>
#include <tools/svlibrary.h>
#include <jsdialog/jsdialogbuilder.hxx>

#if defined(DISABLE_DYNLOADING) || defined(LINUX)
#include <dlfcn.h>
#endif

bool toBool(std::u16string_view rValue)
{
    return (!rValue.empty() && (rValue[0] == 't' || rValue[0] == 'T' || rValue[0] == '1'));
}

namespace
{
    const OUString & mapStockToImageResource(std::u16string_view sType)
    {
        if (sType == u"view-refresh")
            return SV_RESID_BITMAP_REFRESH;
        else if (sType == u"dialog-error")
            return IMG_ERROR;
        else if (sType == u"list-add")
            return IMG_ADD;
        else if (sType == u"list-remove")
            return IMG_REMOVE;
        else if (sType == u"edit-copy")
            return IMG_COPY;
        else if (sType == u"edit-paste")
            return IMG_PASTE;
        else if (sType == u"document-open")
            return IMG_OPEN;
        else if (sType == u"open-menu-symbolic")
            return IMG_MENU;
        else if (sType == u"window-close-symbolic")
            return SV_RESID_BITMAP_CLOSEDOC;
        else if (sType == u"x-office-calendar")
            return IMG_CALENDAR;
        else if (sType == u"accessories-character-map")
            return IMG_CHARACTER_MAP;
        return EMPTY_OUSTRING;
    }

}

SymbolType VclBuilder::mapStockToSymbol(std::u16string_view sType)
{
    SymbolType eRet = SymbolType::DONTKNOW;
    if (sType == u"media-skip-forward")
        eRet = SymbolType::NEXT;
    else if (sType == u"media-skip-backward")
        eRet = SymbolType::PREV;
    else if (sType == u"media-playback-start")
        eRet = SymbolType::PLAY;
    else if (sType == u"media-playback-stop")
        eRet = SymbolType::STOP;
    else if (sType == u"go-first")
        eRet = SymbolType::FIRST;
    else if (sType == u"go-last")
        eRet = SymbolType::LAST;
    else if (sType == u"go-previous")
        eRet = SymbolType::ARROW_LEFT;
    else if (sType == u"go-next")
        eRet = SymbolType::ARROW_RIGHT;
    else if (sType == u"go-up")
        eRet = SymbolType::ARROW_UP;
    else if (sType == u"go-down")
        eRet = SymbolType::ARROW_DOWN;
    else if (sType == u"missing-image")
        eRet = SymbolType::IMAGE;
    else if (sType == u"help-browser" || sType == u"help-browser-symbolic")
        eRet = SymbolType::HELP;
    else if (sType == u"window-close")
        eRet = SymbolType::CLOSE;
    else if (sType == u"document-new")
        eRet = SymbolType::PLUS;
    else if (sType == u"pan-down-symbolic")
        eRet = SymbolType::SPIN_DOWN;
    else if (sType == u"pan-up-symbolic")
        eRet = SymbolType::SPIN_UP;
    else if (!mapStockToImageResource(sType).isEmpty())
        eRet = SymbolType::IMAGE;
    return eRet;
}

namespace
{
    void setupFromActionName(Button *pButton, VclBuilder::stringmap &rMap, const css::uno::Reference<css::frame::XFrame>& rFrame);

#if defined SAL_LOG_WARN
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
#endif

}

std::unique_ptr<weld::Builder> Application::CreateBuilder(weld::Widget* pParent, const OUString &rUIFile, bool bMobile, sal_uInt64 nLOKWindowId)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        if (jsdialog::isBuilderEnabledForSidebar(rUIFile))
            return JSInstanceBuilder::CreateSidebarBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile, nLOKWindowId);
        else if (jsdialog::isBuilderEnabledForPopup(rUIFile))
            return JSInstanceBuilder::CreatePopupBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile);
        else if (jsdialog::isBuilderEnabled(rUIFile, bMobile))
            return JSInstanceBuilder::CreateDialogBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile);
    }

    return ImplGetSVData()->mpDefInst->CreateBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile);
}

std::unique_ptr<weld::Builder> Application::CreateInterimBuilder(vcl::Window* pParent, const OUString &rUIFile, bool bAllowCycleFocusOut, sal_uInt64 nLOKWindowId)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        // Notebookbar sub controls
        if (jsdialog::isInterimBuilderEnabledForNotebookbar(rUIFile))
            return JSInstanceBuilder::CreateNotebookbarBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile, css::uno::Reference<css::frame::XFrame>(), nLOKWindowId);
        else if (rUIFile == u"modules/scalc/ui/inputbar.ui")
            return JSInstanceBuilder::CreateFormulabarBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile, nLOKWindowId);
    }

    return ImplGetSVData()->mpDefInst->CreateInterimBuilder(pParent, AllSettings::GetUIRootDir(), rUIFile, bAllowCycleFocusOut, nLOKWindowId);
}

weld::MessageDialog* Application::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType,
                                                      VclButtonsType eButtonType, const OUString& rPrimaryMessage,
                                                      const ILibreOfficeKitNotifier* pNotifier)
{
    if (comphelper::LibreOfficeKit::isActive())
        return JSInstanceBuilder::CreateMessageDialog(pParent, eMessageType, eButtonType, rPrimaryMessage, pNotifier);
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
        sal_Int64 min, max;
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

    sal_Int64 SpinButton::denormalize(sal_Int64 nValue) const
    {
        const int nFactor = Power10(get_digits());

        if ((nValue < (std::numeric_limits<sal_Int64>::min() + nFactor)) ||
            (nValue > (std::numeric_limits<sal_Int64>::max() - nFactor)))
        {
            return nValue / nFactor;
        }

        const int nHalf = nFactor / 2;

        if (nValue < 0)
            return (nValue - nHalf) / nFactor;
        return (nValue + nHalf) / nFactor;
    }

    OUString MetricSpinButton::format_number(sal_Int64 nValue) const
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
                OUString sDoublePrime = u"\u2033"_ustr;
                if (aSuffix != "\"" && aSuffix != sDoublePrime)
                    aStr += " ";
                else
                    aSuffix = sDoublePrime;
            }
            else if (m_eSrcUnit == FieldUnit::FOOT)
            {
                OUString sPrime = u"\u2032"_ustr;
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
        sal_Int64 step, page;
        get_increments(step, page, m_eSrcUnit);
        sal_Int64 value = get_value(m_eSrcUnit);
        m_xSpinButton->set_digits(digits);
        set_increments(step, page, m_eSrcUnit);
        set_value(value, m_eSrcUnit);
        update_width_chars();
    }

    void MetricSpinButton::set_unit(FieldUnit eUnit)
    {
        if (eUnit != m_eSrcUnit)
        {
            sal_Int64 step, page;
            get_increments(step, page, m_eSrcUnit);
            sal_Int64 value = get_value(m_eSrcUnit);
            m_eSrcUnit = eUnit;
            set_increments(step, page, m_eSrcUnit);
            set_value(value, m_eSrcUnit);
            spin_button_output(*m_xSpinButton);
            update_width_chars();
        }
    }

    sal_Int64 MetricSpinButton::ConvertValue(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit) const
    {
        return vcl::ConvertValue(nValue, 0, m_xSpinButton->get_digits(), eInUnit, eOutUnit);
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

// static
void BuilderBase::reportException(const css::uno::Exception& rExcept)
{
    CrashReporter::addKeyValue(u"VclBuilderException"_ustr,
                               "Unable to read .ui file: " + rExcept.Message, CrashReporter::Write);
}

BuilderBase::BuilderBase(std::u16string_view sUIDir, const OUString& rUIFile, bool bLegacy)
    : m_pParserState(new ParserState)
    , m_sUIFileUrl(sUIDir + rUIFile)
    , m_sHelpRoot(rUIFile)
    , m_bLegacy(bLegacy)
{
    const sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
    if (nIdx != -1)
        m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
    m_sHelpRoot += "/";
}

const std::locale& BuilderBase::getResLocale() const
{
    assert(m_pParserState && "parser state no more valid");
    return m_pParserState->m_aResLocale;
}

const std::vector<BuilderBase::SizeGroup>& BuilderBase::getSizeGroups() const
{
    assert(m_pParserState && "parser state no more valid");
    return m_pParserState->m_aSizeGroups;
}

const std::vector<BuilderBase::MnemonicWidgetMap>& BuilderBase::getMnemonicWidgetMaps() const {
    assert(m_pParserState && "parser state no more valid");
    return m_pParserState->m_aMnemonicWidgetMaps;
}

OUString BuilderBase::finalizeValue(const OString& rContext, const OString& rValue,
                                    const bool bTranslate) const
{
    OUString sFinalValue;
    if (bTranslate)
    {
        sFinalValue
            = Translate::get(TranslateId{ rContext.getStr(), rValue.getStr() }, getResLocale());
    }
    else
        sFinalValue = OUString::fromUtf8(rValue);

    if (ResHookProc pStringReplace = Translate::GetReadStringHook())
        sFinalValue = (*pStringReplace)(sFinalValue);

    return sFinalValue;
}

void BuilderBase::resetParserState() { m_pParserState.reset(); }

VclBuilder::VclBuilder(vcl::Window* pParent, std::u16string_view sUIDir, const OUString& sUIFile,
                       OUString sID, css::uno::Reference<css::frame::XFrame> xFrame,
                       bool bLegacy, const NotebookBarAddonsItem* pNotebookBarAddonsItem)
    : WidgetBuilder(sUIDir, sUIFile, bLegacy)
    , m_pNotebookBarAddonsItem(pNotebookBarAddonsItem
                                   ? new NotebookBarAddonsItem(*pNotebookBarAddonsItem)
                                   : new NotebookBarAddonsItem{})
    , m_sID(std::move(sID))
    , m_pParent(pParent)
    , m_bToplevelParentFound(false)
    , m_pVclParserState(new VclParserState)
    , m_xFrame(std::move(xFrame))
{
    m_bToplevelHasDeferredInit = pParent &&
        ((pParent->IsSystemWindow() && static_cast<SystemWindow*>(pParent)->isDeferredInit()) ||
         (pParent->IsDockingWindow() && static_cast<DockingWindow*>(pParent)->isDeferredInit()));
    m_bToplevelHasDeferredProperties = m_bToplevelHasDeferredInit;

    processUIFile(pParent);

    //Set a11y relations and role when everything has been imported
    for (auto const& elemAtk : m_pVclParserState->m_aAtkInfo)
    {
        vcl::Window *pSource = elemAtk.first;
        const stringmap &rMap = elemAtk.second;

        for (auto const& [ rType, rParam ] : rMap)
        {
            if (rType == "role")
            {
                sal_Int16 role = BuilderUtils::getRoleFromName(rParam);
                if (role != css::accessibility::AccessibleRole::UNKNOWN)
                    pSource->SetAccessibleRole(role);
            }
            else
            {
                vcl::Window *pTarget = get(rParam);
                SAL_WARN_IF(!pTarget, "vcl", "missing parameter of a11y relation: " << rParam);
                if (!pTarget)
                    continue;
                if (rType == "labelled-by")
                    pSource->SetAccessibleRelationLabeledBy(pTarget);
                else if (rType == "label-for")
                    pSource->SetAccessibleRelationLabelFor(pTarget);
                else
                {
                    SAL_WARN("vcl.builder", "unhandled a11y relation :" << rType);
                }
            }
        }
    }

    //Set radiobutton groups when everything has been imported
    for (auto const& elem : m_pVclParserState->m_aGroupMaps)
    {
        RadioButton *pOne = get<RadioButton>(elem.m_sID);
        RadioButton *pOther = get<RadioButton>(elem.m_sValue);
        SAL_WARN_IF(!pOne || !pOther, "vcl", "missing member of radiobutton group");
        if (pOne && pOther)
        {
            if (isLegacy())
                pOne->group(*pOther);
            else
            {
                pOther->group(*pOne);
                std::stable_sort(pOther->m_xGroup->begin(), pOther->m_xGroup->end(), sortIntoBestTabTraversalOrder(this));
            }
        }
    }

#ifndef NDEBUG
    o3tl::sorted_vector<OUString> models;
#endif
    //Set ComboBox models when everything has been imported
    for (auto const& elem : m_pVclParserState->m_aModelMaps)
    {
        assert(models.insert(elem.m_sValue).second && "a liststore or treestore is used in duplicate widgets");
        vcl::Window* pTarget = get(elem.m_sID);
        ListBox *pListBoxTarget = dynamic_cast<ListBox*>(pTarget);
        ComboBox *pComboBoxTarget = dynamic_cast<ComboBox*>(pTarget);
        SvTabListBox *pTreeBoxTarget = dynamic_cast<SvTabListBox*>(pTarget);
        // pStore may be empty
        const ListStore *pStore = get_model_by_name(elem.m_sValue);
        SAL_WARN_IF(!pListBoxTarget && !pComboBoxTarget && !pTreeBoxTarget && !dynamic_cast<IconView*>(pTarget), "vcl", "missing elements of combobox");
        if (pListBoxTarget && pStore)
            mungeModel(*pListBoxTarget, *pStore, elem.m_nActiveId);
        else if (pComboBoxTarget && pStore)
            mungeModel(*pComboBoxTarget, *pStore, elem.m_nActiveId);
        else if (pTreeBoxTarget && pStore)
            mungeModel(*pTreeBoxTarget, *pStore, elem.m_nActiveId);
    }

    //Set TextView buffers when everything has been imported
    for (auto const& elem : m_pVclParserState->m_aTextBufferMaps)
    {
        VclMultiLineEdit *pTarget = get<VclMultiLineEdit>(elem.m_sID);
        const TextBuffer *pBuffer = get_buffer_by_name(elem.m_sValue);
        SAL_WARN_IF(!pTarget || !pBuffer, "vcl", "missing elements of textview/textbuffer");
        if (pTarget && pBuffer)
            mungeTextBuffer(*pTarget, *pBuffer);
    }

    //Set SpinButton adjustments when everything has been imported
    for (auto const& elem : m_pVclParserState->m_aNumericFormatterAdjustmentMaps)
    {
        NumericFormatter *pTarget = dynamic_cast<NumericFormatter*>(get(elem.m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue);
        SAL_WARN_IF(!pTarget, "vcl", "missing NumericFormatter element of spinbutton/adjustment");
        SAL_WARN_IF(!pAdjustment, "vcl", "missing Adjustment element of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    for (auto const& elem : m_pVclParserState->m_aFormattedFormatterAdjustmentMaps)
    {
        FormattedField *pTarget = dynamic_cast<FormattedField*>(get(elem.m_sID));
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue);
        SAL_WARN_IF(!pTarget, "vcl", "missing FormattedField element of spinbutton/adjustment");
        SAL_WARN_IF(!pAdjustment, "vcl", "missing Adjustment element of spinbutton/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    //Set ScrollBar adjustments when everything has been imported
    for (auto const& elem : m_pVclParserState->m_aScrollAdjustmentMaps)
    {
        ScrollBar *pTarget = get<ScrollBar>(elem.m_sID);
        const Adjustment *pAdjustment = get_adjustment_by_name(elem.m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of scrollbar/adjustment");
        if (pTarget && pAdjustment)
            mungeAdjustment(*pTarget, *pAdjustment);
    }

    //Set Scale(Slider) adjustments
    for (auto const& elem : m_pVclParserState->m_aSliderAdjustmentMaps)
    {
        Slider* pTarget = dynamic_cast<Slider*>(get(elem.m_sID));
        const Adjustment* pAdjustment = get_adjustment_by_name(elem.m_sValue);
        SAL_WARN_IF(!pTarget || !pAdjustment, "vcl", "missing elements of scale(slider)/adjustment");
        if (pTarget && pAdjustment)
        {
            mungeAdjustment(*pTarget, *pAdjustment);
        }
    }

    //Set size-groups when all widgets have been imported
    for (auto const& sizeGroup : getSizeGroups())
    {
        std::shared_ptr<VclSizeGroup> xGroup(std::make_shared<VclSizeGroup>());

        for (auto const& [ rKey, rValue ] : sizeGroup.m_aProperties)
            xGroup->set_property(rKey, rValue);

        for (auto const& elem : sizeGroup.m_aWidgets)
        {
            vcl::Window* pWindow = get(elem);
            pWindow->add_to_size_group(xGroup);
        }
    }

    //Set button images when everything has been imported
    std::set<OUString> aImagesToBeRemoved;
    for (auto const& elem : m_pVclParserState->m_aButtonImageWidgetMaps)
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

        FixedImage *pImage = get<FixedImage>(elem.m_sValue);
        SAL_WARN_IF(!pTarget || !pImage,
            "vcl", "missing elements of button/image/stock");
        if (!pTarget || !pImage)
            continue;
        aImagesToBeRemoved.insert(elem.m_sValue);

        if (!elem.m_bRadio)
        {
            const Image& rImage = pImage->GetImage();
            SymbolType eSymbol = mapStockToSymbol(rImage.GetStock());
            if (eSymbol != SymbolType::IMAGE && eSymbol != SymbolType::DONTKNOW)
            {
                pTargetButton->SetSymbol(eSymbol);
                //fdo#76457 keep symbol images small e.g. tools->customize->menu
                //but images the right size. Really the PushButton::CalcMinimumSize
                //and PushButton::ImplDrawPushButton are the better place to handle
                //this, but its such a train-wreck
                pTargetButton->SetStyle(pTargetButton->GetStyle() | WB_SMALLSTYLE);
            }
            else
            {
                pTargetButton->SetModeImage(rImage);
                if (pImage->GetStyle() & WB_SMALLSTYLE)
                {
                    Size aSz(rImage.GetSizePixel());
                    aSz.AdjustWidth(6);
                    aSz.AdjustHeight(6);
                    if (pTargetButton->get_width_request() == -1)
                        pTargetButton->set_width_request(aSz.Width());
                    if (pTargetButton->get_height_request() == -1)
                        pTargetButton->set_height_request(aSz.Height());
                }
            }
        }
        else
            pTargetRadio->SetModeRadioImage(pImage->GetImage());

        auto aFind = m_pVclParserState->m_aImageSizeMap.find(elem.m_sValue);
        if (aFind != m_pVclParserState->m_aImageSizeMap.end())
        {
            switch (aFind->second)
            {
                case 1:
                    pTarget->SetSmallSymbol();
                    break;
                case 2:
                    assert(pImage->GetStyle() & WB_SMALLSTYLE);
                    pTarget->SetStyle(pTarget->GetStyle() | WB_SMALLSTYLE);
                    break;
                case 3:
                    pTarget->SetStyle(pTarget->GetStyle() | WB_SMALLSTYLE);
                    // large toolbar, make bigger than normal (4)
                    pTarget->set_width_request(pTarget->GetOptimalSize().Width() * 1.5);
                    pTarget->set_height_request(pTarget->GetOptimalSize().Height() * 1.5);
                    break;
                case 4:
                    break;
                default:
                    SAL_WARN("vcl.builder", "unsupported image size " << aFind->second);
                    break;
            }
            m_pVclParserState->m_aImageSizeMap.erase(aFind);
        }
    }

    //There may be duplicate use of an Image, so we used a set to collect and
    //now we can remove them from the tree after their final munge
    for (auto const& elem : aImagesToBeRemoved)
    {
        delete_by_name(elem);
    }

    //Set button menus when everything has been imported
    for (auto const& elem : m_pVclParserState->m_aButtonMenuMaps)
    {
        MenuButton *pTarget = get<MenuButton>(elem.m_sID);
        PopupMenu *pMenu = get_menu(elem.m_sValue);
        SAL_WARN_IF(!pTarget || !pMenu,
            "vcl", "missing elements of button/menu");
        if (!pTarget || !pMenu)
            continue;
        pTarget->SetPopupMenu(pMenu, true);
    }

    //Remove ScrollWindow parent widgets whose children in vcl implement scrolling
    //internally.
    for (auto const& elem : m_pVclParserState->m_aRedundantParentWidgets)
    {
        delete_by_window(elem.first);
    }

    //fdo#67378 merge the label into the disclosure button
    for (auto const& elem : m_pVclParserState->m_aExpanderWidgets)
    {
        vcl::Window *pChild = elem->get_child();
        vcl::Window* pLabel = elem->GetWindow(GetWindowType::LastChild);
        if (pLabel && pLabel != pChild && pLabel->GetType() == WindowType::FIXEDTEXT)
        {
            FixedText *pLabelWidget = static_cast<FixedText*>(pLabel);
            elem->set_label(pLabelWidget->GetText());
            if (pLabelWidget->IsControlFont())
                elem->get_label_widget()->SetControlFont(pLabelWidget->GetControlFont());
            delete_by_window(pLabel);
        }
    }

    // create message dialog message area now
    for (auto const& elem : m_pVclParserState->m_aMessageDialogs)
        elem->create_message_area();

    //drop maps, etc. that we don't need again
    resetParserState();

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
        if (vcl::Window *pHelpButton = get(u"help"))
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
    inline OUString extractStringEntry(BuilderBase::stringmap& rMap, const OUString& rKey,
                                       const OUString& rDefaultValue = OUString())
    {
        BuilderBase::stringmap::iterator aFind = rMap.find(rKey);
        if (aFind != rMap.end())
        {
            const OUString sValue = aFind->second;
            rMap.erase(aFind);
            return sValue;
        }
        return rDefaultValue;
    }

    inline bool extractBoolEntry(BuilderBase::stringmap& rMap, const OUString& rKey, bool bDefaultValue)
    {
        BuilderBase::stringmap::iterator aFind = rMap.find(rKey);
        if (aFind != rMap.end())
        {
            const bool bValue = toBool(aFind->second);
            rMap.erase(aFind);
            return bValue;
        }
        return bDefaultValue;
    }

    bool extractHasFrame(VclBuilder::stringmap& rMap)
    {
        return extractBoolEntry(rMap, u"has-frame"_ustr, true);
    }

    bool extractDrawValue(VclBuilder::stringmap& rMap)
    {
        return extractBoolEntry(rMap, u"draw-value"_ustr, true);
    }

    OUString extractPopupMenu(VclBuilder::stringmap& rMap)
    {
        return extractStringEntry(rMap, u"popup"_ustr);
    }

    OUString extractWidgetName(VclBuilder::stringmap& rMap)
    {
        return extractStringEntry(rMap, u"name"_ustr);
    }

    OUString extractValuePos(VclBuilder::stringmap& rMap)
    {
        return extractStringEntry(rMap,u"value-pos"_ustr, u"top"_ustr);
    }

    OUString extractTypeHint(VclBuilder::stringmap &rMap)
    {
        return extractStringEntry(rMap, u"type-hint"_ustr, u"normal"_ustr);
    }

#if HAVE_FEATURE_DESKTOP
    bool extractModal(VclBuilder::stringmap &rMap)
    {
        return extractBoolEntry(rMap, u"modal"_ustr, false);
    }
#endif

    bool extractDecorated(VclBuilder::stringmap &rMap)
    {
        return extractBoolEntry(rMap, u"decorated"_ustr, true);
    }

    bool extractCloseable(VclBuilder::stringmap &rMap)
    {
        return extractBoolEntry(rMap, u"deletable"_ustr, true);
    }

    bool extractVerticalTabPos(VclBuilder::stringmap &rMap)
    {
        bool bVertical = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(u"tab-pos"_ustr);
        if (aFind != rMap.end())
        {
            bVertical = aFind->second.equalsIgnoreAsciiCase("left") ||
                        aFind->second.equalsIgnoreAsciiCase("right");
            rMap.erase(aFind);
        }
        return bVertical;
    }

    bool extractVerticalTabsWithIcons(VclBuilder::stringmap &rMap)
    {
        bool bWithIcons = false;
        VclBuilder::stringmap::iterator aFind = rMap.find(u"group-name"_ustr);
        if (aFind != rMap.end())
        {
            bWithIcons = aFind->second.equalsIgnoreAsciiCase("icons");
            rMap.erase(aFind);
        }
        return bWithIcons;
    }

    bool extractInconsistent(VclBuilder::stringmap &rMap)
    {
        return extractBoolEntry(rMap, u"inconsistent"_ustr, false);
    }

    WinBits extractRelief(VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_3DLOOK;
        VclBuilder::stringmap::iterator aFind = rMap.find(u"relief"_ustr);
        if (aFind != rMap.end())
        {
            assert(aFind->second != "half" && "relief of 'half' unsupported");
            if (aFind->second == "none")
                nBits = WB_FLATBUTTON;
            rMap.erase(aFind);
        }
        return nBits;
    }

    OUString extractLabel(VclBuilder::stringmap &rMap)
    {
        return extractStringEntry(rMap, u"label"_ustr);
    }

    OUString extractActionName(VclBuilder::stringmap &rMap)
    {
        return extractStringEntry(rMap, u"action-name"_ustr);
    }

    Size extractSizeRequest(VclBuilder::stringmap &rMap)
    {
        OUString sWidthRequest(u"0"_ustr);
        OUString sHeightRequest(u"0"_ustr);
        VclBuilder::stringmap::iterator aFind = rMap.find(u"width-request"_ustr);
        if (aFind != rMap.end())
        {
            sWidthRequest = aFind->second;
            rMap.erase(aFind);
        }
        aFind = rMap.find(u"height-request"_ustr);
        if (aFind != rMap.end())
        {
            sHeightRequest = aFind->second;
            rMap.erase(aFind);
        }
        return Size(sWidthRequest.toInt32(), sHeightRequest.toInt32());
    }

    float extractAlignment(VclBuilder::stringmap &rMap)
    {
        float f = 0.0;
        VclBuilder::stringmap::iterator aFind = rMap.find(u"alignment"_ustr);
        if (aFind != rMap.end())
        {
            f = aFind->second.toFloat();
            rMap.erase(aFind);
        }
        return f;
    }

    OUString extractTitle(VclBuilder::stringmap &rMap)
    {
        return extractStringEntry(rMap, u"title"_ustr);
    }

    bool extractSortIndicator(VclBuilder::stringmap &rMap)
    {
        return extractBoolEntry(rMap, u"sort-indicator"_ustr, false);
    }

    bool extractClickable(VclBuilder::stringmap &rMap)
    {
        return extractBoolEntry(rMap, u"clickable"_ustr, false);
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

        pButton->SetCommandHandler(aCommand, rFrame);
    }

    VclPtr<Button> extractStockAndBuildPushButton(vcl::Window *pParent, VclBuilder::stringmap &rMap, bool bToggle)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER;
        if (bToggle)
            nBits |= WB_TOGGLE;

        nBits |= extractRelief(rMap);

        VclPtr<Button> xWindow = VclPtr<PushButton>::Create(pParent, nBits);
        return xWindow;
    }

    VclPtr<MenuButton> extractStockAndBuildMenuButton(vcl::Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;

        nBits |= extractRelief(rMap);

        VclPtr<MenuButton> xWindow = VclPtr<MenuButton>::Create(pParent, nBits);
        return xWindow;
    }

    VclPtr<MenuButton> extractStockAndBuildMenuToggleButton(vcl::Window *pParent, VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_3DLOOK;

        nBits |= extractRelief(rMap);

        VclPtr<MenuButton> xWindow = VclPtr<MenuToggleButton>::Create(pParent, nBits);
        return xWindow;
    }

    WinBits extractDeferredBits(VclBuilder::stringmap &rMap)
    {
        WinBits nBits = WB_3DLOOK|WB_HIDE;
        if (BuilderBase::extractResizable(rMap))
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

void VclBuilder::extractGroup(const OUString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(u"group"_ustr);
    if (aFind != rMap.end())
    {
        OUString sID = aFind->second;
        sal_Int32 nDelim = sID.indexOf(':');
        if (nDelim != -1)
            sID = sID.copy(0, nDelim);
        m_pVclParserState->m_aGroupMaps.emplace_back(id, sID);
        rMap.erase(aFind);
    }
}

void VclBuilder::connectNumericFormatterAdjustment(const OUString &id, const OUString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pVclParserState->m_aNumericFormatterAdjustmentMaps.emplace_back(id, rAdjustment);
}

void VclBuilder::connectFormattedFormatterAdjustment(const OUString &id, const OUString &rAdjustment)
{
    if (!rAdjustment.isEmpty())
        m_pVclParserState->m_aFormattedFormatterAdjustmentMaps.emplace_back(id, rAdjustment);
}

bool VclBuilder::extractAdjustmentToMap(const OUString& id, VclBuilder::stringmap& rMap, std::vector<WidgetAdjustmentMap>& rAdjustmentMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(u"adjustment"_ustr);
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
    bool extractSelectable(VclBuilder::stringmap &rMap)
    {
        return extractBoolEntry(rMap, u"selectable"_ustr, false);
    }

    OUString extractAdjustment(VclBuilder::stringmap &rMap)
    {
        OUString sAdjustment;
        VclBuilder::stringmap::iterator aFind = rMap.find(u"adjustment"_ustr);
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
        return extractBoolEntry(rMap, u"draw-indicator"_ustr, false);
    }
}

void VclBuilder::extractModel(const OUString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(u"model"_ustr);
    if (aFind != rMap.end())
    {
        m_pVclParserState->m_aModelMaps.emplace_back(id, aFind->second,
            extractActive(rMap));
        rMap.erase(aFind);
    }
}

void VclBuilder::extractBuffer(const OUString &id, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(u"buffer"_ustr);
    if (aFind != rMap.end())
    {
        m_pVclParserState->m_aTextBufferMaps.emplace_back(id, aFind->second);
        rMap.erase(aFind);
    }
}

int VclBuilder::getImageSize(const stringmap &rMap)
{
    int nSize = 4;
    auto aFind = rMap.find(u"icon-size"_ustr);
    if (aFind != rMap.end())
        nSize = aFind->second.toInt32();
    return nSize;
}

void VclBuilder::extractButtonImage(const OUString &id, stringmap &rMap, bool bRadio)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(u"image"_ustr);
    if (aFind != rMap.end())
    {
        m_pVclParserState->m_aButtonImageWidgetMaps.emplace_back(id, aFind->second, bRadio);
        rMap.erase(aFind);
    }
}

void BuilderBase::extractMnemonicWidget(const OUString &rLabelID, stringmap &rMap)
{
    VclBuilder::stringmap::iterator aFind = rMap.find(u"mnemonic-widget"_ustr);
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
    rMap[u"width-request"_ustr] = OUString::number(nWidthReq);
    sal_Int32 nHeightReq = pScrollParent->get_height_request();
    rMap[u"height-request"_ustr] = OUString::number(nHeightReq);

    m_pVclParserState->m_aRedundantParentWidgets[pScrollParent] = pWindow;
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

namespace vcl {

void VclBuilderPreload()
{
#ifndef DISABLE_DYNLOADING

#if ENABLE_MERGELIBS
    g_pMergedLib->loadRelative(&thisModule, SVLIBRARY("merged"));
#else
// find -name '*ui*' | xargs grep 'class=".*lo-' |
//     sed 's/.*class="//' | sed 's/-.*$//' | sort | uniq
    static const char *aWidgetLibs[] = {
        "sfxlo",  "svtlo"
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

}

#if defined DISABLE_DYNLOADING && !HAVE_FEATURE_DESKTOP

// This ifdef branch is mainly for building for the Collabora Online
// -based mobile apps for Android and iOS.

extern "C" VclBuilder::customMakeWidget lo_get_custom_widget_func(const char* name);

#elif defined EMSCRIPTEN && !ENABLE_QT5

// This branch is mainly for building for WASM, and especially for
// Collabora Online in the browser, where code from core and Collabora
// Online is compiled to WASM and linked into a single WASM binary.
// (Not for Allotropia's Qt-based LibreOffice in the browser.)

// When building core for WASM it doesn't use the same
// solenv/bin/native-code.py thing as the mobile apps, even if in both
// cases everything is linked statically. So there is no generated
// native-code.h, and we can't use lo_get_custom_widget_func() from
// that. So cheat and duplicate the code from an existing generated
// native-code.h. It's just a handful of lines anyway.

extern "C" void makeNotebookbarTabControl(VclPtr<vcl::Window> &rRet, const VclPtr<vcl::Window> &pParent, VclBuilder::stringmap &rVec);
extern "C" void makeNotebookbarToolBox(VclPtr<vcl::Window> &rRet, const VclPtr<vcl::Window> &pParent, VclBuilder::stringmap &rVec);

static struct { const char *name; VclBuilder::customMakeWidget func; } custom_widgets[] = {
    { "makeNotebookbarTabControl", makeNotebookbarTabControl },
    { "makeNotebookbarToolBox", makeNotebookbarToolBox },
};

static VclBuilder::customMakeWidget lo_get_custom_widget_func(const char* name)
{
    for (size_t i = 0; i < sizeof(custom_widgets) / sizeof(custom_widgets[0]); i++)
        if (strcmp(name, custom_widgets[i].name) == 0)
            return custom_widgets[i].func;
    return nullptr;
}

#endif

namespace
{
// Takes a string like "sfxlo-NotebookbarToolBox"
VclBuilder::customMakeWidget GetCustomMakeWidget(const OUString& rName)
{
    const OUString name = rName == "sfxlo-SidebarToolBox" ? u"sfxlo-NotebookbarToolBox"_ustr : rName;
    VclBuilder::customMakeWidget pFunction = nullptr;
    if (sal_Int32 nDelim = name.indexOf('-'); nDelim != -1)
    {
        const OUString sFunction(OUString::Concat("make") + name.subView(nDelim + 1));

#ifndef DISABLE_DYNLOADING
        const OUString sModule = OUString::Concat(SAL_DLLPREFIX)
                                 + name.subView(0, nDelim)
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
                        pFunction = reinterpret_cast<VclBuilder::customMakeWidget>(dlsym(RTLD_DEFAULT, OUStringToOString(sFunction, RTL_TEXTENCODING_UTF8).getStr()));
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
#elif !HAVE_FEATURE_DESKTOP || (defined EMSCRIPTEN && !ENABLE_QT5)
        // This ifdef branch is mainly for building for either the
        // Android or iOS apps, or the Collabora Online as WASM thing.
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

VclPtr<vcl::Window> VclBuilder::makeObject(vcl::Window *pParent, const OUString &name, const OUString &id,
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
                    OUString sTabPageId = get_by_window(pParent) +
                        "-page" +
                        OUString::number(nNewPageCount);
                    m_aChildren.emplace_back(sTabPageId, pPage, false);
                    pPage->SetHelpId(getHelpRoot() + sTabPageId);

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
        m_pVclParserState->m_aMessageDialogs.push_back(xDialog);
        xWindow = xDialog;
#if defined _WIN32
        xWindow->set_border_width(3);
#else
        xWindow->set_border_width(12);
#endif
    }
    else if (name == "GtkBox" || name == "GtkStatusbar")
    {
        bVertical = hasOrientationVertical(rMap);
        if (bVertical)
            xWindow = VclPtr<VclVBox>::Create(pParent);
        else
            xWindow = VclPtr<VclHBox>::Create(pParent);

        if (name == "GtkStatusbar")
            xWindow->SetAccessibleRole(css::accessibility::AccessibleRole::STATUS_BAR);
    }
    else if (name == "GtkPaned")
    {
        bVertical = hasOrientationVertical(rMap);
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
        bVertical = hasOrientationVertical(rMap);
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
        m_pVclParserState->m_aExpanderWidgets.push_back(pExpander);
        xWindow = pExpander;
    }
    else if (name == "GtkButton" || (!isLegacy() && name == "GtkToggleButton"))
    {
        VclPtr<Button> xButton;
        OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
        if (sMenu.isEmpty())
            xButton = extractStockAndBuildPushButton(pParent, rMap, name == "GtkToggleButton");
        else
        {
            assert(isLegacy() && "use GtkMenuButton");
            xButton = extractStockAndBuildMenuButton(pParent, rMap);
            m_pVclParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
        }
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        setupFromActionName(xButton, rMap, m_xFrame);
        xWindow = xButton;
    }
    else if (name == "GtkMenuButton")
    {
        VclPtr<MenuButton> xButton;

        OUString sMenu = extractPopupMenu(rMap);
        if (!sMenu.isEmpty())
            m_pVclParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);

        OUString sType = extractWidgetName(rMap);
        if (sType.isEmpty())
        {
            xButton = extractStockAndBuildMenuButton(pParent, rMap);
            xButton->SetAccessibleRole(css::accessibility::AccessibleRole::BUTTON_MENU);
        }
        else
        {
            xButton = extractStockAndBuildMenuToggleButton(pParent, rMap);
        }

        xButton->SetImageAlign(ImageAlign::Left); //default to left

        if (!extractDrawIndicator(rMap))
            xButton->SetDropDown(PushButtonDropdownStyle::NONE);

        setupFromActionName(xButton, rMap, m_xFrame);
        xWindow = xButton;
    }
    else if (name == "GtkToggleButton" && isLegacy())
    {
        VclPtr<Button> xButton;
        OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
        assert(sMenu.getLength() && "not implemented yet");
        xButton = extractStockAndBuildMenuToggleButton(pParent, rMap);
        m_pVclParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        setupFromActionName(xButton, rMap, m_xFrame);
        xWindow = xButton;
    }
    else if (name == "GtkRadioButton")
    {
        extractGroup(id, rMap);
        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;
        VclPtr<RadioButton> xButton = VclPtr<RadioButton>::Create(pParent, true, nBits);
        xButton->SetImageAlign(ImageAlign::Left); //default to left
        xWindow = xButton;
    }
    else if (name == "GtkCheckButton")
    {
        WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;
        bool bIsTriState = extractInconsistent(rMap);
        VclPtr<CheckBox> xCheckBox = VclPtr<CheckBox>::Create(pParent, nBits);
        if (bIsTriState)
        {
            xCheckBox->EnableTriState(true);
            xCheckBox->SetState(TRISTATE_INDET);
        }
        xCheckBox->SetImageAlign(ImageAlign::Left); //default to left

        xWindow = xCheckBox;
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
    else if (name == "svtlo-ManagedMenuButton")
    {
        // like tdf#135495 keep the name svtlo-ManagedMenuButton even though it's a misnomer
        // and is not dlsymed from the svt library
        xWindow = VclPtr<ManagedMenuButton>::Create(pParent, WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_FLATBUTTON);
        OUString sMenu = BuilderUtils::extractCustomProperty(rMap);
        if (!sMenu.isEmpty())
            m_pVclParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
        setupFromActionName(static_cast<Button*>(xWindow.get()), rMap, m_xFrame);
    }
    else if (name == "sfxlo-PriorityMergedHBox")
    {
        // like tdf#135495 above, keep the sfxlo-PriorityMergedHBox even though its not in sfx anymore
        xWindow = VclPtr<PriorityMergedHBox>::Create(pParent);
    }
    else if (name == "sfxlo-PriorityHBox")
    {
        // like tdf#135495 above, keep the sfxlo-PriorityMergedHBox even though its not in sfx anymore
        xWindow = VclPtr<PriorityHBox>::Create(pParent);
    }
    else if (name == "sfxlo-DropdownBox")
    {
        // like tdf#135495 above, keep the sfxlo-PriorityMergedHBox even though its not in sfx anymore
        xWindow = VclPtr<DropdownBox>::Create(pParent);
    }
    else if (name == "sfxlo-ContextVBox")
    {
        // like tdf#135495 above, keep the sfxlo-PriorityMergedHBox even though its not in sfx anymore
        xWindow = VclPtr<ContextVBox>::Create(pParent);
    }
    else if (name == "GtkIconView")
    {
        assert(rMap.find(u"model"_ustr) != rMap.end() && "GtkIconView must have a model");

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
        if (!isLegacy())
        {
            assert(rMap.find(u"model"_ustr) != rMap.end() && "GtkTreeView must have a model");
        }

        //window we want to apply the packing props for this GtkTreeView to
        VclPtr<vcl::Window> xWindowForPackingProps;
        //To-Do
        //a) make SvHeaderTabListBox/SvTabListBox the default target for GtkTreeView
        //b) remove the non-drop down mode of ListBox and convert
        //   everything over to SvHeaderTabListBox/SvTabListBox
        extractModel(id, rMap);
        WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;
        if (isLegacy())
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
        if (isLegacy())
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
                OUString containerid(id + "-container");
                xContainer->SetHelpId(getHelpRoot() + containerid);
                m_aChildren.emplace_back(containerid, xContainer, true);

                VclPtrInstance<HeaderBar> xHeader(xContainer, WB_BUTTONSTYLE | WB_BORDER | WB_TABSTOP | WB_3DLOOK);
                xHeader->set_width_request(0); // let the headerbar width not affect the size request
                OUString headerid(id + "-header");
                xHeader->SetHelpId(getHelpRoot() + headerid);
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
        if (!isLegacy())
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
        VclPtr<FixedImage> xFixedImage = VclPtr<FixedImage>::Create(pParent, WB_CENTER|WB_VCENTER|WB_3DLOOK|WB_SCALE);
        OUString sIconName = extractIconName(rMap);
        if (!sIconName.isEmpty())
            xFixedImage->SetImage(loadThemeImage(sIconName));
        m_pVclParserState->m_aImageSizeMap[id] = getImageSize(rMap);
        xWindow = xFixedImage;
        //such parentless GtkImages are temps used to set icons on buttons
        //default them to hidden to stop e.g. insert->index entry flicking temp
        //full screen windows
        if (!pParent)
        {
            rMap[u"visible"_ustr] = "false";
        }
    }
    else if (name == "GtkSeparator")
    {
        bVertical = hasOrientationVertical(rMap);
        xWindow = VclPtr<FixedLine>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
    }
    else if (name == "GtkScrollbar")
    {
        extractAdjustmentToMap(id, rMap, m_pVclParserState->m_aScrollAdjustmentMaps);
        bVertical = hasOrientationVertical(rMap);
        xWindow = VclPtr<ScrollBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ);
    }
    else if (name == "GtkProgressBar")
    {
        extractAdjustmentToMap(id, rMap, m_pVclParserState->m_aScrollAdjustmentMaps);
        bVertical = hasOrientationVertical(rMap);
        xWindow = VclPtr<ProgressBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ, ProgressBar::BarStyle::Progress);
    }
    else if (name == "GtkLevelBar")
    {
        extractAdjustmentToMap(id, rMap, m_pVclParserState->m_aScrollAdjustmentMaps);
        bVertical = hasOrientationVertical(rMap);
        xWindow = VclPtr<ProgressBar>::Create(pParent, bVertical ? WB_VERT : WB_HORZ, ProgressBar::BarStyle::Level);
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
            xWindow = VclPtr<VerticalTabControl>::Create(pParent, extractVerticalTabsWithIcons(rMap));
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
        extractAdjustmentToMap(id, rMap, m_pVclParserState->m_aSliderAdjustmentMaps);
        bool bDrawValue = extractDrawValue(rMap);
        if (bDrawValue)
        {
            OUString sValuePos = extractValuePos(rMap);
            (void)sValuePos;
        }
        bVertical = hasOrientationVertical(rMap);

        WinBits nWinStyle = bVertical ? WB_VERT : WB_HORZ;

        xWindow = VclPtr<Slider>::Create(pParent, nWinStyle);
    }
    else if (name == "GtkToolbar")
    {
        xWindow = VclPtr<ToolBox>::Create(pParent, WB_3DLOOK | WB_TABSTOP);
    }
    else if(name == "NotebookBarAddonsToolMergePoint")
    {
        customMakeWidget pFunction = GetCustomMakeWidget(u"sfxlo-NotebookbarToolBox"_ustr);
        if(pFunction != nullptr)
            NotebookBarAddonsMerger::MergeNotebookBarAddons(pParent, pFunction, m_xFrame, *m_pNotebookBarAddonsItem, rMap);
        return nullptr;
    }
    else if (isToolbarItemClass(name))
    {
        if (pToolBox)
        {
            OUString aCommand(extractActionName(rMap));

            ToolBoxItemId nItemId(0);
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
                nItemId = ToolBoxItemId(pToolBox->GetItemCount() + 1);
                    //TODO: ImplToolItems::size_type -> sal_uInt16!
                if (aCommand.isEmpty() && !isLegacy())
                    aCommand = id;
                pToolBox->InsertItem(nItemId, extractLabel(rMap), aCommand, nBits);
            }

            pToolBox->SetHelpId(nItemId, getHelpRoot() + id);
            OUString sTooltip(extractTooltipText(rMap));
            if (!sTooltip.isEmpty())
                pToolBox->SetQuickHelpText(nItemId, sTooltip);

            OUString sIconName(extractIconName(rMap));
            if (!sIconName.isEmpty())
                pToolBox->SetItemImage(nItemId, loadThemeImage(sIconName));

            if (!extractVisible(rMap))
                pToolBox->HideItem(nItemId);

            m_pVclParserState->m_nLastToolbarId = nItemId;

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
                    m_pVclParserState->m_aButtonMenuMaps.emplace_back(id, sMenu);
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

        xWindow->SetHelpId(getHelpRoot() + id);
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
            pToolBox->SetItemWindow(m_pVclParserState->m_nLastToolbarId, xWindow);
            pToolBox->SetItemExpand(m_pVclParserState->m_nLastToolbarId, true);
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
        for (auto const& [rKey, rValue] : rProps)
            pWindow->set_property(rKey, rValue);
    }

    OUString convertMnemonicMarkup(std::u16string_view rIn)
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
        return extractStringEntry(rMap, u"customproperty"_ustr);
    }

    void ensureDefaultWidthChars(VclBuilder::stringmap &rMap)
    {
        OUString sWidthChars(u"width-chars"_ustr);
        VclBuilder::stringmap::iterator aFind = rMap.find(sWidthChars);
        if (aFind == rMap.end())
            rMap[sWidthChars] = "20";
    }

    bool extractDropdown(VclBuilder::stringmap &rMap)
    {
        return extractBoolEntry(rMap, u"dropdown"_ustr, true);
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

    sal_Int16 getRoleFromName(const OUString& roleName)
    {
        using namespace com::sun::star::accessibility;

        static const std::unordered_map<OUString, sal_Int16> aAtkRoleToAccessibleRole = {
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
            { "notification",          AccessibleRole::NOTIFICATION },
            { "info bar",              AccessibleRole::UNKNOWN },
            { "level bar",             AccessibleRole::UNKNOWN },
            { "title bar",             AccessibleRole::UNKNOWN },
            { "block quote",           AccessibleRole::BLOCK_QUOTE },
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

VclPtr<vcl::Window> VclBuilder::insertObject(vcl::Window* pParent, const OUString& rClass,
                                             std::string_view, const OUString& rID,
                                             stringmap& rProps, stringmap& rPango, stringmap& rAtk)
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
            pCurrentChild->SetHelpId(getHelpRoot() + m_sID);
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
        pCurrentChild->set_id(rID);
        if (pCurrentChild == m_pParent.get() && m_bToplevelHasDeferredProperties)
            m_aDeferredProperties = rProps;
        else
            BuilderUtils::set_properties(pCurrentChild, rProps);

        // tdf#119827 handle size before scale so we can trivially
        // scale on the current font size whether size is present
        // or not.
        VclBuilder::stringmap::iterator aSize = rPango.find(u"size"_ustr);
        if (aSize != rPango.end())
        {
            pCurrentChild->set_font_attribute(aSize->first, aSize->second);
            rPango.erase(aSize);
        }
        for (auto const& [ rKey, rValue ] : rPango)
            pCurrentChild->set_font_attribute(rKey, rValue);

        m_pVclParserState->m_aAtkInfo[pCurrentChild] = rAtk;
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

void VclBuilder::applyTabChildProperties(vcl::Window* pParent, const std::vector<OUString>& rIDs,
                                         std::vector<vcl::EnumContext::Context>& rContext, stringmap& rProperties,
                                         stringmap& rAtkProperties)
{
    TabControl* pTabControl = isHorizontalTabControl(pParent) ? static_cast<TabControl*>(pParent) : nullptr;
    VerticalTabControl *pVerticalTabControl = pParent->GetType() == WindowType::VERTICALTABCONTROL ?
        static_cast<VerticalTabControl*>(pParent) : nullptr;
    assert(pTabControl || pVerticalTabControl);
    VclBuilder::stringmap::iterator aFind = rProperties.find(u"label"_ustr);
    if (aFind != rProperties.end())
    {
        OUString sTooltip(extractTooltipText(rProperties));
        if (pTabControl)
        {
            sal_uInt16 nPageId = pTabControl->GetCurPageId();
            pTabControl->SetPageText(nPageId, aFind->second);
            pTabControl->SetPageName(nPageId, rIDs.back());
            pTabControl->SetHelpText(nPageId, sTooltip);
            if (!rContext.empty())
            {
                TabPage* pPage = pTabControl->GetTabPage(nPageId);
                pPage->SetContext(std::move(rContext));
            }

            for (auto const& [ rKey, rValue ] : rAtkProperties)
            {
                if (rKey == "AtkObject::accessible-name")
                    pTabControl->SetAccessibleName(nPageId, rValue);
                else if (rKey == "AtkObject::accessible-description")
                    pTabControl->SetAccessibleDescription(nPageId, rValue);
                else
                    SAL_INFO("vcl.builder", "unhandled atk property: " << rKey);
            }

        }
        else
        {
            OUString sLabel(BuilderUtils::convertMnemonicMarkup(aFind->second));
            OUString sIconName(extractIconName(rProperties));
            pVerticalTabControl->InsertPage(rIDs.front(), sLabel, loadThemeImage(sIconName), sTooltip,
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

void VclBuilder::tweakInsertedChild(vcl::Window *pParent, vcl::Window* pCurrentChild,
                                    std::string_view sType, std::string_view sInternalChild)
{
    assert(pCurrentChild);

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
        if (sInternalChild.starts_with("vbox") || sInternalChild.starts_with("messagedialog-vbox"))
        {
            if (Dialog *pBoxParent = dynamic_cast<Dialog*>(pParent))
                pBoxParent->set_content_area(static_cast<VclBox*>(pCurrentChild)); // FIXME-VCLPTR
        }
        else if (sInternalChild.starts_with("action_area") || sInternalChild.starts_with("messagedialog-action_area"))
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

void BuilderBase::collectPangoAttribute(xmlreader::XmlReader& reader, stringmap& rMap)
{
    xmlreader::Span span;
    int nsId;

    OUString sProperty;
    OUString sValue;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span == "name")
        {
            span = reader.getAttributeValue(false);
            sProperty = OUString(span.begin, span.length, RTL_TEXTENCODING_UTF8);
        }
        else if (span == "value")
        {
            span = reader.getAttributeValue(false);
            sValue = OUString(span.begin, span.length, RTL_TEXTENCODING_UTF8);
        }
    }

    if (!sProperty.isEmpty())
        rMap[sProperty] = sValue;
}

void BuilderBase::collectAtkRelationAttribute(xmlreader::XmlReader& reader, stringmap& rMap)
{
    xmlreader::Span span;
    int nsId;

    OUString sProperty;
    OUString sValue;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span == "type")
        {
            span = reader.getAttributeValue(false);
            sProperty = OUString(span.begin, span.length, RTL_TEXTENCODING_UTF8);
        }
        else if (span == "target")
        {
            span = reader.getAttributeValue(false);
            sValue = OUString(span.begin, span.length, RTL_TEXTENCODING_UTF8);
            sal_Int32 nDelim = sValue.indexOf(':');
            if (nDelim != -1)
                sValue = sValue.copy(0, nDelim);
        }
    }

    if (!sProperty.isEmpty())
        rMap[sProperty] = sValue;
}

void BuilderBase::collectAtkRoleAttribute(xmlreader::XmlReader& reader, stringmap& rMap)
{
    xmlreader::Span span;
    int nsId;

    OUString sProperty;

    while (reader.nextAttribute(&nsId, &span))
    {
        if (span == "type")
        {
            span = reader.getAttributeValue(false);
            sProperty = OUString(span.begin, span.length, RTL_TEXTENCODING_UTF8);
        }
    }

    if (!sProperty.isEmpty())
        rMap[u"role"_ustr] = sProperty;
}

void BuilderBase::handleRow(xmlreader::XmlReader& reader, const OUString& rID)
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
                        nId = o3tl::toUInt32(std::string_view(name.begin, name.length));
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
                    sFinalValue = Translate::get(TranslateId{ sContext.getStr(), sValue.getStr() },
                                                 getResLocale());
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

void BuilderBase::handleListStore(xmlreader::XmlReader& reader, const OUString& rID, std::u16string_view rClass)
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
                bool bNotTreeStore = rClass != u"GtkTreeStore";
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

BuilderBase::stringmap BuilderBase::handleAtkObject(xmlreader::XmlReader& reader) const
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

void VclBuilder::applyAtkProperties(vcl::Window *pWindow, const stringmap& rProperties, bool bToolbarItem)
{
    assert(pWindow);
    for (auto const& [ rKey, rValue ] : rProperties)
    {
        if (bToolbarItem)
        {
            // apply property to the corresponding toolbar item (which is not a vcl::Window itself)
            // rather than the toolbar itself
            ToolBox* pToolBox = dynamic_cast<ToolBox*>(pWindow);
            if (pToolBox)
            {
                if (rKey == u"AtkObject::accessible-name")
                    pToolBox->SetAccessibleName(m_pVclParserState->m_nLastToolbarId, rValue);
            }
        }
        else if (pWindow && rKey.match("AtkObject::"))
            pWindow->set_property(rKey.copy(RTL_CONSTASCII_LENGTH("AtkObject::")), rValue);
        else
            SAL_WARN("vcl.builder", "unhandled atk prop: " << rKey);
    }
}

void VclBuilder::setMnemonicWidget(const OUString& rLabelId, const OUString& rMnemonicWidgetId)
{
    FixedText* pOne = get<FixedText>(rLabelId);
    vcl::Window* pOther = get(rMnemonicWidgetId);
    SAL_WARN_IF(!pOne || !pOther, "vcl",
                "missing either source " << rLabelId << " or target " << rMnemonicWidgetId
                                         << " member of Mnemonic Widget Mapping");
    if (pOne && pOther)
        pOne->set_mnemonic_widget(pOther);
}

void VclBuilder::setPriority(vcl::Window* pWindow, int nPriority)
{
    vcl::IPrioritable* pPrioritable = dynamic_cast<vcl::IPrioritable*>(pWindow);
    SAL_WARN_IF(!pPrioritable, "vcl", "priority set for not supported item");
    if (pPrioritable)
        pPrioritable->SetPriority(nPriority);
}
void VclBuilder::setContext(vcl::Window* pWindow, std::vector<vcl::EnumContext::Context>&& aContext)
{
    vcl::IContext* pContextControl = dynamic_cast<vcl::IContext*>(pWindow);
    SAL_WARN_IF(!pContextControl, "vcl", "context set for not supported item");
    if (pContextControl)
        pContextControl->SetContext(std::move(aContext));
}

bool VclBuilder::isHorizontalTabControl(vcl::Window* pWindow)
{
    return pWindow && pWindow->GetType() == WindowType::TABCONTROL;
}

VclPtr<PopupMenu> VclBuilder::createMenu(const OUString& rID)
{
    VclPtr<PopupMenu> pMenu = VclPtr<PopupMenu>::Create();
    pMenu->set_id(rID);
    return pMenu;
}

std::vector<ComboBoxTextItem> BuilderBase::handleItems(xmlreader::XmlReader& reader) const
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
                OString sContext;
                OUString sId;

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
                        sId = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
                    }
                }

                (void)reader.nextItem(
                    xmlreader::XmlReader::Text::Raw, &name, &nsId);

                OString sValue(name.begin, name.length);
                const OUString sFinalValue = finalizeValue(sContext, sValue, bTranslated);
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

void BuilderBase::handleSizeGroup(xmlreader::XmlReader& reader)
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
                        OUString sWidget(name.begin, name.length, RTL_TEXTENCODING_UTF8);
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
    vcl::KeyCode makeKeyCode(const std::pair<OUString,OUString> &rKey)
    {
        bool bShift = rKey.second.indexOf("GDK_SHIFT_MASK") != -1;
        bool bMod1 = rKey.second.indexOf("GDK_CONTROL_MASK") != -1;
        bool bMod2 = rKey.second.indexOf("GDK_ALT_MASK") != -1;
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
        else if (rKey.first.getLength() > 1 && rKey.first[0] == 'F')
        {
            sal_uInt32 nIndex = o3tl::toUInt32(rKey.first.subView(1));
            assert(nIndex >= 1 && nIndex <= 26);
            return vcl::KeyCode(KEY_F1 + nIndex - 1, bShift, bMod1, bMod2, bMod3);
        }

        assert (rKey.first.getLength() == 1);
        sal_Unicode cChar = rKey.first.toChar();

        if (cChar >= 'a' && cChar <= 'z')
            return vcl::KeyCode(KEY_A + (cChar - 'a'), bShift, bMod1, bMod2, bMod3);
        else if (cChar >= 'A' && cChar <= 'Z')
            return vcl::KeyCode(KEY_A + (cChar - 'A'), bShift, bMod1, bMod2, bMod3);
        else if (cChar >= '0' && cChar <= '9')
            return vcl::KeyCode(KEY_0 + (cChar - 'A'), bShift, bMod1, bMod2, bMod3);

        return vcl::KeyCode(cChar, bShift, bMod1, bMod2, bMod3);
    }
}

void VclBuilder::insertMenuObject(PopupMenu* pParent, PopupMenu* pSubMenu, const OUString& rClass,
                                  const OUString& rID, stringmap& rProps, stringmap& rAtkProps,
                                  accelmap& rAccels)
{
    sal_uInt16 nOldCount = pParent->GetItemCount();
    sal_uInt16 nNewId = ++m_pVclParserState->m_nLastMenuItemId;

    if(rClass == "NotebookBarAddonsMenuMergePoint")
    {
        NotebookBarAddonsMerger::MergeNotebookBarMenuAddons(pParent, nNewId, rID, *m_pNotebookBarAddonsItem);
        m_pVclParserState->m_nLastMenuItemId = pParent->GetItemCount();
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
        pParent->SetHelpId(nNewId, getHelpRoot() + rID);
        if (!extractVisible(rProps))
            pParent->HideItem(nNewId);

        for (auto const& [ rKey, rValue ] : rProps)
        {
            if (rKey == "tooltip-markup")
                pParent->SetTipHelpText(nNewId, rValue);
            else if (rKey == "tooltip-text")
                pParent->SetTipHelpText(nNewId, rValue);
            else
                SAL_INFO("vcl.builder", "unhandled property: " << rKey);
        }

        for (auto const& [ rKey, rValue ] : rAtkProps)
        {
            if (rKey == "AtkObject::accessible-name")
                pParent->SetAccessibleName(nNewId, rValue);
            else if (rKey == "AtkObject::accessible-description")
                pParent->SetAccessibleDescription(nNewId, rValue);
            else
                SAL_INFO("vcl.builder", "unhandled atk property: " << rKey);
        }

        for (auto const& [ rSignal, rValue ] : rAccels)
        {
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

    sal_uInt16 nActiveId = BuilderBase::extractActive(rMap);
    for (auto const& item : rItems)
    {
        sal_Int32 nPos = pContainer->InsertEntry(item.m_sItem);
        if (!item.m_sId.isEmpty())
        {
            rUserData.emplace_back(std::make_unique<OUString>(item.m_sId));
            pContainer->SetEntryData(nPos, rUserData.back().get());
        }
    }
    if (nActiveId < rItems.size())
        pContainer->SelectEntryPos(nActiveId);

    return true;
}

void BuilderBase::extractClassAndIdAndCustomProperty(xmlreader::XmlReader& reader, OUString& rClass,
                                                     OUString& rId, OUString& rCustomProperty)
{
    xmlreader::Span name;
    int nsId;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name == "class")
        {
            name = reader.getAttributeValue(false);
            rClass = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
        }
        else if (name == "id")
        {
            name = reader.getAttributeValue(false);
            rId = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
            if (isLegacy())
            {
                sal_Int32 nDelim = rId.indexOf(':');
                if (nDelim != -1)
                {
                    rCustomProperty = rId.subView(nDelim+1);
                    rId = rId.copy(0, nDelim);
                }
            }
        }
    }
}


Image BuilderBase::loadThemeImage(const OUString& rFileName)
{
    return Image(StockImage::Yes, rFileName);
}

void BuilderBase::handleInterfaceDomain(xmlreader::XmlReader& rReader)
{
    xmlreader::Span name = rReader.getAttributeValue(false);
    const OString sPrefixName(name.begin, name.length);
    m_pParserState->m_aResLocale = Translate::Create(sPrefixName);
}

BuilderBase::stringmap BuilderBase::collectPackingProperties(xmlreader::XmlReader& reader)
{
    int nLevel = 1;
    stringmap aPackingProperties;

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
            if (name == "property")
                collectProperty(reader, aPackingProperties);
        }

        if (res == xmlreader::XmlReader::Result::End)
        {
            --nLevel;
        }

        if (!nLevel)
            break;
    }

    return aPackingProperties;
}

void VclBuilder::applyPackingProperties(vcl::Window* pCurrent, vcl::Window* pParent,
                                        const stringmap& rPackingProperties)
{
    if (!pCurrent)
        return;

    //ToolBoxItems are not true widgets just elements
    //of the ToolBox itself
    ToolBox *pToolBoxParent = nullptr;
    if (pCurrent == pParent)
        pToolBoxParent = dynamic_cast<ToolBox*>(pParent);

    if (pCurrent->GetType() == WindowType::SCROLLWINDOW)
    {
        auto aFind = m_pVclParserState->m_aRedundantParentWidgets.find(VclPtr<vcl::Window>(pCurrent));
        if (aFind != m_pVclParserState->m_aRedundantParentWidgets.end())
        {
            pCurrent = aFind->second;
            assert(pCurrent);
        }
    }

    for (auto const& [rKey, rValue] : rPackingProperties)
    {
        if (rKey == u"expand" || rKey == u"resize")
        {
            bool bTrue = toBool(rValue);
            if (pToolBoxParent)
                pToolBoxParent->SetItemExpand(m_pVclParserState->m_nLastToolbarId, bTrue);
            else
                pCurrent->set_expand(bTrue);
            continue;
        }

        if (pToolBoxParent)
            continue;

        if (rKey == u"fill")
        {
            pCurrent->set_fill(toBool(rValue));
        }
        else if (rKey == u"pack-type")
        {
            VclPackType ePackType = (!rValue.isEmpty() && (rValue[0] == 'e' || rValue[0] == 'E')) ? VclPackType::End : VclPackType::Start;
            pCurrent->set_pack_type(ePackType);
        }
        else if (rKey == u"left-attach")
        {
            pCurrent->set_grid_left_attach(rValue.toInt32());
        }
        else if (rKey == u"top-attach")
        {
            pCurrent->set_grid_top_attach(rValue.toInt32());
        }
        else if (rKey == u"width")
        {
            pCurrent->set_grid_width(rValue.toInt32());
        }
        else if (rKey == u"height")
        {
            pCurrent->set_grid_height(rValue.toInt32());
        }
        else if (rKey == u"padding")
        {
            pCurrent->set_padding(rValue.toInt32());
        }
        else if (rKey == u"position")
        {
            set_window_packing_position(pCurrent, rValue.toInt32());
        }
        else if (rKey == u"secondary")
        {
            pCurrent->set_secondary(toBool(rValue));
        }
        else if (rKey == u"non-homogeneous")
        {
            pCurrent->set_non_homogeneous(toBool(rValue));
        }
        else if (rKey == u"homogeneous")
        {
            pCurrent->set_non_homogeneous(!toBool(rValue));
        }
        else
        {
            SAL_WARN_IF(rKey != u"shrink", "vcl.builder", "unknown packing: " << rKey);
        }
    }
}

std::vector<vcl::EnumContext::Context> BuilderBase::handleStyle(xmlreader::XmlReader &reader, int &nPriority)
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
                OUString classStyle = getStyleClass(reader);
                std::u16string_view rest;

                if (classStyle.startsWith("context-", &rest))
                {
                    aContext.push_back(vcl::EnumContext::GetContextEnum(OUString(rest)));
                }
                else if (classStyle.startsWith("priority-", &rest))
                {
                    nPriority = o3tl::toInt32(rest);
                }
                else if (classStyle != "small-button" && classStyle != "destructive-action" && classStyle != "suggested-action")
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

OUString BuilderBase::getStyleClass(xmlreader::XmlReader &reader)
{
    xmlreader::Span name;
    int nsId;
    OUString aRet;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name == "name")
        {
            name = reader.getAttributeValue(false);
            aRet = OUString (name.begin, name.length, RTL_TEXTENCODING_UTF8);
        }
    }

    return aRet;
}

bool BuilderBase::hasOrientationVertical(VclBuilder::stringmap &rMap)
{
    bool bVertical = false;
    VclBuilder::stringmap::iterator aFind = rMap.find(u"orientation"_ustr);
    if (aFind != rMap.end())
    {
        bVertical = aFind->second.equalsIgnoreAsciiCase("vertical");
        rMap.erase(aFind);
    }
    return bVertical;
}

sal_Int32 BuilderBase::extractActive(VclBuilder::stringmap& rMap)
{
    sal_Int32 nActiveId = 0;
    VclBuilder::stringmap::iterator aFind = rMap.find(u"active"_ustr);
    if (aFind != rMap.end())
    {
        nActiveId = aFind->second.toInt32();
        rMap.erase(aFind);
    }
    return nActiveId;
}

bool BuilderBase::extractEntry(VclBuilder::stringmap &rMap)
{
    return extractBoolEntry(rMap, u"has-entry"_ustr, false);
}

bool BuilderBase::extractHeadersVisible(VclBuilder::stringmap& rMap)
{
    return extractBoolEntry(rMap, u"headers-visible"_ustr, true);
}

OUString BuilderBase::extractIconName(VclBuilder::stringmap &rMap)
{
    OUString sIconName;
    // allow pixbuf, but prefer icon-name
    {
        VclBuilder::stringmap::iterator aFind = rMap.find(u"pixbuf"_ustr);
        if (aFind != rMap.end())
        {
            sIconName = aFind->second;
            rMap.erase(aFind);
        }
    }
    {
        VclBuilder::stringmap::iterator aFind = rMap.find(u"icon-name"_ustr);
        if (aFind != rMap.end())
        {
            sIconName = aFind->second;
            rMap.erase(aFind);
        }
    }
    if (sIconName == "missing-image")
        return OUString();
    OUString sReplace = mapStockToImageResource(sIconName);
    return !sReplace.isEmpty() ? sReplace : sIconName;
}

bool BuilderBase::extractResizable(stringmap& rMap)
{
    return extractBoolEntry(rMap, u"resizable"_ustr, true);
}

bool BuilderBase::extractShowExpanders(VclBuilder::stringmap& rMap)
{
    return extractBoolEntry(rMap, u"show-expanders"_ustr, true);
}

OUString BuilderBase::extractTooltipText(stringmap& rMap)
{
    OUString sTooltipText;
    VclBuilder::stringmap::iterator aFind = rMap.find(u"tooltip-text"_ustr);
    if (aFind == rMap.end())
        aFind = rMap.find(u"tooltip-markup"_ustr);
    if (aFind != rMap.end())
    {
        sTooltipText = aFind->second;
        rMap.erase(aFind);
    }
    return sTooltipText;
}

bool BuilderBase::extractVisible(VclBuilder::stringmap& rMap)
{
    return extractBoolEntry(rMap, u"visible"_ustr, false);
}

void BuilderBase::collectProperty(xmlreader::XmlReader& reader, stringmap& rMap) const
{
    xmlreader::Span name;
    int nsId;

    OUString sProperty;
    OString sContext;

    bool bTranslated = false;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name == "name")
        {
            name = reader.getAttributeValue(false);
            sProperty = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
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

    if (!sProperty.isEmpty())
    {
        OString sValue(name.begin, name.length);
        const OUString sFinalValue = finalizeValue(sContext, sValue, bTranslated);
        sProperty = sProperty.replace('_', '-');
        rMap[sProperty] = sFinalValue;
    }
}

void BuilderBase::handleActionWidget(xmlreader::XmlReader &reader)
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
    OUString sID(name.begin, name.length, RTL_TEXTENCODING_UTF8);
    sal_Int32 nDelim = sID.indexOf(':');
    if (nDelim != -1)
        sID = sID.copy(0, nDelim);

    short nResponse = sResponse.toInt32();
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

    set_response(sID, nResponse);
}

void BuilderBase::collectAccelerator(xmlreader::XmlReader& reader, accelmap& rMap)
{
    xmlreader::Span name;
    int nsId;

    OUString sProperty;
    OUString sValue;
    OUString sModifiers;

    while (reader.nextAttribute(&nsId, &name))
    {
        if (name == "key")
        {
            name = reader.getAttributeValue(false);
            sValue = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
        }
        else if (name == "signal")
        {
            name = reader.getAttributeValue(false);
            sProperty = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
        }
        else if (name == "modifiers")
        {
            name = reader.getAttributeValue(false);
            sModifiers = OUString(name.begin, name.length, RTL_TEXTENCODING_UTF8);
        }
    }

    if (!sProperty.isEmpty() && !sValue.isEmpty())
    {
        rMap[sProperty] = std::make_pair(sValue, sModifiers);
    }
}

bool BuilderBase::isToolbarItemClass(std::u16string_view sClass)
{
    return sClass == u"GtkToolButton" || sClass == u"GtkMenuToolButton"
           || sClass == u"GtkToggleToolButton" || sClass == u"GtkRadioToolButton"
           || sClass == u"GtkToolItem";
}

vcl::Window *VclBuilder::get_widget_root()
{
    return m_aChildren.empty() ? nullptr : m_aChildren[0].m_pWindow.get();
}

void VclBuilder::resetParserState()
{
    m_pVclParserState.reset();
    BuilderBase::resetParserState();
}

vcl::Window *VclBuilder::get_by_name(std::u16string_view sID)
{
    for (auto const& child : m_aChildren)
    {
        if (child.m_sID == sID)
            return child.m_pWindow;
    }

    return nullptr;
}

void VclBuilder::set_response(std::u16string_view sID, short nResponse)
{
    PushButton* pPushButton = get<PushButton>(sID);
    assert(pPushButton);
    Dialog* pDialog = pPushButton->GetParentDialog();
    assert(pDialog);
    pDialog->add_button(pPushButton, nResponse, false);
    return;
}

void VclBuilder::delete_by_name(const OUString& sID)
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

OUString VclBuilder::get_by_window(const vcl::Window *pWindow) const
{
    for (auto const& child : m_aChildren)
    {
        if (child.m_pWindow == pWindow)
            return child.m_sID;
    }

    return {};
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

const BuilderBase::ListStore* BuilderBase::get_model_by_name(const OUString& sID) const
{
    const auto aI = m_pParserState->m_aModels.find(sID);
    if (aI != m_pParserState->m_aModels.end())
        return &(aI->second);
    return nullptr;
}

void BuilderBase::addTextBuffer(const OUString& sID, const TextBuffer& rTextBuffer)
{
    m_pParserState->m_aTextBuffers[sID] = rTextBuffer;
}

const BuilderBase::TextBuffer* BuilderBase::get_buffer_by_name(const OUString& sID) const
{
    const auto aI = m_pParserState->m_aTextBuffers.find(sID);
    if (aI != m_pParserState->m_aTextBuffers.end())
        return &(aI->second);
    return nullptr;
}

void BuilderBase::addAdjustment(const OUString& sID, const Adjustment& rAdjustment)
{
    m_pParserState->m_aAdjustments[sID] = rAdjustment;
}

const BuilderBase::Adjustment* BuilderBase::get_adjustment_by_name(const OUString& sID) const
{
    const auto aI = m_pParserState->m_aAdjustments.find(sID);
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
            if (isLegacy())
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
            if (isLegacy())
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
            if (isLegacy())
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

void VclBuilder::insertComboBoxOrListBoxItems(vcl::Window *pWindow, VclBuilder::stringmap &rMap,
                                  const std::vector<ComboBoxTextItem>& rItems)
{
    // try to fill-in the items
    if (!insertItems<ComboBox>(pWindow, rMap, m_aUserData, rItems))
        insertItems<ListBox>(pWindow, rMap, m_aUserData, rItems);
}

void VclBuilder::mungeAdjustment(NumericFormatter &rTarget, const Adjustment &rAdjustment)
{
    int nMul = rtl_math_pow10Exp(1, rTarget.GetDecimalDigits());

    for (auto const& [ rKey, rValue ] : rAdjustment)
    {
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

    for (auto const& [ rKey, rValue ] : rAdjustment)
    {
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
    for (auto const& [ rKey, rValue ] : rAdjustment)
    {
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
    for (auto const& [ rKey, rValue ] : rAdjustment)
    {
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
    for (auto const& [ rKey, rValue ] : rTextBuffer)
    {
        if (rKey == "text")
            rTarget.SetText(rValue);
        else
        {
            SAL_INFO("vcl.builder", "unhandled property :" << rKey);
        }
    }
}

VclBuilder::VclParserState::VclParserState()
    : m_nLastToolbarId(0)
    , m_nLastMenuItemId(0)
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
