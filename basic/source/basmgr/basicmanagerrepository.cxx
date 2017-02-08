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

#include <basic/basicmanagerrepository.hxx>
#include <basic/basmgr.hxx>
#include "scriptcont.hxx"
#include "dlgcont.hxx"
#include <basic/sbuno.hxx>
#include "sbintern.hxx"

#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/hint.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentinfo.hxx>
#include <unotools/eventlisteneradapter.hxx>

#include <osl/getglobalmutex.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>

#include <map>


namespace basic
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::Desktop;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::script::XPersistentLibraryContainer;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::document::XStorageBasedDocument;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::document::XEmbeddedScripts;

    typedef ::std::map< Reference< XInterface >, BasicManager*, ::comphelper::OInterfaceCompare< XInterface > > BasicManagerStore;

    typedef ::std::vector< BasicManagerCreationListener* >  CreationListeners;

    class ImplRepository : public ::utl::OEventListenerAdapter, public SfxListener
    {
    private:
        friend struct CreateImplRepository;
        ImplRepository();

    private:
        BasicManagerStore   m_aStore;
        CreationListeners   m_aCreationListeners;

    public:
        static ImplRepository& Instance();

        BasicManager*   getDocumentBasicManager( const Reference< XModel >& _rxDocumentModel );
        BasicManager*   getOrCreateApplicationBasicManager();
        BasicManager*   getApplicationBasicManager() const;
        void            setApplicationBasicManager( BasicManager* _pBasicManager );
        void    registerCreationListener( BasicManagerCreationListener& _rListener );
        void    revokeCreationListener( BasicManagerCreationListener& _rListener );

    private:
        /** retrieves the location at which the BasicManager for the given model
            is stored.

            If previously, the BasicManager for this model has never been requested,
            then the model is added to the map, with an initial NULL BasicManager.

            @param _rxDocumentModel
                the model whose BasicManager's location is to be retrieved. Must not be <NULL/>.

            @precond
                our mutex is locked
        */
        BasicManager*&
                impl_getLocationForModel( const Reference< XModel >& _rxDocumentModel );

        /** tests if there is a location set at which the BasicManager for the given model
            is stored.

            @param _rxDocumentModel
                the model whose BasicManager's location is to be retrieved. Must not be <NULL/>.

            @precond
                our mutex is locked
        */
        bool impl_hasLocationForModel( const Reference< XModel >& _rxDocumentModel ) const;

        /** creates a new BasicManager instance for the given model

            @param _out_rpBasicManager
                reference to the pointer variable that will hold the new
                BasicManager.

            @param _rxDocumentModel
                the model whose BasicManager will be created. Must not be <NULL/>.
        */
        bool impl_createManagerForModel(
                    BasicManager*& _out_rpBasicManager,
                    const Reference< XModel >& _rxDocumentModel );

        /** creates the application-wide BasicManager
        */
        BasicManager* impl_createApplicationBasicManager();

        /** notifies all listeners which expressed interest in the creation of BasicManager instances.
        */
        void    impl_notifyCreationListeners(
                    const Reference< XModel >& _rxDocumentModel,
                    BasicManager& _rManager
                 );

        /** retrieves the current storage of a given document

            @param  _rxDocument
                the document whose storage is to be retrieved.

            @param  _out_rStorage
                takes the storage upon successful return. Note that this might be <NULL/> even
                if <TRUE/> is returned. In this case, the document has not yet been saved.

            @return
                <TRUE/> if the storage could be successfully retrieved (in which case
                <arg>_out_rStorage</arg> might or might not be <NULL/>), <FALSE/> otherwise.
                In the latter case, processing this document should stop.
        */
        static bool impl_getDocumentStorage_nothrow( const Reference< XModel >& _rxDocument, Reference< XStorage >& _out_rStorage );

        /** retrieves the containers for Basic and Dialog libraries for a given document

            @param  _rxDocument
                the document whose containers are to be retrieved.

            @param _out_rxBasicLibraries
                takes the basic library container upon successful return

            @param _out_rxDialogLibraries
                takes the dialog library container upon successful return

            @return
                <TRUE/> if and only if both containers exist, and could successfully be retrieved
        */
        static bool impl_getDocumentLibraryContainers_nothrow(
                    const Reference< XModel >& _rxDocument,
                    Reference< XPersistentLibraryContainer >& _out_rxBasicLibraries,
                    Reference< XPersistentLibraryContainer >& _out_rxDialogLibraries
                );

        /** initializes the given library containers, which belong to a document
        */
        static void impl_initDocLibraryContainers_nothrow(
                    const Reference< XPersistentLibraryContainer >& _rxBasicLibraries,
                    const Reference< XPersistentLibraryContainer >& _rxDialogLibraries
                );

        // OEventListenerAdapter overridables
        virtual void _disposing( const css::lang::EventObject& _rSource ) override;

        // SfxListener overridables
        virtual void Notify( SfxBroadcaster& _rBC, const SfxHint& _rHint ) override;

        /** removes the Model/BasicManager pair given by iterator from our store
        */
        void impl_removeFromRepository( const BasicManagerStore::iterator& _pos );

    private:
        StarBASIC* impl_getDefaultAppBasicLibrary();
    };


    struct CreateImplRepository
    {
        ImplRepository* operator()()
        {
            static ImplRepository* pRepository = new ImplRepository;
            return pRepository;
        }
    };


    ImplRepository::ImplRepository()
    {
    }


    ImplRepository& ImplRepository::Instance()
    {
        return *rtl_Instance< ImplRepository, CreateImplRepository, ::osl::MutexGuard, ::osl::GetGlobalMutex >::
            create( CreateImplRepository(), ::osl::GetGlobalMutex() );
    }

    BasicManager* ImplRepository::getDocumentBasicManager( const Reference< XModel >& _rxDocumentModel )
    {
        SolarMutexGuard g;

        /*  #163556# (DR) - This function may be called recursively while
            constructing the Basic manager and loading the Basic storage. By
            passing the map entry received from impl_getLocationForModel() to
            the function impl_createManagerForModel(), the new Basic manager
            will be put immediately into the map of existing Basic managers,
            thus a recursive call of this function will find and return it
            without creating another instance.
         */
        BasicManager*& pBasicManager = impl_getLocationForModel( _rxDocumentModel );
        if (pBasicManager != nullptr)
            return pBasicManager;
        if (impl_createManagerForModel(pBasicManager, _rxDocumentModel))
            return pBasicManager;
        return nullptr;
    }

    BasicManager* ImplRepository::getOrCreateApplicationBasicManager()
    {
        SolarMutexGuard g;

        BasicManager* pAppManager = GetSbData()->pAppBasMgr;
        if (pAppManager == nullptr)
            pAppManager = impl_createApplicationBasicManager();
        return pAppManager;
    }

    BasicManager* ImplRepository::getApplicationBasicManager() const
    {
        SolarMutexGuard g;

        return GetSbData()->pAppBasMgr;
    }

    void ImplRepository::setApplicationBasicManager( BasicManager* _pBasicManager )
    {
        SolarMutexGuard g;

        BasicManager* pPreviousManager = getApplicationBasicManager();
        delete pPreviousManager;

        GetSbData()->pAppBasMgr = _pBasicManager;
    }


    BasicManager* ImplRepository::impl_createApplicationBasicManager()
    {
        SolarMutexGuard g;

        OSL_PRECOND(getApplicationBasicManager() == nullptr, "ImplRepository::impl_createApplicationBasicManager: there already is one!");

        // Determine Directory
        SvtPathOptions aPathCFG;
        OUString aAppBasicDir( aPathCFG.GetBasicPath() );
        if ( aAppBasicDir.isEmpty() )
        {
            aPathCFG.SetBasicPath("$(prog)");
        }

        // Create basic and load it
        // AppBasicDir is now a PATH
        INetURLObject aAppBasic( SvtPathOptions().SubstituteVariable("$(progurl)") );
        aAppBasic.insertName( Application::GetAppName() );

        BasicManager* pBasicManager = new BasicManager( new StarBASIC, &aAppBasicDir );
        setApplicationBasicManager( pBasicManager );

        // The first dir in the path as destination:
        OUString aFileName( aAppBasic.getName() );
        aAppBasic = INetURLObject( aAppBasicDir.getToken(1, ';') );
        DBG_ASSERT(aAppBasic.GetProtocol() != INetProtocol::NotValid,
            OString("Invalid URL: \"" +
                    OUStringToOString(aAppBasicDir, osl_getThreadTextEncoding()) +
                    "\"").getStr());
        aAppBasic.insertName( aFileName );
        pBasicManager->SetStorageName( aAppBasic.PathToFileName() );

        // Basic container
        SfxScriptLibraryContainer* pBasicCont = new SfxScriptLibraryContainer( Reference< XStorage >() );
        Reference< XPersistentLibraryContainer > xBasicCont( pBasicCont );
        pBasicCont->setBasicManager( pBasicManager );

        // Dialog container
        SfxDialogLibraryContainer* pDialogCont = new SfxDialogLibraryContainer( Reference< XStorage >() );
        Reference< XPersistentLibraryContainer > xDialogCont( pDialogCont );

        LibraryContainerInfo aInfo( xBasicCont, xDialogCont, static_cast< OldBasicPassword* >( pBasicCont ) );
        pBasicManager->SetLibraryContainerInfo( aInfo );

        // global constants

        // StarDesktop
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        pBasicManager->SetGlobalUNOConstant( "StarDesktop", css::uno::Any( Desktop::create(xContext)));

        // (BasicLibraries and DialogLibraries have automatically been added in SetLibraryContainerInfo)

        // notify
        impl_notifyCreationListeners( nullptr, *pBasicManager );

        // outta here
        return pBasicManager;
    }


    void ImplRepository::registerCreationListener( BasicManagerCreationListener& _rListener )
    {
        SolarMutexGuard g;

        m_aCreationListeners.push_back( &_rListener );
    }


    void ImplRepository::revokeCreationListener( BasicManagerCreationListener& _rListener )
    {
        SolarMutexGuard g;

        CreationListeners::iterator pos = ::std::find( m_aCreationListeners.begin(), m_aCreationListeners.end(), &_rListener );
        if ( pos != m_aCreationListeners.end() )
            m_aCreationListeners.erase( pos );
        else {
            OSL_FAIL( "ImplRepository::revokeCreationListener: listener is not registered!" );
        }
    }


    void ImplRepository::impl_notifyCreationListeners( const Reference< XModel >& _rxDocumentModel, BasicManager& _rManager )
    {
        for (   CreationListeners::const_iterator loop = m_aCreationListeners.begin();
                loop != m_aCreationListeners.end();
                ++loop
            )
        {
            (*loop)->onBasicManagerCreated( _rxDocumentModel, _rManager );
        }
    }


    StarBASIC* ImplRepository::impl_getDefaultAppBasicLibrary()
    {
        BasicManager* pAppManager = getOrCreateApplicationBasicManager();

        StarBASIC* pAppBasic = pAppManager ? pAppManager->GetLib(0) : nullptr;
        DBG_ASSERT( pAppBasic != nullptr, "impl_getApplicationBasic: unable to determine the default application's Basic library!" );
        return pAppBasic;
    }

    BasicManager*& ImplRepository::impl_getLocationForModel( const Reference< XModel >& _rxDocumentModel )
    {
        Reference< XInterface > xNormalized( _rxDocumentModel, UNO_QUERY );
        DBG_ASSERT( _rxDocumentModel.is(), "ImplRepository::impl_getLocationForModel: invalid model!" );

        BasicManager*& location = m_aStore[ xNormalized ];
        return location;
    }

    bool ImplRepository::impl_hasLocationForModel( const Reference< XModel >& _rxDocumentModel ) const
    {
        Reference< XInterface > xNormalized( _rxDocumentModel, UNO_QUERY );
        DBG_ASSERT( _rxDocumentModel.is(), "ImplRepository::impl_getLocationForModel: invalid model!" );

        return m_aStore.find(xNormalized) != m_aStore.end();
    }

    void ImplRepository::impl_initDocLibraryContainers_nothrow( const Reference< XPersistentLibraryContainer >& _rxBasicLibraries, const Reference< XPersistentLibraryContainer >& _rxDialogLibraries )
    {
        OSL_PRECOND( _rxBasicLibraries.is() && _rxDialogLibraries.is(),
            "ImplRepository::impl_initDocLibraryContainers_nothrow: illegal library containers, this will crash!" );

        try
        {
            // ensure there's a standard library in the basic container
            OUString aStdLibName( "Standard" );
            if ( !_rxBasicLibraries->hasByName( aStdLibName ) )
            {
                _rxBasicLibraries->createLibrary( aStdLibName );
            }
            // as well as in the dialog container
            if ( !_rxDialogLibraries->hasByName( aStdLibName ) )
            {
                _rxDialogLibraries->createLibrary( aStdLibName );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    bool ImplRepository::impl_createManagerForModel( BasicManager*& _out_rpBasicManager, const Reference< XModel >& _rxDocumentModel )
    {
        StarBASIC* pAppBasic = impl_getDefaultAppBasicLibrary();

        _out_rpBasicManager = nullptr;
        Reference< XStorage > xStorage;
        if ( !impl_getDocumentStorage_nothrow( _rxDocumentModel, xStorage ) )
        {
            // the document is not able to provide the storage it is based on.
            return false;
        }
        Reference< XPersistentLibraryContainer > xBasicLibs;
        Reference< XPersistentLibraryContainer > xDialogLibs;
        if ( !impl_getDocumentLibraryContainers_nothrow( _rxDocumentModel, xBasicLibs, xDialogLibs ) )
            // the document does not have BasicLibraries and DialogLibraries
            return false;

        if ( xStorage.is() )
        {
            // load BASIC-manager
            SfxErrorContext aErrContext( ERRCTX_SFX_LOADBASIC,
                ::comphelper::DocumentInfo::getDocumentTitle( _rxDocumentModel ) );
            OUString aAppBasicDir = SvtPathOptions().GetBasicPath();

            // Storage and BaseURL are only needed by binary documents!
            tools::SvRef<SotStorage> xDummyStor = new SotStorage( OUString() );
            _out_rpBasicManager = new BasicManager( *xDummyStor, OUString() /* TODO/LATER: xStorage */,
                                                                pAppBasic,
                                                                &aAppBasicDir, true );
            if ( !_out_rpBasicManager->GetErrors().empty() )
            {
                // handle errors
                std::vector<BasicError>& aErrors = _out_rpBasicManager->GetErrors();
                for(const auto& rError : aErrors)
                {
                    // show message to user
                    if ( ErrorHandlerFlags::ButtonsCancel == ErrorHandler::HandleError( rError.GetErrorId() ) )
                    {
                        // user wants to break loading of BASIC-manager
                        delete _out_rpBasicManager;
                        _out_rpBasicManager = nullptr;
                        xStorage.clear();
                        break;
                    }
                }
            }
        }

        // not loaded?
        if ( !xStorage.is() )
        {
            // create new BASIC-manager
            StarBASIC* pBasic = new StarBASIC( pAppBasic );
            pBasic->SetFlag( SbxFlagBits::ExtSearch );
            _out_rpBasicManager = new BasicManager( pBasic, nullptr, true );
        }

        // knit the containers with the BasicManager
        LibraryContainerInfo aInfo( xBasicLibs, xDialogLibs, dynamic_cast< OldBasicPassword* >( xBasicLibs.get() ) );
        OSL_ENSURE( aInfo.mpOldBasicPassword, "ImplRepository::impl_createManagerForModel: wrong BasicLibraries implementation!" );
        _out_rpBasicManager->SetLibraryContainerInfo( aInfo );

        // initialize the containers
        impl_initDocLibraryContainers_nothrow( xBasicLibs, xDialogLibs );

        // so that also dialogs etc. could be 'qualified' addressed
        _out_rpBasicManager->GetLib(0)->SetParent( pAppBasic );

        // global properties in the document's Basic
        _out_rpBasicManager->SetGlobalUNOConstant( "ThisComponent", css::uno::Any( _rxDocumentModel ) );

        // notify
        impl_notifyCreationListeners( _rxDocumentModel, *_out_rpBasicManager );

        // register as listener for this model being disposed/closed
        OSL_ENSURE( _rxDocumentModel.is(), "ImplRepository::impl_createManagerForModel: the document must be an XComponent!" );
        assert(impl_hasLocationForModel(_rxDocumentModel));
        startComponentListening( _rxDocumentModel );

        bool bOk = false;
        // startComponentListening may fail in a disposed _rxDocumentModel, in which case _out_rpBasicManager will be removed
        // from the map and destroyed
        if (impl_hasLocationForModel(_rxDocumentModel))
        {
            bOk = true;
            // register as listener for the BasicManager being destroyed
            StartListening( *_out_rpBasicManager );
        }

        // #i104876: Library container must not be modified just after
        // creation. This happens as side effect when creating default
        // "Standard" libraries and needs to be corrected here
        xBasicLibs->setModified( false );
        xDialogLibs->setModified( false );
        return bOk;
    }

    bool ImplRepository::impl_getDocumentStorage_nothrow( const Reference< XModel >& _rxDocument, Reference< XStorage >& _out_rStorage )
    {
        _out_rStorage.clear();
        try
        {
            Reference< XStorageBasedDocument > xStorDoc( _rxDocument, UNO_QUERY_THROW );
            _out_rStorage.set( xStorDoc->getDocumentStorage() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return false;
        }
        return true;
    }


    bool ImplRepository::impl_getDocumentLibraryContainers_nothrow( const Reference< XModel >& _rxDocument,
        Reference< XPersistentLibraryContainer >& _out_rxBasicLibraries, Reference< XPersistentLibraryContainer >& _out_rxDialogLibraries )
    {
        _out_rxBasicLibraries.clear();
        _out_rxDialogLibraries.clear();
        try
        {
            Reference< XEmbeddedScripts > xScripts( _rxDocument, UNO_QUERY_THROW );
            _out_rxBasicLibraries.set( xScripts->getBasicLibraries(), UNO_QUERY_THROW );
            _out_rxDialogLibraries.set( xScripts->getDialogLibraries(), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return _out_rxBasicLibraries.is() && _out_rxDialogLibraries.is();
    }


    void ImplRepository::impl_removeFromRepository( const BasicManagerStore::iterator& _pos )
    {
        OSL_PRECOND( _pos != m_aStore.end(), "ImplRepository::impl_removeFromRepository: invalid position!" );

        BasicManager* pManager = _pos->second;

        // *first* remove from map (else Notify won't work properly)
        m_aStore.erase( _pos );

        // *then* delete the BasicManager
        EndListening( *pManager );
        delete pManager;
    }


    void ImplRepository::_disposing( const css::lang::EventObject& _rSource )
    {
        SolarMutexGuard g;

        Reference< XInterface > xNormalizedSource( _rSource.Source, UNO_QUERY );

        for (   BasicManagerStore::iterator loop = m_aStore.begin();
                loop != m_aStore.end();
                ++loop
            )
        {
            if ( loop->first.get() == xNormalizedSource.get() )
            {
                impl_removeFromRepository( loop );
                return;
            }
        }

        OSL_FAIL( "ImplRepository::_disposing: where does this come from?" );
    }


    void ImplRepository::Notify( SfxBroadcaster& _rBC, const SfxHint& _rHint )
    {
        if ( _rHint.GetId() != SfxHintId::Dying )
            // not interested in
            return;

        BasicManager* pManager = dynamic_cast< BasicManager* >( &_rBC );
        OSL_ENSURE( pManager, "ImplRepository::Notify: where does this come from?" );

        for (   BasicManagerStore::iterator loop = m_aStore.begin();
                loop != m_aStore.end();
                ++loop
            )
        {
            if ( loop->second == pManager )
            {
                // a BasicManager which is still in our repository is being deleted.
                // That's bad, since by definition, we *own* all instances in our
                // repository.
                OSL_FAIL( "ImplRepository::Notify: nobody should tamper with the managers, except ourself!" );
                m_aStore.erase( loop );
                break;
            }
        }
    }

    BasicManager* BasicManagerRepository::getDocumentBasicManager( const Reference< XModel >& _rxDocumentModel )
    {
        return ImplRepository::Instance().getDocumentBasicManager( _rxDocumentModel );
    }

    BasicManager* BasicManagerRepository::getApplicationBasicManager()
    {
        return ImplRepository::Instance().getOrCreateApplicationBasicManager();
    }

    void BasicManagerRepository::resetApplicationBasicManager()
    {
        ImplRepository::Instance().setApplicationBasicManager( nullptr );
    }

    void BasicManagerRepository::registerCreationListener( BasicManagerCreationListener& _rListener )
    {
        ImplRepository::Instance().registerCreationListener( _rListener );
    }

    void BasicManagerRepository::revokeCreationListener( BasicManagerCreationListener& _rListener )
    {
        ImplRepository::Instance().revokeCreationListener( _rListener );
    }

} // namespace basic


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
