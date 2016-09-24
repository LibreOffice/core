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
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "uistrings.hrc"
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/syslocale.hxx>
#include <com/sun/star/form/inspection/FormComponentPropertyHandler.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/FormattedString.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
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
#include "RptResId.hrc"
#include "PropertyForward.hxx"

namespace rptui
{

using namespace ::com::sun::star;

DataProviderHandler::DataProviderHandler(uno::Reference< uno::XComponentContext > const & context)
    :DataProviderHandler_Base(m_aMutex)
    ,m_xContext(context)
{
    try
    {
        m_xFormComponentHandler = form::inspection::FormComponentPropertyHandler::create(m_xContext);
        m_xTypeConverter = script::Converter::create(m_xContext);

    }catch(const uno::Exception &)
    {
    }
}

OUString SAL_CALL DataProviderHandler::getImplementationName(  ) throw(uno::RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL DataProviderHandler::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL DataProviderHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_static();
}

OUString DataProviderHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.report.DataProviderHandler");
}

uno::Sequence< OUString > DataProviderHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSupported { "com.sun.star.report.inspection.DataProviderHandler" };
    return aSupported;
}

uno::Reference< uno::XInterface > SAL_CALL DataProviderHandler::create( const uno::Reference< uno::XComponentContext >& _rxContext )
{
    return *(new DataProviderHandler( _rxContext ));
}
// override WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL DataProviderHandler::disposing()
{
    ::comphelper::disposeComponent(m_xFormComponentHandler);
    ::comphelper::disposeComponent( m_xMasterDetails );
    ::comphelper::disposeComponent(m_xTypeConverter);
}
void SAL_CALL DataProviderHandler::addEventListener(const uno::Reference< lang::XEventListener > & xListener) throw (uno::RuntimeException, std::exception)
{
    m_xFormComponentHandler->addEventListener(xListener);
}

void SAL_CALL DataProviderHandler::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw (uno::RuntimeException, std::exception)
{
    m_xFormComponentHandler->removeEventListener(aListener);
}

// inspection::XPropertyHandler:

/********************************************************************************/
void SAL_CALL DataProviderHandler::inspect(const uno::Reference< uno::XInterface > & Component) throw (uno::RuntimeException, lang::NullPointerException, std::exception)
{
    try
    {
        uno::Reference< container::XNameContainer > xNameCont(Component,uno::UNO_QUERY);
        const OUString sFormComponent("FormComponent");
        if ( xNameCont->hasByName(sFormComponent) )
        {
            uno::Reference<beans::XPropertySet> xProp(xNameCont->getByName(sFormComponent),uno::UNO_QUERY);
            const OUString sModel("Model");
            if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(sModel) )
            {
                m_xChartModel.set(xProp->getPropertyValue(sModel),uno::UNO_QUERY);
                if ( m_xChartModel.is() )
                    m_xFormComponent = m_xChartModel->getDataProvider();
            }
        }
        m_xDataProvider.set(m_xFormComponent,uno::UNO_QUERY);
        m_xReportComponent.set( xNameCont->getByName("ReportComponent"), uno::UNO_QUERY );
        if ( m_xDataProvider.is() )
        {
            std::shared_ptr<AnyConverter> aNoConverter(new AnyConverter());
            TPropertyNamePair aPropertyMediation;
            aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_MASTERFIELDS, TPropertyConverter(PROPERTY_MASTERFIELDS,aNoConverter) ) );
            aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_DETAILFIELDS, TPropertyConverter(PROPERTY_DETAILFIELDS,aNoConverter) ) );

            m_xMasterDetails = new OPropertyMediator( m_xDataProvider.get(), m_xReportComponent.get(), aPropertyMediation,true );
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

uno::Any SAL_CALL DataProviderHandler::getPropertyValue(const OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue;
    const sal_Int32 nId = OPropertyInfoService::getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            // TODO: We need a possibility to get the UI of the selected chart type
            // LEM: this business of ignoring ChartType seems very fishy!
            //if( m_xChartModel.is() )
            //{
            //    uno::Reference< chart2::XDiagram > xDiagram( m_xChartModel->getFirstDiagram() );
            //    if( xDiagram.is() )
            //    {
            //        OUString sChartTypes;
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
            //                sChartTypes += ";";
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

void SAL_CALL DataProviderHandler::setPropertyValue(const OUString & PropertyName, const uno::Any & Value) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    const sal_Int32 nId = OPropertyInfoService::getPropertyId(PropertyName);
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

void DataProviderHandler::impl_updateChartTitle_throw(const uno::Any& _aValue)
{
    uno::Reference<chart2::XTitled> xTitled(m_xChartModel,uno::UNO_QUERY);
    if ( xTitled.is() )
    {
        uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
        if ( !xTitle.is() )
        {
            xTitle.set(m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.chart2.Title",m_xContext),uno::UNO_QUERY);
            xTitled->setTitleObject(xTitle);
        }
        if ( xTitle.is() )
        {
            uno::Reference< chart2::XFormattedString2> xFormatted = chart2::FormattedString::create(m_xContext);
            OUString sStr;
            _aValue >>= sStr;
            xFormatted->setString(sStr);
            uno::Sequence< uno::Reference< chart2::XFormattedString> > aArgs(1);
            aArgs[0] = xFormatted;
            xTitle->setText(aArgs);
        }
    }
}

beans::PropertyState SAL_CALL DataProviderHandler::getPropertyState(const OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    return m_xFormComponentHandler->getPropertyState(PropertyName);
}

inspection::LineDescriptor SAL_CALL DataProviderHandler::describePropertyLine(const OUString & PropertyName,  const uno::Reference< inspection::XPropertyControlFactory > & _xControlFactory) throw (beans::UnknownPropertyException, lang::NullPointerException,uno::RuntimeException, std::exception)
{
    inspection::LineDescriptor aOut;
    const sal_Int32 nId = OPropertyInfoService::getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            aOut.PrimaryButtonId = UID_RPT_PROP_CHARTTYPE_DLG;
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::TextField , true);
            aOut.HasPrimaryButton = true;
            break;
        case PROPERTY_ID_PREVIEW_COUNT:
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::NumericField , false);
            break;
        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::StringListField , false);
            aOut.PrimaryButtonId = UID_RPT_PROP_DLG_LINKFIELDS;
            aOut.HasPrimaryButton = true;
            break;
        default:
            aOut = m_xFormComponentHandler->describePropertyLine(PropertyName, _xControlFactory);
    }
    if ( nId != -1 )
    {
        aOut.Category = (OPropertyInfoService::getPropertyUIFlags(nId ) & PropUIFlags::DataProperty) ?
                                    OUString("Data")
                                                        :
                                    OUString("General");
        aOut.HelpURL = HelpIdUrl::getHelpURL( OPropertyInfoService::getPropertyHelpId( nId ) );
        aOut.DisplayName = OPropertyInfoService::getPropertyTranslation(nId);
    }
    return aOut;
}

uno::Any SAL_CALL DataProviderHandler::convertToPropertyValue(const OUString & _rPropertyValue, const uno::Any & _rControlValue) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue( _rControlValue );
    const sal_Int32 nId = OPropertyInfoService::getPropertyId(_rPropertyValue);
    switch(nId)
    {
        case PROPERTY_ID_CHARTTYPE:
            break;
        case PROPERTY_ID_PREVIEW_COUNT:
            try
            {
                aPropertyValue = m_xTypeConverter->convertTo( _rControlValue, ::cppu::UnoType<sal_Int32>::get());
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

uno::Any SAL_CALL DataProviderHandler::convertToControlValue(const OUString & _rPropertyName, const uno::Any & _rPropertyValue, const uno::Type & ControlValueType) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    uno::Any aControlValue( _rPropertyValue );
    if ( !aControlValue.hasValue() )
        // NULL is converted to NULL
        return aControlValue;

    ::osl::MutexGuard aGuard( m_aMutex );
    const sal_Int32 nId = OPropertyInfoService::getPropertyId(_rPropertyName);
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

void SAL_CALL DataProviderHandler::addPropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & Listener) throw (uno::RuntimeException, lang::NullPointerException, std::exception)
{
    m_xFormComponentHandler->addPropertyChangeListener(Listener);
}

void SAL_CALL DataProviderHandler::removePropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener) throw (uno::RuntimeException, std::exception)
{
    m_xFormComponentHandler->removePropertyChangeListener(_rxListener);
}

uno::Sequence< beans::Property > SAL_CALL DataProviderHandler::getSupportedProperties() throw (uno::RuntimeException, std::exception)
{
    ::std::vector< beans::Property > aNewProps;
    if( m_xChartModel.is() )
    {
        rptui::OPropertyInfoService::getExcludeProperties( aNewProps, m_xFormComponentHandler );
        beans::Property aValue;
        static const OUStringLiteral s_pProperties[] =
        {
             OUStringLiteral(PROPERTY_CHARTTYPE)
            ,OUStringLiteral(PROPERTY_MASTERFIELDS)
            ,OUStringLiteral(PROPERTY_DETAILFIELDS)
            ,OUStringLiteral(PROPERTY_PREVIEW_COUNT)
        };

        for (const auto & rName : s_pProperties)
        {
            aValue.Name = rName;
            aNewProps.push_back(aValue);
        }
    }
    return uno::Sequence< beans::Property >(aNewProps.data(), aNewProps.size());
}

uno::Sequence< OUString > SAL_CALL DataProviderHandler::getSupersededProperties() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aRet { PROPERTY_TITLE }; // have a look at OPropertyInfoService::getExcludeProperties
    return aRet;
}

uno::Sequence< OUString > SAL_CALL DataProviderHandler::getActuatingProperties() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Sequence< OUString > aSeq { PROPERTY_TITLE };
    return ::comphelper::concatSequences(m_xFormComponentHandler->getActuatingProperties(),aSeq);
}

sal_Bool SAL_CALL DataProviderHandler::isComposable( const OUString& _rPropertyName ) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    return OPropertyInfoService::isComposable( _rPropertyName, m_xFormComponentHandler );
}

inspection::InteractiveSelectionResult SAL_CALL DataProviderHandler::onInteractivePropertySelection(const OUString & PropertyName, sal_Bool Primary, uno::Any & out_Data, const uno::Reference< inspection::XObjectInspectorUI > & _rxInspectorUI) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::NullPointerException, std::exception)
{
    if ( !_rxInspectorUI.is() )
        throw lang::NullPointerException();

    inspection::InteractiveSelectionResult eResult = inspection::InteractiveSelectionResult_Cancelled;
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    const sal_Int32 nId = OPropertyInfoService::getPropertyId(PropertyName);
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

void SAL_CALL DataProviderHandler::actuatingPropertyChanged(const OUString & ActuatingPropertyName, const uno::Any & NewValue, const uno::Any & OldValue, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI, sal_Bool FirstTimeInit) throw (uno::RuntimeException, lang::NullPointerException, std::exception)
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

            bool bModified = xReport->isModified();
            // this fills the chart again
            ::comphelper::NamedValueCollection aArgs;
            aArgs.put( "CellRangeRepresentation", uno::makeAny( OUString( "all" ) ) );
            aArgs.put( "HasCategories", uno::makeAny( true ) );
            aArgs.put( "FirstCellAsLabel", uno::makeAny( true ) );
            aArgs.put( "DataRowSource", uno::makeAny( chart::ChartDataRowSource_COLUMNS ) );
            uno::Reference< chart2::data::XDataReceiver > xReceiver(m_xChartModel,uno::UNO_QUERY_THROW);
            xReceiver->setArguments( aArgs.getPropertyValues() );
            if ( !bModified )
                xReport->setModified(false);
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
        const sal_Int32 nId = OPropertyInfoService::getPropertyId(ActuatingPropertyName);
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

sal_Bool SAL_CALL DataProviderHandler::suspend(sal_Bool Suspend) throw (uno::RuntimeException, std::exception)
{
    return m_xFormComponentHandler->suspend(Suspend);
}
bool DataProviderHandler::impl_dialogLinkedFields_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
{
    uno::Sequence<uno::Any> aSeq(6);
    beans::PropertyValue aParam;
    aParam.Name = "ParentWindow";
    aParam.Value <<= m_xContext->getValueByName("DialogParentWindow");
    aSeq[0] <<= aParam;
    aParam.Name = "Detail";
    aParam.Value <<= m_xDataProvider;
    aSeq[1] <<= aParam;
    aParam.Name = "Master";
    aParam.Value <<= m_xReportComponent->getSection()->getReportDefinition();
    aSeq[2] <<= aParam;

    aParam.Name = "Explanation";
    aParam.Value <<= OUString(ModuleRes(RID_STR_EXPLANATION));
    aSeq[3] <<= aParam;
    aParam.Name = "DetailLabel";
    aParam.Value <<= OUString(ModuleRes(RID_STR_DETAILLABEL));
    aSeq[4] <<= aParam;
    aParam.Name = "MasterLabel";
    aParam.Value <<= OUString(ModuleRes(RID_STR_MASTERLABEL));
    aSeq[5] <<= aParam;

    uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "org.openoffice.comp.form.ui.MasterDetailLinkDialog", aSeq, m_xContext),
        uno::UNO_QUERY);

    _rClearBeforeDialog.clear();
    return ( xDialog->execute() != 0 );
}

bool DataProviderHandler::impl_dialogChartType_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
{
    uno::Sequence<uno::Any> aSeq(2);
    beans::PropertyValue aParam;
    aParam.Name = "ParentWindow";
    aParam.Value <<= m_xContext->getValueByName("DialogParentWindow");
    aSeq[0] <<= aParam;
    aParam.Name = "ChartModel";
    aParam.Value <<= m_xChartModel;
    aSeq[1] <<= aParam;

    uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.chart2.ChartTypeDialog", aSeq, m_xContext),
        uno::UNO_QUERY);

    _rClearBeforeDialog.clear();
    return ( xDialog->execute() != 0 );
}

} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
