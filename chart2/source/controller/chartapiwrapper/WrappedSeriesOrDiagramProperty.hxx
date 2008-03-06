/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedSeriesOrDiagramProperty.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:17:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART_WRAPPED_SERIES_OR_DIAGRAM_PROPERTY_HXX
#define CHART_WRAPPED_SERIES_OR_DIAGRAM_PROPERTY_HXX

#include "WrappedProperty.hxx"
#include "Chart2ModelContact.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif

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
