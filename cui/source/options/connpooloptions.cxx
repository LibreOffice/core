/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include "connpooloptions.hxx"
#include "connpoolsettings.hxx"
#include <svl/eitem.hxx>
#include <svx/databaseregistrationui.hxx>
#include <strings.hrc>
#include <dialmgr.hxx>
#include <officecfg/Office/DataAccess.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

namespace offapp
{
    bool ConnectionPoolOptionsPage::isModifiedDriverList() const
    {
        if (m_aSettings.size() != m_aSavedSettings.size())
            return true;

        DriverPoolingSettings::const_iterator aSaved = m_aSavedSettings.begin();
        for (auto const& currentSetting : m_aSettings)
        {
            if (currentSetting != *aSaved)
                return true;
            ++aSaved;
        }

        return false;
    }

    ConnectionPoolOptionsPage::ConnectionPoolOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
        : SfxTabPage(pPage, pController, u"cui/ui/connpooloptions.ui"_ustr, u"ConnPoolPage"_ustr, &_rAttrSet)
        , m_sYes(CuiResId(RID_CUISTR_YES))
        , m_sNo(CuiResId(RID_CUISTR_NO))
        , m_xEnablePooling(m_xBuilder->weld_check_button(u"connectionpooling"_ustr))
        , m_xEnablePoolingImg(m_xBuilder->weld_widget(u"lockconnectionpooling"_ustr))
        , m_xDriversLabel(m_xBuilder->weld_label(u"driverslabel"_ustr))
        , m_xDriverList(m_xBuilder->weld_tree_view(u"driverlist"_ustr))
        , m_xDriverLabel(m_xBuilder->weld_label(u"driverlabel"_ustr))
        , m_xDriver(m_xBuilder->weld_label(u"driver"_ustr))
        , m_xDriverPoolingEnabled(m_xBuilder->weld_check_button(u"enablepooling"_ustr))
        , m_xDriverPoolingEnabledImg(m_xBuilder->weld_widget(u"lockenablepooling"_ustr))
        , m_xTimeoutLabel(m_xBuilder->weld_label(u"timeoutlabel"_ustr))
        , m_xTimeout(m_xBuilder->weld_spin_button(u"timeout"_ustr))
        , m_xTimeoutImg(m_xBuilder->weld_widget(u"locktimeout"_ustr))
    {
        m_xDriverList->set_size_request(m_xDriverList->get_approximate_digit_width() * 60,
                                        m_xDriverList->get_height_rows(15));
        m_xDriverList->show();

        std::vector<int> aWidths
        {
            o3tl::narrowing<int>(m_xDriverList->get_approximate_digit_width() * 50),
            o3tl::narrowing<int>(m_xDriverList->get_approximate_digit_width() * 8)
        };
        m_xDriverList->set_column_fixed_widths(aWidths);

        css::uno::Reference < css::uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
        m_xReadWriteAccess = css::configuration::ReadWriteAccess::create(xContext, u"*"_ustr);

        m_xEnablePooling->connect_toggled( LINK(this, ConnectionPoolOptionsPage, OnEnabledDisabled) );
        m_xDriverPoolingEnabled->connect_toggled( LINK(this, ConnectionPoolOptionsPage, OnEnabledDisabled) );

        m_xDriverList->connect_changed(LINK(this, ConnectionPoolOptionsPage, OnDriverRowChanged));
        m_xTimeout->connect_value_changed(LINK(this, ConnectionPoolOptionsPage, OnSpinValueChanged));
    }

    void ConnectionPoolOptionsPage::updateRow(size_t nRow)
    {
        auto const& currentSetting = m_aSettings[nRow];
        m_xDriverList->set_text(nRow, currentSetting.sName, 0);
        if (currentSetting.bEnabled)
        {
            m_xDriverList->set_text(nRow, m_sYes, 1);
            m_xDriverList->set_text(nRow, OUString::number(currentSetting.nTimeoutSeconds), 2);
        }
        else
        {
            m_xDriverList->set_text(nRow, m_sNo, 1);
            m_xDriverList->set_text(nRow, u"-"_ustr, 2);
        }
    }

    void ConnectionPoolOptionsPage::updateCurrentRow()
    {
        int nRow = m_xDriverList->get_selected_index();
        if (nRow == -1)
            return;
        updateRow(nRow);
    }

    void ConnectionPoolOptionsPage::UpdateDriverList(const DriverPoolingSettings& _rSettings)
    {
        m_aSettings = _rSettings;

        m_xDriverList->freeze();
        m_xDriverList->clear();

        for (size_t i = 0; i < m_aSettings.size(); ++i)
        {
            m_xDriverList->append();
            updateRow(i);
        }

        m_xDriverList->thaw();

        if (!m_aSettings.empty())
        {
            m_xDriverList->select(0);
            OnDriverRowChanged(*m_xDriverList);
        }
    }

    ConnectionPoolOptionsPage::~ConnectionPoolOptionsPage()
    {
    }

    std::unique_ptr<SfxTabPage> ConnectionPoolOptionsPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet)
    {
        return std::make_unique<ConnectionPoolOptionsPage>(pPage, pController, *_rAttrSet);
    }

    void ConnectionPoolOptionsPage::implInitControls(const SfxItemSet& _rSet)
    {
        // the enabled flag
        const SfxBoolItem* pEnabled = _rSet.GetItem<SfxBoolItem>(SID_SB_POOLING_ENABLED);
        OSL_ENSURE(pEnabled, "ConnectionPoolOptionsPage::implInitControls: missing the Enabled item!");
        m_xEnablePooling->set_active(pEnabled == nullptr || pEnabled->GetValue());
        m_xEnablePooling->set_sensitive(!officecfg::Office::DataAccess::ConnectionPool::EnablePooling::isReadOnly());
        m_xEnablePoolingImg->set_visible(officecfg::Office::DataAccess::ConnectionPool::EnablePooling::isReadOnly());

        m_xEnablePooling->save_state();

        // the settings for the single drivers
        const DriverPoolingSettingsItem* pDriverSettings = _rSet.GetItem<DriverPoolingSettingsItem>(SID_SB_DRIVER_TIMEOUTS);
        if (pDriverSettings)
            UpdateDriverList(pDriverSettings->getSettings());
        else
        {
            SAL_WARN("cui.options", "ConnectionPoolOptionsPage::implInitControls: missing the DriverTimeouts item!");
            UpdateDriverList(DriverPoolingSettings());
        }
        saveDriverList();

        // reflect the new settings
        OnEnabledDisabled(*m_xEnablePooling);
    }

    IMPL_LINK_NOARG(ConnectionPoolOptionsPage, OnSpinValueChanged, weld::SpinButton&, void)
    {
        commitTimeoutField();
    }

    OUString ConnectionPoolOptionsPage::GetAllStrings()
    {
        OUString sAllStrings;
        OUString labels[] = { u"label1"_ustr, u"driverslabel"_ustr, u"driverlabel"_ustr, u"timeoutlabel"_ustr, u"driver"_ustr };

        for (const auto& label : labels)
        {
            if (const auto& pString = m_xBuilder->weld_label(label))
                sAllStrings += pString->get_label() + " ";
        }

        OUString checkButton[] = { u"connectionpooling"_ustr, u"enablepooling"_ustr };

        for (const auto& check : checkButton)
        {
            if (const auto& pString = m_xBuilder->weld_check_button(check))
                sAllStrings += pString->get_label() + " ";
        }

        return sAllStrings.replaceAll("_", "");
    }

    bool ConnectionPoolOptionsPage::FillItemSet(SfxItemSet* _rSet)
    {
        commitTimeoutField();

        bool bModified = false;
        // the enabled flag
        if (m_xEnablePooling->get_state_changed_from_saved())
        {
            _rSet->Put(SfxBoolItem(SID_SB_POOLING_ENABLED, m_xEnablePooling->get_active()));
            bModified = true;
        }

        // the settings for the single drivers
        if (isModifiedDriverList())
        {
            _rSet->Put(DriverPoolingSettingsItem(SID_SB_DRIVER_TIMEOUTS, m_aSettings));
            bModified = true;
        }

        return bModified;
    }

    void ConnectionPoolOptionsPage::ActivatePage( const SfxItemSet& _rSet)
    {
        SfxTabPage::ActivatePage(_rSet);
        implInitControls(_rSet);
    }

    void ConnectionPoolOptionsPage::Reset(const SfxItemSet* _rSet)
    {
        implInitControls(*_rSet);
    }

    IMPL_LINK_NOARG(ConnectionPoolOptionsPage, OnDriverRowChanged, weld::TreeView&, void)
    {
        const int nDriverPos = m_xDriverList->get_selected_index();
        bool bValidRow = (nDriverPos != -1);
        m_xDriverPoolingEnabled->set_sensitive(bValidRow && m_xEnablePooling->get_active());
        m_xTimeoutLabel->set_sensitive(bValidRow);
        m_xTimeout->set_sensitive(bValidRow);

        if (!bValidRow)
        {   // positioned on an invalid row
            m_xDriver->set_label(OUString());
        }
        else
        {
            auto const& currentSetting = m_aSettings[nDriverPos];
            m_xDriver->set_label(currentSetting.sName);
            m_xDriverPoolingEnabled->set_active(currentSetting.bEnabled);
            m_xTimeout->set_value(currentSetting.nTimeoutSeconds);

            OUString aConfigPath = officecfg::Office::DataAccess::ConnectionPool::DriverSettings::path() + "/" + currentSetting.sName;
            css::beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath + "/Enable");
            bool bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;

            m_xDriverPoolingEnabled->set_sensitive(!bReadOnly);
            m_xDriverPoolingEnabledImg->set_visible(bReadOnly);

            aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath + "/Timeout");
            bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;

            m_xTimeout->set_sensitive(!bReadOnly);
            m_xTimeoutLabel->set_sensitive(!bReadOnly);
            m_xTimeoutImg->set_visible(bReadOnly);

            OnEnabledDisabled(*m_xDriverPoolingEnabled);
        }
    }

    void ConnectionPoolOptionsPage::commitTimeoutField()
    {
        const int nDriverPos = m_xDriverList->get_selected_index();
        if (nDriverPos == -1)
            return;
        m_aSettings[nDriverPos].nTimeoutSeconds = m_xTimeout->get_value();
        updateCurrentRow();
    }

    IMPL_LINK( ConnectionPoolOptionsPage, OnEnabledDisabled, weld::Toggleable&, rCheckBox, void )
    {
        bool bGloballyEnabled = m_xEnablePooling->get_active();
        bool bLocalDriverChanged = m_xDriverPoolingEnabled.get() == &rCheckBox;

        if (m_xEnablePooling.get() == &rCheckBox)
        {
            m_xDriversLabel->set_sensitive(bGloballyEnabled);
            m_xDriverList->set_sensitive(bGloballyEnabled);
            if (!bGloballyEnabled)
                m_xDriverList->select(-1);
            m_xDriverLabel->set_sensitive(bGloballyEnabled);
            m_xDriver->set_sensitive(bGloballyEnabled);
            m_xDriverPoolingEnabled->set_sensitive(bGloballyEnabled && !m_xDriverPoolingEnabledImg->get_visible());
        }
        else
            OSL_ENSURE(bLocalDriverChanged, "ConnectionPoolOptionsPage::OnEnabledDisabled: where did this come from?");

        m_xTimeoutLabel->set_sensitive(bGloballyEnabled && m_xDriverPoolingEnabled->get_active() && !m_xTimeoutImg->get_visible());
        m_xTimeout->set_sensitive(bGloballyEnabled && m_xDriverPoolingEnabled->get_active() && !m_xTimeoutImg->get_visible());

        if (bLocalDriverChanged)
        {
            // update the list
            const int nDriverPos = m_xDriverList->get_selected_index();
            if (nDriverPos == -1)
                return;
            m_aSettings[nDriverPos].bEnabled = m_xDriverPoolingEnabled->get_active();
            updateCurrentRow();
        }
    }

} // namespace offapp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
