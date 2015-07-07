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


#include <oooresourceloader.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/simplerm.hxx>
#include <tools/rcid.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>

#include <boost/shared_ptr.hpp>
#include <map>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::resource;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


namespace extensions { namespace resource
{
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
                the caller checked via <code>_resourceManager.IsAvailable( getResourceType(), _resourceId )</code>
                that the required resource really exists
        */
        virtual Any getResource( SimpleResMgr& _resourceManager, sal_Int32 _resourceId ) const = 0;

        virtual ~IResourceType() { };
    };

    class StringResourceAccess : public IResourceType
    {
    public:
        StringResourceAccess();

        // IResourceType
        virtual RESOURCE_TYPE getResourceType() const SAL_OVERRIDE;
        virtual Any getResource( SimpleResMgr& _resourceManager, sal_Int32 _resourceId ) const SAL_OVERRIDE;
    };

    StringResourceAccess::StringResourceAccess()
    {
    }

    RESOURCE_TYPE StringResourceAccess::getResourceType() const
    {
        return RSC_STRING;
    }

    Any StringResourceAccess::getResource( SimpleResMgr& _resourceManager, sal_Int32 _resourceId ) const
    {
        OSL_PRECOND( _resourceManager.IsAvailable( getResourceType(), _resourceId ), "StringResourceAccess::getResource: precondition not met!" );
        Any aResource;
        aResource <<= OUString( _resourceManager.ReadString( _resourceId ) );
        return aResource;
    }

    typedef ::cppu::WeakImplHelper1 <   XResourceBundle
                                    >   OpenOfficeResourceBundle_Base;
    class OpenOfficeResourceBundle : public OpenOfficeResourceBundle_Base
    {
    private:
        typedef ::boost::shared_ptr< IResourceType >            ResourceTypePtr;
        typedef ::std::map< OUString, ResourceTypePtr >  ResourceTypes;

        ::osl::Mutex                    m_aMutex;
        Reference< XResourceBundle >    m_xParent;
        Locale                          m_aLocale;
        SimpleResMgr*                   m_pResourceManager;
        ResourceTypes                   m_aResourceTypes;

    public:
        OpenOfficeResourceBundle(
            const Reference< XComponentContext >& _rxContext,
            const OUString& _rBaseName,
            const Locale& _rLocale
        );

    protected:
        virtual ~OpenOfficeResourceBundle();

    public:
        // XResourceBundle
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::resource::XResourceBundle > SAL_CALL getParent() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::resource::XResourceBundle >& _parent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getDirectElement( const OUString& key ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XNameAccess (base of XResourceBundle)
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XElementAccess (base of XNameAccess)
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
        bool    impl_getDirectElement_nothrow( const OUString& _key, Any& _out_Element ) const;

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
        bool    impl_getResourceTypeAndId_nothrow( const OUString& _key, ResourceTypePtr& _out_resourceType, sal_Int32& _out_resourceId ) const;
    };

    OpenOfficeResourceLoader::OpenOfficeResourceLoader( Reference< XComponentContext > const& _rxContext )
        :m_xContext( _rxContext )
    {
    }


    Reference< XResourceBundle > SAL_CALL OpenOfficeResourceLoader::loadBundle_Default( const OUString& _baseName ) throw (MissingResourceException, RuntimeException, std::exception)
    {
        return loadBundle( _baseName, Application::GetSettings().GetUILanguageTag().getLocale() );
    }


    Reference< XResourceBundle > SAL_CALL OpenOfficeResourceLoader::loadBundle( const OUString& _baseName, const Locale& _locale ) throw (MissingResourceException, RuntimeException, std::exception)
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

    OpenOfficeResourceBundle::OpenOfficeResourceBundle( const Reference< XComponentContext >& /*_rxContext*/, const OUString& _rBaseName, const Locale& _rLocale )
        :m_aLocale( _rLocale )
        ,m_pResourceManager( NULL )
    {
        OUString sBaseName( _rBaseName );
        m_pResourceManager = new SimpleResMgr( OUStringToOString( sBaseName, RTL_TEXTENCODING_UTF8 ).getStr(),
                LanguageTag( m_aLocale) );

        if ( !m_pResourceManager->IsValid() )
        {
            delete m_pResourceManager, m_pResourceManager = NULL;
            throw MissingResourceException();
        }

        // supported resource types so far: strings
        m_aResourceTypes[ OUString( "string" ) ] =
            ResourceTypePtr( new StringResourceAccess );
    }

    OpenOfficeResourceBundle::~OpenOfficeResourceBundle()
    {
        delete m_pResourceManager;
    }

    Reference< XResourceBundle > SAL_CALL OpenOfficeResourceBundle::getParent() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_xParent;
    }

    void SAL_CALL OpenOfficeResourceBundle::setParent( const Reference< XResourceBundle >& _parent ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_xParent = _parent;
    }

    Locale SAL_CALL OpenOfficeResourceBundle::getLocale(  ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_aLocale;
    }

    bool OpenOfficeResourceBundle::impl_getResourceTypeAndId_nothrow( const OUString& _key, ResourceTypePtr& _out_resourceType, sal_Int32& _out_resourceId ) const
    {
        sal_Int32 typeSeparatorPos = _key.indexOf( ':' );
        if ( typeSeparatorPos == -1 )
            // invalid key
            return false;

        OUString resourceType = _key.copy( 0, typeSeparatorPos );

        ResourceTypes::const_iterator typePos = m_aResourceTypes.find( resourceType );
        if ( typePos == m_aResourceTypes.end() )
            // don't know this resource type
            return false;

        _out_resourceType = typePos->second;
        _out_resourceId = _key.copy( typeSeparatorPos + 1 ).toInt32();
        return true;
    }

    bool OpenOfficeResourceBundle::impl_getDirectElement_nothrow( const OUString& _key, Any& _out_Element ) const
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

    Any SAL_CALL OpenOfficeResourceBundle::getDirectElement( const OUString& _key ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Any aElement;
        impl_getDirectElement_nothrow( _key, aElement );
        return aElement;
    }

    Any SAL_CALL OpenOfficeResourceBundle::getByName( const OUString& _key ) throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Any aElement;
        if ( !impl_getDirectElement_nothrow( _key, aElement ) )
        {
            if ( m_xParent.is() )
                aElement = m_xParent->getByName( _key );
        }

        if ( !aElement.hasValue() )
            throw NoSuchElementException( OUString(), *this );

        return aElement;
    }

    Sequence< OUString > SAL_CALL OpenOfficeResourceBundle::getElementNames(  ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_FAIL( "OpenOfficeResourceBundle::getElementNames: not implemented!" );
            // the (Simple)ResManager does not provide an API to enumerate the resources
        return Sequence< OUString >( );
    }

    sal_Bool SAL_CALL OpenOfficeResourceBundle::hasByName( const OUString& _key ) throw (RuntimeException, std::exception)
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

    Type SAL_CALL OpenOfficeResourceBundle::getElementType(  ) throw (RuntimeException, std::exception)
    {
        return ::cppu::UnoType< Any >::get();
    }

    sal_Bool SAL_CALL OpenOfficeResourceBundle::hasElements(  ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_FAIL( "OpenOfficeResourceBundle::hasElements: not implemented!" );
            // the (Simple)ResManager does not provide an API to enumerate the resources
        return sal_Bool( );
    }

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
