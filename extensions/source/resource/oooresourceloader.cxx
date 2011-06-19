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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_RESOURCE_OOORESOURCELOADER_CXX
#define EXTENSIONS_SOURCE_RESOURCE_OOORESOURCELOADER_CXX
#include "res_services.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/resource/XResourceBundleLoader.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
/** === end UNO includes === **/
#include <vcl/svapp.hxx>
#include <tools/simplerm.hxx>
#include <tools/rcid.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>

#include <boost/shared_ptr.hpp>
#include <map>

//........................................................................
namespace res
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::resource::XResourceBundleLoader;
    using ::com::sun::star::resource::XResourceBundle;
    using ::com::sun::star::resource::MissingResourceException;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::WeakReference;
    /** === end UNO using === **/

    //====================================================================
    //= helper
    //====================================================================
    typedef ::std::pair< ::rtl::OUString, Locale >  ResourceBundleDescriptor;

    struct ResourceBundleDescriptorLess : public ::std::binary_function< ResourceBundleDescriptor, ResourceBundleDescriptor, bool >
    {
        bool operator()( const ResourceBundleDescriptor& _lhs, const ResourceBundleDescriptor& _rhs ) const
        {
            if ( _lhs.first < _rhs.first )
                return true;
            if ( _lhs.second.Language < _rhs.second.Language )
                return true;
            if ( _lhs.second.Country < _rhs.second.Country )
                return true;
            if ( _lhs.second.Variant < _rhs.second.Variant )
                return true;
            return false;
        }
    };

    //====================================================================
    //= OpenOfficeResourceLoader
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   XResourceBundleLoader
                                    >   OpenOfficeResourceLoader_Base;
    class OpenOfficeResourceLoader : public OpenOfficeResourceLoader_Base
    {
    private:
        typedef ::std::map< ResourceBundleDescriptor, WeakReference< XResourceBundle >, ResourceBundleDescriptorLess >
                                        ResourceBundleCache;

    private:
        Reference< XComponentContext >  m_xContext;
        ::osl::Mutex                    m_aMutex;
        ResourceBundleCache             m_aBundleCache;

    protected:
        OpenOfficeResourceLoader( const Reference< XComponentContext >& _rxContext );

    public:
        static Sequence< ::rtl::OUString > getSupportedServiceNames_static();
        static ::rtl::OUString  getImplementationName_static();
        static ::rtl::OUString  getSingletonName_static();
        static Reference< XInterface > Create( const Reference< XComponentContext >& _rxContext );

        // XResourceBundleLoader
        virtual Reference< XResourceBundle > SAL_CALL loadBundle_Default( const ::rtl::OUString& aBaseName ) throw (MissingResourceException, RuntimeException);
        virtual Reference< XResourceBundle > SAL_CALL loadBundle( const ::rtl::OUString& abaseName, const Locale& aLocale ) throw (MissingResourceException, RuntimeException);

    private:
        OpenOfficeResourceLoader();                                             // never implemented
        OpenOfficeResourceLoader( const OpenOfficeResourceLoader& );            // never implemented
        OpenOfficeResourceLoader& operator=( const OpenOfficeResourceLoader& ); // never implemented
    };

    //====================================================================
    //= IResourceType
    //====================================================================
    /** encapsulates access to a fixed resource type
    */
    class IResourceType
    {
    public:
        /** returns the RESOURCE_TYPE associated with this instance
        */
        virtual RESOURCE_TYPE getResourceType() const = 0;

        /** reads a single resource from the given resource manager
            @param  _resourceManager
                the resource manager to read from
            @param  _resourceId
                the id of the resource to read
            @return
                the required resource
            @precond
                the caler checked via <code>_resourceManager.IsAvailable( getResourceType(), _resourceId )</code>
                that the required resource really exists
        */
        virtual Any getResource( SimpleResMgr& _resourceManager, sal_Int32 _resourceId ) const = 0;

        virtual ~IResourceType() { };
    };

    //====================================================================
    //= StringResourceAccess
    //====================================================================
    class StringResourceAccess : public IResourceType
    {
    public:
        StringResourceAccess();

        // IResourceType
        virtual RESOURCE_TYPE getResourceType() const;
        virtual Any getResource( SimpleResMgr& _resourceManager, sal_Int32 _resourceId ) const;
    };

    //--------------------------------------------------------------------
    StringResourceAccess::StringResourceAccess()
    {
    }

    //--------------------------------------------------------------------
    RESOURCE_TYPE StringResourceAccess::getResourceType() const
    {
        return RSC_STRING;
    }

    //--------------------------------------------------------------------
    Any StringResourceAccess::getResource( SimpleResMgr& _resourceManager, sal_Int32 _resourceId ) const
    {
        OSL_PRECOND( _resourceManager.IsAvailable( getResourceType(), _resourceId ), "StringResourceAccess::getResource: precondition not met!" );
        Any aResource;
        aResource <<= ::rtl::OUString( _resourceManager.ReadString( _resourceId ) );
        return aResource;
    }

    //====================================================================
    //= OpenOfficeResourceBundle
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   XResourceBundle
                                    >   OpenOfficeResourceBundle_Base;
    class OpenOfficeResourceBundle : public OpenOfficeResourceBundle_Base
    {
    private:
        typedef ::boost::shared_ptr< IResourceType >            ResourceTypePtr;
        typedef ::std::map< ::rtl::OUString, ResourceTypePtr >  ResourceTypes;

        ::osl::Mutex                    m_aMutex;
        Reference< XResourceBundle >    m_xParent;
        Locale                          m_aLocale;
        SimpleResMgr*                   m_pResourceManager;
        ResourceTypes                   m_aResourceTypes;

    public:
        OpenOfficeResourceBundle(
            const Reference< XComponentContext >& _rxContext,
            const ::rtl::OUString& _rBaseName,
            const Locale& _rLocale
        );

    protected:
        ~OpenOfficeResourceBundle();

    public:
        // XResourceBundle
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::resource::XResourceBundle > SAL_CALL getParent() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::resource::XResourceBundle >& _parent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getDirectElement( const ::rtl::OUString& key ) throw (::com::sun::star::uno::RuntimeException);

        // XNameAccess (base of XResourceBundle)
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess (base of XNameAccess)
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** retrievs the element with the given key, without asking our parent bundle
            @param  _key
                the key of the element to retrieve
            @param  _out_Element
                will contained the retrieved element upon successful return. If the method is unsuccessful, the
                value will not be touched.
            @return
                <TRUE/> if and only if the element could be retrieved
            @precond
                our mutex is locked
        */
        bool    impl_getDirectElement_nothrow( const ::rtl::OUString& _key, Any& _out_Element ) const;

        /** retrieves the resource type and id from a given resource key, which assembles those two
            @param  _key
                the resource key as got via a public API call
            @param  _out_resourceType
                the resource type, if successful
            @param  _out_resourceId
                the resource id, if successful
            @return
                <TRUE/> if and only if the given key specifies a known resource type, and contains a valid
                resource id
        */
        bool    impl_getResourceTypeAndId_nothrow( const ::rtl::OUString& _key, ResourceTypePtr& _out_resourceType, sal_Int32& _out_resourceId ) const;
    };

    //====================================================================
    //= OpenOfficeResourceLoader
    //====================================================================
    //--------------------------------------------------------------------
    OpenOfficeResourceLoader::OpenOfficeResourceLoader( const Reference< XComponentContext >& _rxContext )
        :m_xContext( _rxContext )
    {
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > OpenOfficeResourceLoader::getSupportedServiceNames_static()
    {
        Sequence< ::rtl::OUString > aServices( 1 );
        aServices[ 0 ] = getSingletonName_static();
        return aServices;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OpenOfficeResourceLoader::getImplementationName_static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.resource.OpenOfficeResourceLoader" ) );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OpenOfficeResourceLoader::getSingletonName_static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.resource.OfficeResourceLoader" ) );
    }

    //--------------------------------------------------------------------
    Reference< XInterface > OpenOfficeResourceLoader::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new OpenOfficeResourceLoader( _rxContext ) );
    }

    //--------------------------------------------------------------------
    Reference< XResourceBundle > SAL_CALL OpenOfficeResourceLoader::loadBundle_Default( const ::rtl::OUString& _baseName ) throw (MissingResourceException, RuntimeException)
    {
        return loadBundle( _baseName, Application::GetSettings().GetUILocale() );
    }

    //--------------------------------------------------------------------
    Reference< XResourceBundle > SAL_CALL OpenOfficeResourceLoader::loadBundle( const ::rtl::OUString& _baseName, const Locale& _locale ) throw (MissingResourceException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XResourceBundle > xBundle;

        ResourceBundleDescriptor resourceDescriptor( _baseName, _locale );
        ResourceBundleCache::iterator cachePos = m_aBundleCache.find( resourceDescriptor );
        if ( cachePos != m_aBundleCache.end() )
            xBundle = cachePos->second;

        if ( !xBundle.is() )
        {   // not in the cache, or already died
            xBundle = new OpenOfficeResourceBundle( m_xContext, _baseName, _locale );
            m_aBundleCache.insert( ResourceBundleCache::value_type( resourceDescriptor, xBundle ) );
        }

        return xBundle;
    }

    //--------------------------------------------------------------------
    ComponentInfo getComponentInfo_OpenOfficeResourceLoader()
    {
        ComponentInfo aInfo;
        aInfo.aSupportedServices = OpenOfficeResourceLoader::getSupportedServiceNames_static();
        aInfo.sImplementationName = OpenOfficeResourceLoader::getImplementationName_static();
        aInfo.sSingletonName = OpenOfficeResourceLoader::getSingletonName_static();
        aInfo.pFactory = &OpenOfficeResourceLoader::Create;
        return aInfo;
    }

    //====================================================================
    //= OpenOfficeResourceBundle
    //====================================================================
    //--------------------------------------------------------------------
    OpenOfficeResourceBundle::OpenOfficeResourceBundle( const Reference< XComponentContext >& /*_rxContext*/, const ::rtl::OUString& _rBaseName, const Locale& _rLocale )
        :m_aLocale( _rLocale )
        ,m_pResourceManager( NULL )
    {
        ::rtl::OUString sBaseName( _rBaseName );
        m_pResourceManager = new SimpleResMgr( sBaseName, m_aLocale );

        if ( !m_pResourceManager->IsValid() )
        {
            delete m_pResourceManager, m_pResourceManager = NULL;
            throw MissingResourceException();
        }

        // supported resource types so far: strings
        m_aResourceTypes[ ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "string" ) ) ] =
            ResourceTypePtr( new StringResourceAccess );
    }

    //--------------------------------------------------------------------
    OpenOfficeResourceBundle::~OpenOfficeResourceBundle()
    {
        delete m_pResourceManager;
    }

    //--------------------------------------------------------------------
    Reference< XResourceBundle > SAL_CALL OpenOfficeResourceBundle::getParent() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_xParent;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OpenOfficeResourceBundle::setParent( const Reference< XResourceBundle >& _parent ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_xParent = _parent;
    }

    //--------------------------------------------------------------------
    Locale SAL_CALL OpenOfficeResourceBundle::getLocale(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_aLocale;
    }

    //--------------------------------------------------------------------
    bool OpenOfficeResourceBundle::impl_getResourceTypeAndId_nothrow( const ::rtl::OUString& _key, ResourceTypePtr& _out_resourceType, sal_Int32& _out_resourceId ) const
    {
        sal_Int32 typeSeparatorPos = _key.indexOf( ':' );
        if ( typeSeparatorPos == -1 )
            // invalid key
            return false;

        ::rtl::OUString resourceType = _key.copy( 0, typeSeparatorPos );

        ResourceTypes::const_iterator typePos = m_aResourceTypes.find( resourceType );
        if ( typePos == m_aResourceTypes.end() )
            // don't know this resource type
            return false;

        _out_resourceType = typePos->second;
        _out_resourceId = _key.copy( typeSeparatorPos + 1 ).toInt32();
        return true;
    }

    //--------------------------------------------------------------------
    bool OpenOfficeResourceBundle::impl_getDirectElement_nothrow( const ::rtl::OUString& _key, Any& _out_Element ) const
    {
        ResourceTypePtr resourceType;
        sal_Int32 resourceId( 0 );
        if ( !impl_getResourceTypeAndId_nothrow( _key, resourceType, resourceId ) )
            return false;

        if ( !m_pResourceManager->IsAvailable( resourceType->getResourceType(), resourceId ) )
            // no such resource with the given type/id
            return false;

        _out_Element = resourceType->getResource( *m_pResourceManager, resourceId );
        return _out_Element.hasValue();
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OpenOfficeResourceBundle::getDirectElement( const ::rtl::OUString& _key ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Any aElement;
        impl_getDirectElement_nothrow( _key, aElement );
        return aElement;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OpenOfficeResourceBundle::getByName( const ::rtl::OUString& _key ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Any aElement;
        if ( !impl_getDirectElement_nothrow( _key, aElement ) )
        {
            if ( m_xParent.is() )
                aElement = m_xParent->getByName( _key );
        }

        if ( !aElement.hasValue() )
            throw NoSuchElementException( ::rtl::OUString(), *this );

        return aElement;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OpenOfficeResourceBundle::getElementNames(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_FAIL( "OpenOfficeResourceBundle::getElementNames: not implemented!" );
            // the (Simple)ResManager does not provide an API to enumerate the resources
        return Sequence< ::rtl::OUString >( );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL OpenOfficeResourceBundle::hasByName( const ::rtl::OUString& _key ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ResourceTypePtr resourceType;
        sal_Int32 resourceId( 0 );
        if ( !impl_getResourceTypeAndId_nothrow( _key, resourceType, resourceId ) )
            return sal_False;

        if ( !m_pResourceManager->IsAvailable( resourceType->getResourceType(), resourceId ) )
            return sal_False;

        return sal_True;
    }

    //--------------------------------------------------------------------
    Type SAL_CALL OpenOfficeResourceBundle::getElementType(  ) throw (RuntimeException)
    {
        return ::cppu::UnoType< Any >::get();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL OpenOfficeResourceBundle::hasElements(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_FAIL( "OpenOfficeResourceBundle::hasElements: not implemented!" );
            // the (Simple)ResManager does not provide an API to enumerate the resources
        return ::sal_Bool( );
    }

//........................................................................
} // namespace res
//........................................................................

#endif // EXTENSIONS_SOURCE_RESOURCE_OOORESOURCELOADER_CXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
