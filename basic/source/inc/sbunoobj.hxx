/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbunoobj.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-29 16:31:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SB_UNO_OBJ
#define SB_UNO_OBJ

#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <sbxobj.hxx>
#endif
#ifndef __SBX_SBXMETHOD_HXX //autogen
#include <sbxmeth.hxx>
#endif
#ifndef __SBX_SBXPROPERTY_HXX //autogen
#include <sbxprop.hxx>
#endif
#ifndef __SBX_SBX_FACTORY_HXX //autogen
#include <sbxfac.hxx>
#endif
#ifndef __SBX_SBX_HXX //autogen
#include <sbx.hxx>
#endif

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
#include <rtl/ustring>
#endif

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
    BOOL bNeedIntrospection;
    Any maTmpUnoObj;    // Only to save obj for doIntrospection!

    // Hilfs-Methode zum Anlegen der dbg_-Properties
    void implCreateDbgProperties( void );

    // Hilfs-Methode zum Anlegen aller Properties und Methoden
    // (Beim on-demand-Mechanismus erforderlich fuer die dbg_-Properties)
    void implCreateAll( void );

public:
    TYPEINFO();
    SbUnoObject( const String& aName, const Any& aUnoObj_ );
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
    Reference< XIntrospectionAccess > getIntrospectionAccess( void )    { return mxUnoAccess; }
    Reference< XInvocation > getInvocation( void )          { return mxInvocation; }

    void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
};
SV_DECL_IMPL_REF(SbUnoObject);


// #67781 Rueckgabewerte der Uno-Methoden loeschen
void clearUnoMethods( void );

class SbUnoMethod : public SbxMethod
{
    friend class SbUnoObject;
    friend void clearUnoMethods( void );

    Reference< XIdlMethod > m_xUnoMethod;
    Sequence<ParamInfo>* pParamInfoSeq;

    // #67781 Verweis auf vorige und naechste Methode in der Methoden-Liste
    SbUnoMethod* pPrev;
    SbUnoMethod* pNext;

public:
    TYPEINFO();

    SbUnoMethod( const String& aName, SbxDataType eSbxType, Reference< XIdlMethod > xUnoMethod_ );
    virtual ~SbUnoMethod();
    virtual SbxInfo* GetInfo();

    const Sequence<ParamInfo>& getParamInfos( void );
};


class SbUnoProperty : public SbxProperty
{
    friend class SbUnoObject;

    // Daten der Uno-Property
    Property aUnoProp;
    UINT32 nId;

    virtual ~SbUnoProperty();
public:
    TYPEINFO();
    SbUnoProperty( const String& aName, SbxDataType eSbxType,
        const Property& aUnoProp_, UINT32 nId_ );
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
    const Reference< XIdlClass >    m_xClass;

public:
    TYPEINFO();
    SbUnoClass( const String& aName )
        : SbxObject( aName )
    {}
    SbUnoClass( const String& aName, const Reference< XIdlClass >& xClass_ )
        : SbxObject( aName )
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
    Sequence< ::rtl::OUString >     maNameSeq;
public:
    TYPEINFO();
    AutomationNamedArgsSbxArray( sal_Int32 nSeqSize )
        : maNameSeq( nSeqSize )
    {}

    Sequence< ::rtl::OUString >& getNames( void )
        { return maNameSeq; }
};


class StarBASIC;

// Impl-Methoden fuer RTL
void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );


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
    void CollAdd( SbxArray* pPar );
    void CollItem( SbxArray* pPar );
    void CollRemove( SbxArray* pPar );

public:
    TYPEINFO();
    BasicCollection( const String& rClassname );
    virtual SbxVariable* Find( const String&, SbxClassType );
    virtual void Clear();
};

#endif


