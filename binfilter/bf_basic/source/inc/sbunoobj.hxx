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

#include "sbxobj.hxx"
#include "sbxmeth.hxx"
#include "sbxprop.hxx"
#include "sbxfac.hxx"
#include "sbx.hxx"

#ifndef _COM_SUN_STAR_BEANS_XMATERIALHOLDER_HPP_
#include <com/sun/star/beans/XMaterialHolder.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XEXACTNAME_HPP_
#include <com/sun/star/beans/XExactName.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTIONACCESS_HPP_
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTION_HPP_
#include <com/sun/star/beans/XIntrospection.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XINVOCATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLCLASS_HPP_
#include <com/sun/star/reflection/XIdlClass.hpp>
#endif
#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

namespace binfilter {

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;

class SbUnoObject: public SbxObject
{
    Reference< XIntrospectionAccess > mxUnoAccess;
    Reference< XMaterialHolder > mxMaterialHolder;
    Reference< XInvocation > mxInvocation;
    Reference< XExactName > mxExactName;
    Reference< XExactName > mxExactNameInvocation;
    BOOL bNeedIntrospection;
    Any maTmpUnoObj;	// Only to save obj for doIntrospection!

    // Hilfs-Methode zum Anlegen der dbg_-Properties
    void implCreateDbgProperties( void );

    // Hilfs-Methode zum Anlegen aller Properties und Methoden
    // (Beim on-demand-Mechanismus erforderlich fuer die dbg_-Properties)
    void implCreateAll( void );

public:
    TYPEINFO();
    SbUnoObject( const String& aName_, const Any& aUnoObj_ );
    ~SbUnoObject();

    // #76470 Introspection on Demand durchfuehren
    void doIntrospection( void );

    // Find ueberladen, um z.B. NameAccess zu unterstuetzen
    virtual SbxVariable* Find( const String&, SbxClassType );

    // Force creation of all properties for debugging
    void createAllProperties( void  )
        { implCreateAll(); }

    // Wert rausgeben
    Any getUnoAny( void );
    Reference< XIntrospectionAccess > getIntrospectionAccess( void )	{ return mxUnoAccess; }
    Reference< XInvocation > getInvocation( void )			{ return mxInvocation; }

    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoObject);


class SbUnoMethod : public SbxMethod
{
    friend class SbUnoObject;

    Reference< XIdlMethod > m_xUnoMethod;
    Sequence<ParamInfo>* pParamInfoSeq;

    // #67781 Verweis auf vorige und naechste Methode in der Methoden-Liste
    SbUnoMethod* pPrev;
    SbUnoMethod* pNext;

    bool mbInvocation;		// Method is based on invocation

public:
    TYPEINFO();

    SbUnoMethod( const String& aName_, SbxDataType eSbxType, Reference< XIdlMethod > xUnoMethod_, 
        bool bInvocation );
    virtual ~SbUnoMethod();
    virtual SbxInfo* GetInfo();

    const Sequence<ParamInfo>& getParamInfos( void );

    bool isInvocationBased( void )
        { return mbInvocation; }
};


class SbUnoProperty : public SbxProperty
{
    friend class SbUnoObject;

    // Daten der Uno-Property
    Property aUnoProp;
    INT32 nId;

    bool mbInvocation;		// Property is based on invocation

    virtual ~SbUnoProperty();
public:
    TYPEINFO();
    SbUnoProperty( const String& aName_, SbxDataType eSbxType,
        const Property& aUnoProp_, INT32 nId_, bool bInvocation );

    bool isInvocationBased( void )
        { return mbInvocation; }
};

// Factory-Klasse fuer das Anlegen von Uno-Structs per DIM AS NEW
class SbUnoFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( UINT16 nSbxId, UINT32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

// Wrapper fuer eine Uno-Klasse
class SbUnoClass: public SbxObject
{
    const Reference< XIdlClass >	m_xClass;

public:
    TYPEINFO();
    SbUnoClass( const String& aName_ )
        : SbxObject( aName_ )
    {}
    SbUnoClass( const String& aName_, const Reference< XIdlClass >& xClass_ )
        : SbxObject( aName_ )
        , m_xClass( xClass_ )
    {}
    //~SbUnoClass();

    // Find ueberladen, um Elemente on Demand anzulegen
    virtual SbxVariable* Find( const String&, SbxClassType );

    // Wert rausgeben
    const Reference< XIdlClass >& getUnoClass( void ) { return m_xClass; }

    //void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoClass);


// Funktion, um einen globalen Bezeichner im
// UnoScope zu suchen und fuer Sbx zu wrappen
SbxVariable* findUnoClass( const String& rName );


// #105565 Special Object to wrap a strongly typed Uno Any
class SbUnoAnyObject: public SbxObject
{
    Any     mVal;

public:
    SbUnoAnyObject( const Any& rVal )
        : SbxObject( String() )
        , mVal( rVal )
    {}

    const Any& getValue( void )
        { return mVal; }

    TYPEINFO();
};


// #112509 Special SbxArray to transport named parameters for calls 
// to OLEAutomation objects through the UNO OLE automation bridge

class AutomationNamedArgsSbxArray : public SbxArray
{
    Sequence< ::rtl::OUString >		maNameSeq;
public:
    TYPEINFO();
    AutomationNamedArgsSbxArray( sal_Int32 nSeqSize )
        : maNameSeq( nSeqSize )
    {}

    Sequence< ::rtl::OUString >& getNames( void )
        { return maNameSeq; }
};


class StarBASIC;

//========================================================================
// #118116 Collection object

class BasicCollection : public SbxObject
{
    friend class SbiRuntime;
    SbxArrayRef xItemArray;

    void Initialize();
    virtual ~BasicCollection();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
    INT32 implGetIndex( SbxVariable* pIndexVar );
    INT32 implGetIndexForName( const String& rName );
    void CollAdd( SbxArray* pPar_ );
    void CollItem( SbxArray* pPar_ );
    void CollRemove( SbxArray* pPar_ );

public:
    TYPEINFO();
    BasicCollection( const String& rClassname );
    virtual SbxVariable* Find( const String&, SbxClassType );
    virtual void Clear();
};

}

#endif
