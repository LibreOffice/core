/*************************************************************************
 *
 *  $RCSfile: testpropshlp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:11 $
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
#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>

#include <cppuhelper/proptypehlp.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <cppuhelper/implbase3.hxx>

using namespace ::cppu;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;



/**********************
*
**********************/
static Property * getPropertyTable1()
{
    static Property *pTable = 0;

    if( ! pTable )  {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pTable ) {
            static Property aTable[] =
            {
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("a") ), 0, getCppuType( (OUString *)0) ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //OUString
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("b") ), 1, getCppuCharType( ) ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //Char
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("c") ), 2, getCppuType( (sal_Int32*)0) ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //sal_Int32
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("d") ), 5, getCppuType( (double*)0)     ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //double
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("e") ), 7, getCppuBooleanType()         ,
                             PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), //BOOL
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("f") ), 8, getCppuType( (Any*)0)        ,
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
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("f") ), 8, getCppuType( (Any *)0)   ,
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // Any
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("b") ), 1, getCppuCharType( ),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // Char
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("a") ), 0, getCppuType( (OUString*)0),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // OUString
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("d") ), 5, getCppuType( (double*)0) ,
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // Double
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("c") ), 2, getCppuType( (sal_Int32*)0),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // sal_Int32
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("e") ), 7, getCppuBooleanType()         ,
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
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("b") ), 1, getCppuCharType( ),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // Char
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("f") ), 8, getCppuType( (Any *)0)   ,
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // any
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("a") ), 0, getCppuType( (OUString*)0),
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
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("a") ), 0, getCppuType( (OUString*)0),
                                    PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ),   // OUString
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("b") ), 1, getCppuCharType( ),
                                     PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID ), // Char
                Property( OUString( RTL_CONSTASCII_USTRINGPARAM("f") ), 2, getCppuType( (Any *)0)   ,
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
        OSL_ENSHURE( 6 == a1.getCount(), "not all properties inserted" );
        Sequence< Property > aProps = a1.getProperties();
        Property * pP = aProps.getArray();
        OSL_ENSHURE( 6 == aProps.getLength(), "getProperties() gives not all properties" );
        for( int i = 0; i < 6; i++ )
        {
            OSL_ENSHURE( pP[i].Name         == getPropertyTable1()[i].Name , "Name not correct" );
            OSL_ENSHURE( pP[i].Handle       == getPropertyTable1()[i].Handle, "Handle not correct" );
            OSL_ENSHURE( pP[i].Attributes   == getPropertyTable1()[i].Attributes, "Attributes not correct" );
            OSL_ENSHURE( pP[i].Type == getPropertyTable1()[i].Type, "Type not correct" );
        }
    }

    // Test sorting
    {
          OPropertyArrayHelper a1( getPropertyTable2(), 6, sal_False );
          Sequence< Property > aProps = a1.getProperties();
          Property * pP = aProps.getArray();
          OSL_ENSHURE( 6 == aProps.getLength(), "getProperties() gives not all properties" );
          for( int i = 0; i < 6; i++ )
          {
              OSL_ENSHURE( pP[i].Name == getPropertyTable2()[i].Name , "Name not correct" );
              OSL_ENSHURE( pP[i].Handle == getPropertyTable2()[i].Handle, "Handle not correct" );
              OSL_ENSHURE( pP[i].Attributes == getPropertyTable2()[i].Attributes, "Attributes not correct" );
              OSL_ENSHURE( pP[i].Type == getPropertyTable2()[i].Type, "Type not correct" );
          }
    }

    // Test sorting
    {
          OPropertyArrayHelper a1( getPropertyTable3(), 3, sal_False );
          Sequence< Property > aProps = a1.getProperties();
          Property * pP = aProps.getArray();
          OSL_ENSHURE( 3 == aProps.getLength(), "getProperties() gives not all properties" );
          for( int i = 0; i < 3; i++ )
          {
              OSL_ENSHURE( pP[i].Name == getPropertyTable3()[i].Name , "Name not correct" );
              OSL_ENSHURE( pP[i].Handle == getPropertyTable3()[i].Handle, "Handle not correct" );
              OSL_ENSHURE( pP[i].Attributes == getPropertyTable3()[i].Attributes, "Attributes not correct" );
              OSL_ENSHURE( pP[i].Type == getPropertyTable3()[i].Type, "Type not correct" );
          }
    }

    // Test getPropertyByName and hasPropertyByName
    {
        OPropertyArrayHelper a1( getPropertyTable1(), 6 );
        for( int i = 0; i < 6; i++ )
        {
            OSL_ENSHURE( a1.hasPropertyByName( getPropertyTable1()[i].Name ), "hasPropertyByName not correct" );
            Property aP = a1.getPropertyByName( getPropertyTable1()[i].Name );
            OSL_ENSHURE( aP.Name == getPropertyTable1()[i].Name , "Name not correct" );
            OSL_ENSHURE( aP.Handle == getPropertyTable1()[i].Handle, "Handle not correct" );
            OSL_ENSHURE( aP.Attributes == getPropertyTable1()[i].Attributes, "Attributes not correct" );
            OSL_ENSHURE( aP.Type == getPropertyTable1()[i].Type, "Type not correct" );
        }

        OSL_ENSHURE( !a1.hasPropertyByName( OUString( RTL_CONSTASCII_USTRINGPARAM("never exist") ) ), "hasPropertyByName not correct" );
        try
        {
            a1.getPropertyByName( OUString( RTL_CONSTASCII_USTRINGPARAM("never exist") ) );
            OSL_ENSHURE( sal_False, "exeption not thrown" );
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
            OSL_ENSHURE( Handle == getPropertyTable1()[i].Handle, "Handle not correct" );
        }
        sal_Int32 Handle = a1.getHandleByName( OUString( RTL_CONSTASCII_USTRINGPARAM("asdaf") ) );
        OSL_ENSHURE( Handle == -1, "Handle not correct" );
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
            OSL_ENSHURE( b, "fillPropertyMembersByHandle: handle not found" );
            OSL_ENSHURE( nAttributes == getPropertyTable1()[i].Attributes, "fillPropertyMembersByHandle: Attributes not correct" );
            OSL_ENSHURE( aPropName == getPropertyTable1()[i].Name , "fillPropertyMembersByHandle: Name not correct" );
        }
        OSL_ENSHURE( !a1.fillPropertyMembersByHandle( NULL, NULL, 66666 ), "fillPropertyMembersByHandle: handle found" );
        // optimized table
        OPropertyArrayHelper a4( getPropertyTable4(), 3 );
        for( i = 0; i < 3; i++ )
        {
            sal_Int16   nAttributes;
            OUString aPropName;
            sal_Bool b = a1.fillPropertyMembersByHandle( &aPropName, &nAttributes, getPropertyTable4()[i].Handle );
            OSL_ENSHURE( b, "fillPropertyMembersByHandle: handle not found" );
            OSL_ENSHURE( nAttributes == getPropertyTable1()[i].Attributes, "fillPropertyMembersByHandle: Attributes not correct" );
            OSL_ENSHURE( aPropName == getPropertyTable1()[i].Name , "fillPropertyMembersByHandle: Name not correct" );
        }
        OSL_ENSHURE( !a4.fillPropertyMembersByHandle( NULL, NULL, 66666 ), "fillPropertyMembersByHandle: handle found" );
    }

    // Test fillHandles
    {
        OPropertyArrayHelper a1( getPropertyTable1(), 6 );
        Sequence< OUString > aS( 4 );
        sal_Int32 Handles[4];
        // muss sortiert sein
        aS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("a") );
        aS.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("d") );
        aS.getArray()[2] = OUString( RTL_CONSTASCII_USTRINGPARAM("f") );
        aS.getArray()[3] = OUString( RTL_CONSTASCII_USTRINGPARAM("t") );
        sal_Int32 nHitCount = a1.fillHandles( Handles, aS );
        OSL_ENSHURE( nHitCount == 3, "wrong number of hits " );
        OSL_ENSHURE( Handles[0] == getPropertyTable1()[0].Handle, "Handle not correct" );
        OSL_ENSHURE( Handles[1] == getPropertyTable1()[3].Handle, "Handle not correct" );
        OSL_ENSHURE( Handles[2] == getPropertyTable1()[5].Handle, "Handle not correct" );
        OSL_ENSHURE( Handles[3] == -1, "Handle not correct" );
    }
}




//----------------------------------------------------
//  test_OPropertySetHelper
//----------------------------------------------------
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
//                      , OPropertySetHelper( *SAL_STATIC_CAST(OBroadcastHelper *,this))
                        // MSCI 4 bug ! :
                        //      OBroadcastHelper == OBroadcastHelperVar<OMultiTypeInterfaceContainerHelper>
                        , OPropertySetHelper(
                                *SAL_STATIC_CAST(OBroadcastHelperVar<OMultiTypeInterfaceContainerHelper> *,this))
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
                            aEvt.Source = Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * ,this) );

                            rBHelper.aLC.disposeAndClear( aEvt );
                            rBHelper.bDisposed = sal_True;
                            rBHelper.bInDispose = sal_False;
                        }
                    }

    // XInterface
    Any             SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
                    {
                        Any aRet( OPropertySetHelper::queryInterface( rType ) );
                        return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
                    }
    void            SAL_CALL acquire()
                            { OWeakObject::acquire(); }
    void            SAL_CALL release()
                            { OWeakObject::release(); }

    // XPropertySet
    Reference < XPropertySetInfo >  SAL_CALL getPropertySetInfo();

    sal_Bool                    bBOOL;
    sal_Int16                   nINT16;
    sal_Int32                   nINT32;
    Property *                  pBasicProps;
    sal_Int32                   nPropCount;
protected:
    IPropertyArrayHelper & SAL_CALL getInfoHelper();
    sal_Bool SAL_CALL convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue,
                                        sal_Int32 nHandle, const Any& rValue );
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue );
    void SAL_CALL getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const;
};

//----------------------------------------------------------------------
//------ The Properties of this implementation -------------------------
//----------------------------------------------------------------------
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
                 Property( OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") )  , PROPERTY_BOOL , getCppuBooleanType(), PropertyAttribute::READONLY ),
                 Property( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ) , PROPERTY_INT16,
                                 getCppuType( (sal_Int16*)0 ),  PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED ),
                 Property( OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") ) , PROPERTY_INT32, getCppuType( (sal_Int32*)0 ), PropertyAttribute::BOUND ),
                 Property( OUString( RTL_CONSTASCII_USTRINGPARAM("TEST") )  , 55            , getCppuType( (sal_Int32*)0),  PropertyAttribute::BOUND )
            };
            pTable = aBasicProps;
        }
    }
    return pTable;
}


//----------------------------------------------------
//  test_OPropertySetHelper_Listener
//----------------------------------------------------
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

    sal_Int32   getRefCount()
        { return m_refCount; }

    // XEventListener
    void SAL_CALL disposing(const EventObject& evt) throw ( RuntimeException)
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
        OSL_ENSHURE( nCurrent +1 < nCount, "PropertySetHelper: too many listener calls" );

        switch( evt.PropertyHandle )
        {
            case PROPERTY_BOOL:
                {
                OSL_ENSHURE( sal_False          , "PropertySetHelper: BOOL cannot change" );
                OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") ), "PropertySetHelper: wrong name" );
                }
            break;

            case PROPERTY_INT16:
                {
                OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), "PropertySetHelper: wrong name" );

                OSL_ENSHURE( pExceptedListenerValues[nCurrent].getValueType().getTypeClass() == TypeClass_SHORT ,
                            "PropertySetHelper: wrong data type" );

                sal_Int16 nInt16, nOldInt16;
                pExceptedListenerValues[nCurrent]   >>= nInt16;
                evt.OldValue                        >>= nOldInt16;
                OSL_ENSHURE( nInt16 == nOldInt16, "PropertySetHelper: wrong old value" );


                pExceptedListenerValues[nCurrent+1] >>= nInt16;
                evt.NewValue                        >>= nOldInt16;
                OSL_ENSHURE( nInt16 == nOldInt16 ,  "PropertySetHelper: wrong new value" );
                }
            break;

            case PROPERTY_INT32:
                {
                    OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") ), "PropertySetHelper: wrong name" );

                    sal_Int32 nInt32,nOldInt32;

                    pExceptedListenerValues[nCurrent] >>= nInt32;
                    evt.OldValue >>= nOldInt32;
                    OSL_ENSHURE( nInt32 == nOldInt32 ,  "PropertySetHelper: wrong old value" );

                    pExceptedListenerValues[nCurrent+1] >>= nInt32;
                    evt.NewValue >>= nOldInt32;
                    OSL_ENSHURE( nInt32 == nOldInt32 ,  "PropertySetHelper: wrong new value" );
                }
            break;

            default:
                OSL_ENSHURE( sal_False, "XPropeSetHelper: invalid property handle" );
        }
        nCurrent += 2;
    }

    // XVetoableChangeListener
    void SAL_CALL vetoableChange(const PropertyChangeEvent& evt) throw  (PropertyVetoException, RuntimeException)
    {
        if( !pExceptedListenerValues )
            return;
        MutexGuard aGuard( aMutex );
        OSL_ENSHURE( nCurrent +1 < nCount, "PropertySetHelper: too many listener calls" );

        switch( evt.PropertyHandle )
        {
            case PROPERTY_BOOL:
                {
                    OSL_ENSHURE( sal_False  , "PropertySetHelper: BOOL cannot change" );
                    OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") ), "PropertySetHelper: wrong name" );
                }
            break;

            case PROPERTY_INT16:
                {
                    OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), "PropertySetHelper: wrong name" );

                    sal_Int16 nInt16, nOldInt16;
                    pExceptedListenerValues[nCurrent]   >>= nInt16;
                    evt.OldValue                        >>= nOldInt16;

                    OSL_ENSHURE( nInt16 == nOldInt16,"PropertySetHelper: wrong old value" );

                    pExceptedListenerValues[nCurrent+1]     >>= nInt16;
                    evt.NewValue                            >>= nOldInt16;
                    OSL_ENSHURE( nInt16 == nOldInt16 ,  "PropertySetHelper: wrong new value" );

                    if( nOldInt16 == 100 )
                    {
                        nCurrent += 2;
                        throw PropertyVetoException();
                    }
                }
            break;

            case PROPERTY_INT32:
                {
                    OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") ), "PropertySetHelper: wrong name" );

                    sal_Int32 nInt32,nOldInt32;
                    pExceptedListenerValues[nCurrent] >>= nInt32;
                    evt.OldValue >>= nOldInt32;
                    OSL_ENSHURE( nInt32 == nOldInt32 , "PropertySetHelper: wrong old value" );

                    pExceptedListenerValues[nCurrent+1] >>= nInt32;
                    evt.NewValue >>= nOldInt32;
                    OSL_ENSHURE( nInt32 == nOldInt32 , "PropertySetHelper: wrong new value" );
                }
            break;

            default:
                OSL_ENSHURE( sal_False, "XPropeSetHelper: invalid property handle" );
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
            OSL_ENSHURE( nCurrent +1 < nCount, "PropertySetHelper: too many listener calls" );

            switch( evt.PropertyHandle )
            {
                case PROPERTY_BOOL:
                    {
                    OSL_ENSHURE( sal_False, "PropertySetHelper: BOOL cannot change" );
                    OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") ), "PropertySetHelper: wrong name" );
                    }
                break;

                case PROPERTY_INT16:
                    {
                    OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), "PropertySetHelper: wrong name" );

                    sal_Int16 nInt16, nOldInt16;
                    pExceptedListenerValues[nCurrent]   >>= nInt16;
                    evt.OldValue                        >>= nOldInt16;
                    OSL_ENSHURE( nInt16 == nOldInt16 , "PropertySetHelper: wrong old value" );


                    pExceptedListenerValues[nCurrent+1] >>= nInt16;
                    evt.NewValue                        >>= nOldInt16;
                    OSL_ENSHURE( nInt16 == nOldInt16 , "PropertySetHelper: wrong new value" );
                    }
                break;

                case PROPERTY_INT32:
                    {
                    OSL_ENSHURE( evt.PropertyName == OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") ), "PropertySetHelper: wrong name" );


                    sal_Int32 nInt32,nOldInt32;
                    pExceptedListenerValues[nCurrent] >>= nInt32;
                    evt.OldValue >>= nOldInt32;
                    OSL_ENSHURE( nInt32 == nOldInt32 , "PropertySetHelper: wrong old value" );

                    pExceptedListenerValues[nCurrent+1] >>= nInt32;
                    evt.NewValue >>= nOldInt32;
                    OSL_ENSHURE( nInt32 == nOldInt32 ,  "PropertySetHelper: wrong new value" );
                    }
                break;

                default:
                    OSL_ENSHURE( sal_False, "XPropeSetHelper: invalid property handle" );
            }
            nCurrent += 2;
        }
    }
};

/**
 * Create a table that map names to index values.
 */
IPropertyArrayHelper & test_OPropertySetHelper::getInfoHelper()
{
    // no multi thread protection
    static OPropertyArrayHelper aInfo( pBasicProps, nPropCount );
    return aInfo;
}

// XPropertySet
Reference < XPropertySetInfo >  test_OPropertySetHelper::getPropertySetInfo()
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
)
{
    switch( nHandle )
    {
        case PROPERTY_BOOL:
            {
            sal_Bool b;
            convertPropertyValue( b , rValue );
            if( b != bBOOL )
            {

                rConvertedValue.setValue( &b , ::getCppuBooleanType()  );
                rOldValue.setValue( & bBOOL , ::getCppuBooleanType() );
                return sal_True;
            }
            else
                return sal_False;
            }
        break;

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
        break;

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
        break;

        default:
            OSL_ENSHURE( nHandle == -1, "invalid property handle" );
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
)
{
    switch( nHandle )
    {
        case PROPERTY_BOOL:
            OSL_ENSHURE( rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN, "invalid type" );
            bBOOL = *((sal_Bool*)rValue.getValue());
        break;

        case PROPERTY_INT16:
            OSL_ENSHURE( rValue.getValueType().getTypeClass() == TypeClass_SHORT, "invalid type" );
            rValue >>= nINT16;
        break;

        case PROPERTY_INT32:
            OSL_ENSHURE( rValue.getValueType().getTypeClass() == TypeClass_LONG, "invalid type" );
            rValue >>= nINT32;
        break;

        default:
            OSL_ENSHURE( nHandle == -1, "invalid property handle" );
    }
}

//--------------------------
void test_OPropertySetHelper::getFastPropertyValue( Any & rRet, sal_Int32 nHandle ) const
{
    switch( nHandle )
    {
        case PROPERTY_BOOL:
            rRet.setValue( &bBOOL , getCppuBooleanType() );
        break;

        case PROPERTY_INT16:
            rRet <<= nINT16;
        break;

        case PROPERTY_INT32:
            rRet <<= nINT32;
        break;

        default:
            OSL_ENSHURE( nHandle == -1, "invalid property handle" );
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
        OSL_ENSHURE( rProp.is() , "PropertySetHelper: XPropertySet nor supported" );

        Reference < XMultiPropertySet > rMulti( xPS , UNO_QUERY );
        OSL_ENSHURE( rMulti.is() , "PropertySetHelper: XMultiPropertySet nor supported" );

        Reference < XFastPropertySet > rFast( xPS , UNO_QUERY );
        OSL_ENSHURE( rFast.is() , "PropertySetHelper: XFastPropertySet nor supported" );

        x = Reference < XInterface > ();

        // Test add-remove listener
        {
            Reference < XPropertiesChangeListener >     x1( xPS_L, UNO_QUERY );
            Reference < XVetoableChangeListener >       x2( xPS_L, UNO_QUERY );

            xPS->addPropertyChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), xPS_L );
            Sequence<OUString> szPN( 3 );
            szPN.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") );
            szPN.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") );
            szPN.getArray()[2] = OUString( RTL_CONSTASCII_USTRINGPARAM("Does not exist") ); // must ne ignored by the addPropertiesChangeListener method
            pPS->addPropertiesChangeListener( szPN, x1 );

            szPN = Sequence<OUString>();
            pPS->addPropertiesChangeListener( szPN, x1 );
            pPS->addVetoableChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), x2 );

            xPS->removePropertyChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), xPS_L );
            pPS->removePropertiesChangeListener( x1 );
            pPS->removePropertiesChangeListener( x1 );
            pPS->removeVetoableChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), x2 );

            // this exception must thrown
            try
            {
                xPS->addPropertyChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("Does not exist") ), xPS_L );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( UnknownPropertyException & e )
            {

            }

            try
            {
                xPS->addVetoableChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("Does not exist") ), x2 );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( UnknownPropertyException & e )
            {

            }

        }
        OSL_ENSHURE( pPS_L->getRefCount() == 1, "PropertySetHelper: wrong reference count" );

        // Test disposing
        {
            Reference < XPropertiesChangeListener >  x1( xPS_L, UNO_QUERY );
            Reference < XVetoableChangeListener >    x2( xPS_L, UNO_QUERY );

            xPS->addPropertyChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), xPS_L );
            Sequence<OUString> szPN( 2 );
            szPN.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") );
            szPN.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") );
            pPS->addPropertiesChangeListener( szPN, x1 );
            szPN = Sequence<OUString>();
            pPS->addPropertiesChangeListener( szPN, x1 );
            pPS->addVetoableChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), x2 );
            pPS->dispose();
        }
        OSL_ENSHURE( pPS_L->nDisposing == 4     , "PropertySetHelper: wrong disposing count" );
        OSL_ENSHURE( pPS_L->getRefCount() == 1  , "PropertySetHelper: wrong reference count" );
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
                aBool.setValue( &b , getCppuBooleanType() );
                xPS->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") ), aBool );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( PropertyVetoException & e )
            {
            }

            try
            {
                // Readonly raises a vetoable exception
                sal_Bool b = sal_True;
                Any aBool;
                aBool.setValue( &b , getCppuBooleanType() );
                // BOOL i s0
                pPS->setFastPropertyValue( PROPERTY_BOOL, aBool );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( PropertyVetoException & e )
            {
            }

            // set unknown property
            try
            {
                sal_Bool b = sal_True;
                Any aBool;
                aBool.setValue( &b , getCppuBooleanType() );
                xPS->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Does not exist") ), aBool );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( UnknownPropertyException & e )
            {
            }

            try
            {
                sal_Bool b = sal_True;
                Any aBool;
                aBool.setValue( &b , getCppuBooleanType() );
                pPS->setFastPropertyValue( 3, aBool );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( UnknownPropertyException & e )
            {
            }

            // get unknown property
            try
            {
                Any aBool;
                aBool = xPS->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Does not exist") ) );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( UnknownPropertyException & e )
            {
            }

            try
            {
                Any aBool;
                aBool = ((XFastPropertySet *)pPS)->getFastPropertyValue( 3 );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( UnknownPropertyException & e )
            {
            }

            // set property with invalid type
            try
            {
                Any aBool;
                xPS->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") ), aBool );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( IllegalArgumentException & e )
            {
            }

            try
            {
                Any aBool;
                pPS->setFastPropertyValue( PROPERTY_INT32, aBool );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( IllegalArgumentException & e )
            {
            }

            // narrowing conversion is not allowed!
            try
            {
                Any aINT32;
                aINT32 <<= (sal_Int32 ) 16;
                xPS->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), aINT32 );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( IllegalArgumentException & e )
            {
            }


            try
            {
                Any aINT32;
                aINT32 <<= (sal_Int32) 16;
                pPS->setFastPropertyValue( PROPERTY_INT16, aINT32 );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( IllegalArgumentException & e )
            {
            }


            Any aValue;
            aValue = xPS->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") ) );
            sal_Bool b = *( (sal_Bool*)aValue.getValue());
            OSL_ENSHURE( ! b, "PropertySetHelper: wrong BOOL value" );
            aValue = ((XFastPropertySet *)pPS)->getFastPropertyValue( PROPERTY_BOOL );

            b = *((sal_Bool*)aValue.getValue());
            OSL_ENSHURE( !b, "PropertySetHelper: wrong BOOL value" );

            sal_Int16 n16;
            aValue <<=(sal_Int16)22;
            xPS->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), aValue );
            aValue = xPS->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ) );
            aValue >>= n16;
            OSL_ENSHURE( 22 == n16 , "PropertySetHelper: wrong INT16 value" );
            aValue <<= (sal_Int16)44;
            ((XFastPropertySet *)pPS)->setFastPropertyValue( PROPERTY_INT16, aValue );

            aValue = ((XFastPropertySet *)pPS)->getFastPropertyValue( PROPERTY_INT16 );
            aValue >>= n16;
            OSL_ENSHURE( 44 == n16, "PropertySetHelper: wrong INT16 value" );

            // widening conversion
            aValue <<= (sal_Int16)55;
            xPS->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") ), aValue );
            aValue = xPS->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") ) );
            sal_Int32 n32;
            aValue >>= n32;
            OSL_ENSHURE( 55 == n32 , "PropertySetHelper: wrong INT32 value" );
            aValue <<= (sal_Int16)66;
            ((XFastPropertySet *)pPS)->setFastPropertyValue( PROPERTY_INT32, aValue );
            aValue = ((XFastPropertySet *)pPS)->getFastPropertyValue( PROPERTY_INT32 );
            aValue >>= n32;
            OSL_ENSHURE( 66 == n32, "PropertySetHelper: wrong INT32 value" );

            Sequence< OUString >valueNames = Sequence<OUString>( 3 );
            valueNames.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") );
            valueNames.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") );
            valueNames.getArray()[2] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") );
            Sequence< Any > aValues = pPS->getPropertyValues( valueNames );

            b = *((sal_Bool*)aValues.getConstArray()[0].getValue());
            aValues.getConstArray()[1] >>= n16;
            aValues.getConstArray()[2] >>= n32;

            OSL_ENSHURE( !b, "PropertySetHelper: wrong BOOL value" );
            OSL_ENSHURE( 44 == n16, "PropertySetHelper: wrong INT16 value" );
            OSL_ENSHURE( 66 == n32, "PropertySetHelper: wrong INT32 value" );
        }
        pPS->nINT32 = 0;
        pPS->nINT16 = 0;

        // Test add-remove listener
        {
            Reference < XVetoableChangeListener >  x2( xPS_L, UNO_QUERY );

            xPS->addPropertyChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), xPS_L );
            pPS->addVetoableChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), x2 );

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
            xPS->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), aValue );
            aValue <<= (sal_Int16) 44;
            ((XFastPropertySet *)pPS)->setFastPropertyValue( PROPERTY_INT16, aValue );
            aValue <<= (sal_Int16)100;// exception

            try
            {
                ((XFastPropertySet *)pPS)->setFastPropertyValue( PROPERTY_INT16, aValue );
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch( PropertyVetoException & e )
            {
            }

            OSL_ENSHURE( pPS_L->nCount == pPS_L->nCurrent, "not all listeners called" );
            pPS->nINT32 = 0;
            pPS->nINT16 = 0;
            pPS_L->nCount = 0;
            pPS_L->nCurrent = 0;
            pPS_L->pExceptedListenerValues = NULL;
            xPS->removePropertyChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), xPS_L );
            pPS->removeVetoableChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), x2 );
        }

        // Test multi property set listener
        {
            Reference < XPropertiesChangeListener > x1( xPS_L, UNO_QUERY );
            Reference < XVetoableChangeListener >  x2( xPS_L, UNO_QUERY );

            pPS->addVetoableChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ) , x2 );
            Sequence<OUString> szPN( 4 );
            szPN.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("BOOL") );
            szPN.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") );
            szPN.getArray()[2] = OUString( RTL_CONSTASCII_USTRINGPARAM("Does not exist") ); // must ne ignored by the addPropertiesChangeListener method
            szPN.getArray()[3] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") );
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
            szPN.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") );
            szPN.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT32") );
            Sequence< Any > aValues( 2 );
            aValues.getArray()[0] <<= (sal_Int16) 22;
            aValues.getArray()[1] <<= (sal_Int16) 44;
            pPS->setPropertyValues( szPN, aValues );
            OSL_ENSHURE( pPS_L->nCount == pPS_L->nCurrent, "not all listeners called" );

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
            OSL_ENSHURE( pPS_L->nCount == pPS_L->nCurrent, "not all listeners called" );


            //vetoable exception with multible
            szPN.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") );
            szPN.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") );
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
                OSL_ENSHURE( sal_False, "PropertySetHelper: exeption not thrown" );
            }
            catch ( PropertyVetoException & e )
            {
            }

            OSL_ENSHURE( pPS_L->nCount == pPS_L->nCurrent, "not all listeners called" );
            pPS->removePropertiesChangeListener( x1 );
            pPS->removeVetoableChangeListener( OUString( RTL_CONSTASCII_USTRINGPARAM("INT16") ), x2 );
        }
    }
}
