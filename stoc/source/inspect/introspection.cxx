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
#include <cppuhelper/supportsservice.hxx>
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

#define IMPLEMENTATION_NAME "com.sun.star.comp.stoc.Introspection"
#define SERVICE_NAME        "com.sun.star.beans.Introspection"

namespace stoc_inspect
{

typedef WeakImplHelper3< XIntrospectionAccess, XMaterialHolder, XExactName > IntrospectionAccessHelper;





#define  MethodConcept_NORMAL_IMPL        0x80000000



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





#define MAP_PROPERTY_SET    0

#define MAP_FIELD            1

#define MAP_GETSET            2

#define MAP_SETONLY            3



#define ARRAY_SIZE_STEP        20










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
    friend class ImplIntrospection;
    friend class ImplIntrospectionAccess;

    
    Reference< XIdlReflection > mxCoreReflection;

    
    
    Sequence< Reference<XInterface> > aInterfaceSeq1;
    Sequence< Reference<XInterface> > aInterfaceSeq2;

    
    IntrospectionNameMap maPropertyNameMap;
    IntrospectionNameMap maMethodNameMap;
    LowerToExactNameMap  maLowerToExactNameMap;

    
    Sequence<Property> maAllPropertySeq;

    
    Sequence<sal_Int16> maMapTypeSeq;

    
    Sequence<sal_Int32> maPropertyConceptSeq;

    
    sal_Int32 mnPropCount;

    
    
    sal_Int32 mnPropertySetPropCount;
    sal_Int32 mnAttributePropCount;
    sal_Int32 mnMethodPropCount;

    
    sal_Bool mbFastPropSet;

    
    sal_Int32* mpOrgPropertyHandleArray;

    
    Sequence< Reference<XIdlMethod> > maAllMethodSeq;

    
    Sequence<sal_Int32> maMethodConceptSeq;

    
    sal_Int32 mnMethCount;

    
    Sequence< Type > maSupportedListenerSeq;

    
    void BaseInit( void );

    
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

    
    void setPropertyValue(const Any& obj, const OUString& aPropertyName, const Any& aValue) const;

    Any getPropertyValue(const Any& obj, const OUString& aPropertyName) const;
    void setPropertyValueByIndex(const Any& obj, sal_Int32 nIndex, const Any& aValue) const;

    Any getPropertyValueByIndex(const Any& obj, sal_Int32 nIndex) const;

    Sequence<Property> getProperties(void) const                        { return maAllPropertySeq; }
    Sequence< Reference<XIdlMethod> > getMethods(void) const            { return maAllMethodSeq; }
    Sequence< Type > getSupportedListeners(void) const                    { return maSupportedListenerSeq; }
    Sequence<sal_Int32> getPropertyConcepts(void) const                    { return maPropertyConceptSeq; }
    Sequence<sal_Int32> getMethodConcepts(void) const                    { return maMethodConceptSeq; }
};



IntrospectionAccessStatic_Impl::IntrospectionAccessStatic_Impl( Reference< XIdlReflection > xCoreReflection_ )
    : mxCoreReflection( xCoreReflection_ )
{
    aInterfaceSeq1.realloc( ARRAY_SIZE_STEP );
    aInterfaceSeq2.realloc( ARRAY_SIZE_STEP );

    
    maAllPropertySeq.realloc( ARRAY_SIZE_STEP );
    maMapTypeSeq.realloc( ARRAY_SIZE_STEP );
    maPropertyConceptSeq.realloc( ARRAY_SIZE_STEP );

    mbFastPropSet = sal_False;
    mpOrgPropertyHandleArray = NULL;

    mnPropCount = 0;
    
    mnPropertySetPropCount = 0;
    mnAttributePropCount = 0;
    mnMethodPropCount = 0;

    
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
    
    else
    {
        sal_Int32 nSearchFrom = aMethodName.getLength();
        while( true )
        {
            
            sal_Int32 nFound = aMethodName.lastIndexOf( '_', nSearchFrom );
            if( nFound == -1 )
                break;

            OUString aPureMethodName = aMethodName.copy( nFound + 1 );

            aIt = pThis->maMethodNameMap.find( aPureMethodName );
            if( !( aIt == pThis->maMethodNameMap.end() ) )
            {
                
                
                OUString aStr = aMethodName.copy( 0, nFound );
                OUString aTypeName = aStr.replace( '_', '.' );
                Reference< XIdlClass > xClass = mxCoreReflection->forName( aTypeName );
                if( xClass.is() )
                {
                    

                    
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

{
    sal_Int32 i = getPropertyIndex( aPropertyName );
    if( i != -1 )
        setPropertyValueByIndex( obj, (sal_Int32)i, aValue );
    else
        throw UnknownPropertyException();
}

void IntrospectionAccessStatic_Impl::setPropertyValueByIndex(const Any& obj, sal_Int32 nSequenceIndex, const Any& aValue) const

{
    
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
            
            const Property& rProp = maAllPropertySeq.getConstArray()[ nSequenceIndex ];

            
            sal_Bool bUseCopy = sal_False;
            Any aRealValue;

            TypeClass eValType = aValue.getValueType().getTypeClass();
            if( eValType == TypeClass_INTERFACE )
            {
                Type aPropType = rProp.Type;
                OUString aTypeName( aPropType.getTypeName() );
                Reference< XIdlClass > xPropClass = mxCoreReflection->forName( aTypeName );
                
                if( xPropClass.is() && xPropClass->getTypeClass() == TypeClass_INTERFACE )
                {
                    Reference<XInterface> valInterface = *(Reference<XInterface>*)aValue.getValue();
                    if( valInterface.is() )
                    {
                        
                        aRealValue = valInterface->queryInterface( aPropType );
                        if( aRealValue.hasValue() )
                            bUseCopy = sal_True;
                    }
                }
            }

            
            
            
            sal_Int32 nOrgHandle;
            if( mbFastPropSet && ( nOrgHandle = mpOrgPropertyHandleArray[ nSequenceIndex ] ) != -1 )
            {
                
                Reference<XFastPropertySet> xFastPropSet =
                    Reference<XFastPropertySet>::query( xInterface );
                if( xFastPropSet.is() )
                {
                    xFastPropSet->setFastPropertyValue( nOrgHandle, bUseCopy ? aRealValue : aValue );
                }
                else
                {
                    
                }
            }
            
            else
            {
                
                Reference<XPropertySet> xPropSet =
                    Reference<XPropertySet>::query( xInterface );
                if( xPropSet.is() )
                {
                    xPropSet->setPropertyValue( rProp.Name, bUseCopy ? aRealValue : aValue );
                }
                else
                {
                    
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
                
                
            } else
            if( xField.is() )
            {
                xField->set( obj, aValue );
                
                
            }
            else
            {
                
            }
        }
        break;

        case MAP_GETSET:
        case MAP_SETONLY:
        {
            
            Reference<XIdlMethod> xMethod = (XIdlMethod*)(aInterfaceSeq2.getConstArray()[ nSequenceIndex ].get());
            if( xMethod.is() )
            {
                Sequence<Any> args( 1 );
                args.getArray()[0] = aValue;
                xMethod->invoke( obj, args );
            }
            else
            {
                
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

    
    TypeClass eObjType = obj.getValueType().getTypeClass();

    Reference<XInterface> xInterface;
    if( eObjType == TypeClass_INTERFACE )
    {
        xInterface = *(Reference<XInterface>*)obj.getValue();
    }
    else if( nSequenceIndex >= mnPropCount || ( eObjType != TypeClass_STRUCT && eObjType != TypeClass_EXCEPTION ) )
    {
        
        return aRet;
    }

    const sal_Int16* pMapTypeArray = maMapTypeSeq.getConstArray();
    switch( pMapTypeArray[ nSequenceIndex ] )
    {
        case MAP_PROPERTY_SET:
        {
            
            const Property& rProp = maAllPropertySeq.getConstArray()[ nSequenceIndex ];

            
            
            
            sal_Int32 nOrgHandle;
            if( mbFastPropSet && ( nOrgHandle = mpOrgPropertyHandleArray[ nSequenceIndex ] ) != -1 )
            {
                
                Reference<XFastPropertySet> xFastPropSet =
                    Reference<XFastPropertySet>::query( xInterface );
                if( xFastPropSet.is() )
                {
                    aRet = xFastPropSet->getFastPropertyValue( nOrgHandle);
                }
                else
                {
                    
                    return aRet;
                }
            }
            
            else
            {
                
                Reference<XPropertySet> xPropSet =
                    Reference<XPropertySet>::query( xInterface );
                if( xPropSet.is() )
                {
                    aRet = xPropSet->getPropertyValue( rProp.Name );
                }
                else
                {
                    
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
                
                
            }
            else
            {
                
                return aRet;
            }
        }
        break;

        case MAP_GETSET:
        {
            
            Reference<XIdlMethod> xMethod = (XIdlMethod*)(aInterfaceSeq1.getConstArray()[ nSequenceIndex ].get());
            if( xMethod.is() )
            {
                Sequence<Any> args;
                aRet = xMethod->invoke( obj, args );
            }
            else
            {
                
                return aRet;
            }
        }
        break;

        case MAP_SETONLY:
            
            
            return aRet;
    }
    return aRet;
}



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
        
        sal_Int32 nMissingSize = iNextIndex - nLen + 1;
        sal_Int32 nSteps = nMissingSize / ARRAY_SIZE_STEP + 1;
        sal_Int32 nNewSize = nLen + nSteps * ARRAY_SIZE_STEP;

        rSeq.realloc( nNewSize );
        rpInterfaceArray = rSeq.getArray();
    }
}









class ImplIntrospectionAccess : public IntrospectionAccessHelper
{
    friend class ImplIntrospection;

    
    Any maInspectedObject;

    
    Reference<XInterface> mxIface;

    
    rtl::Reference< IntrospectionAccessStatic_Impl > mpStaticImpl;

    
    WeakReference< XInterface > maAdapter;

    
    Sequence<Property> maLastPropertySeq;
    sal_Int32 mnLastPropertyConcept;

    
    Sequence<Reference<XIdlMethod> > maLastMethodSeq;
    sal_Int32 mnLastMethodConcept;

public:
    ImplIntrospectionAccess( const Any& obj, rtl::Reference< IntrospectionAccessStatic_Impl > const & pStaticImpl_ );
    ~ImplIntrospectionAccess();

    
    virtual sal_Int32 SAL_CALL getSuppliedMethodConcepts(void)
        throw( RuntimeException );
    virtual sal_Int32 SAL_CALL getSuppliedPropertyConcepts(void)
        throw( RuntimeException );
    virtual Property SAL_CALL getProperty(const OUString& Name, sal_Int32 PropertyConcepts)
        throw( NoSuchElementException, RuntimeException );
    virtual sal_Bool SAL_CALL hasProperty(const OUString& Name, sal_Int32 PropertyConcepts)
        throw( RuntimeException );
    virtual Sequence< Property > SAL_CALL getProperties(sal_Int32 PropertyConcepts)
          throw( RuntimeException );
    virtual Reference<XIdlMethod> SAL_CALL getMethod(const OUString& Name, sal_Int32 MethodConcepts)
          throw( NoSuchMethodException, RuntimeException );
    virtual sal_Bool SAL_CALL hasMethod(const OUString& Name, sal_Int32 MethodConcepts)
          throw( RuntimeException );
    virtual Sequence< Reference<XIdlMethod> > SAL_CALL getMethods(sal_Int32 MethodConcepts)
          throw( RuntimeException );
    virtual Sequence< Type > SAL_CALL getSupportedListeners(void)
          throw( RuntimeException );
    using OWeakObject::queryAdapter;
    virtual Reference<XInterface> SAL_CALL queryAdapter( const Type& rType )
          throw( IllegalTypeException, RuntimeException );

    
    virtual Any SAL_CALL getMaterial(void) throw(RuntimeException);

    
    virtual OUString SAL_CALL getExactName( const OUString& rApproximateName ) throw( RuntimeException );
};

ImplIntrospectionAccess::ImplIntrospectionAccess
    ( const Any& obj, rtl::Reference< IntrospectionAccessStatic_Impl > const & pStaticImpl_ )
        : maInspectedObject( obj ), mpStaticImpl( pStaticImpl_ ), maAdapter()
{
    
    TypeClass eType = maInspectedObject.getValueType().getTypeClass();
    if( eType == TypeClass_INTERFACE )
        mxIface = *(Reference<XInterface>*)maInspectedObject.getValue();

    mnLastPropertyConcept = -1;
    mnLastMethodConcept = -1;
}

ImplIntrospectionAccess::~ImplIntrospectionAccess()
{
}









class ImplIntrospectionAdapter :
    public XPropertySet, public XFastPropertySet, public XPropertySetInfo,
    public XNameContainer, public XIndexContainer,
    public XEnumerationAccess, public  XIdlArray,
    public OWeakObject
{
    
    ::rtl::Reference< ImplIntrospectionAccess > mpAccess;

    
    const Any& mrInspectedObject;

    
    rtl::Reference< IntrospectionAccessStatic_Impl > mpStaticImpl;

    
    Reference<XInterface> mxIface;

    
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

    
    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    virtual void        SAL_CALL release() throw() { OWeakObject::release(); }

    
    virtual Reference<XPropertySetInfo> SAL_CALL getPropertySetInfo() throw( RuntimeException );
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException );
    virtual Any SAL_CALL getPropertyValue(const OUString& aPropertyName)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );

    
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException );
    virtual Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException );

    
    virtual Sequence< Property > SAL_CALL getProperties(void) throw( RuntimeException );
    virtual Property SAL_CALL getPropertyByName(const OUString& Name) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& Name) throw( RuntimeException );

    
    virtual Type SAL_CALL getElementType(void) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasElements(void) throw( RuntimeException );

    
    virtual Any SAL_CALL getByName(const OUString& Name)
        throw( NoSuchElementException, WrappedTargetException, RuntimeException );
    virtual Sequence< OUString > SAL_CALL getElementNames(void) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( RuntimeException );

    
    virtual void SAL_CALL insertByName(const OUString& Name, const Any& Element)
        throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL replaceByName(const OUString& Name, const Any& Element)
        throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeByName(const OUString& Name)
        throw( NoSuchElementException, WrappedTargetException, RuntimeException );

    
    virtual sal_Int32 SAL_CALL getCount(void) throw( RuntimeException );
    virtual Any SAL_CALL getByIndex(sal_Int32 Index)
        throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    
    virtual void SAL_CALL insertByIndex(sal_Int32 Index, const Any& Element)
        throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL replaceByIndex(sal_Int32 Index, const Any& Element)
        throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeByIndex(sal_Int32 Index)
        throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    
    virtual Reference<XEnumeration> SAL_CALL createEnumeration(void) throw( RuntimeException );

    
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







Reference<XPropertySetInfo> ImplIntrospectionAdapter::getPropertySetInfo(void)
    throw( RuntimeException )
{
    return (XPropertySetInfo *)this;
}

void ImplIntrospectionAdapter::setPropertyValue(const OUString& aPropertyName, const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    mpStaticImpl->setPropertyValue( mrInspectedObject, aPropertyName, aValue );
}

Any ImplIntrospectionAdapter::getPropertyValue(const OUString& aPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    return mpStaticImpl->getPropertyValue( mrInspectedObject, aPropertyName );
}

void ImplIntrospectionAdapter::addPropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    if( mxIface.is() )
    {
        Reference<XPropertySet> xPropSet =
            Reference<XPropertySet>::query( mxIface );
        
        if( xPropSet.is() )
            xPropSet->addPropertyChangeListener(aPropertyName, aListener);
    }
}

void ImplIntrospectionAdapter::removePropertyChangeListener(const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    if( mxIface.is() )
    {
        Reference<XPropertySet> xPropSet =
            Reference<XPropertySet>::query( mxIface );
        
        if( xPropSet.is() )
            xPropSet->removePropertyChangeListener(aPropertyName, aListener);
    }
}

void ImplIntrospectionAdapter::addVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    if( mxIface.is() )
    {
        Reference<XPropertySet> xPropSet =
            Reference<XPropertySet>::query( mxIface );
        
        if( xPropSet.is() )
            xPropSet->addVetoableChangeListener(aPropertyName, aListener);
    }
}

void ImplIntrospectionAdapter::removeVetoableChangeListener(const OUString& aPropertyName, const Reference<XVetoableChangeListener>& aListener)
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



void ImplIntrospectionAdapter::setFastPropertyValue(sal_Int32, const Any&)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
}

Any ImplIntrospectionAdapter::getFastPropertyValue(sal_Int32)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    return Any();
}


Sequence< Property > ImplIntrospectionAdapter::getProperties(void) throw( RuntimeException )
{
    return mpStaticImpl->getProperties();
}

Property ImplIntrospectionAdapter::getPropertyByName(const OUString& Name)
    throw( RuntimeException )
{
    return mpAccess->getProperty( Name, PropertyConcept::ALL );
}

sal_Bool ImplIntrospectionAdapter::hasPropertyByName(const OUString& Name)
    throw( RuntimeException )
{
    return mpAccess->hasProperty( Name, PropertyConcept::ALL );
}


Type ImplIntrospectionAdapter::getElementType(void) throw( RuntimeException )
{
    return mxObjElementAccess->getElementType();
}

sal_Bool ImplIntrospectionAdapter::hasElements(void) throw( RuntimeException )
{
    return mxObjElementAccess->hasElements();
}


Any ImplIntrospectionAdapter::getByName(const OUString& Name)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    return mxObjNameAccess->getByName( Name );
}

Sequence< OUString > ImplIntrospectionAdapter::getElementNames(void)
    throw( RuntimeException )
{
    return mxObjNameAccess->getElementNames();
}

sal_Bool ImplIntrospectionAdapter::hasByName(const OUString& Name)
    throw( RuntimeException )
{
    return mxObjNameAccess->hasByName( Name );
}


void ImplIntrospectionAdapter::insertByName(const OUString& Name, const Any& Element)
    throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException )
{
    mxObjNameContainer->insertByName( Name, Element );
}

void ImplIntrospectionAdapter::replaceByName(const OUString& Name, const Any& Element)
    throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException )
{
    mxObjNameContainer->replaceByName( Name, Element );
}

void ImplIntrospectionAdapter::removeByName(const OUString& Name)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    mxObjNameContainer->removeByName( Name );
}



sal_Int32 ImplIntrospectionAdapter::getCount(void) throw( RuntimeException )
{
    return mxObjIndexAccess->getCount();
}

Any ImplIntrospectionAdapter::getByIndex(sal_Int32 Index)
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    return mxObjIndexAccess->getByIndex( Index );
}


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



Reference<XEnumeration> ImplIntrospectionAdapter::createEnumeration(void) throw( RuntimeException )
{
    return mxObjEnumerationAccess->createEnumeration();
}


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

Property ImplIntrospectionAccess::getProperty(const OUString& Name, sal_Int32 PropertyConcepts)
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

sal_Bool ImplIntrospectionAccess::hasProperty(const OUString& Name, sal_Int32 PropertyConcepts)
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
    
    sal_Int32 nAllSupportedMask =    PROPERTYSET |
                                    ATTRIBUTES |
                                    METHODS;
    if( ( PropertyConcepts & nAllSupportedMask ) == nAllSupportedMask )
    {
        return mpStaticImpl->getProperties();
    }

    
    if( mnLastPropertyConcept == PropertyConcepts )
    {
        return maLastPropertySeq;
    }

    
    sal_Int32 nCount = 0;

    
    
    
    if( PropertyConcepts & PROPERTYSET )
        nCount += mpStaticImpl->mnPropertySetPropCount;
    if( PropertyConcepts & ATTRIBUTES )
        nCount += mpStaticImpl->mnAttributePropCount;
    if( PropertyConcepts & METHODS )
        nCount += mpStaticImpl->mnMethodPropCount;

    
    ImplIntrospectionAccess* pThis = (ImplIntrospectionAccess*)this;    
    pThis->maLastPropertySeq.realloc( nCount );
    Property* pDestProps = pThis->maLastPropertySeq.getArray();

    
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

    
    pThis->mnLastPropertyConcept = PropertyConcepts;

    
    return maLastPropertySeq;
}

Reference<XIdlMethod> ImplIntrospectionAccess::getMethod(const OUString& Name, sal_Int32 MethodConcepts)
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

sal_Bool ImplIntrospectionAccess::hasMethod(const OUString& Name, sal_Int32 MethodConcepts)
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
    ImplIntrospectionAccess* pThis = (ImplIntrospectionAccess*)this;    

    
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

    
    if( mnLastMethodConcept == MethodConcepts )
    {
        return maLastMethodSeq;
    }

    
    Sequence< Reference<XIdlMethod> > aMethodSeq = mpStaticImpl->getMethods();
    const Reference<XIdlMethod>* pSourceMethods = aMethodSeq.getConstArray();
    const sal_Int32* pConcepts = mpStaticImpl->getMethodConcepts().getConstArray();
    sal_Int32 nLen = aMethodSeq.getLength();

    
    
    
    
    pThis->maLastMethodSeq.realloc( nLen );
    Reference<XIdlMethod>* pDestMethods = pThis->maLastMethodSeq.getArray();

    
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

    
    pThis->maLastMethodSeq.realloc( iDest );

    
    pThis->mnLastMethodConcept = MethodConcepts;

    
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


Any ImplIntrospectionAccess::getMaterial(void) throw(RuntimeException)
{
    return maInspectedObject;
}


OUString toLower( OUString aUStr )
{
    
    OUString aOWStr( aUStr.getStr() );
    OUString aOWLowerStr = aOWStr.toAsciiLowerCase();
    OUString aLowerUStr( aOWLowerStr.getStr() );
    return aLowerUStr;
}


OUString ImplIntrospectionAccess::getExactName( const OUString& rApproximateName ) throw( RuntimeException )
{
    OUString aRetStr;
    LowerToExactNameMap::iterator aIt =
        mpStaticImpl->maLowerToExactNameMap.find( toLower( rApproximateName ) );
    if( !( aIt == mpStaticImpl->maLowerToExactNameMap.end() ) )
        aRetStr = (*aIt).second;
    return aRetStr;
}




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
            return false;

        sal_Int32 nCount1 = rObj1.aIdlClasses.getLength();
        sal_Int32 nCount2 = rObj2.aIdlClasses.getLength();
        if( nCount1 != nCount2 )
            return false;

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
            return false;

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

    
    sal_Int32 nId32 = 0;
    sal_Int32 i;
    for( i = 0 ; i < nCount32 ; i++ )
        nId32 ^= *(pBytesAsInt32Array++);

    
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





struct OIntrospectionMutex
{
    Mutex                            m_mutex;
};

class ImplIntrospection : public XIntrospection
                        , public XServiceInfo
                        , public OIntrospectionMutex
                        , public OComponentHelper
{
    
    rtl::Reference< IntrospectionAccessStatic_Impl > implInspect(const Any& aToInspectObj);

    
    Reference<XMultiServiceFactory> m_xSMgr;

    
    Reference< XIdlReflection > mxCoreReflection;

    
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

    
    virtual Any            SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw() { OComponentHelper::acquire(); }
    virtual void        SAL_CALL release() throw() { OComponentHelper::release(); }

    
    Sequence< Type >    SAL_CALL getTypes(  ) throw( RuntimeException );
    Sequence<sal_Int8>    SAL_CALL getImplementationId(  ) throw( RuntimeException );

    
    OUString                     SAL_CALL getImplementationName() throw();
    sal_Bool                    SAL_CALL supportsService(const OUString& ServiceName) throw();
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw();
    static OUString SAL_CALL    getImplementationName_Static(  );
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(void) throw();

    
    virtual Reference<XIntrospectionAccess> SAL_CALL inspect(const Any& aToInspectObj)
                throw( RuntimeException );

protected:
    
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
};

enum MethodType
{
    STANDARD_METHOD,            
    GETSET_METHOD,                
    ADD_LISTENER_METHOD,        
    REMOVE_LISTENER_METHOD,        
    INVALID_METHOD                
};


ImplIntrospection::ImplIntrospection( const Reference<XMultiServiceFactory> & rXSMgr )
    : OComponentHelper( m_mutex )
    , m_xSMgr( rXSMgr )
{
    mnCacheEntryCount = 0;
    mnTPCacheEntryCount = 0;
    mpCache = NULL;
    mpTypeProviderCache = NULL;

    Reference< XPropertySet > xProps( rXSMgr, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    if (xProps.is())
    {
        Reference< XComponentContext > xContext;
        xProps->getPropertyValue(
            OUString("DefaultContext") ) >>= xContext;
        OSL_ASSERT( xContext.is() );
        if (xContext.is())
        {
            mxCoreReflection = css::reflection::theCoreReflection::get(
                xContext);
        }
    }
    if (! mxCoreReflection.is())
    {
        throw DeploymentException(
            OUString("/singletons/com.sun.star.reflection.theCoreReflection singleton not accessible"),
            Reference< XInterface >() );
    }

    mxElementAccessClass = mxCoreReflection->forName("com.sun.star.container.XElementAccess");
    mxNameContainerClass = mxCoreReflection->forName("com.sun.star.container.XNameContainer");
    mxNameAccessClass = mxCoreReflection->forName("com.sun.star.container.XNameAccess");
    mxIndexContainerClass = mxCoreReflection->forName("com.sun.star.container.XIndexContainer");
    mxIndexAccessClass = mxCoreReflection->forName("com.sun.star.container.XIndexAccess");
    mxEnumerationAccessClass = mxCoreReflection->forName("com.sun.star.container.XEnumerationAccess");
    mxInterfaceClass = mxCoreReflection->forName("com.sun.star.uno.XInterface");
    mxAggregationClass = mxCoreReflection->forName("com.sun.star.uno.XAggregation");
    mbDisposed = sal_False;
}


void ImplIntrospection::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    OComponentHelper::dispose();

    
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





Any ImplIntrospection::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface(
        rType,
        static_cast< XIntrospection * >( this ),
        static_cast< XServiceInfo * >( this ) ) );

    return (aRet.hasValue() ? aRet : OComponentHelper::queryInterface( rType ));
}


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



OUString ImplIntrospection::getImplementationName() throw()
{
    return getImplementationName_Static();
}


sal_Bool ImplIntrospection::supportsService(const OUString& ServiceName) throw()
{
    return cppu::supportsService(this, ServiceName);
}


Sequence< OUString > ImplIntrospection::getSupportedServiceNames(void) throw()
{
    return getSupportedServiceNames_Static();
}



OUString ImplIntrospection::getImplementationName_Static(  )
{
    return OUString::createFromAscii( IMPLEMENTATION_NAME );
}


Sequence< OUString > ImplIntrospection::getSupportedServiceNames_Static(void) throw()
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
    return aSNS;
}




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




Reference<XIdlClass> TypeToIdlClass( const Type& rType, const Reference< XIdlReflection > & xRefl )
{
    
    Reference<XIdlClass> xRetClass;
    typelib_TypeDescription * pTD = 0;
    rType.getDescription( &pTD );
    if( pTD )
    {
        OUString sOWName( pTD->pTypeName );
        xRetClass = xRefl->forName( sOWName );
    }
    return xRetClass;
}


rtl::Reference< IntrospectionAccessStatic_Impl > ImplIntrospection::implInspect(const Any& aToInspectObj)
{
    MutexGuard aGuard( m_mutex );

    
    if( mbDisposed )
        return NULL;

    
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE && eType != TypeClass_STRUCT  && eType != TypeClass_EXCEPTION )
        return NULL;

    Reference<XInterface> x;
    if( eType == TypeClass_INTERFACE )
    {
        
        x = *(Reference<XInterface>*)aToInspectObj.getValue();
        if( !x.is() )
            return NULL;
    }

    
    if( !mpCache )
        mpCache = new IntrospectionAccessCacheMap;
    if( !mpTypeProviderCache )
        mpTypeProviderCache = new TypeProviderAccessCacheMap;
    IntrospectionAccessCacheMap& aCache = *mpCache;
    TypeProviderAccessCacheMap& aTPCache = *mpTypeProviderCache;

    
    rtl::Reference< IntrospectionAccessStatic_Impl > pAccess;

    
    Sequence< Reference<XIdlClass> >    SupportedClassSeq;
    Sequence< Type >                    SupportedTypesSeq;
    Reference<XTypeProvider>            xTypeProvider;
    Reference<XIdlClass>                xImplClass;
    Reference<XPropertySetInfo>            xPropSetInfo;
    Reference<XPropertySet>                xPropSet;

    
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
                    pClasses[ i ] = TypeToIdlClass( pTypes[ i ], mxCoreReflection );
                }
                
            }
        }
        else
        {
            xImplClass = TypeToIdlClass( aToInspectObj.getValueType(), mxCoreReflection );
            SupportedClassSeq.realloc( 1 );
            SupportedClassSeq.getArray()[ 0 ] = xImplClass;
        }

        xPropSet = Reference<XPropertySet>::query( x );
        
        if( xPropSet.is() )
            xPropSetInfo = xPropSet->getPropertySetInfo();
    }
    else
    {
        xImplClass = TypeToIdlClass( aToInspectObj.getValueType(), mxCoreReflection );
    }

    if( xTypeProvider.is() )
    {
        Sequence< sal_Int8 > aImpIdSeq = xTypeProvider->getImplementationId();
        sal_Int32 nIdLen = aImpIdSeq.getLength();

        if( nIdLen )
        {
            
            hashTypeProviderKey_Impl aKeySeq( xPropSetInfo, aImpIdSeq );

            TypeProviderAccessCacheMap::iterator aIt = aTPCache.find( aKeySeq );
            if( aIt == aTPCache.end() )
            {
                
                
                pAccess = new IntrospectionAccessStatic_Impl( mxCoreReflection );

                
                if( mnTPCacheEntryCount > INTROSPECTION_CACHE_MAX_SIZE )
                {
                    
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

                
                aKeySeq.nHitCount = 1;
                aTPCache[ aKeySeq ] = pAccess;

            }
            else
            {
                
                (*aIt).first.IncHitCount();
                return (*aIt).second;
            }
        }
    }
    else if( xImplClass.is() )
    {
        
        hashIntrospectionKey_Impl    aKeySeq( SupportedClassSeq, xPropSetInfo, xImplClass );

        IntrospectionAccessCacheMap::iterator aIt = aCache.find( aKeySeq );
        if( aIt == aCache.end() )
        {
            
            
            pAccess = new IntrospectionAccessStatic_Impl( mxCoreReflection );

            
            if( mnCacheEntryCount > INTROSPECTION_CACHE_MAX_SIZE )
            {
                
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

            
            aKeySeq.nHitCount = 1;
            aCache[ aKeySeq ] = pAccess;

        }
        else
        {
            
            (*aIt).first.IncHitCount();
            return (*aIt).second;
        }
    }

    
    Property* pAllPropArray;
    Reference<XInterface>* pInterfaces1;
    Reference<XInterface>* pInterfaces2;
    sal_Int16* pMapTypeArray;
    sal_Int32* pPropertyConceptArray;
    sal_Int32 i;

    if( !pAccess.is() )
        pAccess = new IntrospectionAccessStatic_Impl( mxCoreReflection );

    
    sal_Int32& rPropCount = pAccess->mnPropCount;
    IntrospectionNameMap& rPropNameMap = pAccess->maPropertyNameMap;
    IntrospectionNameMap& rMethodNameMap = pAccess->maMethodNameMap;
    LowerToExactNameMap& rLowerToExactNameMap = pAccess->maLowerToExactNameMap;

    
    pAllPropArray = pAccess->maAllPropertySeq.getArray();
    pInterfaces1 = pAccess->aInterfaceSeq1.getArray();
    pInterfaces2 = pAccess->aInterfaceSeq2.getArray();
    pMapTypeArray = pAccess->maMapTypeSeq.getArray();
    pPropertyConceptArray = pAccess->maPropertyConceptSeq.getArray();

    
    
    
    if( eType == TypeClass_INTERFACE )
    {
        
        

        
        if( xPropSet.is() && xPropSetInfo.is() )
        {
            
            Reference<XFastPropertySet> xDummy = Reference<XFastPropertySet>::query( x );
            sal_Bool bFast = pAccess->mbFastPropSet = xDummy.is();

            Sequence<Property> aPropSeq = xPropSetInfo->getProperties();
            const Property* pProps = aPropSeq.getConstArray();
            sal_Int32 nLen = aPropSeq.getLength();

            
            if( bFast )
                pAccess->mpOrgPropertyHandleArray = new sal_Int32[ nLen ];

            for( i = 0 ; i < nLen ; i++ )
            {
                
                pAccess->checkPropertyArraysSize
                    ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );
                Property& rProp = pAllPropArray[ rPropCount ];
                rProp = pProps[ i ];

                if( bFast )
                    pAccess->mpOrgPropertyHandleArray[ i ] = rProp.Handle;

                
                rProp.Handle = rPropCount;

                
                pMapTypeArray[ rPropCount ] = MAP_PROPERTY_SET;
                pPropertyConceptArray[ rPropCount ] = PROPERTYSET;
                pAccess->mnPropertySetPropCount++;

                
                OUString aPropName = rProp.Name;

                
                IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                if( aIt == rPropNameMap.end() )
                {
                    
                    rPropNameMap[ aPropName ] = rPropCount;

                    
                    rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;
                }
                else
                {
                    SAL_WARN( "stoc", "Introspection: Property \"" <<
                        aPropName << "\" found more than once in PropertySet" );
                }

                
                rPropCount++;
            }
        }

        
        sal_Int32 iAllExportedMethod = 0;
        sal_Int32 iAllSupportedListener = 0;

        
        CheckedInterfacesMap aCheckedInterfacesMap;

        
        
        sal_Bool bXInterfaceIsInvalid = sal_False;

        
        
        
        sal_Bool bFoundXInterface = sal_False;

        sal_Int32 nClassCount = SupportedClassSeq.getLength();
        for( sal_Int32 nIdx = 0 ; nIdx < nClassCount; nIdx++ )
        {
            Reference<XIdlClass> xImplClass2 = SupportedClassSeq.getConstArray()[nIdx];
            while( xImplClass2.is() )
            {
                
                Sequence< Reference<XIdlClass> > aClassSeq = xImplClass2->getInterfaces();
                sal_Int32 nIfaceCount = aClassSeq.getLength();

                aClassSeq.realloc( nIfaceCount + 1 );
                aClassSeq.getArray()[ nIfaceCount ] = xImplClass2;
                nIfaceCount++;

                const Reference<XIdlClass>* pParamArray = aClassSeq.getConstArray();

                for( sal_Int32 j = 0 ; j < nIfaceCount ; j++ )
                {
                    const Reference<XIdlClass>& rxIfaceClass = pParamArray[j];

                    
                    XInterface* pIface = ( static_cast< XInterface* >( rxIfaceClass.get() ) );
                    if( aCheckedInterfacesMap.count( pIface ) > 0 )
                    {
                        
                        continue;
                    }
                    else
                    {
                        
                        aCheckedInterfacesMap[ pIface ] = pIface;
                    }

                    

                    

                    
                    Sequence< Reference<XIdlField> > fields = rxIfaceClass->getFields();
                    const Reference<XIdlField>* pFields = fields.getConstArray();
                    sal_Int32 nLen = fields.getLength();

                    for( i = 0 ; i < nLen ; i++ )
                    {
                        Reference<XIdlField> xField = pFields[i];
                        Reference<XIdlClass> xPropType = xField->getType();

                        
                        pAccess->checkPropertyArraysSize
                            ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );

                        
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

                        
                        OUString aPropName = rProp.Name;

                        
                        IntrospectionNameMap::iterator aIt = rPropNameMap.find( aPropName );
                        if( !( aIt == rPropNameMap.end() ) )
                            continue;

                        
                        rPropNameMap[ aPropName ] = rPropCount;

                        
                        rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;

                        
                        pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                                                          pInterfaces1, rPropCount );
                        pInterfaces1[ rPropCount ] = xField;

                        
                        pMapTypeArray[ rPropCount ] = MAP_FIELD;
                        pPropertyConceptArray[ rPropCount ] = ATTRIBUTES;
                        pAccess->mnAttributePropCount++;

                        
                        rPropCount++;
                    }

                    

                    

                    
                    sal_Int32 nListenerCount = 0;

                    
                    Sequence< Reference<XIdlMethod> > methods = rxIfaceClass->getMethods();
                    const Reference<XIdlMethod>* pSourceMethods = methods.getConstArray();
                    sal_Int32 nSourceMethodCount = methods.getLength();

                    

                    
                    
                    
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
                        
                        const Reference<XIdlMethod>& rxMethod_i = pSourceMethods[i];
                        sal_Int32& rMethodConcept_i = pLocalMethodConcepts[ i ];

                        
                        aMethName = rxMethod_i->getName();

                        
                        
                        
                        if( rxMethod_i->getDeclaringClass()->equals( mxInterfaceClass ) )
                        {
                            
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

                        
                        if( aMethName.getLength() <= 3 )
                            continue;

                        
                        aStartStr = aMethName.copy( 0, 3 );
                        if( aStartStr == "get" )
                        {
                            
                            aPropName = aMethName.copy( 3 );

                            
                            Sequence< Reference<XIdlClass> > getParams = rxMethod_i->getParameterTypes();
                            if( getParams.getLength() > 0 )
                            {
                                continue;
                            }

                            
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

                            
                            rMethodConcept_i |= PROPERTY;

                            pMethodTypes[i] = GETSET_METHOD;
                            Reference<XIdlClass> xGetRetType = rxMethod_i->getReturnType();

                            
                            pAccess->checkPropertyArraysSize
                                ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );

                            
                            Property& rProp = pAllPropArray[ rPropCount ];
                            rProp.Name = aPropName;
                            rProp.Handle = rPropCount;
                            rProp.Type = Type( xGetRetType->getTypeClass(), xGetRetType->getName() );
                            rProp.Attributes = READONLY;

                            
                            rPropNameMap[ aPropName ] = rPropCount;

                            
                            rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;

                            
                            pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                                                              pInterfaces1, rPropCount );
                            pInterfaces1[ rPropCount ] = rxMethod_i;

                            
                            pMapTypeArray[ rPropCount ] = MAP_GETSET;
                            pPropertyConceptArray[ rPropCount ] = METHODS;
                            pAccess->mnMethodPropCount++;

                            
                            sal_Int32 k;
                            for( k = 0 ; k < nSourceMethodCount ; k++ )
                            {
                                
                                const Reference<XIdlMethod>& rxMethod_k = pSourceMethods[k];

                                
                                if( k == i || pMethodTypes[k] != STANDARD_METHOD )
                                    continue;

                                
                                OUString aMethName2 = rxMethod_k->getName();
                                OUString aStartStr2 = aMethName2.copy( 0, 3 );
                                
                                if( !( aStartStr2 == "set" ) )
                                    continue;

                                
                                OUString aPropName2 = aMethName2.copy( 3 );
                                
                                if( !( aPropName == aPropName2 ) )
                                    continue;

                                
                                Reference<XIdlClass> xSetRetType = rxMethod_k->getReturnType();
                                if( xSetRetType->getTypeClass() != TypeClass_VOID )
                                {
                                    continue;
                                }

                                
                                Sequence< Reference<XIdlClass> > setParams = rxMethod_k->getParameterTypes();
                                sal_Int32 nParamCount = setParams.getLength();
                                if( nParamCount != 1 )
                                {
                                    continue;
                                }

                                
                                const Reference<XIdlClass>* pParamArray2 = setParams.getConstArray();
                                Reference<XIdlClass> xParamType = pParamArray2[ 0 ];
                                if( xParamType->equals( xGetRetType ) )
                                {
                                    pLocalMethodConcepts[ k ] = PROPERTY;

                                    pMethodTypes[k] = GETSET_METHOD;

                                    
                                    rProp.Attributes &= ~READONLY;

                                    
                                    pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq2,
                                                                      pInterfaces2, rPropCount );
                                    pInterfaces2[ rPropCount ] = rxMethod_k;
                                }
                            }

                            
                            rPropCount++;
                        }

                        
                        else if( aStartStr == "add" )
                        {
                            OUString aListenerStr( "Listener" );

                            
                            sal_Int32 nStrLen = aMethName.getLength();
                            sal_Int32 nCopyLen = nStrLen - aListenerStr.getLength();
                            OUString aEndStr = aMethName.copy( nCopyLen > 0 ? nCopyLen : 0 );

                            
                            
                            if( !( aEndStr == aListenerStr ) )
                                continue;

                            
                            OUString aListenerName = aMethName.copy( 3, nStrLen - aListenerStr.getLength() - 3 );

                            
                            
                            


                            
                            sal_Int32 k;
                            for( k = 0 ; k < nSourceMethodCount ; k++ )
                            {
                                
                                const Reference<XIdlMethod>& rxMethod_k = pSourceMethods[k];

                                
                                if( k == i || pMethodTypes[k] != STANDARD_METHOD )
                                    continue;

                                
                                OUString aMethName2 = rxMethod_k->getName();
                                sal_Int32 nNameLen = aMethName2.getLength();
                                sal_Int32 nCopyLen2 = (nNameLen < 6) ? nNameLen : 6;
                                OUString aStartStr2 = aMethName2.copy( 0, nCopyLen2 );
                                OUString aRemoveStr("remove" );
                                
                                if( !( aStartStr2 == aRemoveStr ) )
                                    continue;

                                
                                if( aMethName2.getLength() - aRemoveStr.getLength() <= aListenerStr.getLength() )
                                    continue;
                                OUString aListenerName2 = aMethName2.copy
                                      ( 6, aMethName2.getLength() - aRemoveStr.getLength() - aListenerStr.getLength() );
                                
                                if( !( aListenerName == aListenerName2 ) )
                                    continue;

                                
                                
                                


                                
                                rMethodConcept_i |= LISTENER;
                                pLocalMethodConcepts[ k ] |= LISTENER;

                                pMethodTypes[i] = ADD_LISTENER_METHOD;
                                pMethodTypes[k] = REMOVE_LISTENER_METHOD;
                                nListenerCount++;
                            }
                        }
                    }


                    
                    
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        
                        const Reference<XIdlMethod>& rxMethod_i = pSourceMethods[i];

                        
                        if( pMethodTypes[i] != STANDARD_METHOD )
                            continue;

                        
                        aMethName = rxMethod_i->getName();

                        
                        if( aMethName.getLength() <= 3 )
                            continue;

                        
                        aStartStr = aMethName.copy( 0, 3 );
                        if( aStartStr == "set" )
                        {
                            
                            aPropName = aMethName.copy( 3 );

                            
                            Reference<XIdlClass> xSetRetType = rxMethod_i->getReturnType();
                            if( xSetRetType->getTypeClass() != TypeClass_VOID )
                            {
                                continue;
                            }

                            
                            Sequence< Reference<XIdlClass> > setParams = rxMethod_i->getParameterTypes();
                            sal_Int32 nParamCount = setParams.getLength();
                            if( nParamCount != 1 )
                            {
                                continue;
                            }

                            
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

                            
                            pLocalMethodConcepts[ i ] = PROPERTY;

                            pMethodTypes[i] = GETSET_METHOD;
                            Reference<XIdlClass> xGetRetType = setParams.getConstArray()[0];

                            
                            pAccess->checkPropertyArraysSize
                                ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );

                            
                            Property& rProp = pAllPropArray[ rPropCount ];
                            rProp.Name = aPropName;
                            rProp.Handle = rPropCount;
                            rProp.Type = Type( xGetRetType->getTypeClass(), xGetRetType->getName() );
                            rProp.Attributes = 0;    

                            
                            rPropNameMap[ aPropName ] = rPropCount;

                            
                            rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;

                            
                            pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq2,
                                                              pInterfaces2, rPropCount );
                            pInterfaces2[ rPropCount ] = rxMethod_i;

                            
                            pMapTypeArray[ rPropCount ] = MAP_SETONLY;
                            pPropertyConceptArray[ rPropCount ] = METHODS;
                            pAccess->mnMethodPropCount++;

                            
                            rPropCount++;
                        }
                    }


                    

                    

                    
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

                    
                    pAccess->maAllMethodSeq.realloc( nExportedMethodCount + iAllExportedMethod );
                    pAccess->maMethodConceptSeq.realloc( nExportedMethodCount + iAllExportedMethod );
                    pAccess->maSupportedListenerSeq.realloc( nSupportedListenerCount + iAllSupportedListener );

                    
                    Reference<XIdlMethod>* pDestMethods = pAccess->maAllMethodSeq.getArray();
                    sal_Int32* pMethodConceptArray = pAccess->maMethodConceptSeq.getArray();
                    Type* pListenerClassRefs = pAccess->maSupportedListenerSeq.getArray();
                    for( i = 0 ; i < nSourceMethodCount ; i++ )
                    {
                        if( pMethodTypes[ i ] != INVALID_METHOD )
                        {
                            
                            const Reference<XIdlMethod>& rxMethod = pSourceMethods[i];

                            
                            OUString aMethName2 = rxMethod->getName();
                            IntrospectionNameMap::iterator aIt = rMethodNameMap.find( aMethName2 );
                            if( aIt == rMethodNameMap.end() )
                            {
                                
                                rMethodNameMap[ aMethName2 ] = iAllExportedMethod;

                                
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

                            
                            sal_Int32& rMethodConcept_i = pLocalMethodConcepts[ i ];
                            if( !rMethodConcept_i )
                                rMethodConcept_i = MethodConcept_NORMAL_IMPL;
                            pMethodConceptArray[ iAllExportedMethod ] = rMethodConcept_i;
                            iAllExportedMethod++;
                        }
                        if( pMethodTypes[ i ] == ADD_LISTENER_METHOD )
                        {
                            
                            const Reference<XIdlMethod>& rxMethod = pSourceMethods[i];

                            
                            Reference<XIdlClass> xListenerClass = TypeToIdlClass( getCppuVoidType(), mxCoreReflection );
                            

                            
                            
                            Sequence< Reference<XIdlClass> > aParams = rxMethod->getParameterTypes();
                            const Reference<XIdlClass>* pParamArray2 = aParams.getConstArray();

                            Reference<XIdlClass> xEventListenerClass = TypeToIdlClass( getCppuType( (Reference<XEventListener>*) NULL ), mxCoreReflection );
                            
                            sal_Int32 nParamCount = aParams.getLength();
                            sal_Int32 k;
                            for( k = 0 ; k < nParamCount ; k++ )
                            {
                                const Reference<XIdlClass>& rxClass = pParamArray2[k];

                                
                                if( rxClass->equals( xEventListenerClass ) ||
                                    isDerivedFrom( rxClass, xEventListenerClass ) )
                                {
                                    xListenerClass = rxClass;
                                    break;
                                }
                            }

                            
                            
                            
                            
                            
                            Type aListenerType( TypeClass_INTERFACE, xListenerClass->getName() );
                            pListenerClassRefs[ iAllSupportedListener ] = aListenerType;
                            iAllSupportedListener++;
                        }
                    }

                    
                    
                    if( bFoundXInterface )
                        bXInterfaceIsInvalid = sal_True;

                    delete[] pMethodTypes;
                    delete[] pLocalMethodConcepts;
                }

                
                Sequence< Reference<XIdlClass> > aSuperClassSeq = xImplClass2->getSuperclasses();

                
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

        
        
        
        sal_Int32& rMethCount = pAccess->mnMethCount;
        rMethCount = iAllExportedMethod;
        pAccess->maAllMethodSeq.realloc( rMethCount );
        pAccess->maMethodConceptSeq.realloc( rMethCount );

        
        pAccess->maAllPropertySeq.realloc( rPropCount );
        pAccess->maPropertyConceptSeq.realloc( rPropCount );
        pAccess->maMapTypeSeq.realloc( rPropCount );
    }
    
    else 
    {
        
        
        Reference<XIdlClass> xClassRef = TypeToIdlClass( aToInspectObj.getValueType(), mxCoreReflection );
        if( !xClassRef.is() )
        {
            SAL_WARN( "stoc", "Can't get XIdlClass from Reflection" );
            return pAccess;
        }

        
        Sequence< Reference<XIdlField> > fields = xClassRef->getFields();
        const Reference<XIdlField>* pFields = fields.getConstArray();
        sal_Int32 nLen = fields.getLength();

        for( i = 0 ; i < nLen ; i++ )
        {
            Reference<XIdlField> xField = pFields[i];
            Reference<XIdlClass> xPropType = xField->getType();
            OUString aPropName = xField->getName();

            
            pAccess->checkPropertyArraysSize
                ( pAllPropArray, pMapTypeArray, pPropertyConceptArray, rPropCount );

            
            Property& rProp = pAllPropArray[ rPropCount ];
            rProp.Name = aPropName;
            rProp.Handle = rPropCount;
            rProp.Type = Type( xPropType->getTypeClass(), xPropType->getName() );
            FieldAccessMode eAccessMode = xField->getAccessMode();
            rProp.Attributes = (eAccessMode == FieldAccessMode_READONLY ||
                                eAccessMode == FieldAccessMode_CONST)
                                ? READONLY : 0;

            
            
                

            
            rPropNameMap[ aPropName ] = rPropCount;

            
            rLowerToExactNameMap[ toLower( aPropName ) ] = aPropName;

            
            pAccess->checkInterfaceArraySize( pAccess->aInterfaceSeq1,
                pInterfaces1, rPropCount );
            pInterfaces1[ rPropCount ] = xField;

            
            pMapTypeArray[ rPropCount ] = MAP_FIELD;
            pPropertyConceptArray[ rPropCount ] = ATTRIBUTES;
            pAccess->mnAttributePropCount++;

            
            rPropCount++;
        }
    }

    
    pAccess->maAllPropertySeq.realloc( pAccess->mnPropCount );

    return pAccess;
}


Reference< XInterface > SAL_CALL ImplIntrospection_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr )
    throw( RuntimeException )
{
    Reference< XInterface > xService = (OWeakObject*)(OComponentHelper*)new ImplIntrospection( rSMgr );
    return xService;
}

}

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL introspection_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager,
    SAL_UNUSED_PARAMETER void * )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
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
