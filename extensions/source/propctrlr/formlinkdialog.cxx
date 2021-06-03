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


#include "formlinkdialog.hxx"

#include "modulepcr.hxx"
#include <strings.hrc>
#include "formstrings.hxx"
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>

namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;


    //= FieldLinkRow

    class FieldLinkRow
    {
    private:
        std::unique_ptr<weld::ComboBox> m_xDetailColumn;
        std::unique_ptr<weld::ComboBox> m_xMasterColumn;

        Link<FieldLinkRow&,void> m_aLinkChangeHandler;

    public:
        FieldLinkRow(std::unique_ptr<weld::ComboBox> xDetailColumn,
                     std::unique_ptr<weld::ComboBox> xMasterColumn);


        void         SetLinkChangeHandler( const Link<FieldLinkRow&,void>& _rHdl ) { m_aLinkChangeHandler = _rHdl; }

        enum LinkParticipant
        {
            eDetailField,
            eMasterField
        };
        /** retrieves the selected field name for either the master or the detail field
            @return <TRUE/> if and only a valid field is selected
        */
        bool    GetFieldName( LinkParticipant _eWhich, OUString& /* [out] */ _rName ) const;
        void    SetFieldName( LinkParticipant _eWhich, const OUString& _rName );

        void    fillList( LinkParticipant _eWhich, const Sequence< OUString >& _rFieldNames );

        void    Show()
        {
            m_xDetailColumn->show();
            m_xMasterColumn->show();
        }

    private:
        DECL_LINK( OnFieldNameChanged, weld::ComboBox&, void );
    };


    FieldLinkRow::FieldLinkRow(std::unique_ptr<weld::ComboBox> xDetailColumn,
                               std::unique_ptr<weld::ComboBox> xMasterColumn)
        : m_xDetailColumn(std::move(xDetailColumn))
        , m_xMasterColumn(std::move(xMasterColumn))
    {
        m_xDetailColumn->connect_changed( LINK( this, FieldLinkRow, OnFieldNameChanged ) );
        m_xMasterColumn->connect_changed( LINK( this, FieldLinkRow, OnFieldNameChanged ) );
    }

    void FieldLinkRow::fillList( LinkParticipant _eWhich, const Sequence< OUString >& _rFieldNames )
    {
        weld::ComboBox* pBox = ( _eWhich == eDetailField ) ? m_xDetailColumn.get() : m_xMasterColumn.get();

        const OUString* pFieldName    = _rFieldNames.getConstArray();
        const OUString* pFieldNameEnd = pFieldName + _rFieldNames.getLength();
        for ( ; pFieldName != pFieldNameEnd; ++pFieldName )
            pBox->append_text( *pFieldName );
    }

    bool FieldLinkRow::GetFieldName( LinkParticipant _eWhich, OUString& /* [out] */ _rName ) const
    {
        const weld::ComboBox* pBox = ( _eWhich == eDetailField ) ? m_xDetailColumn.get() : m_xMasterColumn.get();
        _rName = pBox->get_active_text();
        return !_rName.isEmpty();
    }

    void FieldLinkRow::SetFieldName( LinkParticipant _eWhich, const OUString& _rName )
    {
        weld::ComboBox* pBox = ( _eWhich == eDetailField ) ? m_xDetailColumn.get() : m_xMasterColumn.get();
        pBox->set_entry_text( _rName );
    }

    IMPL_LINK_NOARG( FieldLinkRow, OnFieldNameChanged, weld::ComboBox&, void )
    {
        m_aLinkChangeHandler.Call( *this );
    }

    //= FormLinkDialog

    FormLinkDialog::FormLinkDialog(weld::Window* _pParent, const Reference< XPropertySet >& _rxDetailForm,
            const Reference< XPropertySet >& _rxMasterForm, const Reference< XComponentContext >& _rxContext,
            const OUString& _sExplanation,
            const OUString& _sDetailLabel,
            const OUString& _sMasterLabel)
        : GenericDialogController(_pParent, "modules/spropctrlr/ui/formlinksdialog.ui", "FormLinks")
        , m_xContext    ( _rxContext )
        , m_xDetailForm( _rxDetailForm )
        , m_xMasterForm( _rxMasterForm )
        , m_sDetailLabel(_sDetailLabel)
        , m_sMasterLabel(_sMasterLabel)
        , m_xExplanation(m_xBuilder->weld_label("explanationLabel"))
        , m_xDetailLabel(m_xBuilder->weld_label("detailLabel"))
        , m_xMasterLabel(m_xBuilder->weld_label("masterLabel"))
        , m_xRow1(std::make_unique<FieldLinkRow>(m_xBuilder->weld_combo_box("detailCombobox1"),
                                                 m_xBuilder->weld_combo_box("masterCombobox1")))
        , m_xRow2(std::make_unique<FieldLinkRow>(m_xBuilder->weld_combo_box("detailCombobox2"),
                                                 m_xBuilder->weld_combo_box("masterCombobox2")))
        , m_xRow3(std::make_unique<FieldLinkRow>(m_xBuilder->weld_combo_box("detailCombobox3"),
                                                 m_xBuilder->weld_combo_box("masterCombobox3")))
        , m_xRow4(std::make_unique<FieldLinkRow>(m_xBuilder->weld_combo_box("detailCombobox4"),
                                                 m_xBuilder->weld_combo_box("masterCombobox4")))
        , m_xOK(m_xBuilder->weld_button("ok"))
        , m_xSuggest(m_xBuilder->weld_button("suggestButton"))
    {
        m_xRow1->Show();
        m_xRow2->Show();
        m_xRow3->Show();
        m_xRow4->Show();
        m_xDialog->set_size_request(600, -1);

        if ( !_sExplanation.isEmpty() )
            m_xExplanation->set_label(_sExplanation);

        m_xSuggest->connect_clicked(LINK(this, FormLinkDialog, OnSuggest));
        m_xRow1->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_xRow2->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_xRow3->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_xRow4->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );

        Application::PostUserEvent(LINK(this, FormLinkDialog, OnInitialize));

        updateOkButton();
    }

    FormLinkDialog::~FormLinkDialog()
    {
    }

    void FormLinkDialog::commitLinkPairs()
    {
        // collect the field lists from the rows
        std::vector< OUString > aDetailFields; aDetailFields.reserve( 4 );
        std::vector< OUString > aMasterFields; aMasterFields.reserve( 4 );

        const FieldLinkRow* aRows[] = {
            m_xRow1.get(), m_xRow2.get(), m_xRow3.get(), m_xRow4.get()
        };

        for (const FieldLinkRow* aRow : aRows)
        {
            OUString sDetailField, sMasterField;
            aRow->GetFieldName( FieldLinkRow::eDetailField, sDetailField );
            aRow->GetFieldName( FieldLinkRow::eMasterField, sMasterField );
            if ( sDetailField.isEmpty() && sMasterField.isEmpty() )
                continue;

            aDetailFields.push_back( sDetailField );
            aMasterFields.push_back( sMasterField );
        }

        // and set as property values
        try
        {
            if ( m_xDetailForm.is() )
            {
                m_xDetailForm->setPropertyValue( PROPERTY_DETAILFIELDS, makeAny( Sequence< OUString >( aDetailFields.data(), aDetailFields.size() ) ) );
                m_xDetailForm->setPropertyValue( PROPERTY_MASTERFIELDS, makeAny( Sequence< OUString >( aMasterFields.data(), aMasterFields.size() ) ) );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION("extensions.propctrlr",
                                 "caught an exception while setting the properties!");
        }
    }

    short FormLinkDialog::run()
    {
        short nResult = GenericDialogController::run();

        if ( RET_OK == nResult )
            commitLinkPairs();

        return nResult;
    }

    void FormLinkDialog::initializeFieldLists()
    {
        Sequence< OUString > sDetailFields;
        getFormFields( m_xDetailForm, sDetailFields );

        Sequence< OUString > sMasterFields;
        getFormFields( m_xMasterForm, sMasterFields );

        FieldLinkRow* aRows[] = {
            m_xRow1.get(), m_xRow2.get(), m_xRow3.get(), m_xRow4.get()
        };
        for (FieldLinkRow* aRow : aRows)
        {
            aRow->fillList( FieldLinkRow::eDetailField, sDetailFields );
            aRow->fillList( FieldLinkRow::eMasterField, sMasterFields );
        }

    }


    void FormLinkDialog::initializeColumnLabels()
    {
        // label for the detail form
        OUString sDetailType = getFormDataSourceType( m_xDetailForm );
        if ( sDetailType.isEmpty() )
        {
            if ( m_sDetailLabel.isEmpty() )
            {
                m_sDetailLabel = PcrRes(STR_DETAIL_FORM);
            }
            sDetailType = m_sDetailLabel;
        }
        m_xDetailLabel->set_label( sDetailType );

        // label for the master form
        OUString sMasterType = getFormDataSourceType( m_xMasterForm );
        if ( sMasterType.isEmpty() )
        {
            if ( m_sMasterLabel.isEmpty() )
            {
                m_sMasterLabel = PcrRes(STR_MASTER_FORM);
            }
            sMasterType = m_sMasterLabel;
        }
        m_xMasterLabel->set_label( sMasterType );
    }

    void FormLinkDialog::initializeFieldRowsFrom( std::vector< OUString >& _rDetailFields, std::vector< OUString >& _rMasterFields )
    {
        // our UI does allow 4 fields max
        _rDetailFields.resize( 4 );
        _rMasterFields.resize( 4 );

        FieldLinkRow* aRows[] = {
            m_xRow1.get(), m_xRow2.get(), m_xRow3.get(), m_xRow4.get()
        };
        for ( sal_Int32 i = 0; i < 4; ++i )
        {
            aRows[ i ]->SetFieldName( FieldLinkRow::eDetailField, _rDetailFields[i] );
            aRows[ i ]->SetFieldName( FieldLinkRow::eMasterField, _rMasterFields[i] );
        }
    }


    void FormLinkDialog::initializeLinks()
    {
        try
        {
            Sequence< OUString > aDetailFields;
            Sequence< OUString > aMasterFields;

            if ( m_xDetailForm.is() )
            {
                m_xDetailForm->getPropertyValue( PROPERTY_DETAILFIELDS ) >>= aDetailFields;
                m_xDetailForm->getPropertyValue( PROPERTY_MASTERFIELDS ) >>= aMasterFields;
            }

            std::vector< OUString > aDetailFields1;
            comphelper::sequenceToContainer(aDetailFields1, aDetailFields);
            std::vector< OUString > aMasterFields1;
            comphelper::sequenceToContainer(aMasterFields1, aMasterFields);
            initializeFieldRowsFrom( aDetailFields1, aMasterFields1 );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "FormLinkDialog::initializeLinks" );
        }
    }


    void FormLinkDialog::updateOkButton()
    {
        // in all rows, there must be either two valid selections, or none at all
        // If there is at least one row with exactly one valid selection, then the
        // OKButton needs to be disabled
        bool bEnable = true;

        const FieldLinkRow* aRows[] = {
            m_xRow1.get(), m_xRow2.get(), m_xRow3.get(), m_xRow4.get()
        };

        for ( sal_Int32 i = 0; ( i < 4 ) && bEnable; ++i )
        {
            OUString sNotInterestedInRightNow;
            if  (  aRows[ i ]->GetFieldName( FieldLinkRow::eDetailField, sNotInterestedInRightNow )
                != aRows[ i ]->GetFieldName( FieldLinkRow::eMasterField, sNotInterestedInRightNow )
                )
                bEnable = false;
        }

        m_xOK->set_sensitive(bEnable);
    }

    OUString FormLinkDialog::getFormDataSourceType( const Reference< XPropertySet >& _rxForm )
    {
        OUString sReturn;
        if ( !_rxForm.is() )
            return sReturn;

        try
        {
            sal_Int32       nCommandType = CommandType::COMMAND;
            OUString sCommand;

            _rxForm->getPropertyValue( PROPERTY_COMMANDTYPE ) >>= nCommandType;
            _rxForm->getPropertyValue( PROPERTY_COMMAND     ) >>= sCommand;

            if  (  ( nCommandType == CommandType::TABLE )
                || ( nCommandType == CommandType::QUERY )
                )
                sReturn = sCommand;
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "FormLinkDialog::getFormDataSourceType" );
        }
        return sReturn;
    }

    void FormLinkDialog::getFormFields( const Reference< XPropertySet >& _rxForm, Sequence< OUString >& /* [out] */ _rNames ) const
    {
        _rNames.realloc( 0 );

        ::dbtools::SQLExceptionInfo aErrorInfo;
        OUString sCommand;
        try
        {
            weld::WaitObject aWaitCursor(m_xDialog.get());

            OSL_ENSURE( _rxForm.is(), "FormLinkDialog::getFormFields: invalid form!" );

            sal_Int32       nCommandType = CommandType::COMMAND;

            _rxForm->getPropertyValue( PROPERTY_COMMANDTYPE ) >>= nCommandType;
            _rxForm->getPropertyValue( PROPERTY_COMMAND     ) >>= sCommand;

            Reference< XConnection > xConnection;
            ensureFormConnection( _rxForm, xConnection );

            _rNames = ::dbtools::getFieldNamesByCommandDescriptor(
                xConnection,
                nCommandType,
                sCommand,
                &aErrorInfo
            );
        }
        catch (const SQLContext& e)    { aErrorInfo = e; }
        catch (const SQLWarning& e)    { aErrorInfo = e; }
        catch (const SQLException& e ) { aErrorInfo = e; }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "FormLinkDialog::getFormFields: caught a non-SQL exception!" );
        }

        if ( !aErrorInfo.isValid() )
            return;

        OUString sErrorMessage;
        {
            sErrorMessage = PcrRes(STR_ERROR_RETRIEVING_COLUMNS);
            sErrorMessage = sErrorMessage.replaceFirst("#", sCommand);
        }

        SQLContext aContext;
        aContext.Message = sErrorMessage;
        aContext.NextException = aErrorInfo.get();
        ::dbtools::showError(aContext, m_xDialog->GetXWindow(), m_xContext);
    }

    void FormLinkDialog::ensureFormConnection( const Reference< XPropertySet >& _rxFormProps, Reference< XConnection >& /* [out] */ _rxConnection ) const
    {
        OSL_PRECOND( _rxFormProps.is(), "FormLinkDialog::ensureFormConnection: invalid form!" );
        if ( !_rxFormProps.is() )
            return;
        if ( _rxFormProps->getPropertySetInfo()->hasPropertyByName(PROPERTY_ACTIVE_CONNECTION) )
            _rxConnection.set(_rxFormProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);

        if ( !_rxConnection.is() )
            _rxConnection = ::dbtools::connectRowset( Reference< XRowSet >( _rxFormProps, UNO_QUERY ), m_xContext, nullptr );
    }


    void FormLinkDialog::getConnectionMetaData( const Reference< XPropertySet >& _rxFormProps, Reference< XDatabaseMetaData >& /* [out] */ _rxMeta )
    {
        if ( _rxFormProps.is() )
        {
            Reference< XConnection > xConnection;
            if ( !::dbtools::isEmbeddedInDatabase( _rxFormProps, xConnection ) )
                _rxFormProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;
            if ( xConnection.is() )
                _rxMeta = xConnection->getMetaData();
        }
    }


    Reference< XPropertySet > FormLinkDialog::getCanonicUnderlyingTable( const Reference< XPropertySet >& _rxFormProps ) const
    {
        Reference< XPropertySet > xTable;
        try
        {
            Reference< XTablesSupplier > xTablesInForm( ::dbtools::getCurrentSettingsComposer( _rxFormProps, m_xContext, nullptr ), UNO_QUERY );
            Reference< XNameAccess > xTables;
            if ( xTablesInForm.is() )
                xTables = xTablesInForm->getTables();
            Sequence< OUString > aTableNames;
            if ( xTables.is() )
                aTableNames = xTables->getElementNames();

            if ( aTableNames.getLength() == 1 )
            {
                xTables->getByName( aTableNames[ 0 ] ) >>= xTable;
                OSL_ENSURE( xTable.is(), "FormLinkDialog::getCanonicUnderlyingTable: invalid table!" );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "FormLinkDialog::getCanonicUnderlyingTable" );
        }
        return xTable;
    }


    bool FormLinkDialog::getExistingRelation( const Reference< XPropertySet >& _rxLHS, const Reference< XPropertySet >& /*_rxRHS*/,
            // TODO: fix the usage of _rxRHS. This is issue #i81956#.
        std::vector< OUString >& _rLeftFields, std::vector< OUString >& _rRightFields )
    {
        try
        {
            Reference< XKeysSupplier > xSuppKeys( _rxLHS, UNO_QUERY );
            Reference< XIndexAccess >  xKeys;
            if ( xSuppKeys.is() )
                xKeys = xSuppKeys->getKeys();

            if ( xKeys.is() )
            {
                Reference< XPropertySet >     xKey;
                Reference< XColumnsSupplier > xKeyColSupp( xKey, UNO_QUERY );
                Reference< XIndexAccess >     xKeyColumns;
                Reference< XPropertySet >     xKeyColumn;
                OUString sColumnName, sRelatedColumnName;

                const sal_Int32 keyCount = xKeys->getCount();
                for ( sal_Int32 key = 0; key < keyCount; ++key )
                {
                    xKeys->getByIndex( key ) >>= xKey;
                    sal_Int32 nKeyType = 0;
                    xKey->getPropertyValue("Type") >>= nKeyType;
                    if ( nKeyType != KeyType::FOREIGN )
                        continue;

                    xKeyColumns.clear();
                    xKeyColSupp.set(xKey, css::uno::UNO_QUERY);
                    if ( xKeyColSupp.is() )
                        xKeyColumns.set(xKeyColSupp->getColumns(), css::uno::UNO_QUERY);
                    OSL_ENSURE( xKeyColumns.is(), "FormLinkDialog::getExistingRelation: could not obtain the columns for the key!" );

                    if ( !xKeyColumns.is() )
                        continue;

                    const sal_Int32 columnCount = xKeyColumns->getCount();
                    _rLeftFields.resize( columnCount );
                    _rRightFields.resize( columnCount );
                    for ( sal_Int32 column = 0; column < columnCount; ++column )
                    {
                        xKeyColumn.clear();
                        xKeyColumns->getByIndex( column ) >>= xKeyColumn;
                        OSL_ENSURE( xKeyColumn.is(), "FormLinkDialog::getExistingRelation: invalid key column!" );
                        if ( xKeyColumn.is() )
                        {
                            xKeyColumn->getPropertyValue( PROPERTY_NAME ) >>= sColumnName;
                            xKeyColumn->getPropertyValue("RelatedColumn") >>= sRelatedColumnName;

                            _rLeftFields[ column ]  = sColumnName;
                            _rRightFields[ column ] = sRelatedColumnName;
                        }
                    }
                }
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "FormLinkDialog::getExistingRelation" );
        }

        return ( !_rLeftFields.empty() ) && ( !_rLeftFields[ 0 ].isEmpty() );
    }


    void FormLinkDialog::initializeSuggest()
    {
        if ( !m_xDetailForm.is() || !m_xMasterForm.is() )
            return;

        try
        {
            // only show the button when both forms are based on the same data source
            OUString sMasterDS, sDetailDS;
            m_xMasterForm->getPropertyValue( PROPERTY_DATASOURCE ) >>= sMasterDS;
            m_xDetailForm->getPropertyValue( PROPERTY_DATASOURCE ) >>= sDetailDS;
            bool bEnable = ( sMasterDS == sDetailDS );

            // only show the button when the connection supports relations
            if ( bEnable )
            {
                Reference< XDatabaseMetaData > xMeta;
                getConnectionMetaData( m_xDetailForm, xMeta );
                OSL_ENSURE( xMeta.is(), "FormLinkDialog::initializeSuggest: unable to retrieve the meta data for the connection!" );
                try
                {
                    bEnable = xMeta.is() && xMeta->supportsIntegrityEnhancementFacility();
                }
                catch(const Exception&)
                {
                    bEnable = false;
                }
            }

            // only enable the button if there is a "canonic" table underlying both forms
            Reference< XPropertySet > xDetailTable, xMasterTable;
            if ( bEnable )
            {
                xDetailTable = getCanonicUnderlyingTable( m_xDetailForm );
                xMasterTable = getCanonicUnderlyingTable( m_xMasterForm );
                bEnable = xDetailTable.is() && xMasterTable.is();
            }

            // only enable the button if there is a relation between both tables
            m_aRelationDetailColumns.clear();
            m_aRelationMasterColumns.clear();
            if ( bEnable )
            {
                bEnable = getExistingRelation( xDetailTable, xMasterTable, m_aRelationDetailColumns, m_aRelationMasterColumns );
                SAL_WARN_IF( m_aRelationMasterColumns.size() != m_aRelationDetailColumns.size(),
                    "extensions.propctrlr",
                    "FormLinkDialog::initializeSuggest: nonsense!" );
                if ( m_aRelationMasterColumns.empty() )
                {   // okay, there is no relation "pointing" (via a foreign key) from the detail table to the master table
                    // but perhaps the other way round (would make less sense, but who knows ...)
                    bEnable = getExistingRelation( xMasterTable, xDetailTable, m_aRelationMasterColumns, m_aRelationDetailColumns );
                }
            }

            // only enable the button if the relation contains at most 4 field pairs
            if ( bEnable )
            {
                bEnable = ( m_aRelationMasterColumns.size() <= 4 );
            }

            m_xSuggest->set_sensitive(bEnable);
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "FormLinkDialog::initializeSuggest" );
        }
    }

    IMPL_LINK_NOARG( FormLinkDialog, OnSuggest, weld::Button&, void )
    {
        initializeFieldRowsFrom( m_aRelationDetailColumns, m_aRelationMasterColumns );
    }

    IMPL_LINK_NOARG( FormLinkDialog, OnFieldChanged, FieldLinkRow&, void )
    {
        updateOkButton();
    }

    IMPL_LINK_NOARG( FormLinkDialog, OnInitialize, void*, void )
    {
        initializeColumnLabels();
        initializeFieldLists();
        initializeLinks();
        initializeSuggest();
    }

}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
