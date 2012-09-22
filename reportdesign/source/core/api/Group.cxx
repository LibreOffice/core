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
#include "Functions.hxx"

// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
DBG_NAME( rpt_OGroup )
// -----------------------------------------------------------------------------
OGroup::OGroup(const uno::Reference< report::XGroups >& _xParent
               ,const uno::Reference< uno::XComponentContext >& _xContext)
:GroupBase(m_aMutex)
,GroupPropertySet(_xContext,static_cast< GroupPropertySet::Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< ::rtl::OUString >())
,m_xContext(_xContext)
,m_xParent(_xParent)
{
    DBG_CTOR( rpt_OGroup,NULL);
    osl_atomic_increment(&m_refCount);
    {
        m_xFunctions = new OFunctions(this,m_xContext);
    }
    osl_atomic_decrement( &m_refCount );
}
//--------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
OGroup::~OGroup()
{
    DBG_DTOR( rpt_OGroup,NULL);
}
//--------------------------------------------------------------------------
void OGroup::copyGroup(const uno::Reference< report::XGroup >& _xSource)
{
    ::comphelper::copyProperties(_xSource.get(),static_cast<GroupPropertySet*>(this));

    if ( _xSource->getHeaderOn() )
    {
        setHeaderOn(sal_True);
        OSection::lcl_copySection(_xSource->getHeader(),m_xHeader);
    }

    if ( _xSource->getFooterOn() )
    {
        setFooterOn(sal_True);
        OSection::lcl_copySection(_xSource->getFooter(),m_xFooter);
    }
}
//--------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OGroup,GroupBase,GroupPropertySet)
//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGroup::getImplementationName(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.Group"));
}
//------------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString> OGroup::getSupportedServiceNames_Static(void) throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = SERVICE_GROUP;
    return aSupported;
}
//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString> SAL_CALL OGroup::getSupportedServiceNames() throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OGroup::supportsService( const ::rtl::OUString& _rServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(_rServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::dispose() throw(uno::RuntimeException)
{
    GroupPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
void SAL_CALL OGroup::disposing()
{
    m_xHeader.clear();
    m_xFooter.clear();
    ::comphelper::disposeComponent(m_xFunctions);
    m_xContext.clear();
}
// -----------------------------------------------------------------------------
// XGroup
::sal_Bool SAL_CALL OGroup::getSortAscending() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_eSortAscending;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setSortAscending( ::sal_Bool _sortascending ) throw (uno::RuntimeException)
{
    set(PROPERTY_SORTASCENDING,_sortascending,m_aProps.m_eSortAscending);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroup::getHeaderOn() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xHeader.is();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setHeaderOn( ::sal_Bool _headeron ) throw (uno::RuntimeException)
{
    if ( _headeron != m_xHeader.is() )
    {
        ::rtl::OUString sName(RPT_RESSTRING(RID_STR_GROUP_HEADER,m_xContext->getServiceManager()));
        setSection(PROPERTY_HEADERON,_headeron,sName,m_xHeader);
    }
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroup::getFooterOn() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xFooter.is();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setFooterOn( ::sal_Bool _footeron ) throw (uno::RuntimeException)
{
    if ( _footeron != m_xFooter.is() )
    {
        ::rtl::OUString sName(RPT_RESSTRING(RID_STR_GROUP_FOOTER,m_xContext->getServiceManager()));
        setSection(PROPERTY_FOOTERON,_footeron,sName,m_xFooter);
    }
}
// -----------------------------------------------------------------------------
uno::Reference< report::XSection > SAL_CALL OGroup::getHeader() throw (container::NoSuchElementException, uno::RuntimeException)
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
// -----------------------------------------------------------------------------
uno::Reference< report::XSection > SAL_CALL OGroup::getFooter() throw (container::NoSuchElementException, uno::RuntimeException)
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
// -----------------------------------------------------------------------------
::sal_Int16 SAL_CALL OGroup::getGroupOn() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nGroupOn;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setGroupOn( ::sal_Int16 _groupon ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( _groupon < report::GroupOn::DEFAULT || _groupon > report::GroupOn::INTERVAL )
        throwIllegallArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com::sun::star::report::GroupOn"))
                        ,*this
                        ,1
                        ,m_xContext);
    set(PROPERTY_GROUPON,_groupon,m_aProps.m_nGroupOn);
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OGroup::getGroupInterval() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nGroupInterval;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setGroupInterval( ::sal_Int32 _groupinterval ) throw (uno::RuntimeException)
{
    set(PROPERTY_GROUPINTERVAL,_groupinterval,m_aProps.m_nGroupInterval);
}
// -----------------------------------------------------------------------------
::sal_Int16 SAL_CALL OGroup::getKeepTogether() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nKeepTogether;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setKeepTogether( ::sal_Int16 _keeptogether ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( _keeptogether < report::KeepTogether::NO || _keeptogether > report::KeepTogether::WITH_FIRST_DETAIL )
        throwIllegallArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com::sun::star::report::KeepTogether"))
                        ,*this
                        ,1
                        ,m_xContext);
    set(PROPERTY_KEEPTOGETHER,_keeptogether,m_aProps.m_nKeepTogether);
}
// -----------------------------------------------------------------------------
uno::Reference< report::XGroups > SAL_CALL OGroup::getGroups() throw (uno::RuntimeException)
{
    return m_xParent;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGroup::getExpression() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_sExpression;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setExpression( const ::rtl::OUString& _expression ) throw (uno::RuntimeException)
{
    set(PROPERTY_EXPRESSION,_expression,m_aProps.m_sExpression);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroup::getStartNewColumn() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_bStartNewColumn;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setStartNewColumn( ::sal_Bool _startnewcolumn ) throw (uno::RuntimeException)
{
    set(PROPERTY_STARTNEWCOLUMN,_startnewcolumn,m_aProps.m_bStartNewColumn);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroup::getResetPageNumber() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_bResetPageNumber;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setResetPageNumber( ::sal_Bool _resetpagenumber ) throw (uno::RuntimeException)
{
    set(PROPERTY_RESETPAGENUMBER,_resetpagenumber,m_aProps.m_bResetPageNumber);
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OGroup::getParent(  ) throw (uno::RuntimeException)
{
    return m_xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ ) throw (lang::NoSupportException, uno::RuntimeException)
{
    throw lang::NoSupportException();
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OGroup::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return GroupPropertySet::getPropertySetInfo();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OGroup::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return GroupPropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void OGroup::setSection(     const ::rtl::OUString& _sProperty
                            ,const sal_Bool& _bOn
                            ,const ::rtl::OUString& _sName
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
// -----------------------------------------------------------------------------
uno::Reference< report::XFunctions > SAL_CALL OGroup::getFunctions() throw (uno::RuntimeException)
{
    return m_xFunctions;
}
// =============================================================================
} // namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
