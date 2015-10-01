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

#ifndef INCLUDED_FORMS_SOURCE_INC_PROPERTY_HXX
#define INCLUDED_FORMS_SOURCE_INC_PROPERTY_HXX

#include <sal/config.h>

#include <vector>

#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <comphelper/property.hxx>
#include <comphelper/propagg.hxx>
#include <tools/debug.hxx>

using namespace comphelper;

//= property helper classes

namespace frm
{

//= assigment property handle <-> property name
//= used by the PropertySetAggregationHelper


class PropertyInfoService
{

    struct PropertyAssignment
    {
        OUString     sName;
        sal_Int32           nHandle;

        PropertyAssignment() { nHandle = -1; }
        PropertyAssignment(const PropertyAssignment& _rSource)
            :sName(_rSource.sName), nHandle(_rSource.nHandle) { }
        PropertyAssignment(const OUString& _rName, sal_Int32 _nHandle)
            :sName(_rName), nHandle(_nHandle) { }

    };

    typedef std::vector<PropertyAssignment> PropertyMap;
    static PropertyMap      s_AllKnownProperties;


    // comparing two PropertyAssignment's
public:
    typedef PropertyAssignment PUBLIC_SOLARIS_COMPILER_HACK;
        // did not get the following compiled under with SUNPRO 5 without this
        // public typedef
private:
    friend struct PropertyAssignmentNameCompareLess;
    typedef ::std::binary_function< PUBLIC_SOLARIS_COMPILER_HACK, PUBLIC_SOLARIS_COMPILER_HACK, sal_Bool > PropertyAssignmentNameCompareLess_Base;
    struct PropertyAssignmentNameCompareLess : public PropertyAssignmentNameCompareLess_Base
    {
        inline bool operator() (const PUBLIC_SOLARIS_COMPILER_HACK& _rL, const PUBLIC_SOLARIS_COMPILER_HACK& _rR) const
        {
            return (_rL.sName.compareTo(_rR.sName) < 0);
        }
    };

public:
    PropertyInfoService() { }

    static sal_Int32            getPropertyId(const OUString& _rName);

private:
    static void initialize();
};


// a class implementing the comphelper::IPropertyInfoService
class ConcreteInfoService : public ::comphelper::IPropertyInfoService
{
public:
    virtual ~ConcreteInfoService() {}

    virtual sal_Int32 getPreferredPropertyId(const OUString& _rName) SAL_OVERRIDE;
};


#define DECL_PROP_IMPL(varname, type) \
    *pProperties++ = css::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, cppu::UnoType<type>::get(),


#define DECL_BOOL_PROP_IMPL(varname) \
    *pProperties++ = css::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, cppu::UnoType<bool>::get(),


#define DECL_IFACE_PROP_IMPL(varname, type) \
    *pProperties++ = css::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, cppu::UnoType<type>::get(),


#define BEGIN_DESCRIBE_PROPERTIES( count, baseclass )   \
    baseclass::describeFixedProperties( _rProps ); \
    sal_Int32 nOldCount = _rProps.getLength(); \
    _rProps.realloc( nOldCount + ( count ) );   \
    css::beans::Property* pProperties = _rProps.getArray() + nOldCount;       \


#define BEGIN_DESCRIBE_BASE_PROPERTIES( count ) \
    _rProps.realloc( count );   \
    css::beans::Property* pProperties = _rProps.getArray();       \


#define BEGIN_DESCRIBE_AGGREGATION_PROPERTIES( count, aggregate )   \
    _rProps.realloc( count );   \
    css::beans::Property* pProperties = _rProps.getArray();       \
    \
    if (aggregate.is()) \
        _rAggregateProps = aggregate->getPropertySetInfo()->getProperties();    \



#define DECL_PROP0(varname, type)   \
    DECL_PROP_IMPL(varname, type) 0)


#define DECL_PROP1(varname, type, attrib1)  \
        DECL_PROP_IMPL(varname, type) css::beans::PropertyAttribute::attrib1)


#define DECL_PROP2(varname, type, attrib1, attrib2) \
        DECL_PROP_IMPL(varname, type) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2)


#define DECL_PROP3(varname, type, attrib1, attrib2, attrib3)    \
        DECL_PROP_IMPL(varname, type) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 | css::beans::PropertyAttribute::attrib3)


#define DECL_PROP4(varname, type, attrib1, attrib2, attrib3, attrib4)   \
        DECL_PROP_IMPL(varname, type) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 | css::beans::PropertyAttribute::attrib3 | css::beans::PropertyAttribute::attrib4)

// === some property types require special handling
// === such as interfaces

#define DECL_IFACE_PROP2(varname, type, attrib1, attrib2)   \
        DECL_IFACE_PROP_IMPL(varname, type) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2)


#define DECL_IFACE_PROP3(varname, type, attrib1, attrib2, attrib3)  \
    DECL_IFACE_PROP_IMPL(varname, type) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 | css::beans::PropertyAttribute::attrib3)


#define DECL_IFACE_PROP4(varname, type, attrib1, attrib2, attrib3, attrib4) \
    DECL_IFACE_PROP_IMPL(varname, type) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 | css::beans::PropertyAttribute::attrib3 | PropertyAttribute::attrib4)

// === or Boolean properties

#define DECL_BOOL_PROP1(varname, attrib1)   \
        DECL_BOOL_PROP_IMPL(varname) css::beans::PropertyAttribute::attrib1)


#define DECL_BOOL_PROP2(varname, attrib1, attrib2)  \
        DECL_BOOL_PROP_IMPL(varname) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2)


#define DECL_BOOL_PROP3( varname, attrib1, attrib2, attrib3 )   \
        DECL_BOOL_PROP_IMPL(varname) css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 | css::beans::PropertyAttribute::attrib3 )



#define END_DESCRIBE_PROPERTIES()   \
    DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?"); \



#define REGISTER_PROP_2( prop, member, attrib1, attrib2 ) \
    registerProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::attrib1 | PropertyAttribute::attrib2, \
        &member, cppu::UnoType<decltype(member)>::get() );

#define REGISTER_PROP_3( prop, member, attrib1, attrib2, attrib3 ) \
    registerProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::attrib1 | PropertyAttribute::attrib2 | PropertyAttribute::attrib3, \
        &member, cppu::UnoType<decltype(member)>::get() );


#define REGISTER_VOID_PROP_2( prop, memberAny, type, attrib1, attrib2 ) \
    registerMayBeVoidProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::MAYBEVOID | PropertyAttribute::attrib1 | PropertyAttribute::attrib2, \
        &memberAny, cppu::UnoType<type>::get() );


}
//... namespace frm .......................................................

#endif // INCLUDED_FORMS_SOURCE_INC_PROPERTY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
