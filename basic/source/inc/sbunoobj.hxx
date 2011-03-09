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
#ifndef SB_UNO_OBJ
#define SB_UNO_OBJ

#include <basic/sbxobj.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/sbxprop.hxx>
#include <basic/sbxfac.hxx>
#ifndef __SBX_SBX_HXX //autogen
#include <basic/sbx.hxx>
#endif
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/reflection/XSingletonTypeDescription.hpp>
#include <rtl/ustring.hxx>

class SbUnoObject: public SbxObject
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > mxUnoAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMaterialHolder > mxMaterialHolder;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > mxInvocation;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XExactName > mxExactName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XExactName > mxExactNameInvocation;
    sal_Bool bNeedIntrospection;
    sal_Bool bNativeCOMObject;
    ::com::sun::star::uno::Any maTmpUnoObj; // Only to save obj for doIntrospection!

    // Hilfs-Methode zum Anlegen der dbg_-Properties
    void implCreateDbgProperties( void );

    // Hilfs-Methode zum Anlegen aller Properties und Methoden
    // (Beim on-demand-Mechanismus erforderlich fuer die dbg_-Properties)
    void implCreateAll( void );

public:
    static bool getDefaultPropName( SbUnoObject* pUnoObj, String& sDfltProp );
    TYPEINFO();
    SbUnoObject( const String& aName_, const ::com::sun::star::uno::Any& aUnoObj_ );
    ~SbUnoObject();

    // #76470 Introspection on Demand durchfuehren
    void doIntrospection( void );

    // Find ueberladen, um z.B. NameAccess zu unterstuetzen
    virtual SbxVariable* Find( const String&, SbxClassType );

    // Force creation of all properties for debugging
    void createAllProperties( void  )
        { implCreateAll(); }

    // Wert rausgeben
    ::com::sun::star::uno::Any getUnoAny( void );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > getIntrospectionAccess( void )    { return mxUnoAccess; }
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > getInvocation( void )         { return mxInvocation; }

    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );

    bool isNativeCOMObject( void )
        { return bNativeCOMObject; }
};
SV_DECL_IMPL_REF(SbUnoObject);


// #67781 Rueckgabewerte der Uno-Methoden loeschen
void clearUnoMethods( void );
void clearUnoMethodsForBasic( StarBASIC* pBasic );

class SbUnoMethod : public SbxMethod
{
    friend class SbUnoObject;
    friend void clearUnoMethods( void );
    friend void clearUnoMethodsForBasic( StarBASIC* pBasic );

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlMethod > m_xUnoMethod;
    ::com::sun::star::uno::Sequence< ::com::sun::star::reflection::ParamInfo >* pParamInfoSeq;

    // #67781 Verweis auf vorige und naechste Methode in der Methoden-Liste
    SbUnoMethod* pPrev;
    SbUnoMethod* pNext;

    bool mbInvocation;       // Method is based on invocation
    bool mbDirectInvocation; // Method should be used with XDirectInvocation interface

public:
    TYPEINFO();

    SbUnoMethod( const String& aName_, SbxDataType eSbxType, ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlMethod > xUnoMethod_,
        bool bInvocation,
        bool bDirect = false );
    virtual ~SbUnoMethod();
    virtual SbxInfo* GetInfo();

    const ::com::sun::star::uno::Sequence< ::com::sun::star::reflection::ParamInfo >& getParamInfos( void );

    bool isInvocationBased( void )
        { return mbInvocation; }
    bool needsDirectInvocation( void )
        { return mbDirectInvocation; }
};


class SbUnoProperty : public SbxProperty
{
    friend class SbUnoObject;

    // Daten der Uno-Property
    ::com::sun::star::beans::Property aUnoProp;
    sal_Int32 nId;

    bool mbInvocation;      // Property is based on invocation

    virtual ~SbUnoProperty();
public:
    TYPEINFO();
    SbUnoProperty( const String& aName_, SbxDataType eSbxType,
        const ::com::sun::star::beans::Property& aUnoProp_, sal_Int32 nId_, bool bInvocation );

    bool isInvocationBased( void )
        { return mbInvocation; }
};

// Factory-Klasse fuer das Anlegen von Uno-Structs per DIM AS NEW
class SbUnoFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

// Wrapper fuer eine Uno-Klasse
class SbUnoClass : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >   m_xClass;

public:
    TYPEINFO();
    SbUnoClass( const String& aName_ )
        : SbxObject( aName_ )
    {}
    SbUnoClass( const String& aName_, const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >& xClass_ )
        : SbxObject( aName_ )
        , m_xClass( xClass_ )
    {}
    //~SbUnoClass();

    // Find ueberladen, um Elemente on Demand anzulegen
    virtual SbxVariable* Find( const String&, SbxClassType );

    // Wert rausgeben
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >& getUnoClass( void ) { return m_xClass; }

    //void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoClass);


// Funktion, um einen globalen Bezeichner im
// UnoScope zu suchen und fuer Sbx zu wrappen
SbUnoClass* findUnoClass( const String& rName );


// Wrapper for UNO Service
class SbUnoService : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceTypeDescription2 >    m_xServiceTypeDesc;
    bool                                                                                                m_bNeedsInit;

public:
    TYPEINFO();
    SbUnoService( const String& aName_,
        const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceTypeDescription2 >& xServiceTypeDesc )
            : SbxObject( aName_ )
            , m_xServiceTypeDesc( xServiceTypeDesc )
            , m_bNeedsInit( true )
    {}

    virtual SbxVariable* Find( const String&, SbxClassType );

    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoService);

SbUnoService* findUnoService( const String& rName );


void clearUnoServiceCtors( void );

class SbUnoServiceCtor : public SbxMethod
{
    friend class SbUnoService;
    friend void clearUnoServiceCtors( void );

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > m_xServiceCtorDesc;

    SbUnoServiceCtor* pPrev;
    SbUnoServiceCtor* pNext;

public:
    TYPEINFO();

    SbUnoServiceCtor( const String& aName_, ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > xServiceCtorDesc );
    virtual ~SbUnoServiceCtor();
    virtual SbxInfo* GetInfo();

    ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XServiceConstructorDescription > getServiceCtorDesc( void )
        { return m_xServiceCtorDesc; }
};


// Wrapper for UNO Singleton
class SbUnoSingleton : public SbxObject
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XSingletonTypeDescription >   m_xSingletonTypeDesc;

public:
    TYPEINFO();
    SbUnoSingleton( const String& aName_,
        const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XSingletonTypeDescription >& xSingletonTypeDesc );

    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoSingleton);

SbUnoSingleton* findUnoSingleton( const String& rName );


// #105565 Special Object to wrap a strongly typed Uno Any
class SbUnoAnyObject: public SbxObject
{
    ::com::sun::star::uno::Any     mVal;

public:
    SbUnoAnyObject( const ::com::sun::star::uno::Any& rVal )
        : SbxObject( String() )
        , mVal( rVal )
    {}

    const ::com::sun::star::uno::Any& getValue( void )
        { return mVal; }

    TYPEINFO();
};


// #112509 Special SbxArray to transport named parameters for calls
// to OLEAutomation objects through the UNO OLE automation bridge

class AutomationNamedArgsSbxArray : public SbxArray
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >      maNameSeq;
public:
    TYPEINFO();
    AutomationNamedArgsSbxArray( sal_Int32 nSeqSize )
        : maNameSeq( nSeqSize )
    {}

    ::com::sun::star::uno::Sequence< ::rtl::OUString >& getNames( void )
        { return maNameSeq; }
};


class StarBASIC;

// Impl-Methoden fuer RTL
void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_CreateUnoServiceWithArguments( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );

void disposeComVariablesForBasic( StarBASIC* pBasic );
void clearNativeObjectWrapperVector( void );


//========================================================================
// #118116 Collection object

class BasicCollection : public SbxObject
{
    friend class SbiRuntime;
    SbxArrayRef xItemArray;
    static SbxInfoRef xAddInfo;
    static SbxInfoRef xItemInfo;

    void Initialize();
    virtual ~BasicCollection();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
    sal_Int32 implGetIndex( SbxVariable* pIndexVar );
    sal_Int32 implGetIndexForName( const String& rName );
    void CollAdd( SbxArray* pPar_ );
    void CollItem( SbxArray* pPar_ );
    void CollRemove( SbxArray* pPar_ );

public:
    TYPEINFO();
    BasicCollection( const String& rClassname );
    virtual SbxVariable* Find( const String&, SbxClassType );
    virtual void Clear();
};

#endif


