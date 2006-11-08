/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicmanagerrepository.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-08 11:52:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef BASICMANAGERREPOSITORY_HXX
#include "basicmanagerrepository.hxx"
#endif
#ifndef _BASMGR_HXX
#include "basmgr.hxx"
#endif
#ifndef BASIC_SCRIPTCONTAINER_HXX
#include "scriptcont.hxx"
#endif
#ifndef BASIC_DIALOGCONTAINER_HXX
#include "dlgcont.hxx"
#endif
#ifndef _SB_SBUNO_HXX
#include "sbuno.hxx"
#endif
#ifndef _SB_INTERN_HXX
#include "sbintern.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSTORAGEBASEDDOCUMENT_HPP_
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#endif
/** === end UNO includes === **/

#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
#endif

#ifndef INCLUDED_OSL_DOUBLECHECKEDLOCKING_H
#include <rtl/instance.hxx>
#endif

#include <map>

//........................................................................
namespace basic
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::script::XLibraryContainer;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::document::XDocumentInfoSupplier;
    using ::com::sun::star::document::XStorageBasedDocument;
    using ::com::sun::star::lang::XComponent;
    /** === end UNO using === **/

    typedef BasicManager*   BasicManagerPointer;
    typedef ::std::map< Reference< XInterface >, BasicManagerPointer, ::comphelper::OInterfaceCompare< XInterface > > BasicManagerStore;

    typedef ::std::vector< BasicManagerCreationListener* >  CreationListeners;

    //====================================================================
    //= BasicManagerCleaner
    //====================================================================
    /// is the only instance which is allowed to delete a BasicManager instance
    class BasicManagerCleaner
    {
    public:
        static void deleteBasicManager( BasicManager*& _rpManager )
        {
            delete _rpManager;
            _rpManager = NULL;
        }
    };

    //====================================================================
    //= ImplRepository
    //====================================================================
    class ImplRepository : public ::utl::OEventListenerAdapter, public SfxListener
    {
    private:
        friend struct CreateImplRepository;
        ImplRepository();

    private:
        ::osl::Mutex        m_aMutex;
        BasicManagerStore   m_aStore;
        CreationListeners   m_aCreationListeners;

    public:
        static ImplRepository& Instance();

        BasicManager*   getDocumentBasicManager( const Reference< XModel >& _rxDocumentModel );
        BasicManager*   getApplicationBasicManager( bool _bCreate );
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
        BasicManagerPointer&
                impl_getLocationForModel( const Reference< XModel >& _rxDocumentModel );

        /** creates a new BasicManager instance for the given model
        */
        BasicManagerPointer
                impl_createManagerForModel( const Reference< XModel >& _rxDocumentModel );

        /** creates the application-wide BasicManager
        */
        BasicManagerPointer impl_createApplicationBasicManager();

        /** notifies all listeners which expressed interest in the creation of BasicManager instances.
        */
        void    impl_notifyCreationListeners(
                    const Reference< XModel >& _rxDocumentModel,
                    BasicManager& _rManager
                 );

        /** returns a title for the given document
        */
        ::rtl::OUString
                impl_getDocumentTitle_nothrow( const Reference< XModel >& _rxDocument );

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
        bool    impl_getDocumentStorage_nothrow( const Reference< XModel >& _rxDocument, Reference< XStorage >& _out_rStorage );

        // OEventListenerAdapter overridables
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

        // SfxListener overridables
        virtual void Notify( SfxBroadcaster& _rBC, const SfxHint& _rHint );

        /** removes the Model/BasicManager pair given by iterator from our store
        */
        void impl_removeFromRepository( BasicManagerStore::iterator _pos );

    private:
        StarBASIC* impl_getDefaultAppBasicLibrary();
    };

    //====================================================================
    //= CreateImplRepository
    //====================================================================
    struct CreateImplRepository
    {
        ImplRepository* operator()()
        {
            static ImplRepository* pRepository = new ImplRepository;
            return pRepository;
        }
    };


    //====================================================================
    //= ImplRepository
    //====================================================================
    //--------------------------------------------------------------------
    ImplRepository::ImplRepository()
    {
    }

    //--------------------------------------------------------------------
    ImplRepository& ImplRepository::Instance()
    {
        return *rtl_Instance< ImplRepository, CreateImplRepository, ::osl::MutexGuard, ::osl::GetGlobalMutex >::
            create( CreateImplRepository(), ::osl::GetGlobalMutex() );
    }

    //--------------------------------------------------------------------
    BasicManager* ImplRepository::getDocumentBasicManager( const Reference< XModel >& _rxDocumentModel )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        BasicManagerPointer& pBasicManager = impl_getLocationForModel( _rxDocumentModel );
        if ( pBasicManager == NULL )
            pBasicManager = impl_createManagerForModel( _rxDocumentModel );

        return pBasicManager;
    }

    //--------------------------------------------------------------------
    BasicManager* ImplRepository::getApplicationBasicManager( bool _bCreate )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        BasicManager* pAppManager = GetSbData()->pAppBasMgr;
        if ( ( pAppManager == NULL ) && _bCreate )
            pAppManager = impl_createApplicationBasicManager();

        return pAppManager;
    }

    //--------------------------------------------------------------------
    void ImplRepository::setApplicationBasicManager( BasicManager* _pBasicManager )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        BasicManager* pPreviousManager = getApplicationBasicManager( false );
        BasicManagerCleaner::deleteBasicManager( pPreviousManager );

        GetSbData()->pAppBasMgr = _pBasicManager;
    }

    //--------------------------------------------------------------------
    BasicManager* ImplRepository::impl_createApplicationBasicManager()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_PRECOND( getApplicationBasicManager( false ) == NULL, "ImplRepository::impl_createApplicationBasicManager: there already is one!" );

        // Directory bestimmen
        SvtPathOptions aPathCFG;
        String aAppBasicDir( aPathCFG.GetBasicPath() );
        if ( !aAppBasicDir.Len() )
            aPathCFG.SetBasicPath( String::CreateFromAscii("$(prog)") );

        // #58293# soffice.new nur im User-Dir suchen => erstes Verzeichnis
        String aAppFirstBasicDir = aAppBasicDir.GetToken(1);

        // Basic erzeugen und laden
        // MT: #47347# AppBasicDir ist jetzt ein PATH!
        INetURLObject aAppBasic( SvtPathOptions().SubstituteVariable( String::CreateFromAscii("$(progurl)") ) );
        aAppBasic.insertName( Application::GetAppName() );

        BasicManager* pBasicManager = new BasicManager( new StarBASIC, &aAppBasicDir );
        setApplicationBasicManager( pBasicManager );

        // Als Destination das erste Dir im Pfad:
        String aFileName( aAppBasic.getName() );
        aAppBasic = INetURLObject( aAppBasicDir.GetToken(1) );
        DBG_ASSERT( aAppBasic.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
        aAppBasic.insertName( aFileName );
        pBasicManager->SetStorageName( aAppBasic.PathToFileName() );

        // Basic container
        SfxScriptLibraryContainer* pBasicCont = new SfxScriptLibraryContainer(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarBasic" ) ), pBasicManager );
        Reference< XLibraryContainer > xBasicCont = static_cast< XLibraryContainer* >( pBasicCont );
        pBasicCont->setBasicManager( pBasicManager );

        // Dialog container
        SfxDialogLibraryContainer* pDialogCont = new SfxDialogLibraryContainer( Reference< XStorage >() );
        Reference< XLibraryContainer > xDialogCont = static_cast< XLibraryContainer* >( pDialogCont );

        LibraryContainerInfo* pInfo = new LibraryContainerInfo
            ( xBasicCont, xDialogCont, static_cast< OldBasicPassword* >( pBasicCont ) );
        pBasicManager->SetLibraryContainerInfo( pInfo );

        // global constants

        // StarDesktop
        Reference< XMultiServiceFactory > xSMgr = ::comphelper::getProcessServiceFactory();
        pBasicManager->InsertGlobalUNOConstant(
            "StarDesktop",
            makeAny( xSMgr->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ) )
         );

        // (BasicLibraries and DialogLibraries have automatically been added in SetLibraryContainerInfo)

        // notify
        impl_notifyCreationListeners( NULL, *pBasicManager );

        // outta here
        return pBasicManager;
    }

    //--------------------------------------------------------------------
    void ImplRepository::registerCreationListener( BasicManagerCreationListener& _rListener )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aCreationListeners.push_back( &_rListener );
    }

    //--------------------------------------------------------------------
    void ImplRepository::revokeCreationListener( BasicManagerCreationListener& _rListener )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        CreationListeners::iterator pos = ::std::find( m_aCreationListeners.begin(), m_aCreationListeners.end(), &_rListener );
        if ( pos != m_aCreationListeners.end() )
            m_aCreationListeners.erase( pos );
        else
            DBG_ERROR( "ImplRepository::revokeCreationListener: listener is not registered!" );
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    StarBASIC* ImplRepository::impl_getDefaultAppBasicLibrary()
    {
        BasicManager* pAppManager = getApplicationBasicManager( true );

        StarBASIC* pAppBasic = pAppManager ? pAppManager->GetLib(0) : NULL;
        DBG_ASSERT( pAppBasic != NULL, "impl_getApplicationBasic: unable to determine the default application's Basic library!" );
        return pAppBasic;
    }

    //--------------------------------------------------------------------
    BasicManagerPointer& ImplRepository::impl_getLocationForModel( const Reference< XModel >& _rxDocumentModel )
    {
        Reference< XInterface > xNormalized( _rxDocumentModel, UNO_QUERY );
        DBG_ASSERT( xNormalized.is(), "ImplRepository::impl_getLocationForModel: invalid model!" );

        BasicManagerPointer& location = m_aStore[ xNormalized ];
        return location;
    }

    //--------------------------------------------------------------------
    BasicManagerPointer ImplRepository::impl_createManagerForModel( const Reference< XModel >& _rxDocumentModel )
    {
        StarBASIC* pAppBasic = impl_getDefaultAppBasicLibrary();

        BasicManager* pBasicManager( NULL );
        Reference< XStorage > xStorage;
        if ( !impl_getDocumentStorage_nothrow( _rxDocumentModel, xStorage ) )
            // the document is not able to provide the storage it is based on.
            return pBasicManager;

        if ( xStorage.is() )
        {
            // load BASIC-manager
            SfxErrorContext aErrContext( ERRCTX_SFX_LOADBASIC, impl_getDocumentTitle_nothrow( _rxDocumentModel ) );
            String aAppBasicDir = SvtPathOptions().GetBasicPath();

            // Storage and BaseURL are only needed by binary documents!
            SotStorageRef xDummyStor = new SotStorage( ::rtl::OUString() );
            pBasicManager = new BasicManager( *xDummyStor, String() /* TODO/LATER: xStorage */,
                                                                pAppBasic,
                                                                &aAppBasicDir );
            if ( pBasicManager->HasErrors() )
            {
                // handle errors
                BasicError* pErr = pBasicManager->GetFirstError();
                while ( pErr )
                {
                    // show message to user
                    if ( ERRCODE_BUTTON_CANCEL == ErrorHandler::HandleError( pErr->GetErrorId() ) )
                    {
                        // user wants to break loading of BASIC-manager
                        BasicManagerCleaner::deleteBasicManager( pBasicManager );
                        xStorage.clear();
                        break;
                    }
                    pErr = pBasicManager->GetNextError();
                }
            }
        }

        // not loaded?
        if ( !xStorage.is() )
        {
            // create new BASIC-manager
            StarBASIC* pBasic = new StarBASIC( pAppBasic );
            pBasic->SetFlag( SBX_EXTSEARCH );
            pBasicManager = new BasicManager( pBasic );
        }

        // Basic container
        SfxScriptLibraryContainer* pBasicCont = new SfxScriptLibraryContainer(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarBasic" ) ),
            pBasicManager, xStorage );
        Reference< XLibraryContainer > xBasicCont = static_cast< XLibraryContainer* >( pBasicCont );
        sal_Bool bBasicModified = pBasicCont->isContainerModified();

        // Dialog container
        SfxDialogLibraryContainer* pDialogCont = new SfxDialogLibraryContainer( xStorage );
        Reference< XLibraryContainer > xDialogCont = static_cast< XLibraryContainer* >( pDialogCont );
        sal_Bool bDialogModified = pDialogCont->isContainerModified();

        LibraryContainerInfo* pInfo = new LibraryContainerInfo(
            xBasicCont, xDialogCont, static_cast< OldBasicPassword* >( pBasicCont ) );
        pBasicManager->SetLibraryContainerInfo( pInfo );
        pBasicCont->setBasicManager( pBasicManager );

        // ensure there's a standard library in the basic container
        ::rtl::OUString aStdLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
        if ( xBasicCont.is() && !xBasicCont->hasByName( aStdLibName ) )
            xBasicCont->createLibrary( aStdLibName );
        // as well as in the dialog container
        if ( xDialogCont.is() && !xDialogCont->hasByName( aStdLibName ) )
            xDialogCont->createLibrary( aStdLibName );  // create Standard library

        // damit auch Dialoge etc. 'qualifiziert' angesprochen werden k"onnen
        StarBASIC* pDefaultBasicLib = pBasicManager->GetLib(0);
        sal_Bool bWasModified = pDefaultBasicLib->IsModified();
        pDefaultBasicLib->SetParent( pAppBasic );

        // Properties im Doc-BASIC
        // ThisComponent
        pBasicManager->InsertGlobalUNOConstant( "ThisComponent", makeAny( _rxDocumentModel ) );
        // (BasicLibraries and DialogLibraries have automatically been added in SetLibraryContainerInfo)

        // reset the modify flags (have been set in MakeVariable)
        pDefaultBasicLib->SetModified( bWasModified );
        pBasicCont->setContainerModified( bBasicModified );
        pDialogCont->setContainerModified( bDialogModified );

        // notify
        impl_notifyCreationListeners( _rxDocumentModel, *pBasicManager );

        // register as listener for this model being disposed/closed
        Reference< XComponent > xDocumentComponent( _rxDocumentModel, UNO_QUERY );
        OSL_ENSURE( xDocumentComponent.is(), "ImplRepository::impl_createManagerForModel: the document must be an XComponent!" );
        startComponentListening( xDocumentComponent );

        // register as listener for the BasicManager being destroyed
        StartListening( *pBasicManager );

        return pBasicManager;
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    ::rtl::OUString ImplRepository::impl_getDocumentTitle_nothrow( const Reference< XModel >& _rxDocument )
    {
        OSL_PRECOND( _rxDocument.is(), "ImplRepository::impl_getDocumentTitle: invalid document: this will crash!" );

        ::rtl::OUString sTitle;

        // try the title from the DocumentInfo
        try
        {
            Reference< XDocumentInfoSupplier > xSuppInfo( _rxDocument, UNO_QUERY_THROW );
            Reference< XPropertySet > xInfoProps( xSuppInfo->getDocumentInfo(), UNO_QUERY_THROW );
            OSL_VERIFY( xInfoProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ) ) >>= sTitle );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        if ( sTitle.getLength() > 0 )
            return sTitle;

        // get the URL
        try
        {
            ::rtl::OUString sURL( _rxDocument->getURL() );
            if ( sURL.getLength() )
            {
                INetURLObject aURL( sURL );
                sTitle = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return sTitle;
    }

    //--------------------------------------------------------------------
    void ImplRepository::impl_removeFromRepository( BasicManagerStore::iterator _pos )
    {
        OSL_PRECOND( _pos != m_aStore.end(), "ImplRepository::impl_removeFromRepository: invalid position!" );

        BasicManager* pManager = _pos->second;

        // *first* remove from map (else Notify won't work properly)
        m_aStore.erase( _pos );

        // *then* delete the BasicManager
        EndListening( *pManager );
        BasicManagerCleaner::deleteBasicManager( pManager );
    }

    //--------------------------------------------------------------------
    void ImplRepository::_disposing( const ::com::sun::star::lang::EventObject& _rSource )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XInterface > xNormalizedSource( _rSource.Source, UNO_QUERY );
    #if OSL_DEBUG_LEVEL > 0
        bool bFound = false;
    #endif

        for (   BasicManagerStore::iterator loop = m_aStore.begin();
                loop != m_aStore.end();
                ++loop
            )
        {
            if ( loop->first.get() == xNormalizedSource.get() )
            {
                impl_removeFromRepository( loop );
            #if OSL_DEBUG_LEVEL > 0
                bFound = true;
            #endif
                break;
            }
        }

        OSL_ENSURE( bFound, "ImplRepository::_disposing: where does this come from?" );
    }

    //--------------------------------------------------------------------
    void ImplRepository::Notify( SfxBroadcaster& _rBC, const SfxHint& _rHint )
    {
        const SfxSimpleHint* pSimpleHint = dynamic_cast< const SfxSimpleHint* >( &_rHint );
        if ( !pSimpleHint || ( pSimpleHint->GetId() != SFX_HINT_DYING ) )
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
                OSL_ENSURE( false, "ImplRepository::Notify: nobody should tamper with the managers, except ourself!" );
                m_aStore.erase( loop );
                break;
            }
        }
    }

    //====================================================================
    //= BasicManagerRepository
    //====================================================================
    //--------------------------------------------------------------------
    BasicManager* BasicManagerRepository::getDocumentBasicManager( const Reference< XModel >& _rxDocumentModel )
    {
        return ImplRepository::Instance().getDocumentBasicManager( _rxDocumentModel );
    }

    //--------------------------------------------------------------------
    BasicManager* BasicManagerRepository::getApplicationBasicManager( bool _bCreate )
    {
        return ImplRepository::Instance().getApplicationBasicManager( _bCreate );
    }

    //--------------------------------------------------------------------
    void BasicManagerRepository::resetApplicationBasicManager()
    {
        return ImplRepository::Instance().setApplicationBasicManager( NULL );
    }

    //--------------------------------------------------------------------
    void BasicManagerRepository::registerCreationListener( BasicManagerCreationListener& _rListener )
    {
        ImplRepository::Instance().registerCreationListener( _rListener );
    }

    //--------------------------------------------------------------------
    void BasicManagerRepository::revokeCreationListener( BasicManagerCreationListener& _rListener )
    {
        ImplRepository::Instance().revokeCreationListener( _rListener );
    }

//........................................................................
} // namespace basic
//........................................................................

