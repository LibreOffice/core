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

#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <set>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <boost/unordered_map.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::container;
using namespace com::sun::star::registry;
using namespace com::sun::star::beans;
using namespace com::sun::star::beans::PropertyAttribute;
using namespace com::sun::star::beans::PropertyConcept;
using namespace com::sun::star::beans::MethodConcept;
using namespace cppu;
using namespace osl;

namespace
{

typedef WeakImplHelper3< XIntrospectionAccess, XMaterialHolder, XExactName > IntrospectionAccessHelper;




// Special value for Method-Concept, to be able to mark "normal" functions
#define  MethodConcept_NORMAL_IMPL        0x80000000


// Method to assert, if a class is derived from another class
sal_Bool isDerivedFrom( Reference<XIdlClass> xToTestClass, Reference<XIdlClass> xDerivedFromClass )
{
    Sequence< Reference<XIdlClass> > aClassesSeq = xToTestClass->getSuperclasses();
    const Reference<XIdlClass>* pClassesArray = aClassesSeq.getConstArray();

    sal_Int32 nSuperClassCount = aClassesSeq.getLength();
    for ( sal_Int32 i = 0; i < nSuperClassCount; ++i )
    {
        const Reference<XIdlClass>& rxClass = pClassesArray[i];

        if ( xDerivedFromClass->equals( rxClass ) ||
             isDerivedFrom( rxClass, xDerivedFromClass )
           )
            return sal_True;
    }

    return sal_False;
}



// *** Classification of Properties (no enum, to be able to use Sequence) ***
// Properties from a PropertySet-Interface
#define MAP_PROPERTY_SET    0
// Properties from Fields
#define MAP_FIELD            1
// Properties, that get described with get/set-Methods
#define MAP_GETSET            2
// Properties, with only a set-Method
#define MAP_SETONLY            3


// Increments, in which the size of Sequences get adjusted
#define ARRAY_SIZE_STEP        20




//*** IntrospectionAccessStatic_Impl ***

// Equals to the old IntrospectionAccessImpl, forms now a static
// part of the new Instance-related ImplIntrospectionAccess

// Hashtable for the search of names
struct hashName_Impl
{
    size_t operator()(const OUString Str) const
    {
        return (size_t)Str.hashCode();
    }
};

struct eqName_Impl
{
    sal_Bool operator()(const OUString Str1, const OUString Str2) const
    {
        return ( Str1 == Str2 );
    }
};

typedef boost::unordered_map
<
    OUString,
    sal_Int32,
    hashName_Impl,
    eqName_Impl
>
IntrospectionNameMap;


// Hashtable to assign exact names to the Lower-Case
// converted names, for the support of XExactName
typedef boost::unordered_map
<
    OUString,
    OUString,
    hashName_Impl,
    eqName_Impl
>
LowerToExactNameMap;


class ImplIntrospectionAccess;
class IntrospectionAccessStatic_Impl: public salhelper::SimpleReferenceObject
{
    friend class Implementation;
    friend class ImplIntrospectionAccess;

    // Holding CoreReflection
    Reference< XIdlReflection > mxCoreReflection;

    // InterfaceSequences, to save additional information in a property
    // for example the Field at MAP_FIELD, the get/set-Methods at MAP_GETSET, et cetera
    Sequence< Reference<XInterface> > aInterfaceSeq1;
    Sequence< Reference<XInterface> > aInterfaceSeq2;

    // Hashtables for names
    IntrospectionNameMap maPropertyNameMap;
    IntrospectionNameMap maMethodNameMap;
    LowerToExactNameMap  maLowerToExactNameMap;

    // Sequence of all Properties, also for delivering from getProperties()
    Sequence<Property> maAllPropertySeq;

    // Mapping of properties to Access-Types
    Sequence<sal_Int16> maMapTypeSeq;

    // Classification of found methods
    Sequence<sal_Int32> maPropertyConceptSeq;

    // Number of Properties
    sal_Int32 mnPropCount;

    // Number of Properties, which are assigned to particular concepts
    //sal_Int32 mnDangerousPropCount;
    sal_Int32 mnPropertySetPropCount;
    sal_Int32 mnAttributePropCount;
    sal_Int32 mnMethodPropCount;

    // Flag, if a FastPropertySet is supported
    sal_Bool mbFastPropSet;

    // Original-Handles of FastPropertySets
    sal_Int32* mpOrgPropertyHandleArray;

    // MethodSequence, that accepts all methods
    Sequence< Reference<XIdlMethod> > maAllMethodSeq;

    // Classification of found methods
    Sequence<sal_Int32> maMethodConceptSeq;

    // Number of methods
    sal_Int32 mnMethCount;

    // Sequence of Listener, that can be registered
    Sequence< Type > maSupportedListenerSeq;

    // BaseInit (should be done later in the application!)
    void BaseInit( void );

    // Helper-methods for adjusting sizes of Sequences
    void checkPropertyArraysSize
    (
        Property*& rpAllPropArray,
        sal_Int16*& rpMapTypeArray,
        sal_Int32*& rpPropertyConceptArray,
        sal_Int32 iNextIndex
    );
    void checkInterfaceArraySize( Sequence< Reference<XInterface> >& rSeq, Reference<XInterface>*& rpInterfaceArray,
        sal_Int32 iNextIndex );

public:
    IntrospectionAccessStatic_Impl( Reference< XIdlReflection > xCoreReflection_ );
    ~IntrospectionAccessStatic_Impl()
    {
        delete[] mpOrgPropertyHandleArray;
    }
    sal_Int32 getPropertyIndex( const OUString& aPropertyName ) const;
    sal_Int32 getMethodIndex( const OUString& aMethodName ) const;

    // Methods of XIntrospectionAccess (OLD, now only Impl)
    void setPropertyValue(const Any& obj, const OUString& aPropertyName, const Any& aValue) const;
//    void setPropertyValue(Any& obj, const OUString& aPropertyName, const Any& aValue) const;
    Any getPropertyValue(const Any& obj, const OUString& aPropertyName) const;
    void setPropertyValueByIndex(const Any& obj, sal_Int32 nIndex, const Any& aValue) const;
//    void setPropertyValueByIndex(Any& obj, sal_Int32 nIndex, const Any& aValue) const;
    Any getPropertyValueByIndex(const Any& obj, sal_Int32 nIndex) const;

    Sequence<Property> getProperties(void) const                        { return maAllPropertySeq; }
    Sequence< Reference<XIdlMethod> > getMethods(void) const            { return maAllMethodSeq; }
    Sequence< Type > getSupportedListeners(void) const                    { return maSupportedListenerSeq; }
    Sequence<sal_Int32> getPropertyConcepts(void) const                    { return maPropertyConceptSeq; }
    Sequence<sal_Int32> getMethodConcepts(void) const                    { return maMethodConceptSeq; }
};


// Ctor
IntrospectionAccessStatic_Impl::IntrospectionAccessStatic_Impl( Reference< XIdlReflection > xCoreReflection_ )
    : mxCoreReflection( xCoreReflection_ )
{
    aInterfaceSeq1.realloc( ARRAY_SIZE_STEP );
    aInterfaceSeq2.realloc( ARRAY_SIZE_STEP );

    // Property-Data
    maAllPropertySeq.realloc( ARRAY_SIZE_STEP );
    maMapTypeSeq.realloc( ARRAY_SIZE_STEP );
    maPropertyConceptSeq.realloc( ARRAY_SIZE_STEP );

    mbFastPropSet = sal_False;
    mpOrgPropertyHandleArray = NULL;

    mnPropCount = 0;
    //mnDangerousPropCount = 0;
    mnPropertySetPropCount = 0;
    mnAttributePropCount = 0;
    mnMethodPropCount = 0;

    // Method-Data
    mnMethCount = 0;
}

sal_Int32 IntrospectionAccessStatic_Impl::getPropertyIndex( const OUString& aPropertyName ) const
{
    sal_Int32 iHashResult = -1;
    IntrospectionAccessStatic_Impl* pThis = (IntrospectionAccessStatic_Impl*)this;
    IntrospectionNameMap::iterator aIt = pThis->maPropertyNameMap.find( aPropertyName );
    if( !( aIt == pThis->maPropertyNameMap.end() ) )
        iHashResult = (*aIt).second;
    return iHashResult;
}

sal_Int32 IntrospectionAccessStatic_Impl::getMethodIndex( const OUString& aMethodName ) const
{
    sal_Int32 iHashResult = -1;
    IntrospectionAccessStatic_Impl* pThis = (IntrospectionAccessStatic_Impl*)this;
    IntrospectionNameMap::iterator aIt = pThis->maMethodNameMap.find( aMethodName );
    if( !( aIt == pThis->maMethodNameMap.end() ) )
    {
        iHashResult = (*aIt).second;
    }
    // #95159 Check if full qualified name matches
    else
    {
        sal_Int32 nSearchFrom = aMethodName.getLength();
        while( true )
        {
            // Strategy: Search back until the first '_' is found
            sal_Int32 nFound = aMethodName.lastIndexOf( '_', nSearchFrom );
            if( nFound == -1 )
                break;

            OUString aPureMethodName = aMethodName.copy( nFound + 1 );

            aIt = pThis->maMethodNameMap.find( aPureMethodName );
            if( !( aIt == pThis->maMethodNameMap.end() ) )
            {
                // Check if it can be a type?
                // Problem: Does not work if package names contain _ ?!
                OUString aStr = aMethodName.copy( 0, nFound );
                OUString aTypeName = aStr.replace( '_', '.' );
                Reference< XIdlClass > xClass = mxCoreReflection->forName( aTypeName );
                if( xClass.is() )
                {
                    // If this is a valid class it could be the right method

                    // Could be the right method, type has to be checked
                    iHashResult = (*aIt).second;

                    const Reference<XIdlMethod>* pMethods = maAllMethodSeq.getConstArray();
                    const Reference<XIdlMethod> xMethod = pMethods[ iHashResult ];

                    Reference< XIdlClass > xMethClass = xMethod->getDeclaringClass();
                    if( xClass->equals( xMethClass ) )
                    {
                        break;
                    }
                    else
                    {
                        iHashResult = -1;

                        // Could also be another method with the same name
                        // Iterate over all methods
                        sal_Int32 nLen = maAllMethodSeq.getLength();
                        for( int i = 0 ; i < nLen ; ++i )
                        {
                            const Reference<XIdlMethod> xMethod2 = pMethods[ i ];
                            if( xMethod2->getName() == aPureMethodName )
                            {
                                Reference< XIdlClass > xMethClass2 = xMethod2->getDeclaringClass();

                                if( xClass->equals( xMethClass2 ) )
                                {
                                    iHashResult = i;
                                    break;
                                }
                            }
                        }

                        if( iHashResult != -1 )
                            break;
                    }
                }
            }

            nSearchFrom = nFound - 1;
            if( nSearchFrom < 0 )
                break;
        }
    }
    return iHashResult;
}

void IntrospectionAccessStatic_Impl::setPropertyValue( const Any& obj, const OUString& aPropertyName, const Any& aValue ) const
//void IntrospectionAccessStatic_Impl::setPropertyValue( Any& obj, const OUString& aPropertyName, const Any& aValue ) const
{
    sal_Int32 i = getPropertyIndex( aPropertyName );
    if( i != -1 )
        setPropertyValueByIndex( obj, (sal_Int32)i, aValue );
    else
        throw UnknownPropertyException();
}

void IntrospectionAccessStatic_Impl::setPropertyValueByIndex(const Any& obj, sal_Int32 nSequenceIndex, const Any& aValue) const
//void IntrospectionAccessStatic_Impl::setPropertyValueByIndex( Any& obj, sal_Int32 nSequenceIndex, const Any& aValue) const
{
    // Is the passed object something that fits?
    TypeClass eObjType = obj.getValueType().getTypeClass();

    Reference<XInterface> xInterface;
    if( eObjType == TypeClass_INTERFACE )
    {
        xInterface = *( Reference<XInterface>*)obj.getValue();
    }
    else if( nSequenceIndex >= mnPropCount || ( eObjType != TypeClass_STRUCT && eObjType != TypeClass_EXCEPTION ) )
    {
        throw IllegalArgumentException();
    }

    // Test flags
    const Property* pProps = maAllPropertySeq.getConstArray();
    if( (pProps[ nSequenceIndex ].Attributes & READONLY) != 0 )
    {
        throw UnknownPropertyException();
    }

    const sal_Int16* pMapTypeArray = maMapTypeSeq.getConstArray();
    switch( pMapTypeArray[ nSequenceIndex ] )
    {
        case MAP_PROPERTY_SET:
        {
            // Get Property
            const Property& rProp = maAllPropertySeq.getConstArray()[ nSequenceIndex ];

            // Convert Interface-Parameter to the correct type
            sal_Bool bUseCopy = sal_False;
            Any aRealValue;

            TypeClass eValType = aValue.getValueType().getTypeClass();
            if( eValType == TypeClass_INTERFACE )
            {
                Type aPropType = rProp.Type;
                OUString aTypeName( aPropType.getTypeName() );
                Reference< XIdlClass > xPropClass = mxCoreReflection->forName( aTypeName );
                //Reference<XIdlClass> xPropClass = rProp.Type;
                if( xPropClass.is() && xPropClass->getTypeClass() == TypeClass_INTERFACE )
                {
                    Reference<XInterface> valInterface = *(Reference<XInterface>*)aValue.getValue();
                    if( valInterface.is() )
                    {
                        //Any queryInterface( const Type& rType );
                        aRealValue = valInterface->queryInterface( aPropType );
                        if( aRealValue.hasValue() )
                            bUseCopy = sal_True;
                    }
                }
            }

            // Do we have a FastPropertySet and a valid Handle?
            // CAUTION: At this point we exploit, that the PropertySet
            // gets queried at the beginning of the Introspection-Process.
            sal_Int32 nOrgHandle;
            if( mbFastPropSet && ( nOrgHandle = mpOrgPropertyHandleArray[ nSequenceIndex ] ) != -1 )
            {
                // Retrieve PropertySet-Interface
                Reference<XFastPropertySet> xFastPropSet =
                    Reference<XFastPropertySet>::query( xInterface );
                if( xFastPropSet.is() )
                {
                    xFastPropSet->setFastPropertyValue( nOrgHandle, bUseCopy ? aRealValue : aValue );
                }
                else
                {
                    // throw UnknownPropertyException
                }
            }
            // else take the normal one
            else
            {
                // Retrieve PropertySet-Interface
                Reference<XPropertySet> xPropSet =
                    Reference<XPropertySet>::query( xInterface );
                if( xPropSet.is() )
                {
                    xPropSet->setPropertyValue( rProp.Name, bUseCopy ? aRealValue : aValue );
                }
                else
                {
                    // throw UnknownPropertyException
                }
            }
        }
        break;

        case MAP_FIELD:
        {
            Reference<XIdlField> xField = (XIdlField*)(aInterfaceSeq1.getConstArray()[ nSequenceIndex ].get());
            Reference<XIdlField2> xField2(xField, UNO_QUERY);
            if( xField2.is() )
            {
                xField2->set( (Any&)obj, aValue );
                // IllegalArgumentException
                // NullPointerException
            } else
            if( xField.is() )
            {
                xField->set( obj, aValue );
                // IllegalArgumentException
                // NullPointerException
            }
            else
            {
                // throw IllegalArgumentException();
            }
        }
        break;

        case MAP_GETSET:
        case MAP_SETONLY:
        {
            // Retrieve set-Methods
            Reference<XIdlMethod> xMethod = (XIdlMethod*)(aInterfaceSeq2.getConstArray()[ nSequenceIndex ].get());
            if( xMethod.is() )
            {
                Sequence<Any> args( 1 );
                args.getArray()[0] = aValue;
                xMethod->invoke( obj, args );
            }
            else
            {
                // throw IllegalArgumentException();
            }
        }
        break;
    }
}

Any IntrospectionAccessStatic_Impl::getPropertyValue( const Any& obj, const OUString& aPropertyName ) const
{
    sal_Int32 i = getPropertyIndex( aPropertyName );
    if( i != -1 )
        return getPropertyValueByIndex( obj, i );

    throw UnknownPropertyException();
}

Any IntrospectionAccessStatic_Impl::getPropertyValueByIndex(const Any& obj, sal_Int32 nSequenceIndex) const
{
    Any aRet;

    // Handelt es sich bei dem uebergebenen Objekt ueberhaupt um was passendes?
    TypeClass eObjType = obj.getValueType().getTypeClass();

    Reference<XInterface> xInterface;
    if( eObjType == TypeClass_INTERFACE )
    {
        xInterface = *(Reference<XInterface>*)obj.getValue();
    }
    else if( nSequenceIndex >= mnPropCount || ( eObjType != TypeClass_STRUCT && eObjType != TypeClass_EXCEPTION ) )
    {
        // throw IllegalArgumentException();
        return aRet;
    }

    const sal_Int16* pMapTypeArray = maMapTypeSeq.getConstArray();
    switch( pMapTypeArray[ nSequenceIndex ] )
    {
        case MAP_PROPERTY_SET:
        {
            // Property besorgen
            const Property& rProp = maAllPropertySeq.getConstArray()[ nSequenceIndex ];

            // Haben wir ein FastPropertySet und ein gueltiges Handle?
            // ACHTUNG: An dieser Stelle wird ausgenutzt, dass das PropertySet
            // zu Beginn des Introspection-Vorgangs abgefragt wird.
            sal_Int32 nOrgHandle;
            if( mbFastPropSet && ( nOrgHandle = mpOrgPropertyHandleArray[ nSequenceIndex ] ) != -1 )
            {
                // PropertySet-Interface holen
                Reference<XFastPropertySet> xFastPropSet =
                    Reference<XFastPropertySet>::query( xInterface );
                if( xFastPropSet.is() )
                {
                    aRet = xFastPropSet->getFastPropertyValue( nOrgHandle);
                }
                else
                {
                    // throw UnknownPropertyException
                    return aRet;
                }
            }
            // sonst eben das normale nehmen
            else
            {
                // PropertySet-Interface holen
                Reference<XPropertySet> xPropSet =
                    Reference<XPropertySet>::query( xInterface );
                if( xPropSet.is() )
                {
                    aRet = xPropSet->getPropertyValue( rProp.Name );
                }
                else
                {
                    // throw UnknownPropertyException
                    return aRet;
                }
            }
        }
        break;

        case MAP_FIELD:
        {
            Reference<XIdlField> xField = (XIdlField*)(aInterfaceSeq1.getConstArray()[ nSequenceIndex ].get());
            if( xField.is() )
            {
                aRet = xField->get( obj );
                // IllegalArgumentException
                // NullPointerException
            }
            else
            {
                // throw IllegalArgumentException();
                return aRet;
            }
        }
        break;

        case MAP_GETSET:
        {
            // get-Methode holen
            Reference<XIdlMethod> xMethod = (XIdlMethod*)(aInterfaceSeq1.getConstArray()[ nSequenceIndex ].get());
            if( xMethod.is() )
            {
                Sequence<Any> args;
                aRet = xMethod->invoke( obj, args );
            }
            else
            {
                // throw IllegalArgumentException();
                return aRet;
            }
        }
        break;

        case MAP_SETONLY:
            // get-Methode gibt es nicht
            // throw WriteOnlyPropertyException();
            return aRet;
    }
    return aRet;
}


// Hilfs-Methoden zur Groessen-Anpassung der Sequences
void IntrospectionAccessStatic_Impl::checkPropertyArraysSize
(
    Property*& rpAllPropArray,
    sal_Int16*& rpMapTypeArray,
    sal_Int32*& rpPropertyConceptArray,
    sal_Int32 iNextIndex
)
{
    sal_Int32 nLen = maAllPropertySeq.getLength();
    if( iNextIndex >= nLen )
    {
        maAllPropertySeq.realloc( nLen + ARRAY_SIZE_STEP );
        rpAllPropArray = maAllPropertySeq.getArray();

        maMapTypeSeq.realloc( nLen + ARRAY_SIZE_STEP );
        rpMapTypeArray = maMapTypeSeq.getArray();

        maPropertyConceptSeq.realloc( nLen + ARRAY_SIZE_STEP );
        rpPropertyConceptArray = maPropertyConceptSeq.getArray();
    }
}

void IntrospectionAccessStatic_Impl::checkInterfaceArraySize( Sequence< Reference<XInterface> >& rSeq,
    Reference<XInterface>*& rpInterfaceArray, sal_Int32 iNextIndex )
{
    sal_Int32 nLen = rSeq.getLength();
    if( iNextIndex >= nLen )
    {
        // Neue Groesse mit ARRAY_SIZE_STEP abgleichen
        sal_Int32 nMissingSize = iNextIndex - nLen + 1;
        sal_Int32 nSteps = nMissingSize / ARRAY_SIZE_STEP + 1;
        sal_Int32 nNewSize = nLen + nSteps * ARRAY_SIZE_STEP;

        rSeq.realloc( nNewSize );
        rpInterfaceArray = rSeq.getArray();
    }
}



//*** ImplIntrospectionAccess ***


// Neue Impl-Klasse im Rahmen der Introspection-Umstellung auf Instanz-gebundene
// Introspection mit Property-Zugriff ueber XPropertySet. Die alte Klasse
// ImplIntrospectionAccess lebt als IntrospectionAccessStatic_Impl
class ImplIntrospectionAccess : public IntrospectionAccessHelper
{
    friend class Implementation;

    // Untersuchtes Objekt
    Any maInspectedObject;

    // Als Interface
    Reference<XInterface> mxIface;

    // Statische Daten der Introspection
    rtl::Reference< IntrospectionAccessStatic_Impl > mpStaticImpl;

    // Adapter-Implementation
    WeakReference< XInterface > maAdapter;

    // Letzte Sequence, die bei getProperties geliefert wurde (Optimierung)
    Sequence<Property> maLastPropertySeq;
    sal_Int32 mnLastPropertyConcept;

    // Letzte Sequence, die bei getMethods geliefert wurde (Optimierung)
    Sequence<Reference<XIdlMethod> > maLastMethodSeq;
    sal_Int32 mnLastMethodConcept;

public:
    ImplIntrospectionAccess( const Any& obj, rtl::Reference< IntrospectionAccessStatic_Impl > const & pStaticImpl_ );
    ~ImplIntrospectionAccess();

    // Methoden von XIntrospectionAccess
    virtual sal_Int32 SAL_CALL getSuppliedMethodConcepts(void)
        throw( RuntimeException, std::exception );
    virtual sal_Int32 SAL_CALL getSuppliedPropertyConcepts(void)
        throw( RuntimeException, std::exception );
    virtual Property SAL_CALL getProperty(const OUString& Name, sal_Int32 PropertyConcepts)
        throw( NoSuchElementException, RuntimeException, std::exception );
    virtual sal_Bool SAL_CALL hasProperty(const OUString& Name, sal_Int32 PropertyConcepts)
        throw( RuntimeException, std::exception );
    virtual Sequence< Property > SAL_CALL getProperties(sal_Int32 PropertyConcepts)
          throw( RuntimeException, std::exception );
    virtual Reference<XIdlMethod> SAL_CALL getMethod(const OUString& Name, sal_Int32 MethodConcepts)
          throw( NoSuchMethodException, RuntimeException, std::exception );
    virtual sal_Bool SAL_CALL hasMethod(const OUString& Name, sal_Int32 MethodConcepts)
          throw( RuntimeException, std::exception );
    virtual Sequence< Reference<XIdlMethod> > SAL_CALL getMethods(sal_Int32 MethodConcepts)
          throw( RuntimeException, std::exception );
    virtual Sequence< Type > SAL_CALL getSupportedListeners(void)
          throw( RuntimeException, std::exception );
    using OWeakObject::queryAdapter;
    virtual Reference<XInterface> SAL_CALL queryAdapter( const Type& rType )
          throw( IllegalTypeException, RuntimeException, std::exception );

    // Methoden von XMaterialHolder
    virtual Any SAL_CALL getMaterial(void) throw(RuntimeException, std::exception);

    // Methoden von XExactName
    virtual OUString SAL_CALL getExactName( const OUString& rApproximateName ) throw( RuntimeException, std::exception );
};

ImplIntrospectionAccess::ImplIntrospectionAccess
    ( const Any& obj, rtl::Reference< IntrospectionAccessStatic_Impl > const & pStaticImpl_ )
        : maInspectedObject( obj ), mpStaticImpl( pStaticImpl_ ), maAdapter()
{
    // Objekt als Interface merken, wenn moeglich
    TypeClass eType = maInspectedObject.getValueType().getTypeClass();
    if( eType == TypeClass_INTERFACE )
        mxIface = *(Reference<XInterface>*)maInspectedObject.getValue();

    mnLastPropertyConcept = -1;
    mnLastMethodConcept = -1;
}

ImplIntrospectionAccess::~ImplIntrospectionAccess()
{
}



//*** ImplIntrospectionAdapter ***


// Neue Impl-Klasse im Rahmen der Introspection-Umstellung auf Instanz-gebundene
// Introspection mit Property-Zugriff ueber XPropertySet. Die alte Klasse
// ImplIntrospectionAccess lebt als IntrospectionAccessStatic_Impl
class ImplIntrospectionAdapter :
    public XPropertySet, public XFastPropertySet, public XPropertySetInfo,
    public XNameContainer, public XIndexContainer,
    public XEnumerationAccess, public  XIdlArray,
    public OWeakObject
{
    // Parent-Objekt
    ::rtl::Reference< ImplIntrospectionAccess > mpAccess;

    // Untersuchtes Objekt
    const Any& mrInspectedObject;

    // Statische Daten der Introspection
    rtl::Reference< IntrospectionAccessStatic_Impl > mpStaticImpl;

    // Objekt als Interface
    Reference<XInterface> mxIface;

    // Original-Interfaces des Objekts
    Reference<XElementAccess>        mxObjElementAccess;
    Reference<XNameContainer>        mxObjNameContainer;
    Reference<XNameAccess>            mxObjNameAccess;
    Reference<XIndexAccess>            mxObjIndexAccess;
    Reference<XIndexContainer>        mxObjIndexContainer;
    Reference<XEnumerationAccess>    mxObjEnumerationAccess;
    Reference<XIdlArray>            mxObjIdlArray;

public:
    ImplIntrospectionAdapter( ImplIntrospectionAccess* pAccess_,
        const Any& obj,
        rtl::Reference< IntrospectionAccessStatic_Impl > const & pStaticImpl_ );

    // Methoden von XInterface
    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException, std::exception );
    virtual void        SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    virtual void        SAL_CALL release() throw() { OWeakObject::release(); }

    // Methoden von XPropertySet
    virtual Reference<XPropertySetInfo> SAL_CALL getPropertySetInfo() throw( RuntimeException, std::exception );
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception );
    virtual Any SAL_CALL getPropertyValue(const OUString& aPropertyName)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception );
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception );
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception );
    virtual void SAL_CALL addVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception );
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception );

    // Methoden von XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception );
    virtual Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception );

    // Methoden von XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(void) throw( RuntimeException, std::exception );
    virtual Property SAL_CALL getPropertyByName(const OUString& Name) throw( RuntimeException, std::exception );
    virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& Name) throw( RuntimeException, std::exception );

    // Methoden von XElementAccess
    virtual Type SAL_CALL getElementType(void) throw( RuntimeException, std::exception );
    virtual sal_Bool SAL_CALL hasElements(void) throw( RuntimeException, std::exception );

    // Methoden von XNameAccess
    virtual Any SAL_CALL getByName(const OUString& Name)
        throw( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception );
    virtual Sequence< OUString > SAL_CALL getElementNames(void) throw( RuntimeException, std::exception );
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( RuntimeException, std::exception );

    // Methoden von XNameContainer
    virtual void SAL_CALL insertByName(const OUString& Name, const Any& Element)
        throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception );
    virtual void SAL_CALL replaceByName(const OUString& Name, const Any& Element)
        throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception );
    virtual void SAL_CALL removeByName(const OUString& Name)
        throw( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception );

    // Methoden von XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( RuntimeException, std::exception );
    virtual Any SAL_CALL getByIndex(sal_Int32 Index)
        throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception );

    // Methoden von XIndexContainer
    virtual void SAL_CALL insertByIndex(sal_Int32 Index, const Any& Element)
        throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception );
    virtual void SAL_CALL replaceByIndex(sal_Int32 Index, const Any& Element)
        throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception );
    virtual void SAL_CALL removeByIndex(sal_Int32 Index)
        throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception );

    // Methoden von XEnumerationAccess
    virtual Reference<XEnumeration> SAL_CALL createEnumeration(void) throw( RuntimeException, std::exception );

    // Methoden von XIdlArray
    virtual void SAL_CALL realloc(Any& array, sal_Int32 length)
        throw( IllegalArgumentException, RuntimeException, std::exception );
    virtual sal_Int32 SAL_CALL getLen(const Any& array) throw( IllegalArgumentException, RuntimeException, std::exception );
    virtual Any SAL_CALL get(const Any& array, sal_Int32 index)
        throw( IllegalArgumentException, ArrayIndexOutOfBoundsException, RuntimeException, std::exception );
    virtual void SAL_CALL set(Any& array, sal_Int32 index, const Any& value)
        throw( IllegalArgumentException, ArrayIndexOutOfBoundsException, RuntimeException, std::exception );
};

ImplIntrospectionAdapter::ImplIntrospectionAdapter( ImplIntrospectionAccess* pAccess_,
    const Any& obj,
    rtl::Reference< IntrospectionAccessStatic_Impl > const & pStaticImpl_ )
        : mpAccess( pAccess_), mrInspectedObject( obj ), mpStaticImpl( pStaticImpl_ )
{
    // Objekt als Interfaceholen
    TypeClass eType = mrInspectedObject.getValueType().getTypeClass();
    if( eType == TypeClass_INTERFACE )
    {
        mxIface = *( Reference< XInterface >*)mrInspectedObject.getValue();

        mxObjElementAccess = Reference<XElementAccess>::query( mxIface );
        mxObjNameAccess = Reference<XNameAccess>::query( mxIface );
        mxObjNameContainer = Reference<XNameContainer>::query( mxIface );
        mxObjIndexAccess = Reference<XIndexAccess>::query( mxIface );
        mxObjIndexContainer = Reference<XIndexContainer>::query( mxIface );
        mxObjEnumerationAccess = Reference<XEnumerationAccess>::query( mxIface );
        mxObjIdlArray = Reference<XIdlArray>::query( mxIface );
    }
}

// Methoden von XInterface
Any SAL_CALL ImplIntrospectionAdapter::queryInterface( const Type& rType )
    throw( RuntimeException, std::exception )
{
    Any aRet( ::cppu::queryInterface(
        rType,
        static_cast< XPropertySet * >( this ),
        static_cast< XFastPropertySet * >( this ),
        static_cast< XPropertySetInfo * >( this ) ) );
    if( !aRet.hasValue() )
        aRet = OWeakObject::queryInterface( rType );

    if( !aRet.hasValue() )
    {
        // Wrapper fuer die Objekt-Interfaces
        if(   ( mxObjElementAccess.is() && (aRet = ::cppu::queryInterface
                    ( rType, static_cast< XElementAccess* >( static_cast< XNameAccess* >( this ) ) ) ).hasValue() )
            || ( mxObjNameAccess.is() && (aRet = ::cppu::queryInterface( rType, static_cast< XNameAccess* >( this ) ) ).hasValue() )
            || ( mxObjNameContainer.is() && (aRet = ::cppu::queryInterface( rType, static_cast< XNameContainer* >( this ) ) ).hasValue() )
            || ( mxObjIndexAccess.is() && (aRet = ::cppu::queryInterface( rType, static_cast< XIndexAccess* >( this ) ) ).hasValue() )
            || ( mxObjIndexContainer.is() && (aRet = ::cppu::queryInterface( rType, static_cast< XIndexContainer* >( this ) ) ).hasValue() )
            || ( mxObjEnumerationAccess    .is() && (aRet = ::cppu::queryInterface( rType, static_cast< XEnumerationAccess* >( this ) ) ).hasValue() )
            || ( mxObjIdlArray.is() && (aRet = ::cppu::queryInterface( rType, static_cast< XIdlArray* >( this ) ) ).hasValue() )
          )
        {
        }
    }
    return aRet;
}



//*** Implementation von ImplIntrospectionAdapter ***


// Methoden von XPropertySet
Reference<XPropertySetInfo> ImplIntrospectionAdapter::getPropertySetInfo(void)
    throw( RuntimeException, std::exception )
{
    return (XPropertySetInfo *)this;
}

void ImplIntrospectionAdapter::setPropertyValue(const OUString& aPropertyName, const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception )
{
    mpStaticImpl->setPropertyValue( mrInspectedObject, aPropertyName, aValue );
}

Any ImplIntrospectionAdapter::getPropertyValue(const OUString& aPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
    return mpStaticImpl->getPropertyValue( mrInspectedObject, aPropertyName );
}

void ImplIntrospectionAdapter::addPropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
    if( mxIface.is() )
    {
        Reference<XPropertySet> xPropSet =
            Reference<XPropertySet>::query( mxIface );
        //Reference<XPropertySet> xPropSet( mxIface, USR_QUERY );
        if( xPropSet.is() )
            xPropSet->addPropertyChangeListener(aPropertyName, aListener);
    }
}

void ImplIntrospectionAdapter::removePropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
    if( mxIface.is() )
    {
        Reference<XPropertySet> xPropSet =
            Reference<XPropertySet>::query( mxIface );
        //Reference<XPropertySet> xPropSet( mxIface, USR_QUERY );
        if( xPropSet.is() )
            xPropSet->removePropertyChangeListener(aPropertyName, aListener);
    }
}

void ImplIntrospectionAdapter::addVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
    if( mxIface.is() )
    {
        Reference<XPropertySet> xPropSet =
            Reference<XPropertySet>::query( mxIface );
        //Reference<XPropertySet> xPropSet( mxIface, USR_QUERY );
        if( xPropSet.is() )
            xPropSet->addVetoableChangeListener(aPropertyName, aListener);
    }
}

void ImplIntrospectionAdapter::removeVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
    if( mxIface.is() )
    {
        Reference<XPropertySet> xPropSet =
            Reference<XPropertySet>::query( mxIface );
        if( xPropSet.is() )
            xPropSet->removeVetoableChangeListener(aPropertyName, aListener);
    }
}


// Methoden von XFastPropertySet
void ImplIntrospectionAdapter::setFastPropertyValue(sal_Int32, const Any&)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception )
{
}

Any ImplIntrospectionAdapter::getFastPropertyValue(sal_Int32)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
    return Any();
}

// Methoden von XPropertySetInfo
Sequence< Property > ImplIntrospectionAdapter::getProperties(void) throw( RuntimeException, std::exception )
{
    return mpStaticImpl->getProperties();
}

Property ImplIntrospectionAdapter::getPropertyByName(const OUString& Name)
    throw( RuntimeException, std::exception )
{
    return mpAccess->getProperty( Name, PropertyConcept::ALL );
}

sal_Bool ImplIntrospectionAdapter::hasPropertyByName(const OUString& Name)
    throw( RuntimeException, std::exception )
{
    return mpAccess->hasProperty( Name, PropertyConcept::ALL );
}

// Methoden von XElementAccess
Type ImplIntrospectionAdapter::getElementType(void) throw( RuntimeException, std::exception )
{
    return mxObjElementAccess->getElementType();
}

sal_Bool ImplIntrospectionAdapter::hasElements(void) throw( RuntimeException, std::exception )
{
    return mxObjElementAccess->hasElements();
}

// Methoden von XNameAccess
Any ImplIntrospectionAdapter::getByName(const OUString& Name)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception )
{
    return mxObjNameAccess->getByName( Name );
}

Sequence< OUString > ImplIntrospectionAdapter::getElementNames(void)
    throw( RuntimeException, std::exception )
{
    return mxObjNameAccess->getElementNames();
}

sal_Bool ImplIntrospectionAdapter::hasByName(const OUString& Name)
    throw( RuntimeException, std::exception )
{
    return mxObjNameAccess->hasByName( Name );
}

// Methoden von XNameContainer
void ImplIntrospectionAdapter::insertByName(const OUString& Name, const Any& Element)
    throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception )
{
    mxObjNameContainer->insertByName( Name, Element );
}

void ImplIntrospectionAdapter::replaceByName(const OUString& Name, const Any& Element)
    throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception )
{
    mxObjNameContainer->replaceByName( Name, Element );
}

void ImplIntrospectionAdapter::removeByName(const OUString& Name)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception )
{
    mxObjNameContainer->removeByName( Name );
}

// Methoden von XIndexAccess
// Schon in XNameAccess: virtual Reference<XIdlClass> getElementType(void) const
sal_Int32 ImplIntrospectionAdapter::getCount(void) throw( RuntimeException, std::exception )
{
    return mxObjIndexAccess->getCount();
}

Any ImplIntrospectionAdapter::getByIndex(sal_Int32 Index)
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
{
    return mxObjIndexAccess->getByIndex( Index );
}

// Methoden von XIndexContainer
void ImplIntrospectionAdapter::insertByIndex(sal_Int32 Index, const Any& Element)
    throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
{
    mxObjIndexContainer->insertByIndex( Index, Element );
}

void ImplIntrospectionAdapter::replaceByIndex(sal_Int32 Index, const Any& Element)
    throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
{
    mxObjIndexContainer->replaceByIndex( Index, Element );
}

void ImplIntrospectionAdapter::removeByIndex(sal_Int32 Index)
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
{
    mxObjIndexContainer->removeByIndex( Index );
}

// Methoden von XEnumerationAccess
// Schon in XNameAccess: virtual Reference<XIdlClass> getElementType(void) const;
Reference<XEnumeration> ImplIntrospectionAdapter::createEnumeration(void) throw( RuntimeException, std::exception )
{
    return mxObjEnumerationAccess->createEnumeration();
}

// Methoden von XIdlArray
void ImplIntrospectionAdapter::realloc(Any& array, sal_Int32 length)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    mxObjIdlArray->realloc( array, length );
}

sal_Int32 ImplIntrospectionAdapter::getLen(const Any& array)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    return mxObjIdlArray->getLen( array );
}

Any ImplIntrospectionAdapter::get(const Any& array, sal_Int32 index)
    throw( IllegalArgumentException, ArrayIndexOutOfBoundsException, RuntimeException, std::exception )
{
    return mxObjIdlArray->get( array, index );
}

void ImplIntrospectionAdapter::set(Any& array, sal_Int32 index, const Any& value)
    throw( IllegalArgumentException, ArrayIndexOutOfBoundsException, RuntimeException, std::exception )
{
    mxObjIdlArray->set( array, index, value );
}



//*** Implementation von ImplIntrospectionAccess ***


// Methoden von XIntrospectionAccess
sal_Int32 ImplIntrospectionAccess::getSuppliedMethodConcepts(void)
    throw( RuntimeException, std::exception )
{
    return    MethodConcept::DANGEROUS |
            PROPERTY |
            LISTENER |
            ENUMERATION |
            NAMECONTAINER |
            INDEXCONTAINER;
}

sal_Int32 ImplIntrospectionAccess::getSuppliedPropertyConcepts(void)
    throw( RuntimeException, std::exception )
{
    return    PropertyConcept::DANGEROUS |
            PROPERTYSET |
            ATTRIBUTES |
            METHODS;
}

Property ImplIntrospectionAccess::getProperty(const OUString& Name, sal_Int32 PropertyConcepts)
    throw( NoSuchElementException, RuntimeException, std::exception )
{
    Property aRet;
    sal_Int32 i = mpStaticImpl->getPropertyIndex( Name );
    sal_Bool bFound = sal_False;
    if( i != -1 )
    {
        sal_Int32 nConcept = mpStaticImpl->getPropertyConcepts().getConstArray()[ i ];
        if( (PropertyConcepts & nConcept) != 0 )
        {
            const Property* pProps = mpStaticImpl->getProperties().getConstArray();
            aRet = pProps[ i ];
            bFound = sal_True;
        }
    }
    if( !bFound )
        throw NoSuchElementException() ;
    return aRet;
}

sal_Bool ImplIntrospectionAccess::hasProperty(const OUString& Name, sal_Int32 PropertyConcepts)
    throw( RuntimeException, std::exception )
{
    sal_Int32 i = mpStaticImpl->getPropertyIndex( Name );
    sal_Bool bRet = sal_False;
    if( i != -1 )
    {
        sal_Int32 nConcept = mpStaticImpl->getPropertyConcepts().getConstArray()[ i ];
        if( (PropertyConcepts & nConcept) != 0 )
            bRet = sal_True;
    }
    return bRet;
}

Sequence< Property > ImplIntrospectionAccess::getProperties(sal_Int32 PropertyConcepts)
    throw( RuntimeException, std::exception )
{
    // Wenn alle unterstuetzten Konzepte gefordert werden, Sequence einfach durchreichen
    sal_Int32 nAllSupportedMask =    PROPERTYSET |
                                    ATTRIBUTES |
                                    METHODS;
    if( ( PropertyConcepts & nAllSupportedMask ) == nAllSupportedMask )
    {
        return mpStaticImpl->getProperties();
    }

    // Gleiche Sequence wie beim vorigen mal?
    if( mnLastPropertyConcept == PropertyConcepts )
    {
        return maLastPropertySeq;
    }

    // Anzahl der zu liefernden Properties
    sal_Int32 nCount = 0;

    // Es gibt zur Zeit keine DANGEROUS-Properties
    // if( PropertyConcepts & DANGEROUS )
    //    nCount += mpStaticImpl->mnDangerousPropCount;
    if( PropertyConcepts & PROPERTYSET )
        nCount += mpStaticImpl->mnPropertySetPropCount;
    if( PropertyConcepts & ATTRIBUTES )
        nCount += mpStaticImpl->mnAttributePropCount;
    if( PropertyConcepts & METHODS )
        nCount += mpStaticImpl->mnMethodPropCount;

    // Sequence entsprechend der geforderten Anzahl reallocieren
    ImplIntrospectionAccess* pThis = (ImplIntrospectionAccess*)this;    // const umgehen
    pThis->maLastPropertySeq.realloc( nCount );
    Property* pDestProps = pThis->maLastPropertySeq.getArray();

    // Alle Properties durchgehen und entsprechend der Concepte uebernehmen
    Sequence<Property> aPropSeq = mpStaticImpl->getProperties();
    const Property* pSourceProps = aPropSeq.getConstArray();
    const sal_Int32* pConcepts = mpStaticImpl->getPropertyConcepts().getConstArray();
    sal_Int32 nLen = aPropSeq.getLength();

    sal_Int32 iDest = 0;
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        sal_Int32 nConcept = pConcepts[ i ];
        if( nConcept & PropertyConcepts )
            pDestProps[ iDest++ ] = pSourceProps[ i ];
    }

    // PropertyConcept merken, dies entspricht maLastPropertySeq
    pThis->mnLastPropertyConcept = PropertyConcepts;

    // Zusammengebastelte Sequence liefern
    return maLastPropertySeq;
}

Reference<XIdlMethod> ImplIntrospectionAccess::getMethod(const OUString& Name, sal_Int32 MethodConcepts)
    throw( NoSuchMethodException, RuntimeException, std::exception )
{
    Reference<XIdlMethod> xRet;
    sal_Int32 i = mpStaticImpl->getMethodIndex( Name );
    if( i != -1 )
    {

        sal_Int32 nConcept = mpStaticImpl->getMethodConcepts().getConstArray()[ i ];
        if( (MethodConcepts & nConcept) != 0 )
        {
            const Reference<XIdlMethod>* pMethods = mpStaticImpl->getMethods().getConstArray();
            xRet = pMethods[i];
        }
    }
    if( !xRet.is() )
        throw NoSuchMethodException();
    return xRet;
}

sal_Bool ImplIntrospectionAccess::hasMethod(const OUString& Name, sal_Int32 MethodConcepts)
    throw( RuntimeException, std::exception )
{
    sal_Int32 i = mpStaticImpl->getMethodIndex( Name );
    sal_Bool bRet = sal_False;
    if( i != -1 )
    {
        sal_Int32 nConcept = mpStaticImpl->getMethodConcepts().getConstArray()[ i ];
        if( (MethodConcepts & nConcept) != 0 )
            bRet = sal_True;
    }
    return bRet;
}

Sequence< Reference<XIdlMethod> > ImplIntrospectionAccess::getMethods(sal_Int32 MethodConcepts)
    throw( RuntimeException, std::exception )
{
    ImplIntrospectionAccess* pThis = (ImplIntrospectionAccess*)this;    // const umgehen

    // Wenn alle unterstuetzten Konzepte gefordert werden, Sequence einfach durchreichen
    sal_Int32 nAllSupportedMask =     MethodConcept::DANGEROUS |
                                    PROPERTY |
                                    LISTENER |
                                    ENUMERATION |
                                    NAMECONTAINER |
                                    INDEXCONTAINER |
                                    MethodConcept_NORMAL_IMPL;
    if( ( MethodConcepts & nAllSupportedMask ) == nAllSupportedMask )
    {
        return mpStaticImpl->getMethods();
    }

    // Gleiche Sequence wie beim vorigen mal?
    if( mnLastMethodConcept == MethodConcepts )
    {
        return maLastMethodSeq;
    }

    // Methoden-Sequences besorgen
    Sequence< Reference<XIdlMethod> > aMethodSeq = mpStaticImpl->getMethods();
    const Reference<XIdlMethod>* pSourceMethods = aMethodSeq.getConstArray();
    const sal_Int32* pConcepts = mpStaticImpl->getMethodConcepts().getConstArray();
    sal_Int32 nLen = aMethodSeq.getLength();

    // Sequence entsprechend der geforderten Anzahl reallocieren
    // Anders als bei den Properties kann die Anzahl nicht durch
    // Zaehler in inspect() vorher ermittelt werden, da Methoden
    // mehreren Konzepten angehoeren koennen
    pThis->maLastMethodSeq.realloc( nLen );
    Reference<XIdlMethod>* pDestMethods = pThis->maLastMethodSeq.getArray();

    // Alle Methods durchgehen und entsprechend der Concepte uebernehmen
    sal_Int32 iDest = 0;
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        sal_Int32 nConcept = pConcepts[ i ];
        if( nConcept & MethodConcepts )
            pDestMethods[ iDest++ ] = pSourceMethods[ i ];

    #if OSL_DEBUG_LEVEL > 0
        static bool debug = false;
        if ( debug )
        {
            // Methode mit Concepts ausgeben
            const Reference< XIdlMethod >& rxMethod = pSourceMethods[ i ];
            OString aNameStr = OUStringToOString( rxMethod->getName(), osl_getThreadTextEncoding() );
            OString ConceptStr;
            if( nConcept & MethodConcept::DANGEROUS )
                ConceptStr += "DANGEROUS |";
            if( nConcept & MethodConcept::PROPERTY )
                ConceptStr += "PROPERTY |";
            if( nConcept & MethodConcept::LISTENER )
                ConceptStr += "LISTENER |";
            if( nConcept & MethodConcept::ENUMERATION )
                ConceptStr += "ENUMERATION |";
            if( nConcept & MethodConcept::NAMECONTAINER )
                ConceptStr += "NAMECONTAINER |";
            if( nConcept & MethodConcept::INDEXCONTAINER )
                ConceptStr += "INDEXCONTAINER |";
            OSL_TRACE( "Method %ld: %s, Concepts = %s", i, aNameStr.getStr(), ConceptStr.getStr() );
        }
    #endif
    }

    // Auf die richtige Laenge bringen
    pThis->maLastMethodSeq.realloc( iDest );

    // MethodConcept merken, dies entspricht maLastMethodSeq
    pThis->mnLastMethodConcept = MethodConcepts;

    // Zusammengebastelte Sequence liefern
    return maLastMethodSeq;
}

Sequence< Type > ImplIntrospectionAccess::getSupportedListeners(void)
    throw( RuntimeException, std::exception )
{
    return mpStaticImpl->getSupportedListeners();
}

Reference<XInterface> SAL_CALL ImplIntrospectionAccess::queryAdapter( const Type& rType )
    throw( IllegalTypeException, RuntimeException, std::exception )
{
    // Gibt es schon einen Adapter?
    Reference< XInterface > xAdapter( maAdapter );
    if( !xAdapter.is() )
    {
        xAdapter = *( new ImplIntrospectionAdapter( this, maInspectedObject, mpStaticImpl ) );
        maAdapter = xAdapter;
    }

    Reference<XInterface> xRet;
    xAdapter->queryInterface( rType ) >>= xRet;
    return xRet;
}

// Methoden von XMaterialHolder
Any ImplIntrospectionAccess::getMaterial(void) throw(RuntimeException, std::exception)
{
    return maInspectedObject;
}

// Hilfs-Funktion zur LowerCase-Wandlung eines OUString
OUString toLower( OUString aUStr )
{
    // Tabelle fuer XExactName pflegen
    OUString aOWStr( aUStr.getStr() );
    OUString aOWLowerStr = aOWStr.toAsciiLowerCase();
    OUString aLowerUStr( aOWLowerStr.getStr() );
    return aLowerUStr;
}

// Methoden von XExactName
OUString ImplIntrospectionAccess::getExactName( const OUString& rApproximateName ) throw( RuntimeException, std::exception )
{
    OUString aRetStr;
    LowerToExactNameMap::iterator aIt =
        mpStaticImpl->maLowerToExactNameMap.find( toLower( rApproximateName ) );
    if( !( aIt == mpStaticImpl->maLowerToExactNameMap.end() ) )
        aRetStr = (*aIt).second;
    return aRetStr;
}

struct ClassKey {
    ClassKey(
        css::uno::Reference<css::beans::XPropertySetInfo> const & theProperties,
        css::uno::Reference<css::reflection::XIdlClass> const &
            theImplementation,
        css::uno::Sequence< css::uno::Reference<css::reflection::XIdlClass> >
            const & theClasses):
        properties(theProperties), implementation(theImplementation),
        classes(theClasses)
    {}

    css::uno::Reference<css::beans::XPropertySetInfo> properties;
    css::uno::Reference<css::reflection::XIdlClass> implementation;
    css::uno::Sequence< css::uno::Reference<css::reflection::XIdlClass> >
        classes;
};

struct ClassKeyLess {
    bool operator ()(ClassKey const & key1, ClassKey const & key2) const {
        if (key1.properties.get() < key2.properties.get()) {
            return true;
        }
        if (key1.properties.get() > key2.properties.get()) {
            return false;
        }
        if (key1.implementation.get() < key2.implementation.get()) {
            return true;
        }
        if (key1.implementation.get() > key2.implementation.get()) {
            return false;
        }
        if (key1.classes.getLength() < key2.classes.getLength()) {
            return true;
        }
        if (key1.classes.getLength() > key2.classes.getLength()) {
            return false;
        }
        for (sal_Int32 i = 0; i != key1.classes.getLength(); ++i) {
            if (key1.classes[i].get() < key2.classes[i].get()) {
                return true;
            }
            if (key1.classes[i].get() > key2.classes[i].get()) {
                return false;
            }
        }
        return false;
    }
};

struct TypeKey {
    TypeKey(
        css::uno::Reference<css::beans::XPropertySetInfo> const & theProperties,
        css::uno::Sequence<css::uno::Type> const & theTypes):
        properties(theProperties)
    {
        //TODO: Could even sort the types lexicographically first, to increase
        // the chance of matches between different implementations' getTypes(),
        // but the old scheme of using getImplementationId() would have missed
        // those matches, too:
        OUStringBuffer b;
        for (sal_Int32 i = 0; i != theTypes.getLength(); ++i) {
            b.append(theTypes[i].getTypeName());
            b.append('*'); // arbitrary delimiter not used by type grammar
        }
        types = b.makeStringAndClear();
    }

    css::uno::Reference<css::beans::XPropertySetInfo> properties;
    OUString types;
};

struct TypeKeyLess {
    bool operator ()(TypeKey const & key1, TypeKey const & key2) const {
        if (key1.properties.get() < key2.properties.get()) {
            return true;
        }
        if (key1.properties.get() > key2.properties.get()) {
            return false;
        }
        return key1.types < key2.types;
    }
};

template<typename Key, typename Less> class Cache {
public:
    rtl::Reference<IntrospectionAccessStatic_Impl> find(Key const & key) const {
        typename Map::const_iterator i(map_.find(key));
        if (i == map_.end()) {
            return rtl::Reference<IntrospectionAccessStatic_Impl>();
        } else {
            if (i->second.hits < std::numeric_limits<unsigned>::max()) {
                ++i->second.hits;
            }
            assert(i->second.access.is());
            return i->second.access;
        }
    }

    void insert(
        Key const & key,
        rtl::Reference<IntrospectionAccessStatic_Impl> const & access)
    {
        assert(access.is());
        typename Map::size_type const MAX = 100;
        assert(map_.size() <= MAX);
        if (map_.size() == MAX) {
            typename Map::iterator del(map_.begin());
            for (typename Map::iterator i(map_.begin()); i != map_.end(); ++i) {
                if (i->second.hits < del->second.hits) {
                    del = i;
                }
            }
            map_.erase(del);
        }
        bool ins = map_.insert(typename Map::value_type(key, Data(access)))
            .second;
        assert(ins); (void)ins;
    }

    void clear() { map_.clear(); }

private:
    struct Data {
        explicit Data(
            rtl::Reference<IntrospectionAccessStatic_Impl> const & theAccess):
            access(theAccess), hits(1)
        {}

        rtl::Reference<IntrospectionAccessStatic_Impl> access;
        mutable unsigned hits;
    };

    typedef std::map<Key, Data, Less> Map;

    Map map_;
};

typedef
    cppu::WeakComponentImplHelper2<
        css::lang::XServiceInfo, css::beans::XIntrospection>
    Implementation_Base;

class Implementation: private osl::Mutex, public Implementation_Base {
public:
    explicit Implementation(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        Implementation_Base(*static_cast<Mutex *>(this)),
        reflection_(css::reflection::theCoreReflection::get(context))
    {}

private:
    virtual void SAL_CALL disposing() SAL_OVERRIDE {
        reflection_.clear();
        classCache_.clear();
        typeCache_.clear();
    }

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.comp.stoc.Introspection"); }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence<OUString> SAL_CALL
    getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        Sequence<OUString> s(1);
        s[0] = "com.sun.star.beans.Introspection";
        return s;
    }

    virtual css::uno::Reference<css::beans::XIntrospectionAccess> SAL_CALL
    inspect(css::uno::Any const & aObject)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Reference<css::reflection::XIdlReflection> reflection_;
    Cache<ClassKey, ClassKeyLess> classCache_;
    Cache<TypeKey, TypeKeyLess> typeCache_;
};

css::uno::Reference<css::beans::XIntrospectionAccess> Implementation::inspect(
    css::uno::Any const & aObject)
    throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(this);
    if (rBHelper.bDisposed) {
        throw css::lang::DisposedException(
            getImplementationName(), static_cast<OWeakObject *>(this));
    }
    css::uno::Any aToInspectObj;
    css::uno::Type t;
    if (aObject >>= t) {
        css::uno::Reference<css::reflection::XIdlClass> c(
            reflection_->forName(t.getTypeName()));
        if (!c.is()) {
            SAL_WARN("stoc", "cannot reflect type " << t.getTypeName());
            return css::uno::Reference<css::beans::XIntrospectionAccess>();
        }
        aToInspectObj <<= c;
    } else {
        aToInspectObj = aObject;
    }

    // Objekt untersuchen
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE && eType != TypeClass_STRUCT  && eType != TypeClass_EXCEPTION )
        return css::uno::Reference<css::beans::XIntrospectionAccess>();

    Reference<XInterface> x;
    if( eType == TypeClass_INTERFACE )
    {
        // Interface aus dem Any besorgen
        x = *(Reference<XInterface>*)aToInspectObj.getValue();
        if( !x.is() )
            return css::uno::Reference<css::beans::XIntrospectionAccess>();
    }

    // Pointer auf ggf. noetige neue IntrospectionAccess-Instanz
    rtl::Reference< IntrospectionAccessStatic_Impl > pAccess;

    // Pruefen: Ist schon ein passendes Access-Objekt gecached?
    Sequence< Reference<XIdlClass> >    SupportedClassSeq;
    Sequence< Type >                    SupportedTypesSeq;
    Reference<XTypeProvider>            xTypeProvider;
    Reference<XIdlClass>                xImplClass;
    Reference<XPropertySetInfo>            xPropSetInfo;
    Reference<XPropertySet>                xPropSet;

    // Look for interfaces XTypeProvider and PropertySet
    if( eType == TypeClass_INTERFACE )
    {
        xTypeProvider = Reference<XTypeProvider>::query( x );
        if( xTypeProvider.is() )
        {
            SupportedTypesSeq = xTypeProvider->getTypes();
            sal_Int32 nTypeCount = SupportedTypesSeq.getLength();
            if( nTypeCount )
            {
                SupportedClassSeq.realloc( nTypeCount );
                Reference<XIdlClass>* pClasses = SupportedClassSeq.getArray();

                const Type* pTypes = SupportedTypesSeq.getConstArray();
                for( sal_Int32 i = 0 ; i < nTypeCount ; i++ )
                {
                    pClasses[i] = reflection_->forName(pTypes[i].getTypeName());
                }
                // TODO: Caching!
            }
        } else {
            SAL_WARN(
                "stoc",
                "object of type \"" << aToInspectObj.getValueTypeName()
                    << "\" lacks XTypeProvider");
            xImplClass = reflection_->forName(aToInspectObj.getValueTypeName());
            SupportedClassSeq.realloc(1);
            SupportedClassSeq[0] = xImplClass;
        }

        xPropSet = Reference<XPropertySet>::query( x );
        // Jetzt versuchen, das PropertySetInfo zu bekommen
        if( xPropSet.is() )
            xPropSetInfo = xPropSet->getPropertySetInfo();
    } else {
        xImplClass = reflection_->forName(aToInspectObj.getValueTypeName());
    }

    if (xTypeProvider.is()) {
        TypeKey key(xPropSetInfo, xTypeProvider->getTypes());
        pAccess = typeCache_.find(key);
        if (pAccess.is()) {
            return new ImplIntrospectionAccess(aToInspectObj, pAccess);
        }
        pAccess = new IntrospectionAccessStatic_Impl(reflection_);
        typeCache_.insert(key, pAccess);
    } else if (xImplClass.is()) {
        ClassKey key(xPropSetInfo, xImplClass, SupportedClassSeq);
        pAccess = classCache_.find(key);
        if (pAccess.is()) {
            return new ImplIntrospectionAccess(aToInspectObj, pAccess);
        }
        pAccess = new IntrospectionAccessStatic_Impl(reflection_);
        classCache_.insert(key, pAccess);
    }

    // Kein Access gecached -> neu anlegen
    Property* pAllPropArray;
    Reference<XInterface>* pInterfaces1;
    Reference<XInterface>* pInterfaces2;
    sal_Int16* pMapTypeArray;
    sal_Int32* pPropertyConceptArray;
    sal_Int32 i;

    if( !pAccess.is() )
        pAccess = new IntrospectionAccessStatic_Impl( reflection_ );

    // Referenzen auf wichtige Daten von pAccess
    sal_Int32& rPropCount = pAccess->mnPropCount;
    IntrospectionNameMap& rPropNameMap = pAccess->maPropertyNameMap;
    IntrospectionNameMap& rMethodNameMap = pAccess->maMethodNameMap;
    LowerToExactNameMap& rLowerToExactNameMap = pAccess->maLowerToExactNameMap;

    // Schon mal Pointer auf das eigene Property-Feld holen
    pAllPropArray = pAccess->maAllPropertySeq.getArray();
    pInterfaces1 = pAccess->aInterfaceSeq1.getArray();
    pInterfaces2 = pAccess->aInterfaceSeq2.getArray();
    pMapTypeArray = pAccess->maMapTypeSeq.getArray();
    pPropertyConceptArray = pAccess->maPropertyConceptSeq.getArray();


    //*** Analyse vornehmen ***

    if( eType == TypeClass_INTERFACE )
    {
        // Zunaechst nach speziellen Interfaces suchen, die fuer
        // die Introspection von besonderer Bedeutung sind.

        // XPropertySet vorhanden?
        if( xPropSet.is() && xPropSetInfo.is() )
        {
            // Gibt es auch ein FastPropertySet?
            Reference<XFastPropertySet> xDummy = Reference<XFastPropertySet>::query( x );
            sal_Bool bFast = pAccess->mbFastPropSet = xDummy.is();

            Sequence<Property> aPropSeq = xPropSetInfo->getProperties();
            const Property* pProps = aPropSeq.getConstArray();
            sal_Int32 nLen = aPropSeq.getLength();

            // Bei FastPropertySet muessen wir uns die Original-Handles merken
            if( bFast )
                pAccess->mpOrgPropertyHandleArray = new sal_Int32[ nLen ];

            for( i = 0 ; i < nLen ; i++ )
            {
                // Property in eigene Liste uebernehmen
                pAccess->checkPropertyArraysSize
                    ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );
                Property& rProp = pAllPropArray[ rPropCount ];
                rProp = pProps[ i ];

                if( bFast )
                    pAccess->mpOrgPropertyHandleArray[ i ] = rProp.Handle;

                // PropCount als Handle fuer das eigene FastPropertySet eintragen
                rProp.Handle = rPropCount;

                // Art der Property merken
                pMapTypeArray[ rPropCount ] = MAP_PROPERTY_SET;
                pPropertyConceptArray[ rPropCount ] = PROPERTYSET;
                pAccess->mnPropertySetPropCount++;

                // Namen in Hashtable eintragen, wenn nicht schon bekannt
                OUString aPropName = rProp.Name;

                // Haben wir den Namen schon?
                IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                if( aIt == rPropNameMap.end() )
                {
                    // Neuer Eintrag in die Hashtable
                    rPropNameMap[ aPropName ] = rPropCount;

                    // Tabelle fuer XExactName pflegen
                    rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;
                }
                else
                {
                    SAL_WARN( "stoc", "Introspection: Property \"" <<
                        aPropName << "\" found more than once in PropertySet" );
                }

                // Count pflegen
                rPropCount++;
            }
        }

        // Indizes in die Export-Tabellen
        sal_Int32 iAllExportedMethod = 0;
        sal_Int32 iAllSupportedListener = 0;

        std::set<OUString> seen;

        // Flag, ob XInterface-Methoden erfasst werden sollen
        // (das darf nur einmal erfolgen, initial zulassen)
        sal_Bool bXInterfaceIsInvalid = sal_False;

        // Flag, ob die XInterface-Methoden schon erfasst wurden. Wenn sal_True,
        // wird bXInterfaceIsInvalid am Ende der Iface-Schleife aktiviert und
        // XInterface-Methoden werden danach abgeklemmt.
        sal_Bool bFoundXInterface = sal_False;

        sal_Int32 nClassCount = SupportedClassSeq.getLength();
        for( sal_Int32 nIdx = 0 ; nIdx < nClassCount; nIdx++ )
        {
            Reference<XIdlClass> xImplClass2 = SupportedClassSeq.getConstArray()[nIdx];
            while( xImplClass2.is() )
            {
                // Interfaces der Implementation holen
                Sequence< Reference<XIdlClass> > aClassSeq = xImplClass2->getInterfaces();
                sal_Int32 nIfaceCount = aClassSeq.getLength();

                aClassSeq.realloc( nIfaceCount + 1 );
                aClassSeq.getArray()[ nIfaceCount ] = xImplClass2;
                nIfaceCount++;

                const Reference<XIdlClass>* pParamArray = aClassSeq.getConstArray();

                for( sal_Int32 j = 0 ; j < nIfaceCount ; j++ )
                {
                    const Reference<XIdlClass>& rxIfaceClass = pParamArray[j];
                    if (!seen.insert(rxIfaceClass->getName()).second) {
                        continue;
                    }

                    // 2. Fields als Properties registrieren

                    // Felder holen
                    Sequence< Reference<XIdlField> > fields = rxIfaceClass->getFields();
                    const Reference<XIdlField>* pFields = fields.getConstArray();
                    sal_Int32 nLen = fields.getLength();

                    for( i = 0 ; i < nLen ; i++ )
                    {
                        Reference<XIdlField> xField = pFields[i];
                        Reference<XIdlClass> xPropType = xField->getType();

                        // Ist die PropertySequence gross genug?
                        pAccess->checkPropertyArraysSize
                            ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );

                        // In eigenes Property-Array eintragen
                        Property& rProp = pAllPropArray[ rPropCount ];
                        OUString aFieldName = xField->getName();
                        rProp.Name = aFieldName;
                        rProp.Handle = rPropCount;
                        Type aFieldType( xPropType->getTypeClass(), xPropType->getName() );
                        rProp.Type = aFieldType;
                        FieldAccessMode eAccessMode = xField->getAccessMode();
                        rProp.Attributes = (eAccessMode == FieldAccessMode_READONLY ||
                                            eAccessMode == FieldAccessMode_CONST)
                            ? READONLY : 0;

                        // Namen in Hashtable eintragen
                        OUString aPropName = rProp.Name;

                        // Haben wir den Namen schon?
                        IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                        if( !( aIt == rPropNameMap.end() ) )
                            continue;

                        // Neuer Eintrag in die Hashtable
                        rPropNameMap[ aPropName ] = rPropCount;

                        // Tabelle fuer XExactName pflegen
                        rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;

                        // Field merken
                        pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                                                          pInterfaces1, rPropCount );
                        pInterfaces1[ rPropCount ] = xField;

                        // Art der Property merken
                        pMapTypeArray[ rPropCount ] = MAP_FIELD;
                        pPropertyConceptArray[ rPropCount ] = ATTRIBUTES;
                        pAccess->mnAttributePropCount++;

                        // Count pflegen
                        rPropCount++;
                    }



                    // 3. Methoden

                    // Zaehler fuer die gefundenen Listener
                    sal_Int32 nListenerCount = 0;

                    // Alle Methoden holen und merken
                    Sequence< Reference<XIdlMethod> > methods = rxIfaceClass->getMethods();
                    const Reference<XIdlMethod>* pSourceMethods = methods.getConstArray();
                    sal_Int32 nSourceMethodCount = methods.getLength();

                    // 3. a) get/set- und Listener-Methoden suchen

                    // Feld fuer Infos ueber die Methoden anlegen, damit spaeter leicht die Methoden
                    // gefunden werden koennen, die nicht im Zusammenhang mit Properties oder Listenern
                    // stehen. NEU: auch MethodConceptArray initialisieren
                    enum MethodType
                    {
                        STANDARD_METHOD,            // normale Methode, kein Bezug zu Properties oder Listenern
                        GETSET_METHOD,                // gehoert zu einer get/set-Property
                        ADD_LISTENER_METHOD,        // add-Methode einer Listener-Schnittstelle
                        REMOVE_LISTENER_METHOD,        // remove-Methode einer Listener-Schnittstelle
                        INVALID_METHOD                // Methode, deren Klasse nicht beruecksichtigt wird, z.B. XPropertySet
                    };
                    MethodType* pMethodTypes = new MethodType[ nSourceMethodCount ];
                    sal_Int32* pLocalMethodConcepts = new sal_Int32[ nSourceMethodCount ];
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        pMethodTypes[ i ] = STANDARD_METHOD;
                        pLocalMethodConcepts[ i ] = 0;
                    }

                    OUString aMethName;
                    OUString aPropName;
                    OUString aStartStr;
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        // Methode ansprechen
                        const Reference<XIdlMethod>& rxMethod_i = pSourceMethods[i];
                        sal_Int32& rMethodConcept_i = pLocalMethodConcepts[ i ];

                        // Namen besorgen
                        aMethName = rxMethod_i->getName();

                        // Methoden katalogisieren
                        // Alle (?) Methoden von XInterface filtern, damit z.B. nicht
                        // vom Scripting aus acquire oder release gerufen werden kann
                        rtl::OUString className(
                            rxMethod_i->getDeclaringClass()->getName());
                        if (className == "com.sun.star.uno.XInterface") {
                            bFoundXInterface = sal_True;

                            if( bXInterfaceIsInvalid )
                            {
                                pMethodTypes[ i ] = INVALID_METHOD;
                                continue;
                            }
                            else
                            {
                                if( aMethName != "queryInterface" )
                                {
                                    rMethodConcept_i |= MethodConcept::DANGEROUS;
                                    continue;
                                }
                            }
                        } else if (className == "com.sun.star.uno.XAggregation")
                        {
                            if( aMethName == "setDelegator" )
                            {
                                rMethodConcept_i |= MethodConcept::DANGEROUS;
                                continue;
                            }
                        } else if (className
                                   == "com.sun.star.container.XElementAccess")
                        {
                            rMethodConcept_i |= ( NAMECONTAINER  |
                                                  INDEXCONTAINER |
                                                  ENUMERATION );
                        } else if ((className
                                    == "com.sun.star.container.XNameContainer")
                                   || (className
                                       == "com.sun.star.container.XNameAccess"))
                        {
                            rMethodConcept_i |= NAMECONTAINER;
                        } else if ((className
                                    == "com.sun.star.container.XIndexContainer")
                                   || (className
                                       == "com.sun.star.container.XIndexAccess"))
                        {
                            rMethodConcept_i |= INDEXCONTAINER;
                        } else if (className
                                   == "com.sun.star.container.XEnumerationAccess")
                        {
                            rMethodConcept_i |= ENUMERATION;
                        }

                        // Wenn der Name zu kurz ist, wird's sowieso nichts
                        if( aMethName.getLength() <= 3 )
                            continue;

                        // Ist es eine get-Methode?
                        aStartStr = aMethName.copy( 0, 3 );
                        if( aStartStr == "get" )
                        {
                            // Namen der potentiellen Property
                            aPropName = aMethName.copy( 3 );

                            // get-Methode darf keinen Parameter haben
                            Sequence< Reference<XIdlClass> > getParams = rxMethod_i->getParameterTypes();
                            if( getParams.getLength() > 0 )
                            {
                                continue;
                            }

                            // Haben wir den Namen schon?
                            IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                            if( !( aIt == rPropNameMap.end() ) )
                            {
                                /* TODO
                                   OSL_TRACE(
                                   String( "Introspection: Property \"" ) +
                                   OOUStringToString( aPropName, CHARSET_SYSTEM ) +
                                   String( "\" found more than once" ) );
                                */
                                continue;
                            }

                            // Eine readonly-Property ist es jetzt mindestens schon
                            rMethodConcept_i |= PROPERTY;

                            pMethodTypes[i] = GETSET_METHOD;
                            Reference<XIdlClass> xGetRetType = rxMethod_i->getReturnType();

                            // Ist die PropertySequence gross genug?
                            pAccess->checkPropertyArraysSize
                                ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );

                            // In eigenes Property-Array eintragen
                            Property& rProp = pAllPropArray[ rPropCount ];
                            rProp.Name = aPropName;
                            rProp.Handle = rPropCount;
                            rProp.Type = Type( xGetRetType->getTypeClass(), xGetRetType->getName() );
                            rProp.Attributes = READONLY;

                            // Neuer Eintrag in die Hashtable
                            rPropNameMap[ aPropName ] = rPropCount;

                            // Tabelle fuer XExactName pflegen
                            rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;

                            // get-Methode merken
                            pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                                                              pInterfaces1, rPropCount );
                            pInterfaces1[ rPropCount ] = rxMethod_i;

                            // Art der Property merken
                            pMapTypeArray[ rPropCount ] = MAP_GETSET;
                            pPropertyConceptArray[ rPropCount ] = METHODS;
                            pAccess->mnMethodPropCount++;

                            // Passende set-Methode suchen
                            sal_Int32 k;
                            for( k = 0 ; k < nSourceMethodCount ; k++ )
                            {
                                // Methode ansprechen
                                const Reference<XIdlMethod>& rxMethod_k = pSourceMethods[k];

                                // Nur Methoden nehmen, die nicht schon zugeordnet sind
                                if( k == i || pMethodTypes[k] != STANDARD_METHOD )
                                    continue;

                                // Name holen und auswerten
                                OUString aMethName2 = rxMethod_k->getName();
                                OUString aStartStr2 = aMethName2.copy( 0, 3 );
                                // ACHTUNG: Wegen SDL-Bug NICHT != bei OUString verwenden !!!
                                if( !( aStartStr2 == "set" ) )
                                    continue;

                                // Ist es denn der gleiche Name?
                                OUString aPropName2 = aMethName2.copy( 3 );
                                // ACHTUNG: Wegen SDL-Bug NICHT != bei OUString verwenden !!!
                                if( !( aPropName == aPropName2 ) )
                                    continue;

                                // set-Methode muss void returnen
                                Reference<XIdlClass> xSetRetType = rxMethod_k->getReturnType();
                                if( xSetRetType->getTypeClass() != TypeClass_VOID )
                                {
                                    continue;
                                }

                                // set-Methode darf nur einen Parameter haben
                                Sequence< Reference<XIdlClass> > setParams = rxMethod_k->getParameterTypes();
                                sal_Int32 nParamCount = setParams.getLength();
                                if( nParamCount != 1 )
                                {
                                    continue;
                                }

                                // Jetzt muss nur noch der return-Typ dem Parameter-Typ entsprechen
                                const Reference<XIdlClass>* pParamArray2 = setParams.getConstArray();
                                Reference<XIdlClass> xParamType = pParamArray2[ 0 ];
                                if( xParamType->equals( xGetRetType ) )
                                {
                                    pLocalMethodConcepts[ k ] = PROPERTY;

                                    pMethodTypes[k] = GETSET_METHOD;

                                    // ReadOnly-Flag wieder loschen
                                    rProp.Attributes &= ~READONLY;

                                    // set-Methode merken
                                    pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq2,
                                                                      pInterfaces2, rPropCount );
                                    pInterfaces2[ rPropCount ] = rxMethod_k;
                                }
                            }

                            // Count pflegen
                            rPropCount++;
                        }

                        // Ist es eine addListener-Methode?
                        else if( aStartStr == "add" )
                        {
                            OUString aListenerStr( "Listener" );

                            // Namen der potentiellen Property
                            sal_Int32 nStrLen = aMethName.getLength();
                            sal_Int32 nCopyLen = nStrLen - aListenerStr.getLength();
                            OUString aEndStr = aMethName.copy( nCopyLen > 0 ? nCopyLen : 0 );

                            // Endet das Teil auf Listener?
                            // ACHTUNG: Wegen SDL-Bug NICHT != bei OUString verwenden !!!
                            if( !( aEndStr == aListenerStr ) )
                                continue;

                            // Welcher Listener?
                            OUString aListenerName = aMethName.copy( 3, nStrLen - aListenerStr.getLength() - 3 );

                            // TODO: Hier koennten noch genauere Pruefungen vorgenommen werden
                            // - Rueckgabe-Typ
                            // - Anzahl und Art der Parameter


                            // Passende remove-Methode suchen, sonst gilt's nicht
                            sal_Int32 k;
                            for( k = 0 ; k < nSourceMethodCount ; k++ )
                            {
                                // Methode ansprechen
                                const Reference<XIdlMethod>& rxMethod_k = pSourceMethods[k];

                                // Nur Methoden nehmen, die nicht schon zugeordnet sind
                                if( k == i || pMethodTypes[k] != STANDARD_METHOD )
                                    continue;

                                // Name holen und auswerten
                                OUString aMethName2 = rxMethod_k->getName();
                                sal_Int32 nNameLen = aMethName2.getLength();
                                sal_Int32 nCopyLen2 = (nNameLen < 6) ? nNameLen : 6;
                                OUString aStartStr2 = aMethName2.copy( 0, nCopyLen2 );
                                OUString aRemoveStr("remove" );
                                // ACHTUNG: Wegen SDL-Bug NICHT != bei OUString verwenden !!!
                                if( !( aStartStr2 == aRemoveStr ) )
                                    continue;

                                // Ist es denn der gleiche Listener?
                                if( aMethName2.getLength() - aRemoveStr.getLength() <= aListenerStr.getLength() )
                                    continue;
                                OUString aListenerName2 = aMethName2.copy
                                      ( 6, aMethName2.getLength() - aRemoveStr.getLength() - aListenerStr.getLength() );
                                // ACHTUNG: Wegen SDL-Bug NICHT != bei OUString verwenden !!!
                                if( !( aListenerName == aListenerName2 ) )
                                    continue;

                                // TODO: Hier koennten noch genauere Pruefungen vorgenommen werden
                                // - Rueckgabe-Typ
                                // - Anzahl und Art der Parameter


                                // Methoden sind als Listener-Schnittstelle erkannt
                                rMethodConcept_i |= LISTENER;
                                pLocalMethodConcepts[ k ] |= LISTENER;

                                pMethodTypes[i] = ADD_LISTENER_METHOD;
                                pMethodTypes[k] = REMOVE_LISTENER_METHOD;
                                nListenerCount++;
                            }
                        }
                    }


                    // Jetzt koennen noch SET-Methoden ohne zugehoerige GET-Methode existieren,
                    // diese muessen zu Write-Only-Properties gemachte werden.
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        // Methode ansprechen
                        const Reference<XIdlMethod>& rxMethod_i = pSourceMethods[i];

                        // Nur Methoden nehmen, die nicht schon zugeordnet sind
                        if( pMethodTypes[i] != STANDARD_METHOD )
                            continue;

                        // Namen besorgen
                        aMethName = rxMethod_i->getName();

                        // Wenn der Name zu kurz ist, wird's sowieso nichts
                        if( aMethName.getLength() <= 3 )
                            continue;

                        // Ist es eine set-Methode ohne zugehoerige get-Methode?
                        aStartStr = aMethName.copy( 0, 3 );
                        if( aStartStr == "set" )
                        {
                            // Namen der potentiellen Property
                            aPropName = aMethName.copy( 3 );

                            // set-Methode muss void returnen
                            Reference<XIdlClass> xSetRetType = rxMethod_i->getReturnType();
                            if( xSetRetType->getTypeClass() != TypeClass_VOID )
                            {
                                continue;
                            }

                            // set-Methode darf nur einen Parameter haben
                            Sequence< Reference<XIdlClass> > setParams = rxMethod_i->getParameterTypes();
                            sal_Int32 nParamCount = setParams.getLength();
                            if( nParamCount != 1 )
                            {
                                continue;
                            }

                            // Haben wir den Namen schon?
                            IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                            if( !( aIt == rPropNameMap.end() ) )
                            {
                                /* TODO:
                                   OSL_TRACE(
                                   String( "Introspection: Property \"" ) +
                                   OOUStringToString( aPropName, CHARSET_SYSTEM ) +
                                   String( "\" found more than once" ) );
                                */
                                continue;
                            }

                            // Alles klar, es ist eine Write-Only-Property
                            pLocalMethodConcepts[ i ] = PROPERTY;

                            pMethodTypes[i] = GETSET_METHOD;
                            Reference<XIdlClass> xGetRetType = setParams.getConstArray()[0];

                            // Ist die PropertySequence gross genug?
                            pAccess->checkPropertyArraysSize
                                ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );

                            // In eigenes Property-Array eintragen
                            Property& rProp = pAllPropArray[ rPropCount ];
                            rProp.Name = aPropName;
                            rProp.Handle = rPropCount;
                            rProp.Type = Type( xGetRetType->getTypeClass(), xGetRetType->getName() );
                            rProp.Attributes = 0;    // PROPERTY_WRITEONLY ???

                            // Neuer Eintrag in die Hashtable
                            rPropNameMap[ aPropName ] = rPropCount;

                            // Tabelle fuer XExactName pflegen
                            rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;

                            // set-Methode merken
                            pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq2,
                                                              pInterfaces2, rPropCount );
                            pInterfaces2[ rPropCount ] = rxMethod_i;

                            // Art der Property merken
                            pMapTypeArray[ rPropCount ] = MAP_SETONLY;
                            pPropertyConceptArray[ rPropCount ] = METHODS;
                            pAccess->mnMethodPropCount++;

                            // Count pflegen
                            rPropCount++;
                        }
                    }




                    // 4. Methoden in die Gesamt-Sequence uebernehmen

                    // Wieviele Methoden muessen in die Method-Sequence?
                    sal_Int32 nExportedMethodCount = 0;
                    sal_Int32 nSupportedListenerCount = 0;
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        if( pMethodTypes[ i ] != INVALID_METHOD )
                        {
                            nExportedMethodCount++;
                        }
                        if( pMethodTypes[ i ] == ADD_LISTENER_METHOD )
                        {
                            nSupportedListenerCount++;
                        }
                    }

                    // Sequences im Access-Objekt entsprechend aufbohren
                    pAccess->maAllMethodSeq.realloc( nExportedMethodCount + iAllExportedMethod );
                    pAccess->maMethodConceptSeq.realloc( nExportedMethodCount + iAllExportedMethod );
                    pAccess->maSupportedListenerSeq.realloc( nSupportedListenerCount + iAllSupportedListener );

                    // Methoden reinschreiben
                    Reference<XIdlMethod>* pDestMethods = pAccess->maAllMethodSeq.getArray();
                    sal_Int32* pMethodConceptArray = pAccess->maMethodConceptSeq.getArray();
                    Type* pListenerClassRefs = pAccess->maSupportedListenerSeq.getArray();
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        if( pMethodTypes[ i ] != INVALID_METHOD )
                        {
                            // Methode ansprechen
                            const Reference<XIdlMethod>& rxMethod = pSourceMethods[i];

                            // Namen in Hashtable eintragen, wenn nicht schon bekannt
                            OUString aMethName2 = rxMethod->getName();
                            IntrospectionNameMap::iterator aIt = rMethodNameMap.find( aMethName2 );
                            if( aIt == rMethodNameMap.end() )
                            {
                                // Eintragen
                                rMethodNameMap[ aMethName2 ] = iAllExportedMethod;

                                // Tabelle fuer XExactName pflegen
                                rLowerToExactNameMap[ toLower( aMethName2 ) ] = aMethName2;
                            }
                            else
                            {
                                sal_Int32 iHashResult = (*aIt).second;

                                Reference<XIdlMethod> xExistingMethod = pDestMethods[ iHashResult ];

                                Reference< XIdlClass > xExistingMethClass =
                                    xExistingMethod->getDeclaringClass();
                                Reference< XIdlClass > xNewMethClass = rxMethod->getDeclaringClass();
                                if( xExistingMethClass->equals( xNewMethClass ) )
                                    continue;
                            }

                            pDestMethods[ iAllExportedMethod ] = rxMethod;

                            // Wenn kein Concept gesetzt wurde, ist die Methode "normal"
                            sal_Int32& rMethodConcept_i = pLocalMethodConcepts[ i ];
                            if( !rMethodConcept_i )
                                rMethodConcept_i = MethodConcept_NORMAL_IMPL;
                            pMethodConceptArray[ iAllExportedMethod ] = rMethodConcept_i;
                            iAllExportedMethod++;
                        }
                        if( pMethodTypes[ i ] == ADD_LISTENER_METHOD )
                        {
                            // Klasse des Listeners ermitteln
                            const Reference<XIdlMethod>& rxMethod = pSourceMethods[i];

                            // void als Default-Klasse eintragen
                            css::uno::Reference<css::reflection::XIdlClass>
                                xListenerClass(
                                    reflection_->forName(
                                        cppu::UnoType<cppu::UnoVoidType>::get()
                                        .getTypeName()));
                            // ALT: Reference<XIdlClass> xListenerClass = Void_getReflection()->getIdlClass();

                            // 1. Moeglichkeit: Parameter nach einer Listener-Klasse durchsuchen
                            // Nachteil: Superklassen muessen rekursiv durchsucht werden
                            Sequence< Reference<XIdlClass> > aParams = rxMethod->getParameterTypes();
                            const Reference<XIdlClass>* pParamArray2 = aParams.getConstArray();

                            css::uno::Reference<css::reflection::XIdlClass>
                                xEventListenerClass(
                                    reflection_->forName(
                                        cppu::UnoType<
                                            css::lang::XEventListener>::get()
                                        .getTypeName()));
                            // ALT: Reference<XIdlClass> xEventListenerClass = XEventListener_getReflection()->getIdlClass();
                            sal_Int32 nParamCount = aParams.getLength();
                            sal_Int32 k;
                            for( k = 0 ; k < nParamCount ; k++ )
                            {
                                const Reference<XIdlClass>& rxClass = pParamArray2[k];

                                // Sind wir von einem Listener abgeleitet?
                                if( rxClass->equals( xEventListenerClass ) ||
                                    isDerivedFrom( rxClass, xEventListenerClass ) )
                                {
                                    xListenerClass = rxClass;
                                    break;
                                }
                            }

                            // 2. Moeglichkeit: Namen der Methode auswerden
                            // Nachteil: geht nicht bei Test-Listenern, die es nicht gibt
                            //aMethName = rxMethod->getName();
                            //aListenerName = aMethName.Copy( 3, aMethName.Len()-8-3 );
                            //Reference<XIdlClass> xListenerClass = reflection->forName( aListenerName );
                            Type aListenerType( TypeClass_INTERFACE, xListenerClass->getName() );
                            pListenerClassRefs[ iAllSupportedListener ] = aListenerType;
                            iAllSupportedListener++;
                        }
                    }

                    // Wenn in diesem Durchlauf XInterface-Methoden
                    // dabei waren, diese zukuenftig ignorieren
                    if( bFoundXInterface )
                        bXInterfaceIsInvalid = sal_True;

                    delete[] pMethodTypes;
                    delete[] pLocalMethodConcepts;
                }

                // Super-Klasse(n) vorhanden? Dann dort fortsetzen
                Sequence< Reference<XIdlClass> > aSuperClassSeq = xImplClass2->getSuperclasses();

                // Zur Zeit wird nur von einer Superklasse ausgegangen
                if( aSuperClassSeq.getLength() >= 1 )
                {
                    xImplClass2 = aSuperClassSeq.getConstArray()[0];
                    OSL_ENSURE( xImplClass2.is(), "super class null" );
                }
                else
                {
                    xImplClass2 = NULL;
                }
            }
        }

        // Anzahl der exportierten Methoden uebernehmen und Sequences anpassen
        // (kann abweichen, weil doppelte Methoden erst nach der Ermittlung
        //  von nExportedMethodCount herausgeworfen werden)
        sal_Int32& rMethCount = pAccess->mnMethCount;
        rMethCount = iAllExportedMethod;
        pAccess->maAllMethodSeq.realloc( rMethCount );
        pAccess->maMethodConceptSeq.realloc( rMethCount );

        // Groesse der Property-Sequences anpassen
        pAccess->maAllPropertySeq.realloc( rPropCount );
        pAccess->maPropertyConceptSeq.realloc( rPropCount );
        pAccess->maMapTypeSeq.realloc( rPropCount );
    }
    // Bei structs Fields als Properties registrieren
    else //if( eType == TypeClass_STRUCT )
    {
        // Ist es ein Interface oder eine struct?
        //Reference<XIdlClass> xClassRef = aToInspectObj.getReflection()->getIdlClass();
        css::uno::Reference<css::reflection::XIdlClass> xClassRef(
            reflection_->forName(aToInspectObj.getValueTypeName()));
        if( !xClassRef.is() )
        {
            SAL_WARN( "stoc", "Can't get XIdlClass from Reflection" );
            return new ImplIntrospectionAccess(aToInspectObj, pAccess);
        }

        // Felder holen
        Sequence< Reference<XIdlField> > fields = xClassRef->getFields();
        const Reference<XIdlField>* pFields = fields.getConstArray();
        sal_Int32 nLen = fields.getLength();

        for( i = 0 ; i < nLen ; i++ )
        {
            Reference<XIdlField> xField = pFields[i];
            Reference<XIdlClass> xPropType = xField->getType();
            OUString aPropName = xField->getName();

            // Ist die PropertySequence gross genug?
            pAccess->checkPropertyArraysSize
                ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );

            // In eigenes Property-Array eintragen
            Property& rProp = pAllPropArray[ rPropCount ];
            rProp.Name = aPropName;
            rProp.Handle = rPropCount;
            rProp.Type = Type( xPropType->getTypeClass(), xPropType->getName() );
            FieldAccessMode eAccessMode = xField->getAccessMode();
            rProp.Attributes = (eAccessMode == FieldAccessMode_READONLY ||
                                eAccessMode == FieldAccessMode_CONST)
                                ? READONLY : 0;

            //FieldAccessMode eAccessMode = xField->getAccessMode();
            //rProp.Attributes = (eAccessMode == FieldAccessMode::READONLY || eAccessMode == CONST)
                //? PropertyAttribute::READONLY : 0;

            // Namen in Hashtable eintragen
            rPropNameMap[ aPropName ] = rPropCount;

            // Tabelle fuer XExactName pflegen
            rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;

            // Field merken
            pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                pInterfaces1, rPropCount );
            pInterfaces1[ rPropCount ] = xField;

            // Art der Property merken
            pMapTypeArray[ rPropCount ] = MAP_FIELD;
            pPropertyConceptArray[ rPropCount ] = ATTRIBUTES;
            pAccess->mnAttributePropCount++;

            // Count pflegen
            rPropCount++;
        }
    }

    // Property-Sequence auf die richtige Laenge bringen
    pAccess->maAllPropertySeq.realloc( pAccess->mnPropCount );

    return new ImplIntrospectionAccess(aToInspectObj, pAccess);
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(new Implementation(context))
    {}

    rtl::Reference<cppu::OWeakObject> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_stoc_Introspection_get_implementation(
    css::uno::XComponentContext * context,
    css::uno::Sequence<css::uno::Any> const & arguments)
{
    SAL_WARN_IF(
        arguments.hasElements(), "stoc", "unexpected singleton arguments");
    return cppu::acquire(Singleton::get(context).instance.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
