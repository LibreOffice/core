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

#include "dbdocrecovery.hxx"
#include "sdbcoretools.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <connectivity/dbtools.hxx>
#include <rtl/strbuf.hxx>
#include <tools/diagnose_ex.h>

#include <hash_map>

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
    using ::com::sun::star::frame::XModuleManager;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::document::XStorageBasedDocument;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::beans::Pair;
    using ::com::sun::star::io::XStream;
    using ::com::sun::star::io::XOutputStream;
    using ::com::sun::star::io::XTextOutputStream;
    using ::com::sun::star::io::XActiveDataSource;
    /** === end UNO using === **/

    namespace ElementModes = ::com::sun::star::embed::ElementModes;

    //====================================================================
    //= MapStringToString
    //====================================================================
    typedef ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >   MapStringToString;

    //====================================================================
    //= helpers
    //====================================================================
    namespace
    {
        // .........................................................................
        const ::rtl::OUString& lcl_getRecoveryDataSubStorageName()
        {
            static const ::rtl::OUString s_sRecDataStorName( RTL_CONSTASCII_USTRINGPARAM( "recovery" ) );
            return s_sRecDataStorName;
        }

        // .........................................................................
        const ::rtl::OUString& lcl_getFormsStorageName()
        {
            static const ::rtl::OUString s_sFormsStorageName( RTL_CONSTASCII_USTRINGPARAM( "forms" ) );
            return s_sFormsStorageName;
        }

        // .........................................................................
        const ::rtl::OUString& lcl_getReportsStorageName()
        {
            static const ::rtl::OUString s_sReportsStorageName( RTL_CONSTASCII_USTRINGPARAM( "reports" ) );
            return s_sReportsStorageName;
        }

        // .........................................................................
        const ::rtl::OUString& lcl_getObjectMapStreamName()
        {
            static const ::rtl::OUString s_sObjectMapStreamName( RTL_CONSTASCII_USTRINGPARAM( "storage-component-map.ini" ) );
            return s_sObjectMapStreamName;
        }

        // .........................................................................
        enum SubComponentType
        {
            WRITER_FORM,
            WRITER_REPORT,
            SRB_REPORT,
            TABLE_DESIGN,
            QUERY_DESIGN,
            TABLE_DATA_VIEW,
            RELATION_DESIGN,

            UNKNOWN
        };

        // .........................................................................
        SubComponentType lcl_identifyComponent_throw( const ::comphelper::ComponentContext& i_rContext, const Reference< XComponent >& i_rComponent )
        {
            Reference< XModuleManager > xModuleManager( i_rContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );
            const ::rtl::OUString sModuleIdentifier = xModuleManager->identify( i_rComponent );

            struct ComponentTypeMapping
            {
                const sal_Char*     pAsciiModuleName;
                SubComponentType    eComponentType;
            };
            const ComponentTypeMapping aKnownCompTypes[] =
            {
                { "com.sun.star.sdb.FormDesign", WRITER_FORM },
                { "com.sun.star.sdb.TextReportDesign", WRITER_REPORT },
                { "com.sun.star.report.ReportDefinition", WRITER_REPORT },
                { "com.sun.star.sdb.TableDesign", TABLE_DESIGN },
                { "com.sun.star.sdb.QueryDesign", QUERY_DESIGN },
                { "com.sun.star.sdb.TableDataView", TABLE_DATA_VIEW },
                { "com.sun.star.sdb.RelationDesign", RELATION_DESIGN }
            };

            for ( size_t i=0; i < sizeof( aKnownCompTypes ) / sizeof( aKnownCompTypes[0] ); ++i )
            {
                if ( sModuleIdentifier.equalsAscii( aKnownCompTypes[i].pAsciiModuleName ) )
                    return aKnownCompTypes[i].eComponentType;
            }

    #if OSL_DEBUG_LEVEL > 0
            ::rtl::OStringBuffer message;
            message.append( "lcl_identifyComponent_throw: unknown module identifier: '" );
            message.append( ::rtl::OUStringToOString( sModuleIdentifier, RTL_TEXTENCODING_ASCII_US ) );
            message.append( "'" );
            OSL_ENSURE( false, message.makeStringAndClear().getStr() );
    #endif
            return UNKNOWN;
        }

        // .........................................................................
        void lcl_writeObjectMap_throw( const ::comphelper::ComponentContext& i_rContext, const Reference< XStorage >& i_rStorage,
            const MapStringToString& i_mapStorageToObjectName )
        {
            if ( i_mapStorageToObjectName.empty() )
                // nothing to do
                return;

            ENSURE_OR_THROW( i_rStorage.is(), "invalid storage" );
            Reference< XStream > xIniStream( i_rStorage->openStreamElement(
                lcl_getObjectMapStreamName(), ElementModes::WRITE | ElementModes::TRUNCATE ), UNO_SET_THROW );

            Reference< XTextOutputStream > xTextOutput( i_rContext.createComponent( "com.sun.star.io.TextOutputStream" ), UNO_QUERY_THROW );
            xTextOutput->setEncoding( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "utf-8" ) ) );

            Reference< XActiveDataSource > xDataSource( xTextOutput, UNO_QUERY_THROW );
            xDataSource->setOutputStream( xIniStream->getOutputStream() );

            const ::rtl::OUString sLineFeed( sal_Unicode( '\n' ) );
            xTextOutput->writeString( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "[storages]" ) ) );
            xTextOutput->writeString( sLineFeed );

            for (   MapStringToString::const_iterator stor = i_mapStorageToObjectName.begin();
                    stor != i_mapStorageToObjectName.end();
                    ++stor
                )
            {
                ::rtl::OUStringBuffer aLine;
                aLine.append( stor->first );
                aLine.append( sal_Unicode( '=' ) );
                aLine.append( stor->second );
                aLine.append( sLineFeed );
                xTextOutput->writeString( aLine.makeStringAndClear() );
            }

            xTextOutput->writeString( sLineFeed );
        }
    }

    //====================================================================
    //= DatabaseDocumentRecovery_Data
    //====================================================================
    struct DatabaseDocumentRecovery_Data
    {
        const ::comphelper::ComponentContext    aContext;
        const Reference< XStorage >             xDocSalvageStorage;

        DatabaseDocumentRecovery_Data( const ::comphelper::ComponentContext& i_rContext, const Reference< XStorage >& i_rDocumentSalvageStorage )
            :aContext( i_rContext )
            ,xDocSalvageStorage( i_rDocumentSalvageStorage )
        {
        }
    };

    //====================================================================
    //= SubComponentRecovery
    //====================================================================
    class SubComponentRecovery
    {
    public:
        SubComponentRecovery( const ::comphelper::ComponentContext& i_rContext, const Reference< XDatabaseDocumentUI >& i_rController,
                const Reference< XComponent >& i_rComponent )
            :m_rContext( i_rContext )
            ,m_xComponent( i_rComponent )
        {
            ENSURE_OR_THROW( i_rController.is(), "illegal controller" );

            // retrieve the name of the object to which the component belongs to
            Pair< sal_Int32, ::rtl::OUString > aComponentIdentity = i_rController->identifySubComponent( i_rComponent );
            m_sObjectName = aComponentIdentity.Second;
        }

        void saveToRecoveryStorage( const Reference< XStorage >& i_rRecoveryStorage, MapStringToString& io_mapStorageToObjectName );

    private:
        void impl_saveWriterSubComponent_throw(
            const SubComponentType i_eType,
            const Reference< XStorage >& i_rRecoveryStorage,
            MapStringToString& io_mapStorageToObjectName
        );

    private:
        const ::comphelper::ComponentContext&   m_rContext;
        const Reference< XComponent >           m_xComponent;
        ::rtl::OUString                         m_sObjectName;
    };

    //--------------------------------------------------------------------
    void SubComponentRecovery::saveToRecoveryStorage( const Reference< XStorage >& i_rRecoveryStorage, MapStringToString& io_mapStorageToObjectName )
    {
        SubComponentType eComponentType = lcl_identifyComponent_throw( m_rContext, m_xComponent );
        if ( eComponentType == UNKNOWN )
            // quite fatal, but has already been reported as assertion in lcl_identifyComponent_throw
            return;

        switch ( eComponentType )
        {
        case WRITER_FORM:
        case WRITER_REPORT:
            impl_saveWriterSubComponent_throw( eComponentType, i_rRecoveryStorage, io_mapStorageToObjectName );
            break;

        default:
            // TODO
            OSL_ENSURE( false, "SubComponentRecoverys::saveToRecoveryStorage: unimplemented case!" );
            break;
        }
    }

    //--------------------------------------------------------------------
    void SubComponentRecovery::impl_saveWriterSubComponent_throw( const SubComponentType i_eType, const Reference< XStorage >& i_rRecoveryStorage,
        MapStringToString& io_mapStorageToObjectName )
    {
        ENSURE_OR_THROW( ( i_eType == WRITER_FORM ) || ( i_eType == WRITER_REPORT ), "illegal sub component type" );
        ENSURE_OR_THROW( i_rRecoveryStorage.is(), "illegal storage" );

        // open the sub storage for the given kind of documents
        const ::rtl::OUString& rStorageName( i_eType == WRITER_FORM ? lcl_getFormsStorageName() : lcl_getReportsStorageName() );
        const Reference< XStorage > xDocsStor( i_rRecoveryStorage->openStorageElement( rStorageName, ElementModes::READWRITE ), UNO_QUERY_THROW );

        // find a free sub storage name, and create Yet Another Sub Storage
        const ::rtl::OUString sBaseName( ::rtl::OUString::createFromAscii( i_eType == WRITER_FORM ? "form" : "report" ) );
        const ::rtl::OUString sCompStorName = ::dbtools::createUniqueName( xDocsStor.get(), sBaseName, true );
        const Reference< XStorage > xCompStor( xDocsStor->openStorageElement( sCompStorName, ElementModes::READWRITE ), UNO_QUERY_THROW );

        // store the document into the storage
        Reference< XStorageBasedDocument > xStorageDocument( m_xComponent, UNO_QUERY_THROW );
        xStorageDocument->storeToStorage( xCompStor, Sequence< PropertyValue >() );

        // remember the map from the component name to the storage name
        ::rtl::OUStringBuffer aSubStoragePath;
        aSubStoragePath.append( rStorageName );
        aSubStoragePath.append( sal_Unicode( '/' ) );
        aSubStoragePath.append( sCompStorName );
        const ::rtl::OUString sSubStoragePath = aSubStoragePath.makeStringAndClear();

        OSL_ENSURE( io_mapStorageToObjectName.find( sSubStoragePath ) == io_mapStorageToObjectName.end(),
            "SubComponentRecoverys::impl_saveWriterSubComponent_throw: object name already used!" );
        io_mapStorageToObjectName[ sSubStoragePath ] = m_sObjectName;

        // commit the storage for the documents collection
        tools::stor::commitStorageIfWriteable( xDocsStor );
    }

    //====================================================================
    //= DatabaseDocumentRecovery
    //====================================================================
    //--------------------------------------------------------------------
    DatabaseDocumentRecovery::DatabaseDocumentRecovery( const ::comphelper::ComponentContext& i_rContext, const Reference< XStorage >& i_rDocumentSalvageStorage )
        :m_pData( new DatabaseDocumentRecovery_Data( i_rContext, i_rDocumentSalvageStorage ) )
    {
        ENSURE_OR_THROW( m_pData->xDocSalvageStorage.is(), "invalid document storage" );
    }

    //--------------------------------------------------------------------
    DatabaseDocumentRecovery::~DatabaseDocumentRecovery()
    {
    }

    //--------------------------------------------------------------------
    void DatabaseDocumentRecovery::saveModifiedSubComponents( const ::std::vector< Reference< XController > >& i_rControllers )
    {
        // create a sub storage for recovery data
        if ( m_pData->xDocSalvageStorage->hasByName( lcl_getRecoveryDataSubStorageName() ) )
            m_pData->xDocSalvageStorage->removeElement( lcl_getRecoveryDataSubStorageName() );
        Reference< XStorage > xRecoveryStorage = m_pData->xDocSalvageStorage->openStorageElement( lcl_getRecoveryDataSubStorageName(), ElementModes::READWRITE );

        // store recovery data for open sub components of the given controller(s)
        if ( !i_rControllers.empty() )
        {
            ENSURE_OR_THROW( i_rControllers.size() == 1, "can't handle more than one controller" );
            // At the moment, there can be only one view to a database document. If we ever allow for more than this,
            // then we need a concept for sub documents opened from different controllers (i.e. two document views,
            // and the user opens the very same form in both views). And depending on this, we need a concept for
            // how those are saved to the recovery file.

            MapStringToString aMapStorageToObjectName;

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
                    aComponentRecovery.saveToRecoveryStorage( xRecoveryStorage, aMapStorageToObjectName );
                }
            }

            lcl_writeObjectMap_throw( m_pData->aContext, xRecoveryStorage, aMapStorageToObjectName );
        }

        // commit the recovery storage
        tools::stor::commitStorageIfWriteable( xRecoveryStorage );
    }

//........................................................................
} // namespace dbaccess
//........................................................................
