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

#ifndef INCLUDED_COMPHELPER_PROPAGG_HXX
#define INCLUDED_COMPHELPER_PROPAGG_HXX

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <comphelper/propstate.hxx>
#include <comphelper/comphelperdllapi.h>

#include <map>
#include <memory>
#include <vector>


//= property helper classes


namespace comphelper
{


//= OPropertyAccessor
//= internal helper class for OPropertyArrayAggregationHelper

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

    typedef std::map< sal_Int32, OPropertyAccessor >  PropertyAccessorMap;
    typedef PropertyAccessorMap::const_iterator     ConstPropertyAccessorMapIterator;
}


/**
 * used as callback for a OPropertyArrayAggregationHelper
 */
class IPropertyInfoService
{
public:
    /** get the preferred handle for the given property
        @param      _rName      the property name
        @return                 the handle the property should be referred by, or -1 if there are no
                                preferences for the given property
    */
    virtual sal_Int32           getPreferredPropertyId(const OUString& _rName) = 0;

protected:
    ~IPropertyInfoService() {}
};

/**
 * used for implementing an cppu::IPropertyArrayHelper for classes
 * aggregating property sets
 */

#define DEFAULT_AGGREGATE_PROPERTY_ID   10000

class COMPHELPER_DLLPUBLIC OPropertyArrayAggregationHelper final : public ::cppu::IPropertyArrayHelper
{
    friend class OPropertySetAggregationHelper;

    std::vector<css::beans::Property>         m_aProperties;
    internal::PropertyAccessorMap             m_aPropertyAccessors;

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
                                for referring the aggregate's properties from outside.
                                If one of the properties returned from the info service conflict with other handles
                                already present (e.g. through _rProperties), the property is handled as if -1 was returned.
                                If NULL (or, for a special property, a call to getPreferredPropertyId returns -1),
                                the aggregate property(ies) get a new handle which they can be referred by from outside.
        @param  _nFirstAggregateId
                                if the object is about to create new handles for the aggregate properties, it uses
                                id's ascending from this given id.
                                No checks are made if the handle range determined by _nFirstAggregateId conflicts with other
                                handles within _rProperties.
    */
    OPropertyArrayAggregationHelper(const css::uno::Sequence< css::beans::Property>& _rProperties,
                                    const css::uno::Sequence< css::beans::Property>& _rAggProperties,
                                    IPropertyInfoService* _pInfoService = nullptr,
                                    sal_Int32 _nFirstAggregateId = DEFAULT_AGGREGATE_PROPERTY_ID);


    /// inherited from IPropertyArrayHelper
    virtual sal_Bool SAL_CALL fillPropertyMembersByHandle( OUString* _pPropName, sal_Int16* _pAttributes,
                                            sal_Int32 _nHandle) override ;

    /// inherited from IPropertyArrayHelper
    virtual css::uno::Sequence< css::beans::Property> SAL_CALL getProperties() override;
    /// inherited from IPropertyArrayHelper
    virtual css::beans::Property SAL_CALL getPropertyByName(const OUString& _rPropertyName) override;

    /// inherited from IPropertyArrayHelper
    virtual sal_Bool  SAL_CALL hasPropertyByName(const OUString& _rPropertyName) override ;
    /// inherited from IPropertyArrayHelper
    virtual sal_Int32 SAL_CALL getHandleByName(const OUString & _rPropertyName) override;
    /// inherited from IPropertyArrayHelper
    virtual sal_Int32 SAL_CALL fillHandles( /*out*/sal_Int32* _pHandles, const css::uno::Sequence< OUString >& _rPropNames ) override;

    /** returns information about a property of the aggregate.
        @param  _pPropName          points to a string to receive the property name. No name is returned if this is NULL.
        @param  _pOriginalHandle    points to a sal_Int32 to receive the original property handle. No original handle is returned
                                    if this is NULL.
        @param  _nHandle            the handle of the property as got by, for instance, fillHandles

        @return sal_True, if _nHandle marks an aggregate property, otherwise sal_False
    */
    bool fillAggregatePropertyInfoByHandle(OUString* _pPropName, sal_Int32* _pOriginalHandle,
                                                   sal_Int32 _nHandle) const;

    /** returns information about a property given by handle
    */
    bool getPropertyByHandle( sal_Int32 _nHandle, css::beans::Property& _rProperty ) const;


    enum class PropertyOrigin
    {
        Aggregate,
        Delegator,
        Unknown
    };
    /** prefer this one over the XPropertySetInfo of the aggregate!

        <p>The reason is that OPropertyArrayAggregationHelper is the only instance which really knows
        which properties of the aggregate are to be exposed. <br/>

        For instance, some derivee of OPropertySetAggregationHelper may decide to create an
        OPropertyArrayAggregationHelper which contains only a subset of the aggregate properties. This way,
        some of the aggregate properties may be hidden to the public.<br/>

        When using the XPropertySetInfo of the aggregate set to determine the existence of a property, then this
        would return false positives.</p>
    */
    PropertyOrigin  classifyProperty( const OUString& _rName );

private:
    const css::beans::Property* findPropertyByName(const OUString& _rName) const;
};


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
                                    ,public css::beans::XPropertiesChangeListener
                                    ,public css::beans::XVetoableChangeListener
{
    friend class internal::PropertyForwarder;

protected:
    css::uno::Reference< css::beans::XPropertyState>      m_xAggregateState;
    css::uno::Reference< css::beans::XPropertySet>        m_xAggregateSet;
    css::uno::Reference< css::beans::XMultiPropertySet>   m_xAggregateMultiSet;
    css::uno::Reference< css::beans::XFastPropertySet>    m_xAggregateFastSet;

    std::unique_ptr<internal::PropertyForwarder>          m_pForwarder;
    bool                            m_bListening : 1;

public:
    OPropertySetAggregationHelper( ::cppu::OBroadcastHelper& rBHelper );

    virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& aType) override;

// XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

// XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) override;

// XPropertySet
    virtual void SAL_CALL           addPropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener) override;
    virtual void SAL_CALL           addVetoableChangeListener(const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener) override;

// XPropertiesChangeListener
    virtual void SAL_CALL propertiesChange(const css::uno::Sequence< css::beans::PropertyChangeEvent >& evt) override;

// XVetoableChangeListener
    virtual void SAL_CALL vetoableChange(const css::beans::PropertyChangeEvent& aEvent) override;

// XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues(const css::uno::Sequence< OUString >& PropertyNames, const css::uno::Sequence< css::uno::Any >& Values) override;
    virtual void SAL_CALL   addPropertiesChangeListener(const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener) override;

// XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState(const OUString& PropertyName) override;
    virtual void SAL_CALL                                   setPropertyToDefault(const OUString& PropertyName) override;
    virtual css::uno::Any SAL_CALL             getPropertyDefault(const OUString& aPropertyName) override;

// OPropertySetHelper
    /** still waiting to be overwritten ...
        you <B>must<B/> use an OPropertyArrayAggregationHelper here, as the implementation strongly relies on this.
    */
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override = 0;

    /** only implemented for "forwarded" properties, every other property must be handled
        in the derivee, and will assert if passed herein
    */
    virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue ) override;

    /** only implemented for "forwarded" properties, every other property must be handled
        in the derivee, and will assert if passed herein
    */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const css::uno::Any& _rValue ) override;

protected:
    virtual ~OPropertySetAggregationHelper() override;

    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const override;
    void disposing();

    sal_Int32       getOriginalHandle( sal_Int32 _nHandle ) const;
    OUString getPropertyName( sal_Int32 _nHandle ) const;

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
    virtual void forwardingPropertyValue( sal_Int32 _nHandle );

    /** called immediately after a property value which is overwritten in this instance
        has been forwarded to the aggregate

        @see declareForwardedProperty
        @see forwardingPropertyValue
    */
    virtual void forwardedPropertyValue( sal_Int32 _nHandle );

    /// must be called before aggregation, if aggregation is used
    ///
    /// @throws css::lang::IllegalArgumentException
    void setAggregation(const css::uno::Reference< css::uno::XInterface >&);
    void startListening();
};


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROPAGG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
