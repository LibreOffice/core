/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormatCondition.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:13 $
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
#ifndef RPT_FORMATCONDITION_HXX
#include "FormatCondition.hxx"
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
#ifndef _COM_SUN_STAR_TEXT_PARAGRAPHVERTALIGN_HPP_
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#endif
#include "ReportHelperImpl.hxx"
#include "corestrings.hrc"
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OFormatCondition::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFormatCondition(xContext));
}

DBG_NAME( rpt_OFormatCondition )
// -----------------------------------------------------------------------------
OFormatCondition::OFormatCondition(uno::Reference< uno::XComponentContext > const & _xContext)
:FormatConditionBase(m_aMutex)
,FormatConditionPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< ::rtl::OUString >())
,m_bEnabled(sal_True)
{
    DBG_CTOR( rpt_OFormatCondition,NULL);
}
// -----------------------------------------------------------------------------
OFormatCondition::~OFormatCondition()
{
    DBG_DTOR( rpt_OFormatCondition,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OFormatCondition,FormatConditionBase,FormatConditionPropertySet)
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::dispose() throw(uno::RuntimeException)
{
    FormatConditionPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
::rtl::OUString OFormatCondition::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.OFormatCondition"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormatCondition::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OFormatCondition::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_FORMATCONDITION;

    return aServices;
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OFormatCondition::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OFormatCondition::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OFormatCondition::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return FormatConditionPropertySet::getPropertySetInfo();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFormatCondition::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return FormatConditionPropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
// XFormatCondition
::sal_Bool SAL_CALL OFormatCondition::getEnabled() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bEnabled;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::setEnabled( ::sal_Bool _enabled ) throw (uno::RuntimeException)
{
    set(PROPERTY_ENABLED,_enabled,m_bEnabled);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormatCondition::getFormula() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_sFormula;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::setFormula( const ::rtl::OUString& _formula ) throw (uno::RuntimeException)
{
    set(PROPERTY_FORMULA,_formula,m_sFormula);
}
// -----------------------------------------------------------------------------
// XReportControlFormat
REPORTCONTROLFORMAT_IMPL(OFormatCondition,m_aFormatProperties)
// =============================================================================
} // namespace reportdesign
// =============================================================================
