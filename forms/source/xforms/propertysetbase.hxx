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
#ifndef _PROPERTYSETBASE_HXX
#define _PROPERTYSETBASE_HXX

#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <comphelper/propstate.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/proparrhlp.hxx>
#include <rtl/ref.hxx>

// include for inlined helper function below
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <map>

// forward declarations for method arguments
namespace com { namespace sun { namespace star { namespace uno {
        class Any;
        class RuntimeException;
        template<class T> class Sequence;
} } } }

/** base class which encapsulates accessing (reading/writing) concrete property values
*/
class PropertyAccessorBase : public ::rtl::IReference
{
private:
    oslInterlockedCount m_refCount;

protected:
    PropertyAccessorBase() : m_refCount( 0 ) { }
    virtual ~PropertyAccessorBase();

public:
    virtual oslInterlockedCount SAL_CALL acquire();
    virtual oslInterlockedCount SAL_CALL release();

    virtual bool    approveValue( const com::sun::star::uno::Any& rValue ) const = 0;
    virtual void    setValue( const com::sun::star::uno::Any& rValue ) = 0;
    virtual void    getValue( com::sun::star::uno::Any& rValue ) const = 0;
    virtual bool    isWriteable() const = 0;
};


/** helper class for implementing property accessors through public member functions
*/
template< typename CLASS, typename VALUE, class WRITER, class READER >
class GenericPropertyAccessor : public PropertyAccessorBase
{
public:
    typedef WRITER  Writer;
    typedef READER  Reader;

private:
    CLASS*      m_pInstance;
    Writer      m_pWriter;
    Reader      m_pReader;

public:
    GenericPropertyAccessor( CLASS* pInstance, Writer pWriter, Reader pReader )
        :m_pInstance( pInstance )
        ,m_pWriter( pWriter )
        ,m_pReader( pReader )
    {
    }

    virtual bool    approveValue( const com::sun::star::uno::Any& rValue ) const
    {
        VALUE aVal;
        return ( rValue >>= aVal );
    }

    virtual void    setValue( const com::sun::star::uno::Any& rValue )
    {
        VALUE aTypedVal = VALUE();
        OSL_VERIFY( rValue >>= aTypedVal );
        (m_pInstance->*m_pWriter)( aTypedVal );
    }

    virtual void getValue( com::sun::star::uno::Any& rValue ) const
    {
        rValue = com::sun::star::uno::makeAny( (m_pInstance->*m_pReader)() );
    }

    virtual bool isWriteable() const
    {
        return m_pWriter != 0;
    }
};

/** helper class for implementing property accessors via non-UNO methods
*/
template< typename CLASS, typename VALUE >
class DirectPropertyAccessor
    :public GenericPropertyAccessor < CLASS
                                , VALUE
                                , void (CLASS::*)( const VALUE& )
                                , VALUE (CLASS::*)() const
                                >
{
protected:
    typedef void (CLASS::*Writer)( const VALUE& );
    typedef VALUE (CLASS::*Reader)() const;
public:
    DirectPropertyAccessor( CLASS* pInstance, Writer pWriter, Reader pReader )
        :GenericPropertyAccessor< CLASS, VALUE, Writer, Reader >( pInstance, pWriter, pReader )
    {
    }
};

/** helper class for implementing non-UNO accessors to a boolean property
*/
template< typename CLASS, typename DUMMY >
class BooleanPropertyAccessor
    :public GenericPropertyAccessor < CLASS
                                , bool
                                , void (CLASS::*)( bool )
                                , bool (CLASS::*)() const
                                >
{
protected:
    typedef void (CLASS::*Writer)( bool );
    typedef bool (CLASS::*Reader)() const;
public:
    BooleanPropertyAccessor( CLASS* pInstance, Writer pWriter, Reader pReader )
        :GenericPropertyAccessor< CLASS, bool, Writer, Reader >( pInstance, pWriter, pReader )
    {
    }
};

/** helper class for implementing property accessors via UNO methods
*/
template< typename CLASS, typename VALUE >
class APIPropertyAccessor
    :public GenericPropertyAccessor < CLASS
                                    , VALUE
                                    , void (SAL_CALL CLASS::*)( const VALUE& )
                                    , VALUE (SAL_CALL CLASS::*)()
                                    >
{
protected:
    typedef void (SAL_CALL CLASS::*Writer)( const VALUE& );
    typedef VALUE (SAL_CALL CLASS::*Reader)();
public:
    APIPropertyAccessor( CLASS* pInstance, Writer pWriter, Reader pReader )
        :GenericPropertyAccessor< CLASS, VALUE, Writer, Reader >( pInstance, pWriter, pReader )
    {
    }
};

/** bridges two XPropertySet helper implementations

    The <type scope="comphelper">OStatefulPropertySet</type> (basically, the
    <type scope="cppu">OPropertySetHelper</type>) implements a comprehensive framework
    for property sets, including property change notifications.
    However, it lacks some easy possibilities to declare the supported properties.
    Other helper structs and classes allow for this, but are lacking needed features
    such as property change notifications.

    The <type>PropertySetBase</type> bridges various implementations,
    so you have the best of both worlds.
 */
class PropertySetBase : public ::comphelper::OStatefulPropertySet
{
private:
    typedef com::sun::star::uno::Any    Any_t;

    typedef ::std::map< const sal_Int32, ::rtl::Reference< PropertyAccessorBase > >     PropertyAccessors;
    typedef ::std::vector< ::com::sun::star::beans::Property >                          PropertyArray;
    typedef ::std::map< const sal_Int32, Any_t >                                        PropertyValueCache;

    PropertyArray                   m_aProperties;
    cppu::IPropertyArrayHelper*     m_pProperties;
    PropertyAccessors               m_aAccessors;
    PropertyValueCache              m_aCache;

protected:
    PropertySetBase();
    virtual ~PropertySetBase();

    /** registers a new property to be supported by this instance
        @param rProperty
            the descriptor for the to-be-supported property
        @param rAccessor
            an instance which is able to provide read and possibly write access to
            the property.
        @precond
            Must not be called after any of the property set related UNO interfaces
            has been used. Usually, you will do a number of <member>registerProperty</member>
            calls in the constructor of your class.
    */
    void registerProperty(
        const com::sun::star::beans::Property& rProperty,
        const ::rtl::Reference< PropertyAccessorBase >& rAccessor
    );

    /** notifies a change in a given property value, if necessary

        The necessity of the notification is determined by a cached value for the given
        property. Caching happens after notification.

        That is, when you call <member>notifyAndCachePropertyValue</member> for the first time,
        a value for the given property is default constructed, and considered to be the "old value".
        If this value differs from the current value, then this change is notified to all interested
        listeners. Finally, the current value is remembered.

        Subsequent calls to <member>notifyAndCachePropertyValue</member> use the remembered value as
        "old value", and from then on behave as the first call.

        @param nHandle
            the handle of the property. Must denote a property supported by this instance, i.e.
            one previously registered via <member>registerProperty</member>.

        @precond
            our ref count must not be 0. The reason is that during this method's execution,
            the instance might be acquired and released, which would immediately destroy
            the instance if it has a ref count of 0.

        @seealso initializePropertyValueCache
    */
    void notifyAndCachePropertyValue( sal_Int32 nHandle );

    /** initializes the property value cache for the given property, with its current value

        Usually used to initialize the cache with values which are different from default
        constructed values. Say you have a boolean property whose initial state
        is <TRUE/>. Say you call <member>notifyAndCachePropertyValue</member> the first time: it will
        default construct the "old value" for this property as <FALSE/>, and thus <b>not</b> do
        any notifications if the "current value" is also <FALSE/> - which might be wrong, since
        the guessing of the "old value" differed from the real initial value which was <TRUE/>.

        Too confusing? Okay, than just call this method for every property you have.

        @param nHandle
            the handle of the property. Must denote a property supported by this instance, i.e.
            one previously registered via <member>registerProperty</member>.
        @param rValue
            the value to cache
        @seealso notifyAndCachePropertyValue
    */
    void initializePropertyValueCache( sal_Int32 nHandle );

    /// OPropertysetHelper methods
    virtual sal_Bool SAL_CALL convertFastPropertyValue( Any_t& rConvertedValue, Any_t& rOldValue, sal_Int32 nHandle, const Any_t& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any_t& rValue )
        throw (::com::sun::star::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue( Any_t& rValue, sal_Int32 nHandle ) const;

    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

public:
    /// helper struct for granting selective access to some notification-related methods
    struct NotifierAccess { friend struct PropertyChangeNotifier; private: NotifierAccess() { } };
    /** retrieves the current property value for the given handle
        @param nHandle
            the handle of the property. Must denote a property supported by this instance, i.e.
            one previously registered via <member>registerProperty</member>.
        @see registerProperty
    */
    inline void getCurrentPropertyValueByHandle( sal_Int32 nHandle, Any_t& /* [out] */ rValue, const NotifierAccess& ) const
    {
        getFastPropertyValue( rValue, nHandle );
    }

    /** notifies a change in a given property to all interested listeners
    */
    inline void notifyPropertyChange( sal_Int32 nHandle, const Any_t& rOldValue, const Any_t& rNewValue, const NotifierAccess& ) const
    {
        const_cast< PropertySetBase* >( this )->firePropertyChange( nHandle, rNewValue, rOldValue );
    }

    using ::comphelper::OStatefulPropertySet::getFastPropertyValue;

private:
    /** locates a property given by handle
        @param nHandle
            the handle of the property. Must denote a property supported by this instance, i.e.
            one previously registered via <member>registerProperty</member>.
        @see registerProperty
    */
    PropertyAccessorBase& locatePropertyHandler( sal_Int32 nHandle ) const;
};

/** a helper class for notifying property changes in a <type>PropertySetBase</type> instance.

    You can create an instance of this class on the stack of a method which is to programmatically
    change the value of a property. In its constructor, the instance will acquire the current property
    value, and in its destructor, it will notify the change of this property's value (if necessary).

    You do not need this class if you are modifying property values by using the X(Fast|Multi)PropertSet
    methods, since those already care for property notifications. You only need it if you're changing
    the internal representation of your property directly.

    Also note that usually, notifications in the UNO world should be done without a locked mutex. So
    if you use this class in conjunction with a <type>MutexGuard</type>, ensure that you <b>first</b>
    instantiate the <type>PropertyChangeNotifier</type>, and <b>then</b> the <type>MutexGuard</type>,
    so your mutex is released before the notification happens.
*/
struct PropertyChangeNotifier
{
private:
    const PropertySetBase&      m_rPropertySet;
    sal_Int32                   m_nHandle;
    com::sun::star::uno::Any    m_aOldValue;

public:
    /** constructs a PropertyChangeNotifier
        @param rPropertySet
            the property set implementation whose property is going to be changed. Note
            that this property set implementation must live at least as long as the
            PropertyChangeNotifier instance does.
        @param nHandle
            the handle of the property which is going to be changed. Must be a valid property
            handle for the given <arg>rPropertySet</arg>
    */
    inline PropertyChangeNotifier( const PropertySetBase& rPropertySet, sal_Int32 nHandle )
        :m_rPropertySet( rPropertySet )
        ,m_nHandle( nHandle )
    {
        m_rPropertySet.getCurrentPropertyValueByHandle( m_nHandle, m_aOldValue, PropertySetBase::NotifierAccess() );
    }
    inline ~PropertyChangeNotifier()
    {
        com::sun::star::uno::Any aNewValue;
        m_rPropertySet.getCurrentPropertyValueByHandle( m_nHandle, aNewValue, PropertySetBase::NotifierAccess() );
        if ( aNewValue != m_aOldValue )
        {
            m_rPropertySet.notifyPropertyChange( m_nHandle, m_aOldValue, aNewValue, PropertySetBase::NotifierAccess() );
        }
    }
};


#define PROPERTY_FLAGS( NAME, TYPE, FLAG ) com::sun::star::beans::Property( \
    ::rtl::OUString( #NAME, sizeof( #NAME ) - 1, RTL_TEXTENCODING_ASCII_US ), \
    HANDLE_##NAME, getCppuType( static_cast< TYPE* >( NULL ) ), FLAG )
#define PROPERTY( NAME, TYPE )      PROPERTY_FLAGS( NAME, TYPE, com::sun::star::beans::PropertyAttribute::BOUND )
#define PROPERTY_RO( NAME, TYPE )   PROPERTY_FLAGS( NAME, TYPE, com::sun::star::beans::PropertyAttribute::BOUND | com::sun::star::beans::PropertyAttribute::READONLY )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
