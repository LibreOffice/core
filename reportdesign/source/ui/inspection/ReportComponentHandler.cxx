/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportComponentHandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:10:38 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPT_REPORTCOMPONENTHANDLER_HXX
#include "ReportComponentHandler.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_PROPERTYCONTROLTYPE_HPP_
#include <com/sun/star/inspection/PropertyControlType.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XNUMERICCONTROL_HPP_
#include <com/sun/star/inspection/XNumericControl.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_MEASUREUNIT_HPP_
#include <com/sun/star/util/MeasureUnit.hpp>
#endif
#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif
#ifndef RPTUI_METADATA_HXX_
#include "metadata.hxx"
#endif

//........................................................................
namespace rptui
{
//........................................................................
using namespace ::com::sun::star;
// using namespace comphelper;

ReportComponentHandler::ReportComponentHandler(uno::Reference< uno::XComponentContext > const & context)
    :ReportComponentHandler_Base(m_aMutex)
    ,m_xContext(context)
    ,m_pInfoService( new OPropertyInfoService() )
{
    try
    {
        m_xFormComponentHandler.set(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.inspection.FormComponentPropertyHandler")),m_xContext),uno::UNO_QUERY_THROW);

    }catch(uno::Exception)
    {
    }
}

//------------------------------------------------------------------------
::rtl::OUString SAL_CALL ReportComponentHandler::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL ReportComponentHandler::supportsService( const ::rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ReportComponentHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

//------------------------------------------------------------------------
::rtl::OUString ReportComponentHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.ReportComponentHandler"));
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > ReportComponentHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.inspection.ReportComponentHandler"));
    return aSupported;
}

//------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL ReportComponentHandler::create( const uno::Reference< uno::XComponentContext >& _rxContext )
{
    return *(new ReportComponentHandler( _rxContext ));
}
// overload WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL ReportComponentHandler::disposing()
{
    ::comphelper::disposeComponent(m_xFormComponentHandler);
}
void SAL_CALL ReportComponentHandler::addEventListener(const uno::Reference< lang::XEventListener > & xListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->addEventListener(xListener);
}

void SAL_CALL ReportComponentHandler::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removeEventListener(aListener);
}

// inspection::XPropertyHandler:

/********************************************************************************/
void SAL_CALL ReportComponentHandler::inspect(const uno::Reference< uno::XInterface > & Component) throw (uno::RuntimeException, lang::NullPointerException)
{
    try
    {
        uno::Reference< container::XNameContainer > xNameCont(Component,uno::UNO_QUERY);
        const ::rtl::OUString sFormComponent(RTL_CONSTASCII_USTRINGPARAM("FormComponent"));
        if ( xNameCont->hasByName(sFormComponent) )
            xNameCont->getByName(sFormComponent) >>= m_xFormComponent;
        const ::rtl::OUString sRowSet(RTL_CONSTASCII_USTRINGPARAM("RowSet"));
        if ( xNameCont->hasByName(sRowSet) )
        {
            uno::Reference<beans::XPropertySet> xProp(m_xFormComponentHandler,uno::UNO_QUERY);
            xProp->setPropertyValue(sRowSet,xNameCont->getByName(sRowSet));
        }
    }
    catch(uno::Exception)
    {
        throw lang::NullPointerException();
    }
    if ( m_xFormComponent.is() )
    {
        m_xFormComponentHandler->inspect(m_xFormComponent);
    }
}

uno::Any SAL_CALL ReportComponentHandler::getPropertyValue(const ::rtl::OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->getPropertyValue(PropertyName);
}

void SAL_CALL ReportComponentHandler::setPropertyValue(const ::rtl::OUString & PropertyName, const uno::Any & Value) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    m_xFormComponentHandler->setPropertyValue(PropertyName, Value);
}

beans::PropertyState SAL_CALL ReportComponentHandler::getPropertyState(const ::rtl::OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->getPropertyState(PropertyName);
}

inspection::LineDescriptor SAL_CALL ReportComponentHandler::describePropertyLine(const ::rtl::OUString & PropertyName,  const uno::Reference< inspection::XPropertyControlFactory > & ControlFactory) throw (beans::UnknownPropertyException, lang::NullPointerException,uno::RuntimeException)
{
    return m_xFormComponentHandler->describePropertyLine(PropertyName, ControlFactory);
}

uno::Any SAL_CALL ReportComponentHandler::convertToPropertyValue(const ::rtl::OUString & PropertyName, const uno::Any & ControlValue) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->convertToPropertyValue(PropertyName, ControlValue);
}

uno::Any SAL_CALL ReportComponentHandler::convertToControlValue(const ::rtl::OUString & PropertyName, const uno::Any & PropertyValue, const uno::Type & ControlValueType) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->convertToControlValue(PropertyName, PropertyValue, ControlValueType);
}

void SAL_CALL ReportComponentHandler::addPropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & Listener) throw (uno::RuntimeException, lang::NullPointerException)
{
    m_xFormComponentHandler->addPropertyChangeListener(Listener);
}

void SAL_CALL ReportComponentHandler::removePropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removePropertyChangeListener(_rxListener);
}

uno::Sequence< beans::Property > SAL_CALL ReportComponentHandler::getSupportedProperties() throw (uno::RuntimeException)
{
    ::std::vector< beans::Property > aNewProps;
    m_pInfoService->getExcludeProperties( aNewProps, m_xFormComponentHandler );

    return aNewProps.empty() ? uno::Sequence< beans::Property > () : uno::Sequence< beans::Property > (&(*aNewProps.begin()),aNewProps.size());
}

uno::Sequence< ::rtl::OUString > SAL_CALL ReportComponentHandler::getSupersededProperties() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet;
    return aRet;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ReportComponentHandler::getActuatingProperties() throw (uno::RuntimeException)
{
    return m_xFormComponentHandler->getActuatingProperties();
}

::sal_Bool SAL_CALL ReportComponentHandler::isComposable( const ::rtl::OUString& _rPropertyName ) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_pInfoService->isComposable( _rPropertyName, m_xFormComponentHandler );
}

inspection::InteractiveSelectionResult SAL_CALL ReportComponentHandler::onInteractivePropertySelection(const ::rtl::OUString & PropertyName, ::sal_Bool Primary, uno::Any & out_Data, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::NullPointerException)
{
    return m_xFormComponentHandler->onInteractivePropertySelection(PropertyName, Primary, out_Data, InspectorUI);
}

void SAL_CALL ReportComponentHandler::actuatingPropertyChanged(const ::rtl::OUString & ActuatingPropertyName, const uno::Any & NewValue, const uno::Any & OldValue, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI, ::sal_Bool FirstTimeInit) throw (uno::RuntimeException, lang::NullPointerException)
{
    m_xFormComponentHandler->actuatingPropertyChanged(ActuatingPropertyName, NewValue, OldValue, InspectorUI, FirstTimeInit);
}

::sal_Bool SAL_CALL ReportComponentHandler::suspend(::sal_Bool Suspend) throw (uno::RuntimeException)
{
    return m_xFormComponentHandler->suspend(Suspend);
}

//........................................................................
} // namespace rptui
//........................................................................

