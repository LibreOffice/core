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
#include "Shape.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <svx/unoshape.hxx>

#include "corestrings.hrc"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "Tools.hxx"
#include "RptObject.hxx"
#include "FormatCondition.hxx"
#include "ReportHelperImpl.hxx"
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
uno::Sequence< ::rtl::OUString > lcl_getShapeOptionals()
{
    ::rtl::OUString pProps[] = {
        PROPERTY_DATAFIELD
        ,PROPERTY_CONTROLBACKGROUND
        ,PROPERTY_CONTROLBACKGROUNDTRANSPARENT
    };
    return uno::Sequence< ::rtl::OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
}

DBG_NAME( rpt_OShape )
// -----------------------------------------------------------------------------
OShape::OShape(uno::Reference< uno::XComponentContext > const & _xContext)
:ShapeBase(m_aMutex)
,ShapePropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getShapeOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nZOrder(0)
,m_bOpaque(sal_False)
{
    DBG_CTOR( rpt_OShape,NULL);
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_SHAPE,m_aProps.aComponent.m_xContext->getServiceManager());
}
// -----------------------------------------------------------------------------
OShape::OShape(uno::Reference< uno::XComponentContext > const & _xContext
               ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
               ,uno::Reference< drawing::XShape >& _xShape
               ,const ::rtl::OUString& _sServiceName)
:ShapeBase(m_aMutex)
,ShapePropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getShapeOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nZOrder(0)
,m_bOpaque(sal_False)
,m_sServiceName(_sServiceName)
{
    DBG_CTOR( rpt_OShape,NULL);
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_SHAPE,m_aProps.aComponent.m_xContext->getServiceManager());
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
// -----------------------------------------------------------------------------
OShape::~OShape()
{
    DBG_DTOR( rpt_OShape,NULL);
}
// -----------------------------------------------------------------------------
//IMPLEMENT_FORWARD_XINTERFACE2(OShape,ShapeBase,ShapePropertySet)
IMPLEMENT_FORWARD_REFCOUNT( OShape, ShapeBase )
// --------------------------------------------------------------------------------
uno::Any SAL_CALL OShape::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException)
{
    uno::Any aReturn = ShapeBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = ShapePropertySet::queryInterface(_rType);

    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn.hasValue() ? aReturn : (m_aProps.aComponent.m_xProxy.is() ? m_aProps.aComponent.m_xProxy->queryAggregation(_rType) : aReturn);
}

// -----------------------------------------------------------------------------
void SAL_CALL OShape::dispose() throw(uno::RuntimeException)
{
    ShapePropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
::rtl::OUString OShape::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.Shape"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OShape::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OShape::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_SHAPE;

    return aServices;
}
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OShape::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OShape(xContext));
}

//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OShape::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OShape::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{

    return m_sServiceName == ServiceName || ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
// XReportComponent
REPORTCOMPONENT_IMPL(OShape,m_aProps.aComponent)
REPORTCOMPONENT_IMPL2(OShape,m_aProps.aComponent)
REPORTCOMPONENT_MASTERDETAIL(OShape,m_aProps.aComponent)
REPORTCONTROLFORMAT_IMPL2(OShape,m_aProps.aFormatProperties)
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OShape::getControlBackground() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setControlBackground( ::sal_Int32 /*_backgroundcolor*/ ) throw (uno::RuntimeException,beans::UnknownPropertyException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OShape::getControlBackgroundTransparent() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setControlBackgroundTransparent( ::sal_Bool /*_controlbackgroundtransparent*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OShape::getPropertySetInfo(  ) throw(uno::RuntimeException)
{

    //return ShapePropertySet::getPropertySetInfo();
    return cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
}
// -----------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OShape::getInfoHelper()
{
    if ( !m_pAggHelper.get() )
    {
        uno::Sequence<beans::Property> aAggSeq;
        if ( m_aProps.aComponent.m_xProperty.is() )
            aAggSeq = m_aProps.aComponent.m_xProperty->getPropertySetInfo()->getProperties();
        m_pAggHelper.reset(new OPropertyArrayAggregationHelper(ShapePropertySet::getPropertySetInfo()->getProperties(),aAggSeq));
    }
    return *(m_pAggHelper.get());
}

// -----------------------------------------------------------------------------
void SAL_CALL OShape::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::AGGREGATE_PROPERTY )
        m_aProps.aComponent.m_xProperty->setPropertyValue( aPropertyName,aValue);
    // can be in both
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::DELEGATOR_PROPERTY )
        ShapePropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OShape::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::AGGREGATE_PROPERTY )
        return m_aProps.aComponent.m_xProperty->getPropertyValue( PropertyName);
    // can be in both
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::DELEGATOR_PROPERTY )
        return ShapePropertySet::getPropertyValue( PropertyName);
    return uno::Any();
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::AGGREGATE_PROPERTY || aPropertyName.isEmpty() )
        m_aProps.aComponent.m_xProperty->addPropertyChangeListener( aPropertyName, xListener);
    // can be in both
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::DELEGATOR_PROPERTY || aPropertyName.isEmpty() )
        ShapePropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::AGGREGATE_PROPERTY || aPropertyName.isEmpty() )
        m_aProps.aComponent.m_xProperty->removePropertyChangeListener( aPropertyName, aListener );
    // can be in both
    if( m_pAggHelper->classifyProperty(aPropertyName) == OPropertyArrayAggregationHelper::DELEGATOR_PROPERTY || aPropertyName.isEmpty() )
        ShapePropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::AGGREGATE_PROPERTY || PropertyName.isEmpty() )
        m_aProps.aComponent.m_xProperty->addVetoableChangeListener( PropertyName, aListener );
    // can be in both
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::DELEGATOR_PROPERTY || PropertyName.isEmpty() )
        ShapePropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    getInfoHelper();
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::AGGREGATE_PROPERTY || PropertyName.isEmpty() )
        m_aProps.aComponent.m_xProperty->removeVetoableChangeListener( PropertyName, aListener );
    // can be in both
    if( m_pAggHelper->classifyProperty(PropertyName) == OPropertyArrayAggregationHelper::DELEGATOR_PROPERTY || PropertyName.isEmpty() )
        ShapePropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
// XReportControlModel
::rtl::OUString SAL_CALL OShape::getDataField() throw ( beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setDataField( const ::rtl::OUString& /*_datafield*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OShape::getPrintWhenGroupChange() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.bPrintWhenGroupChange;
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setPrintWhenGroupChange( ::sal_Bool _printwhengroupchange ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    set(PROPERTY_PRINTWHENGROUPCHANGE,_printwhengroupchange,m_aProps.bPrintWhenGroupChange);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OShape::getConditionalPrintExpression() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aConditionalPrintExpression;
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setConditionalPrintExpression( const ::rtl::OUString& _conditionalprintexpression ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    set(PROPERTY_CONDITIONALPRINTEXPRESSION,_conditionalprintexpression,m_aProps.aConditionalPrintExpression);
}
// -----------------------------------------------------------------------------

// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OShape::createClone(  ) throw (uno::RuntimeException)
{
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XReportComponent> xSet;
    try
    {
        SvxShape* pShape = SvxShape::getImplementation( xSource );
        if ( pShape )
        {
            SdrObject* pObject = pShape->GetSdrObject();
            if ( pObject )
            {
                SdrObject* pClone = pObject->Clone();
                if ( pClone )
                {
                    xSet.set(pClone->getUnoShape(),uno::UNO_QUERY_THROW );
                }
            }
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return xSet.get();
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OShape::getParent(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getParent(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setParent( const uno::Reference< uno::XInterface >& Parent ) throw (lang::NoSupportException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xParent = uno::Reference< container::XChild >(Parent,uno::UNO_QUERY);
}
uno::Reference< report::XFormatCondition > SAL_CALL OShape::createFormatCondition(  ) throw (uno::Exception, uno::RuntimeException)
{
    return new OFormatCondition(m_aProps.aComponent.m_xContext);
}
// -----------------------------------------------------------------------------
// XContainer
void SAL_CALL OShape::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aProps.addContainerListener(xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aProps.removeContainerListener(xListener);
}
// -----------------------------------------------------------------------------
// XElementAccess
uno::Type SAL_CALL OShape::getElementType(  ) throw (uno::RuntimeException)
{
    return ::getCppuType(static_cast< uno::Reference<report::XFormatCondition>*>(NULL));
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OShape::hasElements(  ) throw (uno::RuntimeException)
{
    return m_aProps.hasElements();
}
// -----------------------------------------------------------------------------
// XIndexContainer
void SAL_CALL OShape::insertByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.insertByIndex(Index,Element);
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::removeByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.removeByIndex(Index);
}
// -----------------------------------------------------------------------------
// XIndexReplace
void SAL_CALL OShape::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.replaceByIndex(Index,Element);
}
// -----------------------------------------------------------------------------
// XIndexAccess
::sal_Int32 SAL_CALL OShape::getCount(  ) throw (uno::RuntimeException)
{
    return m_aProps.getCount();
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OShape::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    return m_aProps.getByIndex( Index );
}
// -----------------------------------------------------------------------------
// XShape
awt::Point SAL_CALL OShape::getPosition(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getPosition(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setPosition( const awt::Point& aPosition ) throw (uno::RuntimeException)
{
    OShapeHelper::setPosition(aPosition,this);
}
// -----------------------------------------------------------------------------
awt::Size SAL_CALL OShape::getSize(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getSize(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setSize( const awt::Size& aSize ) throw (beans::PropertyVetoException, uno::RuntimeException)
{
    OShapeHelper::setSize(aSize,this);
}
// -----------------------------------------------------------------------------

// XShapeDescriptor
::rtl::OUString SAL_CALL OShape::getShapeType(  ) throw (uno::RuntimeException)
{
   return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CustomShape"));
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OShape::getZOrder() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_ZORDER) >>= m_nZOrder;
    return m_nZOrder;
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setZOrder( ::sal_Int32 _zorder ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_ZORDER,uno::makeAny(_zorder));
    set(PROPERTY_ZORDER,_zorder,m_nZOrder);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OShape::getOpaque() throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bOpaque;
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setOpaque( ::sal_Bool _opaque ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    set(PROPERTY_OPAQUE,_opaque,m_bOpaque);
}
// -----------------------------------------------------------------------------
drawing::HomogenMatrix3 SAL_CALL OShape::getTransformation() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_TRANSFORMATION) >>= m_Transformation;
    return m_Transformation;
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setTransformation( const drawing::HomogenMatrix3& _transformation ) throw (uno::RuntimeException)
{
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_TRANSFORMATION,uno::makeAny(_transformation));
    set(PROPERTY_TRANSFORMATION,_transformation,m_Transformation);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OShape::getCustomShapeEngine() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_CUSTOMSHAPEENGINE) >>= m_CustomShapeEngine;

    return m_CustomShapeEngine;
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setCustomShapeEngine( const ::rtl::OUString& _customshapeengine ) throw (uno::RuntimeException)
{
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_CUSTOMSHAPEENGINE,uno::makeAny(_customshapeengine));
    set(PROPERTY_CUSTOMSHAPEENGINE,_customshapeengine,m_CustomShapeEngine);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OShape::getCustomShapeData() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_CUSTOMSHAPEDATA) >>= m_CustomShapeData;
    return m_CustomShapeData;
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setCustomShapeData( const ::rtl::OUString& _customshapedata ) throw (uno::RuntimeException)
{
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_CUSTOMSHAPEDATA,uno::makeAny(_customshapedata));
    set(PROPERTY_CUSTOMSHAPEDATA,_customshapedata,m_CustomShapeData);
}
// -----------------------------------------------------------------------------
uno::Sequence< beans::PropertyValue > SAL_CALL OShape::getCustomShapeGeometry() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps.aComponent.m_xProperty->getPropertyValue(PROPERTY_CUSTOMSHAPEGEOMETRY) >>= m_CustomShapeGeometry;
    return m_CustomShapeGeometry;
}
// -----------------------------------------------------------------------------
void SAL_CALL OShape::setCustomShapeGeometry( const uno::Sequence< beans::PropertyValue >& _customshapegeometry ) throw (uno::RuntimeException)
{
    m_aProps.aComponent.m_xProperty->setPropertyValue(PROPERTY_CUSTOMSHAPEGEOMETRY,uno::makeAny(_customshapegeometry));
    set(PROPERTY_CUSTOMSHAPEGEOMETRY,_customshapegeometry,m_CustomShapeGeometry);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// =============================================================================
}// namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
