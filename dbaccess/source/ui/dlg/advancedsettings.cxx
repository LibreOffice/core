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
#include "advancedsettingsdlg.hxx"
#include "moduledbu.hxx"
#include "dsitems.hxx"
#include "DbAdminImpl.hxx"
#include "DriverSettings.hxx"
#include "optionalboolitem.hxx"
#include "dbu_resource.hrc"
#include "dbu_dlg.hrc"

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

#include <vcl/msgbox.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
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
    SpecialSettingsPage::SpecialSettingsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs, const DataSourceMetaData& _rDSMeta )
        : OGenericAdministrationPage(pParent, "SpecialSettingsPage",
            "dbaccess/ui/specialsettingspage.ui", _rCoreAttrs)
        , m_pIsSQL92Check( NULL )
        , m_pAppendTableAlias( NULL )
        , m_pAsBeforeCorrelationName( NULL )
        , m_pEnableOuterJoin( NULL )
        , m_pIgnoreDriverPrivileges( NULL )
        , m_pParameterSubstitution( NULL )
        , m_pSuppressVersionColumn( NULL )
        , m_pCatalog( NULL )
        , m_pSchema( NULL )
        , m_pIndexAppendix( NULL )
        , m_pDosLineEnds( NULL )
        , m_pCheckRequiredFields( NULL )
        , m_pIgnoreCurrency(NULL)
        , m_pEscapeDateTime(NULL)
        , m_pPrimaryKeySupport(NULL)
        , m_pRespectDriverResultSetType(NULL)
        , m_pBooleanComparisonModeLabel( NULL )
        , m_pBooleanComparisonMode( NULL )
        , m_pMaxRowScanLabel( NULL )
        , m_pMaxRowScan( NULL )
        , m_aControlDependencies()
        , m_aBooleanSettings()
        , m_bHasBooleanComparisonMode( _rDSMeta.getFeatureSet().has( DSID_BOOLEANCOMPARISON ) )
        , m_bHasMaxRowScan( _rDSMeta.getFeatureSet().has( DSID_MAX_ROW_SCAN ) )
    {
        impl_initBooleanSettings();

        const FeatureSet& rFeatures( _rDSMeta.getFeatureSet() );
        // create all the check boxes for the boolean settings
        for (   BooleanSettingDescs::const_iterator setting = m_aBooleanSettings.begin();
                setting != m_aBooleanSettings.end();
                ++setting
             )
        {
            sal_uInt16 nItemId = setting->nItemId;
            if ( rFeatures.has( nItemId ) )
            {
                get(*setting->ppControl, setting->sControlId);
                (*setting->ppControl)->SetClickHdl( getControlModifiedClickLink() );
                (*setting->ppControl)->Show();

                // check whether this must be a tristate check box
                const SfxPoolItem& rItem = _rCoreAttrs.Get( nItemId );
                if ( 0 != dynamic_cast< const OptionalBoolItem* >(&rItem) )
                    (*setting->ppControl)->EnableTriState();
            }
        }

        if ( m_pAsBeforeCorrelationName && m_pAppendTableAlias )
            // make m_pAsBeforeCorrelationName depend on m_pAppendTableAlias
            m_aControlDependencies.enableOnCheckMark( *m_pAppendTableAlias, *m_pAsBeforeCorrelationName );

        // create the controls for the boolean comparison mode
        if ( m_bHasBooleanComparisonMode )
        {
            get(m_pBooleanComparisonModeLabel, "comparisonft");
            get(m_pBooleanComparisonMode, "comparison");
            m_pBooleanComparisonMode->SetDropDownLineCount( 4 );
            m_pBooleanComparisonMode->SetSelectHdl( getControlModifiedLink() );
            m_pBooleanComparisonModeLabel->Show();
            m_pBooleanComparisonMode->Show();
        }
        // create the controls for the max row scan
        if ( m_bHasMaxRowScan )
        {
            get(m_pMaxRowScanLabel, "rowsft");
            get(m_pMaxRowScan, "rows");
            m_pMaxRowScan->SetModifyHdl(getControlModifiedLink());
            m_pMaxRowScan->SetUseThousandSep(false);
            m_pMaxRowScanLabel->Show();
            m_pMaxRowScan->Show();
        }
    }

    SpecialSettingsPage::~SpecialSettingsPage()
    {
        disposeOnce();
    }

    void SpecialSettingsPage::dispose()
    {
        m_aControlDependencies.clear();
        m_pIsSQL92Check.clear();
        m_pAppendTableAlias.clear();
        m_pAsBeforeCorrelationName.clear();
        m_pEnableOuterJoin.clear();
        m_pIgnoreDriverPrivileges.clear();
        m_pParameterSubstitution.clear();
        m_pSuppressVersionColumn.clear();
        m_pCatalog.clear();
        m_pSchema.clear();
        m_pIndexAppendix.clear();
        m_pDosLineEnds.clear();
        m_pCheckRequiredFields.clear();
        m_pIgnoreCurrency.clear();
        m_pEscapeDateTime.clear();
        m_pPrimaryKeySupport.clear();
        m_pRespectDriverResultSetType.clear();
        m_pBooleanComparisonModeLabel.clear();
        m_pBooleanComparisonMode.clear();
        m_pMaxRowScanLabel.clear();
        m_pMaxRowScan.clear();
        OGenericAdministrationPage::dispose();
    }

    void SpecialSettingsPage::impl_initBooleanSettings()
    {
        OSL_PRECOND( m_aBooleanSettings.empty(), "SpecialSettingsPage::impl_initBooleanSettings: called twice!" );

        // for easier maintenance, write the table in this form, then copy it to m_aBooleanSettings
        BooleanSettingDesc aSettings[] = {
            { std::addressof(m_pIsSQL92Check),                 "usesql92",        DSID_SQL92CHECK,            false },
            { std::addressof(m_pAppendTableAlias),             "append",          DSID_APPEND_TABLE_ALIAS,    false },
            { std::addressof(m_pAsBeforeCorrelationName),      "useas",           DSID_AS_BEFORE_CORRNAME,    false },
            { std::addressof(m_pEnableOuterJoin),              "useoj",           DSID_ENABLEOUTERJOIN,       false },
            { std::addressof(m_pIgnoreDriverPrivileges),       "ignoreprivs",     DSID_IGNOREDRIVER_PRIV,     false },
            { std::addressof(m_pParameterSubstitution),        "replaceparams",   DSID_PARAMETERNAMESUBST,    false },
            { std::addressof(m_pSuppressVersionColumn),        "displayver",      DSID_SUPPRESSVERSIONCL,     true  },
            { std::addressof(m_pCatalog),                      "usecatalogname",  DSID_CATALOG,               false },
            { std::addressof(m_pSchema),                       "useschemaname",   DSID_SCHEMA,                false },
            { std::addressof(m_pIndexAppendix),                "createindex",     DSID_INDEXAPPENDIX,         false },
            { std::addressof(m_pDosLineEnds),                  "eol",             DSID_DOSLINEENDS,           false },
            { std::addressof(m_pCheckRequiredFields),          "ignorecurrency",  DSID_CHECK_REQUIRED_FIELDS, false },
            { std::addressof(m_pIgnoreCurrency),               "inputchecks",     DSID_IGNORECURRENCY,        false },
            { std::addressof(m_pEscapeDateTime),               "useodbcliterals", DSID_ESCAPE_DATETIME,       false },
            { std::addressof(m_pPrimaryKeySupport),            "primarykeys",     DSID_PRIMARY_KEY_SUPPORT,   false },
            { std::addressof(m_pRespectDriverResultSetType),   "resulttype",      DSID_RESPECTRESULTSETTYPE,  false }
        };

        for ( const BooleanSettingDesc& pCopy : aSettings )
        {
            m_aBooleanSettings.push_back( pCopy );
        }
    }

    void SpecialSettingsPage::fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        if ( m_bHasBooleanComparisonMode )
        {
            _rControlList.push_back( new ODisableWrapper< FixedText >( m_pBooleanComparisonModeLabel ) );
        }
        if ( m_bHasMaxRowScan )
        {
            _rControlList.push_back( new ODisableWrapper< FixedText >( m_pMaxRowScanLabel ) );
        }
    }

    void SpecialSettingsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        for (   BooleanSettingDescs::const_iterator setting = m_aBooleanSettings.begin();
                setting != m_aBooleanSettings.end();
                ++setting
             )
        {
            if ( (*setting->ppControl) )
            {
                _rControlList.push_back( new OSaveValueWrapper< CheckBox >( *setting->ppControl ) );
            }
        }

        if ( m_bHasBooleanComparisonMode )
            _rControlList.push_back( new OSaveValueWrapper< ListBox >( m_pBooleanComparisonMode ) );
        if ( m_bHasMaxRowScan )
            _rControlList.push_back(new OSaveValueWrapper<NumericField>(m_pMaxRowScan));
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
        for (   BooleanSettingDescs::const_iterator setting = m_aBooleanSettings.begin();
                setting != m_aBooleanSettings.end();
                ++setting
             )
        {
            if ( !(*setting->ppControl) )
                continue;

            ::boost::optional< bool > aValue(false);
            aValue.reset();

            SFX_ITEMSET_GET( _rSet, pItem, SfxPoolItem, setting->nItemId, true );
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
                (*setting->ppControl)->SetState( TRISTATE_INDET );
            }
            else
            {
                bool bValue = *aValue;
                if ( setting->bInvertedDisplay )
                    bValue = !bValue;
                (*setting->ppControl)->Check( bValue );
            }
        }

        // the non-boolean items
        if ( m_bHasBooleanComparisonMode )
        {
            SFX_ITEMSET_GET( _rSet, pBooleanComparison, SfxInt32Item, DSID_BOOLEANCOMPARISON, true );
            m_pBooleanComparisonMode->SelectEntryPos( static_cast< sal_uInt16 >( pBooleanComparison->GetValue() ) );
        }

        if ( m_bHasMaxRowScan )
        {
            SFX_ITEMSET_GET(_rSet, pMaxRowScan, SfxInt32Item, DSID_MAX_ROW_SCAN, true);
            m_pMaxRowScan->SetValue(pMaxRowScan->GetValue());
        }

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    bool SpecialSettingsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;

        // the boolean items
        for (   BooleanSettingDescs::const_iterator setting = m_aBooleanSettings.begin();
                setting != m_aBooleanSettings.end();
                ++setting
             )
        {
            if ( !*setting->ppControl )
                continue;
            fillBool( *_rSet, *setting->ppControl, setting->nItemId, bChangedSomething, setting->bInvertedDisplay );
        }

        // the non-boolean items
        if ( m_bHasBooleanComparisonMode )
        {
            if ( m_pBooleanComparisonMode->IsValueChangedFromSaved() )
            {
                _rSet->Put( SfxInt32Item( DSID_BOOLEANCOMPARISON, m_pBooleanComparisonMode->GetSelectEntryPos() ) );
                bChangedSomething = true;
            }
        }
        if ( m_bHasMaxRowScan )
        {
            fillInt32(*_rSet,m_pMaxRowScan,DSID_MAX_ROW_SCAN,bChangedSomething);
        }
        return bChangedSomething;
    }

    // GeneratedValuesPage
    GeneratedValuesPage::GeneratedValuesPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        : OGenericAdministrationPage(pParent, "GeneratedValuesPage",
            "dbaccess/ui/generatedvaluespage.ui", _rCoreAttrs)
    {
        get(m_pAutoFrame, "GeneratedValuesPage");
        get(m_pAutoRetrievingEnabled, "autoretrieve");
        get(m_pAutoIncrementLabel, "statementft");
        get(m_pAutoIncrement, "statement");
        get(m_pAutoRetrievingLabel, "queryft");
        get(m_pAutoRetrieving, "query");

        m_pAutoRetrievingEnabled->SetClickHdl( getControlModifiedClickLink() );
        m_pAutoIncrement->SetModifyHdl( getControlModifiedLink() );
        m_pAutoRetrieving->SetModifyHdl( getControlModifiedLink() );

        m_aControlDependencies.enableOnCheckMark( *m_pAutoRetrievingEnabled,
            *m_pAutoIncrementLabel, *m_pAutoIncrement, *m_pAutoRetrievingLabel, *m_pAutoRetrieving );
    }

    GeneratedValuesPage::~GeneratedValuesPage()
    {
        disposeOnce();
    }

    void GeneratedValuesPage::dispose()
    {
        m_aControlDependencies.clear();
        m_pAutoFrame.clear();
        m_pAutoRetrievingEnabled.clear();
        m_pAutoIncrementLabel.clear();
        m_pAutoIncrement.clear();
        m_pAutoRetrievingLabel.clear();
        m_pAutoRetrieving.clear();
        OGenericAdministrationPage::dispose();
    }

    void GeneratedValuesPage::fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new ODisableWrapper< VclFrame >( m_pAutoFrame ) );
    }

    void GeneratedValuesPage::fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new OSaveValueWrapper< CheckBox >( m_pAutoRetrievingEnabled ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( m_pAutoIncrement ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( m_pAutoRetrieving ) );
    }

    void GeneratedValuesPage::implInitControls( const SfxItemSet& _rSet, bool _bSaveValue )
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // collect the items
        SFX_ITEMSET_GET(_rSet, pAutoIncrementItem, SfxStringItem, DSID_AUTOINCREMENTVALUE, true);
        SFX_ITEMSET_GET(_rSet, pAutoRetrieveValueItem, SfxStringItem, DSID_AUTORETRIEVEVALUE, true);
        SFX_ITEMSET_GET(_rSet, pAutoRetrieveEnabledItem, SfxBoolItem, DSID_AUTORETRIEVEENABLED, true);

        // forward the values to the controls
        if (bValid)
        {
            bool bEnabled = pAutoRetrieveEnabledItem->GetValue();
            m_pAutoRetrievingEnabled->Check( bEnabled );

            m_pAutoIncrement->SetText( pAutoIncrementItem->GetValue() );
            m_pAutoIncrement->ClearModifyFlag();
            m_pAutoRetrieving->SetText( pAutoRetrieveValueItem->GetValue() );
            m_pAutoRetrieving->ClearModifyFlag();
        }
        OGenericAdministrationPage::implInitControls( _rSet, _bSaveValue );
    }

    bool GeneratedValuesPage::FillItemSet(SfxItemSet* _rSet)
    {
        bool bChangedSomething = false;

        fillString( *_rSet, m_pAutoIncrement, DSID_AUTOINCREMENTVALUE, bChangedSomething );
        fillBool( *_rSet, m_pAutoRetrievingEnabled, DSID_AUTORETRIEVEENABLED, bChangedSomething );
        fillString( *_rSet, m_pAutoRetrieving, DSID_AUTORETRIEVEVALUE, bChangedSomething );

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
        delete pExampleSet;
        pExampleSet = new SfxItemSet(*GetInputSetImpl());

        const OUString eType = dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(*_pItems);

        DataSourceMetaData aMeta( eType );
        const FeatureSet& rFeatures( aMeta.getFeatureSet() );

        // auto-generated values?
        if (rFeatures.supportsGeneratedValues())
            AddTabPage("generated", ODriversSettings::CreateGeneratedValuesPage, NULL);
        else
            RemoveTabPage("generated");

        // any "special settings"?
        if (rFeatures.supportsAnySpecialSetting())
            AddTabPage("special", ODriversSettings::CreateSpecialSettingsPage, NULL);
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
        SetInputSet(NULL);
        DELETEZ(pExampleSet);
        SfxTabDialog::dispose();
    }

    bool AdvancedSettingsDialog::doesHaveAnyAdvancedSettings( const OUString& _sURL )
    {
        DataSourceMetaData aMeta( _sURL );
        const FeatureSet& rFeatures( aMeta.getFeatureSet() );
        if ( rFeatures.supportsGeneratedValues() || rFeatures.supportsAnySpecialSetting() )
            return true;
        return false;
    }

    short AdvancedSettingsDialog::Execute()
    {
        short nRet = SfxTabDialog::Execute();
        if ( nRet == RET_OK )
        {
            pExampleSet->Put(*GetOutputItemSet());
            m_pImpl->saveChanges(*pExampleSet);
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
        return pExampleSet;
    }

    SfxItemSet* AdvancedSettingsDialog::getWriteOutputSet()
    {
        return pExampleSet;
    }

    ::std::pair< Reference< XConnection >, sal_Bool > AdvancedSettingsDialog::createConnection()
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

    void AdvancedSettingsDialog::enableConfirmSettings( bool _bEnable )
    {
        (void)_bEnable;
    }

    bool AdvancedSettingsDialog::saveDatasource()
    {
        return PrepareLeaveCurrentPage();
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
