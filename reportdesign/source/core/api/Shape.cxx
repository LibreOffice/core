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
#include <Shape.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>
#include <svx/unoshape.hxx>

#include <strings.hxx>
#include <strings.hrc>
#include <core_resource.hxx>
#include <Tools.hxx>
#include <FormatCondition.hxx>
#include <ReportHelperImpl.hxx>

namespace reportdesign
{

    using namespace com::sun::star;
    using namespace comphelper;
static uno::Sequence< OUString > lcl_getShapeOptionals()
{
    return {
        PROPERTY_DATAFIELD
        ,PROPERTY_CONTROLBACKGROUND
        ,PROPERTY_CONTROLBACKGROUNDTRANSPARENT
    };
}


OShape::OShape(uno::Reference< uno::XComponentContext > const & _xContext)
:ShapeBase(m_aMutex)
,ShapePropertySet(_xContext,IMPLEMENTS_PROPERTY_SET,lcl_getShapeOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nZOrder(0)
,m_bOpaque(false)
{
    m_aProps.aComponent.m_sName  = RptResId(RID_STR_SHAPE);
}

OShape::OShape(uno::Reference< uno::XComponentContext > const & _xContext
               ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
               ,uno::Reference< drawing::XShape >& _xShape
               ,const OUString& _sServiceName)
:ShapeBase(m_aMutex)
,ShapePropertySet(_xContext,IMPLEMENTS_PROPERTY_SET,lcl_getShapeOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nZOrder(0)
,m_bOpaque(false)
,m_sServiceName(_sServiceName)
{
    m_aProps.aComponent.m_sName  = RptResId(RID_STR_SHAPE);
    m_aProps.aComponent.m_xFactory = _xFactory;
    osl_atomic_increment( &m_refCount );
    {
        uno::Reference<beans::XPropertySet> xProp(_xShape,uno::UNO_QUERY);
        if ( xProp.is() )
        {
            xProp->getPropertyValue(PROPERTY_ZORDER)  >>= m_nZOrder;
            xProp.clear();
        }
        m_aProps.aComponent.setShape(_xShape,this,m_refCount);
    }
    osl_atomic_decrement( &m_refCount );
}

OShape::~OShape()
{
}

//IMPLEMENT_FORWARD_XINTERFACE2(OShape,ShapeBase,ShapePropertySet)
IMPLEMENT_FORWARD_REFCOUNT( OShape, ShapeBase )

uno::Any SAL_CALL OShape::queryInterface( const uno::Type& _rType )
{
    uno::Any aReturn = ShapeBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = ShapePropertySet::queryInterface(_rType);

    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn.hasValue() ? aReturn : (m_aProps.aComponent.m_xProxy.is() ? m_aProps.aComponent.m_xProxy->queryAggregation(_rType) : aReturn);
}


void SAL_CALL OShape::dispose()
{
    ShapePropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}

uno::Reference< uno::XInterface > OShape::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OShape(xContext));
}


OUString OShape::getImplementationName_Static(  )
{
    return "com.sun.star.comp.report.Shape";
}


OUString SAL_CALL OShape::getImplementationName(  )
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OShape::getSupportedServiceNames_Static(  )
{
    return { SERVICE_SHAPE };
}

uno::Sequence< OUString > SAL_CALL OShape::getSupportedServiceNames(  )
{
    if(m_sServiceName.isEmpty())
    {
        return getSupportedServiceNames_Static();
    }
    else
    {
        return { SERVICE_SHAPE, m_sServiceName };
    }
}

sal_Bool SAL_CALL OShape::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XReportComponent
REPORTCOMPONENT_IMPL(OShape,m_aProps.aComponent)
REPORTCOMPONENT_IMPL2(OShape,m_aProps.aComponent)
REPORTCOMPONENT_MASTERDETAIL(OShape,m_aProps.aComponent)
REPORTCONTROLFORMAT_IMPL2(OShape,m_aProps.aFormatProperties)

::sal_Int32 SAL_CALL OShape::getControlBackground()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OShape::setControlBackground( ::sal_Int32 /*_backgroundcolor*/ )
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL OShape::getControlBackgroundTransparent()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OShape::setControlBackgroundTransparent( sal_Bool /*_controlbackgroundtransparent*/ )
{
    throw beans::UnknownPropertyException();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OShape::getPropertySetInfo(  )
{

    //return ShapePropertySet::getPropertySetInfo();
    return cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
}

cppu::IPropertyArrayHelper& OShape::getInfoHelper()
{
    if (!m_pAggHelper)
    {
        uno::Sequence<beans::Property> aAggSeq;
        if ( m_aProps.aComponent.m_xProperty.is() )
            aAggSeq = m_aProps.aComponent.m_xProperty->getPropertySetInfo()->getProperties();
        m_pAggHelper.reset(new OPropertyArrayAggregationHelper(ShapePropertySet::getPropertySetInfo()->getProperties(),aAggSeq));
    }
    return *m_pAggHelper;
}


void SAL_CALL OShape::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Aggregate )
        m_aProps.aComponent.m_xProperty->setPropertyValue( aPropertyName,aValue);
    // can be in both
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Delegator )
        ShapePropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OShape::getPropertyValue( const OUString& PropertyName )
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Aggregate )
        return m_aProps.aComponent.m_xProperty->getPropertyValue( PropertyName);
    // can be in both
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Delegator )
        return ShapePropertySet::getPropertyValue( PropertyName);
    return uno::Any();
}

void SAL_CALL OShape::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Aggregate || aPropertyName.isEmpty() )
        m_aProps.aComponent.m_xProperty->addPropertyChangeListener( aPropertyName, xListener);
    // can be in both
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Delegator || aPropertyName.isEmpty() )
        ShapePropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OShape::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Aggregate || aPropertyName.isEmpty() )
        m_aProps.aComponent.m_xProperty->removePropertyChangeListener( aPropertyName, aListener );
    // can be in both
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Delegator || aPropertyName.isEmpty() )
        ShapePropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OShape::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Aggregate || PropertyName.isEmpty() )
        m_aProps.aComponent.m_xProperty->addVetoableChangeListener( PropertyName, aListener );
    // can be in both
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Delegator || PropertyName.isEmpty() )
        ShapePropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OShape::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Aggregate || PropertyName.isEmpty() )
        m_aProps.aComponent.m_xProperty->removeVetoableChangeListener( PropertyName, aListener );
    // can be in both
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::PropertyOrigin::Delegator || PropertyName.isEmpty() )
        ShapePropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

// XReportControlModel
OUString SAL_CALL OShape::getDataField()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OShape::setDataField( const OUString& /*_datafield*/ )
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL OShape::getPrintWhenGroupChange()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.bPrintWhenGroupChange;
}

void SAL_CALL OShape::setPrintWhenGroupChange( sal_Bool _printwhengroupchange )
{
    set(PROPERTY_PRINTWHENGROUPCHANGE,_printwhengroupchange,m_aProps.bPrintWhenGroupChange);
}

OUString SAL_CALL OShape::getConditionalPrintExpression()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aConditionalPrintExpression;
}

void SAL_CALL OShape::setConditionalPrintExpression( const OUString& _conditionalprintexpression )
{
    set(PROPERTY_CONDITIONALPRINTEXPRESSION,_conditionalprintexpression,m_aProps.aConditionalPrintExpression);
}


// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OShape::createClone(  )
{
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XReportComponent> xSet;
    try
    {
        SdrObject* pObject = SdrObject::getSdrObjectFromXShape( xSource );
        if ( pObject )
        {
            SdrObject* pClone(pObject->CloneSdrObject(pObject->getSdrModelFromSdrObject()));
            if ( pClone )
            {
                xSet.set(pClone->getUnoShape(),uno::UNO_QUERY_THROW );
            }
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
    return xSet;
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OShape::getParent(  )
{
    return OShapeHelper::getParent(this);
}

void SAL_CALL OShape::setParent( const uno::Reference< uno::XInterface >& Parent )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xParent = uno::Reference< container::XChild >(Parent,uno::UNO_QUERY);
}
uno::Reference< report::XFormatCondition > SAL_CALL OShape::createFormatCondition(  )
{
    return new OFormatCondition(m_aProps.aComponent.m_xContext);
}

// XContainer
void SAL_CALL OShape::addContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aProps.addContainerListener(xListener);
}

void SAL_CALL OShape::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aProps.removeContainerListener(xListener);
}

// XElementAccess
uno::Type SAL_CALL OShape::getElementType(  )
{
    return cppu::UnoType<report::XFormatCondition>::get();
}

sal_Bool SAL_CALL OShape::hasElements(  )
{
    return m_aProps.hasElements();
}

// XIndexContainer
void SAL_CALL OShape::insertByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    m_aProps.insertByIndex(Index,Element);
}

void SAL_CALL OShape::removeByIndex( ::sal_Int32 Index )
{
    m_aProps.removeByIndex(Index);
}

// XIndexReplace
void SAL_CALL OShape::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    m_aProps.replaceByIndex(Index,Element);
}

// XIndexAccess
::sal_Int32 SAL_CALL OShape::getCount(  )
{
    return m_aProps.getCount();
}

uno::Any SAL_CALL OShape::getByIndex( ::sal_Int32 Index )
{
    return m_aProps.getByIndex( Index );
}

// XShape
awt::Point SAL_CALL OShape::getPosition(  )
{
    return OShapeHelper::getPosition(this);
}

void SAL_CALL OShape::setPosition( const awt::Point& aPosition )
{
    OShapeHelper::setPosition(aPosition,this);
}

awt::Size SAL_CALL OShape::getSize(  )
{
    return OShapeHelper::getSize(this);
}

void SAL_CALL OShape::setSize( const awt::Size& aSize )
{
    OShapeHelper::setSize(aSize,this);
}


// XShapeDescriptor
OUString SAL_CALL OShape::getShapeType(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_aProps.aComponent.m_xShape.is() )
        return m_aProps.aComponent.m_xShape->getShapeType();
    return "com.sun.star.drawing.CustomShape";
}

::sal_Int32 SAL_CALL OShape::getZOrder()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_ZORDER) >>= m_nZOrder;
    return m_nZOrder;
}

void SAL_CALL OShape::setZOrder( ::sal_Int32 _zorder )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_ZORDER,uno::makeAny(_zorder));
    set(PROPERTY_ZORDER,_zorder,m_nZOrder);
}

sal_Bool SAL_CALL OShape::getOpaque()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bOpaque;
}

void SAL_CALL OShape::setOpaque( sal_Bool _opaque )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    set(PROPERTY_OPAQUE,_opaque,m_bOpaque);
}

drawing::HomogenMatrix3 SAL_CALL OShape::getTransformation()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_TRANSFORMATION) >>= m_Transformation;
    return m_Transformation;
}

void SAL_CALL OShape::setTransformation( const drawing::HomogenMatrix3& _transformation )
{
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_TRANSFORMATION,uno::makeAny(_transformation));
    set(PROPERTY_TRANSFORMATION,_transformation,m_Transformation);
}

OUString SAL_CALL OShape::getCustomShapeEngine()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_CUSTOMSHAPEENGINE) >>= m_CustomShapeEngine;

    return m_CustomShapeEngine;
}

void SAL_CALL OShape::setCustomShapeEngine( const OUString& _customshapeengine )
{
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_CUSTOMSHAPEENGINE,uno::makeAny(_customshapeengine));
    set(PROPERTY_CUSTOMSHAPEENGINE,_customshapeengine,m_CustomShapeEngine);
}

OUString SAL_CALL OShape::getCustomShapeData()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_CUSTOMSHAPEDATA) >>= m_CustomShapeData;
    return m_CustomShapeData;
}

void SAL_CALL OShape::setCustomShapeData( const OUString& _customshapedata )
{
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_CUSTOMSHAPEDATA,uno::makeAny(_customshapedata));
    set(PROPERTY_CUSTOMSHAPEDATA,_customshapedata,m_CustomShapeData);
}

uno::Sequence< beans::PropertyValue > SAL_CALL OShape::getCustomShapeGeometry()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_CUSTOMSHAPEGEOMETRY) >>= m_CustomShapeGeometry;
    return m_CustomShapeGeometry;
}

void SAL_CALL OShape::setCustomShapeGeometry( const uno::Sequence< beans::PropertyValue >& _customshapegeometry )
{
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_CUSTOMSHAPEGEOMETRY,uno::makeAny(_customshapegeometry));
    set(PROPERTY_CUSTOMSHAPEGEOMETRY,_customshapegeometry,m_CustomShapeGeometry);
}


}// namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
