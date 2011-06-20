/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

#include "precompiled_dbaccess.hxx"

#include "recovery/dbdocrecovery.hxx"
#include "sdbcoretools.hxx"
#include "storagetextstream.hxx"
#include "subcomponentrecovery.hxx"
#include "subcomponents.hxx"
#include "dbastrings.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <sal/macros.h>

#include <algorithm>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::sdb::application::XDatabaseDocumentUI;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::document::XStorageBasedDocument;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::io::XStream;
    using ::com::sun::star::io::XTextOutputStream;
    using ::com::sun::star::io::XActiveDataSource;
    using ::com::sun::star::io::XTextInputStream;
    using ::com::sun::star::io::XActiveDataSink;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::util::XModifiable;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::lang::XMultiServiceFactory;
    /** === end UNO using === **/

    namespace ElementModes = ::com::sun::star::embed::ElementModes;

    //====================================================================
    //= helpers
    //====================================================================
    namespace
    {
        // .........................................................................
        static void lcl_getPersistentRepresentation( const MapStringToCompDesc::value_type& i_rComponentDesc, ::rtl::OUStringBuffer& o_rBuffer )
        {
            o_rBuffer.append( i_rComponentDesc.first );
            o_rBuffer.append( sal_Unicode( '=' ) );
            o_rBuffer.append( i_rComponentDesc.second.sName );
            o_rBuffer.append( sal_Unicode( ',' ) );
            o_rBuffer.append( sal_Unicode( i_rComponentDesc.second.bForEditing ? '1' : '0' ) );
        }

        // .........................................................................
        static bool lcl_extractCompDesc( const ::rtl::OUString& i_rIniLine, ::rtl::OUString& o_rStorName, SubComponentDescriptor& o_rCompDesc )
        {
            const sal_Int32 nEqualSignPos = i_rIniLine.indexOf( sal_Unicode( '=' ) );
            if ( nEqualSignPos < 1 )
            {
                OSL_FAIL( "lcl_extractCompDesc: invalid map file entry - unexpected pos of '='" );
                return false;
            }
            o_rStorName = i_rIniLine.copy( 0, nEqualSignPos );

            const sal_Int32 nCommaPos = i_rIniLine.lastIndexOf( sal_Unicode( ',' ) );
            if ( nCommaPos != i_rIniLine.getLength() - 2 )
            {
                OSL_FAIL( "lcl_extractCompDesc: invalid map file entry - unexpected pos of ','" );
                return false;
            }
            o_rCompDesc.sName = i_rIniLine.copy( nEqualSignPos + 1, nCommaPos - nEqualSignPos - 1 );
            o_rCompDesc.bForEditing = ( i_rIniLine.getStr()[ nCommaPos + 1 ] == '1' );
            return true;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getRecoveryDataSubStorageName()
        {
            static const ::rtl::OUString s_sRecDataStorName( RTL_CONSTASCII_USTRINGPARAM( "recovery" ) );
            return s_sRecDataStorName;
        }
        // .........................................................................
        static const ::rtl::OUString& lcl_getObjectMapStreamName()
        {
            static const ::rtl::OUString s_sObjectMapStreamName( RTL_CONSTASCII_USTRINGPARAM( "storage-component-map.ini" ) );
            return s_sObjectMapStreamName;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getMapStreamEncodingName()
        {
            static const ::rtl::OUString s_sMapStreamEncodingName( RTL_CONSTASCII_USTRINGPARAM( "UTF-8" ) );
            return s_sMapStreamEncodingName;
        }

        // .........................................................................
        static void lcl_writeObjectMap_throw( const ::comphelper::ComponentContext& i_rContext, const Reference< XStorage >& i_rStorage,
            const MapStringToCompDesc& i_mapStorageToCompDesc )
        {
            if ( i_mapStorageToCompDesc.empty() )
                // nothing to do
                return;

            StorageTextOutputStream aTextOutput( i_rContext, i_rStorage, lcl_getObjectMapStreamName() );

            aTextOutput.writeLine( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "[storages]" ) ) );

            for (   MapStringToCompDesc::const_iterator stor = i_mapStorageToCompDesc.begin();
                    stor != i_mapStorageToCompDesc.end();
                    ++stor
                )
            {
                ::rtl::OUStringBuffer aLine;
                lcl_getPersistentRepresentation( *stor, aLine );

                aTextOutput.writeLine( aLine.makeStringAndClear() );
            }

            aTextOutput.writeLine();
        }

        // .........................................................................
        static bool lcl_isSectionStart( const ::rtl::OUString& i_rIniLine, ::rtl::OUString& o_rSectionName )
        {
            const sal_Int32 nLen = i_rIniLine.getLength();
            if ( ( nLen > 0 ) && ( i_rIniLine.getStr()[0] == '[' ) && ( i_rIniLine.getStr()[ nLen - 1 ] == ']' ) )
            {
                o_rSectionName = i_rIniLine.copy( 1, nLen -2 );
                return true;
            }
            return false;
        }

        // .........................................................................
        static void lcl_stripTrailingLineFeed( ::rtl::OUString& io_rLine )
        {
            const sal_Int32 nLen = io_rLine.getLength();
            if ( ( nLen > 0 ) && ( io_rLine.getStr()[ nLen - 1 ] == '\n' ) )
                io_rLine = io_rLine.copy( 0, nLen - 1 );
        }

        // .........................................................................
        static void lcl_readObjectMap_throw( const ::comphelper::ComponentContext& i_rContext, const Reference< XStorage >& i_rStorage,
            MapStringToCompDesc& o_mapStorageToObjectName )
        {
            ENSURE_OR_THROW( i_rStorage.is(), "invalid storage" );
            if ( !i_rStorage->hasByName( lcl_getObjectMapStreamName() ) )
            {   // nothing to do, though suspicious
                OSL_FAIL( "lcl_readObjectMap_throw: if there's no map file, then there's expected to be no storage, too!" );
                return;
            }

            Reference< XStream > xIniStream( i_rStorage->openStreamElement(
                lcl_getObjectMapStreamName(), ElementModes::READ ), UNO_SET_THROW );

            Reference< XTextInputStream > xTextInput( i_rContext.createComponent( "com.sun.star.io.TextInputStream" ), UNO_QUERY_THROW );
            xTextInput->setEncoding( lcl_getMapStreamEncodingName() );

            Reference< XActiveDataSink > xDataSink( xTextInput, UNO_QUERY_THROW );
            xDataSink->setInputStream( xIniStream->getInputStream() );

            ::rtl::OUString sCurrentSection;
            bool bCurrentSectionIsKnownToBeUnsupported = true;
            while ( !xTextInput->isEOF() )
            {
                ::rtl::OUString sLine = xTextInput->readLine();
                lcl_stripTrailingLineFeed( sLine );

                if ( sLine.getLength() == 0 )
                    continue;

                if ( lcl_isSectionStart( sLine, sCurrentSection ) )
                {
                    bCurrentSectionIsKnownToBeUnsupported = false;
                    continue;
                }

                if ( bCurrentSectionIsKnownToBeUnsupported )
                    continue;

                // the only section we support so far is "storages"
                if ( !sCurrentSection.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "storages" ) ) )
                {
                    bCurrentSectionIsKnownToBeUnsupported = true;
                    continue;
                }

                ::rtl::OUString sStorageName;
                SubComponentDescriptor aCompDesc;
                if ( !lcl_extractCompDesc( sLine, sStorageName, aCompDesc ) )
                    continue;
                o_mapStorageToObjectName[ sStorageName ] = aCompDesc;
            }
        }

        // .........................................................................
        static void lcl_markModified( const Reference< XComponent >& i_rSubComponent )
        {
            const Reference< XModifiable > xModify( i_rSubComponent, UNO_QUERY );
            if ( !xModify.is() )
            {
                OSL_FAIL( "lcl_markModified: unhandled case!" );
                return;
            }

            xModify->setModified( sal_True );
        }
    }

    //====================================================================
    //= DatabaseDocumentRecovery_Data
    //====================================================================
    struct DBACCESS_DLLPRIVATE DatabaseDocumentRecovery_Data
    {
        const ::comphelper::ComponentContext aContext;

        DatabaseDocumentRecovery_Data( const ::comphelper::ComponentContext& i_rContext )
            :aContext( i_rContext )
        {
        }
    };

    //====================================================================
    //= DatabaseDocumentRecovery
    //====================================================================
    //--------------------------------------------------------------------
    DatabaseDocumentRecovery::DatabaseDocumentRecovery( const ::comphelper::ComponentContext& i_rContext )
        :m_pData( new DatabaseDocumentRecovery_Data( i_rContext ) )
    {
    }

    //--------------------------------------------------------------------
    DatabaseDocumentRecovery::~DatabaseDocumentRecovery()
    {
    }

    //--------------------------------------------------------------------
    void DatabaseDocumentRecovery::saveModifiedSubComponents( const Reference< XStorage >& i_rTargetStorage,
        const ::std::vector< Reference< XController > >& i_rControllers )
    {
        ENSURE_OR_THROW( i_rTargetStorage.is(), "invalid document storage" );

        // create a sub storage for recovery data
        if ( i_rTargetStorage->hasByName( lcl_getRecoveryDataSubStorageName() ) )
            i_rTargetStorage->removeElement( lcl_getRecoveryDataSubStorageName() );
        Reference< XStorage > xRecoveryStorage = i_rTargetStorage->openStorageElement( lcl_getRecoveryDataSubStorageName(), ElementModes::READWRITE );

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

    //--------------------------------------------------------------------
    void DatabaseDocumentRecovery::recoverSubDocuments( const Reference< XStorage >& i_rDocumentStorage,
        const Reference< XController >& i_rTargetController )
    {
        ENSURE_OR_THROW( i_rDocumentStorage.is(), "illegal document storage" );
        Reference< XDatabaseDocumentUI > xDocumentUI( i_rTargetController, UNO_QUERY_THROW );

        if ( !i_rDocumentStorage->hasByName( lcl_getRecoveryDataSubStorageName() ) )
            // that's allowed
            return;

        // the "recovery" sub storage
        Reference< XStorage > xRecoveryStorage = i_rDocumentStorage->openStorageElement( lcl_getRecoveryDataSubStorageName(), ElementModes::READ );

        // read the map from sub storages to object names
        MapCompTypeToCompDescs aMapCompDescs;
        SubComponentType aKnownTypes[] = { TABLE, QUERY, FORM, REPORT, RELATION_DESIGN };
        for ( size_t i = 0; i < SAL_N_ELEMENTS( aKnownTypes ); ++i )
        {
            if ( !xRecoveryStorage->hasByName( SubComponentRecovery::getComponentsStorageName( aKnownTypes[i] ) ) )
                continue;

            Reference< XStorage > xComponentsStor( xRecoveryStorage->openStorageElement(
                SubComponentRecovery::getComponentsStorageName( aKnownTypes[i] ), ElementModes::READ ) );
            lcl_readObjectMap_throw( m_pData->aContext, xComponentsStor, aMapCompDescs[ aKnownTypes[i] ] );
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

            // loop thru all components of this type
            for (   MapStringToCompDesc::const_iterator stor = map->second.begin();
                    stor != map->second.end();
                    ++stor
                )
            {
                const ::rtl::OUString sComponentName( stor->second.sName );
                if ( !xComponentsStor->hasByName( stor->first ) )
                {
                #if OSL_DEBUG_LEVEL > 0
                    ::rtl::OStringBuffer message;
                    message.append( "DatabaseDocumentRecovery::recoverSubDocuments: inconsistent recovery storage: storage '" );
                    message.append( ::rtl::OUStringToOString( stor->first, RTL_TEXTENCODING_ASCII_US ) );
                    message.append( "' not found in '" );
                    message.append( ::rtl::OUStringToOString( SubComponentRecovery::getComponentsStorageName( eComponentType ), RTL_TEXTENCODING_ASCII_US ) );
                    message.append( "', but required per map file!" );
                    OSL_FAIL( message.makeStringAndClear() );
                #endif
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
            i_rDocumentStorage->removeElement( lcl_getRecoveryDataSubStorageName() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
} // namespace dbaccess
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
