/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/resource/XStringResourceWithStorage.hpp>
#include <com/sun/star/resource/XStringResourceWithLocation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>


namespace stringresource
{


// class stringresourceImpl


// Hashtable to map string ids to string
typedef std::unordered_map
<
    OUString,
    OUString
>
IdToStringMap;

typedef std::unordered_map
<
    OUString,
    sal_Int32
>
IdToIndexMap;


struct LocaleItem
{
    css::lang::Locale      m_locale;
    IdToStringMap          m_aIdToStringMap;
    IdToIndexMap           m_aIdToIndexMap;
    sal_Int32              m_nNextIndex;
    bool                   m_bLoaded;
    bool                   m_bModified;

    LocaleItem( css::lang::Locale locale, bool bLoaded=true )
        : m_locale(std::move( locale ))
        , m_nNextIndex( 0 )
        , m_bLoaded( bLoaded )
        , m_bModified( false )
    {}
};

typedef ::cppu::WeakImplHelper<
    css::lang::XServiceInfo,
    css::resource::XStringResourceManager > StringResourceImpl_BASE;

class StringResourceImpl : public StringResourceImpl_BASE
{
protected:
    std::mutex                                                m_aMutex;
    cpo::uno::Reference< cpo::uno::XComponentContext >        m_xContext;

    LocaleItem*                                               m_pCurrentLocaleItem;
    LocaleItem*                                               m_pDefaultLocaleItem;
    bool                                                      m_bDefaultModified;

    ::comphelper::OInterfaceContainerHelper4<css::util::XModifyListener> m_aListenerContainer;

    std::vector< std::unique_ptr<LocaleItem> >                m_aLocaleItemVector;
    std::vector< std::unique_ptr<LocaleItem> >                m_aDeletedLocaleItemVector;
    std::vector< std::unique_ptr<LocaleItem> >                m_aChangedDefaultLocaleVector;

    bool                                                      m_bModified;
    bool                                                      m_bReadOnly;

    sal_Int32                                                 m_nNextUniqueNumericId;

    // Scans ResourceID to start with number and adapt m_nNextUniqueNumericId
    void implScanIdForNumber(std::unique_lock<std::mutex>& rGuard, const OUString& ResourceID);
    const static sal_Int32 UNIQUE_NUMBER_NEEDS_INITIALISATION = -1;

    // Checks read only status and throws exception if it's true
    /// @throws css::lang::NoSupportException
    void implCheckReadOnly( const char* pExceptionMsg );

    // Returns the LocalItem for a given locale, if it exists, otherwise NULL
    // This method compares the locales exactly, no closest match search is performed
    /// @throws css::lang::IllegalArgumentException
    LocaleItem* getItemForLocale( const css::lang::Locale& locale, bool bException );

    // Returns the LocalItem for a given locale, if it exists, otherwise NULL
    // This method performs a closest match search, at least the language must match
    LocaleItem* getClosestMatchItemForLocale( const css::lang::Locale& locale );
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    void implSetCurrentLocale( std::unique_lock<std::mutex>& rGuard, const css::lang::Locale& locale,
        bool FindClosestMatch, bool bUseDefaultIfNoMatch );

    void implModified(std::unique_lock<std::mutex>&);
    void implNotifyListeners(std::unique_lock<std::mutex>&);

    //=== Impl methods for ...ForLocale methods ===
    /// @throws css::resource::MissingResourceException
    OUString implResolveString(std::unique_lock<std::mutex>& rGuard, const OUString& ResourceID, LocaleItem* pLocaleItem);
    bool implHasEntryForId(std::unique_lock<std::mutex>& rGuard, const OUString& ResourceID, LocaleItem* pLocaleItem);
    cpo::uno::Sequence< OUString > implGetResourceIDs(std::unique_lock<std::mutex>& rGuard, LocaleItem* pLocaleItem);
    void implSetString( std::unique_lock<std::mutex>& rGuard, const OUString& ResourceID,
        const OUString& Str, LocaleItem* pLocaleItem );
    /// @throws css::resource::MissingResourceException
    void implRemoveId( std::unique_lock<std::mutex>& rGuard, const OUString& ResourceID, LocaleItem* pLocaleItem );

    // Method to load a locale if necessary, returns true if loading was
    // successful. Default implementation in base class just returns true.
    virtual bool loadLocale(std::unique_lock<std::mutex>& rGuard, LocaleItem* pLocaleItem);

    virtual void implLoadAllLocales(std::unique_lock<std::mutex>& rGuard);

public:
    explicit StringResourceImpl(
        const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    virtual ~StringResourceImpl() override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XModifyBroadcaster
    virtual void addModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void removeModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;

     // XStringResourceResolver
    virtual OUString resolveString( const OUString& ResourceID ) override;
    virtual OUString resolveStringForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual bool hasEntryForId( const OUString& ResourceID ) override;
    virtual bool hasEntryForIdAndLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual cpo::uno::Sequence< OUString > getResourceIDs(  ) override;
    virtual cpo::uno::Sequence< OUString > getResourceIDsForLocale
        ( const css::lang::Locale& locale ) override;
    virtual css::lang::Locale getCurrentLocale(  ) override;
    virtual css::lang::Locale getDefaultLocale(  ) override;
    virtual cpo::uno::Sequence< css::lang::Locale > getLocales(  ) override;

    // XStringResourceManager
    virtual bool isReadOnly() override;
    virtual void setCurrentLocale( const css::lang::Locale& locale, bool FindClosestMatch ) override;
    virtual void setDefaultLocale( const css::lang::Locale& locale ) override;
    virtual void setString( const OUString& ResourceID, const OUString& Str ) override;
    virtual void setStringForLocale( const OUString& ResourceID, const OUString& Str,
        const css::lang::Locale& locale ) override;
    virtual void removeId( const OUString& ResourceID ) override;
    virtual void removeIdForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual void newLocale( const css::lang::Locale& locale ) override;
    virtual void removeLocale( const css::lang::Locale& locale ) override;
    virtual ::sal_Int32 getUniqueNumericId(  ) override;
 };

typedef ::cppu::ImplInheritanceHelper<
        StringResourceImpl,
        css::resource::XStringResourcePersistence > StringResourcePersistenceImpl_BASE;

class BinaryOutput;

class StringResourcePersistenceImpl : public StringResourcePersistenceImpl_BASE
{
protected:
    OUString                                                             m_aNameBase;
    OUString                                                             m_aComment;

    /// @throws cpo::uno::Exception
    /// @throws cpo::uno::RuntimeException
    void implInitializeCommonParameters( std::unique_lock<std::mutex>& rGuard, const cpo::uno::Sequence< cpo::uno::Any >& aArguments );

    // Scan locale properties files
    virtual void implScanLocales(std::unique_lock<std::mutex>& rGuard);

    // Method to load a locale if necessary, returns true if loading was successful
    virtual bool loadLocale(std::unique_lock<std::mutex>& rGuard, LocaleItem* pLocaleItem) override;

    // does the actual loading
    virtual bool implLoadLocale(std::unique_lock<std::mutex>& rGuard, LocaleItem* pLocaleItem);

    virtual void implLoadAllLocales(std::unique_lock<std::mutex>& rGuard) override;

    void implScanLocaleNames( const cpo::uno::Sequence< OUString >& aContentSeq );
    static OUString implGetFileNameForLocaleItem( LocaleItem const * pLocaleItem, const OUString& aNameBase );
    static OUString implGetPathForLocaleItem( LocaleItem const * pLocaleItem, const OUString& aNameBase,
        std::u16string_view aLocation, bool bDefaultFile=false );

    bool implReadPropertiesFile(std::unique_lock<std::mutex>& rGuard, LocaleItem* pLocaleItem,
        const cpo::uno::Reference< css::io::XInputStream >& xInput);

    bool implWritePropertiesFile( LocaleItem const * pLocaleItem,
              const cpo::uno::Reference< css::io::XOutputStream >& xOutputStream,
              const OUString& aComment );

    void implWriteLocaleBinary( LocaleItem* pLocaleItem, BinaryOutput& rOut );

    /// @throws cpo::uno::Exception
    /// @throws cpo::uno::RuntimeException
    void implStoreAtStorage
    (
        std::unique_lock<std::mutex>& rGuard,
        const OUString& aNameBase,
        const OUString& aComment,
        const cpo::uno::Reference< css::embed::XStorage >& Storage,
        bool bUsedForStore,
        bool bStoreAll
    );

    /// @throws cpo::uno::Exception
    /// @throws cpo::uno::RuntimeException
    void implKillRemovedLocaleFiles
    (
        std::u16string_view Location,
        const OUString& aNameBase,
        const cpo::uno::Reference< css::ucb::XSimpleFileAccess >& xFileAccess
    );

    /// @throws cpo::uno::Exception
    /// @throws cpo::uno::RuntimeException
    void implKillChangedDefaultFiles
    (
        std::u16string_view Location,
        const OUString& aNameBase,
        const cpo::uno::Reference< css::ucb::XSimpleFileAccess >& xFileAccess
    );

    /// @throws cpo::uno::Exception
    /// @throws cpo::uno::RuntimeException
    void implStoreAtLocation
    (
        std::unique_lock<std::mutex>& rGuard,
        std::u16string_view Location,
        const OUString& aNameBase,
        const OUString& aComment,
        const cpo::uno::Reference< css::ucb::XSimpleFileAccess >& xFileAccess,
        bool bUsedForStore,
        bool bStoreAll,
        bool bKillAll = false
    );

public:
    explicit StringResourcePersistenceImpl(
        const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    virtual ~StringResourcePersistenceImpl() override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XModifyBroadcaster
    virtual void addModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void removeModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;

     // XStringResourceResolver
    virtual OUString resolveString( const OUString& ResourceID ) override;
    virtual OUString resolveStringForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual bool hasEntryForId( const OUString& ResourceID ) override;
    virtual bool hasEntryForIdAndLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual cpo::uno::Sequence< OUString > getResourceIDs(  ) override;
    virtual cpo::uno::Sequence< OUString > getResourceIDsForLocale
        ( const css::lang::Locale& locale ) override;
    virtual css::lang::Locale getCurrentLocale(  ) override;
    virtual css::lang::Locale getDefaultLocale(  ) override;
    virtual cpo::uno::Sequence< css::lang::Locale > getLocales(  ) override;

    // XStringResourceManager
    virtual bool isReadOnly() override;
    virtual void setCurrentLocale( const css::lang::Locale& locale, bool FindClosestMatch ) override;
    virtual void setDefaultLocale( const css::lang::Locale& locale ) override;
    virtual void setString( const OUString& ResourceID, const OUString& Str ) override;
    virtual void setStringForLocale( const OUString& ResourceID, const OUString& Str,
        const css::lang::Locale& locale ) override;
    virtual void removeId( const OUString& ResourceID ) override;
    virtual void removeIdForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual void newLocale( const css::lang::Locale& locale ) override;
    virtual void removeLocale( const css::lang::Locale& locale ) override;
    virtual ::sal_Int32 getUniqueNumericId(  ) override;

    // XStringResourcePersistence
    virtual void store(  ) override;
    virtual bool isModified(  ) override;
    virtual void setComment( const OUString& Comment ) override;
    virtual void storeToStorage
        ( const cpo::uno::Reference< css::embed::XStorage >& Storage,
          const OUString& NameBase, const OUString& Comment ) override;
    virtual void storeToURL( const OUString& URL, const OUString& NameBase,
        const OUString& Comment, const cpo::uno::Reference
        < css::task::XInteractionHandler >& Handler ) override;
    virtual cpo::uno::Sequence< ::sal_Int8 > exportBinary(  ) override;
    virtual void importBinary( const cpo::uno::Sequence< ::sal_Int8 >& Data ) override;
};


typedef ::cppu::ImplInheritanceHelper<
        StringResourcePersistenceImpl,
        css::lang::XInitialization,
        css::resource::XStringResourceWithStorage > StringResourceWithStorageImpl_BASE;

class StringResourceWithStorageImpl : public StringResourceWithStorageImpl_BASE
{
    cpo::uno::Reference< css::embed::XStorage >       m_xStorage;
    bool                                              m_bStorageChanged;

    virtual void implScanLocales(std::unique_lock<std::mutex>& rGuard) override;
    virtual bool implLoadLocale(std::unique_lock<std::mutex>& rGuard, LocaleItem* pLocaleItem) override;

public:
    explicit StringResourceWithStorageImpl( const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    virtual ~StringResourceWithStorageImpl() override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XInitialization
    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

    // XModifyBroadcaster
    virtual void addModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void removeModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;

     // XStringResourceResolver
    virtual OUString resolveString( const OUString& ResourceID ) override;
    virtual OUString resolveStringForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual bool hasEntryForId( const OUString& ResourceID ) override;
    virtual bool hasEntryForIdAndLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual cpo::uno::Sequence< OUString > getResourceIDs(  ) override;
    virtual cpo::uno::Sequence< OUString > getResourceIDsForLocale
        ( const css::lang::Locale& locale ) override;
    virtual css::lang::Locale getCurrentLocale(  ) override;
    virtual css::lang::Locale getDefaultLocale(  ) override;
    virtual cpo::uno::Sequence< css::lang::Locale > getLocales(  ) override;

    // XStringResourceManager
    virtual bool isReadOnly() override;
    virtual void setCurrentLocale( const css::lang::Locale& locale, bool FindClosestMatch ) override;
    virtual void setDefaultLocale( const css::lang::Locale& locale ) override;
    virtual void setString( const OUString& ResourceID, const OUString& Str ) override;
    virtual void setStringForLocale( const OUString& ResourceID, const OUString& Str,
        const css::lang::Locale& locale ) override;
    virtual void removeId( const OUString& ResourceID ) override;
    virtual void removeIdForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual void newLocale( const css::lang::Locale& locale ) override;
    virtual void removeLocale( const css::lang::Locale& locale ) override;
    virtual ::sal_Int32 getUniqueNumericId(  ) override;

    // XStringResourcePersistence
    virtual void store(  ) override;
    virtual bool isModified(  ) override;
    virtual void setComment( const OUString& Comment ) override;
    virtual void storeToStorage
        ( const cpo::uno::Reference< css::embed::XStorage >& Storage,
          const OUString& NameBase, const OUString& Comment ) override;
    virtual void storeToURL( const OUString& URL, const OUString& NameBase,
        const OUString& Comment, const cpo::uno::Reference
        < css::task::XInteractionHandler >& Handler ) override;
    virtual cpo::uno::Sequence< ::sal_Int8 > exportBinary(  ) override;
    virtual void importBinary( const cpo::uno::Sequence< ::sal_Int8 >& Data ) override;

    // XStringResourceWithStorage
    virtual void storeAsStorage
        ( const cpo::uno::Reference< css::embed::XStorage >& Storage ) override;
    virtual void setStorage
        ( const cpo::uno::Reference< css::embed::XStorage >& Storage ) override;
};


typedef ::cppu::ImplInheritanceHelper<
        StringResourcePersistenceImpl,
        css::lang::XInitialization,
        css::resource::XStringResourceWithLocation > StringResourceWithLocationImpl_BASE;

class StringResourceWithLocationImpl : public StringResourceWithLocationImpl_BASE
{
    OUString                                              m_aLocation;
    bool                                                  m_bLocationChanged;
    cpo::uno::Reference< css::ucb::XSimpleFileAccess >    m_xSFI;
    cpo::uno::Reference< css::task::XInteractionHandler > m_xInteractionHandler;

    const cpo::uno::Reference< css::ucb::XSimpleFileAccess > & getFileAccessImpl();

    virtual void implScanLocales(std::unique_lock<std::mutex>& rGuard) override;
    virtual bool implLoadLocale(std::unique_lock<std::mutex>& rGuard, LocaleItem* pLocaleItem) override;

public:
    explicit StringResourceWithLocationImpl( const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    virtual ~StringResourceWithLocationImpl() override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XInitialization
    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

    // XModifyBroadcaster
    virtual void addModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void removeModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;

     // XStringResourceResolver
    virtual OUString resolveString( const OUString& ResourceID ) override;
    virtual OUString resolveStringForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual bool hasEntryForId( const OUString& ResourceID ) override;
    virtual bool hasEntryForIdAndLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual cpo::uno::Sequence< OUString > getResourceIDs(  ) override;
    virtual cpo::uno::Sequence< OUString > getResourceIDsForLocale
        ( const css::lang::Locale& locale ) override;
    virtual css::lang::Locale getCurrentLocale(  ) override;
    virtual css::lang::Locale getDefaultLocale(  ) override;
    virtual cpo::uno::Sequence< css::lang::Locale > getLocales(  ) override;

    // XStringResourceManager
    virtual bool isReadOnly() override;
    virtual void setCurrentLocale( const css::lang::Locale& locale, bool FindClosestMatch ) override;
    virtual void setDefaultLocale( const css::lang::Locale& locale ) override;
    virtual void setString( const OUString& ResourceID, const OUString& Str ) override;
    virtual void setStringForLocale( const OUString& ResourceID, const OUString& Str,
        const css::lang::Locale& locale ) override;
    virtual void removeId( const OUString& ResourceID ) override;
    virtual void removeIdForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale ) override;
    virtual void newLocale( const css::lang::Locale& locale ) override;
    virtual void removeLocale( const css::lang::Locale& locale ) override;
    virtual ::sal_Int32 getUniqueNumericId(  ) override;

    // XStringResourcePersistence
    virtual void store(  ) override;
    virtual bool isModified(  ) override;
    virtual void setComment( const OUString& Comment ) override;
    virtual void storeToStorage
        ( const cpo::uno::Reference< css::embed::XStorage >& Storage,
          const OUString& NameBase, const OUString& Comment ) override;
    virtual void storeToURL( const OUString& URL, const OUString& NameBase,
        const OUString& Comment, const cpo::uno::Reference
        < css::task::XInteractionHandler >& Handler ) override;
    virtual cpo::uno::Sequence< ::sal_Int8 > exportBinary(  ) override;
    virtual void importBinary( const cpo::uno::Sequence< ::sal_Int8 >& Data ) override;

    // XStringResourceWithLocation
    virtual void storeAsURL( const OUString& URL ) override;
    virtual void setURL( const OUString& URL ) override;
};


}   // namespace stringtable

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
