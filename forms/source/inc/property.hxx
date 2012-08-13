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

#ifndef _FRM_PROPERTY_HXX_
#define _FRM_PROPERTY_HXX_

#include  <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <comphelper/property.hxx>
#include <comphelper/propagg.hxx>
#include <comphelper/stl_types.hxx>

using namespace comphelper;

//=========================================================================
//= property helper classes
//=========================================================================

//... namespace frm .......................................................
namespace frm
{
//.........................................................................

//==================================================================
//= assigment property handle <-> property name
//= used by the PropertySetAggregationHelper
//==================================================================

class PropertyInfoService
{
    //..................................................................
    struct PropertyAssignment
    {
        ::rtl::OUString     sName;
        sal_Int32           nHandle;

        PropertyAssignment() { nHandle = -1; }
        PropertyAssignment(const PropertyAssignment& _rSource)
            :sName(_rSource.sName), nHandle(_rSource.nHandle) { }
        PropertyAssignment(const ::rtl::OUString& _rName, sal_Int32 _nHandle)
            :sName(_rName), nHandle(_nHandle) { }

    };

    DECLARE_STL_VECTOR(PropertyAssignment, PropertyMap);
    static PropertyMap      s_AllKnownProperties;

    //..................................................................
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
        inline sal_Bool operator() (const PUBLIC_SOLARIS_COMPILER_HACK& _rL, const PUBLIC_SOLARIS_COMPILER_HACK& _rR) const
        {
            return (_rL.sName.compareTo(_rR.sName) < 0);
        }
    };

public:
    PropertyInfoService() { }

public:
    static sal_Int32            getPropertyId(const ::rtl::OUString& _rName);
    static ::rtl::OUString      getPropertyName(sal_Int32 _nHandle);

private:
    static void initialize();
};

//..................................................................
// a class implementing the comphelper::IPropertyInfoService
class ConcreteInfoService : public ::comphelper::IPropertyInfoService
{
public:
    virtual ~ConcreteInfoService() {}

    virtual sal_Int32 getPreferedPropertyId(const ::rtl::OUString& _rName);
};

//------------------------------------------------------------------------------
#define DECL_PROP_IMPL(varname, type) \
    *pProperties++ = com::sun::star::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, ::getCppuType(static_cast< type* >(0)),

//------------------------------------------------------------------------------
#define DECL_BOOL_PROP_IMPL(varname) \
    *pProperties++ = com::sun::star::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, ::getBooleanCppuType(),

//------------------------------------------------------------------------------
#define DECL_IFACE_PROP_IMPL(varname, type) \
    *pProperties++ = com::sun::star::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, ::getCppuType(static_cast< com::sun::star::uno::Reference< type >* >(0)),

//------------------------------------------------------------------------------
#define BEGIN_DESCRIBE_PROPERTIES( count, baseclass )   \
    baseclass::describeFixedProperties( _rProps ); \
    sal_Int32 nOldCount = _rProps.getLength(); \
    _rProps.realloc( nOldCount + ( count ) );   \
    ::com::sun::star::beans::Property* pProperties = _rProps.getArray() + nOldCount;       \

//------------------------------------------------------------------------------
#define BEGIN_DESCRIBE_BASE_PROPERTIES( count ) \
    _rProps.realloc( count );   \
    ::com::sun::star::beans::Property* pProperties = _rProps.getArray();       \

//------------------------------------------------------------------------------
#define BEGIN_DESCRIBE_AGGREGATION_PROPERTIES( count, aggregate )   \
    _rProps.realloc( count );   \
    ::com::sun::star::beans::Property* pProperties = _rProps.getArray();       \
    \
    if (aggregate.is()) \
        _rAggregateProps = aggregate->getPropertySetInfo()->getProperties();    \

// ===
//------------------------------------------------------------------------------
#define DECL_PROP0(varname, type)   \
    DECL_PROP_IMPL(varname, type) 0)

//------------------------------------------------------------------------------
#define DECL_PROP1(varname, type, attrib1)  \
        DECL_PROP_IMPL(varname, type) com::sun::star::beans::PropertyAttribute::attrib1)

//------------------------------------------------------------------------------
#define DECL_PROP2(varname, type, attrib1, attrib2) \
        DECL_PROP_IMPL(varname, type) com::sun::star::beans::PropertyAttribute::attrib1 | com::sun::star::beans::PropertyAttribute::attrib2)

//------------------------------------------------------------------------------
#define DECL_PROP3(varname, type, attrib1, attrib2, attrib3)    \
        DECL_PROP_IMPL(varname, type) com::sun::star::beans::PropertyAttribute::attrib1 | com::sun::star::beans::PropertyAttribute::attrib2 | com::sun::star::beans::PropertyAttribute::attrib3)

//------------------------------------------------------------------------------
#define DECL_PROP4(varname, type, attrib1, attrib2, attrib3, attrib4)   \
        DECL_PROP_IMPL(varname, type) com::sun::star::beans::PropertyAttribute::attrib1 | com::sun::star::beans::PropertyAttribute::attrib2 | com::sun::star::beans::PropertyAttribute::attrib3 | com::sun::star::beans::PropertyAttribute::attrib4)

// === some property types require special handling
// === such as interfaces
//------------------------------------------------------------------------------
#define DECL_IFACE_PROP0(varname, type) \
    DECL_IFACE_PROP_IMPL(varname, type) 0)

//------------------------------------------------------------------------------
#define DECL_IFACE_PROP1(varname, type, attrib1)    \
    DECL_IFACE_PROP_IMPL(varname, type) starbeans::PropertyAttribute::attrib1)

//------------------------------------------------------------------------------
#define DECL_IFACE_PROP2(varname, type, attrib1, attrib2)   \
        DECL_IFACE_PROP_IMPL(varname, type) com::sun::star::beans::PropertyAttribute::attrib1 | com::sun::star::beans::PropertyAttribute::attrib2)

//------------------------------------------------------------------------------
#define DECL_IFACE_PROP3(varname, type, attrib1, attrib2, attrib3)  \
    DECL_IFACE_PROP_IMPL(varname, type) starbeans::PropertyAttribute::attrib1 | starbeans::PropertyAttribute::attrib2 | starbeans::PropertyAttribute::attrib3)

//------------------------------------------------------------------------------
#define DECL_IFACE_PROP4(varname, type, attrib1, attrib2, attrib3, attrib4) \
    DECL_IFACE_PROP_IMPL(varname, type) starbeans::PropertyAttribute::attrib1 | starbeans::PropertyAttribute::attrib2 | starbeans::PropertyAttribute::attrib3 | PropertyAttribute::attrib4)

// === or Boolean properties
//------------------------------------------------------------------------------
#define DECL_BOOL_PROP0(varname)    \
    DECL_BOOL_PROP_IMPL(varname) 0)

//------------------------------------------------------------------------------
#define DECL_BOOL_PROP1(varname, attrib1)   \
        DECL_BOOL_PROP_IMPL(varname) com::sun::star::beans::PropertyAttribute::attrib1)

//------------------------------------------------------------------------------
#define DECL_BOOL_PROP2(varname, attrib1, attrib2)  \
        DECL_BOOL_PROP_IMPL(varname) com::sun::star::beans::PropertyAttribute::attrib1 | com::sun::star::beans::PropertyAttribute::attrib2)

//------------------------------------------------------------------------------
#define DECL_BOOL_PROP3( varname, attrib1, attrib2, attrib3 )   \
        DECL_BOOL_PROP_IMPL(varname) com::sun::star::beans::PropertyAttribute::attrib1 | com::sun::star::beans::PropertyAttribute::attrib2 | com::sun::star::beans::PropertyAttribute::attrib3 )

// ===
//------------------------------------------------------------------------------
#define END_DESCRIBE_PROPERTIES()   \
    DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?"); \

//==============================================================================
//------------------------------------------------------------------------------
#define REGISTER_PROP_1( prop, member, attrib1 ) \
    registerProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::attrib1, \
        &member, ::getCppuType( &member ) );

#define REGISTER_PROP_2( prop, member, attrib1, attrib2 ) \
    registerProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::attrib1 | PropertyAttribute::attrib2, \
        &member, ::getCppuType( &member ) );

#define REGISTER_PROP_3( prop, member, attrib1, attrib2, attrib3 ) \
    registerProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::attrib1 | PropertyAttribute::attrib2 | PropertyAttribute::attrib3, \
        &member, ::getCppuType( &member ) );

//------------------------------------------------------------------------------
#define REGISTER_VOID_PROP_1( prop, memberAny, type, attrib1 ) \
    registerMayBeVoidProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::MAYBEVOID | PropertyAttribute::attrib1, \
        &memberAny, ::getCppuType( static_cast< type* >( NULL ) ) );

#define REGISTER_VOID_PROP_2( prop, memberAny, type, attrib1, attrib2 ) \
    registerMayBeVoidProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::MAYBEVOID | PropertyAttribute::attrib1 | PropertyAttribute::attrib2, \
        &memberAny, ::getCppuType( static_cast< type* >( NULL ) ) );

//.........................................................................
}
//... namespace frm .......................................................

#endif // _FRM_PROPERTY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
