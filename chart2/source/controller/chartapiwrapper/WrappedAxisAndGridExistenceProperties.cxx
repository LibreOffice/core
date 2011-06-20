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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedAxisAndGridExistenceProperties.hxx"
#include "AxisHelper.hxx"
#include "ChartModelHelper.hxx"
#include "TitleHelper.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

//.............................................................................
namespace chart
{
namespace wrapper
{

class WrappedAxisAndGridExistenceProperty : public WrappedProperty
{
public:
    WrappedAxisAndGridExistenceProperty( bool bAxis, bool bMain, sal_Int32 nDimensionIndex
        , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedAxisAndGridExistenceProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    bool        m_bAxis;
    bool        m_bMain;
    sal_Int32   m_nDimensionIndex;
};

void WrappedAxisAndGridExistenceProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedAxisAndGridExistenceProperty( true, true, 0, spChart2ModelContact ) );//x axis
    rList.push_back( new WrappedAxisAndGridExistenceProperty( true, false, 0, spChart2ModelContact ) );//x secondary axis
    rList.push_back( new WrappedAxisAndGridExistenceProperty( false, true, 0, spChart2ModelContact ) );//x grid
    rList.push_back( new WrappedAxisAndGridExistenceProperty( false, false, 0, spChart2ModelContact ) );//x help grid

    rList.push_back( new WrappedAxisAndGridExistenceProperty( true, true, 1, spChart2ModelContact ) );//y axis
    rList.push_back( new WrappedAxisAndGridExistenceProperty( true, false, 1, spChart2ModelContact ) );//y secondary axis
    rList.push_back( new WrappedAxisAndGridExistenceProperty( false, true, 1, spChart2ModelContact ) );//y grid
    rList.push_back( new WrappedAxisAndGridExistenceProperty( false, false, 1, spChart2ModelContact ) );//y help grid

    rList.push_back( new WrappedAxisAndGridExistenceProperty( true, true, 2, spChart2ModelContact ) );//z axis
    rList.push_back( new WrappedAxisAndGridExistenceProperty( false, true, 2, spChart2ModelContact ) );//z grid
    rList.push_back( new WrappedAxisAndGridExistenceProperty( false, false, 2, spChart2ModelContact ) );//z help grid
}

WrappedAxisAndGridExistenceProperty::WrappedAxisAndGridExistenceProperty( bool bAxis, bool bMain, sal_Int32 nDimensionIndex
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_bAxis( bAxis )
            , m_bMain( bMain )
            , m_nDimensionIndex( nDimensionIndex )
{
    switch( m_nDimensionIndex )
    {
        case 0:
        {
            if( m_bAxis )
            {
                if( m_bMain )
                    m_aOuterName = C2U( "HasXAxis" );
                else
                    m_aOuterName = C2U( "HasSecondaryXAxis" );
            }
            else
            {
                if( m_bMain )
                    m_aOuterName = C2U( "HasXAxisGrid" );
                else
                    m_aOuterName = C2U( "HasXAxisHelpGrid" );
            }
        }
        break;
        case 2:
        {
            if( m_bAxis )
            {
                OSL_ENSURE(m_bMain == true,"there is no secondary z axis at the old api");
                m_bMain = true;
                m_aOuterName = C2U( "HasZAxis" );
            }
            else
            {
                if( m_bMain )
                    m_aOuterName = C2U( "HasZAxisGrid" );
                else
                    m_aOuterName = C2U( "HasZAxisHelpGrid" );
            }
        }
        break;
        default:
        {
            if( m_bAxis )
            {
                if( m_bMain )
                    m_aOuterName = C2U( "HasYAxis" );
                else
                    m_aOuterName = C2U( "HasSecondaryYAxis" );
            }
            else
            {
                if( m_bMain )
                    m_aOuterName = C2U( "HasYAxisGrid" );
                else
                    m_aOuterName = C2U( "HasYAxisHelpGrid" );
            }
        }
        break;
    }
}

WrappedAxisAndGridExistenceProperty::~WrappedAxisAndGridExistenceProperty()
{
}

void WrappedAxisAndGridExistenceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( C2U("Has axis or grid properties require boolean values"), 0, 0 );

    sal_Bool bOldValue = sal_False;
    getPropertyValue( xInnerPropertySet ) >>= bOldValue;

    if( bOldValue == bNewValue )
        return;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( bNewValue )
    {
        if( m_bAxis )
            AxisHelper::showAxis( m_nDimensionIndex, m_bMain, xDiagram, m_spChart2ModelContact->m_xContext );
        else
            AxisHelper::showGrid( m_nDimensionIndex, 0, m_bMain, xDiagram, m_spChart2ModelContact->m_xContext );
    }
    else
    {
        if( m_bAxis )
            AxisHelper::hideAxis( m_nDimensionIndex, m_bMain, xDiagram );
        else
            AxisHelper::hideGrid( m_nDimensionIndex, 0, m_bMain, xDiagram );
    }
}

Any WrappedAxisAndGridExistenceProperty::getPropertyValue( const Reference< beans::XPropertySet >& /* xInnerPropertySet */ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if(m_bAxis)
    {
        sal_Bool bShown = AxisHelper::isAxisShown( m_nDimensionIndex, m_bMain, xDiagram );
        aRet <<= bShown;
    }
    else
    {
        sal_Bool bShown = AxisHelper::isGridShown( m_nDimensionIndex, 0, m_bMain, xDiagram );
        aRet <<= bShown;
    }
    return aRet;
}

Any WrappedAxisAndGridExistenceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

//---------------------------------------------------------------------------------------------------------------

class WrappedAxisTitleExistenceProperty : public WrappedProperty
{
public:
    WrappedAxisTitleExistenceProperty( sal_Int32 nTitleIndex
        , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedAxisTitleExistenceProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    TitleHelper::eTitleType             m_eTitleType;
};

void WrappedAxisTitleExistenceProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedAxisTitleExistenceProperty( 0, spChart2ModelContact ) );//x axis title
    rList.push_back( new WrappedAxisTitleExistenceProperty( 1, spChart2ModelContact ) );//y axis title
    rList.push_back( new WrappedAxisTitleExistenceProperty( 2, spChart2ModelContact ) );//z axis title
    rList.push_back( new WrappedAxisTitleExistenceProperty( 3, spChart2ModelContact ) );//secondary x axis title
    rList.push_back( new WrappedAxisTitleExistenceProperty( 4, spChart2ModelContact ) );//secondary y axis title
}

WrappedAxisTitleExistenceProperty::WrappedAxisTitleExistenceProperty( sal_Int32 nTitleIndex
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_eTitleType( TitleHelper::Y_AXIS_TITLE )
{
    switch( nTitleIndex )
    {
        case 0:
            m_aOuterName = C2U( "HasXAxisTitle" );
            m_eTitleType = TitleHelper::X_AXIS_TITLE;
            break;
        case 2:
            m_aOuterName = C2U( "HasZAxisTitle" );
            m_eTitleType = TitleHelper::Z_AXIS_TITLE;
            break;
        case 3:
            m_aOuterName = C2U( "HasSecondaryXAxisTitle" );
            m_eTitleType = TitleHelper::SECONDARY_X_AXIS_TITLE;
            break;
        case 4:
            m_aOuterName = C2U( "HasSecondaryYAxisTitle" );
            m_eTitleType = TitleHelper::SECONDARY_Y_AXIS_TITLE;
            break;
        default:
            m_aOuterName = C2U( "HasYAxisTitle" );
            m_eTitleType = TitleHelper::Y_AXIS_TITLE;
            break;
    }
}

WrappedAxisTitleExistenceProperty::~WrappedAxisTitleExistenceProperty()
{
}

void WrappedAxisTitleExistenceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( C2U("Has axis or grid properties require boolean values"), 0, 0 );

    sal_Bool bOldValue = sal_False;
    getPropertyValue( xInnerPropertySet ) >>= bOldValue;

    if( bOldValue == bNewValue )
        return;

    if( bNewValue )
    {
        rtl::OUString aTitleText;
        TitleHelper::createTitle(  m_eTitleType, aTitleText
            , m_spChart2ModelContact->getChartModel(), m_spChart2ModelContact->m_xContext );
    }
    else
    {
        TitleHelper::removeTitle( m_eTitleType, m_spChart2ModelContact->getChartModel() );
    }
}

Any WrappedAxisTitleExistenceProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bHasTitle = sal_False;

    Reference< chart2::XTitle > xTitle( TitleHelper::getTitle( m_eTitleType, m_spChart2ModelContact->getChartModel() ) );
    if( xTitle.is() && (TitleHelper::getCompleteString( xTitle ).getLength() != 0) )
        bHasTitle = sal_True;

    Any aRet;
    aRet <<= bHasTitle;
    return aRet;

}

Any WrappedAxisTitleExistenceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Bool( sal_False );
    return aRet;
}

//---------------------------------------------------------------------------------------------------------------

class WrappedAxisLabelExistenceProperty : public WrappedProperty
{
public:
    WrappedAxisLabelExistenceProperty( bool bMain, sal_Int32 nDimensionIndex
        , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedAxisLabelExistenceProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    bool        m_bMain;
    sal_Int32   m_nDimensionIndex;
};

void WrappedAxisLabelExistenceProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedAxisLabelExistenceProperty( true, 0, spChart2ModelContact ) );//x axis
    rList.push_back( new WrappedAxisLabelExistenceProperty( true, 1, spChart2ModelContact ) );//y axis
    rList.push_back( new WrappedAxisLabelExistenceProperty( true, 2, spChart2ModelContact ) );//z axis
    rList.push_back( new WrappedAxisLabelExistenceProperty( false, 0, spChart2ModelContact ) );//secondary x axis
    rList.push_back( new WrappedAxisLabelExistenceProperty( false, 1, spChart2ModelContact ) );//secondary y axis
}

WrappedAxisLabelExistenceProperty::WrappedAxisLabelExistenceProperty( bool bMain, sal_Int32 nDimensionIndex
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_bMain( bMain )
            , m_nDimensionIndex( nDimensionIndex )
{
    switch( m_nDimensionIndex )
    {
        case 0:
            m_aOuterName = m_bMain ? C2U( "HasXAxisDescription" ) : C2U( "HasSecondaryXAxisDescription" );
            break;
        case 2:
            OSL_ENSURE(m_bMain,"there is no description available for a secondary z axis");
            m_aOuterName = C2U( "HasZAxisDescription" );
            break;
        default:
            m_aOuterName = m_bMain ? C2U( "HasYAxisDescription" ) : C2U( "HasSecondaryYAxisDescription" );
            break;
    }
}

WrappedAxisLabelExistenceProperty::~WrappedAxisLabelExistenceProperty()
{
}

void WrappedAxisLabelExistenceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( C2U("Has axis or grid properties require boolean values"), 0, 0 );

    sal_Bool bOldValue = sal_False;
    getPropertyValue( xInnerPropertySet ) >>= bOldValue;

    if( bOldValue == bNewValue )
        return;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< beans::XPropertySet > xProp( AxisHelper::getAxis( m_nDimensionIndex, m_bMain, xDiagram ), uno::UNO_QUERY );
    if( !xProp.is() && bNewValue )
    {
        //create axis if needed
        xProp.set( AxisHelper::createAxis( m_nDimensionIndex, m_bMain, xDiagram, m_spChart2ModelContact->m_xContext ), uno::UNO_QUERY );
        if( xProp.is() )
            xProp->setPropertyValue( C2U( "Show" ), uno::makeAny( sal_False ) );
    }
    if( xProp.is() )
        xProp->setPropertyValue( C2U( "DisplayLabels" ), rOuterValue );
}

Any WrappedAxisLabelExistenceProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< beans::XPropertySet > xProp( AxisHelper::getAxis( m_nDimensionIndex, m_bMain, xDiagram ), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue( C2U( "DisplayLabels" ));
    else
        aRet <<= sal_False;
    return aRet;
}

Any WrappedAxisLabelExistenceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Bool( sal_True );
    return aRet;
}

} //namespace wrapper
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
