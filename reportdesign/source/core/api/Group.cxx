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
#include "Group.hxx"
#include "Section.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/report/GroupOn.hpp>
#include <com/sun/star/report/KeepTogether.hpp>
#include "corestrings.hrc"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "Tools.hxx"
#include <tools/debug.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "Functions.hxx"


namespace reportdesign
{

    using namespace com::sun::star;
    using namespace comphelper;

OGroup::OGroup(const uno::Reference< report::XGroups >& _xParent
               ,const uno::Reference< uno::XComponentContext >& _xContext)
:GroupBase(m_aMutex)
,GroupPropertySet(_xContext,static_cast< GroupPropertySet::Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< OUString >())
,m_xContext(_xContext)
,m_xParent(_xParent)
{
    osl_atomic_increment(&m_refCount);
    {
        m_xFunctions = new OFunctions(this,m_xContext);
    }
    osl_atomic_decrement( &m_refCount );
}

// TODO: VirtualFunctionFinder: This is virtual function!

OGroup::~OGroup()
{
}

IMPLEMENT_FORWARD_XINTERFACE2(OGroup,GroupBase,GroupPropertySet)

OUString SAL_CALL OGroup::getImplementationName(  ) throw(uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.report.Group");
}

uno::Sequence< OUString> OGroup::getSupportedServiceNames_Static() throw( uno::RuntimeException )
{
    uno::Sequence<OUString> aSupported { SERVICE_GROUP };
    return aSupported;
}

uno::Sequence< OUString> SAL_CALL OGroup::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OGroup::supportsService( const OUString& _rServiceName ) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

void SAL_CALL OGroup::dispose() throw(uno::RuntimeException, std::exception)
{
    GroupPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}

// TODO: VirtualFunctionFinder: This is virtual function!

void SAL_CALL OGroup::disposing()
{
    m_xHeader.clear();
    m_xFooter.clear();
    ::comphelper::disposeComponent(m_xFunctions);
    m_xContext.clear();
}

// XGroup
sal_Bool SAL_CALL OGroup::getSortAscending() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_eSortAscending;
}

void SAL_CALL OGroup::setSortAscending( sal_Bool _sortascending ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_SORTASCENDING,_sortascending,m_aProps.m_eSortAscending);
}

sal_Bool SAL_CALL OGroup::getHeaderOn() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xHeader.is();
}

void SAL_CALL OGroup::setHeaderOn( sal_Bool _headeron ) throw (uno::RuntimeException, std::exception)
{
    if ( bool(_headeron) != m_xHeader.is() )
    {
        OUString sName(RPT_RESSTRING(RID_STR_GROUP_HEADER,m_xContext->getServiceManager()));
        setSection(PROPERTY_HEADERON,_headeron,sName,m_xHeader);
    }
}

sal_Bool SAL_CALL OGroup::getFooterOn() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xFooter.is();
}

void SAL_CALL OGroup::setFooterOn( sal_Bool _footeron ) throw (uno::RuntimeException, std::exception)
{
    if ( bool(_footeron) != m_xFooter.is() )
    {
        OUString sName(RPT_RESSTRING(RID_STR_GROUP_FOOTER,m_xContext->getServiceManager()));
        setSection(PROPERTY_FOOTERON,_footeron,sName,m_xFooter);
    }
}

uno::Reference< report::XSection > SAL_CALL OGroup::getHeader() throw (container::NoSuchElementException, uno::RuntimeException, std::exception)
{
    uno::Reference< report::XSection > xRet;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        xRet = m_xHeader;
    }

    if ( !xRet.is() )
        throw container::NoSuchElementException();
    return xRet;
}

uno::Reference< report::XSection > SAL_CALL OGroup::getFooter() throw (container::NoSuchElementException, uno::RuntimeException, std::exception)
{
    uno::Reference< report::XSection > xRet;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        xRet = m_xFooter;
    }

    if ( !xRet.is() )
        throw container::NoSuchElementException();
    return xRet;
}

::sal_Int16 SAL_CALL OGroup::getGroupOn() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nGroupOn;
}

void SAL_CALL OGroup::setGroupOn( ::sal_Int16 _groupon ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    if ( _groupon < report::GroupOn::DEFAULT || _groupon > report::GroupOn::INTERVAL )
        throwIllegallArgumentException("css::report::GroupOn"
                        ,*this
                        ,1
                        ,m_xContext);
    set(PROPERTY_GROUPON,_groupon,m_aProps.m_nGroupOn);
}

::sal_Int32 SAL_CALL OGroup::getGroupInterval() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nGroupInterval;
}

void SAL_CALL OGroup::setGroupInterval( ::sal_Int32 _groupinterval ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_GROUPINTERVAL,_groupinterval,m_aProps.m_nGroupInterval);
}

::sal_Int16 SAL_CALL OGroup::getKeepTogether() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nKeepTogether;
}

void SAL_CALL OGroup::setKeepTogether( ::sal_Int16 _keeptogether ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    if ( _keeptogether < report::KeepTogether::NO || _keeptogether > report::KeepTogether::WITH_FIRST_DETAIL )
        throwIllegallArgumentException("css::report::KeepTogether"
                        ,*this
                        ,1
                        ,m_xContext);
    set(PROPERTY_KEEPTOGETHER,_keeptogether,m_aProps.m_nKeepTogether);
}

uno::Reference< report::XGroups > SAL_CALL OGroup::getGroups() throw (uno::RuntimeException, std::exception)
{
    return m_xParent;
}

OUString SAL_CALL OGroup::getExpression() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_sExpression;
}

void SAL_CALL OGroup::setExpression( const OUString& _expression ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_EXPRESSION,_expression,m_aProps.m_sExpression);
}

sal_Bool SAL_CALL OGroup::getStartNewColumn() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_bStartNewColumn;
}

void SAL_CALL OGroup::setStartNewColumn( sal_Bool _startnewcolumn ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_STARTNEWCOLUMN,_startnewcolumn,m_aProps.m_bStartNewColumn);
}


sal_Bool SAL_CALL OGroup::getResetPageNumber() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_bResetPageNumber;
}

void SAL_CALL OGroup::setResetPageNumber( sal_Bool _resetpagenumber ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_RESETPAGENUMBER,_resetpagenumber,m_aProps.m_bResetPageNumber);
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OGroup::getParent(  ) throw (uno::RuntimeException, std::exception)
{
    return m_xParent;
}

void SAL_CALL OGroup::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ ) throw (lang::NoSupportException, uno::RuntimeException, std::exception)
{
    throw lang::NoSupportException();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OGroup::getPropertySetInfo(  ) throw(uno::RuntimeException, std::exception)
{
    return GroupPropertySet::getPropertySetInfo();
}

void SAL_CALL OGroup::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    GroupPropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OGroup::getPropertyValue( const OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    return GroupPropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OGroup::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    GroupPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OGroup::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    GroupPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OGroup::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    GroupPropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OGroup::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    GroupPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

void OGroup::setSection(     const OUString& _sProperty
                            ,const bool& _bOn
                            ,const OUString& _sName
                            ,uno::Reference< report::XSection>& _member)
{
    BoundListeners l;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        prepareSet(_sProperty, uno::makeAny(_member), uno::makeAny(_bOn), &l);
        lcl_createSectionIfNeeded(_bOn ,this,_member);
        if ( _member.is() )
            _member->setName(_sName);
    }
    l.notify();
}

uno::Reference< report::XFunctions > SAL_CALL OGroup::getFunctions() throw (uno::RuntimeException, std::exception)
{
    return m_xFunctions;
}

} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
