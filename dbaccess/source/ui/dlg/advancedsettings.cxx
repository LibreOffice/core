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
#include <dbu_pageids.hxx>
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
        VclPtr<CheckBox>* ppControl;    // the dialog's control which displays this setting
        OString     sControlId;         // the widget name of the control in the .ui
        sal_uInt16  nItemId;            // the ID of the item (in an SfxItemSet) which corresponds to this setting
        bool        bInvertedDisplay;   // true if and only if the checkbox is checked when the item is sal_False, and vice versa
    };

    // SpecialSettingsPage
    SpecialSettingsPage::SpecialSettingsPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs, const DataSourceMetaData& _rDSMeta)
        : OGenericAdministrationPage(pParent, "dbaccess/ui/specialsettingspage.ui", "SpecialSettingsPage", _rCoreAttrs)
        , m_aControlDependencies()
        , m_aBooleanSettings()
        , m_bHasBooleanComparisonMode( _rDSMeta.getFeatureSet().has( DSID_BOOLEANCOMPARISON ) )
        , m_bHasMaxRowScan( _rDSMeta.getFeatureSet().has( DSID_MAX_ROW_SCAN ) )
    {
        impl_initBooleanSettings();

        const FeatureSet& rFeatures( _rDSMeta.getFeatureSet() );
        // create all the check boxes for the boolean settings
        for (auto const& booleanSetting : m_aBooleanSettings)
        {
            sal_uInt16 nItemId = booleanSetting.nItemId;
            if ( rFeatures.has( nItemId ) )
            {
                *booleanSetting.ppControl = m_xBuilder->weld_check_button(booleanSetting.sControlId);
                (*booleanSetting.ppControl)->SetClickHdl( LINK(this, OGenericAdministrationPage, OnControlModifiedClick) );
                (*booleanSetting.ppControl)->Show();

                // check whether this must be a tristate check box
                const SfxPoolItem& rItem = _rCoreAttrs.Get( nItemId );
                if ( nullptr != dynamic_cast< const OptionalBoolItem* >(&rItem) )
                    (*booleanSetting.ppControl)->EnableTriState();
            }
        }

        if (m_xAsBeforeCorrelationName && m_xAppendTableAlias)
            // make m_xAsBeforeCorrelationName depend on m_xAppendTableAlias
            m_aControlDependencies.enableOnCheckMark( *m_xAppendTableAlias, *m_xAsBeforeCorrelationName );

        // create the controls for the boolean comparison mode
        if ( m_bHasBooleanComparisonMode )
        {
            m_xBooleanComparisonModeLabel = m_xBuilder->weld_label("comparisonft");
            m_xBooleanComparisonMode = m_xBuilder->weld_combo("comparison");
            m_xBooleanComparisonMode->SetDropDownLineCount( 4 );
            m_xBooleanComparisonMode->SetSelectHdl( LINK(this, SpecialSettingsPage, BooleanComparisonSelectHdl) );
            m_xBooleanComparisonModeLabel->show();
            m_xBooleanComparisonMode->show();
        }
        // create the controls for the max row scan
        if ( m_bHasMaxRowScan )
        {
            m_xMaxRowScanLabel  = m_xBuilder->weld_label("rowsft");
            m_xMaxRowScan = m_xBuilder->weld_spin_button("rows");
            m_xMaxRowScan->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
            m_xMaxRowScan->SetUseThousandSep(false);
            m_xMaxRowScanLabel->show();
            m_xMaxRowScan->show();
        }
    }

    IMPL_LINK(SpecialSettingsPage, BooleanComparisonSelectHdl, ListBox&, rControl, void)
    {
        callModifiedHdl(&rControl);
    }

    SpecialSettingsPage::~SpecialSettingsPage()
    {
        disposeOnce();
    }

    void SpecialSettingsPage::impl_initBooleanSettings()
    {
        OSL_PRECOND( m_aBooleanSettings.empty(), "SpecialSettingsPage::impl_initBooleanSettings: called twice!" );

        // for easier maintenance, write the table in this form, then copy it to m_aBooleanSettings
        BooleanSettingDesc aSettings[] = {
            { std::addressof(m_xIsSQL92Check),                 "usesql92",        DSID_SQL92CHECK,            false },
            { std::addressof(m_xAppendTableAlias),             "append",          DSID_APPEND_TABLE_ALIAS,    false },
            { std::addressof(m_xAsBeforeCorrelationName),      "useas",           DSID_AS_BEFORE_CORRNAME,    false },
            { std::addressof(m_xEnableOuterJoin),              "useoj",           DSID_ENABLEOUTERJOIN,       false },
            { std::addressof(m_xIgnoreDriverPrivileges),       "ignoreprivs",     DSID_IGNOREDRIVER_PRIV,     false },
            { std::addressof(m_xParameterSubstitution),        "replaceparams",   DSID_PARAMETERNAMESUBST,    false },
            { std::addressof(m_xSuppressVersionColumn),        "displayver",      DSID_SUPPRESSVERSIONCL,     true  },
            { std::addressof(m_xCatalog),                      "usecatalogname",  DSID_CATALOG,               false },
            { std::addressof(m_xSchema),                       "useschemaname",   DSID_SCHEMA,                false },
            { std::addressof(m_xIndexAppendix),                "createindex",     DSID_INDEXAPPENDIX,         false },
            { std::addressof(m_xDosLineEnds),                  "eol",             DSID_DOSLINEENDS,           false },
            { std::addressof(m_xCheckRequiredFields),          "ignorecurrency",  DSID_CHECK_REQUIRED_FIELDS, false },
            { std::addressof(m_xIgnoreCurrency),               "inputchecks",     DSID_IGNORECURRENCY,        false },
            { std::addressof(m_xEscapeDateTime),               "useodbcliterals", DSID_ESCAPE_DATETIME,       false },
            { std::addressof(m_xPrimaryKeySupport),            "primarykeys",     DSID_PRIMARY_KEY_SUPPORT,   false },
            { std::addressof(m_xRespectDriverResultSetType),   "resulttype",      DSID_RESPECTRESULTSETTYPE,  false }
        };

        for ( const BooleanSettingDesc& rDesc : aSettings )
        {
            m_aBooleanSettings.push_back( rDesc );
        }
    }

    void SpecialSettingsPage::fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        if ( m_bHasBooleanComparisonMode )
        {
            _rControlList.emplace_back(new ODisableWrapper<weld::Label>(m_xBooleanComparisonModeLabel));
        }
        if ( m_bHasMaxRowScan )
        {
            _rControlList.emplace_back(new ODisableWrapper<weld::Label>(m_xMaxRowScanLabel));
        }
    }

    void SpecialSettingsPage::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        for (auto const& booleanSetting : m_aBooleanSettings)
        {
            if ( *booleanSetting.ppControl )
            {
                _rControlList.emplace_back( new OSaveValueWrapper< CheckBox >( *booleanSetting.ppControl ) );
            }
        }

        if ( m_bHasBooleanComparisonMode )
            _rControlList.emplace_back(new OSaveValueWrapper<weld::ComboBox>(m_xBooleanComparisonMode));
        if ( m_bHasMaxRowScan )
            _rControlList.emplace_back(new OSaveValueWrapper<weld::SpinButton>(m_xMaxRowScan));
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

        // the boolean items
        for (auto const& booleanSetting : m_aBooleanSettings)
        {
            if ( !(*booleanSetting.ppControl) )
                continue;

            ::boost::optional< bool > aValue(false);
            aValue.reset();

            const SfxPoolItem* pItem = _rSet.GetItem<SfxPoolItem>(booleanSetting.nItemId);
            if (const SfxBoolItem *pBoolItem = dynamic_cast<const SfxBoolItem*>( pItem) )
            {
                aValue.reset( pBoolItem->GetValue() );
            }
            else if (const OptionalBoolItem *pOptionalItem = dynamic_cast<const OptionalBoolItem*>( pItem) )
            {
                aValue = pOptionalItem->GetFullValue();
            }
            else
                OSL_FAIL( "SpecialSettingsPage::implInitControls: unknown boolean item type!" );

            if ( !aValue )
            {
                (*booleanSetting.ppControl)->SetState( TRISTATE_INDET );
            }
            else
            {
                bool bValue = *aValue;
                if ( booleanSetting.bInvertedDisplay )
                    bValue = !bValue;
                (*booleanSetting.ppControl)->Check( bValue );
            }
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
            if ( !*booleanSetting.ppControl )
                continue;
            fillBool( *_rSet, *booleanSetting.ppControl, booleanSetting.nItemId, bChangedSomething, booleanSetting.bInvertedDisplay );
        }

        // the non-boolean items
        if ( m_bHasBooleanComparisonMode )
        {
            if (m_xBooleanComparisonMode->IsValueChangedFromSaved())
            {
                _rSet->Put(SfxInt32Item(DSID_BOOLEANCOMPARISON, m_xBooleanComparisonMode->get_active()));
                bChangedSomething = true;
            }
        }
        if ( m_bHasMaxRowScan )
        {
            fillInt32(*_rSet,m_xMaxRowScan,DSID_MAX_ROW_SCAN,bChangedSomething);
        }
        return bChangedSomething;
    }

    // GeneratedValuesPage
    GeneratedValuesPage::GeneratedValuesPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pParent, "dbaccess/ui/generatedvaluespage.ui", "GeneratedValuesPage", _rCoreAttrs)
        , m_xAutoRetrievingEnabled(m_xBuilder->weld_check_button("autoretrieve"))
        , m_xAutoIncrementLabel(m_xBuilder->weld_label("statementft"))
        , m_xAutoIncrement(m_xBuilder->weld_entry("statement"))
        , m_xAutoRetrievingLabel(m_xBuilder->weld_label("queryft"))
        , m_xAutoRetrieving(m_xBuilder->weld_entry("query"))
    {
        m_xAutoRetrievingEnabled->connect_toggled(LINK(this, OGenericAdministrationPage, OnControlModifiedClick));
        m_xAutoIncrement->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_xAutoRetrieving->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));

        m_aControlDependencies.enableOnCheckMark( *m_xAutoRetrievingEnabled,
            *m_xAutoIncrementLabel, *m_xAutoIncrement, *m_xAutoRetrievingLabel, *m_xAutoRetrieving );
    }

    GeneratedValuesPage::~GeneratedValuesPage()
    {
        disposeOnce();
    }

    void GeneratedValuesPage::fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        _rControlList.emplace_back(new ODisableWrapper<VclFrame>(m_xContainer));
    }

    void GeneratedValuesPage::fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        _rControlList.emplace_back( new OSaveValueWrapper<weld::CheckButton>( m_xAutoRetrievingEnabled ) );
        _rControlList.emplace_back( new OSaveValueWrapper<weld::Entry>( m_xAutoIncrement ) );
        _rControlList.emplace_back( new OSaveValueWrapper<weld::Entry>( m_xAutoRetrieving ) );
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

        fillString( *_rSet, m_xAutoIncrement, DSID_AUTOINCREMENTVALUE, bChangedSomething );
        fillBool( *_rSet, m_xAutoRetrievingEnabled, DSID_AUTORETRIEVEENABLED, bChangedSomething );
        fillString( *_rSet, m_xAutoRetrieving, DSID_AUTORETRIEVEVALUE, bChangedSomething );

        return bChangedSomething;
    }

    // AdvancedSettingsDialog
    AdvancedSettingsDialog::AdvancedSettingsDialog( vcl::Window* _pParent, SfxItemSet* _pItems,
        const Reference< XComponentContext >& _rxContext, const Any& _aDataSourceName )
        : SfxTabDialog(_pParent, "AdvancedSettingsDialog",
            "dbaccess/ui/advancedsettingsdialog.ui", _pItems)
    {
        m_pImpl.reset(new ODbDataSourceAdministrationHelper(_rxContext,_pParent,this));
        m_pImpl->setDataSourceOrName(_aDataSourceName);
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        m_pImpl->translateProperties(xDatasource, *_pItems);
        SetInputSet(_pItems);
        // propagate this set as our new input set and reset the example set
        delete m_pExampleSet;
        m_pExampleSet = new SfxItemSet(*GetInputSetImpl());

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
        disposeOnce();
    }

    void AdvancedSettingsDialog::dispose()
    {
        SetInputSet(nullptr);
        DELETEZ(m_pExampleSet);
        SfxTabDialog::dispose();
    }

    bool AdvancedSettingsDialog::doesHaveAnyAdvancedSettings( const OUString& _sURL )
    {
        DataSourceMetaData aMeta( _sURL );
        const FeatureSet& rFeatures( aMeta.getFeatureSet() );
        return rFeatures.supportsGeneratedValues() || rFeatures.supportsAnySpecialSetting();
    }

    short AdvancedSettingsDialog::Execute()
    {
        short nRet = SfxTabDialog::Execute();
        if ( nRet == RET_OK )
        {
            m_pExampleSet->Put(*GetOutputItemSet());
            m_pImpl->saveChanges(*m_pExampleSet);
        }
        return nRet;
    }

    void AdvancedSettingsDialog::PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage)
    {
        // register ourself as modified listener
        static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory( getORB() );
        static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);

        vcl::Window *pWin = GetViewWindow();
        if(pWin)
            pWin->Invalidate();

        SfxTabDialog::PageCreated(_nId, _rPage);
    }

    const SfxItemSet* AdvancedSettingsDialog::getOutputSet() const
    {
        return m_pExampleSet;
    }

    SfxItemSet* AdvancedSettingsDialog::getWriteOutputSet()
    {
        return m_pExampleSet;
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
        SetText(_sTitle);
    }

    void AdvancedSettingsDialog::enableConfirmSettings( bool ) {}

    void AdvancedSettingsDialog::saveDatasource()
    {
        PrepareLeaveCurrentPage();
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
