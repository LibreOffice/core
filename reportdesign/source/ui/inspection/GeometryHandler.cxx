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
#include "GeometryHandler.hxx"

#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include <comphelper/mimeconfighelper.hxx>

#include "uistrings.hrc"
#include "reportformula.hxx"

#include <unotools/textsearch.hxx>
#include <unotools/configmgr.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/syslocale.hxx>
#include <tools/diagnose_ex.h>
#include <tools/StringListResource.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include "com/sun/star/form/inspection/FormComponentPropertyHandler.hpp"
#include "com/sun/star/inspection/StringRepresentation.hpp"
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/XStringListControl.hpp>
#include <com/sun/star/report/Function.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/FilterDialog.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/inspection/XNumericControl.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <tools/fldunit.hxx>
#include <vcl/stdtext.hxx>

#include "ModuleHelper.hxx"
#include "RptResId.hrc"
#include "RptDef.hxx"
#include "UITools.hxx"

#include <connectivity/dbexception.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>

#include <boost/bind.hpp>
#include <tools/string.hxx>
#include "metadata.hxx"
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svx/xdef.hxx>
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svxids.hrc>
#include <svx/drawitem.hxx>
#include <editeng/brushitem.hxx>
#include <sfx2/docfilt.hxx>

#include "dlgpage.hxx"
#include "helpids.hrc"
#include <toolkit/helper/convert.hxx>

#include <o3tl/compat_functional.hxx>

#define DATA_OR_FORMULA     0
#define FUNCTION            1
#define COUNTER             2
#define USER_DEF_FUNCTION   3
#define UNDEF_DATA          4

//........................................................................
namespace rptui
{
//........................................................................
using namespace ::com::sun::star;

namespace{
// comparing two property instances
struct PropertyCompare : public ::std::binary_function< beans::Property, OUString , bool >
{
    bool operator() (const beans::Property& x, const OUString& y) const
    {
        return x.Name.equals(y);
    }
    bool operator() (const OUString& x,const beans::Property& y) const
    {
        return x.equals(y.Name);
    }
};

// -----------------------------------------------------------------------------
OUString lcl_getQuotedFunctionName(const OUString& _sFunction)
{
    OUString sQuotedFunctionName("[");
    sQuotedFunctionName += _sFunction + OUString("]");
    return sQuotedFunctionName;
}
// -----------------------------------------------------------------------------
OUString lcl_getQuotedFunctionName(const uno::Reference< report::XFunction>& _xFunction)
{
    return lcl_getQuotedFunctionName(_xFunction->getName());
}
// -----------------------------------------------------------------------------
void lcl_collectFunctionNames(const uno::Reference< report::XFunctions>& _xFunctions,TFunctions& _rFunctionNames)
{
    uno::Reference< report::XFunctionsSupplier> xParent(_xFunctions->getParent(),uno::UNO_QUERY_THROW);
    const sal_Int32 nCount = _xFunctions->getCount();
    for (sal_Int32 i = 0; i < nCount ; ++i)
    {
        uno::Reference< report::XFunction > xFunction(_xFunctions->getByIndex(i),uno::UNO_QUERY_THROW);
        _rFunctionNames.insert(TFunctions::value_type(lcl_getQuotedFunctionName(xFunction),TFunctionPair(xFunction,xParent)));
    }
}
// -----------------------------------------------------------------------------
void lcl_collectFunctionNames(const uno::Reference< report::XSection>& _xSection,TFunctions& _rFunctionNames)
{
    const uno::Reference< report::XReportDefinition> xReportDefinition = _xSection->getReportDefinition();
    const uno::Reference< report::XGroups> xGroups = xReportDefinition->getGroups();
    sal_Int32 nPos = -1;
    uno::Reference< report::XGroup> xGroup = _xSection->getGroup();
    if ( xGroup.is() )
        nPos = getPositionInIndexAccess(xGroups.get(),xGroup);
    else if ( _xSection == xReportDefinition->getDetail() )
        nPos = xGroups->getCount()-1;

    for (sal_Int32 i = 0 ; i <= nPos ; ++i)
    {
        xGroup.set(xGroups->getByIndex(i),uno::UNO_QUERY_THROW);
        lcl_collectFunctionNames(xGroup->getFunctions(),_rFunctionNames);
    }
    lcl_collectFunctionNames(xReportDefinition->getFunctions(),_rFunctionNames);
}
// -----------------------------------------------------------------------------
void lcl_convertFormulaTo(const uno::Any& _aPropertyValue,uno::Any& _rControlValue)
{
    OUString sName;
    _aPropertyValue >>= sName;
    const sal_Int32 nLen = sName.getLength();
    if ( nLen )
    {
        ReportFormula aFormula( sName );
        _rControlValue <<= aFormula.getUndecoratedContent();
    }
}

// return value rounded to the nearest multiple of base
// if equidistant of two multiples, round up (for positive numbers)
// T is assumed to be an integer type
template <typename T, T base> T lcl_round(T value)
{
    OSL_ENSURE(value >= 0, "lcl_round: positive numbers only please");
    const T threshold = (base % 2 == 0) ? (base/2) : (base/2 + 1);
    const T rest = value % base;
    if ( rest >= threshold )
        return value + (base - rest);
    else
        return value - rest;
}

} // anonymous namespace
// -----------------------------------------------------------------------------
bool GeometryHandler::impl_isDataField(const OUString& _sName) const
{
    const OUString* pEnd = m_aFieldNames.getConstArray() + m_aFieldNames.getLength();
    bool bIsField = ( ::std::find( m_aFieldNames.getConstArray(), pEnd, _sName ) != pEnd );

    if ( !bIsField )
    {
        pEnd = m_aParamNames.getConstArray() + m_aParamNames.getLength();
        bIsField = ( ::std::find( m_aParamNames.getConstArray(), pEnd, _sName ) != pEnd );
    }
    return bIsField;
}
// -----------------------------------------------------------------------------
OUString GeometryHandler::impl_convertToFormula( const uno::Any& _rControlValue )
{
    OUString sName;
    _rControlValue >>= sName;

    if ( sName.isEmpty() )
        return sName;

    ReportFormula aParser( sName );
    if ( aParser.isValid() )
        return sName;

    aParser = ReportFormula( impl_isDataField(sName) ? ReportFormula::Field : ReportFormula::Expression, sName );
    return aParser.getCompleteFormula();
}
DBG_NAME(rpt_GeometryHandler)
GeometryHandler::GeometryHandler(uno::Reference< uno::XComponentContext > const & context) :
    GeometryHandler_Base(m_aMutex)
    ,m_aPropertyListeners( m_aMutex )
    ,m_xContext(context)
    ,m_pInfoService(new OPropertyInfoService())
    ,m_nDataFieldType(0)
    ,m_bIn(false)
{
    DBG_CTOR(rpt_GeometryHandler,NULL);
    try
    {
        m_xFormComponentHandler = form::inspection::FormComponentPropertyHandler::create(m_xContext);
        m_xTypeConverter = script::Converter::create(context);
        loadDefaultFunctions();
    }
    catch(const uno::Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
GeometryHandler::~GeometryHandler()
{
    DBG_DTOR(rpt_GeometryHandler,NULL);
}
//------------------------------------------------------------------------
OUString SAL_CALL GeometryHandler::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL GeometryHandler::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
}

//------------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL GeometryHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

//------------------------------------------------------------------------
OUString GeometryHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.report.GeometryHandler");
}

//------------------------------------------------------------------------
uno::Sequence< OUString > GeometryHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSupported(1);
    aSupported[0] = OUString("com.sun.star.report.inspection.GeometryHandler");
    return aSupported;
}

//------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL GeometryHandler::create( const uno::Reference< uno::XComponentContext >& _rxContext )
{
    return *(new GeometryHandler( _rxContext ));
}
// overload WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL GeometryHandler::disposing()
{
    try
    {
        ::comphelper::disposeComponent(m_xFormComponentHandler);
        ::comphelper::disposeComponent(m_xTypeConverter);
        if ( m_xReportComponent.is() && m_xReportComponent->getPropertySetInfo()->hasPropertyByName(PROPERTY_DATAFIELD) )
            m_xReportComponent->removePropertyChangeListener(PROPERTY_DATAFIELD,static_cast< beans::XPropertyChangeListener* >( this ));

        m_xReportComponent.clear();
        m_xRowSet.clear();
        m_aPropertyListeners.clear();
    }
    catch(uno::Exception&)
    {}
}
void SAL_CALL GeometryHandler::addEventListener(const uno::Reference< lang::XEventListener > & xListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->addEventListener(xListener);
}

void SAL_CALL GeometryHandler::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removeEventListener(aListener);
}

// inspection::XPropertyHandler:

/********************************************************************************/
void SAL_CALL GeometryHandler::inspect( const uno::Reference< uno::XInterface > & _rxInspectee ) throw (uno::RuntimeException, lang::NullPointerException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_sScope = m_sDefaultFunction = OUString();
    m_bNewFunction = false;
    m_nDataFieldType = 0;
    m_xFunction.clear();
    m_aFunctionNames.clear();
    try
    {
        if ( m_xReportComponent.is() && m_xReportComponent->getPropertySetInfo()->hasPropertyByName(PROPERTY_DATAFIELD) )
            m_xReportComponent->removePropertyChangeListener(PROPERTY_DATAFIELD,static_cast< beans::XPropertyChangeListener* >( this ));

        const uno::Reference< container::XNameContainer > xObjectAsContainer( _rxInspectee, uno::UNO_QUERY );
        m_xReportComponent.set( xObjectAsContainer->getByName("ReportComponent"), uno::UNO_QUERY );

        const OUString sRowSet("RowSet");
        if ( xObjectAsContainer->hasByName( sRowSet ) )
        {
            const uno::Any aRowSet( xObjectAsContainer->getByName(sRowSet) );
            aRowSet >>= m_xRowSet;
            // forward the rowset to our delegator handler
            uno::Reference< beans::XPropertySet > xProp( m_xFormComponentHandler,uno::UNO_QUERY );
            xProp->setPropertyValue( sRowSet, aRowSet );

            m_aParamNames = getParameterNames( m_xRowSet );
            impl_initFieldList_nothrow(m_aFieldNames);
            if ( m_xReportComponent->getPropertySetInfo()->hasPropertyByName(PROPERTY_DATAFIELD) )
                m_xReportComponent->addPropertyChangeListener(PROPERTY_DATAFIELD,static_cast< beans::XPropertyChangeListener* >( this ));
        }

        const uno::Reference< report::XReportComponent> xReportComponent( m_xReportComponent, uno::UNO_QUERY);
        uno::Reference< report::XSection> xSection( m_xReportComponent, uno::UNO_QUERY );
        if ( !xSection.is() && xReportComponent.is() )
            xSection = xReportComponent->getSection();
        if ( xSection.is() )
            lcl_collectFunctionNames( xSection, m_aFunctionNames );
    }
    catch(const uno::Exception &)
    {
        throw lang::NullPointerException();
    }
    m_xFormComponentHandler->inspect(m_xReportComponent);
}

uno::Any SAL_CALL GeometryHandler::getPropertyValue(const OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue;
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_CONDITIONALPRINTEXPRESSION:
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
        case PROPERTY_ID_DATAFIELD:
            aPropertyValue = m_xReportComponent->getPropertyValue( PropertyName );
            lcl_convertFormulaTo(aPropertyValue,aPropertyValue);
            if ( PROPERTY_ID_DATAFIELD == nId )
            {
                OUString sDataField;
                aPropertyValue >>= sDataField;
                switch(m_nDataFieldType)
                {
                    case DATA_OR_FORMULA:
                        break;
                    case FUNCTION:
                        if ( isDefaultFunction(sDataField,sDataField) )
                            aPropertyValue <<= sDataField;
                        else if ( sDataField.isEmpty() )
                            aPropertyValue = uno::Any();
                        break;
                    case COUNTER:
                    case USER_DEF_FUNCTION:
                        aPropertyValue = uno::Any();
                        break;
                }

            }
            break;
        case PROPERTY_ID_TYPE:
            {
                const sal_uInt32 nOldDataFieldType = m_nDataFieldType;
                m_nDataFieldType = impl_getDataFieldType_throw();
                if ( UNDEF_DATA == m_nDataFieldType )
                    m_nDataFieldType = nOldDataFieldType;
                aPropertyValue <<= m_nDataFieldType;
            }
            break;
        case PROPERTY_ID_FORMULALIST:
        case PROPERTY_ID_SCOPE:
            {
                uno::Any aDataField = m_xReportComponent->getPropertyValue( PROPERTY_DATAFIELD );
                lcl_convertFormulaTo(aDataField,aDataField);
                OUString sDataField;
                aDataField >>= sDataField;
                switch(m_nDataFieldType)
                {
                    case DATA_OR_FORMULA:
                        break;
                    case FUNCTION:
                        if ( isDefaultFunction(sDataField,sDataField,uno::Reference< report::XFunctionsSupplier>(),true) )
                            aPropertyValue <<= (PROPERTY_ID_FORMULALIST == nId ? m_sDefaultFunction : m_sScope);
                        break;
                    case USER_DEF_FUNCTION:
                        if ( !sDataField.isEmpty() && PROPERTY_ID_FORMULALIST == nId )
                            aPropertyValue = aDataField;
                        break;
                    case COUNTER:
                        if ( PROPERTY_ID_SCOPE == nId && impl_isCounterFunction_throw(sDataField,m_sScope) )
                            aPropertyValue <<= m_sScope;
                        break;
                }

            }
            break;
        case PROPERTY_ID_BACKCOLOR:
        case PROPERTY_ID_CONTROLBACKGROUND:
            {
                aPropertyValue = m_xReportComponent->getPropertyValue( PropertyName );
                sal_Int32 nColor = COL_TRANSPARENT;
                if ( (aPropertyValue >>= nColor) && static_cast<sal_Int32>(COL_TRANSPARENT) == nColor )
                    aPropertyValue.clear();
            }
            break;
        case PROPERTY_ID_MIMETYPE:
            {
                OUString sValue;
                m_xReportComponent->getPropertyValue( PropertyName ) >>= sValue;
                aPropertyValue <<= impl_ConvertMimeTypeToUI_nothrow(sValue);
            }
            break;
        default:
            aPropertyValue = m_xReportComponent->getPropertyValue( PropertyName );
            break;
    }
    return aPropertyValue;
}

void SAL_CALL GeometryHandler::setPropertyValue(const OUString & PropertyName, const uno::Any & Value) throw (uno::RuntimeException, beans::UnknownPropertyException, beans::PropertyVetoException)
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    uno::Any aNewValue = Value;
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    bool bHandled = false;
    switch(nId)
    {
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
            break;
        case PROPERTY_ID_DATAFIELD:
            {
                OBlocker aBlocker(m_bIn);
                m_xReportComponent->setPropertyValue(PropertyName, aNewValue);
                bHandled = true;
                const OUString sOldFunctionName = m_sDefaultFunction;
                const OUString sOldScope = m_sScope;

                uno::Any aPropertyValue;
                lcl_convertFormulaTo(Value,aPropertyValue);
                OUString sDataField;
                aPropertyValue >>= sDataField;

                m_sScope = m_sDefaultFunction = OUString();
                m_xFunction.clear();
                const sal_uInt32 nOldDataFieldType = m_nDataFieldType;
                if ( !sDataField.isEmpty() )
                {
                    if ( isDefaultFunction(sDataField,sDataField,uno::Reference< report::XFunctionsSupplier>(),true) )
                        m_nDataFieldType = FUNCTION;
                    else if ( m_aFunctionNames.find(sDataField) != m_aFunctionNames.end() )
                        m_nDataFieldType = USER_DEF_FUNCTION;
                }

                resetOwnProperties(aGuard,sOldFunctionName,sOldScope,nOldDataFieldType);
            }
            break;
        case PROPERTY_ID_TYPE:
            {
                bHandled = true;
                Value >>= m_nDataFieldType;

                const OUString sOldFunctionName = m_sDefaultFunction;
                const OUString sOldScope = m_sScope;
                m_sDefaultFunction = m_sScope = OUString();

                if ( m_nDataFieldType == COUNTER )
                {
                    impl_setCounterFunction_throw();
                }
                else
                {
                    if ( m_bNewFunction )
                        removeFunction();
                    m_xFunction.clear();
                    OBlocker aBlocker(m_bIn);
                    m_xReportComponent->setPropertyValue(PROPERTY_DATAFIELD,uno::makeAny(OUString()));
                }
                resetOwnProperties(aGuard,sOldFunctionName,sOldScope,m_nDataFieldType);
            }
            break;
        case PROPERTY_ID_FORMULALIST:
            {
                bHandled = true;
                OUString sFunction;
                if ( !(Value >>= sFunction) || sFunction.isEmpty() )
                {
                    if ( m_nDataFieldType == FUNCTION )
                    {
                        m_sDefaultFunction = OUString();
                        if ( m_bNewFunction )
                            removeFunction();
                        m_xFunction.clear();

                        beans::PropertyChangeEvent aEvent;
                        aEvent.PropertyName = PROPERTY_SCOPE;
                        aEvent.OldValue <<= m_sScope;
                        m_sScope = OUString();
                        aEvent.NewValue <<= m_sScope;
                        aGuard.clear();
                        m_aPropertyListeners.notify( aEvent, &beans::XPropertyChangeListener::propertyChange );
                    }
                    else if ( m_nDataFieldType == USER_DEF_FUNCTION )
                    {
                        OBlocker aBlocker(m_bIn);
                        m_xReportComponent->setPropertyValue(PROPERTY_DATAFIELD,uno::makeAny(OUString()));
                    }
                }
                else if ( m_nDataFieldType == USER_DEF_FUNCTION )
                {
                    OUString sDataField;
                    OBlocker aBlocker(m_bIn);
                    const sal_uInt32 nNewDataType = impl_getDataFieldType_throw(sFunction);
                    if ( nNewDataType != UNDEF_DATA && nNewDataType != m_nDataFieldType )
                    {
                        const OUString sOldFunctionName = m_sDefaultFunction;
                        const OUString sOldScope = m_sScope;
                        m_sScope = m_sDefaultFunction = OUString();
                        m_xFunction.clear();
                        if ( nNewDataType == COUNTER )
                            impl_isCounterFunction_throw(sFunction,m_sScope);
                        else
                        {
                            OUString sNamePostFix;
                            const uno::Reference< report::XFunctionsSupplier> xFunctionsSupplier = fillScope_throw(sNamePostFix);
                            isDefaultFunction(sFunction,sDataField,xFunctionsSupplier,true);
                        }
                        const sal_uInt32 nOldDataFieldType = m_nDataFieldType;
                        m_nDataFieldType = nNewDataType;
                        m_xReportComponent->setPropertyValue(PROPERTY_DATAFIELD,uno::makeAny(impl_convertToFormula( uno::makeAny(sFunction))));
                        resetOwnProperties(aGuard,sOldFunctionName,sOldScope,nOldDataFieldType);
                    }
                    else
                        m_xReportComponent->setPropertyValue(PROPERTY_DATAFIELD,uno::makeAny(impl_convertToFormula( uno::makeAny(sFunction))));
                }
                else if ( m_nDataFieldType == FUNCTION )
                {
                    uno::Any aPropertyValue = m_xReportComponent->getPropertyValue(PROPERTY_DATAFIELD);
                    lcl_convertFormulaTo(aPropertyValue,aPropertyValue);
                    OUString sDataField;
                    aPropertyValue >>= sDataField;
                    if ( m_nDataFieldType == FUNCTION && (!isDefaultFunction(sDataField,sDataField) || m_sDefaultFunction != sFunction) )
                    {
                        if ( m_bNewFunction )
                            removeFunction();
                        // function currently does not exist
                        createDefaultFunction(aGuard,sFunction,sDataField);
                        m_sDefaultFunction = sFunction;
                    }
                }
            }

            break;
        case PROPERTY_ID_SCOPE:
            if ( !(Value >>= m_sScope) )
                m_sScope = OUString();
            else
            {
                if ( m_bNewFunction )
                    removeFunction();
                if ( m_nDataFieldType == COUNTER )
                    impl_setCounterFunction_throw();
                else
                {
                    OSL_ENSURE(m_xFunction.is(),"Where is my function gone!");

                    OUString sNamePostFix;
                    const uno::Reference< report::XFunctionsSupplier> xFunctionsSupplier = fillScope_throw(sNamePostFix);

                    OUString sQuotedFunctionName(lcl_getQuotedFunctionName(m_xFunction));
                    if ( isDefaultFunction(sQuotedFunctionName,sQuotedFunctionName,xFunctionsSupplier,true) )
                        m_bNewFunction = false;
                    else
                    {
                        OUString sDefaultFunctionName;
                        OUString sDataField;
                        OSL_VERIFY( impl_isDefaultFunction_nothrow(m_xFunction,sDataField,sDefaultFunctionName) );
                        m_sDefaultFunction = sDefaultFunctionName;
                        createDefaultFunction(aGuard,m_sDefaultFunction,sDataField);
                    }
                }
            }
            bHandled = true;
            break;
        case PROPERTY_ID_POSITIONX:
        case PROPERTY_ID_POSITIONY:
        case PROPERTY_ID_HEIGHT:
        case PROPERTY_ID_WIDTH:
            {
                const uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY);
                if ( xSourceReportComponent.is() ) // check only report components
                {
                    sal_Int32 nNewValue = 0;
                    Value >>= nNewValue;
                    OSL_ENSURE(nNewValue >= 0, "A position/dimension should not be negative!");
                    nNewValue = lcl_round<sal_Int32, 10>(nNewValue);
                    awt::Point aAwtPoint = xSourceReportComponent->getPosition();
                    awt::Size aAwtSize = xSourceReportComponent->getSize();
                    if ( nId == PROPERTY_ID_POSITIONX )
                        aAwtPoint.X = nNewValue;
                    else if ( nId == PROPERTY_ID_POSITIONY )
                        aAwtPoint.Y = nNewValue;
                    else if ( nId == PROPERTY_ID_HEIGHT )
                        aAwtSize.Height = nNewValue;
                    else if ( nId == PROPERTY_ID_WIDTH )
                        aAwtSize.Width = nNewValue;

                    checkPosAndSize(aAwtPoint,aAwtSize);
                }
            }
            break;
        case PROPERTY_ID_FONT:
            {
                const uno::Reference< report::XReportControlFormat > xReportControlFormat( m_xReportComponent,uno::UNO_QUERY_THROW );
                uno::Sequence< beans::NamedValue > aFontSettings;
                OSL_VERIFY( Value >>= aFontSettings );
                applyCharacterSettings( xReportControlFormat, aFontSettings );
                bHandled = true;
            }
            break;
        case PROPERTY_ID_MIMETYPE:
            {
                OUString sValue;
                Value >>= sValue;
                aNewValue <<= impl_ConvertUIToMimeType_nothrow(sValue);
            }
        default:
            break;
    }

    if ( !bHandled )
        m_xReportComponent->setPropertyValue(PropertyName, aNewValue);
}

// -----------------------------------------------------------------------------
beans::PropertyState SAL_CALL GeometryHandler::getPropertyState(const OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_xFormComponentHandler->getPropertyState(PropertyName);
}
// -----------------------------------------------------------------------------
void GeometryHandler::implCreateListLikeControl(
        const uno::Reference< inspection::XPropertyControlFactory >& _rxControlFactory
        ,inspection::LineDescriptor & out_Descriptor
        ,sal_uInt16 _nResId
        ,sal_Bool _bReadOnlyControl
        ,sal_Bool _bTrueIfListBoxFalseIfComboBox
    )
{
    ::std::vector< OUString > aList;
    tools::StringListResource aRes(ModuleRes(_nResId),aList);

    implCreateListLikeControl(_rxControlFactory,out_Descriptor,aList,_bReadOnlyControl,_bTrueIfListBoxFalseIfComboBox);
}
// -----------------------------------------------------------------------------
void GeometryHandler::implCreateListLikeControl(
        const uno::Reference< inspection::XPropertyControlFactory >& _rxControlFactory
        ,inspection::LineDescriptor & out_Descriptor
        ,const ::std::vector< OUString>& _aEntries
        ,sal_Bool _bReadOnlyControl
        ,sal_Bool _bTrueIfListBoxFalseIfComboBox
    )
{
    const uno::Reference< inspection::XStringListControl > xListControl(
        _rxControlFactory->createPropertyControl(
            _bTrueIfListBoxFalseIfComboBox ? inspection::PropertyControlType::ListBox : inspection::PropertyControlType::ComboBox, _bReadOnlyControl
        ),
        uno::UNO_QUERY_THROW
    );

    out_Descriptor.Control = xListControl.get();
    ::std::for_each( _aEntries.begin(), _aEntries.end(),::boost::bind( &inspection::XStringListControl::appendListEntry, xListControl,_1 ));
}
// -----------------------------------------------------------------------------

inspection::LineDescriptor SAL_CALL GeometryHandler::describePropertyLine(const OUString & PropertyName, const uno::Reference< inspection::XPropertyControlFactory > & _xControlFactory) throw (beans::UnknownPropertyException, lang::NullPointerException,uno::RuntimeException)
{
    inspection::LineDescriptor aOut;
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_FORCENEWPAGE:
        case PROPERTY_ID_NEWROWORCOL:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_FORCENEWPAGE_CONST,sal_False,sal_True);
            break;
        case PROPERTY_ID_GROUPKEEPTOGETHER:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_GROUPKEEPTOGETHER_CONST,sal_False,sal_True);
            break;
        case PROPERTY_ID_PAGEHEADEROPTION:
        case PROPERTY_ID_PAGEFOOTEROPTION:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_REPORTPRINTOPTION_CONST,sal_False,sal_True);
            break;
        case PROPERTY_ID_FORMULALIST:
            {
                ::std::vector< OUString > aList;
                impl_fillFormulaList_nothrow(aList);
                implCreateListLikeControl(_xControlFactory,aOut,aList,sal_False,sal_True);
            }
            break;
        case PROPERTY_ID_SCOPE:
            {
                ::std::vector< OUString > aList;
                impl_fillScopeList_nothrow(aList);
                implCreateListLikeControl(_xControlFactory,aOut,aList,sal_False,sal_True);
            }
            break;
        case PROPERTY_ID_MIMETYPE:
            {
                ::std::vector< OUString > aList;
                impl_fillMimeTypes_nothrow(aList);
                implCreateListLikeControl(_xControlFactory,aOut,aList,sal_False,sal_True);
            }
            break;
        case PROPERTY_ID_TYPE:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_TYPE_CONST,sal_False,sal_True);
            break;
        case PROPERTY_ID_VISIBLE:
        case PROPERTY_ID_CANGROW:
        case PROPERTY_ID_CANSHRINK:
        case PROPERTY_ID_REPEATSECTION:
        case PROPERTY_ID_PRINTREPEATEDVALUES:
        case PROPERTY_ID_STARTNEWCOLUMN:
        case PROPERTY_ID_RESETPAGENUMBER:
        case PROPERTY_ID_PRINTWHENGROUPCHANGE:
        case PROPERTY_ID_KEEPTOGETHER:
        case PROPERTY_ID_DEEPTRAVERSING:
        case PROPERTY_ID_PREEVALUATED:
        case PROPERTY_ID_PRESERVEIRI:
        case PROPERTY_ID_BACKTRANSPARENT:
        case PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT:
            {
                sal_uInt16 nResId = RID_STR_BOOL;
                if ( PROPERTY_ID_KEEPTOGETHER == nId && uno::Reference< report::XGroup>(m_xReportComponent,uno::UNO_QUERY).is())
                    nResId = RID_STR_KEEPTOGETHER_CONST;
                implCreateListLikeControl(_xControlFactory,aOut,nResId,sal_False,sal_True);
            }
            break;
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
            aOut.PrimaryButtonId = OUString::createFromAscii(UID_RPT_PROP_FORMULA);
            aOut.HasPrimaryButton = sal_True;
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::MultiLineTextField , sal_False);
            break;
        case PROPERTY_ID_CONDITIONALPRINTEXPRESSION:
            aOut.PrimaryButtonId = OUString::createFromAscii(UID_RPT_PROP_FORMULA);
            aOut.HasPrimaryButton = sal_True;
            aOut.Control = _xControlFactory->createPropertyControl(inspection::PropertyControlType::MultiLineTextField , sal_False);
            break;
        case PROPERTY_ID_DATAFIELD:
            {
                uno::Reference< inspection::XStringListControl > xListControl(
                    _xControlFactory->createPropertyControl(
                        m_nDataFieldType == DATA_OR_FORMULA ? inspection::PropertyControlType::ComboBox : inspection::PropertyControlType::ListBox, sal_False
                    ),
                    uno::UNO_QUERY_THROW
                );

                if ( m_nDataFieldType == DATA_OR_FORMULA )
                {
                    aOut.PrimaryButtonId = OUString::createFromAscii(UID_RPT_PROP_FORMULA);
                    aOut.HasPrimaryButton = sal_True;
                }

                aOut.Control = xListControl.get();
                if ( m_nDataFieldType == USER_DEF_FUNCTION )
                {
                    // add function names
                    ::std::for_each( m_aFunctionNames.begin(), m_aFunctionNames.end(),
                        ::o3tl::compose1(
                            ::boost::bind( &inspection::XStringListControl::appendListEntry, xListControl,_1 ),
                            ::o3tl::select1st<TFunctions::value_type>()));
                }
                else
                {
                    ::std::for_each( m_aFieldNames.getConstArray(), m_aFieldNames.getConstArray() + m_aFieldNames.getLength(),
                        ::boost::bind( &inspection::XStringListControl::appendListEntry, xListControl, _1 ) );
                    ::std::for_each( m_aParamNames.getConstArray(), m_aParamNames.getConstArray() + m_aParamNames.getLength(),
                        ::boost::bind( &inspection::XStringListControl::appendListEntry, xListControl, _1 ) );
                }
            }
            break;
        case PROPERTY_ID_BACKCOLOR:
        case PROPERTY_ID_CONTROLBACKGROUND:
            aOut.Control = _xControlFactory->createPropertyControl( inspection::PropertyControlType::ColorListBox, sal_False );
            break;
        case PROPERTY_ID_FONT:
            aOut.PrimaryButtonId = OUString::createFromAscii(UID_RPT_RPT_PROP_DLG_FONT_TYPE);
            aOut.Control = _xControlFactory->createPropertyControl( inspection::PropertyControlType::TextField, sal_True );
            aOut.HasPrimaryButton = sal_True;
            break;
        case PROPERTY_ID_AREA:
            aOut.PrimaryButtonId = OUString::createFromAscii(UID_RPT_RPT_PROP_DLG_AREA);
            aOut.Control = _xControlFactory->createPropertyControl( inspection::PropertyControlType::TextField, sal_True );
            aOut.HasPrimaryButton = sal_True;
            break;
        case PROPERTY_ID_VERTICALALIGN:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_VERTICAL_ALIGN_CONST,sal_False,sal_True);
            break;
        case PROPERTY_ID_PARAADJUST:
            implCreateListLikeControl(_xControlFactory,aOut,RID_STR_PARAADJUST_CONST,sal_False,sal_True);
            break;
        default:
            {
            aOut = m_xFormComponentHandler->describePropertyLine(PropertyName, _xControlFactory);
            }
    }

    if ( nId != -1 )
    {
        aOut.Category = ((m_pInfoService->getPropertyUIFlags(nId ) & PROP_FLAG_DATA_PROPERTY) != 0) ?
                                    OUString("Data")
                                                        :
                                    OUString("General");
        aOut.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( nId ) );
        aOut.DisplayName = m_pInfoService->getPropertyTranslation(nId);
    }

    if  (  ( nId == PROPERTY_ID_POSITIONX )
        || ( nId == PROPERTY_ID_POSITIONY )
        || ( nId == PROPERTY_ID_WIDTH )
        || ( nId == PROPERTY_ID_HEIGHT )
        )
    {
        const MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
        const sal_Int16 nDisplayUnit = VCLUnoHelper::ConvertToMeasurementUnit( MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH, 1 );
        uno::Reference< inspection::XNumericControl > xNumericControl(aOut.Control,uno::UNO_QUERY);
        xNumericControl->setDecimalDigits( 2 );
        xNumericControl->setValueUnit( util::MeasureUnit::MM_100TH );
        uno::Reference< drawing::XShapeDescriptor> xShapeDesc(m_xReportComponent,uno::UNO_QUERY);
        bool bSetMin = !xShapeDesc.is() || xShapeDesc->getShapeType() != OUString("com.sun.star.drawing.CustomShape");
        if ( bSetMin )
            xNumericControl->setMinValue(beans::Optional<double>(sal_True,0.0));
        if ( nDisplayUnit != -1 )
            xNumericControl->setDisplayUnit( nDisplayUnit );
        uno::Reference< report::XReportComponent> xComp(m_xReportComponent,uno::UNO_QUERY);
        if ( xComp.is() && xComp->getSection().is() )
        {
            uno::Reference< report::XReportDefinition > xReport = xComp->getSection()->getReportDefinition();
            OSL_ENSURE(xReport.is(),"Why is the report definition NULL!");
            if ( xReport.is() )
            {
                const awt::Size aSize = getStyleProperty<awt::Size>(xReport,PROPERTY_PAPERSIZE);
                const sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReport,PROPERTY_LEFTMARGIN);
                const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReport,PROPERTY_RIGHTMARGIN);
                switch(nId)
                {
                    case PROPERTY_ID_POSITIONX:
                    case PROPERTY_ID_POSITIONY:
                    case PROPERTY_ID_WIDTH:
                        if ( bSetMin )
                            xNumericControl->setMinValue(beans::Optional<double>(sal_True,0.0));
                        xNumericControl->setMaxValue(beans::Optional<double>(sal_True,double(aSize.Width - nLeftMargin - nRightMargin)));
                        if ( PROPERTY_ID_WIDTH == nId )
                        {
                            uno::Reference<report::XFixedLine> xFixedLine(m_xReportComponent,uno::UNO_QUERY);
                            if ( xFixedLine.is() && xFixedLine->getOrientation() == 1 ) // vertical
                                xNumericControl->setMinValue(beans::Optional<double>(sal_True,0.08 ));
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        else if ( PROPERTY_ID_HEIGHT == nId )
        {
            const uno::Reference< report::XSection> xSection(m_xReportComponent,uno::UNO_QUERY);
            if ( xSection.is() )
            {
                sal_Int32 nHeight = 0;
                const sal_Int32 nCount = xSection->getCount();
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    uno::Reference<drawing::XShape> xShape(xSection->getByIndex(i),uno::UNO_QUERY);
                    nHeight = ::std::max<sal_Int32>(nHeight,xShape->getPosition().Y + xShape->getSize().Height);
                }
                xNumericControl->setMinValue(beans::Optional<double>(sal_True,nHeight ));
            }
        }
    }
    return aOut;
}
// -----------------------------------------------------------------------------
beans::Property GeometryHandler::getProperty(const OUString & PropertyName)
{
    uno::Sequence< beans::Property > aProps = getSupportedProperties();
    const beans::Property* pIter = aProps.getConstArray();
    const beans::Property* pEnd  = pIter + aProps.getLength();
    const beans::Property* pFind = ::std::find_if(pIter,pEnd,::std::bind2nd(PropertyCompare(),boost::cref(PropertyName)));
    if ( pFind == pEnd )
        return beans::Property();
    return *pFind;
}
uno::Any GeometryHandler::getConstantValue(sal_Bool _bToControlValue,sal_uInt16 _nResId,const uno::Any& _aValue,const OUString& _sConstantName,const OUString & PropertyName )
{
    ::std::vector< OUString > aList;
    tools::StringListResource aRes(ModuleRes(_nResId),aList);
    uno::Sequence< OUString > aSeq(aList.size());
    ::std::copy( aList.begin(), aList.end(), aSeq.getArray() );

    uno::Reference< inspection::XStringRepresentation > xConversionHelper = inspection::StringRepresentation::createConstant( m_xContext,m_xTypeConverter,_sConstantName,aSeq);
    if ( _bToControlValue )
    {
        return uno::makeAny( xConversionHelper->convertToControlValue( _aValue ) );
    }
    else
    {
        OUString sControlValue;
        _aValue >>= sControlValue;
        const beans::Property aProp = getProperty(PropertyName);
        return xConversionHelper->convertToPropertyValue( sControlValue, aProp.Type );
    }
}

uno::Any SAL_CALL GeometryHandler::convertToPropertyValue(const OUString & PropertyName, const uno::Any & _rControlValue) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Any aPropertyValue( _rControlValue );
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_FORCENEWPAGE:
        case PROPERTY_ID_NEWROWORCOL:
            aPropertyValue = getConstantValue(sal_False,RID_STR_FORCENEWPAGE_CONST,_rControlValue,OUString("com.sun.star.report.ForceNewPage"),PropertyName);
            break;
        case PROPERTY_ID_GROUPKEEPTOGETHER:
            aPropertyValue = getConstantValue(sal_False,RID_STR_GROUPKEEPTOGETHER_CONST,_rControlValue,OUString("com.sun.star.report.GroupKeepTogether"),PropertyName);
            break;
        case PROPERTY_ID_PAGEHEADEROPTION:
        case PROPERTY_ID_PAGEFOOTEROPTION:
            aPropertyValue = getConstantValue(sal_False,RID_STR_REPORTPRINTOPTION_CONST,_rControlValue,OUString("com.sun.star.report.ReportPrintOption"),PropertyName);
            break;
        case PROPERTY_ID_BACKCOLOR:
        case PROPERTY_ID_CONTROLBACKGROUND:
            if ( !_rControlValue.hasValue() )
            {
                aPropertyValue <<= static_cast<sal_Int32>(COL_TRANSPARENT);
                break;
            }
            // run through

        case PROPERTY_ID_KEEPTOGETHER:
            if ( uno::Reference< report::XGroup>(m_xReportComponent,uno::UNO_QUERY).is())
            {
                aPropertyValue = getConstantValue(sal_False,RID_STR_KEEPTOGETHER_CONST,_rControlValue,OUString("com.sun.star.report.KeepTogether"),PropertyName);
                break;
            }
            // run through

        case PROPERTY_ID_VISIBLE:
        case PROPERTY_ID_CANGROW:
        case PROPERTY_ID_CANSHRINK:
        case PROPERTY_ID_REPEATSECTION:
        case PROPERTY_ID_PRINTREPEATEDVALUES:
        case PROPERTY_ID_STARTNEWCOLUMN:
        case PROPERTY_ID_RESETPAGENUMBER:
        case PROPERTY_ID_PRINTWHENGROUPCHANGE:
        case PROPERTY_ID_DEEPTRAVERSING:
        case PROPERTY_ID_PREEVALUATED:
        case PROPERTY_ID_PRESERVEIRI:
        case PROPERTY_ID_BACKTRANSPARENT:
        case PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT:
        {
            if ( aPropertyValue.hasValue() )
            {
                const beans::Property aProp = getProperty(PropertyName);
                if ( aPropertyValue.getValueType().equals( aProp.Type ) )
                    // nothing to do, type is already as desired
                    return aPropertyValue;

                if ( _rControlValue.getValueType().getTypeClass() == uno::TypeClass_STRING )
                {
                    OUString sControlValue;
                    _rControlValue >>= sControlValue;

                    const uno::Reference< inspection::XStringRepresentation > xConversionHelper = inspection::StringRepresentation::create( m_xContext,m_xTypeConverter );
                    aPropertyValue = xConversionHelper->convertToPropertyValue( sControlValue, aProp.Type );
                }
                else
                {
                    try
                    {
                        aPropertyValue = m_xTypeConverter->convertTo( _rControlValue, aProp.Type );
                    }
                    catch( const uno::Exception& )
                    {
                        OSL_FAIL( "GeometryHandler::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
                    }
                }
            }

            break;
        }
        case PROPERTY_ID_CONDITIONALPRINTEXPRESSION:
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
            return uno::makeAny( impl_convertToFormula( _rControlValue ) );
        case PROPERTY_ID_DATAFIELD:
            {
                OUString sDataField;
                _rControlValue >>= sDataField;
                if ( isDefaultFunction(sDataField,sDataField) )
                {
                    OSL_ENSURE(m_xFunction.is(),"No function set!");
                    aPropertyValue <<= impl_convertToFormula( uno::makeAny(lcl_getQuotedFunctionName(m_xFunction)) );
                }
                else
                    aPropertyValue <<= impl_convertToFormula( _rControlValue );
            }
            break;
        case PROPERTY_ID_POSITIONX:
            {
                aPropertyValue = m_xFormComponentHandler->convertToPropertyValue(PropertyName, _rControlValue);
                sal_Int32 nPosX = 0;
                aPropertyValue >>= nPosX;
                const uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY);
                if ( xSourceReportComponent->getSection().is() )
                    nPosX += getStyleProperty<sal_Int32>(xSourceReportComponent->getSection()->getReportDefinition(),PROPERTY_LEFTMARGIN);
                aPropertyValue <<= nPosX;
            }
            break;
        case PROPERTY_ID_FONT:
            aPropertyValue = m_xFormComponentHandler->convertToPropertyValue(PROPERTY_FONT, _rControlValue);
            break;
        case PROPERTY_ID_SCOPE:
        case PROPERTY_ID_FORMULALIST:
        case PROPERTY_ID_AREA:
            aPropertyValue = _rControlValue;
            break;
        case PROPERTY_ID_TYPE:
            {
                OUString sValue;
                _rControlValue >>= sValue;
                ::std::vector< OUString > aList;
                tools::StringListResource aRes(ModuleRes(RID_STR_TYPE_CONST),aList);
                ::std::vector< OUString >::iterator aFind = ::std::find(aList.begin(),aList.end(),sValue);
                if ( aFind != aList.end() )
                    aPropertyValue <<= static_cast<sal_uInt32>(aFind - aList.begin());
            }
            break;
        case PROPERTY_ID_MIMETYPE:
            aPropertyValue = _rControlValue;
            break;
        case PROPERTY_ID_VERTICALALIGN:
            {
                OUString sValue;
                _rControlValue >>= sValue;
                ::std::vector< OUString > aList;
                tools::StringListResource aRes(ModuleRes(RID_STR_VERTICAL_ALIGN_CONST),aList);
                ::std::vector< OUString >::iterator aFind = ::std::find(aList.begin(),aList.end(),sValue);
                if ( aFind != aList.end() )
                    aPropertyValue <<= static_cast<style::VerticalAlignment>(aFind - aList.begin());
            }
            break;
        case PROPERTY_ID_PARAADJUST:
            {
                OUString sValue;
                _rControlValue >>= sValue;
                ::std::vector< OUString > aList;
                tools::StringListResource aRes(ModuleRes(RID_STR_PARAADJUST_CONST),aList);
                ::std::vector< OUString >::iterator aFind = ::std::find(aList.begin(),aList.end(),sValue);
                if ( aFind != aList.end() )
                    aPropertyValue <<= static_cast<sal_Int16>(aFind - aList.begin());
            }
            break;
        default:
            return m_xFormComponentHandler->convertToPropertyValue(PropertyName, _rControlValue);
    }
    return aPropertyValue;
}

uno::Any SAL_CALL GeometryHandler::convertToControlValue(const OUString & PropertyName, const uno::Any & _rPropertyValue, const uno::Type & _rControlValueType) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    uno::Any aControlValue( _rPropertyValue );
    if ( !aControlValue.hasValue() )
        // NULL is converted to NULL
        return aControlValue;

    uno::Any aPropertyValue(_rPropertyValue);

    ::osl::MutexGuard aGuard( m_aMutex );
    const sal_Int32 nId = m_pInfoService->getPropertyId(PropertyName);
    switch(nId)
    {
        case PROPERTY_ID_AREA:
            break;
        case PROPERTY_ID_FORCENEWPAGE:
        case PROPERTY_ID_NEWROWORCOL:
            aControlValue = getConstantValue(sal_True,RID_STR_FORCENEWPAGE_CONST,aPropertyValue,OUString("com.sun.star.report.ForceNewPage"),PropertyName);
            break;
        case PROPERTY_ID_GROUPKEEPTOGETHER:
            aControlValue = getConstantValue(sal_True,RID_STR_GROUPKEEPTOGETHER_CONST,aPropertyValue,OUString("com.sun.star.report.GroupKeepTogether"),PropertyName);
            break;
        case PROPERTY_ID_PAGEHEADEROPTION:
        case PROPERTY_ID_PAGEFOOTEROPTION:
            aControlValue = getConstantValue(sal_True,RID_STR_REPORTPRINTOPTION_CONST,aPropertyValue,OUString("com.sun.star.report.ReportPrintOption"),PropertyName);
            break;
        case PROPERTY_ID_KEEPTOGETHER:
            if ( uno::Reference< report::XGroup>(m_xReportComponent,uno::UNO_QUERY).is())
            {
                aControlValue = getConstantValue(sal_True,RID_STR_KEEPTOGETHER_CONST,aPropertyValue,OUString("com.sun.star.report.KeepTogether"),PropertyName);
                break;
            }
            // run through
        case PROPERTY_ID_VISIBLE:
        case PROPERTY_ID_CANGROW:
        case PROPERTY_ID_CANSHRINK:
        case PROPERTY_ID_REPEATSECTION:
        case PROPERTY_ID_PRINTREPEATEDVALUES:
        case PROPERTY_ID_STARTNEWCOLUMN:
        case PROPERTY_ID_RESETPAGENUMBER:
        case PROPERTY_ID_PRINTWHENGROUPCHANGE:
        case PROPERTY_ID_DEEPTRAVERSING:
        case PROPERTY_ID_PREEVALUATED:
        case PROPERTY_ID_PRESERVEIRI:
        case PROPERTY_ID_BACKTRANSPARENT:
        case PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT:
        {
            if ( _rControlValueType.getTypeClass() == uno::TypeClass_STRING )
            {
                const uno::Reference< inspection::XStringRepresentation > xConversionHelper = inspection::StringRepresentation::create( m_xContext,m_xTypeConverter );
                aControlValue <<= xConversionHelper->convertToControlValue( aPropertyValue );
            }
            else
            {
                try
                {
                    aControlValue = m_xTypeConverter->convertTo( aPropertyValue, _rControlValueType );
                }
                catch( const uno::Exception& )
                {
                    OSL_FAIL( "GeometryHandler::convertToControlValue: caught an exception while converting via TypeConverter!" );
                }
            }
            break;
        }
        case PROPERTY_ID_CONDITIONALPRINTEXPRESSION:
        case PROPERTY_ID_INITIALFORMULA:
        case PROPERTY_ID_FORMULA:
            lcl_convertFormulaTo(aPropertyValue,aControlValue);
            break;
        case PROPERTY_ID_DATAFIELD:
            {
                OUString sValue;
                aControlValue >>= sValue;
                if ( isDefaultFunction(sValue,sValue) )
                    aControlValue <<= sValue;
                else
                    lcl_convertFormulaTo(aPropertyValue,aControlValue);
            }
            break;
        case PROPERTY_ID_FONT:
            aControlValue = m_xFormComponentHandler->convertToControlValue(PROPERTY_FONT, aPropertyValue, _rControlValueType);
            break;
        case PROPERTY_ID_POSITIONX:
            {
                sal_Int32 nPosX = 0;
                aPropertyValue >>= nPosX;
                const uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY);
                if ( xSourceReportComponent->getSection().is() )
                    nPosX -= getStyleProperty<sal_Int32>(xSourceReportComponent->getSection()->getReportDefinition(),PROPERTY_LEFTMARGIN);
                aPropertyValue <<= nPosX;
                aControlValue = m_xFormComponentHandler->convertToControlValue(PropertyName, aPropertyValue, _rControlValueType);
            }
            break;
        case PROPERTY_ID_FORMULALIST:
            aControlValue <<= m_sDefaultFunction;
            break;
        case PROPERTY_ID_SCOPE:
            aControlValue <<= m_sScope;
            break;
        case PROPERTY_ID_MIMETYPE:
            aControlValue = aPropertyValue;
            break;
        case PROPERTY_ID_TYPE:
            {
                ::std::vector< OUString > aList;
                tools::StringListResource aRes(ModuleRes(RID_STR_TYPE_CONST),aList);
                if ( m_nDataFieldType < aList.size() )
                    aControlValue <<= aList[m_nDataFieldType];
            }
            break;
        case PROPERTY_ID_VERTICALALIGN:
            {
                style::VerticalAlignment nParagraphVertAlign = style::VerticalAlignment_TOP;
                aPropertyValue >>= nParagraphVertAlign;
                ::std::vector< OUString > aList;
                tools::StringListResource aRes(ModuleRes(RID_STR_VERTICAL_ALIGN_CONST),aList);
                if ( static_cast<sal_Int16>(nParagraphVertAlign) < static_cast<sal_Int16>(aList.size()) )
                    aControlValue <<= aList[nParagraphVertAlign];
            }
            break;
        case PROPERTY_ID_PARAADJUST:
            {
                sal_Int16 nParagraphAdjust = style::ParagraphAdjust_LEFT;
                aPropertyValue >>= nParagraphAdjust;
                ::std::vector< OUString > aList;
                tools::StringListResource aRes(ModuleRes(RID_STR_PARAADJUST_CONST),aList);
                if ( nParagraphAdjust < static_cast<sal_Int16>(aList.size()) )
                    aControlValue <<= aList[nParagraphAdjust];
            }
            break;
        case PROPERTY_ID_BACKCOLOR:
        case PROPERTY_ID_CONTROLBACKGROUND:
            {
                sal_Int32 nColor = COL_TRANSPARENT;
                if ( (aPropertyValue >>= nColor) && static_cast<sal_Int32>(COL_TRANSPARENT) == nColor )
                    aPropertyValue.clear();
            }
            // run through
        default:
            aControlValue = m_xFormComponentHandler->convertToControlValue(PropertyName, aPropertyValue, _rControlValueType);
    }
    return aControlValue;
}
void SAL_CALL GeometryHandler::addPropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener) throw (uno::RuntimeException, lang::NullPointerException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_aPropertyListeners.addListener( _rxListener );
    m_xFormComponentHandler->addPropertyChangeListener(_rxListener);
}

void SAL_CALL GeometryHandler::removePropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_aPropertyListeners.removeListener( _rxListener );
    m_xFormComponentHandler->removePropertyChangeListener(_rxListener);
}
// -----------------------------------------------------------------------------
//--------------------------------------------------------------------------
uno::Sequence< beans::Property > SAL_CALL GeometryHandler::getSupportedProperties() throw (uno::RuntimeException)
{
    ::std::vector< beans::Property > aNewProps;
    aNewProps.reserve(20); // only a guess
    m_pInfoService->getExcludeProperties( aNewProps, m_xFormComponentHandler );

    const OUString pIncludeProperties[] =
    {
         OUString(PROPERTY_FORCENEWPAGE)
        ,OUString(PROPERTY_KEEPTOGETHER)
        ,OUString(PROPERTY_CANGROW)
        ,OUString(PROPERTY_CANSHRINK)
        ,OUString(PROPERTY_REPEATSECTION)
        ,OUString(PROPERTY_PRINTREPEATEDVALUES)
        ,OUString(PROPERTY_CONDITIONALPRINTEXPRESSION)
        ,OUString(PROPERTY_STARTNEWCOLUMN)
        ,OUString(PROPERTY_RESETPAGENUMBER)
        ,OUString(PROPERTY_PRINTWHENGROUPCHANGE)
        ,OUString(PROPERTY_VISIBLE)
        ,OUString(PROPERTY_PAGEHEADEROPTION)
        ,OUString(PROPERTY_PAGEFOOTEROPTION)
        ,OUString("ControlLabel")
        ,OUString(PROPERTY_POSITIONX)
        ,OUString(PROPERTY_POSITIONY)
        ,OUString(PROPERTY_WIDTH)
        ,OUString(PROPERTY_HEIGHT)
        ,OUString(PROPERTY_PREEVALUATED)
        ,OUString(PROPERTY_DEEPTRAVERSING)
        ,OUString(PROPERTY_FORMULA)
        ,OUString(PROPERTY_INITIALFORMULA)
        ,OUString(PROPERTY_PRESERVEIRI)
        ,OUString(PROPERTY_DATAFIELD)
        ,OUString(PROPERTY_FONT)
        ,OUString(PROPERTY_BACKCOLOR)
        ,OUString(PROPERTY_BACKTRANSPARENT)
        ,OUString(PROPERTY_CONTROLBACKGROUND)
        ,OUString(PROPERTY_CONTROLBACKGROUNDTRANSPARENT)
        ,OUString(PROPERTY_LABEL)
        ,OUString(PROPERTY_MIMETYPE)
        ,OUString(PROPERTY_VERTICALALIGN)
        ,OUString(PROPERTY_PARAADJUST)
    };
    const uno::Reference < beans::XPropertySetInfo > xInfo = m_xReportComponent->getPropertySetInfo();
    const uno::Sequence< beans::Property> aSeq = xInfo->getProperties();
    for (size_t i = 0; i < sizeof(pIncludeProperties)/sizeof(pIncludeProperties[0]) ;++i )
    {
        const beans::Property* pIter = aSeq.getConstArray();
        const beans::Property* pEnd  = pIter + aSeq.getLength();
        const beans::Property* pFind = ::std::find_if(pIter,pEnd,::std::bind2nd(PropertyCompare(),boost::cref(pIncludeProperties[i])));
        if ( pFind != pEnd )
        {
            // special case for controls which contain a data field
            if ( PROPERTY_DATAFIELD == pIncludeProperties[i] )
            {
                beans::Property aValue;
                aValue.Name = PROPERTY_FORMULALIST;
                aNewProps.push_back(aValue);
                aValue.Name = PROPERTY_SCOPE;
                aNewProps.push_back(aValue);
                aValue.Name = PROPERTY_TYPE;
                aNewProps.push_back(aValue);
            }
            aNewProps.push_back(*pFind);
        }
    }

    // special property for shapes
//    if ( uno::Reference< report::XShape>(m_xReportComponent,uno::UNO_QUERY).is() )
//    {
//        beans::Property aValue;
//        aValue.Name = PROPERTY_AREA;
//        aNewProps.push_back(aValue);
//    }
    // re-enable when the remaining issues of #i88727# are fixed

    return uno::Sequence< beans::Property > (&(*aNewProps.begin()),aNewProps.size());
}

uno::Sequence< OUString > SAL_CALL GeometryHandler::getSupersededProperties() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet;
    const uno::Reference<report::XReportDefinition> xReport(m_xReportComponent,uno::UNO_QUERY);
    if ( xReport.is() && !uno::Reference< report::XSection>(xReport->getParent(),uno::UNO_QUERY).is() )
    {
        aRet.realloc(5);
        OUString* pIter = aRet.getArray();
        *pIter++ = PROPERTY_POSITIONX;
        *pIter++ = PROPERTY_POSITIONY;
        *pIter++ = PROPERTY_WIDTH;
        *pIter++ = PROPERTY_HEIGHT;
        *pIter++ = PROPERTY_DATAFIELD;
    }
    return aRet;
}

uno::Sequence< OUString > SAL_CALL GeometryHandler::getActuatingProperties() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Sequence< OUString > aSeq(5);
    aSeq[0] = PROPERTY_BACKTRANSPARENT;
    aSeq[1] = PROPERTY_CONTROLBACKGROUNDTRANSPARENT;
    aSeq[2] = PROPERTY_FORMULALIST;
    aSeq[3] = PROPERTY_TYPE;
    aSeq[4] = PROPERTY_DATAFIELD;

    return ::comphelper::concatSequences(m_xFormComponentHandler->getActuatingProperties(),aSeq);
}

::sal_Bool SAL_CALL GeometryHandler::isComposable(const OUString & _rPropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_pInfoService->isComposable( _rPropertyName, m_xFormComponentHandler );
}

inspection::InteractiveSelectionResult SAL_CALL GeometryHandler::onInteractivePropertySelection(const OUString & PropertyName, ::sal_Bool Primary, uno::Any & _rData, const uno::Reference< inspection::XObjectInspectorUI > & _rxInspectorUI) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::NullPointerException)
{
    if ( !_rxInspectorUI.is() )
        throw lang::NullPointerException();
    if (PropertyName == PROPERTY_FILTER)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        inspection::InteractiveSelectionResult eResult = inspection::InteractiveSelectionResult_Cancelled;
        OUString sClause;
        if ( impl_dialogFilter_nothrow( sClause, aGuard ) )
        {
            _rData <<= sClause;
            eResult = inspection::InteractiveSelectionResult_ObtainedValue;
        }
        return eResult;
    }
    else if (PropertyName == PROPERTY_FONT)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        inspection::InteractiveSelectionResult eResult = inspection::InteractiveSelectionResult_Cancelled;
        const uno::Reference< awt::XWindow> xInspectorWindow(m_xContext->getValueByName("DialogParentWindow") ,uno::UNO_QUERY);
        const uno::Reference< report::XReportControlFormat> xReportControlFormat(m_xReportComponent,uno::UNO_QUERY);
        aGuard.clear();

        uno::Sequence< beans::NamedValue > aFontSettings;
        if ( rptui::openCharDialog( xReportControlFormat, xInspectorWindow, aFontSettings ) )
        {
            _rData <<= aFontSettings;
            eResult = inspection::InteractiveSelectionResult_ObtainedValue;
        }
        return eResult;
    }
    else if (      PropertyName == PROPERTY_FORMULA
                || PropertyName == PROPERTY_INITIALFORMULA
                || PropertyName == PROPERTY_DATAFIELD
                || PropertyName == PROPERTY_CONDITIONALPRINTEXPRESSION)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );


        OUString sFormula;
        m_xReportComponent->getPropertyValue(PropertyName) >>= sFormula;
        const uno::Reference< awt::XWindow> xInspectorWindow(m_xContext->getValueByName("DialogParentWindow") ,uno::UNO_QUERY);
        uno::Reference< uno::XComponentContext > xContext = m_xContext;
        uno::Reference< beans::XPropertySet > xRowSet( m_xRowSet,uno::UNO_QUERY);
        aGuard.clear();

        inspection::InteractiveSelectionResult eResult = inspection::InteractiveSelectionResult_Cancelled;
        if ( rptui::openDialogFormula_nothrow( sFormula, xContext,xInspectorWindow,xRowSet ) )
        {
            _rData <<= sFormula;
            eResult = inspection::InteractiveSelectionResult_ObtainedValue;
        }
        return eResult;
    }
    else if (PropertyName == PROPERTY_AREA)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        inspection::InteractiveSelectionResult eResult = inspection::InteractiveSelectionResult_Cancelled;
        const uno::Reference< awt::XWindow> xInspectorWindow(m_xContext->getValueByName("DialogParentWindow") ,uno::UNO_QUERY);
        const uno::Reference< report::XShape> xShape(m_xReportComponent,uno::UNO_QUERY);
        aGuard.clear();

        if ( rptui::openAreaDialog( xShape, xInspectorWindow) )
        {
            eResult = inspection::InteractiveSelectionResult_ObtainedValue;
            beans::PropertyChangeEvent aScopeEvent;
            aScopeEvent.PropertyName = PROPERTY_FILLCOLOR;
            aScopeEvent.NewValue <<= xShape->getPropertyValue(PROPERTY_FILLCOLOR);
            m_aPropertyListeners.notify( aScopeEvent, &beans::XPropertyChangeListener::propertyChange );
        }
        return eResult;
    }


    return m_xFormComponentHandler->onInteractivePropertySelection(PropertyName, Primary, _rData, _rxInspectorUI);
}

void SAL_CALL GeometryHandler::actuatingPropertyChanged(const OUString & ActuatingPropertyName, const uno::Any & NewValue, const uno::Any & OldValue, const uno::Reference< inspection::XObjectInspectorUI > & _rxInspectorUI, ::sal_Bool _bFirstTimeInit) throw (uno::RuntimeException, lang::NullPointerException)
{
    if ( !_rxInspectorUI.is() )
        throw lang::NullPointerException();

    ::osl::MutexGuard aGuard( m_aMutex );
    const sal_Int32 nId = m_pInfoService->getPropertyId(ActuatingPropertyName);
    switch(nId)
    {
        case PROPERTY_ID_TYPE:
            {
                sal_uInt32 nNewVal = 0;
                NewValue >>= nNewVal;
                switch(nNewVal)
                {
                    case DATA_OR_FORMULA:
                        _rxInspectorUI->rebuildPropertyUI(PROPERTY_DATAFIELD);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_DATAFIELD,sal_True);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_FORMULALIST,sal_False);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_SCOPE,sal_False);
                        OSL_ENSURE(m_sDefaultFunction.isEmpty(),"Why is the m_sDefaultFunction set?");
                        OSL_ENSURE(m_sScope.isEmpty(),"Why is the m_sScope set?");
                        break;
                    case FUNCTION:
                        _rxInspectorUI->rebuildPropertyUI(PROPERTY_DATAFIELD);
                        _rxInspectorUI->rebuildPropertyUI(PROPERTY_FORMULALIST);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_DATAFIELD,sal_True);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_FORMULALIST,!m_sDefaultFunction.isEmpty());
                        _rxInspectorUI->enablePropertyUI(PROPERTY_SCOPE,!m_sScope.isEmpty());
                        break;
                    case USER_DEF_FUNCTION:
                        _rxInspectorUI->enablePropertyUI(PROPERTY_DATAFIELD,sal_False);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_FORMULALIST,sal_True);
                        _rxInspectorUI->rebuildPropertyUI(PROPERTY_FORMULALIST);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_SCOPE,sal_False);
                        break;
                    case COUNTER:
                        _rxInspectorUI->enablePropertyUI(PROPERTY_DATAFIELD,sal_False);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_FORMULALIST,sal_False);
                        _rxInspectorUI->enablePropertyUI(PROPERTY_SCOPE,sal_True);
                        break;
                }
            }
            break;
        case PROPERTY_ID_DATAFIELD:
            {
                sal_Bool bEnable = (m_nDataFieldType != DATA_OR_FORMULA && m_nDataFieldType != COUNTER );
                if ( bEnable )
                {
                    OUString sValue;
                    m_xReportComponent->getPropertyValue( PROPERTY_DATAFIELD ) >>= sValue;
                    bEnable = !sValue.isEmpty();
                }
                _rxInspectorUI->enablePropertyUI(PROPERTY_FORMULALIST,bEnable);
                if ( bEnable )
                {
                    _rxInspectorUI->rebuildPropertyUI(PROPERTY_DATAFIELD);
                    _rxInspectorUI->rebuildPropertyUI(PROPERTY_FORMULALIST);
                }
                m_xFormComponentHandler->actuatingPropertyChanged(ActuatingPropertyName, NewValue, OldValue, _rxInspectorUI, _bFirstTimeInit);
            }
            break;
        case PROPERTY_ID_FORMULALIST:
            {
                _rxInspectorUI->enablePropertyUI(PROPERTY_SCOPE,m_nDataFieldType == FUNCTION || m_nDataFieldType == COUNTER);
            }
            break;
        case PROPERTY_ID_BACKTRANSPARENT:
        case PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT:
            {
                sal_Bool bValue = sal_False;
                NewValue >>= bValue;
                bValue = !bValue;
                _rxInspectorUI->enablePropertyUI(PROPERTY_BACKCOLOR,bValue);
                _rxInspectorUI->enablePropertyUI(PROPERTY_CONTROLBACKGROUND,bValue);
            }
            break;
        default:
            m_xFormComponentHandler->actuatingPropertyChanged(ActuatingPropertyName, NewValue, OldValue, _rxInspectorUI, _bFirstTimeInit);
            break;
    }
}

::sal_Bool SAL_CALL GeometryHandler::suspend(::sal_Bool Suspend) throw (uno::RuntimeException)
{
    return m_xFormComponentHandler->suspend(Suspend);
}
// -----------------------------------------------------------------------------
bool GeometryHandler::impl_dialogFilter_nothrow( OUString& _out_rSelectedClause, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
{
    _out_rSelectedClause = OUString();
    bool bSuccess = false;
    ::dbtools::SQLExceptionInfo aErrorInfo;
    uno::Reference< awt::XWindow > xInspectorWindow;
    uno::Reference< lang::XMultiComponentFactory > xFactory;
    try
    {
        xFactory = m_xContext->getServiceManager();
        xInspectorWindow.set(m_xContext->getValueByName("DialogParentWindow") ,uno::UNO_QUERY);
        uno::Reference<sdbc::XConnection> xCon(m_xContext->getValueByName("ActiveConnection") ,uno::UNO_QUERY);
        if ( !xCon.is() )
            return false;

        uno::Reference< beans::XPropertySet> xRowSetProp(m_xRowSet,uno::UNO_QUERY);
        if ( !m_xRowSet.is() )
        {
            m_xRowSet.set(xFactory->createInstanceWithContext("com.sun.star.sdb.RowSet",m_xContext),uno::UNO_QUERY);
            xRowSetProp.set(m_xRowSet,uno::UNO_QUERY);
            xRowSetProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,uno::makeAny(xCon));
            ::comphelper::copyProperties(m_xReportComponent,xRowSetProp);
        }

        // get a composer for the statement which the form is currently based on
        uno::Reference< sdb::XSingleSelectQueryComposer > xComposer( ::dbtools::getCurrentSettingsComposer( xRowSetProp, m_xContext ) );
        OSL_ENSURE( xComposer.is(), "GeometryHandler::impl_dialogFilter_nothrow: could not obtain a composer!" );
        if ( !xComposer.is() )
            return false;

        // create the dialog
        uno::Reference< ui::dialogs::XExecutableDialog > xDialog = sdb::FilterDialog::createWithQuery(m_xContext, xComposer, m_xRowSet, xInspectorWindow);

        const String aGcc3WorkaroundTemporary( ModuleRes(RID_STR_FILTER));
        const OUString sPropertyUIName( aGcc3WorkaroundTemporary );
        // initialize the dialog
        xDialog->setTitle( sPropertyUIName );

        _rClearBeforeDialog.clear();
        bSuccess = ( xDialog->execute() != 0 );
        if ( bSuccess )
            _out_rSelectedClause = xComposer->getFilter();
    }
    catch (const sdb::SQLContext& e) { aErrorInfo = e; }
    catch (const sdbc::SQLWarning& e) { aErrorInfo = e; }
    catch (const sdbc::SQLException& e) { aErrorInfo = e; }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "GeometryHandler::impl_dialogFilter_nothrow: caught an exception!" );
    }

    if ( aErrorInfo.isValid() )
        ::dbtools::showError( aErrorInfo, xInspectorWindow, m_xContext );

    return bSuccess;
}
// -----------------------------------------------------------------------------
void GeometryHandler::checkPosAndSize(  const awt::Point& _aNewPos,
                                    const awt::Size& _aSize)
{
    const uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY);
    const uno::Reference< report::XSection> xSection(xSourceReportComponent->getParent(),uno::UNO_QUERY);
    if ( !xSection.is() || uno::Reference< report::XShape>(xSourceReportComponent,uno::UNO_QUERY).is() ) // shapes can overlap.
        return;

    ::Point aPos(VCLPoint(_aNewPos));
    if ( aPos.X() < 0 || aPos.Y() < 0 ) // TODO: have to check size with pos aka || (aPos.X() + aAwtSize.Width) > m_xSection->getReportDefinition()->
        throw beans::PropertyVetoException(String(ModuleRes(RID_STR_ILLEGAL_POSITION)),xSourceReportComponent);

    ::Rectangle aSourceRect(aPos,VCLSize(_aSize));

    const sal_Int32 nCount = xSection->getCount();
    for (sal_Int32 i = 0; i < nCount ; ++i)
    {
        const uno::Reference< report::XReportComponent> xReportComponent(xSection->getByIndex(i),uno::UNO_QUERY);
        if ( xReportComponent.is() && xReportComponent != xSourceReportComponent )
        {
            const ::Rectangle aBoundRect(VCLPoint(xReportComponent->getPosition()),VCLSize(xReportComponent->getSize()));
            const ::Rectangle aRect = aSourceRect.GetIntersection(aBoundRect);
            if ( !aRect.IsEmpty() && (aRect.Left() != aRect.Right() && aRect.Top() != aRect.Bottom() ) )
                throw beans::PropertyVetoException(String(ModuleRes( RID_STR_OVERLAP_OTHER_CONTROL)),xSourceReportComponent);
        }
    }
}
// -----------------------------------------------------------------------------
void GeometryHandler::impl_fillFormulaList_nothrow(::std::vector< OUString >& _out_rList) const
{
    if ( m_nDataFieldType == FUNCTION )
        ::std::transform(m_aDefaultFunctions.begin(),m_aDefaultFunctions.end(),::std::back_inserter(_out_rList),::boost::bind( &DefaultFunction::getName, _1 ));
    else if ( m_nDataFieldType == USER_DEF_FUNCTION )
        ::std::transform(m_aFunctionNames.begin(),m_aFunctionNames.end(),::std::back_inserter(_out_rList),::o3tl::select1st<TFunctions::value_type>());
}
// -----------------------------------------------------------------------------
OUString GeometryHandler::impl_ConvertUIToMimeType_nothrow(const OUString& _sUIName) const
{
    ::std::vector< OUString > aList;
    impl_fillMimeTypes_nothrow(aList);
    OUString sRet;
    ::std::vector< OUString >::const_iterator aFind = ::std::find(aList.begin(),aList.end(),_sUIName);
    if ( aFind != aList.end() )
    {
        const sal_Size nPos = aFind - aList.begin();
        const uno::Reference< report::XReportDefinition> xReportDefinition(m_xReportComponent,uno::UNO_QUERY);
        if ( xReportDefinition.is() )
        {
            const uno::Sequence< OUString > aMimeTypes( xReportDefinition->getAvailableMimeTypes() );
            sRet = aMimeTypes[nPos];
        }
    }
    return sRet;
}
// -----------------------------------------------------------------------------
OUString GeometryHandler::impl_ConvertMimeTypeToUI_nothrow(const OUString& _sMimetype) const
{
    ::comphelper::MimeConfigurationHelper aMimeHelper(m_xContext);
    OUString sRet;
    const SfxFilter* pFilter = SfxFilter::GetDefaultFilter( aMimeHelper.GetDocServiceNameFromMediaType(_sMimetype) );
    if ( pFilter )
        sRet = pFilter->GetUIName();
    if ( sRet.isEmpty() )
        sRet = _sMimetype;
    return sRet;
}
// -----------------------------------------------------------------------------
void GeometryHandler::impl_fillMimeTypes_nothrow(::std::vector< OUString >& _out_rList) const
{
    try
    {
        const uno::Reference< report::XReportDefinition> xReportDefinition(m_xReportComponent,uno::UNO_QUERY);
        if ( xReportDefinition.is() )
        {
            uno::Sequence< OUString > aMimeTypes( xReportDefinition->getAvailableMimeTypes() );
            const OUString* pIter = aMimeTypes.getConstArray();
            const OUString* pEnd  = pIter + aMimeTypes.getLength();
            for(;pIter != pEnd; ++pIter)
            {
                const OUString sDocName( impl_ConvertMimeTypeToUI_nothrow(*pIter) );
                if ( !sDocName.isEmpty() )
                    _out_rList.push_back(sDocName);
            }
        }
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Exception caught!");
    }
}
// -----------------------------------------------------------------------------
void GeometryHandler::impl_fillScopeList_nothrow(::std::vector< OUString >& _out_rList) const
{
    try
    {
        const uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY_THROW);
        const uno::Reference< report::XSection> xSection(xSourceReportComponent->getParent(),uno::UNO_QUERY_THROW);

        const uno::Reference< report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();
        const uno::Reference< report::XGroups> xGroups = xReportDefinition->getGroups();
        sal_Int32 nPos = -1;
        uno::Reference< report::XGroup> xGroup = xSection->getGroup();
        if ( xGroup.is() )
            nPos = getPositionInIndexAccess(xGroups.get(),xGroup);
        else if ( xSection == xReportDefinition->getDetail() )
            nPos = xGroups->getCount()-1;

        const String sGroup = String(ModuleRes(RID_STR_SCOPE_GROUP));
        for (sal_Int32 i = 0 ; i <= nPos ; ++i)
        {
            xGroup.set(xGroups->getByIndex(i),uno::UNO_QUERY_THROW);
            String sGroupName = sGroup;
            sGroupName.SearchAndReplaceAscii("%1",xGroup->getExpression());
            _out_rList.push_back(sGroupName);
        }
        _out_rList.push_back(xReportDefinition->getName());
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Exception caught!");
    }
}
// -----------------------------------------------------------------------------
uno::Reference< report::XFunctionsSupplier> GeometryHandler::fillScope_throw(OUString& _rsNamePostFix)
{
    uno::Reference< report::XFunctionsSupplier> xReturn;

    const uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY_THROW);
    const uno::Reference< report::XSection> xSection(xSourceReportComponent->getParent(),uno::UNO_QUERY_THROW);
    const uno::Reference< report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();
    if ( m_sScope.isEmpty() )
    {
        const uno::Reference< report::XGroup> xGroup(xSection->getGroup(),uno::UNO_QUERY);
        if ( xGroup.is() )
        {
            String sGroupName = String(ModuleRes(RID_STR_SCOPE_GROUP));
            _rsNamePostFix = xGroup->getExpression();
            sGroupName.SearchAndReplaceAscii("%1",_rsNamePostFix);
            m_sScope = sGroupName;
            xReturn = xGroup.get();
        }
        else if ( xSection == xReportDefinition->getDetail() )
        {
            const uno::Reference< report::XGroups> xGroups = xReportDefinition->getGroups();
            const sal_Int32 nCount = xGroups->getCount();
            if ( nCount )
            {
                const uno::Reference< report::XGroup> xGroup2(xGroups->getByIndex(nCount - 1),uno::UNO_QUERY_THROW);
                String sGroupName = String(ModuleRes(RID_STR_SCOPE_GROUP));
                _rsNamePostFix = xGroup2->getExpression();
                sGroupName.SearchAndReplaceAscii("%1",_rsNamePostFix);
                m_sScope = sGroupName;
                xReturn = xGroup2.get();
            }
        }
        if ( m_sScope.isEmpty() )
        {
            xReturn = xReportDefinition.get();
            _rsNamePostFix = m_sScope = xReportDefinition->getName();
        }
    }
    else if ( m_sScope == xReportDefinition->getName() )
    {
        xReturn = xReportDefinition.get();
        _rsNamePostFix = m_sScope;
    }
    else
    {
        uno::Reference< report::XGroups> xGroups = xReportDefinition->getGroups();
        const sal_Int32 nCount = xGroups->getCount();

        for (sal_Int32 i = 0 ; i < nCount; ++i)
        {
            const uno::Reference< report::XGroup> xGroup(xGroups->getByIndex(i),uno::UNO_QUERY_THROW);
            String sGroupName = String(ModuleRes(RID_STR_SCOPE_GROUP));
            sGroupName.SearchAndReplaceAscii("%1",xGroup->getExpression());
            if ( m_sScope == OUString(sGroupName) )
            {
                _rsNamePostFix = xGroup->getExpression();
                xReturn = xGroup.get();
                break;
            }
        }

    }
    OSL_ENSURE(xReturn.is(),"Why don't we have a functionssupplier here!");

    return xReturn;
}
// -----------------------------------------------------------------------------
sal_Bool GeometryHandler::isDefaultFunction( const OUString& _sQuotedFunction
                                            ,OUString& _rDataField
                                            ,const uno::Reference< report::XFunctionsSupplier>& _xFunctionsSupplier
                                            ,bool _bSet) const
{
    sal_Bool bDefaultFunction = sal_False;
    try
    {
        const uno::Reference< report::XReportComponent> xSourceReportComponent(m_xReportComponent,uno::UNO_QUERY_THROW);
        const uno::Reference< report::XSection> xSection(xSourceReportComponent->getParent(),uno::UNO_QUERY_THROW);
        const uno::Reference< report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();

        ::std::pair<TFunctions::const_iterator,TFunctions::const_iterator> aFind = m_aFunctionNames.equal_range(_sQuotedFunction);
        while ( aFind.first != aFind.second )
        {
            if ( !_xFunctionsSupplier.is() || _xFunctionsSupplier == aFind.first->second.second )
            {
                const beans::Optional< OUString> aInitalFormula = aFind.first->second.first->getInitialFormula();
                if ( aInitalFormula.IsPresent )
                {
                    OUString sDefaultFunctionName;
                    bDefaultFunction = impl_isDefaultFunction_nothrow(aFind.first->second.first,_rDataField,sDefaultFunctionName);
                    if ( bDefaultFunction )
                    {
                        m_xFunction = aFind.first->second.first;
                        if ( _bSet )
                        {
                            m_sDefaultFunction = sDefaultFunctionName;
                            uno::Reference< report::XGroup> xGroup(aFind.first->second.second,uno::UNO_QUERY);
                            if ( xGroup.is() )
                            {
                                String sGroupName = String(ModuleRes(RID_STR_SCOPE_GROUP));
                                sGroupName.SearchAndReplaceAscii("%1",xGroup->getExpression());
                                m_sScope = sGroupName;
                            }
                            else
                                m_sScope = xReportDefinition->getName();
                        }
                    }
                    break;
                }
            }
            ++(aFind.first);
        }
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Exception caught!");
    }
    return bDefaultFunction;
}
// -----------------------------------------------------------------------------
sal_Bool GeometryHandler::impl_isDefaultFunction_nothrow( const uno::Reference< report::XFunction>& _xFunction
                                            ,OUString& _rDataField
                                            ,OUString& _rsDefaultFunctionName) const
{
    sal_Bool bDefaultFunction = sal_False;
    try
    {
        const String sFormula( _xFunction->getFormula() );
        util::SearchOptions aSearchOptions;
        aSearchOptions.algorithmType = util::SearchAlgorithms_REGEXP;
        aSearchOptions.searchFlag = 0x00000100;
        ::std::vector< DefaultFunction >::const_iterator aIter = m_aDefaultFunctions.begin();
        ::std::vector< DefaultFunction >::const_iterator aDeEnd = m_aDefaultFunctions.end();
        for (; aIter != aDeEnd; ++aIter)
        {
            aSearchOptions.searchString = aIter->m_sSearchString;
            utl::TextSearch aTextSearch(aSearchOptions);
            sal_Int32 start = 0;
            sal_Int32 end = sFormula.Len();
            if ( aTextSearch.SearchForward(sFormula,&start,&end) && start == 0 && end == sFormula.Len()) // default function found
            {
                aSearchOptions.searchString = OUString("\\[[:alpha:]+([:space:]*[:alnum:]*)*\\]");
                utl::TextSearch aDataSearch(aSearchOptions);
                aDataSearch.SearchForward(sFormula,&start,&end );
                ++start;
                _rDataField = sFormula.Copy(start,end-start-1);
                _rsDefaultFunctionName = aIter->m_sName;
                break;
            }
        }

        bDefaultFunction = aIter != aDeEnd;
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Exception caught!");
    }
    return bDefaultFunction;
}
// -----------------------------------------------------------------------------
void GeometryHandler::loadDefaultFunctions()
{
    if ( m_aDefaultFunctions.empty() )
    {
        m_aCounterFunction.m_bPreEvaluated = sal_False;
        m_aCounterFunction.m_bDeepTraversing = sal_False;
        m_aCounterFunction.m_sName = String(ModuleRes(RID_STR_F_COUNTER));
        m_aCounterFunction.m_sFormula = OUString("rpt:[%FunctionName] + 1");
        m_aCounterFunction.m_sSearchString = OUString("rpt:\\[[:alpha:]+([:space:]*[:alnum:]*)*\\][:space:]*\\+[:space:]*[:digit:]*");
        m_aCounterFunction.m_sInitialFormula.IsPresent = sal_True;
        m_aCounterFunction.m_sInitialFormula.Value = OUString("rpt:1");

        DefaultFunction aDefault;
        aDefault.m_bDeepTraversing = sal_False;

        aDefault.m_bPreEvaluated = sal_True;

        aDefault.m_sName = String(ModuleRes(RID_STR_F_ACCUMULATION));
        aDefault.m_sFormula = OUString("rpt:[%Column] + [%FunctionName]");
        aDefault.m_sSearchString = OUString("rpt:\\[[:alpha:]+([:space:]*[:alnum:]*)*\\][:space:]*\\+[:space:]*\\[[:alpha:]+([:space:]*[:alnum:]*)*\\]");
        aDefault.m_sInitialFormula.IsPresent = sal_True;
        aDefault.m_sInitialFormula.Value = OUString("rpt:[%Column]");
        m_aDefaultFunctions.push_back(aDefault);

        aDefault.m_sName = String(ModuleRes(RID_STR_F_MINIMUM));
        aDefault.m_sFormula = OUString("rpt:IF([%Column] < [%FunctionName];[%Column];[%FunctionName])");
        aDefault.m_sSearchString = OUString("rpt:IF\\((\\[[:alpha:]+([:space:]*[:alnum:]*)*\\])[:space:]*<[:space:]*(\\[[:alpha:]+([:space:]*[:alnum:]*)*\\]);[:space:]*\\1[:space:]*;[:space:]*\\3[:space:]*\\)");
        aDefault.m_sInitialFormula.IsPresent = sal_True;
        aDefault.m_sInitialFormula.Value = OUString("rpt:[%Column]");
        m_aDefaultFunctions.push_back(aDefault);

        aDefault.m_sName = String(ModuleRes(RID_STR_F_MAXIMUM));
        aDefault.m_sFormula = OUString("rpt:IF([%Column] > [%FunctionName];[%Column];[%FunctionName])");
        aDefault.m_sSearchString = OUString("rpt:IF\\((\\[[:alpha:]+([:space:]*[:alnum:]*)*\\])[:space:]*>[:space:]*(\\[[:alpha:]+([:space:]*[:alnum:]*)*\\]);[:space:]*\\1[:space:]*;[:space:]*\\3[:space:]*\\)");
        aDefault.m_sInitialFormula.IsPresent = sal_True;
        aDefault.m_sInitialFormula.Value = OUString("rpt:[%Column]");
        m_aDefaultFunctions.push_back(aDefault);
    }
}
// -----------------------------------------------------------------------------
void GeometryHandler::createDefaultFunction(::osl::ResettableMutexGuard& _aGuard ,const OUString& _sFunction,const OUString& _sDataField)
{
    try
    {
        OUString sNamePostFix;
        const uno::Reference< report::XFunctionsSupplier> xFunctionsSupplier = fillScope_throw(sNamePostFix);

        ::std::vector< DefaultFunction >::const_iterator aIter = m_aDefaultFunctions.begin();
        ::std::vector< DefaultFunction >::const_iterator aDeEnd = m_aDefaultFunctions.end();
        for (; aIter != aDeEnd; ++aIter)
        {
            if ( aIter->m_sName == _sFunction )
            {
                const OUString sFunctionName( _sFunction + _sDataField + sNamePostFix);
                const OUString sQuotedFunctionName(lcl_getQuotedFunctionName(sFunctionName));

                beans::PropertyChangeEvent aEvent;
                aEvent.PropertyName = PROPERTY_SCOPE;
                aEvent.OldValue <<= m_sScope;

                ::std::pair<TFunctions::const_iterator,TFunctions::const_iterator> aFind = m_aFunctionNames.equal_range(sQuotedFunctionName);
                while ( aFind.first != aFind.second )
                {
                    if ( xFunctionsSupplier == aFind.first->second.second )
                    {
                        m_xFunction = aFind.first->second.first;
                        OUString sTemp;
                        isDefaultFunction(sQuotedFunctionName,sTemp,uno::Reference< report::XFunctionsSupplier>(),true); // implicitly sets the m_sScope
                        break;
                    }
                    ++(aFind.first);
                }
                if ( aFind.first == aFind.second )
                    impl_createFunction(sFunctionName,_sDataField,*aIter);

                OBlocker aBlocker(m_bIn);
                m_xReportComponent->setPropertyValue(PROPERTY_DATAFIELD,uno::makeAny( impl_convertToFormula( uno::makeAny(sQuotedFunctionName) )));
                aEvent.NewValue <<= m_sScope;
                _aGuard.clear();
                m_aPropertyListeners.notify( aEvent, &beans::XPropertyChangeListener::propertyChange );
                break;
            }
        }
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Exception caught!");
    }
}
// -----------------------------------------------------------------------------
void GeometryHandler::removeFunction()
{
    if ( m_xFunction.is() )
    {
        const OUString sQuotedFunctionName(lcl_getQuotedFunctionName(m_xFunction));
        ::std::pair<TFunctions::iterator,TFunctions::iterator> aFind = m_aFunctionNames.equal_range(sQuotedFunctionName);
        while ( aFind.first != aFind.second )
        {
            if ( aFind.first->second.first == m_xFunction )
            {
                uno::Reference< report::XFunctions> xFunctions = aFind.first->second.second->getFunctions();
                xFunctions->removeByIndex(xFunctions->getCount() - 1 ); /// TODO: insert new method in XFunctions: removeFunction(xfunction)
                m_aFunctionNames.erase(aFind.first);
                m_bNewFunction = false;
                break;
            }
            ++(aFind.first);
        }
    }
}
// -----------------------------------------------------------------------------
void GeometryHandler::resetOwnProperties(::osl::ResettableMutexGuard& _aGuard,const OUString& _sOldFunctionName,const OUString& _sOldScope,const sal_uInt32 _nOldDataFieldType)
{
    const OUString sNewFunction = m_sDefaultFunction;
    const OUString sNewScope = m_sScope;
    const sal_uInt32 nNewDataFieldType = m_nDataFieldType;
    _aGuard.clear();
    if ( _nOldDataFieldType != nNewDataFieldType )
    {
        beans::PropertyChangeEvent aScopeEvent;
        aScopeEvent.PropertyName = PROPERTY_TYPE;
        aScopeEvent.OldValue <<= _nOldDataFieldType;
        aScopeEvent.NewValue <<= nNewDataFieldType;
        m_aPropertyListeners.notify( aScopeEvent, &beans::XPropertyChangeListener::propertyChange );
    }
    if ( _sOldFunctionName != sNewFunction )
    {
        beans::PropertyChangeEvent aFormulaEvent;
        aFormulaEvent.PropertyName = PROPERTY_FORMULALIST;
        aFormulaEvent.OldValue <<= _sOldFunctionName;
        aFormulaEvent.NewValue <<= sNewFunction;

        m_aPropertyListeners.notify( aFormulaEvent, &beans::XPropertyChangeListener::propertyChange );
    }
    if ( _sOldScope != sNewScope )
    {
        beans::PropertyChangeEvent aScopeEvent;
        aScopeEvent.PropertyName = PROPERTY_SCOPE;
        aScopeEvent.OldValue <<= _sOldScope;
        aScopeEvent.NewValue <<= sNewScope;
        m_aPropertyListeners.notify( aScopeEvent, &beans::XPropertyChangeListener::propertyChange );
    }

    _aGuard.reset();
}
//------------------------------------------------------------------------
void GeometryHandler::impl_initFieldList_nothrow( uno::Sequence< OUString >& _rFieldNames ) const
{
    _rFieldNames.realloc(0);
    try
    {
        uno::Reference< awt::XWindow> xInspectorWindow(m_xContext->getValueByName("DialogParentWindow") ,uno::UNO_QUERY);
        Window* pInspectorWindow = VCLUnoHelper::GetWindow( xInspectorWindow );
        WaitObject aWaitCursor( pInspectorWindow );

        uno::Reference< sdbc::XPreparedStatement >  xStatement;

        // get the form of the control we're inspecting
        uno::Reference< beans::XPropertySet > xFormSet( m_xRowSet, uno::UNO_QUERY );
        if ( !xFormSet.is() )
            return;

        OUString sObjectName;
        OSL_VERIFY( xFormSet->getPropertyValue( PROPERTY_COMMAND ) >>= sObjectName );
        // when there is no command we don't need to ask for columns
        uno::Reference<sdbc::XConnection> xCon(m_xContext->getValueByName("ActiveConnection") ,uno::UNO_QUERY);
        if ( !sObjectName.isEmpty() && xCon.is() )
        {
            sal_Int32 nObjectType = sdb::CommandType::COMMAND;
            OSL_VERIFY( xFormSet->getPropertyValue( PROPERTY_COMMANDTYPE ) >>= nObjectType );

            _rFieldNames = ::dbtools::getFieldNamesByCommandDescriptor( xCon, nObjectType, sObjectName );
        }
    }
    catch (uno::Exception&)
    {
        OSL_FAIL( "GeometryHandler::impl_initFieldList_nothrow: caught an exception!" );
    }
}
// -----------------------------------------------------------------------------
bool GeometryHandler::impl_isCounterFunction_throw(const OUString& _sQuotedFunctionName,OUString& _Out_sScope) const
{
    ::std::pair<TFunctions::const_iterator,TFunctions::const_iterator> aFind = m_aFunctionNames.equal_range(_sQuotedFunctionName);
    while ( aFind.first != aFind.second )
    {
        const beans::Optional< OUString> aInitalFormula = aFind.first->second.first->getInitialFormula();
        if ( aInitalFormula.IsPresent )
        {
            const String sFormula( aFind.first->second.first->getFormula() );
            util::SearchOptions aSearchOptions;
            aSearchOptions.algorithmType = util::SearchAlgorithms_REGEXP;
            aSearchOptions.searchFlag = 0x00000100;
            aSearchOptions.searchString = m_aCounterFunction.m_sSearchString;
            utl::TextSearch aTextSearch(aSearchOptions);
            sal_Int32 start = 0;
            sal_Int32 end = sFormula.Len();
            if ( aTextSearch.SearchForward(sFormula,&start,&end) && start == 0 && end == sFormula.Len()) // counter function found
            {
                const uno::Reference< report::XGroup > xGroup(aFind.first->second.second,uno::UNO_QUERY);
                if ( xGroup.is() )
                {
                    String sGroupName = String(ModuleRes(RID_STR_SCOPE_GROUP));
                    sGroupName.SearchAndReplaceAscii("%1",xGroup->getExpression());
                    _Out_sScope = sGroupName;
                }
                else
                    _Out_sScope = uno::Reference< report::XReportDefinition >(aFind.first->second.second,uno::UNO_QUERY_THROW)->getName();
                break;
            }
        }
        ++(aFind.first);
    }
    return aFind.first != aFind.second;
}
// -----------------------------------------------------------------------------
void GeometryHandler::impl_createFunction(const OUString& _sFunctionName,const OUString& _sDataField,const DefaultFunction& _aFunction)
{
    if ( m_bNewFunction )
        removeFunction();

    const OUString sQuotedFunctionName(lcl_getQuotedFunctionName(_sFunctionName));
    m_xFunction.set(report::Function::create(m_xContext));
    m_xFunction->setName( _sFunctionName );

    const String sPlaceHolder1(RTL_CONSTASCII_USTRINGPARAM("%Column"));
    const String sPlaceHolder2(RTL_CONSTASCII_USTRINGPARAM("%FunctionName"));
    String sFormula(_aFunction.m_sFormula);
    sFormula.SearchAndReplaceAll(sPlaceHolder1,_sDataField);
    sFormula.SearchAndReplaceAll(sPlaceHolder2,_sFunctionName);

    m_xFunction->setFormula(sFormula);
    m_xFunction->setPreEvaluated(_aFunction.m_bPreEvaluated);
    m_xFunction->setDeepTraversing(_aFunction.m_bDeepTraversing);
    if ( _aFunction.m_sInitialFormula.IsPresent )
    {
        beans::Optional< OUString> aInitialFormula = _aFunction.m_sInitialFormula;
        String sInitialFormula = aInitialFormula.Value;
        sInitialFormula.SearchAndReplaceAll(sPlaceHolder1,_sDataField);
        sInitialFormula.SearchAndReplaceAll(sPlaceHolder2,_sFunctionName);
        aInitialFormula.Value = sInitialFormula;
        m_xFunction->setInitialFormula( aInitialFormula );
    }
    OUString sNamePostFix;
    const uno::Reference< report::XFunctionsSupplier> xFunctionsSupplier = fillScope_throw(sNamePostFix);
    const uno::Reference< container::XIndexContainer> xFunctions(xFunctionsSupplier->getFunctions(),uno::UNO_QUERY_THROW);
    xFunctions->insertByIndex(xFunctions->getCount(),uno::makeAny(m_xFunction));
    m_aFunctionNames.insert(TFunctions::value_type(sQuotedFunctionName,TFunctionPair(m_xFunction,xFunctionsSupplier)));
    m_bNewFunction = true;
}
// -----------------------------------------------------------------------------
void GeometryHandler::impl_setCounterFunction_throw()
{
    OUString sNamePostFix;
    fillScope_throw(sNamePostFix);
    OUString sFunctionName = m_aCounterFunction.m_sName;
    sFunctionName += sNamePostFix;
    const OUString sQuotedFunctionName = lcl_getQuotedFunctionName(sFunctionName);
    OUString sScope;
    if ( !(!sFunctionName.isEmpty() && m_aFunctionNames.find(sQuotedFunctionName) != m_aFunctionNames.end() && impl_isCounterFunction_throw(sQuotedFunctionName,sScope)) )
        impl_createFunction(sFunctionName,OUString(),m_aCounterFunction);

    OBlocker aBlocker(m_bIn);
    m_xReportComponent->setPropertyValue(PROPERTY_DATAFIELD,uno::makeAny(impl_convertToFormula( uno::makeAny(sQuotedFunctionName))));
}
// -----------------------------------------------------------------------------
sal_uInt32 GeometryHandler::impl_getDataFieldType_throw(const OUString& _sDataField) const
{
    sal_uInt32 nDataFieldType = UNDEF_DATA;
    OUString sDataField;
    if ( !_sDataField.isEmpty() )
        sDataField = _sDataField;
    else
    {
        uno::Any aDataField( m_xReportComponent->getPropertyValue( PROPERTY_DATAFIELD ) );
        lcl_convertFormulaTo(aDataField,aDataField);
        aDataField >>= sDataField;
    }

    if ( !sDataField.isEmpty() )
    {
        if ( impl_isDataField(sDataField) )
            nDataFieldType = DATA_OR_FORMULA;
        else if ( isDefaultFunction(sDataField,sDataField) )
            nDataFieldType = FUNCTION;
        else if ( m_aFunctionNames.find(sDataField) != m_aFunctionNames.end() )
        {
            nDataFieldType = USER_DEF_FUNCTION;
            OUString sScope;
            if ( impl_isCounterFunction_throw(sDataField,sScope) )
                nDataFieldType = COUNTER;
        }
        else
            nDataFieldType = DATA_OR_FORMULA;
    }
    return nDataFieldType;
}
// -----------------------------------------------------------------------------
// XEventListener
void SAL_CALL GeometryHandler::disposing(const lang::EventObject& ) throw( uno::RuntimeException )
{
}
// XPropertyChangeListener
void SAL_CALL GeometryHandler::propertyChange(const beans::PropertyChangeEvent& /*evt*/) throw(uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( !m_bIn )
    {
        const sal_uInt32 nOldDataFieldType = m_nDataFieldType;
        const OUString sOldFunctionName = m_sDefaultFunction;
        const OUString sOldScope = m_sScope;
        m_sDefaultFunction = m_sScope = OUString();
        m_nDataFieldType = impl_getDataFieldType_throw();
        if ( UNDEF_DATA == m_nDataFieldType )
            m_nDataFieldType = nOldDataFieldType;
        uno::Any aDataField = m_xReportComponent->getPropertyValue( PROPERTY_DATAFIELD );
        lcl_convertFormulaTo(aDataField,aDataField);
        OUString sDataField;
        aDataField >>= sDataField;
        switch(m_nDataFieldType)
        {
            case FUNCTION:
                isDefaultFunction(sDataField,sDataField,uno::Reference< report::XFunctionsSupplier>(),true);
                break;
            case COUNTER:
                impl_isCounterFunction_throw(sDataField,m_sScope);
                break;
            default:
                ;
        }

        resetOwnProperties(aGuard,sOldFunctionName,sOldScope,nOldDataFieldType);
    }
}
//........................................................................
} // namespace rptui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
