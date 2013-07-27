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
#include "formlinkdialog.hrc"

#include "modulepcr.hxx"
#include "formresid.hrc"
#include "formstrings.hxx"
#include <vcl/combobox.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <svtools/localresaccess.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>


//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //========================================================================
    //= FieldLinkRow
    //========================================================================
    class FieldLinkRow : public Window
    {
    private:
        ComboBox    m_aDetailColumn;
        FixedText   m_aEqualSign;
        ComboBox    m_aMasterColumn;

        Link        m_aLinkChangeHandler;

    public:
        FieldLinkRow( Window* _pParent, const ResId& _rId );

        inline void         SetLinkChangeHandler( const Link& _rHdl ) { m_aLinkChangeHandler = _rHdl; }
        inline const Link&  GetLinkChangeHandler( ) const             { return m_aLinkChangeHandler;  }

        enum LinkParticipant
        {
            eDetailField,
            eMasterField
        };
        /** retrieves the selected field name for either the master or the detail field
            @return <TRUE/> if and only a valid field is selected
        */
        bool    GetFieldName( LinkParticipant _eWhich, String& /* [out] */ _rName ) const;
        void    SetFieldName( LinkParticipant _eWhich, const String& _rName );

        void    fillList( LinkParticipant _eWhich, const Sequence< OUString >& _rFieldNames );

    private:
        DECL_LINK( OnFieldNameChanged, ComboBox* );
    };

    //------------------------------------------------------------------------
    FieldLinkRow::FieldLinkRow( Window* _pParent, const ResId& _rId )
        :Window( _pParent, _rId )
        ,m_aDetailColumn( this, ResId( 1, *_rId.GetResMgr() ) )
        ,m_aEqualSign   ( this, ResId( 1, *_rId.GetResMgr() ) )
        ,m_aMasterColumn( this, ResId( 2, *_rId.GetResMgr() ) )
    {
        FreeResource();

        m_aDetailColumn.SetDropDownLineCount( 10 );
        m_aMasterColumn.SetDropDownLineCount( 10 );

        m_aDetailColumn.SetModifyHdl( LINK( this, FieldLinkRow, OnFieldNameChanged ) );
        m_aMasterColumn.SetModifyHdl( LINK( this, FieldLinkRow, OnFieldNameChanged ) );
    }

    //------------------------------------------------------------------------
    void FieldLinkRow::fillList( LinkParticipant _eWhich, const Sequence< OUString >& _rFieldNames )
    {
        ComboBox* pBox = ( _eWhich == eDetailField ) ? &m_aDetailColumn : &m_aMasterColumn;

        const OUString* pFieldName    = _rFieldNames.getConstArray();
        const OUString* pFieldNameEnd = pFieldName + _rFieldNames.getLength();
        for ( ; pFieldName != pFieldNameEnd; ++pFieldName )
            pBox->InsertEntry( *pFieldName );
    }

    //------------------------------------------------------------------------
    bool FieldLinkRow::GetFieldName( LinkParticipant _eWhich, String& /* [out] */ _rName ) const
    {
        const ComboBox* pBox = ( _eWhich == eDetailField ) ? &m_aDetailColumn : &m_aMasterColumn;
        _rName = pBox->GetText();
        return _rName.Len() != 0;
    }

    //------------------------------------------------------------------------
    void FieldLinkRow::SetFieldName( LinkParticipant _eWhich, const String& _rName )
    {
        ComboBox* pBox = ( _eWhich == eDetailField ) ? &m_aDetailColumn : &m_aMasterColumn;
        pBox->SetText( _rName );
    }

    //------------------------------------------------------------------------
    IMPL_LINK( FieldLinkRow, OnFieldNameChanged, ComboBox*, /*_pBox*/ )
    {
        if ( m_aLinkChangeHandler.IsSet() )
            return m_aLinkChangeHandler.Call( this );

        return 0L;
    }

    //========================================================================
    //= FormLinkDialog
    //========================================================================
    //------------------------------------------------------------------------
    FormLinkDialog::FormLinkDialog( Window* _pParent, const Reference< XPropertySet >& _rxDetailForm,
            const Reference< XPropertySet >& _rxMasterForm, const Reference< XComponentContext >& _rxContext,
            const OUString& _sExplanation,
            const OUString& _sDetailLabel,
            const OUString& _sMasterLabel)
        :ModalDialog( _pParent, PcrRes( RID_DLG_FORMLINKS ) )
        ,m_aExplanation( this, PcrRes( FT_EXPLANATION  ) )
        ,m_aDetailLabel( this, PcrRes( FT_DETAIL_LABEL ) )
        ,m_aMasterLabel( this, PcrRes( FT_MASTER_LABEL ) )
        ,m_aRow1       ( new FieldLinkRow( this, PcrRes( 1 ) ) )
        ,m_aRow2       ( new FieldLinkRow( this, PcrRes( 2 ) ) )
        ,m_aRow3       ( new FieldLinkRow( this, PcrRes( 3 ) ) )
        ,m_aRow4       ( new FieldLinkRow( this, PcrRes( 4 ) ) )
        ,m_aOK         ( this, PcrRes( PB_OK           ) )
        ,m_aCancel     ( this, PcrRes( PB_CANCEL       ) )
        ,m_aHelp       ( this, PcrRes( PB_HELP         ) )
        ,m_aSuggest    ( this, PcrRes( PB_SUGGEST      ) )
        ,m_xContext    ( _rxContext        )
        ,m_xDetailForm( _rxDetailForm )
        ,m_xMasterForm( _rxMasterForm )
        ,m_sDetailLabel(_sDetailLabel)
        ,m_sMasterLabel(_sMasterLabel)
    {
        FreeResource();
        if ( !_sExplanation.isEmpty() )
            m_aExplanation.SetText(_sExplanation);

        m_aSuggest.SetClickHdl       ( LINK( this, FormLinkDialog, OnSuggest      ) );
        m_aRow1->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_aRow2->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_aRow3->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );
        m_aRow4->SetLinkChangeHandler( LINK( this, FormLinkDialog, OnFieldChanged ) );

        PostUserEvent( LINK( this, FormLinkDialog, OnInitialize ) );

        updateOkButton();
    }

    //------------------------------------------------------------------------
    FormLinkDialog::~FormLinkDialog( )
    {
    }

    //------------------------------------------------------------------------
    void FormLinkDialog::commitLinkPairs()
    {
        // collect the field lists from the rows
        ::std::vector< OUString > aDetailFields; aDetailFields.reserve( 4 );
        ::std::vector< OUString > aMasterFields; aMasterFields.reserve( 4 );

        const FieldLinkRow* aRows[] = {
            m_aRow1.get(), m_aRow2.get(), m_aRow3.get(), m_aRow4.get()
        };

        for ( sal_Int32 i = 0; i < 4; ++i )
        {
            String sDetailField, sMasterField;
            aRows[ i ]->GetFieldName( FieldLinkRow::eDetailField, sDetailField );
            aRows[ i ]->GetFieldName( FieldLinkRow::eMasterField, sMasterField );
            if ( !sDetailField.Len() && !sMasterField.Len() )
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
                OUString *pFields = aDetailFields.empty() ? 0 : &aDetailFields[0];
                xDetailFormProps->setPropertyValue( PROPERTY_DETAILFIELDS, makeAny( Sequence< OUString >( pFields, aDetailFields.size() ) ) );
                pFields = aMasterFields.empty() ? 0 : &aMasterFields[0];
                xDetailFormProps->setPropertyValue( PROPERTY_MASTERFIELDS, makeAny( Sequence< OUString >( pFields, aMasterFields.size() ) ) );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormLinkDialog::commitLinkPairs: caught an exception while setting the properties!" );
        }
    }

    //------------------------------------------------------------------------
    short FormLinkDialog::Execute()
    {
        short nResult = ModalDialog::Execute();

        if ( RET_OK == nResult )
            commitLinkPairs();

        return nResult;
    }

    //------------------------------------------------------------------------
    void FormLinkDialog::initializeFieldLists()
    {
        Sequence< OUString > sDetailFields;
        getFormFields( m_xDetailForm, sDetailFields );

        Sequence< OUString > sMasterFields;
        getFormFields( m_xMasterForm, sMasterFields );

        FieldLinkRow* aRows[] = {
            m_aRow1.get(), m_aRow2.get(), m_aRow3.get(), m_aRow4.get()
        };
        for ( sal_Int32 i = 0; i < 4 ; ++i )
        {
            aRows[i]->fillList( FieldLinkRow::eDetailField, sDetailFields );
            aRows[i]->fillList( FieldLinkRow::eMasterField, sMasterFields );
        }

    }

    //------------------------------------------------------------------------
    void FormLinkDialog::initializeColumnLabels()
    {
        // label for the detail form
        String sDetailType = getFormDataSourceType( m_xDetailForm );
        if ( !sDetailType.Len() )
        {
            if ( m_sDetailLabel.isEmpty() )
            {
                ::svt::OLocalResourceAccess aStringAccess( PcrRes( RID_DLG_FORMLINKS ), RSC_MODALDIALOG );
                m_sDetailLabel = PcrRes(STR_DETAIL_FORM).toString();
            }
            sDetailType = m_sDetailLabel;
        }
        m_aDetailLabel.SetText( sDetailType );

        // label for the master form
        String sMasterType = getFormDataSourceType( m_xMasterForm );
        if ( !sMasterType.Len() )
        {
            if ( m_sMasterLabel.isEmpty() )
            {
                ::svt::OLocalResourceAccess aStringAccess( PcrRes( RID_DLG_FORMLINKS ), RSC_MODALDIALOG );
                m_sMasterLabel = PcrRes(STR_MASTER_FORM).toString();
            }
            sMasterType = m_sMasterLabel;
        }
        m_aMasterLabel.SetText( sMasterType );
    }

    //------------------------------------------------------------------------
    void FormLinkDialog::initializeFieldRowsFrom( Sequence< OUString >& _rDetailFields, Sequence< OUString >& _rMasterFields )
    {
        // our UI does allow 4 fields max
        _rDetailFields.realloc( 4 );
        _rMasterFields.realloc( 4 );

        const OUString* pDetailFields = _rDetailFields.getConstArray();
        const OUString* pMasterFields = _rMasterFields.getConstArray();

        FieldLinkRow* aRows[] = {
            m_aRow1.get(), m_aRow2.get(), m_aRow3.get(), m_aRow4.get()
        };
        for ( sal_Int32 i = 0; i < 4; ++i, ++pDetailFields, ++pMasterFields )
        {
            aRows[ i ]->SetFieldName( FieldLinkRow::eDetailField, *pDetailFields );
            aRows[ i ]->SetFieldName( FieldLinkRow::eMasterField, *pMasterFields );
        }
    }

    //------------------------------------------------------------------------
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

            initializeFieldRowsFrom( aDetailFields, aMasterFields );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormLinkDialog::initializeLinks: caught an exception!" );
        }
    }

    //------------------------------------------------------------------------
    void FormLinkDialog::updateOkButton()
    {
        // in all rows, there must be either two valid selections, or none at all
        // If there is at least one row with exactly one valid selection, then the
        // OKButton needs to be disabled
        sal_Bool bEnable = sal_True;

        const FieldLinkRow* aRows[] = {
            m_aRow1.get(), m_aRow2.get(), m_aRow3.get(), m_aRow4.get()
        };

        for ( sal_Int32 i = 0; ( i < 4 ) && bEnable; ++i )
        {
            String sNotInterestedInRightNow;
            if  (  aRows[ i ]->GetFieldName( FieldLinkRow::eDetailField, sNotInterestedInRightNow )
                != aRows[ i ]->GetFieldName( FieldLinkRow::eMasterField, sNotInterestedInRightNow )
                )
                bEnable = sal_False;
        }

        m_aOK.Enable( bEnable );
    }

    //------------------------------------------------------------------------
    String FormLinkDialog::getFormDataSourceType( const Reference< XPropertySet >& _rxForm ) const SAL_THROW(())
    {
        String sReturn;
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

    //------------------------------------------------------------------------
    void FormLinkDialog::getFormFields( const Reference< XPropertySet >& _rxForm, Sequence< OUString >& /* [out] */ _rNames ) const SAL_THROW(( ))
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
            String sErrorMessage;
            {
                ::svt::OLocalResourceAccess aStringAccess( PcrRes( RID_DLG_FORMLINKS ), RSC_MODALDIALOG );
                sErrorMessage = PcrRes(STR_ERROR_RETRIEVING_COLUMNS).toString();
                sErrorMessage.SearchAndReplace(OUString('#'), sCommand);
            }

            SQLContext aContext;
            aContext.Message = sErrorMessage;
            aContext.NextException = aErrorInfo.get();
            ::dbtools::showError( aContext, VCLUnoHelper::GetInterface( const_cast< FormLinkDialog* >( this ) ), m_xContext );
        }
    }

    //------------------------------------------------------------------------
    void FormLinkDialog::ensureFormConnection( const Reference< XPropertySet >& _rxFormProps, Reference< XConnection >& /* [out] */ _rxConnection ) const SAL_THROW(( Exception ))
    {
        OSL_PRECOND( _rxFormProps.is(), "FormLinkDialog::ensureFormConnection: invalid form!" );
        if ( !_rxFormProps.is() )
            return;
        if ( _rxFormProps->getPropertySetInfo()->hasPropertyByName(PROPERTY_ACTIVE_CONNECTION) )
            _rxConnection.set(_rxFormProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);

        if ( !_rxConnection.is() )
            _rxConnection = ::dbtools::connectRowset( Reference< XRowSet >( _rxFormProps, UNO_QUERY ), m_xContext, sal_True );
    }

    //------------------------------------------------------------------------
    void FormLinkDialog::getConnectionMetaData( const Reference< XPropertySet >& _rxFormProps, Reference< XDatabaseMetaData >& /* [out] */ _rxMeta ) const SAL_THROW(( Exception ))
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

    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
    sal_Bool FormLinkDialog::getExistingRelation( const Reference< XPropertySet >& _rxLHS, const Reference< XPropertySet >& /*_rxRHS*/,
            // TODO: fix the usage of _rxRHS. This is issue #i81956#.
        Sequence< OUString >& _rLeftFields, Sequence< OUString >& _rRightFields ) const
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
                    xKeyColSupp = xKeyColSupp.query( xKey );
                    if ( xKeyColSupp.is() )
                        xKeyColumns = xKeyColumns.query( xKeyColSupp->getColumns() );
                    OSL_ENSURE( xKeyColumns.is(), "FormLinkDialog::getExistingRelation: could not obtain the columns for the key!" );

                    if ( !xKeyColumns.is() )
                        continue;

                    const sal_Int32 columnCount = xKeyColumns->getCount();
                    _rLeftFields.realloc( columnCount );
                    _rRightFields.realloc( columnCount );
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

        return ( _rLeftFields.getLength() > 0 ) && ( !_rLeftFields[ 0 ].isEmpty() );
    }

    //------------------------------------------------------------------------
    void FormLinkDialog::initializeSuggest()
    {
        Reference< XPropertySet > xDetailFormProps( m_xDetailForm, UNO_QUERY );
        Reference< XPropertySet > xMasterFormProps( m_xMasterForm, UNO_QUERY );
        if ( !xDetailFormProps.is() || !xMasterFormProps.is() )
            return;

        try
        {
            sal_Bool bEnable = sal_True;

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
                    bEnable = sal_False;
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
            m_aRelationDetailColumns.realloc( 0 );
            m_aRelationMasterColumns.realloc( 0 );
            if ( bEnable )
            {
                bEnable = getExistingRelation( xDetailTable, xMasterTable, m_aRelationDetailColumns, m_aRelationMasterColumns );
                OSL_POSTCOND( m_aRelationMasterColumns.getLength() == m_aRelationDetailColumns.getLength(), "FormLinkDialog::initializeSuggest: nonsense!" );
                if ( m_aRelationMasterColumns.getLength() == 0 )
                {   // okay, there is no relation "pointing" (via a foreign key) from the detail table to the master table
                    // but perhaps the other way round (would make less sense, but who knows ...)
                    bEnable = getExistingRelation( xMasterTable, xDetailTable, m_aRelationMasterColumns, m_aRelationDetailColumns );
                }
            }

            // only enable the button if the relation contains at most 4 field pairs
            if ( bEnable )
            {
                bEnable = ( m_aRelationMasterColumns.getLength() <= 4 );
            }

            m_aSuggest.Enable( bEnable );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormLinkDialog::initializeSuggest: caught an exception!" );
        }
    }

    //------------------------------------------------------------------------
    IMPL_LINK( FormLinkDialog, OnSuggest, void*, /*_pNotInterestedIn*/ )
    {
        initializeFieldRowsFrom( m_aRelationDetailColumns, m_aRelationMasterColumns );
        return 0L;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( FormLinkDialog, OnFieldChanged, FieldLinkRow*, /*_pRow*/ )
    {
        updateOkButton();
        return 0L;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( FormLinkDialog, OnInitialize, void*, /*_pNotInterestedIn*/ )
    {
        initializeColumnLabels();
        initializeFieldLists();
        initializeLinks();
        initializeSuggest();
        return 0L;
    }
//............................................................................
}   // namespace pcr
//............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
