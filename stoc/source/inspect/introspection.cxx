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

#include <string.h>

#define INTROSPECTION_CACHE_MAX_SIZE 100

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
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
#include <rtl/ref.hxx>
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

#define IMPLEMENTATION_NAME "com.sun.star.comp.stoc.Introspection"
#define SERVICE_NAME        "com.sun.star.beans.Introspection"

namespace stoc_inspect
{

typedef WeakImplHelper3< XIntrospectionAccess, XMaterialHolder, XExactName > IntrospectionAccessHelper;


//==================================================================================================

// Spezial-Wert fuer Method-Concept, um "normale" Funktionen kennzeichnen zu koennen
#define  MethodConcept_NORMAL_IMPL        0x80000000


// Methode zur Feststellung, ob eine Klasse von einer anderen abgeleitet ist
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

//========================================================================

// *** Klassifizierung der Properties (kein enum, um Sequence verwenden zu koennen) ***
// Properties aus einem PropertySet-Interface
#define MAP_PROPERTY_SET    0
// Properties aus Fields
#define MAP_FIELD            1
// Properties, die durch get/set-Methoden beschrieben werden
#define MAP_GETSET            2
// Properties, die nur eine set-Methode haben
#define MAP_SETONLY            3


// Schrittweite, in der die Groesse der Sequences angepasst wird
#define ARRAY_SIZE_STEP        20



//**************************************
//*** IntrospectionAccessStatic_Impl ***
//**************************************
// Entspricht dem alten IntrospectionAccessImpl, bildet jetzt den statischen
// Anteil des neuen Instanz-bezogenen ImplIntrospectionAccess

// Hashtable fuer die Suche nach Namen
struct hashName_Impl
{
    size_t operator()(const ::rtl::OUString Str) const
    {
        return (size_t)Str.hashCode();
    }
};

struct eqName_Impl
{
    sal_Bool operator()(const ::rtl::OUString Str1, const ::rtl::OUString Str2) const
    {
        return ( Str1 == Str2 );
    }
};

typedef boost::unordered_map
<
    ::rtl::OUString,
    sal_Int32,
    hashName_Impl,
    eqName_Impl
>
IntrospectionNameMap;


// Hashtable zur Zuordnung der exakten Namen zu den zu Lower-Case
// konvertierten Namen, dient zur Unterst�tzung von XExactName
typedef boost::unordered_map
<
    ::rtl::OUString,
    ::rtl::OUString,
    hashName_Impl,
    eqName_Impl
>
LowerToExactNameMap;


class ImplIntrospectionAccess;
class IntrospectionAccessStatic_Impl: public salhelper::SimpleReferenceObject
{
    friend class ImplIntrospection;
    friend class ImplIntrospectionAccess;

    // CoreReflection halten
    Reference< XIdlReflection > mxCoreReflection;

    // InterfaceSequences, um Zusatz-Infos zu einer Property speichern zu koennen.
    // z.B. das Field bei MAP_FIELD, die get/set-Methoden bei MAP_GETSET usw.
    Sequence< Reference<XInterface> > aInterfaceSeq1;
    Sequence< Reference<XInterface> > aInterfaceSeq2;

    // Hashtables fuer die Namen
    IntrospectionNameMap maPropertyNameMap;
    IntrospectionNameMap maMethodNameMap;
    LowerToExactNameMap  maLowerToExactNameMap;

    // Sequence aller Properties, auch zum Liefern aus getProperties()
    Sequence<Property> maAllPropertySeq;

    // Mapping der Properties auf Zugriffs-Arten
    Sequence<sal_Int16> maMapTypeSeq;

    // Klassifizierung der gefundenen Methoden
    Sequence<sal_Int32> maPropertyConceptSeq;

    // Anzahl der Properties
    sal_Int32 mnPropCount;

    // Anzahl der Properties, die den jeweiligen Konzepten zugeordnet sind
    //sal_Int32 mnDangerousPropCount;
    sal_Int32 mnPropertySetPropCount;
    sal_Int32 mnAttributePropCount;
    sal_Int32 mnMethodPropCount;

    // Flag, ob ein FastPropertySet unterstuetzt wird
    sal_Bool mbFastPropSet;

    // Original-Handles eines FastPropertySets
    sal_Int32* mpOrgPropertyHandleArray;

    // MethodSequence, die alle Methoden aufnimmt
    Sequence< Reference<XIdlMethod> > maAllMethodSeq;

    // Klassifizierung der gefundenen Methoden
    Sequence<sal_Int32> maMethodConceptSeq;

    // Anzahl der Methoden
    sal_Int32 mnMethCount;

    // Sequence der Listener, die angemeldet werden koennen
    Sequence< Type > maSupportedListenerSeq;

    // BaseInit (soll spaeter in der Applikation erfolgen!)
    void BaseInit( void );

    // Hilfs-Methoden zur Groessen-Anpassung der Sequences
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
    sal_Int32 getPropertyIndex( const ::rtl::OUString& aPropertyName ) const;
    sal_Int32 getMethodIndex( const ::rtl::OUString& aMethodName ) const;

    // Methoden von XIntrospectionAccess (ALT, jetzt nur Impl)
    void setPropertyValue(const Any& obj, const ::rtl::OUString& aPropertyName, const Any& aValue) const;
//    void setPropertyValue(Any& obj, const ::rtl::OUString& aPropertyName, const Any& aValue) const;
    Any getPropertyValue(const Any& obj, const ::rtl::OUString& aPropertyName) const;
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

    // Property-Daten
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

    // Method-Daten
    mnMethCount = 0;
}

sal_Int32 IntrospectionAccessStatic_Impl::getPropertyIndex( const ::rtl::OUString& aPropertyName ) const
{
    sal_Int32 iHashResult = -1;
    IntrospectionAccessStatic_Impl* pThis = (IntrospectionAccessStatic_Impl*)this;
    IntrospectionNameMap::iterator aIt = pThis->maPropertyNameMap.find( aPropertyName );
    if( !( aIt == pThis->maPropertyNameMap.end() ) )
        iHashResult = (*aIt).second;
    return iHashResult;
}

sal_Int32 IntrospectionAccessStatic_Impl::getMethodIndex( const ::rtl::OUString& aMethodName ) const
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
        nSearchFrom = aMethodName.getLength();
        while( true )
        {
            // Strategy: Search back until the first '_' is found
            sal_Int32 nFound = aMethodName.lastIndexOf( '_', nSearchFrom );
            if( nFound == -1 )
                break;

            ::rtl::OUString aPureMethodName = aMethodName.copy( nFound + 1 );

            aIt = pThis->maMethodNameMap.find( aPureMethodName );
            if( !( aIt == pThis->maMethodNameMap.end() ) )
            {
                // Check if it can be a type?
                // Problem: Does not work if package names contain _ ?!
                ::rtl::OUString aStr = aMethodName.copy( 0, nFound );
                ::rtl::OUString aTypeName = aStr.replace( '_', '.' );
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

                            ::rtl::OUString aTestClassName = xMethod2->getDeclaringClass()->getName();
                            ::rtl::OUString aTestMethodName = xMethod2->getName();

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

void IntrospectionAccessStatic_Impl::setPropertyValue( const Any& obj, const ::rtl::OUString& aPropertyName, const Any& aValue ) const
//void IntrospectionAccessStatic_Impl::setPropertyValue( Any& obj, const ::rtl::OUString& aPropertyName, const Any& aValue ) const
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
    // Handelt es sich bei dem uebergebenen Objekt ueberhaupt um was passendes?
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

    // Flags pruefen
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
            // Property besorgen
            const Property& rProp = maAllPropertySeq.getConstArray()[ nSequenceIndex ];

            // Interface-Parameter auf den richtigen Typ bringen
            sal_Bool bUseCopy = sal_False;
            Any aRealValue;

            TypeClass eValType = aValue.getValueType().getTypeClass();
            if( eValType == TypeClass_INTERFACE )
            {
                Type aPropType = rProp.Type;
                ::rtl::OUString aTypeName( aPropType.getTypeName() );
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
                    xFastPropSet->setFastPropertyValue( nOrgHandle, bUseCopy ? aRealValue : aValue );
                }
                else
                {
                    // throw UnknownPropertyException
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
            // set-Methode holen
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

Any IntrospectionAccessStatic_Impl::getPropertyValue( const Any& obj, const ::rtl::OUString& aPropertyName ) const
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


//*******************************
//*** ImplIntrospectionAccess ***
//*******************************

// Neue Impl-Klasse im Rahmen der Introspection-Umstellung auf Instanz-gebundene
// Introspection mit Property-Zugriff ueber XPropertySet. Die alte Klasse
// ImplIntrospectionAccess lebt als IntrospectionAccessStatic_Impl
class ImplIntrospectionAccess : public IntrospectionAccessHelper
{
    friend class ImplIntrospection;

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
        throw( RuntimeException );
    virtual sal_Int32 SAL_CALL getSuppliedPropertyConcepts(void)
        throw( RuntimeException );
    virtual Property SAL_CALL getProperty(const ::rtl::OUString& Name, sal_Int32 PropertyConcepts)
        throw( NoSuchElementException, RuntimeException );
    virtual sal_Bool SAL_CALL hasProperty(const ::rtl::OUString& Name, sal_Int32 PropertyConcepts)
        throw( RuntimeException );
    virtual Sequence< Property > SAL_CALL getProperties(sal_Int32 PropertyConcepts)
          throw( RuntimeException );
    virtual Reference<XIdlMethod> SAL_CALL getMethod(const ::rtl::OUString& Name, sal_Int32 MethodConcepts)
          throw( NoSuchMethodException, RuntimeException );
    virtual sal_Bool SAL_CALL hasMethod(const ::rtl::OUString& Name, sal_Int32 MethodConcepts)
          throw( RuntimeException );
    virtual Sequence< Reference<XIdlMethod> > SAL_CALL getMethods(sal_Int32 MethodConcepts)
          throw( RuntimeException );
    virtual Sequence< Type > SAL_CALL getSupportedListeners(void)
          throw( RuntimeException );
    using OWeakObject::queryAdapter;
    virtual Reference<XInterface> SAL_CALL queryAdapter( const Type& rType )
          throw( IllegalTypeException, RuntimeException );

    // Methoden von XMaterialHolder
    virtual Any SAL_CALL getMaterial(void) throw(RuntimeException);

    // Methoden von XExactName
    virtual ::rtl::OUString SAL_CALL getExactName( const ::rtl::OUString& rApproximateName ) throw( RuntimeException );
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


//*******************************
//*** ImplIntrospectionAdapter ***
//*******************************

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
    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    virtual void        SAL_CALL release() throw() { OWeakObject::release(); }

    // Methoden von XPropertySet
    virtual Reference<XPropertySetInfo> SAL_CALL getPropertySetInfo() throw( RuntimeException );
    virtual void SAL_CALL setPropertyValue(const ::rtl::OUString& aPropertyName, const Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException );
    virtual Any SAL_CALL getPropertyValue(const ::rtl::OUString& aPropertyName)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );

    // Methoden von XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException );
    virtual Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );

    // Methoden von XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(void) throw( RuntimeException );
    virtual Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name) throw( RuntimeException );

    // Methoden von XElementAccess
    virtual Type SAL_CALL getElementType(void) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasElements(void) throw( RuntimeException );

    // Methoden von XNameAccess
    virtual Any SAL_CALL getByName(const ::rtl::OUString& Name)
        throw( NoSuchElementException, WrappedTargetException, RuntimeException );
    virtual Sequence< ::rtl::OUString > SAL_CALL getElementNames(void) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const ::rtl::OUString& Name) throw( RuntimeException );

    // Methoden von XNameContainer
    virtual void SAL_CALL insertByName(const ::rtl::OUString& Name, const Any& Element)
        throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL replaceByName(const ::rtl::OUString& Name, const Any& Element)
        throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeByName(const ::rtl::OUString& Name)
        throw( NoSuchElementException, WrappedTargetException, RuntimeException );

    // Methoden von XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( RuntimeException );
    virtual Any SAL_CALL getByIndex(sal_Int32 Index)
        throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    // Methoden von XIndexContainer
    virtual void SAL_CALL insertByIndex(sal_Int32 Index, const Any& Element)
        throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL replaceByIndex(sal_Int32 Index, const Any& Element)
        throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeByIndex(sal_Int32 Index)
        throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    // Methoden von XEnumerationAccess
    virtual Reference<XEnumeration> SAL_CALL createEnumeration(void) throw( RuntimeException );

    // Methoden von XIdlArray
    virtual void SAL_CALL realloc(Any& array, sal_Int32 length)
        throw( IllegalArgumentException, RuntimeException );
    virtual sal_Int32 SAL_CALL getLen(const Any& array) throw( IllegalArgumentException, RuntimeException );
    virtual Any SAL_CALL get(const Any& array, sal_Int32 index)
        throw( IllegalArgumentException, ArrayIndexOutOfBoundsException, RuntimeException );
    virtual void SAL_CALL set(Any& array, sal_Int32 index, const Any& value)
        throw( IllegalArgumentException, ArrayIndexOutOfBoundsException, RuntimeException );
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
    throw( RuntimeException )
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


//***************************************************
//*** Implementation von ImplIntrospectionAdapter ***
//***************************************************

// Methoden von XPropertySet
Reference<XPropertySetInfo> ImplIntrospectionAdapter::getPropertySetInfo(void)
    throw( RuntimeException )
{
    return (XPropertySetInfo *)this;
}

void ImplIntrospectionAdapter::setPropertyValue(const ::rtl::OUString& aPropertyName, const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    mpStaticImpl->setPropertyValue( mrInspectedObject, aPropertyName, aValue );
}

Any ImplIntrospectionAdapter::getPropertyValue(const ::rtl::OUString& aPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    return mpStaticImpl->getPropertyValue( mrInspectedObject, aPropertyName );
}

void ImplIntrospectionAdapter::addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
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

void ImplIntrospectionAdapter::removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
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

void ImplIntrospectionAdapter::addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
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

void ImplIntrospectionAdapter::removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
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
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
}

Any ImplIntrospectionAdapter::getFastPropertyValue(sal_Int32)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    return Any();
}

// Methoden von XPropertySetInfo
Sequence< Property > ImplIntrospectionAdapter::getProperties(void) throw( RuntimeException )
{
    return mpStaticImpl->getProperties();
}

Property ImplIntrospectionAdapter::getPropertyByName(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    return mpAccess->getProperty( Name, PropertyConcept::ALL );
}

sal_Bool ImplIntrospectionAdapter::hasPropertyByName(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    return mpAccess->hasProperty( Name, PropertyConcept::ALL );
}

// Methoden von XElementAccess
Type ImplIntrospectionAdapter::getElementType(void) throw( RuntimeException )
{
    return mxObjElementAccess->getElementType();
}

sal_Bool ImplIntrospectionAdapter::hasElements(void) throw( RuntimeException )
{
    return mxObjElementAccess->hasElements();
}

// Methoden von XNameAccess
Any ImplIntrospectionAdapter::getByName(const ::rtl::OUString& Name)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    return mxObjNameAccess->getByName( Name );
}

Sequence< ::rtl::OUString > ImplIntrospectionAdapter::getElementNames(void)
    throw( RuntimeException )
{
    return mxObjNameAccess->getElementNames();
}

sal_Bool ImplIntrospectionAdapter::hasByName(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    return mxObjNameAccess->hasByName( Name );
}

// Methoden von XNameContainer
void ImplIntrospectionAdapter::insertByName(const ::rtl::OUString& Name, const Any& Element)
    throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException )
{
    mxObjNameContainer->insertByName( Name, Element );
}

void ImplIntrospectionAdapter::replaceByName(const ::rtl::OUString& Name, const Any& Element)
    throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException )
{
    mxObjNameContainer->replaceByName( Name, Element );
}

void ImplIntrospectionAdapter::removeByName(const ::rtl::OUString& Name)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    mxObjNameContainer->removeByName( Name );
}

// Methoden von XIndexAccess
// Schon in XNameAccess: virtual Reference<XIdlClass> getElementType(void) const
sal_Int32 ImplIntrospectionAdapter::getCount(void) throw( RuntimeException )
{
    return mxObjIndexAccess->getCount();
}

Any ImplIntrospectionAdapter::getByIndex(sal_Int32 Index)
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    return mxObjIndexAccess->getByIndex( Index );
}

// Methoden von XIndexContainer
void ImplIntrospectionAdapter::insertByIndex(sal_Int32 Index, const Any& Element)
    throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    mxObjIndexContainer->insertByIndex( Index, Element );
}

void ImplIntrospectionAdapter::replaceByIndex(sal_Int32 Index, const Any& Element)
    throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    mxObjIndexContainer->replaceByIndex( Index, Element );
}

void ImplIntrospectionAdapter::removeByIndex(sal_Int32 Index)
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    mxObjIndexContainer->removeByIndex( Index );
}

// Methoden von XEnumerationAccess
// Schon in XNameAccess: virtual Reference<XIdlClass> getElementType(void) const;
Reference<XEnumeration> ImplIntrospectionAdapter::createEnumeration(void) throw( RuntimeException )
{
    return mxObjEnumerationAccess->createEnumeration();
}

// Methoden von XIdlArray
void ImplIntrospectionAdapter::realloc(Any& array, sal_Int32 length)
    throw( IllegalArgumentException, RuntimeException )
{
    mxObjIdlArray->realloc( array, length );
}

sal_Int32 ImplIntrospectionAdapter::getLen(const Any& array)
    throw( IllegalArgumentException, RuntimeException )
{
    return mxObjIdlArray->getLen( array );
}

Any ImplIntrospectionAdapter::get(const Any& array, sal_Int32 index)
    throw( IllegalArgumentException, ArrayIndexOutOfBoundsException, RuntimeException )
{
    return mxObjIdlArray->get( array, index );
}

void ImplIntrospectionAdapter::set(Any& array, sal_Int32 index, const Any& value)
    throw( IllegalArgumentException, ArrayIndexOutOfBoundsException, RuntimeException )
{
    mxObjIdlArray->set( array, index, value );
}


//**************************************************
//*** Implementation von ImplIntrospectionAccess ***
//**************************************************

// Methoden von XIntrospectionAccess
sal_Int32 ImplIntrospectionAccess::getSuppliedMethodConcepts(void)
    throw( RuntimeException )
{
    return    MethodConcept::DANGEROUS |
            PROPERTY |
            LISTENER |
            ENUMERATION |
            NAMECONTAINER |
            INDEXCONTAINER;
}

sal_Int32 ImplIntrospectionAccess::getSuppliedPropertyConcepts(void)
    throw( RuntimeException )
{
    return    PropertyConcept::DANGEROUS |
            PROPERTYSET |
            ATTRIBUTES |
            METHODS;
}

Property ImplIntrospectionAccess::getProperty(const ::rtl::OUString& Name, sal_Int32 PropertyConcepts)
    throw( NoSuchElementException, RuntimeException )
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

sal_Bool ImplIntrospectionAccess::hasProperty(const ::rtl::OUString& Name, sal_Int32 PropertyConcepts)
    throw( RuntimeException )
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
    throw( RuntimeException )
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

Reference<XIdlMethod> ImplIntrospectionAccess::getMethod(const ::rtl::OUString& Name, sal_Int32 MethodConcepts)
    throw( NoSuchMethodException, RuntimeException )
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

sal_Bool ImplIntrospectionAccess::hasMethod(const ::rtl::OUString& Name, sal_Int32 MethodConcepts)
    throw( RuntimeException )
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
    throw( RuntimeException )
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
            ::rtl::OString aNameStr = ::rtl::OUStringToOString( rxMethod->getName(), osl_getThreadTextEncoding() );
            ::rtl::OString ConceptStr;
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
    throw( RuntimeException )
{
    return mpStaticImpl->getSupportedListeners();
}

Reference<XInterface> SAL_CALL ImplIntrospectionAccess::queryAdapter( const Type& rType )
    throw( IllegalTypeException, RuntimeException )
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
Any ImplIntrospectionAccess::getMaterial(void) throw(RuntimeException)
{
    return maInspectedObject;
}

// Hilfs-Funktion zur LowerCase-Wandlung eines ::rtl::OUString
::rtl::OUString toLower( ::rtl::OUString aUStr )
{
    // Tabelle fuer XExactName pflegen
    ::rtl::OUString aOWStr( aUStr.getStr() );
    ::rtl::OUString aOWLowerStr = aOWStr.toAsciiLowerCase();
    ::rtl::OUString aLowerUStr( aOWLowerStr.getStr() );
    return aLowerUStr;
}

// Methoden von XExactName
::rtl::OUString ImplIntrospectionAccess::getExactName( const ::rtl::OUString& rApproximateName ) throw( RuntimeException )
{
    ::rtl::OUString aRetStr;
    LowerToExactNameMap::iterator aIt =
        mpStaticImpl->maLowerToExactNameMap.find( toLower( rApproximateName ) );
    if( !( aIt == mpStaticImpl->maLowerToExactNameMap.end() ) )
        aRetStr = (*aIt).second;
    return aRetStr;
}


//-----------------------------------------------------------------------------

struct hashIntrospectionKey_Impl
{
    Sequence< Reference<XIdlClass> >    aIdlClasses;
    Reference<XPropertySetInfo>            xPropInfo;
    Reference<XIdlClass>                xImplClass;
    sal_Int32                            nHitCount;

    void    IncHitCount() const { ((hashIntrospectionKey_Impl*)this)->nHitCount++; }
    hashIntrospectionKey_Impl() : nHitCount( 0 ) {}
    hashIntrospectionKey_Impl( const Sequence< Reference<XIdlClass> > & rIdlClasses,
                                        const Reference<XPropertySetInfo> & rxPropInfo,
                                        const Reference<XIdlClass> & rxImplClass );
};

hashIntrospectionKey_Impl::hashIntrospectionKey_Impl
(
    const Sequence< Reference<XIdlClass> > & rIdlClasses,
    const Reference<XPropertySetInfo> & rxPropInfo,
    const Reference<XIdlClass> & rxImplClass
)
        : aIdlClasses( rIdlClasses )
        , xPropInfo( rxPropInfo )
        , xImplClass( rxImplClass )
        , nHitCount( 0 )
{}


struct hashIntrospectionAccessCache_Impl
{
    size_t operator()(const hashIntrospectionKey_Impl & rObj ) const
    {
        return (size_t)rObj.xImplClass.get() ^ (size_t)rObj.xPropInfo.get();
    }

    bool operator()( const hashIntrospectionKey_Impl & rObj1,
                     const hashIntrospectionKey_Impl & rObj2 ) const
    {
        if( rObj1.xPropInfo != rObj2.xPropInfo
          || rObj1.xImplClass != rObj2.xImplClass )
            return sal_False;

        sal_Int32 nCount1 = rObj1.aIdlClasses.getLength();
        sal_Int32 nCount2 = rObj2.aIdlClasses.getLength();
        if( nCount1 != nCount2 )
            return sal_False;

        const Reference<XIdlClass>* pRefs1 = rObj1.aIdlClasses.getConstArray();
        const Reference<XIdlClass>* pRefs2 = rObj2.aIdlClasses.getConstArray();
        return memcmp( pRefs1, pRefs2, nCount1 * sizeof( Reference<XIdlClass> ) ) == 0;
    }

};

typedef boost::unordered_map
<
    hashIntrospectionKey_Impl,
    rtl::Reference< IntrospectionAccessStatic_Impl >,
    hashIntrospectionAccessCache_Impl,
    hashIntrospectionAccessCache_Impl
>
IntrospectionAccessCacheMap;

// For XTypeProvider
struct hashTypeProviderKey_Impl
{
    Reference<XPropertySetInfo>            xPropInfo;
    Sequence< sal_Int8 >                maImpIdSeq;
    sal_Int32                            nHitCount;

    void    IncHitCount() const { ((hashTypeProviderKey_Impl*)this)->nHitCount++; }
    hashTypeProviderKey_Impl() : nHitCount( 0 ) {}
    hashTypeProviderKey_Impl( const Reference<XPropertySetInfo> & rxPropInfo, const Sequence< sal_Int8 > & aImpIdSeq_ );
};

hashTypeProviderKey_Impl::hashTypeProviderKey_Impl
(
    const Reference<XPropertySetInfo> & rxPropInfo,
    const Sequence< sal_Int8 > & aImpIdSeq_
)
    : xPropInfo( rxPropInfo )
    , maImpIdSeq( aImpIdSeq_ )
    , nHitCount( 0 )
{}


struct TypeProviderAccessCache_Impl
{
    size_t operator()(const hashTypeProviderKey_Impl & rObj ) const;

    bool operator()( const hashTypeProviderKey_Impl & rObj1,
                     const hashTypeProviderKey_Impl & rObj2 ) const
    {
        if( rObj1.xPropInfo != rObj2.xPropInfo )
            return sal_False;

        bool bEqual = false;
        sal_Int32 nLen1 = rObj1.maImpIdSeq.getLength();
        sal_Int32 nLen2 = rObj2.maImpIdSeq.getLength();
        if( nLen1 == nLen2 && nLen1 > 0 )
        {
            const sal_Int8* pId1 = rObj1.maImpIdSeq.getConstArray();
            const sal_Int8* pId2 = rObj2.maImpIdSeq.getConstArray();
            bEqual = (memcmp( pId1, pId2, nLen1 * sizeof( sal_Int8 ) ) == 0 );
        }
        return bEqual;
    }
};

size_t TypeProviderAccessCache_Impl::operator()(const hashTypeProviderKey_Impl & rObj ) const
{
    const sal_Int32* pBytesAsInt32Array = (const sal_Int32*)rObj.maImpIdSeq.getConstArray();
    sal_Int32 nLen = rObj.maImpIdSeq.getLength();
    sal_Int32 nCount32 = nLen / 4;
    sal_Int32 nMod32 = nLen % 4;

    // XOR with full 32 bit values
    sal_Int32 nId32 = 0;
    sal_Int32 i;
    for( i = 0 ; i < nCount32 ; i++ )
        nId32 ^= *(pBytesAsInt32Array++);

    // XOR with remaining byte values
    if( nMod32 )
    {
        const sal_Int8* pBytes = (const sal_Int8*)pBytesAsInt32Array;
        sal_Int8* pInt8_Id32 = (sal_Int8*)&nId32;
        for( i = 0 ; i < nMod32 ; i++ )
            *(pInt8_Id32++) ^= *(pBytes++);
    }

    return (size_t)nId32;
}


typedef boost::unordered_map
<
    hashTypeProviderKey_Impl,
    rtl::Reference< IntrospectionAccessStatic_Impl >,
    TypeProviderAccessCache_Impl,
    TypeProviderAccessCache_Impl
>
TypeProviderAccessCacheMap;

//*************************
//*** ImplIntrospection ***
//*************************

struct OIntrospectionMutex
{
    Mutex                            m_mutex;
};

class ImplIntrospection : public XIntrospection
                        , public XServiceInfo
                        , public OIntrospectionMutex
                        , public OComponentHelper
{
    // Implementation der Introspection.
    rtl::Reference< IntrospectionAccessStatic_Impl > implInspect(const Any& aToInspectObj);

    // Save XMultiServiceFactory from createComponent
    Reference<XMultiServiceFactory> m_xSMgr;

    // CoreReflection halten
    Reference< XIdlReflection > mxCoreReflection;

    // Klassen, deren Methoden eine spezielle Rolle spielen
    Reference<XIdlClass> mxElementAccessClass;
    Reference<XIdlClass> mxNameContainerClass;
    Reference<XIdlClass> mxNameAccessClass;
    Reference<XIdlClass> mxIndexContainerClass;
    Reference<XIdlClass> mxIndexAccessClass;
    Reference<XIdlClass> mxEnumerationAccessClass;
    Reference<XIdlClass> mxInterfaceClass;
    Reference<XIdlClass> mxAggregationClass;
    sal_Bool mbDisposed;

    sal_uInt16 mnCacheEntryCount;
    sal_uInt16 mnTPCacheEntryCount;
    IntrospectionAccessCacheMap* mpCache;
    TypeProviderAccessCacheMap* mpTypeProviderCache;

public:
    ImplIntrospection( const Reference<XMultiServiceFactory> & rXSMgr );

    // Methoden von XInterface
    virtual Any            SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw() { OComponentHelper::acquire(); }
    virtual void        SAL_CALL release() throw() { OComponentHelper::release(); }

    // XTypeProvider
    Sequence< Type >    SAL_CALL getTypes(  ) throw( RuntimeException );
    Sequence<sal_Int8>    SAL_CALL getImplementationId(  ) throw( RuntimeException );

    // XServiceInfo
    ::rtl::OUString                     SAL_CALL getImplementationName() throw();
    sal_Bool                    SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw();
    Sequence< ::rtl::OUString >         SAL_CALL getSupportedServiceNames(void) throw();
    static ::rtl::OUString SAL_CALL    getImplementationName_Static(  );
    static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(void) throw();

    // Methoden von XIntrospection
    virtual Reference<XIntrospectionAccess> SAL_CALL inspect(const Any& aToInspectObj)
                throw( RuntimeException );

protected:
    // some XComponent part from OComponentHelper
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
};

enum MethodType
{
    STANDARD_METHOD,            // normale Methode, kein Bezug zu Properties oder Listenern
    GETSET_METHOD,                // gehoert zu einer get/set-Property
    ADD_LISTENER_METHOD,        // add-Methode einer Listener-Schnittstelle
    REMOVE_LISTENER_METHOD,        // remove-Methode einer Listener-Schnittstelle
    INVALID_METHOD                // Methode, deren Klasse nicht beruecksichtigt wird, z.B. XPropertySet
};

// Ctor
ImplIntrospection::ImplIntrospection( const Reference<XMultiServiceFactory> & rXSMgr )
    : OComponentHelper( m_mutex )
    , m_xSMgr( rXSMgr )
{
    mnCacheEntryCount = 0;
    mnTPCacheEntryCount = 0;
    mpCache = NULL;
    mpTypeProviderCache = NULL;

    // Spezielle Klassen holen
//     Reference< XInterface > xServiceIface = m_xSMgr->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.CoreReflection")) );
//     if( xServiceIface.is() )
//         mxCoreReflection = Reference< XIdlReflection >::query( xServiceIface );
    Reference< XPropertySet > xProps( rXSMgr, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    if (xProps.is())
    {
        Reference< XComponentContext > xContext;
        xProps->getPropertyValue(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xContext;
        OSL_ASSERT( xContext.is() );
        if (xContext.is())
        {
            xContext->getValueByName(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection") ) ) >>= mxCoreReflection;
            OSL_ENSURE( mxCoreReflection.is(), "### CoreReflection singleton not accessible!?" );
        }
    }
    if (! mxCoreReflection.is())
    {
        throw DeploymentException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection singleton not accessible") ),
            Reference< XInterface >() );
    }

    mxElementAccessClass = mxCoreReflection->forName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.XElementAccess")) );
    mxNameContainerClass = mxCoreReflection->forName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.XNameContainer")) );
    mxNameAccessClass = mxCoreReflection->forName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.XNameAccess")) );
    mxIndexContainerClass = mxCoreReflection->forName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.XIndexContainer")) );
    mxIndexAccessClass = mxCoreReflection->forName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.XIndexAccess")) );
    mxEnumerationAccessClass = mxCoreReflection->forName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.XEnumerationAccess")) );
    mxInterfaceClass = mxCoreReflection->forName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface")) );
    mxAggregationClass = mxCoreReflection->forName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XAggregation")) );
    mbDisposed = sal_False;
}

// XComponent
void ImplIntrospection::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    OComponentHelper::dispose();

    // Cache loeschen
    delete mpCache;
    mpCache = NULL;
    delete mpTypeProviderCache;
    mpTypeProviderCache = NULL;

    mxElementAccessClass = NULL;
    mxNameContainerClass = NULL;
    mxNameAccessClass = NULL;
    mxIndexContainerClass = NULL;
    mxIndexAccessClass = NULL;
    mxEnumerationAccessClass = NULL;
    mxInterfaceClass = NULL;
    mxAggregationClass = NULL;
    mbDisposed = sal_True;
}


//-----------------------------------------------------------------------------

// XInterface
Any ImplIntrospection::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface(
        rType,
        static_cast< XIntrospection * >( this ),
        static_cast< XServiceInfo * >( this ) ) );

    return (aRet.hasValue() ? aRet : OComponentHelper::queryInterface( rType ));
}

// XTypeProvider
Sequence< Type > ImplIntrospection::getTypes()
    throw( RuntimeException )
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XIntrospection > *)0 ),
                ::getCppuType( (const Reference< XServiceInfo > *)0 ),
                OComponentHelper::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}

Sequence< sal_Int8 > ImplIntrospection::getImplementationId()
    throw( RuntimeException )
{
    static OImplementationId * s_pId = 0;
    if (! s_pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pId)
        {
            static OImplementationId s_aId;
            s_pId = &s_aId;
        }
    }
    return s_pId->getImplementationId();
}


// XServiceInfo
::rtl::OUString ImplIntrospection::getImplementationName() throw()
{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool ImplIntrospection::supportsService(const ::rtl::OUString& ServiceName) throw()
{
    Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// XServiceInfo
Sequence< ::rtl::OUString > ImplIntrospection::getSupportedServiceNames(void) throw()
{
    return getSupportedServiceNames_Static();
}

//*************************************************************************
// Helper XServiceInfo
::rtl::OUString ImplIntrospection::getImplementationName_Static(  )
{
    return ::rtl::OUString::createFromAscii( IMPLEMENTATION_NAME );
}

// ORegistryServiceManager_Static
Sequence< ::rtl::OUString > ImplIntrospection::getSupportedServiceNames_Static(void) throw()
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii( SERVICE_NAME );
    return aSNS;
}

//*************************************************************************

// Methoden von XIntrospection
Reference<XIntrospectionAccess> ImplIntrospection::inspect(const Any& aToInspectObj)
    throw( RuntimeException )
{
    Reference<XIntrospectionAccess> xAccess;

    if ( aToInspectObj.getValueType().getTypeClass() == TypeClass_TYPE )
    {
        Type aType;
        aToInspectObj >>= aType;

        Reference< XIdlClass > xIdlClass = mxCoreReflection->forName(((Type*)(aToInspectObj.getValue()))->getTypeName());

        if ( xIdlClass.is() )
        {
            Any aRealInspectObj;
            aRealInspectObj <<= xIdlClass;

            rtl::Reference< IntrospectionAccessStatic_Impl > pStaticImpl( implInspect( aRealInspectObj ) );
            if( pStaticImpl.is() )
                xAccess = new ImplIntrospectionAccess( aRealInspectObj, pStaticImpl );
        }
    }
    else
    {
        rtl::Reference< IntrospectionAccessStatic_Impl > pStaticImpl( implInspect( aToInspectObj ) );
        if( pStaticImpl.is() )
            xAccess = new ImplIntrospectionAccess( aToInspectObj, pStaticImpl );
    }

    return xAccess;
}

//-----------------------------------------------------------------------------

// Hashtable fuer Pruefung auf mehrfache Beruecksichtigung von Interfaces
struct hashInterface_Impl
{
    size_t operator()(const void* p) const
    {
        return (size_t)p;
    }
};

struct eqInterface_Impl
{
    bool operator()(const void* p1, const void* p2) const
    {
        return ( p1 == p2 );
    }
};

typedef boost::unordered_map
<
    void*,
    void*,
    hashInterface_Impl,
    eqInterface_Impl
>
CheckedInterfacesMap;



// TODO: Spaeter auslagern
Reference<XIdlClass> TypeToIdlClass( const Type& rType, const Reference< XMultiServiceFactory > & xMgr )
{
    static Reference< XIdlReflection > xRefl;

    // void als Default-Klasse eintragen
    Reference<XIdlClass> xRetClass;
    typelib_TypeDescription * pTD = 0;
    rType.getDescription( &pTD );
    if( pTD )
    {
        ::rtl::OUString sOWName( pTD->pTypeName );
        if( !xRefl.is() )
        {
            xRefl = Reference< XIdlReflection >( xMgr->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.CoreReflection")) ), UNO_QUERY );
            OSL_ENSURE( xRefl.is(), "### no corereflection!" );
        }
        xRetClass = xRefl->forName( sOWName );
    }
    return xRetClass;
}

// Implementation der Introspection.
rtl::Reference< IntrospectionAccessStatic_Impl > ImplIntrospection::implInspect(const Any& aToInspectObj)
{
    MutexGuard aGuard( m_mutex );

    // Wenn die Introspection schon disposed ist, wird nur ein leeres Ergebnis geliefert
    if( mbDisposed )
        return NULL;

    // Objekt untersuchen
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE && eType != TypeClass_STRUCT  && eType != TypeClass_EXCEPTION )
        return NULL;

    Reference<XInterface> x;
    if( eType == TypeClass_INTERFACE )
    {
        // Interface aus dem Any besorgen
        x = *(Reference<XInterface>*)aToInspectObj.getValue();
        if( !x.is() )
            return NULL;
    }

    // Haben wir schon eine Cache-Instanz
    if( !mpCache )
        mpCache = new IntrospectionAccessCacheMap;
    if( !mpTypeProviderCache )
        mpTypeProviderCache = new TypeProviderAccessCacheMap;
    IntrospectionAccessCacheMap& aCache = *mpCache;
    TypeProviderAccessCacheMap& aTPCache = *mpTypeProviderCache;

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
                    pClasses[ i ] = TypeToIdlClass( pTypes[ i ], m_xSMgr );
                }
                // TODO: Caching!
            }
        }
        else
        {
            xImplClass = TypeToIdlClass( aToInspectObj.getValueType(), m_xSMgr );
            SupportedClassSeq.realloc( 1 );
            SupportedClassSeq.getArray()[ 0 ] = xImplClass;
        }

        xPropSet = Reference<XPropertySet>::query( x );
        // Jetzt versuchen, das PropertySetInfo zu bekommen
        if( xPropSet.is() )
            xPropSetInfo = xPropSet->getPropertySetInfo();
    }
    else
    {
        xImplClass = TypeToIdlClass( aToInspectObj.getValueType(), m_xSMgr );
    }

    if( xTypeProvider.is() )
    {
        Sequence< sal_Int8 > aImpIdSeq = xTypeProvider->getImplementationId();
        sal_Int32 nIdLen = aImpIdSeq.getLength();

        if( nIdLen )
        {
            // cache only, if the descriptor class is set
            hashTypeProviderKey_Impl aKeySeq( xPropSetInfo, aImpIdSeq );

            TypeProviderAccessCacheMap::iterator aIt = aTPCache.find( aKeySeq );
            if( aIt == aTPCache.end() )
            {
                // not found
                // Neue Instanz anlegen und unter dem gegebenen Key einfuegen
                pAccess = new IntrospectionAccessStatic_Impl( mxCoreReflection );

                // Groesse begrenzen, alten Eintrag wieder rausschmeissen
                if( mnTPCacheEntryCount > INTROSPECTION_CACHE_MAX_SIZE )
                {
                    // Access mit dem kleinsten HitCount suchen
                    TypeProviderAccessCacheMap::iterator iter = aTPCache.begin();
                    TypeProviderAccessCacheMap::iterator end = aTPCache.end();
                    TypeProviderAccessCacheMap::iterator toDelete = iter;
                    while( iter != end )
                    {
                        if( (*iter).first.nHitCount < (*toDelete).first.nHitCount )
                            toDelete = iter;
                        ++iter;
                    }
                    aTPCache.erase( toDelete );
                }
                else
                    mnTPCacheEntryCount++;

                // Neuer Eintrage rein in die Table
                aKeySeq.nHitCount = 1;
                aTPCache[ aKeySeq ] = pAccess;

            }
            else
            {
                // Hit-Count erhoehen
                (*aIt).first.IncHitCount();
                return (*aIt).second;
            }
        }
    }
    else if( xImplClass.is() )
    {
        // cache only, if the descriptor class is set
        hashIntrospectionKey_Impl    aKeySeq( SupportedClassSeq, xPropSetInfo, xImplClass );

        IntrospectionAccessCacheMap::iterator aIt = aCache.find( aKeySeq );
        if( aIt == aCache.end() )
        {
            // not found
            // Neue Instanz anlegen und unter dem gegebenen Key einfuegen
            pAccess = new IntrospectionAccessStatic_Impl( mxCoreReflection );

            // Groesse begrenzen, alten Eintrag wieder rausschmeissen
            if( mnCacheEntryCount > INTROSPECTION_CACHE_MAX_SIZE )
            {
                // Access mit dem kleinsten HitCount suchen
                IntrospectionAccessCacheMap::iterator iter = aCache.begin();
                IntrospectionAccessCacheMap::iterator end = aCache.end();
                IntrospectionAccessCacheMap::iterator toDelete = iter;
                while( iter != end )
                {
                    if( (*iter).first.nHitCount < (*toDelete).first.nHitCount )
                        toDelete = iter;
                    ++iter;
                }
                aCache.erase( toDelete );
            }
            else
                mnCacheEntryCount++;

            // Neuer Eintrage rein in die Table
            aKeySeq.nHitCount = 1;
            aCache[ aKeySeq ] = pAccess;

        }
        else
        {
            // Hit-Count erhoehen
            (*aIt).first.IncHitCount();
            return (*aIt).second;
        }
    }

    // Kein Access gecached -> neu anlegen
    Property* pAllPropArray;
    Reference<XInterface>* pInterfaces1;
    Reference<XInterface>* pInterfaces2;
    sal_Int16* pMapTypeArray;
    sal_Int32* pPropertyConceptArray;
    sal_Int32 i;

    if( !pAccess.is() )
        pAccess = new IntrospectionAccessStatic_Impl( mxCoreReflection );

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

    //*************************
    //*** Analyse vornehmen ***
    //*************************
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
                ::rtl::OUString aPropName = rProp.Name;

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
                    OSL_FAIL(
                        ( ::rtl::OString( "Introspection: Property \"" ) +
                        ::rtl::OUStringToOString( aPropName, RTL_TEXTENCODING_UTF8 ) +
                        ::rtl::OString( "\" found more than once in PropertySet" ) ).getStr() );
                }

                // Count pflegen
                rPropCount++;
            }
        }

        // Indizes in die Export-Tabellen
        sal_Int32 iAllExportedMethod = 0;
        sal_Int32 iAllSupportedListener = 0;

        // Hashtable fuer Pruefung auf mehrfache Beruecksichtigung von Interfaces
        CheckedInterfacesMap aCheckedInterfacesMap;

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

                    // Pruefen, ob das Interface schon beruecksichtigt wurde.
                    XInterface* pIface = ( static_cast< XInterface* >( rxIfaceClass.get() ) );
                    if( aCheckedInterfacesMap.count( pIface ) > 0 )
                    {
                        // Kennen wir schon
                        continue;
                    }
                    else
                    {
                        // Sonst eintragen
                        aCheckedInterfacesMap[ pIface ] = pIface;
                    }

                    //********************************************************************

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
                        ::rtl::OUString aFieldName = xField->getName();
                        rProp.Name = aFieldName;
                        rProp.Handle = rPropCount;
                        Type aFieldType( xPropType->getTypeClass(), xPropType->getName() );
                        rProp.Type = aFieldType;
                        FieldAccessMode eAccessMode = xField->getAccessMode();
                        rProp.Attributes = (eAccessMode == FieldAccessMode_READONLY ||
                                            eAccessMode == FieldAccessMode_CONST)
                            ? READONLY : 0;

                        // Namen in Hashtable eintragen
                        ::rtl::OUString aPropName = rProp.Name;

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

                    //********************************************************************

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
                    MethodType* pMethodTypes = new MethodType[ nSourceMethodCount ];
                    sal_Int32* pLocalMethodConcepts = new sal_Int32[ nSourceMethodCount ];
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        pMethodTypes[ i ] = STANDARD_METHOD;
                        pLocalMethodConcepts[ i ] = 0;
                    }

                    ::rtl::OUString aMethName;
                    ::rtl::OUString aPropName;
                    ::rtl::OUString aStartStr;
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
                        if( rxMethod_i->getDeclaringClass()->equals( mxInterfaceClass ) )
                        {
                            // XInterface-Methoden sind hiermit einmal beruecksichtigt
                            bFoundXInterface = sal_True;

                            if( bXInterfaceIsInvalid )
                            {
                                pMethodTypes[ i ] = INVALID_METHOD;
                                continue;
                            }
                            else
                            {
                                if( aMethName != ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("queryInterface")) )
                                {
                                    rMethodConcept_i |= MethodConcept::DANGEROUS;
                                    continue;
                                }
                            }
                        }
                        else if( rxMethod_i->getDeclaringClass()->equals( mxAggregationClass ) )
                        {
                            if( aMethName == "setDelegator" )
                            {
                                rMethodConcept_i |= MethodConcept::DANGEROUS;
                                continue;
                            }
                        }
                        else if( rxMethod_i->getDeclaringClass()->equals( mxElementAccessClass ) )
                        {
                            rMethodConcept_i |= ( NAMECONTAINER  |
                                                  INDEXCONTAINER |
                                                  ENUMERATION );
                        }
                        else if( rxMethod_i->getDeclaringClass()->equals( mxNameContainerClass ) ||
                                 rxMethod_i->getDeclaringClass()->equals( mxNameAccessClass ) )
                        {
                            rMethodConcept_i |= NAMECONTAINER;
                        }
                        else if( rxMethod_i->getDeclaringClass()->equals( mxIndexContainerClass ) ||
                                 rxMethod_i->getDeclaringClass()->equals( mxIndexAccessClass ) )
                        {
                            rMethodConcept_i |= INDEXCONTAINER;
                        }
                        else if( rxMethod_i->getDeclaringClass()->equals( mxEnumerationAccessClass ) )
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
                                ::rtl::OUString aMethName2 = rxMethod_k->getName();
                                ::rtl::OUString aStartStr2 = aMethName2.copy( 0, 3 );
                                // ACHTUNG: Wegen SDL-Bug NICHT != bei ::rtl::OUString verwenden !!!
                                if( !( aStartStr2 == "set" ) )
                                    continue;

                                // Ist es denn der gleiche Name?
                                ::rtl::OUString aPropName2 = aMethName2.copy( 3 );
                                // ACHTUNG: Wegen SDL-Bug NICHT != bei ::rtl::OUString verwenden !!!
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
                            ::rtl::OUString aListenerStr( "Listener" );

                            // Namen der potentiellen Property
                            sal_Int32 nStrLen = aMethName.getLength();
                            sal_Int32 nCopyLen = nStrLen - aListenerStr.getLength();
                            ::rtl::OUString aEndStr = aMethName.copy( nCopyLen > 0 ? nCopyLen : 0 );

                            // Endet das Teil auf Listener?
                            // ACHTUNG: Wegen SDL-Bug NICHT != bei ::rtl::OUString verwenden !!!
                            if( !( aEndStr == aListenerStr ) )
                                continue;

                            // Welcher Listener?
                            ::rtl::OUString aListenerName = aMethName.copy( 3, nStrLen - aListenerStr.getLength() - 3 );

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
                                ::rtl::OUString aMethName2 = rxMethod_k->getName();
                                sal_Int32 nNameLen = aMethName2.getLength();
                                sal_Int32 nCopyLen2 = (nNameLen < 6) ? nNameLen : 6;
                                ::rtl::OUString aStartStr2 = aMethName2.copy( 0, nCopyLen2 );
                                ::rtl::OUString aRemoveStr( RTL_CONSTASCII_USTRINGPARAM("remove" ) );
                                // ACHTUNG: Wegen SDL-Bug NICHT != bei ::rtl::OUString verwenden !!!
                                if( !( aStartStr2 == aRemoveStr ) )
                                    continue;

                                // Ist es denn der gleiche Listener?
                                if( aMethName2.getLength() - aRemoveStr.getLength() <= aListenerStr.getLength() )
                                    continue;
                                ::rtl::OUString aListenerName2 = aMethName2.copy
                                      ( 6, aMethName2.getLength() - aRemoveStr.getLength() - aListenerStr.getLength() );
                                // ACHTUNG: Wegen SDL-Bug NICHT != bei ::rtl::OUString verwenden !!!
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


                    //********************************************************************

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
                            ::rtl::OUString aMethName2 = rxMethod->getName();
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
                            Reference<XIdlClass> xListenerClass = TypeToIdlClass( getCppuVoidType(), m_xSMgr );
                            // ALT: Reference<XIdlClass> xListenerClass = Void_getReflection()->getIdlClass();

                            // 1. Moeglichkeit: Parameter nach einer Listener-Klasse durchsuchen
                            // Nachteil: Superklassen muessen rekursiv durchsucht werden
                            Sequence< Reference<XIdlClass> > aParams = rxMethod->getParameterTypes();
                            const Reference<XIdlClass>* pParamArray2 = aParams.getConstArray();

                            Reference<XIdlClass> xEventListenerClass = TypeToIdlClass( getCppuType( (Reference<XEventListener>*) NULL ), m_xSMgr );
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
        Reference<XIdlClass> xClassRef = TypeToIdlClass( aToInspectObj.getValueType(), m_xSMgr );
        if( !xClassRef.is() )
        {
            OSL_FAIL( "Can't get XIdlClass from Reflection" );
            return pAccess;
        }

        // Felder holen
        Sequence< Reference<XIdlField> > fields = xClassRef->getFields();
        const Reference<XIdlField>* pFields = fields.getConstArray();
        sal_Int32 nLen = fields.getLength();

        for( i = 0 ; i < nLen ; i++ )
        {
            Reference<XIdlField> xField = pFields[i];
            Reference<XIdlClass> xPropType = xField->getType();
            ::rtl::OUString aPropName = xField->getName();

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

    return pAccess;
}

//*************************************************************************
Reference< XInterface > SAL_CALL ImplIntrospection_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr )
    throw( RuntimeException )
{
    Reference< XInterface > xService = (OWeakObject*)(OComponentHelper*)new ImplIntrospection( rSMgr );
    return xService;
}

}

#ifdef DISABLE_DYNLOADING
#define component_getFactory introspection_component_getFactory
#endif

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager,
    SAL_UNUSED_PARAMETER void * )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            ::rtl::OUString::createFromAscii( pImplName ),
            stoc_inspect::ImplIntrospection_CreateInstance,
            stoc_inspect::ImplIntrospection::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
