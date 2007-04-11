/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rtti.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:17:17 $
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

#ifndef _RTTI_HXX
#define _RTTI_HXX

#include <string.h>

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

typedef void* (*TypeId)();

//-------------------------------------------------------------------------

#define TYPEINFO() \
        static  void*  CreateType(); \
        static  TypeId StaticType(); \
        static  BOOL   IsOf( TypeId aSameOrSuperType ); \
        virtual TypeId Type() const; \
        virtual BOOL   IsA( TypeId aSameOrSuperType ) const

#define TYPEINIT_FACTORY(sType, Factory ) \
        void*  sType::CreateType() { return Factory; } \
        TypeId sType::StaticType() { return &CreateType; } \
        TypeId sType::Type() const { return &CreateType; } \
        BOOL sType::IsOf( TypeId aSameOrSuperType ) \
        { \
            if ( aSameOrSuperType == StaticType() ) \
                return TRUE

#define STATICTYPE(sType) (sType::StaticType())

//-------------------------------------------------------------------------

#define TYPEINIT_AUTOFACTORY(sType) TYPEINIT_FACTORY(sType, new sType)
#define TYPEINIT(sType) TYPEINIT_FACTORY(sType, 0)

#define SUPERTYPE(sSuper) \
            if ( sSuper::IsOf(aSameOrSuperType ) ) \
                return TRUE

#define TYPEINIT_END(sType) \
            return FALSE; \
        } \
        BOOL sType::IsA( TypeId aSameOrSuperType ) const \
        { return IsOf( aSameOrSuperType ); }

#define TYPEINIT0_FACTORY(sType, Factory) \
        TYPEINIT_FACTORY(sType, Factory); \
        TYPEINIT_END(sType)
#define TYPEINIT0_AUTOFACTORY(sType) TYPEINIT0_FACTORY(sType, new sType)
#define TYPEINIT0(sType) TYPEINIT0_FACTORY(sType, 0)

#define TYPEINIT1_FACTORY(sType, sSuper, Factory) \
        TYPEINIT_FACTORY(sType, Factory); \
            SUPERTYPE(sSuper); \
        TYPEINIT_END(sType)
#define TYPEINIT1_AUTOFACTORY(sType, sSuper) \
            TYPEINIT1_FACTORY(sType, sSuper, new sType)
#define TYPEINIT1(sType, sSuper) \
            TYPEINIT1_FACTORY(sType, sSuper, 0)

#define TYPEINIT2_FACTORY(sType, sSuper1, sSuper2, Factory) \
        TYPEINIT_FACTORY(sType, Factory); \
            SUPERTYPE(sSuper1); \
            SUPERTYPE(sSuper2); \
        TYPEINIT_END(sType)
#define TYPEINIT2_AUTOFACTORY(sType, sSuper1, sSuper2) \
            TYPEINIT2_FACTORY(sType, sSuper1, sSuper2, new sType)
#define TYPEINIT2(sType, sSuper1, sSuper2) \
            TYPEINIT2_FACTORY(sType, sSuper1, sSuper2, 0)

#define TYPEINIT3_FACTORY(sType, sSuper1, sSuper2, sSuper3, Factory) \
        TYPEINIT_FACTORY(sType, Factory); \
            SUPERTYPE(sSuper1); \
            SUPERTYPE(sSuper2); \
            SUPERTYPE(sSuper3); \
        TYPEINIT_END(sType)
#define TYPEINIT3_AUTOFACTORY(sType, sSuper1, sSuper2, sSuper3) \
            TYPEINIT3_FACTORY(sType, sSuper1, sSuper2, sSuper3, new sType)
#define TYPEINIT3(sType, sSuper1, sSuper2, sSuper3) \
            TYPEINIT3_FACTORY(sType, sSuper1, sSuper2, sSuper3, 0)

#define TYPE(sType) (sType::StaticType())
#define ISA(sType) IsA(sType::StaticType())
#define ISOF(sType) IsOf(sType::StaticType())
#define CREATE(TypeId) (TypeId())

//-------------------------------------------------------------------------
// On-Demand-faehige persistent-TypeId Version

#define TYPEINFO_ID(id) \
        static  TypeId StaticType() { return (TypeId) ( id | 0xF000000L ); } \
        static  BOOL   IsOf( TypeId aSameOrSuperType ); \
        virtual TypeId Type() const; \
        virtual BOOL   IsA( TypeId aSameOrSuperType ) const

#define TYPEINIT_ID(sType) \
        TypeId sType::Type() const { return StaticType(); } \
        BOOL   sType::IsOf( TypeId aSameOrSuperType ) \
        { \
            if ( aSameOrSuperType == StaticType() ) \
                return TRUE

#define TYPEINIT0_ID(sType) \
        TYPEINIT_ID(sType); \
        TYPEINIT_END(sType)

#define TYPEINIT1_ID(sType, sSuper) \
        TYPEINIT_ID(sType); \
            SUPERTYPE(sSuper); \
        TYPEINIT_END(sType)

#define TYPEINIT2_ID(sType, sSuper1, sSuper2) \
        TYPEINIT_ID(sType); \
            SUPERTYPE(sSuper1); \
            SUPERTYPE(sSuper2); \
        TYPEINIT_END(sType)

//-------------------------------------------------------------------------

//      Die (exemplarischen) Makros fuer die Anwendung ( hier fuer
//      Pointer, kann aber nach dem gleichen Strickmuster fuer
//      Referenzen erweitert werden.
//      PTR_CAST: sicheres Casten eines Pointers auf einen Pointer
//      einer abgeleiteten Klasse. Liefert im Fehlerfall einen
//      Nullpointer (wahrscheinlich die haeufigste Anwendung)
//
//      T: Typ, auf den gecastet werden soll
//      p: Pointer, der gecastet werden soll
#define PTR_CAST( T, pObj ) \
        ( pObj && (pObj)->IsA( TYPE(T) ) ? (T*)(pObj) : 0 )

//      Abfrage, ob ein Objekt eine bestimmte Klasse als
//      Basisklasse hat (oder genau von dieser Klasse ist)
#define HAS_BASE( T, pObj ) \
        ( pObj && (pObj)->IsA( TYPE(T) ) )

//      Abfrage, ob ein Pointer auf ein Objekt eines bestimmten
//      Typs zeigt
#define IS_TYPE(T,pObj) \
        ( pObj && (pObj)->Type() == TYPE(T) )

#endif // _RTTI_HXX
