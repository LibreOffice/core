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

#include "WrappedAxisAndGridExistenceProperties.hxx"
#include "AxisHelper.hxx"
#include "ChartModelHelper.hxx"
#include "TitleHelper.hxx"
#include "macros.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;

namespace chart
{
namespace wrapper
{

class WrappedAxisAndGridExistenceProperty : public WrappedProperty
{
public:
    WrappedAxisAndGridExistenceProperty( bool bAxis, bool bMain, sal_Int32 nDimensionIndex
        , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact );

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    bool        m_bAxis;
    bool        m_bMain;
    sal_Int32   m_nDimensionIndex;
};

void WrappedAxisAndGridExistenceProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
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
                , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact )
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
                    m_aOuterName = "HasXAxis";
                else
                    m_aOuterName = "HasSecondaryXAxis";
            }
            else
            {
                if( m_bMain )
                    m_aOuterName = "HasXAxisGrid";
                else
                    m_aOuterName = "HasXAxisHelpGrid";
            }
        }
        break;
        case 2:
        {
            if( m_bAxis )
            {
                OSL_ENSURE(m_bMain,"there is no secondary z axis at the old api");
                m_bMain = true;
                m_aOuterName = "HasZAxis";
            }
            else
            {
                if( m_bMain )
                    m_aOuterName = "HasZAxisGrid";
                else
                    m_aOuterName = "HasZAxisHelpGrid";
            }
        }
        break;
        default:
        {
            if( m_bAxis )
            {
                if( m_bMain )
                    m_aOuterName = "HasYAxis";
                else
                    m_aOuterName = "HasSecondaryYAxis";
            }
            else
            {
                if( m_bMain )
                    m_aOuterName = "HasYAxisGrid";
                else
                    m_aOuterName = "HasYAxisHelpGrid";
            }
        }
        break;
    }
}

void WrappedAxisAndGridExistenceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( "Has axis or grid properties require boolean values", nullptr, 0 );

    bool bOldValue = false;
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
{
    Any aRet;
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if(m_bAxis)
    {
        bool bShown = AxisHelper::isAxisShown( m_nDimensionIndex, m_bMain, xDiagram );
        aRet <<= bShown;
    }
    else
    {
        bool bShown = AxisHelper::isGridShown( m_nDimensionIndex, 0, m_bMain, xDiagram );
        aRet <<= bShown;
    }
    return aRet;
}

Any WrappedAxisAndGridExistenceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

class WrappedAxisTitleExistenceProperty : public WrappedProperty
{
public:
    WrappedAxisTitleExistenceProperty( sal_Int32 nTitleIndex
        , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact );

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    TitleHelper::eTitleType             m_eTitleType;
};

void WrappedAxisTitleExistenceProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    rList.push_back( new WrappedAxisTitleExistenceProperty( 0, spChart2ModelContact ) );//x axis title
    rList.push_back( new WrappedAxisTitleExistenceProperty( 1, spChart2ModelContact ) );//y axis title
    rList.push_back( new WrappedAxisTitleExistenceProperty( 2, spChart2ModelContact ) );//z axis title
    rList.push_back( new WrappedAxisTitleExistenceProperty( 3, spChart2ModelContact ) );//secondary x axis title
    rList.push_back( new WrappedAxisTitleExistenceProperty( 4, spChart2ModelContact ) );//secondary y axis title
}

WrappedAxisTitleExistenceProperty::WrappedAxisTitleExistenceProperty(sal_Int32 nTitleIndex
                , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_eTitleType( TitleHelper::Y_AXIS_TITLE )
{
    switch( nTitleIndex )
    {
        case 0:
            m_aOuterName = "HasXAxisTitle";
            m_eTitleType = TitleHelper::X_AXIS_TITLE;
            break;
        case 2:
            m_aOuterName = "HasZAxisTitle";
            m_eTitleType = TitleHelper::Z_AXIS_TITLE;
            break;
        case 3:
            m_aOuterName = "HasSecondaryXAxisTitle";
            m_eTitleType = TitleHelper::SECONDARY_X_AXIS_TITLE;
            break;
        case 4:
            m_aOuterName = "HasSecondaryYAxisTitle";
            m_eTitleType = TitleHelper::SECONDARY_Y_AXIS_TITLE;
            break;
        default:
            m_aOuterName = "HasYAxisTitle";
            m_eTitleType = TitleHelper::Y_AXIS_TITLE;
            break;
    }
}

void WrappedAxisTitleExistenceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( "Has axis or grid properties require boolean values", nullptr, 0 );

    bool bOldValue = false;
    getPropertyValue( xInnerPropertySet ) >>= bOldValue;

    if( bOldValue == bNewValue )
        return;

    if( bNewValue )
    {
        OUString aTitleText;
        TitleHelper::createTitle(  m_eTitleType, aTitleText
            , m_spChart2ModelContact->getChartModel(), m_spChart2ModelContact->m_xContext );
    }
    else
    {
        TitleHelper::removeTitle( m_eTitleType, m_spChart2ModelContact->getChartModel() );
    }
}

Any WrappedAxisTitleExistenceProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bHasTitle = false;

    Reference< chart2::XTitle > xTitle( TitleHelper::getTitle( m_eTitleType, m_spChart2ModelContact->getChartModel() ) );
    if( xTitle.is() && !TitleHelper::getCompleteString( xTitle ).isEmpty() )
        bHasTitle = true;

    Any aRet;
    aRet <<= bHasTitle;
    return aRet;

}

Any WrappedAxisTitleExistenceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

class WrappedAxisLabelExistenceProperty : public WrappedProperty
{
public:
    WrappedAxisLabelExistenceProperty( bool bMain, sal_Int32 nDimensionIndex
        , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact );

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    bool        m_bMain;
    sal_Int32   m_nDimensionIndex;
};

void WrappedAxisLabelExistenceProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    rList.push_back( new WrappedAxisLabelExistenceProperty( true, 0, spChart2ModelContact ) );//x axis
    rList.push_back( new WrappedAxisLabelExistenceProperty( true, 1, spChart2ModelContact ) );//y axis
    rList.push_back( new WrappedAxisLabelExistenceProperty( true, 2, spChart2ModelContact ) );//z axis
    rList.push_back( new WrappedAxisLabelExistenceProperty( false, 0, spChart2ModelContact ) );//secondary x axis
    rList.push_back( new WrappedAxisLabelExistenceProperty( false, 1, spChart2ModelContact ) );//secondary y axis
}

WrappedAxisLabelExistenceProperty::WrappedAxisLabelExistenceProperty(bool bMain, sal_Int32 nDimensionIndex
                , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_bMain( bMain )
            , m_nDimensionIndex( nDimensionIndex )
{
    switch( m_nDimensionIndex )
    {
        case 0:
            m_bMain ? m_aOuterName = "HasXAxisDescription" : m_aOuterName = "HasSecondaryXAxisDescription";
            break;
        case 2:
            OSL_ENSURE(m_bMain,"there is no description available for a secondary z axis");
            m_aOuterName = "HasZAxisDescription";
            break;
        default:
            m_bMain ? m_aOuterName = "HasYAxisDescription" : m_aOuterName = "HasSecondaryYAxisDescription";
            break;
    }
}

void WrappedAxisLabelExistenceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( "Has axis or grid properties require boolean values", nullptr, 0 );

    bool bOldValue = false;
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
            xProp->setPropertyValue( "Show", uno::Any( false ) );
    }
    if( xProp.is() )
        xProp->setPropertyValue( "DisplayLabels", rOuterValue );
}

Any WrappedAxisLabelExistenceProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Any aRet;
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< beans::XPropertySet > xProp( AxisHelper::getAxis( m_nDimensionIndex, m_bMain, xDiagram ), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue( "DisplayLabels" );
    else
        aRet <<= false;
    return aRet;
}

Any WrappedAxisLabelExistenceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= true;
    return aRet;
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
