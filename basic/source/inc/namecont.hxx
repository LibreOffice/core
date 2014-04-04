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

#ifndef INCLUDED_BASIC_SOURCE_INC_NAMECONT_HXX
#define INCLUDED_BASIC_SOURCE_INC_NAMECONT_HXX

#include <boost/unordered_map.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/script/XLibraryContainerExport.hpp>
#include <com/sun/star/script/XLibraryQueryExecutable.hpp>
#include <com/sun/star/script/XLibraryContainer3.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
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
#include <cppuhelper/basemutex.hxx>
#include <sot/storage.hxx>
#include <comphelper/listenernotification.hxx>
#include <xmlscript/xmllib_imexp.hxx>
#include <cppuhelper/compbase9.hxx>

class BasicManager;

namespace basic
{
typedef ::cppu::WeakImplHelper3<
    ::com::sun::star::container::XNameContainer,
    ::com::sun::star::container::XContainer,
    ::com::sun::star::util::XChangesNotifier > NameContainer_BASE;



class NameContainer : public ::cppu::BaseMutex, public NameContainer_BASE
{
    typedef boost::unordered_map < OUString, sal_Int32, OUStringHash > NameContainerNameMap;

    NameContainerNameMap mHashMap;
    ::com::sun::star::uno::Sequence< OUString > mNames;
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
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XChangesNotifier
    virtual void SAL_CALL addChangesListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XChangesListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeChangesListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XChangesListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};



class ModifiableHelper
{
private:
    ::cppu::OInterfaceContainerHelper   m_aModifyListeners;
    ::cppu::OWeakObject&                m_rEventSource;
    bool                                mbModified;

public:
    ModifiableHelper( ::cppu::OWeakObject& _rEventSource, ::osl::Mutex& _rMutex )
        :m_aModifyListeners( _rMutex )
        ,m_rEventSource( _rEventSource )
        ,mbModified( false )
    {
    }

    inline  bool    isModified() const  { return mbModified; }
            void    setModified( bool _bModified );

    inline  void    addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.addInterface( _rxListener );
    }

    inline  void    removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.removeInterface( _rxListener );
    }
};



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
        throw (::com::sun::star::uno::Exception) SAL_OVERRIDE;
};



class SfxLibrary;

typedef ::cppu::WeakComponentImplHelper9<
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::script::XStorageBasedLibraryContainer,
    ::com::sun::star::script::XLibraryContainerPassword,
    ::com::sun::star::script::XLibraryContainerExport,
    ::com::sun::star::script::XLibraryContainer3,
    ::com::sun::star::container::XContainer,
    ::com::sun::star::script::XLibraryQueryExecutable,
    ::com::sun::star::script::vba::XVBACompatibility,
    ::com::sun::star::lang::XServiceInfo > SfxLibraryContainer_BASE;

class SfxLibraryContainer : public SfxLibraryContainer_BASE, public ::utl::OEventListenerAdapter
{
    VBAScriptListenerContainer maVBAScriptListeners;
    sal_Int32 mnRunningVBAScripts;
    bool mbVBACompat;
    OUString msProjectName;
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >       mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >      mxSFI;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XMacroExpander >         mxMacroExpander;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringSubstitution >    mxStringSubstitution;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >            mxOwnerDocument;

    ::osl::Mutex        maMutex;
    ModifiableHelper    maModifiable;

    NameContainer maNameContainer;
    bool    mbOldInfoFormat;
    bool    mbOasis2OOoFormat;

    OUString maInitialDocumentURL;
    OUString maInfoFileName;
    OUString maOldInfoFileName;
    OUString maLibElementFileExtension;
    OUString maLibraryPath;
    OUString maLibrariesDir;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > mxStorage;
    BasicManager*   mpBasMgr;
    bool        mbOwnBasMgr;

    enum InitMode
    {
        DEFAULT,
        CONTAINER_INIT_FILE,
        LIBRARY_INIT_FILE,
        OFFICE_DOCUMENT,
        OLD_BASIC_STORAGE
    } meInitMode;

    void implStoreLibrary( SfxLibrary* pLib,
                            const OUString& aName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    // New variant for library export
    void implStoreLibrary( SfxLibrary* pLib,
                            const OUString& aName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                            const OUString& aTargetURL,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 > xToUseSFI,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );

    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    // New variant for library export
    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    const OUString& aTargetURL,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 > xToUseSFI );

    bool implLoadLibraryIndexFile( SfxLibrary* pLib,
                                    ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    const OUString& aIndexFileName );

    void implImportLibDescriptor( SfxLibrary* pLib, ::xmlscript::LibDescriptor& rLib );

    // Methods to distinguish between deffirent library types
    virtual SfxLibrary* SAL_CALL implCreateLibrary( const OUString& aName ) = 0;
    virtual SfxLibrary* SAL_CALL implCreateLibraryLink
        ( const OUString& aName, const OUString& aLibInfoFileURL,
          const OUString& StorageURL, bool ReadOnly ) = 0;
    virtual ::com::sun::star::uno::Any SAL_CALL createEmptyLibraryElement( void ) = 0;
    virtual bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const = 0;
    virtual void SAL_CALL writeLibraryElement
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutput
    )
        throw(::com::sun::star::uno::Exception) = 0;

    virtual ::com::sun::star::uno::Any SAL_CALL importLibraryElement
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const OUString& aFile,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xElementStream ) = 0;
    virtual void SAL_CALL importFromOldStorage( const OUString& aFile ) = 0;

    // Password encryption
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );

    // New variant for library export
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                        const OUString& aTargetURL,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 > xToUseSFI, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );

    virtual bool implLoadPasswordLibrary( SfxLibrary* pLib, const OUString& Name,
                                          bool bVerifyPasswordOnly=false )
            throw(::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);

    virtual void onNewRootStorage() = 0;


    // #56666, Creates another library container
    //         instance of the same derived class
    virtual SfxLibraryContainer* createInstanceImpl( void ) = 0;


    // Interface to get the BasicManager (Hack for password implementation)
    BasicManager* getBasicManager( void );
    OUString createAppLibraryFolder( SfxLibrary* pLib, const OUString& aName );

    void init( const OUString& rInitialDocumentURL,
               const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxInitialStorage );

    virtual const sal_Char* SAL_CALL    getInfoFileName() const = 0;
    virtual const sal_Char* SAL_CALL    getOldInfoFileName() const = 0;
    virtual const sal_Char* SAL_CALL    getLibElementFileExtension() const = 0;
    virtual const sal_Char* SAL_CALL    getLibrariesDir() const = 0;

    // Handle maLibInfoFileURL and maStorageURL correctly
    void checkStorageURL
    (
        const OUString& aSourceURL,
        OUString& aLibInfoFileURL,
        OUString& aStorageURL,
        OUString& aUnexpandedStorageURL
    );
    OUString expand_url( const OUString& url )
        throw(::com::sun::star::uno::RuntimeException);

    SfxLibrary* getImplLib( const OUString& rLibraryName );

    void storeLibraries_Impl(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                            bool bComplete );

    void SAL_CALL initializeFromDocumentURL( const OUString& _rInitialDocumentURL );
    void SAL_CALL initializeFromDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageBasedDocument >& _rxDocument );

    // OEventListenerAdapter
    virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource ) SAL_OVERRIDE;

    // OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

private:
    void init_Impl( const OUString& rInitialDocumentURL,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxInitialStorage );
    void implScanExtensions( void );

public:
    SfxLibraryContainer( void );
    virtual ~SfxLibraryContainer();


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
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements()
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Members XStorageBasedLibraryContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getRootStorage() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRootStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rootstorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL storeLibrariesToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& RootStorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XModifiable (base of XPersistentLibraryContainer)
    virtual sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XPersistentLibraryContainer (base of XStorageBasedLibraryContainer)
    virtual ::com::sun::star::uno::Any SAL_CALL getRootLocation() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getContainerLocationName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL storeLibraries(  ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //Methods XLibraryContainer3
    virtual OUString SAL_CALL getOriginalLibraryLinkURL( const OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XLibraryContainer2 (base of XPersistentLibraryContainer)
    virtual sal_Bool SAL_CALL isLibraryLink( const OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getLibraryLinkURL( const OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLibraryReadOnly( const OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLibraryReadOnly( const OUString& Name, sal_Bool bReadOnly )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL renameLibrary( const OUString& Name, const OUString& NewName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::container::ElementExistException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XLibraryContainer (base of XLibraryContainer2)
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL
        createLibrary( const OUString& Name )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL createLibraryLink
        ( const OUString& Name, const OUString& StorageURL, sal_Bool ReadOnly )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeLibrary( const OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLibraryLoaded( const OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL loadLibrary( const OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const OUString& Name, const OUString& Password )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL changeLibraryPassword( const OUString& Name,
        const OUString& OldPassword, const OUString& NewPassword )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XLibraryContainerExport
    virtual void SAL_CALL exportLibrary( const OUString& Name, const OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::container::NoSuchElementException,
                   ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XServiceInfo
    virtual OUString SAL_CALL getImplementationName( )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;
    // Methods XVBACompatibility
    virtual sal_Bool SAL_CALL getVBACompatibilityMode() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVBACompatibilityMode( sal_Bool _vbacompatmodeon ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getProjectName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return msProjectName; }
    virtual void SAL_CALL setProjectName( const OUString& _projectname ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getRunningVBAScripts()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVBAScriptListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::vba::XVBAScriptListener >& Listener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVBAScriptListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::vba::XVBAScriptListener >& Listener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL broadcastVBAScriptEvent( sal_Int32 nIdentifier, const OUString& rModuleName )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};



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

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >       mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >      mxSFI;

    ModifiableHelper&   mrModifiable;
    NameContainer       maNameContainer;

    bool mbLoaded;
    bool mbIsModified;
    bool mbInitialised;

private:

    OUString maLibElementFileExtension;
    OUString maLibInfoFileURL;
    OUString maStorageURL;
    OUString maUnexpandedStorageURL;
    OUString maOriginalStorageURL;

    bool mbLink;
    bool mbReadOnly;
    bool mbReadOnlyLink;
    bool mbPreload;

    bool mbPasswordProtected;
    bool mbPasswordVerified;
    bool mbDoc50Password;
    OUString maPassword;

    bool mbSharedIndexFile;
    bool mbExtension;

    // Additional functionality for localisation
    // Provide modify state including resources
    virtual bool isModified( void ) = 0;
    virtual void storeResources( void ) = 0;
    virtual void storeResourcesAsURL( const OUString& URL, const OUString& NewName ) = 0;
    virtual void storeResourcesToURL( const OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler ) = 0;
    virtual void storeResourcesToStorage( const ::com::sun::star::uno::Reference
        < ::com::sun::star::embed::XStorage >& xStorage ) = 0;

protected:
    inline  bool    implIsModified() const  { return mbIsModified; }
            void    implSetModified( bool _bIsModified );

private:
    /** checks whether the lib is readonly, or a readonly link, throws an IllegalArgumentException if so
    */
    void    impl_checkReadOnly();
    /** checks whether the library is loaded, throws a LibraryNotLoadedException (wrapped in a WrappedTargetException),
        if not.
    */
    void    impl_checkLoaded();

private:
    void    impl_removeWithoutChecks( const OUString& _rElementName );

public:
    SfxLibrary(
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Type& aType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >& xSFI
    );
    SfxLibrary(
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Type& aType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >& xSFI,
        const OUString& aLibInfoFileURL,
        const OUString& aStorageURL,
        bool ReadOnly
    );

    // Methods XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw() SAL_OVERRIDE { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw() SAL_OVERRIDE { OComponentHelper::release(); }

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods XChangesNotifier
    virtual void SAL_CALL addChangesListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XChangesListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeChangesListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XChangesListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public:
    struct LibraryContainerAccess { friend class SfxLibraryContainer; private: LibraryContainerAccess() { } };
    void    removeElementWithoutChecks( const OUString& _rElementName, LibraryContainerAccess )
    {
        impl_removeWithoutChecks( _rElementName );
    }

protected:
    virtual bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const = 0;
};



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



class ScriptExtensionIterator
{
public:
    ScriptExtensionIterator( void );
    OUString nextBasicOrDialogLibrary( bool& rbPureDialogLib );

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



}   // namespace basic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
