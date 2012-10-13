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
#include "FixedText.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "corestrings.hrc"
#include <com/sun/star/beans/XPropertyState.hpp>
#include "core_resource.hrc"
#include "core_resource.hxx"
#include <comphelper/sequence.hxx>
#include "Tools.hxx"
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <comphelper/property.hxx>
#include "FormatCondition.hxx"
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include "ReportHelperImpl.hxx"
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
uno::Sequence< ::rtl::OUString > lcl_getFixedTextOptionals()
{
    ::rtl::OUString pProps[] = { PROPERTY_DATAFIELD,PROPERTY_MASTERFIELDS,PROPERTY_DETAILFIELDS };
    return uno::Sequence< ::rtl::OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
}
DBG_NAME( rpt_OFixedText )
// -----------------------------------------------------------------------------
OFixedText::OFixedText(uno::Reference< uno::XComponentContext > const & _xContext)
:FixedTextBase(m_aMutex)
,FixedTextPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getFixedTextOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
{
    DBG_CTOR( rpt_OFixedText,NULL);
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_FIXEDTEXT,m_aProps.aComponent.m_xContext->getServiceManager());
    m_aProps.aComponent.m_nBorder = 0; // no border
}
// -----------------------------------------------------------------------------
OFixedText::OFixedText(uno::Reference< uno::XComponentContext > const & _xContext
                       ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
                       ,uno::Reference< drawing::XShape >& _xShape)
:FixedTextBase(m_aMutex)
,FixedTextPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getFixedTextOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
{
    DBG_CTOR( rpt_OFixedText,NULL);
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_FIXEDTEXT,m_aProps.aComponent.m_xContext->getServiceManager());
    m_aProps.aComponent.m_nBorder = 0; // no border
    m_aProps.aComponent.m_xFactory = _xFactory;
    osl_atomic_increment( &m_refCount );
    {
        m_aProps.aComponent.setShape(_xShape,this,m_refCount);
    }
    osl_atomic_decrement( &m_refCount );
}
// -----------------------------------------------------------------------------
OFixedText::~OFixedText()
{
    DBG_DTOR( rpt_OFixedText,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT( OFixedText, FixedTextBase )
// --------------------------------------------------------------------------------
uno::Any SAL_CALL OFixedText::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException)
{
    uno::Any aReturn = FixedTextBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = FixedTextPropertySet::queryInterface(_rType);
    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn.hasValue() ? aReturn : (m_aProps.aComponent.m_xProxy.is() ? m_aProps.aComponent.m_xProxy->queryAggregation(_rType) : aReturn);
}

// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::dispose() throw(uno::RuntimeException)
{
    FixedTextPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
    uno::Reference< report::XFixedText> xHoldAlive = this;
}
// -----------------------------------------------------------------------------
::rtl::OUString OFixedText::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.OFixedText"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFixedText::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OFixedText::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_FIXEDTEXT;

    return aServices;
}
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OFixedText::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFixedText(xContext));
}

//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OFixedText::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OFixedText::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
// XReportComponent
REPORTCOMPONENT_IMPL(OFixedText,m_aProps.aComponent)
REPORTCOMPONENT_IMPL2(OFixedText,m_aProps.aComponent)
REPORTCOMPONENT_NOMASTERDETAIL(OFixedText)
REPORTCONTROLFORMAT_IMPL(OFixedText,m_aProps.aFormatProperties)
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OFixedText::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return FixedTextPropertySet::getPropertySetInfo();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedTextPropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFixedText::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return FixedTextPropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedTextPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedTextPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedTextPropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedTextPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
// XReportControlModel
::rtl::OUString SAL_CALL OFixedText::getDataField() throw ( beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::setDataField( const ::rtl::OUString& /*_datafield*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OFixedText::getPrintWhenGroupChange() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.bPrintWhenGroupChange;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::setPrintWhenGroupChange( ::sal_Bool _printwhengroupchange ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    set(PROPERTY_PRINTWHENGROUPCHANGE,_printwhengroupchange,m_aProps.bPrintWhenGroupChange);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFixedText::getConditionalPrintExpression() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aConditionalPrintExpression;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::setConditionalPrintExpression( const ::rtl::OUString& _conditionalprintexpression ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    set(PROPERTY_CONDITIONALPRINTEXPRESSION,_conditionalprintexpression,m_aProps.aConditionalPrintExpression);
}
// -----------------------------------------------------------------------------

// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OFixedText::createClone(  ) throw (uno::RuntimeException)
{
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XFixedText> xSet(cloneObject(xSource,m_aProps.aComponent.m_xFactory,SERVICE_FIXEDTEXT),uno::UNO_QUERY_THROW);
    return xSet.get();
}
// -----------------------------------------------------------------------------

// XFixedText
::rtl::OUString SAL_CALL OFixedText::getLabel() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_sLabel;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::setLabel( const ::rtl::OUString& _label ) throw (uno::RuntimeException)
{
    set(PROPERTY_LABEL,_label,m_sLabel);
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OFixedText::getParent(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getParent(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::setParent( const uno::Reference< uno::XInterface >& Parent ) throw (lang::NoSupportException, uno::RuntimeException)
{
    OShapeHelper::setParent(Parent,this);
}
// -----------------------------------------------------------------------------
uno::Reference< report::XFormatCondition > SAL_CALL OFixedText::createFormatCondition(  ) throw (uno::Exception, uno::RuntimeException)
{
    return new OFormatCondition(m_aProps.aComponent.m_xContext);
}
// -----------------------------------------------------------------------------
// XContainer
void SAL_CALL OFixedText::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aProps.addContainerListener(xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aProps.removeContainerListener(xListener);
}
// -----------------------------------------------------------------------------
// XElementAccess
uno::Type SAL_CALL OFixedText::getElementType(  ) throw (uno::RuntimeException)
{
    return ::getCppuType(static_cast< uno::Reference<report::XFormatCondition>*>(NULL));
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OFixedText::hasElements(  ) throw (uno::RuntimeException)
{
    return m_aProps.hasElements();
}
// -----------------------------------------------------------------------------
// XIndexContainer
void SAL_CALL OFixedText::insertByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.insertByIndex(Index,Element);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::removeByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.removeByIndex(Index);
}
// -----------------------------------------------------------------------------
// XIndexReplace
void SAL_CALL OFixedText::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.replaceByIndex(Index,Element);
}
// -----------------------------------------------------------------------------
// XIndexAccess
::sal_Int32 SAL_CALL OFixedText::getCount(  ) throw (uno::RuntimeException)
{
    return m_aProps.getCount();
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFixedText::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    return m_aProps.getByIndex( Index );
}
// -----------------------------------------------------------------------------
// XShape
awt::Point SAL_CALL OFixedText::getPosition(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getPosition(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::setPosition( const awt::Point& aPosition ) throw (uno::RuntimeException)
{
    OShapeHelper::setPosition(aPosition,this);
}
// -----------------------------------------------------------------------------
awt::Size SAL_CALL OFixedText::getSize(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getSize(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedText::setSize( const awt::Size& aSize ) throw (beans::PropertyVetoException, uno::RuntimeException)
{
    OShapeHelper::setSize(aSize,this);
}
// -----------------------------------------------------------------------------

// XShapeDescriptor
::rtl::OUString SAL_CALL OFixedText::getShapeType(  ) throw (uno::RuntimeException)
{
   return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ControlShape"));
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// =============================================================================
} // namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
