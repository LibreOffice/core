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

#include <sal/config.h>

#include <string_view>

#include <DataProviderHandler.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <strings.hxx>
#include <com/sun/star/form/inspection/FormComponentPropertyHandler.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/FormattedString.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <metadata.hxx>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <core_resource.hxx>
#include <helpids.h>
#include <strings.hrc>
#include <PropertyForward.hxx>

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

OUString SAL_CALL DataProviderHandler::getImplementationName(  )
{
    return "com.sun.star.comp.report.DataProviderHandler";
}

sal_Bool SAL_CALL DataProviderHandler::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL DataProviderHandler::getSupportedServiceNames(  )
{
    return { "com.sun.star.report.inspection.DataProviderHandler" };
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
void SAL_CALL DataProviderHandler::addEventListener(const uno::Reference< lang::XEventListener > & xListener)
{
    m_xFormComponentHandler->addEventListener(xListener);
}

void SAL_CALL DataProviderHandler::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
{
    m_xFormComponentHandler->removeEventListener(aListener);
}

// inspection::XPropertyHandler:

/********************************************************************************/
void SAL_CALL DataProviderHandler::inspect(const uno::Reference< uno::XInterface > & Component)
{
    try
    {
        uno::Reference< container::XNameContainer > xNameCont(Component,uno::UNO_QUERY);
        static const OUStringLiteral sFormComponent(u"FormComponent");
        if ( xNameCont->hasByName(sFormComponent) )
        {
            uno::Reference<beans::XPropertySet> xProp(xNameCont->getByName(sFormComponent),uno::UNO_QUERY);
            static const OUStringLiteral sModel(u"Model");
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
            auto aNoConverter = std::make_shared<AnyConverter>();
            TPropertyNamePair aPropertyMediation;
            aPropertyMediation.emplace( PROPERTY_MASTERFIELDS, TPropertyConverter(PROPERTY_MASTERFIELDS,aNoConverter) );
            aPropertyMediation.emplace( PROPERTY_DETAILFIELDS, TPropertyConverter(PROPERTY_DETAILFIELDS,aNoConverter) );

            m_xMasterDetails = new OPropertyMediator( m_xDataProvider, m_xReportComponent, std::move(aPropertyMediation), true );
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

uno::Any SAL_CALL DataProviderHandler::getPropertyValue(const OUString & PropertyName)
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

void SAL_CALL DataProviderHandler::setPropertyValue(const OUString & PropertyName, const uno::Any & Value)
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
    if ( !xTitled.is() )
        return;

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
        xTitle->setText({ xFormatted });
    }
}

beans::PropertyState SAL_CALL DataProviderHandler::getPropertyState(const OUString & PropertyName)
{
    return m_xFormComponentHandler->getPropertyState(PropertyName);
}

inspection::LineDescriptor SAL_CALL DataProviderHandler::describePropertyLine(const OUString & PropertyName,  const uno::Reference< inspection::XPropertyControlFactory > & _xControlFactory)
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
                                    std::u16string_view(u"Data")
                                                        :
                                    std::u16string_view(u"General");
        aOut.HelpURL = HelpIdUrl::getHelpURL( OPropertyInfoService::getPropertyHelpId( nId ) );
        aOut.DisplayName = OPropertyInfoService::getPropertyTranslation(nId);
    }
    return aOut;
}

uno::Any SAL_CALL DataProviderHandler::convertToPropertyValue(const OUString & _rPropertyValue, const uno::Any & _rControlValue)
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
                TOOLS_WARN_EXCEPTION( "reportdesign", "DataProviderHandler::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
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

uno::Any SAL_CALL DataProviderHandler::convertToControlValue(const OUString & _rPropertyName, const uno::Any & _rPropertyValue, const uno::Type & ControlValueType)
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
                TOOLS_WARN_EXCEPTION( "reportdesign", "GeometryHandler::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
            }
            break;
        default:
            aControlValue = m_xFormComponentHandler->convertToControlValue(_rPropertyName, _rPropertyValue, ControlValueType);
    }
    return aControlValue;
}

void SAL_CALL DataProviderHandler::addPropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & Listener)
{
    m_xFormComponentHandler->addPropertyChangeListener(Listener);
}

void SAL_CALL DataProviderHandler::removePropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener)
{
    m_xFormComponentHandler->removePropertyChangeListener(_rxListener);
}

uno::Sequence< beans::Property > SAL_CALL DataProviderHandler::getSupportedProperties()
{
    ::std::vector< beans::Property > aNewProps;
    if( m_xChartModel.is() )
    {
        rptui::OPropertyInfoService::getExcludeProperties( aNewProps, m_xFormComponentHandler );
        beans::Property aValue;
        static const rtl::OUStringConstExpr s_pProperties[]
        {
             PROPERTY_CHARTTYPE
            ,PROPERTY_MASTERFIELDS
            ,PROPERTY_DETAILFIELDS
            ,PROPERTY_PREVIEW_COUNT
        };

        for (const auto & rName : s_pProperties)
        {
            aValue.Name = rName;
            aNewProps.push_back(aValue);
        }
    }
    return uno::Sequence< beans::Property >(aNewProps.data(), aNewProps.size());
}

uno::Sequence< OUString > SAL_CALL DataProviderHandler::getSupersededProperties()
{
    uno::Sequence< OUString > aRet { PROPERTY_TITLE }; // have a look at OPropertyInfoService::getExcludeProperties
    return aRet;
}

uno::Sequence< OUString > SAL_CALL DataProviderHandler::getActuatingProperties()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Sequence< OUString > aSeq { PROPERTY_TITLE };
    return ::comphelper::concatSequences(m_xFormComponentHandler->getActuatingProperties(),aSeq);
}

sal_Bool SAL_CALL DataProviderHandler::isComposable( const OUString& _rPropertyName )
{
    return OPropertyInfoService::isComposable( _rPropertyName, m_xFormComponentHandler );
}

inspection::InteractiveSelectionResult SAL_CALL DataProviderHandler::onInteractivePropertySelection(const OUString & PropertyName, sal_Bool Primary, uno::Any & out_Data, const uno::Reference< inspection::XObjectInspectorUI > & _rxInspectorUI)
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

void SAL_CALL DataProviderHandler::actuatingPropertyChanged(const OUString & ActuatingPropertyName, const uno::Any & NewValue, const uno::Any & OldValue, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI, sal_Bool FirstTimeInit)
{
    osl::MutexGuard aGuard( m_aMutex );

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

sal_Bool SAL_CALL DataProviderHandler::suspend(sal_Bool Suspend)
{
    return m_xFormComponentHandler->suspend(Suspend);
}
bool DataProviderHandler::impl_dialogLinkedFields_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
{
    uno::Sequence<uno::Any> aSeq(comphelper::InitAnyPropertySequence(
    {
        {"ParentWindow", m_xContext->getValueByName("DialogParentWindow")},
        {"Detail", uno::Any(m_xDataProvider)},
        {"Master", uno::Any(m_xReportComponent->getSection()->getReportDefinition())},
        {"Explanation", uno::Any(RptResId(TranslateId(nullptr, RID_STR_EXPLANATION)))},
        {"DetailLabel", uno::Any(RptResId(RID_STR_DETAILLABEL))},
        {"MasterLabel", uno::Any(RptResId(RID_STR_MASTERLABEL))},
    }));

    uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "org.openoffice.comp.form.ui.MasterDetailLinkDialog", aSeq, m_xContext),
        uno::UNO_QUERY);

    _rClearBeforeDialog.clear();
    return ( xDialog->execute() != 0 );
}

bool DataProviderHandler::impl_dialogChartType_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
{
    uno::Sequence<uno::Any> aSeq(comphelper::InitAnyPropertySequence(
    {
        {"ParentWindow", m_xContext->getValueByName("DialogParentWindow")},
        {"ChartModel", uno::Any(m_xChartModel)}
    }));

    uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.chart2.ChartTypeDialog", aSeq, m_xContext),
        uno::UNO_QUERY);

    _rClearBeforeDialog.clear();
    return ( xDialog->execute() != 0 );
}

} // namespace rptui


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_DataProviderHandler_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new rptui::DataProviderHandler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
