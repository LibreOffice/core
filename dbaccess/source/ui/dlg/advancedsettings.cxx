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

#include <sal/config.h>

#include <memory>

#include "advancedsettings.hxx"
#include <advancedsettingsdlg.hxx>
#include <dsitems.hxx>
#include "DbAdminImpl.hxx"
#include "DriverSettings.hxx"
#include "optionalboolitem.hxx"
#include <dbu_dlg.hxx>

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XDriver;

    // SpecialSettingsPage
    struct BooleanSettingDesc
    {
        std::unique_ptr<weld::CheckButton>& xControl; // the dialog's control which displays this setting
        OString     sControlId;         // the widget name of the control in the .ui
        sal_uInt16  nItemId;            // the ID of the item (in an SfxItemSet) which corresponds to this setting
        bool        bInvertedDisplay;   // true if and only if the checkbox is checked when the item is sal_False, and vice versa
        bool        bOptionalBool;      // type is OptionalBool
    };

    // SpecialSettingsPage
    SpecialSettingsPage::SpecialSettingsPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs, const DataSourceMetaData& _rDSMeta)
        : OGenericAdministrationPage(pParent, "dbaccess/ui/specialsettingspage.ui", "SpecialSettingsPage", _rCoreAttrs)
        , m_aBooleanSettings {
            { m_xIsSQL92Check,               "usesql92",        DSID_SQL92CHECK,            false, false },
            { m_xAppendTableAlias,           "append",          DSID_APPEND_TABLE_ALIAS,    false, false },
            { m_xAsBeforeCorrelationName,    "useas",           DSID_AS_BEFORE_CORRNAME,    false, false },
            { m_xEnableOuterJoin,            "useoj",           DSID_ENABLEOUTERJOIN,       false, false },
            { m_xIgnoreDriverPrivileges,     "ignoreprivs",     DSID_IGNOREDRIVER_PRIV,     false, false },
            { m_xParameterSubstitution,      "replaceparams",   DSID_PARAMETERNAMESUBST,    false, false },
            { m_xSuppressVersionColumn,      "displayver",      DSID_SUPPRESSVERSIONCL,     true,  false },
            { m_xCatalog,                    "usecatalogname",  DSID_CATALOG,               false, false },
            { m_xSchema,                     "useschemaname",   DSID_SCHEMA,                false, false },
            { m_xIndexAppendix,              "createindex",     DSID_INDEXAPPENDIX,         false, false },
            { m_xDosLineEnds,                "eol",             DSID_DOSLINEENDS,           false, false },
            { m_xCheckRequiredFields,        "inputchecks",     DSID_CHECK_REQUIRED_FIELDS, false, false },
            { m_xIgnoreCurrency,             "ignorecurrency",  DSID_IGNORECURRENCY,        false, false },
            { m_xEscapeDateTime,             "useodbcliterals", DSID_ESCAPE_DATETIME,       false, false },
            { m_xPrimaryKeySupport,          "primarykeys",     DSID_PRIMARY_KEY_SUPPORT,   false, false },
            { m_xRespectDriverResultSetType, "resulttype",      DSID_RESPECTRESULTSETTYPE,  false, false } }
        , m_bHasBooleanComparisonMode( _rDSMeta.getFeatureSet().has( DSID_BOOLEANCOMPARISON ) )
        , m_bHasMaxRowScan( _rDSMeta.getFeatureSet().has( DSID_MAX_ROW_SCAN ) )
    {
        const FeatureSet& rFeatures( _rDSMeta.getFeatureSet() );
        // create all the check boxes for the boolean settings
        for (auto & booleanSetting : m_aBooleanSettings)
        {
            sal_uInt16 nItemId = booleanSetting.nItemId;
            if ( rFeatures.has( nItemId ) )
            {
                // check whether this must be a tristate check box
                const SfxPoolItem& rItem = _rCoreAttrs.Get(nItemId);
                booleanSetting.bOptionalBool = dynamic_cast<const OptionalBoolItem*>(&rItem) != nullptr;
                booleanSetting.xControl = m_xBuilder->weld_check_button(booleanSetting.sControlId);
                if (booleanSetting.bOptionalBool)
                    booleanSetting.xControl->connect_toggled(LINK(this, SpecialSettingsPage, OnTriStateToggleHdl));
                else
                    booleanSetting.xControl->connect_toggled(LINK(this, SpecialSettingsPage, OnToggleHdl));
                booleanSetting.xControl->show();
            }
        }

        // create the controls for the boolean comparison mode
        if ( m_bHasBooleanComparisonMode )
        {
            m_xBooleanComparisonModeLabel = m_xBuilder->weld_label("comparisonft");
            m_xBooleanComparisonMode = m_xBuilder->weld_combo_box("comparison");
            m_xBooleanComparisonMode->connect_changed(LINK(this, SpecialSettingsPage, BooleanComparisonSelectHdl));
            m_xBooleanComparisonModeLabel->show();
            m_xBooleanComparisonMode->show();
        }
        // create the controls for the max row scan
        if ( m_bHasMaxRowScan )
        {
            m_xMaxRowScanLabel  = m_xBuilder->weld_label("rowsft");
            m_xMaxRowScan = m_xBuilder->weld_spin_button("rows");
            m_xMaxRowScan->connect_value_changed(LINK(this, OGenericAdministrationPage, OnControlSpinButtonModifyHdl));
            m_xMaxRowScanLabel->show();
            m_xMaxRowScan->show();
        }
    }

    IMPL_LINK(SpecialSettingsPage, OnTriStateToggleHdl, weld::ToggleButton&, rToggle, void)
    {
        auto eOldState = m_aTriStates[&rToggle];
        switch (eOldState)
        {
            case TRISTATE_INDET:
                rToggle.set_state(TRISTATE_FALSE);
                break;
            case TRISTATE_TRUE:
                rToggle.set_state(TRISTATE_INDET);
                break;
            case TRISTATE_FALSE:
                rToggle.set_state(TRISTATE_TRUE);
                break;
        }
        m_aTriStates[&rToggle] = rToggle.get_state();
        OnToggleHdl(rToggle);
    }

    IMPL_LINK(SpecialSettingsPage, OnToggleHdl, weld::ToggleButton&, rBtn, void)
    {
        if (&rBtn == m_xAppendTableAlias.get() && m_xAsBeforeCorrelationName)
        {
            // make m_xAsBeforeCorrelationName depend on m_xAppendTableAlias
            m_xAsBeforeCorrelationName->set_sensitive(m_xAppendTableAlias->get_active());
        }
        OnControlModifiedButtonClick(rBtn);
    }

    IMPL_LINK(SpecialSettingsPage, BooleanComparisonSelectHdl, weld::ComboBox&, rControl, void)
    {
        callModifiedHdl(&rControl);
    }

    SpecialSettingsPage::~SpecialSettingsPage()
    {
        disposeOnce();
    }

    void SpecialSettingsPage::fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        if ( m_bHasBooleanComparisonMode )
        {
            _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xBooleanComparisonModeLabel.get()));
        }
        if ( m_bHasMaxRowScan )
        {
            _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xMaxRowScanLabel.get()));
        }
    }

    void SpecialSettingsPage::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        for (auto const& booleanSetting : m_aBooleanSettings)
        {
            if (booleanSetting.xControl)
            {
                _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::CheckButton>(booleanSetting.xControl.get()));
            }
        }

        if ( m_bHasBooleanComparisonMode )
            _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ComboBox>(m_xBooleanComparisonMode.get()));
        if ( m_bHasMaxRowScan )
            _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::SpinButton>(m_xMaxRowScan.get()));
    }

    void SpecialSettingsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags( _rSet, bValid, bReadonly );

        if ( !bValid )
        {
            OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
            return;
        }

        m_aTriStates.clear();

        // the boolean items
        for (auto const& booleanSetting : m_aBooleanSettings)
        {
            if (!booleanSetting.xControl)
                continue;

            bool bTriState = false;

            boost::optional<bool> aValue;

            const SfxPoolItem* pItem = _rSet.GetItem<SfxPoolItem>(booleanSetting.nItemId);
            if (const SfxBoolItem *pBoolItem = dynamic_cast<const SfxBoolItem*>( pItem) )
            {
                aValue = pBoolItem->GetValue();
            }
            else if (const OptionalBoolItem *pOptionalItem = dynamic_cast<const OptionalBoolItem*>( pItem) )
            {
                aValue = pOptionalItem->GetFullValue();
                bTriState = true;
            }
            else
                OSL_FAIL( "SpecialSettingsPage::implInitControls: unknown boolean item type!" );

            if ( !aValue )
            {
                booleanSetting.xControl->set_state(TRISTATE_INDET);
            }
            else
            {
                bool bValue = *aValue;
                if ( booleanSetting.bInvertedDisplay )
                    bValue = !bValue;
                booleanSetting.xControl->set_active(bValue);
            }
            if (bTriState)
                m_aTriStates[booleanSetting.xControl.get()] = booleanSetting.xControl->get_state();
        }

        if (m_xAppendTableAlias && m_xAsBeforeCorrelationName)
        {
            // make m_xAsBeforeCorrelationName depend on m_xAppendTableAlias
            m_xAsBeforeCorrelationName->set_sensitive(m_xAppendTableAlias->get_active());
        }

        // the non-boolean items
        if ( m_bHasBooleanComparisonMode )
        {
            const SfxInt32Item* pBooleanComparison = _rSet.GetItem<SfxInt32Item>(DSID_BOOLEANCOMPARISON);
            m_xBooleanComparisonMode->set_active(static_cast<sal_uInt16>(pBooleanComparison->GetValue()));
        }

        if ( m_bHasMaxRowScan )
        {
            const SfxInt32Item* pMaxRowScan = _rSet.GetItem<SfxInt32Item>(DSID_MAX_ROW_SCAN);
            m_xMaxRowScan->set_value(pMaxRowScan->GetValue());
        }

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    bool SpecialSettingsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;

        // the boolean items
        for (auto const& booleanSetting : m_aBooleanSettings)
        {
            if (!booleanSetting.xControl)
                continue;
            fillBool(*_rSet, booleanSetting.xControl.get(), booleanSetting.nItemId, booleanSetting.bOptionalBool, bChangedSomething, booleanSetting.bInvertedDisplay);
        }

        // the non-boolean items
        if ( m_bHasBooleanComparisonMode )
        {
            if (m_xBooleanComparisonMode->get_value_changed_from_saved())
            {
                _rSet->Put(SfxInt32Item(DSID_BOOLEANCOMPARISON, m_xBooleanComparisonMode->get_active()));
                bChangedSomething = true;
            }
        }
        if ( m_bHasMaxRowScan )
        {
            fillInt32(*_rSet,m_xMaxRowScan.get(),DSID_MAX_ROW_SCAN,bChangedSomething);
        }
        return bChangedSomething;
    }

    // GeneratedValuesPage
    GeneratedValuesPage::GeneratedValuesPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pParent, "dbaccess/ui/generatedvaluespage.ui", "GeneratedValuesPage", _rCoreAttrs)
        , m_xAutoRetrievingEnabled(m_xBuilder->weld_check_button("autoretrieve"))
        , m_xGrid(m_xBuilder->weld_widget("grid"))
        , m_xAutoIncrementLabel(m_xBuilder->weld_label("statementft"))
        , m_xAutoIncrement(m_xBuilder->weld_entry("statement"))
        , m_xAutoRetrievingLabel(m_xBuilder->weld_label("queryft"))
        , m_xAutoRetrieving(m_xBuilder->weld_entry("query"))
    {
        m_xAutoRetrievingEnabled->connect_toggled(LINK(this, GeneratedValuesPage, OnAutoToggleHdl));
        m_xAutoIncrement->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xAutoRetrieving->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
    }

    IMPL_LINK(GeneratedValuesPage, OnAutoToggleHdl, weld::ToggleButton&, rBtn, void)
    {
        m_xGrid->set_sensitive(rBtn.get_active());
        OnControlModifiedButtonClick(rBtn);
    }

    GeneratedValuesPage::~GeneratedValuesPage()
    {
        disposeOnce();
    }

    void GeneratedValuesPage::fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Widget>(m_xContainer.get()));
    }

    void GeneratedValuesPage::fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        _rControlList.emplace_back( new OSaveValueWidgetWrapper<weld::CheckButton>( m_xAutoRetrievingEnabled.get() ) );
        _rControlList.emplace_back( new OSaveValueWidgetWrapper<weld::Entry>( m_xAutoIncrement.get() ) );
        _rControlList.emplace_back( new OSaveValueWidgetWrapper<weld::Entry>( m_xAutoRetrieving.get() ) );
    }

    void GeneratedValuesPage::implInitControls( const SfxItemSet& _rSet, bool _bSaveValue )
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // collect the items
        const SfxStringItem* pAutoIncrementItem = _rSet.GetItem<SfxStringItem>(DSID_AUTOINCREMENTVALUE);
        const SfxStringItem* pAutoRetrieveValueItem = _rSet.GetItem<SfxStringItem>(DSID_AUTORETRIEVEVALUE);
        const SfxBoolItem* pAutoRetrieveEnabledItem = _rSet.GetItem<SfxBoolItem>(DSID_AUTORETRIEVEENABLED);

        // forward the values to the controls
        if (bValid)
        {
            bool bEnabled = pAutoRetrieveEnabledItem->GetValue();
            m_xAutoRetrievingEnabled->set_active(bEnabled);

            m_xAutoIncrement->set_text(pAutoIncrementItem->GetValue());
            m_xAutoIncrement->save_value();
            m_xAutoRetrieving->set_text(pAutoRetrieveValueItem->GetValue());
            m_xAutoRetrieving->save_value();
        }
        OGenericAdministrationPage::implInitControls( _rSet, _bSaveValue );
    }

    bool GeneratedValuesPage::FillItemSet(SfxItemSet* _rSet)
    {
        bool bChangedSomething = false;

        fillString( *_rSet, m_xAutoIncrement.get(), DSID_AUTOINCREMENTVALUE, bChangedSomething );
        fillBool( *_rSet, m_xAutoRetrievingEnabled.get(), DSID_AUTORETRIEVEENABLED, false, bChangedSomething );
        fillString( *_rSet, m_xAutoRetrieving.get(), DSID_AUTORETRIEVEVALUE, bChangedSomething );

        return bChangedSomething;
    }

    // AdvancedSettingsDialog
    AdvancedSettingsDialog::AdvancedSettingsDialog(weld::Window* pParent, SfxItemSet* _pItems,
        const Reference< XComponentContext >& _rxContext, const Any& _aDataSourceName )
        : SfxTabDialogController(pParent, "dbaccess/ui/advancedsettingsdialog.ui", "AdvancedSettingsDialog", _pItems)
    {
        m_pImpl.reset(new ODbDataSourceAdministrationHelper(_rxContext, m_xDialog.get(), pParent, this));
        m_pImpl->setDataSourceOrName(_aDataSourceName);
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        m_pImpl->translateProperties(xDatasource, *_pItems);
        SetInputSet(_pItems);
        // propagate this set as our new input set and reset the example set
        m_xExampleSet.reset(new SfxItemSet(*GetInputSetImpl()));

        const OUString eType = dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(*_pItems);

        DataSourceMetaData aMeta( eType );
        const FeatureSet& rFeatures( aMeta.getFeatureSet() );

        // auto-generated values?
        if (rFeatures.supportsGeneratedValues())
            AddTabPage("generated", ODriversSettings::CreateGeneratedValuesPage, nullptr);
        else
            RemoveTabPage("generated");

        // any "special settings"?
        if (rFeatures.supportsAnySpecialSetting())
            AddTabPage("special", ODriversSettings::CreateSpecialSettingsPage, nullptr);
        else
            RemoveTabPage("special");

        // remove the reset button - it's meaning is much too ambiguous in this dialog
        RemoveResetButton();
    }

    AdvancedSettingsDialog::~AdvancedSettingsDialog()
    {
        SetInputSet(nullptr);
    }

    bool AdvancedSettingsDialog::doesHaveAnyAdvancedSettings( const OUString& _sURL )
    {
        DataSourceMetaData aMeta( _sURL );
        const FeatureSet& rFeatures( aMeta.getFeatureSet() );
        return rFeatures.supportsGeneratedValues() || rFeatures.supportsAnySpecialSetting();
    }

    short AdvancedSettingsDialog::Ok()
    {
        short nRet = SfxTabDialogController::Ok();
        if ( nRet == RET_OK )
        {
            m_xExampleSet->Put(*GetOutputItemSet());
            m_pImpl->saveChanges(*m_xExampleSet);
        }
        return nRet;
    }

    void AdvancedSettingsDialog::PageCreated(const OString& rId, SfxTabPage& _rPage)
    {
        // register ourself as modified listener
        static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory( getORB() );
        static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);
        SfxTabDialogController::PageCreated(rId, _rPage);
    }

    const SfxItemSet* AdvancedSettingsDialog::getOutputSet() const
    {
        return m_xExampleSet.get();
    }

    SfxItemSet* AdvancedSettingsDialog::getWriteOutputSet()
    {
        return m_xExampleSet.get();
    }

    std::pair< Reference< XConnection >, bool > AdvancedSettingsDialog::createConnection()
    {
        return m_pImpl->createConnection();
    }

    Reference< XComponentContext > AdvancedSettingsDialog::getORB() const
    {
        return m_pImpl->getORB();
    }

    Reference< XDriver > AdvancedSettingsDialog::getDriver()
    {
        return m_pImpl->getDriver();
    }

    OUString AdvancedSettingsDialog::getDatasourceType(const SfxItemSet& _rSet) const
    {
        return dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(_rSet);
    }

    void AdvancedSettingsDialog::clearPassword()
    {
        m_pImpl->clearPassword();
    }

    void AdvancedSettingsDialog::setTitle(const OUString& _sTitle)
    {
        m_xDialog->set_title(_sTitle);
    }

    void AdvancedSettingsDialog::enableConfirmSettings( bool ) {}

    void AdvancedSettingsDialog::saveDatasource()
    {
        PrepareLeaveCurrentPage();
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
