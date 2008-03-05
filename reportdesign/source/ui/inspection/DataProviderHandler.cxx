/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataProviderHandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:09:53 $
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

#ifndef RPT_DATAPROVIDERHANDLER_HXX
#include "DataProviderHandler.hxx"
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
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>

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
#include "helpids.hrc"
#include "uistrings.hrc"
#include "RptResId.hrc"
#include "PropertyForward.hxx"
//........................................................................
namespace rptui
{
//........................................................................
using namespace ::com::sun::star;
// using namespace comphelper;

DataProviderHandler::DataProviderHandler(uno::Reference< uno::XComponentContext > const & context)
    :DataProviderHandler_Base(m_aMutex)
    ,m_xContext(context)
    ,m_pInfoService( new OPropertyInfoService() )
{
    try
    {
        m_xFormComponentHandler.set(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.inspection.FormComponentPropertyHandler")),m_xContext),uno::UNO_QUERY_THROW);
        m_xTypeConverter.set(m_xContext->getServiceManager()->createInstanceWithContext( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter" )),m_xContext),uno::UNO_QUERY_THROW);

    }catch(uno::Exception)
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
            TPropertyNamePair aPropertyMediation;
            aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_MASTERFIELDS, PROPERTY_MASTERFIELDS ) );
            aPropertyMediation.insert( TPropertyNamePair::value_type( PROPERTY_DETAILFIELDS, PROPERTY_DETAILFIELDS ) );

            m_xMasterDetails = new OPropertyMediator( m_xDataProvider.get(), m_xReportComponent.get(), aPropertyMediation );
        }

        //const ::rtl::OUString sRowSet(RTL_CONSTASCII_USTRINGPARAM("RowSet"));
        //if ( xNameCont->hasByName(sRowSet) )
        //{
        //    uno::Reference<beans::XPropertySet> xProp(m_xFormComponentHandler,uno::UNO_QUERY);
        //    xProp->setPropertyValue(sRowSet,xNameCont->getByName(sRowSet));
        //}
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
            if ( PropertyName == PROPERTY_COMMAND )
            {

                uno::Sequence< beans::PropertyValue > aArgs( 4 );
                aArgs[0] = beans::PropertyValue(
                    ::rtl::OUString::createFromAscii("CellRangeRepresentation"), -1,
                    uno::makeAny( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("all")) ), beans::PropertyState_DIRECT_VALUE );
                aArgs[1] = beans::PropertyValue(
                    ::rtl::OUString::createFromAscii("HasCategories"), -1,
                    uno::makeAny( sal_True ), beans::PropertyState_DIRECT_VALUE );
                aArgs[2] = beans::PropertyValue(
                    ::rtl::OUString::createFromAscii("FirstCellAsLabel"), -1,
                    uno::makeAny( sal_False ), beans::PropertyState_DIRECT_VALUE );
                aArgs[3] = beans::PropertyValue(
                    ::rtl::OUString::createFromAscii("DataRowSource"), -1,
                    uno::makeAny( chart::ChartDataRowSource_COLUMNS ), beans::PropertyState_DIRECT_VALUE );
                uno::Reference< chart2::data::XDataReceiver > xReceiver(m_xChartModel,uno::UNO_QUERY);
                if ( xReceiver.is() )
                    xReceiver->setArguments( aArgs );
            }
            break;
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
            aOut.PrimaryButtonId = UID_RPT_PROP_CHARTTYPE_DLG;
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::TextField , sal_True);
            aOut.HasPrimaryButton = sal_True;
            break;
        case PROPERTY_ID_PREVIEW_COUNT:
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::NumericField , sal_False);
            break;
        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::StringListField , sal_False);
            aOut.PrimaryButtonId = UID_RPT_PROP_DLG_LINKFIELDS;
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
                OSL_ENSURE( sal_False, "DataProviderHandler::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
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
                OSL_ENSURE( sal_False, "GeometryHandler::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
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
        ;
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
    uno::Sequence< ::rtl::OUString > aRet;
    return aRet;
}

uno::Sequence< ::rtl::OUString > SAL_CALL DataProviderHandler::getActuatingProperties() throw (uno::RuntimeException)
{
    return m_xFormComponentHandler->getActuatingProperties();
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
        uno::Reference< report::XReportDefinition> xReport = m_xReportComponent->getSection()->getReportDefinition();
        bool bDoEnableMasterDetailFields = xReport.is() && xReport->getCommand().getLength() && m_xDataProvider->getCommand().getLength();
        InspectorUI->enablePropertyUIElements( PROPERTY_DETAILFIELDS, inspection::PropertyLineElement::PrimaryButton, bDoEnableMasterDetailFields );
        InspectorUI->enablePropertyUIElements( PROPERTY_MASTERFIELDS, inspection::PropertyLineElement::PrimaryButton, bDoEnableMasterDetailFields );
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

