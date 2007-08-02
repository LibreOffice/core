/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Function.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:29:49 $
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
#ifndef RPT_FUNCTION_HXX
#include "Function.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef REPORTDESIGN_SHARED_CORESTRINGS_HRC
#include "corestrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef REPORTDESIGN_TOOLS_HXX
#include "Tools.hxx"
#endif
#include "corestrings.hrc"
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OFunction::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFunction(xContext));
}

DBG_NAME( rpt_OFunction )
// -----------------------------------------------------------------------------
OFunction::OFunction(uno::Reference< uno::XComponentContext > const & _xContext)
:FunctionBase(m_aMutex)
,FunctionPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< ::rtl::OUString >())
,m_xContext(_xContext)
,m_bPreEvaluated(sal_False)
,m_bDeepTraversing(sal_False)
{
    m_sInitialFormula.IsPresent = sal_False;
    DBG_CTOR( rpt_OFunction,NULL);
}
// -----------------------------------------------------------------------------
OFunction::~OFunction()
{
    DBG_DTOR( rpt_OFunction,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OFunction,FunctionBase,FunctionPropertySet)
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::dispose() throw(uno::RuntimeException)
{
    FunctionPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
::rtl::OUString OFunction::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.OFunction"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFunction::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OFunction::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_FUNCTION;

    return aServices;
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OFunction::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OFunction::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OFunction::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return FunctionPropertySet::getPropertySetInfo();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFunction::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return FunctionPropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
// report::XFunction:
::sal_Bool SAL_CALL OFunction::getPreEvaluated() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_bPreEvaluated;
}
// -----------------------------------------------------------------------------

void SAL_CALL OFunction::setPreEvaluated(::sal_Bool the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_PREEVALUATED,the_value,m_bPreEvaluated);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OFunction::getDeepTraversing() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_bDeepTraversing;
}
// -----------------------------------------------------------------------------

void SAL_CALL OFunction::setDeepTraversing(::sal_Bool the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_DEEPTRAVERSING,the_value,m_bDeepTraversing);
}
// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL OFunction::getName() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_sName;
}
// -----------------------------------------------------------------------------

void SAL_CALL OFunction::setName(const ::rtl::OUString & the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_NAME,the_value,m_sName);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFunction::getFormula() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_sFormula;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::setFormula(const ::rtl::OUString & the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_FORMULA,the_value,m_sFormula);
}
// -----------------------------------------------------------------------------
beans::Optional< ::rtl::OUString> SAL_CALL OFunction::getInitialFormula() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_sInitialFormula;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::setInitialFormula(const beans::Optional< ::rtl::OUString> & the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_INITIALFORMULA,the_value,m_sInitialFormula);
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OFunction::getParent(  ) throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::setParent( const uno::Reference< uno::XInterface >& Parent ) throw (lang::NoSupportException, uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    if ( Parent.is() )
    {
        uno::Reference< report::XFunctions> xFunctions(Parent,uno::UNO_QUERY_THROW);
        m_xParent = xFunctions;
    }
    else
        m_xParent = uno::WeakReference< report::XFunctions >();
}
// -----------------------------------------------------------------------------

// =============================================================================
} // namespace reportdesign
// =============================================================================
