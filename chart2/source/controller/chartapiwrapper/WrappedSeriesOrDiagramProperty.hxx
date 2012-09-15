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
#ifndef CHART_WRAPPED_SERIES_OR_DIAGRAM_PROPERTY_HXX
#define CHART_WRAPPED_SERIES_OR_DIAGRAM_PROPERTY_HXX

#include "WrappedProperty.hxx"
#include "Chart2ModelContact.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include <com/sun/star/chart2/XDataSeries.hpp>

#include <boost/shared_ptr.hpp>
#include <vector>

//.............................................................................
namespace chart
{
namespace wrapper
{

enum tSeriesOrDiagramPropertyType
{
    DATA_SERIES,
    DIAGRAM
};

//PROPERTYTYPE is the type of the outer property

template< typename PROPERTYTYPE >
class WrappedSeriesOrDiagramProperty : public WrappedProperty
{
public:
    virtual PROPERTYTYPE getValueFromSeries( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSeriesPropertySet ) const =0;
    virtual void setValueToSeries( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSeriesPropertySet, PROPERTYTYPE aNewValue ) const =0;

    explicit WrappedSeriesOrDiagramProperty( const ::rtl::OUString& rName, const ::com::sun::star::uno::Any& rDefaulValue
        , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact
        , tSeriesOrDiagramPropertyType ePropertyType )
            : WrappedProperty(rName,::rtl::OUString())
            , m_spChart2ModelContact(spChart2ModelContact)
            , m_aOuterValue(rDefaulValue)
            , m_aDefaultValue(rDefaulValue)
            , m_ePropertyType( ePropertyType )
    {
    }
    virtual ~WrappedSeriesOrDiagramProperty() {};

    bool detectInnerValue( PROPERTYTYPE& rValue, bool& rHasAmbiguousValue ) const
    {
        bool bHasDetectableInnerValue = false;
        rHasAmbiguousValue = false;
        if( m_ePropertyType == DIAGRAM &&
            m_spChart2ModelContact.get() )
        {
            ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > > aSeriesVector(
                ::chart::DiagramHelper::getDataSeriesFromDiagram( m_spChart2ModelContact->getChart2Diagram() ) );
            ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >::const_iterator aIter =
                    aSeriesVector.begin();
            for( ; aIter != aSeriesVector.end(); ++aIter )
            {
                PROPERTYTYPE aCurValue = getValueFromSeries( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >::query( *aIter ) );
                if( !bHasDetectableInnerValue )
                    rValue = aCurValue;
                else
                {
                    if( rValue != aCurValue )
                    {
                        rHasAmbiguousValue = true;
                        break;
                    }
                    else
                        rValue = aCurValue;
                }
                bHasDetectableInnerValue = true;
            }
        }
        return bHasDetectableInnerValue;
    }
    void setInnerValue( PROPERTYTYPE aNewValue ) const
    {
        if( m_ePropertyType == DIAGRAM &&
            m_spChart2ModelContact.get() )
        {
            ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > > aSeriesVector(
                ::chart::DiagramHelper::getDataSeriesFromDiagram( m_spChart2ModelContact->getChart2Diagram() ) );
            ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >::const_iterator aIter =
                    aSeriesVector.begin();
            for( ; aIter != aSeriesVector.end(); ++aIter )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xSeriesPropertySet( *aIter, ::com::sun::star::uno::UNO_QUERY );
                if( xSeriesPropertySet.is() )
                {
                    setValueToSeries( xSeriesPropertySet, aNewValue );
                }
            }
        }
    }
    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                    throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        PROPERTYTYPE aNewValue = PROPERTYTYPE();
        if( ! (rOuterValue >>= aNewValue) )
            throw ::com::sun::star::lang::IllegalArgumentException( "statistic property requires different type", 0, 0 );

        if( m_ePropertyType == DIAGRAM )
        {
            m_aOuterValue = rOuterValue;

            bool bHasAmbiguousValue = false;
            PROPERTYTYPE aOldValue = PROPERTYTYPE();
            if( detectInnerValue( aOldValue, bHasAmbiguousValue ) )
            {
                if( bHasAmbiguousValue || aNewValue != aOldValue )
                    setInnerValue( aNewValue );
            }
        }
        else
        {
            setValueToSeries( xInnerPropertySet, aNewValue );
        }
    }

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        if( m_ePropertyType == DIAGRAM )
        {
            bool bHasAmbiguousValue = false;
            PROPERTYTYPE aValue;
            if( detectInnerValue( aValue, bHasAmbiguousValue ) )
            {
                if(bHasAmbiguousValue)
                    m_aOuterValue <<= m_aDefaultValue;
                else
                    m_aOuterValue <<= aValue;
            }
            return m_aOuterValue;
        }
        else
        {
            ::com::sun::star::uno::Any aRet( m_aDefaultValue );
            aRet <<= getValueFromSeries( xInnerPropertySet );
            return aRet;
        }
    }

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& /* xInnerPropertyState */ ) const
                            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        return m_aDefaultValue;
    }

protected:
    ::boost::shared_ptr< Chart2ModelContact >  m_spChart2ModelContact;
    mutable ::com::sun::star::uno::Any         m_aOuterValue;
    ::com::sun::star::uno::Any                 m_aDefaultValue;
    tSeriesOrDiagramPropertyType               m_ePropertyType;
};

} //namespace wrapper
} //namespace chart
//.............................................................................

// CHART_WRAPPED_SERIES_OR_DIAGRAM_PROPERTY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
