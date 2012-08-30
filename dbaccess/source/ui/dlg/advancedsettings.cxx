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

#include "advancedsettings.hxx"
#include "advancedsettingsdlg.hxx"
#include "moduledbu.hxx"
#include "dsitems.hxx"
#include "DbAdminImpl.hxx"
#include "DriverSettings.hxx"
#include "optionalboolitem.hxx"
#include "dbu_resource.hrc"
#include "dbu_dlg.hrc"
#include "dbadmin.hrc"
#include "advancedsettings.hrc"

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

#include <vcl/msgbox.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XDriver;
    /** === end UNO using === **/

    //========================================================================
    //= SpecialSettingsPage
    //========================================================================
    struct BooleanSettingDesc
    {
        CheckBox**  ppControl;          // the dialog's control which displays this setting
        sal_uInt16      nControlResId;      // the resource ID to load the control from
        sal_uInt16      nItemId;            // the ID of the item (in an SfxItemSet) which corresponds to this setting
        bool        bInvertedDisplay;   // true if and only if the checkbox is checked when the item is sal_False, and vice versa
    };

    //========================================================================
    //= SpecialSettingsPage
    //========================================================================
    SpecialSettingsPage::SpecialSettingsPage( Window* pParent, const SfxItemSet& _rCoreAttrs, const DataSourceMetaData& _rDSMeta )
        :OGenericAdministrationPage( pParent, ModuleRes( PAGE_ADVANCED_SETTINGS_SPECIAL ), _rCoreAttrs )
        ,m_aTopLine( this, ModuleRes( FL_DATAHANDLING ) )
        ,m_pIsSQL92Check( NULL )
        ,m_pAppendTableAlias( NULL )
        ,m_pAsBeforeCorrelationName( NULL )
        ,m_pEnableOuterJoin( NULL )
        ,m_pIgnoreDriverPrivileges( NULL )
        ,m_pParameterSubstitution( NULL )
        ,m_pSuppressVersionColumn( NULL )
        ,m_pCatalog( NULL )
        ,m_pSchema( NULL )
        ,m_pIndexAppendix( NULL )
        ,m_pDosLineEnds( NULL )
        ,m_pCheckRequiredFields( NULL )
        ,m_pIgnoreCurrency(NULL)
        ,m_pEscapeDateTime(NULL)
        ,m_pPrimaryKeySupport(NULL)
        ,m_pRespectDriverResultSetType(NULL)
        ,m_pBooleanComparisonModeLabel( NULL )
        ,m_pBooleanComparisonMode( NULL )
        ,m_pMaxRowScanLabel( NULL )
        ,m_pMaxRowScan( NULL )
        ,m_aControlDependencies()
        ,m_aBooleanSettings()
        ,m_bHasBooleanComparisonMode( _rDSMeta.getFeatureSet().has( DSID_BOOLEANCOMPARISON ) )
        ,m_bHasMaxRowScan( _rDSMeta.getFeatureSet().has( DSID_MAX_ROW_SCAN ) )
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
                sal_uInt16 nResourceId = setting->nControlResId;
                (*setting->ppControl) = new CheckBox( this, ModuleRes( nResourceId ) );
                (*setting->ppControl)->SetClickHdl( getControlModifiedLink() );

                // check whether this must be a tristate check box
                const SfxPoolItem& rItem = _rCoreAttrs.Get( nItemId );
                if ( rItem.ISA( OptionalBoolItem ) )
                    (*setting->ppControl)->EnableTriState( sal_True );
            }
        }

        if ( m_pAsBeforeCorrelationName && m_pAppendTableAlias )
            // make m_pAsBeforeCorrelationName depend on m_pAppendTableAlias
            m_aControlDependencies.enableOnCheckMark( *m_pAppendTableAlias, *m_pAsBeforeCorrelationName );

        // move the controls to the appropriate positions
        Point aPos( m_aTopLine.GetPosPixel() );
        aPos.Move( 0, m_aTopLine.GetSizePixel().Height() );
        Size aFirstDistance( LogicToPixel( Size( INDENTED_X, RELATED_CONTROLS ), MAP_APPFONT ) );
        aPos.Move( aFirstDistance.Width(), aFirstDistance.Height() );

        Size aUnrelatedControls( LogicToPixel( Size( RELATED_CONTROLS, RELATED_CONTROLS ), MAP_APPFONT ) );

        for (   BooleanSettingDescs::const_iterator setting = m_aBooleanSettings.begin();
                setting != m_aBooleanSettings.end();
                ++setting
             )
        {
            if ( !*setting->ppControl )
                continue;

            (*setting->ppControl)->SetPosPixel( aPos );
            aPos.Move( 0, (*setting->ppControl)->GetSizePixel().Height() );
            aPos.Move( 0, aUnrelatedControls.Height() );
        }

        // create the controls for the boolean comparison mode
        if ( m_bHasBooleanComparisonMode )
        {
            m_pBooleanComparisonModeLabel = new FixedText( this, ModuleRes( FT_BOOLEANCOMPARISON ) );
            m_pBooleanComparisonMode = new ListBox( this, ModuleRes( LB_BOOLEANCOMPARISON ) );
            m_pBooleanComparisonMode->SetDropDownLineCount( 4 );
            m_pBooleanComparisonMode->SetSelectHdl( getControlModifiedLink() );

            Point aLabelPos( m_pBooleanComparisonModeLabel->GetPosPixel() );
            Point aControlPos( m_pBooleanComparisonMode->GetPosPixel() );
            long nMoveUp = aControlPos.Y() - aPos.Y();

            m_pBooleanComparisonModeLabel->SetPosPixel( Point( aLabelPos.X(), aLabelPos.Y() - nMoveUp ) );
            m_pBooleanComparisonMode->SetPosPixel( Point( aControlPos.X(), aControlPos.Y() - nMoveUp ) );
        }
        // create the controls for the max row scan
        if ( m_bHasMaxRowScan )
        {
            m_pMaxRowScanLabel = new FixedText( this, ModuleRes( FT_MAXROWSCAN ) );
            m_pMaxRowScan = new NumericField( this, ModuleRes( NF_MAXROWSCAN ) );
            m_pMaxRowScan->SetModifyHdl(getControlModifiedLink());
            m_pMaxRowScan->SetUseThousandSep(sal_False);

            Point aLabelPos( m_pMaxRowScanLabel->GetPosPixel() );
            Point aControlPos( m_pMaxRowScan->GetPosPixel() );
            long nMoveUp = aControlPos.Y() - aPos.Y();

            m_pMaxRowScanLabel->SetPosPixel( Point( aLabelPos.X(), aLabelPos.Y() - nMoveUp ) );
            m_pMaxRowScan->SetPosPixel( Point( aControlPos.X(), aControlPos.Y() - nMoveUp ) );
        }

        FreeResource();
    }

    // -----------------------------------------------------------------------
    SpecialSettingsPage::~SpecialSettingsPage()
    {
        m_aControlDependencies.clear();

        DELETEZ( m_pIsSQL92Check );
        DELETEZ( m_pAppendTableAlias );
        DELETEZ( m_pAsBeforeCorrelationName );
        DELETEZ( m_pParameterSubstitution );
        DELETEZ( m_pIgnoreDriverPrivileges );
        DELETEZ( m_pSuppressVersionColumn );
        DELETEZ( m_pEnableOuterJoin );
        DELETEZ( m_pCatalog );
        DELETEZ( m_pSchema );
        DELETEZ( m_pIndexAppendix );
        DELETEZ( m_pDosLineEnds );
        DELETEZ( m_pCheckRequiredFields );
        DELETEZ( m_pIgnoreCurrency );
        DELETEZ( m_pEscapeDateTime );
        DELETEZ( m_pPrimaryKeySupport );
        DELETEZ( m_pRespectDriverResultSetType );
        DELETEZ( m_pBooleanComparisonModeLabel );
        DELETEZ( m_pBooleanComparisonMode );
        DELETEZ( m_pMaxRowScanLabel );
        DELETEZ( m_pMaxRowScan );
    }

    // -----------------------------------------------------------------------
    void SpecialSettingsPage::impl_initBooleanSettings()
    {
        OSL_PRECOND( m_aBooleanSettings.empty(), "SpecialSettingsPage::impl_initBooleanSettings: called twice!" );

        // for easier maintainance, write the table in this form, then copy it to m_aBooleanSettings
        BooleanSettingDesc aSettings[] = {
            { &m_pIsSQL92Check,                 CB_SQL92CHECK,          DSID_SQL92CHECK,            false },
            { &m_pAppendTableAlias,             CB_APPENDTABLEALIAS,    DSID_APPEND_TABLE_ALIAS,    false },
            { &m_pAsBeforeCorrelationName,      CB_AS_BEFORE_CORR_NAME, DSID_AS_BEFORE_CORRNAME,    false },
            { &m_pEnableOuterJoin,              CB_ENABLEOUTERJOIN,     DSID_ENABLEOUTERJOIN,       false },
            { &m_pIgnoreDriverPrivileges,       CB_IGNOREDRIVER_PRIV,   DSID_IGNOREDRIVER_PRIV,     false },
            { &m_pParameterSubstitution,        CB_PARAMETERNAMESUBST,  DSID_PARAMETERNAMESUBST,    false },
            { &m_pSuppressVersionColumn,        CB_SUPPRESVERSIONCL,    DSID_SUPPRESSVERSIONCL,     true },
            { &m_pCatalog,                      CB_CATALOG,             DSID_CATALOG,               false },
            { &m_pSchema,                       CB_SCHEMA,              DSID_SCHEMA,                false },
            { &m_pIndexAppendix,                CB_IGNOREINDEXAPPENDIX, DSID_INDEXAPPENDIX,         false },
            { &m_pDosLineEnds,                  CB_DOSLINEENDS,         DSID_DOSLINEENDS,           false },
            { &m_pCheckRequiredFields,          CB_CHECK_REQUIRED,      DSID_CHECK_REQUIRED_FIELDS, false },
            { &m_pIgnoreCurrency,               CB_IGNORECURRENCY,      DSID_IGNORECURRENCY,        false },
            { &m_pEscapeDateTime,               CB_ESCAPE_DATETIME,     DSID_ESCAPE_DATETIME,       false },
            { &m_pPrimaryKeySupport,            CB_PRIMARY_KEY_SUPPORT, DSID_PRIMARY_KEY_SUPPORT,   false },
            { &m_pRespectDriverResultSetType,   CB_RESPECTRESULTSETTYPE,DSID_RESPECTRESULTSETTYPE,  false },
            { NULL,                         0,                      0,                          false }
        };

        for ( const BooleanSettingDesc* pCopy = aSettings; pCopy->nItemId != 0; ++pCopy )
        {
            m_aBooleanSettings.push_back( *pCopy );
        }
    }

    // -----------------------------------------------------------------------
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

    // -----------------------------------------------------------------------
    void SpecialSettingsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        for (   BooleanSettingDescs::const_iterator setting = m_aBooleanSettings.begin();
                setting != m_aBooleanSettings.end();
                ++setting
             )
        {
            if ( *setting->ppControl )
            {
                _rControlList.push_back( new OSaveValueWrapper< CheckBox >( *setting->ppControl ) );
            }
        }

        if ( m_bHasBooleanComparisonMode )
            _rControlList.push_back( new OSaveValueWrapper< ListBox >( m_pBooleanComparisonMode ) );
        if ( m_bHasMaxRowScan )
            _rControlList.push_back(new OSaveValueWrapper<NumericField>(m_pMaxRowScan));
    }

    // -----------------------------------------------------------------------
    void SpecialSettingsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
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
            if ( !*setting->ppControl )
                continue;

            ::boost::optional< bool > aValue(false);
            aValue.reset();

            SFX_ITEMSET_GET( _rSet, pItem, SfxPoolItem, setting->nItemId, sal_True );
            if ( pItem->ISA( SfxBoolItem ) )
            {
                aValue.reset( PTR_CAST( SfxBoolItem, pItem )->GetValue() );
            }
            else if ( pItem->ISA( OptionalBoolItem ) )
            {
                aValue = PTR_CAST( OptionalBoolItem, pItem )->GetFullValue();
            }
            else
                OSL_FAIL( "SpecialSettingsPage::implInitControls: unknown boolean item type!" );

            if ( !aValue )
            {
                (*setting->ppControl)->SetState( STATE_DONTKNOW );
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
            SFX_ITEMSET_GET( _rSet, pBooleanComparison, SfxInt32Item, DSID_BOOLEANCOMPARISON, sal_True );
            m_pBooleanComparisonMode->SelectEntryPos( static_cast< sal_uInt16 >( pBooleanComparison->GetValue() ) );
        }

        if ( m_bHasMaxRowScan )
        {
            SFX_ITEMSET_GET(_rSet, pMaxRowScan, SfxInt32Item, DSID_MAX_ROW_SCAN, sal_True);
            m_pMaxRowScan->SetValue(pMaxRowScan->GetValue());
        }

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    // -----------------------------------------------------------------------
    sal_Bool SpecialSettingsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = sal_False;

        // the boolean items
        for (   BooleanSettingDescs::const_iterator setting = m_aBooleanSettings.begin();
                setting != m_aBooleanSettings.end();
                ++setting
             )
        {
            if ( !*setting->ppControl )
                continue;
            fillBool( _rSet, *setting->ppControl, setting->nItemId, bChangedSomething, setting->bInvertedDisplay );
        }

        // the non-boolean items
        if ( m_bHasBooleanComparisonMode )
        {
            if ( m_pBooleanComparisonMode->GetSelectEntryPos() != m_pBooleanComparisonMode->GetSavedValue() )
            {
                _rSet.Put( SfxInt32Item( DSID_BOOLEANCOMPARISON, m_pBooleanComparisonMode->GetSelectEntryPos() ) );
                bChangedSomething = sal_True;
            }
        }
        if ( m_bHasMaxRowScan )
        {
            fillInt32(_rSet,m_pMaxRowScan,DSID_MAX_ROW_SCAN,bChangedSomething);
        }
        return bChangedSomething;
    }

    //========================================================================
    //= GeneratedValuesPage
    //========================================================================
    //------------------------------------------------------------------------
    GeneratedValuesPage::GeneratedValuesPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OGenericAdministrationPage(pParent, ModuleRes( PAGE_GENERATED_VALUES ), _rCoreAttrs)
        ,m_aAutoFixedLine        ( this, ModuleRes( FL_SEPARATORAUTO ) )
        ,m_aAutoRetrievingEnabled( this, ModuleRes( CB_RETRIEVE_AUTO ) )
        ,m_aAutoIncrementLabel   ( this, ModuleRes( FT_AUTOINCREMENTVALUE ) )
        ,m_aAutoIncrement        ( this, ModuleRes( ET_AUTOINCREMENTVALUE ) )
        ,m_aAutoRetrievingLabel  ( this, ModuleRes( FT_RETRIEVE_AUTO ) )
        ,m_aAutoRetrieving       ( this, ModuleRes( ET_RETRIEVE_AUTO ) )
    {
        m_aAutoRetrievingEnabled.SetClickHdl( getControlModifiedLink() );
        m_aAutoIncrement.SetModifyHdl( getControlModifiedLink() );
        m_aAutoRetrieving.SetModifyHdl( getControlModifiedLink() );

        m_aControlDependencies.enableOnCheckMark( m_aAutoRetrievingEnabled,
            m_aAutoIncrementLabel, m_aAutoIncrement, m_aAutoRetrievingLabel, m_aAutoRetrieving );

        FreeResource();
    }

    // -----------------------------------------------------------------------
    GeneratedValuesPage::~GeneratedValuesPage()
    {
        m_aControlDependencies.clear();
    }

    // -----------------------------------------------------------------------
    void GeneratedValuesPage::fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new ODisableWrapper< FixedLine >( &m_aAutoFixedLine ) );
        _rControlList.push_back( new ODisableWrapper< FixedText >( &m_aAutoIncrementLabel ) );
        _rControlList.push_back( new ODisableWrapper< FixedText >( &m_aAutoRetrievingLabel ) );
    }

    // -----------------------------------------------------------------------
    void GeneratedValuesPage::fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new OSaveValueWrapper< CheckBox >( &m_aAutoRetrievingEnabled ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( &m_aAutoIncrement ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( &m_aAutoRetrieving ) );
    }

    // -----------------------------------------------------------------------
    void GeneratedValuesPage::implInitControls( const SfxItemSet& _rSet, sal_Bool _bSaveValue )
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // collect the items
        SFX_ITEMSET_GET(_rSet, pAutoIncrementItem, SfxStringItem, DSID_AUTOINCREMENTVALUE, sal_True);
        SFX_ITEMSET_GET(_rSet, pAutoRetrieveValueItem, SfxStringItem, DSID_AUTORETRIEVEVALUE, sal_True);
        SFX_ITEMSET_GET(_rSet, pAutoRetrieveEnabledItem, SfxBoolItem, DSID_AUTORETRIEVEENABLED, sal_True);

        // forward the values to the controls
        if (bValid)
        {
            sal_Bool bEnabled = pAutoRetrieveEnabledItem->GetValue();
            m_aAutoRetrievingEnabled.Check( bEnabled );

            m_aAutoIncrement.SetText( pAutoIncrementItem->GetValue() );
            m_aAutoIncrement.ClearModifyFlag();
            m_aAutoRetrieving.SetText( pAutoRetrieveValueItem->GetValue() );
            m_aAutoRetrieving.ClearModifyFlag();
        }
        OGenericAdministrationPage::implInitControls( _rSet, _bSaveValue );
    }

    // -----------------------------------------------------------------------
    sal_Bool GeneratedValuesPage::FillItemSet(SfxItemSet& _rSet)
    {
        sal_Bool bChangedSomething = sal_False;

        fillString( _rSet, &m_aAutoIncrement, DSID_AUTOINCREMENTVALUE, bChangedSomething );
        fillBool( _rSet, &m_aAutoRetrievingEnabled, DSID_AUTORETRIEVEENABLED, bChangedSomething );
        fillString( _rSet, &m_aAutoRetrieving, DSID_AUTORETRIEVEVALUE, bChangedSomething );

        return bChangedSomething;
    }

    //========================================================================
    //= AdvancedSettingsDialog
    //========================================================================
    //------------------------------------------------------------------------
    AdvancedSettingsDialog::AdvancedSettingsDialog( Window* _pParent, SfxItemSet* _pItems,
        const Reference< XMultiServiceFactory >& _rxORB, const Any& _aDataSourceName )
        :SfxTabDialog(_pParent, ModuleRes(DLG_DATABASE_ADVANCED), _pItems)
    {
        m_pImpl = ::std::auto_ptr<ODbDataSourceAdministrationHelper>(new ODbDataSourceAdministrationHelper(_rxORB,_pParent,this));
        m_pImpl->setDataSourceOrName(_aDataSourceName);
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        m_pImpl->translateProperties(xDatasource, *_pItems);
        SetInputSet(_pItems);
        // propagate this set as our new input set and reset the example set
        delete pExampleSet;
        pExampleSet = new SfxItemSet(*GetInputSetImpl());

        const ::rtl::OUString eType = m_pImpl->getDatasourceType(*_pItems);

        DataSourceMetaData aMeta( eType );
        const FeatureSet& rFeatures( aMeta.getFeatureSet() );

        // auto-generated values?
        if ( rFeatures.supportsGeneratedValues() )
            AddTabPage( PAGE_GENERATED_VALUES, String( ModuleRes( STR_GENERATED_VALUE ) ), ODriversSettings::CreateGeneratedValuesPage, NULL );

        // any "special settings"?
        if ( rFeatures.supportsAnySpecialSetting() )
            AddTabPage( PAGE_ADVANCED_SETTINGS_SPECIAL, String( ModuleRes( STR_DS_BEHAVIOUR ) ), ODriversSettings::CreateSpecialSettingsPage, NULL );

        // remove the reset button - it's meaning is much too ambiguous in this dialog
        RemoveResetButton();
        FreeResource();
    }

    // -----------------------------------------------------------------------
    AdvancedSettingsDialog::~AdvancedSettingsDialog()
    {
        SetInputSet(NULL);
        DELETEZ(pExampleSet);
    }

    // -----------------------------------------------------------------------
    bool AdvancedSettingsDialog::doesHaveAnyAdvancedSettings( const ::rtl::OUString& _sURL )
    {
        DataSourceMetaData aMeta( _sURL );
        const FeatureSet& rFeatures( aMeta.getFeatureSet() );
        if ( rFeatures.supportsGeneratedValues() || rFeatures.supportsAnySpecialSetting() )
            return true;
        return false;
    }

    // -----------------------------------------------------------------------
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

    //-------------------------------------------------------------------------
    void AdvancedSettingsDialog::PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage)
    {
        // register ourself as modified listener
        static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory(m_pImpl->getORB());
        static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);

        AdjustLayout();
        Window *pWin = GetViewWindow();
        if(pWin)
            pWin->Invalidate();

        SfxTabDialog::PageCreated(_nId, _rPage);
    }

    // -----------------------------------------------------------------------------
    const SfxItemSet* AdvancedSettingsDialog::getOutputSet() const
    {
        return pExampleSet;
    }

    // -----------------------------------------------------------------------------
    SfxItemSet* AdvancedSettingsDialog::getWriteOutputSet()
    {
        return pExampleSet;
    }

    // -----------------------------------------------------------------------------
    ::std::pair< Reference< XConnection >, sal_Bool > AdvancedSettingsDialog::createConnection()
    {
        return m_pImpl->createConnection();
    }

    // -----------------------------------------------------------------------------
    Reference< XMultiServiceFactory > AdvancedSettingsDialog::getORB() const
    {
        return m_pImpl->getORB();
    }

    // -----------------------------------------------------------------------------
    Reference< XDriver > AdvancedSettingsDialog::getDriver()
    {
        return m_pImpl->getDriver();
    }

    // -----------------------------------------------------------------------------
    ::rtl::OUString AdvancedSettingsDialog::getDatasourceType(const SfxItemSet& _rSet) const
    {
        return m_pImpl->getDatasourceType(_rSet);
    }

    // -----------------------------------------------------------------------------
    void AdvancedSettingsDialog::clearPassword()
    {
        m_pImpl->clearPassword();
    }

    // -----------------------------------------------------------------------------
    void AdvancedSettingsDialog::setTitle(const ::rtl::OUString& _sTitle)
    {
        SetText(_sTitle);
    }

    //-------------------------------------------------------------------------
    void AdvancedSettingsDialog::enableConfirmSettings( bool _bEnable )
    {
        (void)_bEnable;
    }

    //-------------------------------------------------------------------------
    sal_Bool AdvancedSettingsDialog::saveDatasource()
    {
        return PrepareLeaveCurrentPage();
    }

//........................................................................
} // namespace dbaui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
