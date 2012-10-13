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
#include "DataProviderHandler.hxx"
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include "uistrings.hrc"
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/syslocale.hxx>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/inspection/XNumericControl.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <tools/fldunit.hxx>
#include "metadata.hxx"
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include "helpids.hrc"
#include "uistrings.hrc"
#include "RptResId.hrc"
#include "PropertyForward.hxx"
//........................................................................
namespace rptui
{
//........................................................................
using namespace ::com::sun::star;

DataProviderHandler::DataProviderHandler(uno::Reference< uno::XComponentContext > const & context)
    :DataProviderHandler_Base(m_aMutex)
    ,m_xContext(context)
    ,m_pInfoService( new OPropertyInfoService() )
{
    try
    {
        m_xFormComponentHandler.set(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.inspection.FormComponentPropertyHandler")),m_xContext),uno::UNO_QUERY_THROW);
        m_xTypeConverter.set(script::Converter::create(m_xContext));

    }catch(const uno::Exception &)
    {
    }
}

//------------------------------------------------------------------------
::rtl::OUString SAL_CALL DataProviderHandler::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL DataProviderHandler::supportsService( const ::rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL DataProviderHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

//------------------------------------------------------------------------
::rtl::OUString DataProviderHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.DataProviderHandler"));
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > DataProviderHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.inspection.DataProviderHandler"));
    return aSupported;
}

//------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL DataProviderHandler::create( const uno::Reference< uno::XComponentContext >& _rxContext )
{
    return *(new DataProviderHandler( _rxContext ));
}
// overload WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL DataProviderHandler::disposing()
{
    ::comphelper::disposeComponent(m_xFormComponentHandler);
    ::comphelper::disposeComponent( m_xMasterDetails );
    ::comphelper::disposeComponent(m_xTypeConverter);
}
void SAL_CALL DataProviderHandler::addEventListener(const uno::Reference< lang::XEventListener > & xListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->addEventListener(xListener);
}

void SAL_CALL DataProviderHandler::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removeEventListener(aListener);
}

// inspection::XPropertyHandler:

/********************************************************************************/
void SAL_CALL DataProviderHandler::inspect(const uno::Reference< uno::XInterface > & Component) throw (uno::RuntimeException, lang::NullPointerException)
{
    try
    {
        uno::Reference< container::XNameContainer > xNameCont(Component,uno::UNO_QUERY);
        const ::rtl::OUString sFormComponent(RTL_CONSTASCII_USTRINGPARAM("FormComponent"));
        if ( xNameCont->hasByName(sFormComponent) )
        {
            uno::Reference<beans::XPropertySet> xProp(xNameCont->getByName(sFormComponent),uno::UNO_QUERY);
            const ::rtl::OUString sModel(RTL_CONSTASCII_USTRINGPARAM("Model"));
            if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(sModel) )
            {
                m_xChartModel.set(xProp->getPropertyValue(sModel),uno::UNO_QUERY);
                if ( m_xChartModel.is() )
                    m_xFormComponent = m_xChartModel->getDataProvider();
            }
        }
        m_xDataProvider.set(m_xFormComponent,uno::UNO_QUERY);
        m_xReportComponent.set( xNameCont->getByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ReportComponent" ) ) ), uno::UNO_QUERY );
        if ( m_xDataProvider.is() )
        {
            ::boost::shared_ptr<AnyConverter> aNoConverter(new AnyConverter());
            TPropertyNamePair aPropertyMediation;
            aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_MASTERFIELDS, TPropertyConverter(PROPERTY_MASTERFIELDS,aNoConverter) ) );
            aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_DETAILFIELDS, TPropertyConverter(PROPERTY_DETAILFIELDS,aNoConverter) ) );

            m_xMasterDetails = new OPropertyMediator( m_xDataProvider.get(), m_xReportComponent.get(), aPropertyMediation,sal_True );
        }
    }
    catch(const uno::Exception &)
    {
        throw lang::NullPointerException();
    }
    if ( m_xFormComponent.is() )
    {
        m_xFormComponentHandler->inspect(m_xFormComponent);
    }
}

uno::Any SAL_CALL DataProviderHandler::getPropertyValue(const ::rtl::OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue;
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            // TODO: We need a possiblity to get the UI of the selected chart type
            //if( m_xChartModel.is() )
            //{
            //    uno::Reference< chart2::XDiagram > xDiagram( m_xChartModel->getFirstDiagram() );
            //    if( xDiagram.is() )
            //    {
            //        ::rtl::OUString sChartTypes;
            //        uno::Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
            //        const uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
            //        const uno::Reference< chart2::XCoordinateSystem >* pIter = aCooSysSeq.getConstArray();
            //        const uno::Reference< chart2::XCoordinateSystem >* pEnd     = pIter + aCooSysSeq.getLength();
            //        for(;pIter != pEnd;++pIter)
            //        {
            //            const uno::Reference< chart2::XChartTypeContainer > xCTCnt( *pIter, uno::UNO_QUERY_THROW );
            //            const uno::Sequence< uno::Reference< chart2::XChartType > > aCTSeq( xCTCnt->getChartTypes());
            //            const uno::Reference< chart2::XChartType >* pChartTypeIter = aCTSeq.getConstArray();
            //            const uno::Reference< chart2::XChartType >* pChartTypeEnd  = pChartTypeIter + aCTSeq.getLength();
            //            for(;pChartTypeIter != pChartTypeEnd;++pChartTypeIter)
            //            {
            //                sChartTypes += (*pChartTypeIter)->getChartType();
            //                sChartTypes += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";"));
            //            }
            //        }
            //        aPropertyValue;// <<= sChartTypes;
            //    }
            //}
            break;
        case PROPERTY_ID_PREVIEW_COUNT:
            aPropertyValue <<= m_xDataProvider->getRowLimit();
            break;
        default:
            aPropertyValue = m_xFormComponentHandler->getPropertyValue( PropertyName );
            break;
    }
    return aPropertyValue;
}

void SAL_CALL DataProviderHandler::setPropertyValue(const ::rtl::OUString & PropertyName, const uno::Any & Value) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue;
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            break;
        case PROPERTY_ID_PREVIEW_COUNT:
            m_xDataProvider->setPropertyValue(PropertyName,Value);
            break;
        default:
            m_xFormComponentHandler->setPropertyValue(PropertyName, Value);
            break;
    }
}
// -----------------------------------------------------------------------------
void DataProviderHandler::impl_updateChartTitle_throw(const uno::Any& _aValue)
{
    uno::Reference<chart2::XTitled> xTitled(m_xChartModel,uno::UNO_QUERY);
    if ( xTitled.is() )
    {
        uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
        if ( !xTitle.is() )
        {
            xTitle.set(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.Title")),m_xContext),uno::UNO_QUERY);
            xTitled->setTitleObject(xTitle);
        }
        if ( xTitle.is() )
        {
            uno::Reference< chart2::XFormattedString> xFormatted(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.FormattedString")),m_xContext),uno::UNO_QUERY);
            ::rtl::OUString sStr;
            _aValue>>= sStr;
            xFormatted->setString(sStr);
            uno::Sequence< uno::Reference< chart2::XFormattedString> > aArgs(1);
            aArgs[0] = xFormatted;
            xTitle->setText(aArgs);
        }
    }
}

beans::PropertyState SAL_CALL DataProviderHandler::getPropertyState(const ::rtl::OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->getPropertyState(PropertyName);
}

inspection::LineDescriptor SAL_CALL DataProviderHandler::describePropertyLine(const ::rtl::OUString & PropertyName,  const uno::Reference< inspection::XPropertyControlFactory > & _xControlFactory) throw (beans::UnknownPropertyException, lang::NullPointerException,uno::RuntimeException)
{
    inspection::LineDescriptor aOut;
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            aOut.PrimaryButtonId = rtl::OUString::createFromAscii(UID_RPT_PROP_CHARTTYPE_DLG);
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::TextField , sal_True);
            aOut.HasPrimaryButton = sal_True;
            break;
        case PROPERTY_ID_PREVIEW_COUNT:
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::NumericField , sal_False);
            break;
        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::StringListField , sal_False);
            aOut.PrimaryButtonId = rtl::OUString::createFromAscii(UID_RPT_PROP_DLG_LINKFIELDS);
            aOut.HasPrimaryButton = sal_True;
            break;
        default:
            aOut = m_xFormComponentHandler->describePropertyLine(PropertyName, _xControlFactory);
    }
    if ( nId != -1 )
    {
        aOut.Category = ((m_pInfoService->getPropertyUIFlags(nId ) & PROP_FLAG_DATA_PROPERTY) != 0) ?
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Data"))
                                                        :
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("General"));
        aOut.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( nId ) );
        aOut.DisplayName = m_pInfoService->getPropertyTranslation(nId);
    }
    return aOut;
}

uno::Any SAL_CALL DataProviderHandler::convertToPropertyValue(const ::rtl::OUString & _rPropertyValue, const uno::Any & _rControlValue) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue( _rControlValue );
    const sal_Int32 nId = m_pInfoService->getPropertyId(_rPropertyValue);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            break;
        case PROPERTY_ID_PREVIEW_COUNT:
            try
            {
                aPropertyValue = m_xTypeConverter->convertTo( _rControlValue, ::getCppuType((const sal_Int32*)0));
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "DataProviderHandler::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
            }
            break;
        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            break;
        default:
            aPropertyValue = m_xFormComponentHandler->convertToPropertyValue(_rPropertyValue, _rControlValue);
    }
    return aPropertyValue;
}

uno::Any SAL_CALL DataProviderHandler::convertToControlValue(const ::rtl::OUString & _rPropertyName, const uno::Any & _rPropertyValue, const uno::Type & ControlValueType) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    uno::Any aControlValue( _rPropertyValue );
    if ( !aControlValue.hasValue() )
        // NULL is converted to NULL
        return aControlValue;

    ::osl::MutexGuard aGuard( m_aMutex );
    const sal_Int32 nId = m_pInfoService->getPropertyId(_rPropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            break;
        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
        case PROPERTY_ID_PREVIEW_COUNT:
            try
            {
                aControlValue = m_xTypeConverter->convertTo( _rPropertyValue, ControlValueType);
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "GeometryHandler::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
            }
            break;
        default:
            aControlValue = m_xFormComponentHandler->convertToControlValue(_rPropertyName, _rPropertyValue, ControlValueType);
    }
    return aControlValue;
}

void SAL_CALL DataProviderHandler::addPropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & Listener) throw (uno::RuntimeException, lang::NullPointerException)
{
    m_xFormComponentHandler->addPropertyChangeListener(Listener);
}

void SAL_CALL DataProviderHandler::removePropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removePropertyChangeListener(_rxListener);
}

uno::Sequence< beans::Property > SAL_CALL DataProviderHandler::getSupportedProperties() throw (uno::RuntimeException)
{
    ::std::vector< beans::Property > aNewProps;
    if( m_xChartModel.is() )
    {
        m_pInfoService->getExcludeProperties( aNewProps, m_xFormComponentHandler );
        beans::Property aValue;
        static const ::rtl::OUString s_pProperties[] =
        {
             PROPERTY_CHARTTYPE
            ,PROPERTY_MASTERFIELDS
            ,PROPERTY_DETAILFIELDS
            ,PROPERTY_PREVIEW_COUNT
        };

        for (size_t nPos = 0; nPos < sizeof(s_pProperties)/sizeof(s_pProperties[0]) ;++nPos )
        {
            aValue.Name = s_pProperties[nPos];
            aNewProps.push_back(aValue);
        }
    }
    return aNewProps.empty() ? uno::Sequence< beans::Property > () : uno::Sequence< beans::Property > (&(*aNewProps.begin()),aNewProps.size());
}

uno::Sequence< ::rtl::OUString > SAL_CALL DataProviderHandler::getSupersededProperties() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = PROPERTY_TITLE; // have a look at OPropertyInfoService::getExcludeProperties
    return aRet;
}

uno::Sequence< ::rtl::OUString > SAL_CALL DataProviderHandler::getActuatingProperties() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Sequence< ::rtl::OUString > aSeq(1);
    aSeq[0] = PROPERTY_TITLE;
    return ::comphelper::concatSequences(m_xFormComponentHandler->getActuatingProperties(),aSeq);
}

::sal_Bool SAL_CALL DataProviderHandler::isComposable( const ::rtl::OUString& _rPropertyName ) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_pInfoService->isComposable( _rPropertyName, m_xFormComponentHandler );
}

inspection::InteractiveSelectionResult SAL_CALL DataProviderHandler::onInteractivePropertySelection(const ::rtl::OUString & PropertyName, ::sal_Bool Primary, uno::Any & out_Data, const uno::Reference< inspection::XObjectInspectorUI > & _rxInspectorUI) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::NullPointerException)
{
    if ( !_rxInspectorUI.is() )
        throw lang::NullPointerException();

    inspection::InteractiveSelectionResult eResult = inspection::InteractiveSelectionResult_Cancelled;
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            if ( impl_dialogChartType_nothrow(aGuard) )
                eResult = inspection::InteractiveSelectionResult_ObtainedValue;
            break;
        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            if ( impl_dialogLinkedFields_nothrow( aGuard ) )
                eResult = inspection::InteractiveSelectionResult_Success;
            break;
        default:
            eResult = m_xFormComponentHandler->onInteractivePropertySelection(PropertyName, Primary, out_Data, _rxInspectorUI);
    }

    return eResult;
}

void SAL_CALL DataProviderHandler::actuatingPropertyChanged(const ::rtl::OUString & ActuatingPropertyName, const uno::Any & NewValue, const uno::Any & OldValue, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI, ::sal_Bool FirstTimeInit) throw (uno::RuntimeException, lang::NullPointerException)
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    if ( ActuatingPropertyName == PROPERTY_COMMAND )
    {
        if ( NewValue != OldValue )
        {
            uno::Reference< report::XReportDefinition> xReport = m_xReportComponent->getSection()->getReportDefinition();
            bool bDoEnableMasterDetailFields = xReport.is() && !xReport->getCommand().isEmpty() && !m_xDataProvider->getCommand().isEmpty();
            InspectorUI->enablePropertyUIElements( PROPERTY_DETAILFIELDS, inspection::PropertyLineElement::PrimaryButton, bDoEnableMasterDetailFields );
            InspectorUI->enablePropertyUIElements( PROPERTY_MASTERFIELDS, inspection::PropertyLineElement::PrimaryButton, bDoEnableMasterDetailFields );

            sal_Bool bModified = xReport->isModified();
            // this fills the chart again
            ::comphelper::NamedValueCollection aArgs;
            aArgs.put( "CellRangeRepresentation", uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "all" ) ) ) );
            aArgs.put( "HasCategories", uno::makeAny( sal_True ) );
            aArgs.put( "FirstCellAsLabel", uno::makeAny( sal_True ) );
            aArgs.put( "DataRowSource", uno::makeAny( chart::ChartDataRowSource_COLUMNS ) );
            uno::Reference< chart2::data::XDataReceiver > xReceiver(m_xChartModel,uno::UNO_QUERY_THROW);
            xReceiver->setArguments( aArgs.getPropertyValues() );
            if ( !bModified )
                xReport->setModified(sal_False);
        }
        m_xFormComponentHandler->actuatingPropertyChanged(ActuatingPropertyName, NewValue, OldValue, InspectorUI, FirstTimeInit);
    }
    else if ( ActuatingPropertyName == PROPERTY_TITLE )
    {
        if ( NewValue != OldValue )
            impl_updateChartTitle_throw(NewValue);
    }
    else
    {
        const sal_Int32 nId = m_pInfoService->getPropertyId(ActuatingPropertyName);
        switch(nId)
        {

            case PROPERTY_ID_MASTERFIELDS:
                break;
            case PROPERTY_ID_DETAILFIELDS:
                break;
            default:
                m_xFormComponentHandler->actuatingPropertyChanged(ActuatingPropertyName, NewValue, OldValue, InspectorUI, FirstTimeInit);
        }
    }
}

::sal_Bool SAL_CALL DataProviderHandler::suspend(::sal_Bool Suspend) throw (uno::RuntimeException)
{
    return m_xFormComponentHandler->suspend(Suspend);
}
bool DataProviderHandler::impl_dialogLinkedFields_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
{
    uno::Sequence<uno::Any> aSeq(6);
    beans::PropertyValue aParam;
    aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow"));
    aParam.Value <<= m_xContext->getValueByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DialogParentWindow")));
    aSeq[0] <<= aParam;
    aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Detail"));
    aParam.Value <<= m_xDataProvider;
    aSeq[1] <<= aParam;
    aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Master"));
    aParam.Value <<= m_xReportComponent->getSection()->getReportDefinition();
    aSeq[2] <<= aParam;

    aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Explanation"));
    aParam.Value <<= ::rtl::OUString(String(ModuleRes(RID_STR_EXPLANATION)));
    aSeq[3] <<= aParam;
    aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DetailLabel"));
    aParam.Value <<= ::rtl::OUString(String(ModuleRes(RID_STR_DETAILLABEL)));
    aSeq[4] <<= aParam;
    aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MasterLabel"));
    aParam.Value <<= ::rtl::OUString(String(ModuleRes(RID_STR_MASTERLABEL)));
    aSeq[5] <<= aParam;

    uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.form.ui.MasterDetailLinkDialog")),aSeq
            , m_xContext), uno::UNO_QUERY);

    _rClearBeforeDialog.clear();
    return ( xDialog->execute() != 0 );
}
// -----------------------------------------------------------------------------
bool DataProviderHandler::impl_dialogChartType_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
{
    uno::Sequence<uno::Any> aSeq(2);
    beans::PropertyValue aParam;
    aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow"));
    aParam.Value <<= m_xContext->getValueByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DialogParentWindow")));
    aSeq[0] <<= aParam;
    aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ChartModel"));
    aParam.Value <<= m_xChartModel;
    aSeq[1] <<= aParam;

    uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.chart2.ChartTypeDialog")),aSeq
            , m_xContext), uno::UNO_QUERY);

    _rClearBeforeDialog.clear();
    return ( xDialog->execute() != 0 );
}
//........................................................................
} // namespace rptui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
