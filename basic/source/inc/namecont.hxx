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

#pragma once

#include <unordered_map>
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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAScriptListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>

#include <osl/mutex.hxx>
#include <unotools/eventlisteneradapter.hxx>
#include <comphelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <xmlscript/xmllib_imexp.hxx>

class BasicManager;

namespace basic
{
typedef ::comphelper::WeakImplHelper<
    css::container::XNameContainer,
    css::container::XContainer,
    css::util::XChangesNotifier > NameContainer_BASE;


class NameContainer final : public NameContainer_BASE
{
    typedef std::unordered_map < OUString, sal_Int32 > NameContainerNameMap;

    NameContainerNameMap mHashMap;
    std::vector< OUString > mNames;
    std::vector< css::uno::Any > mValues;
    sal_Int32 mnElementCount;

    css::uno::Type mType;
    css::uno::XInterface* mpxEventSource;

    ::comphelper::OInterfaceContainerHelper4<css::container::XContainerListener> maContainerListeners;
    ::comphelper::OInterfaceContainerHelper4<css::util::XChangesListener> maChangesListeners;

public:
    NameContainer( const css::uno::Type& rType )
        : mnElementCount( 0 )
        , mType( rType )
        , mpxEventSource( nullptr )
    {}

    void setEventSource( css::uno::XInterface* pxEventSource )
        { mpxEventSource = pxEventSource; }

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::container::ElementExistException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void insertCheck(const OUString& aName, const css::uno::Any& aElement);

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void insertNoCheck(const OUString& aName, const css::uno::Any& aElement);

    // Methods XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // Methods XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference<css::container::XContainerListener >& xListener ) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference<css::container::XContainerListener >& xListener ) override;

    // Methods XChangesNotifier
    virtual void SAL_CALL addChangesListener( const css::uno::Reference<css::util::XChangesListener >& xListener ) override;
    virtual void SAL_CALL removeChangesListener( const css::uno::Reference<css::util::XChangesListener >& xListener ) override;
};


class ModifiableHelper
{
private:
    ::comphelper::OInterfaceContainerHelper3<css::util::XModifyListener> m_aModifyListeners;
    ::cppu::OWeakObject&                m_rEventSource;
    bool                                mbModified;

public:
    ModifiableHelper( ::cppu::OWeakObject& _rEventSource, ::osl::Mutex& _rMutex )
        :m_aModifyListeners( _rMutex )
        ,m_rEventSource( _rEventSource )
        ,mbModified( false )
    {
    }

    bool    isModified() const  { return mbModified; }
    void    setModified( bool _bModified );

    void    addModifyListener( const css::uno::Reference< css::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.addInterface( _rxListener );
    }

    void    removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.removeInterface( _rxListener );
    }
};


typedef ::comphelper::OInterfaceContainerHelper3<
    css::script::vba::XVBAScriptListener > VBAScriptListenerContainer;

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
    css::lang::XServiceInfo,
    css::beans::XPropertySet> SfxLibraryContainer_BASE;

class SfxLibraryContainer
    : public ::cppu::BaseMutex
    , public SfxLibraryContainer_BASE
    , public ::utl::OEventListenerAdapter
{
    VBAScriptListenerContainer maVBAScriptListeners;
    sal_Int32 mnRunningVBAScripts;
    bool mbVBACompat;
    OUString msProjectName;
    rtl_TextEncoding meVBATextEncoding;
protected:
    css::uno::Reference< css::uno::XComponentContext >       mxContext;
    css::uno::Reference< css::ucb::XSimpleFileAccess3 >      mxSFI;
    css::uno::Reference< css::util::XStringSubstitution >    mxStringSubstitution;
    css::uno::WeakReference< css::frame::XModel >            mxOwnerDocument;

    ModifiableHelper    maModifiable;

    rtl::Reference<NameContainer> maNameContainer;
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
                            std::u16string_view rName,
                            const css::uno::Reference< css::embed::XStorage >& rStorage );

    // New variant for library export
    void implStoreLibrary( SfxLibrary* pLib,
                            std::u16string_view rName,
                            const css::uno::Reference< css::embed::XStorage >& rStorage,
                            std::u16string_view rTargetURL,
                            const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& rToUseSFI,
                            const css::uno::Reference< css::task::XInteractionHandler >& rHandler );

    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const css::uno::Reference< css::embed::XStorage >& xStorage );

    // New variant for library export
    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    std::u16string_view aTargetURL,
                                    const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& rToUseSFI );

    bool implLoadLibraryIndexFile( SfxLibrary* pLib,
                                    ::xmlscript::LibDescriptor& rLib,
                                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    const OUString& aIndexFileName );

    void implImportLibDescriptor( SfxLibrary* pLib, ::xmlscript::LibDescriptor const & rLib );

    // Methods to distinguish between different library types
    virtual rtl::Reference<SfxLibrary> implCreateLibrary( const OUString& aName ) = 0;
    virtual rtl::Reference<SfxLibrary> implCreateLibraryLink
        ( const OUString& aName, const OUString& aLibInfoFileURL,
          const OUString& StorageURL, bool ReadOnly ) = 0;
    virtual css::uno::Any createEmptyLibraryElement() = 0;
    virtual bool isLibraryElementValid(const css::uno::Any& rElement) const = 0;
    /// @throws css::uno::Exception
    virtual void writeLibraryElement
    (
        const css::uno::Reference< css::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const css::uno::Reference< css::io::XOutputStream >& xOutput
    ) = 0;

    virtual css::uno::Any importLibraryElement
    (
        const css::uno::Reference< css::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const OUString& aFile,
        const css::uno::Reference< css::io::XInputStream >& xElementStream ) = 0;
    virtual void importFromOldStorage( const OUString& aFile ) = 0;

    // Password encryption
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    // New variant for library export
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const css::uno::Reference< css::embed::XStorage >& rStorage,
                        const OUString& aTargetURL,
                        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& rToUseSFI, const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual bool implLoadPasswordLibrary( SfxLibrary* pLib, const OUString& Name,
                                          bool bVerifyPasswordOnly=false );

    virtual void onNewRootStorage() = 0;


    // #56666, Creates another library container
    //         instance of the same derived class
    virtual rtl::Reference<SfxLibraryContainer> createInstanceImpl() = 0;


    // Interface to get the BasicManager (Hack for password implementation)
    BasicManager* getBasicManager();
    OUString createAppLibraryFolder( SfxLibrary* pLib, std::u16string_view aName );

    void init( const OUString& rInitialDocumentURL,
               const css::uno::Reference< css::embed::XStorage >& _rxInitialStorage );

    virtual OUString getInfoFileName() const = 0;
    virtual OUString getOldInfoFileName() const = 0;
    virtual OUString getLibElementFileExtension() const = 0;
    virtual OUString getLibrariesDir() const = 0;

    // Handle maLibInfoFileURL and maStorageURL correctly
    void checkStorageURL
    (
        const OUString& aSourceURL,
        OUString& aLibInfoFileURL,
        OUString& aStorageURL,
        OUString& aUnexpandedStorageURL
    );
    /// @throws css::uno::RuntimeException
    OUString expand_url( const OUString& url );

    SfxLibrary* getImplLib( const OUString& rLibraryName );

    void storeLibraries_Impl(
                            const css::uno::Reference< css::embed::XStorage >& xStorage,
                            bool bComplete );

    void initializeFromDocument( const css::uno::Reference< css::document::XStorageBasedDocument >& _rxDocument );

    // OEventListenerAdapter
    virtual void _disposing( const css::lang::EventObject& _rSource ) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

private:
    void init_Impl( const OUString& rInitialDocumentURL,
                    const css::uno::Reference< css::embed::XStorage >& _rxInitialStorage );
    void implScanExtensions();
    static constexpr OUString sVBATextEncodingPropName = u"VBATextEncoding"_ustr;

public:
    SfxLibraryContainer();
    virtual ~SfxLibraryContainer() override;


    // Interface to set the BasicManager (Hack for password implementation)
    void setBasicManager( BasicManager* pBasMgr )
    {
        mpBasMgr = pBasMgr;
    }

    void    enterMethod();
    static void leaveMethod();

    // Methods XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // Methods XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // Members XStorageBasedLibraryContainer
    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getRootStorage() override;
    virtual void SAL_CALL setRootStorage( const css::uno::Reference< css::embed::XStorage >& _rootstorage ) override;
    virtual void SAL_CALL storeLibrariesToStorage( const css::uno::Reference< css::embed::XStorage >& RootStorage ) override;

    // Methods XModifiable (base of XPersistentLibraryContainer)
    virtual sal_Bool SAL_CALL isModified(  ) override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) override;
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // Methods XPersistentLibraryContainer (base of XStorageBasedLibraryContainer)
    virtual css::uno::Any SAL_CALL getRootLocation() override;
    virtual OUString SAL_CALL getContainerLocationName() override;
    virtual void SAL_CALL storeLibraries(  ) override;

    //Methods XLibraryContainer3
    virtual OUString SAL_CALL getOriginalLibraryLinkURL( const OUString& Name ) override;

    // Methods XLibraryContainer2 (base of XPersistentLibraryContainer)
    virtual sal_Bool SAL_CALL isLibraryLink( const OUString& Name ) override;
    virtual OUString SAL_CALL getLibraryLinkURL( const OUString& Name ) override;
    virtual sal_Bool SAL_CALL isLibraryReadOnly( const OUString& Name ) override;
    virtual void SAL_CALL setLibraryReadOnly( const OUString& Name, sal_Bool bReadOnly ) override;
    virtual void SAL_CALL renameLibrary( const OUString& Name, const OUString& NewName ) override;

    // Methods XLibraryContainer (base of XLibraryContainer2)
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL
        createLibrary( const OUString& Name ) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL createLibraryLink
        ( const OUString& Name, const OUString& StorageURL, sal_Bool ReadOnly ) override;
    virtual void SAL_CALL removeLibrary( const OUString& Name ) override;
    virtual sal_Bool SAL_CALL isLibraryLoaded( const OUString& Name ) override;
    virtual void SAL_CALL loadLibrary( const OUString& Name ) override;

    // Methods XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence<
        css::uno::Any >& aArguments ) override;

    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const OUString& Name ) override;
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const OUString& Name ) override;
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const OUString& Name, const OUString& Password ) override;
    virtual void SAL_CALL changeLibraryPassword( const OUString& Name,
        const OUString& OldPassword, const OUString& NewPassword ) override;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference<
        css::container::XContainerListener >& xListener ) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference<
        css::container::XContainerListener >& xListener ) override;

    // Methods XLibraryContainerExport
    virtual void SAL_CALL exportLibrary( const OUString& Name, const OUString& URL,
        const css::uno::Reference< css::task::XInteractionHandler >& Handler ) override;

    // Methods XServiceInfo
    virtual OUString SAL_CALL getImplementationName( ) override = 0;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( ) override = 0;
    // Methods XVBACompatibility
    virtual sal_Bool SAL_CALL getVBACompatibilityMode() override;
    virtual void SAL_CALL setVBACompatibilityMode( sal_Bool _vbacompatmodeon ) override;
    virtual OUString SAL_CALL getProjectName() override { return msProjectName; }
    virtual void SAL_CALL setProjectName( const OUString& _projectname ) override;
    virtual sal_Int32 SAL_CALL getRunningVBAScripts() override;
    virtual void SAL_CALL addVBAScriptListener(
        const css::uno::Reference< css::script::vba::XVBAScriptListener >& Listener ) override;
    virtual void SAL_CALL removeVBAScriptListener(
        const css::uno::Reference< css::script::vba::XVBAScriptListener >& Listener ) override;
    virtual void SAL_CALL broadcastVBAScriptEvent( sal_Int32 nIdentifier, const OUString& rModuleName ) override;

    // css::beans::XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName,
                                           const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString& aPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(
        const OUString& aPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString& PropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString& PropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;

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
    , public ::comphelper::WeakComponentImplHelper<>
{
    friend class SfxLibraryContainer;
    friend class SfxDialogLibraryContainer;
    friend class SfxScriptLibraryContainer;

    css::uno::Reference< css::ucb::XSimpleFileAccess3 >   mxSFI;

    ModifiableHelper&                                     mrModifiable;
    rtl::Reference<NameContainer>                         maNameContainer;

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
    bool    implIsModified() const  { return mbIsModified; }
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
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI
    );
    SfxLibrary(
        ModifiableHelper& _rModifiable,
        const css::uno::Type& aType,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI,
        OUString aLibInfoFileURL,
        OUString aStorageURL,
        bool ReadOnly
    );

    // Methods XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;
    virtual void SAL_CALL acquire() noexcept override { WeakComponentImplHelper::acquire(); }
    virtual void SAL_CALL release() noexcept override { WeakComponentImplHelper::release(); }

    // Methods XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // Methods XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // XTypeProvider
    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference<
        css::container::XContainerListener >& xListener ) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference<
        css::container::XContainerListener >& xListener ) override;

    // Methods XChangesNotifier
    virtual void SAL_CALL addChangesListener( const css::uno::Reference<
        css::util::XChangesListener >& xListener ) override;
    virtual void SAL_CALL removeChangesListener( const css::uno::Reference<
        css::util::XChangesListener >& xListener ) override;

public:
    struct LibraryContainerAccess { friend class SfxLibraryContainer; private: LibraryContainerAccess() { } };
    void    removeElementWithoutChecks( const OUString& _rElementName, LibraryContainerAccess )
    {
        impl_removeWithoutChecks( _rElementName );
    }

protected:
    virtual bool isLoadedStorable();

    virtual bool isLibraryElementValid(const css::uno::Any& rElement) const = 0;
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
    ScriptSubPackageIterator( css::uno::Reference< css::deployment::XPackage > const & xMainPackage );

    css::uno::Reference< css::deployment::XPackage > getNextScriptSubPackage( bool& rbPureDialogLib );
};


class ScriptExtensionIterator final
{
public:
    ScriptExtensionIterator();
    OUString nextBasicOrDialogLibrary( bool& rbPureDialogLib );

private:
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
