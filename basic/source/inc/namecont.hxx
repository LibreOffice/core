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

#include <unordered_map>
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
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/basemutex.hxx>
#include <sot/storage.hxx>
#include <comphelper/listenernotification.hxx>
#include <xmlscript/xmllib_imexp.hxx>

class BasicManager;

namespace basic
{
typedef ::cppu::WeakImplHelper<
    css::container::XNameContainer,
    css::container::XContainer,
    css::util::XChangesNotifier > NameContainer_BASE;



class NameContainer : public ::cppu::BaseMutex, public NameContainer_BASE
{
    typedef std::unordered_map < OUString, sal_Int32, OUStringHash > NameContainerNameMap;

    NameContainerNameMap mHashMap;
    css::uno::Sequence< OUString > mNames;
    css::uno::Sequence< css::uno::Any > mValues;
    sal_Int32 mnElementCount;

    css::uno::Type mType;
    css::uno::XInterface* mpxEventSource;

    ::cppu::OInterfaceContainerHelper maContainerListeners;
    ::cppu::OInterfaceContainerHelper maChangesListeners;

public:
    NameContainer( const css::uno::Type& rType )
        : mnElementCount( 0 )
        , mType( rType )
        , mpxEventSource( NULL )
        , maContainerListeners( m_aMutex )
        , maChangesListeners( m_aMutex )
    {}

    void setEventSource( css::uno::XInterface* pxEventSource )
        { mpxEventSource = pxEventSource; }

    void insertCheck(const OUString& aName, const css::uno::Any& aElement)
        throw (css::lang::IllegalArgumentException,
               css::container::ElementExistException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception);

    void insertNoCheck(const OUString& aName, const css::uno::Any& aElement)
        throw (css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception);

    // Methods XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(css::uno::RuntimeException, std::exception) override;

    // Methods XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(css::uno::RuntimeException, std::exception) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement )
        throw(css::lang::IllegalArgumentException,
              css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement )
        throw(css::lang::IllegalArgumentException,
              css::container::ElementExistException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference<css::container::XContainerListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference<css::container::XContainerListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;

    // Methods XChangesNotifier
    virtual void SAL_CALL addChangesListener( const css::uno::Reference<css::util::XChangesListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChangesListener( const css::uno::Reference<css::util::XChangesListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;
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

    inline  void    addModifyListener( const css::uno::Reference< css::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.addInterface( _rxListener );
    }

    inline  void    removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.removeInterface( _rxListener );
    }
};



typedef ::comphelper::OListenerContainerBase<
    css::script::vba::XVBAScriptListener,
    css::script::vba::VBAScriptEvent > VBAScriptListenerContainer_BASE;

class VBAScriptListenerContainer : public VBAScriptListenerContainer_BASE
{
public:
    explicit VBAScriptListenerContainer( ::osl::Mutex& rMutex );

private:
    virtual bool implTypedNotify(
        const css::uno::Reference< css::script::vba::XVBAScriptListener >& rxListener,
        const css::script::vba::VBAScriptEvent& rEvent )
        throw (css::uno::Exception) override;
};



class SfxLibrary;

typedef ::cppu::WeakComponentImplHelper<
    css::lang::XInitialization,
    css::script::XStorageBasedLibraryContainer,
    css::script::XLibraryContainerPassword,
    css::script::XLibraryContainerExport,
    css::script::XLibraryContainer3,
    css::container::XContainer,
    css::script::XLibraryQueryExecutable,
    css::script::vba::XVBACompatibility,
    css::lang::XServiceInfo > SfxLibraryContainer_BASE;

class SfxLibraryContainer : public SfxLibraryContainer_BASE, public ::utl::OEventListenerAdapter
{
    VBAScriptListenerContainer maVBAScriptListeners;
    sal_Int32 mnRunningVBAScripts;
    bool mbVBACompat;
    OUString msProjectName;
protected:
    css::uno::Reference< css::uno::XComponentContext >       mxContext;
    css::uno::Reference< css::ucb::XSimpleFileAccess3 >      mxSFI;
    css::uno::Reference< css::util::XStringSubstitution >    mxStringSubstitution;
    css::uno::WeakReference< css::frame::XModel >            mxOwnerDocument;

    ::osl::Mutex        maMutex;
    ModifiableHelper    maModifiable;

    css::uno::Reference<NameContainer> maNameContainer;
    bool    mbOldInfoFormat;
    bool    mbOasis2OOoFormat;

    OUString maInitialDocumentURL;
    OUString maInfoFileName;
    OUString maOldInfoFileName;
    OUString maLibElementFileExtension;
    OUString maLibraryPath;
    OUString maLibrariesDir;

    css::uno::Reference< css::embed::XStorage > mxStorage;
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
                            const OUString& rName,
                            const css::uno::Reference< css::embed::XStorage >& rStorage );

    // New variant for library export
    void implStoreLibrary( SfxLibrary* pLib,
                            const OUString& rName,
                            const css::uno::Reference< css::embed::XStorage >& rStorage,
                            const OUString& rTargetURL,
                            const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& rToUseSFI,
                            const css::uno::Reference< css::task::XInteractionHandler >& rHandler );

    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const css::uno::Reference< css::embed::XStorage >& xStorage );

    // New variant for library export
    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    const OUString& aTargetURL,
                                    const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& rToUseSFI );

    bool implLoadLibraryIndexFile( SfxLibrary* pLib,
                                    ::xmlscript::LibDescriptor& rLib,
                                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    const OUString& aIndexFileName );

    void implImportLibDescriptor( SfxLibrary* pLib, ::xmlscript::LibDescriptor& rLib );

    // Methods to distinguish between deffirent library types
    virtual SfxLibrary* SAL_CALL implCreateLibrary( const OUString& aName ) = 0;
    virtual SfxLibrary* SAL_CALL implCreateLibraryLink
        ( const OUString& aName, const OUString& aLibInfoFileURL,
          const OUString& StorageURL, bool ReadOnly ) = 0;
    virtual css::uno::Any SAL_CALL createEmptyLibraryElement() = 0;
    virtual bool SAL_CALL isLibraryElementValid(const css::uno::Any& rElement) const = 0;
    virtual void SAL_CALL writeLibraryElement
    (
        const css::uno::Reference< css::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const css::uno::Reference< css::io::XOutputStream >& xOutput
    )
        throw(css::uno::Exception) = 0;

    virtual css::uno::Any SAL_CALL importLibraryElement
    (
        const css::uno::Reference< css::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const OUString& aFile,
        const css::uno::Reference< css::io::XInputStream >& xElementStream ) = 0;
    virtual void SAL_CALL importFromOldStorage( const OUString& aFile ) = 0;

    // Password encryption
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    // New variant for library export
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const css::uno::Reference< css::embed::XStorage >& rStorage,
                        const OUString& aTargetURL,
                        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& rToUseSFI, const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    virtual bool implLoadPasswordLibrary( SfxLibrary* pLib, const OUString& Name,
                                          bool bVerifyPasswordOnly=false )
            throw(css::lang::WrappedTargetException,
                  css::uno::RuntimeException, std::exception);

    virtual void onNewRootStorage() = 0;


    // #56666, Creates another library container
    //         instance of the same derived class
    virtual SfxLibraryContainer* createInstanceImpl() = 0;


    // Interface to get the BasicManager (Hack for password implementation)
    BasicManager* getBasicManager();
    OUString createAppLibraryFolder( SfxLibrary* pLib, const OUString& aName );

    void init( const OUString& rInitialDocumentURL,
               const css::uno::Reference< css::embed::XStorage >& _rxInitialStorage );

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
        throw(css::uno::RuntimeException);

    SfxLibrary* getImplLib( const OUString& rLibraryName );

    void storeLibraries_Impl(
                            const css::uno::Reference< css::embed::XStorage >& xStorage,
                            bool bComplete );

    void SAL_CALL initializeFromDocumentURL( const OUString& _rInitialDocumentURL );
    void SAL_CALL initializeFromDocument( const css::uno::Reference< css::document::XStorageBasedDocument >& _rxDocument );

    // OEventListenerAdapter
    virtual void _disposing( const css::lang::EventObject& _rSource ) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

private:
    void init_Impl( const OUString& rInitialDocumentURL,
                    const css::uno::Reference< css::embed::XStorage >& _rxInitialStorage );
    void implScanExtensions();

public:
    SfxLibraryContainer();
    virtual ~SfxLibraryContainer();


    // Interface to set the BasicManager (Hack for password implementation)
    void setBasicManager( BasicManager* pBasMgr )
    {
        mpBasMgr = pBasMgr;
    }

    void    enterMethod();
    static void leaveMethod();
    bool    isDisposed() const { return rBHelper.bInDispose || rBHelper.bDisposed; }
    void    checkDisposed() const;

    // Methods XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw(css::uno::RuntimeException, std::exception) override;

    // Methods XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(css::uno::RuntimeException, std::exception) override;

    // Members XStorageBasedLibraryContainer
    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getRootStorage() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRootStorage( const css::uno::Reference< css::embed::XStorage >& _rootstorage ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeLibrariesToStorage( const css::uno::Reference< css::embed::XStorage >& RootStorage ) throw (css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // Methods XModifiable (base of XPersistentLibraryContainer)
    virtual sal_Bool SAL_CALL isModified(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // Methods XPersistentLibraryContainer (base of XStorageBasedLibraryContainer)
    virtual css::uno::Any SAL_CALL getRootLocation() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getContainerLocationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeLibraries(  ) throw (css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //Methods XLibraryContainer3
    virtual OUString SAL_CALL getOriginalLibraryLinkURL( const OUString& Name )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;

    // Methods XLibraryContainer2 (base of XPersistentLibraryContainer)
    virtual sal_Bool SAL_CALL isLibraryLink( const OUString& Name )
        throw (css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLibraryLinkURL( const OUString& Name )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isLibraryReadOnly( const OUString& Name )
        throw (css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLibraryReadOnly( const OUString& Name, sal_Bool bReadOnly )
        throw (css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL renameLibrary( const OUString& Name, const OUString& NewName )
        throw (css::container::NoSuchElementException,
               css::container::ElementExistException,
               css::uno::RuntimeException, std::exception) override;

    // Methods XLibraryContainer (base of XLibraryContainer2)
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL
        createLibrary( const OUString& Name )
            throw(css::lang::IllegalArgumentException,
                  css::container::ElementExistException,
                  css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL createLibraryLink
        ( const OUString& Name, const OUString& StorageURL, sal_Bool ReadOnly )
            throw(css::lang::IllegalArgumentException,
                  css::container::ElementExistException,
                  css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeLibrary( const OUString& Name )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isLibraryLoaded( const OUString& Name )
        throw(css::container::NoSuchElementException,
              css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL loadLibrary( const OUString& Name )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;

    // Methods XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence<
        css::uno::Any >& aArguments )
            throw (css::uno::Exception,
                   css::uno::RuntimeException, std::exception) override;

    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const OUString& Name )
        throw (css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const OUString& Name )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const OUString& Name, const OUString& Password )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL changeLibraryPassword( const OUString& Name,
        const OUString& OldPassword, const OUString& NewPassword )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference<
        css::container::XContainerListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference<
        css::container::XContainerListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;

    // Methods XLibraryContainerExport
    virtual void SAL_CALL exportLibrary( const OUString& Name, const OUString& URL,
        const css::uno::Reference< css::task::XInteractionHandler >& Handler )
            throw (css::uno::Exception,
                   css::container::NoSuchElementException,
                   css::uno::RuntimeException, std::exception) override;

    // Methods XServiceInfo
    virtual OUString SAL_CALL getImplementationName( )
        throw (css::uno::RuntimeException, std::exception) override = 0;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( )
        throw (css::uno::RuntimeException, std::exception) override = 0;
    // Methods XVBACompatibility
    virtual sal_Bool SAL_CALL getVBACompatibilityMode() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setVBACompatibilityMode( sal_Bool _vbacompatmodeon ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getProjectName() throw (css::uno::RuntimeException, std::exception) override { return msProjectName; }
    virtual void SAL_CALL setProjectName( const OUString& _projectname ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getRunningVBAScripts()
            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVBAScriptListener(
        const css::uno::Reference< css::script::vba::XVBAScriptListener >& Listener )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVBAScriptListener(
        const css::uno::Reference< css::script::vba::XVBAScriptListener >& Listener )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL broadcastVBAScriptEvent( sal_Int32 nIdentifier, const OUString& rModuleName )
            throw (css::uno::RuntimeException, std::exception) override;
};



class LibraryContainerMethodGuard
{
public:
    LibraryContainerMethodGuard( SfxLibraryContainer& _rContainer )
    {
        _rContainer.enterMethod();
    }

    ~LibraryContainerMethodGuard()
    {
        basic::SfxLibraryContainer::leaveMethod();
    }
};



class SfxLibrary
    : public css::container::XNameContainer
    , public css::container::XContainer
    , public css::util::XChangesNotifier
    , public ::cppu::BaseMutex
    , public ::cppu::OComponentHelper
{
    friend class SfxLibraryContainer;
    friend class SfxDialogLibraryContainer;
    friend class SfxScriptLibraryContainer;

    css::uno::Reference< css::uno::XComponentContext >    mxContext;
    css::uno::Reference< css::ucb::XSimpleFileAccess3 >   mxSFI;

    ModifiableHelper&                                     mrModifiable;
    css::uno::Reference<NameContainer>                    maNameContainer;

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

protected:
    bool mbPasswordProtected;
private:
    bool mbPasswordVerified;
    bool mbDoc50Password;
    OUString maPassword;

    bool mbSharedIndexFile;
    bool mbExtension;

    // Additional functionality for localisation
    // Provide modify state including resources
    virtual bool isModified() = 0;
    virtual void storeResources() = 0;
    virtual void storeResourcesAsURL( const OUString& URL, const OUString& NewName ) = 0;
    virtual void storeResourcesToURL( const OUString& URL,
        const css::uno::Reference< css::task::XInteractionHandler >& xHandler ) = 0;
    virtual void storeResourcesToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage ) = 0;

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
        const css::uno::Type& aType,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI
    );
    SfxLibrary(
        ModifiableHelper& _rModifiable,
        const css::uno::Type& aType,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI,
        const OUString& aLibInfoFileURL,
        const OUString& aStorageURL,
        bool ReadOnly
    );

    // Methods XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw() override { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw() override { OComponentHelper::release(); }

    // Methods XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(css::uno::RuntimeException, std::exception) override;

    // Methods XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(css::uno::RuntimeException, std::exception) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement )
        throw(css::lang::IllegalArgumentException,
              css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement )
        throw(css::lang::IllegalArgumentException,
              css::container::ElementExistException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  )
        throw( css::uno::RuntimeException, std::exception ) override;
    css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  )
        throw( css::uno::RuntimeException, std::exception ) override;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference<
        css::container::XContainerListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference<
        css::container::XContainerListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;

    // Methods XChangesNotifier
    virtual void SAL_CALL addChangesListener( const css::uno::Reference<
        css::util::XChangesListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChangesListener( const css::uno::Reference<
        css::util::XChangesListener >& xListener )
            throw (css::uno::RuntimeException, std::exception) override;

public:
    struct LibraryContainerAccess { friend class SfxLibraryContainer; private: LibraryContainerAccess() { } };
    void    removeElementWithoutChecks( const OUString& _rElementName, LibraryContainerAccess )
    {
        impl_removeWithoutChecks( _rElementName );
    }

protected:
    virtual bool isLoadedStorable();

    virtual bool SAL_CALL isLibraryElementValid(const css::uno::Any& rElement) const = 0;
};



class ScriptSubPackageIterator
{
    css::uno::Reference< css::deployment::XPackage > m_xMainPackage;

    bool m_bIsValid;
    bool m_bIsBundle;

    css::uno::Sequence< css::uno::Reference< css::deployment::XPackage > > m_aSubPkgSeq;
    sal_Int32 m_nSubPkgCount;
    sal_Int32 m_iNextSubPkg;

    static css::uno::Reference< css::deployment::XPackage >
        implDetectScriptPackage( const css::uno::Reference
            < css::deployment::XPackage >& rPackage, bool& rbPureDialogLib );

public:
    ScriptSubPackageIterator( css::uno::Reference< css::deployment::XPackage > xMainPackage );

    css::uno::Reference< css::deployment::XPackage > getNextScriptSubPackage( bool& rbPureDialogLib );
};



class ScriptExtensionIterator
{
public:
    ScriptExtensionIterator();
    OUString nextBasicOrDialogLibrary( bool& rbPureDialogLib );

protected:
    css::uno::Reference< css::deployment::XPackage >
        implGetNextUserScriptPackage( bool& rbPureDialogLib );
    css::uno::Reference< css::deployment::XPackage >
        implGetNextSharedScriptPackage( bool& rbPureDialogLib );
    css::uno::Reference< css::deployment::XPackage >
        implGetNextBundledScriptPackage( bool& rbPureDialogLib );

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    enum IteratorState
    {
        USER_EXTENSIONS,
        SHARED_EXTENSIONS,
        BUNDLED_EXTENSIONS,
        END_REACHED
    } m_eState;

    css::uno::Sequence< css::uno::Reference< css::deployment::XPackage > > m_aUserPackagesSeq;
    bool m_bUserPackagesLoaded;

    css::uno::Sequence< css::uno::Reference< css::deployment::XPackage > > m_aSharedPackagesSeq;
    bool m_bSharedPackagesLoaded;

    css::uno::Sequence< css::uno::Reference< css::deployment::XPackage > > m_aBundledPackagesSeq;
    bool m_bBundledPackagesLoaded;

    int m_iUserPackage;
    int m_iSharedPackage;
       int m_iBundledPackage;

    ScriptSubPackageIterator* m_pScriptSubPackageIterator;

}; // end class ScriptExtensionIterator



}   // namespace basic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
