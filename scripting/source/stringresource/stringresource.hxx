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

#ifndef INCLUDED_SCRIPTING_SOURCE_STRINGRESOURCE_STRINGRESOURCE_HXX
#define INCLUDED_SCRIPTING_SOURCE_STRINGRESOURCE_STRINGRESOURCE_HXX

#include <com/sun/star/resource/XStringResourceWithStorage.hpp>
#include <com/sun/star/resource/XStringResourceWithLocation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>

#include <unordered_map>
#include <vector>


namespace stringresource
{



// mutex


::osl::Mutex& getMutex();



// class stringresourceImpl


// Hashtable to map string ids to string
typedef std::unordered_map
<
    OUString,
    OUString,
    OUStringHash
>
IdToStringMap;

typedef std::unordered_map
<
    OUString,
    sal_Int32,
    OUStringHash
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
        : m_locale( locale )
        , m_nNextIndex( 0 )
        , m_bLoaded( bLoaded )
        , m_bModified( false )
    {}
};

typedef std::vector< LocaleItem* > LocaleItemVector;
typedef std::vector< LocaleItem* >::iterator LocaleItemVectorIt;
typedef std::vector< LocaleItem* >::const_iterator LocaleItemVectorConstIt;

typedef ::cppu::WeakImplHelper<
    css::lang::XServiceInfo,
    css::resource::XStringResourceManager > StringResourceImpl_BASE;

class StringResourceImpl : public StringResourceImpl_BASE
{
protected:
    css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory >  m_xMCF;

    LocaleItem*                                               m_pCurrentLocaleItem;
    LocaleItem*                                               m_pDefaultLocaleItem;
    bool                                                      m_bDefaultModified;

    ::cppu::OInterfaceContainerHelper                         m_aListenerContainer;

    LocaleItemVector                                          m_aLocaleItemVector;
    LocaleItemVector                                          m_aDeletedLocaleItemVector;
    LocaleItemVector                                          m_aChangedDefaultLocaleVector;

    bool                                                      m_bModified;
    bool                                                      m_bReadOnly;

    sal_Int32                                                 m_nNextUniqueNumericId;

    // Scans ResourceID to start with number and adapt m_nNextUniqueNumericId
    void implScanIdForNumber( const OUString& ResourceID );
    const static sal_Int32 UNIQUE_NUMBER_NEEDS_INITIALISATION = -1;

    // Checks read only status and throws exception if it's true
    void implCheckReadOnly( const sal_Char* pExceptionMsg )
        throw (css::lang::NoSupportException);

    // Return the context's MultiComponentFactory
    css::uno::Reference< css::lang::XMultiComponentFactory >
        getMultiComponentFactory();

    // Returns the LocalItem for a given locale, if it exists, otherwise NULL
    // This method compares the locales exactly, no closest match search is performed
    LocaleItem* getItemForLocale( const css::lang::Locale& locale, bool bException )
        throw (css::lang::IllegalArgumentException);

    // Returns the LocalItem for a given locale, if it exists, otherwise NULL
    // This method performs a closest match search, at least the language must match
    LocaleItem* getClosestMatchItemForLocale( const css::lang::Locale& locale );
    void implSetCurrentLocale( const css::lang::Locale& locale,
        bool FindClosestMatch, bool bUseDefaultIfNoMatch )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    void implModified();
    void implNotifyListeners();

    //=== Impl methods for ...ForLocale methods ===
    OUString SAL_CALL implResolveString( const OUString& ResourceID, LocaleItem* pLocaleItem )
        throw (css::resource::MissingResourceException);
    bool implHasEntryForId( const OUString& ResourceID, LocaleItem* pLocaleItem );
    css::uno::Sequence< OUString > implGetResourceIDs( LocaleItem* pLocaleItem );
    void implSetString( const OUString& ResourceID,
        const OUString& Str, LocaleItem* pLocaleItem );
    void implRemoveId( const OUString& ResourceID, LocaleItem* pLocaleItem )
        throw (css::resource::MissingResourceException);

    // Method to load a locale if necessary, returns true if loading was
    // successful. Default implementation in base class just returns true.
    virtual bool loadLocale( LocaleItem* pLocaleItem );

    virtual void implLoadAllLocales();

public:
    StringResourceImpl(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~StringResourceImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

     // XStringResourceResolver
    virtual OUString SAL_CALL resolveString( const OUString& ResourceID )
        throw (css::resource::MissingResourceException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL resolveStringForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw ( css::resource::MissingResourceException,
                    css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEntryForId( const OUString& ResourceID )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEntryForIdAndLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getResourceIDs(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getResourceIDsForLocale
        ( const css::lang::Locale& locale )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getCurrentLocale(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getDefaultLocale(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getLocales(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XStringResourceManager
    virtual sal_Bool SAL_CALL isReadOnly()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentLocale( const css::lang::Locale& locale, sal_Bool FindClosestMatch )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultLocale( const css::lang::Locale& locale )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL setString( const OUString& ResourceID, const OUString& Str )
        throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStringForLocale( const OUString& ResourceID, const OUString& Str,
        const css::lang::Locale& locale )
            throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeId( const OUString& ResourceID )
        throw (css::resource::MissingResourceException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL removeIdForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw (css::resource::MissingResourceException, css::uno::RuntimeException,
                   css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL newLocale( const css::lang::Locale& locale )
        throw (css::container::ElementExistException, css::lang::IllegalArgumentException,
               css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeLocale( const css::lang::Locale& locale )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getUniqueNumericId(  )
        throw (css::lang::NoSupportException,
               css::uno::RuntimeException, std::exception) override;
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

    void SAL_CALL implInitializeCommonParameters( const css::uno::Sequence< css::uno::Any >& aArguments )
            throw (css::uno::Exception, css::uno::RuntimeException);

    // Scan locale properties files
    virtual void implScanLocales();

    // Method to load a locale if necessary, returns true if loading was successful
    virtual bool loadLocale( LocaleItem* pLocaleItem ) override;

    // does the actual loading
    virtual bool implLoadLocale( LocaleItem* pLocaleItem );

    virtual void implLoadAllLocales() override;

    void implScanLocaleNames( const css::uno::Sequence< OUString >& aContentSeq );
    static OUString implGetFileNameForLocaleItem( LocaleItem* pLocaleItem, const OUString& aNameBase );
    static OUString implGetPathForLocaleItem( LocaleItem* pLocaleItem, const OUString& aNameBase,
        const OUString& aLocation, bool bDefaultFile=false );

    bool implReadPropertiesFile( LocaleItem* pLocaleItem,
        const css::uno::Reference< css::io::XInputStream >& xInput );

    bool implWritePropertiesFile( LocaleItem* pLocaleItem,
              const css::uno::Reference< css::io::XOutputStream >& xOutputStream,
              const OUString& aComment );

    void implWriteLocaleBinary( LocaleItem* pLocaleItem, BinaryOutput& rOut );

    void implStoreAtStorage
    (
        const OUString& aNameBase,
        const OUString& aComment,
        const css::uno::Reference< css::embed::XStorage >& Storage,
        bool bUsedForStore,
        bool bStoreAll
    )
    throw (css::uno::Exception, css::uno::RuntimeException, std::exception);

    void implKillRemovedLocaleFiles
    (
        const OUString& Location,
        const OUString& aNameBase,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xFileAccess
    )
    throw (css::uno::Exception, css::uno::RuntimeException, std::exception);

    void implKillChangedDefaultFiles
    (
        const OUString& Location,
        const OUString& aNameBase,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xFileAccess
    )
    throw (css::uno::Exception, css::uno::RuntimeException, std::exception);

    void implStoreAtLocation
    (
        const OUString& Location,
        const OUString& aNameBase,
        const OUString& aComment,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xFileAccess,
        bool bUsedForStore,
        bool bStoreAll,
        bool bKillAll = false
    )
    throw (css::uno::Exception, css::uno::RuntimeException, std::exception);

public:
    StringResourcePersistenceImpl(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~StringResourcePersistenceImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

     // XStringResourceResolver
    virtual OUString SAL_CALL resolveString( const OUString& ResourceID )
        throw (css::resource::MissingResourceException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL resolveStringForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw ( css::resource::MissingResourceException,
                    css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEntryForId( const OUString& ResourceID )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEntryForIdAndLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getResourceIDs(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getResourceIDsForLocale
        ( const css::lang::Locale& locale )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getCurrentLocale(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getDefaultLocale(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getLocales(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XStringResourceManager
    virtual sal_Bool SAL_CALL isReadOnly()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentLocale( const css::lang::Locale& locale, sal_Bool FindClosestMatch )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultLocale( const css::lang::Locale& locale )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL setString( const OUString& ResourceID, const OUString& Str )
        throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStringForLocale( const OUString& ResourceID, const OUString& Str,
        const css::lang::Locale& locale )
            throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeId( const OUString& ResourceID )
        throw (css::resource::MissingResourceException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL removeIdForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw (css::resource::MissingResourceException, css::uno::RuntimeException,
                   css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL newLocale( const css::lang::Locale& locale )
        throw (css::container::ElementExistException, css::lang::IllegalArgumentException,
               css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeLocale( const css::lang::Locale& locale )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getUniqueNumericId(  )
        throw (css::lang::NoSupportException,
               css::uno::RuntimeException, std::exception) override;

    // XStringResourcePersistence
    virtual void SAL_CALL store(  )
        throw (css::lang::NoSupportException,
               css::uno::Exception,
               css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isModified(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setComment( const OUString& Comment )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeToStorage
        ( const css::uno::Reference< css::embed::XStorage >& Storage,
          const OUString& NameBase, const OUString& Comment )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeToURL( const OUString& URL, const OUString& NameBase,
        const OUString& Comment, const css::uno::Reference
        < css::task::XInteractionHandler >& Handler )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL exportBinary(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL importBinary( const css::uno::Sequence< ::sal_Int8 >& Data )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
};


typedef ::cppu::ImplInheritanceHelper<
        StringResourcePersistenceImpl,
        css::lang::XInitialization,
        css::resource::XStringResourceWithStorage > StringResourceWithStorageImpl_BASE;

class StringResourceWithStorageImpl : public StringResourceWithStorageImpl_BASE
{
    css::uno::Reference< css::embed::XStorage >       m_xStorage;
    bool                                              m_bStorageChanged;

    virtual void implScanLocales() override;
    virtual bool implLoadLocale( LocaleItem* pLocaleItem ) override;

public:
    StringResourceWithStorageImpl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~StringResourceWithStorageImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

     // XStringResourceResolver
    virtual OUString SAL_CALL resolveString( const OUString& ResourceID )
        throw (css::resource::MissingResourceException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL resolveStringForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw ( css::resource::MissingResourceException,
                    css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEntryForId( const OUString& ResourceID )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEntryForIdAndLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getResourceIDs(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getResourceIDsForLocale
        ( const css::lang::Locale& locale )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getCurrentLocale(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getDefaultLocale(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getLocales(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XStringResourceManager
    virtual sal_Bool SAL_CALL isReadOnly()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentLocale( const css::lang::Locale& locale, sal_Bool FindClosestMatch )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultLocale( const css::lang::Locale& locale )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL setString( const OUString& ResourceID, const OUString& Str )
        throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStringForLocale( const OUString& ResourceID, const OUString& Str,
        const css::lang::Locale& locale )
            throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeId( const OUString& ResourceID )
        throw (css::resource::MissingResourceException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL removeIdForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw (css::resource::MissingResourceException, css::uno::RuntimeException,
                   css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL newLocale( const css::lang::Locale& locale )
        throw (css::container::ElementExistException, css::lang::IllegalArgumentException,
               css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeLocale( const css::lang::Locale& locale )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getUniqueNumericId(  )
        throw (css::lang::NoSupportException,
               css::uno::RuntimeException, std::exception) override;

    // XStringResourcePersistence
    virtual void SAL_CALL store(  )
        throw (css::lang::NoSupportException,
               css::uno::Exception,
               css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isModified(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setComment( const OUString& Comment )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeToStorage
        ( const css::uno::Reference< css::embed::XStorage >& Storage,
          const OUString& NameBase, const OUString& Comment )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeToURL( const OUString& URL, const OUString& NameBase,
        const OUString& Comment, const css::uno::Reference
        < css::task::XInteractionHandler >& Handler )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL exportBinary(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL importBinary( const css::uno::Sequence< ::sal_Int8 >& Data )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XStringResourceWithStorage
    virtual void SAL_CALL storeAsStorage
        ( const css::uno::Reference< css::embed::XStorage >& Storage )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStorage
        ( const css::uno::Reference< css::embed::XStorage >& Storage )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
};


typedef ::cppu::ImplInheritanceHelper<
        StringResourcePersistenceImpl,
        css::lang::XInitialization,
        css::resource::XStringResourceWithLocation > StringResourceWithLocationImpl_BASE;

class StringResourceWithLocationImpl : public StringResourceWithLocationImpl_BASE
{
    OUString                                              m_aLocation;
    bool                                                  m_bLocationChanged;
    css::uno::Reference< css::ucb::XSimpleFileAccess3 >   m_xSFI;
    css::uno::Reference< css::task::XInteractionHandler > m_xInteractionHandler;

    const css::uno::Reference< css::ucb::XSimpleFileAccess3 > getFileAccess();

    virtual void implScanLocales() override;
    virtual bool implLoadLocale( LocaleItem* pLocaleItem ) override;

public:
    StringResourceWithLocationImpl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~StringResourceWithLocationImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

     // XStringResourceResolver
    virtual OUString SAL_CALL resolveString( const OUString& ResourceID )
        throw (css::resource::MissingResourceException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL resolveStringForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw ( css::resource::MissingResourceException,
                    css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEntryForId( const OUString& ResourceID )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEntryForIdAndLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getResourceIDs(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getResourceIDsForLocale
        ( const css::lang::Locale& locale )
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getCurrentLocale(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getDefaultLocale(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getLocales(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XStringResourceManager
    virtual sal_Bool SAL_CALL isReadOnly()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentLocale( const css::lang::Locale& locale, sal_Bool FindClosestMatch )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultLocale( const css::lang::Locale& locale )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL setString( const OUString& ResourceID, const OUString& Str )
        throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStringForLocale( const OUString& ResourceID, const OUString& Str,
        const css::lang::Locale& locale )
            throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeId( const OUString& ResourceID )
        throw (css::resource::MissingResourceException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL removeIdForLocale( const OUString& ResourceID,
        const css::lang::Locale& locale )
            throw (css::resource::MissingResourceException, css::uno::RuntimeException,
                   css::lang::NoSupportException, std::exception) override;
    virtual void SAL_CALL newLocale( const css::lang::Locale& locale )
        throw (css::container::ElementExistException, css::lang::IllegalArgumentException,
               css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeLocale( const css::lang::Locale& locale )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::lang::NoSupportException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getUniqueNumericId(  )
        throw (css::lang::NoSupportException,
               css::uno::RuntimeException, std::exception) override;

    // XStringResourcePersistence
    virtual void SAL_CALL store(  )
        throw (css::lang::NoSupportException,
               css::uno::Exception,
               css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isModified(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setComment( const OUString& Comment )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeToStorage
        ( const css::uno::Reference< css::embed::XStorage >& Storage,
          const OUString& NameBase, const OUString& Comment )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeToURL( const OUString& URL, const OUString& NameBase,
        const OUString& Comment, const css::uno::Reference
        < css::task::XInteractionHandler >& Handler )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL exportBinary(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL importBinary( const css::uno::Sequence< ::sal_Int8 >& Data )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XStringResourceWithLocation
    virtual void SAL_CALL storeAsURL( const OUString& URL )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setURL( const OUString& URL )
        throw (css::lang::IllegalArgumentException, css::lang::NoSupportException,
               css::uno::RuntimeException, std::exception) override;
};


}   // namespace stringtable


#endif // INCLUDED_SCRIPTING_SOURCE_STRINGRESOURCE_STRINGRESOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
