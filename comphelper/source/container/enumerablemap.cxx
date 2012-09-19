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


#include "comphelper_module.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/anycompare.hxx"
#include "comphelper/componentbase.hxx"
#include "comphelper/extract.hxx"

#include <com/sun/star/container/XEnumerableMap.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <typelib/typedescription.hxx>

#include <map>
#include <boost/shared_ptr.hpp>

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
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
    using ::com::sun::star::uno::TypeClass_UNION;
    using ::com::sun::star::uno::TypeClass_FLOAT;
    using ::com::sun::star::uno::TypeClass_DOUBLE;
    using ::com::sun::star::uno::TypeClass_INTERFACE;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::uno::TypeDescription;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::lang::DisposedException;
    /** === end UNO using === **/

    //====================================================================
    //= MapData
    //====================================================================
    class IMapModificationListener;
    typedef ::std::vector< IMapModificationListener* > MapListeners;

    typedef ::std::map< Any, Any, LessPredicateAdapter > KeyedValues;
    struct MapData
    {
        Type                                        m_aKeyType;
        Type                                        m_aValueType;
        ::std::auto_ptr< KeyedValues >              m_pValues;
        ::boost::shared_ptr< IKeyPredicateLess >    m_pKeyCompare;
        bool                                        m_bMutable;
        MapListeners                                m_aModListeners;

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
        MapData& operator=( const MapData& _source );   // not implemented
    };

    //====================================================================
    //= IMapModificationListener
    //====================================================================
    /** implemented by components who want to be notified of modifications in the MapData they work with
    */
    class SAL_NO_VTABLE IMapModificationListener
    {
    public:
        /// called when the map was modified
        virtual void mapModified() = 0;
        virtual ~IMapModificationListener()
        {
        }
    };

    //====================================================================
    //= MapData helpers
    //====================================================================
    //--------------------------------------------------------------------
    static void lcl_registerMapModificationListener( MapData& _mapData, IMapModificationListener& _listener )
    {
    #if OSL_DEBUG_LEVEL > 0
        for (   MapListeners::const_iterator lookup = _mapData.m_aModListeners.begin();
                lookup != _mapData.m_aModListeners.end();
                ++lookup
             )
        {
            OSL_ENSURE( *lookup != &_listener, "lcl_registerMapModificationListener: this listener is already registered!" );
        }
    #endif
        _mapData.m_aModListeners.push_back( &_listener );
    }

    //--------------------------------------------------------------------
    static void lcl_revokeMapModificationListener( MapData& _mapData, IMapModificationListener& _listener )
    {
        for (   MapListeners::iterator lookup = _mapData.m_aModListeners.begin();
                lookup != _mapData.m_aModListeners.end();
                ++lookup
             )
        {
            if ( *lookup == &_listener )
            {
                _mapData.m_aModListeners.erase( lookup );
                return;
            }
        }
        OSL_FAIL( "lcl_revokeMapModificationListener: the listener is not registered!" );
    }

    //--------------------------------------------------------------------
    static void lcl_notifyMapDataListeners_nothrow( const MapData& _mapData )
    {
        for (   MapListeners::const_iterator loop = _mapData.m_aModListeners.begin();
                loop != _mapData.m_aModListeners.end();
                ++loop
            )
        {
            (*loop)->mapModified();
        }
    }

    //====================================================================
    //= EnumerableMap
    //====================================================================
    typedef ::cppu::WeakAggComponentImplHelper3 <   XInitialization
                                                ,   XEnumerableMap
                                                ,   XServiceInfo
                                                > Map_IFace;

    class COMPHELPER_DLLPRIVATE EnumerableMap :public Map_IFace
                                    ,public ComponentBase
    {
    protected:
        EnumerableMap();
        virtual ~EnumerableMap();

        // XInitialization
        virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException);

        // XEnumerableMap
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createKeyEnumeration( ::sal_Bool _Isolated ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createValueEnumeration( ::sal_Bool _Isolated ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createElementEnumeration( ::sal_Bool _Isolated ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // XMap
        virtual Type SAL_CALL getKeyType() throw (RuntimeException);
        virtual Type SAL_CALL getValueType() throw (RuntimeException);
        virtual void SAL_CALL clear(  ) throw (NoSupportException, RuntimeException);
        virtual ::sal_Bool SAL_CALL containsKey( const Any& _key ) throw (IllegalTypeException, IllegalArgumentException, RuntimeException);
        virtual ::sal_Bool SAL_CALL containsValue( const Any& _value ) throw (IllegalTypeException, IllegalArgumentException, RuntimeException);
        virtual Any SAL_CALL get( const Any& _key ) throw (IllegalTypeException, IllegalArgumentException, NoSuchElementException, RuntimeException);
        virtual Any SAL_CALL put( const Any& _key, const Any& _value ) throw (NoSupportException, IllegalTypeException, IllegalArgumentException, RuntimeException);
        virtual Any SAL_CALL remove( const Any& _key ) throw (NoSupportException, IllegalTypeException, IllegalArgumentException, NoSuchElementException, RuntimeException);

        // XElementAccess (base of XMap)
        virtual Type SAL_CALL getElementType() throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements() throw (RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

    public:
        // XServiceInfo, static version (used for component registration)
        static ::rtl::OUString SAL_CALL getImplementationName_static(  );
        static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  );
        static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& );

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

        ::std::vector< ::com::sun::star::uno::WeakReference< XInterface > >
                            m_aDependentComponents;
    };

    //====================================================================
    //= EnumerationType
    //====================================================================
    enum EnumerationType
    {
        eKeys, eValues, eBoth
    };

    //====================================================================
    //= MapEnumerator
    //====================================================================
    class MapEnumerator : public IMapModificationListener
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

        virtual ~MapEnumerator()
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

        // XEnumeration equivalents
        ::sal_Bool hasMoreElements();
        Any nextElement();

        // IMapModificationListener
        virtual void mapModified();

    private:
        ::cppu::OWeakObject&        m_rParent;
        MapData&                    m_rMapData;
        const EnumerationType       m_eType;
        KeyedValues::const_iterator m_mapPos;
        bool                        m_disposed;

    private:
        MapEnumerator();                                    // not implemented
        MapEnumerator( const MapEnumerator& );              // not implemented
        MapEnumerator& operator=( const MapEnumerator& );   // not implemented
    };

    //====================================================================
    //= MapEnumeration
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   XEnumeration
                                    >   MapEnumeration_Base;
    class MapEnumeration :public ComponentBase
                         ,public MapEnumeration_Base
    {
    public:
        MapEnumeration( ::cppu::OWeakObject& _parentMap, MapData& _mapData, ::cppu::OBroadcastHelper& _rBHelper,
                        const EnumerationType _type, const bool _isolated )
            :ComponentBase( _rBHelper, ComponentBase::NoInitializationNeeded() )
            ,m_xKeepMapAlive( _parentMap )
            ,m_pMapDataCopy( _isolated ? new MapData( _mapData ) : NULL )
            ,m_aEnumerator( *this, _isolated ? *m_pMapDataCopy : _mapData, _type )
        {
        }

        // XEnumeration
        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (RuntimeException);
        virtual Any SAL_CALL nextElement(  ) throw (NoSuchElementException, WrappedTargetException, RuntimeException);

    protected:
        virtual ~MapEnumeration()
        {
            acquire();
            {
                ::osl::MutexGuard aGuard( getMutex() );
                m_aEnumerator.dispose();
                m_pMapDataCopy.reset();
            }
        }

    private:
        // sicne we share our mutex with the main map, we need to keep it alive as long as we live
        Reference< XInterface >     m_xKeepMapAlive;
        ::std::auto_ptr< MapData >  m_pMapDataCopy;
        MapEnumerator               m_aEnumerator;
    };

    //====================================================================
    //= EnumerableMap
    //====================================================================
    //--------------------------------------------------------------------
    EnumerableMap::EnumerableMap()
        :Map_IFace( m_aMutex )
        ,ComponentBase( Map_IFace::rBHelper )
    {
    }

    //--------------------------------------------------------------------
    EnumerableMap::~EnumerableMap()
    {
        if ( !impl_isDisposed() )
        {
            acquire();
            dispose();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL EnumerableMap::initialize( const Sequence< Any >& _arguments ) throw (Exception, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this, ComponentMethodGuard::WithoutInit );
        if ( impl_isInitialized_nothrow() )
            throw AlreadyInitializedException();

        sal_Int32 nArgumentCount = _arguments.getLength();
        if ( ( nArgumentCount != 2 ) && ( nArgumentCount != 3 ) )
            throw IllegalArgumentException();

        Type aKeyType, aValueType;
        if ( !( _arguments[0] >>= aKeyType ) )
            throw IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Type expected.")), *this, 1 );
        if ( !( _arguments[1] >>= aValueType ) )
            throw IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Type expected.")), *this, 2 );

        Sequence< Pair< Any, Any > > aInitialValues;
        bool bMutable = true;
        if ( nArgumentCount == 3 )
        {
            if ( !( _arguments[2] >>= aInitialValues ) )
                throw IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[]com.sun.star.beans.Pair<any,any> expected.")), *this, 2 );
            bMutable = false;
        }

        // for the value, anything is allowed, except VOID
        if ( ( aValueType.getTypeClass() == TypeClass_VOID ) || ( aValueType.getTypeClass() == TypeClass_UNKNOWN ) )
            throw IllegalTypeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unsupported value type." ) ), *this );

        // create the comparator for the KeyType, and throw if the type is not supported
        ::std::auto_ptr< IKeyPredicateLess > pComparator( getStandardLessPredicate( aKeyType, NULL ) );
        if ( !pComparator.get() )
            throw IllegalTypeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unsupported key type." ) ), *this );

        // init members
        m_aData.m_aKeyType = aKeyType;
        m_aData.m_aValueType = aValueType;
        m_aData.m_pKeyCompare = pComparator;
        m_aData.m_pValues.reset( new KeyedValues( *m_aData.m_pKeyCompare ) );
        m_aData.m_bMutable = bMutable;

        if ( aInitialValues.getLength() )
            impl_initValues_throw( aInitialValues );

        setInitialized();
    }

    //--------------------------------------------------------------------
    void EnumerableMap::impl_initValues_throw( const Sequence< Pair< Any, Any > >& _initialValues )
    {
        OSL_PRECOND( m_aData.m_pValues.get() && m_aData.m_pValues->empty(), "EnumerableMap::impl_initValues_throw: illegal call!" );
        if ( !m_aData.m_pValues.get() || !m_aData.m_pValues->empty() )
            throw RuntimeException();

        const Pair< Any, Any >* mapping = _initialValues.getConstArray();
        const Pair< Any, Any >* mappingEnd = mapping + _initialValues.getLength();
        Any normalizedValue;
        for ( ; mapping != mappingEnd; ++mapping )
        {
            impl_checkValue_throw( mapping->Second );
            (*m_aData.m_pValues)[ mapping->First ] = mapping->Second;
        }
    }

    //--------------------------------------------------------------------
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
                Any aTypedValue;
                if ( xValue.is() )
                    // XInterface is not-NULL, but is X(ValueType) not-NULL, too?
                    xValue.set( xValue->queryInterface( m_aData.m_aValueType ), UNO_QUERY );
                bValid = xValue.is();
            }
        }
        break;
        case TypeClass_EXCEPTION:
        case TypeClass_STRUCT:
        case TypeClass_UNION:
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
                bValid = ( pValueCompoundTypeDesc != NULL );
            }
        }
        break;
        }

        if ( !bValid )
        {
            ::rtl::OUStringBuffer aMessage;
            aMessage.appendAscii( "Incompatible value type. Found '" );
            aMessage.append( _value.getValueTypeName() );
            aMessage.appendAscii( "', where '" );
            aMessage.append( m_aData.m_aValueType.getTypeName() );
            aMessage.appendAscii( "' (or compatible type) is expected." );
            throw IllegalTypeException( aMessage.makeStringAndClear(), *const_cast< EnumerableMap* >( this ) );
        }

        impl_checkNaN_throw( _value, m_aData.m_aValueType );
    }

    //--------------------------------------------------------------------
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
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NaN (not-a-number) not supported by this implementation." ) ),
                        *const_cast< EnumerableMap* >( this ), 0 );
            // (note that the case of _key not containing a float/double value is handled in the
            // respective IKeyPredicateLess implementation, so there's no need to handle this here.)
        }
    }

    //--------------------------------------------------------------------
    void EnumerableMap::impl_checkKey_throw( const Any& _key ) const
    {
        if ( !_key.hasValue() )
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NULL keys not supported by this implementation." ) ),
                *const_cast< EnumerableMap* >( this ), 0 );

        impl_checkNaN_throw( _key, m_aData.m_aKeyType );
    }

    //--------------------------------------------------------------------
    void EnumerableMap::impl_checkMutable_throw() const
    {
        if ( !m_aData.m_bMutable )
            throw NoSupportException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The map is immutable." ) ),
                    *const_cast< EnumerableMap* >( this ) );
    }

    //--------------------------------------------------------------------
    Reference< XEnumeration > SAL_CALL EnumerableMap::createKeyEnumeration( ::sal_Bool _Isolated ) throw (NoSupportException, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        return new MapEnumeration( *this, m_aData, getBroadcastHelper(), eKeys, _Isolated );
    }

    //--------------------------------------------------------------------
    Reference< XEnumeration > SAL_CALL EnumerableMap::createValueEnumeration( ::sal_Bool _Isolated ) throw (NoSupportException, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        return new MapEnumeration( *this, m_aData, getBroadcastHelper(), eValues, _Isolated );
    }

    //--------------------------------------------------------------------
    Reference< XEnumeration > SAL_CALL EnumerableMap::createElementEnumeration( ::sal_Bool _Isolated ) throw (NoSupportException, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        return new MapEnumeration( *this, m_aData, getBroadcastHelper(), eBoth, _Isolated );
    }

    //--------------------------------------------------------------------
    Type SAL_CALL EnumerableMap::getKeyType() throw (RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        return m_aData.m_aKeyType;
    }

    //--------------------------------------------------------------------
    Type SAL_CALL EnumerableMap::getValueType() throw (RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        return m_aData.m_aValueType;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EnumerableMap::clear(  ) throw (NoSupportException, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkMutable_throw();

        m_aData.m_pValues->clear();

        lcl_notifyMapDataListeners_nothrow( m_aData );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL EnumerableMap::containsKey( const Any& _key ) throw (IllegalTypeException, IllegalArgumentException, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkKey_throw( _key );

        KeyedValues::const_iterator pos = m_aData.m_pValues->find( _key );
        return ( pos != m_aData.m_pValues->end() );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL EnumerableMap::containsValue( const Any& _value ) throw (IllegalTypeException, IllegalArgumentException, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkValue_throw( _value );

        for (   KeyedValues::const_iterator mapping = m_aData.m_pValues->begin();
                mapping != m_aData.m_pValues->end();
                ++mapping
            )
        {
            if ( mapping->second == _value )
                return sal_True;
        }
        return sal_False;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EnumerableMap::get( const Any& _key ) throw (IllegalTypeException, IllegalArgumentException, NoSuchElementException, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        impl_checkKey_throw( _key );

        KeyedValues::const_iterator pos = m_aData.m_pValues->find( _key );
        if ( pos == m_aData.m_pValues->end() )
            throw NoSuchElementException( anyToString( _key ), *this );

        return pos->second;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EnumerableMap::put( const Any& _key, const Any& _value ) throw (NoSupportException, IllegalTypeException, IllegalArgumentException, RuntimeException)
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

    //--------------------------------------------------------------------
    Any SAL_CALL EnumerableMap::remove( const Any& _key ) throw (NoSupportException, IllegalTypeException, IllegalArgumentException, NoSuchElementException, RuntimeException)
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

    //--------------------------------------------------------------------
    Type SAL_CALL EnumerableMap::getElementType() throw (RuntimeException)
    {
        return ::cppu::UnoType< Pair< Any, Any > >::get();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL EnumerableMap::hasElements() throw (RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        return m_aData.m_pValues->empty();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EnumerableMap::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL EnumerableMap::supportsService( const ::rtl::OUString& _serviceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( getSupportedServiceNames() );
        for ( sal_Int32 i=0; i<aServices.getLength(); ++i )
            if ( _serviceName == aServices[i] )
                return sal_True;
        return sal_False;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EnumerableMap::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EnumerableMap::getImplementationName_static(  )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.comphelper.EnumerableMap" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EnumerableMap::getSupportedServiceNames_static(  )
    {
        Sequence< ::rtl::OUString > aServiceNames(1);
        aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.container.EnumerableMap" ) );
        return aServiceNames;
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL EnumerableMap::Create( SAL_UNUSED_PARAMETER const Reference< XComponentContext >& )
    {
        return *new EnumerableMap;
    }

    //====================================================================
    //= MapEnumerator
    //====================================================================
    //--------------------------------------------------------------------
    ::sal_Bool MapEnumerator::hasMoreElements()
    {
        if ( m_disposed )
            throw DisposedException( ::rtl::OUString(), m_rParent );
        return m_mapPos != m_rMapData.m_pValues->end();
    }

    //--------------------------------------------------------------------
    Any MapEnumerator::nextElement()
    {
        if ( m_disposed )
            throw DisposedException( ::rtl::OUString(), m_rParent );
        if ( m_mapPos == m_rMapData.m_pValues->end() )
            throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No more elements." ) ), m_rParent );

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

    //--------------------------------------------------------------------
    void MapEnumerator::mapModified()
    {
        m_disposed = true;
    }

    //====================================================================
    //= MapEnumeration - implementation
    //====================================================================
    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL MapEnumeration::hasMoreElements(  ) throw (RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        return m_aEnumerator.hasMoreElements();
    }

    //--------------------------------------------------------------------
    Any SAL_CALL MapEnumeration::nextElement(  ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ComponentMethodGuard aGuard( *this );
        return m_aEnumerator.nextElement();
    }

//........................................................................
} // namespace comphelper
//........................................................................

void createRegistryInfo_Map()
{
    ::comphelper::module::OAutoRegistration< ::comphelper::EnumerableMap > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
