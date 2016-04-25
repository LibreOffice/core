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

#include "recovery/dbdocrecovery.hxx"
#include "sdbcoretools.hxx"
#include "storagetextstream.hxx"
#include "subcomponentrecovery.hxx"
#include "subcomponents.hxx"
#include "dbastrings.hrc"

#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/TextInputStream.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>

namespace dbaccess
{

    using css::uno::Reference;
    using css::uno::UNO_QUERY;
    using css::uno::UNO_QUERY_THROW;
    using css::uno::UNO_SET_THROW;
    using css::uno::Exception;
    using css::uno::Sequence;
    using css::uno::XComponentContext;
    using css::embed::XStorage;
    using css::frame::XController;
    using css::sdb::application::XDatabaseDocumentUI;
    using css::lang::XComponent;
    using css::io::XStream;
    using css::io::TextInputStream;
    using css::io::XTextInputStream2;
    using css::util::XModifiable;

    namespace ElementModes = css::embed::ElementModes;

    // helpers
    namespace
    {
        void lcl_getPersistentRepresentation( const MapStringToCompDesc::value_type& i_rComponentDesc, OUStringBuffer& o_rBuffer )
        {
            o_rBuffer.append( i_rComponentDesc.first );
            o_rBuffer.append( '=' );
            o_rBuffer.append( i_rComponentDesc.second.sName );
            o_rBuffer.append( ',' );
            o_rBuffer.append( sal_Unicode( i_rComponentDesc.second.bForEditing ? '1' : '0' ) );
        }

        bool lcl_extractCompDesc( const OUString& i_rIniLine, OUString& o_rStorName, SubComponentDescriptor& o_rCompDesc )
        {
            const sal_Int32 nEqualSignPos = i_rIniLine.indexOf( '=' );
            if ( nEqualSignPos < 1 )
            {
                OSL_FAIL( "lcl_extractCompDesc: invalid map file entry - unexpected pos of '='" );
                return false;
            }
            o_rStorName = i_rIniLine.copy( 0, nEqualSignPos );

            const sal_Int32 nCommaPos = i_rIniLine.lastIndexOf( ',' );
            if ( nCommaPos != i_rIniLine.getLength() - 2 )
            {
                OSL_FAIL( "lcl_extractCompDesc: invalid map file entry - unexpected pos of ','" );
                return false;
            }
            o_rCompDesc.sName = i_rIniLine.copy( nEqualSignPos + 1, nCommaPos - nEqualSignPos - 1 );
            o_rCompDesc.bForEditing = ( i_rIniLine[ nCommaPos + 1 ] == '1' );
            return true;
        }

        static const char sRecoveryDataSubStorageName[] = "recovery";

        static const char sObjectMapStreamName[] = "storage-component-map.ini";

        void lcl_writeObjectMap_throw( const Reference<XComponentContext> & i_rContext, const Reference< XStorage >& i_rStorage,
            const MapStringToCompDesc& i_mapStorageToCompDesc )
        {
            if ( i_mapStorageToCompDesc.empty() )
                // nothing to do
                return;

            StorageTextOutputStream aTextOutput( i_rContext, i_rStorage, sObjectMapStreamName );

            aTextOutput.writeLine( "[storages]" );

            for (   MapStringToCompDesc::const_iterator stor = i_mapStorageToCompDesc.begin();
                    stor != i_mapStorageToCompDesc.end();
                    ++stor
                )
            {
                OUStringBuffer aLine;
                lcl_getPersistentRepresentation( *stor, aLine );

                aTextOutput.writeLine( aLine.makeStringAndClear() );
            }

            aTextOutput.writeLine();
        }

        bool lcl_isSectionStart( const OUString& i_rIniLine, OUString& o_rSectionName )
        {
            const sal_Int32 nLen = i_rIniLine.getLength();
            if ( i_rIniLine.startsWith("[") && i_rIniLine.endsWith("]") )
            {
                o_rSectionName = i_rIniLine.copy( 1, nLen -2 );
                return true;
            }
            return false;
        }

        void lcl_stripTrailingLineFeed( OUString& io_rLine )
        {
            const sal_Int32 nLen = io_rLine.getLength();
            if ( io_rLine.endsWith("\n") )
                io_rLine = io_rLine.copy( 0, nLen - 1 );
        }

        void lcl_readObjectMap_throw( const Reference<XComponentContext> & i_rxContext, const Reference< XStorage >& i_rStorage,
            MapStringToCompDesc& o_mapStorageToObjectName )
        {
            ENSURE_OR_THROW( i_rStorage.is(), "invalid storage" );
            if ( !i_rStorage->hasByName( sObjectMapStreamName ) )
            {   // nothing to do, though suspicious
                OSL_FAIL( "lcl_readObjectMap_throw: if there's no map file, then there's expected to be no storage, too!" );
                return;
            }

            Reference< XStream > xIniStream( i_rStorage->openStreamElement(
                sObjectMapStreamName, ElementModes::READ ), UNO_SET_THROW );

            Reference< XTextInputStream2 > xTextInput = TextInputStream::create( i_rxContext );
            xTextInput->setEncoding( "UTF-8" );
            xTextInput->setInputStream( xIniStream->getInputStream() );

            OUString sCurrentSection;
            bool bCurrentSectionIsKnownToBeUnsupported = true;
            while ( !xTextInput->isEOF() )
            {
                OUString sLine = xTextInput->readLine();
                lcl_stripTrailingLineFeed( sLine );

                if ( sLine.isEmpty() )
                    continue;

                if ( lcl_isSectionStart( sLine, sCurrentSection ) )
                {
                    bCurrentSectionIsKnownToBeUnsupported = false;
                    continue;
                }

                if ( bCurrentSectionIsKnownToBeUnsupported )
                    continue;

                // the only section we support so far is "storages"
                if ( sCurrentSection != "storages" )
                {
                    bCurrentSectionIsKnownToBeUnsupported = true;
                    continue;
                }

                OUString sStorageName;
                SubComponentDescriptor aCompDesc;
                if ( !lcl_extractCompDesc( sLine, sStorageName, aCompDesc ) )
                    continue;
                o_mapStorageToObjectName[ sStorageName ] = aCompDesc;
            }
        }

        void lcl_markModified( const Reference< XComponent >& i_rSubComponent )
        {
            const Reference< XModifiable > xModify( i_rSubComponent, UNO_QUERY );
            if ( !xModify.is() )
            {
                OSL_FAIL( "lcl_markModified: unhandled case!" );
                return;
            }

            xModify->setModified( true );
        }
    }

    // DatabaseDocumentRecovery_Data
    struct DBACCESS_DLLPRIVATE DatabaseDocumentRecovery_Data
    {
        const Reference<XComponentContext> aContext;

        explicit DatabaseDocumentRecovery_Data( const Reference<XComponentContext> & i_rContext )
            :aContext( i_rContext )
        {
        }
    };

    // DatabaseDocumentRecovery
    DatabaseDocumentRecovery::DatabaseDocumentRecovery( const Reference<XComponentContext> & i_rContext )
        :m_pData( new DatabaseDocumentRecovery_Data( i_rContext ) )
    {
    }

    DatabaseDocumentRecovery::~DatabaseDocumentRecovery()
    {
    }

    void DatabaseDocumentRecovery::saveModifiedSubComponents( const Reference< XStorage >& i_rTargetStorage,
        const ::std::vector< Reference< XController > >& i_rControllers )
    {
        ENSURE_OR_THROW( i_rTargetStorage.is(), "invalid document storage" );

        // create a sub storage for recovery data
        if ( i_rTargetStorage->hasByName( sRecoveryDataSubStorageName ) )
            i_rTargetStorage->removeElement( sRecoveryDataSubStorageName );
        Reference< XStorage > xRecoveryStorage = i_rTargetStorage->openStorageElement( sRecoveryDataSubStorageName, ElementModes::READWRITE );

        // store recovery data for open sub components of the given controller(s)
        if ( !i_rControllers.empty() )
        {
            ENSURE_OR_THROW( i_rControllers.size() == 1, "can't handle more than one controller" );
            // At the moment, there can be only one view to a database document. If we ever allow for more than this,
            // then we need a concept for sub documents opened from different controllers (i.e. two document views,
            // and the user opens the very same form in both views). And depending on this, we need a concept for
            // how those are saved to the recovery file.

            MapCompTypeToCompDescs aMapCompDescs;

            for (   ::std::vector< Reference< XController > >::const_iterator ctrl = i_rControllers.begin();
                    ctrl != i_rControllers.end();
                    ++ctrl
                )
            {
                Reference< XDatabaseDocumentUI > xDatabaseUI( *ctrl, UNO_QUERY_THROW );
                Sequence< Reference< XComponent > > aComponents( xDatabaseUI->getSubComponents() );

                const Reference< XComponent >* component = aComponents.getConstArray();
                const Reference< XComponent >* componentEnd = aComponents.getConstArray() + aComponents.getLength();
                for ( ; component != componentEnd; ++component )
                {
                    SubComponentRecovery aComponentRecovery( m_pData->aContext, xDatabaseUI, *component );
                    aComponentRecovery.saveToRecoveryStorage( xRecoveryStorage, aMapCompDescs );
                }
            }

            for (   MapCompTypeToCompDescs::const_iterator map = aMapCompDescs.begin();
                    map != aMapCompDescs.end();
                    ++map
                )
            {
                Reference< XStorage > xComponentsStor( xRecoveryStorage->openStorageElement(
                    SubComponentRecovery::getComponentsStorageName( map->first ), ElementModes::WRITE | ElementModes::NOCREATE ) );
                lcl_writeObjectMap_throw( m_pData->aContext, xComponentsStor, map->second );
                tools::stor::commitStorageIfWriteable( xComponentsStor );
            }
        }

        // commit the recovery storage
        tools::stor::commitStorageIfWriteable( xRecoveryStorage );
    }

    void DatabaseDocumentRecovery::recoverSubDocuments( const Reference< XStorage >& i_rDocumentStorage,
        const Reference< XController >& i_rTargetController )
    {
        ENSURE_OR_THROW( i_rDocumentStorage.is(), "illegal document storage" );
        Reference< XDatabaseDocumentUI > xDocumentUI( i_rTargetController, UNO_QUERY_THROW );

        if ( !i_rDocumentStorage->hasByName( sRecoveryDataSubStorageName ) )
            // that's allowed
            return;

        // the "recovery" sub storage
        Reference< XStorage > xRecoveryStorage = i_rDocumentStorage->openStorageElement( sRecoveryDataSubStorageName, ElementModes::READ );

        // read the map from sub storages to object names
        MapCompTypeToCompDescs aMapCompDescs;
        const SubComponentType aKnownTypes[] = { TABLE, QUERY, FORM, REPORT, RELATION_DESIGN };
        for (SubComponentType aKnownType : aKnownTypes)
        {
            if ( !xRecoveryStorage->hasByName( SubComponentRecovery::getComponentsStorageName( aKnownType ) ) )
                continue;

            Reference< XStorage > xComponentsStor( xRecoveryStorage->openStorageElement(
                SubComponentRecovery::getComponentsStorageName( aKnownType ), ElementModes::READ ) );
            lcl_readObjectMap_throw( m_pData->aContext, xComponentsStor, aMapCompDescs[ aKnownType ] );
            xComponentsStor->dispose();
        }

        // recover all sub components as indicated by the map
        for (   MapCompTypeToCompDescs::const_iterator map = aMapCompDescs.begin();
                map != aMapCompDescs.end();
                ++map
            )
        {
            const SubComponentType eComponentType = map->first;

            // the storage for all components of the current type
            Reference< XStorage > xComponentsStor( xRecoveryStorage->openStorageElement(
                SubComponentRecovery::getComponentsStorageName( eComponentType ), ElementModes::READ ), UNO_QUERY_THROW );

            // loop through all components of this type
            for (   MapStringToCompDesc::const_iterator stor = map->second.begin();
                    stor != map->second.end();
                    ++stor
                )
            {
                const OUString sComponentName( stor->second.sName );
                if ( !xComponentsStor->hasByName( stor->first ) )
                {
                    SAL_WARN( "dbaccess",
                              "DatabaseDocumentRecovery::recoverSubDocuments: inconsistent recovery storage: storage '" <<
                              stor->first <<
                              "' not found in '" <<
                              SubComponentRecovery::getComponentsStorageName( eComponentType ) <<
                              "', but required per map file!" );
                    continue;
                }

                // the controller needs to have a connection to be able to open sub components
                if ( !xDocumentUI->isConnected() )
                    xDocumentUI->connect();

                // recover the single component
                Reference< XStorage > xCompStor( xComponentsStor->openStorageElement( stor->first, ElementModes::READ ) );
                SubComponentRecovery aComponentRecovery( m_pData->aContext, xDocumentUI, eComponentType );
                Reference< XComponent > xSubComponent( aComponentRecovery.recoverFromStorage( xCompStor, sComponentName, stor->second.bForEditing ) );

                // at the moment, we only store, during session save, sub components which are modified. So, set this
                // recovered sub component to "modified", too.
                lcl_markModified( xSubComponent );
            }

            xComponentsStor->dispose();
        }

        xRecoveryStorage->dispose();

        // now that we successfully recovered, removed the "recovery" sub storage
        try
        {
            i_rDocumentStorage->removeElement( sRecoveryDataSubStorageName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
