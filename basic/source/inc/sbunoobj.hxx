/*************************************************************************
 *
 *  $RCSfile: sbunoobj.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#ifndef SB_UNO_OBJ
#define SB_UNO_OBJ

#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <svtools/sbxobj.hxx>
#endif
#ifndef __SBX_SBXMETHOD_HXX //autogen
#include <svtools/sbxmeth.hxx>
#endif
#ifndef __SBX_SBXPROPERTY_HXX //autogen
#include <svtools/sbxprop.hxx>
#endif
#ifndef __SBX_SBX_FACTORY_HXX //autogen
#include <svtools/sbxfac.hxx>
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
    //virtual SbxInfo* GetInfo() { return NULL; }

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
    const Reference< XIdlClass > m_xClass;
public:
    TYPEINFO();
    SbUnoClass( const String& aName, const Reference< XIdlClass >& xClass_ )
        : SbxObject( aName ), m_xClass( xClass_ ) {}
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


class StarBASIC;

// Impl-Methoden fuer RTL
void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );

#endif


