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
#ifndef CHART_WRAPPED_GAPWIDTH_PROPERTY_HXX
#define CHART_WRAPPED_GAPWIDTH_PROPERTY_HXX

#include "WrappedDefaultProperty.hxx"
#include "Chart2ModelContact.hxx"

#include <boost/shared_ptr.hpp>

namespace chart
{
namespace wrapper
{

class WrappedBarPositionProperty_Base : public WrappedDefaultProperty
{
public:
    WrappedBarPositionProperty_Base(
              const ::rtl::OUString& rOuterName
            , const ::rtl::OUString& rInnerSequencePropertyName
            , sal_Int32 nDefaultValue
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedBarPositionProperty_Base();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    void setDimensionAndAxisIndex( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

protected:
    sal_Int32 m_nDimensionIndex;
    sal_Int32 m_nAxisIndex;
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;

    sal_Int32                                   m_nDefaultValue;
    ::rtl::OUString                             m_InnerSequencePropertyName;

    mutable ::com::sun::star::uno::Any          m_aOuterValue;
};

class WrappedGapwidthProperty : public WrappedBarPositionProperty_Base
{
public:
    WrappedGapwidthProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedGapwidthProperty();
};

class WrappedBarOverlapProperty : public WrappedBarPositionProperty_Base
{
public:
    WrappedBarOverlapProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedBarOverlapProperty();
};

} //  namespace wrapper
} //  namespace chart

// CHART_WRAPPED_GAPWIDTH_PROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
