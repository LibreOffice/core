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
#include "storagexmlstream.hxx"
#include "storagetextstream.hxx"
#include "subcomponentloader.hxx"
#include "dbastrings.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <connectivity/dbtools.hxx>
#include <rtl/strbuf.hxx>
#include <tools/diagnose_ex.h>
#include <xmloff/XMLSettingsExportContext.hxx>
#include <xmloff/SettingsExportHelper.hxx>

#include <hash_map>
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
    using ::com::sun::star::frame::XModuleManager;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::document::XStorageBasedDocument;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::beans::Pair;
    using ::com::sun::star::io::XStream;
    using ::com::sun::star::io::XTextOutputStream;
    using ::com::sun::star::io::XActiveDataSource;
    using ::com::sun::star::io::XTextInputStream;
    using ::com::sun::star::io::XActiveDataSink;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::util::XModifiable;
    using ::com::sun::star::sdb::XFormDocumentsSupplier;
    using ::com::sun::star::sdb::XReportDocumentsSupplier;
    using ::com::sun::star::ucb::XCommandProcessor;
    using ::com::sun::star::container::XHierarchicalNameAccess;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::lang::XMultiServiceFactory;
    /** === end UNO using === **/

    namespace ElementModes = ::com::sun::star::embed::ElementModes;
    namespace DatabaseObject = ::com::sun::star::sdb::application::DatabaseObject;

    //====================================================================
    //= helpers
    //====================================================================
    namespace
    {
        // .........................................................................
        enum SubComponentType
        {
            TABLE = DatabaseObject::TABLE,
            QUERY = DatabaseObject::QUERY,
            FORM = DatabaseObject::FORM,
            REPORT = DatabaseObject::REPORT,

            RELATION_DESIGN = 1000,

            UNKNOWN         = 10001
        };

        struct DBACCESS_DLLPRIVATE SubComponentDescriptor
        {
            ::rtl::OUString     sName;
            bool                bForEditing;

            SubComponentDescriptor()
                :sName()
                ,bForEditing( false )
            {
            }

            SubComponentDescriptor( const ::rtl::OUString& i_rName, const bool i_bForEditing )
                :sName( i_rName )
                ,bForEditing( i_bForEditing )
            {
            }
        };

        // .........................................................................
        typedef ::std::hash_map< ::rtl::OUString, SubComponentDescriptor, ::rtl::OUStringHash > MapStringToCompDesc;
        typedef ::std::map< SubComponentType, MapStringToCompDesc > MapCompTypeToCompDescs;

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
                OSL_ENSURE( false, "lcl_extractCompDesc: invalid map file entry - unexpected pos of '='" );
                return false;
            }
            o_rStorName = i_rIniLine.copy( 0, nEqualSignPos );

            const sal_Int32 nCommaPos = i_rIniLine.lastIndexOf( sal_Unicode( ',' ) );
            if ( nCommaPos != i_rIniLine.getLength() - 2 )
            {
                OSL_ENSURE( false, "lcl_extractCompDesc: invalid map file entry - unexpected pos of ','" );
                return false;
            }
            o_rCompDesc.sName = i_rIniLine.copy( nEqualSignPos + 1, nCommaPos - nEqualSignPos - 1 );
            o_rCompDesc.bForEditing = ( i_rIniLine.getStr()[ nCommaPos + 1 ] == '1' );
            return true;
        }

        // .........................................................................
        static SubComponentType lcl_databaseObjectToSubComponentType( const sal_Int32 i_nObjectType )
        {
            switch ( i_nObjectType )
            {
            case DatabaseObject::TABLE: return TABLE;
            case DatabaseObject::QUERY: return QUERY;
            case DatabaseObject::FORM:  return FORM;
            case DatabaseObject::REPORT:return REPORT;
            default:
                break;
            }
            return UNKNOWN;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getRecoveryDataSubStorageName()
        {
            static const ::rtl::OUString s_sRecDataStorName( RTL_CONSTASCII_USTRINGPARAM( "recovery" ) );
            return s_sRecDataStorName;
        }
        // .........................................................................
        static const ::rtl::OUString& lcl_getComponentStorageBaseName( const SubComponentType i_eType )
        {
            static const ::rtl::OUString s_sFormBaseName( RTL_CONSTASCII_USTRINGPARAM( "form" ) );
            static const ::rtl::OUString s_sReportBaseName( RTL_CONSTASCII_USTRINGPARAM( "report" ) );
            static const ::rtl::OUString s_sTableBaseName( RTL_CONSTASCII_USTRINGPARAM( "table" ) );
            static const ::rtl::OUString s_sQueryBaseName( RTL_CONSTASCII_USTRINGPARAM( "query" ) );

            switch ( i_eType )
            {
            case FORM:
                return s_sFormBaseName;
            case REPORT:
                return s_sReportBaseName;
            case TABLE:
                return s_sTableBaseName;
            case QUERY:
                return s_sQueryBaseName;
            default:
                break;
            }

            OSL_ENSURE( false, "lcl_getComponentStorageBaseName: unimplemented case!" );
            static const ::rtl::OUString s_sFallback;
            return s_sFallback;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getComponentsStorageName( const SubComponentType i_eType )
        {
            static const ::rtl::OUString s_sFormsStorageName( RTL_CONSTASCII_USTRINGPARAM( "forms" ) );
            static const ::rtl::OUString s_sReportsStorageName( RTL_CONSTASCII_USTRINGPARAM( "reports" ) );
            static const ::rtl::OUString s_sTablesStorageName( RTL_CONSTASCII_USTRINGPARAM( "tables" ) );
            static const ::rtl::OUString s_sQueriesStorageName( RTL_CONSTASCII_USTRINGPARAM( "queries" ) );
            static const ::rtl::OUString s_sRelationsStorageName( RTL_CONSTASCII_USTRINGPARAM( "relations" ) );

            switch ( i_eType )
            {
            case FORM:
                return s_sFormsStorageName;
            case REPORT:
                return s_sReportsStorageName;
            case TABLE:
                return s_sTablesStorageName;
            case QUERY:
                return s_sQueriesStorageName;
            case RELATION_DESIGN:
                return s_sRelationsStorageName;
            default:
                break;
            }

            OSL_ENSURE( false, "lcl_getComponentsStorageName: unimplemented case!" );
            static const ::rtl::OUString s_sFallback;
            return s_sFallback;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getObjectMapStreamName()
        {
            static const ::rtl::OUString s_sObjectMapStreamName( RTL_CONSTASCII_USTRINGPARAM( "storage-component-map.ini" ) );
            return s_sObjectMapStreamName;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getSettingsStreamName()
        {
            static const ::rtl::OUString s_sStatementStreamName( RTL_CONSTASCII_USTRINGPARAM( "settings.xml" ) );
            return s_sStatementStreamName;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getCurrentQueryDesignName()
        {
            static const ::rtl::OUString s_sQuerySettingsName( RTL_CONSTASCII_USTRINGPARAM( "ooo:current-query-design" ) );
            return s_sQuerySettingsName;
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
                OSL_ENSURE( false, "lcl_readObjectMap_throw: if there's no map file, then there's expected to be no storage, too!" );
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
                if ( !sCurrentSection.equalsAscii( "storages" ) )
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
        static bool lcl_determineReadOnly( const Reference< XComponent >& i_rComponent )
        {
            Reference< XModel > xDocument( i_rComponent, UNO_QUERY );
            if ( !xDocument.is() )
            {
                Reference< XController > xController( i_rComponent, UNO_QUERY_THROW );
                xDocument = xController->getModel();
            }

            if ( !xDocument.is() )
                return false;

            ::comphelper::NamedValueCollection aDocArgs( xDocument->getArgs() );
            return aDocArgs.getOrDefault( "ReadOnly", false );
        }

        // .........................................................................
        static void lcl_markModified( const Reference< XComponent >& i_rSubComponent )
        {
            const Reference< XModifiable > xModify( i_rSubComponent, UNO_QUERY );
            if ( !xModify.is() )
            {
                OSL_ENSURE( false, "lcl_markModified: unhandled case!" );
                return;
            }

            xModify->setModified( sal_True );
        }

        // .........................................................................
        static Reference< XCommandProcessor > lcl_getSubComponentDef_nothrow( const Reference< XController >& i_rAppUI,
            SubComponentType i_eType, const ::rtl::OUString& i_rName )
        {
            ENSURE_OR_RETURN( i_rAppUI.is(), "lcl_getSubComponentDef_nothrow: illegal controller", NULL );
            ENSURE_OR_RETURN( ( i_eType == FORM ) || ( i_eType == REPORT ), "lcl_getSubComponentDef_nothrow: illegal controller", NULL );

            Reference< XCommandProcessor > xCommandProcessor;
            try
            {
                Reference< XHierarchicalNameAccess > xDefinitionContainer;
                if ( i_eType == FORM )
                {
                    Reference< XFormDocumentsSupplier > xSuppForms( i_rAppUI->getModel(), UNO_QUERY_THROW );
                    xDefinitionContainer.set( xSuppForms->getFormDocuments(), UNO_QUERY_THROW );
                }
                else
                {
                    Reference< XReportDocumentsSupplier > xSuppReports( i_rAppUI->getModel(), UNO_QUERY_THROW );
                    xDefinitionContainer.set( xSuppReports->getReportDocuments(), UNO_QUERY_THROW );
                }
                xCommandProcessor.set( xDefinitionContainer->getByHierarchicalName( i_rName ), UNO_QUERY_THROW );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return xCommandProcessor;
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
    //= SettingsExportContext
    //====================================================================
    class DBACCESS_DLLPRIVATE SettingsExportContext : public ::xmloff::XMLSettingsExportContext
    {
    public:
        SettingsExportContext( const ::comphelper::ComponentContext& i_rContext, const StorageXMLOutputStream& i_rDelegator )
            :m_rContext( i_rContext )
            ,m_rDelegator( i_rDelegator )
            ,m_aNamespace( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_NP_CONFIG ) )
        {
        }

    public:
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, const ::rtl::OUString& i_rValue );
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, enum ::xmloff::token::XMLTokenEnum i_eValue );
        virtual void    StartElement( enum ::xmloff::token::XMLTokenEnum i_eName, const sal_Bool i_bIgnoreWhitespace );
        virtual void    EndElement  ( const sal_Bool i_bIgnoreWhitespace );
        virtual void    Characters( const ::rtl::OUString& i_rCharacters );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        GetServiceFactory() const;

    private:
        ::rtl::OUString impl_prefix( const ::xmloff::token::XMLTokenEnum i_eToken )
        {
            ::rtl::OUStringBuffer aQualifiedName( m_aNamespace );
            aQualifiedName.append( sal_Unicode( ':' ) );
            aQualifiedName.append( ::xmloff::token::GetXMLToken( i_eToken ) );
            return aQualifiedName.makeStringAndClear();
        }

    private:
        const ::comphelper::ComponentContext&   m_rContext;
        const StorageXMLOutputStream&           m_rDelegator;
        const ::rtl::OUStringBuffer             m_aNamespace;
    };

    //--------------------------------------------------------------------
    void SettingsExportContext::AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, const ::rtl::OUString& i_rValue )
    {
        m_rDelegator.addAttribute( impl_prefix( i_eName ), i_rValue );
    }

    //--------------------------------------------------------------------
    void SettingsExportContext::AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, enum ::xmloff::token::XMLTokenEnum i_eValue )
    {
        m_rDelegator.addAttribute( impl_prefix( i_eName ), ::xmloff::token::GetXMLToken( i_eValue ) );
    }

    //--------------------------------------------------------------------
    void SettingsExportContext::StartElement( enum ::xmloff::token::XMLTokenEnum i_eName, const sal_Bool i_bIgnoreWhitespace )
    {
        if ( i_bIgnoreWhitespace )
            m_rDelegator.ignorableWhitespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );

        m_rDelegator.startElement( impl_prefix( i_eName ) );
    }

    //--------------------------------------------------------------------
    void SettingsExportContext::EndElement( const sal_Bool i_bIgnoreWhitespace )
    {
        if ( i_bIgnoreWhitespace )
            m_rDelegator.ignorableWhitespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );
        m_rDelegator.endElement();
    }

    //--------------------------------------------------------------------
    void SettingsExportContext::Characters( const ::rtl::OUString& i_rCharacters )
    {
        m_rDelegator.characters( i_rCharacters );
    }

    //--------------------------------------------------------------------
    Reference< XMultiServiceFactory > SettingsExportContext::GetServiceFactory() const
    {
        return m_rContext.getLegacyServiceFactory();
    }

    //====================================================================
    //= SubComponentRecovery
    //====================================================================
    class DBACCESS_DLLPRIVATE SubComponentRecovery
    {
    public:
        SubComponentRecovery( const ::comphelper::ComponentContext& i_rContext, const Reference< XDatabaseDocumentUI >& i_rController,
                const Reference< XComponent >& i_rComponent )
            :m_rContext( i_rContext )
            ,m_xComponent( i_rComponent )
            ,m_eType( UNKNOWN )
            ,m_aCompDesc()
        {
            impl_identifyComponent_throw( i_rController );
        }

        void saveToRecoveryStorage( const Reference< XStorage >& i_rRecoveryStorage, MapCompTypeToCompDescs& io_mapCompDescs );

    private:
        void    impl_saveSubDocument_throw(
                    const Reference< XStorage >& i_rObjectStorage
                );

        void    impl_saveQueryDesign_throw(
                    const Reference< XStorage >& i_rObjectStorage
                );

        void    impl_identifyComponent_throw(
                    const Reference< XDatabaseDocumentUI >& i_rController
                );

    private:
        const ::comphelper::ComponentContext&   m_rContext;
        const Reference< XComponent >           m_xComponent;
        SubComponentType                        m_eType;
        SubComponentDescriptor                  m_aCompDesc;
    };

    //--------------------------------------------------------------------
    void SubComponentRecovery::saveToRecoveryStorage( const Reference< XStorage >& i_rRecoveryStorage,
        MapCompTypeToCompDescs& io_mapCompDescs )
    {

        if ( m_eType == UNKNOWN )
            // quite fatal, but has already been reported (as assertion) before
            return;

        // open the sub storage for the given kind of components
        const ::rtl::OUString& rStorageName( lcl_getComponentsStorageName( m_eType ) );
        const Reference< XStorage > xComponentsStorage( i_rRecoveryStorage->openStorageElement(
            rStorageName, ElementModes::READWRITE ), UNO_QUERY_THROW );

        // find a free sub storage name, and create Yet Another Sub Storage
        const ::rtl::OUString& rBaseName( lcl_getComponentStorageBaseName( m_eType ) );
        const ::rtl::OUString sStorName = ::dbtools::createUniqueName( xComponentsStorage.get(), rBaseName, true );
        const Reference< XStorage > xObjectStor( xComponentsStorage->openStorageElement(
            sStorName, ElementModes::READWRITE ), UNO_QUERY_THROW );

        switch ( m_eType )
        {
        case FORM:
        case REPORT:
            impl_saveSubDocument_throw( xObjectStor );
            break;

        case QUERY:
            impl_saveQueryDesign_throw( xObjectStor );
            break;

        default:
            // TODO
            OSL_ENSURE( false, "SubComponentRecoverys::saveToRecoveryStorage: unimplemented case!" );
            break;
        }

        // commit the storage(s)
        tools::stor::commitStorageIfWriteable( xObjectStor );
        tools::stor::commitStorageIfWriteable( xComponentsStorage );

        // remember the relationship from the component name to the storage name
        MapStringToCompDesc& rMapCompDescs = io_mapCompDescs[ m_eType ];
        OSL_ENSURE( rMapCompDescs.find( sStorName ) == rMapCompDescs.end(),
            "SubComponentRecoverys::saveToRecoveryStorage: object name already used!" );
        rMapCompDescs[ sStorName ] = m_aCompDesc;
    }

    //--------------------------------------------------------------------
    void SubComponentRecovery::impl_identifyComponent_throw( const Reference< XDatabaseDocumentUI >& i_rController )
    {
        ENSURE_OR_THROW( i_rController.is(), "illegal controller" );

        // ask the controller
        Pair< sal_Int32, ::rtl::OUString > aComponentIdentity = i_rController->identifySubComponent( m_xComponent );
        m_eType = lcl_databaseObjectToSubComponentType( aComponentIdentity.First );
        m_aCompDesc.sName = aComponentIdentity.Second;

        // what the controller didn't give us is the information whether this is in edit mode or not ...
        Reference< XModuleManager > xModuleManager( m_rContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );
        const ::rtl::OUString sModuleIdentifier = xModuleManager->identify( m_xComponent );

        switch ( m_eType )
        {
        case TABLE:
            m_aCompDesc.bForEditing = sModuleIdentifier.equalsAscii( "com.sun.star.sdb.TableDesign" );
            break;

        case QUERY:
            m_aCompDesc.bForEditing = sModuleIdentifier.equalsAscii( "com.sun.star.sdb.QueryDesign" );
            break;

        case REPORT:
            if ( sModuleIdentifier.equalsAscii( "com.sun.star.report.ReportDefinition" ) )
            {
                // it's an SRB report desginer
                m_aCompDesc.bForEditing = true;
                break;
            }
            // fall through

        case FORM:
            m_aCompDesc.bForEditing = !lcl_determineReadOnly( m_xComponent );
            break;

        default:
            if ( sModuleIdentifier.equalsAscii( "com.sun.star.sdb.RelationDesign" ) )
            {
                m_eType = RELATION_DESIGN;
                m_aCompDesc.bForEditing = true;
            }
            else
            {
                OSL_ENSURE( false, "SubComponentRecovery::impl_identifyComponent_throw: couldn't classify the given sub component!" );
            }
            break;
        }

        OSL_POSTCOND( m_eType != UNKNOWN,
            "SubComponentRecovery::impl_identifyComponent_throw: couldn't classify the component!" );
    }

    //--------------------------------------------------------------------
    void SubComponentRecovery::impl_saveQueryDesign_throw( const Reference< XStorage >& i_rObjectStorage )
    {
        ENSURE_OR_THROW( m_eType == QUERY, "illegal sub component type" );
        ENSURE_OR_THROW( i_rObjectStorage.is(), "illegal storage" );

        // retrieve the current query design (which might differ from what we can retrieve as ActiveCommand property, since
        // the latter is updated only upon successful save of the design)
        Reference< XPropertySet > xDesignerProps( m_xComponent, UNO_QUERY_THROW );
        Sequence< PropertyValue > aCurrentQueryDesign;
        OSL_VERIFY( xDesignerProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CurrentQueryDesign" ) ) ) >>= aCurrentQueryDesign );

        // write the query design
        StorageXMLOutputStream aDesignOutput( m_rContext, i_rObjectStorage, lcl_getSettingsStreamName() );
        SettingsExportContext aSettingsExportContext( m_rContext, aDesignOutput );

        const ::rtl::OUString sWhitespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );

        aDesignOutput.startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "office:settings" ) ) );
        aDesignOutput.ignorableWhitespace( sWhitespace );

        XMLSettingsExportHelper aSettingsExporter( aSettingsExportContext );
        aSettingsExporter.exportAllSettings( aCurrentQueryDesign, lcl_getCurrentQueryDesignName() );

        aDesignOutput.ignorableWhitespace( sWhitespace );
        aDesignOutput.endElement();
        aDesignOutput.close();
    }

    //--------------------------------------------------------------------
    void SubComponentRecovery::impl_saveSubDocument_throw( const Reference< XStorage >& i_rObjectStorage )
    {
        ENSURE_OR_THROW( ( m_eType == FORM ) || ( m_eType == REPORT ), "illegal sub component type" );
        ENSURE_OR_THROW( i_rObjectStorage.is(), "illegal storage" );

        // store the document into the storage
        Reference< XStorageBasedDocument > xStorageDocument( m_xComponent, UNO_QUERY_THROW );
        xStorageDocument->storeToStorage( i_rObjectStorage, Sequence< PropertyValue >() );
    }

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
                    lcl_getComponentsStorageName( map->first ), ElementModes::WRITE | ElementModes::NOCREATE ) );
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
        for ( size_t i = 0; i < sizeof( aKnownTypes ) / sizeof( aKnownTypes[0] ); ++i )
        {
            if ( !xRecoveryStorage->hasByName( lcl_getComponentsStorageName( aKnownTypes[i] ) ) )
                continue;

            Reference< XStorage > xComponentsStor( xRecoveryStorage->openStorageElement(
                lcl_getComponentsStorageName( aKnownTypes[i] ), ElementModes::READ ) );
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
            if ( ( eComponentType != FORM ) && ( eComponentType != REPORT ) )
            {
                // nobody saves tables/queries/relations at the moment, so encountering those is worth an assertion
                OSL_ENSURE( false, "DatabaseDocumentRecovery::recoverSubDocuments: only embedded objects can be recovered currently!" );
                continue;
            }

            // the storage for all components of the current type
            Reference< XStorage > xComponentsStor( xRecoveryStorage->openStorageElement(
                lcl_getComponentsStorageName( eComponentType ), ElementModes::READ ), UNO_QUERY_THROW );

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
                    message.append( ::rtl::OUStringToOString( lcl_getComponentsStorageName( eComponentType ), RTL_TEXTENCODING_ASCII_US ) );
                    message.append( "', but required per map file!" );
                    OSL_ENSURE( false, message.makeStringAndClear() );
                #endif
                    continue;
                }

                // the controller needs to have a connection to be able to open sub components
                if ( !xDocumentUI->isConnected() )
                    xDocumentUI->connect();

                // recover the single component
                Reference< XStorage > xCompStor( xComponentsStor->openStorageElement( stor->first, ElementModes::READ ) );
                ::comphelper::NamedValueCollection aLoadArgs;
                aLoadArgs.put( "RecoveryStorage", xCompStor );

                // load/create the sub component hidden. We'll show it when the main app window is shown.
                aLoadArgs.put( "Hidden", true );

                Reference< XComponent > xSubComponent;
                Reference< XCommandProcessor > xDocDefinition;

                if ( sComponentName.getLength() )
                {
                    xDocDefinition = lcl_getSubComponentDef_nothrow( i_rTargetController, eComponentType, sComponentName );
                    xSubComponent.set( xDocumentUI->loadComponentWithArguments(
                            eComponentType,
                            sComponentName,
                            stor->second.bForEditing,
                            aLoadArgs.getPropertyValues()
                        ),
                        UNO_SET_THROW
                    );
                }
                else
                {
                    Reference< XComponent > xDocDefComponent;
                    xSubComponent.set( xDocumentUI->createComponentWithArguments(
                            eComponentType,
                            aLoadArgs.getPropertyValues(),
                            xDocDefComponent
                        ),
                        UNO_SET_THROW
                    );

                    xDocDefinition.set( xDocDefComponent, UNO_QUERY );
                    OSL_ENSURE( xDocDefinition.is(), "DatabaseDocumentRecovery::recoverSubDocuments: loaded a form/report, but don't have a document definition?!" );
                }

                if ( xDocDefinition.is() )
                {
                    Reference< XInterface > xLoader( *new SubComponentLoader( i_rTargetController, xDocDefinition ) );
                    (void)xLoader;
                }

                // at the moment, we only store, during session save, sub components which are modified. So, set this
                // recovered sub component to "modified", too.
                lcl_markModified( xSubComponent );
            }

            xComponentsStor->dispose();
        }
    }

//........................................................................
} // namespace dbaccess
//........................................................................
