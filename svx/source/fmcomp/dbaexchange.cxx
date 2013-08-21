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

#include <svx/dbaexchange.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include "fmprop.hrc"
#include <comphelper/extract.hxx>
#include <sot/formats.hxx>
#include <sot/exchange.hxx>
#include <comphelper/propertysetinfo.hxx>

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::datatransfer;
    using namespace ::comphelper;

    //====================================================================
    //= OColumnTransferable
    //====================================================================
    //--------------------------------------------------------------------
    OColumnTransferable::OColumnTransferable(const OUString& _rDatasource
                                            ,const OUString& _rConnectionResource
                                            ,const sal_Int32        _nCommandType
                                            ,const OUString& _rCommand
                                            ,const OUString& _rFieldName
                                            ,sal_Int32  _nFormats)
        :m_nFormatFlags(_nFormats)
    {
        implConstruct(_rDatasource,_rConnectionResource,_nCommandType, _rCommand, _rFieldName);
    }

    //--------------------------------------------------------------------
    OColumnTransferable::OColumnTransferable(const ODataAccessDescriptor& _rDescriptor, sal_Int32 _nFormats )
        :m_nFormatFlags(_nFormats)
    {
        OUString sDataSource, sDatabaseLocation, sConnectionResource, sCommand, sFieldName;
        if ( _rDescriptor.has( daDataSource ) )         _rDescriptor[ daDataSource ] >>= sDataSource;
        if ( _rDescriptor.has( daDatabaseLocation ) )   _rDescriptor[ daDatabaseLocation ] >>= sDatabaseLocation;
        if ( _rDescriptor.has( daConnectionResource ) ) _rDescriptor[ daConnectionResource ] >>= sConnectionResource;
        if ( _rDescriptor.has( daCommand ) )            _rDescriptor[ daCommand ] >>= sCommand;
        if ( _rDescriptor.has( daColumnName ) )         _rDescriptor[ daColumnName ] >>= sFieldName;

        sal_Int32 nCommandType = CommandType::TABLE;
        OSL_VERIFY( _rDescriptor[ daCommandType ] >>= nCommandType );


        implConstruct(
            sDataSource.isEmpty() ? sDatabaseLocation : sDataSource,
            sConnectionResource, nCommandType, sCommand, sFieldName );

        if ( m_nFormatFlags & CTF_COLUMN_DESCRIPTOR )
        {
            if ( _rDescriptor.has( daConnection ) )
                m_aDescriptor[ daConnection ] = _rDescriptor[ daConnection ];
            if ( _rDescriptor.has( daColumnObject ) )
                m_aDescriptor[ daColumnObject ] = _rDescriptor[ daColumnObject ];
        }
    }

    //--------------------------------------------------------------------
    OColumnTransferable::OColumnTransferable(const Reference< XPropertySet >& _rxForm,
            const OUString& _rFieldName, const Reference< XPropertySet >& _rxColumn,
            const Reference< XConnection >& _rxConnection, sal_Int32 _nFormats)
        :m_nFormatFlags(_nFormats)
    {
        OSL_ENSURE(_rxForm.is(), "OColumnTransferable::OColumnTransferable: invalid form!");
        // collect the necessary information from the form
        OUString sCommand;
        sal_Int32       nCommandType = CommandType::TABLE;
        OUString sDatasource,sURL;

        sal_Bool        bTryToParse = sal_True;
        try
        {
            _rxForm->getPropertyValue(FM_PROP_COMMANDTYPE)  >>= nCommandType;
            _rxForm->getPropertyValue(FM_PROP_COMMAND)      >>= sCommand;
            _rxForm->getPropertyValue(FM_PROP_DATASOURCE)   >>= sDatasource;
            _rxForm->getPropertyValue(FM_PROP_URL)          >>= sURL;
            bTryToParse = ::cppu::any2bool(_rxForm->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));
        }
        catch(Exception&)
        {
            OSL_FAIL("OColumnTransferable::OColumnTransferable: could not collect essential data source attributes !");
        }

        // If the data source is an SQL-statement and simple enough (means "select <field list> from <table> where ....")
        // we are able to fake the drag information we are about to create.
        if (bTryToParse && (CommandType::COMMAND == nCommandType))
        {
            try
            {
                Reference< XTablesSupplier > xSupTab;
                _rxForm->getPropertyValue("SingleSelectQueryComposer") >>= xSupTab;

                if(xSupTab.is())
                {
                    Reference< XNameAccess > xNames = xSupTab->getTables();
                    if (xNames.is())
                    {
                        Sequence< OUString > aTables = xNames->getElementNames();
                        if (1 == aTables.getLength())
                        {
                            sCommand        = aTables[0];
                            nCommandType    = CommandType::TABLE;
                        }
                    }
                }
            }
            catch(Exception&)
            {
                OSL_FAIL("OColumnTransferable::OColumnTransferable: could not collect essential data source attributes (part two) !");
            }
        }

        implConstruct(sDatasource, sURL,nCommandType, sCommand, _rFieldName);

        if ((m_nFormatFlags & CTF_COLUMN_DESCRIPTOR) == CTF_COLUMN_DESCRIPTOR)
        {
            if (_rxColumn.is())
                m_aDescriptor[daColumnObject] <<= _rxColumn;
            if (_rxConnection.is())
                m_aDescriptor[daConnection] <<= _rxConnection;
        }
    }

    //--------------------------------------------------------------------
    sal_uInt32 OColumnTransferable::getDescriptorFormatId()
    {
        static sal_uInt32 s_nFormat = (sal_uInt32)-1;
        if ((sal_uInt32)-1 == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(OUString("application/x-openoffice;windows_formatname=\"dbaccess.ColumnDescriptorTransfer\""));
            OSL_ENSURE((sal_uInt32)-1 != s_nFormat, "OColumnTransferable::getDescriptorFormatId: bad exchange id!");
        }
        return s_nFormat;
    }

    //--------------------------------------------------------------------
    void OColumnTransferable::implConstruct( const OUString& _rDatasource
                                            ,const OUString& _rConnectionResource
                                            ,const sal_Int32 _nCommandType
                                            ,const OUString& _rCommand
                                            , const OUString& _rFieldName)
    {
        const sal_Unicode       cSeparator = sal_Unicode(11);
        const OUString   sSeparator(&cSeparator, 1);

        m_sCompatibleFormat = OUString();
        m_sCompatibleFormat += _rDatasource;
        m_sCompatibleFormat += sSeparator;
        m_sCompatibleFormat += _rCommand;
        m_sCompatibleFormat += sSeparator;

        sal_Unicode cCommandType;
        switch (_nCommandType)
        {
            case CommandType::TABLE:
                cCommandType = '0';
                break;
            case CommandType::QUERY:
                cCommandType = '1';
                break;
            default:
                cCommandType = '2';
                break;
        }
        m_sCompatibleFormat += OUString(&cCommandType, 1);
        m_sCompatibleFormat += sSeparator;
        m_sCompatibleFormat += _rFieldName;

        m_aDescriptor.clear();
        if ((m_nFormatFlags & CTF_COLUMN_DESCRIPTOR) == CTF_COLUMN_DESCRIPTOR)
        {
            m_aDescriptor.setDataSource(_rDatasource);
            if ( !_rConnectionResource.isEmpty() )
                m_aDescriptor[daConnectionResource] <<= _rConnectionResource;

            m_aDescriptor[daCommand]        <<= _rCommand;
            m_aDescriptor[daCommandType]    <<= _nCommandType;
            m_aDescriptor[daColumnName]     <<= _rFieldName;
        }
    }

    //--------------------------------------------------------------------
    void OColumnTransferable::AddSupportedFormats()
    {
        if (CTF_CONTROL_EXCHANGE & m_nFormatFlags)
            AddFormat(SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE);

        if (CTF_FIELD_DESCRIPTOR & m_nFormatFlags)
            AddFormat(SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE);

        if (CTF_COLUMN_DESCRIPTOR & m_nFormatFlags)
            AddFormat(getDescriptorFormatId());
    }

    //--------------------------------------------------------------------
    sal_Bool OColumnTransferable::GetData( const DataFlavor& _rFlavor )
    {
        const sal_uInt32 nFormatId = SotExchange::GetFormat(_rFlavor);
        switch (nFormatId)
        {
            case SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE:
            case SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE:
                return SetString(m_sCompatibleFormat, _rFlavor);
        }
        if (nFormatId == getDescriptorFormatId())
            return SetAny( makeAny( m_aDescriptor.createPropertyValueSequence() ), _rFlavor );

        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool OColumnTransferable::canExtractColumnDescriptor(const DataFlavorExVector& _rFlavors, sal_Int32 _nFormats)
    {
        sal_Bool bFieldFormat       = 0 != (_nFormats & CTF_FIELD_DESCRIPTOR);
        sal_Bool bControlFormat     = 0 != (_nFormats & CTF_CONTROL_EXCHANGE);
        sal_Bool bDescriptorFormat  = 0 != (_nFormats & CTF_COLUMN_DESCRIPTOR);
        for (   DataFlavorExVector::const_iterator aCheck = _rFlavors.begin();
                aCheck != _rFlavors.end();
                ++aCheck
            )
        {
            if (bFieldFormat && (SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE == aCheck->mnSotId))
                return sal_True;
            if (bControlFormat && (SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE == aCheck->mnSotId))
                return sal_True;
            if (bDescriptorFormat && (getDescriptorFormatId() == aCheck->mnSotId))
                return sal_True;
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor OColumnTransferable::extractColumnDescriptor(const TransferableDataHelper& _rData)
    {
        if (_rData.HasFormat(getDescriptorFormatId()))
        {
            // the object has a real descriptor object (not just the old compatible format)

            // extract the any from the transferable
            DataFlavor aFlavor;
#if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
#endif
            SotExchange::GetFormatDataFlavor(getDescriptorFormatId(), aFlavor);
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid data format (no flavor)!");

            Any aDescriptor = _rData.GetAny(aFlavor);

            // extract the property value sequence
            Sequence< PropertyValue > aDescriptorProps;
#if OSL_DEBUG_LEVEL > 0
            bSuccess =
#endif
            aDescriptor >>= aDescriptorProps;
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid clipboard format!");

            // build the real descriptor
            return ODataAccessDescriptor(aDescriptorProps);
        }

        // only the old (compatible) format exists -> use the other extract method ...
        OUString sDatasource, sCommand, sFieldName,sDatabaseLocation,sConnectionResource;
        sal_Int32 nCommandType = CommandType::COMMAND;

        ODataAccessDescriptor aDescriptor;
        if (extractColumnDescriptor(_rData, sDatasource, sDatabaseLocation,sConnectionResource,nCommandType, sCommand, sFieldName))
        {
            // and build an own descriptor
            if ( !sDatasource.isEmpty() )
                aDescriptor[daDataSource]   <<= sDatasource;
            if ( !sDatabaseLocation.isEmpty() )
                aDescriptor[daDatabaseLocation] <<= sDatabaseLocation;
            if ( !sConnectionResource.isEmpty() )
                aDescriptor[daConnectionResource]   <<= sConnectionResource;

            aDescriptor[daCommand]      <<= sCommand;
            aDescriptor[daCommandType]  <<= nCommandType;
            aDescriptor[daColumnName]   <<= sFieldName;
        }
        return aDescriptor;
    }

    //--------------------------------------------------------------------
    sal_Bool OColumnTransferable::extractColumnDescriptor(const TransferableDataHelper& _rData
                                            ,OUString& _rDatasource
                                            ,OUString& _rDatabaseLocation
                                            ,OUString& _rConnectionResource
                                            ,sal_Int32& _nCommandType
                                            ,OUString& _rCommand
                                            ,OUString& _rFieldName)
    {
        if ( _rData.HasFormat(getDescriptorFormatId()) )
        {
            ODataAccessDescriptor aDescriptor = extractColumnDescriptor(_rData);
            if ( aDescriptor.has(daDataSource) )
                aDescriptor[daDataSource]           >>= _rDatasource;
            if ( aDescriptor.has(daDatabaseLocation) )
                aDescriptor[daDatabaseLocation]     >>= _rDatabaseLocation;
            if ( aDescriptor.has(daConnectionResource) )
                aDescriptor[daConnectionResource]   >>= _rConnectionResource;

            aDescriptor[daCommand]              >>= _rCommand;
            aDescriptor[daCommandType]          >>= _nCommandType;
            aDescriptor[daColumnName]           >>= _rFieldName;
            return sal_True;
        }

        // check if we have a (string) format we can use ....
        SotFormatStringId   nRecognizedFormat = 0;
        if (_rData.HasFormat(SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE))
            nRecognizedFormat = SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE;
        if (_rData.HasFormat(SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE))
            nRecognizedFormat = SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE;
        if (!nRecognizedFormat)
            return sal_False;

        OUString sFieldDescription;
        const_cast<TransferableDataHelper&>(_rData).GetString(nRecognizedFormat, sFieldDescription);

        const sal_Unicode cSeparator = sal_Unicode(11);
        _rDatasource    = sFieldDescription.getToken(0, cSeparator);
        _rCommand       = sFieldDescription.getToken(1, cSeparator);
        _nCommandType   = sFieldDescription.getToken(2, cSeparator).toInt32();
        _rFieldName     = sFieldDescription.getToken(3, cSeparator);

        return sal_True;
    }

    //--------------------------------------------------------------------
    void OColumnTransferable::addDataToContainer( TransferDataContainer* _pContainer )
    {
        OSL_ENSURE( _pContainer, "OColumnTransferable::addDataToContainer: invalid container!" );
        if ( _pContainer )
        {
            if ( m_nFormatFlags & CTF_FIELD_DESCRIPTOR )
                _pContainer->CopyAny( SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE, makeAny( m_sCompatibleFormat ) );

            if ( m_nFormatFlags & CTF_CONTROL_EXCHANGE )
                _pContainer->CopyAny( SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE, makeAny( m_sCompatibleFormat ) );

            if ( m_nFormatFlags & CTF_COLUMN_DESCRIPTOR )
            {
                Any aContent = makeAny( m_aDescriptor.createPropertyValueSequence() );
                _pContainer->CopyAny(
                    sal::static_int_cast< sal_uInt16 >( getDescriptorFormatId() ),
                    aContent );
            }
        }
    }

    //====================================================================
    //= ODataAccessObjectTransferable
    //====================================================================
    ODataAccessObjectTransferable::ODataAccessObjectTransferable(
            const OUString&  _rDatasource
            ,const OUString& _rConnectionResource
            ,const sal_Int32        _nCommandType
            ,const OUString& _rCommand
        )
    {
        construct(_rDatasource,_rConnectionResource,_nCommandType,_rCommand,NULL,(CommandType::COMMAND == _nCommandType),_rCommand);
    }
    //--------------------------------------------------------------------
    ODataAccessObjectTransferable::ODataAccessObjectTransferable(
                    const OUString&  _rDatasource
                    ,const OUString& _rConnectionResource
                    ,const sal_Int32        _nCommandType
                    ,const OUString& _rCommand
                    ,const Reference< XConnection >& _rxConnection)
    {
        OSL_ENSURE(_rxConnection.is(),"Wrong ctor used.!");
        construct(_rDatasource,_rConnectionResource,_nCommandType,_rCommand,_rxConnection,(CommandType::COMMAND == _nCommandType),_rCommand);
    }

    // -----------------------------------------------------------------------------
    ODataAccessObjectTransferable::ODataAccessObjectTransferable(const Reference< XPropertySet >& _rxLivingForm)
    {
        // collect some properties of the form
        OUString sDatasourceName,sConnectionResource;
        sal_Int32       nObjectType = CommandType::COMMAND;
        OUString sObjectName;
        Reference< XConnection > xConnection;
        try
        {
            _rxLivingForm->getPropertyValue(FM_PROP_COMMANDTYPE) >>= nObjectType;
            _rxLivingForm->getPropertyValue(FM_PROP_COMMAND) >>= sObjectName;
            _rxLivingForm->getPropertyValue(FM_PROP_DATASOURCE) >>= sDatasourceName;
            _rxLivingForm->getPropertyValue(FM_PROP_URL) >>= sConnectionResource;
            _rxLivingForm->getPropertyValue(FM_PROP_ACTIVE_CONNECTION) >>= xConnection;
        }
        catch(Exception&)
        {
            OSL_FAIL("ODataAccessObjectTransferable::ODataAccessObjectTransferable: could not collect essential form attributes !");
            return;
        }

        // check if the SQL-statement is modified
        OUString sCompleteStatement;
        try
        {
            _rxLivingForm->getPropertyValue(FM_PROP_ACTIVECOMMAND) >>= sCompleteStatement;
        }
        catch (const Exception&)
        {
            OSL_FAIL("ODataAccessObjectTransferable::ODataAccessObjectTransferable: could not collect essential form attributes (part two) !");
            return;
        }

        construct(  sDatasourceName
                    ,sConnectionResource
                    ,nObjectType
                    ,sObjectName,xConnection
                    ,!((CommandType::QUERY == nObjectType))
                    ,sCompleteStatement);
    }

    // -----------------------------------------------------------------------------
    void ODataAccessObjectTransferable::AddSupportedFormats()
    {
        sal_Int32 nObjectType = CommandType::COMMAND;
        m_aDescriptor[daCommandType] >>= nObjectType;
        switch (nObjectType)
        {
            case CommandType::TABLE:
                AddFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE);
                break;
            case CommandType::QUERY:
                AddFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY);
                break;
            case CommandType::COMMAND:
                AddFormat(SOT_FORMATSTR_ID_DBACCESS_COMMAND);
                break;
        }

        sal_Int32 nDescriptorLen = m_sCompatibleObjectDescription.getLength();
        if (nDescriptorLen)
        {
            if (m_sCompatibleObjectDescription.getStr()[nDescriptorLen] == 11)
                m_sCompatibleObjectDescription = m_sCompatibleObjectDescription.copy(0, nDescriptorLen - 1);

            if (nDescriptorLen)
                AddFormat(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE);
        }
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODataAccessObjectTransferable::GetData( const DataFlavor& rFlavor )
    {
        sal_uIntPtr nFormat = SotExchange::GetFormat(rFlavor);
        switch (nFormat)
        {
            case SOT_FORMATSTR_ID_DBACCESS_TABLE:
            case SOT_FORMATSTR_ID_DBACCESS_QUERY:
            case SOT_FORMATSTR_ID_DBACCESS_COMMAND:
                return SetAny( makeAny(m_aDescriptor.createPropertyValueSequence()), rFlavor );

            case SOT_FORMATSTR_ID_SBA_DATAEXCHANGE:
                return SetString(m_sCompatibleObjectDescription, rFlavor);
        }
        return sal_False;
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODataAccessObjectTransferable::canExtractObjectDescriptor(const DataFlavorExVector& _rFlavors)
    {
        for (   DataFlavorExVector::const_iterator aCheck = _rFlavors.begin();
                aCheck != _rFlavors.end();
                ++aCheck
            )
        {
            if (SOT_FORMATSTR_ID_DBACCESS_TABLE == aCheck->mnSotId)
                return sal_True;
            if (SOT_FORMATSTR_ID_DBACCESS_QUERY == aCheck->mnSotId)
                return sal_True;
            if (SOT_FORMATSTR_ID_DBACCESS_COMMAND == aCheck->mnSotId)
                return sal_True;
        }
        return sal_False;
    }

    // -----------------------------------------------------------------------------
    ODataAccessDescriptor ODataAccessObjectTransferable::extractObjectDescriptor(const TransferableDataHelper& _rData)
    {
        sal_Int32 nKnownFormatId = 0;
        if ( _rData.HasFormat( SOT_FORMATSTR_ID_DBACCESS_TABLE ) )
            nKnownFormatId = SOT_FORMATSTR_ID_DBACCESS_TABLE;
        if ( _rData.HasFormat( SOT_FORMATSTR_ID_DBACCESS_QUERY ) )
            nKnownFormatId = SOT_FORMATSTR_ID_DBACCESS_QUERY;
        if ( _rData.HasFormat( SOT_FORMATSTR_ID_DBACCESS_COMMAND ) )
            nKnownFormatId = SOT_FORMATSTR_ID_DBACCESS_COMMAND;

        if (0 != nKnownFormatId)
        {
            // extract the any from the transferable
            DataFlavor aFlavor;
#if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
#endif
            SotExchange::GetFormatDataFlavor(nKnownFormatId, aFlavor);
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid data format (no flavor)!");

            Any aDescriptor = _rData.GetAny(aFlavor);

            // extract the property value sequence
            Sequence< PropertyValue > aDescriptorProps;
#if OSL_DEBUG_LEVEL > 0
            bSuccess =
#endif
            aDescriptor >>= aDescriptorProps;
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid clipboard format!");

            // build the real descriptor
            return ODataAccessDescriptor(aDescriptorProps);
        }

        OSL_FAIL( "OColumnTransferable::extractColumnDescriptor: unsupported formats only!" );
        return ODataAccessDescriptor();
    }

    // -----------------------------------------------------------------------------
    void ODataAccessObjectTransferable::addCompatibleSelectionDescription( const Sequence< Any >& _rSelRows )
    {
        const sal_Unicode       cSeparator(11);
        const OUString   sSeparator(&cSeparator, 1);

        const Any* pSelRows = _rSelRows.getConstArray();
        const Any* pSelRowsEnd = pSelRows + _rSelRows.getLength();
        for ( ; pSelRows < pSelRowsEnd; ++pSelRows )
        {
            sal_Int32 nSelectedRow( 0 );
            OSL_VERIFY( *pSelRows >>= nSelectedRow );

            m_sCompatibleObjectDescription += OUString::number(nSelectedRow);
            m_sCompatibleObjectDescription += sSeparator;
        }
    }

    // -----------------------------------------------------------------------------
    void ODataAccessObjectTransferable::ObjectReleased()
    {
        m_aDescriptor.clear();
    }
    // -----------------------------------------------------------------------------
    void ODataAccessObjectTransferable::construct(  const OUString&  _rDatasource
                                                    ,const OUString& _rConnectionResource
                                                    ,const sal_Int32        _nCommandType
                                                    ,const OUString& _rCommand
                                                    ,const Reference< XConnection >& _rxConnection
                                                    ,sal_Bool _bAddCommand
                                                    ,const OUString& _sActiveCommand)
    {
        m_aDescriptor.setDataSource(_rDatasource);
        // build the descriptor (the property sequence)
        if ( !_rConnectionResource.isEmpty() )
            m_aDescriptor[daConnectionResource] <<= _rConnectionResource;
        if ( _rxConnection.is() )
            m_aDescriptor[daConnection]     <<= _rxConnection;
        m_aDescriptor[daCommand]        <<= _rCommand;
        m_aDescriptor[daCommandType]    <<= _nCommandType;

        // extract the single values from the sequence

        OUString sObjectName;
        OUString sDatasourceName = _rDatasource;
        sObjectName = _rCommand;

        // for compatibility: create a string which can be used for the SOT_FORMATSTR_ID_SBA_DATAEXCHANGE format

        sal_Bool bTreatAsStatement = (CommandType::COMMAND == _nCommandType);
            // statements are - in this old and ugly format - described as queries

        const sal_Unicode       cSeparator = sal_Unicode(11);
        const OUString   sSeparator(&cSeparator, 1);

        const sal_Unicode       cTableMark = '1';
        const sal_Unicode       cQueryMark = '0';

        // build the descriptor string
        m_sCompatibleObjectDescription += sDatasourceName;
        m_sCompatibleObjectDescription += sSeparator;
        m_sCompatibleObjectDescription += bTreatAsStatement ? OUString() : sObjectName;
        m_sCompatibleObjectDescription += sSeparator;
        switch (_nCommandType)
        {
            case CommandType::TABLE:
                m_sCompatibleObjectDescription += OUString(&cTableMark, 1);
                break;
            case CommandType::QUERY:
                m_sCompatibleObjectDescription += OUString(&cQueryMark, 1);
                break;
            case CommandType::COMMAND:
                m_sCompatibleObjectDescription += OUString(&cQueryMark, 1);
                // think of it as a query
                break;
        }
        m_sCompatibleObjectDescription += sSeparator;
        m_sCompatibleObjectDescription += _bAddCommand ? _sActiveCommand : OUString();
        m_sCompatibleObjectDescription += sSeparator;
    }

    //--------------------------------------------------------------------
    OMultiColumnTransferable::OMultiColumnTransferable(const Sequence< PropertyValue >& _aDescriptors) : m_aDescriptors(_aDescriptors)
    {
    }
    //--------------------------------------------------------------------
    sal_uInt32 OMultiColumnTransferable::getDescriptorFormatId()
    {
        static sal_uInt32 s_nFormat = (sal_uInt32)-1;
        if ((sal_uInt32)-1 == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(OUString("application/x-openoffice;windows_formatname=\"dbaccess.MultipleColumnDescriptorTransfer\""));
            OSL_ENSURE((sal_uInt32)-1 != s_nFormat, "OColumnTransferable::getDescriptorFormatId: bad exchange id!");
        }
        return s_nFormat;
    }
    //--------------------------------------------------------------------
    void OMultiColumnTransferable::AddSupportedFormats()
    {
        AddFormat(getDescriptorFormatId());
    }
    //--------------------------------------------------------------------
    sal_Bool OMultiColumnTransferable::GetData( const DataFlavor& _rFlavor )
    {
        const sal_uInt32 nFormatId = SotExchange::GetFormat(_rFlavor);
        if (nFormatId == getDescriptorFormatId())
        {
            return SetAny( makeAny( m_aDescriptors ), _rFlavor );
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool OMultiColumnTransferable::canExtractDescriptor(const DataFlavorExVector& _rFlavors)
    {
        DataFlavorExVector::const_iterator aCheck = _rFlavors.begin();
        for (   ;
                aCheck != _rFlavors.end() && getDescriptorFormatId() == aCheck->mnSotId;
                ++aCheck
            )
            ;

        return aCheck == _rFlavors.end();
    }

    //--------------------------------------------------------------------
    Sequence< PropertyValue > OMultiColumnTransferable::extractDescriptor(const TransferableDataHelper& _rData)
    {
        Sequence< PropertyValue > aList;
        if (_rData.HasFormat(getDescriptorFormatId()))
        {
            // extract the any from the transferable
            DataFlavor aFlavor;
#if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
#endif
            SotExchange::GetFormatDataFlavor(getDescriptorFormatId(), aFlavor);
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid data format (no flavor)!");

            _rData.GetAny(aFlavor) >>= aList;
        } // if (_rData.HasFormat(getDescriptorFormatId()))
        return aList;
    }
    // -----------------------------------------------------------------------------
    void OMultiColumnTransferable::ObjectReleased()
    {
        m_aDescriptors.realloc(0);
    }

//........................................................................
}   // namespace svx
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
