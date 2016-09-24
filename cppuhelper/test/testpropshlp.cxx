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

#if !defined(OSL_DEBUG_LEVEL) || OSL_DEBUG_LEVEL == 0
# undef OSL_DEBUG_LEVEL
# define OSL_DEBUG_LEVEL 2
#endif


#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>

#include <cppuhelper/proptypehlp.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <cppuhelper/implbase3.hxx>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

static Property * getPropertyTable1()
{
    static Property *pTable = 0;

    if( ! pTable )  {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pTable ) {
            static Property aTable[] =
            {
                Property( OUString("a"), 0, cppu::UnoType<OUString>::get() ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //OUString
                Property( OUString("b"), 1, cppu::UnoType<cppu::UnoCharType>::get() ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //Char
                Property( OUString("c"), 2, cppu::UnoType<sal_Int32>::get(),
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //sal_Int32
                Property( OUString("d"), 5, cppu::UnoType<double>::get()     ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //double
                Property( OUString("e"), 7, cppu::UnoType<bool>::get(),
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //BOOL
                Property( OUString("f"), 8, cppu::UnoType<Any>::get()        ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID )  //Any
            };
            pTable = aTable;
        }
    }
    return pTable;
}


static Property * getPropertyTable2()
{
    static Property *pTable = 0;

    if( ! pTable )  {
        MutexGuard guard(  ::osl::Mutex::getGlobalMutex() );
        if( ! pTable ) {
            static Property aTable[] =
            {
                Property( OUString("f"), 8, cppu::UnoType<Any>::get()   ,
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // Any
                Property( OUString("b"), 1, cppu::UnoType<cppu::UnoCharType>::get(),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // Char
                Property( OUString("a"), 0, cppu::UnoType<OUString>::get(),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // OUString
                Property( OUString("d"), 5, cppu::UnoType<double>::get() ,
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // Double
                Property( OUString("c"), 2, cppu::UnoType<sal_Int32>::get(),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // sal_Int32
                Property( OUString("e"), 7, cppu::UnoType<bool>::get(),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID )    // Bool
            };
            pTable = aTable;
        }
    }
    return pTable;
}

static Property * getPropertyTable3()
{
    static Property *pTable = 0;

    if( ! pTable )  {
        MutexGuard guard(  ::osl::Mutex::getGlobalMutex() );
        if( ! pTable ) {
            static Property aTable[] =
            {
                Property( OUString("b"), 1, cppu::UnoType<cppu::UnoCharType>::get(),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // Char
                Property( OUString("f"), 8, cppu::UnoType<Any>::get()   ,
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // any
                Property( OUString("a"), 0, cppu::UnoType<OUString>::get(),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID )    // OUString
            };
            pTable = aTable;
        }
    }
    return pTable;
}


static Property * getPropertyTable4()
{
    static Property *pTable = 0;

    if( ! pTable )  {
        MutexGuard guard(  ::osl::Mutex::getGlobalMutex() );
        if( ! pTable ) {
            static Property aTable[] =
            {
                Property( OUString("a"), 0, cppu::UnoType<OUString>::get(),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // OUString
                Property( OUString("b"), 1, cppu::UnoType<cppu::UnoCharType>::get(),
                                     PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), // Char
                Property( OUString("f"), 2, cppu::UnoType<Any>::get()   ,
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID )    // Any
            };
            pTable = aTable;
        }
    }
    return pTable;
}


/**********************
*
* Note : all Property names must be in the 127 ASCII subset !
*
**********************/


void test_PropertyArrayHelper()
{
    // Test getProperties() and getCount()
    {
        OPropertyArrayHelper a1( getPropertyTable1(), 6 );
        OSL_ENSURE( 6 == a1.getCount(), "not all properties inserted" );
        Sequence< Property > aProps = a1.getProperties();
        Property * pP = aProps.getArray();
        OSL_ENSURE( 6 == aProps.getLength(), "getProperties() gives not all properties" );
        for( int i = 0; i < 6; i++ )
        {
            OSL_ENSURE( pP[i].Name      == getPropertyTable1()[i].Name , "Name not correct" );
            OSL_ENSURE( pP[i].Handle        == getPropertyTable1()[i].Handle, "Handle not correct" );
            OSL_ENSURE( pP[i].Attributes    == getPropertyTable1()[i].Attributes, "Attributes not correct" );
            OSL_ENSURE( pP[i].Type == getPropertyTable1()[i].Type, "Type not correct" );
        }
    }

    // Test sorting
    {
          OPropertyArrayHelper a1( getPropertyTable2(), 6, sal_False );
          Sequence< Property > aProps = a1.getProperties();
          Property * pP = aProps.getArray();
          OSL_ENSURE( 6 == aProps.getLength(), "getProperties() gives not all properties" );

        // table to switch to sorted
        int a[] = { 2 , 1 , 4, 3, 5, 0 };
          for( int i = 0; i < 6; i++ )
          {
              OSL_ENSURE( pP[i].Name == getPropertyTable2()[a[i]].Name , "Name not correct" );
              OSL_ENSURE( pP[i].Handle == getPropertyTable2()[a[i]].Handle, "Handle not correct" );
              OSL_ENSURE( pP[i].Attributes == getPropertyTable2()[a[i]].Attributes, "Attributes not correct" );
              OSL_ENSURE( pP[i].Type == getPropertyTable2()[a[i]].Type, "Type not correct" );
          }
    }

    // Test sorting
    {
          OPropertyArrayHelper a1( getPropertyTable3(), 3, sal_False );
          Sequence< Property > aProps = a1.getProperties();
          Property * pP = aProps.getArray();
          OSL_ENSURE( 3 == aProps.getLength(), "getProperties() gives not all properties" );
        // table to switch to sorted
        int a[] = { 2 , 0 , 1 };
          for( int i = 0; i < 3; i++ )
          {
              OSL_ENSURE( pP[i].Name == getPropertyTable3()[a[i]].Name , "Name not correct" );
              OSL_ENSURE( pP[i].Handle == getPropertyTable3()[a[i]].Handle, "Handle not correct" );
              OSL_ENSURE( pP[i].Attributes == getPropertyTable3()[a[i]].Attributes, "Attributes not correct" );
              OSL_ENSURE( pP[i].Type == getPropertyTable3()[a[i]].Type, "Type not correct" );
          }
    }

    // Test getPropertyByName and hasPropertyByName
    {
        OPropertyArrayHelper a1( getPropertyTable1(), 6 );
        for( int i = 0; i < 6; i++ )
        {
            OSL_ENSURE( a1.hasPropertyByName( getPropertyTable1()[i].Name ), "hasPropertyByName not correct" );
            Property aP = a1.getPropertyByName( getPropertyTable1()[i].Name );
            OSL_ENSURE( aP.Name == getPropertyTable1()[i].Name , "Name not correct" );
            OSL_ENSURE( aP.Handle == getPropertyTable1()[i].Handle, "Handle not correct" );
            OSL_ENSURE( aP.Attributes == getPropertyTable1()[i].Attributes, "Attributes not correct" );
            OSL_ENSURE( aP.Type == getPropertyTable1()[i].Type, "Type not correct" );
        }

        OSL_ENSURE( !a1.hasPropertyByName("never exist"), "hasPropertyByName not correct" );
        try
        {
            a1.getPropertyByName( OUString("never exist") );
            OSL_FAIL( "exception not thrown" );
        }
        catch( UnknownPropertyException & )
        {
        }
    }

    // Test getHandleByName
    {
        OPropertyArrayHelper a1( getPropertyTable1(), 6 );
        for( int i = 0; i < 6; i++ )
        {
            sal_Int32 Handle = a1.getHandleByName( getPropertyTable1()[i].Name );
            OSL_ENSURE( Handle == getPropertyTable1()[i].Handle, "Handle not correct" );
        }
        sal_Int32 Handle = a1.getHandleByName( OUString("asdaf") );
        OSL_ENSURE( Handle == -1, "Handle not correct" );
    }

    // Test fillPropertyMembersByHandle
    {
        OPropertyArrayHelper a1( getPropertyTable1(), 6 );
        int i;
        for( i = 0; i < 6; i++ )
        {
            sal_Int16   nAttributes;
            OUString aPropName;
            sal_Bool b = a1.fillPropertyMembersByHandle( &aPropName, &nAttributes, getPropertyTable1()[i].Handle );
            OSL_ENSURE( b, "fillPropertyMembersByHandle: handle not found" );
            OSL_ENSURE( nAttributes == getPropertyTable1()[i].Attributes, "fillPropertyMembersByHandle: Attributes not correct" );
            OSL_ENSURE( aPropName == getPropertyTable1()[i].Name , "fillPropertyMembersByHandle: Name not correct" );
        }
        OSL_ENSURE( !a1.fillPropertyMembersByHandle( NULL, NULL, 66666 ), "fillPropertyMembersByHandle: handle found" );
        // optimized table
        OPropertyArrayHelper a4( getPropertyTable4(), 3 );
        for( i = 0; i < 3; i++ )
        {
            sal_Int16   nAttributes;
            OUString aPropName;
            sal_Bool b = a1.fillPropertyMembersByHandle( &aPropName, &nAttributes, getPropertyTable4()[i].Handle );
            OSL_ENSURE( b, "fillPropertyMembersByHandle: handle not found" );
            OSL_ENSURE( nAttributes == getPropertyTable1()[i].Attributes, "fillPropertyMembersByHandle: Attributes not correct" );
            OSL_ENSURE( aPropName == getPropertyTable1()[i].Name , "fillPropertyMembersByHandle: Name not correct" );
        }
        OSL_ENSURE( !a4.fillPropertyMembersByHandle( NULL, NULL, 66666 ), "fillPropertyMembersByHandle: handle found" );
    }

    // Test fillHandles
    {
        OPropertyArrayHelper a1( getPropertyTable1(), 6 );
        Sequence< OUString > aS( 4 );
        sal_Int32 Handles[4];
        // muss sortiert sein
        aS[0] = "a";
        aS[1] = "d";
        aS[2] = "f";
        aS[3] = "t";
        sal_Int32 nHitCount = a1.fillHandles( Handles, aS );
        OSL_ENSURE( nHitCount == 3, "wrong number of hits " );
        OSL_ENSURE( Handles[0] == getPropertyTable1()[0].Handle, "Handle not correct" );
        OSL_ENSURE( Handles[1] == getPropertyTable1()[3].Handle, "Handle not correct" );
        OSL_ENSURE( Handles[2] == getPropertyTable1()[5].Handle, "Handle not correct" );
        OSL_ENSURE( Handles[3] == -1, "Handle not correct" );
    }
}


//  test_OPropertySetHelper

struct MutexContainer
{
    Mutex aMutex;
};
class test_OPropertySetHelper :
            public MutexContainer,
            public OBroadcastHelper ,
            public OPropertySetHelper,
            public OWeakObject
{
public:

                    test_OPropertySetHelper( Property * p, sal_Int32    n )
                        : MutexContainer()
                        , OBroadcastHelper( ((MutexContainer *)this)->aMutex )
//                      , OPropertySetHelper( *(static_cast< OBroadcastHelper * >(this)))
                        // MSCI 4 bug ! :
                        //      OBroadcastHelper == OBroadcastHelperVar<OMultiTypeInterfaceContainerHelper>
                        , OPropertySetHelper(
                                *(static_cast< OBroadcastHelper * >(this)))
                        , bBOOL( sal_False )
                        , nINT16( 0 )
                        , nINT32( 0 )
                        , pBasicProps( p )
                        , nPropCount( n )
                    {
                    }


                    ~test_OPropertySetHelper()
                    {
                    }

    void dispose()
                    {
                        // see comphlp.cxx
                        sal_Bool bDoDispose = sal_False;
                        {
                            MutexGuard aGuard( rBHelper.rMutex );
                            if( !rBHelper.bDisposed && !rBHelper.bInDispose )
                            {
                                rBHelper.bInDispose = sal_True;
                                bDoDispose = sal_True;
                            }
                        }
                        if( bDoDispose )
                        {
                            disposing();
                            EventObject aEvt;
                            aEvt.Source.set( (static_cast< OWeakObject *  >(this)) );

                            rBHelper.aLC.disposeAndClear( aEvt );
                            rBHelper.bDisposed = sal_True;
                            rBHelper.bInDispose = sal_False;
                        }
                    }

    // XInterface
    Any             SAL_CALL queryInterface( const css::uno::Type & rType ) throw(RuntimeException)
                    {
                        Any aRet( OPropertySetHelper::queryInterface( rType ) );
                        return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
                    }
    void            SAL_CALL acquire() throw()
                            { OWeakObject::acquire(); }
    void            SAL_CALL release() throw()
                            { OWeakObject::release(); }

    // XPropertySet
    Reference < XPropertySetInfo >  SAL_CALL getPropertySetInfo()throw(RuntimeException);

    using OPropertySetHelper::getFastPropertyValue;

    sal_Bool                    bBOOL;
    sal_Int16                   nINT16;
    sal_Int32                   nINT32;
    Property *                  pBasicProps;
    sal_Int32                   nPropCount;
protected:
    IPropertyArrayHelper & SAL_CALL getInfoHelper() throw(RuntimeException);
    sal_Bool SAL_CALL convertFastPropertyValue(
        Any & rConvertedValue, Any & rOldValue,
        sal_Int32 nHandle, const Any& rValue )
        throw(IllegalArgumentException);
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw(RuntimeException);
    void SAL_CALL getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const throw(RuntimeException);
};


//------ The Properties of this implementation -------------------------

// Id must be the index into the array
#define PROPERTY_BOOL       0
#define PROPERTY_INT16      1
#define PROPERTY_INT32      2

// Max number of properties
#define PROPERTY_COUNT                      4
// Names of Properties
/**
 * All Properties of this implementation. Must be sorted by name.
 */
Property * getBasicProps()
{
    static Property *pTable = 0;

    if( ! pTable )  {
        MutexGuard guard(  ::osl::Mutex::getGlobalMutex() );
        if( ! pTable ) {

            static Property aBasicProps[PROPERTY_COUNT] =
            {
                 Property( OUString("BOOL")  , PROPERTY_BOOL , cppu::UnoType<bool>::get(), PropertyAttribute::READONLY ),
                 Property( OUString("INT16") , PROPERTY_INT16,
                                 cppu::UnoType<sal_Int16>::get(),  PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED ),
                 Property( OUString("INT32") , PROPERTY_INT32, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::BOUND ),
                 Property( OUString("TEST")  , 55            , cppu::UnoType<sal_Int32>::get(),  PropertyAttribute::BOUND )
            };
            pTable = aBasicProps;
        }
    }
    return pTable;
}


//  test_OPropertySetHelper_Listener

class test_OPropertySetHelper_Listener
    : public WeakImplHelper3< XPropertyChangeListener, XPropertiesChangeListener, XVetoableChangeListener >
{
public:
    sal_Int32           nDisposing;
    Mutex               aMutex;
    Any *               pExceptedListenerValues;
    sal_Int32           nCount;
    sal_Int32           nCurrent;

                    test_OPropertySetHelper_Listener( Any * p = 0, sal_Int32 n = 0 )
                        : nDisposing( 0 )
                        , pExceptedListenerValues( p )
                        , nCount( n )
                        , nCurrent( 0 )
                    {
                    }
                    ~test_OPropertySetHelper_Listener()
                    {
                    }

    sal_Int32   getRefCount() const
        { return m_refCount; }

    // XEventListener
    void SAL_CALL disposing(const EventObject& /*evt*/) throw ( RuntimeException)
    {
        MutexGuard aGuard( aMutex );
        nDisposing++;
    }

    // XPropertyChangeListener
    void SAL_CALL propertyChange(const PropertyChangeEvent& evt) throw (RuntimeException)
    {
        if( !pExceptedListenerValues )
            return;
        MutexGuard aGuard( aMutex );
        OSL_ENSURE( nCurrent +1 < nCount, "PropertySetHelper: too many listener calls" );

        switch( evt.PropertyHandle )
        {
            case PROPERTY_BOOL:
                {
                OSL_FAIL( "PropertySetHelper: BOOL cannot change" );
                OSL_ENSURE( evt.PropertyName == "BOOL", "PropertySetHelper: wrong name" );
                }
            break;

            case PROPERTY_INT16:
                {
                OSL_ENSURE( evt.PropertyName == "INT16", "PropertySetHelper: wrong name" );

                OSL_ENSURE( pExceptedListenerValues[nCurrent].getValueType().getTypeClass() == TypeClass_SHORT ,
                            "PropertySetHelper: wrong data type" );

                sal_Int16 nInt16(0), nOldInt16(0);
                pExceptedListenerValues[nCurrent]   >>= nInt16;
                evt.OldValue                        >>= nOldInt16;
                OSL_ENSURE( nInt16 == nOldInt16, "PropertySetHelper: wrong old value" );


                pExceptedListenerValues[nCurrent+1] >>= nInt16;
                evt.NewValue                        >>= nOldInt16;
                OSL_ENSURE( nInt16 == nOldInt16 ,   "PropertySetHelper: wrong new value" );
                }
            break;

            case PROPERTY_INT32:
                {
                    OSL_ENSURE( evt.PropertyName == "INT32", "PropertySetHelper: wrong name" );

                    sal_Int32 nInt32(0),nOldInt32(0);

                    pExceptedListenerValues[nCurrent] >>= nInt32;
                    evt.OldValue >>= nOldInt32;
                    OSL_ENSURE( nInt32 == nOldInt32 ,   "PropertySetHelper: wrong old value" );

                    pExceptedListenerValues[nCurrent+1] >>= nInt32;
                    evt.NewValue >>= nOldInt32;
                    OSL_ENSURE( nInt32 == nOldInt32 ,   "PropertySetHelper: wrong new value" );
                }
            break;

            default:
                OSL_FAIL( "XPropeSetHelper: invalid property handle" );
        }
        nCurrent += 2;
    }

    // XVetoableChangeListener
    void SAL_CALL vetoableChange(const PropertyChangeEvent& evt) throw  (PropertyVetoException, RuntimeException)
    {
        if( !pExceptedListenerValues )
            return;
        MutexGuard aGuard( aMutex );
        OSL_ENSURE( nCurrent +1 < nCount, "PropertySetHelper: too many listener calls" );

        switch( evt.PropertyHandle )
        {
            case PROPERTY_BOOL:
                {
                    OSL_FAIL( "PropertySetHelper: BOOL cannot change" );
                    OSL_ENSURE( evt.PropertyName == "BOOL", "PropertySetHelper: wrong name" );
                }
            break;

            case PROPERTY_INT16:
                {
                    OSL_ENSURE( evt.PropertyName == "INT16", "PropertySetHelper: wrong name" );

                    sal_Int16 nInt16(0), nOldInt16(0);
                    pExceptedListenerValues[nCurrent]   >>= nInt16;
                    evt.OldValue                        >>= nOldInt16;

                    OSL_ENSURE( nInt16 == nOldInt16,"PropertySetHelper: wrong old value" );

                    pExceptedListenerValues[nCurrent+1]     >>= nInt16;
                    evt.NewValue                            >>= nOldInt16;
                    OSL_ENSURE( nInt16 == nOldInt16 ,   "PropertySetHelper: wrong new value" );

                    if( nOldInt16 == 100 )
                    {
                        nCurrent += 2;
                        throw PropertyVetoException();
                    }
                }
            break;

            case PROPERTY_INT32:
                {
                    OSL_ENSURE( evt.PropertyName == "INT32", "PropertySetHelper: wrong name" );

                    sal_Int32 nInt32(0),nOldInt32(0);
                    pExceptedListenerValues[nCurrent] >>= nInt32;
                    evt.OldValue >>= nOldInt32;
                    OSL_ENSURE( nInt32 == nOldInt32 , "PropertySetHelper: wrong old value" );

                    pExceptedListenerValues[nCurrent+1] >>= nInt32;
                    evt.NewValue >>= nOldInt32;
                    OSL_ENSURE( nInt32 == nOldInt32 , "PropertySetHelper: wrong new value" );
                }
            break;

            default:
                OSL_FAIL( "XPropeSetHelper: invalid property handle" );
        }
        nCurrent += 2;
    }

    // XPropertiesChangeListener
    void SAL_CALL propertiesChange(const Sequence< PropertyChangeEvent >& evtSeq) throw (RuntimeException)
    {
        if( !pExceptedListenerValues )
            return;
        MutexGuard aGuard( aMutex );
        for( sal_Int32 i = 0; i < evtSeq.getLength(); i++ )
        {
            const PropertyChangeEvent & evt = evtSeq.getConstArray()[i];
            OSL_ENSURE( nCurrent +1 < nCount, "PropertySetHelper: too many listener calls" );

            switch( evt.PropertyHandle )
            {
                case PROPERTY_BOOL:
                    {
                    OSL_FAIL( "PropertySetHelper: BOOL cannot change" );
                    OSL_ENSURE( evt.PropertyName == "BOOL", "PropertySetHelper: wrong name" );
                    }
                break;

                case PROPERTY_INT16:
                    {
                    OSL_ENSURE( evt.PropertyName == "INT16", "PropertySetHelper: wrong name" );

                    sal_Int16 nInt16(0), nOldInt16(0);
                    pExceptedListenerValues[nCurrent]   >>= nInt16;
                    evt.OldValue                        >>= nOldInt16;
                    OSL_ENSURE( nInt16 == nOldInt16 , "PropertySetHelper: wrong old value" );


                    pExceptedListenerValues[nCurrent+1] >>= nInt16;
                    evt.NewValue                        >>= nOldInt16;
                    OSL_ENSURE( nInt16 == nOldInt16 , "PropertySetHelper: wrong new value" );
                    }
                break;

                case PROPERTY_INT32:
                    {
                    OSL_ENSURE( evt.PropertyName == "INT32", "PropertySetHelper: wrong name" );


                    sal_Int32 nInt32(0),nOldInt32(0);
                    pExceptedListenerValues[nCurrent] >>= nInt32;
                    evt.OldValue >>= nOldInt32;
                    OSL_ENSURE( nInt32 == nOldInt32 , "PropertySetHelper: wrong old value" );

                    pExceptedListenerValues[nCurrent+1] >>= nInt32;
                    evt.NewValue >>= nOldInt32;
                    OSL_ENSURE( nInt32 == nOldInt32 ,   "PropertySetHelper: wrong new value" );
                    }
                break;

                default:
                    OSL_FAIL( "XPropeSetHelper: invalid property handle" );
            }
            nCurrent += 2;
        }
    }
};

/**
 * Create a table that map names to index values.
 */
IPropertyArrayHelper & test_OPropertySetHelper::getInfoHelper() throw(RuntimeException)
{
    // no multi thread protection
    static OPropertyArrayHelper aInfo( pBasicProps, nPropCount );
    return aInfo;
}

// XPropertySet
Reference < XPropertySetInfo >  test_OPropertySetHelper::getPropertySetInfo()
    throw(RuntimeException)
{
    // no multi thread protection
    static Reference < XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// Return sal_True, value changed
sal_Bool test_OPropertySetHelper::convertFastPropertyValue
(
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue
)throw(IllegalArgumentException)
{
    switch( nHandle )
    {
        case PROPERTY_BOOL:
            {
            sal_Bool b;
            convertPropertyValue( b , rValue );
            if( b != bBOOL )
            {

                rConvertedValue.setValue( &b , cppu::UnoType<bool>::get()  );
                rOldValue.setValue( & bBOOL , cppu::UnoType<bool>::get() );
                return sal_True;
            }
            else
                return sal_False;
            }

        case PROPERTY_INT16:
            {
            sal_Int16 n16;
            convertPropertyValue( n16 , rValue );

            if( n16 != nINT16 )
            {
                rConvertedValue <<=  n16;
                rOldValue       <<= nINT16;
                return sal_True;
            }
            else
                return sal_False;
            }

        case PROPERTY_INT32:
            {
            sal_Int32 n32;
            convertPropertyValue( n32 , rValue );
            if( n32 != nINT32 )
            {
                rConvertedValue <<= n32;
                rOldValue       <<= nINT32;
                return sal_True;
            }
            else
                return sal_False;
            }

        default:
            OSL_ENSURE( nHandle == -1, "invalid property handle" );
            return sal_False;
    }
}

/**
 * only set the value.
 */
void test_OPropertySetHelper::setFastPropertyValue_NoBroadcast
(
    sal_Int32 nHandle,
    const Any& rValue
)throw(RuntimeException)
{
    switch( nHandle )
    {
        case PROPERTY_BOOL:
            OSL_ENSURE( rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN, "invalid type" );
            bBOOL = *((sal_Bool*)rValue.getValue());
        break;

        case PROPERTY_INT16:
            OSL_ENSURE( rValue.getValueType().getTypeClass() == TypeClass_SHORT, "invalid type" );
            rValue >>= nINT16;
        break;

        case PROPERTY_INT32:
            OSL_ENSURE( rValue.getValueType().getTypeClass() == TypeClass_LONG, "invalid type" );
            rValue >>= nINT32;
        break;

        default:
            OSL_ENSURE( nHandle == -1, "invalid property handle" );
    }
}


void test_OPropertySetHelper::getFastPropertyValue( Any & rRet, sal_Int32 nHandle ) const
    throw(RuntimeException)
{
    switch( nHandle )
    {
        case PROPERTY_BOOL:
            rRet.setValue( &bBOOL , cppu::UnoType<bool>::get() );
        break;

        case PROPERTY_INT16:
            rRet <<= nINT16;
        break;

        case PROPERTY_INT32:
            rRet <<= nINT32;
        break;

        default:
            OSL_ENSURE( nHandle == -1, "invalid property handle" );
    }
}


void test_PropertySetHelper()
{
    test_PropertyArrayHelper();

    test_OPropertySetHelper * pPS;

    Reference < XPropertySet >  xPS;
    Reference < XPropertyChangeListener > xPS_L;
    test_OPropertySetHelper_Listener * pPS_L;

    Reference < XInterface > x;

    for( int z = 0; z < 2; z++ )
    {
        // first test aBasicProps Handles are { 0, 1, 2, 55 }
        // first test getBasicProps() Handles are { 0, 1, 2 }
        xPS = pPS = new test_OPropertySetHelper( getBasicProps(), PROPERTY_COUNT - z );
        xPS_L = static_cast< XPropertyChangeListener * >( pPS_L = new test_OPropertySetHelper_Listener() );

        // Test queryInterface
        Reference < XPropertySet > rProp( xPS , UNO_QUERY );
        OSL_ENSURE( rProp.is() , "PropertySetHelper: XPropertySet nor supported" );

        Reference < XMultiPropertySet > rMulti( xPS , UNO_QUERY );
        OSL_ENSURE( rMulti.is() , "PropertySetHelper: XMultiPropertySet nor supported" );

        Reference < XFastPropertySet > rFast( xPS , UNO_QUERY );
        OSL_ENSURE( rFast.is() , "PropertySetHelper: XFastPropertySet nor supported" );

        x.clear();

        // Test add-remove listener
        {
            Reference < XPropertiesChangeListener >     x1( xPS_L, UNO_QUERY );
            Reference < XVetoableChangeListener >       x2( xPS_L, UNO_QUERY );

            xPS->addPropertyChangeListener( OUString("INT16"), xPS_L );
            Sequence<OUString> szPN( 3 );
            szPN[0] = "BOOL";
            szPN[1] = "INT32";
            szPN[2] = "Does not exist"; // must ne ignored by the addPropertiesChangeListener method
            pPS->addPropertiesChangeListener( szPN, x1 );

            szPN = Sequence<OUString>();
            pPS->addPropertiesChangeListener( szPN, x1 );
            pPS->addVetoableChangeListener( OUString("INT16"), x2 );

            xPS->removePropertyChangeListener( OUString("INT16"), xPS_L );
            pPS->removePropertiesChangeListener( x1 );
            pPS->removePropertiesChangeListener( x1 );
            pPS->removeVetoableChangeListener( OUString("INT16"), x2 );

            // this exception must thrown
            try
            {
                xPS->addPropertyChangeListener( OUString("Does not exist"), xPS_L );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( UnknownPropertyException & /*e*/ )
            {

            }

            try
            {
                xPS->addVetoableChangeListener( OUString("Does not exist"), x2 );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( UnknownPropertyException & /*e*/ )
            {

            }

        }
        OSL_ENSURE( pPS_L->getRefCount() == 1, "PropertySetHelper: wrong reference count" );

        // Test disposing
        {
            Reference < XPropertiesChangeListener >  x1( xPS_L, UNO_QUERY );
            Reference < XVetoableChangeListener >    x2( xPS_L, UNO_QUERY );

            xPS->addPropertyChangeListener( OUString("INT16"), xPS_L );
            Sequence<OUString> szPN( 2 );
            szPN[0] = "BOOL";
            szPN[1] = "INT32";
            pPS->addPropertiesChangeListener( szPN, x1 );
            szPN = Sequence<OUString>();
            pPS->addPropertiesChangeListener( szPN, x1 );
            pPS->addVetoableChangeListener( OUString("INT16"), x2 );
            pPS->dispose();
        }
        OSL_ENSURE( pPS_L->nDisposing == 4      , "PropertySetHelper: wrong disposing count" );
        OSL_ENSURE( pPS_L->getRefCount() == 1   , "PropertySetHelper: wrong reference count" );
        pPS_L->nDisposing = 0;
        xPS = pPS = new test_OPropertySetHelper( getBasicProps(), PROPERTY_COUNT - z );

        // Test set- and get- (Fast) propertyValue
        {
            // set read only property
            try
            {
                // Readonly raises a vetoable exception
                sal_Bool b = sal_True;
                Any aBool;
                aBool.setValue( &b , cppu::UnoType<bool>::get() );
                xPS->setPropertyValue("BOOL", aBool );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( PropertyVetoException & /*e*/ )
            {
            }

            try
            {
                // Readonly raises a vetoable exception
                sal_Bool b = sal_True;
                Any aBool;
                aBool.setValue( &b , cppu::UnoType<bool>::get() );
                // BOOL i s0
                pPS->setFastPropertyValue( PROPERTY_BOOL, aBool );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( PropertyVetoException & /*e*/ )
            {
            }

            // set unknown property
            try
            {
                sal_Bool b = sal_True;
                Any aBool;
                aBool.setValue( &b , cppu::UnoType<bool>::get() );
                xPS->setPropertyValue("Does not exist", aBool );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( UnknownPropertyException & /*e*/ )
            {
            }

            try
            {
                sal_Bool b = sal_True;
                Any aBool;
                aBool.setValue( &b , cppu::UnoType<bool>::get() );
                pPS->setFastPropertyValue( 3, aBool );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( UnknownPropertyException & /*e*/ )
            {
            }

            // get unknown property
            try
            {
                Any aBool;
                aBool = xPS->getPropertyValue("Does not exist");
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( UnknownPropertyException & /*e*/ )
            {
            }

            try
            {
                Any aBool;
                aBool = ((XFastPropertySet *)pPS)->getFastPropertyValue( 3 );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( UnknownPropertyException & /*e*/ )
            {
            }

            // set property with invalid type
            try
            {
                Any aBool;
                xPS->setPropertyValue("INT32", aBool );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( IllegalArgumentException & /*e*/ )
            {
            }

            try
            {
                Any aBool;
                pPS->setFastPropertyValue( PROPERTY_INT32, aBool );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( IllegalArgumentException & /*e*/ )
            {
            }

            // narrowing conversion is not allowed!
            try
            {
                Any aINT32;
                aINT32 <<= (sal_Int32 ) 16;
                xPS->setPropertyValue("INT16", aINT32 );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( IllegalArgumentException & /*e*/ )
            {
            }


            try
            {
                Any aINT32;
                aINT32 <<= (sal_Int32) 16;
                pPS->setFastPropertyValue( PROPERTY_INT16, aINT32 );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( IllegalArgumentException & /*e*/ )
            {
            }


            Any aValue;
            aValue = xPS->getPropertyValue("BOOL");
            sal_Bool b = *( (sal_Bool*)aValue.getValue());
            OSL_ENSURE( ! b, "PropertySetHelper: wrong BOOL value" );
            aValue = ((XFastPropertySet *)pPS)->getFastPropertyValue( PROPERTY_BOOL );

            b = *((sal_Bool*)aValue.getValue());
            OSL_ENSURE( !b, "PropertySetHelper: wrong BOOL value" );

            sal_Int16 n16(0);
            aValue <<=(sal_Int16)22;
            xPS->setPropertyValue("INT16", aValue );
            aValue = xPS->getPropertyValue("INT16");
            aValue >>= n16;
            OSL_ENSURE( 22 == n16 , "PropertySetHelper: wrong INT16 value" );
            aValue <<= (sal_Int16)44;
            ((XFastPropertySet *)pPS)->setFastPropertyValue( PROPERTY_INT16, aValue );

            aValue = ((XFastPropertySet *)pPS)->getFastPropertyValue( PROPERTY_INT16 );
            aValue >>= n16;
            OSL_ENSURE( 44 == n16, "PropertySetHelper: wrong INT16 value" );

            // widening conversion
            aValue <<= (sal_Int16)55;
            xPS->setPropertyValue("INT32", aValue );
            aValue = xPS->getPropertyValue("INT32");
            sal_Int32 n32(0);
            aValue >>= n32;
            OSL_ENSURE( 55 == n32 , "PropertySetHelper: wrong INT32 value" );
            aValue <<= (sal_Int16)66;
            ((XFastPropertySet *)pPS)->setFastPropertyValue( PROPERTY_INT32, aValue );
            aValue = ((XFastPropertySet *)pPS)->getFastPropertyValue( PROPERTY_INT32 );
            aValue >>= n32;
            OSL_ENSURE( 66 == n32, "PropertySetHelper: wrong INT32 value" );

            Sequence< OUString >valueNames = Sequence<OUString>( 3 );
            valueNames[0] = "BOOL";
            valueNames[1] = "INT16";
            valueNames[2] = "INT32";
            Sequence< Any > aValues = pPS->getPropertyValues( valueNames );

            b = *((sal_Bool*)aValues.getConstArray()[0].getValue());
            aValues.getConstArray()[1] >>= n16;
            aValues.getConstArray()[2] >>= n32;

            OSL_ENSURE( !b, "PropertySetHelper: wrong BOOL value" );
            OSL_ENSURE( 44 == n16, "PropertySetHelper: wrong INT16 value" );
            OSL_ENSURE( 66 == n32, "PropertySetHelper: wrong INT32 value" );
        }
        pPS->nINT32 = 0;
        pPS->nINT16 = 0;

        // Test add-remove listener
        {
            Reference < XVetoableChangeListener >  x2( xPS_L, UNO_QUERY );

            xPS->addPropertyChangeListener( OUString("INT16"), xPS_L );
            pPS->addVetoableChangeListener( OUString("INT16"), x2 );

            pPS_L->nCount = 10;
            Sequence< Any > aSeq( pPS_L->nCount );
            pPS_L->nCurrent = 0;

            pPS_L->pExceptedListenerValues = aSeq.getArray();

            pPS_L->pExceptedListenerValues[0] <<= (sal_Int16) 0; // old value   vetoable
            pPS_L->pExceptedListenerValues[1] <<= (sal_Int16) 22; // new value  vetoable
            pPS_L->pExceptedListenerValues[2] <<= (sal_Int16) 0; // old value   bound
            pPS_L->pExceptedListenerValues[3] <<= (sal_Int16) 22; // new value  bound
            pPS_L->pExceptedListenerValues[4] <<= (sal_Int16) 22; // old value  vetoable
            pPS_L->pExceptedListenerValues[5] <<= (sal_Int16) 44; // new value  vetoable
            pPS_L->pExceptedListenerValues[6] <<= (sal_Int16) 22; // old value  bound
            pPS_L->pExceptedListenerValues[7] <<= (sal_Int16) 44; // new value  bound
            pPS_L->pExceptedListenerValues[8] <<= (sal_Int16) 44; // old value  vetoable
            pPS_L->pExceptedListenerValues[9] <<= (sal_Int16) 100; // new value vetoable exception

            Any aValue;
            aValue <<= (sal_Int16)22;
            xPS->setPropertyValue("INT16", aValue );
            aValue <<= (sal_Int16) 44;
            ((XFastPropertySet *)pPS)->setFastPropertyValue( PROPERTY_INT16, aValue );
            aValue <<= (sal_Int16)100;// exception

            try
            {
                ((XFastPropertySet *)pPS)->setFastPropertyValue( PROPERTY_INT16, aValue );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch( PropertyVetoException & /*e*/ )
            {
            }

            OSL_ENSURE( pPS_L->nCount == pPS_L->nCurrent, "not all listeners called" );
            pPS->nINT32 = 0;
            pPS->nINT16 = 0;
            pPS_L->nCount = 0;
            pPS_L->nCurrent = 0;
            pPS_L->pExceptedListenerValues = NULL;
            xPS->removePropertyChangeListener( OUString("INT16"), xPS_L );
            pPS->removeVetoableChangeListener( OUString("INT16"), x2 );
        }

        // Test multi property set listener
        {
            Reference < XPropertiesChangeListener > x1( xPS_L, UNO_QUERY );
            Reference < XVetoableChangeListener >  x2( xPS_L, UNO_QUERY );

            pPS->addVetoableChangeListener( OUString("INT16") , x2 );
            Sequence<OUString> szPN( 4 );
            szPN[0] = "BOOL";
            szPN[1] = "INT32";
            szPN[2] = "Does not exist"; // must ne ignored by the addPropertiesChangeListener method
            szPN[3] = "INT16";
            pPS->addPropertiesChangeListener( szPN, x1 );

            pPS_L->nCount = 6;
            Sequence< Any > aSeq( pPS_L->nCount );
            pPS_L->nCurrent = 0;
            pPS_L->pExceptedListenerValues = aSeq.getArray();
            pPS_L->pExceptedListenerValues[0] <<= (sal_Int16) 0; // old value   vetoable
            pPS_L->pExceptedListenerValues[1] <<= (sal_Int16 ) 22; // new value vetoable
            // INT32 is not constrained
            pPS_L->pExceptedListenerValues[2] <<= (sal_Int16) 0; // old value   bound
            pPS_L->pExceptedListenerValues[3] <<= (sal_Int16) 22; // new value  bound
            pPS_L->pExceptedListenerValues[4] <<= (sal_Int32) 0; // old value   bound
            pPS_L->pExceptedListenerValues[5] <<= (sal_Int32) 44; // new value  bound

            szPN = Sequence<OUString>( 2 );
            szPN[0] = "INT16";
            szPN[1] = "INT32";
            Sequence< Any > aValues( 2 );
            aValues.getArray()[0] <<= (sal_Int16) 22;
            aValues.getArray()[1] <<= (sal_Int16) 44;
            pPS->setPropertyValues( szPN, aValues );
            OSL_ENSURE( pPS_L->nCount == pPS_L->nCurrent, "not all listeners called" );

            //firePropertiesChangeEvent
            pPS->nINT16 = 8;
            pPS->nINT32 = 5;
            pPS_L->nCount = 4;
            pPS_L->nCurrent = 0;
            pPS_L->pExceptedListenerValues[0] <<= (sal_Int16) 8; // old value
            pPS_L->pExceptedListenerValues[1] <<= (sal_Int16) 8; // new value
            pPS_L->pExceptedListenerValues[2] <<= (sal_Int32) 5; // old value
            pPS_L->pExceptedListenerValues[3] <<= (sal_Int32) 5; // new value
            pPS->firePropertiesChangeEvent( szPN, pPS_L );
            OSL_ENSURE( pPS_L->nCount == pPS_L->nCurrent, "not all listeners called" );


            //vetoable exception with multiple
            szPN[0] = "INT16";
            szPN[1] = "INT16";
            pPS->nINT32 = 0;
            pPS->nINT16 = 0;
            pPS_L->nCount = 4;
            pPS_L->nCurrent = 0;
            pPS_L->pExceptedListenerValues[0] <<= (sal_Int16) 0; // old value   vetoable
            pPS_L->pExceptedListenerValues[1] <<= (sal_Int16) 44; // new value  vetoable
            pPS_L->pExceptedListenerValues[2] <<= (sal_Int16) 0; // old value   vetoable
            pPS_L->pExceptedListenerValues[3] <<= (sal_Int16) 100; // new value vetoable

            try
            {
                aValues.getArray()[0] <<= (sal_Int16)44;
                aValues.getArray()[1] <<= (sal_Int16)100;
                pPS->setPropertyValues( szPN, aValues );
                OSL_FAIL( "PropertySetHelper: exception not thrown" );
            }
            catch ( PropertyVetoException & /*e*/ )
            {
            }

            OSL_ENSURE( pPS_L->nCount == pPS_L->nCurrent, "not all listeners called" );
            pPS->removePropertiesChangeListener( x1 );
            pPS->removeVetoableChangeListener( OUString("INT16"), x2 );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
