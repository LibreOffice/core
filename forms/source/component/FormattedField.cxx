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
#include "FormattedField.hxx"
#include <services.hxx>
#include <property.hxx>
#include <frm_resource.hxx>
#include <strings.hrc>
#include <propertybaghelper.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <o3tl/any.hxx>
#include <svl/zforlist.hxx>
#include <svl/numuno.hxx>
#include <vcl/keycod.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/debug.hxx>
#include <tools/wintypes.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/textenc.h>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/form/XSubmit.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <osl/mutex.hxx>
// needed as long as we use the SolarMutex
#include <comphelper/streamsection.hxx>
#include <cppuhelper/weakref.hxx>
#include <unotools/desktopterminationobserver.hxx>
#include <vector>
#include <algorithm>


using namespace dbtools;
using namespace css::uno;
using namespace css::sdb;
using namespace css::sdbc;
using namespace css::sdbcx;
using namespace css::beans;
using namespace css::container;
using namespace css::form;
using namespace css::awt;
using namespace css::io;
using namespace css::lang;
using namespace css::util;
using namespace css::form::binding;

namespace frm
{
class StandardFormatsSupplier : protected SvNumberFormatsSupplierObj, public ::utl::ITerminationListener
{
protected:
            std::unique_ptr<SvNumberFormatter>       m_pMyPrivateFormatter;
    static  WeakReference< XNumberFormatsSupplier >  s_xDefaultFormatsSupplier;
public:
    static Reference< XNumberFormatsSupplier > get( const Reference< XComponentContext >& _rxORB );
protected:
    StandardFormatsSupplier(const Reference< XComponentContext >& _rxFactory,LanguageType _eSysLanguage);
    virtual ~StandardFormatsSupplier() override;
protected:
    virtual bool    queryTermination() const override;
    virtual void    notifyTermination() override;
};
WeakReference< XNumberFormatsSupplier > StandardFormatsSupplier::s_xDefaultFormatsSupplier;
StandardFormatsSupplier::StandardFormatsSupplier(const Reference< XComponentContext > & _rxContext,LanguageType _eSysLanguage)
    :SvNumberFormatsSupplierObj()
    ,m_pMyPrivateFormatter(new SvNumberFormatter(_rxContext, _eSysLanguage))
{
    SetNumberFormatter(m_pMyPrivateFormatter.get());
    // #i29147#
    ::utl::DesktopTerminationObserver::registerTerminationListener( this );
}
StandardFormatsSupplier::~StandardFormatsSupplier()
{
    ::utl::DesktopTerminationObserver::revokeTerminationListener( this );
}
Reference< XNumberFormatsSupplier > StandardFormatsSupplier::get( const Reference< XComponentContext >& _rxORB )
{
    LanguageType eSysLanguage = LANGUAGE_SYSTEM;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        Reference< XNumberFormatsSupplier > xSupplier = s_xDefaultFormatsSupplier;
        if ( xSupplier.is() )
            return xSupplier;
        // get the Office's locale
        eSysLanguage = SvtSysLocale().GetLanguageTag().getLanguageType( false);
    }
    StandardFormatsSupplier* pSupplier = new StandardFormatsSupplier( _rxORB, eSysLanguage );
    Reference< XNumberFormatsSupplier > xNewlyCreatedSupplier( pSupplier );
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        Reference< XNumberFormatsSupplier > xSupplier = s_xDefaultFormatsSupplier;
        if ( xSupplier.is() )
            // somebody used the small time frame where the mutex was not locked to create and set
            // the supplier
            return xSupplier;
        s_xDefaultFormatsSupplier = xNewlyCreatedSupplier;
    }
    return xNewlyCreatedSupplier;
}
bool StandardFormatsSupplier::queryTermination() const
{
    return true;
}
void StandardFormatsSupplier::notifyTermination()
{
    Reference< XNumberFormatsSupplier > xKeepAlive = this;
    // when the application is terminating, release our static reference so that we are cleared/destructed
    // earlier than upon unloading the library
    // #i29147#
    s_xDefaultFormatsSupplier = WeakReference< XNumberFormatsSupplier >( );
    SetNumberFormatter( nullptr );
    m_pMyPrivateFormatter.reset();
}
Sequence<Type> OFormattedControl::_getTypes()
{
    return ::comphelper::concatSequences(
        OFormattedControl_BASE::getTypes(),
        OBoundControl::_getTypes()
    );
}
Any SAL_CALL OFormattedControl::queryAggregation(const Type& _rType)
{
    Any aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OFormattedControl_BASE::queryInterface(_rType);
    return aReturn;
}
OFormattedControl::OFormattedControl(const Reference<XComponentContext>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_FORMATTEDFIELD)
               ,m_nKeyEvent(nullptr)
{
    osl_atomic_increment(&m_refCount);
    {
        Reference<XWindow>  xComp;
        if (query_aggregation(m_xAggregate, xComp))
        {
            xComp->addKeyListener(this);
        }
    }
    osl_atomic_decrement(&m_refCount);
}
OFormattedControl::~OFormattedControl()
{
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}

// XKeyListener
void OFormattedControl::disposing(const EventObject& _rSource)
{
    OBoundControl::disposing(_rSource);
}
void OFormattedControl::keyPressed(const css::awt::KeyEvent& e)
{
    if( e.KeyCode != KEY_RETURN || e.Modifiers != 0 )
        return;
    // Is the control located in a form with a Submit URL?
    Reference<css::beans::XPropertySet>  xSet(getModel(), UNO_QUERY);
    if( !xSet.is() )
        return;
    Reference<XFormComponent>  xFComp(xSet, UNO_QUERY);
    css::uno::Reference<css::uno::XInterface>  xParent = xFComp->getParent();
    if( !xParent.is() )
        return;
    Reference<css::beans::XPropertySet>  xFormSet(xParent, UNO_QUERY);
    if( !xFormSet.is() )
        return;
    Any aTmp(xFormSet->getPropertyValue( PROPERTY_TARGET_URL ));
    if (!aTmp.has<OUString>() ||
        getString(aTmp).isEmpty() )
        return;
    Reference<XIndexAccess>  xElements(xParent, UNO_QUERY);
    sal_Int32 nCount = xElements->getCount();
    if( nCount > 1 )
    {
        Reference<css::beans::XPropertySet>  xFCSet;
        for( sal_Int32 nIndex=0; nIndex < nCount; nIndex++ )
        {
            //  Any aElement(xElements->getByIndex(nIndex));
            xElements->getByIndex(nIndex) >>= xFCSet;
            if (hasProperty(PROPERTY_CLASSID, xFCSet) &&
                getINT16(xFCSet->getPropertyValue(PROPERTY_CLASSID)) == FormComponentType::TEXTFIELD)
            {
                // Found another Edit -> Do not submit then
                if (xFCSet != xSet)
                    return;
            }
        }
    }
    // Because we're still in the Handler, execute submit asynchronously
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );
    m_nKeyEvent = Application::PostUserEvent( LINK(this, OFormattedControl,
                                            OnKeyPressed) );
}

void OFormattedControl::keyReleased(const css::awt::KeyEvent& /*e*/)
{
}

IMPL_LINK_NOARG(OFormattedControl, OnKeyPressed, void*, void)
{
    m_nKeyEvent = nullptr;
    Reference<XFormComponent>  xFComp(getModel(), UNO_QUERY);
    css::uno::Reference<css::uno::XInterface>  xParent = xFComp->getParent();
    Reference<XSubmit>  xSubmit(xParent, UNO_QUERY);
    if (xSubmit.is())
        xSubmit->submit( Reference<XControl> (), css::awt::MouseEvent() );
}

css::uno::Sequence<OUString>  OFormattedControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);
    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_FORMATTEDFIELD;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_FORMATTEDFIELD;
    return aSupported;
}

void OFormattedModel::implConstruct()
{
    // members
    m_bOriginalNumeric = false;
    m_bNumeric = false;
    m_xOriginalFormatter = nullptr;
    m_nKeyType = NumberFormat::UNDEFINED;
    m_aNullDate = DBTypeConversion::getStandardDate();
    // default our formats supplier
    osl_atomic_increment(&m_refCount);
    setPropertyToDefaultByHandle(PROPERTY_ID_FORMATSSUPPLIER);
    osl_atomic_decrement(&m_refCount);
    startAggregatePropertyListening( PROPERTY_FORMATKEY );
    startAggregatePropertyListening( PROPERTY_FORMATSSUPPLIER );
}
OFormattedModel::OFormattedModel(const Reference<XComponentContext>& _rxFactory)
    :OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_FORMATTEDFIELD, FRM_SUN_CONTROL_FORMATTEDFIELD, true, true )
    // use the old control name for compytibility reasons
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
{
    implConstruct();
    m_nClassId = FormComponentType::TEXTFIELD;
    initValueProperty( PROPERTY_EFFECTIVE_VALUE, PROPERTY_ID_EFFECTIVE_VALUE );
}
OFormattedModel::OFormattedModel( const OFormattedModel* _pOriginal, const Reference< XComponentContext >& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
{
    implConstruct();
}

OFormattedModel::~OFormattedModel()
{
}

// XCloneable
IMPLEMENT_DEFAULT_CLONING( OFormattedModel )

void SAL_CALL OFormattedModel::disposing()
{
    OErrorBroadcaster::disposing();
    OEditBaseModel::disposing();
}

// XServiceInfo
css::uno::Sequence<OUString> OFormattedModel::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OEditBaseModel::getSupportedServiceNames();
    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 9 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;
    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;
    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = FRM_SUN_COMPONENT_FORMATTEDFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_FORMATTED_FIELD;
    *pStoreTo++ = FRM_COMPONENT_FORMATTEDFIELD;
    return aSupported;
}

// XAggregation
Any SAL_CALL OFormattedModel::queryAggregation(const Type& _rType)
{
    Any aReturn = OEditBaseModel::queryAggregation( _rType );
    return aReturn.hasValue() ? aReturn : OErrorBroadcaster::queryInterface( _rType );
}

// XTypeProvider
Sequence< Type > OFormattedModel::_getTypes()
{
    return ::comphelper::concatSequences(
        OEditBaseModel::_getTypes(),
        OErrorBroadcaster::getTypes()
    );
}

// XPersistObject
OUString SAL_CALL OFormattedModel::getServiceName()
{
    return OUString(FRM_COMPONENT_EDIT);
}

// XPropertySet
void OFormattedModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 3, OEditBaseModel )
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                          BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,              BOUND);
        DECL_BOOL_PROP2(FILTERPROPOSAL,                         BOUND, MAYBEDEFAULT);
    END_DESCRIBE_PROPERTIES();
}

void OFormattedModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
{
    OEditBaseModel::describeAggregateProperties( _rAggregateProps );
    // TreatAsNumeric is not transient: we want to attach it to the UI
    // This is necessary to make EffectiveDefault (which may be text or a number) meaningful
    ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TREATASNUMERIC, 0, PropertyAttribute::TRANSIENT);
    // Same for FormatKey
    // (though the paragraph above for the TreatAsNumeric does not hold anymore - we do not have an UI for this.
    // But we have for the format key ...)
    ModifyPropertyAttributes(_rAggregateProps, PROPERTY_FORMATKEY, 0, PropertyAttribute::TRANSIENT);
    RemoveProperty(_rAggregateProps, PROPERTY_STRICTFORMAT);
    // no strict format property for formatted fields: it does not make sense, 'cause
    // there is no general way to decide which characters/sub strings are allowed during the input of an
    // arbitrary formatted control
}

void OFormattedModel::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
    {
        Reference<XNumberFormatsSupplier>  xSupplier = calcDefaultFormatsSupplier();
        DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::setPropertyToDefaultByHandle(FORMATSSUPPLIER) : have no aggregate !");
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(xSupplier));
    }
    else
        OEditBaseModel::setPropertyToDefaultByHandle(nHandle);
}

void OFormattedModel::setPropertyToDefault(const OUString& aPropertyName)
{
    OPropertyArrayAggregationHelper& rPH = m_aPropertyBagHelper.getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName( aPropertyName );
    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
        setPropertyToDefaultByHandle(PROPERTY_ID_FORMATSSUPPLIER);
    else
        OEditBaseModel::setPropertyToDefault(aPropertyName);
}

Any OFormattedModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
    {
        Reference<XNumberFormatsSupplier>  xSupplier = calcDefaultFormatsSupplier();
        return makeAny(xSupplier);
    }
    else
        return OEditBaseModel::getPropertyDefaultByHandle(nHandle);
}

Any SAL_CALL OFormattedModel::getPropertyDefault( const OUString& aPropertyName )
{
    OPropertyArrayAggregationHelper& rPH = m_aPropertyBagHelper.getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName( aPropertyName );
    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
        return getPropertyDefaultByHandle(PROPERTY_ID_FORMATSSUPPLIER);
    else
        return OEditBaseModel::getPropertyDefault(aPropertyName);
}

void OFormattedModel::_propertyChanged( const css::beans::PropertyChangeEvent& evt )
{
    // TODO: check how this works with external bindings
    OSL_ENSURE( evt.Source == m_xAggregateSet, "OFormattedModel::_propertyChanged: where did this come from?" );
    if ( evt.Source == m_xAggregateSet )
    {
        if ( evt.PropertyName == PROPERTY_FORMATKEY )
        {
            if ( evt.NewValue.getValueType().getTypeClass() == TypeClass_LONG )
            {
                try
                {
                    ::osl::MutexGuard aGuard( m_aMutex );
                    Reference<XNumberFormatsSupplier> xSupplier( calcFormatsSupplier() );
                    m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), getINT32( evt.NewValue ) );
                    // as m_aSaveValue (which is used by commitControlValueToDbColumn) is format dependent we have
                    // to recalc it, which is done by translateDbColumnToControlValue
                    if ( m_xColumn.is() && m_xAggregateFastSet.is()  && !m_xCursor->isBeforeFirst() && !m_xCursor->isAfterLast())
                    {
                        setControlValue( translateDbColumnToControlValue(), eOther );
                    }
                    // if we're connected to an external value binding, then re-calculate the type
                    // used to exchange the value - it depends on the format, too
                    if ( hasExternalValueBinding() )
                    {
                        calculateExternalValueType();
                    }
                }
                catch(const Exception&)
                {
                }
            }
            return;
        }
        if ( evt.PropertyName == PROPERTY_FORMATSSUPPLIER )
        {
            updateFormatterNullDate();
            return;
        }
        OBoundControlModel::_propertyChanged( evt );
    }
}

void OFormattedModel::updateFormatterNullDate()
{
    // calc the current NULL date
    Reference< XNumberFormatsSupplier > xSupplier( calcFormatsSupplier() );
    if ( xSupplier.is() )
        xSupplier->getNumberFormatSettings()->getPropertyValue("NullDate") >>= m_aNullDate;
}

Reference< XNumberFormatsSupplier > OFormattedModel::calcFormatsSupplier() const
{
    Reference<XNumberFormatsSupplier>  xSupplier;
    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::calcFormatsSupplier : have no aggregate !");
    // Does my aggregate model have a FormatSupplier?
    if( m_xAggregateSet.is() )
        m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER) >>= xSupplier;
    if (!xSupplier.is())
        // check if my parent form has a supplier
        xSupplier = calcFormFormatsSupplier();
    if (!xSupplier.is())
        xSupplier = calcDefaultFormatsSupplier();
    DBG_ASSERT(xSupplier.is(), "OFormattedModel::calcFormatsSupplier : no supplier !");
    // We should have one by now
    return xSupplier;
}

Reference<XNumberFormatsSupplier>  OFormattedModel::calcFormFormatsSupplier() const
{
    Reference<XChild> xMe(
        static_cast<XWeak*>(const_cast<OFormattedModel*>(this)),
        css::uno::UNO_QUERY);
    // By this we make sure that we get the right object even when aggregating
    DBG_ASSERT(xMe.is(), "OFormattedModel::calcFormFormatsSupplier : I should have a content interface !");
    // Iterate through until we reach a StartForm (starting with an own Parent)
    Reference<XChild>  xParent(xMe->getParent(), UNO_QUERY);
    Reference<XForm>  xNextParentForm(xParent, UNO_QUERY);
    while (!xNextParentForm.is() && xParent.is())
    {
        xParent.set(xParent->getParent(), css::uno::UNO_QUERY);
        xNextParentForm.set(xParent, css::uno::UNO_QUERY);
    }
    if (!xNextParentForm.is())
    {
        OSL_FAIL("OFormattedModel::calcFormFormatsSupplier : have no ancestor which is a form !");
        return nullptr;
    }
    // The FormatSupplier of my ancestor (if it has one)
    Reference< XRowSet > xRowSet( xNextParentForm, UNO_QUERY );
    Reference< XNumberFormatsSupplier > xSupplier;
    if (xRowSet.is())
        xSupplier = getNumberFormats( getConnection(xRowSet), true, getContext() );
    return xSupplier;
}

Reference< XNumberFormatsSupplier > OFormattedModel::calcDefaultFormatsSupplier() const
{
    return StandardFormatsSupplier::get( getContext() );
}

// XBoundComponent
void OFormattedModel::loaded(const EventObject& rEvent)
{
    // HACK: our onConnectedDbColumn accesses our NumberFormatter which locks the solar mutex (as it doesn't have
    // an own one). To prevent deadlocks with other threads which may request a property from us in an
    // UI-triggered action (e.g. an tooltip) we lock the solar mutex _here_ before our base class locks
    // its own mutex (which is used for property requests)
    // alternative a): we use two mutexes, one which is passed to the OPropertysetHelper and used for
    // property requests and one for our own code. This would need a lot of code rewriting
    // alternative b): The NumberFormatter has to be really threadsafe (with an own mutex), which is
    // the only "clean" solution for me.
    SolarMutexGuard aGuard;
    OEditBaseModel::loaded(rEvent);
}

void OFormattedModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    m_xOriginalFormatter = nullptr;
    // get some properties of the field
    Reference<XPropertySet> xField = getField();
    sal_Int32 nFormatKey = 0;
    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::onConnectedDbColumn : have no aggregate !");
    if (m_xAggregateSet.is())
    {   // all the following doesn't make any sense if we have no aggregate ...
        Any aSupplier = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER);
        DBG_ASSERT( aSupplier.hasValue(), "OFormattedModel::onConnectedDbColumn : invalid property value !" );
        // This should've been set to the correct value in the ctor or in the read
        Any aFmtKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        if ( !(aFmtKey >>= nFormatKey ) )
        {   // nobody gave us a format to use. So we examine the field we're bound to for a
            // format key, and use it ourself, too
            sal_Int32 nType = DataType::VARCHAR;
            if (xField.is())
            {
                aFmtKey = xField->getPropertyValue(PROPERTY_FORMATKEY);
                xField->getPropertyValue(PROPERTY_FIELDTYPE) >>= nType ;
            }
            Reference<XNumberFormatsSupplier>  xSupplier = calcFormFormatsSupplier();
            DBG_ASSERT(xSupplier.is(), "OFormattedModel::onConnectedDbColumn : bound to a field but no parent with a formatter ? how this ?");
            if (xSupplier.is())
            {
                m_bOriginalNumeric = getBOOL(getPropertyValue(PROPERTY_TREATASNUMERIC));
                if (!aFmtKey.hasValue())
                {   // we aren't bound to a field (or this field's format is invalid)
                    // -> determine the standard text (or numeric) format of the supplier
                    Reference<XNumberFormatTypes>  xTypes(xSupplier->getNumberFormats(), UNO_QUERY);
                    if (xTypes.is())
                    {
                        Locale aApplicationLocale = Application::GetSettings().GetUILanguageTag().getLocale();
                        if (m_bOriginalNumeric)
                            aFmtKey <<= static_cast<sal_Int32>(xTypes->getStandardFormat(NumberFormat::NUMBER, aApplicationLocale));
                        else
                            aFmtKey <<= static_cast<sal_Int32>(xTypes->getStandardFormat(NumberFormat::TEXT, aApplicationLocale));
                    }
                }
                aSupplier >>= m_xOriginalFormatter;
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(xSupplier));
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, aFmtKey);
                // Adapt the NumericFalg to my bound field
                if (xField.is())
                {
                    m_bNumeric = false;
                    switch (nType)
                    {
                        case DataType::BIT:
                        case DataType::BOOLEAN:
                        case DataType::TINYINT:
                        case DataType::SMALLINT:
                        case DataType::INTEGER:
                        case DataType::BIGINT:
                        case DataType::FLOAT:
                        case DataType::REAL:
                        case DataType::DOUBLE:
                        case DataType::NUMERIC:
                        case DataType::DECIMAL:
                        case DataType::DATE:
                        case DataType::TIME:
                        case DataType::TIMESTAMP:
                            m_bNumeric = true;
                            break;
                    }
                }
                else
                    m_bNumeric = m_bOriginalNumeric;
                setPropertyValue(PROPERTY_TREATASNUMERIC, makeAny(m_bNumeric));
                OSL_VERIFY( aFmtKey >>= nFormatKey );
            }
        }
    }
    Reference<XNumberFormatsSupplier>  xSupplier = calcFormatsSupplier();
    m_bNumeric = getBOOL( getPropertyValue( PROPERTY_TREATASNUMERIC ) );
    m_nKeyType  = getNumberFormatType( xSupplier->getNumberFormats(), nFormatKey );
    xSupplier->getNumberFormatSettings()->getPropertyValue("NullDate") >>= m_aNullDate;
    OEditBaseModel::onConnectedDbColumn( _rxForm );
}

void OFormattedModel::onDisconnectedDbColumn()
{
    OEditBaseModel::onDisconnectedDbColumn();
    if (m_xOriginalFormatter.is())
    {   // Our aggregated model does not hold any Format information
        m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(m_xOriginalFormatter));
        m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, Any());
        setPropertyValue(PROPERTY_TREATASNUMERIC, makeAny(m_bOriginalNumeric));
        m_xOriginalFormatter = nullptr;
    }
    m_nKeyType   = NumberFormat::UNDEFINED;
    m_aNullDate  = DBTypeConversion::getStandardDate();
}

void OFormattedModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
{
    OEditBaseModel::write(_rxOutStream);
    _rxOutStream->writeShort(0x0003);
    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::write : have no aggregate !");
    // Bring my Format (may be void) to a persistent Format.
    // The Supplier together with the Key is already persistent, but that doesn't mean
    // we have to save the Supplier (which would be quite some overhead)
    Reference<XNumberFormatsSupplier>  xSupplier;
    Any aFmtKey;
    bool bVoidKey = true;
    if (m_xAggregateSet.is())
    {
        Any aSupplier = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER);
        if (aSupplier.getValueType().getTypeClass() != TypeClass_VOID)
        {
            OSL_VERIFY( aSupplier >>= xSupplier );
        }
        aFmtKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        bVoidKey = (!xSupplier.is() || !aFmtKey.hasValue()) || (isLoaded() && m_xOriginalFormatter.is());
        // (no Format and/or Key) OR (loaded and faked Formatter)
    }
    _rxOutStream->writeBoolean(!bVoidKey);
    if (!bVoidKey)
    {
        // Create persistent values from the FormatKey and the Formatter
        Any aKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        sal_Int32 nKey = aKey.hasValue() ? getINT32(aKey) : 0;
        Reference<XNumberFormats>  xFormats = xSupplier->getNumberFormats();
        OUString         sFormatDescription;
        LanguageType    eFormatLanguage = LANGUAGE_DONTKNOW;
        static const char s_aLocaleProp[] = "Locale";
        Reference<css::beans::XPropertySet>  xFormat = xFormats->getByKey(nKey);
        if (hasProperty(s_aLocaleProp, xFormat))
        {
            Any aLocale = xFormat->getPropertyValue(s_aLocaleProp);
            DBG_ASSERT(aLocale.has<Locale>(), "OFormattedModel::write : invalid language property !");
            if (auto pLocale = o3tl::tryAccess<Locale>(aLocale))
            {
                eFormatLanguage = LanguageTag::convertToLanguageType( *pLocale, false);
            }
        }
        static const char s_aFormatStringProp[] = "FormatString";
        if (hasProperty(s_aFormatStringProp, xFormat))
            xFormat->getPropertyValue(s_aFormatStringProp) >>= sFormatDescription;
        _rxOutStream->writeUTF(sFormatDescription);
        _rxOutStream->writeLong(static_cast<sal_uInt16>(eFormatLanguage));
    }
    // version 2 : write the properties common to all OEditBaseModels
    writeCommonEditProperties(_rxOutStream);
    // version 3 : write the effective value property of the aggregate
    // Due to a bug within the UnoControlFormattedFieldModel implementation (our default aggregate)
    // this props value isn't correctly read and this can't be corrected without being incompatible.
    // so we have our own handling.
    // and to be a little bit more compatible we make the following section skippable
    {
        OStreamSection aDownCompat(_rxOutStream);
        // a sub version within the skippable block
        _rxOutStream->writeShort(0x0000);
        // version 0: the effective value of the aggregate
        Any aEffectiveValue;
        if (m_xAggregateSet.is())
        {
            try { aEffectiveValue = m_xAggregateSet->getPropertyValue(PROPERTY_EFFECTIVE_VALUE); } catch(const Exception&) { }
        }
        {
            OStreamSection aDownCompat2(_rxOutStream);
            switch (aEffectiveValue.getValueType().getTypeClass())
            {
                case TypeClass_STRING:
                    _rxOutStream->writeShort(0x0000);
                    _rxOutStream->writeUTF(::comphelper::getString(aEffectiveValue));
                    break;
                case TypeClass_DOUBLE:
                    _rxOutStream->writeShort(0x0001);
                    _rxOutStream->writeDouble(::comphelper::getDouble(aEffectiveValue));
                    break;
                default:    // void and all unknown states
                    DBG_ASSERT(!aEffectiveValue.hasValue(), "FmXFormattedModel::write : unknown property value type !");
                    _rxOutStream->writeShort(0x0002);
                    break;
            }
        }
    }
}

void OFormattedModel::read(const Reference<XObjectInputStream>& _rxInStream)
{
    OEditBaseModel::read(_rxInStream);
    sal_uInt16 nVersion = _rxInStream->readShort();
    Reference<XNumberFormatsSupplier>  xSupplier;
    sal_Int32 nKey = -1;
    switch (nVersion)
    {
        case 0x0001 :
        case 0x0002 :
        case 0x0003 :
        {
            bool bNonVoidKey = _rxInStream->readBoolean();
            if (bNonVoidKey)
            {
                // read string and language...
                OUString sFormatDescription = _rxInStream->readUTF();
                LanguageType eDescriptionLanguage(_rxInStream->readLong());
                // and let a formatter roll dice based on that to create a key...
                xSupplier = calcFormatsSupplier();
                // calcFormatsSupplier first takes the one from the model, then one from the starform, then a new one...
                Reference<XNumberFormats>  xFormats = xSupplier->getNumberFormats();
                if (xFormats.is())
                {
                    Locale aDescriptionLanguage( LanguageTag::convertToLocale(eDescriptionLanguage));
                    nKey = xFormats->queryKey(sFormatDescription, aDescriptionLanguage, false);
                    if (nKey == sal_Int32(-1))
                    {   // does not yet exist in my formatter...
                        nKey = xFormats->addNew(sFormatDescription, aDescriptionLanguage);
                    }
                }
            }
            if ((nVersion == 0x0002) || (nVersion == 0x0003))
                readCommonEditProperties(_rxInStream);
            if (nVersion == 0x0003)
            {   // since version 3 there is a "skippable" block at this position
                OStreamSection aDownCompat(_rxInStream);
                _rxInStream->readShort(); // sub-version
                // version 0 and higher : the "effective value" property
                Any aEffectiveValue;
                {
                    OStreamSection aDownCompat2(_rxInStream);
                    switch (_rxInStream->readShort())
                    {
                        case 0: // String
                            aEffectiveValue <<= _rxInStream->readUTF();
                            break;
                        case 1: // double
                            aEffectiveValue <<= _rxInStream->readDouble();
                            break;
                        case 2:
                            break;
                        case 3:
                            OSL_FAIL("FmXFormattedModel::read : unknown effective value type !");
                    }
                }
                // this property is only to be set if we have no control source : in all other cases the base class did a
                // reset after it's read and this set the effective value to a default value
                if ( m_xAggregateSet.is() && getControlSource().isEmpty() )
                {
                    try
                    {
                        m_xAggregateSet->setPropertyValue(PROPERTY_EFFECTIVE_VALUE, aEffectiveValue);
                    }
                    catch(const Exception&)
                    {
                    }
                }
            }
        }
        break;
        default :
            OSL_FAIL("OFormattedModel::read : unknown version !");
            // then the format of the aggregated set stay like it was during creation: void
            defaultCommonEditProperties();
            break;
    }
    if ((nKey != -1) && m_xAggregateSet.is())
    {
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(xSupplier));
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, makeAny(nKey));
    }
    else
    {
        setPropertyToDefault(PROPERTY_FORMATSSUPPLIER);
        setPropertyToDefault(PROPERTY_FORMATKEY);
    }
}

sal_uInt16 OFormattedModel::getPersistenceFlags() const
{
    return (OEditBaseModel::getPersistenceFlags() & ~PF_HANDLE_COMMON_PROPS);
    // a) we do our own call to writeCommonEditProperties
}

bool OFormattedModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aControlValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );
    if ( aControlValue != m_aSaveValue )
    {
        // empty string + EmptyIsNull = void
        if  (   !aControlValue.hasValue()
            ||  (   ( aControlValue.getValueType().getTypeClass() == TypeClass_STRING )
                &&  getString( aControlValue ).isEmpty()
                &&  m_bEmptyIsNull
                )
            )
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                double f = 0.0;
                if ( aControlValue.getValueType().getTypeClass() == TypeClass_DOUBLE || (aControlValue >>= f)) // #i110323
                {
                    DBTypeConversion::setValue( m_xColumnUpdate, m_aNullDate, getDouble( aControlValue ), m_nKeyType );
                }
                else
                {
                    DBG_ASSERT( aControlValue.getValueType().getTypeClass() == TypeClass_STRING, "OFormattedModel::commitControlValueToDbColumn: invalid value type!" );
                    m_xColumnUpdate->updateString( getString( aControlValue ) );
                }
            }
            catch(const Exception&)
            {
                return false;
            }
        }
        m_aSaveValue = aControlValue;
    }
    return true;
}

void OFormattedModel::onConnectedExternalValue( )
{
    OEditBaseModel::onConnectedExternalValue();
    updateFormatterNullDate();
}

Any OFormattedModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
{
    Any aControlValue;
    switch( _rExternalValue.getValueTypeClass() )
    {
    case TypeClass_VOID:
        break;
    case TypeClass_STRING:
        aControlValue = _rExternalValue;
        break;
    case TypeClass_BOOLEAN:
    {
        bool bExternalValue = false;
        _rExternalValue >>= bExternalValue;
        aControlValue <<= static_cast<double>( bExternalValue ? 1 : 0 );
    }
    break;
    default:
    {
        if ( _rExternalValue.getValueType().equals( cppu::UnoType< css::util::Date >::get() ) )
        {
            css::util::Date aDate;
            _rExternalValue >>= aDate;
            aControlValue <<= DBTypeConversion::toDouble( aDate, m_aNullDate );
        }
        else if ( _rExternalValue.getValueType().equals( cppu::UnoType< css::util::Time >::get() ) )
        {
            css::util::Time aTime;
            _rExternalValue >>= aTime;
            aControlValue <<= DBTypeConversion::toDouble( aTime );
        }
        else if ( _rExternalValue.getValueType().equals( cppu::UnoType< css::util::DateTime >::get() ) )
        {
            css::util::DateTime aDateTime;
            _rExternalValue >>= aDateTime;
            aControlValue <<= DBTypeConversion::toDouble( aDateTime, m_aNullDate );
        }
        else
        {
            OSL_ENSURE( _rExternalValue.getValueTypeClass() == TypeClass_DOUBLE,
                "OFormattedModel::translateExternalValueToControlValue: don't know how to translate this type!" );
            double fValue = 0;
            OSL_VERIFY( _rExternalValue >>= fValue );
            aControlValue <<= fValue;
        }
    }
    }
    return aControlValue;
}

Any OFormattedModel::translateControlValueToExternalValue( ) const
{
    OSL_PRECOND( hasExternalValueBinding(),
        "OFormattedModel::translateControlValueToExternalValue: precondition not met!" );
    Any aControlValue( getControlValue() );
    if ( !aControlValue.hasValue() )
        return aControlValue;
    Any aExternalValue;
    // translate into the external value type
    Type aExternalValueType( getExternalValueType() );
    switch ( aExternalValueType.getTypeClass() )
    {
    case TypeClass_STRING:
    {
        OUString sString;
        if ( aControlValue >>= sString )
        {
            aExternalValue <<= sString;
            break;
        }
        [[fallthrough]];
    }
    case TypeClass_BOOLEAN:
    {
        double fValue = 0;
        OSL_VERIFY( aControlValue >>= fValue );
        // if this asserts ... well, the somebody set the TreatAsNumeric property to false,
        // and the control value is a string. This implies some weird misconfiguration
        // of the FormattedModel, so we won't care for it for the moment.
        aExternalValue <<= fValue != 0.0;
    }
    break;
    default:
    {
        double fValue = 0;
        OSL_VERIFY( aControlValue >>= fValue );
        // if this asserts ... well, the somebody set the TreatAsNumeric property to false,
        // and the control value is a string. This implies some weird misconfiguration
        // of the FormattedModel, so we won't care for it for the moment.
        if ( aExternalValueType.equals( cppu::UnoType< css::util::Date >::get() ) )
        {
            aExternalValue <<= DBTypeConversion::toDate( fValue, m_aNullDate );
        }
        else if ( aExternalValueType.equals( cppu::UnoType< css::util::Time >::get() ) )
        {
            aExternalValue <<= DBTypeConversion::toTime( fValue );
        }
        else if ( aExternalValueType.equals( cppu::UnoType< css::util::DateTime >::get() ) )
        {
            aExternalValue <<= DBTypeConversion::toDateTime( fValue, m_aNullDate );
        }
        else
        {
            OSL_ENSURE( aExternalValueType.equals( cppu::UnoType< double >::get() ),
                "OFormattedModel::translateControlValueToExternalValue: don't know how to translate this type!" );
            aExternalValue <<= fValue;
        }
    }
    break;
    }
    return aExternalValue;
}

Any OFormattedModel::translateDbColumnToControlValue()
{
    if ( m_bNumeric )
        m_aSaveValue <<= DBTypeConversion::getValue( m_xColumn, m_aNullDate ); // #100056# OJ
    else
        m_aSaveValue <<= m_xColumn->getString();
    if ( m_xColumn->wasNull() )
        m_aSaveValue.clear();
    return m_aSaveValue;
}

Sequence< Type > OFormattedModel::getSupportedBindingTypes()
{
    ::std::vector< Type > aTypes;
    switch ( m_nKeyType & ~NumberFormat::DEFINED )
    {
    case NumberFormat::DATE:
        aTypes.push_back(cppu::UnoType< css::util::Date >::get() );
        break;
    case NumberFormat::TIME:
        aTypes.push_back(cppu::UnoType< css::util::Time >::get() );
        break;
    case NumberFormat::DATETIME:
        aTypes.push_back(cppu::UnoType< css::util::DateTime >::get() );
        break;
    case NumberFormat::TEXT:
        aTypes.push_back(cppu::UnoType< OUString >::get() );
        break;
    case NumberFormat::LOGICAL:
        aTypes.push_back(cppu::UnoType< sal_Bool >::get() );
        break;
    }
    aTypes.push_back( cppu::UnoType< double >::get() );
    return comphelper::containerToSequence(aTypes);
}

Any OFormattedModel::getDefaultForReset() const
{
    return m_xAggregateSet->getPropertyValue( PROPERTY_EFFECTIVE_DEFAULT );
}

void OFormattedModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OFormattedControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OFormattedControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
