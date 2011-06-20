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
            for( ; aIter != aSeriesVector.end(); aIter++ )
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
            for( ; aIter != aSeriesVector.end(); aIter++ )
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
            throw ::com::sun::star::lang::IllegalArgumentException( C2U("statistic property requires different type"), 0, 0 );

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
