/*************************************************************************
 *
 *  $RCSfile: propagg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#define _COMPHELPER_PROPERTY_AGGREGATION_HXX_

#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_STATE_HXX_
#include <comphelper/propstate.hxx>
#endif

#ifndef __SGI_STL_MAP
#include <stl/map>
#endif

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
        sal_Bool    bAggregate;

        OPropertyAccessor(sal_Int32 _nOriginalHandle, sal_Int32 _nPos, sal_Bool _bAggregate)
            :nOriginalHandle(_nOriginalHandle) ,nPos(_nPos) ,bAggregate(_bAggregate) { }
        OPropertyAccessor()
            :nOriginalHandle(-1) ,nPos(-1) ,bAggregate(sal_False) { }

        sal_Bool operator==(const OPropertyAccessor& rOb) const { return nPos == rOb.nPos; }
        sal_Bool operator <(const OPropertyAccessor& rOb) const { return nPos < rOb.nPos; }
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
};

/**
 * used for implementing an cppu::IPropertyArrayHelper for classes
 * aggregating property sets
 */

#define DEFAULT_AGGREGATE_PROPERTY_ID   10000
//------------------------------------------------------------------
class OPropertyArrayAggregationHelper: public cppu::IPropertyArrayHelper
{
    friend class OPropertySetAggregationHelper;
protected:

    staruno::Sequence<starbeans::Property>  m_aProperties;
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
    OPropertyArrayAggregationHelper(const staruno::Sequence<starbeans::Property>& _rProperties,
                                    const staruno::Sequence<starbeans::Property>& _rAggProperties,
                                    IPropertyInfoService* _pInfoService = NULL,
                                    sal_Int32 _nFirstAggregateId = DEFAULT_AGGREGATE_PROPERTY_ID);


    /// inherited from IPropertyArrayHelper
    virtual sal_Bool SAL_CALL fillPropertyMembersByHandle( ::rtl::OUString* _pPropName, sal_Int16* _pAttributes,
                                            sal_Int32 _nHandle) ;

    /// inherited from IPropertyArrayHelper
    virtual staruno::Sequence<starbeans::Property> SAL_CALL getProperties();
    /// inherited from IPropertyArrayHelper
    virtual starbeans::Property SAL_CALL getPropertyByName(const ::rtl::OUString& _rPropertyName)
                                throw(starbeans::UnknownPropertyException);

    /// inherited from IPropertyArrayHelper
    virtual sal_Bool  SAL_CALL hasPropertyByName(const ::rtl::OUString& _rPropertyName) ;
    /// inherited from IPropertyArrayHelper
    virtual sal_Int32 SAL_CALL getHandleByName(const ::rtl::OUString & _rPropertyName);
    /// inherited from IPropertyArrayHelper
    virtual sal_Int32 SAL_CALL fillHandles( /*out*/sal_Int32* _pHandles, const staruno::Sequence< ::rtl::OUString >& _rPropNames );

    /** returns information about a property of the aggregate.
        @param  _pPropName          points to a string to recieve the property name. No name is returned if this is NULL.
        @param  _pOriginalHandle    points to a sal_Int32 to recieve the original property hande. No original handle is returned
                                    if this is NULL.
        @param  _nHandle            the handle of the property as got by, for instance, fillHandles

        @return sal_True, if _nHandle marks an aggregate property, otherwise sal_False
    */
    virtual sal_Bool SAL_CALL fillAggregatePropertyInfoByHandle(::rtl::OUString* _pPropName, sal_Int32* _pOriginalHandle,
                                                   sal_Int32 _nHandle) const;

protected:
    starbeans::Property* findPropertyByName(const ::rtl::OUString& _rName) const;
};

//==================================================================
/**
 * helper class for implementing the property-set-related interfaces
 * for an object doin' aggregation
 * supports at least XPropertySet and XMultiPropertySet
 *
 */

class OPropertySetAggregationHelper :public OPropertyStateHelper
                                    ,public starbeans::XPropertiesChangeListener
                                    ,public starbeans::XVetoableChangeListener
{
protected:
    staruno::Reference<starbeans::XPropertyState>       m_xAggregateState;
    staruno::Reference<starbeans::XPropertySet>         m_xAggregateSet;
    staruno::Reference<starbeans::XMultiPropertySet>    m_xAggregateMultiSet;
    staruno::Reference<starbeans::XFastPropertySet>     m_xAggregateFastSet;

    sal_Bool                    m_bListening : 1;
public:
    OPropertySetAggregationHelper(cppu::OBroadcastHelper& rBHelper)
        :OPropertyStateHelper(rBHelper) ,m_bListening(sal_False) { }

    virtual staruno::Any SAL_CALL queryInterface(const staruno::Type& aType) throw(staruno::RuntimeException);

// XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& Source) throw (staruno::RuntimeException);

// XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const staruno::Any& aValue) throw(starbeans::UnknownPropertyException, starbeans::PropertyVetoException, starlang::IllegalArgumentException, starlang::WrappedTargetException, staruno::RuntimeException);
    virtual staruno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException);

// XPropertySet
    virtual void SAL_CALL           addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const staruno::Reference< starbeans::XPropertyChangeListener >& xListener) throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException);
    virtual void SAL_CALL           addVetoableChangeListener(const ::rtl::OUString& PropertyName, const staruno::Reference< starbeans::XVetoableChangeListener >& aListener) throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException);

// XPropertiesChangeListener
    virtual void SAL_CALL propertiesChange(const staruno::Sequence< starbeans::PropertyChangeEvent >& evt) throw(staruno::RuntimeException);

// XVetoableChangeListener
    virtual void SAL_CALL vetoableChange(const starbeans::PropertyChangeEvent& aEvent) throw(starbeans::PropertyVetoException, staruno::RuntimeException);

// XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues(const staruno::Sequence< ::rtl::OUString >& PropertyNames, const staruno::Sequence< staruno::Any >& Values) throw(starbeans::PropertyVetoException, starlang::IllegalArgumentException, starlang::WrappedTargetException, staruno::RuntimeException);
    virtual void SAL_CALL   addPropertiesChangeListener(const staruno::Sequence< ::rtl::OUString >& aPropertyNames, const staruno::Reference< starbeans::XPropertiesChangeListener >& xListener) throw(staruno::RuntimeException);

// XPropertyState
    virtual starbeans::PropertyState SAL_CALL   getPropertyState(const ::rtl::OUString& PropertyName) throw(starbeans::UnknownPropertyException, staruno::RuntimeException);
    virtual void SAL_CALL                       setPropertyToDefault(const ::rtl::OUString& PropertyName) throw(starbeans::UnknownPropertyException, staruno::RuntimeException);
    virtual staruno::Any SAL_CALL               getPropertyDefault(const ::rtl::OUString& aPropertyName) throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException);

// OPropertySetHelper
    /** still waiting to be overwritten ...
        you <B>must<B/> use an OPropertyArrayAggregationHelper here, as the implementation strongly relies on this.
    */
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() = 0;

protected:

    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL disposing();

    sal_Int32   getOriginalHandle(sal_Int32 nHandle) const;

    /// must be called before aggregation, if aggregation is used
    void setAggregation(const staruno::Reference< staruno::XInterface >&) throw( starlang::IllegalArgumentException );
    void startListening();
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_PROPERTY_AGGREGATION_HXX_

