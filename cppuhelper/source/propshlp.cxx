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


#include "osl/diagnose.h"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "cppuhelper/propshlp.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/lang/DisposedException.hpp"


using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace cppu;

using ::rtl::OUString;

namespace cppu {

IPropertyArrayHelper::~IPropertyArrayHelper()
{
}

inline const ::com::sun::star::uno::Type & getPropertyTypeIdentifier( ) SAL_THROW(())
{
    return ::getCppuType( (Reference< XPropertyChangeListener > *)0 );
}
inline const ::com::sun::star::uno::Type & getPropertiesTypeIdentifier() SAL_THROW(())
{
    return ::getCppuType( (Reference< XPropertiesChangeListener > *)0 );
}
inline const ::com::sun::star::uno::Type & getVetoableTypeIdentifier() SAL_THROW(())
{
    return ::getCppuType( (Reference< XVetoableChangeListener > *)0 );
}

extern "C" {

static int compare_OUString_Property_Impl( const void *arg1, const void *arg2 )
    SAL_THROW_EXTERN_C()
{
   return ((OUString *)arg1)->compareTo( ((Property *)arg2)->Name );
}

}

/**
 * The class which implements the PropertySetInfo interface.
 */

class OPropertySetHelperInfo_Impl
    : public WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
    Sequence < Property > aInfos;

public:
    OPropertySetHelperInfo_Impl( IPropertyArrayHelper & rHelper_ ) SAL_THROW(());

    
    virtual Sequence< Property > SAL_CALL getProperties(void) throw(::com::sun::star::uno::RuntimeException);
    virtual Property SAL_CALL getPropertyByName(const OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& PropertyName) throw(::com::sun::star::uno::RuntimeException);
};


/**
 * Create an object that implements XPropertySetInfo IPropertyArrayHelper.
 */
OPropertySetHelperInfo_Impl::OPropertySetHelperInfo_Impl(
    IPropertyArrayHelper & rHelper_ )
    SAL_THROW(())
    :aInfos( rHelper_.getProperties() )
{
}

/**
 * Return the sequence of properties, which are provided throug the constructor.
 */
Sequence< Property > OPropertySetHelperInfo_Impl::getProperties(void) throw(::com::sun::star::uno::RuntimeException)

{
    return aInfos;
}

/**
 * Return the sequence of properties, which are provided throug the constructor.
 */
Property OPropertySetHelperInfo_Impl::getPropertyByName( const OUString & PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    Property * pR;
    pR = (Property *)bsearch( &PropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    if( !pR ) {
        throw UnknownPropertyException();
    }

    return *pR;
}

/**
 * Return the sequence of properties, which are provided throug the constructor.
 */
sal_Bool OPropertySetHelperInfo_Impl::hasPropertyByName( const OUString & PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    Property * pR;
    pR = (Property *)bsearch( &PropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    return pR != NULL;
}




class OPropertySetHelper::Impl {

public:
    Impl(   bool i_bIgnoreRuntimeExceptionsWhileFiring,
            IEventNotificationHook *i_pFireEvents
        )
        :m_bIgnoreRuntimeExceptionsWhileFiring( i_bIgnoreRuntimeExceptionsWhileFiring )
        ,m_bFireEvents(true)
        ,m_pFireEvents( i_pFireEvents )
    {
    }

    bool m_bIgnoreRuntimeExceptionsWhileFiring;
    bool m_bFireEvents;
    class IEventNotificationHook * const m_pFireEvents;

    ::std::vector< sal_Int32 >  m_handles;
    ::std::vector< Any >        m_newValues;
    ::std::vector< Any >        m_oldValues;
};





OPropertySetHelper::OPropertySetHelper(
    OBroadcastHelper  & rBHelper_ ) SAL_THROW(())
    : rBHelper( rBHelper_ ),
      aBoundLC( rBHelper_.rMutex ),
      aVetoableLC( rBHelper_.rMutex ),
      m_pReserved( new Impl(false, 0) )
{
}

OPropertySetHelper::OPropertySetHelper(
    OBroadcastHelper  & rBHelper_, bool bIgnoreRuntimeExceptionsWhileFiring )
    : rBHelper( rBHelper_ ),
      aBoundLC( rBHelper_.rMutex ),
      aVetoableLC( rBHelper_.rMutex ),
      m_pReserved( new Impl( bIgnoreRuntimeExceptionsWhileFiring, 0 ) )
{
}

OPropertySetHelper::OPropertySetHelper(
    OBroadcastHelper  & rBHelper_, IEventNotificationHook * i_pFireEvents,
    bool bIgnoreRuntimeExceptionsWhileFiring)
    : rBHelper( rBHelper_ ),
      aBoundLC( rBHelper_.rMutex ),
      aVetoableLC( rBHelper_.rMutex ),
      m_pReserved(
        new Impl( bIgnoreRuntimeExceptionsWhileFiring, i_pFireEvents) )
{
}

OPropertySetHelper2::OPropertySetHelper2(
        OBroadcastHelper & irBHelper,
        IEventNotificationHook *i_pFireEvents,
        bool bIgnoreRuntimeExceptionsWhileFiring)
            :OPropertySetHelper( irBHelper, i_pFireEvents, bIgnoreRuntimeExceptionsWhileFiring )
{
}

/**
 * You must call disposing before.
 */
OPropertySetHelper::~OPropertySetHelper() SAL_THROW(())
{
}
OPropertySetHelper2::~OPropertySetHelper2() SAL_THROW(())
{
}


Any OPropertySetHelper::queryInterface( const ::com::sun::star::uno::Type & rType )
    throw (RuntimeException)
{
    return ::cppu::queryInterface(
        rType,
        static_cast< XPropertySet * >( this ),
        static_cast< XMultiPropertySet * >( this ),
        static_cast< XFastPropertySet * >( this ) );
}

Any OPropertySetHelper2::queryInterface( const ::com::sun::star::uno::Type & rType )
    throw (RuntimeException)
{
    Any cnd(cppu::queryInterface(rType, static_cast< XPropertySetOption * >(this)));
    if ( cnd.hasValue() )
        return cnd;
    else
        return OPropertySetHelper::queryInterface(rType);
}

/**
 * called from the derivee's XTypeProvider::getTypes implementation
 */
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > OPropertySetHelper::getTypes()
    throw (RuntimeException)
{
    Sequence< ::com::sun::star::uno::Type > aTypes( 4 );
    aTypes[ 0 ] = XPropertySet::static_type();
    aTypes[ 1 ] = XPropertySetOption::static_type();
    aTypes[ 2 ] = XMultiPropertySet::static_type();
    aTypes[ 3 ] = XFastPropertySet::static_type();
    return aTypes;
}


void OPropertySetHelper::disposing() SAL_THROW(())
{
    
    Reference < XPropertySet  > rSource( (static_cast< XPropertySet *  >(this)) , UNO_QUERY );
    EventObject aEvt;
    aEvt.Source = rSource;

    
    
    aBoundLC.disposeAndClear( aEvt );
    aVetoableLC.disposeAndClear( aEvt );
}

Reference < XPropertySetInfo > OPropertySetHelper::createPropertySetInfo(
    IPropertyArrayHelper & rProperties ) SAL_THROW(())
{
    return static_cast< XPropertySetInfo * >( new OPropertySetHelperInfo_Impl( rProperties ) );
}


void OPropertySetHelper::setPropertyValue(
    const OUString& rPropertyName, const Any& rValue )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    
    IPropertyArrayHelper & rPH = getInfoHelper();
    
    sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
    
    setFastPropertyValue( nHandle, rValue );
}


Any OPropertySetHelper::getPropertyValue(
    const OUString& rPropertyName )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    
    IPropertyArrayHelper & rPH = getInfoHelper();
    
    sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
    
    return getFastPropertyValue( nHandle );
}


void OPropertySetHelper::addPropertyChangeListener(
    const OUString& rPropertyName,
    const Reference < XPropertyChangeListener > & rxListener )
     throw(::com::sun::star::beans::UnknownPropertyException,
           ::com::sun::star::lang::WrappedTargetException,
           ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bInDispose, "do not addPropertyChangeListener in the dispose call" );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
    {
        
        
        if( !rPropertyName.isEmpty() )
        {
            
            IPropertyArrayHelper & rPH = getInfoHelper();
            
            sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
            if( nHandle == -1 ) {
                
                throw  UnknownPropertyException() ;
            }

            sal_Int16 nAttributes;
            rPH.fillPropertyMembersByHandle( NULL, &nAttributes, nHandle );
            if( !(nAttributes & ::com::sun::star::beans::PropertyAttribute::BOUND) )
            {
                OSL_FAIL( "add listener to an unbound property" );
                
                return;
            }
            

            aBoundLC.addInterface( (sal_Int32)nHandle, rxListener );
        }
        else
            
            rBHelper.aLC.addInterface(
                            getPropertyTypeIdentifier(  ),
                            rxListener
                                     );
    }
}



void OPropertySetHelper::removePropertyChangeListener(
    const OUString& rPropertyName,
    const Reference < XPropertyChangeListener >& rxListener )
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::lang::WrappedTargetException,
          ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    
    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
    {
        if( !rPropertyName.isEmpty() )
        {
            
            IPropertyArrayHelper & rPH = getInfoHelper();
            
            sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
            if( nHandle == -1 )
                
                throw UnknownPropertyException();
            aBoundLC.removeInterface( (sal_Int32)nHandle, rxListener );
        }
        else {
            
            rBHelper.aLC.removeInterface(
                            getPropertyTypeIdentifier(  ),
                            rxListener
                                        );
        }
    }
}


void OPropertySetHelper::addVetoableChangeListener(
    const OUString& rPropertyName,
    const Reference< XVetoableChangeListener > & rxListener )
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::lang::WrappedTargetException,
          ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bInDispose, "do not addVetoableChangeListener in the dispose call" );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
    {
        
        
        if( !rPropertyName.isEmpty() )
        {
            
            IPropertyArrayHelper & rPH = getInfoHelper();
            
            sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
            if( nHandle == -1 ) {
                
                throw UnknownPropertyException();
            }

            sal_Int16 nAttributes;
            rPH.fillPropertyMembersByHandle( NULL, &nAttributes, nHandle );
            if( !(nAttributes & PropertyAttribute::CONSTRAINED) )
            {
                OSL_FAIL( "addVetoableChangeListener, and property is not constrained" );
                
                return;
            }
            
            aVetoableLC.addInterface( (sal_Int32)nHandle, rxListener );
        }
        else
            
            rBHelper.aLC.addInterface(
                                getVetoableTypeIdentifier(  ),
                                rxListener
                                     );
    }
}


void OPropertySetHelper::removeVetoableChangeListener(
    const OUString& rPropertyName,
    const Reference < XVetoableChangeListener > & rxListener )
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::lang::WrappedTargetException,
          ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    
    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
    {
        if( !rPropertyName.isEmpty() )
        {
            
            IPropertyArrayHelper & rPH = getInfoHelper();
            
            sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
            if( nHandle == -1 ) {
                
                throw UnknownPropertyException();
            }
            
            aVetoableLC.removeInterface( (sal_Int32)nHandle, rxListener );
        }
        else
            
            rBHelper.aLC.removeInterface(
                                getVetoableTypeIdentifier( ),
                                rxListener
                                        );
    }
}

void OPropertySetHelper::setDependentFastPropertyValue( sal_Int32 i_handle, const ::com::sun::star::uno::Any& i_value )
{
    
        

    sal_Int16 nAttributes(0);
    IPropertyArrayHelper& rInfo = getInfoHelper();
    if ( !rInfo.fillPropertyMembersByHandle( NULL, &nAttributes, i_handle ) )
        
        throw UnknownPropertyException();

    
    
    

    Any aConverted, aOld;
    bool bChanged = convertFastPropertyValue( aConverted, aOld, i_handle, i_value );
    if ( !bChanged )
        return;

    
    
    OSL_ENSURE( ( nAttributes & PropertyAttribute::CONSTRAINED ) == 0,
        "OPropertySetHelper::setDependentFastPropertyValue: not to be used for constrained properties!" );
    (void)nAttributes;

    
    try
    {
        setFastPropertyValue_NoBroadcast( i_handle, aConverted );
    }
    catch (const UnknownPropertyException& )    { throw;    /* allowed to leave */ }
    catch (const PropertyVetoException& )       { throw;    /* allowed to leave */ }
    catch (const IllegalArgumentException& )    { throw;    /* allowed to leave */ }
    catch (const WrappedTargetException& )      { throw;    /* allowed to leave */ }
    catch (const RuntimeException& )            { throw;    /* allowed to leave */ }
    catch (const Exception& )
    {
        
        WrappedTargetException aWrapped;
        aWrapped.TargetException <<= ::cppu::getCaughtException();
        aWrapped.Context = static_cast< XPropertySet* >( this );
        throw aWrapped;
    }

    
    m_pReserved->m_handles.push_back( i_handle );
    m_pReserved->m_newValues.push_back( aConverted );   
    m_pReserved->m_oldValues.push_back( aOld );
}


void OPropertySetHelper::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue )
     throw(::com::sun::star::beans::UnknownPropertyException,
           ::com::sun::star::beans::PropertyVetoException,
           ::com::sun::star::lang::IllegalArgumentException,
           ::com::sun::star::lang::WrappedTargetException,
           ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( !rBHelper.bInDispose, "do not setFastPropertyValue in the dispose call" );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );

    IPropertyArrayHelper & rInfo = getInfoHelper();
    sal_Int16 nAttributes;
    if( !rInfo.fillPropertyMembersByHandle( NULL, &nAttributes, nHandle ) ) {
        
        throw UnknownPropertyException();
    }
    if( nAttributes & PropertyAttribute::READONLY )
        throw PropertyVetoException();

    Any aConvertedVal;
    Any aOldVal;

    
    bool bChanged;
    {
        MutexGuard aGuard( rBHelper.rMutex );
        bChanged = convertFastPropertyValue( aConvertedVal, aOldVal, nHandle, rValue );
        
    }
    if( bChanged )
    {
        
        if( nAttributes & PropertyAttribute::CONSTRAINED )
        {
            
            
            
            fire( &nHandle, &rValue, &aOldVal, 1, sal_True );
        }

        {
            MutexGuard aGuard( rBHelper.rMutex );
            try
            {
                
                setFastPropertyValue_NoBroadcast( nHandle, aConvertedVal );
            }
            catch (const ::com::sun::star::beans::UnknownPropertyException& )   { throw;    /* allowed to leave */ }
            catch (const ::com::sun::star::beans::PropertyVetoException& )      { throw;    /* allowed to leave */ }
            catch (const ::com::sun::star::lang::IllegalArgumentException& )    { throw;    /* allowed to leave */ }
            catch (const ::com::sun::star::lang::WrappedTargetException& )      { throw;    /* allowed to leave */ }
            catch (const ::com::sun::star::uno::RuntimeException& )             { throw;    /* allowed to leave */ }
            catch (const ::com::sun::star::uno::Exception& e )
            {
                
                ::com::sun::star::lang::WrappedTargetException aWrap;
                aWrap.Context = static_cast< ::com::sun::star::beans::XPropertySet* >( this );
                aWrap.TargetException <<= e;

                throw ::com::sun::star::lang::WrappedTargetException( aWrap );
            }

            
        }
        
        impl_fireAll( &nHandle, &rValue, &aOldVal, 1 );
    }
}


Any OPropertySetHelper::getFastPropertyValue( sal_Int32 nHandle )
     throw(::com::sun::star::beans::UnknownPropertyException,
           ::com::sun::star::lang::WrappedTargetException,
           ::com::sun::star::uno::RuntimeException)

{
    IPropertyArrayHelper & rInfo = getInfoHelper();
    if( !rInfo.fillPropertyMembersByHandle( NULL, NULL, nHandle ) )
        
        throw UnknownPropertyException();

    Any aRet;
    MutexGuard aGuard( rBHelper.rMutex );
    getFastPropertyValue( aRet, nHandle );
    return aRet;
}


void OPropertySetHelper::impl_fireAll( sal_Int32* i_handles, const Any* i_newValues, const Any* i_oldValues, sal_Int32 i_count )
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if ( m_pReserved->m_handles.empty() )
    {
        aGuard.clear();
        fire( i_handles, i_newValues, i_oldValues, i_count, sal_False );
        return;
    }

    const size_t additionalEvents = m_pReserved->m_handles.size();
    OSL_ENSURE( additionalEvents == m_pReserved->m_newValues.size()
            &&  additionalEvents == m_pReserved->m_oldValues.size(),
            "OPropertySetHelper::impl_fireAll: inconsistency!" );

    ::std::vector< sal_Int32 > allHandles( additionalEvents + i_count );
    ::std::copy( m_pReserved->m_handles.begin(), m_pReserved->m_handles.end(), allHandles.begin() );
    ::std::copy( i_handles, i_handles + i_count, allHandles.begin() + additionalEvents );

    ::std::vector< Any > allNewValues( additionalEvents + i_count );
    ::std::copy( m_pReserved->m_newValues.begin(), m_pReserved->m_newValues.end(), allNewValues.begin() );
    ::std::copy( i_newValues, i_newValues + i_count, allNewValues.begin() + additionalEvents );

    ::std::vector< Any > allOldValues( additionalEvents + i_count );
    ::std::copy( m_pReserved->m_oldValues.begin(), m_pReserved->m_oldValues.end(), allOldValues.begin() );
    ::std::copy( i_oldValues, i_oldValues + i_count, allOldValues.begin() + additionalEvents );

    m_pReserved->m_handles.clear();
    m_pReserved->m_newValues.clear();
    m_pReserved->m_oldValues.clear();

    aGuard.clear();
    fire( &allHandles[0], &allNewValues[0], &allOldValues[0], additionalEvents + i_count, sal_False );
}


void OPropertySetHelper::fire
(
    sal_Int32 * pnHandles,
    const Any * pNewValues,
    const Any * pOldValues,
    sal_Int32 nHandles, 
    sal_Bool bVetoable
)
{
    OSL_ENSURE( m_pReserved.get(), "No OPropertySetHelper::Impl" );

    if (! m_pReserved->m_bFireEvents)
        return;

    if (m_pReserved->m_pFireEvents) {
        m_pReserved->m_pFireEvents->fireEvents(
            pnHandles, nHandles, bVetoable,
            m_pReserved->m_bIgnoreRuntimeExceptionsWhileFiring);
    }

    
    if( nHandles )
    {
        
        Sequence< PropertyChangeEvent > aEvts( nHandles );
        PropertyChangeEvent * pEvts = aEvts.getArray();
        Reference < XInterface > xSource( (XPropertySet *)this, UNO_QUERY );
        sal_Int32 i;
        sal_Int32 nChangesLen = 0;
        
        for( i = 0; i < nHandles; i++ )
        {
            
            
            IPropertyArrayHelper & rInfo = getInfoHelper();
            sal_Int16   nAttributes;
            OUString aPropName;
            rInfo.fillPropertyMembersByHandle( &aPropName, &nAttributes, pnHandles[i] );

            if(
               (bVetoable && (nAttributes & PropertyAttribute::CONSTRAINED)) ||
               (!bVetoable && (nAttributes & PropertyAttribute::BOUND))
              )
            {
                pEvts[nChangesLen].Source = xSource;
                pEvts[nChangesLen].PropertyName = aPropName;
                pEvts[nChangesLen].PropertyHandle = pnHandles[i];
                pEvts[nChangesLen].OldValue = pOldValues[i];
                pEvts[nChangesLen].NewValue = pNewValues[i];
                nChangesLen++;
            }
        }

        bool bIgnoreRuntimeExceptionsWhileFiring =
                m_pReserved->m_bIgnoreRuntimeExceptionsWhileFiring;

        
        for( i = 0; i < nChangesLen; i++ )
        {
            
            OInterfaceContainerHelper * pLC;
            if( bVetoable ) 
                pLC = aVetoableLC.getContainer( pEvts[i].PropertyHandle );
            else
                pLC = aBoundLC.getContainer( pEvts[i].PropertyHandle );
            if( pLC )
            {
                
                OInterfaceIteratorHelper aIt( *pLC);
                while( aIt.hasMoreElements() )
                {
                    XInterface * pL = aIt.next();
                    try
                    {
                        try
                        {
                            if( bVetoable ) 
                            {
                                ((XVetoableChangeListener *)pL)->vetoableChange(
                                    pEvts[i] );
                            }
                            else
                            {
                                ((XPropertyChangeListener *)pL)->propertyChange(
                                    pEvts[i] );
                            }
                        }
                        catch (DisposedException & exc)
                        {
                            OSL_ENSURE( exc.Context.is(),
                                        "DisposedException without Context!" );
                            if (exc.Context == pL)
                                aIt.remove();
                            else
                                throw;
                        }
                    }
                    catch (RuntimeException & exc)
                    {
                        SAL_INFO("cppuhelper", "caught RuntimeException while firing listeners: " << exc.Message);
                        if (! bIgnoreRuntimeExceptionsWhileFiring)
                            throw;
                    }
                }
            }
            
            if( bVetoable ){
                
                pLC = rBHelper.aLC.getContainer(
                            getVetoableTypeIdentifier()
                                                );
            }
            else {
                pLC = rBHelper.aLC.getContainer(
                            getPropertyTypeIdentifier(  )
                                                );
            }
            if( pLC )
            {
                
                OInterfaceIteratorHelper aIt( *pLC);
                while( aIt.hasMoreElements() )
                {
                    XInterface * pL = aIt.next();
                    try
                    {
                        try
                        {
                            if( bVetoable ) 
                            {
                                ((XVetoableChangeListener *)pL)->vetoableChange(
                                    pEvts[i] );
                            }
                            else
                            {
                                ((XPropertyChangeListener *)pL)->propertyChange(
                                    pEvts[i] );
                            }
                        }
                        catch (DisposedException & exc)
                        {
                            OSL_ENSURE( exc.Context.is(),
                                        "DisposedException without Context!" );
                            if (exc.Context == pL)
                                aIt.remove();
                            else
                                throw;
                        }
                    }
                    catch (RuntimeException & exc)
                    {
                        SAL_INFO("cppuhelper", "caught RuntimeException while firing listeners: " << exc.Message);
                        if (! bIgnoreRuntimeExceptionsWhileFiring)
                            throw;
                    }
                }
            }
        }

        
        aEvts.realloc( nChangesLen );

        if( !bVetoable )
        {
            OInterfaceContainerHelper * pCont = 0;
            pCont = rBHelper.aLC.getContainer(
                                getPropertiesTypeIdentifier(  )
                                             );
            if( pCont )
            {
                
                OInterfaceIteratorHelper aIt( *pCont );
                while( aIt.hasMoreElements() )
                {
                    XPropertiesChangeListener * pL =
                        (XPropertiesChangeListener *)aIt.next();
                    try
                    {
                        try
                        {
                            
                            
                            pL->propertiesChange( aEvts );
                        }
                        catch (DisposedException & exc)
                        {
                            OSL_ENSURE( exc.Context.is(),
                                        "DisposedException without Context!" );
                            if (exc.Context == pL)
                                aIt.remove();
                            else
                                throw;
                        }
                    }
                    catch (RuntimeException & exc)
                    {
                        SAL_INFO("cppuhelper", "caught RuntimeException while firing listeners: " << exc.Message);
                        if (! bIgnoreRuntimeExceptionsWhileFiring)
                            throw;
                    }
                }
            }
        }
    }
}


void OPropertySetHelper::setFastPropertyValues(
    sal_Int32 nSeqLen,
    sal_Int32 * pHandles,
    const Any * pValues,
    sal_Int32 nHitCount )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    OSL_ENSURE( !rBHelper.bInDispose, "do not getFastPropertyValue in the dispose call" );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );

    Any * pConvertedValues = NULL;
    Any * pOldValues = NULL;

    try
    {
        
        IPropertyArrayHelper & rPH = getInfoHelper();

        pConvertedValues = new Any[ nHitCount ];
        pOldValues = new Any[ nHitCount ];
        sal_Int32 n = 0;
        sal_Int32 i;

        {
        
        MutexGuard aGuard( rBHelper.rMutex );
        for( i = 0; i < nSeqLen; i++ )
        {
            if( pHandles[i] != -1 )
            {
                sal_Int16 nAttributes;
                rPH.fillPropertyMembersByHandle( NULL, &nAttributes, pHandles[i] );
                if( nAttributes & PropertyAttribute::READONLY ) {
                    throw PropertyVetoException();
                }
                
                if( convertFastPropertyValue( pConvertedValues[ n ], pOldValues[n],
                                            pHandles[i], pValues[i] ) )
                {
                    
                    pHandles[n]         = pHandles[i];
                    n++;
                }
            }
        }
        
        }

        
        fire( pHandles, pConvertedValues, pOldValues, n, sal_True );

        {
        
        MutexGuard aGuard( rBHelper.rMutex );
        
        for( i = 0; i < n; i++ )
        {
            
            setFastPropertyValue_NoBroadcast( pHandles[i], pConvertedValues[i] );
        }
        
        }

        
        impl_fireAll( pHandles, pConvertedValues, pOldValues, n );
    }
    catch( ... )
    {
        delete [] pOldValues;
        delete [] pConvertedValues;
        throw;
    }
    delete [] pOldValues;
    delete [] pConvertedValues;
}


/**
 * The sequence may be conatain not known properties. The implementation
 * must ignore these properties.
 */
void OPropertySetHelper::setPropertyValues(
    const Sequence<OUString>& rPropertyNames,
    const Sequence<Any>& rValues )
    throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 * pHandles = NULL;
    try
    {
        sal_Int32   nSeqLen = rPropertyNames.getLength();
        pHandles = new sal_Int32[ nSeqLen ];
        
        IPropertyArrayHelper & rPH = getInfoHelper();
        
        sal_Int32 nHitCount = rPH.fillHandles( pHandles, rPropertyNames );
        if( nHitCount != 0 )
            setFastPropertyValues( nSeqLen, pHandles, rValues.getConstArray(), nHitCount );
    }
    catch( ... )
    {
        delete [] pHandles;
        throw;
    }
    delete [] pHandles;
}


Sequence<Any> OPropertySetHelper::getPropertyValues( const Sequence<OUString>& rPropertyNames )
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32   nSeqLen = rPropertyNames.getLength();
    sal_Int32 * pHandles = new sal_Int32[ nSeqLen ];
    Sequence< Any > aValues( nSeqLen );

    
    IPropertyArrayHelper & rPH = getInfoHelper();
    
    rPH.fillHandles( pHandles, rPropertyNames );

    Any * pValues = aValues.getArray();

    MutexGuard aGuard( rBHelper.rMutex );
    
    for( sal_Int32 i = 0; i < nSeqLen; i++ )
        getFastPropertyValue( pValues[i], pHandles[i] );

    delete [] pHandles;
    return aValues;
}


void OPropertySetHelper::addPropertiesChangeListener(
    const Sequence<OUString> & ,
    const Reference < XPropertiesChangeListener > & rListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    rBHelper.addListener( getCppuType(&rListener) , rListener );
}


void OPropertySetHelper::removePropertiesChangeListener(
    const Reference < XPropertiesChangeListener > & rListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    rBHelper.removeListener( getCppuType(&rListener) , rListener );
}


void OPropertySetHelper::firePropertiesChangeEvent(
    const Sequence<OUString>& rPropertyNames,
    const Reference < XPropertiesChangeListener >& rListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nLen = rPropertyNames.getLength();
    sal_Int32 * pHandles = new sal_Int32[nLen];
    IPropertyArrayHelper & rPH = getInfoHelper();
    rPH.fillHandles( pHandles, rPropertyNames );
    const OUString* pNames = rPropertyNames.getConstArray();

    
    sal_Int32 nFireLen = 0;
    sal_Int32 i;
    for( i = 0; i < nLen; i++ )
        if( pHandles[i] != -1 )
            nFireLen++;

    Sequence<PropertyChangeEvent> aChanges( nFireLen );
    PropertyChangeEvent* pChanges = aChanges.getArray();

    {
    
    MutexGuard aGuard( rBHelper.rMutex );
    Reference < XInterface > xSource( (XPropertySet *)this, UNO_QUERY );
    sal_Int32 nFirePos = 0;
    for( i = 0; i < nLen; i++ )
    {
        if( pHandles[i] != -1 )
        {
            pChanges[nFirePos].Source = xSource;
            pChanges[nFirePos].PropertyName = pNames[i];
            pChanges[nFirePos].PropertyHandle = pHandles[i];
            getFastPropertyValue( pChanges[nFirePos].OldValue, pHandles[i] );
            pChanges[nFirePos].NewValue = pChanges[nFirePos].OldValue;
            nFirePos++;
        }
    }
    
    }
    if( nFireLen )
        rListener->propertiesChange( aChanges );

    delete [] pHandles;
}

void OPropertySetHelper2::enableChangeListenerNotification( sal_Bool bEnable )
    throw(::com::sun::star::uno::RuntimeException)
{
    m_pReserved->m_bFireEvents = bEnable;
}





extern "C" {

static int compare_Property_Impl( const void *arg1, const void *arg2 )
    SAL_THROW_EXTERN_C()
{
   return ((Property *)arg1)->Name.compareTo( ((Property *)arg2)->Name );
}

}

void OPropertyArrayHelper::init( sal_Bool bSorted ) SAL_THROW(())
{
    sal_Int32 i, nElements = aInfos.getLength();
    const Property* pProperties = aInfos.getConstArray();

    for( i = 1; i < nElements; i++ )
    {
        if(  pProperties[i-1].Name >= pProperties[i].Name )
        {
            if (bSorted) {
                OSL_FAIL( "Property array is not sorted" );
            }
            
            qsort( aInfos.getArray(), nElements, sizeof( Property ),
                    compare_Property_Impl );
            break;
        }
    }
    
    pProperties = aInfos.getConstArray();
    for( i = 0; i < nElements; i++ )
        if( pProperties[i].Handle != i )
            return;
    
    bRightOrdered = sal_True;
}

OPropertyArrayHelper::OPropertyArrayHelper(
    Property * pProps,
    sal_Int32 nEle,
    sal_Bool bSorted )
    SAL_THROW(())
    : m_pReserved(NULL)
    , aInfos(pProps, nEle)
    , bRightOrdered( sal_False )
{
    init( bSorted );
}

OPropertyArrayHelper::OPropertyArrayHelper(
    const Sequence< Property > & aProps,
    sal_Bool bSorted )
    SAL_THROW(())
    : m_pReserved(NULL)
    , aInfos(aProps)
    , bRightOrdered( sal_False )
{
    init( bSorted );
}


sal_Int32 OPropertyArrayHelper::getCount() const
{
    return aInfos.getLength();
}


sal_Bool OPropertyArrayHelper::fillPropertyMembersByHandle
(
    OUString * pPropName,
    sal_Int16 * pAttributes,
    sal_Int32 nHandle
)
{
    const Property* pProperties = aInfos.getConstArray();
    sal_Int32 nElements = aInfos.getLength();

    if( bRightOrdered )
    {
        if( nHandle < 0 || nHandle >= nElements )
            return sal_False;
        if( pPropName )
            *pPropName = pProperties[ nHandle ].Name;
        if( pAttributes )
            *pAttributes = pProperties[ nHandle ].Attributes;
        return sal_True;
    }
    else
    {
        
        for( sal_Int32 i = 0; i < nElements; i++ )
        {
            if( pProperties[i].Handle == nHandle )
            {
                if( pPropName )
                    *pPropName = pProperties[ i ].Name;
                if( pAttributes )
                    *pAttributes = pProperties[ i ].Attributes;
                return sal_True;
            }
        }
    }
    return sal_False;
}


Sequence< Property > OPropertyArrayHelper::getProperties(void)
{
    return aInfos;
}


Property OPropertyArrayHelper::getPropertyByName(const OUString& aPropertyName)
        throw (UnknownPropertyException)
{
    Property * pR;
    pR = (Property *)bsearch( &aPropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    if( !pR ) {
        throw UnknownPropertyException();
    }
    return *pR;
}


sal_Bool OPropertyArrayHelper::hasPropertyByName(const OUString& aPropertyName)
{
    Property * pR;
    pR = (Property *)bsearch( &aPropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    return pR != NULL;
}


sal_Int32 OPropertyArrayHelper::getHandleByName( const OUString & rPropName )
{
    Property * pR;
    pR = (Property *)bsearch( &rPropName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    return pR ? pR->Handle : -1;
}


sal_Int32 OPropertyArrayHelper::fillHandles( sal_Int32 * pHandles, const Sequence< OUString > & rPropNames )
{
    sal_Int32 nHitCount = 0;
    const OUString * pReqProps = rPropNames.getConstArray();
    sal_Int32 nReqLen = rPropNames.getLength();
    const Property * pCur = aInfos.getConstArray();
    const Property * pEnd = pCur + aInfos.getLength();

    for( sal_Int32 i = 0; i < nReqLen; i++ )
    {
        
        sal_Int32 n = (sal_Int32)(pEnd - pCur);
        sal_Int32 nLog = 0;
        while( n )
        {
            nLog += 1;
            n = n >> 1;
        }

        
        
        if( (nReqLen - i) * nLog >= pEnd - pCur )
        {
            
            while( pCur < pEnd && pReqProps[i] > pCur->Name )
            {
                pCur++;
            }
            if( pCur < pEnd && pReqProps[i] == pCur->Name )
            {
                pHandles[i] = pCur->Handle;
                nHitCount++;
            }
            else
                pHandles[i] = -1;
        }
        else
        {
            
            sal_Int32   nCompVal = 1;
            const Property * pOldEnd = pEnd--;
            const Property * pMid = pCur;

            while( nCompVal != 0 && pCur <= pEnd )
            {
                pMid = (pEnd - pCur) / 2 + pCur;

                nCompVal = pReqProps[i].compareTo( pMid->Name );

                if( nCompVal > 0 )
                    pCur = pMid + 1;
                else
                    pEnd = pMid - 1;
            }

            if( nCompVal == 0 )
            {
                pHandles[i] = pMid->Handle;
                nHitCount++;
                pCur = pMid +1;
            }
            else if( nCompVal > 0 )
            {
                pHandles[i] = -1;
                pCur = pMid +1;
            }
            else
            {
                pHandles[i] = -1;
                pCur = pMid;
            }
            pEnd = pOldEnd;
        }
    }
    return nHitCount;
}

} 



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
