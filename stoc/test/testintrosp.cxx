/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include <sal/main.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <osl/diagnose.h>

#include <ModuleA/XIntroTest.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <stdio.h>
#include <string.h>

using namespace cppu;
using namespace ModuleA;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::reflection;
using namespace com::sun::star::container;
using namespace com::sun::star::beans::PropertyAttribute;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;
using ::rtl::OStringToOUString;


typedef WeakImplHelper4< XIntroTest, XPropertySet, XNameAccess, XIndexAccess > ImplIntroTestHelper;
typedef WeakImplHelper1< XPropertySetInfo > ImplPropertySetInfoHelper;


#define DEFAULT_INDEX_ACCESS_COUNT  10
#define DEFAULT_NAME_ACCESS_COUNT   5

#if OSL_DEBUG_LEVEL > 0
#define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
#else
#define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
#endif

//**************************************************************
//*** Hilfs-Funktion, um vom Type eine XIdlClass zu bekommen ***
//**************************************************************
Reference<XIdlClass> TypeToIdlClass( const Type& rType, const Reference< XMultiServiceFactory > & xMgr )
{
    static Reference< XIdlReflection > xRefl;

    // void als Default-Klasse eintragen
    Reference<XIdlClass> xRetClass;
    typelib_TypeDescription * pTD = 0;
    rType.getDescription( &pTD );
    if( pTD )
    {
        OUString sOWName( pTD->pTypeName );
        if( !xRefl.is() )
        {
            xRefl = Reference< XIdlReflection >( xMgr->createInstance(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.CoreReflection")) ), UNO_QUERY );
            OSL_ENSURE( xRefl.is(), "### no corereflection!" );
        }
        xRetClass = xRefl->forName( sOWName );
    }
    return xRetClass;
}


//****************************************************
//*** Hilfs-Funktion, um Any als UString auszugeben ***
//****************************************************
// ACHTUNG: Kann mal an eine zentrale Stelle uebernommen werden
// Wird zunaechst nur fuer einfache Datentypen ausgefuehrt

OUString AnyToString( const Any& aValue, sal_Bool bIncludeType, const Reference< XMultiServiceFactory > & xMgr )
{
    Type aValType = aValue.getValueType();
    TypeClass eType = aValType.getTypeClass();
    char pBuffer[50];

    OUString aRetStr;
    switch( eType )
    {
        case TypeClass_TYPE:            aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE TYPE"));               break;
        case TypeClass_INTERFACE:       aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE INTERFACE"));      break;
        case TypeClass_SERVICE:         aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE SERVICE"));            break;
        case TypeClass_STRUCT:          aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE STRUCT"));         break;
        case TypeClass_TYPEDEF:         aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE TYPEDEF"));            break;
        case TypeClass_UNION:           aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE UNION"));          break;
        case TypeClass_ENUM:            aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE ENUM"));               break;
        case TypeClass_EXCEPTION:       aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE EXCEPTION"));      break;
        case TypeClass_ARRAY:           aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE ARRAY"));          break;
        case TypeClass_SEQUENCE:        aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE SEQUENCE"));           break;
        case TypeClass_VOID:            aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE void"));               break;
        case TypeClass_ANY:             aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE any"));                break;
        case TypeClass_UNKNOWN:         aRetStr = OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE unknown"));            break;
        case TypeClass_BOOLEAN:
        {
            sal_Bool b = *(sal_Bool*)aValue.getValue();
            aRetStr = OUString::valueOf( b );
            break;
        }
        case TypeClass_CHAR:
        {
            sal_Unicode c = *(sal_Unicode*)aValue.getValue();
            aRetStr = OUString::valueOf( c );
            break;
        }
        case TypeClass_STRING:
        {
            aValue >>= aRetStr;
            break;
        }
        case TypeClass_FLOAT:
        {
            float f(0.0);
            aValue >>= f;
            snprintf( pBuffer, sizeof( pBuffer ), "%f", f );
            aRetStr = OUString( pBuffer, strlen( pBuffer ), RTL_TEXTENCODING_ASCII_US );
            break;
        }
        case TypeClass_DOUBLE:
        {
            double d(0.0);
            aValue >>= d;
            snprintf( pBuffer, sizeof( pBuffer ), "%f", d );
            aRetStr = OUString( pBuffer, strlen( pBuffer ), RTL_TEXTENCODING_ASCII_US );
            break;
        }
        case TypeClass_BYTE:
        {
            sal_Int8 n(0);
            aValue >>= n;
            aRetStr = OUString::valueOf( (sal_Int32) n );
            break;
        }
        case TypeClass_SHORT:
        {
            sal_Int16 n(0);
            aValue >>= n;
            aRetStr = OUString::valueOf( (sal_Int32) n );
            break;
        }
        case TypeClass_LONG:
        {
            sal_Int32 n(0);
            aValue >>= n;
            aRetStr = OUString::valueOf( n );
            break;
        }
    default: ;
    }

    if( bIncludeType )
    {
        Reference< XIdlClass > xIdlClass = TypeToIdlClass( aValType, xMgr );
        aRetStr = aRetStr + OUString( OUString(RTL_CONSTASCII_USTRINGPARAM(" (Typ: ")) ) + xIdlClass->getName() + OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
    }
    return aRetStr;
}

//*****************************************
//*** XPropertySetInfo fuer Test-Klasse ***
//*****************************************

class ImplPropertySetInfo : public ImplPropertySetInfoHelper
{
     friend class ImplIntroTest;

      Reference< XMultiServiceFactory > mxMgr;

public:
    ImplPropertySetInfo( const Reference< XMultiServiceFactory > & xMgr )
        : mxMgr( xMgr ) {}

    // Methods of XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(  )
        throw(RuntimeException);
    virtual Property SAL_CALL getPropertyByName( const OUString& aName )
        throw(UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )
        throw(RuntimeException);
};


Sequence< Property > ImplPropertySetInfo::getProperties(void)
    throw( RuntimeException )
{
    static Sequence<Property> * pSeq = NULL;

    if( !pSeq )
    {
        // die Informationen für die Properties "Width", "Height" und "Name" anlegen
        pSeq = new Sequence<Property>( 3 );
        Property * pAry = pSeq->getArray();

        pAry[0].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("Factor"));
        pAry[0].Handle = -1;
        pAry[0].Type = getCppuType( (double*) NULL );
        pAry[0].Attributes = BOUND | TRANSIENT;

        pAry[1].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("MyCount"));
        pAry[1].Handle = -1;
        pAry[1].Type = getCppuType( (sal_Int32*) NULL );
        pAry[1].Attributes = BOUND | TRANSIENT;

        pAry[2].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("Info"));
        pAry[2].Handle = -1;
        pAry[2].Type = getCppuType( (OUString*) NULL );
        pAry[2].Attributes = TRANSIENT;
    }
    // Die Information über alle drei Properties liefern.
    return *pSeq;
}

Property ImplPropertySetInfo::getPropertyByName(const OUString& Name)
    throw( UnknownPropertyException, RuntimeException )
{
    Sequence<Property> aSeq = getProperties();
    const Property * pAry = aSeq.getConstArray();

    for( sal_Int32 i = aSeq.getLength(); i--; )
    {
        if( pAry[i].Name == Name )
            return pAry[i];
    }
    // Property unbekannt, also leere liefern
    return Property();
}

sal_Bool ImplPropertySetInfo::hasPropertyByName(const OUString& Name)
    throw( RuntimeException )
{
    Sequence<Property> aSeq = getProperties();
    const Property * pAry = aSeq.getConstArray();

    for( sal_Int32 i = aSeq.getLength(); i--; )
    {
        if( pAry[i].Name == Name )
            return sal_True;
    }
    // Property unbekannt, also leere liefern
    return sal_False;
}

//*****************************************************************
class ImplIntroTest : public ImplIntroTestHelper
{
      Reference< XMultiServiceFactory > mxMgr;

    friend class ImplPropertySetInfo;

    // Properties fuer das PropertySet
    Any aAnyArray[10];

    Reference< XPropertySetInfo > m_xMyInfo;

    OUString m_ObjectName;

    sal_Int16 m_nMarkusAge;
    sal_Int16 m_nMarkusChildrenCount;

    long m_lDroenk;
    sal_Int16 m_nBla;
    sal_Int16 m_nBlub;
    sal_Int16 m_nGulp;
    sal_Int16 m_nLaber;
    TypeClass eTypeClass;
    Sequence< OUString > aStringSeq;
    Sequence< Sequence< Sequence< sal_Int16 > > > aMultSeq;
    Reference< XIntroTest > m_xIntroTest;

    // Daten fuer NameAccess
    Reference< XIntroTest >* pNameAccessTab;

    // Daten fuer IndexAccess
    Reference< XIntroTest >* pIndexAccessTab;
    sal_Int16 iIndexAccessCount;

    // struct-Properties
    Property m_aFirstStruct;
    PropertyValue m_aSecondStruct;

    // Listener merken (zunaechst einfach, nur einen pro Property)
    Reference< XPropertyChangeListener > aPropChangeListener;
    OUString aPropChangeListenerStr;
    Reference< XVetoableChangeListener > aVetoPropChangeListener;
    OUString aVetoPropChangeListenerStr;

    void Init( void );

public:
    ImplIntroTest( const Reference< XMultiServiceFactory > & xMgr )
        : mxMgr( xMgr )
    {
        Init();
    }

    // Trotz virtual inline, um Schreibarbeit zu sparen (nur fuer Testzwecke)
    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
        throw(RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
            {}
    virtual void SAL_CALL removePropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
            {}
    virtual void SAL_CALL addVetoableChangeListener( const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
            {}
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
            {}

    // XIntroTest-Methoden
    // Attributes
    virtual OUString SAL_CALL getObjectName() throw(RuntimeException)
        { return m_ObjectName; }
    virtual void SAL_CALL setObjectName( const OUString& _objectname ) throw(RuntimeException)
        { m_ObjectName = _objectname; }
    virtual OUString SAL_CALL getFirstName()
        throw(RuntimeException);
    virtual OUString SAL_CALL getLastName() throw(RuntimeException)
        { return OUString( OUString(RTL_CONSTASCII_USTRINGPARAM("Meyer")) ); }
    virtual sal_Int16 SAL_CALL getAge() throw(RuntimeException)
        { return m_nMarkusAge; }
    virtual sal_Int16 SAL_CALL getChildrenCount() throw(RuntimeException)
        { return m_nMarkusChildrenCount; }
    virtual void SAL_CALL setChildrenCount( sal_Int16 _childrencount ) throw(RuntimeException)
        { m_nMarkusChildrenCount = _childrencount; }
    virtual Property SAL_CALL getFirstStruct() throw(RuntimeException)
        { return m_aFirstStruct; }
    virtual void SAL_CALL setFirstStruct( const Property& _firststruct ) throw(RuntimeException)
        { m_aFirstStruct = _firststruct; }
    virtual PropertyValue SAL_CALL getSecondStruct() throw(RuntimeException)
        { return m_aSecondStruct; }
    virtual void SAL_CALL setSecondStruct( const PropertyValue& _secondstruct ) throw(RuntimeException)
        { m_aSecondStruct = _secondstruct; }

    // Methods
    virtual void SAL_CALL writeln( const OUString& Text )
        throw(RuntimeException);
    virtual sal_Int32 SAL_CALL getDroenk(  ) throw(RuntimeException)
        { return m_lDroenk; }
    virtual Reference< ::ModuleA::XIntroTest > SAL_CALL getIntroTest(  ) throw(RuntimeException);
    virtual sal_Int32 SAL_CALL getUps( sal_Int32 l ) throw(RuntimeException)
        { return 2*l; }
    virtual void SAL_CALL setDroenk( sal_Int32 l ) throw(RuntimeException)
        { m_lDroenk = l; }
    virtual sal_Int16 SAL_CALL getBla(  ) throw(RuntimeException)
        { return m_nBla; }
    virtual void SAL_CALL setBla( sal_Int32 n ) throw(RuntimeException)
        { m_nBla = (sal_Int16)n; }
    virtual sal_Int16 SAL_CALL getBlub(  ) throw(RuntimeException)
        { return m_nBlub; }
    virtual void SAL_CALL setBlub( sal_Int16 n ) throw(RuntimeException)
        { m_nBlub = n; }
    virtual sal_Int16 SAL_CALL getGulp(  ) throw(RuntimeException)
        { return m_nGulp; }
    virtual sal_Int16 SAL_CALL setGulp( sal_Int16 n ) throw(RuntimeException)
        { m_nGulp = n; return 1; }
    virtual TypeClass SAL_CALL getTypeClass( sal_Int16 /*n*/ ) throw(RuntimeException)
        { return eTypeClass; }
    virtual void SAL_CALL setTypeClass( TypeClass t, double /*d1*/, double /*d2*/ ) throw(RuntimeException)
        { eTypeClass = t; }
    virtual Sequence< OUString > SAL_CALL getStrings(  ) throw(RuntimeException)
        { return aStringSeq; }
    virtual void SAL_CALL setStrings( const Sequence< OUString >& Strings ) throw(RuntimeException)
        { aStringSeq = Strings; }
    virtual void SAL_CALL setStringsPerMethod( const Sequence< OUString >& Strings, sal_Int16 /*n*/ ) throw(RuntimeException)
        { aStringSeq = Strings; }
    virtual Sequence< Sequence< Sequence< sal_Int16 > > > SAL_CALL getMultiSequence(  ) throw(RuntimeException)
        { return aMultSeq; }
    virtual void SAL_CALL setMultiSequence( const Sequence< Sequence< Sequence< sal_Int16 > > >& Seq ) throw(RuntimeException)
        { aMultSeq = Seq; }
    virtual void SAL_CALL addPropertiesChangeListener( const Sequence< OUString >& PropertyNames, const Reference< XPropertiesChangeListener >& Listener )
        throw(RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& Listener )
        throw(RuntimeException);


    // Methods of XElementAccess
    virtual Type SAL_CALL getElementType(  )
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(RuntimeException);

    // XNameAccess-Methoden
    // Methods
    virtual Any SAL_CALL getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames(  )
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(RuntimeException);

    // XIndexAccess-Methoden
    // Methods
    virtual sal_Int32 SAL_CALL getCount(  )
        throw(RuntimeException);
    virtual Any SAL_CALL getByIndex( sal_Int32 Index )
        throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException);
};

void ImplIntroTest::Init( void )
{
    // Eindeutigen Namen verpassen
    static sal_Int32 nObjCount = 0;
    OUString aName( RTL_CONSTASCII_USTRINGPARAM("IntroTest-Obj Nr. ") );
    aName += OUString::valueOf( nObjCount );
    setObjectName( aName );

    // Properties initialisieren
    aAnyArray[0] <<= 3.14;
    aAnyArray[1] <<= (sal_Int32)42;
    aAnyArray[2] <<= OUString( OUString(RTL_CONSTASCII_USTRINGPARAM("Hallo")) );

    // Einmal fuer den internen Gebrauch die PropertySetInfo abholen
    m_xMyInfo = getPropertySetInfo();
    m_xMyInfo->acquire();       // sonst raucht es am Programm-Ende ab

    m_nMarkusAge = 33;
    m_nMarkusChildrenCount = 2;

    m_lDroenk = 314;
    m_nBla = 42;
    m_nBlub = 111;
    m_nGulp = 99;
    m_nLaber = 1;
    eTypeClass = TypeClass_INTERFACE;

    // String-Sequence intitialisieren
    aStringSeq.realloc( 3 );
    OUString* pStr = aStringSeq.getArray();
    pStr[ 0 ] = OUString( OUString(RTL_CONSTASCII_USTRINGPARAM("String 0")) );
    pStr[ 1 ] = OUString( OUString(RTL_CONSTASCII_USTRINGPARAM("String 1")) );
    pStr[ 2 ] = OUString( OUString(RTL_CONSTASCII_USTRINGPARAM("String 2")) );

    // structs initialisieren
    m_aFirstStruct.Name = OUString(RTL_CONSTASCII_USTRINGPARAM("FirstStruct-Name"));
    m_aFirstStruct.Handle = 77777;
    //XIdlClassRef Type;
    m_aFirstStruct.Attributes = -222;

    //XInterfaceRef Source;
    Any Value;
    Value <<= 2.718281828459;
    m_aSecondStruct.Value = Value;
    //XIdlClassRef ListenerType;
    m_aSecondStruct.State = PropertyState_DIRECT_VALUE;

    // IndexAccess
    iIndexAccessCount = DEFAULT_INDEX_ACCESS_COUNT;
    pIndexAccessTab = NULL;
    pNameAccessTab = NULL;
}

Reference< XPropertySetInfo > ImplIntroTest::getPropertySetInfo()
    throw(RuntimeException)
{
    static ImplPropertySetInfo aInfo( mxMgr );
    // Alle Objekt haben die gleichen Properties, deshalb kann
    // die Info für alle gleich sein
    return &aInfo;
}

void ImplIntroTest::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if( aPropChangeListener.is() && aPropertyName == aPropChangeListenerStr )
    {
        PropertyChangeEvent aEvt;
        aEvt.Source = (OWeakObject*)this;
        aEvt.PropertyName = aPropertyName;
        aEvt.PropertyHandle = 0L;
        aPropChangeListener->propertyChange( aEvt );
    }
    if( aVetoPropChangeListener.is() && aPropertyName == aVetoPropChangeListenerStr )
    {
        PropertyChangeEvent aEvt;
        aEvt.Source = (OWeakObject*)this;
        aEvt.PropertyName = aVetoPropChangeListenerStr;
        aEvt.PropertyHandle = 0L;
        aVetoPropChangeListener->vetoableChange( aEvt );
    }

    Sequence<Property> aPropSeq = m_xMyInfo->getProperties();
    sal_Int32 nLen = aPropSeq.getLength();
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        Property aProp = aPropSeq.getArray()[ i ];
        if( aProp.Name == aPropertyName )
            aAnyArray[i] = aValue;
    }
}

Any ImplIntroTest::getPropertyValue( const OUString& PropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Sequence<Property> aPropSeq = m_xMyInfo->getProperties();
    sal_Int32 nLen = aPropSeq.getLength();
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        Property aProp = aPropSeq.getArray()[ i ];
        if( aProp.Name == PropertyName )
            return aAnyArray[i];
    }
    return Any();
}

OUString ImplIntroTest::getFirstName(void)
    throw(RuntimeException)
{
    return OUString( OUString(RTL_CONSTASCII_USTRINGPARAM("Markus")) );
}

void ImplIntroTest::writeln( const OUString& Text )
    throw(RuntimeException)
{
    OString aStr( Text.getStr(), Text.getLength(), RTL_TEXTENCODING_ASCII_US );

    printf( "%s", aStr.getStr() );
}

Reference< XIntroTest > ImplIntroTest::getIntroTest()
    throw(RuntimeException)
{
    if( !m_xIntroTest.is() )
        m_xIntroTest = new ImplIntroTest( mxMgr );
    return m_xIntroTest;
}

// Methoden von XElementAccess
Type ImplIntroTest::getElementType(  )
    throw(RuntimeException)
{
    // TODO
    Type aRetType;
    return aRetType;
    //return Reference< XIdlClass >();
    //return Void_getReflection()->getIdlClass();
}

sal_Bool ImplIntroTest::hasElements(  )
    throw(RuntimeException)
{
    return sal_True;
}

// XNameAccess-Methoden
sal_Int32 getIndexForName( const OUString& ItemName )
{
    OUString aLeftStr = ItemName.copy( 0, 4 );
    if( aLeftStr == OUString(RTL_CONSTASCII_USTRINGPARAM("Item")) )
    {
        // TODO
        OUString aNumStr = ItemName.copy( 4 );
        //sal_Int32 iIndex = (INT32)UStringToString( aNumStr, CHARSET_SYSTEM );
        //if( iIndex < DEFAULT_NAME_ACCESS_COUNT )
            //return iIndex;
    }
    return -1;
}


Any ImplIntroTest::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    Any aRetAny;

    if( !pNameAccessTab  )
        ((ImplIntroTest*)this)->pNameAccessTab  = new Reference< XIntroTest >[ DEFAULT_NAME_ACCESS_COUNT ];

    sal_Int32 iIndex = getIndexForName( aName );
    if( iIndex != -1 )
    {
        if( !pNameAccessTab[iIndex].is() )
        {
            ImplIntroTest* p = new ImplIntroTest( mxMgr );
            OUString aName2( RTL_CONSTASCII_USTRINGPARAM("IntroTest by Name-Access, Index = ") );
            aName2 += OUString::valueOf( iIndex );
            p->setObjectName( aName2 );
            pNameAccessTab[iIndex] = p;
        }

        Reference< XIntroTest > xRet = pNameAccessTab[iIndex];
        aRetAny = makeAny( xRet );
    }
    return aRetAny;
}

Sequence< OUString > ImplIntroTest::getElementNames(  )
    throw(RuntimeException)
{
    Sequence<OUString> aStrSeq( DEFAULT_NAME_ACCESS_COUNT );
    OUString* pStr = aStrSeq.getArray();
    for( sal_Int32 i = 0 ; i < DEFAULT_NAME_ACCESS_COUNT ; i++ )
    {
        OUString aName( RTL_CONSTASCII_USTRINGPARAM("Item") );
        aName += OUString::valueOf( i );
        pStr[i] = aName;
    }
    return aStrSeq;
}

sal_Bool ImplIntroTest::hasByName( const OUString& aName )
    throw(RuntimeException)
{
    return ( getIndexForName( aName ) != -1 );
}

// XIndexAccess-Methoden
sal_Int32 ImplIntroTest::getCount(  )
    throw(RuntimeException)
{
    return iIndexAccessCount;
}

Any ImplIntroTest::getByIndex( sal_Int32 Index )
    throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    Any aRetAny;

    if( !pIndexAccessTab )
        ((ImplIntroTest*)this)->pIndexAccessTab = new Reference< XIntroTest >[ iIndexAccessCount ];

    if( Index < iIndexAccessCount )
    {
        if( !pNameAccessTab[Index].is() )
        {
            ImplIntroTest* p = new ImplIntroTest( mxMgr );
            OUString aName( RTL_CONSTASCII_USTRINGPARAM("IntroTest by Index-Access, Index = ") );
            aName += OUString::valueOf( Index );
            p->setObjectName( aName );
            pIndexAccessTab[Index] = p;
        }
        Reference< XIntroTest > xRet = pIndexAccessTab[Index];
        aRetAny = makeAny( xRet );
    }
    return aRetAny;
}

void ImplIntroTest::addPropertiesChangeListener( const Sequence< OUString >& /*PropertyNames*/,
                                                 const Reference< XPropertiesChangeListener >& /*Listener*/ )
        throw(RuntimeException)
{
}

void ImplIntroTest::removePropertiesChangeListener
( const Reference< XPropertiesChangeListener >& /*Listener*/ )
        throw(RuntimeException)
{
}



struct DefItem
{
    char const * pName;
    sal_Int32 nConcept;
};

// Spezial-Wert fuer Method-Concept, um "normale" Funktionen kennzeichnen zu koennen
#define  MethodConcept_NORMAL_IMPL      0x80000000


// Test-Objekt liefern
Any getIntrospectionTestObject( const Reference< XMultiServiceFactory > & xMgr )
{
    Any aObjAny;
    Reference< XIntroTest > xTestObj = new ImplIntroTest( xMgr );
    aObjAny.setValue( &xTestObj, ::getCppuType( (const Reference< XIntroTest > *)0 ) );
    return aObjAny;
}

static sal_Bool test_introsp( Reference< XMultiServiceFactory > xMgr,
                              Reference< XIdlReflection > /*xRefl*/, Reference< XIntrospection > xIntrospection )
{
    DefItem pPropertyDefs[] =
    {
        { "Factor", PropertyConcept::PROPERTYSET },
        { "MyCount", PropertyConcept::PROPERTYSET },
        { "Info", PropertyConcept::PROPERTYSET },
        { "ObjectName", PropertyConcept::ATTRIBUTES },
        { "FirstName", PropertyConcept::ATTRIBUTES },
        { "LastName", PropertyConcept::ATTRIBUTES },
        { "Age", PropertyConcept::ATTRIBUTES },
        { "ChildrenCount", PropertyConcept::ATTRIBUTES },
        { "FirstStruct", PropertyConcept::ATTRIBUTES },
        { "SecondStruct", PropertyConcept::ATTRIBUTES },
        { "Droenk", PropertyConcept::METHODS },
        { "IntroTest", PropertyConcept::METHODS },
        { "Bla", PropertyConcept::METHODS },
        { "Blub", PropertyConcept::METHODS },
        { "Gulp", PropertyConcept::METHODS },
        { "Strings", PropertyConcept::METHODS },
        { "MultiSequence", PropertyConcept::METHODS },
        { "PropertySetInfo", PropertyConcept::METHODS },
        { "ElementType", PropertyConcept::METHODS },
        { "ElementNames", PropertyConcept::METHODS },
        { "Count", PropertyConcept::METHODS },
        { "Types", PropertyConcept::METHODS },
        { "ImplementationId", PropertyConcept::METHODS },
        { NULL, 0 }
    };

    char const * pDemandedPropVals[] =
    {
        "3.140000",
        "42",
        "Hallo",
        "IntroTest-Obj Nr. 0",
        "Markus",
        "Meyer",
        "33",
        "2",
        "TYPE STRUCT",
        "TYPE STRUCT",
        "314",
        "TYPE INTERFACE",
        "42",
        "111",
        "99",
        "TYPE SEQUENCE",
        "TYPE SEQUENCE",
        "TYPE INTERFACE",
        "TYPE TYPE",
        "TYPE SEQUENCE",
        "10",
        "TYPE SEQUENCE",
        "TYPE SEQUENCE",
    };

    char const * pDemandedModifiedPropVals[] =
    {
        "4.140000",
        "43",
        "Hallo (Modified!)",
        "IntroTest-Obj Nr. 0 (Modified!)",
        "Markus",
        "Meyer",
        "33",
        "3",
        "Wert wurde nicht modifiziert",
        "Wert wurde nicht modifiziert",
        "315",
        "Wert wurde nicht modifiziert",
        "42",
        "112",
        "99",
        "Wert wurde nicht modifiziert",
        "Wert wurde nicht modifiziert",
        "Wert wurde nicht modifiziert",
        "Wert wurde nicht modifiziert",
        "Wert wurde nicht modifiziert",
        "10",
        "Wert wurde nicht modifiziert"
        "Wert wurde nicht modifiziert"
    };

    char const * pDemandedPropTypes[] =
    {
        "double",
        "long",
        "string",
        "string",
        "string",
        "string",
        "short",
        "short",
        "com.sun.star.beans.Property",
        "com.sun.star.beans.PropertyValue",
        "long",
        "ModuleA.XIntroTest",
        "short",
        "short",
        "short",
        "[]string",
        "[][][]short",
        "com.sun.star.beans.XPropertySetInfo",
        "type",
        "[]string",
        "long",
        "[]type",
        "[]byte",
    };

    DefItem pMethodDefs[] =
    {
        { "queryInterface", MethodConcept_NORMAL_IMPL },
        { "acquire", MethodConcept::DANGEROUS },
        { "release", MethodConcept::DANGEROUS },
        { "writeln", MethodConcept_NORMAL_IMPL },
        { "getDroenk", MethodConcept::PROPERTY },
        { "getIntroTest", MethodConcept::PROPERTY },
        { "getUps", MethodConcept_NORMAL_IMPL },
        { "setDroenk", MethodConcept::PROPERTY },
        { "getBla", MethodConcept::PROPERTY },
        { "setBla", MethodConcept_NORMAL_IMPL },
        { "getBlub", MethodConcept::PROPERTY },
        { "setBlub", MethodConcept::PROPERTY },
        { "getGulp", MethodConcept::PROPERTY },
        { "setGulp", MethodConcept_NORMAL_IMPL },
        { "getTypeClass", MethodConcept_NORMAL_IMPL },
        { "setTypeClass", MethodConcept_NORMAL_IMPL },
        { "getStrings", MethodConcept::PROPERTY },
        { "setStrings", MethodConcept::PROPERTY },
        { "setStringsPerMethod", MethodConcept_NORMAL_IMPL },
        { "getMultiSequence", MethodConcept::PROPERTY },
        { "setMultiSequence", MethodConcept::PROPERTY },
        { "addPropertiesChangeListener", MethodConcept::LISTENER },
        { "removePropertiesChangeListener", MethodConcept::LISTENER },
        { "getPropertySetInfo", MethodConcept::PROPERTY },
        { "setPropertyValue", MethodConcept_NORMAL_IMPL },
        { "getPropertyValue", MethodConcept_NORMAL_IMPL },
        { "addPropertyChangeListener", MethodConcept::LISTENER },
        { "removePropertyChangeListener", MethodConcept::LISTENER },
        { "addVetoableChangeListener", MethodConcept::LISTENER },
        { "removeVetoableChangeListener", MethodConcept::LISTENER },
        { "getElementType", MethodConcept::PROPERTY | MethodConcept::NAMECONTAINER| MethodConcept::INDEXCONTAINER | MethodConcept::ENUMERATION  },
        { "hasElements", MethodConcept::NAMECONTAINER | MethodConcept::INDEXCONTAINER | MethodConcept::ENUMERATION },
        { "getByName", MethodConcept::NAMECONTAINER },
        { "getElementNames", MethodConcept::PROPERTY | MethodConcept::NAMECONTAINER },
        { "hasByName", MethodConcept::NAMECONTAINER },
        { "getCount", MethodConcept::PROPERTY | MethodConcept::INDEXCONTAINER },
        { "getByIndex", MethodConcept::INDEXCONTAINER },
        { "getTypes", MethodConcept::PROPERTY },
        { "getImplementationId", MethodConcept::PROPERTY },
        { "queryAdapter", MethodConcept_NORMAL_IMPL },
        { NULL, 0 }
    };

    OString aErrorStr;

    //******************************************************

    // Test-Objekt anlegen
    Any aObjAny = getIntrospectionTestObject( xMgr );

    // Introspection-Service unspecten
    Reference< XIntrospectionAccess > xAccess = xIntrospection->inspect( aObjAny );
    xAccess = xIntrospection->inspect( aObjAny );
    xAccess = xIntrospection->inspect( aObjAny );
    TEST_ENSHURE( xAccess.is(), "introspection failed, no XIntrospectionAccess returned" );
    if( !xAccess.is() )
        return sal_False;

    // Ergebnis der Introspection pruefen

    // XPropertySet-UIK ermitteln
    Type aType = getCppuType( (Reference< XPropertySet >*) NULL );

    Reference< XInterface > xPropSetIface = xAccess->queryAdapter( aType );
    Reference< XPropertySet > xPropSet( xPropSetIface, UNO_QUERY );
    TEST_ENSHURE( xPropSet.is(), "Could not get XPropertySet by queryAdapter()" );

    // XExactName
    Reference< XExactName > xExactName( xAccess, UNO_QUERY );
    TEST_ENSHURE( xExactName.is(), "Introspection unterstuetzt kein ExactName" );

    // Schleife ueber alle Kombinationen von Concepts
    for( sal_Int32 nConcepts = 0 ; nConcepts < 16 ; nConcepts++ )
    {
        // Wieviele Properties sollten es sein
        sal_Int32 nDemandedPropCount = 0;
        sal_Int32 iList = 0;
        while( pPropertyDefs[ iList ].pName )
        {
            if( pPropertyDefs[ iList ].nConcept & nConcepts )
                nDemandedPropCount++;
            iList++;
        }

        if( xPropSet.is() )
        {
            Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
            Sequence<Property> aRetSeq = xAccess->getProperties( nConcepts );

            sal_Int32 nLen = aRetSeq.getLength();

            aErrorStr  = "Expected to find ";
            aErrorStr += OString::valueOf( nDemandedPropCount );
            aErrorStr += " properties but found ";
            aErrorStr += OString::valueOf( nLen );
            TEST_ENSHURE( nLen == nDemandedPropCount, aErrorStr.getStr() );

            const Property* pProps = aRetSeq.getConstArray();
            Any aPropVal;
            sal_Int32 i;
            iList = 0;
            for( i = 0 ; i < nLen ; i++ )
            {
                const Property aProp = pProps[ i ];

                // Naechste Passende Methode in der Liste suchen
                while( pPropertyDefs[ iList ].pName )
                {
                    if( pPropertyDefs[ iList ].nConcept & nConcepts )
                        break;
                    iList++;
                }
                sal_Int32 iDemanded = iList;
                iList++;

                OUString aPropName = aProp.Name;
                OString aNameStr( aPropName.getStr(), aPropName.getLength(), RTL_TEXTENCODING_ASCII_US );

                OString aDemandedName = pPropertyDefs[ iDemanded ].pName;
                aErrorStr  = "Expected property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\", found \"";
                aErrorStr += aNameStr;
                aErrorStr += "\"";
                TEST_ENSHURE( aNameStr == aDemandedName, aErrorStr.getStr() );

                Type aPropType = aProp.Type;
                OString aTypeNameStr( OUStringToOString(aPropType.getTypeName(), RTL_TEXTENCODING_ASCII_US) );
                OString aDemandedTypeNameStr = pDemandedPropTypes[ iDemanded ];
                aErrorStr  = "Property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\", expected type >";
                aErrorStr += aDemandedTypeNameStr;
                aErrorStr += "< found type >";
                aErrorStr += aTypeNameStr;
                aErrorStr += "<";
                TEST_ENSHURE( aTypeNameStr == aDemandedTypeNameStr, aErrorStr.getStr() );

                // Wert des Property lesen und ausgeben
                aPropVal = xPropSet->getPropertyValue( aPropName );

                OString aValStr = OUStringToOString( AnyToString( aPropVal, sal_False, xMgr ), RTL_TEXTENCODING_ASCII_US );
                OString aDemandedValStr = pDemandedPropVals[ iDemanded ];
                aErrorStr  = "Property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\", expected val >";
                aErrorStr += aDemandedValStr;
                aErrorStr += "< found val >";
                aErrorStr += aValStr;
                aErrorStr += "<";
                TEST_ENSHURE( aValStr == aDemandedValStr, aErrorStr.getStr() );

                // Wert pruefen und typgerecht modifizieren
                TypeClass eType = aPropVal.getValueType().getTypeClass();
                Any aNewVal;
                sal_Bool bModify = sal_True;
                switch( eType )
                {
                    case TypeClass_STRING:
                    {
                        OUString aStr;
                        aPropVal >>= aStr;
                        aStr = aStr + OUString(RTL_CONSTASCII_USTRINGPARAM(" (Modified!)"));
                        aNewVal <<= aStr;
                        break;
                    }
                    case TypeClass_DOUBLE:
                    {
                        double d(0.0);
                        aPropVal >>= d;
                        aNewVal <<= d + 1.0;
                        break;
                    }
                    case TypeClass_SHORT:
                    {
                        sal_Int16 n(0);
                        aPropVal >>= n;
                        aNewVal <<= sal_Int16( n + 1 );
                        break;
                    }
                    case TypeClass_LONG:
                    {
                        sal_Int32 n(0);
                        aPropVal >>= n;
                        aNewVal <<= sal_Int32( n + 1 );
                        break;
                    }
                    default:
                        bModify = sal_False;
                        break;
                }

                // Modifizieren nur beim letzten Durchlauf
                if( nConcepts == 15 )
                {
                    // XExactName pruefen, dafuer alles gross machen
                    // (Introspection ist mit LowerCase implementiert, also anders machen)
                    OUString aUpperUStr = aPropName.toAsciiUpperCase();
                    OUString aExactName = xExactName->getExactName( aUpperUStr );
                    if( aExactName != aPropName )
                    {
                        aErrorStr  = "Property \"";
                        aErrorStr += OUStringToOString( aPropName, RTL_TEXTENCODING_ASCII_US );
                        aErrorStr += "\", not found as \"";
                        aErrorStr += OUStringToOString(aUpperUStr, RTL_TEXTENCODING_ASCII_US );
                        aErrorStr += "\" using XExactName";
                        TEST_ENSHURE( sal_False, aErrorStr.getStr() );
                    }
                }
                else
                {
                    bModify = sal_False;
                }

                // Neuen Wert setzen, wieder lesen und ausgeben
                if( bModify )
                {
                    // UnknownPropertyException bei ReadOnly-Properties abfangen
                    try
                    {
                        xPropSet->setPropertyValue( aPropName, aNewVal );
                    }
                    catch(UnknownPropertyException e1)
                    {
                    }

                    aPropVal = xPropSet->getPropertyValue( aPropName );

                    OUString aStr = AnyToString( aPropVal, sal_False, xMgr );
                    OString aModifiedValStr = OUStringToOString( aStr, RTL_TEXTENCODING_ASCII_US );
                    OString aDemandedModifiedValStr = pDemandedModifiedPropVals[ i ];
                    aErrorStr  = "Property \"";
                    aErrorStr += aDemandedName;
                    aErrorStr += "\", expected modified val >";
                    aErrorStr += aDemandedModifiedValStr;
                    aErrorStr += "< found val >";
                    aErrorStr += aModifiedValStr;
                    aErrorStr += "<";
                    TEST_ENSHURE( aModifiedValStr == aDemandedModifiedValStr, aErrorStr.getStr() );
                }

                // Checken, ob alle Properties auch einzeln gefunden werden
                aErrorStr  = "property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\" not found with hasProperty()";
                OUString aWDemandedName = OStringToOUString(aDemandedName, RTL_TEXTENCODING_ASCII_US );
                sal_Bool bProperty = xAccess->hasProperty( aWDemandedName, nConcepts );
                TEST_ENSHURE( bProperty, aErrorStr.getStr() );

                aErrorStr  = "property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\" not equal to same Property in sequence returned by getProperties()";
                try
                {
                    Property aGetProp = xAccess->getProperty( aWDemandedName, nConcepts );
                }
                catch (RuntimeException e1)
                {
                    aErrorStr  = "property \"";
                    aErrorStr += aDemandedName;
                    aErrorStr += "\", exception was thrown when trying getProperty()";
                    TEST_ENSHURE( sal_False, aErrorStr.getStr() );
                }

            }
        }
    }

    // Schleife ueber alle Kombinationen von Concepts
    for( sal_Int32 nConcepts = 0 ; nConcepts < 128 ; nConcepts++ )
    {
        // Das 2^6-Bit steht fuer "den Rest"
        sal_Int32 nRealConcepts = nConcepts;
        if( nConcepts & 0x40 )
            nRealConcepts |= (0xFFFFFFFF - 0x3F);

        // Wieviele Methoden sollten es sein
        sal_Int32 nDemandedMethCount = 0;
        sal_Int32 iList = 0;
        while( pMethodDefs[ iList ].pName )
        {
            if( pMethodDefs[ iList ].nConcept & nRealConcepts )
                nDemandedMethCount++;
            iList++;
        }

        // Methoden-Array ausgeben
        Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods( nRealConcepts );
        sal_Int32 nLen = aMethodSeq.getLength();

        aErrorStr  = "Expected to find ";
        aErrorStr += OString::valueOf( nDemandedMethCount );
        aErrorStr += " methods but found ";
        aErrorStr += OString::valueOf( nLen );
        TEST_ENSHURE( nLen == nDemandedMethCount, aErrorStr.getStr() );

        const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();
        sal_Int32 i;
        iList = 0;

        for( i = 0 ; i < nLen ; i++ )
        {
            // Methode ansprechen
            const Reference< XIdlMethod >& rxMethod = pMethods[i];

            // Methode ausgeben
            OUString aMethName = rxMethod->getName();
            OString aNameStr = OUStringToOString(aMethName, RTL_TEXTENCODING_ASCII_US );

            // Naechste Passende Methode in der Liste suchen
            while( pMethodDefs[ iList ].pName )
            {
                if( pMethodDefs[ iList ].nConcept & nRealConcepts )
                    break;
                iList++;
            }
            OString aDemandedName = pMethodDefs[ iList ].pName;
            iList++;

            aErrorStr  = "Expected method \"";
            aErrorStr += aDemandedName;
            aErrorStr += "\", found \"";
            aErrorStr += aNameStr;
            aErrorStr += "\"";
            TEST_ENSHURE( aNameStr == aDemandedName, aErrorStr.getStr() );

            // Checken, ob alle Methoden auch einzeln gefunden werden
            aErrorStr  = "method \"";
            aErrorStr += aDemandedName;
            aErrorStr += "\" not found with hasMethod()";
            OUString aWDemandedName = OStringToOUString(aDemandedName, RTL_TEXTENCODING_ASCII_US );
            sal_Bool bMethod = xAccess->hasMethod( aWDemandedName, nRealConcepts );
            TEST_ENSHURE( bMethod, aErrorStr.getStr() );

            aErrorStr  = "method \"";
            aErrorStr += aDemandedName;
            aErrorStr += "\" not equal to same method in sequence returned by getMethods()";
            try
            {
                Reference< XIdlMethod > xGetMethod = xAccess->getMethod( aWDemandedName, nRealConcepts );
                TEST_ENSHURE( xGetMethod == rxMethod , aErrorStr.getStr() );
            }
            catch (RuntimeException e1)
            {
                aErrorStr  = "method \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\", exception was thrown when trying getMethod()";
                TEST_ENSHURE( sal_False, aErrorStr.getStr() );
            }
        }
    }

    // Listener-Klassen ausgeben
    Sequence< Type > aClassSeq = xAccess->getSupportedListeners();
    sal_Int32 nLen = aClassSeq.getLength();

    const Type* pListeners = aClassSeq.getConstArray();
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        // Methode ansprechen
        const Type& aListenerType = pListeners[i];

        // Namen besorgen
        OUString aListenerClassName = aListenerType.getTypeName();
    }

    return sal_True;
}


SAL_IMPLEMENT_MAIN()
{
    Reference< XMultiServiceFactory > xMgr( createRegistryServiceFactory( OUString(RTL_CONSTASCII_USTRINGPARAM("stoctest.rdb")) ) );

    sal_Bool bSucc = sal_False;
    try
    {
        Reference< XImplementationRegistration > xImplReg(
            xMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration")) ), UNO_QUERY );
        OSL_ENSURE( xImplReg.is(), "### no impl reg!" );

        // Register services
        OUString libName( RTL_CONSTASCII_USTRINGPARAM(
                              "reflection.uno" SAL_DLLEXTENSION) );
        fprintf(stderr, "1\n" );
        xImplReg->registerImplementation(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")),
                                         libName, Reference< XSimpleRegistry >() );
        fprintf(stderr, "2\n" );
        Reference< XIdlReflection > xRefl( xMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.CoreReflection")) ), UNO_QUERY );
        OSL_ENSURE( xRefl.is(), "### no corereflection!" );

        // Introspection
        libName = OUString(RTL_CONSTASCII_USTRINGPARAM(
            "introspection.uno" SAL_DLLEXTENSION));
        fprintf(stderr, "3\n" );
        xImplReg->registerImplementation(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")),
                                         libName, Reference< XSimpleRegistry >() );
        fprintf(stderr, "4\n" );
        Reference< XIntrospection > xIntrosp( xMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.Introspection")) ), UNO_QUERY );
        OSL_ENSURE( xRefl.is(), "### no corereflection!" );

        fprintf(stderr, "before test_introsp\n" );
        bSucc = test_introsp( xMgr, xRefl, xIntrosp );
        fprintf(stderr, "after test_introsp\n" );
    }
    catch (Exception & rExc)
    {
        OSL_ENSURE( sal_False, "### exception occurred!" );
        OString aMsg( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### exception occurred: " );
        OSL_TRACE( aMsg.getStr() );
        OSL_TRACE( "\n" );
    }

    Reference< XComponent >( xMgr, UNO_QUERY )->dispose();

    printf( "testintrosp %s !\n", (bSucc ? "succeeded" : "failed") );
    return (bSucc ? 0 : -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
