/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WrappedSceneProperty.cxx,v $
 * $Revision: 1.3 $
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

#include "WrappedSceneProperty.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "BaseGFXHelper.hxx"

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

//static
void WrappedSceneProperty::addWrappedProperties( std::vector< WrappedProperty* >& rList
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedD3DTransformMatrixProperty( spChart2ModelContact ) );
    /*
    rList.push_back( new WrappedD3DSceneShadeModeProperty() );
    rList.push_back( new WrappedD3DScenePerspectiveProperty() );
    rList.push_back( new WrappedD3DSceneAmbientColorProperty() );
    for( sal_Int32 nN=1; nN<=8; nN++ )
    {
        rList.push_back( new WrappedD3DSceneLightColorProperty(nN) );
        rList.push_back( new WrappedD3DSceneLightDirectionProperty(nN) );
        rList.push_back( new WrappedD3DSceneLightOnProperty(nN) );
    }

    rList.push_back( new WrappedD3DSceneDistanceProperty() );
    rList.push_back( new WrappedD3DSceneFocalLengthProperty() );
    rList.push_back( new WrappedD3DSceneShadowSlantProperty() );
    rList.push_back( new WrappedD3DSceneTwoSidedLightingProperty() );
    rList.push_back( new WrappedD3DCameraGeometryProperty() );
    */
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DTransformMatrixProperty::WrappedD3DTransformMatrixProperty(
            ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(C2U("D3DTransformMatrix"),C2U("D3DTransformMatrix"))
            , m_spChart2ModelContact( spChart2ModelContact )
{
}

WrappedD3DTransformMatrixProperty::~WrappedD3DTransformMatrixProperty()
{
}

void WrappedD3DTransformMatrixProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( DiagramHelper::isPieOrDonutChart( m_spChart2ModelContact->getChart2Diagram() ) )
    {
        drawing::HomogenMatrix aHM;
        if( rOuterValue >>= aHM )
        {
            ::basegfx::B3DTuple aRotation( BaseGFXHelper::GetRotationFromMatrix(
                BaseGFXHelper::HomogenMatrixToB3DHomMatrix( aHM ) ) );

            ::basegfx::B3DHomMatrix aMatrix;
            aMatrix.rotate( aRotation.getX(), aRotation.getY(), aRotation.getZ() );
            ::basegfx::B3DHomMatrix aObjectMatrix;
            ::basegfx::B3DHomMatrix aNewMatrix = aMatrix*aObjectMatrix;

            aHM = BaseGFXHelper::B3DHomMatrixToHomogenMatrix(aNewMatrix);

            WrappedProperty::setPropertyValue( uno::makeAny(aHM), xInnerPropertySet );
            return;
        }
    }

    WrappedProperty::setPropertyValue( rOuterValue, xInnerPropertySet );
}

Any WrappedD3DTransformMatrixProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( DiagramHelper::isPieOrDonutChart( m_spChart2ModelContact->getChart2Diagram() ) )
    {
        uno::Any aAMatrix( WrappedProperty::getPropertyValue( xInnerPropertySet ) );
        drawing::HomogenMatrix aHM;
        if( aAMatrix >>= aHM )
        {
            ::basegfx::B3DTuple aRotation( BaseGFXHelper::GetRotationFromMatrix(
                BaseGFXHelper::HomogenMatrixToB3DHomMatrix( aHM ) ) );

            ::basegfx::B3DHomMatrix aMatrix;
            aMatrix.rotate( aRotation.getX(), aRotation.getY(), aRotation.getZ() );
            ::basegfx::B3DHomMatrix aObjectMatrix;
            ::basegfx::B3DHomMatrix aNewMatrix = aMatrix*aObjectMatrix;

            aHM = BaseGFXHelper::B3DHomMatrixToHomogenMatrix(aNewMatrix);

            return uno::makeAny(aHM);
        }
    }

    return WrappedProperty::getPropertyValue( xInnerPropertySet );
}

Any WrappedD3DTransformMatrixProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return WrappedProperty::getPropertyDefault( xInnerPropertyState );
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
/*
WrappedD3DSceneShadeModeProperty::WrappedD3DSceneShadeModeProperty()
            : WrappedProperty(C2U("D3DSceneShadeMode"),OUString())
{
}

WrappedD3DSceneShadeModeProperty::~WrappedD3DSceneShadeModeProperty()
{
}

void WrappedD3DSceneShadeModeProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    drawing::ShadeMode aNewValue;
    if( !(rOuterValue >>= aNewValue) )
        throw lang::IllegalArgumentException( C2U("Property D3DSceneShadeMode requires value of type drawing::ShadeMode"), 0, 0 );

    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneShadeMode" );
        return;
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;

    if( aSceneDescriptor.aShadeMode != aNewValue  )
    {
        aSceneDescriptor.aShadeMode = aNewValue;
        xInnerPropertySet->setPropertyValue( C2U( "SceneProperties" ), uno::makeAny( aSceneDescriptor ) );
    }
}

Any WrappedD3DSceneShadeModeProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DTransformMatrix" );
        return Any();
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;
    return uno::makeAny( aSceneDescriptor.aShadeMode );
}

Any WrappedD3DSceneShadeModeProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( drawing::ShadeMode_SMOOTH );
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DScenePerspectiveProperty::WrappedD3DScenePerspectiveProperty()
            : WrappedProperty(C2U("D3DScenePerspective"),OUString())
{
}

WrappedD3DScenePerspectiveProperty::~WrappedD3DScenePerspectiveProperty()
{
}

void WrappedD3DScenePerspectiveProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    drawing::ProjectionMode aNewValue;
    if( !(rOuterValue >>= aNewValue) )
        throw lang::IllegalArgumentException( C2U("Property D3DScenePerspective requires value of type drawing::ProjectionMode"), 0, 0 );

    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DScenePerspective" );
        return;
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;

    if( aSceneDescriptor.aProjectionMode != aNewValue  )
    {
        aSceneDescriptor.aProjectionMode = aNewValue;
        xInnerPropertySet->setPropertyValue( C2U( "SceneProperties" ), uno::makeAny( aSceneDescriptor ) );
    }
}

Any WrappedD3DScenePerspectiveProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DTransformMatrix" );
        return Any();
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;
    return uno::makeAny( aSceneDescriptor.aProjectionMode );
}

Any WrappedD3DScenePerspectiveProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( drawing::ProjectionMode_PERSPECTIVE );
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DSceneAmbientColorProperty::WrappedD3DSceneAmbientColorProperty()
            : WrappedProperty(C2U("D3DSceneAmbientColor"),OUString())
{
}

WrappedD3DSceneAmbientColorProperty::~WrappedD3DSceneAmbientColorProperty()
{
}

void WrappedD3DSceneAmbientColorProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nNewColor;
    if( !(rOuterValue >>= nNewColor) )
        throw lang::IllegalArgumentException( C2U("Property D3DSceneAmbientColor requires value of type sal_Int32"), 0, 0 );

    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneAmbientColor" );
        return;
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;

    if( aSceneDescriptor.nAmbientLightColor != nNewColor  )
    {
        aSceneDescriptor.nAmbientLightColor = nNewColor;
        xInnerPropertySet->setPropertyValue( C2U( "SceneProperties" ), uno::makeAny( aSceneDescriptor ) );
    }
}

Any WrappedD3DSceneAmbientColorProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneAmbientColor" );
        return Any();
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;
    return uno::makeAny( aSceneDescriptor.nAmbientLightColor );
}

Any WrappedD3DSceneAmbientColorProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( sal_Int32(0x666666) ); // grey40
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DSceneLightColorProperty::WrappedD3DSceneLightColorProperty( sal_Int32 nLightNumber )
            : WrappedProperty(C2U("D3DSceneLightColor"),OUString())
            , m_nLightIndex( nLightNumber-1 )
{
    if( m_nLightIndex < 0 )
        m_nLightIndex = 0;

    m_aOuterName += OUString::valueOf( nLightNumber );
}

WrappedD3DSceneLightColorProperty::~WrappedD3DSceneLightColorProperty()
{
}

void WrappedD3DSceneLightColorProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nNewColor;
    if( !(rOuterValue >>= nNewColor) )
        throw lang::IllegalArgumentException( C2U("Property D3DSceneAmbientColor requires value of type sal_Int32"), 0, 0 );

    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneAmbientColor" );
        return;
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;

    if( aSceneDescriptor.aLightSources.getLength() <= m_nLightIndex )
        aSceneDescriptor.aLightSources.realloc(m_nLightIndex+1);

    if( aSceneDescriptor.aLightSources[m_nLightIndex].nDiffuseColor != nNewColor )
    {
        aSceneDescriptor.aLightSources[m_nLightIndex].nDiffuseColor = nNewColor;
        xInnerPropertySet->setPropertyValue( C2U( "SceneProperties" ), uno::makeAny( aSceneDescriptor ) );
    }
}

Any WrappedD3DSceneLightColorProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneAmbientColor" );
        return Any();
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;
    if( aSceneDescriptor.aLightSources.getLength() <= m_nLightIndex )
        return getPropertyDefault( 0 );

    return uno::makeAny( aSceneDescriptor.aLightSources[m_nLightIndex].nDiffuseColor );
}

Any WrappedD3DSceneLightColorProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(m_nLightIndex==1)
        return uno::makeAny( sal_Int32(0xcccccc) ); // grey80
    return uno::makeAny( sal_Int32(0xffffff) ); // white
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DSceneLightDirectionProperty::WrappedD3DSceneLightDirectionProperty( sal_Int32 nLightNumber )
            : WrappedProperty(C2U("D3DSceneLightDirection"),OUString())
            , m_nLightIndex( nLightNumber-1 )
{
    if( m_nLightIndex < 0 )
        m_nLightIndex = 0;

    m_aOuterName += OUString::valueOf( nLightNumber );
}

WrappedD3DSceneLightDirectionProperty::~WrappedD3DSceneLightDirectionProperty()
{
}

void WrappedD3DSceneLightDirectionProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    drawing::Direction3D aNewValue;
    if( !(rOuterValue >>= aNewValue) )
        throw lang::IllegalArgumentException( C2U("Property D3DSceneLightDirection requires value of type drawing::Direction3D"), 0, 0 );

    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneLightDirection" );
        return;
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;

    if( aSceneDescriptor.aLightSources.getLength() <= m_nLightIndex )
        aSceneDescriptor.aLightSources.realloc(m_nLightIndex+1);

    if( Direction3DToB3DPoint( aSceneDescriptor.aLightSources[m_nLightIndex].aDirection ) != Direction3DToB3DPoint( aNewValue ) )
    {
        aSceneDescriptor.aLightSources[m_nLightIndex].aDirection = aNewValue;
        xInnerPropertySet->setPropertyValue( C2U( "SceneProperties" ), uno::makeAny( aSceneDescriptor ) );
    }
}

Any WrappedD3DSceneLightDirectionProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneLightDirection" );
        return Any();
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;
    if( aSceneDescriptor.aLightSources.getLength() <= m_nLightIndex )
        return getPropertyDefault( 0 );

    return uno::makeAny( aSceneDescriptor.aLightSources[m_nLightIndex].aDirection );
}

Any WrappedD3DSceneLightDirectionProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( drawing::Direction3D( 1, 1, 1 ) );
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DSceneLightOnProperty::WrappedD3DSceneLightOnProperty( sal_Int32 nLightNumber )
            : WrappedProperty(C2U("D3DSceneLightOn"),OUString())
            , m_nLightIndex( nLightNumber-1 )
{
    if( m_nLightIndex < 0 )
        m_nLightIndex = 0;

    m_aOuterName += OUString::valueOf( nLightNumber );
}

WrappedD3DSceneLightOnProperty::~WrappedD3DSceneLightOnProperty()
{
}

void WrappedD3DSceneLightOnProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bNewValue;
    if( !(rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( C2U("Property D3DSceneLightOn requires value of type sal_Bool"), 0, 0 );

    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneLightOn" );
        return;
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;

    if( aSceneDescriptor.aLightSources.getLength() <= m_nLightIndex )
        aSceneDescriptor.aLightSources.realloc(m_nLightIndex+1);

    if( aSceneDescriptor.aLightSources[m_nLightIndex].bIsEnabled != bNewValue )
    {
        aSceneDescriptor.aLightSources[m_nLightIndex].bIsEnabled = bNewValue;
        xInnerPropertySet->setPropertyValue( C2U( "SceneProperties" ), uno::makeAny( aSceneDescriptor ) );
    }
}

Any WrappedD3DSceneLightOnProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR( "need xInnerPropertySet to wrap property D3DSceneLightOn" );
        return Any();
    }

    chart2::SceneDescriptor aSceneDescriptor;
    xInnerPropertySet->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor;
    if( aSceneDescriptor.aLightSources.getLength() <= m_nLightIndex )
        return getPropertyDefault( 0 );

    return uno::makeAny( aSceneDescriptor.aLightSources[m_nLightIndex].bIsEnabled );
}

Any WrappedD3DSceneLightOnProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(m_nLightIndex==1)
        return uno::makeAny( sal_Bool(sal_True) );
    return uno::makeAny( sal_Bool(sal_False) );
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DSceneDistanceProperty::WrappedD3DSceneDistanceProperty()
            : WrappedProperty(C2U("D3DSceneDistance"),OUString())
{
}

WrappedD3DSceneDistanceProperty::~WrappedD3DSceneDistanceProperty()
{
}

void WrappedD3DSceneDistanceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    return; //this property is not supported anymore
}

Any WrappedD3DSceneDistanceProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return getPropertyDefault(0);//this property is not supported anymore
}

Any WrappedD3DSceneDistanceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Int32(419);
    return aRet;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DSceneFocalLengthProperty::WrappedD3DSceneFocalLengthProperty()
            : WrappedProperty(C2U("D3DSceneFocalLength"),OUString())
{
}

WrappedD3DSceneFocalLengthProperty::~WrappedD3DSceneFocalLengthProperty()
{
}

void WrappedD3DSceneFocalLengthProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    return; //this property is not supported anymore
}

Any WrappedD3DSceneFocalLengthProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return getPropertyDefault(0);//this property is not supported anymore
}

Any WrappedD3DSceneFocalLengthProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Int32(800);
    return aRet;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DSceneShadowSlantProperty::WrappedD3DSceneShadowSlantProperty()
            : WrappedProperty(C2U("D3DSceneShadowSlant"),OUString())
{
}

WrappedD3DSceneShadowSlantProperty::~WrappedD3DSceneShadowSlantProperty()
{
}

void WrappedD3DSceneShadowSlantProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    return; //this property is not supported anymore
}

Any WrappedD3DSceneShadowSlantProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return getPropertyDefault(0);//this property is not supported anymore
}

Any WrappedD3DSceneShadowSlantProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Int32(0);
    return aRet;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DSceneTwoSidedLightingProperty::WrappedD3DSceneTwoSidedLightingProperty()
            : WrappedProperty(C2U("D3DSceneTwoSidedLighting"),OUString())
{
}

WrappedD3DSceneTwoSidedLightingProperty::~WrappedD3DSceneTwoSidedLightingProperty()
{
}

void WrappedD3DSceneTwoSidedLightingProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    return; //this property is not supported anymore
}

Any WrappedD3DSceneTwoSidedLightingProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return getPropertyDefault(0);//this property is not supported anymore
}

Any WrappedD3DSceneTwoSidedLightingProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Bool(sal_False);
    return aRet;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

WrappedD3DCameraGeometryProperty::WrappedD3DCameraGeometryProperty()
            : WrappedProperty(C2U("D3DCameraGeometry"),OUString())
{
}

WrappedD3DCameraGeometryProperty::~WrappedD3DCameraGeometryProperty()
{
}

void WrappedD3DCameraGeometryProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    return; //this property is not supported anymore
}

Any WrappedD3DCameraGeometryProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return getPropertyDefault(0);//this property is not supported anymore
}

Any WrappedD3DCameraGeometryProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return Any();
}
*/

} //namespace wrapper
} //namespace chart
//.............................................................................
