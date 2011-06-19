/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SCRIPTING_DLGPROV_HXX
#define SCRIPTING_DLGPROV_HXX

#include <com/sun/star/resource/XStringResourceWithStorage.hpp>
#include <com/sun/star/resource/XStringResourceWithLocation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>

#include <vector>
#include <boost/unordered_map.hpp>

//.........................................................................
namespace stringresource
{
//.........................................................................

// =============================================================================
// mutex
// =============================================================================

::osl::Mutex& getMutex();


// =============================================================================
// class stringresourceImpl
// =============================================================================

// Hashtable to map string ids to string
struct hashName_Impl
{
    size_t operator()(const ::rtl::OUString Str) const
    {
        return (size_t)Str.hashCode();
    }
};

struct eqName_Impl
{
    sal_Bool operator()(const ::rtl::OUString Str1, const ::rtl::OUString Str2) const
    {
        return ( Str1 == Str2 );
    }
};

typedef boost::unordered_map
<
    ::rtl::OUString,
    ::rtl::OUString,
    hashName_Impl,
    eqName_Impl
>
IdToStringMap;

typedef boost::unordered_map
<
    ::rtl::OUString,
    sal_Int32,
    hashName_Impl,
    eqName_Impl
>
IdToIndexMap;


struct LocaleItem
{
    ::com::sun::star::lang::Locale      m_locale;
    IdToStringMap                       m_aIdToStringMap;
    IdToIndexMap                        m_aIdToIndexMap;
    sal_Int32                           m_nNextIndex;
    bool                                m_bLoaded;
    bool                                m_bModified;

    LocaleItem( ::com::sun::star::lang::Locale locale, bool bLoaded=true )
        : m_locale( locale )
        , m_nNextIndex( 0 )
        , m_bLoaded( bLoaded )
        , m_bModified( false )
    {}
};

typedef std::vector< LocaleItem* > LocaleItemVector;
typedef std::vector< LocaleItem* >::iterator LocaleItemVectorIt;
typedef std::vector< LocaleItem* >::const_iterator LocaleItemVectorConstIt;

typedef ::cppu::WeakImplHelper2<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::resource::XStringResourceManager > StringResourceImpl_BASE;

class StringResourceImpl : public StringResourceImpl_BASE
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >  m_xMCF;

    LocaleItem*                                                                         m_pCurrentLocaleItem;
    LocaleItem*                                                                         m_pDefaultLocaleItem;
    bool                                                                                m_bDefaultModified;

    ::cppu::OInterfaceContainerHelper                                                   m_aListenerContainer;

    LocaleItemVector                                                                    m_aLocaleItemVector;
    LocaleItemVector                                                                    m_aDeletedLocaleItemVector;
    LocaleItemVector                                                                    m_aChangedDefaultLocaleVector;

    bool                                                                                m_bModified;
    bool                                                                                m_bReadOnly;

    sal_Int32                                                                           m_nNextUniqueNumericId;

    // Scans ResourceID to start with number and adapt m_nNextUniqueNumericId
    void implScanIdForNumber( const ::rtl::OUString& ResourceID );
    const static sal_Int32 UNIQUE_NUMBER_NEEDS_INITIALISATION = -1;

    // Checks read only status and throws exception if it's true
    void implCheckReadOnly( const sal_Char* pExceptionMsg )
        throw (::com::sun::star::lang::NoSupportException);

    // Return the context's MultiComponentFactory
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >
        getMultiComponentFactory( void );

    // Returns the LocalItem for a given locale, if it exists, otherwise NULL
    // This method compares the locales exactly, no closest match search is performed
    LocaleItem* getItemForLocale( const ::com::sun::star::lang::Locale& locale, sal_Bool bException )
        throw (::com::sun::star::lang::IllegalArgumentException);

    // Returns the LocalItem for a given locale, if it exists, otherwise NULL
    // This method performes a closest match search, at least the language must match
    LocaleItem* getClosestMatchItemForLocale( const ::com::sun::star::lang::Locale& locale );
    void implSetCurrentLocale( const ::com::sun::star::lang::Locale& locale,
        sal_Bool FindClosestMatch, sal_Bool bUseDefaultIfNoMatch )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    void implModified( void );
    void implNotifyListeners( void );

    //=== Impl methods for ...ForLocale methods ===
    ::rtl::OUString SAL_CALL implResolveString( const ::rtl::OUString& ResourceID, LocaleItem* pLocaleItem )
        throw (::com::sun::star::resource::MissingResourceException);
    ::sal_Bool implHasEntryForId( const ::rtl::OUString& ResourceID, LocaleItem* pLocaleItem );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > implGetResourceIDs( LocaleItem* pLocaleItem );
    void implSetString( const ::rtl::OUString& ResourceID,
        const ::rtl::OUString& Str, LocaleItem* pLocaleItem );
    void implRemoveId( const ::rtl::OUString& ResourceID, LocaleItem* pLocaleItem )
        throw (::com::sun::star::resource::MissingResourceException);

    // Method to load a locale if necessary, returns true if loading was
    // successful. Default implementation in base class just returns true.
    virtual bool loadLocale( LocaleItem* pLocaleItem );

    virtual void implLoadAllLocales( void );

public:
    StringResourceImpl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~StringResourceImpl();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

     // XStringResourceResolver
    virtual ::rtl::OUString SAL_CALL resolveString( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL resolveStringForLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw ( ::com::sun::star::resource::MissingResourceException,
                    ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasEntryForId( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasEntryForIdAndLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getResourceIDs(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getResourceIDsForLocale
        ( const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getCurrentLocale(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getDefaultLocale(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XStringResourceManager
    virtual ::sal_Bool SAL_CALL isReadOnly()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCurrentLocale( const ::com::sun::star::lang::Locale& locale, ::sal_Bool FindClosestMatch )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL setString( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str )
        throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStringForLocale( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeId( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL removeIdForLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException,
                   ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL newLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::container::ElementExistException, ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual ::sal_Int32 SAL_CALL getUniqueNumericId(  )
        throw (::com::sun::star::lang::NoSupportException,
               ::com::sun::star::uno::RuntimeException);
 };

typedef ::cppu::ImplInheritanceHelper1<
        StringResourceImpl,
        ::com::sun::star::resource::XStringResourcePersistence > StringResourcePersistenceImpl_BASE;

class BinaryOutput;
class BinaryInput;

class StringResourcePersistenceImpl : public StringResourcePersistenceImpl_BASE
{
protected:
    ::rtl::OUString                                                             m_aNameBase;
    ::rtl::OUString                                                             m_aComment;

    void SAL_CALL implInitializeCommonParameters
        ( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // Scan locale properties files
    virtual void implScanLocales( void );

    // Method to load a locale if necessary, returns true if loading was successful
    virtual bool loadLocale( LocaleItem* pLocaleItem );

    // does the actual loading
    virtual bool implLoadLocale( LocaleItem* pLocaleItem );

    virtual void implLoadAllLocales( void );

    void implScanLocaleNames( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aContentSeq );
    ::rtl::OUString implGetFileNameForLocaleItem( LocaleItem* pLocaleItem, const ::rtl::OUString& aNameBase );
    ::rtl::OUString implGetPathForLocaleItem( LocaleItem* pLocaleItem, const ::rtl::OUString& aNameBase,
        const ::rtl::OUString& aLocation, bool bDefaultFile=false );

    bool implReadPropertiesFile( LocaleItem* pLocaleItem,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInput );

    bool implWritePropertiesFile( LocaleItem* pLocaleItem, const ::com::sun::star::uno::Reference
        < ::com::sun::star::io::XOutputStream >& xOutputStream, const ::rtl::OUString& aComment );

    void implWriteLocaleBinary( LocaleItem* pLocaleItem, BinaryOutput& rOut );

    void implStoreAtStorage
    (
        const ::rtl::OUString& aNameBase,
        const ::rtl::OUString& aComment,
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage,
        bool bUsedForStore,
        bool bStoreAll
    )
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    void implKillRemovedLocaleFiles
    (
        const ::rtl::OUString& Location,
        const ::rtl::OUString& aNameBase,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xFileAccess
    )
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    void implKillChangedDefaultFiles
    (
        const ::rtl::OUString& Location,
        const ::rtl::OUString& aNameBase,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xFileAccess
    )
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    void implStoreAtLocation
    (
        const ::rtl::OUString& Location,
        const ::rtl::OUString& aNameBase,
        const ::rtl::OUString& aComment,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xFileAccess,
        bool bUsedForStore,
        bool bStoreAll,
        bool bKillAll = false
    )
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

public:
    StringResourcePersistenceImpl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~StringResourcePersistenceImpl();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

     // XStringResourceResolver
    virtual ::rtl::OUString SAL_CALL resolveString( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL resolveStringForLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw ( ::com::sun::star::resource::MissingResourceException,
                    ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasEntryForId( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasEntryForIdAndLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getResourceIDs(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getResourceIDsForLocale
        ( const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getCurrentLocale(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getDefaultLocale(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XStringResourceManager
    virtual ::sal_Bool SAL_CALL isReadOnly()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCurrentLocale( const ::com::sun::star::lang::Locale& locale, ::sal_Bool FindClosestMatch )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL setString( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str )
        throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStringForLocale( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeId( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL removeIdForLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException,
                   ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL newLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::container::ElementExistException, ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual ::sal_Int32 SAL_CALL getUniqueNumericId(  )
        throw (::com::sun::star::lang::NoSupportException,
               ::com::sun::star::uno::RuntimeException);

    // XStringResourcePersistence
    virtual void SAL_CALL store(  )
        throw (::com::sun::star::lang::NoSupportException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isModified(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setComment( const ::rtl::OUString& Comment )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToStorage
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage,
          const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToURL( const ::rtl::OUString& URL, const ::rtl::OUString& NameBase,
        const ::rtl::OUString& Comment, const ::com::sun::star::uno::Reference
        < ::com::sun::star::task::XInteractionHandler >& Handler )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL exportBinary(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL importBinary( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& Data )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
};


typedef ::cppu::ImplInheritanceHelper2<
        StringResourcePersistenceImpl,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::resource::XStringResourceWithStorage > StringResourceWithStorageImpl_BASE;

class StringResourceWithStorageImpl : public StringResourceWithStorageImpl_BASE
{
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >       m_xStorage;
    bool                                                                        m_bStorageChanged;

    virtual void implScanLocales( void );
    virtual bool implLoadLocale( LocaleItem* pLocaleItem );

public:
    StringResourceWithStorageImpl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~StringResourceWithStorageImpl();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

     // XStringResourceResolver
    virtual ::rtl::OUString SAL_CALL resolveString( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL resolveStringForLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw ( ::com::sun::star::resource::MissingResourceException,
                    ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasEntryForId( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasEntryForIdAndLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getResourceIDs(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getResourceIDsForLocale
        ( const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getCurrentLocale(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getDefaultLocale(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XStringResourceManager
    virtual ::sal_Bool SAL_CALL isReadOnly()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCurrentLocale( const ::com::sun::star::lang::Locale& locale, ::sal_Bool FindClosestMatch )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL setString( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str )
        throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStringForLocale( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeId( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL removeIdForLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException,
                   ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL newLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::container::ElementExistException, ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual ::sal_Int32 SAL_CALL getUniqueNumericId(  )
        throw (::com::sun::star::lang::NoSupportException,
               ::com::sun::star::uno::RuntimeException);

    // XStringResourcePersistence
    virtual void SAL_CALL store(  )
        throw (::com::sun::star::lang::NoSupportException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isModified(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setComment( const ::rtl::OUString& Comment )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToStorage
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage,
          const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToURL( const ::rtl::OUString& URL, const ::rtl::OUString& NameBase,
        const ::rtl::OUString& Comment, const ::com::sun::star::uno::Reference
        < ::com::sun::star::task::XInteractionHandler >& Handler )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL exportBinary(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL importBinary( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& Data )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XStringResourceWithStorage
    virtual void SAL_CALL storeAsStorage
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStorage
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
};


typedef ::cppu::ImplInheritanceHelper2<
        StringResourcePersistenceImpl,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::resource::XStringResourceWithLocation > StringResourceWithLocationImpl_BASE;

class StringResourceWithLocationImpl : public StringResourceWithLocationImpl_BASE
{
    ::rtl::OUString                                                             m_aLocation;
    bool                                                                        m_bLocationChanged;
    com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess >    m_xSFI;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > m_xInteractionHandler;

    const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > getFileAccess( void );

    virtual void implScanLocales( void );
    virtual bool implLoadLocale( LocaleItem* pLocaleItem );

public:
    StringResourceWithLocationImpl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~StringResourceWithLocationImpl();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

     // XStringResourceResolver
    virtual ::rtl::OUString SAL_CALL resolveString( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL resolveStringForLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw ( ::com::sun::star::resource::MissingResourceException,
                    ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasEntryForId( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasEntryForIdAndLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getResourceIDs(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getResourceIDsForLocale
        ( const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getCurrentLocale(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getDefaultLocale(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XStringResourceManager
    virtual ::sal_Bool SAL_CALL isReadOnly()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCurrentLocale( const ::com::sun::star::lang::Locale& locale, ::sal_Bool FindClosestMatch )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL setString( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str )
        throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStringForLocale( const ::rtl::OUString& ResourceID, const ::rtl::OUString& Str,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeId( const ::rtl::OUString& ResourceID )
        throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL removeIdForLocale( const ::rtl::OUString& ResourceID,
        const ::com::sun::star::lang::Locale& locale )
            throw (::com::sun::star::resource::MissingResourceException, ::com::sun::star::uno::RuntimeException,
                   ::com::sun::star::lang::NoSupportException);
    virtual void SAL_CALL newLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::container::ElementExistException, ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeLocale( const ::com::sun::star::lang::Locale& locale )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::NoSupportException);
    virtual ::sal_Int32 SAL_CALL getUniqueNumericId(  )
        throw (::com::sun::star::lang::NoSupportException,
               ::com::sun::star::uno::RuntimeException);

    // XStringResourcePersistence
    virtual void SAL_CALL store(  )
        throw (::com::sun::star::lang::NoSupportException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isModified(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setComment( const ::rtl::OUString& Comment )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToStorage
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage,
          const ::rtl::OUString& NameBase, const ::rtl::OUString& Comment )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToURL( const ::rtl::OUString& URL, const ::rtl::OUString& NameBase,
        const ::rtl::OUString& Comment, const ::com::sun::star::uno::Reference
        < ::com::sun::star::task::XInteractionHandler >& Handler )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL exportBinary(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL importBinary( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& Data )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XStringResourceWithLocation
    virtual void SAL_CALL storeAsURL( const ::rtl::OUString& URL )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setURL( const ::rtl::OUString& URL )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}   // namespace stringtable
//.........................................................................

#endif // SCRIPTING_DLGPROV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
