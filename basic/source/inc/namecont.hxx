/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

#ifndef BASIC_NAMECONTAINER_HXX
#define BASIC_NAMECONTAINER_HXX

#include <hash_map>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/script/XLibraryContainerExport.hpp>
#include <com/sun/star/script/XLibraryContainer3.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAScriptListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>

#include <osl/mutex.hxx>
#include <unotools/eventlisteneradapter.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/compbase8.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <sot/storage.hxx>
#include <comphelper/listenernotification.hxx>
#include <xmlscript/xmllib_imexp.hxx>

class BasicManager;

namespace basic
{

//============================================================================

typedef ::cppu::WeakImplHelper3<
    ::com::sun::star::container::XNameContainer,
    ::com::sun::star::container::XContainer,
    ::com::sun::star::util::XChangesNotifier > NameContainer_BASE;

class NameContainer : public ::cppu::BaseMutex, public NameContainer_BASE
{
    typedef std::hash_map< ::rtl::OUString, sal_Int32, ::rtl::OUStringHash > NameContainerNameMap;

    NameContainerNameMap mHashMap;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > mNames;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > mValues;
    sal_Int32 mnElementCount;

    ::com::sun::star::uno::Type mType;
    ::com::sun::star::uno::XInterface* mpxEventSource;

    ::cppu::OInterfaceContainerHelper maContainerListeners;
    ::cppu::OInterfaceContainerHelper maChangesListeners;

public:
    NameContainer( const ::com::sun::star::uno::Type& rType )
        : mnElementCount( 0 )
        , mType( rType )
        , mpxEventSource( NULL )
        , maContainerListeners( m_aMutex )
        , maChangesListeners( m_aMutex )
    {}

    void setEventSource( ::com::sun::star::uno::XInterface* pxEventSource )
        { mpxEventSource = pxEventSource; }

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);

    // Methods XChangesNotifier
    virtual void SAL_CALL addChangesListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XChangesListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChangesListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XChangesListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
};

//============================================================================

class ModifiableHelper
{
private:
    ::cppu::OInterfaceContainerHelper   m_aModifyListeners;
    ::cppu::OWeakObject&                m_rEventSource;
    sal_Bool                            mbModified;

public:
    ModifiableHelper( ::cppu::OWeakObject& _rEventSource, ::osl::Mutex& _rMutex )
        :m_aModifyListeners( _rMutex )
        ,m_rEventSource( _rEventSource )
        ,mbModified( sal_False )
    {
    }

    inline  sal_Bool    isModified() const  { return mbModified; }
            void        setModified( sal_Bool _bModified );

    inline  void    addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.addInterface( _rxListener );
    }

    inline  void    removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.removeInterface( _rxListener );
    }
};

//============================================================================

typedef ::comphelper::OListenerContainerBase<
    ::com::sun::star::script::vba::XVBAScriptListener,
    ::com::sun::star::script::vba::VBAScriptEvent > VBAScriptListenerContainer_BASE;

class VBAScriptListenerContainer : public VBAScriptListenerContainer_BASE
{
public:
    explicit VBAScriptListenerContainer( ::osl::Mutex& rMutex );

private:
    virtual bool implTypedNotify(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::vba::XVBAScriptListener >& rxListener,
        const ::com::sun::star::script::vba::VBAScriptEvent& rEvent )
        throw (::com::sun::star::uno::Exception);
};

//============================================================================

class SfxLibrary;

typedef ::cppu::WeakComponentImplHelper8<
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::script::XStorageBasedLibraryContainer,
    ::com::sun::star::script::XLibraryContainerPassword,
    ::com::sun::star::script::XLibraryContainerExport,
    ::com::sun::star::script::XLibraryContainer3,
    ::com::sun::star::container::XContainer,
    ::com::sun::star::script::vba::XVBACompatibility,
    ::com::sun::star::lang::XServiceInfo > SfxLibraryContainer_BASE;

class SfxLibraryContainer : public SfxLibraryContainer_BASE, public ::utl::OEventListenerAdapter
{
    VBAScriptListenerContainer maVBAScriptListeners;
    sal_Int32 mnRunningVBAScripts;
    sal_Bool mbVBACompat;
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >   mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >       mxSFI;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XMacroExpander >         mxMacroExpander;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringSubstitution >    mxStringSubstitution;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >            mxOwnerDocument;

    ::osl::Mutex        maMutex;
    ModifiableHelper    maModifiable;

    NameContainer maNameContainer;
    sal_Bool    mbOldInfoFormat;
    sal_Bool    mbOasis2OOoFormat;

    ::rtl::OUString maInitialDocumentURL;
    ::rtl::OUString maInfoFileName;
    ::rtl::OUString maOldInfoFileName;
    ::rtl::OUString maLibElementFileExtension;
    ::rtl::OUString maLibraryPath;
    ::rtl::OUString maLibrariesDir;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > mxStorage;
    BasicManager*   mpBasMgr;
    sal_Bool        mbOwnBasMgr;

    enum InitMode
    {
        DEFAULT,
        CONTAINER_INIT_FILE,
        LIBRARY_INIT_FILE,
        OFFICE_DOCUMENT,
        OLD_BASIC_STORAGE
    } meInitMode;

    void implStoreLibrary( SfxLibrary* pLib,
                            const ::rtl::OUString& aName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    // New variant for library export
    void implStoreLibrary( SfxLibrary* pLib,
                            const ::rtl::OUString& aName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                            const ::rtl::OUString& aTargetURL,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xToUseSFI,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );

    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    // New variant for library export
    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    const ::rtl::OUString& aTargetURL,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xToUseSFI );

    sal_Bool implLoadLibraryIndexFile( SfxLibrary* pLib,
                                    ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    const ::rtl::OUString& aIndexFileName );

    void implImportLibDescriptor( SfxLibrary* pLib, ::xmlscript::LibDescriptor& rLib );

    // Methods to distinguish between deffirent library types
    virtual SfxLibrary* SAL_CALL implCreateLibrary( const ::rtl::OUString& aName ) = 0;
    virtual SfxLibrary* SAL_CALL implCreateLibraryLink
        ( const ::rtl::OUString& aName, const ::rtl::OUString& aLibInfoFileURL,
          const ::rtl::OUString& StorageURL, sal_Bool ReadOnly ) = 0;
    virtual ::com::sun::star::uno::Any SAL_CALL createEmptyLibraryElement( void ) = 0;
    virtual bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const = 0;
    virtual void SAL_CALL writeLibraryElement
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& xLibrary,
        const ::rtl::OUString& aElementName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutput
    )
        throw(::com::sun::star::uno::Exception) = 0;

    virtual ::com::sun::star::uno::Any SAL_CALL importLibraryElement
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& xLibrary,
        const ::rtl::OUString& aElementName,
        const ::rtl::OUString& aFile,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xElementStream ) = 0;
    virtual void SAL_CALL importFromOldStorage( const ::rtl::OUString& aFile ) = 0;

    // Password encryption
    virtual sal_Bool implStorePasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );

    // New variant for library export
    virtual sal_Bool implStorePasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                        const ::rtl::OUString& aTargetURL,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xToUseSFI, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );

    virtual sal_Bool implLoadPasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& Name,
        sal_Bool bVerifyPasswordOnly=false )
            throw(::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);

    virtual void onNewRootStorage() = 0;


    // #56666, Creates another library container
    //         instance of the same derived class
    virtual SfxLibraryContainer* createInstanceImpl( void ) = 0;


    // Interface to get the BasicManager (Hack for password implementation)
    BasicManager* getBasicManager( void );
    ::rtl::OUString createAppLibraryFolder( SfxLibrary* pLib, const ::rtl::OUString& aName );

    sal_Bool init( const ::rtl::OUString& rInitialDocumentURL,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxInitialStorage );

    virtual const sal_Char* SAL_CALL    getInfoFileName() const = 0;
    virtual const sal_Char* SAL_CALL    getOldInfoFileName() const = 0;
    virtual const sal_Char* SAL_CALL    getLibElementFileExtension() const = 0;
    virtual const sal_Char* SAL_CALL    getLibrariesDir() const = 0;

    // Handle maLibInfoFileURL and maStorageURL correctly
    void checkStorageURL
    (
        const ::rtl::OUString& aSourceURL,
        ::rtl::OUString& aLibInfoFileURL,
        ::rtl::OUString& aStorageURL,
        ::rtl::OUString& aUnexpandedStorageURL
    );
    ::rtl::OUString expand_url( const ::rtl::OUString& url )
        throw(::com::sun::star::uno::RuntimeException);

    SfxLibrary* getImplLib( const String& rLibraryName );

    void storeLibraries_Impl(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                            sal_Bool bComplete );

    void SAL_CALL initializeFromDocumentURL( const ::rtl::OUString& _rInitialDocumentURL );
    void SAL_CALL initializeFromDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageBasedDocument >& _rxDocument );

    // OEventListenerAdapter
    virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

    // OComponentHelper
    virtual void SAL_CALL disposing();

private:
    sal_Bool init_Impl( const ::rtl::OUString& rInitialDocumentURL,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxInitialStorage );
    void implScanExtensions( void );

public:
    SfxLibraryContainer( void );
    ~SfxLibraryContainer();


    // Interface to set the BasicManager (Hack for password implementation)
    void setBasicManager( BasicManager* pBasMgr )
    {
        mpBasMgr = pBasMgr;
    }

    void    enterMethod();
    void    leaveMethod();
    bool    isDisposed() const { return rBHelper.bInDispose || rBHelper.bDisposed; }
    void    checkDisposed() const;

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // Members XStorageBasedLibraryContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getRootStorage() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRootStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rootstorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeLibrariesToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& RootStorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Methods XModifiable (base of XPersistentLibraryContainer)
    virtual ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setModified( ::sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // Methods XPersistentLibraryContainer (base of XStorageBasedLibraryContainer)
    virtual ::com::sun::star::uno::Any SAL_CALL getRootLocation() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getContainerLocationName() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeLibraries(  ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //Methods XLibraryContainer3
    virtual ::rtl::OUString SAL_CALL getOriginalLibraryLinkURL( const ::rtl::OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);

    // Methods XLibraryContainer2 (base of XPersistentLibraryContainer)
    virtual sal_Bool SAL_CALL isLibraryLink( const ::rtl::OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLibraryLinkURL( const ::rtl::OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLibraryReadOnly( const ::rtl::OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLibraryReadOnly( const ::rtl::OUString& Name, sal_Bool bReadOnly )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL renameLibrary( const ::rtl::OUString& Name, const ::rtl::OUString& NewName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::container::ElementExistException,
               ::com::sun::star::uno::RuntimeException);

    // Methods XLibraryContainer (base of XLibraryContainer2)
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL
        createLibrary( const ::rtl::OUString& Name )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL createLibraryLink
        ( const ::rtl::OUString& Name, const ::rtl::OUString& StorageURL, sal_Bool ReadOnly )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeLibrary( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLibraryLoaded( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL loadLibrary( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::uno::RuntimeException);

    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const ::rtl::OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const ::rtl::OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const ::rtl::OUString& Name, const ::rtl::OUString& Password )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changeLibraryPassword( const ::rtl::OUString& Name,
        const ::rtl::OUString& OldPassword, const ::rtl::OUString& NewPassword )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);

    // Methods XLibraryContainerExport
    virtual void SAL_CALL exportLibrary( const ::rtl::OUString& Name, const ::rtl::OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::container::NoSuchElementException,
                   ::com::sun::star::uno::RuntimeException);

    // Methods XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName( )
        throw (::com::sun::star::uno::RuntimeException) = 0;
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames( )
        throw (::com::sun::star::uno::RuntimeException) = 0;
    // Methods XVBACompatibility
    virtual ::sal_Bool SAL_CALL getVBACompatibilityMode()
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setVBACompatibilityMode( ::sal_Bool _vbacompatmodeon )
            throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getRunningVBAScripts()
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVBAScriptListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::vba::XVBAScriptListener >& Listener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVBAScriptListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::vba::XVBAScriptListener >& Listener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL broadcastVBAScriptEvent( sal_Int32 nIdentifier, const ::rtl::OUString& rModuleName )
            throw (::com::sun::star::uno::RuntimeException);
};

//============================================================================

class LibraryContainerMethodGuard
{
private:
    SfxLibraryContainer&    m_rContainer;
public:
    LibraryContainerMethodGuard( SfxLibraryContainer& _rContainer )
        :m_rContainer( _rContainer )
    {
        m_rContainer.enterMethod();
    }

    ~LibraryContainerMethodGuard()
    {
        m_rContainer.leaveMethod();
    }
};

//============================================================================

class SfxLibrary
    : public ::com::sun::star::container::XNameContainer
    , public ::com::sun::star::container::XContainer
    , public ::com::sun::star::util::XChangesNotifier
    , public ::cppu::BaseMutex
    , public ::cppu::OComponentHelper
{
    friend class SfxLibraryContainer;
    friend class SfxDialogLibraryContainer;
    friend class SfxScriptLibraryContainer;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >   mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >       mxSFI;

    ModifiableHelper&   mrModifiable;
    NameContainer       maNameContainer;

    sal_Bool mbLoaded;
    sal_Bool mbIsModified;
    sal_Bool mbInitialised;

private:

    ::rtl::OUString maLibElementFileExtension;
    ::rtl::OUString maLibInfoFileURL;
    ::rtl::OUString maStorageURL;
    ::rtl::OUString maUnexpandedStorageURL;
    ::rtl::OUString maOrignialStorageURL;

    sal_Bool mbLink;
    sal_Bool mbReadOnly;
    sal_Bool mbReadOnlyLink;
    sal_Bool mbPreload;

    sal_Bool mbPasswordProtected;
    sal_Bool mbPasswordVerified;
    sal_Bool mbDoc50Password;
    ::rtl::OUString maPassword;

    sal_Bool mbSharedIndexFile;
    sal_Bool mbExtension;

    // Additional functionality for localisation
    // Provide modify state including resources
    virtual sal_Bool isModified( void ) = 0;
    virtual void storeResources( void ) = 0;
    virtual void storeResourcesAsURL( const ::rtl::OUString& URL, const ::rtl::OUString& NewName ) = 0;
    virtual void storeResourcesToURL( const ::rtl::OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler ) = 0;
    virtual void storeResourcesToStorage( const ::com::sun::star::uno::Reference
        < ::com::sun::star::embed::XStorage >& xStorage ) = 0;

protected:
    inline  sal_Bool    implIsModified() const  { return mbIsModified; }
            void        implSetModified( sal_Bool _bIsModified );

private:
    /** checks whether the lib is readonly, or a readonly link, throws an IllegalArgumentException if so
    */
    void    impl_checkReadOnly();
    /** checks whether the library is loaded, throws a LibraryNotLoadedException (wrapped in a WrappedTargetException),
        if not.
    */
    void    impl_checkLoaded();

private:
    void    impl_removeWithoutChecks( const ::rtl::OUString& _rElementName );

public:
    SfxLibrary(
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Type& aType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSF,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xSFI
    );
    SfxLibrary(
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Type& aType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSF,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xSFI,
        const ::rtl::OUString& aLibInfoFileURL,
        const ::rtl::OUString&
        aStorageURL,
        sal_Bool ReadOnly
    );

    // Methods XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw() { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw() { OComponentHelper::release(); }

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  )
        throw( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  )
        throw( ::com::sun::star::uno::RuntimeException );

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);

    // Methods XChangesNotifier
    virtual void SAL_CALL addChangesListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XChangesListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChangesListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XChangesListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);

public:
    struct LibraryContainerAccess { friend class SfxLibraryContainer; private: LibraryContainerAccess() { } };
    void    removeElementWithoutChecks( const ::rtl::OUString& _rElementName, LibraryContainerAccess )
    {
        impl_removeWithoutChecks( _rElementName );
    }

protected:
    virtual bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const = 0;
};

//============================================================================

class ScriptSubPackageIterator
{
    com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > m_xMainPackage;

    bool m_bIsValid;
    bool m_bIsBundle;

    com::sun::star::uno::Sequence< com::sun::star::uno::Reference
        < com::sun::star::deployment::XPackage > > m_aSubPkgSeq;
    sal_Int32 m_nSubPkgCount;
    sal_Int32 m_iNextSubPkg;

    com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >
        implDetectScriptPackage( const com::sun::star::uno::Reference
            < com::sun::star::deployment::XPackage > xPackage, bool& rbPureDialogLib );

public:
    ScriptSubPackageIterator( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xMainPackage );

    com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > getNextScriptSubPackage( bool& rbPureDialogLib );
};

//============================================================================

class ScriptExtensionIterator
{
public:
    ScriptExtensionIterator( void );
    rtl::OUString nextBasicOrDialogLibrary( bool& rbPureDialogLib );

private:
    com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetScriptPackageFromPackage
        ( const com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage,
          bool& rbPureDialogLib );

protected:
    com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >
        implGetNextUserScriptPackage( bool& rbPureDialogLib );
    com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >
        implGetNextSharedScriptPackage( bool& rbPureDialogLib );
    com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >
        implGetNextBundledScriptPackage( bool& rbPureDialogLib );

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xContext;

    enum IteratorState
    {
        USER_EXTENSIONS,
        SHARED_EXTENSIONS,
        BUNDLED_EXTENSIONS,
        END_REACHED
    } m_eState;

    com::sun::star::uno::Sequence< com::sun::star::uno::Reference
        < com::sun::star::deployment::XPackage > > m_aUserPackagesSeq;
    bool m_bUserPackagesLoaded;

    com::sun::star::uno::Sequence< com::sun::star::uno::Reference
        < com::sun::star::deployment::XPackage > > m_aSharedPackagesSeq;
    bool m_bSharedPackagesLoaded;

      com::sun::star::uno::Sequence< com::sun::star::uno::Reference
        < com::sun::star::deployment::XPackage > > m_aBundledPackagesSeq;
    bool m_bBundledPackagesLoaded;

    int m_iUserPackage;
    int m_iSharedPackage;
       int m_iBundledPackage;

    ScriptSubPackageIterator* m_pScriptSubPackageIterator;

}; // end class ScriptExtensionIterator

//============================================================================

}   // namespace basic

#endif

