/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include "FormattedField.hxx"
#include "services.hxx"
#include "property.hrc"
#include "property.hxx"
#include "frm_resource.hxx"
#include "frm_resource.hrc"
#include "propertybaghelper.hxx"
#include <comphelper/sequence.hxx>
#include <comphelper/numbers.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <svl/zforlist.hxx>
#include <svl/numuno.hxx>
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

#include <comphelper/streamsection.hxx>
#include <cppuhelper/weakref.hxx>
#include <unotools/desktopterminationobserver.hxx>
#include <list>
#include <algorithm>
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::form::binding;
namespace
{
    typedef com::sun::star::util::Date UNODate;
    typedef com::sun::star::util::Time UNOTime;
    typedef com::sun::star::util::DateTime UNODateTime;
}
namespace frm
{
class StandardFormatsSupplier : protected SvNumberFormatsSupplierObj, public ::utl::ITerminationListener
{
protected:
            SvNumberFormatter*                       m_pMyPrivateFormatter;
    static  WeakReference< XNumberFormatsSupplier >  s_xDefaultFormatsSupplier;
public:
    static Reference< XNumberFormatsSupplier > get( const Reference< XComponentContext >& _rxORB );
    using SvNumberFormatsSupplierObj::operator new;
    using SvNumberFormatsSupplierObj::operator delete;
protected:
    StandardFormatsSupplier(const Reference< XComponentContext >& _rxFactory,LanguageType _eSysLanguage);
    ~StandardFormatsSupplier();
protected:
    virtual bool    queryTermination() const;
    virtual void    notifyTermination();
};
WeakReference< XNumberFormatsSupplier > StandardFormatsSupplier::s_xDefaultFormatsSupplier;
StandardFormatsSupplier::StandardFormatsSupplier(const Reference< XComponentContext > & _rxContext,LanguageType _eSysLanguage)
    :SvNumberFormatsSupplierObj()
    ,m_pMyPrivateFormatter(new SvNumberFormatter(_rxContext, _eSysLanguage))
{
    SetNumberFormatter(m_pMyPrivateFormatter);
    
    ::utl::DesktopTerminationObserver::registerTerminationListener( this );
}
StandardFormatsSupplier::~StandardFormatsSupplier()
{
    ::utl::DesktopTerminationObserver::revokeTerminationListener( this );
    DELETEZ( m_pMyPrivateFormatter );
}
Reference< XNumberFormatsSupplier > StandardFormatsSupplier::get( const Reference< XComponentContext >& _rxORB )
{
    LanguageType eSysLanguage = LANGUAGE_SYSTEM;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        Reference< XNumberFormatsSupplier > xSupplier = s_xDefaultFormatsSupplier;
        if ( xSupplier.is() )
            return xSupplier;
        
        eSysLanguage = SvtSysLocale().GetLanguageTag().getLanguageType( false);
    }
    StandardFormatsSupplier* pSupplier = new StandardFormatsSupplier( _rxORB, eSysLanguage );
    Reference< XNumberFormatsSupplier > xNewlyCreatedSupplier( pSupplier );
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        Reference< XNumberFormatsSupplier > xSupplier = s_xDefaultFormatsSupplier;
        if ( xSupplier.is() )
            
            
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
    
    
    
    s_xDefaultFormatsSupplier = WeakReference< XNumberFormatsSupplier >( );
    SetNumberFormatter( NULL );
    DELETEZ( m_pMyPrivateFormatter );
}
InterfaceRef SAL_CALL OFormattedControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormattedControl( comphelper::getComponentContext(_rxFactory) ));
}
Sequence<Type> OFormattedControl::_getTypes()
{
    return ::comphelper::concatSequences(
        OFormattedControl_BASE::getTypes(),
        OBoundControl::_getTypes()
    );
}
Any SAL_CALL OFormattedControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OFormattedControl_BASE::queryInterface(_rType);
    return aReturn;
}
OFormattedControl::OFormattedControl(const Reference<XComponentContext>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_FORMATTEDFIELD)
               ,m_nKeyEvent(0)
{
    increment(m_refCount);
    {
        Reference<XWindow>  xComp;
        if (query_aggregation(m_xAggregate, xComp))
        {
            xComp->addKeyListener(this);
        }
    }
    decrement(m_refCount);
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


void OFormattedControl::disposing(const EventObject& _rSource) throw(RuntimeException)
{
    OBoundControl::disposing(_rSource);
}
void OFormattedControl::keyPressed(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException)
{
    if( e.KeyCode != KEY_RETURN || e.Modifiers != 0 )
        return;
    
    Reference<com::sun::star::beans::XPropertySet>  xSet(getModel(), UNO_QUERY);
    if( !xSet.is() )
        return;
    Reference<XFormComponent>  xFComp(xSet, UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    if( !xParent.is() )
        return;
    Reference<com::sun::star::beans::XPropertySet>  xFormSet(xParent, UNO_QUERY);
    if( !xFormSet.is() )
        return;
    Any aTmp(xFormSet->getPropertyValue( PROPERTY_TARGET_URL ));
    if (!isA(aTmp, static_cast< OUString* >(NULL)) ||
        getString(aTmp).isEmpty() )
        return;
    Reference<XIndexAccess>  xElements(xParent, UNO_QUERY);
    sal_Int32 nCount = xElements->getCount();
    if( nCount > 1 )
    {
        Reference<com::sun::star::beans::XPropertySet>  xFCSet;
        for( sal_Int32 nIndex=0; nIndex < nCount; nIndex++ )
        {
            
            xElements->getByIndex(nIndex) >>= xFCSet;
            if (hasProperty(PROPERTY_CLASSID, xFCSet) &&
                getINT16(xFCSet->getPropertyValue(PROPERTY_CLASSID)) == FormComponentType::TEXTFIELD)
            {
                
                if (xFCSet != xSet)
                    return;
            }
        }
    }
    
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );
    m_nKeyEvent = Application::PostUserEvent( LINK(this, OFormattedControl,
                                            OnKeyPressed) );
}

void OFormattedControl::keyReleased(const ::com::sun::star::awt::KeyEvent& /*e*/) throw ( ::com::sun::star::uno::RuntimeException)
{
}

IMPL_LINK(OFormattedControl, OnKeyPressed, void*, /*EMPTYARG*/)
{
    m_nKeyEvent = 0;
    Reference<XFormComponent>  xFComp(getModel(), UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    Reference<XSubmit>  xSubmit(xParent, UNO_QUERY);
    if (xSubmit.is())
        xSubmit->submit( Reference<XControl> (), ::com::sun::star::awt::MouseEvent() );
    return 0L;
}

StringSequence  OFormattedControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);
    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_FORMATTEDFIELD;
    return aSupported;
}

void OFormattedControl::setDesignMode(sal_Bool bOn) throw ( ::com::sun::star::uno::RuntimeException)
{
    OBoundControl::setDesignMode(bOn);
}

void OFormattedModel::implConstruct()
{
    
    m_bOriginalNumeric = sal_False;
    m_bNumeric = sal_False;
    m_xOriginalFormatter = NULL;
    m_nKeyType = NumberFormat::UNDEFINED;
    m_aNullDate = DBTypeConversion::getStandardDate();
    m_nFieldType =  DataType::OTHER;
    
    increment(m_refCount);
    setPropertyToDefaultByHandle(PROPERTY_ID_FORMATSSUPPLIER);
    decrement(m_refCount);
    startAggregatePropertyListening( PROPERTY_FORMATKEY );
    startAggregatePropertyListening( PROPERTY_FORMATSSUPPLIER );
}
OFormattedModel::OFormattedModel(const Reference<XComponentContext>& _rxFactory)
    :OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_FORMATTEDFIELD, FRM_SUN_CONTROL_FORMATTEDFIELD, sal_True, sal_True )
                            
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


IMPLEMENT_DEFAULT_CLONING( OFormattedModel )

void SAL_CALL OFormattedModel::disposing()
{
    OErrorBroadcaster::disposing();
    OEditBaseModel::disposing();
}


StringSequence OFormattedModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OEditBaseModel::getSupportedServiceNames();
    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 8 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;
    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;
    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = FRM_SUN_COMPONENT_FORMATTEDFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_FORMATTED_FIELD;
    return aSupported;
}


Any SAL_CALL OFormattedModel::queryAggregation(const Type& _rType) throw(RuntimeException)
{
    Any aReturn = OEditBaseModel::queryAggregation( _rType );
    return aReturn.hasValue() ? aReturn : OErrorBroadcaster::queryInterface( _rType );
}


Sequence< Type > OFormattedModel::_getTypes()
{
    return ::comphelper::concatSequences(
        OEditBaseModel::_getTypes(),
        OErrorBroadcaster::getTypes()
    );
}


OUString SAL_CALL OFormattedModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return OUString(FRM_COMPONENT_EDIT);
}


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
    
    
    ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TREATASNUMERIC, 0, PropertyAttribute::TRANSIENT);
    
    
    
    ModifyPropertyAttributes(_rAggregateProps, PROPERTY_FORMATKEY, 0, PropertyAttribute::TRANSIENT);
    RemoveProperty(_rAggregateProps, PROPERTY_STRICTFORMAT);
        
        
        
}

void OFormattedModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    OEditBaseModel::getFastPropertyValue(rValue, nHandle);
}

void OFormattedModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) throw ( ::com::sun::star::uno::Exception)
{
    OEditBaseModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
}

sal_Bool OFormattedModel::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
                                                        throw( IllegalArgumentException )
{
    return OEditBaseModel::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
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

void OFormattedModel::setPropertyToDefault(const OUString& aPropertyName) throw( com::sun::star::beans::UnknownPropertyException, RuntimeException )
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

Any SAL_CALL OFormattedModel::getPropertyDefault( const OUString& aPropertyName ) throw( com::sun::star::beans::UnknownPropertyException, RuntimeException )
{
    OPropertyArrayAggregationHelper& rPH = m_aPropertyBagHelper.getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName( aPropertyName );
    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
        return getPropertyDefaultByHandle(PROPERTY_ID_FORMATSSUPPLIER);
    else
        return OEditBaseModel::getPropertyDefault(aPropertyName);
}

void OFormattedModel::_propertyChanged( const com::sun::star::beans::PropertyChangeEvent& evt ) throw(RuntimeException)
{
    
    OSL_ENSURE( evt.Source == m_xAggregateSet, "OFormattedModel::_propertyChanged: where did this come from?" );
    if ( evt.Source == m_xAggregateSet )
    {
        Reference< XPropertySet > xSourceSet( evt.Source, UNO_QUERY );
        if ( evt.PropertyName.equals( PROPERTY_FORMATKEY ) )
        {
            if ( evt.NewValue.getValueType().getTypeClass() == TypeClass_LONG )
            {
                try
                {
                    ::osl::MutexGuard aGuard( m_aMutex );
                    Reference<XNumberFormatsSupplier> xSupplier( calcFormatsSupplier() );
                    m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), getINT32( evt.NewValue ) );
                    
                    
                    if ( m_xColumn.is() && m_xAggregateFastSet.is()  && !m_xCursor->isBeforeFirst() && !m_xCursor->isAfterLast())
                    {
                        setControlValue( translateDbColumnToControlValue(), eOther );
                    }
                    
                    
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
        if ( evt.PropertyName.equals( PROPERTY_FORMATSSUPPLIER ) )
        {
            updateFormatterNullDate();
            return;
        }
        OBoundControlModel::_propertyChanged( evt );
    }
}

void OFormattedModel::updateFormatterNullDate()
{
    
    Reference< XNumberFormatsSupplier > xSupplier( calcFormatsSupplier() );
    if ( xSupplier.is() )
        xSupplier->getNumberFormatSettings()->getPropertyValue("NullDate") >>= m_aNullDate;
}

Reference< XNumberFormatsSupplier > OFormattedModel::calcFormatsSupplier() const
{
    Reference<XNumberFormatsSupplier>  xSupplier;
    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::calcFormatsSupplier : have no aggregate !");
    
    if( m_xAggregateSet.is() )
        m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER) >>= xSupplier;
    if (!xSupplier.is())
        
        xSupplier = calcFormFormatsSupplier();
    if (!xSupplier.is())
        xSupplier = calcDefaultFormatsSupplier();
    DBG_ASSERT(xSupplier.is(), "OFormattedModel::calcFormatsSupplier : no supplier !");
        
    return xSupplier;
}

Reference<XNumberFormatsSupplier>  OFormattedModel::calcFormFormatsSupplier() const
{
    Reference<XChild>  xMe;
    query_interface(static_cast<XWeak*>(const_cast<OFormattedModel*>(this)), xMe);
    
    
    DBG_ASSERT(xMe.is(), "OFormattedModel::calcFormFormatsSupplier : I should have a content interface !");
    
    Reference<XChild>  xParent(xMe->getParent(), UNO_QUERY);
    Reference<XForm>  xNextParentForm(xParent, UNO_QUERY);
    while (!xNextParentForm.is() && xParent.is())
    {
        xParent         = xParent.query( xParent->getParent() );
        xNextParentForm = xNextParentForm.query( xParent );
    }
    if (!xNextParentForm.is())
    {
        OSL_FAIL("OFormattedModel::calcFormFormatsSupplier : have no ancestor which is a form !");
        return NULL;
    }
    
    Reference< XRowSet > xRowSet( xNextParentForm, UNO_QUERY );
    Reference< XNumberFormatsSupplier > xSupplier;
    if (xRowSet.is())
        xSupplier = getNumberFormats( getConnection(xRowSet), sal_True, getContext() );
    return xSupplier;
}

Reference< XNumberFormatsSupplier > OFormattedModel::calcDefaultFormatsSupplier() const
{
    return StandardFormatsSupplier::get( getContext() );
}


void OFormattedModel::loaded(const EventObject& rEvent) throw ( ::com::sun::star::uno::RuntimeException)
{
    
    
    
    
    
    
    
    
    SolarMutexGuard aGuard;
    OEditBaseModel::loaded(rEvent);
}

void OFormattedModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    m_xOriginalFormatter = NULL;
    
    m_nFieldType = DataType::OTHER;
    Reference<XPropertySet> xField = getField();
    if ( xField.is() )
        xField->getPropertyValue( PROPERTY_FIELDTYPE ) >>= m_nFieldType;
    sal_Int32 nFormatKey = 0;
    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::onConnectedDbColumn : have no aggregate !");
    if (m_xAggregateSet.is())
    {   
        Any aSupplier = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER);
        DBG_ASSERT( aSupplier.hasValue(), "OFormattedModel::onConnectedDbColumn : invalid property value !" );
        
        Any aFmtKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        if ( !(aFmtKey >>= nFormatKey ) )
        {   
            
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
                {   
                    
                    Reference<XNumberFormatTypes>  xTypes(xSupplier->getNumberFormats(), UNO_QUERY);
                    if (xTypes.is())
                    {
                        Locale aApplicationLocale = Application::GetSettings().GetUILanguageTag().getLocale();
                        if (m_bOriginalNumeric)
                            aFmtKey <<= (sal_Int32)xTypes->getStandardFormat(NumberFormat::NUMBER, aApplicationLocale);
                        else
                            aFmtKey <<= (sal_Int32)xTypes->getStandardFormat(NumberFormat::TEXT, aApplicationLocale);
                    }
                }
                aSupplier >>= m_xOriginalFormatter;
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(xSupplier));
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, aFmtKey);
                
                if (xField.is())
                {
                    m_bNumeric = sal_False;
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
                            m_bNumeric = sal_True;
                            break;
                    }
                }
                else
                    m_bNumeric = m_bOriginalNumeric;
                setPropertyValue(PROPERTY_TREATASNUMERIC, makeAny((sal_Bool)m_bNumeric));
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
    {   
        m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(m_xOriginalFormatter));
        m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, Any());
        setPropertyValue(PROPERTY_TREATASNUMERIC, makeAny((sal_Bool)m_bOriginalNumeric));
        m_xOriginalFormatter = NULL;
    }
    m_nFieldType = DataType::OTHER;
    m_nKeyType   = NumberFormat::UNDEFINED;
    m_aNullDate  = DBTypeConversion::getStandardDate();
}

void OFormattedModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OEditBaseModel::write(_rxOutStream);
    _rxOutStream->writeShort(0x0003);
    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::write : have no aggregate !");
    
    
        Reference<XNumberFormatsSupplier>  xSupplier;
        Any aFmtKey;
    sal_Bool bVoidKey = sal_True;
    if (m_xAggregateSet.is())
    {
        Any aSupplier = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER);
        if (aSupplier.getValueType().getTypeClass() != TypeClass_VOID)
        {
            OSL_VERIFY( aSupplier >>= xSupplier );
        }
        aFmtKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        bVoidKey = (!xSupplier.is() || !aFmtKey.hasValue()) || (isLoaded() && m_xOriginalFormatter.is());
            
    }
    _rxOutStream->writeBoolean(!bVoidKey);
    if (!bVoidKey)
    {
        
        Any aKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        sal_Int32 nKey = aKey.hasValue() ? getINT32(aKey) : 0;
        Reference<XNumberFormats>  xFormats = xSupplier->getNumberFormats();
        OUString         sFormatDescription;
        LanguageType    eFormatLanguage = LANGUAGE_DONTKNOW;
        static const OUString s_aLocaleProp ("Locale");
        Reference<com::sun::star::beans::XPropertySet>  xFormat = xFormats->getByKey(nKey);
        if (hasProperty(s_aLocaleProp, xFormat))
        {
            Any aLocale = xFormat->getPropertyValue(s_aLocaleProp);
            DBG_ASSERT(isA(aLocale, static_cast<Locale*>(NULL)), "OFormattedModel::write : invalid language property !");
            if (isA(aLocale, static_cast<Locale*>(NULL)))
            {
                Locale* pLocale = (Locale*)aLocale.getValue();
                eFormatLanguage = LanguageTag::convertToLanguageType( *pLocale, false);
            }
        }
        static const OUString s_aFormatStringProp ("FormatString");
        if (hasProperty(s_aFormatStringProp, xFormat))
            xFormat->getPropertyValue(s_aFormatStringProp) >>= sFormatDescription;
        _rxOutStream->writeUTF(sFormatDescription);
        _rxOutStream->writeLong((sal_Int32)eFormatLanguage);
    }
    
    writeCommonEditProperties(_rxOutStream);
    
    
    
    
    
    {
        OStreamSection aDownCompat(_rxOutStream);
        
        _rxOutStream->writeShort(0x0000);
        
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
                default:    
                    DBG_ASSERT(!aEffectiveValue.hasValue(), "FmXFormattedModel::write : unknown property value type !");
                    _rxOutStream->writeShort(0x0002);
                    break;
            }
        }
    }
}

void OFormattedModel::read(const Reference<XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
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
            sal_Bool bNonVoidKey = _rxInStream->readBoolean();
            if (bNonVoidKey)
            {
                
                OUString sFormatDescription = _rxInStream->readUTF();
                LanguageType eDescriptionLanguage = (LanguageType)_rxInStream->readLong();
                
                xSupplier = calcFormatsSupplier();
                    
                Reference<XNumberFormats>  xFormats = xSupplier->getNumberFormats();
                if (xFormats.is())
                {
                    Locale aDescriptionLanguage( LanguageTag::convertToLocale(eDescriptionLanguage));
                    nKey = xFormats->queryKey(sFormatDescription, aDescriptionLanguage, sal_False);
                    if (nKey == (sal_Int32)-1)
                    {   
                        nKey = xFormats->addNew(sFormatDescription, aDescriptionLanguage);
                    }
                }
            }
            if ((nVersion == 0x0002) || (nVersion == 0x0003))
                readCommonEditProperties(_rxInStream);
            if (nVersion == 0x0003)
            {   
                OStreamSection aDownCompat(_rxInStream);
                sal_Int16 nSubVersion = _rxInStream->readShort();
                (void)nSubVersion;
                
                Any aEffectiveValue;
                {
                    OStreamSection aDownCompat2(_rxInStream);
                    switch (_rxInStream->readShort())
                    {
                        case 0: 
                            aEffectiveValue <<= _rxInStream->readUTF();
                            break;
                        case 1: 
                            aEffectiveValue <<= (double)_rxInStream->readDouble();
                            break;
                        case 2:
                            break;
                        case 3:
                            OSL_FAIL("FmXFormattedModel::read : unknown effective value type !");
                    }
                }
                
                
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
            
            defaultCommonEditProperties();
            break;
    }
    if ((nKey != -1) && m_xAggregateSet.is())
    {
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(xSupplier));
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, makeAny((sal_Int32)nKey));
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
    
}

sal_Bool OFormattedModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aControlValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );
    if ( aControlValue != m_aSaveValue )
    {
        
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
                if ( aControlValue.getValueType().getTypeClass() == TypeClass_DOUBLE || (aControlValue >>= f)) 
                {
                    DBTypeConversion::setValue( m_xColumnUpdate, m_aNullDate, getDouble( aControlValue ), m_nKeyType );
                }
                else
                {
                    DBG_ASSERT( aControlValue.getValueType().getTypeClass() == TypeClass_STRING, "OFormattedModel::commitControlValueToDbColumn: invalud value type !" );
                    m_xColumnUpdate->updateString( getString( aControlValue ) );
                }
            }
            catch(const Exception&)
            {
                return sal_False;
            }
        }
        m_aSaveValue = aControlValue;
    }
    return sal_True;
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
        sal_Bool bExternalValue = sal_False;
        _rExternalValue >>= bExternalValue;
        aControlValue <<= (double)( bExternalValue ? 1 : 0 );
    }
    break;
    default:
    {
        if ( _rExternalValue.getValueType().equals( ::getCppuType( static_cast< UNODate* >( NULL ) ) ) )
        {
            UNODate aDate;
            _rExternalValue >>= aDate;
            aControlValue <<= DBTypeConversion::toDouble( aDate, m_aNullDate );
        }
        else if ( _rExternalValue.getValueType().equals( ::getCppuType( static_cast< UNOTime* >( NULL ) ) ) )
        {
            UNOTime aTime;
            _rExternalValue >>= aTime;
            aControlValue <<= DBTypeConversion::toDouble( aTime );
        }
        else if ( _rExternalValue.getValueType().equals( ::getCppuType( static_cast< UNODateTime* >( NULL ) ) ) )
        {
            UNODateTime aDateTime;
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
    }
    
    case TypeClass_BOOLEAN:
    {
        double fValue = 0;
        OSL_VERIFY( aControlValue >>= fValue );
            
            
            
        aExternalValue <<= (sal_Bool)( fValue ? sal_True : sal_False );
    }
    break;
    default:
    {
        double fValue = 0;
        OSL_VERIFY( aControlValue >>= fValue );
            
            
            
        if ( aExternalValueType.equals( ::getCppuType( static_cast< UNODate* >( NULL ) ) ) )
        {
            aExternalValue <<= DBTypeConversion::toDate( fValue, m_aNullDate );
        }
        else if ( aExternalValueType.equals( ::getCppuType( static_cast< UNOTime* >( NULL ) ) ) )
        {
            aExternalValue <<= DBTypeConversion::toTime( fValue );
        }
        else if ( aExternalValueType.equals( ::getCppuType( static_cast< UNODateTime* >( NULL ) ) ) )
        {
            aExternalValue <<= DBTypeConversion::toDateTime( fValue, m_aNullDate );
        }
        else
        {
            OSL_ENSURE( aExternalValueType.equals( ::getCppuType( static_cast< double* >( NULL ) ) ),
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
        m_aSaveValue <<= DBTypeConversion::getValue( m_xColumn, m_aNullDate ); 
    else
        m_aSaveValue <<= m_xColumn->getString();
    if ( m_xColumn->wasNull() )
        m_aSaveValue.clear();
    return m_aSaveValue;
}

Sequence< Type > OFormattedModel::getSupportedBindingTypes()
{
    ::std::list< Type > aTypes;
    aTypes.push_back( ::getCppuType( static_cast< double* >( NULL ) ) );
    switch ( m_nKeyType & ~NumberFormat::DEFINED )
    {
    case NumberFormat::DATE:
        aTypes.push_front(::getCppuType( static_cast< UNODate* >( NULL ) ) );
        break;
    case NumberFormat::TIME:
        aTypes.push_front(::getCppuType( static_cast< UNOTime* >( NULL ) ) );
        break;
    case NumberFormat::DATETIME:
        aTypes.push_front(::getCppuType( static_cast< UNODateTime* >( NULL ) ) );
        break;
    case NumberFormat::TEXT:
        aTypes.push_front(::getCppuType( static_cast< OUString* >( NULL ) ) );
        break;
    case NumberFormat::LOGICAL:
        aTypes.push_front(::getCppuType( static_cast< sal_Bool* >( NULL ) ) );
        break;
    }
    Sequence< Type > aTypesRet( aTypes.size() );
    ::std::copy( aTypes.begin(), aTypes.end(), aTypesRet.getArray() );
    return aTypesRet;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
