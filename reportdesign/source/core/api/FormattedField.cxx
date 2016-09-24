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
#include "FormattedField.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include "corestrings.hrc"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "Tools.hxx"
#include "FormatCondition.hxx"
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include "ReportHelperImpl.hxx"

namespace reportdesign
{

    using namespace com::sun::star;
    using namespace comphelper;

uno::Reference< uno::XInterface > OFormattedField::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFormattedField(xContext));
}

uno::Sequence< OUString > lcl_getFormattedFieldOptionals()
{
    OUString pProps[] = { OUString(PROPERTY_MASTERFIELDS),OUString(PROPERTY_DETAILFIELDS) };
    return uno::Sequence< OUString >(pProps,SAL_N_ELEMENTS(pProps));
}

OFormattedField::OFormattedField(uno::Reference< uno::XComponentContext > const & _xContext)
:FormattedFieldBase(m_aMutex)
,FormattedFieldPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getFormattedFieldOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nFormatKey(0)
{
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_FORMATTEDFIELD,m_aProps.aComponent.m_xContext->getServiceManager());
}

OFormattedField::OFormattedField(uno::Reference< uno::XComponentContext > const & _xContext
                                 ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
                                 ,uno::Reference< drawing::XShape >& _xShape)
:FormattedFieldBase(m_aMutex)
,FormattedFieldPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getFormattedFieldOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nFormatKey(0)
{
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_FORMATTEDFIELD,m_aProps.aComponent.m_xContext->getServiceManager());
    m_aProps.aComponent.m_xFactory = _xFactory;
    osl_atomic_increment( &m_refCount );
    {
        m_aProps.aComponent.setShape(_xShape,this,m_refCount);
    }
    osl_atomic_decrement( &m_refCount );
}

OFormattedField::~OFormattedField()
{
}

IMPLEMENT_FORWARD_REFCOUNT( OFormattedField, FormattedFieldBase )

uno::Any SAL_CALL OFormattedField::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException, std::exception)
{
    uno::Any aReturn = FormattedFieldBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = FormattedFieldPropertySet::queryInterface(_rType);
    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn.hasValue() ? aReturn : (m_aProps.aComponent.m_xProxy.is() ? m_aProps.aComponent.m_xProxy->queryAggregation(_rType) : aReturn);
}


void SAL_CALL OFormattedField::dispose() throw(uno::RuntimeException, std::exception)
{
    FormattedFieldPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
    m_xFormatsSupplier.clear();
    m_xFunction.clear();
}

OUString OFormattedField::getImplementationName_Static(  ) throw(uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.report.OFormattedField");
}


OUString SAL_CALL OFormattedField::getImplementationName(  ) throw(uno::RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OFormattedField::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aServices(2);
    aServices[0] = SERVICE_FORMATTEDFIELD;
    aServices[1] = "com.sun.star.awt.UnoControlFormattedFieldModel";

    return aServices;
}

uno::Sequence< OUString > SAL_CALL OFormattedField::getSupportedServiceNames(  ) throw(uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OFormattedField::supportsService(const OUString& ServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

// XReportComponent
REPORTCOMPONENT_IMPL(OFormattedField,m_aProps.aComponent)
REPORTCOMPONENT_IMPL2(OFormattedField,m_aProps.aComponent)
REPORTCOMPONENT_NOMASTERDETAIL(OFormattedField)
REPORTCONTROLFORMAT_IMPL(OFormattedField,m_aProps.aFormatProperties)


uno::Reference< beans::XPropertySetInfo > SAL_CALL OFormattedField::getPropertySetInfo(  ) throw(uno::RuntimeException, std::exception)
{
    return FormattedFieldPropertySet::getPropertySetInfo();
}

void SAL_CALL OFormattedField::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    // special case here /// TODO check
    if ( !aValue.hasValue() && aPropertyName == PROPERTY_FORMATKEY )
        m_nFormatKey = 0;
    else
        FormattedFieldPropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OFormattedField::getPropertyValue( const OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    return FormattedFieldPropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OFormattedField::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormattedFieldPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OFormattedField::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormattedFieldPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OFormattedField::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormattedFieldPropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OFormattedField::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormattedFieldPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

// XReportControlModel
OUString SAL_CALL OFormattedField::getDataField() throw ( beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aDataField;
}

void SAL_CALL OFormattedField::setDataField( const OUString& _datafield ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    set(PROPERTY_DATAFIELD,_datafield,m_aProps.aDataField);
}

sal_Bool SAL_CALL OFormattedField::getPrintWhenGroupChange() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.bPrintWhenGroupChange;
}

void SAL_CALL OFormattedField::setPrintWhenGroupChange( sal_Bool _printwhengroupchange ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    set(PROPERTY_PRINTWHENGROUPCHANGE,_printwhengroupchange,m_aProps.bPrintWhenGroupChange);
}

OUString SAL_CALL OFormattedField::getConditionalPrintExpression() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aConditionalPrintExpression;
}

void SAL_CALL OFormattedField::setConditionalPrintExpression( const OUString& _conditionalprintexpression ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    set(PROPERTY_CONDITIONALPRINTEXPRESSION,_conditionalprintexpression,m_aProps.aConditionalPrintExpression);
}


// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OFormattedField::createClone(  ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XFormattedField> xSet(cloneObject(xSource,m_aProps.aComponent.m_xFactory,SERVICE_FORMATTEDFIELD),uno::UNO_QUERY_THROW);

    if ( xSet.is() )
    {
        ::std::vector< uno::Reference< report::XFormatCondition> >::const_iterator aIter = m_aProps.m_aFormatConditions.begin();
        ::std::vector< uno::Reference< report::XFormatCondition> >::const_iterator aEnd  = m_aProps.m_aFormatConditions.end();
        for (sal_Int32 i = 0; aIter != aEnd; ++aIter,++i)
        {
            uno::Reference< report::XFormatCondition > xCond = xSet->createFormatCondition();
            ::comphelper::copyProperties(aIter->get(),xCond.get());
            xSet->insertByIndex(i,uno::makeAny(xCond));
        }
    }
    return xSet.get();
}

// XFormattedField

::sal_Int32 SAL_CALL OFormattedField::getFormatKey() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nFormatKey;
}

void SAL_CALL OFormattedField::setFormatKey(::sal_Int32 _formatkey) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_FORMATKEY,_formatkey,m_nFormatKey);
}

uno::Reference< util::XNumberFormatsSupplier > SAL_CALL OFormattedField::getFormatsSupplier() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_xFormatsSupplier.is() )
    {
        uno::Reference< report::XSection> xSection = getSection();
        if ( xSection.is() )
            m_xFormatsSupplier.set(xSection->getReportDefinition(),uno::UNO_QUERY);
        if ( !m_xFormatsSupplier.is() )
        {
            uno::Reference< beans::XPropertySet> xProp(::dbtools::findDataSource(getParent()),uno::UNO_QUERY);
            if ( xProp.is() )
                m_xFormatsSupplier.set(xProp->getPropertyValue("NumberFormatsSupplier"),uno::UNO_QUERY);
        }
    }
    return m_xFormatsSupplier;
}

void SAL_CALL OFormattedField::setFormatsSupplier( const uno::Reference< util::XNumberFormatsSupplier >& _formatssupplier ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_FORMATSSUPPLIER,_formatssupplier,m_xFormatsSupplier);
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OFormattedField::getParent(  ) throw (uno::RuntimeException, std::exception)
{
    return OShapeHelper::getParent(this);
}

void SAL_CALL OFormattedField::setParent( const uno::Reference< uno::XInterface >& Parent ) throw (lang::NoSupportException, uno::RuntimeException, std::exception)
{
    OShapeHelper::setParent(Parent,this);
}

uno::Reference< report::XFormatCondition > SAL_CALL OFormattedField::createFormatCondition(  ) throw (uno::Exception, uno::RuntimeException, std::exception)
{
    return new OFormatCondition(m_aProps.aComponent.m_xContext);
}

// XContainer
void SAL_CALL OFormattedField::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    m_aProps.addContainerListener(xListener);
}

void SAL_CALL OFormattedField::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    m_aProps.removeContainerListener(xListener);
}

// XElementAccess
uno::Type SAL_CALL OFormattedField::getElementType(  ) throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<report::XFormatCondition>::get();
}

sal_Bool SAL_CALL OFormattedField::hasElements(  ) throw (uno::RuntimeException, std::exception)
{
    return m_aProps.hasElements();
}

// XIndexContainer
void SAL_CALL OFormattedField::insertByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    m_aProps.insertByIndex(Index,Element);
}

void SAL_CALL OFormattedField::removeByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    m_aProps.removeByIndex(Index);
}

// XIndexReplace
void SAL_CALL OFormattedField::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    m_aProps.replaceByIndex(Index,Element);
}

// XIndexAccess
::sal_Int32 SAL_CALL OFormattedField::getCount(  ) throw (uno::RuntimeException, std::exception)
{
    return m_aProps.getCount();
}

uno::Any SAL_CALL OFormattedField::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    return m_aProps.getByIndex( Index );
}

// XShape
awt::Point SAL_CALL OFormattedField::getPosition(  ) throw (uno::RuntimeException, std::exception)
{
    return OShapeHelper::getPosition(this);
}

void SAL_CALL OFormattedField::setPosition( const awt::Point& aPosition ) throw (uno::RuntimeException, std::exception)
{
    OShapeHelper::setPosition(aPosition,this);
}

awt::Size SAL_CALL OFormattedField::getSize(  ) throw (uno::RuntimeException, std::exception)
{
    return OShapeHelper::getSize(this);
}

void SAL_CALL OFormattedField::setSize( const awt::Size& aSize ) throw (beans::PropertyVetoException, uno::RuntimeException, std::exception)
{
    OShapeHelper::setSize(aSize,this);
}


// XShapeDescriptor
OUString SAL_CALL OFormattedField::getShapeType(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_aProps.aComponent.m_xShape.is() )
        return m_aProps.aComponent.m_xShape->getShapeType();
   return OUString("com.sun.star.drawing.ControlShape");
}


} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
