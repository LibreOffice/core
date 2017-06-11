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
#include "strings.hrc"
#include "formstrings.hxx"
#include <sal/log.hxx>
#include <vcl/combobox.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/layout.hxx>
#include <vcl/builderfactory.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;


    //= FieldLinkRow

    class FieldLinkRow : public TabPage
    {
    private:
        VclPtr<ComboBox>   m_pDetailColumn;
        VclPtr<ComboBox>   m_pMasterColumn;

        Link<FieldLinkRow&,void> m_aLinkChangeHandler;

    public:
        explicit FieldLinkRow( vcl::Window* _pParent );
        virtual ~FieldLinkRow() override;
        virtual void dispose() override;

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

    private:
        DECL_LINK( OnFieldNameChanged, Edit&, void );
    };


    FieldLinkRow::FieldLinkRow( vcl::Window* _pParent )
        :TabPage( _pParent, "FieldLinkRow", "modules/spropctrlr/ui/fieldlinkrow.ui" )
    {
        get(m_pDetailColumn, "detailCombobox");
        get(m_pMasterColumn, "masterCombobox");

        m_pDetailColumn->SetDropDownLineCount( 10 );
        m_pMasterColumn->SetDropDownLineCount( 10 );

        m_pDetailColumn->SetModifyHdl( LINK( this, FieldLinkRow, OnFieldNameChanged ) );
        m_pMasterColumn->SetModifyHdl( LINK( this, FieldLinkRow, OnFieldNameChanged ) );
    }

    FieldLinkRow::~FieldLinkRow()
    {
        disposeOnce();
    }

    void FieldLinkRow::dispose()
    {
        m_pDetailColumn.clear();
        m_pMasterColumn.clear();
        TabPage::dispose();
    }

    void FieldLinkRow::fillList( LinkParticipant _eWhich, const Sequence< OUString >& _rFieldNames )
    {
        ComboBox* pBox = ( _eWhich == eDetailField ) ? m_pDetailColumn.get() : m_pMasterColumn.get();

        const OUString* pFieldName    = _rFieldNames.getConstArray();
        const OUString* pFieldNameEnd = pFieldName + _rFieldNames.getLength();
        for ( ; pFieldName != pFieldNameEnd; ++pFieldName )
            pBox->InsertEntry( *pFieldName );
    }


    bool FieldLinkRow::GetFieldName( LinkParticipant _eWhich, OUString& /* [out] */ _rName ) const
    {
        const ComboBox* pBox = ( _eWhich == eDetailField ) ? m_pDetailColumn : m_pMasterColumn;
        _rName = pBox->GetText();
        return !_rName.isEmpty();
    }


    void FieldLinkRow::SetFieldName( LinkParticipant _eWhich, const OUString& _rName )
    {
        ComboBox* pBox = ( _eWhich == eDetailField ) ? m_pDetailColumn.get() : m_pMasterColumn.get();
        pBox->SetText( _rName );
    }


    IMPL_LINK_NOARG( FieldLinkRow, OnFieldNameChanged, Edit&, void )
    {
        m_aLinkChangeHandler.Call( *this );
    }

    VCL_BUILDER_FACTORY(FieldLinkRow)

    //= FormLinkDialog

    FormLinkDialog::FormLinkDialog( vcl::Window* _pParent, const Reference< XPropertySet >& _rxDetailForm,
            const Reference< XPropertySet >& _rxMasterForm, const Reference< XComponentContext >& _rxContext,
            const OUString& _sExplanation,
            const OUString& _sDetailLabel,
            const OUString& _sMasterLabel)
        :ModalDialog( _pParent, "FormLinks", "modules/spropctrlr/ui/formlinksdialog.ui" )
        ,m_aRow1       ( VclPtr<FieldLinkRow>::Create( get<VclVBox>("box") ) )
        ,m_aRow2       ( VclPtr<FieldLinkRow>::Create( get<VclVBox>("box") ) )
        ,m_aRow3       ( VclPtr<FieldLinkRow>::Create( get<VclVBox>("box") ) )
        ,m_aRow4       ( VclPtr<FieldLinkRow>::Create( get<VclVBox>("box") ) )
        ,m_xContext    ( _rxContext )
        ,m_xDetailForm( _rxDetailForm )
        ,m_xMasterForm( _rxMasterForm )
        ,m_sDetailLabel(_sDetailLabel)
        ,m_sMasterLabel(_sMasterLabel)
    {
        get(m_pExplanation, "explanationLabel");
        get(m_pDetailLabel, "detailLabel");
        get(m_pMasterLabel, "masterLabel");
        get(m_pOK, "ok");
        get(m_pSuggest, "suggestButton");
        m_aRow1->Show();
        m_aRow2->Show();
        m_aRow3->Show();
        m_aRow4->Show();
        set_width_request(600);

        if ( !_sExplanation.isEmpty() )
            m_pExplanation->SetText(_sExplanation);

        m_pSuggest->SetClickHdl       ( LINK( this, FormLinkDialog, OnSuggest      ) );
        m_aRow1->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_aRow2->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_aRow3->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_aRow4->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );

        PostUserEvent( LINK( this, FormLinkDialog, OnInitialize ), nullptr, true );

        updateOkButton();
    }


    FormLinkDialog::~FormLinkDialog( )
    {
        disposeOnce();
    }

    void FormLinkDialog::dispose( )
    {
        m_pExplanation.clear();
        m_pDetailLabel.clear();
        m_pMasterLabel.clear();
        m_pOK.clear();
        m_pSuggest.clear();

        m_aRow1.disposeAndClear();
        m_aRow2.disposeAndClear();
        m_aRow3.disposeAndClear();
        m_aRow4.disposeAndClear();

        ModalDialog::dispose();
    }

    void FormLinkDialog::commitLinkPairs()
    {
        // collect the field lists from the rows
        std::vector< OUString > aDetailFields; aDetailFields.reserve( 4 );
        std::vector< OUString > aMasterFields; aMasterFields.reserve( 4 );

        const FieldLinkRow* aRows[] = {
            m_aRow1.get(), m_aRow2.get(), m_aRow3.get(), m_aRow4.get()
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
            Reference< XPropertySet > xDetailFormProps( m_xDetailForm, UNO_QUERY );
            if ( xDetailFormProps.is() )
            {
                xDetailFormProps->setPropertyValue( PROPERTY_DETAILFIELDS, makeAny( Sequence< OUString >( aDetailFields.data(), aDetailFields.size() ) ) );
                xDetailFormProps->setPropertyValue( PROPERTY_MASTERFIELDS, makeAny( Sequence< OUString >( aMasterFields.data(), aMasterFields.size() ) ) );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormLinkDialog::commitLinkPairs: caught an exception while setting the properties!" );
        }
    }


    short FormLinkDialog::Execute()
    {
        short nResult = ModalDialog::Execute();

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
            m_aRow1.get(), m_aRow2.get(), m_aRow3.get(), m_aRow4.get()
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
        m_pDetailLabel->SetText( sDetailType );

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
        m_pMasterLabel->SetText( sMasterType );
    }


    void FormLinkDialog::initializeFieldRowsFrom( std::vector< OUString >& _rDetailFields, std::vector< OUString >& _rMasterFields )
    {
        // our UI does allow 4 fields max
        _rDetailFields.resize( 4 );
        _rMasterFields.resize( 4 );

        FieldLinkRow* aRows[] = {
            m_aRow1.get(), m_aRow2.get(), m_aRow3.get(), m_aRow4.get()
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

            Reference< XPropertySet > xDetailFormProps( m_xDetailForm, UNO_QUERY );
            if ( xDetailFormProps.is() )
            {
                xDetailFormProps->getPropertyValue( PROPERTY_DETAILFIELDS ) >>= aDetailFields;
                xDetailFormProps->getPropertyValue( PROPERTY_MASTERFIELDS ) >>= aMasterFields;
            }

            std::vector< OUString > aDetailFields1;
            comphelper::sequenceToContainer(aDetailFields1, aDetailFields);
            std::vector< OUString > aMasterFields1;
            comphelper::sequenceToContainer(aMasterFields1, aMasterFields);
            initializeFieldRowsFrom( aDetailFields1, aMasterFields1 );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormLinkDialog::initializeLinks: caught an exception!" );
        }
    }


    void FormLinkDialog::updateOkButton()
    {
        // in all rows, there must be either two valid selections, or none at all
        // If there is at least one row with exactly one valid selection, then the
        // OKButton needs to be disabled
        bool bEnable = true;

        const FieldLinkRow* aRows[] = {
            m_aRow1.get(), m_aRow2.get(), m_aRow3.get(), m_aRow4.get()
        };

        for ( sal_Int32 i = 0; ( i < 4 ) && bEnable; ++i )
        {
            OUString sNotInterestedInRightNow;
            if  (  aRows[ i ]->GetFieldName( FieldLinkRow::eDetailField, sNotInterestedInRightNow )
                != aRows[ i ]->GetFieldName( FieldLinkRow::eMasterField, sNotInterestedInRightNow )
                )
                bEnable = false;
        }

        m_pOK->Enable( bEnable );
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
            OSL_FAIL( "FormLinkDialog::getFormDataSourceType: caught an exception!" );
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
            WaitObject aWaitCursor( const_cast< FormLinkDialog* >( this ) );

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
            OSL_FAIL( "FormLinkDialog::getFormFields: caught a non-SQL exception!" );
        }

        if ( aErrorInfo.isValid() )
        {
            OUString sErrorMessage;
            {
                sErrorMessage = PcrRes(STR_ERROR_RETRIEVING_COLUMNS);
                sErrorMessage = sErrorMessage.replaceFirst("#", sCommand);
            }

            SQLContext aContext;
            aContext.Message = sErrorMessage;
            aContext.NextException = aErrorInfo.get();
            ::dbtools::showError( aContext, VCLUnoHelper::GetInterface( const_cast< FormLinkDialog* >( this ) ), m_xContext );
        }
    }


    void FormLinkDialog::ensureFormConnection( const Reference< XPropertySet >& _rxFormProps, Reference< XConnection >& /* [out] */ _rxConnection ) const
    {
        OSL_PRECOND( _rxFormProps.is(), "FormLinkDialog::ensureFormConnection: invalid form!" );
        if ( !_rxFormProps.is() )
            return;
        if ( _rxFormProps->getPropertySetInfo()->hasPropertyByName(PROPERTY_ACTIVE_CONNECTION) )
            _rxConnection.set(_rxFormProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);

        if ( !_rxConnection.is() )
            _rxConnection = ::dbtools::connectRowset( Reference< XRowSet >( _rxFormProps, UNO_QUERY ), m_xContext, true );
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
            Reference< XTablesSupplier > xTablesInForm( ::dbtools::getCurrentSettingsComposer( _rxFormProps, m_xContext ), UNO_QUERY );
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
            OSL_FAIL( "FormLinkDialog::getCanonicUnderlyingTable: caught an exception!" );
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
            OSL_FAIL( "FormLinkDialog::getExistingRelation: caught an exception!" );
        }

        return ( !_rLeftFields.empty() ) && ( !_rLeftFields[ 0 ].isEmpty() );
    }


    void FormLinkDialog::initializeSuggest()
    {
        Reference< XPropertySet > xDetailFormProps( m_xDetailForm, UNO_QUERY );
        Reference< XPropertySet > xMasterFormProps( m_xMasterForm, UNO_QUERY );
        if ( !xDetailFormProps.is() || !xMasterFormProps.is() )
            return;

        try
        {
            bool bEnable = true;

            // only show the button when both forms are based on the same data source
            if ( bEnable )
            {
                OUString sMasterDS, sDetailDS;
                xMasterFormProps->getPropertyValue( PROPERTY_DATASOURCE ) >>= sMasterDS;
                xDetailFormProps->getPropertyValue( PROPERTY_DATASOURCE ) >>= sDetailDS;
                bEnable = ( sMasterDS == sDetailDS );
            }

            // only show the button when the connection supports relations
            if ( bEnable )
            {
                Reference< XDatabaseMetaData > xMeta;
                getConnectionMetaData( xDetailFormProps, xMeta );
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
                xDetailTable = getCanonicUnderlyingTable( xDetailFormProps );
                xMasterTable = getCanonicUnderlyingTable( xMasterFormProps );
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

            m_pSuggest->Enable( bEnable );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormLinkDialog::initializeSuggest: caught an exception!" );
        }
    }


    IMPL_LINK_NOARG( FormLinkDialog, OnSuggest, Button*, void )
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
