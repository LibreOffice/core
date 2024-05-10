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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <fmprop.hxx>
#include <comphelper/extract.hxx>
#include <sot/formats.hxx>
#include <sot/exchange.hxx>
#include <o3tl/string_view.hxx>


namespace svx
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::datatransfer;

    OColumnTransferable::OColumnTransferable(ColumnTransferFormatFlags nFormats)
        : m_nFormatFlags(nFormats)
    {
    }

    void OColumnTransferable::setDescriptor(const ODataAccessDescriptor& rDescriptor)
    {
        ClearFormats();

        OUString sDataSource, sDatabaseLocation, sConnectionResource, sCommand, sFieldName;
        if ( rDescriptor.has( DataAccessDescriptorProperty::DataSource ) )         rDescriptor[ DataAccessDescriptorProperty::DataSource ] >>= sDataSource;
        if ( rDescriptor.has( DataAccessDescriptorProperty::DatabaseLocation ) )   rDescriptor[ DataAccessDescriptorProperty::DatabaseLocation ] >>= sDatabaseLocation;
        if ( rDescriptor.has( DataAccessDescriptorProperty::ConnectionResource ) ) rDescriptor[ DataAccessDescriptorProperty::ConnectionResource ] >>= sConnectionResource;
        if ( rDescriptor.has( DataAccessDescriptorProperty::Command ) )            rDescriptor[ DataAccessDescriptorProperty::Command ] >>= sCommand;
        if ( rDescriptor.has( DataAccessDescriptorProperty::ColumnName ) )         rDescriptor[ DataAccessDescriptorProperty::ColumnName ] >>= sFieldName;

        sal_Int32 nCommandType = CommandType::TABLE;
        OSL_VERIFY( rDescriptor[ DataAccessDescriptorProperty::CommandType ] >>= nCommandType );

        implConstruct(
            sDataSource.isEmpty() ? sDatabaseLocation : sDataSource,
            sConnectionResource, nCommandType, sCommand, sFieldName );

        if ( m_nFormatFlags & ColumnTransferFormatFlags::COLUMN_DESCRIPTOR )
        {
            if ( rDescriptor.has( DataAccessDescriptorProperty::Connection ) )
                m_aDescriptor[ DataAccessDescriptorProperty::Connection ] = rDescriptor[ DataAccessDescriptorProperty::Connection ];
            if ( rDescriptor.has( DataAccessDescriptorProperty::ColumnObject ) )
                m_aDescriptor[ DataAccessDescriptorProperty::ColumnObject ] = rDescriptor[ DataAccessDescriptorProperty::ColumnObject ];
        }
    }

    OColumnTransferable::OColumnTransferable(const Reference< XPropertySet >& _rxForm,
            const OUString& _rFieldName, const Reference< XPropertySet >& _rxColumn,
            const Reference< XConnection >& _rxConnection, ColumnTransferFormatFlags _nFormats)
        :m_nFormatFlags(_nFormats)
    {
        OSL_ENSURE(_rxForm.is(), "OColumnTransferable::OColumnTransferable: invalid form!");
        // collect the necessary information from the form
        OUString sCommand;
        sal_Int32       nCommandType = CommandType::TABLE;
        OUString sDatasource,sURL;

        bool        bTryToParse = true;
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

        // If the data source is an SQL-statement and simple enough (means "select <field list> from <table> where...")
        // we are able to fake the drag information we are about to create.
        if (bTryToParse && (CommandType::COMMAND == nCommandType))
        {
            try
            {
                Reference< XTablesSupplier > xSupTab;
                _rxForm->getPropertyValue(u"SingleSelectQueryComposer"_ustr) >>= xSupTab;

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

        if ((m_nFormatFlags & ColumnTransferFormatFlags::COLUMN_DESCRIPTOR) == ColumnTransferFormatFlags::COLUMN_DESCRIPTOR)
        {
            if (_rxColumn.is())
                m_aDescriptor[DataAccessDescriptorProperty::ColumnObject] <<= _rxColumn;
            if (_rxConnection.is())
                m_aDescriptor[DataAccessDescriptorProperty::Connection] <<= _rxConnection;
        }
    }


    SotClipboardFormatId OColumnTransferable::getDescriptorFormatId()
    {
        static SotClipboardFormatId s_nFormat = static_cast<SotClipboardFormatId>(-1);
        if (static_cast<SotClipboardFormatId>(-1) == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(u"application/x-openoffice;windows_formatname=\"dbaccess.ColumnDescriptorTransfer\""_ustr);
            OSL_ENSURE(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OColumnTransferable::getDescriptorFormatId: bad exchange id!");
        }
        return s_nFormat;
    }


    void OColumnTransferable::implConstruct( const OUString& _rDatasource
                                            ,const OUString& _rConnectionResource
                                            ,const sal_Int32 _nCommandType
                                            ,const OUString& _rCommand
                                            , const OUString& _rFieldName)
    {
        const sal_Unicode       cSeparator = u'\x000B';
        const OUString   sSeparator(&cSeparator, 1);

        m_sCompatibleFormat.clear();
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
        m_sCompatibleFormat += OUStringChar(cCommandType);
        m_sCompatibleFormat += sSeparator;
        m_sCompatibleFormat += _rFieldName;

        m_aDescriptor.clear();
        if ((m_nFormatFlags & ColumnTransferFormatFlags::COLUMN_DESCRIPTOR) == ColumnTransferFormatFlags::COLUMN_DESCRIPTOR)
        {
            m_aDescriptor.setDataSource(_rDatasource);
            if ( !_rConnectionResource.isEmpty() )
                m_aDescriptor[DataAccessDescriptorProperty::ConnectionResource] <<= _rConnectionResource;

            m_aDescriptor[DataAccessDescriptorProperty::Command]        <<= _rCommand;
            m_aDescriptor[DataAccessDescriptorProperty::CommandType]    <<= _nCommandType;
            m_aDescriptor[DataAccessDescriptorProperty::ColumnName]     <<= _rFieldName;
        }
    }


    void OColumnTransferable::AddSupportedFormats()
    {
        if (ColumnTransferFormatFlags::CONTROL_EXCHANGE & m_nFormatFlags)
            AddFormat(SotClipboardFormatId::SBA_CTRLDATAEXCHANGE);

        if (ColumnTransferFormatFlags::FIELD_DESCRIPTOR & m_nFormatFlags)
            AddFormat(SotClipboardFormatId::SBA_FIELDDATAEXCHANGE);

        if (ColumnTransferFormatFlags::COLUMN_DESCRIPTOR & m_nFormatFlags)
            AddFormat(getDescriptorFormatId());
    }


    bool OColumnTransferable::GetData( const DataFlavor& _rFlavor, const OUString& /*rDestDoc*/ )
    {
        const SotClipboardFormatId nFormatId = SotExchange::GetFormat(_rFlavor);
        switch (nFormatId)
        {
            case SotClipboardFormatId::SBA_FIELDDATAEXCHANGE:
            case SotClipboardFormatId::SBA_CTRLDATAEXCHANGE:
                return SetString(m_sCompatibleFormat);
            default: break;
        }
        if (nFormatId == getDescriptorFormatId())
            return SetAny( Any( m_aDescriptor.createPropertyValueSequence() ) );

        return false;
    }


    bool OColumnTransferable::canExtractColumnDescriptor(const DataFlavorExVector& _rFlavors, ColumnTransferFormatFlags _nFormats)
    {
        bool bFieldFormat       = bool(_nFormats & ColumnTransferFormatFlags::FIELD_DESCRIPTOR);
        bool bControlFormat     = bool(_nFormats & ColumnTransferFormatFlags::CONTROL_EXCHANGE);
        bool bDescriptorFormat  = bool(_nFormats & ColumnTransferFormatFlags::COLUMN_DESCRIPTOR);
        SotClipboardFormatId nFormatId = getDescriptorFormatId();
        return std::any_of(_rFlavors.begin(), _rFlavors.end(),
            [&](const DataFlavorEx& rCheck) {
                return (bFieldFormat && (SotClipboardFormatId::SBA_FIELDDATAEXCHANGE == rCheck.mnSotId))
                    || (bControlFormat && (SotClipboardFormatId::SBA_CTRLDATAEXCHANGE == rCheck.mnSotId))
                    || (bDescriptorFormat && (nFormatId == rCheck.mnSotId));
            });
    }


    ODataAccessDescriptor OColumnTransferable::extractColumnDescriptor(const TransferableDataHelper& _rData)
    {
        if (_rData.HasFormat(getDescriptorFormatId()))
        {
            // the object has a real descriptor object (not just the old compatible format)

            // extract the any from the transferable
            DataFlavor aFlavor;
            bool bSuccess =
                SotExchange::GetFormatDataFlavor(getDescriptorFormatId(), aFlavor);
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid data format (no flavor)!");

            Any aDescriptor = _rData.GetAny(aFlavor, OUString());

            // extract the property value sequence
            Sequence< PropertyValue > aDescriptorProps;
            bSuccess = aDescriptor >>= aDescriptorProps;
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
                aDescriptor[DataAccessDescriptorProperty::DataSource]   <<= sDatasource;
            if ( !sDatabaseLocation.isEmpty() )
                aDescriptor[DataAccessDescriptorProperty::DatabaseLocation] <<= sDatabaseLocation;
            if ( !sConnectionResource.isEmpty() )
                aDescriptor[DataAccessDescriptorProperty::ConnectionResource]   <<= sConnectionResource;

            aDescriptor[DataAccessDescriptorProperty::Command]      <<= sCommand;
            aDescriptor[DataAccessDescriptorProperty::CommandType]  <<= nCommandType;
            aDescriptor[DataAccessDescriptorProperty::ColumnName]   <<= sFieldName;
        }
        return aDescriptor;
    }


    bool OColumnTransferable::extractColumnDescriptor(const TransferableDataHelper& _rData
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
            if ( aDescriptor.has(DataAccessDescriptorProperty::DataSource) )
                aDescriptor[DataAccessDescriptorProperty::DataSource]           >>= _rDatasource;
            if ( aDescriptor.has(DataAccessDescriptorProperty::DatabaseLocation) )
                aDescriptor[DataAccessDescriptorProperty::DatabaseLocation]     >>= _rDatabaseLocation;
            if ( aDescriptor.has(DataAccessDescriptorProperty::ConnectionResource) )
                aDescriptor[DataAccessDescriptorProperty::ConnectionResource]   >>= _rConnectionResource;

            aDescriptor[DataAccessDescriptorProperty::Command]              >>= _rCommand;
            aDescriptor[DataAccessDescriptorProperty::CommandType]          >>= _nCommandType;
            aDescriptor[DataAccessDescriptorProperty::ColumnName]           >>= _rFieldName;
            return true;
        }

        // check if we have a (string) format we can use...
        SotClipboardFormatId   nRecognizedFormat = SotClipboardFormatId::NONE;
        if (_rData.HasFormat(SotClipboardFormatId::SBA_FIELDDATAEXCHANGE))
            nRecognizedFormat = SotClipboardFormatId::SBA_FIELDDATAEXCHANGE;
        if (_rData.HasFormat(SotClipboardFormatId::SBA_CTRLDATAEXCHANGE))
            nRecognizedFormat = SotClipboardFormatId::SBA_CTRLDATAEXCHANGE;
        if (nRecognizedFormat == SotClipboardFormatId::NONE)
            return false;

        OUString sFieldDescription;
        (void)_rData.GetString(nRecognizedFormat, sFieldDescription);

        const sal_Unicode cSeparator = u'\x000B';
        sal_Int32 nIdx{ 0 };
        _rDatasource    = sFieldDescription.getToken(0, cSeparator, nIdx);
        _rCommand       = sFieldDescription.getToken(0, cSeparator, nIdx);
        _nCommandType   = o3tl::toInt32(o3tl::getToken(sFieldDescription, 0, cSeparator, nIdx));
        _rFieldName     = sFieldDescription.getToken(0, cSeparator, nIdx);

        return true;
    }

    ODataAccessObjectTransferable::ODataAccessObjectTransferable()
    {
    }

    void ODataAccessObjectTransferable::Update(
            const OUString&  _rDatasource,
            const sal_Int32  _nCommandType,
            const OUString& _rCommand)
    {
        construct(_rDatasource,OUString(),_nCommandType,_rCommand,nullptr,(CommandType::COMMAND == _nCommandType),_rCommand);
    }

    void ODataAccessObjectTransferable::Update(
                    const OUString&  _rDatasource,
                    const sal_Int32 _nCommandType,
                    const OUString& _rCommand,
                    const Reference< XConnection >& _rxConnection)
    {
        OSL_ENSURE(_rxConnection.is(), "Wrong Update used.!");
        construct(_rDatasource,OUString(),_nCommandType,_rCommand,_rxConnection,(CommandType::COMMAND == _nCommandType),_rCommand);
    }

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
                    ,CommandType::QUERY != nObjectType
                    ,sCompleteStatement);
    }


    void ODataAccessObjectTransferable::AddSupportedFormats()
    {
        sal_Int32 nObjectType = CommandType::COMMAND;
        m_aDescriptor[DataAccessDescriptorProperty::CommandType] >>= nObjectType;
        switch (nObjectType)
        {
            case CommandType::TABLE:
                AddFormat(SotClipboardFormatId::DBACCESS_TABLE);
                break;
            case CommandType::QUERY:
                AddFormat(SotClipboardFormatId::DBACCESS_QUERY);
                break;
            case CommandType::COMMAND:
                AddFormat(SotClipboardFormatId::DBACCESS_COMMAND);
                break;
        }

        if (!m_sCompatibleObjectDescription.isEmpty())
            AddFormat(SotClipboardFormatId::SBA_DATAEXCHANGE);
    }


    bool ODataAccessObjectTransferable::GetData( const DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
    {
        SotClipboardFormatId nFormat = SotExchange::GetFormat(rFlavor);
        switch (nFormat)
        {
            case SotClipboardFormatId::DBACCESS_TABLE:
            case SotClipboardFormatId::DBACCESS_QUERY:
            case SotClipboardFormatId::DBACCESS_COMMAND:
                return SetAny( Any(m_aDescriptor.createPropertyValueSequence()) );

            case SotClipboardFormatId::SBA_DATAEXCHANGE:
                return SetString(m_sCompatibleObjectDescription);
            default: break;
        }
        return false;
    }


    bool ODataAccessObjectTransferable::canExtractObjectDescriptor(const DataFlavorExVector& _rFlavors)
    {
        return std::any_of(_rFlavors.begin(), _rFlavors.end(),
            [](const DataFlavorEx& rCheck) {
                return SotClipboardFormatId::DBACCESS_TABLE == rCheck.mnSotId
                    || SotClipboardFormatId::DBACCESS_QUERY == rCheck.mnSotId
                    || SotClipboardFormatId::DBACCESS_COMMAND == rCheck.mnSotId;
            });
    }


    ODataAccessDescriptor ODataAccessObjectTransferable::extractObjectDescriptor(const TransferableDataHelper& _rData)
    {
        SotClipboardFormatId nKnownFormatId = SotClipboardFormatId::NONE;
        if ( _rData.HasFormat( SotClipboardFormatId::DBACCESS_TABLE ) )
            nKnownFormatId = SotClipboardFormatId::DBACCESS_TABLE;
        if ( _rData.HasFormat( SotClipboardFormatId::DBACCESS_QUERY ) )
            nKnownFormatId = SotClipboardFormatId::DBACCESS_QUERY;
        if ( _rData.HasFormat( SotClipboardFormatId::DBACCESS_COMMAND ) )
            nKnownFormatId = SotClipboardFormatId::DBACCESS_COMMAND;

        if (SotClipboardFormatId::NONE != nKnownFormatId)
        {
            // extract the any from the transferable
            DataFlavor aFlavor;
            bool bSuccess =
                SotExchange::GetFormatDataFlavor(nKnownFormatId, aFlavor);
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid data format (no flavor)!");

            Any aDescriptor = _rData.GetAny(aFlavor, OUString());

            // extract the property value sequence
            Sequence< PropertyValue > aDescriptorProps;
            bSuccess = aDescriptor >>= aDescriptorProps;
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid clipboard format!");

            // build the real descriptor
            return ODataAccessDescriptor(aDescriptorProps);
        }

        OSL_FAIL( "OColumnTransferable::extractColumnDescriptor: unsupported formats only!" );
        return ODataAccessDescriptor();
    }


    void ODataAccessObjectTransferable::addCompatibleSelectionDescription( const Sequence< Any >& _rSelRows )
    {
        const sal_Unicode       cSeparator(11);
        const OUString   sSeparator(&cSeparator, 1);

        for ( const Any& rSelRow : _rSelRows )
        {
            sal_Int32 nSelectedRow( 0 );
            OSL_VERIFY( rSelRow >>= nSelectedRow );

            m_sCompatibleObjectDescription += OUString::number(nSelectedRow);
            m_sCompatibleObjectDescription += sSeparator;
        }
    }


    void ODataAccessObjectTransferable::ObjectReleased()
    {
        m_aDescriptor.clear();
    }

    void ODataAccessObjectTransferable::construct(  const OUString&  _rDatasource
                                                    ,const OUString& _rConnectionResource
                                                    ,const sal_Int32        _nCommandType
                                                    ,const OUString& _rCommand
                                                    ,const Reference< XConnection >& _rxConnection
                                                    ,bool _bAddCommand
                                                    ,const OUString& _sActiveCommand)
    {
        m_aDescriptor.setDataSource(_rDatasource);
        // build the descriptor (the property sequence)
        if ( !_rConnectionResource.isEmpty() )
            m_aDescriptor[DataAccessDescriptorProperty::ConnectionResource] <<= _rConnectionResource;
        if ( _rxConnection.is() )
            m_aDescriptor[DataAccessDescriptorProperty::Connection]     <<= _rxConnection;
        m_aDescriptor[DataAccessDescriptorProperty::Command]        <<= _rCommand;
        m_aDescriptor[DataAccessDescriptorProperty::CommandType]    <<= _nCommandType;

        // extract the single values from the sequence

        OUString sObjectName = _rCommand;

        // for compatibility: create a string which can be used for the SotClipboardFormatId::SBA_DATAEXCHANGE format

        bool bTreatAsStatement = (CommandType::COMMAND == _nCommandType);
            // statements are - in this old and ugly format - described as queries

        const sal_Unicode       cSeparator = u'\x000B';
        const OUString   sSeparator(&cSeparator, 1);

        const sal_Unicode       cTableMark = '1';
        const sal_Unicode       cQueryMark = '0';

        // build the descriptor string
        m_sCompatibleObjectDescription += _rDatasource;
        m_sCompatibleObjectDescription += sSeparator;
        m_sCompatibleObjectDescription += bTreatAsStatement ? OUString() : sObjectName;
        m_sCompatibleObjectDescription += sSeparator;
        switch (_nCommandType)
        {
            case CommandType::TABLE:
                m_sCompatibleObjectDescription += OUStringChar(cTableMark);
                break;
            case CommandType::QUERY:
                m_sCompatibleObjectDescription += OUStringChar(cQueryMark);
                break;
            case CommandType::COMMAND:
                m_sCompatibleObjectDescription += OUStringChar(cQueryMark);
                // think of it as a query
                break;
        }
        m_sCompatibleObjectDescription += sSeparator;
        m_sCompatibleObjectDescription += _bAddCommand ? _sActiveCommand : OUString();
        m_sCompatibleObjectDescription += sSeparator;
    }

    OMultiColumnTransferable::OMultiColumnTransferable()
    {
    }

    void OMultiColumnTransferable::setDescriptors(const Sequence< PropertyValue >& rDescriptors)
    {
        ClearFormats();
        m_aDescriptors = rDescriptors;
    }

    SotClipboardFormatId OMultiColumnTransferable::getDescriptorFormatId()
    {
        static SotClipboardFormatId s_nFormat = static_cast<SotClipboardFormatId>(-1);
        if (static_cast<SotClipboardFormatId>(-1) == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(u"application/x-openoffice;windows_formatname=\"dbaccess.MultipleColumnDescriptorTransfer\""_ustr);
            OSL_ENSURE(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OColumnTransferable::getDescriptorFormatId: bad exchange id!");
        }
        return s_nFormat;
    }

    void OMultiColumnTransferable::AddSupportedFormats()
    {
        AddFormat(getDescriptorFormatId());
    }

    bool OMultiColumnTransferable::GetData( const DataFlavor& _rFlavor, const OUString& /*rDestDoc*/ )
    {
        const SotClipboardFormatId nFormatId = SotExchange::GetFormat(_rFlavor);
        if (nFormatId == getDescriptorFormatId())
        {
            return SetAny( Any( m_aDescriptors ) );
        }

        return false;
    }

    bool OMultiColumnTransferable::canExtractDescriptor(const DataFlavorExVector& _rFlavors)
    {
        const SotClipboardFormatId nFormatId = getDescriptorFormatId();
        return std::all_of(_rFlavors.begin(), _rFlavors.end(),
            [&nFormatId](const DataFlavorEx& rCheck) { return nFormatId == rCheck.mnSotId; });
    }

    Sequence< PropertyValue > OMultiColumnTransferable::extractDescriptor(const TransferableDataHelper& _rData)
    {
        Sequence< PropertyValue > aList;
        if (_rData.HasFormat(getDescriptorFormatId()))
        {
            // extract the any from the transferable
            DataFlavor aFlavor;
            bool bSuccess =
                SotExchange::GetFormatDataFlavor(getDescriptorFormatId(), aFlavor);
            OSL_ENSURE(bSuccess, "OColumnTransferable::extractColumnDescriptor: invalid data format (no flavor)!");

            _rData.GetAny(aFlavor, OUString()) >>= aList;
        } // if (_rData.HasFormat(getDescriptorFormatId()))
        return aList;
    }

    void OMultiColumnTransferable::ObjectReleased()
    {
        m_aDescriptors.realloc(0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
