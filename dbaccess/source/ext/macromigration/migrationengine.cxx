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


#include "dbmm_global.hrc"
#include "dbmm_module.hxx"
#include "dbmm_types.hxx"
#include "docinteraction.hxx"
#include "migrationengine.hxx"
#include "migrationerror.hxx"
#include "migrationprogress.hxx"
#include "migrationlog.hxx"
#include "progresscapture.hxx"
#include "progressmixer.hxx"

#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/script/DocumentScriptLibraryContainer.hpp>
#include <com/sun/star/script/DocumentDialogLibraryContainer.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrlReference.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>

#include <comphelper/documentinfo.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <xmlscript/xmldlg_imexp.hxx>

#include <vector>
#include <set>
#include <iterator>

#define DEFAULT_DOC_PROGRESS_RANGE  100000

//........................................................................
namespace dbmm
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::sdb::XFormDocumentsSupplier;
    using ::com::sun::star::sdb::XReportDocumentsSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::util::CloseVetoException;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XComponentLoader;
    using ::com::sun::star::ucb::XCommandProcessor;
    using ::com::sun::star::ucb::XContent;
    using ::com::sun::star::ucb::Command;
    using ::com::sun::star::embed::XComponentSupplier;
    using ::com::sun::star::task::XStatusIndicator;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::document::XStorageBasedDocument;
    using ::com::sun::star::embed::XTransactedObject;
    using ::com::sun::star::frame::XStorable;
    using ::com::sun::star::embed::XEmbedPersist;
    using ::com::sun::star::script::DocumentDialogLibraryContainer;
    using ::com::sun::star::script::DocumentScriptLibraryContainer;
    using ::com::sun::star::script::XStorageBasedLibraryContainer;
    using ::com::sun::star::document::XEmbeddedScripts;
    using ::com::sun::star::container::XNameContainer;
    using ::com::sun::star::document::XEventsSupplier;
    using ::com::sun::star::container::XNameReplace;
    using com::sun::star::uri::UriReferenceFactory;
    using com::sun::star::uri::XUriReferenceFactory;
    using com::sun::star::uri::XVndSunStarScriptUrlReference;
    using ::com::sun::star::form::XFormsSupplier;
    using ::com::sun::star::drawing::XDrawPageSupplier;
    using ::com::sun::star::drawing::XDrawPagesSupplier;
    using ::com::sun::star::drawing::XDrawPage;
    using ::com::sun::star::drawing::XDrawPages;
    using ::com::sun::star::container::XIndexAccess;
    using ::com::sun::star::script::XEventAttacherManager;
    using ::com::sun::star::script::ScriptEventDescriptor;
    using ::com::sun::star::script::XLibraryContainerPassword;
    using ::com::sun::star::io::WrongFormatException;
    using ::com::sun::star::script::XScriptEventsSupplier;
    using ::com::sun::star::io::XInputStreamProvider;
    using ::com::sun::star::io::XInputStream;

    namespace ElementModes = ::com::sun::star::embed::ElementModes;

// migration phases whose progresses are to be mixed into one progress
#define PHASE_JAVASCRIPT    1
#define PHASE_BEANSHELL     2
#define PHASE_PYTHON        3
#define PHASE_JAVA          4
#define PHASE_BASIC         5
#define PHASE_DIALOGS       6

    //====================================================================
    //= SubDocument
    //====================================================================
    struct SubDocument
    {
        Reference< XCommandProcessor >  xCommandProcessor;
        Reference< XModel >             xDocument;          // valid only temporarily
        OUString                        sHierarchicalName;
        SubDocumentType                 eType;
        size_t                          nNumber;

        SubDocument( const Reference< XCommandProcessor >& _rxCommandProcessor, const OUString& _rName,
                const SubDocumentType _eType, const size_t _nNumber )
            :xCommandProcessor( _rxCommandProcessor )
            ,xDocument()
            ,sHierarchicalName( _rName )
            ,eType( _eType )
            ,nNumber( _nNumber )
        {
        }
    };

    typedef ::std::vector< SubDocument >    SubDocuments;

    //====================================================================
    //= helper
    //====================================================================
    //--------------------------------------------------------------------
    typedef ::utl::SharedUNOComponent< XStorage >   SharedStorage;

    namespace
    {
        //----------------------------------------------------------------
        static const OUString& lcl_getScriptsStorageName()
        {
            static const OUString s_sScriptsStorageName( "Scripts" );
            return s_sScriptsStorageName;
        }

        //----------------------------------------------------------------
        static const OUString& lcl_getScriptsSubStorageName( const ScriptType _eType )
        {
            static const OUString s_sBeanShell ( "beanshell" );
            static const OUString s_sJavaScript( "javascript" );
            static const OUString s_sPython    ( "python" );      // TODO: is this correct?
            static const OUString s_sJava      ( "java" );

            switch ( _eType )
            {
            case eBeanShell:    return s_sBeanShell;
            case eJavaScript:   return s_sJavaScript;
            case ePython:       return s_sPython;
            case eJava:         return s_sJava;
            default:
                break;
            }

            OSL_FAIL( "lcl_getScriptsSubStorageName: illegal type!" );
            static OUString s_sEmpty;
            return s_sEmpty;
        }

        //----------------------------------------------------------------
        static bool lcl_getScriptTypeFromLanguage( const OUString& _rLanguage, ScriptType& _out_rScriptType )
        {
            struct LanguageMapping
            {
                const sal_Char*     pAsciiLanguage;
                const ScriptType    eScriptType;

                LanguageMapping( const sal_Char* _pAsciiLanguage, const ScriptType _eScriptType )
                    :pAsciiLanguage( _pAsciiLanguage )
                    ,eScriptType( _eScriptType )
                {
                }
            }
            aLanguageMapping[] =
            {
                LanguageMapping( "JavaScript", eJavaScript ),
                LanguageMapping( "BeanShell",  eBeanShell ),
                LanguageMapping( "Java",       eJava ),
                LanguageMapping( "Python",     ePython ),          // TODO: is this correct?
                LanguageMapping( "Basic",      eBasic )
            };
            for ( size_t i=0; i < sizeof( aLanguageMapping ) / sizeof( aLanguageMapping[0] ); ++i )
            {
                if ( _rLanguage.equalsAscii( aLanguageMapping[i].pAsciiLanguage ) )
                {
                    _out_rScriptType = aLanguageMapping[i].eScriptType;
                    return true;
                }
            }
            OSL_FAIL( "lcl_getScriptTypeFromLanguage: unknown language!" );
            return false;
        }

        //----------------------------------------------------------------
        OUString lcl_getSubDocumentDescription( const SubDocument& _rDocument )
        {
            OUString sObjectName(
                    MacroMigrationResId(
                        _rDocument.eType == eForm ? STR_FORM : STR_REPORT).toString().
                replaceFirst("$name$", _rDocument.sHierarchicalName));
            return sObjectName;
        }

        //----------------------------------------------------------------
        static Any lcl_executeCommand_throw( const Reference< XCommandProcessor >& _rxCommandProc,
            const sal_Char* _pAsciiCommand )
        {
            OSL_PRECOND( _rxCommandProc.is(), "lcl_executeCommand_throw: illegal object!" );
            if ( !_rxCommandProc.is() )
                return Any();

            Command aCommand;
            aCommand.Name = OUString::createFromAscii( _pAsciiCommand );
            return _rxCommandProc->execute(
                aCommand, _rxCommandProc->createCommandIdentifier(), NULL );
        }

        //----------------------------------------------------------------
        OUString lcl_getMimeType_nothrow( const Reference< XCommandProcessor >& _rxContent )
        {
            OUString sMimeType;
            try
            {
                Reference< XContent > xContent( _rxContent, UNO_QUERY_THROW );
                sMimeType = xContent->getContentType();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return sMimeType;
        }

        //----------------------------------------------------------------
        enum OpenDocResult
        {
            eOpenedDoc,
            eIgnoreDoc,
            eFailure
        };

        //----------------------------------------------------------------
        static OpenDocResult lcl_loadSubDocument_nothrow( SubDocument& _rDocument,
            const Reference< XStatusIndicator >& _rxProgress, MigrationLog& _rLogger )
        {
            OSL_PRECOND( !_rDocument.xDocument.is(), "lcl_loadSubDocument_nothrow: already loaded!" );

            try
            {
                ::comphelper::NamedValueCollection aLoadArgs;
                aLoadArgs.put( "Hidden", (sal_Bool)sal_True );
                aLoadArgs.put( "StatusIndicator", _rxProgress );

                Reference< XCommandProcessor > xCommandProcessor( _rDocument.xCommandProcessor, UNO_SET_THROW );
                Command aCommand;
                aCommand.Name = OUString( "openDesign" );
                aCommand.Argument <<= aLoadArgs.getPropertyValues();
                Reference< XComponent > xDocComponent(
                    xCommandProcessor->execute(
                        aCommand, xCommandProcessor->createCommandIdentifier(), NULL
                    ),
                    UNO_QUERY
                );
                OSL_ENSURE( xDocComponent.is(), "lcl_loadSubDocument_nothrow: no component loaded!" );

                _rDocument.xDocument.set( xDocComponent, UNO_QUERY_THROW );
            }
            catch( const Exception& )
            {
                Any aError( ::cppu::getCaughtException() );

                bool bCausedByNewStyleReport =
                        ( _rDocument.eType == eReport )
                    &&  ( aError.isExtractableTo( ::cppu::UnoType< WrongFormatException >::get() ) )
                    &&  ( lcl_getMimeType_nothrow( _rDocument.xCommandProcessor ) == "application/vnd.sun.xml.report" );

                if ( bCausedByNewStyleReport )
                {
                    _rLogger.logRecoverable( MigrationError(
                        ERR_NEW_STYLE_REPORT,
                        lcl_getSubDocumentDescription( _rDocument )
                    ) );
                    return eIgnoreDoc;
                }
                else
                {
                    _rLogger.logFailure( MigrationError(
                        ERR_OPENING_SUB_DOCUMENT_FAILED,
                        lcl_getSubDocumentDescription( _rDocument ),
                        aError
                    ) );
                }
            }
            return _rDocument.xDocument.is() ? eOpenedDoc : eFailure;
        }

        //----------------------------------------------------------------
        static bool lcl_unloadSubDocument_nothrow( SubDocument& _rDocument, MigrationLog& _rLogger )
        {
            bool bSuccess = false;
            Any aException;
            try
            {
                OSL_VERIFY( lcl_executeCommand_throw( _rDocument.xCommandProcessor, "close" ) >>= bSuccess );
            }
            catch( const Exception& )
            {
                aException = ::cppu::getCaughtException();
            }

            // log the failure, if any
            if ( !bSuccess )
            {
                _rLogger.logFailure( MigrationError(
                    ERR_CLOSING_SUB_DOCUMENT_FAILED,
                    lcl_getSubDocumentDescription( _rDocument ),
                    aException
                ) );
            }

            _rDocument.xDocument.clear();
            return bSuccess;
        }

        //----------------------------------------------------------------
        bool lcl_commitStorage_nothrow( const Reference< XStorage >& _rxStorage )
        {
            try
            {
                Reference< XTransactedObject > xTrans( _rxStorage, UNO_QUERY_THROW );
                xTrans->commit();
            }
            catch( const Exception& )
            {
                return false;
            }
            return true;
        }

        //----------------------------------------------------------------
        bool lcl_commitDocumentStorage_nothrow( const Reference< XModel >& _rxDocument, MigrationLog& _rLogger )
        {
            bool bSuccess = false;
            Any aException;
            try
            {
                Reference< XStorageBasedDocument > xStorageDoc( _rxDocument, UNO_QUERY_THROW );
                Reference< XStorage > xDocStorage( xStorageDoc->getDocumentStorage(), UNO_QUERY_THROW );
                bSuccess = lcl_commitStorage_nothrow( xDocStorage );
            }
            catch( const Exception& )
            {
                aException = ::cppu::getCaughtException();
            }

            // log the failure, if any
            if ( !bSuccess )
            {
                _rLogger.logFailure( MigrationError(
                    ERR_STORAGE_COMMIT_FAILED,
                    ::comphelper::DocumentInfo::getDocumentTitle( _rxDocument ),
                    aException
                ) );
            }
            return bSuccess;
        }

        //----------------------------------------------------------------
        bool lcl_storeDocument_nothrow( const Reference< XModel >& _rxDocument, MigrationLog& _rLogger )
        {
            bool bSuccess = false;
            Any aException;
            try
            {
                Reference< XStorable > xStorable( _rxDocument, UNO_QUERY_THROW );
                xStorable->store();
                bSuccess = true;
            }
            catch( const Exception& )
            {
                aException = ::cppu::getCaughtException();
            }

            // log the failure, if any
            if ( !bSuccess )
            {
                _rLogger.logFailure( MigrationError(
                    ERR_STORING_DATABASEDOC_FAILED,
                    aException
                ) );
            }
            return bSuccess;
        }

        //----------------------------------------------------------------
        bool lcl_storeEmbeddedDocument_nothrow( const SubDocument& _rDocument )
        {
            try
            {
                lcl_executeCommand_throw( _rDocument.xCommandProcessor, "store" );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
                return false;
            }
            return true;
        }
    }

    //====================================================================
    //= DrawPageIterator
    //====================================================================
    class DrawPageIterator
    {
    public:
        DrawPageIterator( const Reference< XModel >& _rxDocument )
            :m_xDocument( _rxDocument )
            ,m_nPageCount( 0 )
            ,m_nCurrentPage( 0 )
        {
            Reference< XDrawPageSupplier > xSingle( _rxDocument, UNO_QUERY );
            Reference< XDrawPagesSupplier > xMulti( _rxDocument, UNO_QUERY );
            if ( xSingle.is() )
            {
                m_xSinglePage.set( xSingle->getDrawPage(), UNO_SET_THROW );
                m_nPageCount = 1;
            }
            else if ( xMulti.is() )
            {
                m_xMultiPages.set( xMulti->getDrawPages(), UNO_SET_THROW );
                m_nPageCount = m_xMultiPages->getCount();
            }
        }

        bool hasMore() const
        {
            return m_nCurrentPage < m_nPageCount;
        }

        Reference< XDrawPage > next()
        {
            Reference< XDrawPage > xNextPage;

            if ( m_xSinglePage.is() )
            {
                xNextPage = m_xSinglePage;
            }
            else if ( m_xMultiPages.is() )
            {
                xNextPage.set( m_xMultiPages->getByIndex( m_nCurrentPage ), UNO_QUERY_THROW );
            }
            ++m_nCurrentPage;
            return xNextPage;
        }

    private:
        const Reference< XModel >   m_xDocument;
        Reference< XDrawPage >      m_xSinglePage;
        Reference< XDrawPages >     m_xMultiPages;
        sal_Int32                   m_nPageCount;
        sal_Int32                   m_nCurrentPage;
    };

    //====================================================================
    //= FormComponentScripts
    //====================================================================
    class FormComponentScripts
    {
    public:
        FormComponentScripts(
                const Reference< XInterface >& _rxComponent,
                const Reference< XEventAttacherManager >& _rxManager,
                const sal_Int32 _nIndex
            )
            :m_xComponent( _rxComponent )
            ,m_xManager( _rxManager )
            ,m_nIndex( _nIndex )
        {
        }

        Sequence< ScriptEventDescriptor > getEvents() const
        {
            return m_xManager->getScriptEvents( m_nIndex );
        }

        void setEvents( const Sequence< ScriptEventDescriptor >& _rEvents  ) const
        {
            m_xManager->registerScriptEvents( m_nIndex, _rEvents );
        }

        const Reference< XInterface >& getComponent() const
        {
            return m_xComponent;
        }

    private:
        const Reference< XInterface >               m_xComponent;
        const Reference< XEventAttacherManager >    m_xManager;
        const sal_Int32                             m_nIndex;
    };

    //====================================================================
    //= FormComponentIterator
    //====================================================================
    class FormComponentIterator
    {
    public:
        FormComponentIterator( const Reference< XIndexAccess >& _rxContainer )
            :m_xContainer( _rxContainer )
            ,m_xEventManager( _rxContainer, UNO_QUERY_THROW )
            ,m_nElementCount( _rxContainer->getCount() )
            ,m_nCurrentElement( 0 )
        {
        }

        bool hasMore() const
        {
            return m_nCurrentElement < m_nElementCount;
        }

        FormComponentScripts next()
        {
            FormComponentScripts aComponent(
                Reference< XInterface >( m_xContainer->getByIndex( m_nCurrentElement ), UNO_QUERY_THROW ),
                m_xEventManager,
                m_nCurrentElement
            );
            ++m_nCurrentElement;
            return aComponent;
        }

    private:
        const Reference< XIndexAccess >             m_xContainer;
        const Reference< XEventAttacherManager >    m_xEventManager;
        const sal_Int32                             m_nElementCount;
        sal_Int32                                   m_nCurrentElement;

    };

    //====================================================================
    //= ScriptsStorage - declaration
    //====================================================================
    /** a helper class which encapsulates access to the storages for Java/Script, BeanShell, and Python scripts,
        i.e. all script types which can be manipulated on storage level.
    */
    class ScriptsStorage
    {
    public:
        ScriptsStorage( MigrationLog& _rLogger );
        ScriptsStorage( const Reference< XModel >& _rxDocument, MigrationLog& _rLogger );
        ~ScriptsStorage();

        /** determines whether the instance is valid, i.e. refers to a valid root storage
            for reading/storing scripts
        */
        inline bool isValid() const { return m_xScriptsStorage.is(); }

        /** binds the instance to a new document. Only to be called when the instance is not yet
            bound (i.e. isValid returns <FALSE/>).
        */
        void    bind( const Reference< XModel >& _rxDocument );

        /// determines whether scripts of the given type are present
        bool    hasScripts( const ScriptType _eType ) const;

        /// returns the root storage for the scripts of the given type
        SharedStorage
                getScriptsRoot( const ScriptType _eType ) const;

        /** returns the names of the elements in the "Scripts" storage
        */
        ::std::set< OUString >
                getElementNames() const;

        /** removes the sub storage for a given script type
            @precond
                the respective storage is empty
            @precond
                the ScriptsStorage instance was opened for writing
        */
        void    removeScriptTypeStorage( const ScriptType _eType ) const;

        /** commits the changes at our XStorage object
        */
        bool    commit();

        /** removes the "Scripts" sub storage from the given document's root storage
            @precond
                the "Scripts" storage is empty
        */
        static bool
                removeFromDocument( const Reference< XModel >& _rxDocument, MigrationLog& _rLogger );

    private:
        MigrationLog&   m_rLogger;
        SharedStorage   m_xScriptsStorage;
    };

    //====================================================================
    //= ScriptsStorage - implementation
    //====================================================================
    //--------------------------------------------------------------------
    ScriptsStorage::ScriptsStorage( MigrationLog& _rLogger )
        :m_rLogger( _rLogger )
        ,m_xScriptsStorage()
    {
    }

    //--------------------------------------------------------------------
    ScriptsStorage::ScriptsStorage( const Reference< XModel >& _rxDocument, MigrationLog& _rLogger )
        :m_rLogger( _rLogger )
        ,m_xScriptsStorage()
    {
        bind( _rxDocument );
    }

    //--------------------------------------------------------------------
    ScriptsStorage::~ScriptsStorage()
    {
    }

    //--------------------------------------------------------------------
    bool ScriptsStorage::commit()
    {
        return lcl_commitStorage_nothrow( m_xScriptsStorage );
    }

    //--------------------------------------------------------------------
    void ScriptsStorage::bind( const Reference< XModel >& _rxDocument )
    {
        OSL_PRECOND( !isValid(), "ScriptsStorage:bind: did not bother, yet, to check whether this is allowed!" );
        try
        {
            Reference< XStorageBasedDocument > xStorageDoc( _rxDocument, UNO_QUERY_THROW );
            Reference< XStorage > xDocStorage( xStorageDoc->getDocumentStorage(), UNO_QUERY_THROW );

            // the "Scripts" storage exist, or if it does not (yet) exist and we are in write mode
            // => open the storage
            if  (   (   xDocStorage->hasByName( lcl_getScriptsStorageName() )
                    &&  xDocStorage->isStorageElement( lcl_getScriptsStorageName() )
                    )
                ||  !xDocStorage->hasByName( lcl_getScriptsStorageName() )
                )
            {
                m_xScriptsStorage.set(
                    xDocStorage->openStorageElement(
                        lcl_getScriptsStorageName(), ElementModes::READWRITE
                    ),
                    UNO_QUERY_THROW
                );
            }
        }
        catch( const Exception& )
        {
            m_rLogger.logFailure( MigrationError(
                ERR_BIND_SCRIPT_STORAGE_FAILED,
                ::comphelper::DocumentInfo::getDocumentTitle( _rxDocument ),
                ::cppu::getCaughtException()
            ) );
        }
    }

    //--------------------------------------------------------------------
    bool ScriptsStorage::hasScripts( const ScriptType _eType ) const
    {
        OSL_PRECOND( isValid(), "ScriptsStorage::hasScripts: illegal call!" );
        if ( !isValid() )
            return false;

        const OUString& rSubStorageName( lcl_getScriptsSubStorageName( _eType ) );
        return  m_xScriptsStorage->hasByName( rSubStorageName )
            &&  m_xScriptsStorage->isStorageElement( rSubStorageName );
    }

    //--------------------------------------------------------------------
    SharedStorage ScriptsStorage::getScriptsRoot( const ScriptType _eType ) const
    {
        SharedStorage xStorage;
        if ( isValid() )
        {
            xStorage.reset( m_xScriptsStorage->openStorageElement(
                lcl_getScriptsSubStorageName( _eType ), ElementModes::READWRITE
            ) );
        }
        return xStorage;
    }

    //--------------------------------------------------------------------
    ::std::set< OUString > ScriptsStorage::getElementNames() const
    {
        Sequence< OUString > aElementNames;
        if ( isValid() )
            aElementNames = m_xScriptsStorage->getElementNames();

        ::std::set< OUString > aNames;
        ::std::copy(
            aElementNames.getConstArray(),
            aElementNames.getConstArray() + aElementNames.getLength(),
            ::std::insert_iterator< ::std::set< OUString > >( aNames, aNames.end() )
        );
        return aNames;
    }

    //--------------------------------------------------------------------
    void ScriptsStorage::removeScriptTypeStorage( const ScriptType _eType ) const
    {
        OUString sSubStorageName( lcl_getScriptsSubStorageName( _eType ) );
        if ( m_xScriptsStorage->hasByName( sSubStorageName ) )
            m_xScriptsStorage->removeElement( sSubStorageName );
    }

    //--------------------------------------------------------------------
    bool ScriptsStorage::removeFromDocument( const Reference< XModel >& _rxDocument, MigrationLog& _rLogger )
    {
        try
        {
            Reference< XStorageBasedDocument > xStorageDoc( _rxDocument, UNO_QUERY_THROW );
            Reference< XStorage > xDocStorage( xStorageDoc->getDocumentStorage(), UNO_QUERY_THROW );
            xDocStorage->removeElement( lcl_getScriptsStorageName() );
        }
        catch( const Exception& )
        {
            _rLogger.logFailure( MigrationError(
                ERR_REMOVE_SCRIPTS_STORAGE_FAILED,
                ::comphelper::DocumentInfo::getDocumentTitle( _rxDocument ),
                ::cppu::getCaughtException()
            ) ) ;
            return false;
        }
        return true;
    }

    //====================================================================
    //= ProgressDelegator
    //====================================================================
    class ProgressDelegator : public IProgressConsumer
    {
    public:
        ProgressDelegator(  IMigrationProgress& _rDelegator,
                            const OUString& _rObjectName,
                            const OUString& _rAction
                          )
            :m_rDelegator( _rDelegator )
            ,m_sObjectName( _rObjectName )
            ,m_sAction( _rAction )
        {
        }
        virtual ~ProgressDelegator()
        {
        }

        // IProgressConsumer
        virtual void    start( sal_uInt32 _nRange )
        {
            m_rDelegator.startObject( m_sObjectName, m_sAction, _nRange );
        }
        virtual void    advance( sal_uInt32 _nValue )
        {
            m_rDelegator.setObjectProgressValue( _nValue );
        }
        virtual void    end()
        {
            m_rDelegator.endObject();
        }

    private:
        IMigrationProgress& m_rDelegator;
        OUString     m_sObjectName;
        OUString     m_sAction;
    };

    //====================================================================
    //= PhaseGuard
    //====================================================================
    class PhaseGuard
    {
    public:
        PhaseGuard( ProgressMixer& _rMixer )
            :m_rMixer( _rMixer )
        {
        }

        PhaseGuard( ProgressMixer& _rMixer, const PhaseID _nID, const sal_uInt32 _nPhaseRange )
            :m_rMixer( _rMixer )
        {
            start( _nID, _nPhaseRange );
        }

        ~PhaseGuard()
        {
            m_rMixer.endPhase();
        }

        void start( const PhaseID _nID, const sal_uInt32 _nPhaseRange )
        {
            m_rMixer.startPhase( _nID, _nPhaseRange );
        }

    private:
        ProgressMixer&  m_rMixer;
    };

    //====================================================================
    //= MigrationEngine_Impl - declaration
    //====================================================================
    class MigrationEngine_Impl
    {
    public:
        MigrationEngine_Impl(
            const Reference<XComponentContext>& _rContext,
            const Reference< XOfficeDatabaseDocument >& _rxDocument,
            IMigrationProgress& _rProgress,
            MigrationLog& _rLogger
        );
        ~MigrationEngine_Impl();

        inline  size_t      getFormCount() const    { return m_nFormCount; }
        inline  size_t      getReportCount()const   { return m_nReportCount; }
        bool    migrateAll();

    private:
        Reference<XComponentContext>              m_aContext;
        const Reference< XOfficeDatabaseDocument >  m_xDocument;
        const Reference< XModel >                   m_xDocumentModel;
        IMigrationProgress&                         m_rProgress;
        MigrationLog&                               m_rLogger;
        mutable DocumentID                          m_nCurrentDocumentID;
        SubDocuments                                m_aSubDocs;
        size_t                                      m_nFormCount;
        size_t                                      m_nReportCount;

    private:
        /** collects a description of all sub documents of our database document

            @return
                <TRUE/> if and only if collecting the documents was successful
        */
        bool    impl_collectSubDocuments_nothrow();

        /** migrates the macros/scripts of the given sub document
        */
        bool    impl_handleDocument_nothrow( const SubDocument& _rDocument ) const;

        /** checks the structure of the 'Scripts' folder of a sub document
            for unknown elements

            @return
                <TRUE/> if and only if the 'Scripts' folder contains known elements only.
        */
        bool    impl_checkScriptStorageStructure_nothrow( const SubDocument& _rDocument ) const;

        /** migrates the scripts of the given "storage-based" script type
        */
        bool    impl_migrateScriptStorage_nothrow(
                    const SubDocument& _rDocument,
                    const ScriptType _eScriptType,
                    ProgressMixer& _rProgress,
                    const PhaseID _nPhaseID
                ) const;

        /** migrates the content of the given "container based" libraries (Basic/Dialogs)
        */
        bool    impl_migrateContainerLibraries_nothrow(
                    const SubDocument& _rDocument,
                    const ScriptType _eScriptType,
                    ProgressMixer& _rProgress,
                    const PhaseID _nPhaseID
                ) const;

        /** adjusts the events for the given dialog/element, taking into account the new names
            of the moved libraries
        */
        void    impl_adjustDialogElementEvents_throw(
                    const Reference< XInterface >& _rxElement
                ) const;

        /** adjusts the events in the given dialog, and its controls, taking into account the new names
            of the moved libraries
        */
        bool    impl_adjustDialogEvents_nothrow(
                    Any& _inout_rDialogLibraryElement,
                    const OUString& _rDocName,
                    const OUString& _rDialogLibName,
                    const OUString& _rDialogName
                ) const;

        /** adjust the document-events which refer to macros/scripts in the document, taking into
            account the new names of the moved libraries
        */
        bool    impl_adjustDocumentEvents_nothrow(
                    const SubDocument& _rDocument
                ) const;

        /** adjusts the script references bound to form component events
        */
        bool    impl_adjustFormComponentEvents_nothrow(
                    const SubDocument& _rDocument
                ) const;

        /** adjusts the script references for the elements of the given form component container
        */
        void    impl_adjustFormComponentEvents_throw(
                    const Reference< XIndexAccess >& _rxComponentContainer
                ) const;

        /** adjusts the library name in the given script URL, so that it reflects
            the new name of the library

            @return <TRUE/>
                if and only if adjustments to the script code have been made
        */
        bool    impl_adjustScriptLibrary_nothrow(
                    const OUString& _rScriptType,
                    OUString& _inout_rScriptCode
                ) const;

        bool    impl_adjustScriptLibrary_nothrow( Any& _inout_rScriptDescriptor ) const;
        bool    impl_adjustScriptLibrary_nothrow( ScriptEventDescriptor& _inout_rScriptEvent ) const;

        /** asks the user for a password for the given library, and unprotects the library

            @return <TRUE/>
                if and only if the library could be successfully unprotected
        */
        bool    impl_unprotectPasswordLibrary_throw(
                    const Reference< XLibraryContainerPassword >& _rxPasswordManager,
                    const ScriptType _eScriptType,
                    const OUString& _rLibraryName
                ) const;
    };

    //====================================================================
    //= MigrationEngine_Impl - implementation
    //====================================================================
    //--------------------------------------------------------------------
    MigrationEngine_Impl::MigrationEngine_Impl( const Reference<XComponentContext>& _rContext,
            const Reference< XOfficeDatabaseDocument >& _rxDocument, IMigrationProgress& _rProgress, MigrationLog& _rLogger )
        :m_aContext( _rContext )
        ,m_xDocument( _rxDocument )
        ,m_xDocumentModel( _rxDocument, UNO_QUERY_THROW )
        ,m_rProgress( _rProgress )
        ,m_rLogger( _rLogger )
        ,m_nCurrentDocumentID( - 1 )
        ,m_aSubDocs()
        ,m_nFormCount( 0 )
        ,m_nReportCount( 0 )
    {
        OSL_VERIFY( impl_collectSubDocuments_nothrow() );
    }

    //--------------------------------------------------------------------
    MigrationEngine_Impl::~MigrationEngine_Impl()
    {
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::migrateAll()
    {
        if  ( m_aSubDocs.empty() )
        {
            OSL_FAIL( "MigrationEngine_Impl::migrateAll: no forms/reports found!" );
            // The whole migration wizard is not expected to be called when there are no forms/reports
            // with macros, not to mention when there are no forms/reports at all.
            return false;
        }

        // initialize global progress
        sal_Int32 nOverallRange( m_aSubDocs.size() );
        OUString sProgressSkeleton(
            MacroMigrationResId( STR_OVERALL_PROGRESS).toString().
            replaceFirst("$overall$", OUString::number(nOverallRange)));

        m_rProgress.start( nOverallRange );

        for (   SubDocuments::const_iterator doc = m_aSubDocs.begin();
                doc != m_aSubDocs.end();
                ++doc
            )
        {
            sal_Int32 nOverallProgressValue( doc - m_aSubDocs.begin() + 1 );
            // update overall progress text
            OUString sOverallProgress(
                sProgressSkeleton.replaceFirst("$current$",
                    OUString::number(nOverallProgressValue)));
            m_rProgress.setOverallProgressText( sOverallProgress );

            // migrate document
            if ( !impl_handleDocument_nothrow( *doc ) )
                return false;

            // update overall progress vallue
            m_rProgress.setOverallProgressValue( nOverallProgressValue );
        }

        // commit the root storage of the database document, for all changes made so far to take effect
        if ( !lcl_commitDocumentStorage_nothrow( m_xDocumentModel, m_rLogger ) )
            return false;

        // save the document
        if ( !lcl_storeDocument_nothrow( m_xDocumentModel, m_rLogger ) )
            return false;

        return true;
    }

    //--------------------------------------------------------------------
    namespace
    {
        void lcl_collectHierarchicalElementNames_throw(
            const Reference< XNameAccess >& _rxContainer, const OUString& _rContainerLoc,
            SubDocuments& _out_rDocs, const SubDocumentType _eType, size_t& _io_counter )
        {
            const OUString sHierarhicalBase(
                _rContainerLoc.isEmpty() ? OUString() :
                                           OUStringBuffer( _rContainerLoc ).appendAscii( "/" ).makeStringAndClear());

            Sequence< OUString > aElementNames( _rxContainer->getElementNames() );
            for (   const OUString* elementName = aElementNames.getConstArray();
                    elementName != aElementNames.getConstArray() + aElementNames.getLength();
                    ++elementName
                )
            {
                Any aElement( _rxContainer->getByName( *elementName ) );
                OUString sElementName( sHierarhicalBase + *elementName );

                Reference< XNameAccess > xSubContainer( aElement, UNO_QUERY );
                if ( xSubContainer.is() )
                {
                    lcl_collectHierarchicalElementNames_throw( xSubContainer, sElementName, _out_rDocs, _eType, _io_counter );
                }
                else
                {
                    Reference< XCommandProcessor > xCommandProcessor( aElement, UNO_QUERY );
                    OSL_ENSURE( xCommandProcessor.is(), "lcl_collectHierarchicalElementNames_throw: no container, and no comand processor? What *is* it, then?!" );
                    if ( xCommandProcessor.is() )
                    {
                        _out_rDocs.push_back( SubDocument( xCommandProcessor, sElementName, _eType, ++_io_counter ) );
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_collectSubDocuments_nothrow()
    {
        OSL_PRECOND( m_xDocument.is(), "MigrationEngine_Impl::impl_collectSubDocuments_nothrow: invalid document!" );
        if ( !m_xDocument.is() )
            return false;

        try
        {
            Reference< XNameAccess > xDocContainer( m_xDocument->getFormDocuments(), UNO_SET_THROW );
            m_nFormCount = 0;
            lcl_collectHierarchicalElementNames_throw( xDocContainer, OUString(), m_aSubDocs, eForm, m_nFormCount );

            xDocContainer.set( m_xDocument->getReportDocuments(), UNO_SET_THROW );
            m_nReportCount = 0;
            lcl_collectHierarchicalElementNames_throw( xDocContainer, OUString(), m_aSubDocs, eReport, m_nReportCount );
        }
        catch( const Exception& )
        {
            m_rLogger.logFailure( MigrationError(
                ERR_COLLECTING_DOCUMENTS_FAILED,
                ::cppu::getCaughtException()
            ) );
            return false;
        }
        return true;
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_handleDocument_nothrow( const SubDocument& _rDocument ) const
    {
        OSL_ENSURE( m_nCurrentDocumentID == -1,
            "MigrationEngine_Impl::impl_handleDocument_nothrow: there already is a current document!");
        m_nCurrentDocumentID = m_rLogger.startedDocument( _rDocument.eType, _rDocument.sHierarchicalName );

        // start the progress
        OUString sObjectName( lcl_getSubDocumentDescription( _rDocument ) );
        m_rProgress.startObject( sObjectName, OUString(), DEFAULT_DOC_PROGRESS_RANGE );

        // -----------------
        // load the document
        Reference< ProgressCapture > pStatusIndicator( new ProgressCapture( sObjectName, m_rProgress ) );
        SubDocument aSubDocument( _rDocument );
        OpenDocResult eResult = lcl_loadSubDocument_nothrow( aSubDocument, pStatusIndicator.get(), m_rLogger );
        if ( eResult != eOpenedDoc )
        {
            pStatusIndicator->dispose();
            m_rProgress.endObject();
            m_rLogger.finishedDocument( m_nCurrentDocumentID );
            m_nCurrentDocumentID = -1;
            return ( eResult == eIgnoreDoc );
        }

        // -----------------
        // migrate the libraries
        ProgressDelegator aDelegator(m_rProgress, sObjectName, MacroMigrationResId(STR_MIGRATING_LIBS).toString());
        ProgressMixer aProgressMixer( aDelegator );
        aProgressMixer.registerPhase( PHASE_JAVASCRIPT, 1 );
        aProgressMixer.registerPhase( PHASE_BEANSHELL, 1 );
        aProgressMixer.registerPhase( PHASE_PYTHON, 1 );
        aProgressMixer.registerPhase( PHASE_JAVA, 1 );
        aProgressMixer.registerPhase( PHASE_BASIC, 5 );
            // more weight than the others, assuming that usually, there are many more Basic macros than any other scripts
        aProgressMixer.registerPhase( PHASE_DIALOGS, 1 );

        bool bSuccess = impl_checkScriptStorageStructure_nothrow( aSubDocument );

        // migrate storage-based script libraries (which can be handled by mere storage operations)
        bSuccess = bSuccess
            &&  impl_migrateScriptStorage_nothrow( aSubDocument, eJavaScript, aProgressMixer, PHASE_JAVASCRIPT )
            &&  impl_migrateScriptStorage_nothrow( aSubDocument, eBeanShell, aProgressMixer, PHASE_BEANSHELL )
            &&  impl_migrateScriptStorage_nothrow( aSubDocument, ePython, aProgressMixer, PHASE_PYTHON )
            &&  impl_migrateScriptStorage_nothrow( aSubDocument, eJava, aProgressMixer, PHASE_JAVA );

        // migrate Basic and dialog libraries
        bSuccess =  bSuccess
                &&  impl_migrateContainerLibraries_nothrow( aSubDocument, eBasic, aProgressMixer, PHASE_BASIC )
                &&  impl_migrateContainerLibraries_nothrow( aSubDocument, eDialog, aProgressMixer, PHASE_DIALOGS );
                // order matters: First Basic scripts, then dialogs. So we can adjust references from the latter
                // to the former

        // adjust the events in the document
        // (note that errors are ignored here - failure to convert a script reference
        // is not considered a critical error)
        if ( bSuccess )
        {
            impl_adjustDocumentEvents_nothrow( aSubDocument );
            impl_adjustFormComponentEvents_nothrow( aSubDocument );
        }

        // -----------------
        // clean up
        // store the sub document, including removal of the (now obsolete) "Scripts" sub folder
        if ( m_rLogger.movedAnyLibrary( m_nCurrentDocumentID ) )
        {
            bSuccess =  bSuccess
                    &&  ScriptsStorage::removeFromDocument( aSubDocument.xDocument, m_rLogger )
                    &&  lcl_commitDocumentStorage_nothrow( aSubDocument.xDocument, m_rLogger )
                    &&  lcl_storeEmbeddedDocument_nothrow( aSubDocument );
        }

        // unload in any case, even if we were not successful
        bSuccess =  lcl_unloadSubDocument_nothrow( aSubDocument, m_rLogger )
                &&  bSuccess;

        pStatusIndicator->dispose();

        // end the progress, just in case the ProgressCapture didn't receive the XStatusIndicator::end event
        m_rProgress.endObject();

        m_rLogger.finishedDocument( m_nCurrentDocumentID );
        m_nCurrentDocumentID = -1;
        return bSuccess;
    }

    //--------------------------------------------------------------------
    namespace
    {
        static OUString lcl_createTargetLibName( const SubDocument& _rDocument,
            const OUString& _rSourceLibName, const Reference< XNameAccess >& _rxTargetContainer )
        {
            // The new library name is composed from the prefix, the base name, and the old library name.
            const OUString sPrefix = (_rDocument.eType == eForm)?OUString("Form_"): OUString("Report_");

            OUString sBaseName( _rDocument.sHierarchicalName.copy(
                _rDocument.sHierarchicalName.lastIndexOf( '/' ) + 1 ) );
            // Normalize this name. In our current storage implementation (and script containers in a document
            // are finally mapped to sub storages of the document storage), not all characters are allowed.
            // The bug requesting to change this is #i95409#.
            // Unfortunately, the storage implementation does not complain if you use invalid characters/names, but instead
            // it silently accepts them, and produces garbage in the file (#i95408).
            // So, until especially the former is fixed, we need to strip all invalid characters from the name.
            // #i95865#

            // The general idea is to replace invalid characters with '_'. However, since "valid" essentially means
            // ASCII only, this implies that for a lot of languages, we would simply replace everything with '_',
            // which of course is not desired.
            // So, we use a heuristics: If the name contains at most 3 invalid characters, and as many valid as invalid
            // characters, then we use the replacement. Otherwise, we just use a unambiguous number for the sub document.
            sal_Int32 nValid=0, nInvalid=0;
            const sal_Unicode* pBaseName = sBaseName.getStr();
            const sal_Int32 nBaseNameLen = sBaseName.getLength();
            for ( sal_Int32 i=0; i<nBaseNameLen; ++i )
            {
                if ( ::comphelper::OStorageHelper::IsValidZipEntryFileName( pBaseName + i, 1, sal_False ) )
                    ++nValid;
                else
                    ++nInvalid;
            }
            if ( ( nInvalid <= 3 ) && ( nInvalid * 2 <= nValid ) )
            {   // not "too many" invalid => replace them
                OUStringBuffer aReplacement;
                aReplacement.ensureCapacity( nBaseNameLen );
                aReplacement.append( sBaseName );
                const sal_Unicode* pReplacement = aReplacement.getStr();
                for ( sal_Int32 i=0; i<nBaseNameLen; ++i )
                {
                    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( pReplacement + i, 1, sal_False ) )
                        aReplacement[i] = '_';
                }
                sBaseName = aReplacement.makeStringAndClear();

                OUString sTargetName( sPrefix + sBaseName + "_" + _rSourceLibName );
                if ( !_rxTargetContainer->hasByName( sTargetName ) )
                    return sTargetName;
            }

            // "too many" invalid characters, or the name composed with the base name was already used.
            // (The latter is valid, since there can be multiple sub documents with the same base name,
            // in different levels in the hierarchy.)
            // In this case, just use the umambiguous sub document number.
            return sPrefix + OUString::number( _rDocument.nNumber ) + "_" + _rSourceLibName;
        }
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_checkScriptStorageStructure_nothrow( const SubDocument& _rDocument ) const
    {
        OSL_PRECOND( _rDocument.xDocument.is(), "MigrationEngine_Impl::impl_checkScriptStorageStructure_nothrow: invalid document!" );
        if ( !_rDocument.xDocument.is() )
            return false;

        try
        {
            // the root storage of the document whose scripts are to be migrated
            ScriptsStorage aDocStorage( _rDocument.xDocument, m_rLogger );
            if  ( !aDocStorage.isValid() )
            {   // no scripts at all, or no scripts of the given type
                return !m_rLogger.hadFailure();
            }
            ::std::set< OUString > aElementNames( aDocStorage.getElementNames() );

            ScriptType aKnownStorageBasedTypes[] = {
                eBeanShell, eJavaScript, ePython, eJava
            };
            for ( size_t i=0; i<sizeof( aKnownStorageBasedTypes ) / sizeof( aKnownStorageBasedTypes[0] ); ++i )
                aElementNames.erase( lcl_getScriptsSubStorageName( aKnownStorageBasedTypes[i] ) );

            if ( !aElementNames.empty() )
            {
                m_rLogger.logFailure( MigrationError(
                    ERR_UNKNOWN_SCRIPT_FOLDER,
                    lcl_getSubDocumentDescription( _rDocument ),
                    *aElementNames.begin()
                ) );
                return false;
            }
        }
        catch( const Exception& )
        {
            m_rLogger.logFailure( MigrationError(
                ERR_EXAMINING_SCRIPTS_FOLDER_FAILED,
                lcl_getSubDocumentDescription( _rDocument ),
                ::cppu::getCaughtException()
            ) );
            return false;
        }
        return true;
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_migrateScriptStorage_nothrow( const SubDocument& _rDocument,
        const ScriptType _eScriptType, ProgressMixer& _rProgress, const PhaseID _nPhaseID ) const
    {
        OSL_PRECOND( _rDocument.xDocument.is(), "MigrationEngine_Impl::impl_migrateScriptStorage_nothrow: invalid document!" );
        if ( !_rDocument.xDocument.is() )
            return false;

        ScriptsStorage aDatabaseScripts( m_rLogger );
            // the scripts of our complete database document - created on demand only
        SharedStorage xTargetStorage;
            // the target for moving the scripts storages - created on demand only

        PhaseGuard aPhase( _rProgress );
        bool bSuccess = false;
        Any aException;
        try
        {
            // the root storage of the document whose scripts are to be migrated
            ScriptsStorage aDocStorage( _rDocument.xDocument, m_rLogger );
            if  (   !aDocStorage.isValid()
                ||  !aDocStorage.hasScripts( _eScriptType )
                )
            {
                // no scripts at all, or no scripts of the given type
                _rProgress.startPhase( _nPhaseID, 1 );
                _rProgress.endPhase();
                return !m_rLogger.hadFailure();
            }

            SharedStorage xScriptsRoot( aDocStorage.getScriptsRoot( _eScriptType ) );
            if ( !xScriptsRoot.is() )
                throw RuntimeException("internal error", NULL );

            // loop through the script libraries
            Sequence< OUString > aStorageElements( xScriptsRoot->getElementNames() );
            aPhase.start( _nPhaseID, aStorageElements.getLength() );

            for (   const OUString* element = aStorageElements.getConstArray();
                    element != aStorageElements.getConstArray() + aStorageElements.getLength();
                    ++element
                )
            {
                bool bIsScriptLibrary = xScriptsRoot->isStorageElement( *element );
                OSL_ENSURE( bIsScriptLibrary,
                    "MigrationEngine_Impl::impl_migrateScriptStorage_nothrow: warning: unknown scripts storage structure!" );
                    // we cannot handle this. We would need to copy this stream to the respective scripts storage
                    // of the database document, but we cannot guarantee that the name is not used, yet, and we cannot
                    // simply rename the thing.
                if ( !bIsScriptLibrary )
                {
                    m_rLogger.logFailure( MigrationError(
                        ERR_UNEXPECTED_LIBSTORAGE_ELEMENT,
                        lcl_getSubDocumentDescription( _rDocument ),
                        getScriptTypeDisplayName( _eScriptType ),
                        *element
                    ) );
                    return false;
                }

                // ensure we have access to the DBDoc's scripts storage
                if ( !aDatabaseScripts.isValid() )
                {   // not needed 'til now
                    aDatabaseScripts.bind( m_xDocumentModel );
                    if ( aDatabaseScripts.isValid() )
                        xTargetStorage = aDatabaseScripts.getScriptsRoot( _eScriptType );

                    if ( !xTargetStorage.is() )
                    {
                        m_rLogger.logFailure( MigrationError(
                            ERR_CREATING_DBDOC_SCRIPT_STORAGE_FAILED,
                            getScriptTypeDisplayName( _eScriptType )
                        ) );
                        return false;
                    }
                }

                // move the library to the DBDoc's scripts library, under the new name
                OUString sNewLibName( lcl_createTargetLibName( _rDocument, *element, xTargetStorage.getTyped().get() ) );
                xScriptsRoot->moveElementTo( *element, xTargetStorage, sNewLibName );

                // log the fact that we moved the library
                m_rLogger.movedLibrary( m_nCurrentDocumentID, _eScriptType, *element, sNewLibName );

                // progress
                _rProgress.advancePhase( element - aStorageElements.getConstArray() );
            }

            // commit the storages, so the changes we made persist
            if  (   !lcl_commitStorage_nothrow( xScriptsRoot )
                ||  ( xTargetStorage.is() && !lcl_commitStorage_nothrow( xTargetStorage ) )
                )
            {
                m_rLogger.logFailure( MigrationError(
                    ERR_COMMITTING_SCRIPT_STORAGES_FAILED,
                    getScriptTypeDisplayName( _eScriptType ),
                    lcl_getSubDocumentDescription( _rDocument )
                ) );
                return false;
            }

            // now that the concrete scripts storage does not have any elements anymore,
            // remove it
            xScriptsRoot.reset( NULL ); // need to reset the storage to be allowed to remove it
            aDocStorage.removeScriptTypeStorage( _eScriptType );

            // done so far
            bSuccess =  aDocStorage.commit()
                    &&  aDatabaseScripts.commit();
        }
        catch( const Exception& )
        {
            aException = ::cppu::getCaughtException();
            bSuccess = false;
        }

        // log the error, if any
        if ( !bSuccess )
        {
            m_rLogger.logFailure( MigrationError(
                ERR_GENERAL_SCRIPT_MIGRATION_FAILURE,
                getScriptTypeDisplayName( _eScriptType ),
                lcl_getSubDocumentDescription( _rDocument ),
                aException
            ) );
        }

        return bSuccess;
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_migrateContainerLibraries_nothrow( const SubDocument& _rDocument,
            const ScriptType _eScriptType, ProgressMixer& _rProgress, const PhaseID _nPhaseID ) const
    {
        OSL_PRECOND( ( _eScriptType == eBasic ) || ( _eScriptType == eDialog ),
            "MigrationEngine_Impl::impl_migrateContainerLibraries_nothrow: illegal script type!" );

        bool bSuccess = false;
        PhaseGuard aPhase( _rProgress );
        Any aException;
        do  // artificial loop for flow control only
        {
        try
        {
            // access library container of the sub document
            Reference< XEmbeddedScripts > xSubDocScripts( _rDocument.xDocument, UNO_QUERY );
            if ( !xSubDocScripts.is() )
            {   // no script support in the sub document -> nothing to migrate
                // (though ... this is suspicious, at least ...)
                bSuccess = true;
                break;
            }

            Reference< XStorageBasedLibraryContainer > xSourceLibraries(
                _eScriptType == eBasic ? xSubDocScripts->getBasicLibraries() : xSubDocScripts->getDialogLibraries(),
                UNO_QUERY_THROW
            );
            Reference< XLibraryContainerPassword > xSourcePasswords( xSourceLibraries, UNO_QUERY );
            OSL_ENSURE( xSourcePasswords.is(),
                "MigrationEngine_Impl::impl_migrateContainerLibraries_nothrow: suspicious: no password management for the source libraries!" );

            Sequence< OUString > aSourceLibNames( xSourceLibraries->getElementNames() );
            aPhase.start( _nPhaseID, aSourceLibNames.getLength() );

            if ( !xSourceLibraries->hasElements() )
            {
                bSuccess = true;
                break;
            }

            // create library containers for the document - those will be the target for the migration
            Reference< XStorageBasedDocument > xStorageDoc( m_xDocument, UNO_QUERY_THROW );
            Reference< XStorageBasedLibraryContainer > xTargetLibraries;
            if ( _eScriptType == eBasic )
            {
                xTargetLibraries.set( DocumentScriptLibraryContainer::create(
                    m_aContext, xStorageDoc ), UNO_QUERY_THROW );
            }
            else
            {
                xTargetLibraries.set( DocumentDialogLibraryContainer::create(
                    m_aContext, xStorageDoc ), UNO_QUERY_THROW );
            }

            // copy all libs to the target, with potentially renaming them
            const OUString* pSourceLibBegin = aSourceLibNames.getConstArray();
            const OUString* pSourceLibEnd = pSourceLibBegin + aSourceLibNames.getLength();
            for (   const OUString* pSourceLibName = pSourceLibBegin;
                    pSourceLibName != pSourceLibEnd;
                    ++pSourceLibName
                )
            {
                // if the library is password-protected, ask the user to unprotect it
                if  (   xSourcePasswords.is()
                    &&  xSourcePasswords->isLibraryPasswordProtected( *pSourceLibName )
                    &&  !xSourcePasswords->isLibraryPasswordVerified( *pSourceLibName )
                    )
                {
                    if ( !impl_unprotectPasswordLibrary_throw( xSourcePasswords, _eScriptType, *pSourceLibName ) )
                    {
                        m_rLogger.logFailure( MigrationError(
                            ERR_PASSWORD_VERIFICATION_FAILED,
                            _rDocument.sHierarchicalName,
                            getScriptTypeDisplayName( _eScriptType ),
                            *pSourceLibName
                        ) );
                        return false;
                    }
                }

                OUString sNewLibName( lcl_createTargetLibName( _rDocument, *pSourceLibName, xTargetLibraries.get() ) );

                if ( xSourceLibraries->isLibraryLink( *pSourceLibName ) )
                {
                    // just re-create the link in the target library
                    xTargetLibraries->createLibraryLink(
                        sNewLibName,
                        xSourceLibraries->getLibraryLinkURL( *pSourceLibName ),
                        xSourceLibraries->isLibraryReadOnly( *pSourceLibName )
                    );
                }
                else
                {
                    if ( !xSourceLibraries->isLibraryLoaded( *pSourceLibName ) )
                        xSourceLibraries->loadLibrary( *pSourceLibName );

                    // copy the content of this particular library
                    Reference< XNameAccess > xSourceLib( xSourceLibraries->getByName( *pSourceLibName ), UNO_QUERY_THROW );
                    Reference< XNameContainer > xTargetLib( xTargetLibraries->createLibrary( sNewLibName ), UNO_QUERY_THROW );

                    Sequence< OUString > aLibElementNames( xSourceLib->getElementNames() );
                    for (   const OUString* pSourceElementName = aLibElementNames.getConstArray();
                            pSourceElementName != aLibElementNames.getConstArray() + aLibElementNames.getLength();
                            ++pSourceElementName
                        )
                    {
                        Any aElement = xSourceLib->getByName( *pSourceElementName );
                        OSL_ENSURE( aElement.hasValue(),
                            "MigrationEngine_Impl::impl_migrateContainerLibraries_nothrow: invalid (empty) lib element!" );

                        // if this is a dialog, adjust the references to scripts
                        if ( _eScriptType == eDialog )
                        {
                            impl_adjustDialogEvents_nothrow( aElement, lcl_getSubDocumentDescription( _rDocument ),
                                *pSourceLibName, *pSourceElementName );
                        }

                        xTargetLib->insertByName( *pSourceElementName, aElement );
                    }

                    // transfer the read-only flag
                    xTargetLibraries->setLibraryReadOnly(
                        sNewLibName, xSourceLibraries->isLibraryReadOnly( *pSourceLibName ) );
                }

                // remove the source lib
                xSourceLibraries->removeLibrary( *pSourceLibName );

                // tell the logger
                m_rLogger.movedLibrary( m_nCurrentDocumentID, _eScriptType, *pSourceLibName, sNewLibName );

                // tell the progress
                _rProgress.advancePhase( pSourceLibName - pSourceLibBegin );
            }

            // clean up
            xSourceLibraries->storeLibraries();

            xTargetLibraries->storeLibraries();
            Reference< XStorage > xTargetRoot( xTargetLibraries->getRootLocation(), UNO_QUERY_THROW );
            bSuccess = lcl_commitStorage_nothrow( xTargetRoot );
        }
        catch( const Exception& )
        {
            aException = ::cppu::getCaughtException();
            bSuccess = false;
        }
        } while ( false );

        // log the error, if any
        if ( !bSuccess )
        {
            m_rLogger.logFailure( MigrationError(
                ERR_GENERAL_MACRO_MIGRATION_FAILURE,
                lcl_getSubDocumentDescription( _rDocument ),
                aException
            ) );
        }

        return bSuccess;
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_adjustScriptLibrary_nothrow( const OUString& _rScriptType,
            OUString& _inout_rScriptCode ) const
    {
        OSL_PRECOND( !_inout_rScriptCode.isEmpty(), "MigrationEngine_Impl::impl_adjustScriptLibrary_nothrow: invalid script!" );
        if ( _inout_rScriptCode.isEmpty() )
            return false;

        bool bSuccess = false;
        Any aException;
        try
        {
            if  ( _rScriptType != "Script" || _rScriptType.isEmpty() )
            {
                OSL_FAIL(
                    "MigrationEngine_Impl::impl_adjustScriptLibrary_nothrow: no or unknown script type!" );
                m_rLogger.logRecoverable( MigrationError(
                    ERR_UNKNOWN_SCRIPT_TYPE,
                    _rScriptType
                ) );
                return false;
            }

            // analyze the script URI
            Reference< XUriReferenceFactory > xUriRefFac = UriReferenceFactory::create( m_aContext );
            Reference< XVndSunStarScriptUrlReference > xUri( xUriRefFac->parse( _inout_rScriptCode ), UNO_QUERY_THROW );

            OUString sScriptLanguage = xUri->getParameter( OUString( "language" ) );
            ScriptType eScriptType = eBasic;
            if ( !lcl_getScriptTypeFromLanguage( sScriptLanguage, eScriptType ) )
            {
                OSL_FAIL(
                    "MigrationEngine_Impl::impl_adjustScriptLibrary_nothrow: unknown script language!" );
                m_rLogger.logRecoverable( MigrationError(
                    ERR_UNKNOWN_SCRIPT_LANGUAGE,
                    sScriptLanguage
                ) );
                return false;
            }

            OUString sLocation = xUri->getParameter( OUString( "location" ) );
            if ( sLocation != "document" )
            {
                // only document libraries must be migrated, of course
                return false;
            }

            OUString sScriptName = xUri->getName();
            sal_Int32 nLibModuleSeparator = sScriptName.indexOf( '.' );
            if ( nLibModuleSeparator < 0 )
            {
                OSL_FAIL(
                    "MigrationEngine_Impl::impl_adjustScriptLibrary_nothrow: invalid/unknown location format!" );
                m_rLogger.logRecoverable( MigrationError(
                    ERR_UNKNOWN_SCRIPT_NAME_FORMAT,
                    sScriptName
                ) );
                return false;
            }

            // replace the library name
            OUString sLibrary = sScriptName.copy( 0, nLibModuleSeparator );
            OUString sNewLibName = m_rLogger.getNewLibraryName(
                m_nCurrentDocumentID, eScriptType, sLibrary );
            OSL_ENSURE( sLibrary != sNewLibName,
                "MigrationEngine_Impl::impl_adjustScriptLibrary_nothrow: a library which has not been migrated?" );

            xUri->setName( sNewLibName + sScriptName.copy( nLibModuleSeparator ) );

            // update the new script URL
            _inout_rScriptCode = xUri->getUriReference();
            bSuccess = true;
        }
        catch( const Exception& )
        {
            aException = ::cppu::getCaughtException();
            bSuccess = false;
        }

        // log the failure, if any
        if ( !bSuccess )
        {
            m_rLogger.logRecoverable( MigrationError(
                ERR_SCRIPT_TRANSLATION_FAILURE,
                _rScriptType,
                _inout_rScriptCode,
                aException
            ) );
        }

        return bSuccess;
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_adjustScriptLibrary_nothrow( ScriptEventDescriptor& _inout_rScriptEvent ) const
    {
        if ( !(_inout_rScriptEvent.ScriptType.isEmpty() || _inout_rScriptEvent.ScriptCode.isEmpty()) )
            return impl_adjustScriptLibrary_nothrow( _inout_rScriptEvent.ScriptType, _inout_rScriptEvent.ScriptCode );
        return false;
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_adjustScriptLibrary_nothrow( Any& _inout_rScriptDescriptor ) const
    {
        ::comphelper::NamedValueCollection aScriptDesc( _inout_rScriptDescriptor );

        OUString sScriptType;
        OUString sScript;
        try
        {
            OSL_VERIFY( aScriptDesc.get_ensureType( "EventType", sScriptType ) );
            OSL_VERIFY( aScriptDesc.get_ensureType( "Script", sScript ) );
        }
        catch( const Exception& )
        {
            m_rLogger.logRecoverable( MigrationError(
                ERR_INVALID_SCRIPT_DESCRIPTOR_FORMAT,
                ::cppu::getCaughtException()
            ) );
        }

        if ( !(sScriptType.isEmpty() || sScript.isEmpty()) )
            if ( !impl_adjustScriptLibrary_nothrow( sScriptType, sScript ) )
                return false;

        aScriptDesc.put( "Script", sScript );
        _inout_rScriptDescriptor <<= aScriptDesc.getPropertyValues();
        return true;
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_adjustDocumentEvents_nothrow( const SubDocument& _rDocument ) const
    {
        try
        {
            Reference< XEventsSupplier > xSuppEvents( _rDocument.xDocument, UNO_QUERY );
            if ( !xSuppEvents.is() )
                // this is allowed. E.g. new-style reports currently do not support this
                return true;

            Reference< XNameReplace > xEvents( xSuppEvents->getEvents(), UNO_SET_THROW );
            Sequence< OUString > aEventNames = xEvents->getElementNames();

            Any aEvent;
            for (   const OUString* eventName = aEventNames.getConstArray();
                    eventName != aEventNames.getConstArray() + aEventNames.getLength();
                    ++eventName
                )
            {
                aEvent = xEvents->getByName( *eventName );
                if ( !aEvent.hasValue() )
                    continue;

                // translate
                if ( !impl_adjustScriptLibrary_nothrow( aEvent ) )
                    continue;

                // put back
                xEvents->replaceByName( *eventName, aEvent );
            }
        }
        catch( const Exception& )
        {
            m_rLogger.logRecoverable( MigrationError(
                ERR_ADJUSTING_DOCUMENT_EVENTS_FAILED,
                lcl_getSubDocumentDescription( _rDocument ),
                ::cppu::getCaughtException()
            ) );
            return false;
        }
        return true;
    }

    //--------------------------------------------------------------------
    void MigrationEngine_Impl::impl_adjustDialogElementEvents_throw( const Reference< XInterface >& _rxElement ) const
    {
        Reference< XScriptEventsSupplier > xEventsSupplier( _rxElement, UNO_QUERY_THROW );
        Reference< XNameReplace > xEvents( xEventsSupplier->getEvents(), UNO_QUERY_THROW );
        Sequence< OUString > aEventNames( xEvents->getElementNames() );

        const OUString* eventName = aEventNames.getArray();
        const OUString* eventNamesEnd = eventName + aEventNames.getLength();

        ScriptEventDescriptor aScriptEvent;
        for ( ; eventName != eventNamesEnd; ++eventName )
        {
            OSL_VERIFY( xEvents->getByName( *eventName ) >>= aScriptEvent );

            if ( !impl_adjustScriptLibrary_nothrow( aScriptEvent ) )
                continue;

            xEvents->replaceByName( *eventName, makeAny( aScriptEvent ) );
        }
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_adjustDialogEvents_nothrow( Any& _inout_rDialogLibraryElement,
        const OUString& _rDocName, const OUString& _rDialogLibName, const OUString& _rDialogName ) const
    {
        try
        {
            // load a dialog model from the stream describing it
            Reference< XInputStreamProvider > xISP( _inout_rDialogLibraryElement, UNO_QUERY_THROW );
            Reference< XInputStream > xInput( xISP->createInputStream(), UNO_QUERY_THROW );

            Reference< XNameContainer > xDialogModel( m_aContext->getServiceManager()->createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", m_aContext), UNO_QUERY_THROW );
            ::xmlscript::importDialogModel( xInput, xDialogModel, m_aContext, m_xDocumentModel );

            // adjust the events of the dialog
            impl_adjustDialogElementEvents_throw( xDialogModel );

            // adjust the events of the controls
            Sequence< OUString > aControlNames( xDialogModel->getElementNames() );
            const OUString* controlName = aControlNames.getConstArray();
            const OUString* controlNamesEnd = controlName + aControlNames.getLength();
            for ( ; controlName != controlNamesEnd; ++controlName )
            {
                impl_adjustDialogElementEvents_throw( Reference< XInterface >( xDialogModel->getByName( *controlName ), UNO_QUERY ) );
            }

            // export dialog model
            xISP = ::xmlscript::exportDialogModel( xDialogModel, m_aContext, m_xDocumentModel );
            _inout_rDialogLibraryElement <<= xISP;
        }
        catch( const Exception& )
        {
            m_rLogger.logRecoverable( MigrationError(
                ERR_ADJUSTING_DIALOG_EVENTS_FAILED,
                _rDocName,
                _rDialogLibName,
                _rDialogName,
                ::cppu::getCaughtException()
            ) );
            return false;
        }
        return true;
    }

    //--------------------------------------------------------------------
    void MigrationEngine_Impl::impl_adjustFormComponentEvents_throw( const Reference< XIndexAccess >& _rxComponentContainer ) const
    {
        FormComponentIterator aCompIter( _rxComponentContainer );
        while ( aCompIter.hasMore() )
        {
            // 1. adjust the component's scripts of the current component
            FormComponentScripts aComponent( aCompIter.next() );
            Sequence< ScriptEventDescriptor > aEvents( aComponent.getEvents() );

            bool bChangedComponentEvents = false;
            for (   ScriptEventDescriptor* scriptEvent = aEvents.getArray();
                    scriptEvent != aEvents.getArray() + aEvents.getLength();
                    ++scriptEvent
                )
            {
                if ( !impl_adjustScriptLibrary_nothrow( *scriptEvent ) )
                    continue;

                bChangedComponentEvents = true;
            }

            if ( bChangedComponentEvents )
                aComponent.setEvents( aEvents );

            // 2. step down if the component is a container itself
            Reference< XIndexAccess > xContainer( aComponent.getComponent(), UNO_QUERY );
            if ( xContainer.is() )
                impl_adjustFormComponentEvents_throw( xContainer );
        }
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_adjustFormComponentEvents_nothrow( const SubDocument& _rDocument ) const
    {
        try
        {
            DrawPageIterator aPageIter( _rDocument.xDocument );
            while ( aPageIter.hasMore() )
            {
                Reference< XFormsSupplier > xSuppForms( aPageIter.next(), UNO_QUERY_THROW );
                Reference< XIndexAccess > xForms( xSuppForms->getForms(), UNO_QUERY_THROW );
                impl_adjustFormComponentEvents_throw( xForms );
            }
        }
        catch( const Exception& )
        {
            m_rLogger.logRecoverable( MigrationError(
                ERR_ADJUSTING_FORMCOMP_EVENTS_FAILED,
                lcl_getSubDocumentDescription( _rDocument ),
                ::cppu::getCaughtException()
            ) );
            return false;
        }
        return true;
    }

    //--------------------------------------------------------------------
    bool MigrationEngine_Impl::impl_unprotectPasswordLibrary_throw( const Reference< XLibraryContainerPassword >& _rxPasswordManager,
            const ScriptType _eScriptType, const OUString& _rLibraryName ) const
    {
        // a human-readable description of the affected library
        OUString sLibraryDescription(
            MacroMigrationResId(STR_LIBRARY_TYPE_AND_NAME).toString().
            replaceFirst("$type$",
                getScriptTypeDisplayName(_eScriptType)).
            replaceFirst("$library$", _rLibraryName));
            //TODO: probably broken if first replaceFirst can produce
            // fresh instance of "$library$" in subject string of second
            // replaceFirst

        InteractionHandler aHandler( m_aContext, m_xDocumentModel );
        OUString sPassword;
        while ( true )
        {
            if ( !aHandler.requestDocumentPassword( sLibraryDescription, sPassword ) )
                // aborted by the user
                return false;

            bool bSuccessVerification = _rxPasswordManager->verifyLibraryPassword( _rLibraryName, sPassword );
            if ( bSuccessVerification )
                return true;
        }

    }

    //====================================================================
    //= MigrationEngine
    //====================================================================
    //--------------------------------------------------------------------
    MigrationEngine::MigrationEngine( const Reference<XComponentContext>& _rContext,
            const Reference< XOfficeDatabaseDocument >& _rxDocument, IMigrationProgress& _rProgress,
            MigrationLog& _rLogger )
        :m_pImpl( new MigrationEngine_Impl( _rContext, _rxDocument, _rProgress, _rLogger ) )
    {
    }

    //--------------------------------------------------------------------
    MigrationEngine::~MigrationEngine()
    {
    }

    //--------------------------------------------------------------------
    sal_Int32 MigrationEngine::getFormCount() const
    {
        return m_pImpl->getFormCount();
    }

    //--------------------------------------------------------------------
    sal_Int32 MigrationEngine::getReportCount() const
    {
        return m_pImpl->getReportCount();
    }

    //--------------------------------------------------------------------
    bool MigrationEngine::migrateAll()
    {
        return m_pImpl->migrateAll();
    }

//........................................................................
} // namespace dbmm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
