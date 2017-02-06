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
#include <memory>
#include <set>

#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <sal/log.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/NoSuchMethodException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
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
#include <unordered_map>

using namespace css::uno;
using namespace css::lang;
using namespace css::reflection;
using namespace css::container;
using namespace css::registry;
using namespace css::beans;
using namespace css::beans::PropertyAttribute;
using namespace css::beans::PropertyConcept;
using namespace css::beans::MethodConcept;
using namespace cppu;
using namespace osl;

namespace
{

typedef WeakImplHelper< XIntrospectionAccess, XMaterialHolder, XExactName,
                        XPropertySet, XFastPropertySet, XPropertySetInfo,
                        XNameContainer, XIndexContainer, XEnumerationAccess,
                        XIdlArray, XUnoTunnel > IntrospectionAccessHelper;


// Special value for Method-Concept, to be able to mark "normal" functions
#define  MethodConcept_NORMAL_IMPL        0x80000000


// Method to assert, if a class is derived from another class
bool isDerivedFrom( const Reference<XIdlClass>& xToTestClass, const Reference<XIdlClass>& xDerivedFromClass )
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
            return true;
    }

    return false;
}


// *** Classification of Properties (no enum, to be able to use Sequence) ***
// Properties from a PropertySet-Interface
#define MAP_PROPERTY_SET    0
// Properties from Fields
#define MAP_FIELD            1
// Properties that get described with get/set methods
#define MAP_GETSET            2
// Properties with only a set method
#define MAP_SETONLY            3


// Increments by which the size of sequences get adjusted
#define ARRAY_SIZE_STEP        20


//*** IntrospectionAccessStatic_Impl ***

// Equals to the old IntrospectionAccessImpl, forms now a static
// part of the new Instance-related ImplIntrospectionAccess

// Hashtable for the search of names
typedef std::unordered_map
<
    OUString,
    sal_Int32,
    OUStringHash
>
IntrospectionNameMap;


// Hashtable to assign exact names to the Lower-Case
// converted names, for the support of XExactName
typedef std::unordered_map
<
    OUString,
    OUString,
    OUStringHash
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
    std::vector< Reference<XInterface> > aInterfaceSeq1;
    std::vector< Reference<XInterface> > aInterfaceSeq2;

    // Hashtables for names
    IntrospectionNameMap maPropertyNameMap;
    IntrospectionNameMap maMethodNameMap;
    LowerToExactNameMap  maLowerToExactNameMap;

    // Vector of all Properties, also for delivering from getProperties()
    std::vector<Property> maAllPropertySeq;

    // Mapping of properties to Access-Types
    std::vector<sal_Int16> maMapTypeSeq;

    // Classification of found methods
    std::vector<sal_Int32> maPropertyConceptSeq;

    // Number of Properties
    sal_Int32 mnPropCount;

    // Number of Properties, which are assigned to particular concepts
    //sal_Int32 mnDangerousPropCount;
    sal_Int32 mnPropertySetPropCount;
    sal_Int32 mnAttributePropCount;
    sal_Int32 mnMethodPropCount;

    // Flags which indicate if various interfaces are present
    bool mbFastPropSet;
    bool mbElementAccess;
    bool mbNameAccess;
    bool mbNameReplace;
    bool mbNameContainer;
    bool mbIndexAccess;
    bool mbIndexReplace;
    bool mbIndexContainer;
    bool mbEnumerationAccess;
    bool mbIdlArray;
    bool mbUnoTunnel;

    // Original handles of FastPropertySets
    std::unique_ptr<sal_Int32[]> mpOrgPropertyHandleArray;

    // MethodSequence, that accepts all methods
    std::vector< Reference<XIdlMethod> > maAllMethodSeq;

    // Classification of found methods
    std::vector<sal_Int32> maMethodConceptSeq;

    // Number of methods
    sal_Int32 mnMethCount;

    // Sequence of Listener, that can be registered
    std::vector< Type > maSupportedListenerSeq;

    // Helper-methods for adjusting sizes of Sequences
    void checkPropertyArraysSize( sal_Int32 iNextIndex );
    static void checkInterfaceArraySize( std::vector< Reference<XInterface> >& rSeq, std::vector<Reference<XInterface>>& rInterfaceVec,
        sal_Int32 iNextIndex );

public:
    explicit IntrospectionAccessStatic_Impl( Reference< XIdlReflection > const & xCoreReflection_ );
    sal_Int32 getPropertyIndex( const OUString& aPropertyName ) const;
    sal_Int32 getMethodIndex( const OUString& aMethodName ) const;

    // Methods of XIntrospectionAccess (OLD, now only Impl)
    void setPropertyValue(const Any& obj, const OUString& aPropertyName, const Any& aValue) const;
//    void setPropertyValue(Any& obj, const OUString& aPropertyName, const Any& aValue) const;
    Any getPropertyValue(const Any& obj, const OUString& aPropertyName) const;
    void setPropertyValueByIndex(const Any& obj, sal_Int32 nIndex, const Any& aValue) const;
//    void setPropertyValueByIndex(Any& obj, sal_Int32 nIndex, const Any& aValue) const;
    Any getPropertyValueByIndex(const Any& obj, sal_Int32 nIndex) const;

    const std::vector<Property>& getProperties() const              { return maAllPropertySeq; }
    const std::vector< Reference<XIdlMethod> >& getMethods() const  { return maAllMethodSeq; }
    const std::vector< Type >& getSupportedListeners() const        { return maSupportedListenerSeq; }
    const std::vector<sal_Int32>& getPropertyConcepts() const       { return maPropertyConceptSeq; }
    const std::vector<sal_Int32>& getMethodConcepts() const         { return maMethodConceptSeq; }
};


// Ctor
IntrospectionAccessStatic_Impl::IntrospectionAccessStatic_Impl( Reference< XIdlReflection > const & xCoreReflection_ )
    : mxCoreReflection( xCoreReflection_ )
{
    aInterfaceSeq1.resize( ARRAY_SIZE_STEP );
    aInterfaceSeq2.resize( ARRAY_SIZE_STEP );

    // Property-Data
    maAllPropertySeq.resize( ARRAY_SIZE_STEP );
    maMapTypeSeq.resize( ARRAY_SIZE_STEP );
    maPropertyConceptSeq.resize( ARRAY_SIZE_STEP );

    mbFastPropSet = false;
    mbElementAccess = false;
    mbNameAccess = false;
    mbNameReplace = false;
    mbNameContainer = false;
    mbIndexAccess = false;
    mbIndexReplace = false;
    mbIndexContainer = false;
    mbEnumerationAccess = false;
    mbIdlArray = false;
    mbUnoTunnel = false;

    mpOrgPropertyHandleArray = nullptr;

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
    IntrospectionAccessStatic_Impl* pThis = const_cast<IntrospectionAccessStatic_Impl*>(this);
    IntrospectionNameMap::iterator aIt = pThis->maPropertyNameMap.find( aPropertyName );
    if (aIt != pThis->maPropertyNameMap.end())
        return aIt->second;

    return -1;
}

sal_Int32 IntrospectionAccessStatic_Impl::getMethodIndex( const OUString& aMethodName ) const
{
    IntrospectionAccessStatic_Impl* pThis = const_cast<IntrospectionAccessStatic_Impl*>(this);
    IntrospectionNameMap::iterator aIt = pThis->maMethodNameMap.find( aMethodName );
    if (aIt != pThis->maMethodNameMap.end())
    {
        return aIt->second;
    }

    // #95159 Check if full qualified name matches
    sal_Int32 nSearchFrom = aMethodName.getLength();
    while( true )
    {
        // Strategy: Search back until the first '_' is found
        sal_Int32 nFound = aMethodName.lastIndexOf( '_', nSearchFrom );
        if( nFound == -1 )
            break;

        OUString aPureMethodName = aMethodName.copy( nFound + 1 );

        aIt = pThis->maMethodNameMap.find( aPureMethodName );
        if (aIt != pThis->maMethodNameMap.end())
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
                const sal_Int32 iHashResult = aIt->second;

                const Reference<XIdlMethod> xMethod = maAllMethodSeq[iHashResult];

                Reference< XIdlClass > xMethClass = xMethod->getDeclaringClass();
                if( xClass->equals( xMethClass ) )
                {
                    return iHashResult;
                }
                else
                {
                    // Could also be another method with the same name
                    // Iterate over all methods
                    size_t nLen = maAllMethodSeq.size();
                    for (size_t i = 0; i < nLen; ++i)
                    {
                        const Reference<XIdlMethod> xMethod2 = maAllMethodSeq[ i ];
                        if( xMethod2->getName() == aPureMethodName )
                        {
                            Reference< XIdlClass > xMethClass2 = xMethod2->getDeclaringClass();

                            if( xClass->equals( xMethClass2 ) )
                            {
                                return i;
                            }
                        }
                    }
                }
            }
        }

        nSearchFrom = nFound - 1;
        if( nSearchFrom < 0 )
            break;
    }
    return -1;
}

void IntrospectionAccessStatic_Impl::setPropertyValue( const Any& obj, const OUString& aPropertyName, const Any& aValue ) const
//void IntrospectionAccessStatic_Impl::setPropertyValue( Any& obj, const OUString& aPropertyName, const Any& aValue ) const
{
    sal_Int32 i = getPropertyIndex( aPropertyName );
    if( i != -1 )
        setPropertyValueByIndex( obj, (sal_Int32)i, aValue );
    else
        throw UnknownPropertyException(aPropertyName);
}

void IntrospectionAccessStatic_Impl::setPropertyValueByIndex(const Any& obj, sal_Int32 nSequenceIndex, const Any& aValue) const
//void IntrospectionAccessStatic_Impl::setPropertyValueByIndex( Any& obj, sal_Int32 nSequenceIndex, const Any& aValue) const
{
    // Is the passed object something that fits?
    Reference<XInterface> xInterface;
    if( !(obj >>= xInterface) )
    {
        TypeClass eObjType = obj.getValueType().getTypeClass();
        if( nSequenceIndex >= mnPropCount)
            throw IllegalArgumentException(
                "IntrospectionAccessStatic_Impl::setPropertyValueByIndex(), index > propertyCount, " +
                OUString::number(nSequenceIndex) + " > " + OUString::number(mnPropCount),
                Reference<XInterface>(), 0);
        if( eObjType != TypeClass_STRUCT && eObjType != TypeClass_EXCEPTION )
            throw IllegalArgumentException(
                "IntrospectionAccessStatic_Impl::setPropertyValueByIndex(), expected struct or exception, got" +
                obj.getValueType().getTypeName(), Reference<XInterface>(), 0);
    }

    // Test flags
    if( (maAllPropertySeq[ nSequenceIndex ].Attributes & READONLY) != 0 )
    {
        throw UnknownPropertyException(
            "IntrospectionAccessStatic_Impl::setPropertyValueByIndex(), property at index " + OUString::number(nSequenceIndex) + " is readonly");
    }

    switch( maMapTypeSeq[ nSequenceIndex ] )
    {
        case MAP_PROPERTY_SET:
        {
            // Get Property
            const Property& rProp = maAllPropertySeq[ nSequenceIndex ];

            // Convert Interface-Parameter to the correct type
            bool bUseCopy = false;
            Any aRealValue;

            if( auto valInterface = o3tl::tryAccess<
                    css::uno::Reference<css::uno::XInterface>>(aValue) )
            {
                Type aPropType = rProp.Type;
                OUString aTypeName( aPropType.getTypeName() );
                Reference< XIdlClass > xPropClass = mxCoreReflection->forName( aTypeName );
                //Reference<XIdlClass> xPropClass = rProp.Type;
                if( xPropClass.is() && xPropClass->getTypeClass() == TypeClass_INTERFACE )
                {
                    if( valInterface->is() )
                    {
                        //Any queryInterface( const Type& rType );
                        aRealValue = (*valInterface)->queryInterface( aPropType );
                        if( aRealValue.hasValue() )
                            bUseCopy = true;
                    }
                }
            }

            // Do we have a FastPropertySet and a valid Handle?
            // CAUTION: At this point we exploit that the PropertySet
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
            Reference<XIdlField> xField = static_cast<XIdlField*>(aInterfaceSeq1[ nSequenceIndex ].get());
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
            // Fetch set method
            Reference<XIdlMethod> xMethod = static_cast<XIdlMethod*>(aInterfaceSeq2[ nSequenceIndex ].get());
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

    throw UnknownPropertyException(aPropertyName);
}

Any IntrospectionAccessStatic_Impl::getPropertyValueByIndex(const Any& obj, sal_Int32 nSequenceIndex) const
{
    Any aRet;

    // Is there anything suitable in the passed object?
    Reference<XInterface> xInterface;
    if( !(obj >>= xInterface) )
    {
        TypeClass eObjType = obj.getValueType().getTypeClass();
        if( nSequenceIndex >= mnPropCount || ( eObjType != TypeClass_STRUCT && eObjType != TypeClass_EXCEPTION ) )
        {
            // throw IllegalArgumentException();
            return aRet;
        }
    }

    switch( maMapTypeSeq[ nSequenceIndex ] )
    {
        case MAP_PROPERTY_SET:
        {
            // Acquire property
            const Property& rProp = maAllPropertySeq[ nSequenceIndex ];

            // Do we have a FastPropertySet and a valid handle?
            // NOTE: At this point is exploited that the PropertySet
            // is queried at the beginning of introspection process.
            sal_Int32 nOrgHandle;
            if( mbFastPropSet && ( nOrgHandle = mpOrgPropertyHandleArray[ nSequenceIndex ] ) != -1 )
            {
                // Fetch the PropertySet interface
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
            // Otherwise use the normal one
            else
            {
                // Fetch the PropertySet interface
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
            Reference<XIdlField> xField = static_cast<XIdlField*>(aInterfaceSeq1[ nSequenceIndex ].get());
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
            // Fetch get method
            Reference<XIdlMethod> xMethod = static_cast<XIdlMethod*>(aInterfaceSeq1[ nSequenceIndex ].get());
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
            // Get method does not exist
            // throw WriteOnlyPropertyException();
            return aRet;
    }
    return aRet;
}


// Helper method to adjust the size of the vectors
void IntrospectionAccessStatic_Impl::checkPropertyArraysSize( sal_Int32 iNextIndex )
{
    sal_Int32 nLen = (sal_Int32)maAllPropertySeq.size();
    if( iNextIndex >= nLen )
    {
        maAllPropertySeq.resize( nLen + ARRAY_SIZE_STEP );
        maMapTypeSeq.resize( nLen + ARRAY_SIZE_STEP );
        maPropertyConceptSeq.resize( nLen + ARRAY_SIZE_STEP );
    }
}

void IntrospectionAccessStatic_Impl::checkInterfaceArraySize( std::vector< Reference<XInterface> >& rSeq,
    std::vector<Reference<XInterface>>& rInterfaceVec, sal_Int32 iNextIndex )
{
    sal_Int32 nLen = rSeq.size();
    if( iNextIndex >= nLen )
    {
        // Synchronize new size with ARRAY_SIZE_STEP
        sal_Int32 nMissingSize = iNextIndex - nLen + 1;
        sal_Int32 nSteps = nMissingSize / ARRAY_SIZE_STEP + 1;
        sal_Int32 nNewSize = nLen + nSteps * ARRAY_SIZE_STEP;

        rSeq.resize( nNewSize );
        rInterfaceVec = rSeq;
    }
}


//*** ImplIntrospectionAccess ***


// New Impl class as part of the introspection conversion to instance-bound
// Introspection with property access via XPropertySet. The old class
// ImplIntrospectionAccess lives on as IntrospectionAccessStatic_Impl
class ImplIntrospectionAccess : public IntrospectionAccessHelper
{
    friend class Implementation;

    // Object under examination
    Any maInspectedObject;

    // As interface
    Reference<XInterface> mxIface;

    // Static introspection data
    rtl::Reference< IntrospectionAccessStatic_Impl > mpStaticImpl;

    // Last Sequence that came with getProperties (optimization)
    Sequence<Property> maLastPropertySeq;
    sal_Int32 mnLastPropertyConcept;

    // Last Sequence that came with getMethods (optimization)
    Sequence<Reference<XIdlMethod> > maLastMethodSeq;
    sal_Int32 mnLastMethodConcept;

    // Guards the caching of queried interfaces
    osl::Mutex m_aMutex;

    // Original interfaces of the objects
    Reference<XElementAccess>       mxObjElementAccess;
    Reference<XNameContainer>       mxObjNameContainer;
    Reference<XNameReplace>         mxObjNameReplace;
    Reference<XNameAccess>          mxObjNameAccess;
    Reference<XIndexContainer>      mxObjIndexContainer;
    Reference<XIndexReplace>        mxObjIndexReplace;
    Reference<XIndexAccess>         mxObjIndexAccess;
    Reference<XEnumerationAccess>   mxObjEnumerationAccess;
    Reference<XIdlArray>            mxObjIdlArray;

    Reference<XElementAccess>       getXElementAccess();
    Reference<XNameContainer>       getXNameContainer();
    Reference<XNameReplace>         getXNameReplace();
    Reference<XNameAccess>          getXNameAccess();
    Reference<XIndexContainer>      getXIndexContainer();
    Reference<XIndexReplace>        getXIndexReplace();
    Reference<XIndexAccess>         getXIndexAccess();
    Reference<XEnumerationAccess>   getXEnumerationAccess();
    Reference<XIdlArray>            getXIdlArray();

    void cacheXNameContainer();
    void cacheXIndexContainer();

public:
    ImplIntrospectionAccess( const Any& obj, rtl::Reference< IntrospectionAccessStatic_Impl > const & pStaticImpl_ );

    // Methods from XIntrospectionAccess
    virtual sal_Int32 SAL_CALL getSuppliedMethodConcepts() override;
    virtual sal_Int32 SAL_CALL getSuppliedPropertyConcepts() override;
    virtual Property SAL_CALL getProperty(const OUString& Name, sal_Int32 PropertyConcepts) override;
    virtual sal_Bool SAL_CALL hasProperty(const OUString& Name, sal_Int32 PropertyConcepts) override;
    virtual Sequence< Property > SAL_CALL getProperties(sal_Int32 PropertyConcepts) override;
    virtual Reference<XIdlMethod> SAL_CALL getMethod(const OUString& Name, sal_Int32 MethodConcepts) override;
    virtual sal_Bool SAL_CALL hasMethod(const OUString& Name, sal_Int32 MethodConcepts) override;
    virtual Sequence< Reference<XIdlMethod> > SAL_CALL getMethods(sal_Int32 MethodConcepts) override;
    virtual Sequence< Type > SAL_CALL getSupportedListeners() override;
    using OWeakObject::queryAdapter;
    virtual Reference<XInterface> SAL_CALL queryAdapter( const Type& rType ) override;

    // Methods from XMaterialHolder
    virtual Any SAL_CALL getMaterial() override;

    // Methods from XExactName
    virtual OUString SAL_CALL getExactName( const OUString& rApproximateName ) override;

    // Methods from XInterface
    virtual Any SAL_CALL queryInterface( const Type& rType ) override;
    virtual void        SAL_CALL acquire() throw() override { OWeakObject::acquire(); }
    virtual void        SAL_CALL release() throw() override { OWeakObject::release(); }

    // Methods from XPropertySet
    virtual Reference<XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const Any& aValue) override;
    virtual Any SAL_CALL getPropertyValue(const OUString& aPropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener) override;

    // Methods from XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const Any& aValue) override;
    virtual Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) override;

    // Methods from XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties() override;
    virtual Property SAL_CALL getPropertyByName(const OUString& Name) override;
    virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& Name) override;

    // Methods from XElementAccess
    virtual Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // Methods from XNameAccess
    virtual Any SAL_CALL getByName(const OUString& Name) override;
    virtual Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    // Methods from XNameReplace
    virtual void SAL_CALL replaceByName(const OUString& Name, const Any& Element) override;

    // Methods from XNameContainer
    virtual void SAL_CALL insertByName(const OUString& Name, const Any& Element) override;
    virtual void SAL_CALL removeByName(const OUString& Name) override;

    // Methods from XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual Any SAL_CALL getByIndex(sal_Int32 Index) override;

    // Methods from XIndexReplace
    virtual void SAL_CALL replaceByIndex(sal_Int32 Index, const Any& Element) override;

    // Methods from XIndexContainer
    virtual void SAL_CALL insertByIndex(sal_Int32 Index, const Any& Element) override;
    virtual void SAL_CALL removeByIndex(sal_Int32 Index) override;

    // Methods from XEnumerationAccess
    virtual Reference<XEnumeration> SAL_CALL createEnumeration() override;

    // Methods from XIdlArray
    virtual void SAL_CALL realloc(Any& array, sal_Int32 length) override;
    virtual sal_Int32 SAL_CALL getLen(const Any& array) override;
    virtual Any SAL_CALL get(const Any& array, sal_Int32 index) override;
    virtual void SAL_CALL set(Any& array, sal_Int32 index, const Any& value) override;

    // Methods from XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const Sequence< sal_Int8 >& aIdentifier ) override;
};

ImplIntrospectionAccess::ImplIntrospectionAccess
    ( const Any& obj, rtl::Reference< IntrospectionAccessStatic_Impl > const & pStaticImpl_ )
        : maInspectedObject( obj ), mpStaticImpl( pStaticImpl_ ) //, maAdapter()
{
    // Save object as an interface if possible
    maInspectedObject >>= mxIface;

    mnLastPropertyConcept = -1;
    mnLastMethodConcept = -1;
}

Reference<XElementAccess> ImplIntrospectionAccess::getXElementAccess()
{
    ResettableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjElementAccess.is() )
    {
        aGuard.clear();
        Reference<XElementAccess> xElementAccess( mxIface, UNO_QUERY );
        aGuard.reset();
        if( !mxObjElementAccess.is() )
            mxObjElementAccess = xElementAccess;
    }
    return mxObjElementAccess;
}

void ImplIntrospectionAccess::cacheXNameContainer()
{
    Reference<XNameContainer> xNameContainer;
    Reference<XNameReplace> xNameReplace;
    Reference<XNameAccess> xNameAccess;
    if (mpStaticImpl->mbNameContainer)
    {
        xNameContainer.set( mxIface, UNO_QUERY );
        xNameReplace.set( xNameContainer, UNO_QUERY );
        xNameAccess.set( xNameContainer, UNO_QUERY );
    }
    else if (mpStaticImpl->mbNameReplace)
    {
        xNameReplace.set( mxIface, UNO_QUERY );
        xNameAccess.set( xNameReplace, UNO_QUERY );
    }
    else if (mpStaticImpl->mbNameAccess)
    {
        xNameAccess.set( mxIface, UNO_QUERY );
    }

    {
        MutexGuard aGuard( m_aMutex );
        if( !mxObjNameContainer.is() )
            mxObjNameContainer = xNameContainer;
        if( !mxObjNameReplace.is() )
            mxObjNameReplace = xNameReplace;
        if( !mxObjNameAccess.is() )
            mxObjNameAccess = xNameAccess;
    }
}

Reference<XNameContainer> ImplIntrospectionAccess::getXNameContainer()
{
    ClearableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjNameContainer.is() )
    {
        aGuard.clear();
        cacheXNameContainer();
    }
    return mxObjNameContainer;
}

Reference<XNameReplace> ImplIntrospectionAccess::getXNameReplace()
{
    ClearableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjNameReplace.is() )
    {
        aGuard.clear();
        cacheXNameContainer();
    }
    return mxObjNameReplace;
}

Reference<XNameAccess> ImplIntrospectionAccess::getXNameAccess()
{
    ClearableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjNameAccess.is() )
    {
        aGuard.clear();
        cacheXNameContainer();
    }
    return mxObjNameAccess;
}

void ImplIntrospectionAccess::cacheXIndexContainer()
{
    Reference<XIndexContainer> xIndexContainer;
    Reference<XIndexReplace> xIndexReplace;
    Reference<XIndexAccess> xIndexAccess;
    if (mpStaticImpl->mbIndexContainer)
    {
        xIndexContainer.set( mxIface, UNO_QUERY );
        xIndexReplace.set( xIndexContainer, UNO_QUERY );
        xIndexAccess.set( xIndexContainer, UNO_QUERY );
    }
    else if (mpStaticImpl->mbIndexReplace)
    {
        xIndexReplace.set( mxIface, UNO_QUERY );
        xIndexAccess.set( xIndexReplace, UNO_QUERY );
    }
    else if (mpStaticImpl->mbIndexAccess)
    {
        xIndexAccess.set( mxIface, UNO_QUERY );
    }

    {
        MutexGuard aGuard( m_aMutex );
        if( !mxObjIndexContainer.is() )
            mxObjIndexContainer = xIndexContainer;
        if( !mxObjIndexReplace.is() )
            mxObjIndexReplace = xIndexReplace;
        if( !mxObjIndexAccess.is() )
            mxObjIndexAccess = xIndexAccess;
    }
}

Reference<XIndexContainer> ImplIntrospectionAccess::getXIndexContainer()
{
    ClearableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjIndexContainer.is() )
    {
        aGuard.clear();
        cacheXIndexContainer();
    }
    return mxObjIndexContainer;
}

Reference<XIndexReplace> ImplIntrospectionAccess::getXIndexReplace()
{
    ClearableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjIndexReplace.is() )
    {
        aGuard.clear();
        cacheXIndexContainer();
    }
    return mxObjIndexReplace;
}

Reference<XIndexAccess> ImplIntrospectionAccess::getXIndexAccess()
{
    ClearableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjIndexAccess.is() )
    {
        aGuard.clear();
        cacheXIndexContainer();
    }
    return mxObjIndexAccess;
}

Reference<XEnumerationAccess> ImplIntrospectionAccess::getXEnumerationAccess()
{
    ResettableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjEnumerationAccess.is() )
    {
        aGuard.clear();
        Reference<XEnumerationAccess> xEnumerationAccess( mxIface, UNO_QUERY );
        aGuard.reset();
        if( !mxObjEnumerationAccess.is() )
            mxObjEnumerationAccess = xEnumerationAccess;
    }
    return mxObjEnumerationAccess;
}

Reference<XIdlArray> ImplIntrospectionAccess::getXIdlArray()
{
    ResettableGuard< Mutex > aGuard( m_aMutex );

    if( !mxObjIdlArray.is() )
    {
        aGuard.clear();
        Reference<XIdlArray> xIdlArray( mxIface, UNO_QUERY );
        aGuard.reset();
        if( !mxObjIdlArray.is() )
            mxObjIdlArray = xIdlArray;
    }
    return mxObjIdlArray;
}

// Methods from XInterface
Any SAL_CALL ImplIntrospectionAccess::queryInterface( const Type& rType )
{
    Any aRet( ::cppu::queryInterface(
        rType,
        static_cast< XIntrospectionAccess * >( this ),
        static_cast< XMaterialHolder * >( this ),
        static_cast< XExactName * >( this ),
        static_cast< XPropertySet * >( this ),
        static_cast< XFastPropertySet * >( this ),
        static_cast< XPropertySetInfo * >( this ) ) );
    if( !aRet.hasValue() )
        aRet = OWeakObject::queryInterface( rType );

    if( !aRet.hasValue() )
    {
        // Wrapper for the object interfaces
        if(   ( mpStaticImpl->mbElementAccess && (aRet = ::cppu::queryInterface
                    ( rType, static_cast< XElementAccess* >( static_cast< XNameAccess* >( this ) ) ) ).hasValue() )
            || ( mpStaticImpl->mbNameAccess && (aRet = ::cppu::queryInterface( rType, static_cast< XNameAccess* >( this ) ) ).hasValue() )
            || ( mpStaticImpl->mbNameReplace && (aRet = ::cppu::queryInterface( rType, static_cast< XNameReplace* >( this ) ) ).hasValue() )
            || ( mpStaticImpl->mbNameContainer && (aRet = ::cppu::queryInterface( rType, static_cast< XNameContainer* >( this ) ) ).hasValue() )
            || ( mpStaticImpl->mbIndexAccess && (aRet = ::cppu::queryInterface( rType, static_cast< XIndexAccess* >( this ) ) ).hasValue() )
            || ( mpStaticImpl->mbIndexReplace && (aRet = ::cppu::queryInterface( rType, static_cast< XIndexReplace* >( this ) ) ).hasValue() )
            || ( mpStaticImpl->mbIndexContainer && (aRet = ::cppu::queryInterface( rType, static_cast< XIndexContainer* >( this ) ) ).hasValue() )
            || ( mpStaticImpl->mbEnumerationAccess && (aRet = ::cppu::queryInterface( rType, static_cast< XEnumerationAccess* >( this ) ) ).hasValue() )
            || ( mpStaticImpl->mbIdlArray && (aRet = ::cppu::queryInterface( rType, static_cast< XIdlArray* >( this ) ) ).hasValue() )
            || ( mpStaticImpl->mbUnoTunnel && (aRet = ::cppu::queryInterface( rType, static_cast< XUnoTunnel* >( this ) ) ).hasValue() )
          )
        {
        }
    }
    return aRet;
}


//*** Implementation of ImplIntrospectionAdapter ***


// Methods from XPropertySet
Reference<XPropertySetInfo> ImplIntrospectionAccess::getPropertySetInfo()
{
    return static_cast<XPropertySetInfo *>(this);
}

void ImplIntrospectionAccess::setPropertyValue(const OUString& aPropertyName, const Any& aValue)
{
    mpStaticImpl->setPropertyValue( maInspectedObject, aPropertyName, aValue );
}

Any ImplIntrospectionAccess::getPropertyValue(const OUString& aPropertyName)
{
    return mpStaticImpl->getPropertyValue( maInspectedObject, aPropertyName );
}

void ImplIntrospectionAccess::addPropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
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

void ImplIntrospectionAccess::removePropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
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

void ImplIntrospectionAccess::addVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
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

void ImplIntrospectionAccess::removeVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
{
    if( mxIface.is() )
    {
        Reference<XPropertySet> xPropSet =
            Reference<XPropertySet>::query( mxIface );
        if( xPropSet.is() )
            xPropSet->removeVetoableChangeListener(aPropertyName, aListener);
    }
}


// Methods from XFastPropertySet
void ImplIntrospectionAccess::setFastPropertyValue(sal_Int32, const Any&)
{
}

Any ImplIntrospectionAccess::getFastPropertyValue(sal_Int32)
{
    return Any();
}

// Methods from XPropertySetInfo
Sequence< Property > ImplIntrospectionAccess::getProperties()
{
    return comphelper::containerToSequence(mpStaticImpl->getProperties());
}

Property ImplIntrospectionAccess::getPropertyByName(const OUString& Name)
{
    return getProperty( Name, PropertyConcept::ALL );
}

sal_Bool ImplIntrospectionAccess::hasPropertyByName(const OUString& Name)
{
    return hasProperty( Name, PropertyConcept::ALL );
}

// Methods from XElementAccess
Type ImplIntrospectionAccess::getElementType()
{
    return getXElementAccess()->getElementType();
}

sal_Bool ImplIntrospectionAccess::hasElements()
{
    return getXElementAccess()->hasElements();
}

// Methods from XNameAccess
Any ImplIntrospectionAccess::getByName(const OUString& Name)
{
    return getXNameAccess()->getByName( Name );
}

Sequence< OUString > ImplIntrospectionAccess::getElementNames()
{
    return getXNameAccess()->getElementNames();
}

sal_Bool ImplIntrospectionAccess::hasByName(const OUString& Name)
{
    return getXNameAccess()->hasByName( Name );
}

// Methods from XNameContainer
void ImplIntrospectionAccess::insertByName(const OUString& Name, const Any& Element)
{
    getXNameContainer()->insertByName( Name, Element );
}

void ImplIntrospectionAccess::replaceByName(const OUString& Name, const Any& Element)
{
    getXNameReplace()->replaceByName( Name, Element );
}

void ImplIntrospectionAccess::removeByName(const OUString& Name)
{
    getXNameContainer()->removeByName( Name );
}

// Methods from XIndexAccess
// Already in XNameAccess: virtual Reference<XIdlClass> getElementType() const
sal_Int32 ImplIntrospectionAccess::getCount()
{
    return getXIndexAccess()->getCount();
}

Any ImplIntrospectionAccess::getByIndex(sal_Int32 Index)
{
    return getXIndexAccess()->getByIndex( Index );
}

// Methods from XIndexContainer
void ImplIntrospectionAccess::insertByIndex(sal_Int32 Index, const Any& Element)
{
    getXIndexContainer()->insertByIndex( Index, Element );
}

void ImplIntrospectionAccess::replaceByIndex(sal_Int32 Index, const Any& Element)
{
    getXIndexReplace()->replaceByIndex( Index, Element );
}

void ImplIntrospectionAccess::removeByIndex(sal_Int32 Index)
{
    getXIndexContainer()->removeByIndex( Index );
}

// Methods from XEnumerationAccess
// Already in XNameAccess: virtual Reference<XIdlClass> getElementType() const;
Reference<XEnumeration> ImplIntrospectionAccess::createEnumeration()
{
    return getXEnumerationAccess()->createEnumeration();
}

// Methods from XIdlArray
void ImplIntrospectionAccess::realloc(Any& array, sal_Int32 length)
{
    getXIdlArray()->realloc( array, length );
}

sal_Int32 ImplIntrospectionAccess::getLen(const Any& array)
{
    return getXIdlArray()->getLen( array );
}

Any ImplIntrospectionAccess::get(const Any& array, sal_Int32 index)
{
    return getXIdlArray()->get( array, index );
}

void ImplIntrospectionAccess::set(Any& array, sal_Int32 index, const Any& value)
{
    getXIdlArray()->set( array, index, value );
}

// Methods from XUnoTunnel
sal_Int64 ImplIntrospectionAccess::getSomething( const Sequence< sal_Int8 >& aIdentifier )
{
    return Reference<XUnoTunnel>::query( mxIface )->getSomething( aIdentifier );
}


//*** Implementation of ImplIntrospectionAccess ***

// Methods from XIntrospectionAccess
sal_Int32 ImplIntrospectionAccess::getSuppliedMethodConcepts()
{
    return    MethodConcept::DANGEROUS |
            PROPERTY |
            LISTENER |
            ENUMERATION |
            NAMECONTAINER |
            INDEXCONTAINER;
}

sal_Int32 ImplIntrospectionAccess::getSuppliedPropertyConcepts()
{
    return    PropertyConcept::DANGEROUS |
            PROPERTYSET |
            ATTRIBUTES |
            METHODS;
}

Property ImplIntrospectionAccess::getProperty(const OUString& Name, sal_Int32 PropertyConcepts)
{
    Property aRet;
    sal_Int32 i = mpStaticImpl->getPropertyIndex( Name );
    bool bFound = false;
    if( i != -1 )
    {
        sal_Int32 nConcept = mpStaticImpl->getPropertyConcepts()[ i ];
        if( (PropertyConcepts & nConcept) != 0 )
        {
            aRet = mpStaticImpl->getProperties()[ i ];
            bFound = true;
        }
    }
    if( !bFound )
        throw NoSuchElementException(Name);
    return aRet;
}

sal_Bool ImplIntrospectionAccess::hasProperty(const OUString& Name, sal_Int32 PropertyConcepts)
{
    sal_Int32 i = mpStaticImpl->getPropertyIndex( Name );
    bool bRet = false;
    if( i != -1 )
    {
        sal_Int32 nConcept = mpStaticImpl->getPropertyConcepts()[ i ];
        if( (PropertyConcepts & nConcept) != 0 )
            bRet = true;
    }
    return bRet;
}

Sequence< Property > ImplIntrospectionAccess::getProperties(sal_Int32 PropertyConcepts)
{
    // If all supported concepts are required, simply pass through the sequence
    sal_Int32 nAllSupportedMask =   PROPERTYSET |
                                    ATTRIBUTES |
                                    METHODS;
    if( ( PropertyConcepts & nAllSupportedMask ) == nAllSupportedMask )
    {
        return comphelper::containerToSequence(mpStaticImpl->getProperties());
    }

    // Same sequence as last time?
    if( mnLastPropertyConcept == PropertyConcepts )
    {
        return maLastPropertySeq;
    }

    // Number of properties to be delivered
    sal_Int32 nCount = 0;

    // There are currently no DANGEROUS properties
    // if( PropertyConcepts & DANGEROUS )
    //    nCount += mpStaticImpl->mnDangerousPropCount;
    if( PropertyConcepts & PROPERTYSET )
        nCount += mpStaticImpl->mnPropertySetPropCount;
    if( PropertyConcepts & ATTRIBUTES )
        nCount += mpStaticImpl->mnAttributePropCount;
    if( PropertyConcepts & METHODS )
        nCount += mpStaticImpl->mnMethodPropCount;

    // Realloc sequence according to the required number
    maLastPropertySeq.realloc( nCount );
    Property* pDestProps = maLastPropertySeq.getArray();

    // Go through all the properties and apply according to the concept
    const std::vector<Property>&  rPropSeq = mpStaticImpl->getProperties();
    const std::vector<sal_Int32>& rConcepts = mpStaticImpl->getPropertyConcepts();
    sal_Int32 nLen = (sal_Int32)rPropSeq.size();

    sal_Int32 iDest = 0;
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        sal_Int32 nConcept = rConcepts[ i ];
        if( nConcept & PropertyConcepts )
            pDestProps[ iDest++ ] = rPropSeq[ i ];
    }

    // Remember PropertyConcept representing maLastPropertySeq
    mnLastPropertyConcept = PropertyConcepts;

    // Supply assembled Sequence
    return maLastPropertySeq;
}

Reference<XIdlMethod> ImplIntrospectionAccess::getMethod(const OUString& Name, sal_Int32 MethodConcepts)
{
    Reference<XIdlMethod> xRet;
    sal_Int32 i = mpStaticImpl->getMethodIndex( Name );
    if( i != -1 )
    {

        sal_Int32 nConcept = mpStaticImpl->getMethodConcepts()[ i ];
        if( (MethodConcepts & nConcept) != 0 )
        {
            xRet = mpStaticImpl->getMethods()[i];
        }
    }
    if( !xRet.is() )
        throw NoSuchMethodException(Name);
    return xRet;
}

sal_Bool ImplIntrospectionAccess::hasMethod(const OUString& Name, sal_Int32 MethodConcepts)
{
    sal_Int32 i = mpStaticImpl->getMethodIndex( Name );
    bool bRet = false;
    if( i != -1 )
    {
        sal_Int32 nConcept = mpStaticImpl->getMethodConcepts()[ i ];
        if( (MethodConcepts & nConcept) != 0 )
            bRet = true;
    }
    return bRet;
}

Sequence< Reference<XIdlMethod> > ImplIntrospectionAccess::getMethods(sal_Int32 MethodConcepts)
{
    // If all supported concepts are required, simply pass through the sequence
    sal_Int32 nAllSupportedMask =   MethodConcept::DANGEROUS |
                                    PROPERTY |
                                    LISTENER |
                                    ENUMERATION |
                                    NAMECONTAINER |
                                    INDEXCONTAINER |
                                    MethodConcept_NORMAL_IMPL;
    if( ( MethodConcepts & nAllSupportedMask ) == nAllSupportedMask )
    {
        return comphelper::containerToSequence(mpStaticImpl->getMethods());
    }

    // Same sequence as last time?
    if( mnLastMethodConcept == MethodConcepts )
    {
        return maLastMethodSeq;
    }

    // Get method sequences
    const std::vector< Reference<XIdlMethod> >& aMethodSeq = mpStaticImpl->getMethods();
    sal_Int32 nLen = (sal_Int32)aMethodSeq.size();

    // Realloc sequence according to the required number
    // Unlike Properties, the number can not be determined by counters in
    // inspect() beforehand, since methods can belong to several concepts
    maLastMethodSeq.realloc( nLen );
    Reference<XIdlMethod>* pDestMethods = maLastMethodSeq.getArray();

    // Go through all the methods and apply according to the concept
    sal_Int32 iDest = 0;
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        sal_Int32 nConcept = mpStaticImpl->getMethodConcepts()[ i ];
        if( nConcept & MethodConcepts )
            pDestMethods[ iDest++ ] = aMethodSeq[ i ];
    }

    // Bring to the correct length
    maLastMethodSeq.realloc( iDest );

    // Remember MethodConcept representing maLastMethodSeq
    mnLastMethodConcept = MethodConcepts;

    // Supply assembled Sequence
    return maLastMethodSeq;
}

Sequence< Type > ImplIntrospectionAccess::getSupportedListeners()
{
    return comphelper::containerToSequence(mpStaticImpl->getSupportedListeners());
}

Reference<XInterface> SAL_CALL ImplIntrospectionAccess::queryAdapter( const Type& rType )
{
    Reference<XInterface> xRet;
    if(    rType == cppu::UnoType<XInterface>::get()
        || rType == cppu::UnoType<XPropertySet>::get()
        || rType == cppu::UnoType<XFastPropertySet>::get()
        || rType == cppu::UnoType<XPropertySetInfo>::get()
        || rType == cppu::UnoType<XElementAccess>::get()
        || rType == cppu::UnoType<XNameAccess>::get()
        || rType == cppu::UnoType<XNameReplace>::get()
        || rType == cppu::UnoType<XNameContainer>::get()
        || rType == cppu::UnoType<XIndexAccess>::get()
        || rType == cppu::UnoType<XIndexReplace>::get()
        || rType == cppu::UnoType<XIndexContainer>::get()
        || rType == cppu::UnoType<XEnumerationAccess>::get()
        || rType == cppu::UnoType<XIdlArray>::get()
        || rType == cppu::UnoType<XUnoTunnel>::get() )
    {
        queryInterface( rType ) >>= xRet;
    }
    return xRet;
}

// Methods from XMaterialHolder
Any ImplIntrospectionAccess::getMaterial()
{
    return maInspectedObject;
}

// Methods from XExactName
OUString ImplIntrospectionAccess::getExactName( const OUString& rApproximateName )
{
    OUString aRetStr;
    LowerToExactNameMap::iterator aIt =
        mpStaticImpl->maLowerToExactNameMap.find( rApproximateName.toAsciiLowerCase() );
    if (aIt != mpStaticImpl->maLowerToExactNameMap.end())
        aRetStr = (*aIt).second;
    return aRetStr;
}

struct TypeKey {
    TypeKey(
        css::uno::Reference<css::beans::XPropertySetInfo> const & theProperties,
        std::vector<css::uno::Type> const & theTypes):
        properties(theProperties)
    {
        //TODO: Could even sort the types lexicographically first, to increase
        // the chance of matches between different implementations' getTypes(),
        // but the old scheme of using getImplementationId() would have missed
        // those matches, too:
        OUStringBuffer b;
        for (const css::uno::Type& rType : theTypes) {
            b.append(rType.getTypeName());
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
    cppu::WeakComponentImplHelper<
        css::lang::XServiceInfo, css::beans::XIntrospection>
    Implementation_Base;

class Implementation: private cppu::BaseMutex, public Implementation_Base {
public:
    explicit Implementation(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        Implementation_Base(m_aMutex),
        reflection_(css::reflection::theCoreReflection::get(context))
    {}

private:
    virtual void SAL_CALL disposing() override {
        osl::MutexGuard g(m_aMutex);
        reflection_.clear();
        typeCache_.clear();
    }

    virtual OUString SAL_CALL getImplementationName() override
    { return OUString("com.sun.star.comp.stoc.Introspection"); }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence<OUString> SAL_CALL
    getSupportedServiceNames() override
    {
        Sequence<OUString> s { "com.sun.star.beans.Introspection" };
        return s;
    }

    virtual css::uno::Reference<css::beans::XIntrospectionAccess> SAL_CALL
    inspect(css::uno::Any const & aObject) override;

    css::uno::Reference<css::reflection::XIdlReflection> reflection_;
    Cache<TypeKey, TypeKeyLess> typeCache_;
};

css::uno::Reference<css::beans::XIntrospectionAccess> Implementation::inspect(
    css::uno::Any const & aObject)
{
    css::uno::Reference<css::reflection::XIdlReflection> reflection;
    {
        osl::MutexGuard g(m_aMutex);
        if (rBHelper.bDisposed || rBHelper.bInDispose) {
            throw css::lang::DisposedException(
                getImplementationName(), static_cast<OWeakObject *>(this));
        }
        reflection = reflection_;
    }
    css::uno::Any aToInspectObj;
    css::uno::Type t;
    if (aObject >>= t) {
        css::uno::Reference<css::reflection::XIdlClass> c(
            reflection->forName(t.getTypeName()));
        if (!c.is()) {
            SAL_WARN("stoc", "cannot reflect type " << t.getTypeName());
            return css::uno::Reference<css::beans::XIntrospectionAccess>();
        }
        aToInspectObj <<= c;
    } else {
        aToInspectObj = aObject;
    }

    // Examine object
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE && eType != TypeClass_STRUCT  && eType != TypeClass_EXCEPTION )
        return css::uno::Reference<css::beans::XIntrospectionAccess>();

    if( auto x = o3tl::tryAccess<Reference<XInterface>>(aToInspectObj) )
    {
        if( !x->is() )
            return css::uno::Reference<css::beans::XIntrospectionAccess>();
    }

    // Pointer to possibly needed new IntrospectionAccessStatic_Impl instance
    rtl::Reference< IntrospectionAccessStatic_Impl > pAccess;

    // Check: Is a matching access object already cached?
    std::vector< Reference<XIdlClass> >    SupportedClassSeq;
    std::vector< Type >                    SupportedTypesSeq;
    Reference<XTypeProvider>               xTypeProvider;
    Reference<XPropertySetInfo>            xPropSetInfo;
    Reference<XPropertySet>                xPropSet;

    // Look for interfaces XTypeProvider and PropertySet
    if( eType == TypeClass_INTERFACE )
    {
        xTypeProvider.set( aToInspectObj, UNO_QUERY );
        if( xTypeProvider.is() )
        {
            SupportedTypesSeq = comphelper::sequenceToContainer<std::vector<Type>>(xTypeProvider->getTypes());
        } else {
            SAL_WARN(
                "stoc",
                "object of type \"" << aToInspectObj.getValueTypeName()
                    << "\" lacks XTypeProvider");
            SupportedTypesSeq = { aToInspectObj.getValueType() };
        }
        // Now try to get the PropertySetInfo
        xPropSet.set( aToInspectObj, UNO_QUERY );
        if( xPropSet.is() )
            xPropSetInfo = xPropSet->getPropertySetInfo();

    } else {
        SupportedTypesSeq = { aToInspectObj.getValueType() };
    }

    {
        osl::MutexGuard g(m_aMutex);
        if (rBHelper.bDisposed || rBHelper.bInDispose) {
            throw css::lang::DisposedException(
                getImplementationName(), static_cast<OWeakObject *>(this));
        }
        TypeKey key(xPropSetInfo, SupportedTypesSeq);
        pAccess = typeCache_.find(key);
        if (pAccess.is()) {
            return new ImplIntrospectionAccess(aToInspectObj, pAccess);
        }
        pAccess = new IntrospectionAccessStatic_Impl(reflection);
        typeCache_.insert(key, pAccess);
    }

    // No access cached -> create new
    std::vector<Property>& rAllPropArray = pAccess->maAllPropertySeq;
    std::vector<Reference<XInterface>>& rInterfaces1 = pAccess->aInterfaceSeq1;
    std::vector<Reference<XInterface>>& rInterfaces2 = pAccess->aInterfaceSeq2;
    std::vector<sal_Int16>& rMapTypeArray = pAccess->maMapTypeSeq;
    std::vector<sal_Int32>& rPropertyConceptArray = pAccess->maPropertyConceptSeq;
    sal_Int32 i;

    // References to important data from pAccess
    sal_Int32& rPropCount = pAccess->mnPropCount;
    IntrospectionNameMap& rPropNameMap = pAccess->maPropertyNameMap;
    IntrospectionNameMap& rMethodNameMap = pAccess->maMethodNameMap;
    LowerToExactNameMap& rLowerToExactNameMap = pAccess->maLowerToExactNameMap;


    //*** Perform analysis ***

    if( eType == TypeClass_INTERFACE )
    {
        size_t nTypeCount = SupportedTypesSeq.size();
        if( nTypeCount )
        {
            SupportedClassSeq.resize( nTypeCount );

            for( i = 0 ; i < (sal_Int32)nTypeCount ; i++ )
                SupportedClassSeq[i] = reflection->forName( SupportedTypesSeq[i].getTypeName() );
        }

        // First look for particular interfaces that are of particular
        // importance to the introspection

        // Is XPropertySet present?
        if( xPropSet.is() && xPropSetInfo.is() )
        {
            // Is there also a FastPropertySet?
            Reference<XFastPropertySet> xDummy( aToInspectObj, UNO_QUERY );
            bool bFast = pAccess->mbFastPropSet = xDummy.is();

            Sequence<Property> aPropSeq = xPropSetInfo->getProperties();
            const Property* pProps = aPropSeq.getConstArray();
            sal_Int32 nLen = aPropSeq.getLength();

            // For a FastPropertySet we must remember the original handles
            if( bFast )
                pAccess->mpOrgPropertyHandleArray.reset( new sal_Int32[ nLen ] );

            for( i = 0 ; i < nLen ; i++ )
            {
                // Put property in its own list
                pAccess->checkPropertyArraysSize( rPropCount );
                Property& rProp = rAllPropArray[ rPropCount ];
                rProp = pProps[ i ];

                if( bFast )
                    pAccess->mpOrgPropertyHandleArray[ i ] = rProp.Handle;

                // Enter PropCount as a handle for its own FastPropertySet
                rProp.Handle = rPropCount;

                // Remember type of property
                rMapTypeArray[ rPropCount ] = MAP_PROPERTY_SET;
                rPropertyConceptArray[ rPropCount ] = PROPERTYSET;
                pAccess->mnPropertySetPropCount++;

                // Enter name in hash table if not already known
                OUString aPropName = rProp.Name;

                // Do we already have the name?
                IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                if( aIt == rPropNameMap.end() )
                {
                    // New entry in the hash table
                    rPropNameMap[ aPropName ] = rPropCount;

                    // Maintain table for XExactName
                    rLowerToExactNameMap[ aPropName.toAsciiLowerCase() ] = aPropName;
                }
                else
                {
                    SAL_WARN( "stoc", "Introspection: Property \"" <<
                        aPropName << "\" found more than once in PropertySet" );
                }

                // Adjust count
                rPropCount++;
            }
        }

        // Indices in the export table
        sal_Int32 iAllExportedMethod = 0;
        sal_Int32 iAllSupportedListener = 0;

        std::set<OUString> seen;

        // Flag, whether XInterface methods should be recorded
        // (this must be done only once, allowed initially)
        bool bXInterfaceIsInvalid = false;

        // Flag whether the XInterface methods have already been recorded. If
        // sal_True, bXInterfaceIsInvalid is activated at the end of the interface
        // loop, and XInterface methods are cut off thereafter.
        bool bFoundXInterface = false;

        size_t nClassCount = SupportedClassSeq.size();
        for( sal_Int32 nIdx = 0 ; nIdx < (sal_Int32)nClassCount; nIdx++ )
        {
            Reference<XIdlClass> xImplClass2 = SupportedClassSeq[nIdx];
            while( xImplClass2.is() )
            {
                // Fetch interfaces from the implementation
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

                    // 2. Register fields as properties

                    // Get fields
                    Sequence< Reference<XIdlField> > fields = rxIfaceClass->getFields();
                    const Reference<XIdlField>* pFields = fields.getConstArray();
                    sal_Int32 nLen = fields.getLength();

                    for( i = 0 ; i < nLen ; i++ )
                    {
                        Reference<XIdlField> xField = pFields[i];
                        Reference<XIdlClass> xPropType = xField->getType();

                        // Is the property sequence big enough?
                        pAccess->checkPropertyArraysSize( rPropCount );

                        // Enter in own property array
                        Property& rProp = rAllPropArray[ rPropCount ];
                        OUString aFieldName = xField->getName();
                        rProp.Name = aFieldName;
                        rProp.Handle = rPropCount;
                        Type aFieldType( xPropType->getTypeClass(), xPropType->getName() );
                        rProp.Type = aFieldType;
                        FieldAccessMode eAccessMode = xField->getAccessMode();
                        rProp.Attributes = (eAccessMode == FieldAccessMode_READONLY ||
                                            eAccessMode == FieldAccessMode_CONST)
                            ? READONLY : 0;

                        // Enter name in hash table
                        OUString aPropName = rProp.Name;

                        // Do we have the name already?
                        IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                        if (aIt != rPropNameMap.end())
                            continue;

                        // New entry in the hash table
                        rPropNameMap[ aPropName ] = rPropCount;

                        // Maintain table for XExactName
                        rLowerToExactNameMap[ aPropName.toAsciiLowerCase() ] = aPropName;

                        // Remember field
                        IntrospectionAccessStatic_Impl::checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                                                          rInterfaces1, rPropCount );
                        rInterfaces1[ rPropCount ] = xField;

                        // Remember type of property
                        rMapTypeArray[ rPropCount ] = MAP_FIELD;
                        rPropertyConceptArray[ rPropCount ] = ATTRIBUTES;
                        pAccess->mnAttributePropCount++;

                        // Adjust count
                        rPropCount++;
                    }


                    // 3. Methods

                    // Counter for found listeners
                    sal_Int32 nListenerCount = 0;

                    // Get and remember all methods
                    Sequence< Reference<XIdlMethod> > methods = rxIfaceClass->getMethods();
                    const Reference<XIdlMethod>* pSourceMethods = methods.getConstArray();
                    sal_Int32 nSourceMethodCount = methods.getLength();

                    // 3. a) Search get/set and listener methods

                    // Create field for information about the methods, so that methods which are not
                    // related to properties or listeners can easily be found later.
                    // New: initialise MethodConceptArray
                    enum MethodType
                    {
                        STANDARD_METHOD,            // normal method, not related to properties or listeners
                        GETSET_METHOD,                // belongs to a get/set property
                        ADD_LISTENER_METHOD,        // add method of a listener interface
                        REMOVE_LISTENER_METHOD,        // remove method of a listener interface
                        INVALID_METHOD                // method whose class is not considered, e.g. XPropertySet
                    };
                    std::unique_ptr<MethodType[]> pMethodTypes( new MethodType[ nSourceMethodCount ] );
                    std::unique_ptr<sal_Int32[]> pLocalMethodConcepts( new sal_Int32[ nSourceMethodCount ] );
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        pMethodTypes[ i ] = STANDARD_METHOD;
                        pLocalMethodConcepts[ i ] = 0;
                    }

                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        // Address method
                        const Reference<XIdlMethod>& rxMethod_i = pSourceMethods[i];
                        sal_Int32& rMethodConcept_i = pLocalMethodConcepts[ i ];

                        // Fetch name
                        OUString aMethName = rxMethod_i->getName();

                        // Catalogue methods
                        // Filter all (?) methods of XInterface so e.g. acquire and release
                        // can not be called from scripting
                        rtl::OUString className(
                            rxMethod_i->getDeclaringClass()->getName());
                        if (className == "com.sun.star.uno.XInterface") {
                            bFoundXInterface = true;

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
                            pAccess->mbElementAccess = true;
                        } else if ((className
                                    == "com.sun.star.container.XNameContainer"))
                        {
                            rMethodConcept_i |= NAMECONTAINER;
                            pAccess->mbNameContainer = true;
                            pAccess->mbNameReplace = true;
                            pAccess->mbNameAccess = true;
                            pAccess->mbElementAccess = true;
                        } else if ((className
                                    == "com.sun.star.container.XNameReplace"))
                        {
                            rMethodConcept_i |= NAMECONTAINER;
                            pAccess->mbNameReplace = true;
                            pAccess->mbNameAccess = true;
                            pAccess->mbElementAccess = true;
                        } else if ((className
                                    == "com.sun.star.container.XNameAccess"))
                        {
                            rMethodConcept_i |= NAMECONTAINER;
                            pAccess->mbNameAccess = true;
                            pAccess->mbElementAccess = true;
                        } else if ((className
                                    == "com.sun.star.container.XIndexContainer"))
                        {
                            rMethodConcept_i |= INDEXCONTAINER;
                            pAccess->mbIndexContainer = true;
                            pAccess->mbIndexReplace = true;
                            pAccess->mbIndexAccess = true;
                            pAccess->mbElementAccess = true;
                        } else if ((className
                                    == "com.sun.star.container.XIndexReplace"))
                        {
                            rMethodConcept_i |= INDEXCONTAINER;
                            pAccess->mbIndexReplace = true;
                            pAccess->mbIndexAccess = true;
                            pAccess->mbElementAccess = true;
                        } else if ((className
                                    == "com.sun.star.container.XIndexAccess"))
                        {
                            rMethodConcept_i |= INDEXCONTAINER;
                            pAccess->mbIndexAccess = true;
                            pAccess->mbElementAccess = true;
                        } else if (className
                                   == "com.sun.star.container.XEnumerationAccess")
                        {
                            rMethodConcept_i |= ENUMERATION;
                            pAccess->mbEnumerationAccess = true;
                            pAccess->mbElementAccess = true;
                        } else if (className
                                   == "com.sun.star.reflection.XIdlArray")
                        {
                            pAccess->mbIdlArray = true;
                        } else if (className
                                   == "com.sun.star.lang.XUnoTunnel")
                        {
                            pAccess->mbUnoTunnel = true;
                        }

                        // If the name is too short, it isn't anything
                        if( aMethName.getLength() <= 3 )
                            continue;

                        // Is it a get method?
                        OUString aPropName;
                        if( aMethName.startsWith("get", &aPropName) )
                        {
                            // Get methods must not have any parameters
                            Sequence< Reference<XIdlClass> > getParams = rxMethod_i->getParameterTypes();
                            if( getParams.getLength() > 0 )
                            {
                                continue;
                            }

                            // Do we have the name already?
                            IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                            if (aIt != rPropNameMap.end())
                            {
                                /* TODO
                                   SAL_INFO("stoc",(
                                   String( "Introspection: Property \"" ) +
                                   OOUStringToString( aPropName, CHARSET_SYSTEM ) +
                                   String( "\" found more than once" ) );
                                */
                                continue;
                            }

                            // It is already at least a read-only property
                            rMethodConcept_i |= PROPERTY;

                            pMethodTypes[i] = GETSET_METHOD;
                            Reference<XIdlClass> xGetRetType = rxMethod_i->getReturnType();

                            // Is the property sequence big enough?
                            pAccess->checkPropertyArraysSize( rPropCount );

                            // Write it in its property array
                            Property& rProp = rAllPropArray[ rPropCount ];
                            rProp.Name = aPropName;
                            rProp.Handle = rPropCount;
                            rProp.Type = Type( xGetRetType->getTypeClass(), xGetRetType->getName() );
                            rProp.Attributes = READONLY;

                            // New entry in the hash table
                            rPropNameMap[ aPropName ] = rPropCount;

                            // Maintain table for XExactName
                            rLowerToExactNameMap[ aPropName.toAsciiLowerCase() ] = aPropName;

                            // Remember get method
                            IntrospectionAccessStatic_Impl::checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                                                              rInterfaces1, rPropCount );
                            rInterfaces1[ rPropCount ] = rxMethod_i;

                            // Remember type of property
                            rMapTypeArray[ rPropCount ] = MAP_GETSET;
                            rPropertyConceptArray[ rPropCount ] = METHODS;
                            pAccess->mnMethodPropCount++;

                            // Search for matching set method
                            sal_Int32 k;
                            for( k = 0 ; k < nSourceMethodCount ; k++ )
                            {
                                // Address method
                                const Reference<XIdlMethod>& rxMethod_k = pSourceMethods[k];

                                // Accept only methods that are not already assigned
                                if( k == i || pMethodTypes[k] != STANDARD_METHOD )
                                    continue;

                                // Get name and evaluate
                                OUString aMethName2 = rxMethod_k->getName();
                                OUString aPropName2;
                                if (!(aMethName2.startsWith("set", &aPropName2)
                                      && aPropName2 == aPropName))
                                    continue;

                                // A set method must return void
                                Reference<XIdlClass> xSetRetType = rxMethod_k->getReturnType();
                                if( xSetRetType->getTypeClass() != TypeClass_VOID )
                                {
                                    continue;
                                }

                                // A set method may only have one parameter
                                Sequence< Reference<XIdlClass> > setParams = rxMethod_k->getParameterTypes();
                                sal_Int32 nParamCount = setParams.getLength();
                                if( nParamCount != 1 )
                                {
                                    continue;
                                }

                                // Next, the return type must correspond to the parameter type
                                const Reference<XIdlClass>* pParamArray2 = setParams.getConstArray();
                                Reference<XIdlClass> xParamType = pParamArray2[ 0 ];
                                if( xParamType->equals( xGetRetType ) )
                                {
                                    pLocalMethodConcepts[ k ] = PROPERTY;

                                    pMethodTypes[k] = GETSET_METHOD;

                                    // Delete read-only flag again
                                    rProp.Attributes &= ~READONLY;

                                    // Remember set method
                                    IntrospectionAccessStatic_Impl::checkInterfaceArraySize( pAccess->aInterfaceSeq2,
                                                                      rInterfaces2, rPropCount );
                                    rInterfaces2[ rPropCount ] = rxMethod_k;
                                }
                            }

                            // Adjust count
                            rPropCount++;
                        }

                        // Is it an add listener method?
                        else if( aMethName.startsWith("add", &aPropName) )
                        {
                            // Does it end with "Listener"?
                            OUString aListenerName;
                            if( !aPropName.endsWith("Listener", &aListenerName) )
                                continue;

                            // TODO: More accurate tests could still be carried out here
                            // - Return type
                            // - Number and type of parameters


                            // Search for matching remove method, otherwise not applicable
                            sal_Int32 k;
                            for( k = 0 ; k < nSourceMethodCount ; k++ )
                            {
                                // Address method
                                const Reference<XIdlMethod>& rxMethod_k = pSourceMethods[k];

                                // Accept only methods that are not already assigned
                                if( k == i || pMethodTypes[k] != STANDARD_METHOD )
                                    continue;

                                // Get name and evaluate
                                OUString aMethName2 = rxMethod_k->getName();
                                OUString aListenerName2;
                                if (!(aMethName2.startsWith(
                                          "remove", &aPropName)
                                      && aPropName.endsWith(
                                          "Listener", &aListenerName2)
                                      && aListenerName2 == aListenerName))
                                    continue;

                                // TODO: More accurate tests could still be carried out here
                                // - Return type
                                // - Number and type of parameters


                                // Methods are recognised as a listener interface
                                rMethodConcept_i |= LISTENER;
                                pLocalMethodConcepts[ k ] |= LISTENER;

                                pMethodTypes[i] = ADD_LISTENER_METHOD;
                                pMethodTypes[k] = REMOVE_LISTENER_METHOD;
                                nListenerCount++;
                            }
                        }
                    }


                    // A set method could still exist without a corresponding get method,
                    // this must be a write-only property
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        // Address method
                        const Reference<XIdlMethod>& rxMethod_i = pSourceMethods[i];

                        // Accept only methods that are not already assigned
                        if( pMethodTypes[i] != STANDARD_METHOD )
                            continue;

                        // Get name
                        OUString aMethName = rxMethod_i->getName();

                        // If the name is too short, it isn't anything
                        if( aMethName.getLength() <= 3 )
                            continue;

                        // Is it a set method without associated get method?
                        OUString aPropName;
                        if( aMethName.startsWith("set", &aPropName) )
                        {
                            // A set method must return void
                            Reference<XIdlClass> xSetRetType = rxMethod_i->getReturnType();
                            if( xSetRetType->getTypeClass() != TypeClass_VOID )
                            {
                                continue;
                            }

                            // A set method may only have one parameter
                            Sequence< Reference<XIdlClass> > setParams = rxMethod_i->getParameterTypes();
                            sal_Int32 nParamCount = setParams.getLength();
                            if( nParamCount != 1 )
                            {
                                continue;
                            }

                            // Do we have the name already?
                            IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                            if (aIt != rPropNameMap.end())
                            {
                                /* TODO:
                                   SAL_INFO("stoc",(
                                   String( "Introspection: Property \"" ) +
                                   OOUStringToString( aPropName, CHARSET_SYSTEM ) +
                                   String( "\" found more than once" ) );
                                */
                                continue;
                            }

                            // Now we know it's a write only property
                            pLocalMethodConcepts[ i ] = PROPERTY;

                            pMethodTypes[i] = GETSET_METHOD;
                            Reference<XIdlClass> xGetRetType = setParams.getConstArray()[0];

                            // Is the property sequence big enough?
                            pAccess->checkPropertyArraysSize( rPropCount );

                            // Write it in its property array
                            Property& rProp = rAllPropArray[ rPropCount ];
                            rProp.Name = aPropName;
                            rProp.Handle = rPropCount;
                            rProp.Type = Type( xGetRetType->getTypeClass(), xGetRetType->getName() );
                            rProp.Attributes = 0;    // PROPERTY_WRITEONLY ???

                            // New entry in the hash table
                            rPropNameMap[ aPropName ] = rPropCount;

                            // Maintain table for XExactName
                            rLowerToExactNameMap[ aPropName.toAsciiLowerCase() ] = aPropName;

                            // Remember set method
                            IntrospectionAccessStatic_Impl::checkInterfaceArraySize( pAccess->aInterfaceSeq2,
                                                              rInterfaces2, rPropCount );
                            rInterfaces2[ rPropCount ] = rxMethod_i;

                            // Remember type of property
                            rMapTypeArray[ rPropCount ] = MAP_SETONLY;
                            rPropertyConceptArray[ rPropCount ] = METHODS;
                            pAccess->mnMethodPropCount++;

                            // Adjust count
                            rPropCount++;
                        }
                    }


                    // 4. Place methods in overall sequence

                    // How many methods in the method sequence
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

                    // Enlarge sequences in the access object accordingly
                    pAccess->maAllMethodSeq.resize( nExportedMethodCount + iAllExportedMethod );
                    pAccess->maMethodConceptSeq.resize( nExportedMethodCount + iAllExportedMethod );
                    pAccess->maSupportedListenerSeq.resize( nSupportedListenerCount + iAllSupportedListener );

                    // Write in methods
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        if( pMethodTypes[ i ] != INVALID_METHOD )
                        {
                            // Address method
                            const Reference<XIdlMethod>& rxMethod = pSourceMethods[i];

                            // Enter name in hash table if not already known
                            OUString aMethName2 = rxMethod->getName();
                            IntrospectionNameMap::iterator aIt = rMethodNameMap.find( aMethName2 );
                            if( aIt == rMethodNameMap.end() )
                            {
                                // Enter
                                rMethodNameMap[ aMethName2 ] = iAllExportedMethod;

                                // Maintain table for XExactName
                                rLowerToExactNameMap[ aMethName2.toAsciiLowerCase() ] = aMethName2;
                            }
                            else
                            {
                                sal_Int32 iHashResult = aIt->second;

                                Reference<XIdlMethod> xExistingMethod = pAccess->maAllMethodSeq[iHashResult];

                                Reference< XIdlClass > xExistingMethClass =
                                    xExistingMethod->getDeclaringClass();
                                Reference< XIdlClass > xNewMethClass = rxMethod->getDeclaringClass();
                                if( xExistingMethClass->equals( xNewMethClass ) )
                                    continue;
                            }

                            pAccess->maAllMethodSeq[iAllExportedMethod] = rxMethod;

                            // If a concept has been set, is the method "normal"?
                            sal_Int32& rMethodConcept_i = pLocalMethodConcepts[ i ];
                            if( !rMethodConcept_i )
                                rMethodConcept_i = MethodConcept_NORMAL_IMPL;
                            pAccess->maMethodConceptSeq[ iAllExportedMethod ] = rMethodConcept_i;
                            iAllExportedMethod++;
                        }
                        if( pMethodTypes[ i ] == ADD_LISTENER_METHOD )
                        {
                            // Determine class of listener
                            const Reference<XIdlMethod>& rxMethod = pSourceMethods[i];

                            // Enter void as default class
                            css::uno::Reference<css::reflection::XIdlClass>
                                xListenerClass(
                                    reflection->forName(
                                        cppu::UnoType<void>::get()
                                        .getTypeName()));
                            // Old: Reference<XIdlClass> xListenerClass = Void_getReflection()->getIdlClass();

                            // Option 1: Search for parameters for a listener class
                            // Disadvantage: Superclasses should be searched recursively
                            Sequence< Reference<XIdlClass> > aParams = rxMethod->getParameterTypes();
                            const Reference<XIdlClass>* pParamArray2 = aParams.getConstArray();

                            css::uno::Reference<css::reflection::XIdlClass>
                                xEventListenerClass(
                                    reflection->forName(
                                        cppu::UnoType<
                                            css::lang::XEventListener>::get()
                                        .getTypeName()));
                            // Old: Reference<XIdlClass> xEventListenerClass = XEventListener_getReflection()->getIdlClass();
                            sal_Int32 nParamCount = aParams.getLength();
                            sal_Int32 k;
                            for( k = 0 ; k < nParamCount ; k++ )
                            {
                                const Reference<XIdlClass>& rxClass = pParamArray2[k];

                                // Are we derived from a listener?
                                if( rxClass->equals( xEventListenerClass ) ||
                                    isDerivedFrom( rxClass, xEventListenerClass ) )
                                {
                                    xListenerClass = rxClass;
                                    break;
                                }
                            }

                            // Option 2: Unload the name of the method
                            // Disadvantage: Does not work with test listeners, where it does not exist
                            //aMethName = rxMethod->getName();
                            //aListenerName = aMethName.Copy( 3, aMethName.Len()-8-3 );
                            //Reference<XIdlClass> xListenerClass = reflection->forName( aListenerName );
                            Type aListenerType( TypeClass_INTERFACE, xListenerClass->getName() );
                            pAccess->maSupportedListenerSeq[ iAllSupportedListener ] = aListenerType;
                            iAllSupportedListener++;
                        }
                    }

                    // When there were XInterface methods in this run,
                    // ignore them in the future
                    if( bFoundXInterface )
                        bXInterfaceIsInvalid = true;
                }

                // Do superclasses exist? Then continue here
                Sequence< Reference<XIdlClass> > aSuperClassSeq = xImplClass2->getSuperclasses();

                // Currently only one superclass is considered
                if( aSuperClassSeq.getLength() >= 1 )
                {
                    xImplClass2 = aSuperClassSeq.getConstArray()[0];
                    OSL_ENSURE( xImplClass2.is(), "super class null" );
                }
                else
                {
                    xImplClass2 = nullptr;
                }
            }
        }

        // Apply number of exported methods and adapt Sequences
        // (can be different because duplicate methods are thrown
        // out only after the determination of nExportedMethodCount)
        sal_Int32& rMethCount = pAccess->mnMethCount;
        rMethCount = iAllExportedMethod;
        pAccess->maAllMethodSeq.resize( rMethCount );
        pAccess->maMethodConceptSeq.resize( rMethCount );

        // Resize the property sequences
        pAccess->maAllPropertySeq.resize( rPropCount );
        pAccess->maPropertyConceptSeq.resize( rPropCount );
        pAccess->maMapTypeSeq.resize( rPropCount );
    }
    // Register struct fields as properties
    else //if( eType == TypeClass_STRUCT )
    {
        // Is it an interface or a struct?
        //Reference<XIdlClass> xClassRef = aToInspectObj.getReflection()->getIdlClass();
        css::uno::Reference<css::reflection::XIdlClass> xClassRef(
            reflection->forName(aToInspectObj.getValueTypeName()));
        if( !xClassRef.is() )
        {
            SAL_WARN( "stoc", "Can't get XIdlClass from Reflection" );
            return new ImplIntrospectionAccess(aToInspectObj, pAccess);
        }

        // Get fields
        Sequence< Reference<XIdlField> > fields = xClassRef->getFields();
        const Reference<XIdlField>* pFields = fields.getConstArray();
        sal_Int32 nLen = fields.getLength();

        for( i = 0 ; i < nLen ; i++ )
        {
            Reference<XIdlField> xField = pFields[i];
            Reference<XIdlClass> xPropType = xField->getType();
            OUString aPropName = xField->getName();

            // Is the property sequence big enough?
            pAccess->checkPropertyArraysSize( rPropCount );

            // Write it in its property array
            Property& rProp = rAllPropArray[ rPropCount ];
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

            // Write name in hash table
            rPropNameMap[ aPropName ] = rPropCount;

            // Maintain table for XExactName
            rLowerToExactNameMap[ aPropName.toAsciiLowerCase() ] = aPropName;

            // Remember field
            IntrospectionAccessStatic_Impl::checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                rInterfaces1, rPropCount );
            rInterfaces1[ rPropCount ] = xField;

            // Remember type of property
            rMapTypeArray[ rPropCount ] = MAP_FIELD;
            rPropertyConceptArray[ rPropCount ] = ATTRIBUTES;
            pAccess->mnAttributePropCount++;

            // Adjust count
            rPropCount++;
        }
    }

    // Set property sequence to the correct length
    pAccess->maAllPropertySeq.resize( pAccess->mnPropCount );

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
