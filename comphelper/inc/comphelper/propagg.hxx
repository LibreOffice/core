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

#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#define _COMPHELPER_PROPERTY_AGGREGATION_HXX_

#include <com/sun/star/uno/XAggregation.hpp>
#include <comphelper/propstate.hxx>
#include "comphelper/comphelperdllapi.h"

#include <map>

//=========================================================================
//= property helper classes
//=========================================================================

//.........................................................................
namespace comphelper
{
//.........................................................................

//==================================================================
//= OPropertyAccessor
//= internal helper class for OPropertyArrayAggregationHelper
//==================================================================
namespace internal
{
    struct OPropertyAccessor
    {
        sal_Int32   nOriginalHandle;
        sal_Int32   nPos;
        bool        bAggregate;

        OPropertyAccessor(sal_Int32 _nOriginalHandle, sal_Int32 _nPos, bool _bAggregate)
            :nOriginalHandle(_nOriginalHandle) ,nPos(_nPos) ,bAggregate(_bAggregate) { }
        OPropertyAccessor()
            :nOriginalHandle(-1) ,nPos(-1) ,bAggregate(false) { }

        bool operator==(const OPropertyAccessor& rOb) const { return nPos == rOb.nPos; }
        bool operator <(const OPropertyAccessor& rOb) const { return nPos < rOb.nPos; }
    };

    typedef std::map< sal_Int32, OPropertyAccessor, ::std::less< sal_Int32 > >  PropertyAccessorMap;
    typedef PropertyAccessorMap::iterator           PropertyAccessorMapIterator;
    typedef PropertyAccessorMap::const_iterator     ConstPropertyAccessorMapIterator;
}

//==================================================================
/**
 * used as callback for a OPropertyArrayAggregationHelper
 */
class IPropertyInfoService
{
public:
    /** get the prefered handle for the given property
        @param      _rName      the property name
        @return                 the handle the property should be refered by, or -1 if there are no
                                preferences for the given property
    */
    virtual sal_Int32           getPreferedPropertyId(const ::rtl::OUString& _rName) = 0;

protected:
    ~IPropertyInfoService() {}
};

/**
 * used for implementing an cppu::IPropertyArrayHelper for classes
 * aggregating property sets
 */

#define DEFAULT_AGGREGATE_PROPERTY_ID   10000
//------------------------------------------------------------------
class COMPHELPER_DLLPUBLIC OPropertyArrayAggregationHelper: public ::cppu::IPropertyArrayHelper
{
    friend class OPropertySetAggregationHelper;
protected:

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> m_aProperties;
    internal::PropertyAccessorMap           m_aPropertyAccessors;

public:
    /** construct the object.
        @param  _rProperties    the properties of the object doing the aggregation. These properties
                                are used without any checks, so the caller has to ensure that the names and
                                handles are valid.
        @param  _rAggProperties the properties of the aggregate, usually got via an call to getProperties on the
                                XPropertySetInfo of the aggregate.
                                The names of the properties are used without any checks, so the caller has to ensure
                                that there are no doubles.
                                The handles are stored for later quick access, but the outside-handles the
                                aggregate properties get depend from the following two parameters.
        @param  _pInfoService
                                If not NULL, the object pointed to is used to calc handles which should be used
                                for refering the aggregate's properties from outside.
                                If one of the properties returned from the info service conflict with other handles
                                alread present (e.g. through _rProperties), the property is handled as if -1 was returned.
                                If NULL (or, for a special property, a call to getPreferedPropertyId returns -1),
                                the aggregate property(ies) get a new handle which they can be refered by from outside.
        @param  _nFirstAggregateId
                                if the object is about to create new handles for the aggregate properties, it uses
                                id's ascending from this given id.
                                No checks are made if the handle range determined by _nFirstAggregateId conflicts with other
                                handles within _rProperties.
    */
    OPropertyArrayAggregationHelper(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property>& _rProperties,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property>& _rAggProperties,
                                    IPropertyInfoService* _pInfoService = NULL,
                                    sal_Int32 _nFirstAggregateId = DEFAULT_AGGREGATE_PROPERTY_ID);


    /// inherited from IPropertyArrayHelper
    virtual sal_Bool SAL_CALL fillPropertyMembersByHandle( ::rtl::OUString* _pPropName, sal_Int16* _pAttributes,
                                            sal_Int32 _nHandle) ;

    /// inherited from IPropertyArrayHelper
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> SAL_CALL getProperties();
    /// inherited from IPropertyArrayHelper
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName(const ::rtl::OUString& _rPropertyName)
                                throw(::com::sun::star::beans::UnknownPropertyException);

    /// inherited from IPropertyArrayHelper
    virtual sal_Bool  SAL_CALL hasPropertyByName(const ::rtl::OUString& _rPropertyName) ;
    /// inherited from IPropertyArrayHelper
    virtual sal_Int32 SAL_CALL getHandleByName(const ::rtl::OUString & _rPropertyName);
    /// inherited from IPropertyArrayHelper
    virtual sal_Int32 SAL_CALL fillHandles( /*out*/sal_Int32* _pHandles, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rPropNames );

    /** returns information about a property of the aggregate.
        @param  _pPropName          points to a string to recieve the property name. No name is returned if this is NULL.
        @param  _pOriginalHandle    points to a sal_Int32 to recieve the original property hande. No original handle is returned
                                    if this is NULL.
        @param  _nHandle            the handle of the property as got by, for instance, fillHandles

        @return sal_True, if _nHandle marks an aggregate property, otherwise sal_False
    */
    virtual bool SAL_CALL fillAggregatePropertyInfoByHandle(::rtl::OUString* _pPropName, sal_Int32* _pOriginalHandle,
                                                   sal_Int32 _nHandle) const;

    /** returns information about a property given by handle
    */
    sal_Bool getPropertyByHandle( sal_Int32 _nHandle, ::com::sun::star::beans::Property& _rProperty ) const;


    enum PropertyOrigin
    {
        AGGREGATE_PROPERTY,
        DELEGATOR_PROPERTY,
        UNKNOWN_PROPERTY
    };
    /** prefer this one over the XPropertySetInfo of the aggregate!

        <p>The reason is that OPropertyArrayAggregationHelper is the only instance which really knows
        which properties of the aggregate are to be exposed. <br/>

        For instance, some derivee of OPropertySetAggregationHelper may decide to create an
        OPropertyArrayAggregationHelper which contains only a subset of the aggregate properties. This way,
        some of the aggregate properties may be hidded to the public.<br/>

        When using the XPropertySetInfo of the aggregate set to determine the existence of a property, then this
        would return false positives.</p>
    */
    PropertyOrigin  classifyProperty( const ::rtl::OUString& _rName );

protected:
    const ::com::sun::star::beans::Property* findPropertyByName(const ::rtl::OUString& _rName) const;
};

//==================================================================
namespace internal
{
    class PropertyForwarder;
}

/**
 * helper class for implementing the property-set-related interfaces
 * for an object doin' aggregation
 * supports at least XPropertySet and XMultiPropertySet
 *
 */
class COMPHELPER_DLLPUBLIC OPropertySetAggregationHelper    :public OPropertyStateHelper
                                    ,public ::com::sun::star::beans::XPropertiesChangeListener
                                    ,public ::com::sun::star::beans::XVetoableChangeListener
{
    friend class internal::PropertyForwarder;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState>      m_xAggregateState;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xAggregateSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet>   m_xAggregateMultiSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet>    m_xAggregateFastSet;

    internal::PropertyForwarder*    m_pForwarder;
    sal_Bool                        m_bListening : 1;

public:
    OPropertySetAggregationHelper( ::cppu::OBroadcastHelper& rBHelper );

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type& aType) throw(::com::sun::star::uno::RuntimeException);

// XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException);

// XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// XPropertySet
    virtual void SAL_CALL           addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL           addVetoableChangeListener(const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// XPropertiesChangeListener
    virtual void SAL_CALL propertiesChange(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt) throw(::com::sun::star::uno::RuntimeException);

// XVetoableChangeListener
    virtual void SAL_CALL vetoableChange(const ::com::sun::star::beans::PropertyChangeEvent& aEvent) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

// XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertiesChangeListener(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException);

// XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(const ::rtl::OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL                                   setPropertyToDefault(const ::rtl::OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL             getPropertyDefault(const ::rtl::OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    /** still waiting to be overwritten ...
        you <B>must<B/> use an OPropertyArrayAggregationHelper here, as the implementation strongly relies on this.
    */
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() = 0;

    /** only implemented for "forwarded" properties, every other property must be handled
        in the derivee, and will assert if passed herein
    */
    virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw(::com::sun::star::lang::IllegalArgumentException);

    /** only implemented for "forwarded" properties, every other property must be handled
        in the derivee, and will assert if passed herein
    */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw ( ::com::sun::star::uno::Exception );

protected:
    ~OPropertySetAggregationHelper();

    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL disposing();

    sal_Int32       getOriginalHandle( sal_Int32 _nHandle ) const;
    ::rtl::OUString getPropertyName( sal_Int32 _nHandle ) const;

    /** declares the property with the given (public) handle as one to be forwarded to the aggregate

        Sometimes, you might want to <em>overwrite</em> properties at the aggregate. That is,
        though the aggregate implements this property, and still is to hold the property value,
        you want to do additional handling upon setting the property, but then forward the value
        to the aggregate.

        Use this method to declare such properties.

        When a "forwarded property" is set from outside, the class first calls
        <member>forwardingPropertyValue</member> for any preprocessing, then forwards the property
        value to the aggregate, and then calls <member>forwardedPropertyValue</member>.

        When you declare a property as "forwarded", the class takes care for some multi-threading
        issues, for instance, it won't fire any property change notifications which result from
        forwarding a property value, unless it's safe to do so (i.e. unless our mutex is
        released).

        @see forwardingPropertyValue
        @see forwardedPropertyValue
    */
    void declareForwardedProperty( sal_Int32 _nHandle );

    /** checks whether we're actually forwarding a property value to our aggregate

        @see declareForwardedProperty
        @see forwardingPropertyValue
        @see forwardedPropertyValue
    */
    bool    isCurrentlyForwardingProperty( sal_Int32 _nHandle ) const;

    /** called immediately before a property value which is overwritten in this instance
        is forwarded to the aggregate

        @see declareForwardedProperty
        @see forwardedPropertyValue
    */
    virtual void SAL_CALL forwardingPropertyValue( sal_Int32 _nHandle );

    /** called immediately after a property value which is overwritten in this instance
        has been forwarded to the aggregate

        @see declareForwardedProperty
        @see forwardingPropertyValue
    */
    virtual void SAL_CALL forwardedPropertyValue( sal_Int32 _nHandle, bool _bSuccess );

    /// must be called before aggregation, if aggregation is used
    void setAggregation(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&) throw( ::com::sun::star::lang::IllegalArgumentException );
    void startListening();
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_PROPERTY_AGGREGATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
