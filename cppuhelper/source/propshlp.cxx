/*************************************************************************
 *
 *  $RCSfile: propshlp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 13:39:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif

#include <osl/diagnose.h>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>


using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace rtl;
using namespace cppu;

namespace cppu {

IPropertyArrayHelper::~IPropertyArrayHelper()
{
}

inline const ::com::sun::star::uno::Type & getPropertyTypeIdentifier( ) SAL_THROW( () )
{
    return ::getCppuType( (Reference< XPropertyChangeListener > *)0 );
}
inline const ::com::sun::star::uno::Type & getPropertiesTypeIdentifier() SAL_THROW( () )
{
    return ::getCppuType( (Reference< XPropertiesChangeListener > *)0 );
}
inline const ::com::sun::star::uno::Type & getVetoableTypeIdentifier() SAL_THROW( () )
{
    return ::getCppuType( (Reference< XVetoableChangeListener > *)0 );
}

int SAL_CALL compare_OUString_Property_Impl( const void *arg1, const void *arg2 ) SAL_THROW( () )
{
   return ((OUString *)arg1)->compareTo( ((Property *)arg2)->Name );
}

/**
 * The class which implements the PropertySetInfo interface.
 */
class OPropertySetHelperInfo_Impl
    : public WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
    Sequence < Property > aInfos;

public:
    OPropertySetHelperInfo_Impl( IPropertyArrayHelper & rHelper_ ) SAL_THROW( () );

    // XPropertySetInfo-Methoden
    virtual Sequence< Property > SAL_CALL getProperties(void);
    virtual Property SAL_CALL getPropertyByName(const OUString& PropertyName);
    virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& PropertyName);
};


/**
 * Create an object that implements XPropertySetInfo IPropertyArrayHelper.
 */
OPropertySetHelperInfo_Impl::OPropertySetHelperInfo_Impl(
    IPropertyArrayHelper & rHelper_ )
    SAL_THROW( () )
    :aInfos( rHelper_.getProperties() )
{
}

/**
 * Return the sequence of properties, which are provided throug the constructor.
 */
Sequence< Property > OPropertySetHelperInfo_Impl::getProperties(void)
{
    return aInfos;
}

/**
 * Return the sequence of properties, which are provided throug the constructor.
 */
Property OPropertySetHelperInfo_Impl::getPropertyByName( const OUString & PropertyName )
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
sal_Bool OPropertySetHelperInfo_Impl::hasPropertyByName( const OUString & PropertyName )
{
    Property * pR;
    pR = (Property *)bsearch( &PropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    return pR != NULL;
}

//  ----------------------------------------------------
//  class PropertySetHelper
//  ----------------------------------------------------
OPropertySetHelper::OPropertySetHelper(
    OBroadcastHelper  & rBHelper_ )
    SAL_THROW( () )
    : rBHelper( rBHelper_ )
    , aBoundLC( rBHelper_.rMutex )
    , aVetoableLC( rBHelper_.rMutex )
{
}

/**
 * You must call disposing before.
 */
OPropertySetHelper::~OPropertySetHelper() SAL_THROW( () )
{
}

/**
 * These method is called from queryInterface, if no delegator is set.
 * Otherwise this method is called from the delegator.
 */
// XAggregation
Any OPropertySetHelper::queryInterface( const ::com::sun::star::uno::Type & rType )
    throw (RuntimeException)
{
    return ::cppu::queryInterface(
        rType,
        static_cast< XPropertySet * >( this ),
        static_cast< XMultiPropertySet * >( this ),
        static_cast< XFastPropertySet * >( this ) );
}

// ComponentHelper
void OPropertySetHelper::disposing() SAL_THROW( () )
{
    // Create an event with this as sender
    Reference < XPropertySet  > rSource( SAL_STATIC_CAST( XPropertySet * , this ) , UNO_QUERY );
    EventObject aEvt;
    aEvt.Source = rSource;

    // inform all listeners to reelease this object
    // The listener container are automaticly cleared
    aBoundLC.disposeAndClear( aEvt );
    aVetoableLC.disposeAndClear( aEvt );
}

Reference < XPropertySetInfo > OPropertySetHelper::createPropertySetInfo(
    IPropertyArrayHelper & rProperties ) SAL_THROW( () )
{
    return static_cast< XPropertySetInfo * >( new OPropertySetHelperInfo_Impl( rProperties ) );
}

// XPropertySet
void OPropertySetHelper::setPropertyValue(
    const OUString& rPropertyName, const Any& rValue )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    // get the map table
    IPropertyArrayHelper & rPH = getInfoHelper();
    // map the name to the handle
    sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
    // call the method of the XFastPropertySet interface
    setFastPropertyValue( nHandle, rValue );
}

// XPropertySet
Any OPropertySetHelper::getPropertyValue(
    const OUString& rPropertyName )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    // get the map table
    IPropertyArrayHelper & rPH = getInfoHelper();
    // map the name to the handle
    sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
    // call the method of the XFastPropertySet interface
    return getFastPropertyValue( nHandle );
}

// XPropertySet
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
        // only add listeners if you are not disposed
        // a listener with no name means all properties
        if( rPropertyName.len() )
        {
            // get the map table
            IPropertyArrayHelper & rPH = getInfoHelper();
            // map the name to the handle
            sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
            if( nHandle == -1 ) {
                // property not known throw exception
                throw  UnknownPropertyException() ;
            }

            sal_Int16 nAttributes;
            rPH.fillPropertyMembersByHandle( NULL, &nAttributes, nHandle );
            if( !(nAttributes & ::com::sun::star::beans::PropertyAttribute::BOUND) )
            {
                OSL_ENSURE( sal_False, "add listener to an unbound property" );
                // silent ignore this
                return;
            }
            // add the change listener to the helper container

            aBoundLC.addInterface( (sal_Int32)nHandle, rxListener );
        }
        else
            // add the change listener to the helper container
            rBHelper.aLC.addInterface(
                            getPropertyTypeIdentifier(  ),
                            rxListener
                                     );
    }
}


// XPropertySet
void OPropertySetHelper::removePropertyChangeListener(
    const OUString& rPropertyName,
    const Reference < XPropertyChangeListener >& rxListener )
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::lang::WrappedTargetException,
          ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    // all listeners are automaticly released in a dispose call
    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
    {
        if( rPropertyName.len() )
        {
            // get the map table
            IPropertyArrayHelper & rPH = getInfoHelper();
            // map the name to the handle
            sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
            if( nHandle == -1 )
                // property not known throw exception
                throw UnknownPropertyException();
            aBoundLC.removeInterface( (sal_Int32)nHandle, rxListener );
        }
        else {
            // remove the change listener to the helper container
            rBHelper.aLC.removeInterface(
                            getPropertyTypeIdentifier(  ),
                            rxListener
                                        );
        }
    }
}

// XPropertySet
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
        // only add listeners if you are not disposed
        // a listener with no name means all properties
        if( rPropertyName.len() )
        {
            // get the map table
            IPropertyArrayHelper & rPH = getInfoHelper();
            // map the name to the handle
            sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
            if( nHandle == -1 ) {
                // property not known throw exception
                throw UnknownPropertyException();
            }

            sal_Int16 nAttributes;
            rPH.fillPropertyMembersByHandle( NULL, &nAttributes, nHandle );
            if( !(nAttributes & PropertyAttribute::CONSTRAINED) )
            {
                OSL_ENSURE( sal_False, "addVetoableChangeListener, and property is not constrained" );
                // silent ignore this
                return;
            }
            // add the vetoable listener to the helper container
            aVetoableLC.addInterface( (sal_Int32)nHandle, rxListener );
        }
        else
            // add the vetoable listener to the helper container
            rBHelper.aLC.addInterface(
                                getVetoableTypeIdentifier(  ),
                                rxListener
                                     );
    }
}

// XPropertySet
void OPropertySetHelper::removeVetoableChangeListener(
    const OUString& rPropertyName,
    const Reference < XVetoableChangeListener > & rxListener )
    throw(::com::sun::star::beans::UnknownPropertyException,
          ::com::sun::star::lang::WrappedTargetException,
          ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    // all listeners are automaticly released in a dispose call
    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
    {
        if( rPropertyName.len() )
        {
            // get the map table
            IPropertyArrayHelper & rPH = getInfoHelper();
            // map the name to the handle
            sal_Int32 nHandle = rPH.getHandleByName( rPropertyName );
            if( nHandle == -1 ) {
                // property not known throw exception
                throw UnknownPropertyException();
            }
            // remove the vetoable listener to the helper container
            aVetoableLC.removeInterface( (sal_Int32)nHandle, rxListener );
        }
        else
            // add the vetoable listener to the helper container
            rBHelper.aLC.removeInterface(
                                getVetoableTypeIdentifier( ),
                                rxListener
                                        );
    }
}

// XFastPropertySet
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
        // unknown property
        throw UnknownPropertyException();
    }
    if( nAttributes & PropertyAttribute::READONLY )
        throw PropertyVetoException();

    Any aConvertedVal;
    Any aOldVal;

    // Will the property change?
    sal_Bool bChanged;
    {
        MutexGuard aGuard( rBHelper.rMutex );
        bChanged = convertFastPropertyValue( aConvertedVal, aOldVal, nHandle, rValue );
        // release guard to fire events
    }
    if( bChanged )
    {
        // Is it a constrained property?
        if( nAttributes & PropertyAttribute::CONSTRAINED )
        {
            // In aValue is the converted rValue
            // fire a constarined event
            // second parameter NULL means constrained
            fire( &nHandle, &rValue, &aOldVal, 1, sal_True );
        }

        {
        MutexGuard aGuard( rBHelper.rMutex );
        // set the property to the new value
        setFastPropertyValue_NoBroadcast( nHandle, aConvertedVal );
        // release guard to fire events
        }
        // file a change event, if the value changed
        fire( &nHandle, &rValue, &aOldVal, 1, sal_False );
    }
}

// XFastPropertySet
Any OPropertySetHelper::getFastPropertyValue( sal_Int32 nHandle )
     throw(::com::sun::star::beans::UnknownPropertyException,
           ::com::sun::star::lang::WrappedTargetException,
           ::com::sun::star::uno::RuntimeException)

{
    IPropertyArrayHelper & rInfo = getInfoHelper();
    if( !rInfo.fillPropertyMembersByHandle( NULL, NULL, nHandle ) )
        // unknown property
        throw UnknownPropertyException();

    Any aRet;
    MutexGuard aGuard( rBHelper.rMutex );
    getFastPropertyValue( aRet, nHandle );
    return aRet;
}

//--------------------------------------------------------------------------
void OPropertySetHelper::fire
(
    sal_Int32 * pnHandles,
    const Any * pNewValues,
    const Any * pOldValues,
    sal_Int32 nHandles, // These is the Count of the array
    sal_Bool bVetoable
)
{
    // Only fire, if one or more properties changed
    if( nHandles )
    {
        // create the event sequence of all changed properties
        Sequence< PropertyChangeEvent > aEvts( nHandles );
        PropertyChangeEvent * pEvts = aEvts.getArray();
        Reference < XInterface > xSource( (XPropertySet *)this, UNO_QUERY );
        sal_Int32 i;
        sal_Int32 nChangesLen = 0;
        // Loop over all changed properties to fill the event struct
        for( i = 0; i < nHandles; i++ )
        {
            // Vetoable fire and constrained attribute set or
            // Change fire and Changed and bound attribute set
            IPropertyArrayHelper & rInfo = getInfoHelper();
            sal_Int16   nAttributes;
            OUString aPropName;
            rInfo.fillPropertyMembersByHandle( &aPropName, &nAttributes, pnHandles[i] );

            if( bVetoable && (nAttributes & PropertyAttribute::CONSTRAINED)
              || !bVetoable && (nAttributes & PropertyAttribute::BOUND) )
            {
                pEvts[nChangesLen].Source = xSource;
                pEvts[nChangesLen].PropertyName = aPropName;
                pEvts[nChangesLen].PropertyHandle = pnHandles[i];
                pEvts[nChangesLen].OldValue = pOldValues[i];
                pEvts[nChangesLen].NewValue = pNewValues[i];
                nChangesLen++;
            }
        }

        // fire the events for all changed properties
        for( i = 0; i < nChangesLen; i++ )
        {
            // get the listener container for the property name
            OInterfaceContainerHelper * pLC;
            if( bVetoable ) // fire change Events?
                pLC = aVetoableLC.getContainer( pEvts[i].PropertyHandle );
            else
                pLC = aBoundLC.getContainer( pEvts[i].PropertyHandle );
            if( pLC )
            {
                // Ueber alle Listener iterieren und Events senden
                OInterfaceIteratorHelper aIt( *pLC);
                while( aIt.hasMoreElements() )
                {
                    XInterface * pL = aIt.next();
                    if( bVetoable ) // fire change Events?
                        ((XVetoableChangeListener *)pL)->vetoableChange( pEvts[i] );
                    else
                        ((XPropertyChangeListener *)pL)->propertyChange( pEvts[i] );
                }
            }
            // broadcast to all listeners with "" property name
            if( bVetoable ){
                // fire change Events?
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
                // Ueber alle Listener iterieren und Events senden
                OInterfaceIteratorHelper aIt( *pLC);
                while( aIt.hasMoreElements() )
                {
                    XInterface * pL = aIt.next();
                    if( bVetoable ) // fire change Events?
                        ((XVetoableChangeListener *)pL)->vetoableChange( pEvts[i] );
                    else
                        ((XPropertyChangeListener *)pL)->propertyChange( pEvts[i] );
                }
            }
        }

        // reduce array to changed properties
        aEvts.realloc( nChangesLen );

        if( !bVetoable )
        {
            OInterfaceContainerHelper * pCont = 0;
            pCont = rBHelper.aLC.getContainer(
                                getPropertiesTypeIdentifier(  )
                                             );
            if( pCont )
            {
                // Here is a Bug, unbound properties are also fired
                OInterfaceIteratorHelper aIt( *pCont );
                while( aIt.hasMoreElements() )
                {
                    XPropertiesChangeListener * pL = (XPropertiesChangeListener *)aIt.next();
                    // fire the hole event sequence to the XPropertiesChangeListener's
                    pL->propertiesChange( aEvts );
                }
            }
        }
    }
}

// OPropertySetHelper
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
        // get the map table
        IPropertyArrayHelper & rPH = getInfoHelper();

        pConvertedValues = new Any[ nHitCount ];
        pOldValues = new Any[ nHitCount ];
        sal_Int32 nHitCount = 0;
        sal_Int32 i;

        {
        // must lock the mutex outside the loop. So all values are consistent.
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
                // Will the property change?
                if( convertFastPropertyValue( pConvertedValues[ nHitCount ], pOldValues[nHitCount],
                                            pHandles[i], pValues[i] ) )
                {
                    // only increment if the property really change
                    pHandles[nHitCount]         = pHandles[i];
                    nHitCount++;
                }
            }
        }
        // release guard to fire events
        }

        // fire vetoable events
        fire( pHandles, pConvertedValues, pOldValues, nHitCount, sal_True );

        {
        // must lock the mutex outside the loop.
        MutexGuard aGuard( rBHelper.rMutex );
        // Loop over all changed properties
        for( i = 0; i < nHitCount; i++ )
        {
            // Will the property change?
            setFastPropertyValue_NoBroadcast( pHandles[i], pConvertedValues[i] );
        }
        // release guard to fire events
        }

        // fire change events
        fire( pHandles, pConvertedValues, pOldValues, nHitCount, sal_False );
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

// XMultiPropertySet
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
        // get the map table
        IPropertyArrayHelper & rPH = getInfoHelper();
        // fill the handle array
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

// XMultiPropertySet
Sequence<Any> OPropertySetHelper::getPropertyValues( const Sequence<OUString>& rPropertyNames )
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32   nSeqLen = rPropertyNames.getLength();
    sal_Int32 * pHandles = new sal_Int32[ nSeqLen ];
    Sequence< Any > aValues( nSeqLen );

    // get the map table
    IPropertyArrayHelper & rPH = getInfoHelper();
    // fill the handle array
    rPH.fillHandles( pHandles, rPropertyNames );

    Any * pValues = aValues.getArray();
    const OUString * pNames = rPropertyNames.getConstArray();

    MutexGuard aGuard( rBHelper.rMutex );
    // fill the sequence with the values
    for( sal_Int32 i = 0; i < nSeqLen; i++ )
        getFastPropertyValue( pValues[i], pHandles[i] );

    delete [] pHandles;
    return aValues;
}

// XMultiPropertySet
void OPropertySetHelper::addPropertiesChangeListener(
    const Sequence<OUString> & ,
    const Reference < XPropertiesChangeListener > & rListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    rBHelper.addListener( getCppuType(&rListener) , rListener );
}

// XMultiPropertySet
void OPropertySetHelper::removePropertiesChangeListener(
    const Reference < XPropertiesChangeListener > & rListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    rBHelper.removeListener( getCppuType(&rListener) , rListener );
}

// XMultiPropertySet
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

    // get the count of matching properties
    sal_Int32 nFireLen = 0;
    sal_Int32 i;
    for( i = 0; i < nLen; i++ )
        if( pHandles[i] != -1 )
            nFireLen++;

    Sequence<PropertyChangeEvent> aChanges( nFireLen );
    PropertyChangeEvent* pChanges = aChanges.getArray();

    sal_Int32 nFirePos = 0;
    {
    // must lock the mutex outside the loop. So all values are consistent.
    MutexGuard aGuard( rBHelper.rMutex );
    Reference < XInterface > xSource( (XPropertySet *)this, UNO_QUERY );
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
    // release guard to fire events
    }
    if( nFireLen )
        rListener->propertiesChange( aChanges );

    delete [] pHandles;
}

#ifdef xdvnsdfln
// XPropertyState
PropertyState OPropertySetHelper::getPropertyState( const OUString& PropertyName )
{
    PropertyState aState;
    return aState;
}

// XPropertyState
Sequence< PropertyState > OPropertySetHelper::getPropertyStates( const Sequence< OUString >& PropertyNames )
{
    ULONG nNames = PropertyNames.getLength();
    const OUString* pNames = PropertyNames.getConstArray();

    Sequence< PropertyState > aStates( nNames );
    return aStates;

}

void OPropertySetHelper::setPropertyToDefault( const OUString& aPropertyName )
{
    setPropertyValue( aPropertyName, Any() );
}

Any OPropertySetHelper::getPropertyDefault( const OUString& aPropertyName ) const
{
    return Any();
}

void OPropertySetHelper::addPropertyStateChangeListener( const OUString& aPropertyName, const XPropertyStateChangeListenerRef& Listener )
{
}

void OPropertySetHelper::removePropertyStateChangeListener( const OUString& aPropertyName, const XPropertyStateChangeListenerRef& Listener )
{
}
#endif

//========================================================================
//== OPropertyArrayHelper ================================================
//========================================================================

//========================================================================

//  static OUString makeOUString( sal_Char *p )
//  {
//      sal_Int32 nLen = strlen(p);
//      sal_Unicode *pw = new sal_Unicode[nLen];

//      for( int i = 0 ; i < nLen ; i ++ ) {

//          // Only ascii strings allowed with this helper !
//          OSL_ASSERT( p[i] < 127 );
//          pw[i] = p[i];
//      }
//      OUString ow( pw , nLen );
//      delete pw;
//      return ow;
//  }

int SAL_CALL compare_Property_Impl( const void *arg1, const void *arg2 ) SAL_THROW( () )
{
   return ((Property *)arg1)->Name.compareTo( ((Property *)arg2)->Name );
}

void OPropertyArrayHelper::init( sal_Bool bSorted ) SAL_THROW( () )
{
    sal_Int32 i, nElements = aInfos.getLength();
    const Property* pProperties = aInfos.getConstArray();

    for( i = 1; i < nElements; i++ )
    {
        if(  pProperties[i-1].Name >= pProperties[i].Name )
        {
            OSL_ENSURE( !bSorted, "Property array is not sorted" );
            // not sorted
            qsort( aInfos.getArray(), nElements, sizeof( Property ),
                    compare_Property_Impl );
            break;
        }
    }
    // may be that the array is resorted
    pProperties = aInfos.getConstArray();
    for( i = 0; i < nElements; i++ )
        if( pProperties[i].Handle != i )
            return;
    // The handle is the index
    bRightOrdered = sal_True;
}

OPropertyArrayHelper::OPropertyArrayHelper(
    Property * pProps,
    sal_Int32 nEle,
    sal_Bool bSorted )
    SAL_THROW( () )
    : aInfos(pProps, nEle)
    , bRightOrdered( sal_False )
{
    init( bSorted );
}

OPropertyArrayHelper::OPropertyArrayHelper(
    const Sequence< Property > & aProps,
    sal_Bool bSorted )
    SAL_THROW( () )
    : aInfos(aProps)
    , bRightOrdered( sal_False )
{
    init( bSorted );
}

//========================================================================
sal_Int32 OPropertyArrayHelper::getCount() const
{
    return aInfos.getLength();
}

//========================================================================
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
        // normally the array is sorted
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

//========================================================================
Sequence< Property > OPropertyArrayHelper::getProperties(void)
{
    /*if( aInfos.getLength() != nElements )
    {
        ((OPropertyArrayHelper *)this)->aInfos.realloc( nElements );
        Property * pProps = ((OPropertyArrayHelper *)this)->aInfos.getArray();
        for( sal_Int32 i = 0; i < nElements; i++ )
        {
            pProps[i].Name = pProperties[i].Name;
            pProps[i].Handle = pProperties[i].Handle;
            pProps[i].Type = pProperties[i].Type;
            pProps[i].Attributes = pProperties[i].Attributes;
        }
    }*/
    return aInfos;
}

//========================================================================
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

    /*Property aProp;
    aProp.Name = pR->Name;
    aProp.Handle = pR->Handle;
    aProp.Type = pR->Type;
    aProp.Attributes = pR->Attributes;
    return aProp;*/
    return *pR;
}

//========================================================================
sal_Bool OPropertyArrayHelper::hasPropertyByName(const OUString& aPropertyName)
{
    Property * pR;
    pR = (Property *)bsearch( &aPropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    return pR != NULL;
}

//========================================================================
sal_Int32 OPropertyArrayHelper::getHandleByName( const OUString & rPropName )
{
    Property * pR;
    pR = (Property *)bsearch( &rPropName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    return pR ? pR->Handle : -1;
}

//========================================================================
sal_Int32 OPropertyArrayHelper::fillHandles( sal_Int32 * pHandles, const Sequence< OUString > & rPropNames )
{
    sal_Int32 nHitCount = 0;
    const OUString * pReqProps = rPropNames.getConstArray();
    sal_Int32 nReqLen = rPropNames.getLength();
    const Property * pCur = aInfos.getConstArray();
    const Property * pEnd = pCur + aInfos.getLength();

    for( sal_Int32 i = 0; i < nReqLen; i++ )
    {
        // Logarithmus ermitteln
        sal_Int32 n = (sal_Int32)(pEnd - pCur);
        sal_Int32 nLog = 0;
        while( n )
        {
            nLog += 1;
            n = n >> 1;
        }

        // Anzahl der noch zu suchenden Properties * dem Log2 der verbleibenden
        // zu dursuchenden Properties.
        if( (nReqLen - i) * nLog >= pEnd - pCur )
        {
            // linear search is better
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
            // binary search is better
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

} // end namespace cppu



