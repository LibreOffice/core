/*************************************************************************
 *
 *  $RCSfile: testintrosp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2000-11-08 09:36:19 $
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

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/servicefactory.hxx>

#include <vos/diagnose.hxx>
#include <vos/dynload.hxx>

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
//#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <stdio.h>
#include <string.h>


using namespace rtl;
using namespace cppu;
using namespace vos;
using namespace ModuleA;
//using namespace ModuleB;
//using namespace ModuleC;
//using namespace ModuleA::ModuleB;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::reflection;
using namespace com::sun::star::container;
using namespace com::sun::star::beans::PropertyAttribute;


typedef WeakImplHelper4< XIntroTest, XPropertySet, XNameAccess, XIndexAccess > ImplIntroTestHelper;
typedef WeakImplHelper1< XPropertySetInfo > ImplPropertySetInfoHelper;


#define DEFAULT_INDEX_ACCESS_COUNT  10
#define DEFAULT_NAME_ACCESS_COUNT   5

#ifdef _DEBUG
#define TEST_ENSHURE(c, m)   VOS_ENSHURE(c, m)
#else
#define TEST_ENSHURE(c, m)   VOS_VERIFY(c)
#endif

//class IntroTestWritelnOutput;



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
                OUString::createFromAscii("com.sun.star.reflection.CoreReflection") ), UNO_QUERY );
            VOS_ENSHURE( xRefl.is(), "### no corereflection!" );
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
        case TypeClass_TYPE:            aRetStr = OUString::createFromAscii("TYPE TYPE");               break;
        case TypeClass_INTERFACE:       aRetStr = OUString::createFromAscii("TYPE INTERFACE");      break;
        case TypeClass_SERVICE:         aRetStr = OUString::createFromAscii("TYPE SERVICE");            break;
        case TypeClass_STRUCT:          aRetStr = OUString::createFromAscii("TYPE STRUCT");         break;
        case TypeClass_TYPEDEF:         aRetStr = OUString::createFromAscii("TYPE TYPEDEF");            break;
        case TypeClass_UNION:           aRetStr = OUString::createFromAscii("TYPE UNION");          break;
        case TypeClass_ENUM:            aRetStr = OUString::createFromAscii("TYPE ENUM");               break;
        case TypeClass_EXCEPTION:       aRetStr = OUString::createFromAscii("TYPE EXCEPTION");      break;
        case TypeClass_ARRAY:           aRetStr = OUString::createFromAscii("TYPE ARRAY");          break;
        case TypeClass_SEQUENCE:        aRetStr = OUString::createFromAscii("TYPE SEQUENCE");           break;
        case TypeClass_VOID:            aRetStr = OUString::createFromAscii("TYPE void");               break;
        case TypeClass_ANY:             aRetStr = OUString::createFromAscii("TYPE any");                break;
        case TypeClass_UNKNOWN:         aRetStr = OUString::createFromAscii("TYPE unknown");            break;
        case TypeClass_BOOLEAN:
        {
            sal_Bool b = *(sal_Bool*)aValue.getValue();
            //aRet.setValue( &b, getCppuBooleanType() );
            //aValue >>= b;
            aRetStr = OUString::valueOf( b );
            break;
        }
        case TypeClass_CHAR:
        {
            sal_Unicode c = *(sal_Unicode*)aValue.getValue();
            //aValue >>= c;
            //getCppuCharType()
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
            float f;
            aValue >>= f;
            sprintf( pBuffer, "%f", f );
            aRetStr = OUString( pBuffer, strlen( pBuffer ), RTL_TEXTENCODING_ASCII_US );
            break;
        }
        case TypeClass_DOUBLE:
        {
            double d;
            aValue >>= d;
            sprintf( pBuffer, "%f", d );
            aRetStr = OUString( pBuffer, strlen( pBuffer ), RTL_TEXTENCODING_ASCII_US );
            break;
        }
        case TypeClass_BYTE:
        {
            sal_Int8 n;
            aValue >>= n;
            aRetStr = OUString::valueOf( (sal_Int32) n );
            break;
        }
        case TypeClass_SHORT:
        {
            sal_Int16 n;
            aValue >>= n;
            aRetStr = OUString::valueOf( (sal_Int32) n );
            break;
        }
        case TypeClass_LONG:
        {
            sal_Int32 n;
            aValue >>= n;
            aRetStr = OUString::valueOf( n );
            break;
        }
        /*
        case TypeClass_HYPER:
        {
            aRetStr = L"TYPE HYPER";
            break;
        }
        case TypeClass_UNSIGNED_SHORT:
        {
            aRetStr = StringToUString(WSString(aValue.getUINT16()), CHARSET_SYSTEM);
            break;
        }
        case TypeClass_UNSIGNED_LONG:
        {
            aRetStr = StringToUString(WSString(aValue.getUINT32()), CHARSET_SYSTEM);
            break;
        }
        case TypeClass_UNSIGNED_HYPER:
        {
            aRetStr = L"TYPE UNSIGNED_HYPER";
            break;
        }
        */
    }

    if( bIncludeType )
    {
        Reference< XIdlClass > xIdlClass = TypeToIdlClass( aValType, xMgr );
        aRetStr = aRetStr + OUString( OUString::createFromAscii(" (Typ: ") ) + xIdlClass->getName() + OUString::createFromAscii(")");
    }
    return aRetStr;
}

/*
// Hilfs-Funktion, um ein UString in einen Any zu konvertieren
UsrAny StringToAny( UString aStr, TypeClass eTargetType )
{
    UsrAny aRetAny;
    switch( eTargetType )
    {
        case TypeClass_INTERFACE:       break;
        case TypeClass_SERVICE:         break;
        case TypeClass_STRUCT:          break;
        case TypeClass_TYPEDEF:         break;
        case TypeClass_UNION:           break;
        case TypeClass_ENUM:            break;
        case TypeClass_EXCEPTION:       break;
        case TypeClass_ARRAY:           break;
        case TypeClass_SEQUENCE:        break;
        case TypeClass_VOID:            break;
        case TypeClass_ANY:             break;
        case TypeClass_UNKNOWN:         break;
        case TypeClass_BOOLEAN:         aRetAny.setBOOL( short(aStr)!=0 );  break;
        case TypeClass_CHAR:            aRetAny.setChar( char(aStr) );      break;
        case TypeClass_STRING:          aRetAny.setString( aStr );          break;
        case TypeClass_FLOAT:           aRetAny.setFloat( (float)strtod( aStr.GetStr(), NULL ) );   break;
        case TypeClass_DOUBLE:          aRetAny.setDouble( strtod( aStr.GetStr(), NULL ) ); break;
        case TypeClass_BYTE:            aRetAny.setBYTE( BYTE(short(aStr)) );   break;
        case TypeClass_SHORT:           aRetAny.setINT16( short(aStr) );    break;
        case TypeClass_LONG:            aRetAny.setINT32( long(aStr) );     break;
        case TypeClass_HYPER:           break;
        case TypeClass_UNSIGNED_SHORT:  aRetAny.setUINT16( USHORT(aStr) );  break;
        case TypeClass_UNSIGNED_LONG:   aRetAny.setUINT32( ULONG(aStr) );   break;
        case TypeClass_UNSIGNED_HYPER:  break;
    }
    return aRetAny;
}
*/


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
        //: mxMgr( xMgr ), ImplPropertySetInfoHelper( xMgr ) {}

/*
    // Methoden von XInterface
    virtual sal_Bool    SAL_CALL queryInterface( const Uik & rUik, Any & ifc ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    virtual void        SAL_CALL release() throw() { OWeakObject::release(); }
    //ALT: sal_Bool queryInterface( Uik aUik, Reference<XInterface> & rOut );
*/

    // Methods of XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(  )
        throw(RuntimeException);
    virtual Property SAL_CALL getPropertyByName( const OUString& aName )
        throw(UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )
        throw(RuntimeException);
    //virtual Sequence< Property > SAL_CALL getProperties(void) throw( RuntimeException );
    //virtual Property SAL_CALL getPropertyByName(const OUString& Name) throw( RuntimeException );
    //virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& Name) throw( RuntimeException );
};


/*
// Methoden von XInterface
sal_Bool SAL_CALL ImplPropertySetInfo::queryInterface( const Uik & rUik, Any & ifc )
    throw( RuntimeException )
{
    // PropertySet-Implementation
    if( com::sun::star::uno::queryInterface( rUik, ifc,
                                             SAL_STATIC_CAST(XPropertySetInfo*, this) ) )
        return sal_True;

    return OWeakObject::queryInterface( rUik, ifc );
}

sal_Bool ImplPropertySetInfo::queryInterface( Uik aUik, Reference<XInterface> & rOut )
{
    if( aUik == XPropertySetInfo::getSmartUik() )
        rOut = (XPropertySetInfo *)this;
    else
        UsrObject::queryInterface( aUik, rOut );
    return rOut.is();
}
*/

Sequence< Property > ImplPropertySetInfo::getProperties(void)
    throw( RuntimeException )
{
    static Sequence<Property> * pSeq = NULL;

    if( !pSeq )
    {
        // die Informationen für die Properties "Width", "Height" und "Name" anlegen
        pSeq = new Sequence<Property>( 3 );
        Property * pAry = pSeq->getArray();

        pAry[0].Name = OUString::createFromAscii("Factor");
        pAry[0].Handle = -1;
        pAry[0].Type = getCppuType( (double*) NULL );
        //pAry[0].Type = TypeToIdlClass( getCppuType( (double*) NULL ), mxMgr );
        //pAry[0].Type = Double_getReflection()->getIdlClass();
        pAry[0].Attributes = BOUND | TRANSIENT;

        pAry[1].Name = OUString::createFromAscii("MyCount");
        pAry[1].Handle = -1;
        pAry[1].Type = getCppuType( (sal_Int32*) NULL );
        //pAry[1].Type = TypeToIdlClass( getCppuType( (sal_Int32*) NULL ), mxMgr );
        //pAry[1].Type = INT32_getReflection()->getIdlClass();
        pAry[1].Attributes = BOUND | TRANSIENT;

        pAry[2].Name = OUString::createFromAscii("Info");
        pAry[2].Handle = -1;
        pAry[2].Type = getCppuType( (OUString*) NULL );
        //pAry[2].Type = TypeToIdlClass( getCppuType( (OUString*) NULL ), mxMgr );
        //pAry[2].Type = OUString_getReflection()->getIdlClass();
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

    // Optionale Schnittstelle fuer die writeln-Ausgabe
    //IntroTestWritelnOutput* m_pOutput;

    Reference< XPropertySetInfo > m_xMyInfo;

    OUString m_ObjectName;

    int m_nMarkusAge;
    int m_nMarkusChildrenCount;

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
        //: mxMgr( xMgr ), ImplIntroTestHelper( xMgr )
    {
        Init();
    }

    /*
    ImplIntroTest( IntroTestWritelnOutput* pOutput_ )
    {
        Init();
        m_pOutput = pOutput_;
    }
    */

    //SMART_UNO_DECLARATION(ImplIntroTest,UsrObject);

    //BOOL queryInterface( Uik aUik, Reference< XInterface > & rOut );
    //Reference< XIdlClass > getIdlClass();

    // Trotz virtual inline, um Schreibarbeit zu sparen (nur fuer Testzwecke)
    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
        throw(RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
            {}
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
            {}
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
            {}
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
            {}

    /*
    virtual void setIndexedPropertyValue(const OUString& aPropertyName, INT32 nIndex, const Any& aValue) {}
    virtual Any getIndexedPropertyValue(const UString& aPropertyName, INT32 nIndex) const { return Any(); }
    virtual void addPropertyChangeListener(const UString& aPropertyName, const XPropertyChangeListenerRef& aListener)
        THROWS( (UnknownPropertyException, WrappedTargetException, UsrSystemException) ) {}
    virtual void removePropertyChangeListener(const UString& aPropertyName, const XPropertyChangeListenerRef& aListener)
        THROWS( (UnknownPropertyException, WrappedTargetException, UsrSystemException) ) {}
    virtual void addVetoableChangeListener(const UString& aPropertyName, const XVetoableChangeListenerRef& aListener)
        THROWS( (UnknownPropertyException, WrappedTargetException, UsrSystemException) ) {}
    virtual void removeVetoableChangeListener(const UString& aPropertyName, const XVetoableChangeListenerRef& aListener)
        THROWS( (UnknownPropertyException, WrappedTargetException, UsrSystemException) ) {}
        */

    // XIntroTest-Methoden
    // Attributes
    virtual OUString SAL_CALL getObjectName() throw(RuntimeException)
        { return m_ObjectName; }
    virtual void SAL_CALL setObjectName( const OUString& _objectname ) throw(RuntimeException)
        { m_ObjectName = _objectname; }
    virtual OUString SAL_CALL getFirstName()
        throw(RuntimeException);
    virtual OUString SAL_CALL getLastName() throw(RuntimeException)
        { return OUString( OUString::createFromAscii("Meyer") ); }
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
    virtual TypeClass SAL_CALL getTypeClass( sal_Int16 n ) throw(RuntimeException)
        { return eTypeClass; }
    virtual void SAL_CALL setTypeClass( TypeClass t, double d1, double d2 ) throw(RuntimeException)
        { eTypeClass = t; }
    virtual Sequence< OUString > SAL_CALL getStrings(  ) throw(RuntimeException)
        { return aStringSeq; }
    virtual void SAL_CALL setStrings( const Sequence< OUString >& Strings ) throw(RuntimeException)
        { aStringSeq = Strings; }
    virtual void SAL_CALL setStringsPerMethod( const Sequence< OUString >& Strings, sal_Int16 n ) throw(RuntimeException)
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
    //virtual XIdlClassRef getElementType(void) constTHROWS( (UsrSystemException) );
    //virtual BOOL hasElements(void) const THROWS( (UsrSystemException) );

    // XNameAccess-Methoden
    // Methods
    virtual Any SAL_CALL getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames(  )
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(RuntimeException);
    //virtual Any getByName(const UString& Name) const
        //THROWS( (NoSuchElementException, WrappedTargetException, UsrSystemException) );
    //virtual Sequence<UString> getElementNames(void) const THROWS( (UsrSystemException) );
    //virtual BOOL hasByName(const UString& Name) const THROWS( (UsrSystemException) );

    // XIndexAccess-Methoden
    // Methods
    virtual sal_Int32 SAL_CALL getCount(  )
        throw(RuntimeException);
    virtual Any SAL_CALL getByIndex( sal_Int32 Index )
        throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException);
    //virtual INT32 getCount(void) const THROWS( (UsrSystemException) );
    //virtual Any getByIndex(INT32 Index) const
        //THROWS( (IndexOutOfBoundsException, WrappedTargetException, UsrSystemException) );
};

//SMART_UNO_IMPLEMENTATION(ImplIntroTest,UsrObject)

void ImplIntroTest::Init( void )
{
    // Eindeutigen Namen verpassen
    static sal_Int32 nObjCount = 0;
    OUString aName( OUString::createFromAscii("IntroTest-Obj Nr. ") );
    aName += OUString::valueOf( nObjCount );
    setObjectName( aName );

    // Properties initialisieren
    aAnyArray[0] <<= 3.14;
    aAnyArray[1] <<= (sal_Int32)42;
    aAnyArray[2] <<= OUString( OUString::createFromAscii("Hallo") );

    // Output-Interface
    //m_pOutput = NULL;

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
    pStr[ 0 ] = OUString( OUString::createFromAscii("String 0") );
    pStr[ 1 ] = OUString( OUString::createFromAscii("String 1") );
    pStr[ 2 ] = OUString( OUString::createFromAscii("String 2") );

    // structs initialisieren
    m_aFirstStruct.Name = OUString::createFromAscii("FirstStruct-Name");
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

/*
BOOL ImplIntroTest::queryInterface( Uik aUik, XInterfaceRef & rOut )
{
    if( aUik == XIntroTest::getSmartUik() )
        rOut = (XIntroTest*)this;
    else if( aUik == XPropertySet::getSmartUik() )
        rOut = (XPropertySet*)this;
    else if( aUik == XNameAccess::getSmartUik() )
        rOut = (XNameAccess*)this;
    else if( aUik == XIndexAccess::getSmartUik() )
        rOut = (XIndexAccess*)this;
    else if( aUik == ((XElementAccess*)NULL)->getSmartUik() )
        rOut = (XElementAccess*)(XIndexAccess *)this;
    else
        UsrObject::queryInterface( aUik, rOut );
    return rOut.is();
}

XIdlClassRef ImplIntroTest::getIdlClass()
{
    static XIdlClassRef xClass = createStandardClass( L"ImplIntroTest",
        UsrObject::getUsrObjectIdlClass(), 4,
            XIntroTest_getReflection(),
            XPropertySet_getReflection(),
            XNameAccess_getReflection(),
            XIndexAccess_getReflection() );
    return xClass;
}
*/

Reference< XPropertySetInfo > ImplIntroTest::getPropertySetInfo()
    throw(RuntimeException)
{
    static ImplPropertySetInfo aInfo( mxMgr );
    // Alle Objekt haben die gleichen Properties, deshalb kann
    // die Info für alle gleich sein
    return &aInfo;

    //if( m_xMyInfo == NULL )
    //  ((ImplIntroTest*)this)->m_xMyInfo = new ImplPropertySetInfo( this );
    //return m_xMyInfo;
}

void ImplIntroTest::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
//void ImplIntroTest::setPropertyValue( const UString& aPropertyName, const Any& aValue )
//  THROWS( (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, UsrSystemException) )
{
    if( aPropChangeListener.is() && aPropertyName == aPropChangeListenerStr )
    {
        PropertyChangeEvent aEvt;
        aEvt.Source = (OWeakObject*)this;
        aEvt.PropertyName = aPropertyName;
        aEvt.PropertyHandle = 0L;
        //aEvt.OldValue;
        //aEvt.NewValue;
        //aEvt.PropagationId;
        aPropChangeListener->propertyChange( aEvt );
    }
    if( aVetoPropChangeListener.is() && aPropertyName == aVetoPropChangeListenerStr )
    {
        PropertyChangeEvent aEvt;
        aEvt.Source = (OWeakObject*)this;
        aEvt.PropertyName = aVetoPropChangeListenerStr;
        aEvt.PropertyHandle = 0L;
        //aEvt.OldValue;
        //aEvt.NewValue;
        //aEvt.PropagationId;
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
//Any ImplIntroTest::getPropertyValue(const UString& aPropertyName) const
    //THROWS( (UnknownPropertyException, WrappedTargetException, UsrSystemException) )
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
    return OUString( OUString::createFromAscii("Markus") );
}

void ImplIntroTest::writeln( const OUString& Text )
    throw(RuntimeException)
{
    OString aStr( Text.getStr(), Text.getLength(), RTL_TEXTENCODING_ASCII_US );

    // Haben wir ein Output?
    //if( m_pOutput )
    //{
        //m_pOutput->doWriteln( TextStr );
    //}
    // Sonst einfach rausbraten
    //else
    {
        printf( "%s", aStr.getStr() );
    }
}

Reference< XIntroTest > ImplIntroTest::getIntroTest()
    throw(RuntimeException)
//XIntroTestRef ImplIntroTest::getIntroTest(void) THROWS( (UsrSystemException) )
{
    if( !m_xIntroTest.is() )
        m_xIntroTest = new ImplIntroTest( mxMgr );
    return m_xIntroTest;
}

// Methoden von XElementAccess
Type ImplIntroTest::getElementType(  )
    throw(RuntimeException)
//XIdlClassRef ImplIntroTest::getElementType(void) const THROWS( (UsrSystemException) )
{
    // TODO
    Type aRetType;
    return aRetType;
    //return Reference< XIdlClass >();
    //return Void_getReflection()->getIdlClass();
}

sal_Bool ImplIntroTest::hasElements(  )
    throw(RuntimeException)
//BOOL ImplIntroTest::hasElements(void) const THROWS( (UsrSystemException) )
{
    return sal_True;
}

// XNameAccess-Methoden
sal_Int32 getIndexForName( const OUString& ItemName )
{
    OUString aLeftStr = ItemName.copy( 0, 4 );
    if( aLeftStr == OUString::createFromAscii("Item") )
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
//Any ImplIntroTest::getByName(const UString& Name) const
    //THROWS( (NoSuchElementException, WrappedTargetException, UsrSystemException) )
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
            OUString aName( OUString::createFromAscii("IntroTest by Name-Access, Index = ") );
            aName += OUString::valueOf( iIndex );
            //aName = aName + StringToUString( String( iIndex ), CHARSET_SYSTEM );
            p->setObjectName( aName );
            pNameAccessTab[iIndex] = p;
        }

        Reference< XIntroTest > xRet = pNameAccessTab[iIndex];
        aRetAny = makeAny( xRet );

        //aRetAny.set( &xRet, XIntroTest_getReflection() );
        //return (UsrObject*)(XIntroTest*)pNameAccessTab[iIndex];
    }
    return aRetAny;
}

Sequence< OUString > ImplIntroTest::getElementNames(  )
    throw(RuntimeException)
//Sequence<UString> ImplIntroTest::getElementNames(void) const THROWS( (UsrSystemException) )
{
    Sequence<OUString> aStrSeq( DEFAULT_NAME_ACCESS_COUNT );
    OUString* pStr = aStrSeq.getArray();
    for( sal_Int32 i = 0 ; i < DEFAULT_NAME_ACCESS_COUNT ; i++ )
    {
        OUString aName( OUString::createFromAscii("Item") );
        aName += OUString::valueOf( i );
        //aName = aName + StringToUString( i, CHARSET_SYSTEM );
        pStr[i] = aName;
    }
    return aStrSeq;
}

sal_Bool ImplIntroTest::hasByName( const OUString& aName )
    throw(RuntimeException)
//BOOL ImplIntroTest::hasByName(const UString& Name) const THROWS( (UsrSystemException) )
{
    return ( getIndexForName( aName ) != -1 );
}

// XIndexAccess-Methoden
sal_Int32 ImplIntroTest::getCount(  )
    throw(RuntimeException)
//sal_Int32 ImplIntroTest::getCount(void) const THROWS( (UsrSystemException) )
{
    return iIndexAccessCount;
}

Any ImplIntroTest::getByIndex( sal_Int32 Index )
    throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
//Any ImplIntroTest::getByIndex( sal_Int32 Index ) const
    //THROWS( (IndexOutOfBoundsException, WrappedTargetException, UsrSystemException) )
{
    Any aRetAny;

    if( !pIndexAccessTab )
        ((ImplIntroTest*)this)->pIndexAccessTab = new Reference< XIntroTest >[ iIndexAccessCount ];

    if( Index < iIndexAccessCount )
    {
        if( !pNameAccessTab[Index].is() )
        {
            ImplIntroTest* p = new ImplIntroTest( mxMgr );
            OUString aName( OUString::createFromAscii("IntroTest by Index-Access, Index = ") );
            aName += OUString::valueOf( Index );
            //aName = aName + StringToUString( String( iIndex ), CHARSET_SYSTEM );
            p->setObjectName( aName );
            pIndexAccessTab[Index] = p;
        }
        Reference< XIntroTest > xRet = pIndexAccessTab[Index];
        aRetAny = makeAny( xRet );
    }
    return aRetAny;
}

void ImplIntroTest::addPropertiesChangeListener( const Sequence< OUString >& PropertyNames,
    const Reference< XPropertiesChangeListener >& Listener )
        throw(RuntimeException)
//void ImplIntroTest::addPropertiesChangeListener
//(const Sequence< UString >& PropertyNames, const XPropertiesChangeListenerRef& Listener)
    //THROWS( (UsrSystemException) )
{
}

void ImplIntroTest::removePropertiesChangeListener
    ( const Reference< XPropertiesChangeListener >& Listener )
        throw(RuntimeException)
//void ImplIntroTest::removePropertiesChangeListener(const XPropertiesChangeListenerRef& Listener)
    //THROWS( (UsrSystemException) )
{
}



struct DefItem
{
    char* pName;
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
    Reference< XIdlReflection > xRefl, Reference< XIntrospection > xIntrospection )
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

    // Tabelle der Property-Namen, die gefunden werden muessen
    char* pDemandedPropNames[] =
    {
        "Factor",
        "MyCount",
        "Info",
        "ObjectName",
        "FirstName",
        "LastName",
        "Age",
        "ChildrenCount",
        "FirstStruct",
        "SecondStruct",
        "Droenk",
        "IntroTest",
        "Bla",
        "Blub",
        "Gulp",
        "Strings",
        "MultiSequence",
        "PropertySetInfo",
        "ElementType",
        "ElementNames",
        "Count",
        "Types"
        "ImplementationId"
    };

    char* pDemandedPropVals[] =
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

    char* pDemandedModifiedPropVals[] =
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

    char* pDemandedPropTypes[] =
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
    //is() nDemandedPropCount = 22;


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

    // Introspection-Service holen
    //Reference< XMultiServiceFactory > xServiceManager(getProcessServiceManager(), USR_QUERY);
    //Reference< XIntrospection > xIntrospection( xMgr->createInstance(L"com.sun.star.beans.Introspection"), UNO_QUERY );
    //TEST_ENSHURE( xIntrospection.is(), "Creation of introspection instance failed" );
    //if( !xIntrospection.is() )
        //return sal_False;

    // und unspecten
    Reference< XIntrospectionAccess > xAccess = xIntrospection->inspect( aObjAny );
    xAccess = xIntrospection->inspect( aObjAny );
    xAccess = xIntrospection->inspect( aObjAny );
    TEST_ENSHURE( xAccess.is(), "introspection failed, no XIntrospectionAccess returned" );
    if( !xAccess.is() )
        return sal_False;

    // Ergebnis der Introspection pruefen

    // XPropertySet-UIK ermitteln
    Type aType = getCppuType( (Reference< XPropertySet >*) NULL );
    //typelib_InterfaceTypeDescription* pTypeDesc = NULL;
    //aType.getDescription( (typelib_TypeDescription**)&pTypeDesc );
    //Uik aPropertySetUik = *(Uik*)&pTypeDesc->aUik;
    //typelib_typedescription_release( (typelib_TypeDescription*)pTypeDesc );

    Reference< XInterface > xPropSetIface = xAccess->queryAdapter( aType );
    //Reference< XInterface > xPropSetIface = xAccess->queryAdapter( aPropertySetUik );
    Reference< XPropertySet > xPropSet( xPropSetIface, UNO_QUERY );
    //XPropertySetRef xPropSet = (XPropertySet*)xPropSetIface->
    //  queryInterface( XPropertySet::getSmartUik() );
    TEST_ENSHURE( xPropSet.is(), "Could not get XPropertySet by queryAdapter()" );

    // XExactName
    Reference< XExactName > xExactName( xAccess, UNO_QUERY );
    TEST_ENSHURE( xExactName.is(), "Introspection unterstuetzt kein ExactName" );

    // Schleife ueber alle Kombinationen von Concepts
    for( sal_Int32 nConcepts = 0 ; nConcepts < 16 ; nConcepts++ )
    {
//printf( "*******************************************************\n" );
//printf( "nConcepts = %ld\n", nConcepts );

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
            //Sequence<Property> aRetSeq = xPropSetInfo->getProperties();
            Sequence<Property> aRetSeq = xAccess->getProperties( nConcepts );

            sal_Int32 nLen = aRetSeq.getLength();

            aErrorStr  = "Expected to find ";
            aErrorStr += OString::valueOf( nDemandedPropCount );
            aErrorStr += " properties but found ";
            aErrorStr += OString::valueOf( nLen );
            TEST_ENSHURE( nLen == nDemandedPropCount, aErrorStr.getStr() );

            // cout << "**********************************\n";
            // cout << "*** Ergebnis der Introspection ***\n";
            // cout << "**********************************\n";
            // cout << "\nIntrospection hat " << nLen << " Properties gefunden:\n";

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
                    //UStringToString(aPropName, CHARSET_SYSTEM);

//printf( "Property = %s\n", aNameStr.getStr() );

                OString aDemandedName = pPropertyDefs[ iDemanded ].pName;
                //OString aDemandedName = pDemandedPropNames[ i ];
                aErrorStr  = "Expected property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\", found \"";
                aErrorStr += aNameStr;
                aErrorStr += "\"";
                TEST_ENSHURE( aNameStr == aDemandedName, aErrorStr.getStr() );
                // cout << "Property " << (i+1) << ": \"" << (const char*)UStringToString(aPropName, CHARSET_SYSTEM) << "\"";


                Type aPropType = aProp.Type;
                OString aTypeNameStr( OUStringToOString(aPropType.getTypeName(), RTL_TEXTENCODING_ASCII_US) );
                //Reference< XIdlClass > xPropType = aProp.Type;
                //OString aTypeNameStr( xPropType->getName(), xPropType->getName().getLength(), RTL_TEXTENCODING_ASCII_US );
                OString aDemandedTypeNameStr = pDemandedPropTypes[ iDemanded ];
                //OString aDemandedTypeNameStr = pDemandedPropTypes[ i ];
                aErrorStr  = "Property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\", expected type >";
                aErrorStr += aDemandedTypeNameStr;
                aErrorStr += "< found type >";
                aErrorStr += aTypeNameStr;
                aErrorStr += "<";
                TEST_ENSHURE( aTypeNameStr == aDemandedTypeNameStr, aErrorStr.getStr() );
                // cout << " (Prop-Typ: " << (const char*)UStringToString(xPropType->getName(), CHARSET_SYSTEM) << ")";

                // Wert des Property lesen und ausgeben
                aPropVal = xPropSet->getPropertyValue( aPropName );
                // cout << "\n\tWert = " << (const char*)UStringToString(AnyToString( aPropVal, sal_True ), CHARSET_SYSTEM);

                OString aValStr = OUStringToOString( AnyToString( aPropVal, sal_False, xMgr ), RTL_TEXTENCODING_ASCII_US );
                OString aDemandedValStr = pDemandedPropVals[ iDemanded ];
                //OString aDemandedValStr = pDemandedPropVals[ i ];
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
                //Reference< XIdlClass > xIdlClass = aPropVal.getReflection()->getIdlClass();
                //TypeClass eType = xIdlClass->getTypeClass();
                Any aNewVal;
                sal_Bool bModify = sal_True;
                switch( eType )
                {
                    case TypeClass_STRING:
                    {
                        OUString aStr;
                        aPropVal >>= aStr;
                        //OString aStr = aPropVal.getString();
                        aStr = aStr + OUString::createFromAscii(" (Modified!)");
                        aNewVal <<= aStr;
                        break;
                    }
                    case TypeClass_DOUBLE:
                    {
                        double d;
                        aPropVal >>= d;
                        aNewVal <<= d + 1.0;
                        break;
                    }
                    case TypeClass_SHORT:
                    {
                        sal_Int16 n;
                        aPropVal >>= n;
                        aNewVal <<= sal_Int16( n + 1 );
                        break;
                    }
                    case TypeClass_LONG:
                    {
                        sal_Int32 n;
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
                    OUString aUpperUStr = aPropName.toUpperCase();
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
                    // cout.flush();

                    // 1.7.1999, UnknownPropertyException bei ReadOnly-Properties abfangen
                    try
                    {
                        xPropSet->setPropertyValue( aPropName, aNewVal );
                    }
                    catch(UnknownPropertyException e1)
                    {
                    }

                    aPropVal = xPropSet->getPropertyValue( aPropName );
                    // cout << "\n\tModifizierter Wert = " << (const char*) UStringToString(AnyToString( aPropVal, sal_True ), CHARSET_SYSTEM) << "\n";

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
                else
                {
                    // cout << "\n\tWert wurde nicht modifiziert\n";
                }

                // Checken, ob alle Properties auch einzeln gefunden werden
                aErrorStr  = "property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\" not found with hasProperty()";
                OUString aWDemandedName = OStringToOUString(aDemandedName, RTL_TEXTENCODING_ASCII_US );
                sal_Bool bProperty = xAccess->hasProperty( aWDemandedName, nConcepts );
                //sal_Bool bProperty = xAccess->hasProperty( aWDemandedName, PropertyConcept::ALL - PropertyConcept::DANGEROUS );
                TEST_ENSHURE( bProperty, aErrorStr.getStr() );

                aErrorStr  = "property \"";
                aErrorStr += aDemandedName;
                aErrorStr += "\" not equal to same Property in sequence returned by getProperties()";
                try
                {
                    Property aGetProp = xAccess->getProperty( aWDemandedName, nConcepts );
                    //Property aGetProp = xAccess->getProperty( aWDemandedName, PropertyConcept::ALL );
                    //TEST_ENSHURE( aGetProp == aProp , aErrorStr.getStr() );
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
    for( nConcepts = 0 ; nConcepts < 128 ; nConcepts++ )
    {
//printf( "*******************************************************\n" );
//printf( "nConcepts = %ld\n", nConcepts );

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
        //aMethodSeq = xAccess->getMethods
        Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods( nRealConcepts );
        //Sequence<XIdlMethodRef> aMethodSeq = xAccess->getMethods
        //  ( MethodConcept::ALL - MethodConcept::DANGEROUS - MethodConcept::PROPERTY );
        sal_Int32 nLen = aMethodSeq.getLength();
        // cout << "\n\n*** Methoden ***\n";
        // cout << "Introspection hat " << nLen << " Methoden gefunden:\n";

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

//printf( "Method = %s\n", aNameStr.getStr() );

            // Naechste Passende Methode in der Liste suchen
            while( pMethodDefs[ iList ].pName )
            {
                if( pMethodDefs[ iList ].nConcept & nRealConcepts )
                    break;
                iList++;
            }
            OString aDemandedName = pMethodDefs[ iList ].pName;
            iList++;

            //OString aDemandedName = pDemandedMethNames[ i ];
            aErrorStr  = "Expected method \"";
            aErrorStr += aDemandedName;
            aErrorStr += "\", found \"";
            aErrorStr += aNameStr;
            aErrorStr += "\"";
            TEST_ENSHURE( aNameStr == aDemandedName, aErrorStr.getStr() );
            // cout << "Methode " << (i+1) << ": " << (const char*) UStringToString(rxMethod->getReturnType()->getName(), CHARSET_SYSTEM)
            //   << " " << (const char*) UStringToString(rxMethod->getName(), CHARSET_SYSTEM) << "( ";

            // Checken, ob alle Methoden auch einzeln gefunden werden
            aErrorStr  = "method \"";
            aErrorStr += aDemandedName;
            aErrorStr += "\" not found with hasMethod()";
            OUString aWDemandedName = OStringToOUString(aDemandedName, RTL_TEXTENCODING_ASCII_US );
            sal_Bool bMethod = xAccess->hasMethod( aWDemandedName, nRealConcepts );
            //sal_Bool bMethod = xAccess->hasMethod( aWDemandedName, MethodConcept::ALL );
            TEST_ENSHURE( bMethod, aErrorStr.getStr() );

            aErrorStr  = "method \"";
            aErrorStr += aDemandedName;
            aErrorStr += "\" not equal to same method in sequence returned by getMethods()";
            try
            {
                Reference< XIdlMethod > xGetMethod = xAccess->getMethod( aWDemandedName, nRealConcepts );
                //XIdlMethodRef xGetMethod = xAccess->getMethod( aWDemandedName, MethodConcept::ALL );
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
    // cout << "\n\n*** Anmeldbare Listener ***\n";
    // cout << "Introspection hat " << nLen << " Listener gefunden:\n";

    const Type* pListeners = aClassSeq.getConstArray();
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
    {
        // Methode ansprechen
        const Type& aListenerType = pListeners[i];

        // Namen besorgen
        OUString aListenerClassName = aListenerType.getTypeName();
        // cout << "Listener " << (i+1) << ": " << (const char*)UStringToString(aListenerClassName, CHARSET_SYSTEM) << "\n";
    }


    // Performance bei hasMethod testen.
    //CheckMethodPerformance( xAccess, "queryInterface", 100000 );
    //CheckMethodPerformance( xAccess, "getIdlClasses", 100000 );

    // cout.flush();




    return sal_True;
}




#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
    Reference< XMultiServiceFactory > xMgr( createRegistryServiceFactory( OUString::createFromAscii("stoctest.rdb") ) );

    sal_Bool bSucc = sal_False;
    try
    {
        Reference< XImplementationRegistration > xImplReg(
            xMgr->createInstance( OUString::createFromAscii("com.sun.star.registry.ImplementationRegistration") ), UNO_QUERY );
        VOS_ENSHURE( xImplReg.is(), "### no impl reg!" );

        // Register services
        OUString libName;

        // CoreReflection
#ifdef SAL_W32
        libName = OUString::createFromAscii("corefl.dll");
#else
        libName = OUString::createFromAscii("libcorefl.so");
#endif
//          ORealDynamicLoader::computeLibraryName( OUString::createFromAscii("corefl"), libName);
        xImplReg->registerImplementation(OUString::createFromAscii("com.sun.star.loader.SharedLibrary"),
                                         libName, Reference< XSimpleRegistry >() );
        Reference< XIdlReflection > xRefl( xMgr->createInstance( OUString::createFromAscii("com.sun.star.reflection.CoreReflection") ), UNO_QUERY );
        VOS_ENSHURE( xRefl.is(), "### no corereflection!" );

        // Introspection
#ifdef SAL_W32
        libName = OUString::createFromAscii("insp.dll");
#else
        libName = OUString::createFromAscii("libinsp.so");
#endif
//          ORealDynamicLoader::computeLibraryName( OUString::createFromAscii("insp"), libName);
        xImplReg->registerImplementation(OUString::createFromAscii("com.sun.star.loader.SharedLibrary"),
                                         libName, Reference< XSimpleRegistry >() );
        Reference< XIntrospection > xIntrosp( xMgr->createInstance( OUString::createFromAscii("com.sun.star.beans.Introspection") ), UNO_QUERY );
        VOS_ENSHURE( xRefl.is(), "### no corereflection!" );

        bSucc = test_introsp( xMgr, xRefl, xIntrosp );
        //bSucc = test_corefl( xRefl );
    }
    catch (Exception & rExc)
    {
        VOS_ENSHURE( sal_False, "### exception occured!" );
        OString aMsg( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        VOS_TRACE( "### exception occured: " );
        VOS_TRACE( aMsg.getStr() );
        VOS_TRACE( "\n" );
    }

    Reference< XComponent >( xMgr, UNO_QUERY )->dispose();

    printf( "testintrosp %s !\n", (bSucc ? "succeeded" : "failed") );
    return (bSucc ? 0 : -1);
}







//*****************************
//*** TEST-Implementationen ***
//*****************************
// Bleibt auf Dauer nicht drin, dient als exportierbare Test-Klasse
// z.B. fuer Basic-Anbindung






