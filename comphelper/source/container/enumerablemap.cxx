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


#include <comphelper_module.hxx>
#include <comphelper_services.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/anycompare.hxx>
#include <comphelper/componentbase.hxx>

#include <com/sun/star/container/XEnumerableMap.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <typelib/typedescription.hxx>

#include <map>
#include <memory>
#include <utility>

namespace comphelper
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::container::XEnumerableMap;
    using ::com::sun::star::lang::NoSupportException;
    using ::com::sun::star::beans::IllegalTypeException;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::XInitialization;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::beans::Pair;
    using ::com::sun::star::uno::TypeClass;
    using ::com::sun::star::uno::TypeClass_VOID;
    using ::com::sun::star::uno::TypeClass_UNKNOWN;
    using ::com::sun::star::uno::TypeClass_ANY;
    using ::com::sun::star::uno::TypeClass_EXCEPTION;
    using ::com::sun::star::uno::TypeClass_STRUCT;
    using ::com::sun::star::uno::TypeClass_FLOAT;
    using ::com::sun::star::uno::TypeClass_DOUBLE;
    using ::com::sun::star::uno::TypeClass_INTERFACE;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::uno::TypeDescription;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::lang::DisposedException;

    class MapEnumerator;

    typedef std::map< Any, Any, LessPredicateAdapter > KeyedValues;
    struct MapData
    {
        Type                                        m_aKeyType;
        Type                                        m_aValueType;
        std::unique_ptr< KeyedValues >            m_pValues;
        std::shared_ptr< IKeyPredicateLess >      m_pKeyCompare;
        bool                                        m_bMutable;
        std::vector< MapEnumerator* >             m_aModListeners;

        MapData()
            :m_bMutable( true )
        {
        }

        MapData( const MapData& _source )
            :m_aKeyType( _source.m_aKeyType )
            ,m_aValueType( _source.m_aValueType )
            ,m_pValues( new KeyedValues( *_source.m_pValues ) )
            ,m_pKeyCompare( _source.m_pKeyCompare )
            ,m_bMutable( false )
            ,m_aModListeners()
        {
        }
    private:
        MapData& operator=( const MapData& _source ) = delete;
    };


    static void lcl_registerMapModificationListener( MapData& _mapData, MapEnumerator& _listener )
    {
    #if OSL_DEBUG_LEVEL > 0
        for ( const MapEnumerator* lookup : _mapData.m_aModListeners )
        {
            OSL_ENSURE( lookup != &_listener, "lcl_registerMapModificationListener: this listener is already registered!" );
        }
    #endif
        _mapData.m_aModListeners.push_back( &_listener );
    }


    static void lcl_revokeMapModificationListener( MapData& _mapData, MapEnumerator& _listener )
    {
        auto lookup = std::find(_mapData.m_aModListeners.begin(), _mapData.m_aModListeners.end(), &_listener);
        if (lookup != _mapData.m_aModListeners.end())
        {
            _mapData.m_aModListeners.erase( lookup );
            return;
        }
        OSL_FAIL( "lcl_revokeMapModificationListener: the listener is not registered!" );
    }


    static void lcl_notifyMapDataListeners_nothrow( const MapData& _mapData );


    // EnumerableMap

    typedef ::cppu::WeakAggComponentImplHelper3 <   XInitialization
                                                ,   XEnumerableMap
                                                ,   XServiceInfo
                                                > Map_IFace;

    class EnumerableMap: public Map_IFace, public ComponentBase
    {
    protected:
        EnumerableMap();
        virtual ~EnumerableMap() override;

        // XInitialization
        virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

        // XEnumerableMap
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createKeyEnumeration( sal_Bool Isolated ) override;
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createValueEnumeration( sal_Bool Isolated ) override;
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createElementEnumeration( sal_Bool Isolated ) override;

        // XMap
        virtual Type SAL_CALL getKeyType() override;
        virtual Type SAL_CALL getValueType() override;
        virtual void SAL_CALL clear(  ) override;
        virtual sal_Bool SAL_CALL containsKey( const Any& _key ) override;
        virtual sal_Bool SAL_CALL containsValue( const Any& _value ) override;
        virtual Any SAL_CALL get( const Any& _key ) override;
        virtual Any SAL_CALL put( const Any& _key, const Any& _value ) override;
        virtual Any SAL_CALL remove( const Any& _key ) override;

        // XElementAccess (base of XMap)
        virtual Type SAL_CALL getElementType() override;
        virtual sal_Bool SAL_CALL hasElements() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    public:
        // XServiceInfo, static version (used for component registration)
        static OUString getImplementationName_static(  );
        static Sequence< OUString > getSupportedServiceNames_static(  );
        static Reference< XInterface > Create( const Reference< XComponentContext >& );

    private:
        void    impl_initValues_throw( const Sequence< Pair< Any, Any > >& _initialValues );

        /// throws a IllegalTypeException if the given value is not compatible with our ValueType
        void    impl_checkValue_throw( const Any& _value ) const;
        void    impl_checkKey_throw( const Any& _key ) const;
        void    impl_checkNaN_throw( const Any& _keyOrValue, const Type& _keyOrValueType ) const;
        void    impl_checkMutable_throw() const;

    private:
        ::osl::Mutex        m_aMutex;
        MapData             m_aData;
    };


    enum EnumerationType
    {
        eKeys, eValues, eBoth
    };


    class MapEnumerator final
    {
    public:
        MapEnumerator( ::cppu::OWeakObject& _rParent, MapData& _mapData, const EnumerationType _type )
            :m_rParent( _rParent )
            ,m_rMapData( _mapData )
            ,m_eType( _type )
            ,m_mapPos( _mapData.m_pValues->begin() )
            ,m_disposed( false )
        {
            lcl_registerMapModificationListener( m_rMapData, *this );
        }

        ~MapEnumerator()
        {
            dispose();
        }

        void dispose()
        {
            if ( !m_disposed )
            {
                lcl_revokeMapModificationListener( m_rMapData, *this );
                m_disposed = true;
            }
        }

        // noncopyable
        MapEnumerator(const MapEnumerator&) = delete;
        const MapEnumerator& operator=(const MapEnumerator&) = delete;

        // XEnumeration equivalents
        bool hasMoreElements();
        Any nextElement();

        /// called when the map was modified
        void mapModified();

    private:
        ::cppu::OWeakObject&        m_rParent;
        MapData&                    m_rMapData;
        const EnumerationType       m_eType;
        KeyedValues::const_iterator m_mapPos;
        bool                        m_disposed;
    };

    static void lcl_notifyMapDataListeners_nothrow( const MapData& _mapData )
    {
        for ( MapEnumerator* loop : _mapData.m_aModListeners )
        {
            loop->mapModified();
        }
    }

    typedef ::cppu::WeakImplHelper <   XEnumeration
                                   >   MapEnumeration_Base;
    class MapEnumeration :public ComponentBase
                         ,public MapEnumeration_Base
    {
    public:
        MapEnumeration( ::cppu::OWeakObject& _parentMap, MapData& _mapData, ::cppu::OBroadcastHelper& _rBHelper,
                        const EnumerationType _type, const bool _isolated )
            :ComponentBase( _rBHelper, ComponentBase::NoInitializationNeeded() )
            ,m_xKeepMapAlive( _parentMap )
            ,m_pMapDataCopy( _isolated ? new MapData( _mapData ) : nullptr )
            ,m_aEnumerator( *this, _isolated ? *m_pMapDataCopy : _mapData, _type )
        {
        }

        // XEnumeration
        virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
        virtual Any SAL_CALL nextElement(  ) override;

    protected:
        virtual ~MapEnumeration() override
        {
            acquire();
            {
                ::osl::MutexGuard aGuard( getMutex() );
                m_aEnumerator.dispose();
                m_pMapDataCopy.reset();
            }
        }

    private:
        // since we share our mutex with the main map, we need to keep it alive as long as we live
        Reference< XInterface >     m_xKeepMapAlive;
        std::unique_ptr< MapData > m_pMapDataCopy;
        MapEnumerator               m_aEnumerator;
    };


    EnumerableMap::EnumerableMap()
        :Map_IFace( m_aMutex )
        ,ComponentBase( Map_IFace::rBHelper )
    {
    }


    EnumerableMap::~EnumerableMap()
    {
        if ( !impl_isDisposed() )
        {
            acquire();
            dispose();
        }
    }


    void SAL_CALL EnumerableMap::initialize( const Sequence< Any >& _arguments )
    {
        ComponentMethodGuard aGuard( *this, ComponentMethodGuard::MethodType::WithoutInit );
        if ( impl_isInitialized_nothrow() )
            throw AlreadyInitializedException();

        sal_Int32 nArgumentCount = _arguments.getLength();
        if ( ( nArgumentCount != 2 ) && ( nArgumentCount != 3 ) )
            throw IllegalArgumentException();

        Type aKeyType, aValueType;
        if ( !( _arguments[0] >>= aKeyType ) )
            throw IllegalArgumentException("com.sun.star.uno.Type expected.", *this, 1 );
        if ( !( _arguments[1] >>= aValueType ) )
            throw IllegalArgumentException("com.sun.star.uno.Type expected.", *this, 2 );

        Sequence< Pair< Any, Any > > aInitialValues;
        bool bMutable = true;
        if ( nArgumentCount == 3 )
        {
            if ( !( _arguments[2] >>= aInitialValues ) )
                throw IllegalArgumentException("[]com.sun.star.beans.Pair<any,any> expected.", *this, 2 );
            bMutable = false;
        }

        // for the value, anything is allowed, except VOID
        if ( ( aValueType.getTypeClass() == TypeClass_VOID ) || ( aValueType.getTypeClass() == TypeClass_UNKNOWN ) )
            throw IllegalTypeException("Unsupported value type.", *this );

        // create the comparator for the KeyType, and throw if the type is not supported
        std::unique_ptr< IKeyPredicateLess > pComparator( getStandardLessPredicate( aKeyType, nullptr ) );
        if (!pComparator)
            throw IllegalTypeException("Unsupported key type.", *this );

        // init members
        m_aData.m_aKeyType = aKeyType;
        m_aData.m_aValueType = aValueType;
        m_aData.m_pKeyCompare = std::move(pComparator);
        m_aData.m_pValues.reset( new KeyedValues( *m_aData.m_pKeyCompare ) );
        m_aData.m_bMutable = bMutable;

        if ( aInitialValues.getLength() )
            impl_initValues_throw( aInitialValues );

        setInitialized();
    }


    void EnumerableMap::impl_initValues_throw( const Sequence< Pair< Any, Any > >& _initialValues )
    {
        OSL_PRECOND( m_aData.m_pValues.get() && m_aData.m_pValues->empty(), "EnumerableMap::impl_initValues_throw: illegal call!" );
        if (!m_aData.m_pValues || !m_aData.m_pValues->empty())
            throw RuntimeException();

        const Pair< Any, Any >* mapping = _initialValues.getConstArray();
        const Pair< Any, Any >* mappingEnd = mapping + _initialValues.getLength();
        for ( ; mapping != mappingEnd; ++mapping )
        {
            impl_checkValue_throw( mapping->Second );
            (*m_aData.m_pValues)[ mapping->First ] = mapping->Second;
        }
    }


    void EnumerableMap::impl_checkValue_throw( const Any& _value ) const
    {
        if ( !_value.hasValue() )
            // nothing to do, NULL values are always allowed, regardless of the ValueType
            return;

        TypeClass eAllowedTypeClass = m_aData.m_aValueType.getTypeClass();
        bool bValid = false;

        switch ( eAllowedTypeClass )
        {
        default:
            bValid = ( _value.getValueTypeClass() == eAllowedTypeClass );
            break;
        case TypeClass_ANY:
            bValid = true;
            break;
        case TypeClass_INTERFACE:
        {
            // special treatment: _value might contain the proper type, but the interface
            // might actually be NULL. Which is still valid ...
            if ( m_aData.m_aValueType.isAssignableFrom( _value.getValueType() ) )
                // this also catches the special case where XFoo is our value type,
                // and _value contains a NULL-reference to XFoo, or a derived type
                bValid = true;
            else
            {
                Reference< XInterface > xValue( _value, UNO_QUERY );
                if ( xValue.is() )
                    // XInterface is not-NULL, but is X(ValueType) not-NULL, too?
                    xValue.set( xValue->queryInterface( m_aData.m_aValueType ), UNO_QUERY );
                bValid = xValue.is();
            }
        }
        break;
        case TypeClass_EXCEPTION:
        case TypeClass_STRUCT:
        {
            // values are accepted if and only if their type equals, or is derived from, our value type

            if ( _value.getValueTypeClass() != eAllowedTypeClass )
                bValid = false;
            else
            {
                const TypeDescription aValueTypeDesc( _value.getValueType() );
                const TypeDescription aRequiredTypeDesc( m_aData.m_aValueType );

                const _typelib_CompoundTypeDescription* pValueCompoundTypeDesc =
                    reinterpret_cast< const _typelib_CompoundTypeDescription* >( aValueTypeDesc.get() );

                while ( pValueCompoundTypeDesc )
                {
                    if ( typelib_typedescription_equals( &pValueCompoundTypeDesc->aBase, aRequiredTypeDesc.get() ) )
                        break;
                    pValueCompoundTypeDesc = pValueCompoundTypeDesc->pBaseTypeDescription;
                }
                bValid = ( pValueCompoundTypeDesc != nullptr );
            }
        }
        break;
        }

        if ( !bValid )
        {
            throw IllegalTypeException(
                "Incompatible value type. Found '" + _value.getValueTypeName()
                + "', where '" + m_aData.m_aValueType.getTypeName()
                + "' (or compatible type) is expected.",
                *const_cast< EnumerableMap* >( this ) );
        }

        impl_checkNaN_throw( _value, m_aData.m_aValueType );
    }


    void EnumerableMap::impl_checkNaN_throw( const Any& _keyOrValue, const Type& _keyOrValueType ) const
    {
        if  (   ( _keyOrValueType.getTypeClass() == TypeClass_DOUBLE )
            ||  ( _keyOrValueType.getTypeClass() == TypeClass_FLOAT )
            )
        {
            double nValue(0);
            if ( _keyOrValue >>= nValue )
                if ( ::rtl::math::isNan( nValue ) )
                    throw IllegalArgumentException(
                        "NaN (not-a-number) not supported by this implementation.",
                        *const_cast< EnumerableMap* >( this ), 0 );
            // (note that the case of _key not containing a float/double value is handled in the
            // respective IKeyPredicateLess implementation, so there's no need to handle this here.)
        }
    }


    void EnumerableMap::impl_checkKey_throw( const Any& _key ) const
    {
        if ( !_key.hasValue() )
            throw IllegalArgumentException(
                "NULL keys not supported by this implementation.",
                *const_cast< EnumerableMap* >( this ), 0 );

        impl_checkNaN_throw( _key, m_aData.m_aKeyType );
    }


    void EnumerableMap::impl_checkMutable_throw() const
    {
        if ( !m_aData.m_bMutable )
            throw NoSupportException(
                    "The map is immutable.",
                    *const_cast< EnumerableMap* >( this ) );
    }


    Reference< XEnumeration > SAL_CALL EnumerableMap::createKeyEnumeration( sal_Bool Isolated )
    {
        ComponentMethodGuard aGuard( *this );
        return new MapEnumeration( *this, m_aData, getBroadcastHelper(), eKeys, Isolated );
    }


    Reference< XEnumeration > SAL_CALL EnumerableMap::createValueEnumeration( sal_Bool Isolated )
    {
        ComponentMethodGuard aGuard( *this );
        return new MapEnumeration( *this, m_aData, getBroadcastHelper(), eValues, Isolated );
    }


    Reference< XEnumeration > SAL_CALL EnumerableMap::createElementEnumeration( sal_Bool Isolated )
    {
        ComponentMethodGuard aGuard( *this );
        return new MapEnumeration( *this, m_aData, getBroadcastHelper(), eBoth, Isolated );
    }


    Type SAL_CALL EnumerableMap::getKeyType()
    {
        ComponentMethodGuard aGuard( *this );
        return m_aData.m_aKeyType;
    }


    Type SAL_CALL EnumerableMap::getValueType()
    {
        ComponentMethodGuard aGuard( *this );
        return m_aData.m_aValueType;
    }


    void SAL_CALL EnumerableMap::clear(  )
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkMutable_throw();

        m_aData.m_pValues->clear();

        lcl_notifyMapDataListeners_nothrow( m_aData );
    }


    sal_Bool SAL_CALL EnumerableMap::containsKey( const Any& _key )
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkKey_throw( _key );

        KeyedValues::const_iterator pos = m_aData.m_pValues->find( _key );
        return ( pos != m_aData.m_pValues->end() );
    }


    sal_Bool SAL_CALL EnumerableMap::containsValue( const Any& _value )
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkValue_throw( _value );
        for (auto const& value : *m_aData.m_pValues)
        {
            if ( value.second == _value )
                return true;
        }
        return false;
    }


    Any SAL_CALL EnumerableMap::get( const Any& _key )
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkKey_throw( _key );

        KeyedValues::const_iterator pos = m_aData.m_pValues->find( _key );
        if ( pos == m_aData.m_pValues->end() )
            throw NoSuchElementException( anyToString( _key ), *this );

        return pos->second;
    }


    Any SAL_CALL EnumerableMap::put( const Any& _key, const Any& _value )
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkMutable_throw();
        impl_checkKey_throw( _key );
        impl_checkValue_throw( _value );

        Any previousValue;

        KeyedValues::iterator pos = m_aData.m_pValues->find( _key );
        if ( pos != m_aData.m_pValues->end() )
        {
            previousValue = pos->second;
            pos->second = _value;
        }
        else
        {
            (*m_aData.m_pValues)[ _key ] = _value;
        }

        lcl_notifyMapDataListeners_nothrow( m_aData );

        return previousValue;
    }


    Any SAL_CALL EnumerableMap::remove( const Any& _key )
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkMutable_throw();
        impl_checkKey_throw( _key );

        Any previousValue;

        KeyedValues::iterator pos = m_aData.m_pValues->find( _key );
        if ( pos != m_aData.m_pValues->end() )
        {
            previousValue = pos->second;
            m_aData.m_pValues->erase( pos );
        }

        lcl_notifyMapDataListeners_nothrow( m_aData );

        return previousValue;
    }


    Type SAL_CALL EnumerableMap::getElementType()
    {
        return ::cppu::UnoType< Pair< Any, Any > >::get();
    }


    sal_Bool SAL_CALL EnumerableMap::hasElements()
    {
        ComponentMethodGuard aGuard( *this );
        return m_aData.m_pValues->empty();
    }


    OUString SAL_CALL EnumerableMap::getImplementationName(  )
    {
        return getImplementationName_static();
    }

    sal_Bool SAL_CALL EnumerableMap::supportsService( const OUString& _serviceName )
    {
        return cppu::supportsService(this, _serviceName);
    }


    Sequence< OUString > SAL_CALL EnumerableMap::getSupportedServiceNames(  )
    {
        return getSupportedServiceNames_static();
    }


    OUString EnumerableMap::getImplementationName_static(  )
    {
        return OUString( "org.openoffice.comp.comphelper.EnumerableMap" );
    }


    Sequence< OUString > EnumerableMap::getSupportedServiceNames_static(  )
    {
        Sequence< OUString > aServiceNames { "com.sun.star.container.EnumerableMap" };
        return aServiceNames;
    }


    Reference< XInterface > EnumerableMap::Create( SAL_UNUSED_PARAMETER const Reference< XComponentContext >& )
    {
        return *new EnumerableMap;
    }


    bool MapEnumerator::hasMoreElements()
    {
        if ( m_disposed )
            throw DisposedException( OUString(), m_rParent );
        return m_mapPos != m_rMapData.m_pValues->end();
    }


    Any MapEnumerator::nextElement()
    {
        if ( m_disposed )
            throw DisposedException( OUString(), m_rParent );
        if ( m_mapPos == m_rMapData.m_pValues->end() )
            throw NoSuchElementException("No more elements.", m_rParent );

        Any aNextElement;
        switch ( m_eType )
        {
        case eKeys:     aNextElement = m_mapPos->first; break;
        case eValues:   aNextElement = m_mapPos->second; break;
        case eBoth:     aNextElement <<= Pair< Any, Any >( m_mapPos->first, m_mapPos->second ); break;
        }
        ++m_mapPos;
        return aNextElement;
    }


    void MapEnumerator::mapModified()
    {
        m_disposed = true;
    }


    sal_Bool SAL_CALL MapEnumeration::hasMoreElements(  )
    {
        ComponentMethodGuard aGuard( *this );
        return m_aEnumerator.hasMoreElements();
    }


    Any SAL_CALL MapEnumeration::nextElement(  )
    {
        ComponentMethodGuard aGuard( *this );
        return m_aEnumerator.nextElement();
    }


} // namespace comphelper


void createRegistryInfo_Map()
{
    ::comphelper::module::OAutoRegistration< ::comphelper::EnumerableMap > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
